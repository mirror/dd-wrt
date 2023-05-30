/*
     This file is part of libmicrohttpd
     Copyright (C) 2007-2020 Daniel Pittman and Christian Grothoff
     Copyright (C) 2015-2021 Evgeny Grin (Karlson2k)

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
 * @file connection.c
 * @brief  Methods for managing connections
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */
#include "internal.h"
#include "mhd_limits.h"
#include "connection.h"
#include "memorypool.h"
#include "response.h"
#include "mhd_mono_clock.h"
#include "mhd_str.h"
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
#include "mhd_locks.h"
#endif
#include "mhd_sockets.h"
#include "mhd_compat.h"
#include "mhd_itc.h"
#ifdef MHD_LINUX_SOLARIS_SENDFILE
#include <sys/sendfile.h>
#endif /* MHD_LINUX_SOLARIS_SENDFILE */
#if defined(HAVE_FREEBSD_SENDFILE) || defined(HAVE_DARWIN_SENDFILE)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#endif /* HAVE_FREEBSD_SENDFILE || HAVE_DARWIN_SENDFILE */
#ifdef HTTPS_SUPPORT
#include "connection_https.h"
#endif /* HTTPS_SUPPORT */
#ifdef HAVE_SYS_PARAM_H
/* For FreeBSD version identification */
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */
#include "mhd_send.h"
#include "mhd_assert.h"

/**
 * Message to transmit when http 1.1 request is received
 */
#define HTTP_100_CONTINUE "HTTP/1.1 100 Continue\r\n\r\n"

/**
 * Response text used when the request (http header) is too big to
 * be processed.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_TOO_BIG \
  "<html><head><title>Request too big</title></head><body>Your HTTP header was too big for the memory constraints of this webserver.</body></html>"
#else
#define REQUEST_TOO_BIG ""
#endif

/**
 * Response text used when the request (http header) does not
 * contain a "Host:" header and still claims to be HTTP 1.1.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_LACKS_HOST \
  "<html><head><title>&quot;Host:&quot; header required</title></head><body>In HTTP 1.1, requests must include a &quot;Host:&quot; header, and your HTTP 1.1 request lacked such a header.</body></html>"
#else
#define REQUEST_LACKS_HOST ""
#endif

/**
 * Response text used when the request (http header) is
 * malformed.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_MALFORMED \
  "<html><head><title>Request malformed</title></head><body>Your HTTP request was syntactically incorrect.</body></html>"
#else
#define REQUEST_MALFORMED ""
#endif

/**
 * Response text used when the request HTTP chunked encoding is
 * malformed.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CHUNKED_MALFORMED \
  "<html><head><title>Request malformed</title></head><body>Your HTTP chunked encoding was syntactically incorrect.</body></html>"
#else
#define REQUEST_CHUNKED_MALFORMED ""
#endif

/**
 * Response text used when the request HTTP chunk is too large.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CHUNK_TOO_LARGE \
  "<html><head><title>Request content too large</title></head><body>The chunk size used in your HTTP chunked encoded request is too large.</body></html>"
#else
#define REQUEST_CHUNK_TOO_LARGE ""
#endif

/**
 * Response text used when the request HTTP content is too large.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CONTENTLENGTH_TOOLARGE \
  "<html><head><title>Request content too large</title></head>" \
  "<body>Your HTTP request has too large value for <b>Content-Length</b> header.</body></html>"
#else
#define REQUEST_CONTENTLENGTH_TOOLARGE ""
#endif

/**
 * Response text used when the request HTTP chunked encoding is
 * malformed.
 */
#ifdef HAVE_MESSAGES
#define REQUEST_CONTENTLENGTH_MALFORMED \
  "<html><head><title>Request malformed</title></head>" \
  "<body>Your HTTP request has wrong value for <b>Content-Length</b> header.</body></html>"
#else
#define REQUEST_CONTENTLENGTH_MALFORMED ""
#endif

/**
 * Response text used when there is an internal server error.
 *
 * Intentionally empty here to keep our memory footprint
 * minimal.
 */
#ifdef HAVE_MESSAGES
#define INTERNAL_ERROR \
  "<html><head><title>Internal server error</title></head><body>Please ask the developer of this Web server to carefully read the GNU libmicrohttpd documentation about connection management and blocking.</body></html>"
#else
#define INTERNAL_ERROR ""
#endif

/**
 * Response text used when the request HTTP version is too old.
 */
#ifdef HAVE_MESSAGES
#define REQ_HTTP_VER_IS_TOO_OLD \
  "<html><head><title>Requested HTTP version is not supported</title></head><body>Requested HTTP version is too old and not supported.</body></html>"
#else
#define REQ_HTTP_VER_IS_TOO_OLD ""
#endif

/**
 * Response text used when the request HTTP version is not supported.
 */
#ifdef HAVE_MESSAGES
#define REQ_HTTP_VER_IS_NOT_SUPPORTED \
  "<html><head><title>Requested HTTP version is not supported</title></head><body>Requested HTTP version is not supported.</body></html>"
#else
#define REQ_HTTP_VER_IS_NOT_SUPPORTED ""
#endif


/**
 * sendfile() chuck size
 */
#define MHD_SENFILE_CHUNK_         (0x20000)

/**
 * sendfile() chuck size for thread-per-connection
 */
#define MHD_SENFILE_CHUNK_THR_P_C_ (0x200000)

#ifdef HAVE_MESSAGES
/**
 * Return text description for MHD_ERR_*_ codes
 * @param mhd_err_code the error code
 * @return pointer to static string with error description
 */
static const char *
str_conn_error_ (ssize_t mhd_err_code)
{
  switch (mhd_err_code)
  {
  case MHD_ERR_AGAIN_:
    return _ ("The operation would block, retry later");
  case MHD_ERR_CONNRESET_:
    return _ ("The connection was forcibly closed by remote peer");
  case MHD_ERR_NOTCONN_:
    return _ ("The socket is not connected");
  case MHD_ERR_NOMEM_:
    return _ ("Not enough system resources to serve the request");
  case MHD_ERR_BADF_:
    return _ ("Bad FD value");
  case MHD_ERR_INVAL_:
    return _ ("Argument value is invalid");
  case MHD_ERR_OPNOTSUPP_:
    return _ ("Argument value is not supported");
  case MHD_ERR_PIPE_:
    return _ ("The socket is no longer available for sending");
  case MHD_ERR_TLS_:
    return _ ("TLS encryption or decryption error");
  default:
    break;   /* Mute compiler warning */
  }
  if (0 <= mhd_err_code)
    return _ ("Not an error code");

  mhd_assert (0); /* Should never be reachable */
  return _ ("Wrong error code value");
}


#endif /* HAVE_MESSAGES */

/**
 * Allocate memory from connection's memory pool.
 * If memory pool doesn't have enough free memory but read of write buffer
 * have some unused memory, the size of the buffer will be reduced as needed.
 * @param connection the connection to use
 * @param size the size of allocated memory area
 * @return pointer to allocated memory region in the pool or
 *         NULL if no memory is available
 */
static void *
connection_alloc_memory (struct MHD_Connection *connection,
                         size_t size)
{
  struct MHD_Connection *const c = connection; /* a short alias */
  struct MemoryPool *const pool = c->pool;     /* a short alias */
  size_t need_to_free; /**< The required amount of free memory */
  void *res;

  res = MHD_pool_try_alloc (pool, size, &need_to_free);
  if (NULL == res)
  {
    if (NULL != c->write_buffer)
    {
      /* The connection is in the sending phase */
      mhd_assert (MHD_CONNECTION_START_REPLY <= c->state);
      if (c->write_buffer_size - c->write_buffer_append_offset >= need_to_free)
      {
        char *buf;
        const size_t new_buf_size = c->write_buffer_size - need_to_free;
        buf = MHD_pool_reallocate (pool,
                                   c->write_buffer,
                                   c->write_buffer_size,
                                   new_buf_size);
        mhd_assert (c->write_buffer == buf);
        mhd_assert (c->write_buffer_append_offset <= new_buf_size);
        mhd_assert (c->write_buffer_send_offset <= new_buf_size);
        c->write_buffer_size = new_buf_size;
        c->write_buffer = buf;
      }
      else
        return NULL;
    }
    else if (NULL != c->read_buffer)
    {
      /* The connection is in the receiving phase */
      if (c->read_buffer_size - c->read_buffer_offset >= need_to_free)
      {
        char *buf;
        const size_t new_buf_size = c->read_buffer_size - need_to_free;
        buf = MHD_pool_reallocate (pool,
                                   c->read_buffer,
                                   c->read_buffer_size,
                                   new_buf_size);
        mhd_assert (c->read_buffer == buf);
        mhd_assert (c->read_buffer_offset <= new_buf_size);
        c->read_buffer_size = new_buf_size;
        c->read_buffer = buf;
      }
      else
        return NULL;
    }
    else
      return NULL;
    res = MHD_pool_allocate (pool, size, true);
    mhd_assert (NULL != res); /* It has been checked that pool has enough space */
  }
  return res;
}


/**
 * Callback for receiving data from the socket.
 *
 * @param connection the MHD connection structure
 * @param other where to write received data to
 * @param i maximum size of other (in bytes)
 * @return positive value for number of bytes actually received or
 *         negative value for error number MHD_ERR_xxx_
 */
static ssize_t
recv_param_adapter (struct MHD_Connection *connection,
                    void *other,
                    size_t i)
{
  ssize_t ret;

  if ( (MHD_INVALID_SOCKET == connection->socket_fd) ||
       (MHD_CONNECTION_CLOSED == connection->state) )
  {
    return MHD_ERR_NOTCONN_;
  }
  if (i > MHD_SCKT_SEND_MAX_SIZE_)
    i = MHD_SCKT_SEND_MAX_SIZE_; /* return value limit */

  ret = MHD_recv_ (connection->socket_fd,
                   other,
                   i);
  if (0 > ret)
  {
    const int err = MHD_socket_get_error_ ();
    if (MHD_SCKT_ERR_IS_EAGAIN_ (err))
    {
#ifdef EPOLL_SUPPORT
      /* Got EAGAIN --- no longer read-ready */
      connection->epoll_state &=
        ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
#endif /* EPOLL_SUPPORT */
      return MHD_ERR_AGAIN_;
    }
    if (MHD_SCKT_ERR_IS_EINTR_ (err))
      return MHD_ERR_AGAIN_;
    if (MHD_SCKT_ERR_IS_REMOTE_DISCNN_ (err))
      return MHD_ERR_CONNRESET_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EOPNOTSUPP_))
      return MHD_ERR_OPNOTSUPP_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_ENOTCONN_))
      return MHD_ERR_NOTCONN_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EINVAL_))
      return MHD_ERR_INVAL_;
    if (MHD_SCKT_ERR_IS_LOW_RESOURCES_ (err))
      return MHD_ERR_NOMEM_;
    if (MHD_SCKT_ERR_IS_ (err, MHD_SCKT_EBADF_))
      return MHD_ERR_BADF_;
    /* Treat any other error as a hard error. */
    return MHD_ERR_NOTCONN_;
  }
#ifdef EPOLL_SUPPORT
  else if (i > (size_t) ret)
    connection->epoll_state &=
      ~((enum MHD_EpollState) MHD_EPOLL_STATE_READ_READY);
#endif /* EPOLL_SUPPORT */
  return ret;
}


/**
 * Get all of the headers from the request.
 *
 * @param connection connection to get values from
 * @param kind types of values to iterate over, can be a bitmask
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over
 *         -1 if connection is NULL.
 * @ingroup request
 */
int
MHD_get_connection_values (struct MHD_Connection *connection,
                           enum MHD_ValueKind kind,
                           MHD_KeyValueIterator iterator,
                           void *iterator_cls)
{
  int ret;
  struct MHD_HTTP_Header *pos;

  if (NULL == connection)
    return -1;
  ret = 0;
  for (pos = connection->headers_received; NULL != pos; pos = pos->next)
    if (0 != (pos->kind & kind))
    {
      ret++;
      if ( (NULL != iterator) &&
           (MHD_NO == iterator (iterator_cls,
                                pos->kind,
                                pos->header,
                                pos->value)) )
        return ret;
    }
  return ret;
}


/**
 * Get all of the headers from the request.
 *
 * @param connection connection to get values from
 * @param kind types of values to iterate over, can be a bitmask
 * @param iterator callback to call on each header;
 *        maybe NULL (then just count headers)
 * @param iterator_cls extra argument to @a iterator
 * @return number of entries iterated over,
 *         -1 if connection is NULL.
 * @ingroup request
 */
int
MHD_get_connection_values_n (struct MHD_Connection *connection,
                             enum MHD_ValueKind kind,
                             MHD_KeyValueIteratorN iterator,
                             void *iterator_cls)
{
  int ret;
  struct MHD_HTTP_Header *pos;

  if (NULL == connection)
    return -1;
  ret = 0;

  if (NULL == iterator)
    for (pos = connection->headers_received; NULL != pos; pos = pos->next)
    {
      if (0 != (kind & pos->kind))
        ret++;
    }
  else
    for (pos = connection->headers_received; NULL != pos; pos = pos->next)
      if (0 != (kind & pos->kind))
      {
        ret++;
        if (MHD_NO == iterator (iterator_cls,
                                pos->kind,
                                pos->header,
                                pos->header_size,
                                pos->value,
                                pos->value_size))
          return ret;
      }
  return ret;
}


/**
 * This function can be used to add an arbitrary entry to connection.
 * Internal version of #MHD_set_connection_value_n() without checking
 * of arguments values.
 *
 * @param connection the connection for which a
 *                   value should be set
 * @param kind kind of the value
 * @param key key for the value, must be zero-terminated
 * @param key_size number of bytes in @a key (excluding 0-terminator)
 * @param value the value itself, must be zero-terminated
 * @param value_size number of bytes in @a value (excluding 0-terminator)
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
static enum MHD_Result
MHD_set_connection_value_n_nocheck_ (struct MHD_Connection *connection,
                                     enum MHD_ValueKind kind,
                                     const char *key,
                                     size_t key_size,
                                     const char *value,
                                     size_t value_size)
{
  struct MHD_HTTP_Header *pos;

  pos = connection_alloc_memory (connection,
                                 sizeof (struct MHD_HTTP_Header));
  if (NULL == pos)
    return MHD_NO;
  pos->header = (char *) key;
  pos->header_size = key_size;
  pos->value = (char *) value;
  pos->value_size = value_size;
  pos->kind = kind;
  pos->next = NULL;
  /* append 'pos' to the linked list of headers */
  if (NULL == connection->headers_received_tail)
  {
    connection->headers_received = pos;
    connection->headers_received_tail = pos;
  }
  else
  {
    connection->headers_received_tail->next = pos;
    connection->headers_received_tail = pos;
  }
  return MHD_YES;
}


/**
 * This function can be used to add an arbitrary entry to connection.
 * This function could add entry with binary zero, which is allowed
 * for #MHD_GET_ARGUMENT_KIND. For other kind on entries it is
 * recommended to use #MHD_set_connection_value.
 *
 * This function MUST only be called from within the
 * #MHD_AccessHandlerCallback (otherwise, access maybe improperly
 * synchronized).  Furthermore, the client must guarantee that the key
 * and value arguments are 0-terminated strings that are NOT freed
 * until the connection is closed.  (The easiest way to do this is by
 * passing only arguments to permanently allocated strings.).
 *
 * @param connection the connection for which a
 *  value should be set
 * @param kind kind of the value
 * @param key key for the value, must be zero-terminated
 * @param key_size number of bytes in @a key (excluding 0-terminator)
 * @param value the value itself, must be zero-terminated
 * @param value_size number of bytes in @a value (excluding 0-terminator)
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
enum MHD_Result
MHD_set_connection_value_n (struct MHD_Connection *connection,
                            enum MHD_ValueKind kind,
                            const char *key,
                            size_t key_size,
                            const char *value,
                            size_t value_size)
{
  if ( (MHD_GET_ARGUMENT_KIND != kind) &&
       ( ((key ? strlen (key) : 0) != key_size) ||
         ((value ? strlen (value) : 0) != value_size) ) )
    return MHD_NO; /* binary zero is allowed only in GET arguments */

  return MHD_set_connection_value_n_nocheck_ (connection,
                                              kind,
                                              key,
                                              key_size,
                                              value,
                                              value_size);
}


/**
 * This function can be used to add an entry to the HTTP headers of a
 * connection (so that the #MHD_get_connection_values function will
 * return them -- and the `struct MHD_PostProcessor` will also see
 * them).  This maybe required in certain situations (see Mantis
 * #1399) where (broken) HTTP implementations fail to supply values
 * needed by the post processor (or other parts of the application).
 *
 * This function MUST only be called from within the
 * #MHD_AccessHandlerCallback (otherwise, access maybe improperly
 * synchronized).  Furthermore, the client must guarantee that the key
 * and value arguments are 0-terminated strings that are NOT freed
 * until the connection is closed.  (The easiest way to do this is by
 * passing only arguments to permanently allocated strings.).
 *
 * @param connection the connection for which a
 *  value should be set
 * @param kind kind of the value
 * @param key key for the value
 * @param value the value itself
 * @return #MHD_NO if the operation could not be
 *         performed due to insufficient memory;
 *         #MHD_YES on success
 * @ingroup request
 */
enum MHD_Result
MHD_set_connection_value (struct MHD_Connection *connection,
                          enum MHD_ValueKind kind,
                          const char *key,
                          const char *value)
{
  return MHD_set_connection_value_n_nocheck_ (connection,
                                              kind,
                                              key,
                                              NULL != key
                                              ? strlen (key)
                                              : 0,
                                              value,
                                              NULL != value
                                              ? strlen (value)
                                              : 0);
}


/**
 * Get a particular header value.  If multiple
 * values match the kind, return any one of them.
 *
 * @param connection connection to get values from
 * @param kind what kind of value are we looking for
 * @param key the header to look for, NULL to lookup 'trailing' value without a key
 * @return NULL if no such item was found
 * @ingroup request
 */
const char *
MHD_lookup_connection_value (struct MHD_Connection *connection,
                             enum MHD_ValueKind kind,
                             const char *key)
{
  const char *value;

  value = NULL;
  (void) MHD_lookup_connection_value_n (connection,
                                        kind,
                                        key,
                                        (NULL == key) ? 0 : strlen (key),
                                        &value,
                                        NULL);
  return value;
}


/**
 * Get a particular header value.  If multiple
 * values match the kind, return any one of them.
 * @note Since MHD_VERSION 0x00096304
 *
 * @param connection connection to get values from
 * @param kind what kind of value are we looking for
 * @param key the header to look for, NULL to lookup 'trailing' value without a key
 * @param key_size the length of @a key in bytes
 * @param[out] value_ptr the pointer to variable, which will be set to found value,
 *                       will not be updated if key not found,
 *                       could be NULL to just check for presence of @a key
 * @param[out] value_size_ptr the pointer variable, which will set to found value,
 *                            will not be updated if key not found,
 *                            could be NULL
 * @return #MHD_YES if key is found,
 *         #MHD_NO otherwise.
 * @ingroup request
 */
_MHD_EXTERN enum MHD_Result
MHD_lookup_connection_value_n (struct MHD_Connection *connection,
                               enum MHD_ValueKind kind,
                               const char *key,
                               size_t key_size,
                               const char **value_ptr,
                               size_t *value_size_ptr)
{
  struct MHD_HTTP_Header *pos;

  if (NULL == connection)
    return MHD_NO;

  if (NULL == key)
  {
    for (pos = connection->headers_received; NULL != pos; pos = pos->next)
    {
      if ( (0 != (kind & pos->kind)) &&
           (NULL == pos->header) )
        break;
    }
  }
  else
  {
    for (pos = connection->headers_received; NULL != pos; pos = pos->next)
    {
      if ( (0 != (kind & pos->kind)) &&
           (key_size == pos->header_size) &&
           ( (key == pos->header) ||
             (MHD_str_equal_caseless_bin_n_ (key,
                                             pos->header,
                                             key_size) ) ) )
        break;
    }
  }

  if (NULL == pos)
    return MHD_NO;

  if (NULL != value_ptr)
    *value_ptr = pos->value;

  if (NULL != value_size_ptr)
    *value_size_ptr = pos->value_size;

  return MHD_YES;
}


/**
 * Check whether request header contains particular token.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 * @param connection the connection to get values from
 * @param header     the header name
 * @param header_len the length of header, not including optional
 *                   terminating null-character
 * @param token      the token to find
 * @param token_len  the length of token, not including optional
 *                   terminating null-character.
 * @return true if token is found in specified header,
 *         false otherwise
 */
static bool
MHD_lookup_header_token_ci (const struct MHD_Connection *connection,
                            const char *header,
                            size_t header_len,
                            const char *token,
                            size_t token_len)
{
  struct MHD_HTTP_Header *pos;

  if ((NULL == connection) || (NULL == header) || (0 == header[0]) || (NULL ==
                                                                       token) ||
      (0 ==
       token
       [
         0]) )
    return false;

  for (pos = connection->headers_received; NULL != pos; pos = pos->next)
  {
    if ((0 != (pos->kind & MHD_HEADER_KIND)) &&
        (header_len == pos->header_size) &&
        ( (header == pos->header) ||
          (MHD_str_equal_caseless_bin_n_ (header,
                                          pos->header,
                                          header_len)) ) &&
        (MHD_str_has_token_caseless_ (pos->value, token, token_len)))
      return true;
  }
  return false;
}


/**
 * Check whether request header contains particular static @a tkn.
 *
 * Token could be surrounded by spaces and tabs and delimited by comma.
 * Case-insensitive match used for header names and tokens.
 * @param c   the connection to get values from
 * @param h   the static string of header name
 * @param tkn the static string of token to find
 * @return true if token is found in specified header,
 *         false otherwise
 */
#define MHD_lookup_header_s_token_ci(c,h,tkn) \
  MHD_lookup_header_token_ci ((c),(h),MHD_STATICSTR_LEN_ (h), \
                              (tkn),MHD_STATICSTR_LEN_ (tkn))


/**
 * Do we (still) need to send a 100 continue
 * message for this connection?
 *
 * @param connection connection to test
 * @return false if we don't need 100 CONTINUE, true if we do
 */
static bool
need_100_continue (struct MHD_Connection *connection)
{
  const char *expect;

  return (MHD_IS_HTTP_VER_1_1_COMPAT (connection->http_ver) &&
          (MHD_NO != MHD_lookup_connection_value_n (connection,
                                                    MHD_HEADER_KIND,
                                                    MHD_HTTP_HEADER_EXPECT,
                                                    MHD_STATICSTR_LEN_ (
                                                      MHD_HTTP_HEADER_EXPECT),
                                                    &expect,
                                                    NULL)) &&
          (MHD_str_equal_caseless_ (expect,
                                    "100-continue")) );
}


/**
 * Mark connection as "closed".
 * @remark To be called from any thread.
 *
 * @param connection connection to close
 */
void
MHD_connection_mark_closed_ (struct MHD_Connection *connection)
{
  const struct MHD_Daemon *daemon = connection->daemon;

  if (0 == (daemon->options & MHD_USE_TURBO))
  {
#ifdef HTTPS_SUPPORT
    /* For TLS connection use shutdown of TLS layer
     * and do not shutdown TCP socket. This give more
     * chances to send TLS closure data to remote side.
     * Closure of TLS layer will be interpreted by
     * remote side as end of transmission. */
    if (0 != (daemon->options & MHD_USE_TLS))
    {
      if (! MHD_tls_connection_shutdown (connection))
        shutdown (connection->socket_fd,
                  SHUT_WR);
    }
    else   /* Combined with next 'shutdown()'. */
#endif /* HTTPS_SUPPORT */
    shutdown (connection->socket_fd,
              SHUT_WR);
  }
  connection->state = MHD_CONNECTION_CLOSED;
  connection->event_loop_info = MHD_EVENT_LOOP_INFO_CLEANUP;
}


/**
 * Close the given connection and give the
 * specified termination code to the user.
 * @remark To be called only from thread that
 * process connection's recv(), send() and response.
 *
 * @param connection connection to close
 * @param termination_code termination reason to give
 */
void
MHD_connection_close_ (struct MHD_Connection *connection,
                       enum MHD_RequestTerminationCode termination_code)
{
  struct MHD_Daemon *daemon = connection->daemon;
  struct MHD_Response *resp = connection->response;

  mhd_assert (! connection->suspended);
#ifdef MHD_USE_THREADS
  mhd_assert ( (0 == (daemon->options & MHD_USE_INTERNAL_POLLING_THREAD)) || \
               MHD_thread_ID_match_current_ (connection->pid) );
#endif /* MHD_USE_THREADS */
  if ( (NULL != daemon->notify_completed) &&
       (connection->client_aware) )
    daemon->notify_completed (daemon->notify_completed_cls,
                              connection,
                              &connection->client_context,
                              termination_code);
  connection->client_aware = false;
  if (NULL != resp)
  {
    connection->response = NULL;
    MHD_destroy_response (resp);
  }
  if (NULL != connection->pool)
  {
    MHD_pool_destroy (connection->pool);
    connection->pool = NULL;
  }

  MHD_connection_mark_closed_ (connection);
}


#if defined(HTTPS_SUPPORT) && defined(UPGRADE_SUPPORT)
/**
 * Stop TLS forwarding on upgraded connection and
 * reflect remote disconnect state to socketpair.
 * @remark In thread-per-connection mode this function
 * can be called from any thread, in other modes this
 * function must be called only from thread that process
 * daemon's select()/poll()/etc.
 *
 * @param connection the upgraded connection
 */
void
MHD_connection_finish_forward_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  struct MHD_UpgradeResponseHandle *urh = connection->urh;

#ifdef MHD_USE_THREADS
  mhd_assert ( (0 == (daemon->options & MHD_USE_INTERNAL_POLLING_THREAD)) || \
               (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) || \
               MHD_thread_ID_match_current_ (daemon->pid) );
#endif /* MHD_USE_THREADS */

  if (0 == (daemon->options & MHD_USE_TLS))
    return; /* Nothing to do with non-TLS connection. */

  if (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    DLL_remove (daemon->urh_head,
                daemon->urh_tail,
                urh);
#if EPOLL_SUPPORT
  if ( (0 != (daemon->options & MHD_USE_EPOLL)) &&
       (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                        EPOLL_CTL_DEL,
                        connection->socket_fd,
                        NULL)) )
  {
    MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
  }
  if (urh->in_eready_list)
  {
    EDLL_remove (daemon->eready_urh_head,
                 daemon->eready_urh_tail,
                 urh);
    urh->in_eready_list = false;
  }
#endif /* EPOLL_SUPPORT */
  if (MHD_INVALID_SOCKET != urh->mhd.socket)
  {
#if EPOLL_SUPPORT
    if ( (0 != (daemon->options & MHD_USE_EPOLL)) &&
         (0 != epoll_ctl (daemon->epoll_upgrade_fd,
                          EPOLL_CTL_DEL,
                          urh->mhd.socket,
                          NULL)) )
    {
      MHD_PANIC (_ ("Failed to remove FD from epoll set.\n"));
    }
#endif /* EPOLL_SUPPORT */
    /* Reflect remote disconnect to application by breaking
     * socketpair connection. */
    shutdown (urh->mhd.socket, SHUT_RDWR);
  }
  /* Socketpair sockets will remain open as they will be
   * used with MHD_UPGRADE_ACTION_CLOSE. They will be
   * closed by cleanup_upgraded_connection() during
   * connection's final cleanup.
   */
}


#endif /* HTTPS_SUPPORT && UPGRADE_SUPPORT*/


/**
 * A serious error occurred, close the
 * connection (and notify the application).
 *
 * @param connection connection to close with error
 * @param emsg error message (can be NULL)
 */
static void
connection_close_error (struct MHD_Connection *connection,
                        const char *emsg)
{
  connection->stop_with_error = true;
  connection->discard_request = true;
#ifdef HAVE_MESSAGES
  if (NULL != emsg)
    MHD_DLOG (connection->daemon,
              "%s\n",
              emsg);
#else  /* ! HAVE_MESSAGES */
  (void) emsg; /* Mute compiler warning. */
#endif /* ! HAVE_MESSAGES */
  MHD_connection_close_ (connection,
                         MHD_REQUEST_TERMINATED_WITH_ERROR);
}


/**
 * Macro to only include error message in call to
 * #connection_close_error() if we have HAVE_MESSAGES.
 */
#ifdef HAVE_MESSAGES
#define CONNECTION_CLOSE_ERROR(c, emsg) connection_close_error (c, emsg)
#else
#define CONNECTION_CLOSE_ERROR(c, emsg) connection_close_error (c, NULL)
#endif


/**
 * A serious error occurred, check whether error response is already
 * queued and close the connection if response wasn't queued.
 *
 * @param connection connection to close with error
 * @param emsg error message (can be NULL)
 */
static void
connection_close_error_check (struct MHD_Connection *connection,
                              const char *emsg)
{
  if ( (NULL != connection->response) &&
       (400 <= connection->responseCode) &&
       (NULL == connection->response->crc) && /* Static response only! */
       (connection->stop_with_error) &&
       (MHD_CONNECTION_HEADERS_SENDING == connection->state) )
    return; /* An error response was already queued */

  connection_close_error (connection, emsg);
}


/**
 * Macro to only include error message in call to
 * #connection_close_error_check() if we have HAVE_MESSAGES.
 */
#ifdef HAVE_MESSAGES
#define CONNECTION_CLOSE_ERROR_CHECK(c, emsg) \
  connection_close_error_check (c, emsg)
#else
#define CONNECTION_CLOSE_ERROR_CHECK(c, emsg) \
  connection_close_error_check (c, NULL)
#endif


/**
 * Prepare the response buffer of this connection for
 * sending.  Assumes that the response mutex is
 * already held.  If the transmission is complete,
 * this function may close the socket (and return
 * #MHD_NO).
 *
 * @param connection the connection
 * @return #MHD_NO if readying the response failed (the
 *  lock on the response will have been released already
 *  in this case).
 */
static enum MHD_Result
try_ready_normal_body (struct MHD_Connection *connection)
{
  ssize_t ret;
  struct MHD_Response *response;

  response = connection->response;
  mhd_assert (connection->rp_props.send_reply_body);

  if ( (0 == response->total_size) ||
                     /* TODO: replace the next check with assert */
       (connection->response_write_position == response->total_size) )
    return MHD_YES;  /* 0-byte response is always ready */
  if (NULL != response->data_iov)
  {
    size_t copy_size;

    if (NULL != connection->resp_iov.iov)
      return MHD_YES;
    copy_size = response->data_iovcnt * sizeof(MHD_iovec_);
    connection->resp_iov.iov = connection_alloc_memory (connection,
                                                        copy_size);
    if (NULL == connection->resp_iov.iov)
    {
      MHD_mutex_unlock_chk_ (&response->mutex);
      /* not enough memory */
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection (out of memory)."));
      return MHD_NO;
    }
    memcpy (connection->resp_iov.iov,
            response->data_iov,
            copy_size);
    connection->resp_iov.cnt = response->data_iovcnt;
    connection->resp_iov.sent = 0;
    return MHD_YES;
  }
  if (NULL == response->crc)
    return MHD_YES;
  if ( (response->data_start <=
        connection->response_write_position) &&
       (response->data_size + response->data_start >
        connection->response_write_position) )
    return MHD_YES; /* response already ready */
#if defined(_MHD_HAVE_SENDFILE)
  if (MHD_resp_sender_sendfile == connection->resp_sender)
  {
    /* will use sendfile, no need to bother response crc */
    return MHD_YES;
  }
#endif /* _MHD_HAVE_SENDFILE */

  ret = response->crc (response->crc_cls,
                       connection->response_write_position,
                       response->data,
                       (size_t) MHD_MIN ((uint64_t) response->data_buffer_size,
                                         response->total_size
                                         - connection->response_write_position));
  if (0 > ret)
  {
    /* either error or http 1.0 transfer, close socket! */
    /* TODO: do not update total size, check whether response
     * was really with unknown size */
    response->total_size = connection->response_write_position;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    if (MHD_CONTENT_READER_END_OF_STREAM == ret)
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_COMPLETED_OK);
    else
      CONNECTION_CLOSE_ERROR (connection,
                              _ (
                                "Closing connection (application reported error generating data)."));
    return MHD_NO;
  }
  response->data_start = connection->response_write_position;
  response->data_size = ret;
  if (0 == ret)
  {
    connection->state = MHD_CONNECTION_NORMAL_BODY_UNREADY;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    return MHD_NO;
  }
  return MHD_YES;
}


/**
 * Prepare the response buffer of this connection for sending.
 * Assumes that the response mutex is already held.  If the
 * transmission is complete, this function may close the socket (and
 * return #MHD_NO).
 *
 * @param connection the connection
 * @param[out] p_finished the pointer to variable that will be set to "true"
 *                        when application returned indication of the end
 *                        of the stream
 * @return #MHD_NO if readying the response failed
 */
static enum MHD_Result
try_ready_chunked_body (struct MHD_Connection *connection,
                        bool *p_finished)
{
  ssize_t ret;
  struct MHD_Response *response;
  static const size_t max_chunk = 0xFFFFFF;
  char chunk_hdr[6];            /* 6: max strlen of "FFFFFF" */
  /* "FFFFFF" + "\r\n" */
  static const size_t max_chunk_hdr_len = sizeof(chunk_hdr) + 2;
  /* "FFFFFF" + "\r\n" + "\r\n" (chunk termination) */
  static const size_t max_chunk_overhead = sizeof(chunk_hdr) + 2 + 2;
  size_t chunk_hdr_len;
  uint64_t left_to_send;
  size_t size_to_fill;

  response = connection->response;
  mhd_assert (NULL != response->crc || NULL != response->data);

  mhd_assert (0 == connection->write_buffer_append_offset);

  /* The buffer must be reasonably large enough */
  if (128 > connection->write_buffer_size)
  {
    size_t size;

    size = connection->write_buffer_size + MHD_pool_get_free (connection->pool);
    if (128 > size)
    {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      MHD_mutex_unlock_chk_ (&response->mutex);
#endif
      /* not enough memory */
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection (out of memory)."));
      return MHD_NO;
    }
    /* Limit the buffer size to the largest usable size for chunks */
    if ( (max_chunk + max_chunk_overhead) < size)
      size = max_chunk + max_chunk_overhead;
    connection->write_buffer = MHD_pool_reallocate (connection->pool,
                                                    connection->write_buffer,
                                                    connection->
                                                    write_buffer_size, size);
    mhd_assert (NULL != connection->write_buffer);
    connection->write_buffer_size = size;
  }
  mhd_assert (max_chunk_overhead < connection->write_buffer_size);

  if (MHD_SIZE_UNKNOWN == response->total_size)
    left_to_send = MHD_SIZE_UNKNOWN;
  else
    left_to_send = response->total_size - connection->response_write_position;

  size_to_fill = connection->write_buffer_size - max_chunk_overhead;
  /* Limit size for the callback to the max usable size */
  if (max_chunk < size_to_fill)
    size_to_fill = max_chunk;
  if (left_to_send < size_to_fill)
    size_to_fill = (size_t) left_to_send;

  if (0 == left_to_send)
    /* nothing to send, don't bother calling crc */
    ret = MHD_CONTENT_READER_END_OF_STREAM;
  else if ( (response->data_start <=
             connection->response_write_position) &&
            (response->data_start + response->data_size >
             connection->response_write_position) )
  {
    /* difference between response_write_position and data_start is less
       than data_size which is size_t type, no need to check for overflow */
    const size_t data_write_offset
      = (size_t) (connection->response_write_position - response->data_start);
    /* buffer already ready, use what is there for the chunk */
    ret = response->data_size - data_write_offset;
    if ( ((size_t) ret) > size_to_fill)
      ret = (ssize_t) size_to_fill;
    memcpy (&connection->write_buffer[max_chunk_hdr_len],
            &response->data[data_write_offset],
            ret);
  }
  else
  {
    if (NULL == response->crc)
    { /* There is no way to reach this code */
#if defined(MHD_USE_THREADS)
      MHD_mutex_unlock_chk_ (&response->mutex);
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("No callback for the chunked data."));
      return MHD_NO;
    }
    ret = response->crc (response->crc_cls,
                         connection->response_write_position,
                         &connection->write_buffer[max_chunk_hdr_len],
                         size_to_fill);
  }
  if (MHD_CONTENT_READER_END_WITH_ERROR == ret)
  {
    /* error, close socket! */
    /* TODO: remove update of the response size */
    response->total_size = connection->response_write_position;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            _ (
                              "Closing connection (application error generating response)."));
    return MHD_NO;
  }
  if (MHD_CONTENT_READER_END_OF_STREAM == ret)
  {
    *p_finished = true;
    /* TODO: remove update of the response size */
    response->total_size = connection->response_write_position;
    return MHD_YES;
  }
  if (0 == ret)
  {
    connection->state = MHD_CONNECTION_CHUNKED_BODY_UNREADY;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    return MHD_NO;
  }
  if (size_to_fill < (size_t) ret)
  {
#if defined(MHD_USE_THREADS)
    MHD_mutex_unlock_chk_ (&response->mutex);
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Closing connection (application returned " \
                               "more data than requested)."));
    return MHD_NO;
  }
  chunk_hdr_len = MHD_uint32_to_strx ((uint32_t) ret, chunk_hdr,
                                      sizeof(chunk_hdr));
  mhd_assert (chunk_hdr_len != 0);
  mhd_assert (chunk_hdr_len < sizeof(chunk_hdr));
  *p_finished = false;
  connection->write_buffer_send_offset =
    (max_chunk_hdr_len - (chunk_hdr_len + 2));
  memcpy (connection->write_buffer + connection->write_buffer_send_offset,
          chunk_hdr,
          chunk_hdr_len);
  connection->write_buffer[max_chunk_hdr_len - 2] = '\r';
  connection->write_buffer[max_chunk_hdr_len - 1] = '\n';
  connection->write_buffer[max_chunk_hdr_len + ret] = '\r';
  connection->write_buffer[max_chunk_hdr_len + ret + 1] = '\n';
  connection->response_write_position += ret;
  connection->write_buffer_append_offset = max_chunk_hdr_len + ret + 2;
  return MHD_YES;
}


/**
 * Are we allowed to keep the given connection alive?
 * We can use the TCP stream for a second request if the connection
 * is HTTP 1.1 and the "Connection" header either does not exist or
 * is not set to "close", or if the connection is HTTP 1.0 and the
 * "Connection" header is explicitly set to "keep-alive".
 * If no HTTP version is specified (or if it is not 1.0 or 1.1), we
 * definitively close the connection.  If the "Connection" header is
 * not exactly "close" or "keep-alive", we proceed to use the default
 * for the respective HTTP version.
 * If response has HTTP/1.0 flag or has "Connection: close" header
 * then connection must be closed.
 * If full request has not been read then connection must be closed
 * as well.
 *
 * @param connection the connection to check for keepalive
 * @return MHD_CONN_USE_KEEPALIVE if (based on the request and the response),
 *         a keepalive is legal,
 *         MHD_CONN_MUST_CLOSE if connection must be closed after sending
 *         complete reply,
 *         MHD_CONN_MUST_UPGRADE if connection must be upgraded.
 */
static enum MHD_ConnKeepAlive
keepalive_possible (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->response;  /**< a short alias */

  mhd_assert (NULL != r);
  if (MHD_CONN_MUST_CLOSE == c->keepalive)
    return MHD_CONN_MUST_CLOSE;

#ifdef UPGRADE_SUPPORT
  /* TODO: Move below the next check when MHD stops closing connections
   * when response is queued in first callback */
  if (NULL != r->upgrade_handler)
  {
    /* No "close" token is enforced by 'add_response_header_connection()' */
    mhd_assert (0 == (r->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE));
    /* Valid HTTP version is enforced by 'MHD_queue_response()' */
    mhd_assert (MHD_IS_HTTP_VER_SUPPORTED (c->http_ver));
    mhd_assert (! c->stop_with_error);
    return MHD_CONN_MUST_UPGRADE;
  }
#endif /* UPGRADE_SUPPORT */

  mhd_assert ( (! c->stop_with_error) || (c->discard_request));
  if ((c->read_closed) || (c->discard_request))
    return MHD_CONN_MUST_CLOSE;

  if (0 != (r->flags & MHD_RF_HTTP_1_0_COMPATIBLE_STRICT))
    return MHD_CONN_MUST_CLOSE;
  if (0 != (r->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE))
    return MHD_CONN_MUST_CLOSE;

  if (! MHD_IS_HTTP_VER_SUPPORTED (c->http_ver))
    return MHD_CONN_MUST_CLOSE;

  if (MHD_lookup_header_s_token_ci (c,
                                    MHD_HTTP_HEADER_CONNECTION,
                                    "close"))
    return MHD_CONN_MUST_CLOSE;

  if ((MHD_HTTP_VER_1_0 == connection->http_ver) ||
      (0 != (connection->response->flags & MHD_RF_HTTP_1_0_SERVER)))
  {
    if (MHD_lookup_header_s_token_ci (connection,
                                      MHD_HTTP_HEADER_CONNECTION,
                                      "Keep-Alive"))
      return MHD_CONN_USE_KEEPALIVE;

    return MHD_CONN_MUST_CLOSE;
  }

  if (MHD_IS_HTTP_VER_1_1_COMPAT (c->http_ver))
    return MHD_CONN_USE_KEEPALIVE;

  return MHD_CONN_MUST_CLOSE;
}


/**
 * Produce time stamp.
 *
 * Result is NOT null-terminated.
 * Result is always 29 bytes long.
 *
 * @param[out] date where to write the time stamp, with
 *             at least 29 bytes available space.
 */
static bool
get_date_str (char *date)
{
  static const char *const days[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char *const mons[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  static const size_t buf_len = 29;
  struct tm now;
  time_t t;
  const char *src;
#if ! defined(HAVE_C11_GMTIME_S) && ! defined(HAVE_W32_GMTIME_S) && \
  ! defined(HAVE_GMTIME_R)
  struct tm *pNow;
#endif

  if ((time_t) -1 == time (&t))
    return false;
#if defined(HAVE_C11_GMTIME_S)
  if (NULL == gmtime_s (&t,
                        &now))
    return false;
#elif defined(HAVE_W32_GMTIME_S)
  if (0 != gmtime_s (&now,
                     &t))
    return false;
#elif defined(HAVE_GMTIME_R)
  if (NULL == gmtime_r (&t,
                        &now))
    return false;
#else
  pNow = gmtime (&t);
  if (NULL == pNow)
    return false;
  now = *pNow;
#endif

  /* Day of the week */
  src = days[now.tm_wday % 7];
  date[0] = src[0];
  date[1] = src[1];
  date[2] = src[2];
  date[3] = ',';
  date[4] = ' ';
  /* Day of the month */
  if (2 != MHD_uint8_to_str_pad ((uint8_t) now.tm_mday, 2,
                                 date + 5, buf_len - 5))
    return false;
  date[7] = ' ';
  /* Month */
  src = mons[now.tm_mon % 12];
  date[8] = src[0];
  date[9] = src[1];
  date[10] = src[2];
  date[11] = ' ';
  /* Year */
  if (4 != MHD_uint16_to_str ((uint16_t) (1900 + now.tm_year), date + 12,
                              buf_len - 12))
    return false;
  date[16] = ' ';
  /* Time */
  MHD_uint8_to_str_pad ((uint8_t) now.tm_hour, 2, date + 17, buf_len - 17);
  date[19] = ':';
  MHD_uint8_to_str_pad ((uint8_t) now.tm_min, 2, date + 20, buf_len - 20);
  date[22] = ':';
  MHD_uint8_to_str_pad ((uint8_t) now.tm_sec, 2, date + 23, buf_len - 23);
  date[25] = ' ';
  date[26] = 'G';
  date[27] = 'M';
  date[28] = 'T';

  return true;
}


/**
 * Produce HTTP DATE header.
 * Result is always 37 bytes long (plus one terminating null).
 *
 * @param[out] header where to write the header, with
 *             at least 38 bytes available space.
 */
static bool
get_date_header (char *header)
{
  if (! get_date_str (header + 6))
  {
    header[0] = 0;
    return false;
  }
  header[0] = 'D';
  header[1] = 'a';
  header[2] = 't';
  header[3] = 'e';
  header[4] = ':';
  header[5] = ' ';
  header[35] = '\r';
  header[36] = '\n';
  header[37] = 0;
  return true;
}


/**
 * Try growing the read buffer.  We initially claim half the available
 * buffer space for the read buffer (the other half being left for
 * management data structures; the write buffer can in the end take
 * virtually everything as the read buffer can be reduced to the
 * minimum necessary at that point.
 *
 * @param connection the connection
 * @param required set to 'true' if grow is required, i.e. connection
 *                 will fail if no additional space is granted
 * @return 'true' on success, 'false' on failure
 */
static bool
try_grow_read_buffer (struct MHD_Connection *connection,
                      bool required)
{
  size_t new_size;
  size_t avail_size;
  void *rb;

  avail_size = MHD_pool_get_free (connection->pool);
  if (0 == avail_size)
    return false;               /* No more space available */
  if (0 == connection->read_buffer_size)
    new_size = avail_size / 2;  /* Use half of available buffer for reading */
  else
  {
    size_t grow_size;

    grow_size = avail_size / 8;
    if (MHD_BUF_INC_SIZE > grow_size)
    {                  /* Shortage of space */
      if (! required)
        return false;  /* Grow is not mandatory, leave some space in pool */
      else
      {
        /* Shortage of space, but grow is mandatory */
        static const size_t small_inc = MHD_BUF_INC_SIZE / 8;
        if (small_inc < avail_size)
          grow_size = small_inc;
        else
          grow_size = avail_size;
      }
    }
    new_size = connection->read_buffer_size + grow_size;
  }
  /* we can actually grow the buffer, do it! */
  rb = MHD_pool_reallocate (connection->pool,
                            connection->read_buffer,
                            connection->read_buffer_size,
                            new_size);
  if (NULL == rb)
  {
    /* This should NOT be possible: we just computed 'new_size' so that
       it should fit. If it happens, somehow our read buffer is not in
       the right position in the pool, say because someone called
       MHD_pool_allocate() without 'from_end' set to 'true'? Anyway,
       should be investigated! (Ideally provide all data from
       *pool and connection->read_buffer and new_size for debugging). */
    mhd_assert (0);
    return false;
  }
  connection->read_buffer = rb;
  mhd_assert (NULL != connection->read_buffer);
  connection->read_buffer_size = new_size;
  return true;
}


/**
 * Shrink connection read buffer to the zero size of free space in the buffer
 * @param connection the connection whose read buffer is being manipulated
 */
static void
connection_shrink_read_buffer (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  void *new_buf;

  if ((NULL == c->read_buffer) || (0 == c->read_buffer_size))
  {
    mhd_assert (0 == c->read_buffer_size);
    mhd_assert (0 == c->read_buffer_offset);
    return;
  }

  mhd_assert (c->read_buffer_offset <= c->read_buffer_size);
  new_buf = MHD_pool_reallocate (c->pool, c->read_buffer, c->read_buffer_size,
                                 c->read_buffer_offset);
  mhd_assert (c->read_buffer == new_buf);
  c->read_buffer = new_buf;
  c->read_buffer_size = c->read_buffer_offset;
}


/**
 * Allocate the maximum available amount of memory from MemoryPool
 * for write buffer.
 * @param connection the connection whose write buffer is being manipulated
 * @return the size of free space in write buffer, may be smaller
 *         than requested size.
 */
static size_t
connection_maximize_write_buffer (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MemoryPool *const pool = connection->pool;
  void *new_buf;
  size_t new_size;
  size_t free_size;

  mhd_assert ((NULL != c->write_buffer) || (0 == c->write_buffer_size));
  mhd_assert (c->write_buffer_append_offset >= c->write_buffer_send_offset);
  mhd_assert (c->write_buffer_size >= c->write_buffer_append_offset);

  free_size = MHD_pool_get_free (pool);
  if (0 != free_size)
  {
    new_size = c->write_buffer_size + free_size;
    /* This function must not move the buffer position.
     * MHD_pool_reallocate () may return the new position only if buffer was
     * allocated 'from_end' or is not the last allocation,
     * which should not happen. */
    new_buf = MHD_pool_reallocate (pool,
                                   c->write_buffer,
                                   c->write_buffer_size,
                                   new_size);
    mhd_assert ((c->write_buffer == new_buf) || (NULL == c->write_buffer));
    c->write_buffer = new_buf;
    c->write_buffer_size = new_size;
    if (c->write_buffer_send_offset == c->write_buffer_append_offset)
    {
      /* All data have been sent, reset offsets to zero. */
      c->write_buffer_send_offset = 0;
      c->write_buffer_append_offset = 0;
    }
  }

  return c->write_buffer_size - c->write_buffer_append_offset;
}


#if 0 /* disable unused function */
/**
 * Shrink connection write buffer to the size of unsent data.
 *
 * @note: The number of calls of this function should be limited to avoid extra
 * zeroing of the memory.
 * @param connection the connection whose write buffer is being manipulated
 * @param connection the connection to manipulate write buffer
 */
static void
connection_shrink_write_buffer (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MemoryPool *const pool = connection->pool;
  void *new_buf;

  mhd_assert ((NULL != c->write_buffer) || (0 == c->write_buffer_size));
  mhd_assert (c->write_buffer_append_offset >= c->write_buffer_send_offset);
  mhd_assert (c->write_buffer_size >= c->write_buffer_append_offset);

  if ( (NULL == c->write_buffer) || (0 == c->write_buffer_size))
  {
    mhd_assert (0 == c->write_buffer_append_offset);
    mhd_assert (0 == c->write_buffer_send_offset);
    c->write_buffer = NULL;
    return;
  }
  if (c->write_buffer_append_offset == c->write_buffer_size)
    return;

  new_buf = MHD_pool_reallocate (pool, c->write_buffer, c->write_buffer_size,
                                 c->write_buffer_append_offset);
  mhd_assert ((c->write_buffer == new_buf) || \
              (0 == c->write_buffer_append_offset));
  c->write_buffer_size = c->write_buffer_append_offset;
  if (0 == c->write_buffer_size)
    c->write_buffer = NULL;
  else
    c->write_buffer = new_buf;
}


#endif /* unused function */


/**
 * Switch connection from recv mode to send mode.
 *
 * Current request header or body will not be read anymore,
 * response must be assigned to connection.
 * @param connection the connection to prepare for sending.
 */
static void
connection_switch_from_recv_to_send (struct MHD_Connection *connection)
{
  /* Read buffer is not needed for this request, shrink it.*/
  connection_shrink_read_buffer (connection);
}


/**
 * Check whether reply body-specific headers (namely Content-Length,
 * Transfer-Encoding) are needed.
 *
 * If reply body-specific headers are not needed then body itself
 * is not allowed as well.
 * When reply body-specific headers are needed, the body itself
 * can be present or not, depending on other conditions.
 *
 * @param connection the connection to check
 * @return true if reply body-specific headers are needed,
 *         false otherwise.
 * @sa is_reply_body_needed()
 */
static bool
is_reply_body_headers_needed (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  unsigned rcode;  /**< the response code */

  mhd_assert (100 <= (c->responseCode & (~MHD_ICY_FLAG)) && \
              999 >= (c->responseCode & (~MHD_ICY_FLAG)));

  rcode = (unsigned) (c->responseCode & (~MHD_ICY_FLAG));

  if (199 >= rcode)
    return false;

  if (MHD_HTTP_NO_CONTENT == rcode)
    return false;

#ifdef UPGRADE_SUPPORT
  if (NULL != c->response->upgrade_handler)
    return false;
#endif /* UPGRADE_SUPPORT */

  if ( (MHD_HTTP_MTHD_CONNECT == c->http_mthd) &&
       (2 == rcode / 100) )
    return false; /* Actually pass-through CONNECT is not supported by MHD */

  return true;
}


/**
 * Check whether reply body must be used.
 *
 * If reply body is needed, it could be zero-sized.
 *
 * @param connection the connection to check
 * @return true if reply body must be used,
 *         false otherwise
 * @sa is_reply_body_header_needed()
 */
static bool
is_reply_body_needed (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  unsigned rcode;  /**< the response code */

  mhd_assert (100 <= (c->responseCode & (~MHD_ICY_FLAG)) && \
              999 >= (c->responseCode & (~MHD_ICY_FLAG)));

  if (! is_reply_body_headers_needed (c))
    return false;

  if (MHD_HTTP_MTHD_HEAD == c->http_mthd)
    return false;

  rcode = (unsigned) (c->responseCode & (~MHD_ICY_FLAG));
  if (MHD_HTTP_NOT_MODIFIED == rcode)
    return false;

  return true;
}


/**
 * Setup connection reply properties.
 *
 * Reply properties include presence of reply body, transfer-encoding
 * type and other.
 *
 * @param connection to connection to process
 * @param reply_body_allowed
 */
static void
setup_reply_properties (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->response;  /**< a short alias */
  bool use_chunked;

  mhd_assert (NULL != r);

  /* ** Adjust reply properties ** */

  c->keepalive = keepalive_possible (c);
  c->rp_props.use_reply_body_headers = is_reply_body_headers_needed (c);
  if (c->rp_props.use_reply_body_headers)
    c->rp_props.send_reply_body = is_reply_body_needed (c);
  else
    c->rp_props.send_reply_body = false;

  if (c->rp_props.use_reply_body_headers)
  {
    if ((MHD_SIZE_UNKNOWN == r->total_size) ||
        (0 != (r->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED)))
    { /* Use chunked reply encoding if possible */

      /* Check whether chunked encoding is supported by the client */
      if (! MHD_IS_HTTP_VER_1_1_COMPAT (c->http_ver))
        use_chunked = false;
      /* Check whether chunked encoding is allowed for the reply */
      else if (0 != (r->flags & (MHD_RF_HTTP_1_0_COMPATIBLE_STRICT
                                 | MHD_RF_HTTP_1_0_SERVER)))
        use_chunked = false;
      else
        /* If chunked encoding is supported and allowed, and response size
         * is unknown, use chunked even for non-Keep-Alive connections.
         * See https://datatracker.ietf.org/doc/html/rfc7230#section-3.3.3
         * Also use chunked if it is enforced by application and supported by
         * the client. */
        use_chunked = true;
    }
    else
      use_chunked = false;

    if ( (MHD_SIZE_UNKNOWN == r->total_size) && ! use_chunked)
    {
      /* End of the stream is indicated by closure */
      c->keepalive = MHD_CONN_MUST_CLOSE;
    }
  }
  else
    use_chunked = false; /* chunked encoding cannot be used without body */

  c->rp_props.chunked = use_chunked;
  c->rp_props.set = true;
}


/**
 * Append data to the buffer if enough space is available,
 * update position.
 * @param[out] buf the buffer to append data to
 * @param[in,out] ppos the pointer to position in the @a buffer
 * @param buf_size the size of the @a buffer
 * @param append the data to append
 * @param append_size the size of the @a append
 * @return true if data has been added and position has been updated,
 *         false if not enough space is available
 */
static bool
buffer_append (char *buf,
               size_t *ppos,
               size_t buf_size,
               const char *append,
               size_t append_size)
{
  mhd_assert (NULL != buf); /* Mute static analyzer */
  if (buf_size < *ppos + append_size)
    return false;
  memcpy (buf + *ppos, append, append_size);
  *ppos += append_size;
  return true;
}


/**
 * Append static string to the buffer if enough space is available,
 * update position.
 * @param[out] buf the buffer to append data to
 * @param[in,out] ppos the pointer to position in the @a buffer
 * @param buf_size the size of the @a buffer
 * @param str the static string to append
 * @return true if data has been added and position has been updated,
 *         false if not enough space is available
 */
#define buffer_append_s(buf,ppos,buf_size,str) \
  buffer_append(buf,ppos,buf_size,str, MHD_STATICSTR_LEN_(str))


/**
 * Add user-defined headers from response object to
 * the text buffer.
 *
 * @param buf the buffer to add headers to
 * @param ppos the pointer to the position in the @a buf
 * @param buf_size the size of the @a buf
 * @param response the response
 * @param kind the kind of objects (headers or footers)
 * @param filter_transf_enc skip "Transfer-Encoding" header if any
 * @param add_close add "close" token to the
 *                  "Connection:" header (if any), ignored if no "Connection:"
 *                  header was added by user or if "close" token is already
 *                  present in "Connection:" header
 * @param add_keep_alive add "Keep-Alive" token to the
 *                       "Connection:" header (if any)
 * @return true if succeed,
 *         false if buffer is too small
 */
static bool
add_user_headers (char *buf,
                  size_t *ppos,
                  size_t buf_size,
                  struct MHD_Response *response,
                  enum MHD_ValueKind kind,
                  bool filter_transf_enc,
                  bool add_close,
                  bool add_keep_alive)
{
  struct MHD_Response *const r = response; /**< a short alias */
  struct MHD_HTTP_Header *hdr; /**< Iterates through User-specified headers */
  size_t el_size; /**< the size of current element to be added to the @a buf */

  mhd_assert ((! filter_transf_enc) || MHD_HEADER_KIND == kind);
  mhd_assert ((! add_close) || MHD_HEADER_KIND == kind);
  mhd_assert ((! add_keep_alive) || MHD_HEADER_KIND == kind);
  mhd_assert (! add_close || ! add_keep_alive);

  if (0 == (r->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED))
    filter_transf_enc = false;  /* No such header */
  if (0 == (r->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
  {
    add_close = false;          /* No such header */
    add_keep_alive = false;     /* No such header */
  }
  else if (0 != (r->flags_auto & MHD_RAF_HAS_CONNECTION_CLOSE))
    add_close = false;          /* "close" token was already set */

  for (hdr = r->first_header; NULL != hdr; hdr = hdr->next)
  {
    size_t initial_pos = *ppos;
    if (kind != hdr->kind)
      continue;
    if (filter_transf_enc)
    { /* Need to filter-out "Transfer-Encoding" */
      if ((MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_TRANSFER_ENCODING) ==
           hdr->header_size) &&
          (MHD_str_equal_caseless_bin_n_ (MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                          hdr->header, hdr->header_size)) )
      {
        filter_transf_enc = false; /* There is the only one such header */
        continue; /* Skip "Transfer-Encoding" header */
      }
    }

    /* Add user header */
    el_size = hdr->header_size + 2 + hdr->value_size + 2;
    if (buf_size < *ppos + el_size)
      return false;
    memcpy (buf + *ppos, hdr->header, hdr->header_size);
    (*ppos) += hdr->header_size;
    buf[(*ppos)++] = ':';
    buf[(*ppos)++] = ' ';
    if (add_close || add_keep_alive)
    {
      /* "Connection:" header must be always the first one */
      mhd_assert (MHD_str_equal_caseless_n_ (hdr->header, \
                                             MHD_HTTP_HEADER_CONNECTION, \
                                             hdr->header_size));

      if (add_close)
      {
        el_size += MHD_STATICSTR_LEN_ ("close, ");
        if (buf_size < initial_pos + el_size)
          return false;
        memcpy (buf + *ppos, "close, ",
                MHD_STATICSTR_LEN_ ("close, "));
        *ppos += MHD_STATICSTR_LEN_ ("close, ");
      }
      else
      {
        el_size += MHD_STATICSTR_LEN_ ("Keep-Alive, ");
        if (buf_size < initial_pos + el_size)
          return false;
        memcpy (buf + *ppos, "Keep-Alive, ",
                MHD_STATICSTR_LEN_ ("Keep-Alive, "));
        *ppos += MHD_STATICSTR_LEN_ ("Keep-Alive, ");
      }
      add_close = false;
      add_keep_alive = false;
    }
    if (0 != hdr->value_size)
      memcpy (buf + *ppos, hdr->value, hdr->value_size);
    *ppos += hdr->value_size;
    buf[(*ppos)++] = '\r';
    buf[(*ppos)++] = '\n';
    mhd_assert (initial_pos + el_size == (*ppos));
  }
  return true;
}


/**
 * Allocate the connection's write buffer and fill it with all of the
 * headers from the response.
 * Required headers are added here.
 *
 * @param connection the connection
 * @return #MHD_YES on success, #MHD_NO on failure (out of memory)
 */
static enum MHD_Result
build_header_response (struct MHD_Connection *connection)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Response *const r = c->response;  /**< a short alias */
  char *buf;                                   /**< the output buffer */
  size_t pos;                                  /**< append offset in the @a buf */
  size_t buf_size;                             /**< the size of the @a buf */
  size_t el_size;                              /**< the size of current element to be added to the @a buf */
  unsigned rcode;                              /**< the response code */
  bool use_conn_close;                         /**< Use "Connection: close" header */
  bool use_conn_k_alive;                       /**< Use "Connection: Keep-Alive" header */

  mhd_assert (NULL != r);

  /* ** Adjust response properties ** */

  setup_reply_properties (c);

  mhd_assert (c->rp_props.set);
  mhd_assert ((MHD_CONN_MUST_CLOSE == c->keepalive) || \
              (MHD_CONN_USE_KEEPALIVE == c->keepalive) || \
              (MHD_CONN_MUST_UPGRADE == c->keepalive));
#ifdef UPGRADE_SUPPORT
  mhd_assert ((NULL == r->upgrade_handler) || \
              (MHD_CONN_MUST_UPGRADE == c->keepalive));
#else  /* ! UPGRADE_SUPPORT */
  mhd_assert (MHD_CONN_MUST_UPGRADE != c->keepalive);
#endif /* ! UPGRADE_SUPPORT */
  mhd_assert ((! c->rp_props.chunked) || c->rp_props.use_reply_body_headers);
  mhd_assert ((! c->rp_props.send_reply_body) || \
              c->rp_props.use_reply_body_headers);
#ifdef UPGRADE_SUPPORT
  mhd_assert (NULL == r->upgrade_handler || \
              ! c->rp_props.use_reply_body_headers);
#endif /* UPGRADE_SUPPORT */

  rcode = (unsigned) (c->responseCode & (~MHD_ICY_FLAG));
  if (MHD_CONN_MUST_CLOSE == c->keepalive)
  {
    /* The closure of connection must be always indicated by header
     * to avoid hung connections */
    use_conn_close = true;
    use_conn_k_alive = false;
  }
  else if (MHD_CONN_USE_KEEPALIVE == c->keepalive)
  {
    use_conn_close = false;
    /* Add "Connection: keep-alive" if request is HTTP/1.0 or
     * if reply is HTTP/1.0
     * For HTTP/1.1 add header only if explicitly requested by app
     * (by response flag), as "Keep-Alive" is default for HTTP/1.1. */
    if ((0 != (r->flags & MHD_RF_SEND_KEEP_ALIVE_HEADER)) ||
        (MHD_HTTP_VER_1_0 == c->http_ver) ||
        (0 != (r->flags & MHD_RF_HTTP_1_0_SERVER)))
      use_conn_k_alive = true;
    else
      use_conn_k_alive = false;
  }
  else
  {
    use_conn_close = false;
    use_conn_k_alive = false;
  }

  /* ** Actually build the response header ** */

  /* Get all space available */
  connection_maximize_write_buffer (c);
  buf = c->write_buffer;
  pos = c->write_buffer_append_offset;
  buf_size = c->write_buffer_size;
  if (0 == buf_size)
    return MHD_NO;
  mhd_assert (NULL != buf);

  /* * The status line * */

  /* The HTTP version */
  if (0 == (c->responseCode & MHD_ICY_FLAG))
  { /* HTTP reply */
    if (0 == (r->flags & MHD_RF_HTTP_1_0_SERVER))
    { /* HTTP/1.1 reply */
      /* Use HTTP/1.1 responses for HTTP/1.0 clients.
       * See https://datatracker.ietf.org/doc/html/rfc7230#section-2.6 */
      if (! buffer_append_s (buf, &pos, buf_size, MHD_HTTP_VERSION_1_1))
        return MHD_NO;
    }
    else
    { /* HTTP/1.0 reply */
      if (! buffer_append_s (buf, &pos, buf_size, MHD_HTTP_VERSION_1_0))
        return MHD_NO;
    }
  }
  else
  { /* ICY reply */
    if (! buffer_append_s (buf, &pos, buf_size, "ICY"))
      return MHD_NO;
  }

  /* The response code */
  if (buf_size < pos + 5) /* space + code + space */
    return MHD_NO;
  buf[pos++] = ' ';
  pos += MHD_uint16_to_str (rcode, buf + pos,
                            buf_size - pos);
  buf[pos++] = ' ';

  /* The reason phrase */
  el_size = MHD_get_reason_phrase_len_for (rcode);
  if (0 == el_size)
  {
    if (! buffer_append_s (buf, &pos, buf_size, "Non-Standard Status"))
      return MHD_NO;
  }
  else if (! buffer_append (buf, &pos, buf_size,
                            MHD_get_reason_phrase_for (rcode),
                            el_size))
    return MHD_NO;

  /* The linefeed */
  if (buf_size < pos + 2)
    return MHD_NO;
  buf[pos++] = '\r';
  buf[pos++] = '\n';

  /* * The headers * */

  /* Main automatic headers */

  /* The "Date:" header */
  if ( (0 == (r->flags_auto & MHD_RAF_HAS_DATE_HDR)) &&
       (0 == (c->daemon->options & MHD_USE_SUPPRESS_DATE_NO_CLOCK)) )
  {
    /* Additional byte for unused zero-termination */
    if (buf_size < pos + 38)
      return MHD_NO;
    if (get_date_header (buf + pos))
      pos += 37;
  }
  /* The "Connection:" header */
  mhd_assert (! use_conn_close || ! use_conn_k_alive);
  mhd_assert (! use_conn_k_alive || ! use_conn_close);
  if (0 == (r->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
  {
    if (use_conn_close)
    {
      if (! buffer_append_s (buf, &pos, buf_size,
                             MHD_HTTP_HEADER_CONNECTION ": close\r\n"))
        return MHD_NO;
    }
    else if (use_conn_k_alive)
    {
      if (! buffer_append_s (buf, &pos, buf_size,
                             MHD_HTTP_HEADER_CONNECTION ": Keep-Alive\r\n"))
        return MHD_NO;
    }
  }

  /* User-defined headers */

  if (! add_user_headers (buf, &pos, buf_size, r, MHD_HEADER_KIND,
                          ! c->rp_props.chunked,
                          use_conn_close,
                          use_conn_k_alive))
    return MHD_NO;

  /* Other automatic headers */

  if (c->rp_props.use_reply_body_headers)
  {
    /* Body-specific headers */
    if (c->rp_props.chunked)
    { /* Chunked encoding is used */
      if (0 == (r->flags_auto & MHD_RAF_HAS_TRANS_ENC_CHUNKED))
      { /* No chunked encoding header set by user */
        if (! buffer_append_s (buf, &pos, buf_size,
                               MHD_HTTP_HEADER_TRANSFER_ENCODING ": " \
                               "chunked\r\n"))
          return MHD_NO;
      }
    }
    else
    { /* Chunked encoding is not used */
      if (MHD_SIZE_UNKNOWN != r->total_size)
      {
        if (! buffer_append_s (buf, &pos, buf_size,
                               MHD_HTTP_HEADER_CONTENT_LENGTH ": "))
          return MHD_NO;
        el_size = MHD_uint64_to_str (r->total_size, buf + pos,
                                     buf_size - pos);
        if (0 == el_size)
          return MHD_NO;
        pos += el_size;

        if (buf_size < pos + 2)
          return MHD_NO;
        buf[pos++] = '\r';
        buf[pos++] = '\n';
      }
    }
  }

  /* * Header termination * */
  if (buf_size < pos + 2)
    return MHD_NO;
  buf[pos++] = '\r';
  buf[pos++] = '\n';

  c->write_buffer_append_offset = pos;
  return MHD_YES;
}


/**
 * Allocate the connection's write buffer (if necessary) and fill it
 * with response footers.
 * Works only for chunked responses as other responses do not need
 * and do not support any kind of footers.
 *
 * @param connection the connection
 * @return #MHD_YES on success, #MHD_NO on failure (out of memory)
 */
static enum MHD_Result
build_connection_chunked_response_footer (struct MHD_Connection *connection)
{
  char *buf;           /**< the buffer to write footers to */
  size_t buf_size;     /**< the size of the @a buf */
  size_t used_size;    /**< the used size of the @a buf */
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_HTTP_Header *pos;

  mhd_assert (connection->rp_props.chunked);
  /* TODO: allow combining of the final footer with the last chunk,
   * modify the next assert. */
  mhd_assert (MHD_CONNECTION_BODY_SENT == connection->state);
  mhd_assert (NULL != c->response);

  buf_size = connection_maximize_write_buffer (c);
  /* '5' is the minimal size of chunked footer ("0\r\n\r\n") */
  if (buf_size < 5)
    return MHD_NO;
  mhd_assert (NULL != c->write_buffer);
  buf = c->write_buffer + c->write_buffer_append_offset;
  mhd_assert (NULL != buf);
  used_size = 0;
  buf[used_size++] = '0';
  buf[used_size++] = '\r';
  buf[used_size++] = '\n';

  for (pos = c->response->first_header; NULL != pos; pos = pos->next)
  {
    if (MHD_FOOTER_KIND == pos->kind)
    {
      size_t new_used_size; /* resulting size with this header */
      /* '4' is colon, space, linefeeds */
      new_used_size = used_size + pos->header_size + pos->value_size + 4;
      if (new_used_size > buf_size)
        return MHD_NO;
      memcpy (buf + used_size, pos->header, pos->header_size);
      used_size += pos->header_size;
      buf[used_size++] = ':';
      buf[used_size++] = ' ';
      memcpy (buf + used_size, pos->value, pos->value_size);
      used_size += pos->value_size;
      buf[used_size++] = '\r';
      buf[used_size++] = '\n';
      mhd_assert (used_size == new_used_size);
    }
  }
  if (used_size + 2 > buf_size)
    return MHD_NO;
  buf[used_size++] = '\r';
  buf[used_size++] = '\n';

  c->write_buffer_append_offset += used_size;
  mhd_assert (c->write_buffer_append_offset <= c->write_buffer_size);

  return MHD_YES;
}


/**
 * We encountered an error processing the request.
 * Handle it properly by stopping to read data
 * and sending the indicated response code and message.
 *
 * @param connection the connection
 * @param status_code the response code to send (400, 413 or 414)
 * @param message the error message to send
 * @param message_len the length of the @a message
 */
static void
transmit_error_response_len (struct MHD_Connection *connection,
                             unsigned int status_code,
                             const char *message,
                             size_t message_len)
{
  struct MHD_Response *response;
  enum MHD_Result iret;

  mhd_assert (! connection->stop_with_error); /* Do not send error twice */
  if (connection->stop_with_error)
  { /* Should not happen */
    if (MHD_CONNECTION_CLOSED > connection->state)
      connection->state = MHD_CONNECTION_CLOSED;

    return;
  }
  connection->stop_with_error = true;
  connection->discard_request = true;
#ifdef HAVE_MESSAGES
  MHD_DLOG (connection->daemon,
            _ ("Error processing request (HTTP response code is %u ('%s')). " \
               "Closing connection.\n"),
            status_code,
            message);
#endif
  if (MHD_CONNECTION_START_REPLY < connection->state)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Too late to send an error response, " \
                 "response is being sent already.\n"),
              status_code,
              message);
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Too late for error response."));
    return;
  }
  /* TODO: remove when special error queue function is implemented */
  connection->state = MHD_CONNECTION_FULL_REQ_RECEIVED;
  if (0 != connection->read_buffer_size)
  {
    /* Read buffer is not needed anymore, discard it
     * to free some space for error response. */
    connection->read_buffer = MHD_pool_reallocate (connection->pool,
                                                   connection->read_buffer,
                                                   connection->read_buffer_size,
                                                   0);
    connection->read_buffer_size = 0;
    connection->read_buffer_offset = 0;
  }
  if (NULL != connection->response)
  {
    MHD_destroy_response (connection->response);
    connection->response = NULL;
  }
  response = MHD_create_response_from_buffer (message_len,
                                              (void *) message,
                                              MHD_RESPMEM_PERSISTENT);
  if (NULL == response)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Failed to create error response.\n"),
              status_code,
              message);
#endif
    /* can't even send a reply, at least close the connection */
    connection->state = MHD_CONNECTION_CLOSED;
    return;
  }
  iret = MHD_queue_response (connection,
                             status_code,
                             response);
  MHD_destroy_response (response);
  if (MHD_NO == iret)
  {
    /* can't even send a reply, at least close the connection */
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Closing connection " \
                               "(failed to queue error response)."));
    return;
  }
  mhd_assert (NULL != connection->response);
  /* Do not reuse this connection. */
  connection->keepalive = MHD_CONN_MUST_CLOSE;
  if (MHD_NO == build_header_response (connection))
  {
    /* No memory. Release everything. */
    connection->version = NULL;
    connection->method = NULL;
    connection->url = NULL;
    connection->last = NULL;
    connection->colon = NULL;
    connection->headers_received = NULL;
    connection->headers_received_tail = NULL;
    connection->write_buffer = NULL;
    connection->write_buffer_size = 0;
    connection->write_buffer_send_offset = 0;
    connection->write_buffer_append_offset = 0;
    connection->read_buffer
      = MHD_pool_reset (connection->pool,
                        NULL,
                        0,
                        0);
    connection->read_buffer_size = 0;

    /* Retry with empty buffer */
    if (MHD_NO == build_header_response (connection))
    {
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Closing connection " \
                                 "(failed to create error response header)."));
      return;
    }
  }
  connection->state = MHD_CONNECTION_HEADERS_SENDING;
}


/**
 * Transmit static string as error response
 */
#define transmit_error_response_static(c, code, msg) \
  transmit_error_response_len (c, code, msg, MHD_STATICSTR_LEN_ (msg))

/**
 * Update the 'event_loop_info' field of this connection based on the state
 * that the connection is now in.  May also close the connection or
 * perform other updates to the connection if needed to prepare for
 * the next round of the event loop.
 *
 * @param connection connection to get poll set for
 */
static void
MHD_connection_update_event_loop_info (struct MHD_Connection *connection)
{
  /* Do not update states of suspended connection */
  if (connection->suspended)
    return; /* States will be updated after resume. */
#ifdef HTTPS_SUPPORT
  if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
  {   /* HTTPS connection. */
    switch (connection->tls_state)
    {
    case MHD_TLS_CONN_INIT:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      return;
    case MHD_TLS_CONN_HANDSHAKING:
      if (0 == gnutls_record_get_direction (connection->tls_session))
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      else
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      return;
    default:
      break;
    }
  }
#endif /* HTTPS_SUPPORT */
  while (1)
  {
#if DEBUG_STATES
    MHD_DLOG (connection->daemon,
              _ ("In function %s handling connection at state: %s\n"),
              MHD_FUNC_,
              MHD_state_to_string (connection->state));
#endif
    switch (connection->state)
    {
    case MHD_CONNECTION_INIT:
    case MHD_CONNECTION_REQ_LINE_RECEIVING:
    case MHD_CONNECTION_URL_RECEIVED:
    case MHD_CONNECTION_HEADER_PART_RECEIVED:
      /* while reading headers, we always grow the
         read buffer if needed, no size-check required */
      if ( (connection->read_buffer_offset == connection->read_buffer_size) &&
           (! try_grow_read_buffer (connection, true)) )
      {
        if (connection->url != NULL)
          transmit_error_response_static (connection,
                                          MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
                                          REQUEST_TOO_BIG);
        else
          transmit_error_response_static (connection,
                                          MHD_HTTP_URI_TOO_LONG,
                                          REQUEST_TOO_BIG);
        continue;
      }
      if (! connection->discard_request)
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      else
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_BLOCK;
      break;
    case MHD_CONNECTION_HEADERS_RECEIVED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_HEADERS_PROCESSED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_CONTINUE_SENDING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_CONTINUE_SENT:
      if (connection->read_buffer_offset == connection->read_buffer_size)
      {
        const bool internal_poll = (0 != (connection->daemon->options
                                          & MHD_USE_INTERNAL_POLLING_THREAD));
        if ( (! try_grow_read_buffer (connection, true)) &&
             internal_poll)
        {
          /* failed to grow the read buffer, and the
             client which is supposed to handle the
             received data in a *blocking* fashion
             (in this mode) did not handle the data as
             it was supposed to!
             => we would either have to do busy-waiting
             (on the client, which would likely fail),
             or if we do nothing, we would just timeout
             on the connection (if a timeout is even
             set!).
             Solution: we kill the connection with an error */
          transmit_error_response_static (connection,
                                          MHD_HTTP_INTERNAL_SERVER_ERROR,
                                          INTERNAL_ERROR);
          continue;
        }
      }
      if ( (connection->read_buffer_offset < connection->read_buffer_size) &&
           (! connection->discard_request) )
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      else
        connection->event_loop_info = MHD_EVENT_LOOP_INFO_BLOCK;
      break;
    case MHD_CONNECTION_BODY_RECEIVED:
    case MHD_CONNECTION_FOOTER_PART_RECEIVED:
      /* while reading footers, we always grow the
         read buffer if needed, no size-check required */
      if (connection->read_closed)
      {
        CONNECTION_CLOSE_ERROR (connection,
                                NULL);
        continue;
      }
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_READ;
      /* transition to FOOTERS_RECEIVED
         happens in read handler */
      break;
    case MHD_CONNECTION_FOOTERS_RECEIVED:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_FULL_REQ_RECEIVED:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_BLOCK;
      break;
    case MHD_CONNECTION_START_REPLY:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_HEADERS_SENDING:
      /* headers in buffer, keep writing */
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_HEADERS_SENT:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_NORMAL_BODY_READY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_NORMAL_BODY_UNREADY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_BLOCK;
      break;
    case MHD_CONNECTION_CHUNKED_BODY_READY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_BLOCK;
      break;
    case MHD_CONNECTION_BODY_SENT:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_FOOTERS_SENDING:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_WRITE;
      break;
    case MHD_CONNECTION_FOOTERS_SENT:
      mhd_assert (0);
      break;
    case MHD_CONNECTION_CLOSED:
      connection->event_loop_info = MHD_EVENT_LOOP_INFO_CLEANUP;
      return;           /* do nothing, not even reading */
#ifdef UPGRADE_SUPPORT
    case MHD_CONNECTION_UPGRADE:
      mhd_assert (0);
      break;
#endif /* UPGRADE_SUPPORT */
    default:
      mhd_assert (0);
    }
    break;
  }
}


/**
 * Parse a single line of the HTTP header.  Advance read_buffer (!)
 * appropriately.  If the current line does not fit, consider growing
 * the buffer.  If the line is far too long, close the connection.  If
 * no line is found (incomplete, buffer too small, line too long),
 * return NULL.  Otherwise return a pointer to the line.
 *
 * @param connection connection we're processing
 * @param[out] line_len pointer to variable that receive
 *             length of line or NULL
 * @return NULL if no full line is available; note that the returned
 *         string will not be 0-termianted
 */
static char *
get_next_header_line (struct MHD_Connection *connection,
                      size_t *line_len)
{
  char *rbuf;
  size_t pos;

  if (0 == connection->read_buffer_offset)
    return NULL;
  pos = 0;
  rbuf = connection->read_buffer;
  mhd_assert (NULL != rbuf);

  do
  {
    const char c = rbuf[pos];
    bool found;
    found = false;
    if ( ('\r' == c) && (pos < connection->read_buffer_offset - 1) &&
         ('\n' == rbuf[pos + 1]) )
    { /* Found CRLF */
      found = true;
      if (line_len)
        *line_len = pos;
      rbuf[pos++] = 0; /* Replace CR with zero */
      rbuf[pos++] = 0; /* Replace LF with zero */
    }
    else if ('\n' == c) /* TODO: Add MHD option to disallow */
    { /* Found bare LF */
      found = true;
      if (line_len)
        *line_len = pos;
      rbuf[pos++] = 0; /* Replace LF with zero */
    }
    if (found)
    {
      connection->read_buffer += pos;
      connection->read_buffer_size -= pos;
      connection->read_buffer_offset -= pos;
      return rbuf;
    }
  } while (++pos < connection->read_buffer_offset);

  /* not found, consider growing... */
  if ( (connection->read_buffer_offset == connection->read_buffer_size) &&
       (! try_grow_read_buffer (connection, true)) )
  {
    if (NULL != connection->url)
      transmit_error_response_static (connection,
                                      MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
                                      REQUEST_TOO_BIG);
    else
      transmit_error_response_static (connection,
                                      MHD_HTTP_URI_TOO_LONG,
                                      REQUEST_TOO_BIG);
  }
  if (line_len)
    *line_len = 0;
  return NULL;
}


/**
 * Add an entry to the HTTP headers of a connection.  If this fails,
 * transmit an error response (request too big).
 *
 * @param connection the connection for which a
 *  value should be set
 * @param kind kind of the value
 * @param key key for the value
 * @param key_size number of bytes in @a key
 * @param value the value itself
 * @param value_size number of bytes in @a value
 * @return #MHD_NO on failure (out of memory), #MHD_YES for success
 */
static enum MHD_Result
connection_add_header (struct MHD_Connection *connection,
                       const char *key,
                       size_t key_size,
                       const char *value,
                       size_t value_size,
                       enum MHD_ValueKind kind)
{
  if (MHD_NO ==
      MHD_set_connection_value_n (connection,
                                  kind,
                                  key,
                                  key_size,
                                  value,
                                  value_size))
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Not enough memory in pool to allocate header record!\n"));
#endif
    transmit_error_response_static (connection,
                                    MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
                                    REQUEST_TOO_BIG);
    return MHD_NO;
  }
  return MHD_YES;
}


/**
 * Parse the cookie header (see RFC 2109).
 *
 * @param connection connection to parse header of
 * @return #MHD_YES for success, #MHD_NO for failure (malformed, out of memory)
 */
static enum MHD_Result
parse_cookie_header (struct MHD_Connection *connection)
{
  const char *hdr;
  size_t hdr_len;
  char *cpy;
  char *pos;
  char *sce;
  char *semicolon;
  char *equals;
  char *ekill;
  char *end;
  char old;
  int quotes;

  if (MHD_NO == MHD_lookup_connection_value_n (connection,
                                               MHD_HEADER_KIND,
                                               MHD_HTTP_HEADER_COOKIE,
                                               MHD_STATICSTR_LEN_ (
                                                 MHD_HTTP_HEADER_COOKIE),
                                               &hdr,
                                               &hdr_len))
    return MHD_YES;
  cpy = connection_alloc_memory (connection,
                                 hdr_len + 1);
  if (NULL == cpy)
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Not enough memory in pool to parse cookies!\n"));
#endif
    transmit_error_response_static (connection,
                                    MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE,
                                    REQUEST_TOO_BIG);
    return MHD_NO;
  }
  memcpy (cpy,
          hdr,
          hdr_len);
  cpy[hdr_len] = '\0';
  pos = cpy;
  while (NULL != pos)
  {
    while (' ' == *pos)
      pos++;                    /* skip spaces */

    sce = pos;
    while ( ((*sce) != '\0') &&
            ((*sce) != ',') &&
            ((*sce) != ';') &&
            ((*sce) != '=') )
      sce++;
    /* remove tailing whitespace (if any) from key */
    ekill = sce - 1;
    while ( (*ekill == ' ') &&
            (ekill >= pos) )
      *(ekill--) = '\0';
    old = *sce;
    *sce = '\0';
    if (old != '=')
    {
      /* value part omitted, use empty string... */
      if (MHD_NO ==
          connection_add_header (connection,
                                 pos,
                                 ekill - pos + 1,
                                 "",
                                 0,
                                 MHD_COOKIE_KIND))
        return MHD_NO;
      if (old == '\0')
        break;
      pos = sce + 1;
      continue;
    }
    equals = sce + 1;
    quotes = 0;
    semicolon = equals;
    while ( ('\0' != semicolon[0]) &&
            ( (0 != quotes) ||
              ( (';' != semicolon[0]) &&
                (',' != semicolon[0]) ) ) )
    {
      if ('"' == semicolon[0])
        quotes = (quotes + 1) & 1;
      semicolon++;
    }
    end = semicolon;
    if ('\0' == semicolon[0])
      semicolon = NULL;
    if (NULL != semicolon)
    {
      semicolon[0] = '\0';
      semicolon++;
    }
    /* remove quotes */
    if ( ('"' == equals[0]) &&
         ('"' == end[-1]) )
    {
      equals++;
      end--;
      *end = '\0';
    }
    if (MHD_NO ==
        connection_add_header (connection,
                               pos,
                               ekill - pos + 1,
                               equals,
                               end - equals,
                               MHD_COOKIE_KIND))
      return MHD_NO;
    pos = semicolon;
  }
  return MHD_YES;
}


/**
 * Detect HTTP version
 *
 * @param connection the connection
 * @param http_string the pointer to HTTP version string
 * @param len the length of @a http_string in bytes
 * @return #MHD_YES if HTTP version is correct and supported,
 *         #MHD_NO if HTTP version is not correct or unsupported.
 */
static enum MHD_Result
parse_http_version (struct MHD_Connection *connection,
                    const char *http_string,
                    size_t len)
{
  const char *const h = http_string; /**< short alias */
  mhd_assert (NULL != http_string);

  /* String must starts with 'HTTP/d.d', case-sensetive match.
   * See https://datatracker.ietf.org/doc/html/rfc7230#section-2.6 */
  if ((len != 8) ||
      (h[0] != 'H') || (h[1] != 'T') || (h[2] != 'T') || (h[3] != 'P') ||
      (h[4] != '/')
      || (h[6] != '.') ||
      ((h[5] < '0') || (h[5] > '9')) ||
      ((h[7] < '0') || (h[7] > '9')))
  {
    connection->http_ver = MHD_HTTP_VER_INVALID;
    transmit_error_response_static (connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    REQUEST_MALFORMED);
    return MHD_NO;
  }
  if (1 == h[5] - '0')
  {
    /* HTTP/1.x */
    if (1 == h[7] - '0')
      connection->http_ver = MHD_HTTP_VER_1_1;
    else if (0 == h[7] - '0')
      connection->http_ver = MHD_HTTP_VER_1_0;
    else
      connection->http_ver = MHD_HTTP_VER_1_2__1_9;

    return MHD_YES;
  }

  if (0 == h[5] - '0')
  {
    /* Too old major version */
    connection->http_ver = MHD_HTTP_VER_TOO_OLD;
    transmit_error_response_static (connection,
                                    MHD_HTTP_HTTP_VERSION_NOT_SUPPORTED,
                                    REQ_HTTP_VER_IS_TOO_OLD);
    return MHD_NO;
  }

  connection->http_ver = MHD_HTTP_VER_FUTURE;
  transmit_error_response_static (connection,
                                  MHD_HTTP_HTTP_VERSION_NOT_SUPPORTED,
                                  REQ_HTTP_VER_IS_NOT_SUPPORTED);
  return MHD_NO;
}


/**
 * Detect standard HTTP request method
 *
 * @param connection the connection
 * @param method the pointer to HTTP request method string
 * @param len the length of @a method in bytes
 * @return #MHD_YES if HTTP method is valid string,
 *         #MHD_NO if HTTP method string is not valid.
 */
static enum MHD_Result
parse_http_std_method (struct MHD_Connection *connection,
                       const char *method,
                       size_t len)
{
  const char *const m = method; /**< short alias */
  mhd_assert (NULL != m);

  if (0 == len)
    return MHD_NO;

  if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_GET) == len) &&
      (0 == memcmp (m, MHD_HTTP_METHOD_GET, len)))
    connection->http_mthd = MHD_HTTP_MTHD_GET;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_HEAD) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_HEAD, len)))
    connection->http_mthd = MHD_HTTP_MTHD_HEAD;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_POST) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_POST, len)))
    connection->http_mthd = MHD_HTTP_MTHD_POST;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_PUT) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_PUT, len)))
    connection->http_mthd = MHD_HTTP_MTHD_PUT;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_DELETE) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_DELETE, len)))
    connection->http_mthd = MHD_HTTP_MTHD_DELETE;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_CONNECT) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_CONNECT, len)))
    connection->http_mthd = MHD_HTTP_MTHD_CONNECT;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_OPTIONS) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_OPTIONS, len)))
    connection->http_mthd = MHD_HTTP_MTHD_OPTIONS;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_METHOD_TRACE) == len) &&
           (0 == memcmp (m, MHD_HTTP_METHOD_TRACE, len)))
    connection->http_mthd = MHD_HTTP_MTHD_TRACE;
  else
    connection->http_mthd = MHD_HTTP_MTHD_OTHER;

  /* Any method string with non-zero length is valid */
  return MHD_YES;
}


/**
 * Parse the first line of the HTTP HEADER.
 *
 * @param connection the connection (updated)
 * @param line the first line, not 0-terminated
 * @param line_len length of the first @a line
 * @return #MHD_YES if the line is ok, #MHD_NO if it is malformed
 */
static enum MHD_Result
parse_initial_message_line (struct MHD_Connection *connection,
                            char *line,
                            size_t line_len)
{
  struct MHD_Daemon *daemon = connection->daemon;
  const char *curi;
  char *uri;
  char *http_version;
  char *args;
  unsigned int unused_num_headers;

  if (NULL == (uri = memchr (line,
                             ' ',
                             line_len)))
    return MHD_NO;              /* serious error */
  uri[0] = '\0';
  connection->method = line;
  if (MHD_NO == parse_http_std_method (connection, connection->method,
                                       (size_t) (uri - line)))
    return MHD_NO;
  uri++;
  /* Skip any spaces. Not required by standard but allow
     to be more tolerant. */
  /* TODO: do not skip them in standard mode */
  while ( (' ' == uri[0]) &&
          ( (size_t) (uri - line) < line_len) )
    uri++;
  if ((size_t) (uri - line) == line_len)
  {
    /* No URI and no http version given */
    curi = "";
    uri = NULL;
    connection->version = "";
    args = NULL;
    if (MHD_NO == parse_http_version (connection, connection->version, 0))
      return MHD_NO;
  }
  else
  {
    size_t uri_len;
    curi = uri;
    /* Search from back to accept malformed URI with space */
    http_version = line + line_len - 1;
    /* Skip any trailing spaces */
    /* TODO: do not skip them in standard mode */
    while ( (' ' == http_version[0]) &&
            (http_version > uri) )
      http_version--;
    /* Find first space in reverse direction */
    while ( (' ' != http_version[0]) &&
            (http_version > uri) )
      http_version--;
    if (http_version > uri)
    {
      /* http_version points to character before HTTP version string */
      http_version[0] = '\0';
      connection->version = http_version + 1;
      if (MHD_NO == parse_http_version (connection, connection->version,
                                        line_len
                                        - (connection->version - line)))
        return MHD_NO;
      uri_len = http_version - uri;
    }
    else
    {
      connection->version = "";
      if (MHD_NO == parse_http_version (connection, connection->version, 0))
        return MHD_NO;
      uri_len = line_len - (uri - line);
    }
    /* check for spaces in URI if we are "strict" */
    if ( (1 <= daemon->strict_for_client) &&
         (NULL != memchr (uri,
                          ' ',
                          uri_len)) )
    {
      /* space exists in URI and we are supposed to be strict, reject */
      return MHD_NO;
    }

    args = memchr (uri,
                   '?',
                   uri_len);
  }

  /* log callback before we modify URI *or* args */
  if (NULL != daemon->uri_log_callback)
  {
    connection->client_aware = true;
    connection->client_context
      = daemon->uri_log_callback (daemon->uri_log_callback_cls,
                                  uri,
                                  connection);
  }

  if (NULL != args)
  {
    args[0] = '\0';
    args++;
    /* note that this call clobbers 'args' */
    MHD_parse_arguments_ (connection,
                          MHD_GET_ARGUMENT_KIND,
                          args,
                          &connection_add_header,
                          &unused_num_headers);
  }

  /* unescape URI *after* searching for arguments and log callback */
  if (NULL != uri)
    daemon->unescape_callback (daemon->unescape_callback_cls,
                               connection,
                               uri);
  connection->url = curi;
  return MHD_YES;
}


/**
 * Call the handler of the application for this
 * connection.  Handles chunking of the upload
 * as well as normal uploads.
 *
 * @param connection connection we're processing
 */
static void
call_connection_handler (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  size_t processed;

  if (NULL != connection->response)
    return;                     /* already queued a response */
  processed = 0;
  connection->client_aware = true;
  if (MHD_NO ==
      daemon->default_handler (daemon->default_handler_cls,
                               connection,
                               connection->url,
                               connection->method,
                               connection->version,
                               NULL,
                               &processed,
                               &connection->client_context))
  {
    /* serious internal error, close connection */
    CONNECTION_CLOSE_ERROR (connection,
                            _ (
                              "Application reported internal error, closing connection."));
    return;
  }
}


/**
 * Call the handler of the application for this
 * connection.  Handles chunking of the upload
 * as well as normal uploads.
 *
 * @param connection connection we're processing
 */
static void
process_request_body (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  size_t available;
  bool instant_retry;
  char *buffer_head;

  if (NULL != connection->response)
  {
    /* TODO: discard all read buffer as early response
     * means that connection have to be closed. */
    /* already queued a response, discard remaining upload
       (but not more, there might be another request after it) */
    size_t purge;

    purge = (size_t) MHD_MIN (connection->remaining_upload_size,
                              (uint64_t) connection->read_buffer_offset);
    connection->remaining_upload_size -= purge;
    if (connection->read_buffer_offset > purge)
      memmove (connection->read_buffer,
               &connection->read_buffer[purge],
               connection->read_buffer_offset - purge);
    connection->read_buffer_offset -= purge;
    return;
  }

  buffer_head = connection->read_buffer;
  available = connection->read_buffer_offset;
  do
  {
    size_t to_be_processed;
    size_t left_unprocessed;
    size_t processed_size;

    instant_retry = false;
    if (connection->have_chunked_upload)
    {
      mhd_assert (MHD_SIZE_UNKNOWN == connection->remaining_upload_size);
      if ( (connection->current_chunk_offset ==
            connection->current_chunk_size) &&
           (0 != connection->current_chunk_size) )
      {
        size_t i;
        mhd_assert (0 != available);
        /* skip new line at the *end* of a chunk */
        i = 0;
        if ( (2 <= available) &&
             ('\r' == buffer_head[0]) &&
             ('\n' == buffer_head[1]) )
          i += 2;                        /* skip CRLF */
        else if ('\n' == buffer_head[0]) /* TODO: Add MHD option to disallow */
          i++;                           /* skip bare LF */
        else if (2 > available)
          break;                         /* need more upload data */
        if (0 == i)
        {
          /* malformed encoding */
          transmit_error_response_static (connection,
                                          MHD_HTTP_BAD_REQUEST,
                                          REQUEST_CHUNKED_MALFORMED);
          return;
        }
        available -= i;
        buffer_head += i;
        connection->current_chunk_offset = 0;
        connection->current_chunk_size = 0;
        if (0 == available)
          break;
      }
      if (0 != connection->current_chunk_size)
      {
        uint64_t cur_chunk_left;
        mhd_assert (connection->current_chunk_offset < \
                    connection->current_chunk_size);
        /* we are in the middle of a chunk, give
           as much as possible to the client (without
           crossing chunk boundaries) */
        cur_chunk_left
          = connection->current_chunk_size - connection->current_chunk_offset;
        if (cur_chunk_left > available)
          to_be_processed = available;
        else
        {         /* cur_chunk_left <= (size_t)available */
          to_be_processed = (size_t) cur_chunk_left;
          if (available > to_be_processed)
            instant_retry = true;
        }
      }
      else
      {
        size_t i;
        /** The length of the string with the number of the chunk size */
        size_t chunk_size_len;
        bool found_chunk_size_str;
        bool malformed;

        /* we need to read chunk boundaries */
        i = 0;
        found_chunk_size_str = false;
        chunk_size_len = 0;
        mhd_assert (0 != available);
        do
        {
          if ('\n' == buffer_head[i])
          {
            if ((0 < i) && ('\r' == buffer_head[i - 1]))
            { /* CRLF */
              if (! found_chunk_size_str)
                chunk_size_len = i - 1;
            }
            else
            { /* bare LF */
              /* TODO: Add an option to disallow bare LF */
              if (! found_chunk_size_str)
                chunk_size_len = i;
            }
            found_chunk_size_str = true;
            break; /* Found the end of the string */
          }
          else if (! found_chunk_size_str && (';' == buffer_head[i]))
          { /* Found chunk extension */
            chunk_size_len = i;
            found_chunk_size_str = true;
          }
        } while (available > ++i);
        mhd_assert ((i == available) || found_chunk_size_str);
        mhd_assert ((0 == chunk_size_len) || found_chunk_size_str);
        malformed = ((0 == chunk_size_len) && found_chunk_size_str);
        if (! malformed)
        {
          /* Check whether size is valid hexadecimal number
           * even if end of the string is not found yet. */
          size_t num_dig;
          uint64_t chunk_size;
          mhd_assert (0 < i);
          if (! found_chunk_size_str)
          {
            mhd_assert (i == available);
            /* Check already available part of the size string for valid
             * hexadecimal digits. */
            chunk_size_len = i;
            if ('\r' == buffer_head[i - 1])
            {
              chunk_size_len--;
              malformed = (0 == chunk_size_len);
            }
          }
          num_dig = MHD_strx_to_uint64_n_ (buffer_head,
                                           chunk_size_len,
                                           &chunk_size);
          malformed = malformed || (chunk_size_len != num_dig);

          if ((available != i) && ! malformed)
          {
            /* Found end of the string and the size of the chunk is valid */

            mhd_assert (found_chunk_size_str);
            /* Start reading payload data of the chunk */
            connection->current_chunk_offset = 0;
            connection->current_chunk_size = chunk_size;
            i++; /* Consume the last checked char */
            available -= i;
            buffer_head += i;

            if (0 == connection->current_chunk_size)
            { /* The final (termination) chunk */
              connection->remaining_upload_size = 0;
              break;
            }
            if (available > 0)
              instant_retry = true;
            continue;
          }

          if ((0 == num_dig) && (0 != chunk_size_len))
          { /* Check whether result is invalid due to uint64_t overflow */
            /* At least one byte is always available
             * in the input buffer here. */
            const char d = buffer_head[0]; /**< first digit */
            if ((('0' <= d) && ('9' >= d)) ||
                (('A' <= d) && ('F' >= d)) ||
                (('a' <= d) && ('f' >= d)))
            { /* The first char is a valid hexadecimal digit */
              transmit_error_response_static (connection,
                                              MHD_HTTP_CONTENT_TOO_LARGE,
                                              REQUEST_CHUNK_TOO_LARGE);
              return;
            }
          }
        }
        if (malformed)
        {
          transmit_error_response_static (connection,
                                          MHD_HTTP_BAD_REQUEST,
                                          REQUEST_CHUNKED_MALFORMED);
          return;
        }
        mhd_assert (available == i);
        break; /* The end of the string not found, need more upload data */
      }
    }
    else
    {
      /* no chunked encoding, give all to the client */
      mhd_assert (MHD_SIZE_UNKNOWN != connection->remaining_upload_size);
      mhd_assert (0 != connection->remaining_upload_size);
      if (connection->remaining_upload_size < available)
        to_be_processed = (size_t) connection->remaining_upload_size;
      else
        to_be_processed = available;
    }
    left_unprocessed = to_be_processed;
    connection->client_aware = true;
    if (MHD_NO ==
        daemon->default_handler (daemon->default_handler_cls,
                                 connection,
                                 connection->url,
                                 connection->method,
                                 connection->version,
                                 buffer_head,
                                 &left_unprocessed,
                                 &connection->client_context))
    {
      /* serious internal error, close connection */
      CONNECTION_CLOSE_ERROR (connection,
                              _ ("Application reported internal error, " \
                                 "closing connection."));
      return;
    }
    if (left_unprocessed > to_be_processed)
      mhd_panic (mhd_panic_cls,
                 __FILE__,
                 __LINE__
#ifdef HAVE_MESSAGES
                 , _ ("libmicrohttpd API violation.\n")
#else
                 , NULL
#endif
                 );
    if (0 != left_unprocessed)
    {
      instant_retry = false; /* client did not process everything */
#ifdef HAVE_MESSAGES
      /* client did not process all upload data, complain if
         the setup was incorrect, which may prevent us from
         handling the rest of the request */
      if ( (0 != (daemon->options & MHD_USE_INTERNAL_POLLING_THREAD)) &&
           (! connection->suspended) )
        MHD_DLOG (daemon,
                  _ ("WARNING: incomplete upload processing and connection " \
                     "not suspended may result in hung connection.\n"));
#endif
    }
    processed_size = to_be_processed - left_unprocessed;
    if (connection->have_chunked_upload)
      connection->current_chunk_offset += processed_size;
    /* dh left "processed" bytes in buffer for next time... */
    buffer_head += processed_size;
    available -= processed_size;
    if (! connection->have_chunked_upload)
    {
      mhd_assert (MHD_SIZE_UNKNOWN != connection->remaining_upload_size);
      connection->remaining_upload_size -= processed_size;
    }
    else
      mhd_assert (MHD_SIZE_UNKNOWN == connection->remaining_upload_size);
  } while (instant_retry);
  /* TODO: zero out reused memory region */
  if ( (available > 0) &&
       (buffer_head != connection->read_buffer) )
    memmove (connection->read_buffer,
             buffer_head,
             available);
  else
    mhd_assert ((0 == available) || \
                (connection->read_buffer_offset == available));
  connection->read_buffer_offset = available;
}


/**
 * Check if we are done sending the write-buffer.
 * If so, transition into "next_state".
 *
 * @param connection connection to check write status for
 * @param next_state the next state to transition to
 * @return #MHD_NO if we are not done, #MHD_YES if we are
 */
static enum MHD_Result
check_write_done (struct MHD_Connection *connection,
                  enum MHD_CONNECTION_STATE next_state)
{
  if ( (connection->write_buffer_append_offset !=
        connection->write_buffer_send_offset)
       /* || data_in_tls_buffers == true  */
       )
    return MHD_NO;
  connection->write_buffer_append_offset = 0;
  connection->write_buffer_send_offset = 0;
  connection->state = next_state;
  return MHD_YES;
}


/**
 * We have received (possibly the beginning of) a line in the
 * header (or footer).  Validate (check for ":") and prepare
 * to process.
 *
 * @param connection connection we're processing
 * @param line line from the header to process
 * @return #MHD_YES on success, #MHD_NO on error (malformed @a line)
 */
static enum MHD_Result
process_header_line (struct MHD_Connection *connection,
                     char *line)
{
  char *colon;

  /* line should be normal header line, find colon */
  colon = strchr (line, ':');
  if (NULL == colon)
  {
    /* error in header line, die hard */
    return MHD_NO;
  }
  if (-1 >= connection->daemon->strict_for_client)
  {
    /* check for whitespace before colon, which is not allowed
 by RFC 7230 section 3.2.4; we count space ' ' and
 tab '\t', but not '\r\n' as those would have ended the line. */
    const char *white;

    white = strchr (line, ' ');
    if ( (NULL != white) &&
         (white < colon) )
      return MHD_NO;
    white = strchr (line, '\t');
    if ( (NULL != white) &&
         (white < colon) )
      return MHD_NO;
  }
  /* zero-terminate header */
  colon[0] = '\0';
  colon++;                      /* advance to value */
  while ( ('\0' != colon[0]) &&
          ( (' ' == colon[0]) ||
            ('\t' == colon[0]) ) )
    colon++;
  /* we do the actual adding of the connection
     header at the beginning of the while
     loop since we need to be able to inspect
     the *next* header line (in case it starts
     with a space...) */
  connection->last = line;
  connection->colon = colon;
  return MHD_YES;
}


/**
 * Process a header value that spans multiple lines.
 * The previous line(s) are in connection->last.
 *
 * @param connection connection we're processing
 * @param line the current input line
 * @param kind if the line is complete, add a header
 *        of the given kind
 * @return #MHD_YES if the line was processed successfully
 */
static enum MHD_Result
process_broken_line (struct MHD_Connection *connection,
                     char *line,
                     enum MHD_ValueKind kind)
{
  char *const last_value = connection->colon;
  const size_t last_value_len = strlen (last_value);
  mhd_assert (NULL != connection->last);
  mhd_assert (NULL != connection->colon);

  if ( (' ' == line[0]) ||
       ('\t' == line[0]) )
  {
    /* This line is a continuation of the previous line */
    /* NOTE: this is a simplified implementation only for v0.9.77 */
    size_t num_to_replace = ((size_t) (line - last_value)) - last_value_len;

    /* Only CRLF or LF should be between lines */
    mhd_assert ((2 == num_to_replace) || (1 == num_to_replace));
    /* Replace CRLF with spaces */
    last_value[last_value_len] = ' ';
    if (0 != --num_to_replace)
      last_value[last_value_len + 1] = ' ';
    return MHD_NO;             /* possibly more than 2 lines... */
  }
  if (MHD_NO ==
      connection_add_header (connection,
                             connection->last,
                             strlen (connection->last),
                             last_value,
                             last_value_len,
                             kind))
  {
    /* Error has been queued by connection_add_header() */
    return MHD_NO;
  }
  /* we still have the current line to deal with... */
  if (0 != line[0])
  {
    if (MHD_NO == process_header_line (connection,
                                       line))
    {
      transmit_error_response_static (connection,
                                      MHD_HTTP_BAD_REQUEST,
                                      REQUEST_MALFORMED);
      return MHD_NO;
    }
  }
  return MHD_YES;
}


/**
 * Parse the various headers; figure out the size
 * of the upload and make sure the headers follow
 * the protocol.  Advance to the appropriate state.
 *
 * @param connection connection we're processing
 */
static void
parse_connection_headers (struct MHD_Connection *connection)
{
  const char *clen;
  const char *enc;
  size_t val_len;

  parse_cookie_header (connection);
  if ( (1 <= connection->daemon->strict_for_client) &&
       (MHD_IS_HTTP_VER_1_1_COMPAT (connection->http_ver)) &&
       (MHD_NO ==
        MHD_lookup_connection_value_n (connection,
                                       MHD_HEADER_KIND,
                                       MHD_HTTP_HEADER_HOST,
                                       MHD_STATICSTR_LEN_ (
                                         MHD_HTTP_HEADER_HOST),
                                       NULL,
                                       NULL)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (connection->daemon,
              _ ("Received HTTP/1.1 request without `Host' header.\n"));
#endif
    transmit_error_response_static (connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    REQUEST_LACKS_HOST);
    return;
  }

  connection->remaining_upload_size = 0;
  if (MHD_NO != MHD_lookup_connection_value_n (connection,
                                               MHD_HEADER_KIND,
                                               MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                               MHD_STATICSTR_LEN_ (
                                                 MHD_HTTP_HEADER_TRANSFER_ENCODING),
                                               &enc,
                                               NULL))
  {
    connection->remaining_upload_size = MHD_SIZE_UNKNOWN;
    if (MHD_str_equal_caseless_ (enc,
                                 "chunked"))
      connection->have_chunked_upload = true;
  }
  else
  {
    if (MHD_NO != MHD_lookup_connection_value_n (connection,
                                                 MHD_HEADER_KIND,
                                                 MHD_HTTP_HEADER_CONTENT_LENGTH,
                                                 MHD_STATICSTR_LEN_ (
                                                   MHD_HTTP_HEADER_CONTENT_LENGTH),
                                                 &clen,
                                                 &val_len))
    {
      size_t num_digits;

      num_digits = MHD_str_to_uint64_n_ (clen,
                                         val_len,
                                         &connection->remaining_upload_size);
      if ( (val_len != num_digits) ||
           (0 == num_digits) )
      {
        connection->remaining_upload_size = 0;
        if ((0 == num_digits) &&
            (0 != val_len) &&
            ('0' <= clen[0]) && ('9' >= clen[0]))
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (connection->daemon,
                    _ ("Too large value of 'Content-Length' header. " \
                       "Closing connection.\n"));
#endif
          transmit_error_response_static (connection,
                                          MHD_HTTP_CONTENT_TOO_LARGE,
                                          REQUEST_CONTENTLENGTH_TOOLARGE);
        }
        else
        {
#ifdef HAVE_MESSAGES
          MHD_DLOG (connection->daemon,
                    _ ("Failed to parse `Content-Length' header. " \
                       "Closing connection.\n"));
#endif
          transmit_error_response_static (connection,
                                          MHD_HTTP_BAD_REQUEST,
                                          REQUEST_CONTENTLENGTH_MALFORMED);
        }
      }
    }
  }
}


/**
 * Update the 'last_activity' field of the connection to the current time
 * and move the connection to the head of the 'normal_timeout' list if
 * the timeout for the connection uses the default value.
 *
 * @param connection the connection that saw some activity
 */
void
MHD_update_last_activity_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;

  if (0 == connection->connection_timeout_ms)
    return;  /* Skip update of activity for connections
               without timeout timer. */
  if (connection->suspended)
    return;  /* no activity on suspended connections */

  connection->last_activity = MHD_monotonic_msec_counter ();
  if (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    return; /* each connection has personal timeout */

  if (connection->connection_timeout_ms != daemon->connection_timeout_ms)
    return; /* custom timeout, no need to move it in "normal" DLL */
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  /* move connection to head of timeout list (by remove + add operation) */
  XDLL_remove (daemon->normal_timeout_head,
               daemon->normal_timeout_tail,
               connection);
  XDLL_insert (daemon->normal_timeout_head,
               daemon->normal_timeout_tail,
               connection);
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
}


/**
 * This function handles a particular connection when it has been
 * determined that there is data to be read off a socket. All
 * implementations (multithreaded, external polling, internal polling)
 * call this function to handle reads.
 *
 * @param connection connection to handle
 * @param socket_error set to true if socket error was detected
 */
void
MHD_connection_handle_read (struct MHD_Connection *connection,
                            bool socket_error)
{
  ssize_t bytes_read;

  if ( (MHD_CONNECTION_CLOSED == connection->state) ||
       (connection->suspended) )
    return;
#ifdef HTTPS_SUPPORT
  if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
  {   /* HTTPS connection. */
    if (MHD_TLS_CONN_CONNECTED > connection->tls_state)
    {
      if (! MHD_run_tls_handshake_ (connection))
        return;
    }
  }
#endif /* HTTPS_SUPPORT */

  /* make sure "read" has a reasonable number of bytes
     in buffer to use per system call (if possible) */
  if (connection->read_buffer_offset + connection->daemon->pool_increment >
      connection->read_buffer_size)
    try_grow_read_buffer (connection,
                          (connection->read_buffer_size ==
                           connection->read_buffer_offset));

  if (connection->read_buffer_size == connection->read_buffer_offset)
    return; /* No space for receiving data. */
  bytes_read = connection->recv_cls (connection,
                                     &connection->read_buffer
                                     [connection->read_buffer_offset],
                                     connection->read_buffer_size
                                     - connection->read_buffer_offset);
  if ((bytes_read < 0) || socket_error)
  {
    if ((MHD_ERR_AGAIN_ == bytes_read) && ! socket_error)
      return;     /* No new data to process. */
    if ((bytes_read > 0) && connection->sk_nonblck)
    { /* Try to detect the socket error */
      int dummy;
      bytes_read = connection->recv_cls (connection, &dummy, sizeof (dummy));
    }
    if (MHD_ERR_CONNRESET_ == bytes_read)
    {
      if ( (MHD_CONNECTION_INIT < connection->state) &&
           (MHD_CONNECTION_FULL_REQ_RECEIVED > connection->state) )
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("Socket has been disconnected when reading request.\n"));
#endif
        connection->discard_request = true;
      }
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_READ_ERROR);
      return;
    }

#ifdef HAVE_MESSAGES
    if (MHD_CONNECTION_INIT != connection->state)
      MHD_DLOG (connection->daemon,
                _ ("Connection socket is closed when reading " \
                   "request due to the error: %s\n"),
                (bytes_read < 0) ? str_conn_error_ (bytes_read) :
                "detected connection closure");
#endif
    CONNECTION_CLOSE_ERROR (connection,
                            NULL);
    return;
  }

  if (0 == bytes_read)
  {   /* Remote side closed connection. */
    connection->read_closed = true;
    if ( (MHD_CONNECTION_INIT < connection->state) &&
         (MHD_CONNECTION_FULL_REQ_RECEIVED > connection->state) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Connection was closed by remote side with incomplete "
                   "request.\n"));
#endif
      connection->discard_request = true;
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_CLIENT_ABORT);
    }
    else if (MHD_CONNECTION_INIT == connection->state)
      /* This termination code cannot be reported to the application
       * because application has not been informed yet about this request */
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_COMPLETED_OK);
    else
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_WITH_ERROR);
    return;
  }
  connection->read_buffer_offset += bytes_read;
  MHD_update_last_activity_ (connection);
#if DEBUG_STATES
  MHD_DLOG (connection->daemon,
            _ ("In function %s handling connection at state: %s\n"),
            MHD_FUNC_,
            MHD_state_to_string (connection->state));
#endif
  switch (connection->state)
  {
  case MHD_CONNECTION_INIT:
  case MHD_CONNECTION_REQ_LINE_RECEIVING:
  case MHD_CONNECTION_URL_RECEIVED:
  case MHD_CONNECTION_HEADER_PART_RECEIVED:
  case MHD_CONNECTION_HEADERS_RECEIVED:
  case MHD_CONNECTION_HEADERS_PROCESSED:
  case MHD_CONNECTION_CONTINUE_SENDING:
  case MHD_CONNECTION_CONTINUE_SENT:
  case MHD_CONNECTION_BODY_RECEIVED:
  case MHD_CONNECTION_FOOTER_PART_RECEIVED:
    /* nothing to do but default action */
    if (connection->read_closed)
    {
      /* TODO: check whether this really needed */
      MHD_connection_close_ (connection,
                             MHD_REQUEST_TERMINATED_READ_ERROR);
    }
    return;
  case MHD_CONNECTION_CLOSED:
    return;
#ifdef UPGRADE_SUPPORT
  case MHD_CONNECTION_UPGRADE:
    mhd_assert (0);
    return;
#endif /* UPGRADE_SUPPORT */
  default:
    /* shrink read buffer to how much is actually used */
    if ((0 != connection->read_buffer_size) &&
        (connection->read_buffer_size != connection->read_buffer_offset))
    {
      mhd_assert (NULL != connection->read_buffer);
      connection->read_buffer =
        MHD_pool_reallocate (connection->pool,
                             connection->read_buffer,
                             connection->read_buffer_size,
                             connection->read_buffer_offset);
      connection->read_buffer_size = connection->read_buffer_offset;
    }
    break;
  }
  return;
}


/**
 * This function was created to handle writes to sockets when it has
 * been determined that the socket can be written to.
 *
 * @param connection connection to handle
 */
void
MHD_connection_handle_write (struct MHD_Connection *connection)
{
  struct MHD_Response *response;
  ssize_t ret;
  if (connection->suspended)
    return;

#ifdef HTTPS_SUPPORT
  if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
  {   /* HTTPS connection. */
    if (MHD_TLS_CONN_CONNECTED > connection->tls_state)
    {
      if (! MHD_run_tls_handshake_ (connection))
        return;
    }
  }
#endif /* HTTPS_SUPPORT */

#if DEBUG_STATES
  MHD_DLOG (connection->daemon,
            _ ("In function %s handling connection at state: %s\n"),
            MHD_FUNC_,
            MHD_state_to_string (connection->state));
#endif
  switch (connection->state)
  {
  case MHD_CONNECTION_INIT:
  case MHD_CONNECTION_REQ_LINE_RECEIVING:
  case MHD_CONNECTION_URL_RECEIVED:
  case MHD_CONNECTION_HEADER_PART_RECEIVED:
  case MHD_CONNECTION_HEADERS_RECEIVED:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_HEADERS_PROCESSED:
    return;
  case MHD_CONNECTION_CONTINUE_SENDING:
    ret = MHD_send_data_ (connection,
                          &HTTP_100_CONTINUE
                          [connection->continue_message_write_offset],
                          MHD_STATICSTR_LEN_ (HTTP_100_CONTINUE)
                          - connection->continue_message_write_offset,
                          true);
    if (ret < 0)
    {
      if (MHD_ERR_AGAIN_ == ret)
        return;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to send data in request for %s.\n"),
                connection->url);
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              NULL);
      return;
    }
#if _MHD_DEBUG_SEND_DATA
    fprintf (stderr,
             _ ("Sent 100 continue response: `%.*s'\n"),
             (int) ret,
             &HTTP_100_CONTINUE[connection->continue_message_write_offset]);
#endif
    connection->continue_message_write_offset += ret;
    MHD_update_last_activity_ (connection);
    return;
  case MHD_CONNECTION_CONTINUE_SENT:
  case MHD_CONNECTION_BODY_RECEIVED:
  case MHD_CONNECTION_FOOTER_PART_RECEIVED:
  case MHD_CONNECTION_FOOTERS_RECEIVED:
  case MHD_CONNECTION_FULL_REQ_RECEIVED:
  case MHD_CONNECTION_START_REPLY:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_HEADERS_SENDING:
    {
      struct MHD_Response *const resp = connection->response;
      const size_t wb_ready = connection->write_buffer_append_offset
                              - connection->write_buffer_send_offset;
      mhd_assert (connection->write_buffer_append_offset >= \
                  connection->write_buffer_send_offset);
      mhd_assert (NULL != resp);
      mhd_assert ( (0 == resp->data_size) || \
                   (0 == resp->data_start) || \
                   (NULL != resp->crc) );
      mhd_assert ( (0 == connection->response_write_position) || \
                   (resp->total_size ==
                    connection->response_write_position) || \
                   (MHD_SIZE_UNKNOWN ==
                    connection->response_write_position) );
      mhd_assert ((MHD_CONN_MUST_UPGRADE != connection->keepalive) || \
                  (! connection->rp_props.send_reply_body));

      if ( (connection->rp_props.send_reply_body) &&
           (NULL == resp->crc) &&
           (NULL == resp->data_iov) &&
           /* TODO: remove the next check as 'send_reply_body' is used */
           (0 == connection->response_write_position) &&
           (! connection->rp_props.chunked) )
      {
        mhd_assert (resp->total_size >= resp->data_size);
        mhd_assert (0 == resp->data_start);
        /* Send response headers alongside the response body, if the body
         * data is available. */
        ret = MHD_send_hdr_and_body_ (connection,
                                      &connection->write_buffer
                                      [connection->write_buffer_send_offset],
                                      wb_ready,
                                      false,
                                      resp->data,
                                      resp->data_size,
                                      (resp->total_size == resp->data_size));
      }
      else
      {
        /* This is response for HEAD request or reply body is not allowed
         * for any other reason or reply body is dynamically generated. */
        /* Do not send the body data even if it's available. */
        ret = MHD_send_hdr_and_body_ (connection,
                                      &connection->write_buffer
                                      [connection->write_buffer_send_offset],
                                      wb_ready,
                                      false,
                                      NULL,
                                      0,
                                      ((0 == resp->total_size) ||
                                       (! connection->rp_props.send_reply_body)
                                      ));
      }

      if (ret < 0)
      {
        if (MHD_ERR_AGAIN_ == ret)
          return;
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("Failed to send the response headers for the " \
                     "request for `%s'. Error: %s\n"),
                  connection->url,
                  str_conn_error_ (ret));
#endif
        CONNECTION_CLOSE_ERROR (connection,
                                NULL);
        return;
      }
      /* 'ret' is not negative, it's safe to cast it to 'size_t'. */
      if (((size_t) ret) > wb_ready)
      {
        /* The complete header and some response data have been sent,
         * update both offsets. */
        mhd_assert (0 == connection->response_write_position);
        mhd_assert (! connection->rp_props.chunked);
        mhd_assert (connection->rp_props.send_reply_body);
        connection->write_buffer_send_offset += wb_ready;
        connection->response_write_position = ret - wb_ready;
      }
      else
        connection->write_buffer_send_offset += ret;
      MHD_update_last_activity_ (connection);
      if (MHD_CONNECTION_HEADERS_SENDING != connection->state)
        return;
      check_write_done (connection,
                        MHD_CONNECTION_HEADERS_SENT);
      return;
    }
  case MHD_CONNECTION_HEADERS_SENT:
    return;
  case MHD_CONNECTION_NORMAL_BODY_READY:
    response = connection->response;
    if (connection->response_write_position <
        connection->response->total_size)
    {
      uint64_t data_write_offset;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != response->crc)
        MHD_mutex_lock_chk_ (&response->mutex);
#endif
      if (MHD_NO == try_ready_normal_body (connection))
      {
        /* mutex was already unlocked by try_ready_normal_body */
        return;
      }
#if defined(_MHD_HAVE_SENDFILE)
      if (MHD_resp_sender_sendfile == connection->resp_sender)
      {
        mhd_assert (NULL == response->data_iov);
        ret = MHD_send_sendfile_ (connection);
      }
      else /* combined with the next 'if' */
#endif /* _MHD_HAVE_SENDFILE */
      if (NULL != response->data_iov)
      {
        ret = MHD_send_iovec_ (connection,
                               &connection->resp_iov,
                               true);
      }
      else
      {
        data_write_offset = connection->response_write_position
                            - response->data_start;
        if (data_write_offset > (uint64_t) SIZE_MAX)
          MHD_PANIC (_ ("Data offset exceeds limit.\n"));
        ret = MHD_send_data_ (connection,
                              &response->data
                              [(size_t) data_write_offset],
                              response->data_size
                              - (size_t) data_write_offset,
                              true);
#if _MHD_DEBUG_SEND_DATA
        if (ret > 0)
          fprintf (stderr,
                   _ ("Sent %d-byte DATA response: `%.*s'\n"),
                   (int) ret,
                   (int) ret,
                   &response->data[connection->response_write_position
                                   - response->data_start]);
#endif
      }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != response->crc)
        MHD_mutex_unlock_chk_ (&response->mutex);
#endif
      if (ret < 0)
      {
        if (MHD_ERR_AGAIN_ == ret)
          return;
#ifdef HAVE_MESSAGES
        MHD_DLOG (connection->daemon,
                  _ ("Failed to send the response body for the " \
                     "request for `%s'. Error: %s\n"),
                  connection->url,
                  str_conn_error_ (ret));
#endif
        CONNECTION_CLOSE_ERROR (connection,
                                NULL);
        return;
      }
      connection->response_write_position += ret;
      MHD_update_last_activity_ (connection);
    }
    if (connection->response_write_position ==
        connection->response->total_size)
      connection->state = MHD_CONNECTION_FOOTERS_SENT;   /* have no footers */
    return;
  case MHD_CONNECTION_NORMAL_BODY_UNREADY:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_CHUNKED_BODY_READY:
    ret = MHD_send_data_ (connection,
                          &connection->write_buffer
                          [connection->write_buffer_send_offset],
                          connection->write_buffer_append_offset
                          - connection->write_buffer_send_offset,
                          true);
    if (ret < 0)
    {
      if (MHD_ERR_AGAIN_ == ret)
        return;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to send the chunked response body for the " \
                   "request for `%s'. Error: %s\n"),
                connection->url,
                str_conn_error_ (ret));
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              NULL);
      return;
    }
    connection->write_buffer_send_offset += ret;
    MHD_update_last_activity_ (connection);
    if (MHD_CONNECTION_CHUNKED_BODY_READY != connection->state)
      return;
    check_write_done (connection,
                      (connection->response->total_size ==
                       connection->response_write_position) ?
                      MHD_CONNECTION_BODY_SENT :
                      MHD_CONNECTION_CHUNKED_BODY_UNREADY);
    return;
  case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
  case MHD_CONNECTION_BODY_SENT:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_FOOTERS_SENDING:
    ret = MHD_send_data_ (connection,
                          &connection->write_buffer
                          [connection->write_buffer_send_offset],
                          connection->write_buffer_append_offset
                          - connection->write_buffer_send_offset,
                          true);
    if (ret < 0)
    {
      if (MHD_ERR_AGAIN_ == ret)
        return;
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("Failed to send the footers for the " \
                   "request for `%s'. Error: %s\n"),
                connection->url,
                str_conn_error_ (ret));
#endif
      CONNECTION_CLOSE_ERROR (connection,
                              NULL);
      return;
    }
    connection->write_buffer_send_offset += ret;
    MHD_update_last_activity_ (connection);
    if (MHD_CONNECTION_FOOTERS_SENDING != connection->state)
      return;
    check_write_done (connection,
                      MHD_CONNECTION_FOOTERS_SENT);
    return;
  case MHD_CONNECTION_FOOTERS_SENT:
    mhd_assert (0);
    return;
  case MHD_CONNECTION_CLOSED:
    return;
#ifdef UPGRADE_SUPPORT
  case MHD_CONNECTION_UPGRADE:
    mhd_assert (0);
    return;
#endif /* UPGRADE_SUPPORT */
  default:
    mhd_assert (0);
    CONNECTION_CLOSE_ERROR (connection,
                            _ ("Internal error.\n"));
    break;
  }
  return;
}


/**
 * Check whether connection has timed out.
 * @param c the connection to check
 * @return true if connection has timeout and needs to be closed,
 *         false otherwise.
 */
static bool
connection_check_timedout (struct MHD_Connection *c)
{
  const uint64_t timeout = c->connection_timeout_ms;
  uint64_t now;
  uint64_t since_actv;

  if (c->suspended)
    return false;
  if (0 == timeout)
    return false;
  now = MHD_monotonic_msec_counter ();
  since_actv = now - c->last_activity;
  /* Keep the next lines in sync with #connection_get_wait() to avoid
   * undesired side-effects like busy-waiting. */
  if (timeout < since_actv)
  {
    if (UINT64_MAX / 2 < since_actv)
    {
      const uint64_t jump_back = c->last_activity - now;
      /* Very unlikely that it is more than quarter-million years pause.
       * More likely that system clock jumps back. */
      if (5000 >= jump_back)
      {
#ifdef HAVE_MESSAGES
        MHD_DLOG (c->daemon,
                  _ ("Detected system clock %u milliseconds jump back.\n"),
                  (unsigned int) jump_back);
#endif
        return false;
      }
#ifdef HAVE_MESSAGES
      MHD_DLOG (c->daemon,
                _ ("Detected too large system clock %" PRIu64 " milliseconds "
                   "jump back.\n"),
                jump_back);
#endif
    }
    return true;
  }
  return false;
}


/**
 * Clean up the state of the given connection and move it into the
 * clean up queue for final disposal.
 * @remark To be called only from thread that process connection's
 * recv(), send() and response.
 *
 * @param connection handle for the connection to clean up
 */
static void
cleanup_connection (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
#ifdef MHD_USE_THREADS
  mhd_assert ( (0 == (daemon->options & MHD_USE_INTERNAL_POLLING_THREAD)) || \
               MHD_thread_ID_match_current_ (connection->pid) );
#endif /* MHD_USE_THREADS */

  if (connection->in_cleanup)
    return; /* Prevent double cleanup. */
  connection->in_cleanup = true;
  if (NULL != connection->response)
  {
    MHD_destroy_response (connection->response);
    connection->response = NULL;
  }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if (connection->suspended)
  {
    DLL_remove (daemon->suspended_connections_head,
                daemon->suspended_connections_tail,
                connection);
    connection->suspended = false;
  }
  else
  {
    if (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
    {
      if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
        XDLL_remove (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     connection);
      else
        XDLL_remove (daemon->manual_timeout_head,
                     daemon->manual_timeout_tail,
                     connection);
    }
    DLL_remove (daemon->connections_head,
                daemon->connections_tail,
                connection);
  }
  DLL_insert (daemon->cleanup_head,
              daemon->cleanup_tail,
              connection);
  connection->resuming = false;
  connection->in_idle = false;
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
  if (0 != (daemon->options & MHD_USE_THREAD_PER_CONNECTION))
  {
    /* if we were at the connection limit before and are in
       thread-per-connection mode, signal the main thread
       to resume accepting connections */
    if ( (MHD_ITC_IS_VALID_ (daemon->itc)) &&
         (! MHD_itc_activate_ (daemon->itc, "c")) )
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ (
                  "Failed to signal end of connection via inter-thread communication channel.\n"));
#endif
    }
  }
}


/**
 * Reset connection after request-reply cycle.
 * @param connection the connection to process
 * @param reuse the flag to choose whether to close connection or
 *              prepare connection for the next request processing
 */
static void
connection_reset (struct MHD_Connection *connection,
                  bool reuse)
{
  struct MHD_Connection *const c = connection; /**< a short alias */
  struct MHD_Daemon *const d = connection->daemon;

  if (! reuse)
  {
    /* Next function will destroy response, notify client,
     * destroy memory pool, and set connection state to "CLOSED" */
    MHD_connection_close_ (connection,
                           c->stop_with_error ?
                           MHD_REQUEST_TERMINATED_WITH_ERROR :
                           MHD_REQUEST_TERMINATED_COMPLETED_OK);
    c->read_buffer = NULL;
    c->read_buffer_size = 0;
    c->read_buffer_offset = 0;
    c->write_buffer = NULL;
    c->write_buffer_size = 0;
    c->write_buffer_send_offset = 0;
    c->write_buffer_append_offset = 0;
  }
  else
  {
    /* Reset connection to process the next request */
    size_t new_read_buf_size;
    mhd_assert (! c->stop_with_error);
    mhd_assert (! c->discard_request);

    if ( (NULL != d->notify_completed) &&
         (c->client_aware) )
      d->notify_completed (d->notify_completed_cls,
                           c,
                           &c->client_context,
                           MHD_REQUEST_TERMINATED_COMPLETED_OK);
    c->client_aware = false;

    if (NULL != c->response)
      MHD_destroy_response (c->response);
    c->response = NULL;
    c->version = NULL;
    c->http_ver = MHD_HTTP_VER_UNKNOWN;
    c->last = NULL;
    c->colon = NULL;
    c->header_size = 0;
    c->keepalive = MHD_CONN_KEEPALIVE_UNKOWN;
    /* Reset the read buffer to the starting size,
       preserving the bytes we have already read. */
    new_read_buf_size = c->daemon->pool_size / 2;
    if (c->read_buffer_offset > new_read_buf_size)
      new_read_buf_size = c->read_buffer_offset;

    connection->read_buffer
      = MHD_pool_reset (c->pool,
                        c->read_buffer,
                        c->read_buffer_offset,
                        new_read_buf_size);
    c->read_buffer_size = new_read_buf_size;
    c->continue_message_write_offset = 0;
    c->headers_received = NULL;
    c->headers_received_tail = NULL;
    c->have_chunked_upload = false;
    c->current_chunk_size = 0;
    c->current_chunk_offset = 0;
    c->responseCode = 0;
    c->response_write_position = 0;
    c->method = NULL;
    c->http_mthd = MHD_HTTP_MTHD_NO_METHOD;
    c->url = NULL;
    memset (&c->rp_props, 0, sizeof(c->rp_props));
    c->write_buffer = NULL;
    c->write_buffer_size = 0;
    c->write_buffer_send_offset = 0;
    c->write_buffer_append_offset = 0;
    /* iov (if any) was deallocated by MHD_pool_reset */
    memset (&connection->resp_iov, 0, sizeof(connection->resp_iov));
    c->state = MHD_CONNECTION_INIT;
  }
  connection->client_context = NULL;
}


/**
 * This function was created to handle per-connection processing that
 * has to happen even if the socket cannot be read or written to.
 * @remark To be called only from thread that process connection's
 * recv(), send() and response.
 *
 * @param connection connection to handle
 * @return #MHD_YES if we should continue to process the
 *         connection (not dead yet), #MHD_NO if it died
 */
enum MHD_Result
MHD_connection_handle_idle (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;
  char *line;
  size_t line_len;
  enum MHD_Result ret;
#ifdef MHD_USE_THREADS
  mhd_assert ( (0 == (daemon->options & MHD_USE_INTERNAL_POLLING_THREAD)) || \
               MHD_thread_ID_match_current_ (connection->pid) );
#endif /* MHD_USE_THREADS */
  /* 'daemon' is not used if epoll is not available and asserts are disabled */
  (void) daemon; /* Mute compiler warning */

  connection->in_idle = true;
  while (! connection->suspended)
  {
#ifdef HTTPS_SUPPORT
    if (MHD_TLS_CONN_NO_TLS != connection->tls_state)
    {     /* HTTPS connection. */
      if ((MHD_TLS_CONN_INIT <= connection->tls_state) &&
          (MHD_TLS_CONN_CONNECTED > connection->tls_state))
        break;
    }
#endif /* HTTPS_SUPPORT */
#if DEBUG_STATES
    MHD_DLOG (daemon,
              _ ("In function %s handling connection at state: %s\n"),
              MHD_FUNC_,
              MHD_state_to_string (connection->state));
#endif
    switch (connection->state)
    {
    case MHD_CONNECTION_INIT:
    case MHD_CONNECTION_REQ_LINE_RECEIVING:
      line = get_next_header_line (connection,
                                   &line_len);
      if (NULL != line)
      {
        /* Check for empty string, as we might want
           to tolerate 'spurious' empty lines */
        if (0 == line[0])
        {
          /* TODO: Add MHD option to not tolerate it */
          connection->state = MHD_CONNECTION_INIT;
          continue; /* Process the next line */
        }
        if (MHD_NO == parse_initial_message_line (connection,
                                                  line,
                                                  line_len))
          CONNECTION_CLOSE_ERROR_CHECK (connection,
                                        NULL);
        else
        {
          mhd_assert (MHD_IS_HTTP_VER_SUPPORTED (connection->http_ver));
          connection->state = MHD_CONNECTION_URL_RECEIVED;
        }
        continue;
      }
      /* NULL means we didn't get a full line yet */
      if (connection->discard_request)
      {
        mhd_assert (MHD_CONNECTION_INIT != connection->state);
        continue;
      }
      if (0 < connection->read_buffer_offset)
        connection->state = MHD_CONNECTION_REQ_LINE_RECEIVING;
      break;
    case MHD_CONNECTION_URL_RECEIVED:
      line = get_next_header_line (connection,
                                   NULL);
      if (NULL == line)
      {
        if (MHD_CONNECTION_URL_RECEIVED != connection->state)
          continue;
        if (connection->read_closed)
        {
          CONNECTION_CLOSE_ERROR (connection,
                                  NULL);
          continue;
        }
        break;
      }
      if (0 == line[0])
      {
        connection->state = MHD_CONNECTION_HEADERS_RECEIVED;
        connection->header_size = (size_t) (connection->read_buffer
                                            - connection->method);
        continue;
      }
      if (MHD_NO == process_header_line (connection,
                                         line))
      {
        transmit_error_response_static (connection,
                                        MHD_HTTP_BAD_REQUEST,
                                        REQUEST_MALFORMED);
        break;
      }
      connection->state = MHD_CONNECTION_HEADER_PART_RECEIVED;
      continue;
    case MHD_CONNECTION_HEADER_PART_RECEIVED:
      line = get_next_header_line (connection,
                                   NULL);
      if (NULL == line)
      {
        if (connection->state != MHD_CONNECTION_HEADER_PART_RECEIVED)
          continue;
        if (connection->read_closed)
        {
          CONNECTION_CLOSE_ERROR (connection,
                                  NULL);
          continue;
        }
        break;
      }
      if (MHD_NO ==
          process_broken_line (connection,
                               line,
                               MHD_HEADER_KIND))
        continue;
      if (0 == line[0])
      {
        connection->state = MHD_CONNECTION_HEADERS_RECEIVED;
        connection->header_size = (size_t) (connection->read_buffer
                                            - connection->method);
        continue;
      }
      continue;
    case MHD_CONNECTION_HEADERS_RECEIVED:
      parse_connection_headers (connection);
      if (MHD_CONNECTION_CLOSED == connection->state)
        continue;
      connection->state = MHD_CONNECTION_HEADERS_PROCESSED;
      if (connection->suspended)
        break;
      continue;
    case MHD_CONNECTION_HEADERS_PROCESSED:
      call_connection_handler (connection);     /* first call */
      if (MHD_CONNECTION_CLOSED == connection->state)
        continue;
      if (connection->suspended)
        continue;
      if ( (NULL == connection->response) &&
           (need_100_continue (connection)) )
      {
        connection->state = MHD_CONNECTION_CONTINUE_SENDING;
        break;
      }
      if ( (NULL != connection->response) &&
           (0 != connection->remaining_upload_size) )
      {
        /* we refused (no upload allowed!) */
        connection->remaining_upload_size = 0;
        /* force close, in case client still tries to upload... */
        connection->discard_request = true;
      }
      connection->state = (0 == connection->remaining_upload_size)
                          ? MHD_CONNECTION_FULL_REQ_RECEIVED
                          : MHD_CONNECTION_CONTINUE_SENT;
      if (connection->suspended)
        break;
      continue;
    case MHD_CONNECTION_CONTINUE_SENDING:
      if (connection->continue_message_write_offset ==
          MHD_STATICSTR_LEN_ (HTTP_100_CONTINUE))
      {
        connection->state = MHD_CONNECTION_CONTINUE_SENT;
        continue;
      }
      break;
    case MHD_CONNECTION_CONTINUE_SENT:
      if (0 != connection->read_buffer_offset)
      {
        process_request_body (connection);           /* loop call */
        if (connection->discard_request)
        {
          mhd_assert (MHD_CONNECTION_CONTINUE_SENT != connection->state);
          continue;
        }
      }
      if ( (0 == connection->remaining_upload_size) ||
           ( (MHD_SIZE_UNKNOWN == connection->remaining_upload_size) &&
             (0 == connection->read_buffer_offset) &&
             (connection->discard_request) ) )
      {
        if ( (connection->have_chunked_upload) &&
             (! connection->discard_request) )
          connection->state = MHD_CONNECTION_BODY_RECEIVED;
        else
          connection->state = MHD_CONNECTION_FULL_REQ_RECEIVED;
        if (connection->suspended)
          break;
        continue;
      }
      break;
    case MHD_CONNECTION_BODY_RECEIVED:
      line = get_next_header_line (connection,
                                   NULL);
      if (NULL == line)
      {
        if (connection->state != MHD_CONNECTION_BODY_RECEIVED)
          continue;
        if (connection->read_closed)
        {
          CONNECTION_CLOSE_ERROR (connection,
                                  NULL);
          continue;
        }
        if (0 < connection->read_buffer_offset)
          connection->state = MHD_CONNECTION_FOOTER_PART_RECEIVED;
        break;
      }
      if (0 == line[0])
      {
        connection->state = MHD_CONNECTION_FOOTERS_RECEIVED;
        if (connection->suspended)
          break;
        continue;
      }
      if (MHD_NO == process_header_line (connection,
                                         line))
      {
        transmit_error_response_static (connection,
                                        MHD_HTTP_BAD_REQUEST,
                                        REQUEST_MALFORMED);
        break;
      }
      connection->state = MHD_CONNECTION_FOOTER_PART_RECEIVED;
      continue;
    case MHD_CONNECTION_FOOTER_PART_RECEIVED:
      line = get_next_header_line (connection,
                                   NULL);
      if (NULL == line)
      {
        if (connection->state != MHD_CONNECTION_FOOTER_PART_RECEIVED)
          continue;
        if (connection->read_closed)
        {
          CONNECTION_CLOSE_ERROR (connection,
                                  NULL);
          continue;
        }
        break;
      }
      if (MHD_NO ==
          process_broken_line (connection,
                               line,
                               MHD_FOOTER_KIND))
        continue;
      if (0 == line[0])
      {
        connection->state = MHD_CONNECTION_FOOTERS_RECEIVED;
        if (connection->suspended)
          break;
        continue;
      }
      continue;
    case MHD_CONNECTION_FOOTERS_RECEIVED:
      /* The header, the body, and the footers of the request has been received,
       * switch to the final processing of the request. */
      connection->state = MHD_CONNECTION_FULL_REQ_RECEIVED;
      continue;
    case MHD_CONNECTION_FULL_REQ_RECEIVED:
      call_connection_handler (connection);     /* "final" call */
      if (connection->state == MHD_CONNECTION_CLOSED)
        continue;
      if (NULL == connection->response)
        break;                  /* try again next time */
      /* Response is ready, start reply */
      connection->state = MHD_CONNECTION_START_REPLY;
      continue;
    case MHD_CONNECTION_START_REPLY:
      mhd_assert (NULL != connection->response);
      connection_switch_from_recv_to_send (connection);
      if (MHD_NO == build_header_response (connection))
      {
        /* oops - close! */
        CONNECTION_CLOSE_ERROR (connection,
                                _ ("Closing connection (failed to create "
                                   "response header).\n"));
        continue;
      }
      connection->state = MHD_CONNECTION_HEADERS_SENDING;
      break;

    case MHD_CONNECTION_HEADERS_SENDING:
      /* no default action */
      break;
    case MHD_CONNECTION_HEADERS_SENT:
      /* Some clients may take some actions right after header receive */
#ifdef UPGRADE_SUPPORT
      if (NULL != connection->response->upgrade_handler)
      {
        connection->state = MHD_CONNECTION_UPGRADE;
        /* This connection is "upgraded".  Pass socket to application. */
        if (MHD_NO ==
            MHD_response_execute_upgrade_ (connection->response,
                                           connection))
        {
          /* upgrade failed, fail hard */
          CONNECTION_CLOSE_ERROR (connection,
                                  NULL);
          continue;
        }
        /* Response is not required anymore for this connection. */
        {
          struct MHD_Response *const resp = connection->response;

          connection->response = NULL;
          MHD_destroy_response (resp);
        }
        continue;
      }
#endif /* UPGRADE_SUPPORT */

      if (connection->rp_props.send_reply_body)
      {
        if (connection->rp_props.chunked)
          connection->state = MHD_CONNECTION_CHUNKED_BODY_UNREADY;
        else
          connection->state = MHD_CONNECTION_NORMAL_BODY_UNREADY;
      }
      else
        connection->state = MHD_CONNECTION_FOOTERS_SENT;
      continue;
    case MHD_CONNECTION_NORMAL_BODY_READY:
      /* nothing to do here */
      break;
    case MHD_CONNECTION_NORMAL_BODY_UNREADY:
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != connection->response->crc)
        MHD_mutex_lock_chk_ (&connection->response->mutex);
#endif
      if (0 == connection->response->total_size)
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        if (NULL != connection->response->crc)
          MHD_mutex_unlock_chk_ (&connection->response->mutex);
#endif
        if (connection->rp_props.chunked)
          connection->state = MHD_CONNECTION_BODY_SENT;
        else
          connection->state = MHD_CONNECTION_FOOTERS_SENT;
        continue;
      }
      if (MHD_NO != try_ready_normal_body (connection))
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        if (NULL != connection->response->crc)
          MHD_mutex_unlock_chk_ (&connection->response->mutex);
#endif
        connection->state = MHD_CONNECTION_NORMAL_BODY_READY;
        /* Buffering for flushable socket was already enabled*/

        break;
      }
      /* mutex was already unlocked by "try_ready_normal_body */
      /* not ready, no socket action */
      break;
    case MHD_CONNECTION_CHUNKED_BODY_READY:
      /* nothing to do here */
      break;
    case MHD_CONNECTION_CHUNKED_BODY_UNREADY:
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
      if (NULL != connection->response->crc)
        MHD_mutex_lock_chk_ (&connection->response->mutex);
#endif
      if ( (0 == connection->response->total_size) ||
           (connection->response_write_position ==
            connection->response->total_size) )
      {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
        if (NULL != connection->response->crc)
          MHD_mutex_unlock_chk_ (&connection->response->mutex);
#endif
        connection->state = MHD_CONNECTION_BODY_SENT;
        continue;
      }
      if (1)
      { /* pseudo-branch for local variables scope */
        bool finished;
        if (MHD_NO != try_ready_chunked_body (connection, &finished))
        {
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
          if (NULL != connection->response->crc)
            MHD_mutex_unlock_chk_ (&connection->response->mutex);
#endif
          connection->state = finished ? MHD_CONNECTION_BODY_SENT :
                              MHD_CONNECTION_CHUNKED_BODY_READY;
          continue;
        }
        /* mutex was already unlocked by try_ready_chunked_body */
      }
      break;
    case MHD_CONNECTION_BODY_SENT:
      mhd_assert (connection->rp_props.chunked);

      if (MHD_NO == build_connection_chunked_response_footer (connection))
      {
        /* oops - close! */
        CONNECTION_CLOSE_ERROR (connection,
                                _ (
                                  "Closing connection (failed to create response footer)."));
        continue;
      }
      /* TODO: remove next 'if' */
      if ( (! connection->rp_props.chunked) ||
           (connection->write_buffer_send_offset ==
            connection->write_buffer_append_offset) )
        connection->state = MHD_CONNECTION_FOOTERS_SENT;
      else
        connection->state = MHD_CONNECTION_FOOTERS_SENDING;
      continue;
    case MHD_CONNECTION_FOOTERS_SENDING:
      /* no default action */
      break;
    case MHD_CONNECTION_FOOTERS_SENT:
      if (MHD_HTTP_PROCESSING == connection->responseCode)
      {
        /* After this type of response, we allow sending another! */
        connection->state = MHD_CONNECTION_HEADERS_PROCESSED;
        MHD_destroy_response (connection->response);
        connection->response = NULL;
        /* FIXME: maybe partially reset memory pool? */
        continue;
      }
      /* Reset connection after complete reply */
      connection_reset (connection,
                        MHD_CONN_USE_KEEPALIVE == connection->keepalive &&
                        ! connection->read_closed &&
                        ! connection->discard_request);
      continue;
    case MHD_CONNECTION_CLOSED:
      cleanup_connection (connection);
      connection->in_idle = false;
      return MHD_NO;
#ifdef UPGRADE_SUPPORT
    case MHD_CONNECTION_UPGRADE:
      connection->in_idle = false;
      return MHD_YES;     /* keep open */
#endif /* UPGRADE_SUPPORT */
    default:
      mhd_assert (0);
      break;
    }
    break;
  }
  if (connection_check_timedout (connection))
  {
    MHD_connection_close_ (connection,
                           MHD_REQUEST_TERMINATED_TIMEOUT_REACHED);
    connection->in_idle = false;
    return MHD_YES;
  }
  MHD_connection_update_event_loop_info (connection);
  ret = MHD_YES;
#ifdef EPOLL_SUPPORT
  if ( (! connection->suspended) &&
       (0 != (daemon->options & MHD_USE_EPOLL)) )
  {
    ret = MHD_connection_epoll_update_ (connection);
  }
#endif /* EPOLL_SUPPORT */
  connection->in_idle = false;
  return ret;
}


#ifdef EPOLL_SUPPORT
/**
 * Perform epoll() processing, possibly moving the connection back into
 * the epoll() set if needed.
 *
 * @param connection connection to process
 * @return #MHD_YES if we should continue to process the
 *         connection (not dead yet), #MHD_NO if it died
 */
enum MHD_Result
MHD_connection_epoll_update_ (struct MHD_Connection *connection)
{
  struct MHD_Daemon *daemon = connection->daemon;

  if ( (0 != (daemon->options & MHD_USE_EPOLL)) &&
       (0 == (connection->epoll_state & MHD_EPOLL_STATE_IN_EPOLL_SET)) &&
       (0 == (connection->epoll_state & MHD_EPOLL_STATE_SUSPENDED)) &&
       ( ( (MHD_EVENT_LOOP_INFO_WRITE == connection->event_loop_info) &&
           (0 == (connection->epoll_state & MHD_EPOLL_STATE_WRITE_READY))) ||
         ( (MHD_EVENT_LOOP_INFO_READ == connection->event_loop_info) &&
           (0 == (connection->epoll_state & MHD_EPOLL_STATE_READ_READY)) ) ) )
  {
    /* add to epoll set */
    struct epoll_event event;

    event.events = EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLET;
    event.data.ptr = connection;
    if (0 != epoll_ctl (daemon->epoll_fd,
                        EPOLL_CTL_ADD,
                        connection->socket_fd,
                        &event))
    {
#ifdef HAVE_MESSAGES
      if (0 != (daemon->options & MHD_USE_ERROR_LOG))
        MHD_DLOG (daemon,
                  _ ("Call to epoll_ctl failed: %s\n"),
                  MHD_socket_last_strerr_ ());
#endif
      connection->state = MHD_CONNECTION_CLOSED;
      cleanup_connection (connection);
      return MHD_NO;
    }
    connection->epoll_state |= MHD_EPOLL_STATE_IN_EPOLL_SET;
  }
  return MHD_YES;
}


#endif


/**
 * Set callbacks for this connection to those for HTTP.
 *
 * @param connection connection to initialize
 */
void
MHD_set_http_callbacks_ (struct MHD_Connection *connection)
{
  connection->recv_cls = &recv_param_adapter;
}


/**
 * Obtain information about the given connection.
 *
 * @param connection what connection to get information about
 * @param info_type what information is desired?
 * @param ... depends on @a info_type
 * @return NULL if this information is not available
 *         (or if the @a info_type is unknown)
 * @ingroup specialized
 */
const union MHD_ConnectionInfo *
MHD_get_connection_info (struct MHD_Connection *connection,
                         enum MHD_ConnectionInfoType info_type,
                         ...)
{
  switch (info_type)
  {
#ifdef HTTPS_SUPPORT
  case MHD_CONNECTION_INFO_CIPHER_ALGO:
    if (NULL == connection->tls_session)
      return NULL;
    connection->cipher = gnutls_cipher_get (connection->tls_session);
    return (const union MHD_ConnectionInfo *) &connection->cipher;
  case MHD_CONNECTION_INFO_PROTOCOL:
    if (NULL == connection->tls_session)
      return NULL;
    connection->protocol = gnutls_protocol_get_version (
      connection->tls_session);
    return (const union MHD_ConnectionInfo *) &connection->protocol;
  case MHD_CONNECTION_INFO_GNUTLS_SESSION:
    if (NULL == connection->tls_session)
      return NULL;
    return (const union MHD_ConnectionInfo *) &connection->tls_session;
#endif /* HTTPS_SUPPORT */
  case MHD_CONNECTION_INFO_CLIENT_ADDRESS:
    return (const union MHD_ConnectionInfo *) &connection->addr;
  case MHD_CONNECTION_INFO_DAEMON:
    return (const union MHD_ConnectionInfo *) &connection->daemon;
  case MHD_CONNECTION_INFO_CONNECTION_FD:
    return (const union MHD_ConnectionInfo *) &connection->socket_fd;
  case MHD_CONNECTION_INFO_SOCKET_CONTEXT:
    return (const union MHD_ConnectionInfo *) &connection->socket_context;
  case MHD_CONNECTION_INFO_CONNECTION_SUSPENDED:
    connection->suspended_dummy = connection->suspended ? MHD_YES : MHD_NO;
    return (const union MHD_ConnectionInfo *) &connection->suspended_dummy;
  case MHD_CONNECTION_INFO_CONNECTION_TIMEOUT:
    connection->connection_timeout_dummy =
      (unsigned int) connection->connection_timeout_ms / 1000;
    return (const union MHD_ConnectionInfo *) &connection->
           connection_timeout_dummy;
  case MHD_CONNECTION_INFO_REQUEST_HEADER_SIZE:
    if ( (MHD_CONNECTION_HEADERS_RECEIVED > connection->state) ||
         (MHD_CONNECTION_CLOSED == connection->state) )
      return NULL;   /* invalid, too early! */
    return (const union MHD_ConnectionInfo *) &connection->header_size;
  case MHD_CONNECTION_INFO_HTTP_STATUS:
    if (NULL == connection->response)
      return NULL;
    return (const union MHD_ConnectionInfo *) &connection->responseCode;
  default:
    return NULL;
  }
}


/**
 * Set a custom option for the given connection, overriding defaults.
 *
 * @param connection connection to modify
 * @param option option to set
 * @param ... arguments to the option, depending on the option type
 * @return #MHD_YES on success, #MHD_NO if setting the option failed
 * @ingroup specialized
 */
enum MHD_Result
MHD_set_connection_option (struct MHD_Connection *connection,
                           enum MHD_CONNECTION_OPTION option,
                           ...)
{
  va_list ap;
  struct MHD_Daemon *daemon;
  unsigned int ui_val;

  daemon = connection->daemon;
  switch (option)
  {
  case MHD_CONNECTION_OPTION_TIMEOUT:
    if (0 == connection->connection_timeout_ms)
      connection->last_activity = MHD_monotonic_msec_counter ();
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_lock_chk_ (&daemon->cleanup_connection_mutex);
#endif
    if ( (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
         (! connection->suspended) )
    {
      if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
        XDLL_remove (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     connection);
      else
        XDLL_remove (daemon->manual_timeout_head,
                     daemon->manual_timeout_tail,
                     connection);
    }
    va_start (ap, option);
    ui_val = va_arg (ap, unsigned int);
    va_end (ap);
#if (SIZEOF_UINT64_T - 2) <= SIZEOF_UNSIGNED_INT
    if ((UINT64_MAX / 4000 - 1) < ui_val)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (connection->daemon,
                _ ("The specified connection timeout (%u) is too " \
                   "large. Maximum allowed value (%" PRIu64 ") will be used " \
                   "instead.\n"),
                ui_val,
                (UINT64_MAX / 4000 - 1));
#endif
      ui_val = UINT64_MAX / 4000 - 1;
    }
    else
#endif /* (SIZEOF_UINT64_T - 2) <= SIZEOF_UNSIGNED_INT */
    connection->connection_timeout_ms = ui_val * 1000;
    if ( (0 == (daemon->options & MHD_USE_THREAD_PER_CONNECTION)) &&
         (! connection->suspended) )
    {
      if (connection->connection_timeout_ms == daemon->connection_timeout_ms)
        XDLL_insert (daemon->normal_timeout_head,
                     daemon->normal_timeout_tail,
                     connection);
      else
        XDLL_insert (daemon->manual_timeout_head,
                     daemon->manual_timeout_tail,
                     connection);
    }
#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
    MHD_mutex_unlock_chk_ (&daemon->cleanup_connection_mutex);
#endif
    return MHD_YES;
  default:
    return MHD_NO;
  }
}


/**
 * Queue a response to be transmitted to the client (as soon as
 * possible but after #MHD_AccessHandlerCallback returns).
 *
 * For any active connection this function must be called
 * only by #MHD_AccessHandlerCallback callback.
 * For suspended connection this function can be called at any moment. Response
 * will be sent as soon as connection is resumed.
 *
 * @param connection the connection identifying the client
 * @param status_code HTTP status code (i.e. #MHD_HTTP_OK)
 * @param response response to transmit
 * @return #MHD_NO on error (i.e. reply already sent),
 *         #MHD_YES on success or if message has been queued
 * @ingroup response
 * @sa #MHD_AccessHandlerCallback
 */
enum MHD_Result
MHD_queue_response (struct MHD_Connection *connection,
                    unsigned int status_code,
                    struct MHD_Response *response)
{
  struct MHD_Daemon *daemon;

  if ((NULL == connection) || (NULL == response))
    return MHD_NO;

  daemon = connection->daemon;

#if defined(MHD_USE_POSIX_THREADS) || defined(MHD_USE_W32_THREADS)
  if ( (! connection->suspended) &&
       (0 != (daemon->options & MHD_USE_INTERNAL_POLLING_THREAD)) &&
       (! MHD_thread_ID_match_current_ (connection->pid)) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Attempted to queue response on wrong thread!\n"));
#endif
    return MHD_NO;
  }
#endif

  if (daemon->shutdown)
    return MHD_YES; /* If daemon was shut down in parallel,
                     * response will be aborted now or on later stage. */

  if ( (NULL != connection->response) ||
       ( (MHD_CONNECTION_HEADERS_PROCESSED != connection->state) &&
         (MHD_CONNECTION_FULL_REQ_RECEIVED != connection->state) ) )
    return MHD_NO;

#ifdef UPGRADE_SUPPORT
  if (NULL != response->upgrade_handler)
  {
    struct MHD_HTTP_Header *conn_header;
    if (0 == (daemon->options & MHD_ALLOW_UPGRADE))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Attempted 'upgrade' connection on daemon without" \
                   " MHD_ALLOW_UPGRADE option!\n"));
#endif
      return MHD_NO;
    }
    if (MHD_HTTP_SWITCHING_PROTOCOLS != status_code)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used invalid status code for" \
                   " 'upgrade' response!\n"));
#endif
      return MHD_NO;
    }
    if (0 == (response->flags_auto & MHD_RAF_HAS_CONNECTION_HDR))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used invalid response" \
                   " without \"Connection\" header!\n"));
#endif
      return MHD_NO;
    }
    conn_header = response->first_header;
    mhd_assert (NULL != conn_header);
    mhd_assert (MHD_str_equal_caseless_ (conn_header->header,
                                         MHD_HTTP_HEADER_CONNECTION));
    if (! MHD_str_has_s_token_caseless_ (conn_header->value,
                                         "upgrade"))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Application used invalid response" \
                   " without \"upgrade\" token in" \
                   " \"Connection\" header!\n"));
#endif
      return MHD_NO;
    }
    if (! MHD_IS_HTTP_VER_1_1_COMPAT (connection->http_ver))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Connection \"Upgrade\" can be used " \
                   "with HTTP/1.1 connections!\n"));
#endif
      return MHD_NO;
    }
  }
#endif /* UPGRADE_SUPPORT */
  if ( (100 > (status_code & (~MHD_ICY_FLAG))) ||
       (999 < (status_code & (~MHD_ICY_FLAG))) )
  {
#ifdef HAVE_MESSAGES
    MHD_DLOG (daemon,
              _ ("Refused wrong status code (%u). " \
                 "HTTP requires three digits status code!\n"),
              (status_code & (~MHD_ICY_FLAG)));
#endif
    return MHD_NO;
  }
  if (200 > (status_code & (~MHD_ICY_FLAG)))
  {
    if (MHD_HTTP_VER_1_0 == connection->http_ver)
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Wrong status code (%u) refused. " \
                   "HTTP/1.0 clients do not support 1xx status codes!\n"),
                (status_code & (~MHD_ICY_FLAG)));
#endif
      return MHD_NO;
    }
    if (0 != (response->flags & (MHD_RF_HTTP_1_0_COMPATIBLE_STRICT
                                 | MHD_RF_HTTP_1_0_SERVER)))
    {
#ifdef HAVE_MESSAGES
      MHD_DLOG (daemon,
                _ ("Wrong status code (%u) refused. " \
                   "HTTP/1.0 reply mode does not support 1xx status codes!\n"),
                (status_code & (~MHD_ICY_FLAG)));
#endif
      return MHD_NO;
    }
  }

  MHD_increment_response_rc (response);
  connection->response = response;
  connection->responseCode = status_code;
#if defined(_MHD_HAVE_SENDFILE)
  if ( (response->fd == -1) ||
       (response->is_pipe) ||
       (0 != (connection->daemon->options & MHD_USE_TLS))
#if defined(MHD_SEND_SPIPE_SUPPRESS_NEEDED) && \
       defined(MHD_SEND_SPIPE_SUPPRESS_POSSIBLE)
       || (! daemon->sigpipe_blocked && ! connection->sk_spipe_suppress)
#endif /* MHD_SEND_SPIPE_SUPPRESS_NEEDED &&
          MHD_SEND_SPIPE_SUPPRESS_POSSIBLE */
       )
    connection->resp_sender = MHD_resp_sender_std;
  else
    connection->resp_sender = MHD_resp_sender_sendfile;
#endif /* _MHD_HAVE_SENDFILE */
  /* FIXME: if 'is_pipe' is set, TLS is off, and we have *splice*, we could use splice()
     to avoid two user-space copies... */

  if ( (MHD_HTTP_MTHD_HEAD == connection->http_mthd) ||
       (MHD_HTTP_OK > status_code) ||
       (MHD_HTTP_NO_CONTENT == status_code) ||
       (MHD_HTTP_NOT_MODIFIED == status_code) )
  {
    /* if this is a "HEAD" request, or a status code for
       which a body is not allowed, pretend that we
       have already sent the full message body. */
    /* TODO: remove the next assignment, use 'rp_props.send_reply_body' in
     * checks */
    connection->response_write_position = response->total_size;
  }
  if (MHD_CONNECTION_HEADERS_PROCESSED == connection->state)
  {
    /* response was queued "early", refuse to read body / footers or
       further requests! */
    connection->discard_request = true;
    connection->state = MHD_CONNECTION_START_REPLY;
    connection->remaining_upload_size = 0;
  }
  if (! connection->in_idle)
    (void) MHD_connection_handle_idle (connection);
  MHD_update_last_activity_ (connection);
  return MHD_YES;
}


/* end of connection.c */
