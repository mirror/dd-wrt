/*
     This file is part of GNU libmicrohttpd
     Copyright (C) 2010 Christian Grothoff
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

     GNU libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
     option) any later version.

     GNU libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/

/**
 * @file testcurl/test_put_header_fold.c
 * @brief  Testcase for requests with header fold
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "internal.h"
#include "test_helpers.h"

/* The next macros are borrowed from memorypool.c
   Keep them in sync! */

/**
 * Align to 2x word size (as GNU libc does).
 */
#define ALIGN_SIZE (2 * sizeof(void*))
/**
 * Round up 'n' to a multiple of ALIGN_SIZE.
 */
#define ROUND_TO_ALIGN(n) (((n) + (ALIGN_SIZE - 1)) \
                           / (ALIGN_SIZE) *(ALIGN_SIZE))
#ifndef MHD_ASAN_POISON_ACTIVE
#define _MHD_RED_ZONE_SIZE (0)
#else  /* MHD_ASAN_POISON_ACTIVE */
#define _MHD_RED_ZONE_SIZE (ALIGN_SIZE)
#endif /* MHD_ASAN_POISON_ACTIVE */

#define ROUND_TO_ALIGN_PLUS_RED_ZONE(n) (ROUND_TO_ALIGN(n) + _MHD_RED_ZONE_SIZE)

/* The previous macros are borrowed from memorypool.c
   Keep them in sync! */

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */

#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x) << 16 | (y) << 8 | (z))
#endif /* ! CURL_VERSION_BITS */
#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS (x, y, z))
#endif /* ! CURL_AT_LEAST_VERSION */

#ifndef _MHD_INSTRMACRO
/* Quoted macro parameter */
#define _MHD_INSTRMACRO(a) #a
#endif /* ! _MHD_INSTRMACRO */
#ifndef _MHD_STRMACRO
/* Quoted expanded macro parameter */
#define _MHD_STRMACRO(a) _MHD_INSTRMACRO (a)
#endif /* ! _MHD_STRMACRO */

#if defined(HAVE___FUNC__)
#define externalErrorExit(ignore) \
  _externalErrorExit_func (NULL, __func__, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, __func__, __LINE__)
#define libcurlErrorExit(ignore) \
  _libcurlErrorExit_func (NULL, __func__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func (errDesc, __func__, __LINE__)
#define mhdErrorExit(ignore) \
  _mhdErrorExit_func (NULL, __func__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
  _mhdErrorExit_func (errDesc, __func__, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func ((libcurlcall), _MHD_STRMACRO (libcurlcall), \
                       __func__, __LINE__)
#elif defined(HAVE___FUNCTION__)
#define externalErrorExit(ignore) \
  _externalErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define libcurlErrorExit(ignore) \
  _libcurlErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define mhdErrorExit(ignore) \
  _mhdErrorExit_func (NULL, __FUNCTION__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
  _mhdErrorExit_func (errDesc, __FUNCTION__, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func ((libcurlcall), _MHD_STRMACRO (libcurlcall), \
                       __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func (NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func (errDesc, NULL, __LINE__)
#define libcurlErrorExit(ignore) _libcurlErrorExit_func (NULL, NULL, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func (errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func (NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func (errDesc, NULL, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func ((libcurlcall), _MHD_STRMACRO (libcurlcall), NULL, \
                       __LINE__)
#endif


_MHD_NORETURN static void
_externalErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "System or external library call failed");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  fflush (stderr);
  exit (99);
}


static char libcurl_errbuf[CURL_ERROR_SIZE] = "";

_MHD_NORETURN static void
_libcurlErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "CURL library call failed");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */
  if (0 != libcurl_errbuf[0])
    fprintf (stderr, "Last libcurl error description: %s\n", libcurl_errbuf);

  fflush (stderr);
  exit (99);
}


_MHD_NORETURN static void
_mhdErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
  fflush (stdout);
  if ((NULL != errDesc) && (0 != errDesc[0]))
    fprintf (stderr, "%s", errDesc);
  else
    fprintf (stderr, "MHD unexpected error");
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
#ifdef MHD_WINSOCK_SOCKETS
  fprintf (stderr, "WSAGetLastError() value: %d\n", (int) WSAGetLastError ());
#endif /* MHD_WINSOCK_SOCKETS */

  fflush (stderr);
  exit (8);
}


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 5

#define TEST_UPLOAD_DATA_SIZE 2048U

#define EXPECTED_URI_BASE_PATH  "/"

#define URL_SCHEME "http:/" "/"

#define URL_HOST "127.0.0.1"

#define URL_SCHEME_HOST_PATH URL_SCHEME URL_HOST EXPECTED_URI_BASE_PATH

#define RP_HEADER1_NAME "First"
#define RP_HEADER1_VALUE "1st"
#define RP_HEADER1 RP_HEADER1_NAME ": " RP_HEADER1_VALUE
#define RP_HEADER1_CRLF RP_HEADER1 "\r\n"
#define RP_HEADER2_NAME "Normal"
#define RP_HEADER2_VALUE "it's fine"
#define RP_HEADER2 RP_HEADER2_NAME ": " RP_HEADER2_VALUE
#define RP_HEADER2_CRLF RP_HEADER2 "\r\n"

#define HDR_FOLD "\r\n "
#define RQ_HEADER1_NAME RP_HEADER1_NAME
#define RQ_HEADER1_VALUE RP_HEADER1_VALUE
#define RQ_HEADER1 RQ_HEADER1_NAME ": " RQ_HEADER1_VALUE
#define RQ_HEADER2_NAME "Folded"
#define RQ_HEADER2_VALUE_S "start"
#define RQ_HEADER2_VALUE_E "end"
#define RQ_HEADER2_VALUE \
  RQ_HEADER2_VALUE_S HDR_FOLD RQ_HEADER2_VALUE_E
#define RQ_HEADER2_VALUE_DF \
  RQ_HEADER2_VALUE_S HDR_FOLD HDR_FOLD RQ_HEADER2_VALUE_E
#define RQ_HEADER2 RQ_HEADER2_NAME ": " RQ_HEADER2_VALUE
#define RQ_HEADER2_DF RQ_HEADER2_NAME ": " RQ_HEADER2_VALUE_DF
#define RQ_HEADER3_NAME RP_HEADER2_NAME
#define RQ_HEADER3_VALUE RP_HEADER2_VALUE
#define RQ_HEADER3 RQ_HEADER3_NAME ": " RQ_HEADER3_VALUE

/**
 * The number of request headers: 3 custom headers + 2 automatic headers
 */
#define RQ_NUM_HEADERS (3 + 2)
/**
 * The extra size in the memory pool for pointers to the headers
 */
#define HEADERS_POINTERS_SIZE \
  RQ_NUM_HEADERS * \
  ROUND_TO_ALIGN_PLUS_RED_ZONE(sizeof(struct MHD_HTTP_Header))

#define PAGE \
  "<html><head><title>libmicrohttpd demo page</title></head>" \
  "<body>Success!</body></html>"

/* Global parameters */
static int verbose;
static int oneone;                  /**< If false use HTTP/1.0 for requests*/
static int use_get;
static int use_put;
static int use_put_large;
static int use_double_fold;
static int use_hdr_last; /**< If non-zero, folded header is placed last */
static int use_hdr_large; /**< If non-zero, folded header is large */

/* Static data */
static struct curl_slist *libcurl_headers = NULL;

static char *put_data = NULL;

/**
 * Initialise headers for libcurl
 *
 * @return non-zero if succeed,
 *         zero if failed
 */
static void
libcurl_headers_init (void)
{
  libcurl_headers = curl_slist_append (NULL, RQ_HEADER1);
  if (NULL == libcurl_headers)
    libcurlErrorExitDesc ("curl_slist_append() failed");

  if (use_hdr_last)
  {
    libcurl_headers = curl_slist_append (libcurl_headers, RQ_HEADER3);
    if (NULL == libcurl_headers)
      libcurlErrorExitDesc ("curl_slist_append() failed");
  }

  if (! use_hdr_large)
  {
    if (! use_double_fold)
      libcurl_headers = curl_slist_append (libcurl_headers, RQ_HEADER2);
    else
      libcurl_headers = curl_slist_append (libcurl_headers, RQ_HEADER2_DF);
    if (NULL == libcurl_headers)
      libcurlErrorExitDesc ("curl_slist_append() failed");
  }
  else
  {
    char *buf;
    size_t pos;
    buf = malloc (TEST_UPLOAD_DATA_SIZE + 1);
    if (NULL == buf)
      externalErrorExitDesc ("malloc() failed");
    pos = 0;
    memcpy (buf, RQ_HEADER2_NAME, MHD_STATICSTR_LEN_ (RQ_HEADER2_NAME));
    pos += MHD_STATICSTR_LEN_ (RQ_HEADER2_NAME);
    buf[pos++] = ':';
    buf[pos++] = ' ';
    memcpy (buf + pos,
            RQ_HEADER2_VALUE_S, MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_S));
    pos += MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_S);
    memcpy (buf + pos, HDR_FOLD, MHD_STATICSTR_LEN_ (HDR_FOLD));
    pos += MHD_STATICSTR_LEN_ (HDR_FOLD);
    if (use_double_fold)
    {
      memcpy (buf + pos, HDR_FOLD, MHD_STATICSTR_LEN_ (HDR_FOLD));
      pos += MHD_STATICSTR_LEN_ (HDR_FOLD);
    }
    memset (buf + pos, 'a',
            TEST_UPLOAD_DATA_SIZE - pos
            - MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E) - 1);
    pos += TEST_UPLOAD_DATA_SIZE - pos
           - MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E) - 1;
    buf[pos++] = ' ';
    memcpy (buf + pos,
            RQ_HEADER2_VALUE_E, MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E));
    pos += MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E);
    if (TEST_UPLOAD_DATA_SIZE != pos)
      externalErrorExitDesc ("Position miscalculation");
    buf[pos] = 0;

    libcurl_headers = curl_slist_append (libcurl_headers, buf);
    if (NULL == libcurl_headers)
      libcurlErrorExitDesc ("curl_slist_append() failed");

    free (buf);
  }

  if (! use_hdr_last)
  {
    libcurl_headers = curl_slist_append (libcurl_headers, RQ_HEADER3);
    if (NULL == libcurl_headers)
      libcurlErrorExitDesc ("curl_slist_append() failed");
  }
}


static void
init_put_data (void)
{
  size_t i;
  put_data = malloc (TEST_UPLOAD_DATA_SIZE + 1);
  if (NULL == put_data)
    externalErrorExit ();

  for (i = 0; i < (TEST_UPLOAD_DATA_SIZE - 1); ++i)
  {
    if (0 == (i % 7))
      put_data[i] = ' ';
    else if (0 == (i % 47))
      put_data[i] = '\n';
    else if (0 == (i % 11))
      put_data[i] = (char) ('A' + i % ('Z' - 'A' + 1));
    else
      put_data[i] = (char) ('a' + i % ('z' - 'a' + 1));
  }
  put_data[TEST_UPLOAD_DATA_SIZE - 1] = '\n';
  put_data[TEST_UPLOAD_DATA_SIZE] = 0;
}


static void
test_global_init (void)
{
  libcurl_errbuf[0] = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    externalErrorExit ();

  init_put_data ();
  libcurl_headers_init ();
}


static void
test_global_cleanup (void)
{
  curl_slist_free_all (libcurl_headers);
  curl_global_cleanup ();
  if (NULL != put_data)
    free (put_data);
  put_data = NULL;
}


struct headers_check_result
{
  unsigned int expected_size;
  int header1_found;
  int header2_found;
  unsigned int size_found;
  unsigned int size_broken_found;
};

static size_t
lcurl_hdr_callback (char *buffer, size_t size, size_t nitems,
                    void *userdata)
{
  const size_t data_size = size * nitems;
  struct headers_check_result *check_res =
    (struct headers_check_result *) userdata;

  if ((MHD_STATICSTR_LEN_ (RP_HEADER1_CRLF) == data_size) &&
      (0 == memcmp (RP_HEADER1_CRLF, buffer, data_size)))
    check_res->header1_found++;
  else if ((MHD_STATICSTR_LEN_ (RP_HEADER2_CRLF) == data_size) &&
           (0 == memcmp (RP_HEADER2_CRLF, buffer, data_size)))
    check_res->header2_found++;
  else if ((MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH ": ")
            < data_size) &&
           (0 ==
            memcmp (MHD_HTTP_HEADER_CONTENT_LENGTH ": ", buffer,
                    MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH ": "))))
  {
    char cmpbuf[256];
    int res;
    const unsigned int numbers_pos =
      MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_LENGTH ": ");
    res = snprintf (cmpbuf, sizeof(cmpbuf), "%u", check_res->expected_size);
    if ((res <= 0) || (res > ((int) (sizeof(cmpbuf) - 1))))
      externalErrorExit ();
    if (data_size - numbers_pos <= 2)
    {
      fprintf (stderr, "Broken Content-Length.\n");
      check_res->size_broken_found++;
    }
    else if ((((size_t) res + 2) != data_size - numbers_pos) ||
             (0 != memcmp (buffer + numbers_pos, cmpbuf, (size_t) res)))
    {
      fprintf (stderr, "Wrong Content-Length. "
               "Expected: %u. "
               "Received: %.*s.\n",
               check_res->expected_size,
               (int) (data_size - numbers_pos - 2),
               buffer + numbers_pos);
      check_res->size_broken_found++;
    }
    else if (0 != memcmp ("\r\n", buffer + data_size - 2, 2))
    {
      fprintf (stderr, "The Content-Length header is not "
               "terminated by CRLF.\n");
      check_res->size_broken_found++;
    }
    else
      check_res->size_found++;
  }

  return data_size;
}


struct CBC
{
  /* Upload members */
  size_t up_pos;
  size_t up_size;
  /* Download members */
  char *dn_buf;
  size_t dn_pos;
  size_t dn_buf_size;
};


static size_t
copyBuffer (void *ptr,
            size_t size,
            size_t nmemb,
            void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->dn_pos + size * nmemb > cbc->dn_buf_size)
    return 0;                   /* overflow */
  memcpy (&cbc->dn_buf[cbc->dn_pos], ptr, size * nmemb);
  cbc->dn_pos += size * nmemb;
  return size * nmemb;
}


static size_t
libcurlUploadDataCB (void *stream, size_t item_size, size_t nitems, void *ctx)
{
  size_t to_fill;
  struct CBC *cbc = ctx;

  to_fill = cbc->up_size - cbc->up_pos;
  if (to_fill > item_size * nitems)
    to_fill = item_size * nitems;

  /* Avoid libcurl magic numbers */
#ifdef CURL_READFUNC_PAUSE
  if (CURL_READFUNC_ABORT == to_fill)
    to_fill -= 2;
#endif /* CURL_READFUNC_PAUSE */
#ifdef CURL_READFUNC_ABORT
  if (CURL_READFUNC_ABORT == to_fill)
    --to_fill;
#endif /* CURL_READFUNC_ABORT */

  memcpy (stream, put_data + cbc->up_pos, to_fill);
  cbc->up_pos += to_fill;
  return to_fill;
}


static int
libcurl_debug_cb (CURL *handle,
                  curl_infotype type,
                  char *data,
                  size_t size,
                  void *userptr)
{
  static const char excess_mark[] = "Excess found";
  static const size_t excess_mark_len = MHD_STATICSTR_LEN_ (excess_mark);

  (void) handle;
  (void) userptr;

#ifdef _DEBUG
  switch (type)
  {
  case CURLINFO_TEXT:
    fprintf (stderr, "* %.*s", (int) size, data);
    break;
  case CURLINFO_HEADER_IN:
    fprintf (stderr, "< %.*s", (int) size, data);
    break;
  case CURLINFO_HEADER_OUT:
    fprintf (stderr, "> %.*s", (int) size, data);
    break;
  case CURLINFO_DATA_IN:
#if 0
    fprintf (stderr, "<| %.*s\n", (int) size, data);
#endif
    break;
  case CURLINFO_DATA_OUT:
  case CURLINFO_SSL_DATA_IN:
  case CURLINFO_SSL_DATA_OUT:
  case CURLINFO_END:
  default:
    break;
  }
#endif /* _DEBUG */
  if (CURLINFO_TEXT == type)
  {
    if ((size >= excess_mark_len) &&
        (0 == memcmp (data, excess_mark, excess_mark_len)))
      mhdErrorExitDesc ("Extra data has been detected in MHD reply");
  }
  return 0;
}


static CURL *
setupCURL (void *cbc, uint16_t port,
           struct headers_check_result *hdr_chk_result)
{
  CURL *c;

  c = curl_easy_init ();
  if (NULL == c)
    libcurlErrorExitDesc ("curl_easy_init() failed");

  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                     (oneone) ?
                                     CURL_HTTP_VERSION_1_1 :
                                     CURL_HTTP_VERSION_1_0)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HEADERFUNCTION,
                                     lcurl_hdr_callback)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HEADERDATA,
                                     hdr_chk_result)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER,
                                     libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 0L)) ||
#ifdef _DEBUG
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_VERBOSE, 1L)) ||
#endif /* _DEBUG */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEBUGFUNCTION,
                                     &libcurl_debug_cb)) ||
#if CURL_AT_LEAST_VERSION (7, 85, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS_STR, "http")) ||
#elif CURL_AT_LEAST_VERSION (7, 19, 4)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS, CURLPROTO_HTTP)) ||
#endif /* CURL_AT_LEAST_VERSION (7, 19, 4) */
#if CURL_AT_LEAST_VERSION (7, 45, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEFAULT_PROTOCOL, "http")) ||
#endif /* CURL_AT_LEAST_VERSION (7, 45, 0) */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                     URL_SCHEME_HOST_PATH)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, ((long) port))))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, libcurl_headers))
    libcurlErrorExitDesc ("Failed to set request headers");

  if (use_put)
  {
    if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_UPLOAD, 1L)) ||
        (CURLE_OK != curl_easy_setopt (c, CURLOPT_READFUNCTION,
                                       libcurlUploadDataCB)) ||
        (CURLE_OK != curl_easy_setopt (c, CURLOPT_READDATA,
                                       cbc)))
      libcurlErrorExitDesc ("Failed to configure the PUT upload");
  }

  return c;
}


struct ahc_cls_type
{
  const char *rq_method;
  const char *rq_url;

  unsigned int num_req;
  /* Position in the upload data, not compatible with parallel requests */
  size_t up_pos;
  size_t expected_upload_size;
  unsigned int req_check_error;
};


static enum MHD_Result
ahcCheck (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **req_cls)
{
  static int marker;
  struct MHD_Response *response;
  enum MHD_Result ret;
  struct ahc_cls_type *const param = (struct ahc_cls_type *) cls;

  if (NULL == param)
    mhdErrorExitDesc ("cls parameter is NULL");

  if (oneone)
  {
    if (0 != strcmp (version, MHD_HTTP_VERSION_1_1))
      mhdErrorExitDesc ("Unexpected HTTP version");
  }
  else
  {
    if (0 != strcmp (version, MHD_HTTP_VERSION_1_0))
      mhdErrorExitDesc ("Unexpected HTTP version");
  }

  if (0 != strcmp (url, param->rq_url))
    mhdErrorExitDesc ("Unexpected URI");

  if (0 != strcmp (param->rq_method, method))
    mhdErrorExitDesc ("Unexpected request method");

  if (NULL == upload_data_size)
    mhdErrorExitDesc ("'upload_data_size' pointer is NULL");

  if (NULL != upload_data)
  {
    size_t report_processed_size;
    if (0 == *upload_data_size)
      mhdErrorExitDesc ("'*upload_data_size' value is zero");
    report_processed_size = *upload_data_size;
    /* The next checks are not compatible with parallel requests */
    if (*upload_data_size > param->expected_upload_size - param->up_pos)
    {
      fprintf (stderr, "Unexpected *upload_data_size value: %lu. "
               "Already processed data size: %lu. "
               "Total expected upload size: %lu. "
               "Expected unprocessed upload size: %lu. "
               "The upload data cannot be checked.\n",
               (unsigned long) *upload_data_size,
               (unsigned long) param->up_pos,
               (unsigned long) param->expected_upload_size,
               (unsigned long) (param->expected_upload_size - param->up_pos));
      param->req_check_error++;
    }
    else
    {
      if (0 != memcmp (upload_data, put_data + param->up_pos,
                       *upload_data_size))
      {
        fprintf (stderr, "Wrong upload data.\n"
                 "Expected: '%.*s'\n"
                 "Received: '%.*s'.\n",
                 (int) *upload_data_size, upload_data,
                 (int) *upload_data_size, put_data + param->up_pos);
        param->req_check_error++;
      }
      if (use_put_large &&
          (report_processed_size > param->expected_upload_size / 10))
        report_processed_size = param->expected_upload_size / 10;

      param->up_pos += report_processed_size;
    }
    *upload_data_size -= report_processed_size;
    return MHD_YES;
  }
  else
  {
    if (0 != *upload_data_size)
      mhdErrorExitDesc ("'*upload_data_size' value is not zero");
  }

  if (1)
  {
    /* Check headers */
    const char *value;
    size_t value_len;
    unsigned int header_check_error;

    header_check_error = 0;

    value = MHD_lookup_connection_value (connection, MHD_HEADER_KIND,
                                         RQ_HEADER1_NAME);
    if (NULL == value)
    {
      fprintf (stderr, "Request header '" RQ_HEADER1_NAME "' not found.\n");
      header_check_error++;
    }
    else
    {
      if (0 != strcmp (value, RQ_HEADER1_VALUE))
      {
        fprintf (stderr, "Wrong header '" RQ_HEADER1_NAME "'value. "
                 "Expected: '%s'. Received: '%s'.\n",
                 RQ_HEADER1_VALUE, value);
        header_check_error++;
      }
    }

    if (MHD_YES !=
        MHD_lookup_connection_value_n (connection, MHD_HEADER_KIND,
                                       RQ_HEADER2_NAME,
                                       MHD_STATICSTR_LEN_ (RQ_HEADER2_NAME),
                                       &value, &value_len))
    {
      fprintf (stderr, "Request header '" RQ_HEADER2_NAME "' not found.\n");
      header_check_error++;
    }
    else
    {
      if (NULL == value)
        mhdErrorExitDesc ("The 'value' pointer is NULL");
      if (strlen (value) != value_len)
        mhdErrorExitDesc ("The 'value' length does not match strlen(value)");

      if (value_len < MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_S)
          + MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E))
      {
        fprintf (stderr, "The value_len is too short. The value: '%s'.\n",
                 value);
        header_check_error++;
      }
      if (0 != memcmp (value, RQ_HEADER2_VALUE_S,
                       MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_S)))
      {
        fprintf (stderr, "The 'value' does not start with '"
                 RQ_HEADER2_VALUE_S "'. The 'value' is '%s'. ",
                 value);
        header_check_error++;
      }
      if (0 != memcmp (value
                       + value_len - MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E),
                       RQ_HEADER2_VALUE_E,
                       MHD_STATICSTR_LEN_ (RQ_HEADER2_VALUE_E)))
      {
        fprintf (stderr, "The 'value' does not end with '"
                 RQ_HEADER2_VALUE_E "'. The 'value' is '%s'. ",
                 value);
        header_check_error++;
      }
    }

    value = MHD_lookup_connection_value (connection, MHD_HEADER_KIND,
                                         RQ_HEADER3_NAME);
    if (NULL == value)
    {
      fprintf (stderr, "Request header '" RQ_HEADER3_NAME "' not found.\n");
      header_check_error++;
    }
    else
    {
      if (0 != strcmp (value, RQ_HEADER3_VALUE))
      {
        fprintf (stderr, "Wrong header '" RQ_HEADER3_NAME "'value. "
                 "Expected: '%s'. Received: '%s'.\n",
                 RQ_HEADER3_VALUE, value);
        header_check_error++;
      }
    }
    param->req_check_error += header_check_error;
  }

  if (&marker != *req_cls)
  {
    *req_cls = &marker;
    if (param->num_req)
      mhdErrorExitDesc ("Got unexpected second request");
    param->num_req++;
    return MHD_YES;
  }
  *req_cls = NULL;

  if (0 != strcmp (url, EXPECTED_URI_BASE_PATH))
  {
    fprintf (stderr, "Unexpected URI: '%s'. ", url);
    mhdErrorExitDesc ("Unexpected URI found");
  }

  response =
    MHD_create_response_from_buffer (MHD_STATICSTR_LEN_ (PAGE), PAGE,
                                     MHD_RESPMEM_PERSISTENT);
  if (NULL == response)
    mhdErrorExitDesc ("Failed to create response");

  if (MHD_YES != MHD_add_response_header (response,
                                          RP_HEADER1_NAME,
                                          RP_HEADER1_VALUE))
    mhdErrorExitDesc ("Cannot add header1");
  if (MHD_YES != MHD_add_response_header (response,
                                          RP_HEADER2_NAME,
                                          RP_HEADER2_VALUE))
    mhdErrorExitDesc ("Cannot add header2");

  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (MHD_YES != ret)
    mhdErrorExitDesc ("Failed to queue response");

  return ret;
}


static CURLcode
performQueryExternal (struct MHD_Daemon *d, CURL *c, CURLM **multi_reuse)
{
  CURLM *multi;
  time_t start;
  struct timeval tv;
  CURLcode ret;
  int libcurl_finished;

  ret = CURLE_FAILED_INIT; /* will be replaced with real result */
  if (NULL != *multi_reuse)
    multi = *multi_reuse;
  else
  {
    multi = curl_multi_init ();
    if (multi == NULL)
      libcurlErrorExitDesc ("curl_multi_init() failed");
    *multi_reuse = multi;
  }
  if (CURLM_OK != curl_multi_add_handle (multi, c))
    libcurlErrorExitDesc ("curl_multi_add_handle() failed");
  libcurl_finished = 0;

  start = time (NULL);
  while (time (NULL) - start <= TIMEOUTS_VAL)
  {
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket maxMhdSk;
    int maxCurlSk;
    MHD_UNSIGNED_LONG_LONG time_o;

    maxMhdSk = MHD_INVALID_SOCKET;
    maxCurlSk = -1;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (! libcurl_finished)
    {
      int running;
      curl_multi_perform (multi, &running);
      if (0 == running)
      {
        struct CURLMsg *msg;
        int msgLeft;
        int totalMsgs = 0;
        do
        {
          msg = curl_multi_info_read (multi, &msgLeft);
          if (NULL == msg)
            libcurlErrorExitDesc ("curl_multi_info_read() failed");
          totalMsgs++;
          if (CURLMSG_DONE == msg->msg)
            ret = msg->data.result;
        } while (msgLeft > 0);
        if (1 != totalMsgs)
        {
          fprintf (stderr,
                   "curl_multi_info_read returned wrong "
                   "number of results (%d).\n",
                   totalMsgs);
          externalErrorExit ();
        }
        curl_multi_remove_handle (multi, c);
        libcurl_finished = ! 0;
      }
      else
      {
        if (CURLM_OK != curl_multi_fdset (multi, &rs, &ws, &es, &maxCurlSk))
          libcurlErrorExitDesc ("curl_multi_fdset() failed");
      }
    }
    if (libcurl_finished)
    { /* libcurl has finished, check whether MHD still needs to perform cleanup */
      if (MHD_YES != MHD_get_timeout (d, &time_o))
        break; /* MHD finished as well */
    }
    if (MHD_NO == MHD_get_fdset (d, &rs, &ws, &es, &maxMhdSk))
      mhdErrorExitDesc ("MHD_get_fdset() failed");
    tv.tv_sec = 0;
    tv.tv_usec = 200000;
    if ((MHD_NO != MHD_get_timeout (d, &time_o)) && (0 == time_o))
      tv.tv_usec = 0;
    else
    {
      long curl_to = -1;
      curl_multi_timeout (multi, &curl_to);
      if (0 == curl_to)
        tv.tv_usec = 0;
    }
#ifdef MHD_POSIX_SOCKETS
    if (maxMhdSk > maxCurlSk)
      maxCurlSk = maxMhdSk;
#endif /* MHD_POSIX_SOCKETS */
    if (-1 == select (maxCurlSk + 1, &rs, &ws, &es, &tv))
    {
#ifdef MHD_POSIX_SOCKETS
      if (EINTR != errno)
        externalErrorExitDesc ("Unexpected select() error");
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
        externalErrorExitDesc ("Unexpected select() error");
      Sleep ((unsigned long) tv.tv_usec / 1000);
#endif
    }
    if (MHD_YES != MHD_run_from_select (d, &rs, &ws, &es))
      mhdErrorExitDesc ("MHD_run_from_select() failed");
  }

  return ret;
}


/**
 * Check request result
 * @param curl_code the CURL easy return code
 * @param pcbc the pointer struct CBC
 * @return non-zero if success, zero if failed
 */
static unsigned int
check_result (CURLcode curl_code, CURL *c, long expected_code,
              struct CBC *pcbc, struct headers_check_result *hdr_res,
              struct ahc_cls_type *ahc_cls)
{
  long code;
  unsigned int ret;

  fflush (stderr);
  fflush (stdout);
  if (CURLE_OK != curl_code)
  {
    fflush (stdout);
    if (0 != libcurl_errbuf[0])
      fprintf (stderr, "Request failed. "
               "libcurl error: '%s'.\n"
               "libcurl error description: '%s'.\n",
               curl_easy_strerror (curl_code),
               libcurl_errbuf);
    else
      fprintf (stderr, "Request failed. "
               "libcurl error: '%s'.\n",
               curl_easy_strerror (curl_code));
    return 0;
  }

  if (CURLE_OK != curl_easy_getinfo (c, CURLINFO_RESPONSE_CODE, &code))
    libcurlErrorExit ();

  if (expected_code != code)
  {
    fprintf (stderr, "The response has wrong HTTP code: %ld\tExpected: %ld.\n",
             code, expected_code);
    return 0;
  }
  else if (verbose)
    printf ("The response has expected HTTP code: %ld\n", expected_code);

  ret = 1;

  if (ahc_cls->req_check_error)
  {
    fprintf (stderr, "One or more errors have been detected by access "
             "handler callback.\n");
    ret = 0;
  }
  if (ahc_cls->expected_upload_size != ahc_cls->up_pos)
  {
    fprintf (stderr, "Upload size does not match expected. "
             "Expected: %lu. "
             "Received: %lu.\n",
             (unsigned long) ahc_cls->expected_upload_size,
             (unsigned long) ahc_cls->up_pos);
    ret = 0;
  }

  if (1 != hdr_res->header1_found)
  {
    if (0 == hdr_res->header1_found)
      fprintf (stderr, "Response header1 was not found.\n");
    else
      fprintf (stderr, "Response header1 was found %d times "
               "instead of one time only.\n", hdr_res->header1_found);
    ret = 0;
  }
  else if (verbose)
    printf ("Header1 is present in the response.\n");
  if (1 != hdr_res->header2_found)
  {
    if (0 == hdr_res->header2_found)
      fprintf (stderr, "Response header2 was not found.\n");
    else
      fprintf (stderr, "Response header2 was found %d times "
               "instead of one time only.\n", hdr_res->header2_found);
    ret = 0;
  }
  else if (verbose)
    printf ("Header2 is present in the response.\n");
  if (1 != hdr_res->size_found)
  {
    if (0 == hdr_res->size_found)
      fprintf (stderr, "Correct response 'Content-Length' header "
               "was not found.\n");
    else
      fprintf (stderr, "Correct response 'Content-Length' header "
               "was found %u times instead of one time only.\n",
               hdr_res->size_found);
    ret = 0;
  }
  else if (verbose)
    printf ("'Content-Length' header with correct value "
            "is present in the response.\n");
  if (0 != hdr_res->size_broken_found)
  {
    fprintf (stderr, "Wrong response 'Content-Length' header was found "
             "%u times.\n", hdr_res->size_broken_found);
    ret = 0;
  }

  if (pcbc->dn_pos != MHD_STATICSTR_LEN_ (PAGE))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) pcbc->dn_pos, (int) pcbc->dn_pos, pcbc->dn_buf,
             (unsigned) MHD_STATICSTR_LEN_ (PAGE));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != memcmp (PAGE, pcbc->dn_buf, pcbc->dn_pos))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ",
             (int) pcbc->dn_pos, pcbc->dn_buf);
    mhdErrorExitDesc ("Wrong returned data");
  }
  fflush (stderr);
  fflush (stdout);
  return ret;
}


static unsigned int
performCheck (void)
{
  struct MHD_Daemon *d;
  uint16_t port;
  struct CBC cbc;
  struct ahc_cls_type ahc_param;
  struct headers_check_result rp_headers_check;
  char buf[2048];
  CURL *c;
  CURLM *multi_reuse;
  int failed = 0;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port =  UINT16_C (4220);
    if (! oneone)
      port += UINT16_C (1);
    if (use_put)
      port += UINT16_C (2);
    if (use_put_large)
      port += UINT16_C (4);
    if (use_hdr_last)
      port += UINT16_C (8);
    if (use_hdr_large)
      port += UINT16_C (16);
  }

  if (1)
  {
    size_t mem_limit;
    if (use_put_large)
      mem_limit = (size_t) (TEST_UPLOAD_DATA_SIZE / 2 + HEADERS_POINTERS_SIZE);
    else
      mem_limit = (size_t) ((TEST_UPLOAD_DATA_SIZE * 4) / 3 + 2
                            + HEADERS_POINTERS_SIZE);

    d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                          port, NULL, NULL,
                          &ahcCheck, &ahc_param,
                          MHD_OPTION_CONNECTION_MEMORY_LIMIT, mem_limit,
                          MHD_OPTION_END);
  }
  if (d == NULL)
    return 1;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;

    dinfo = MHD_get_daemon_info (d,
                                 MHD_DAEMON_INFO_BIND_PORT);
    if ( (NULL == dinfo) ||
         (0 == dinfo->port) )
      mhdErrorExitDesc ("MHD_get_daemon_info() failed");
    port = dinfo->port;
  }

  /* First request */
  ahc_param.rq_method = use_put ? MHD_HTTP_METHOD_PUT : MHD_HTTP_METHOD_GET;
  ahc_param.rq_url = EXPECTED_URI_BASE_PATH;
  ahc_param.expected_upload_size = use_put ? TEST_UPLOAD_DATA_SIZE : 0;
  ahc_param.req_check_error = 0;
  ahc_param.num_req = 0;
  ahc_param.up_pos = 0;
  rp_headers_check.expected_size = MHD_STATICSTR_LEN_ (PAGE);
  rp_headers_check.header1_found = 0;
  rp_headers_check.header2_found = 0;
  rp_headers_check.size_found = 0;
  rp_headers_check.size_broken_found = 0;
  cbc.dn_buf = buf;
  cbc.dn_buf_size = sizeof (buf);
  cbc.dn_pos = 0;
  memset (cbc.dn_buf, 0, cbc.dn_buf_size);
  cbc.up_size = TEST_UPLOAD_DATA_SIZE;
  cbc.up_pos = 0;
  c = setupCURL (&cbc, port, &rp_headers_check);
  multi_reuse = NULL;
  /* First request */
  if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                    MHD_HTTP_OK, &cbc, &rp_headers_check, &ahc_param))
  {
    fflush (stderr);
    if (verbose)
      printf ("Got first expected response.\n");
    fflush (stdout);
  }
  else
  {
    fprintf (stderr, "First request FAILED.\n");
    fflush (stderr);
    failed = 1;
  }
  /* Second request */
  ahc_param.req_check_error = 0;
  ahc_param.num_req = 0;
  ahc_param.up_pos = 0;
  rp_headers_check.header1_found = 0;
  rp_headers_check.header2_found = 0;
  rp_headers_check.size_found = 0;
  rp_headers_check.size_broken_found = 0;
  /* Reset buffer position */
  cbc.dn_pos = 0;
  memset (cbc.dn_buf, 0, cbc.dn_buf_size);
  cbc.up_pos = 0;
  if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                    MHD_HTTP_OK, &cbc, &rp_headers_check, &ahc_param))
  {
    fflush (stderr);
    if (verbose)
      printf ("Got second expected response.\n");
    fflush (stdout);
  }
  else
  {
    fprintf (stderr, "Second request FAILED.\n");
    fflush (stderr);
    failed = 1;
  }
  /* Third request */
  ahc_param.req_check_error = 0;
  ahc_param.num_req = 0;
  ahc_param.up_pos = 0;
  rp_headers_check.header1_found = 0;
  rp_headers_check.header2_found = 0;
  rp_headers_check.size_found = 0;
  rp_headers_check.size_broken_found = 0;
  /* Reset buffer position */
  cbc.dn_pos = 0;
  memset (cbc.dn_buf, 0, cbc.dn_buf_size);
  cbc.up_pos = 0;
  if (NULL != multi_reuse)
    curl_multi_cleanup (multi_reuse);
  multi_reuse = NULL; /* Force new connection */
  if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                    MHD_HTTP_OK, &cbc, &rp_headers_check, &ahc_param))
  {
    fflush (stderr);
    if (verbose)
      printf ("Got third expected response.\n");
    fflush (stdout);
  }
  else
  {
    fprintf (stderr, "Third request FAILED.\n");
    fflush (stderr);
    failed = 1;
  }

  curl_easy_cleanup (c);
  if (NULL != multi_reuse)
    curl_multi_cleanup (multi_reuse);

  MHD_stop_daemon (d);
  return failed ? 1 : 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;

  /* Test type and test parameters */
  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));
  oneone = ! has_in_name (argv[0], "10");

  use_get = has_in_name (argv[0], "_get");
  use_put = has_in_name (argv[0], "_put");

  use_double_fold = has_in_name (argv[0], "_double_fold");
  use_put_large = has_in_name (argv[0], "_put_large");
  use_hdr_last = has_in_name (argv[0], "_last");
  use_hdr_large = has_in_name (argv[0], "_fold_large");

  if (1 !=
      ((use_get ? 1 : 0) + (use_put ? 1 : 0)))
  {
    fprintf (stderr, "Wrong test name '%s': no or multiple indications "
             "for the test type.\n", argv[0] ? argv[0] : "(NULL)");
    return 99;
  }

  test_global_init ();

  errorCount += performCheck ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  test_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
