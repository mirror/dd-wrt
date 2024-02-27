/*
     This file is part of libmicrohttpd
     Copyright (C) 2007-2021 Daniel Pittman and Christian Grothoff
     Copyright (C) 2015-2023 Evgeny Grin (Karlson2k)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
 * @file response.c
 * @brief  Methods for managing response objects
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#define MHD_NO_DEPRECATION 1

#include "mhd_options.h"
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif /* HAVE_SYS_IOCTL_H */
#if defined(_WIN32) && ! defined(__CYGWIN__)
#include <windows.h>
#endif /* _WIN32 && !__CYGWIN__ */

#include "internal.h"
#include "response.h"
#include "mhd_limits.h"
#include "mhd_sockets.h"
#include "mhd_itc.h"
#include "mhd_str.h"
#include "connection.h"
#include "memorypool.h"
#include "mhd_send.h"
#include "mhd_compat.h"
#include "mhd_assert.h"


#if defined(MHD_W32_MUTEX_)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif /* MHD_W32_MUTEX_ */
#if defined(_WIN32)
#include <io.h> /* for lseek(), read() */
#endif /* _WIN32 */


/**
 * Size of single file read operation for
 * file-backed responses.
 */
#ifndef MHD_FILE_READ_BLOCK_SIZE
#ifdef _WIN32
#define MHD_FILE_READ_BLOCK_SIZE 16384 /* 16k */
#else /* _WIN32 */
#define MHD_FILE_READ_BLOCK_SIZE 4096 /* 4k */
#endif /* _WIN32 */
#endif /* !MHD_FD_BLOCK_SIZE */

/**
 * Insert a new header at the first position of the response
 */
#define _MHD_insert_header_first(presponse, phdr) do { \
  mhd_assert (NULL == phdr->next); \
  mhd_assert (NULL == phdr->prev); \
  if (NULL == presponse->first_header) \
  { \
    mhd_assert (NULL == presponse->last_header); \
    presponse->first_header = phdr; \
    presponse->last_header = phdr;  \
  } \
  else \
  { \
    mhd_assert (NULL != presponse->last_header);        \
    presponse->first_header->prev = phdr; \
    phdr->next = presponse->first_header; \
    presponse->first_header = phdr;       \
  } \
} while (0)

/**
 * Insert a new header at the last position of the response
 */
#define _MHD_insert_header_last(presponse, phdr) do { \
  mhd_assert (NULL == phdr->next); \
  mhd_assert (NULL == phdr->prev); \
  if (NULL == presponse->last_header) \
  { \
    mhd_assert (NULL == presponse->first_header); \
    presponse->last_header = phdr;  \
    presponse->first_header = phdr; \
  } \
  else \
  { \
    mhd_assert (NULL != presponse->first_header);      \
    presponse->last_header->next = phdr; \
    phdr->prev = presponse->last_header; \
    presponse->last_header = phdr;       \
  } \
} while (0)


/**
 * Remove a header from the response
 */
#define _MHD_remove_header(presponse, phdr) do { \
  mhd_assert (NULL != presponse->first_header); \
  mhd_assert (NULL != presponse->last_header);  \
  if (NULL == phdr->prev) \
  { \
    mhd_assert (phdr == presponse->first_header); \
    presponse->first_header = phdr->next; \
  } \
  else \
  { \
    mhd_assert (phdr != presponse->first_header); \
    mhd_assert (phdr == phdr->prev->next); \
    phdr->prev->next = phdr->next; \
  } \
  if (NULL == phdr->next) \
  { \
    mhd_assert (phdr == presponse->last_header); \
    presponse->last_header = phdr->prev; \
  } \
  else \
  { \
    mhd_assert (phdr != presponse->last_header); \
    mhd_assert (phdr == phdr->next->prev); \
    phdr->next->prev = phdr->prev; \
  } \
} while (0)

/**
 * Add preallocated strings a header or footer line to the response without
 * checking.
 *
 * Header/footer strings are not checked and assumed to be correct.
 *
 * The string must not be statically allocated!
 * The strings must be malloc()'ed and zero terminated. The strings will
 * be free()'ed when the response is destroyed.
 *
 * @param response response to add a header to
 * @param kind header or footer
 * @param header the header string to add, must be malloc()'ed and
 *               zero-terminated
 * @param header_len the length of the @a header
 * @param content the value string to add, must be malloc()'ed and
 *                zero-terminated
 * @param content_len the length of the @a content
 */
bool
MHD_add_response_entry_no_alloc_ (struct MHD_Response *response,
                                  enum MHD_ValueKind kind,
                                  char *header,
                                  size_t header_len,
                                  char *content,
                                  size_t content_len)
{
  struct MHD_HTTP_Res_Header *hdr;

  mhd_assert (0 != header_len);
  mhd_assert (0 != content_len);
  if (NULL == (hdr = MHD_calloc_ (1, sizeof (struct MHD_HTTP_Res_Header))))
    return false;

  hdr->header = header;
  hdr->header_size = header_len;
  hdr->value = content;
  hdr->value_size = content_len;
  hdr->kind = kind;
  _MHD_insert_header_last (response, hdr);

  return true; /* Success exit point */
}


/**
 * Add a header or footer line to the response without checking.
 *
 * It is assumed that parameters are correct.
 *
 * @param response response to add a header to
 * @param kind header or footer
 * @param header the header to add, does not need to be zero-terminated
 * @param header_len the length of the @a header
 * @param content value to add, does not need to be zero-terminated
 * @param content_len the length of the @a content
 * @return false on error (like out-of-memory),
 *         true if succeed
 */
bool
MHD_add_response_entry_no_check_ (struct MHD_Response *response,
                                  enum MHD_ValueKind kind,
                                  const char *header,
                                  size_t header_len,
                                  const char *content,
                                  size_t content_len)
{
  char *header_malloced;
  char *value_malloced;

  mhd_assert (0 != header_len);
  mhd_assert (0 != content_len);
  header_malloced = malloc (header_len + 1);
  if (NULL == header_malloced)
    return false;

  memcpy (header_malloced, header, header_len);
  header_malloced[header_len] = 0;

  value_malloced = malloc (content_len + 1);
  if (NULL != value_malloced)
  {
    memcpy (value_malloced, content, content_len);
    value_malloced[content_len] = 0;

    if (MHD_add_response_entry_no_alloc_ (response, kind,
                                          header_malloced, header_len,
                                          value_malloced, content_len))
      return true; /* Success exit point */

    free (value_malloced);
  }
  free (header_malloced);

  return false; /* Failure exit point */
}


/**
 * Add a header or footer line to the response.
 *
 * @param header the header to add, does not need to be zero-terminated
 * @param header_len the length of the @a header
 * @param content value to add, does not need to be zero-terminated
 * @param content_len the length of the @a content
 * @return false on error (out-of-memory, invalid header or content),
 *         true if succeed
 */
static bool
add_response_entry_n (struct MHD_Response *response,
                      enum MHD_ValueKind kind,
                      const char *header,
                      size_t header_len,
                      const char *content,
                      size_t content_len)
{
  if (NULL == response)
    return false;
  if (0 == header_len)
    return false;
  if (0 == content_len)
    return false;
  if (NULL != memchr (header, '\t', header_len))
    return false;
  if (NULL != memchr (header, ' ', header_len))
    return false;
  if (NULL != memchr (header, '\r', header_len))
    return false;
  if (NULL != memchr (header, '\n', header_len))
    return false;
  if (NULL != memchr (content, '\r', content_len))
    return false;
  if (NULL != memchr (content, '\n', content_len))
    return false;

  return MHD_add_response_entry_no_check_ (response, kind, header, header_len,
                                           content, content_len);
}


/**
 * Add a header or footer line to the response.
 *
 * @param response response to add a header to
 * @param kind header or footer
 * @param header the header to add
 * @param content value to add
 * @return #MHD_NO on error (i.e. invalid header or content format).
 */
static enum MHD_Result
add_response_entry (struct MHD_Response *response,
                    enum MHD_ValueKind kind,
                    const char *header,
                    const char *content)
{
  size_t header_len;
  size_t content_len;

  if (NULL == content)
    return MHD_NO;

  header_len = strlen (header);
  content_len = strlen (content);
  return add_response_entry_n (response, kind, header,
                               header_len, content,
                               content_len) ? MHD_YES : MHD_NO;
}


/**
 * Add "Connection:" header to the response with special processing.
 *
 * "Connection:" header value will be combined with any existing "Connection:"
 * header, "close" token (if any) will be de-duplicated and moved to the first
 * position.
 *
 * @param response the response to add a header to
 * @param value the value to add
 * @return #MHD_NO on error (no memory).
 */
static enum MHD_Result
add_response_header_connection (struct MHD_Response *response,
                                const char *value)
{
  static const char *key = MHD_HTTP_HEADER_CONNECTION;
  /** the length of the "Connection" key */
  static const size_t key_len =
    MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONNECTION);
  size_t value_len; /**< the length of the @a value */
  size_t old_value_len; /**< the length of the existing "Connection" value */
  size_t buf_size;  /**< the size of the buffer */
  size_t norm_len;  /**< the length of the normalised value */
  char *buf;        /**< the temporal buffer */
  struct MHD_HTTP_Res_Header *hdr; /**< existing "Connection" header */
  bool value_has_close; /**< the @a value has "close" token */
  bool already_has_close; /**< existing "Connection" header has "close" token */
  size_t pos = 0;   /**< position of addition in the @a buf */

  if ( (NULL != strchr (value, '\r')) ||
       (NULL != strchr (value, '\n')) )
    return MHD_NO;

  if (0 != (response->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
  {
    hdr = MHD_get_response_element_n_ (response, MHD_HEADER_KIND,
                                       key, key_len);
    already_has_close =
      (0 != (response->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE));
    mhd_assert (already_has_close == (0 == memcmp (hdr->value, "close", 5)));
    mhd_assert (NULL != hdr);
  }
  else
  {
    hdr = NULL;
    already_has_close = false;
    mhd_assert (NULL == MHD_get_response_element_n_ (response,
                                                     MHD_HEADER_KIND,
                                                     key, key_len));
    mhd_assert (0 == (response->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE));
  }
  if (NULL != hdr)
    old_value_len = hdr->value_size + 2; /* additional size for ", " */
  else
    old_value_len = 0;

  value_len = strlen (value);
  if (value_len >= SSIZE_MAX)
    return MHD_NO;
  /* Additional space for normalisation and zero-termination */
  norm_len = value_len + value_len / 2 + 1;
  if (norm_len >= SSIZE_MAX)
    return MHD_NO;
  buf_size = old_value_len + (size_t) norm_len;

  buf = malloc (buf_size);
  if (NULL == buf)
    return MHD_NO;
  if (1)
  { /* local scope */
    ssize_t norm_len_s = (ssize_t) norm_len;
    /* Remove "close" token (if any), it will be moved to the front */
    value_has_close = MHD_str_remove_token_caseless_ (value, value_len, "close",
                                                      MHD_STATICSTR_LEN_ ( \
                                                        "close"),
                                                      buf + old_value_len,
                                                      &norm_len_s);
    mhd_assert (0 <= norm_len_s);
    if (0 > norm_len_s)
    {
      /* Must never happen with realistic sizes */
      free (buf);
      return MHD_NO;
    }
    else
      norm_len = (size_t) norm_len_s;
  }
#ifdef UPGRADE_SUPPORT
  if ( (NULL != response->upgrade_handler) && value_has_close)
  { /* The "close" token cannot be used with connection "upgrade" */
    free (buf);
    return MHD_NO;
  }
#endif /* UPGRADE_SUPPORT */
  if (0 != norm_len)
    MHD_str_remove_tokens_caseless_ (buf + old_value_len, &norm_len,
                                     "keep-alive",
                                     MHD_STATICSTR_LEN_ ("keep-alive"));
  if (0 == norm_len)
  { /* New value is empty after normalisation */
    if (! value_has_close)
    { /* The new value had no tokens */
      free (buf);
      return MHD_NO;
    }
    if (already_has_close)
    { /* The "close" token is already present, nothing to modify */
      free (buf);
      return MHD_YES;
    }
  }
  /* Add "close" token if required */
  if (value_has_close && ! already_has_close)
  {
    /* Need to insert "close" token at the first position */
    mhd_assert (buf_size >= old_value_len + norm_len   \
                + MHD_STATICSTR_LEN_ ("close, ") + 1);
    if (0 != norm_len)
      memmove (buf + MHD_STATICSTR_LEN_ ("close, ") + old_value_len,
               buf + old_value_len, norm_len + 1);
    memcpy (buf, "close", MHD_STATICSTR_LEN_ ("close"));
    pos += MHD_STATICSTR_LEN_ ("close");
  }
  /* Add old value tokens (if any) */
  if (0 != old_value_len)
  {
    if (0 != pos)
    {
      buf[pos++] = ',';
      buf[pos++] = ' ';
    }
    memcpy (buf + pos, hdr->value,
            hdr->value_size);
    pos += hdr->value_size;
  }
  /* Add new value token (if any) */
  if (0 != norm_len)
  {
    if (0 != pos)
    {
      buf[pos++] = ',';
      buf[pos++] = ' ';
    }
    /* The new value tokens must be already at the correct position */
    mhd_assert ((value_has_close && ! already_has_close) ? \
                (MHD_STATICSTR_LEN_ ("close, ") + old_value_len == pos) : \
                (old_value_len == pos));
    pos += norm_len;
  }
  mhd_assert (buf_size > pos);
  buf[pos] = 0; /* Null terminate the result */

  if (NULL == hdr)
  {
    struct MHD_HTTP_Res_Header *new_hdr; /**< new "Connection" header */
    /* Create new response header entry */
    new_hdr = MHD_calloc_ (1, sizeof (struct MHD_HTTP_Res_Header));
    if (NULL != new_hdr)
    {
      new_hdr->header = malloc (key_len + 1);
      if (NULL != new_hdr->header)
      {
        memcpy (new_hdr->header, key, key_len + 1);
        new_hdr->header_size = key_len;
        new_hdr->value = buf;
        new_hdr->value_size = pos;
        new_hdr->kind = MHD_HEADER_KIND;
        if (value_has_close)
          response->flags_auto = (MHD_RAF_HAS_CONNECTION_HDR
                                  | MHD_RAF_HAS_CONNECTION_CLOSE);
        else
          response->flags_auto = MHD_RAF_HAS_CONNECTION_HDR;
        _MHD_insert_header_first (response, new_hdr);
        return MHD_YES;
      }
      free (new_hdr);
    }
    free (buf);
    return MHD_NO;
  }

  /* Update existing header entry */
  free (hdr->value);
  hdr->value = buf;
  hdr->value_size = pos;
  if (value_has_close && ! already_has_close)
    response->flags_auto |= MHD_RAF_HAS_CONNECTION_CLOSE;
  return MHD_YES;
}


/**
 * Remove tokens from "Connection:" header of the response.
 *
 * Provided tokens will be removed from "Connection:" header value.
 *
 * @param response the response to manipulate "Connection:" header
 * @param value the tokens to remove
 * @return #MHD_NO on error (no headers or tokens found).
 */
static enum MHD_Result
del_response_header_connection (struct MHD_Response *response,
                                const char *value)
{
  struct MHD_HTTP_Res_Header *hdr; /**< existing "Connection" header */

  hdr = MHD_get_response_element_n_ (response, MHD_HEADER_KIND,
                                     MHD_HTTP_HEADER_CONNECTION,
                                     MHD_STATICSTR_LEN_ ( \
                                       MHD_HTTP_HEADER_CONNECTION));
  if (NULL == hdr)
    return MHD_NO;

  if (! MHD_str_remove_tokens_caseless_ (hdr->value, &hdr->value_size, value,
                                         strlen (value)))
    return MHD_NO;
  if (0 == hdr->value_size)
  {
    _MHD_remove_header (response, hdr);
    free (hdr->value);
    free (hdr->header);
    free (hdr);
    response->flags_auto &=
      ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONNECTION_HDR
        | (enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONNECTION_CLOSE);
  }
  else
  {
    hdr->value[hdr->value_size] = 0; /* Null-terminate the result */
    if (0 != (response->flags_auto
              & ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONNECTION_CLOSE)))
    {
      if (MHD_STATICSTR_LEN_ ("close") == hdr->value_size)
      {
        if (0 != memcmp (hdr->value, "close", MHD_STATICSTR_LEN_ ("close")))
          response->flags_auto &=
            ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONNECTION_CLOSE);
      }
      else if (MHD_STATICSTR_LEN_ ("close, ") < hdr->value_size)
      {
        if (0 != memcmp (hdr->value, "close, ",
                         MHD_STATICSTR_LEN_ ("close, ")))
          response->flags_auto &=
            ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONNECTION_CLOSE);
      }
      else
        response->flags_auto &=
          ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONNECTION_CLOSE);
    }
  }
  return MHD_YES;
}


/**
 * Add a header line to the response.
 *
 * When reply is generated with queued response, some headers are generated
 * automatically. Automatically generated headers are only sent to the client,
 * but not added back to the response object.
 *
 * The list of automatic headers:
 * + "Date" header is added automatically unless already set by
 *   this function
 *   @see #MHD_USE_SUPPRESS_DATE_NO_CLOCK
 * + "Content-Length" is added automatically when required, attempt to set
 *   it manually by this function is ignored.
 *   @see #MHD_RF_INSANITY_HEADER_CONTENT_LENGTH
 * + "Transfer-Encoding" with value "chunked" is added automatically,
 *   when chunked transfer encoding is used automatically. Same header with
 *   the same value can be set manually by this function to enforce chunked
 *   encoding, however for HTTP/1.0 clients chunked encoding will not be used
 *   and manually set "Transfer-Encoding" header is automatically removed
 *   for HTTP/1.0 clients
 * + "Connection" may be added automatically with value "Keep-Alive" (only
 *   for HTTP/1.0 clients) or "Close". The header "Connection" with value
 *   "Close" could be set by this function to enforce closure of
 *   the connection after sending this response. "Keep-Alive" cannot be
 *   enforced and will be removed automatically.
 *   @see #MHD_RF_SEND_KEEP_ALIVE_HEADER
 *
 * Some headers are pre-processed by this function:
 * * "Connection" headers are combined into single header entry, value is
 *   normilised, "Keep-Alive" tokens are removed.
 * * "Transfer-Encoding" header: the only one header is allowed, the only
 *   allowed value is "chunked".
 * * "Date" header: the only one header is allowed, the second added header
 *   replaces the first one.
 * * "Content-Length" application-defined header is not allowed.
 *   @see #MHD_RF_INSANITY_HEADER_CONTENT_LENGTH
 *
 * Headers are used in order as they were added.
 *
 * @param response the response to add a header to
 * @param header the header name to add, no need to be static, an internal copy
 *               will be created automatically
 * @param content the header value to add, no need to be static, an internal
 *                copy will be created automatically
 * @return #MHD_YES on success,
 *         #MHD_NO on error (i.e. invalid header or content format),
 *         or out of memory
 * @ingroup response
 */
_MHD_EXTERN enum MHD_Result
MHD_add_response_header (struct MHD_Response *response,
                         const char *header,
                         const char *content)
{
  if (MHD_str_equal_caseless_ (header, MHD_HTTP_HEADER_CONNECTION))
    return add_response_header_connection (response, content);

  if (MHD_str_equal_caseless_ (header,
                               MHD_HTTP_HEADER_TRANSFER_ENCODING))
  {
    if (! MHD_str_equal_caseless_ (content, "chunked"))
      return MHD_NO;   /* Only "chunked" encoding is allowed */
    if (0 != (response->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED))
      return MHD_YES;  /* Already has "chunked" encoding header */
    if ( (0 != (response->flags_auto & MHD_RAF_HAS_CONTENT_LENGTH)) &&
         (0 == (MHD_RF_INSANITY_HEADER_CONTENT_LENGTH & response->flags)) )
      return MHD_NO; /* Has "Content-Length" header and no "Insanity" flag */
    if (MHD_NO != add_response_entry (response,
                                      MHD_HEADER_KIND,
                                      header,
                                      content))
    {
      response->flags_auto |= MHD_RAF_HAS_TRANS_ENC_CHUNKED;
      return MHD_YES;
    }
    return MHD_NO;
  }
  if (MHD_str_equal_caseless_ (header,
                               MHD_HTTP_HEADER_DATE))
  {
    if (0 != (response->flags_auto & MHD_RAF_HAS_DATE_HDR))
    {
      struct MHD_HTTP_Res_Header *hdr;
      hdr = MHD_get_response_element_n_ (response, MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_DATE,
                                         MHD_STATICSTR_LEN_ ( \
                                           MHD_HTTP_HEADER_DATE));
      mhd_assert (NULL != hdr);
      _MHD_remove_header (response, hdr);
      if (NULL != hdr->value)
        free (hdr->value);
      free (hdr->header);
      free (hdr);
    }
    if (MHD_NO != add_response_entry (response,
                                      MHD_HEADER_KIND,
                                      header,
                                      content))
    {
      response->flags_auto |= MHD_RAF_HAS_DATE_HDR;
      return MHD_YES;
    }
    return MHD_NO;
  }

  if (MHD_str_equal_caseless_ (header,
                               MHD_HTTP_HEADER_CONTENT_LENGTH))
  {
    /* Generally MHD sets automatically correct "Content-Length" always when
     * needed.
     * Custom "Content-Length" header is allowed only in special cases. */
    if ( (0 != (MHD_RF_INSANITY_HEADER_CONTENT_LENGTH & response->flags)) ||
         ((0 != (MHD_RF_HEAD_ONLY_RESPONSE & response->flags)) &&
          (0 == (response->flags_auto & (MHD_RAF_HAS_TRANS_ENC_CHUNKED
                                         | MHD_RAF_HAS_CONTENT_LENGTH)))) )
    {
      if (MHD_NO != add_response_entry (response,
                                        MHD_HEADER_KIND,
                                        header,
                                        content))
      {
        response->flags_auto |= MHD_RAF_HAS_CONTENT_LENGTH;
        return MHD_YES;
      }
    }
    return MHD_NO;
  }

  return add_response_entry (response,
                             MHD_HEADER_KIND,
                             header,
                             content);
}


/**
 * Add a footer line to the response.
 *
 * @param response response to remove a header from
 * @param footer the footer to delete
 * @param content value to delete
 * @return #MHD_NO on error (i.e. invalid footer or content format).
 * @ingroup response
 */
_MHD_EXTERN enum MHD_Result
MHD_add_response_footer (struct MHD_Response *response,
                         const char *footer,
                         const char *content)
{
  return add_response_entry (response,
                             MHD_FOOTER_KIND,
                             footer,
                             content);
}


/**
 * Delete a header (or footer) line from the response.
 *
 * For "Connection" headers this function remove all tokens from existing
 * value. Successful result means that at least one token has been removed.
 * If all tokens are removed from "Connection" header, the empty "Connection"
 * header removed.
 *
 * @param response response to remove a header from
 * @param header the header to delete
 * @param content value to delete
 * @return #MHD_NO on error (no such header known)
 * @ingroup response
 */
_MHD_EXTERN enum MHD_Result
MHD_del_response_header (struct MHD_Response *response,
                         const char *header,
                         const char *content)
{
  struct MHD_HTTP_Res_Header *pos;
  size_t header_len;
  size_t content_len;

  if ( (NULL == header) ||
       (NULL == content) )
    return MHD_NO;
  header_len = strlen (header);

  if ((0 != (response->flags_auto & MHD_RAF_HAS_CONNECTION_HDR)) &&
      (MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONNECTION) == header_len) &&
      MHD_str_equal_caseless_bin_n_ (header, MHD_HTTP_HEADER_CONNECTION,
                                     header_len))
    return del_response_header_connection (response, content);

  content_len = strlen (content);
  pos = response->first_header;
  while (NULL != pos)
  {
    if ((header_len == pos->header_size) &&
        (content_len == pos->value_size) &&
        (0 == memcmp (header,
                      pos->header,
                      header_len)) &&
        (0 == memcmp (content,
                      pos->value,
                      content_len)))
    {
      _MHD_remove_header (response, pos);
      free (pos->header);
      free (pos->value);
      free (pos);
      if ( (MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_TRANSFER_ENCODING) ==
            header_len) &&
           MHD_str_equal_caseless_bin_n_ (header,
                                          MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                          header_len) )
        response->flags_auto &=
          ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_TRANS_ENC_CHUNKED);
      else if ( (MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_DATE) ==
                 header_len) &&
                MHD_str_equal_caseless_bin_n_ (header,
                                               MHD_HTTP_HEADER_DATE,
                                               header_len) )
        response->flags_auto &=
          ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_DATE_HDR);
      else if ( (MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH) ==
                 header_len) &&
                MHD_str_equal_caseless_bin_n_ (header,
                                               MHD_HTTP_HEADER_CONTENT_LENGTH,
                                               header_len) )
      {
        if (NULL == MHD_get_response_element_n_ (response,
                                                 MHD_HEADER_KIND,
                                                 MHD_HTTP_HEADER_CONTENT_LENGTH,
                                                 header_len))
          response->flags_auto &=
            ~((enum MHD_ResponseAutoFlags) MHD_RAF_HAS_CONTENT_LENGTH);
      }
      return MHD_YES;
    }
    pos = pos->next;
  }
  return MHD_NO;
}


/**
 * Get all of the headers (and footers) added to a response.
 *
 * @param response response to query
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over
 * @ingroup response
 */
_MHD_EXTERN int
MHD_get_response_headers (struct MHD_Response *response,
                          MHD_KeyValueIterator iterator,
                          void *iterator_cls)
{
  int numHeaders = 0;
  struct MHD_HTTP_Res_Header *pos;

  for (pos = response->first_header;
       NULL != pos;
       pos = pos->next)
  {
    numHeaders++;
    if ((NULL != iterator) &&
        (MHD_NO == iterator (iterator_cls,
                             pos->kind,
                             pos->header,
                             pos->value)))
      break;
  }
  return numHeaders;
}


/**
 * Get a particular header (or footer) from the response.
 *
 * @param response response to query
 * @param key which header to get
 * @return NULL if header does not exist
 * @ingroup response
 */
_MHD_EXTERN const char *
MHD_get_response_header (struct MHD_Response *response,
                         const char *key)
{
  struct MHD_HTTP_Res_Header *pos;
  size_t key_size;

  if (NULL == key)
    return NULL;

  key_size = strlen (key);
  for (pos = response->first_header;
       NULL != pos;
       pos = pos->next)
  {
    if ((pos->header_size == key_size) &&
        (MHD_str_equal_caseless_bin_n_ (pos->header, key, pos->header_size)))
      return pos->value;
  }
  return NULL;
}


/**
 * Get a particular header (or footer) element from the response.
 *
 * Function returns the first found element.
 * @param response response to query
 * @param kind the kind of element: header or footer
 * @param key the key which header to get
 * @param key_len the length of the @a key
 * @return NULL if header element does not exist
 * @ingroup response
 */
struct MHD_HTTP_Res_Header *
MHD_get_response_element_n_ (struct MHD_Response *response,
                             enum MHD_ValueKind kind,
                             const char *key,
                             size_t key_len)
{
  struct MHD_HTTP_Res_Header *pos;

  mhd_assert (NULL != key);
  mhd_assert (0 != key[0]);
  mhd_assert (0 != key_len);

  for (pos = response->first_header;
       NULL != pos;
       pos = pos->next)
  {
    if ((pos->header_size == key_len) &&
        (kind == pos->kind) &&
        (MHD_str_equal_caseless_bin_n_ (pos->header, key, pos->header_size)))
      return pos;
  }
  return NULL;
}


/**
 * Check whether response header contains particular token.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 *
 * @param response  the response to query
 * @param key       header name
 * @param key_len   the length of @a key, not including optional
 *                  terminating null-character.
 * @param token     the token to find
 * @param token_len the length of @a token, not including optional
 *                  terminating null-character.
 * @return true if token is found in specified header,
 *         false otherwise
 */
bool
MHD_check_response_header_token_ci (const struct MHD_Response *response,
                                    const char *key,
                                    size_t key_len,
                                    const char *token,
                                    size_t token_len)
{
  struct MHD_HTTP_Res_Header *pos;

  if ( (NULL == key) ||
       ('\0' == key[0]) ||
       (NULL == token) ||
       ('\0' == token[0]) )
    return false;

  /* Token must not contain binary zero! */
  mhd_assert (strlen (token) == token_len);

  for (pos = response->first_header;
       NULL != pos;
       pos = pos->next)
  {
    if ( (pos->kind == MHD_HEADER_KIND) &&
         (key_len == pos->header_size) &&
         MHD_str_equal_caseless_bin_n_ (pos->header,
                                        key,
                                        key_len) &&
         MHD_str_has_token_caseless_ (pos->value,
                                      token,
                                      token_len) )
      return true;
  }
  return false;
}


/**
 * Create a response object.
 * The response object can be extended with header information and then be used
 * any number of times.
 *
 * If response object is used to answer HEAD request then the body of the
 * response is not used, while all headers (including automatic headers) are
 * used.
 *
 * @param size size of the data portion of the response, #MHD_SIZE_UNKNOWN for unknown
 * @param block_size preferred block size for querying crc (advisory only,
 *                   MHD may still call @a crc using smaller chunks); this
 *                   is essentially the buffer size used for IO, clients
 *                   should pick a value that is appropriate for IO and
 *                   memory performance requirements
 * @param crc callback to use to obtain response data
 * @param crc_cls extra argument to @a crc
 * @param crfc callback to call to free @a crc_cls resources
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_callback (uint64_t size,
                                   size_t block_size,
                                   MHD_ContentReaderCallback crc,
                                   void *crc_cls,
                                   MHD_ContentReaderFreeCallback crfc)
{
  struct MHD_Response *response;

  if ((NULL == crc) || (0 == block_size))
    return NULL;
  if (NULL == (response = MHD_calloc_ (1, sizeof (struct MHD_Response)
                                       + block_size)))
    return NULL;
  response->fd = -1;
  response->data = (void *) &response[1];
  response->data_buffer_size = block_size;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (! MHD_mutex_init_ (&response->mutex))
  {
    free (response);
    return NULL;
  }
#endif
  response->crc = crc;
  response->crfc = crfc;
  response->crc_cls = crc_cls;
  response->reference_count = 1;
  response->total_size = size;
  return response;
}


/**
 * Set special flags and options for a response.
 *
 * @param response the response to modify
 * @param flags to set for the response
 * @param ... #MHD_RO_END terminated list of options
 * @return #MHD_YES on success, #MHD_NO on error
 */
_MHD_EXTERN enum MHD_Result
MHD_set_response_options (struct MHD_Response *response,
                          enum MHD_ResponseFlags flags,
                          ...)
{
  va_list ap;
  enum MHD_Result ret;
  enum MHD_ResponseOptions ro;

  if (0 != (response->flags_auto & MHD_RAF_HAS_CONTENT_LENGTH))
  { /* Response has custom "Content-Lengh" header */
    if ( (0 != (response->flags & MHD_RF_INSANITY_HEADER_CONTENT_LENGTH)) &&
         (0 == (flags & MHD_RF_INSANITY_HEADER_CONTENT_LENGTH)))
    { /* Request to remove MHD_RF_INSANITY_HEADER_CONTENT_LENGTH flag */
      return MHD_NO;
    }
    if ( (0 != (response->flags & MHD_RF_HEAD_ONLY_RESPONSE)) &&
         (0 == (flags & MHD_RF_HEAD_ONLY_RESPONSE)))
    { /* Request to remove MHD_RF_HEAD_ONLY_RESPONSE flag */
      if (0 == (flags & MHD_RF_INSANITY_HEADER_CONTENT_LENGTH))
        return MHD_NO;
    }
  }

  if ( (0 != (flags & MHD_RF_HEAD_ONLY_RESPONSE)) &&
       (0 != response->total_size) )
    return MHD_NO;

  ret = MHD_YES;
  response->flags = flags;

  va_start (ap, flags);
  while (MHD_RO_END != (ro = va_arg (ap, enum MHD_ResponseOptions)))
  {
    switch (ro)
    {
    case MHD_RO_END: /* Not possible */
      break;
    default:
      ret = MHD_NO;
      break;
    }
  }
  va_end (ap);
  return ret;
}


/**
 * Given a file descriptor, read data from the file
 * to generate the response.
 *
 * @param cls pointer to the response
 * @param pos offset in the file to access
 * @param buf where to write the data
 * @param max number of bytes to write at most
 * @return number of bytes written
 */
static ssize_t
file_reader (void *cls,
             uint64_t pos,
             char *buf,
             size_t max)
{
  struct MHD_Response *response = cls;
#if ! defined(_WIN32) || defined(__CYGWIN__)
  ssize_t n;
#else  /* _WIN32 && !__CYGWIN__ */
  const HANDLE fh = (HANDLE) (uintptr_t) _get_osfhandle (response->fd);
#endif /* _WIN32 && !__CYGWIN__ */
  const int64_t offset64 = (int64_t) (pos + response->fd_off);

  if (offset64 < 0)
    return MHD_CONTENT_READER_END_WITH_ERROR; /* seek to required position is not possible */

#if ! defined(_WIN32) || defined(__CYGWIN__)
  if (max > SSIZE_MAX)
    max = SSIZE_MAX; /* Clamp to maximum return value. */

#if defined(HAVE_PREAD64)
  n = pread64 (response->fd, buf, max, offset64);
#elif defined(HAVE_PREAD)
  if ( (sizeof(off_t) < sizeof (uint64_t)) &&
       (offset64 > (uint64_t) INT32_MAX) )
    return MHD_CONTENT_READER_END_WITH_ERROR; /* Read at required position is not possible. */

  n = pread (response->fd, buf, max, (off_t) offset64);
#else  /* ! HAVE_PREAD */
#if defined(HAVE_LSEEK64)
  if (lseek64 (response->fd,
               offset64,
               SEEK_SET) != offset64)
    return MHD_CONTENT_READER_END_WITH_ERROR; /* can't seek to required position */
#else  /* ! HAVE_LSEEK64 */
  if ( (sizeof(off_t) < sizeof (uint64_t)) &&
       (offset64 > (uint64_t) INT32_MAX) )
    return MHD_CONTENT_READER_END_WITH_ERROR; /* seek to required position is not possible */

  if (lseek (response->fd,
             (off_t) offset64,
             SEEK_SET) != (off_t) offset64)
    return MHD_CONTENT_READER_END_WITH_ERROR; /* can't seek to required position */
#endif /* ! HAVE_LSEEK64 */
  n = read (response->fd,
            buf,
            max);

#endif /* ! HAVE_PREAD */
  if (0 == n)
    return MHD_CONTENT_READER_END_OF_STREAM;
  if (n < 0)
    return MHD_CONTENT_READER_END_WITH_ERROR;
  return n;
#else /* _WIN32 && !__CYGWIN__ */
  if (INVALID_HANDLE_VALUE == fh)
    return MHD_CONTENT_READER_END_WITH_ERROR; /* Value of 'response->fd' is not valid. */
  else
  {
    OVERLAPPED f_ol = {0, 0, {{0, 0}}, 0};   /* Initialize to zero. */
    ULARGE_INTEGER pos_uli;
    DWORD toRead = (max > INT32_MAX) ? INT32_MAX : (DWORD) max;
    DWORD resRead;

    pos_uli.QuadPart = (uint64_t) offset64;   /* Simple transformation 64bit -> 2x32bit. */
    f_ol.Offset = pos_uli.LowPart;
    f_ol.OffsetHigh = pos_uli.HighPart;
    if (! ReadFile (fh, (void *) buf, toRead, &resRead, &f_ol))
      return MHD_CONTENT_READER_END_WITH_ERROR;   /* Read error. */
    if (0 == resRead)
      return MHD_CONTENT_READER_END_OF_STREAM;
    return (ssize_t) resRead;
  }
#endif /* _WIN32 && !__CYGWIN__ */
}


/**
 * Given a pipe descriptor, read data from the pipe
 * to generate the response.
 *
 * @param cls pointer to the response
 * @param pos offset in the pipe to access (ignored)
 * @param buf where to write the data
 * @param max number of bytes to write at most
 * @return number of bytes written
 */
static ssize_t
pipe_reader (void *cls,
             uint64_t pos,
             char *buf,
             size_t max)
{
  struct MHD_Response *response = cls;
  ssize_t n;

  (void) pos;

#ifndef _WIN32
  if (SSIZE_MAX < max)
    max = SSIZE_MAX;
  n = read (response->fd,
            buf,
            (MHD_SCKT_SEND_SIZE_) max);
#else  /* _WIN32 */
  if (UINT_MAX < max)
    max = INT_MAX;
  n = read (response->fd,
            buf,
            (unsigned int) max);
#endif /* _WIN32 */

  if (0 == n)
    return MHD_CONTENT_READER_END_OF_STREAM;
  if (n < 0)
    return MHD_CONTENT_READER_END_WITH_ERROR;
  return n;
}


/**
 * Destroy file reader context.  Closes the file
 * descriptor.
 *
 * @param cls pointer to file descriptor
 */
static void
free_callback (void *cls)
{
  struct MHD_Response *response = cls;

  (void) close (response->fd);
  response->fd = -1;
}


#undef MHD_create_response_from_fd_at_offset

/**
 * Create a response object with the content of provided file with
 * specified offset used as the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @param offset offset to start reading from in the file;
 *        Be careful! `off_t` may have been compiled to be a
 *        64-bit variable for MHD, in which case your application
 *        also has to be compiled using the same options! Read
 *        the MHD manual for more details.
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd_at_offset (size_t size,
                                       int fd,
                                       off_t offset)
{
  if (0 > offset)
    return NULL;
  return MHD_create_response_from_fd_at_offset64 (size,
                                                  fd,
                                                  (uint64_t) offset);
}


/**
 * Create a response object with the content of provided file with
 * specified offset used as the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response;
 *        sizes larger than 2 GiB may be not supported by OS or
 *        MHD build; see ::MHD_FEATURE_LARGE_FILE
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @param offset offset to start reading from in the file;
 *        reading file beyond 2 GiB may be not supported by OS or
 *        MHD build; see ::MHD_FEATURE_LARGE_FILE
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd_at_offset64 (uint64_t size,
                                         int fd,
                                         uint64_t offset)
{
  struct MHD_Response *response;

#if ! defined(HAVE___LSEEKI64) && ! defined(HAVE_LSEEK64)
  if ( (sizeof(uint64_t) > sizeof(off_t)) &&
       ( (size > (uint64_t) INT32_MAX) ||
         (offset > (uint64_t) INT32_MAX) ||
         ((size + offset) >= (uint64_t) INT32_MAX) ) )
    return NULL;
#endif
  if ( ((int64_t) size < 0) ||
       ((int64_t) offset < 0) ||
       ((int64_t) (size + offset) < 0) )
    return NULL;

  response = MHD_create_response_from_callback (size,
                                                MHD_FILE_READ_BLOCK_SIZE,
                                                &file_reader,
                                                NULL,
                                                &free_callback);
  if (NULL == response)
    return NULL;
  response->fd = fd;
  response->is_pipe = false;
  response->fd_off = offset;
  response->crc_cls = response;
  return response;
}


/**
 * Create a response object with the response body created by reading
 * the provided pipe.
 *
 * The response object can be extended with header information and
 * then be used ONLY ONCE.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param fd file descriptor referring to a read-end of a pipe with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_pipe (int fd)
{
  struct MHD_Response *response;

  response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                MHD_FILE_READ_BLOCK_SIZE,
                                                &pipe_reader,
                                                NULL,
                                                &free_callback);
  if (NULL == response)
    return NULL;
  response->fd = fd;
  response->is_pipe = true;
  response->crc_cls = response;
  return response;
}


/**
 * Create a response object with the content of provided file used as
 * the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response
 * @param fd file descriptor referring to a file on disk with the data
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd (size_t size,
                             int fd)
{
  return MHD_create_response_from_fd_at_offset64 (size,
                                                  fd,
                                                  0);
}


/**
 * Create a response object with the content of provided file used as
 * the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response;
 *        sizes larger than 2 GiB may be not supported by OS or
 *        MHD build; see ::MHD_FEATURE_LARGE_FILE
 * @param fd file descriptor referring to a file on disk with the
 *        data; will be closed when response is destroyed;
 *        fd should be in 'blocking' mode
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_fd64 (uint64_t size,
                               int fd)
{
  return MHD_create_response_from_fd_at_offset64 (size,
                                                  fd,
                                                  0);
}


/**
 * Create a response object.
 * The response object can be extended with header information and then be used
 * any number of times.
 *
 * If response object is used to answer HEAD request then the body of the
 * response is not used, while all headers (including automatic headers) are
 * used.
 *
 * @param size size of the @a data portion of the response
 * @param data the data itself
 * @param must_free libmicrohttpd should free data when done
 * @param must_copy libmicrohttpd must make a copy of @a data
 *        right away, the data may be released anytime after
 *        this call returns
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @deprecated use #MHD_create_response_from_buffer instead
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_data (size_t size,
                               void *data,
                               int must_free,
                               int must_copy)
{
  enum MHD_ResponseMemoryMode mode;

  if (0 != must_copy)
    mode = MHD_RESPMEM_MUST_COPY;
  else if (0 != must_free)
    mode = MHD_RESPMEM_MUST_FREE;
  else
    mode = MHD_RESPMEM_PERSISTENT;

  return MHD_create_response_from_buffer (size, data, mode);
}


/**
 * Create a response object with the content of provided buffer used as
 * the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response
 * @param buffer size bytes containing the response's data portion
 * @param mode flags for buffer management
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_buffer (size_t size,
                                 void *buffer,
                                 enum MHD_ResponseMemoryMode mode)
{
  if (MHD_RESPMEM_MUST_FREE == mode)
    return MHD_create_response_from_buffer_with_free_callback_cls (size,
                                                                   buffer,
                                                                   &free,
                                                                   buffer);
  if (MHD_RESPMEM_MUST_COPY == mode)
    return MHD_create_response_from_buffer_copy (size,
                                                 buffer);

  return MHD_create_response_from_buffer_with_free_callback_cls (size,
                                                                 buffer,
                                                                 NULL,
                                                                 NULL);
}


/**
 * Create a response object with the content of provided statically allocated
 * buffer used as the response body.
 *
 * The buffer must be valid for the lifetime of the response. The easiest way
 * to achieve this is to use a statically allocated buffer.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size the size of the data in @a buffer, can be zero
 * @param buffer the buffer with the data for the response body, can be NULL
 *               if @a size is zero
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @note Available since #MHD_VERSION 0x00097701
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_buffer_static (size_t size,
                                        const void *buffer)
{
  return MHD_create_response_from_buffer_with_free_callback_cls (size,
                                                                 buffer,
                                                                 NULL,
                                                                 NULL);
}


/**
 * Create a response object with the content of provided temporal buffer
 * used as the response body.
 *
 * An internal copy of the buffer will be made automatically, so buffer have
 * to be valid only during the call of this function (as a typical example:
 * buffer is a local (non-static) array).
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size the size of the data in @a buffer, can be zero
 * @param buffer the buffer with the data for the response body, can be NULL
 *               if @a size is zero
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @note Available since #MHD_VERSION 0x00097701
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_buffer_copy (size_t size,
                                      const void *buffer)
{
  struct MHD_Response *r;
  void *mhd_copy;

  if (0 == size)
    return MHD_create_response_from_buffer_with_free_callback_cls (0,
                                                                   NULL,
                                                                   NULL,
                                                                   NULL);
  if (NULL == buffer)
    return NULL;

  mhd_copy = malloc (size);
  if (NULL == mhd_copy)
    return NULL;
  memcpy (mhd_copy, buffer, size);

  r = MHD_create_response_from_buffer_with_free_callback_cls (size,
                                                              mhd_copy,
                                                              &free,
                                                              mhd_copy);
  if (NULL == r)
    free (mhd_copy);
  else
  {
    /* TODO: remove the next assignment, the buffer should not be modifiable */
    r->data_buffer_size = size;
  }

  return r;
}


/**
 * Create a response object with the content of provided buffer used as
 * the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response
 * @param buffer size bytes containing the response's data portion
 * @param crfc function to call to free the @a buffer
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_buffer_with_free_callback (size_t size,
                                                    void *buffer,
                                                    MHD_ContentReaderFreeCallback
                                                    crfc)
{
  return MHD_create_response_from_buffer_with_free_callback_cls (size,
                                                                 buffer,
                                                                 crfc,
                                                                 buffer);
}


/**
 * Create a response object with the content of provided buffer used as
 * the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param size size of the data portion of the response
 * @param buffer size bytes containing the response's data portion
 * @param crfc function to call to cleanup, if set to NULL then callback
 *             is not called
 * @param crfc_cls an argument for @a crfc
 * @return NULL on error (i.e. invalid arguments, out of memory)
 * @note Available since #MHD_VERSION 0x00097302
 * @note 'const' qualifier is used for @a buffer since #MHD_VERSION 0x00097701
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_buffer_with_free_callback_cls (size_t size,
                                                        const void *buffer,
                                                        MHD_ContentReaderFreeCallback
                                                        crfc,
                                                        void *crfc_cls)
{
  struct MHD_Response *r;

  if ((NULL == buffer) && (size > 0))
    return NULL;
#if SIZEOF_SIZE_T >= SIZEOF_UINT64_T
  if (MHD_SIZE_UNKNOWN == size)
    return NULL;
#endif /* SIZEOF_SIZE_T >= SIZEOF_UINT64_T */
  r = MHD_calloc_ (1, sizeof (struct MHD_Response));
  if (NULL == r)
    return NULL;
#if defined(MHD_USE_THREADS)
  if (! MHD_mutex_init_ (&r->mutex))
  {
    free (r);
    return NULL;
  }
#endif
  r->fd = -1;
  r->reference_count = 1;
  r->total_size = size;
  r->data = buffer;
  r->data_size = size;
  r->crfc = crfc;
  r->crc_cls = crfc_cls;
  return r;
}


/**
 * Create a response object with an array of memory buffers
 * used as the response body.
 *
 * The response object can be extended with header information and then
 * be used any number of times.
 *
 * If response object is used to answer HEAD request then the body
 * of the response is not used, while all headers (including automatic
 * headers) are used.
 *
 * @param iov the array for response data buffers, an internal copy of this
 *        will be made
 * @param iovcnt the number of elements in @a iov
 * @param free_cb the callback to clean up any data associated with @a iov when
 *        the response is destroyed.
 * @param cls the argument passed to @a free_cb
 * @return NULL on error (i.e. invalid arguments, out of memory)
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_from_iovec (const struct MHD_IoVec *iov,
                                unsigned int iovcnt,
                                MHD_ContentReaderFreeCallback free_cb,
                                void *cls)
{
  struct MHD_Response *response;
  unsigned int i;
  int i_cp = 0;   /**< Index in the copy of iov */
  uint64_t total_size = 0;
  const void *last_valid_buffer = NULL;

  if ((NULL == iov) && (0 < iovcnt))
    return NULL;

  response = MHD_calloc_ (1, sizeof (struct MHD_Response));
  if (NULL == response)
    return NULL;
  if (! MHD_mutex_init_ (&response->mutex))
  {
    free (response);
    return NULL;
  }
  /* Calculate final size, number of valid elements, and check 'iov' */
  for (i = 0; i < iovcnt; ++i)
  {
    if (0 == iov[i].iov_len)
      continue;     /* skip zero-sized elements */
    if (NULL == iov[i].iov_base)
    {
      i_cp = -1;     /* error */
      break;
    }
    if ( (total_size > (total_size + iov[i].iov_len)) ||
         (INT_MAX == i_cp) ||
         (SSIZE_MAX < (total_size + iov[i].iov_len)) )
    {
      i_cp = -1;     /* overflow */
      break;
    }
    last_valid_buffer = iov[i].iov_base;
    total_size += iov[i].iov_len;
#if defined(MHD_POSIX_SOCKETS) || ! defined(_WIN64)
    i_cp++;
#else  /* ! MHD_POSIX_SOCKETS && _WIN64 */
    {
      int64_t i_add;

      i_add = (int64_t) (iov[i].iov_len / ULONG_MAX);
      if (0 != iov[i].iov_len % ULONG_MAX)
        i_add++;
      if (INT_MAX < (i_add + i_cp))
      {
        i_cp = -1;   /* overflow */
        break;
      }
      i_cp += (int) i_add;
    }
#endif /* ! MHD_POSIX_SOCKETS && _WIN64 */
  }
  if (-1 == i_cp)
  {
    /* Some error condition */
    MHD_mutex_destroy_chk_ (&response->mutex);
    free (response);
    return NULL;
  }
  response->fd = -1;
  response->reference_count = 1;
  response->total_size = total_size;
  response->crc_cls = cls;
  response->crfc = free_cb;
  if (0 == i_cp)
  {
    mhd_assert (0 == total_size);
    return response;
  }
  if (1 == i_cp)
  {
    mhd_assert (NULL != last_valid_buffer);
    response->data = last_valid_buffer;
    response->data_size = (size_t) total_size;
    return response;
  }
  mhd_assert (1 < i_cp);
  if (1)
  { /* for local variables local scope only */
    MHD_iovec_ *iov_copy;
    int num_copy_elements = i_cp;

    iov_copy = MHD_calloc_ ((size_t) num_copy_elements, \
                            sizeof(MHD_iovec_));
    if (NULL == iov_copy)
    {
      MHD_mutex_destroy_chk_ (&response->mutex);
      free (response);
      return NULL;
    }
    i_cp = 0;
    for (i = 0; i < iovcnt; ++i)
    {
      size_t element_size = iov[i].iov_len;
      const uint8_t *buf = (const uint8_t *) iov[i].iov_base;

      if (0 == element_size)
        continue;         /* skip zero-sized elements */
#if defined(MHD_WINSOCK_SOCKETS) && defined(_WIN64)
      while (MHD_IOV_ELMN_MAX_SIZE < element_size)
      {
        iov_copy[i_cp].iov_base = (char *) _MHD_DROP_CONST (buf);
        iov_copy[i_cp].iov_len = ULONG_MAX;
        buf += ULONG_MAX;
        element_size -= ULONG_MAX;
        i_cp++;
      }
#endif /* MHD_WINSOCK_SOCKETS && _WIN64 */
      iov_copy[i_cp].iov_base = _MHD_DROP_CONST (buf);
      iov_copy[i_cp].iov_len = (MHD_iov_size_) element_size;
      i_cp++;
    }
    mhd_assert (num_copy_elements == i_cp);
    mhd_assert (0 <= i_cp);
    response->data_iov = iov_copy;
    response->data_iovcnt = (unsigned int) i_cp;
  }
  return response;
}


/**
 * Create a response object with empty (zero size) body.
 *
 * The response object can be extended with header information and then be used
 * any number of times.
 *
 * This function is a faster equivalent of #MHD_create_response_from_buffer call
 * with zero size combined with call of #MHD_set_response_options.
 *
 * @param flags the flags for the new response object
 * @return NULL on error (i.e. invalid arguments, out of memory),
 *         the pointer to the created response object otherwise
 * @note Available since #MHD_VERSION 0x00097701
 * @ingroup response
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_empty (enum MHD_ResponseFlags flags)
{
  struct MHD_Response *r;
  r = (struct MHD_Response *) MHD_calloc_ (1, sizeof (struct MHD_Response));
  if (NULL != r)
  {
    if (MHD_mutex_init_ (&r->mutex))
    {
      r->fd = -1;
      r->reference_count = 1;
      /* If any flags combination will be not allowed, replace the next
       * assignment with MHD_set_response_options() call. */
      r->flags = flags;

      return r; /* Successful result */
    }
    free (r);
  }
  return NULL; /* Something failed */
}


#ifdef UPGRADE_SUPPORT
/**
 * This connection-specific callback is provided by MHD to
 * applications (unusual) during the #MHD_UpgradeHandler.
 * It allows applications to perform 'special' actions on
 * the underlying socket from the upgrade.
 *
 * @param urh the handle identifying the connection to perform
 *            the upgrade @a action on.
 * @param action which action should be performed
 * @param ... arguments to the action (depends on the action)
 * @return #MHD_NO on error, #MHD_YES on success
 */
_MHD_EXTERN enum MHD_Result
MHD_upgrade_action (struct MHD_UpgradeResponseHandle *urh,
                    enum MHD_UpgradeAction action,
                    ...)
{
  struct MHD_Connection *connection;
  struct MHD_Daemon *daemon;

  if (NULL == urh)
    return MHD_NO;
  connection = urh->connection;

  /* Precaution checks on external data. */
  if (NULL == connection)
    return MHD_NO;
  daemon = connection->daemon;
  if (NULL == daemon)
    return MHD_NO;

  switch (action)
  {
  case MHD_UPGRADE_ACTION_CLOSE:
    if (urh->was_closed)
      return MHD_NO; /* Already closed. */

    /* transition to special 'closed' state for start of cleanup */
#ifdef HTTPS_SUPPORT
    if (0 != (daemon->options & MHD_USE_TLS) )
    {
      /* signal that app is done by shutdown() of 'app' socket */
      /* Application will not use anyway this socket after this command. */
      shutdown (urh->app.socket,
                SHUT_RDWR);
    }
#endif /* HTTPS_SUPPORT */
    mhd_assert (MHD_CONNECTION_UPGRADE == connection->state);
    /* The next function will mark the connection as closed by application
     * by setting 'urh->was_closed'.
     * As soon as connection will be marked with BOTH
     * 'urh->was_closed' AND 'urh->clean_ready', it will
     * be moved to cleanup list by MHD_resume_connection(). */
    MHD_upgraded_connection_mark_app_closed_ (connection);
    return MHD_YES;
  case MHD_UPGRADE_ACTION_CORK_ON:
    /* Unportable API. TODO: replace with portable action. */
    return MHD_connection_set_cork_state_ (connection,
                                           true) ? MHD_YES : MHD_NO;
  case MHD_UPGRADE_ACTION_CORK_OFF:
    /* Unportable API. TODO: replace with portable action. */
    return MHD_connection_set_cork_state_ (connection,
                                           false) ? MHD_YES : MHD_NO;
  default:
    /* we don't understand this one */
    return MHD_NO;
  }
}


/**
 * We are done sending the header of a given response to the client.
 * Now it is time to perform the upgrade and hand over the connection
 * to the application.
 * @remark To be called only from thread that process connection's
 * recv(), send() and response. Must be called right after sending
 * response headers.
 *
 * @param response the response that was created for an upgrade
 * @param connection the specific connection we are upgrading
 * @return #MHD_YES on success, #MHD_NO on failure (will cause
 *        connection to be closed)
 */
enum MHD_Result
MHD_response_execute_upgrade_ (struct MHD_Response *response,
                               struct MHD_Connection *connection)
{
#if defined(HTTPS_SUPPORT) || defined(_DEBUG) || defined(HAVE_MESSAGES)
  struct MHD_Daemon *const daemon = connection->daemon;
#endif /* HTTPS_SUPPORT || _DEBUG || HAVE_MESSAGES */
  struct MHD_UpgradeResponseHandle *urh;
  size_t rbo;

#ifdef MHD_USE_THREADS
  mhd_assert ( (! MHD_D_IS_USING_THREADS_ (daemon)) || \
               MHD_thread_handle_ID_is_current_thread_ (connection->tid) );
#endif /* MHD_USE_THREADS */

  /* "Upgrade" responses accepted only if MHD_ALLOW_UPGRADE is enabled */
  mhd_assert (0 != (daemon->options & MHD_ALLOW_UPGRADE));
  /* The header was checked when response queued */
  mhd_assert (NULL != \
              MHD_get_response_element_n_ (response, MHD_HEADER_KIND,
                                           MHD_HTTP_HEADER_UPGRADE,
                                           MHD_STATICSTR_LEN_ ( \
                                             MHD_HTTP_HEADER_UPGRADE)));

  if (! connection->sk_nonblck)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Cannot execute \"upgrade\" as the socket is in " \
                 "the blocking mode.\n"));
#endif
    return MHD_NO;
  }
  urh = MHD_calloc_ (1, sizeof (struct MHD_UpgradeResponseHandle));
  if (NULL == urh)
    return MHD_NO;
  urh->connection = connection;
  rbo = connection->read_buffer_offset;
  connection->read_buffer_offset = 0;
  MHD_connection_set_nodelay_state_ (connection, false);
  MHD_connection_set_cork_state_ (connection, false);
#ifdef HTTPS_SUPPORT
  if (0 != (daemon->options & MHD_USE_TLS) )
  {
    MHD_socket sv[2];
#if defined(MHD_socket_nosignal_) || ! defined(MHD_socket_pair_nblk_)
    int res1;
    int res2;
#endif /* MHD_socket_nosignal_ || !MHD_socket_pair_nblk_ */

#ifdef MHD_socket_pair_nblk_
    if (! MHD_socket_pair_nblk_ (sv))
    {
      free (urh);
      return MHD_NO;
    }
#else  /* !MHD_socket_pair_nblk_ */
    if (! MHD_socket_pair_ (sv))
    {
      free (urh);
      return MHD_NO;
    }
    res1 = MHD_socket_nonblocking_ (sv[0]);
    res2 = MHD_socket_nonblocking_ (sv[1]);
    if ( (! res1) || (! res2) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to make loopback sockets non-blocking.\n"));
#endif
      if (! res2)
      {
        /* Socketpair cannot be used. */
        MHD_socket_close_chk_ (sv[0]);
        MHD_socket_close_chk_ (sv[1]);
        free (urh);
        return MHD_NO;
      }
    }
#endif /* !MHD_socket_pair_nblk_ */
#ifdef MHD_socket_nosignal_
    res1 = MHD_socket_nosignal_ (sv[0]);
    res2 = MHD_socket_nosignal_ (sv[1]);
    if ( (! res1) || (! res2) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Failed to set SO_NOSIGPIPE on loopback sockets.\n"));
#endif
#ifndef MSG_NOSIGNAL
      if (! res2)
      {
        /* Socketpair cannot be used. */
        MHD_socket_close_chk_ (sv[0]);
        MHD_socket_close_chk_ (sv[1]);
        free (urh);
        return MHD_NO;
      }
#endif /* ! MSG_NOSIGNAL */
    }
#endif /* MHD_socket_nosignal_ */
    if (MHD_D_IS_USING_SELECT_ (daemon) &&
        (! MHD_D_DOES_SCKT_FIT_FDSET_ (sv[1], daemon)) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Socketpair descriptor is not less than FD_SETSIZE: " \
                   "%d >= %d\n"),
                (int) sv[1],
                (int) MHD_D_GET_FD_SETSIZE_ (daemon));
#endif
      MHD_socket_close_chk_ (sv[0]);
      MHD_socket_close_chk_ (sv[1]);
      free (urh);
      return MHD_NO;
    }
    urh->app.socket = sv[0];
    urh->app.urh = urh;
    urh->app.celi = MHD_EPOLL_STATE_UNREADY;
    urh->mhd.socket = sv[1];
    urh->mhd.urh = urh;
    urh->mhd.celi = MHD_EPOLL_STATE_UNREADY;
#ifdef EPOLL_SUPPORT
    /* Launch IO processing by the event loop */
    if (MHD_D_IS_USING_EPOLL_ (daemon))
    {
      /* We're running with epoll(), need to add the sockets
         to the event set of the daemon's `epoll_upgrade_fd` */
      struct epoll_event event;

      mhd_assert (-1 != daemon->epoll_upgrade_fd);
      /* First, add network socket */
      event.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET;
      event.data.ptr = &urh->app;
      if (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                          EPOLL_CTL_ADD,
                          connection->socket_fd,
                          &event))
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Call to epoll_ctl failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
        MHD_socket_close_chk_ (sv[0]);
        MHD_socket_close_chk_ (sv[1]);
        free (urh);
        return MHD_NO;
      }

      /* Second, add our end of the UNIX socketpair() */
      event.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET;
      event.data.ptr = &urh->mhd;
      if (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                          EPOLL_CTL_ADD,
                          urh->mhd.socket,
                          &event))
      {
        event.events = EPOLLIN | EPOLLOUT | EPOLLPRI;
        event.data.ptr = &urh->app;
        if (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                            EPOLL_CTL_DEL,
                            connection->socket_fd,
                            &event))
          MHD_PANIC (_ ("Error cleaning up while handling epoll error.\n"));
#ifdef HAVE_MESSAGES
        MHD_DLOG (daemon,
                  _ ("Call to epoll_ctl failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
        MHD_socket_close_chk_ (sv[0]);
        MHD_socket_close_chk_ (sv[1]);
        free (urh);
        return MHD_NO;
      }
      EDLL_insert (daemon->eready_urh_head,
                   daemon->eready_urh_tail,
                   urh);
      urh->in_eready_list = true;
    }
#endif /* EPOLL_SUPPORT */
    if (! MHD_D_IS_USING_THREAD_PER_CONN_ (daemon))
    {
      /* This takes care of further processing for most event loops:
         simply add to DLL for bi-direcitonal processing */
      DLL_insert (daemon->urh_head,
                  daemon->urh_tail,
                  urh);
    }
    /* In thread-per-connection mode, thread will switch to forwarding once
     * connection.urh is not NULL and connection.state == MHD_CONNECTION_UPGRADE.
     */
  }
  else
  {
    urh->app.socket = MHD_INVALID_SOCKET;
    urh->mhd.socket = MHD_INVALID_SOCKET;
    /* Non-TLS connection do not hold any additional resources. */
    urh->clean_ready = true;
  }
#else  /* ! HTTPS_SUPPORT */
  urh->clean_ready = true;
#endif /* ! HTTPS_SUPPORT */
  connection->urh = urh;
  /* As far as MHD's event loops are concerned, this connection is
     suspended; it will be resumed once application is done by the
     #MHD_upgrade_action() function */
  internal_suspend_connection_ (connection);

  /* hand over socket to application */
  response->upgrade_handler (response->upgrade_handler_cls,
                             connection,
                             connection->rq.client_context,
                             connection->read_buffer,
                             rbo,
#ifdef HTTPS_SUPPORT
                             (0 == (daemon->options & MHD_USE_TLS) ) ?
                             connection->socket_fd : urh->app.socket,
#else  /* ! HTTPS_SUPPORT */
                             connection->socket_fd,
#endif /* ! HTTPS_SUPPORT */
                             urh);

#ifdef HTTPS_SUPPORT
  if (0 != (daemon->options & MHD_USE_TLS))
  {
    struct MemoryPool *const pool = connection->pool;
    size_t avail;
    char *buf;

    /* All data should be sent already */
    mhd_assert (connection->write_buffer_send_offset == \
                connection->write_buffer_append_offset);
    MHD_pool_deallocate (pool, connection->write_buffer,
                         connection->write_buffer_size);
    connection->write_buffer_append_offset = 0;
    connection->write_buffer_send_offset = 0;
    connection->write_buffer_size = 0;
    connection->write_buffer = NULL;

    /* Extra read data should be processed already by the application */
    MHD_pool_deallocate (pool, connection->read_buffer,
                         connection->read_buffer_size);
    connection->read_buffer_offset = 0;
    connection->read_buffer_size = 0;
    connection->read_buffer = NULL;

    avail = MHD_pool_get_free (pool);
    if (avail < RESERVE_EBUF_SIZE)
    {
      /* connection's pool is totally at the limit,
         use our 'emergency' buffer of #RESERVE_EBUF_SIZE bytes. */
      avail = RESERVE_EBUF_SIZE;
      buf = urh->e_buf;
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Memory shortage in connection's memory pool. " \
                   "The \"upgraded\" communication will be inefficient.\n"));
#endif
    }
    else
    {
      /* Normal case: grab all remaining memory from the
         connection's pool for the IO buffers; the connection
         certainly won't need it anymore as we've upgraded
         to another protocol. */
      buf = MHD_pool_allocate (pool,
                               avail,
                               false);
    }
    /* use half the buffer for inbound, half for outbound */
    urh->in_buffer_size = avail / 2;
    urh->out_buffer_size = avail - urh->in_buffer_size;
    urh->in_buffer = buf;
    urh->out_buffer = buf + urh->in_buffer_size;
  }
#endif /* HTTPS_SUPPORT */
  return MHD_YES;
}


/**
 * Create a response object that can be used for 101 UPGRADE
 * responses, for example to implement WebSockets.  After sending the
 * response, control over the data stream is given to the callback (which
 * can then, for example, start some bi-directional communication).
 * If the response is queued for multiple connections, the callback
 * will be called for each connection.  The callback
 * will ONLY be called after the response header was successfully passed
 * to the OS; if there are communication errors before, the usual MHD
 * connection error handling code will be performed.
 *
 * Setting the correct HTTP code (i.e. MHD_HTTP_SWITCHING_PROTOCOLS)
 * and setting correct HTTP headers for the upgrade must be done
 * manually (this way, it is possible to implement most existing
 * WebSocket versions using this API; in fact, this API might be useful
 * for any protocol switch, not just WebSockets).  Note that
 * draft-ietf-hybi-thewebsocketprotocol-00 cannot be implemented this
 * way as the header "HTTP/1.1 101 WebSocket Protocol Handshake"
 * cannot be generated; instead, MHD will always produce "HTTP/1.1 101
 * Switching Protocols" (if the response code 101 is used).
 *
 * As usual, the response object can be extended with header
 * information and then be used any number of times (as long as the
 * header information is not connection-specific).
 *
 * @param upgrade_handler function to call with the 'upgraded' socket
 * @param upgrade_handler_cls closure for @a upgrade_handler
 * @return NULL on error (i.e. invalid arguments, out of memory)
 */
_MHD_EXTERN struct MHD_Response *
MHD_create_response_for_upgrade (MHD_UpgradeHandler upgrade_handler,
                                 void *upgrade_handler_cls)
{
  struct MHD_Response *response;

  if (NULL == upgrade_handler)
    return NULL; /* invalid request */
  response = MHD_calloc_ (1, sizeof (struct MHD_Response));
  if (NULL == response)
    return NULL;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if (! MHD_mutex_init_ (&response->mutex))
  {
    free (response);
    return NULL;
  }
#endif
  response->upgrade_handler = upgrade_handler;
  response->upgrade_handler_cls = upgrade_handler_cls;
  response->total_size = 0;
  response->reference_count = 1;
  if (MHD_NO ==
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_CONNECTION,
                               "Upgrade"))
  {
    MHD_destroy_response (response);
    return NULL;
  }
  return response;
}


#endif /* UPGRADE_SUPPORT */


/**
 * Destroy a response object and associated resources.  Note that
 * libmicrohttpd may keep some of the resources around if the response
 * is still in the queue for some clients, so the memory may not
 * necessarily be freed immediately.
 *
 * @param response response to destroy
 * @ingroup response
 */
_MHD_EXTERN void
MHD_destroy_response (struct MHD_Response *response)
{
  struct MHD_HTTP_Res_Header *pos;

  if (NULL == response)
    return;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&response->mutex);
#endif
  if (0 != --(response->reference_count))
  {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    return;
  }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&response->mutex);
  MHD_mutex_destroy_chk_ (&response->mutex);
#endif
  if (NULL != response->crfc)
    response->crfc (response->crc_cls);

  if (NULL != response->data_iov)
  {
    free (response->data_iov);
  }

  while (NULL != response->first_header)
  {
    pos = response->first_header;
    response->first_header = pos->next;
    free (pos->header);
    free (pos->value);
    free (pos);
  }
  free (response);
}


/**
 * Increments the reference counter for the @a response.
 *
 * @param response object to modify
 */
void
MHD_increment_response_rc (struct MHD_Response *response)
{
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&response->mutex);
#endif
  (response->reference_count)++;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&response->mutex);
#endif
}


/* end of response.c */
