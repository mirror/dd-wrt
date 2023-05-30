/*
     This file is part of libmicrohttpd
     Copyright (C) 2014-2021 Evgeny Grin (Karlson2k)
     Copyright (C) 2007, 2009, 2011 Christian Grothoff

     libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
     option) any later version.

     libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/
/**
 * @file test_toolarge.c
 * @brief  Testcase for handling of data larger then buffers.
 * @details Testcases for handling of various situations when data cannot fit
 *          the buffers. Address sanitizers and debug asserts should increase
 *          number of problems detected by this test (detect actual overrun).
 *          Tests start with valid sizes to ensure that normal data is processed
 *          correctly and then sizes are monotonically increased to ensure that
 *          overflow is handled correctly at all stages in all codepaths.
 *          Tests with valid sizes are repeated several times to ensure that
 *          tests are failed because of overflow, but because of second run.
 * @author Karlson2k (Evgeny Grin)
 * @author Christian Grothoff
 */
#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "test_helpers.h"
#include "mhd_sockets.h" /* only macros used */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif /* !WIN32_LEAN_AND_MEAN */
#include <windows.h>
#endif

#ifndef WINDOWS
#include <unistd.h>
#include <sys/socket.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif
#if MHD_CPU_COUNT > 32
#undef MHD_CPU_COUNT
/* Limit to reasonable value */
#define MHD_CPU_COUNT 32
#endif /* MHD_CPU_COUNT > 32 */


#if defined(HAVE___FUNC__)
#define externalErrorExit(ignore) \
    _externalErrorExit_func(NULL, __func__, __LINE__)
#define externalErrorExitDesc(errDesc) \
    _externalErrorExit_func(errDesc, __func__, __LINE__)
#define libcurlErrorExit(ignore) \
    _libcurlErrorExit_func(NULL, __func__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
    _libcurlErrorExit_func(errDesc, __func__, __LINE__)
#define mhdErrorExit(ignore) \
    _mhdErrorExit_func(NULL, __func__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
    _mhdErrorExit_func(errDesc, __func__, __LINE__)
#elif defined(HAVE___FUNCTION__)
#define externalErrorExit(ignore) \
    _externalErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define externalErrorExitDesc(errDesc) \
    _externalErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#define libcurlErrorExit(ignore) \
    _libcurlErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
    _libcurlErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#define mhdErrorExit(ignore) \
    _mhdErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define mhdErrorExitDesc(errDesc) \
    _mhdErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func(NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func(errDesc, NULL, __LINE__)
#define libcurlErrorExit(ignore) _libcurlErrorExit_func(NULL, NULL, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func(errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func(NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func(errDesc, NULL, __LINE__)
#endif


_MHD_NORETURN static void
_externalErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
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
  if (0 != libcurl_errbuf[0])
    fprintf (stderr, "Last libcurl error details: %s\n", libcurl_errbuf);

  fflush (stderr);
  exit (99);
}


_MHD_NORETURN static void
_mhdErrorExit_func (const char *errDesc, const char *funcName, int lineNum)
{
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

  fflush (stderr);
  exit (8);
}


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 5

#define EXPECTED_URI_BASE_PATH  "/a"

#define URL_SCHEME_HOST "http:/" "/127.0.0.1"

#define N1_HEADER_NAME "n"
#define N1_HEADER_VALUE "1"
#define N1_HEADER N1_HEADER_NAME ": " N1_HEADER_VALUE
#define N1_HEADER_CRLF N1_HEADER "\r\n"

#define BUFFER_SIZE 1024

/* The size of the test element that must pass the test */
#ifndef MHD_ASAN_POISON_ACTIVE
#define TEST_OK_SIZE (BUFFER_SIZE - 384)
#else  /* MHD_ASAN_POISON_ACTIVE */
#define TEST_OK_SIZE (BUFFER_SIZE - 384 - 80)
#endif /* MHD_ASAN_POISON_ACTIVE */

/* The size of the test element where tests are started */
#define TEST_START_SIZE (TEST_OK_SIZE - 16)

/* The size of the test element that must definitely fail */
#define TEST_FAIL_SIZE (BUFFER_SIZE + 32)

/* Special value for request many headers test.
 * MHD uses the same buffer to store headers strings and pointers to the strings
 * so allocation is multiplied for small request header. */
/* The size of the test element that must pass the test */
#define TEST_RQ_N1_OK_SIZE 50

/* The size of the test element where tests are started */
#define TEST_RQ_N1_START_SIZE (TEST_RQ_N1_OK_SIZE - 32)

/* Global parameters */
static int verbose;                 /**< Be verbose */
static int oneone;                  /**< If false use HTTP/1.0 for requests*/
static int global_port;             /**< MHD daemons listen port number */
static int large_req_method;        /**< Large request method */
static int large_req_url;           /**< Large request URL */
static int large_req_header_name; /**< Large request single header name */
static int large_req_header_value; /**< Large request single header value */
static int large_req_headers; /**< Large request headers */
static int large_rsp_header_name; /**< Large response single header name */
static int large_rsp_header_value; /**< Large response single header value */
static int large_rsp_headers; /**< Large response headers */
static int response_timeout_val = TIMEOUTS_VAL;

/* Current test parameters */
/* * Moved to local variables * */

/* Static helper variables */
/* * None for this test * */

static void
test_global_init (void)
{
  libcurl_errbuf[0] = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    externalErrorExit ();
}


static void
test_global_cleanup (void)
{
  curl_global_cleanup ();
}


struct headers_check_result
{
  unsigned int num_n1_headers;
  unsigned int large_header_name_size;
  unsigned int large_header_value_size;
  int large_header_valid;
};

size_t
lcurl_hdr_callback (char *buffer, size_t size, size_t nitems,
                    void *userdata)
{
  const size_t data_size = size * nitems;
  struct headers_check_result *check_res =
    (struct headers_check_result *) userdata;

  if ((6 == data_size) && (0 == memcmp (N1_HEADER_CRLF, buffer, 6)))
    check_res->num_n1_headers++;
  else if ((5 <= data_size) && ('0' == buffer[0]))
  {
    const char *const col_ptr = memchr (buffer, ':', data_size);
    if (0 != check_res->large_header_value_size)
      mhdErrorExitDesc ("Expected only one large header, " \
                        "but found two large headers in the reply");
    if (NULL == col_ptr)
      check_res->large_header_valid = 0;
    else if ((size_t) (col_ptr - buffer) >= data_size - 2)
      check_res->large_header_valid = 0;
    else if (*(col_ptr + 1) != ' ')
      check_res->large_header_valid = 0;
    else
    {
      const char *const name = buffer;
      const size_t name_len = col_ptr - buffer;
      const size_t val_pos = name_len + 2;
      const size_t val_len = data_size - val_pos - 2; /* 2 = strlen("\r\n") */
      const char *const value = buffer + val_pos;
      size_t i;
      check_res->large_header_name_size = name_len;
      check_res->large_header_value_size = val_len;
      check_res->large_header_valid = 1; /* To be reset if any problem found */
      for (i = 1; i < name_len; i++)
        if ('a' + (char) (i % ('z' - 'a' + 1)) != name[i])
        {
          fprintf (stderr, "Wrong sequence in reply header name " \
                   "at position %u. Expected '%c', got '%c'\n",
                   (unsigned int) i,
                   'a' + (char) (i % ('z' - 'a' + 1)),
                   name[i]);
          check_res->large_header_valid = 0;
          break;
        }
      for (i = 0; i < val_len; i++)
        if ('Z' - (char) (i % ('Z' - 'A' + 1)) != value[i])
        {
          fprintf (stderr, "Wrong sequence in reply header value " \
                   "at position %u. Expected '%c', got '%c'\n",
                   (unsigned int) i,
                   'Z' - (char) (i % ('Z' - 'A' + 1)),
                   value[i]);
          check_res->large_header_valid = 0;
          break;
        }
    }
  }

  return data_size;
}


struct lcurl_data_cb_param
{
  char *buf;
  size_t pos;
  size_t size;
};


static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct lcurl_data_cb_param *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    externalErrorExit ();  /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


struct check_uri_cls
{
  const char *volatile uri;
};

static void *
check_uri_cb (void *cls,
              const char *uri,
              struct MHD_Connection *con)
{
  struct check_uri_cls *param = (struct check_uri_cls *) cls;
  (void) con;

  if (0 != strcmp (param->uri,
                   uri))
  {
    fprintf (stderr,
             "Wrong URI: `%s', line: %d\n",
             uri, __LINE__);
    exit (22);
  }
  return NULL;
}


struct mhd_header_checker_param
{
  unsigned int num_n1_headers;
  unsigned int large_header_name_size;
  unsigned int large_header_value_size;
  int large_header_valid;
};

enum MHD_Result
headerCheckerInterator (void *cls,
                        enum MHD_ValueKind kind,
                        const char *key,
                        size_t key_size,
                        const char *value,
                        size_t value_size)
{
  struct mhd_header_checker_param *const param =
    (struct mhd_header_checker_param *) cls;

  if (NULL == param)
    mhdErrorExitDesc ("cls parameter is NULL");

  if (MHD_HEADER_KIND != kind)
    return MHD_YES; /* Continue iteration */

  if (0 == key_size)
    mhdErrorExitDesc ("Zero key length");

  if ((1 == key_size) && (1 == value_size) &&
      ('n' == key[0]) && ('1' == value[0]))
    param->num_n1_headers++;
  else if ('0' == key[0])
  { /* Found 'large' header */
    size_t i;
    param->large_header_name_size = key_size;
    param->large_header_value_size = value_size;
    param->large_header_valid = 1;
    for (i = 1; i < key_size; i++)
      if ('a' + (char) (i % ('z' - 'a' + 1)) != key[i])
      {
        fprintf (stderr, "Wrong sequence in request header name " \
                 "at position %u. Expected '%c', got '%c'\n",
                 (unsigned int) i,
                 'a' + (char) (i % ('z' - 'a' + 1)),
                 key[i]);
        param->large_header_valid = 0;
        break;
      }
    for (i = 0; i < value_size; i++)
      if ('Z' - (char) (i % ('Z' - 'A' + 1)) != value[i])
      {
        fprintf (stderr, "Wrong sequence in request header value " \
                 "at position %u. Expected '%c', got '%c'\n",
                 (unsigned int) i,
                 'Z' - (char) (i % ('Z' - 'A' + 1)),
                 value[i]);
        param->large_header_valid = 0;
        break;
      }
  }
  return MHD_YES;
}


struct ahc_cls_type
{
  const char *volatile rp_data;
  volatile size_t rp_data_size;
  volatile size_t rp_num_n1_hdrs;
  volatile size_t rp_large_hdr_name_size;
  volatile size_t rp_large_hdr_value_size;
  struct mhd_header_checker_param header_check_param;
  const char *volatile rq_method;
  const char *volatile rq_url;
};


static enum MHD_Result
ahcCheck (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **con_cls)
{
  static int ptr;
  struct MHD_Response *response;
  enum MHD_Result ret;
  struct ahc_cls_type *const param = (struct ahc_cls_type *) cls;
  size_t i;

  if (NULL == param)
    mhdErrorExitDesc ("cls parameter is NULL");

  if (0 != strcmp (version, MHD_HTTP_VERSION_1_1))
    mhdErrorExitDesc ("Unexpected HTTP version");

  if (0 != strcmp (url, param->rq_url))
    mhdErrorExitDesc ("Unexpected URI");

  if (NULL != upload_data)
    mhdErrorExitDesc ("'upload_data' is not NULL");

  if (NULL == upload_data_size)
    mhdErrorExitDesc ("'upload_data_size' pointer is NULL");

  if (0 != *upload_data_size)
    mhdErrorExitDesc ("'*upload_data_size' value is not zero");

  if (0 != strcmp (param->rq_method, method))
    mhdErrorExitDesc ("Unexpected request method");

  if (&ptr != *con_cls)
  {
    *con_cls = &ptr;
    return MHD_YES;
  }
  *con_cls = NULL;

  if (1 > MHD_get_connection_values_n (connection, MHD_HEADER_KIND,
                                       &headerCheckerInterator,
                                       &param->header_check_param))
    mhdErrorExitDesc ("Wrong number of headers in the request");

  response = MHD_create_response_from_buffer (param->rp_data_size,
                                              (void *) param->rp_data,
                                              MHD_RESPMEM_MUST_COPY);
  if (NULL == response)
    mhdErrorExitDesc ("Failed to create response");

  for (i = 0; i < param->rp_num_n1_hdrs; i++)
    if (MHD_YES != MHD_add_response_header (response,
                                            N1_HEADER_NAME,
                                            N1_HEADER_VALUE))
      mhdErrorExitDesc ("Cannot add header");

  if (0 != param->rp_large_hdr_name_size)
  {
    const size_t large_hdr_name_size = param->rp_large_hdr_name_size;
    char *large_hrd_name;
    const size_t large_hdr_value_size = param->rp_large_hdr_value_size;
    char *large_hrd_value;

    large_hrd_name = malloc (large_hdr_name_size + 1);
    if (NULL == large_hrd_name)
      externalErrorExit ();
    if (0 != large_hdr_value_size)
      large_hrd_value = malloc (large_hdr_value_size + 1);
    else
      large_hrd_value = NULL;

    if ((0 != large_hdr_value_size) && (NULL == large_hrd_value))
      externalErrorExit ();
    large_hrd_name[0] = '0'; /* Name starts with zero for unique identification */
    for (i = 1; i < large_hdr_name_size; i++)
      large_hrd_name[i] = 'a' + i % ('z' - 'a' + 1);
    large_hrd_name[large_hdr_name_size] = 0;
    for (i = 0; i < large_hdr_value_size; i++)
      large_hrd_value[i] = 'Z' - i % ('Z' - 'A' + 1);
    if (NULL != large_hrd_value)
      large_hrd_value[large_hdr_value_size] = 0;
    if (MHD_YES != MHD_add_response_header (response,
                                            large_hrd_name,
                                            large_hrd_value))
      mhdErrorExitDesc ("Cannot add large header");

    if (NULL != large_hrd_value)
      free (large_hrd_value);
    free (large_hrd_name);
  }

  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (MHD_YES != ret)
    mhdErrorExitDesc ("Failed to queue response");

  return ret;
}


static CURL *
curlEasyInitForTest (const char *queryPath, const char *method,
                     int port,
                     struct lcurl_data_cb_param *dcbp,
                     struct headers_check_result *hdr_chk_result,
                     struct curl_slist *headers)
{
  CURL *c;

  c = curl_easy_init ();
  if (NULL == c)
    libcurlErrorExitDesc ("curl_easy_init() failed");

  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL, queryPath)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, dcbp)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     (long) response_timeout_val)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     (long) response_timeout_val)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER,
                                     libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HEADERFUNCTION,
                                     lcurl_hdr_callback)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HEADERDATA,
                                     hdr_chk_result)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
      (oneone) ?
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                     CURL_HTTP_VERSION_1_1)) :
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                     CURL_HTTP_VERSION_1_0)))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_CUSTOMREQUEST, method))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, headers))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  return c;
}


static CURLcode
performQueryExternal (struct MHD_Daemon *d, CURL *c)
{
  CURLM *multi;
  time_t start;
  struct timeval tv;
  CURLcode ret;

  ret = CURLE_FAILED_INIT; /* will be replaced with real result */
  multi = NULL;
  multi = curl_multi_init ();
  if (multi == NULL)
    libcurlErrorExitDesc ("curl_multi_init() failed");
  if (CURLM_OK != curl_multi_add_handle (multi, c))
    libcurlErrorExitDesc ("curl_multi_add_handle() failed");

  start = time (NULL);
  while (time (NULL) - start <= TIMEOUTS_VAL)
  {
    fd_set rs;
    fd_set ws;
    fd_set es;
    MHD_socket maxMhdSk;
    int maxCurlSk;
    int running;

    maxMhdSk = MHD_INVALID_SOCKET;
    maxCurlSk = -1;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (NULL != multi)
    {
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
        curl_multi_cleanup (multi);
        multi = NULL;
      }
      else
      {
        if (CURLM_OK != curl_multi_fdset (multi, &rs, &ws, &es, &maxCurlSk))
          libcurlErrorExitDesc ("curl_multi_fdset() failed");
      }
    }
    if (NULL == multi)
    { /* libcurl has finished, check whether MHD still needs to perform cleanup */
      unsigned long long to;
      if ((MHD_YES != MHD_get_timeout (d, &to)) || (0 != to))
        break; /* MHD finished as well */
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxMhdSk))
      mhdErrorExitDesc ("MHD_get_fdset() failed");
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
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
      Sleep (1);
#endif
    }
    if (MHD_YES != MHD_run_from_select (d, &rs, &ws, &es))
      mhdErrorExitDesc ("MHD_run_from_select() failed");
  }

  return ret;
}


struct curlQueryParams
{
  /* Destination path for CURL query */
  const char *queryPath;

  /* Custom query method, NULL for default */
  const char *method;

  /* Destination port for CURL query */
  int queryPort;

  /* List of additional request headers */
  struct curl_slist *headers;

  /* CURL query result error flag */
  volatile int queryError;

  /* Response HTTP code, zero if no response */
  volatile int responseCode;
};


/* Returns zero for successful response and non-zero for failed response */
static int
doCurlQueryInThread (struct MHD_Daemon *d,
                     struct curlQueryParams *p,
                     struct headers_check_result *hdr_res,
                     const char *expected_data,
                     size_t expected_data_size)
{
  const union MHD_DaemonInfo *dinfo;
  CURL *c;
  struct lcurl_data_cb_param dcbp;
  CURLcode errornum;
  int use_external_poll;
  long resp_code;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_FLAGS);
  if (NULL == dinfo)
    mhdErrorExitDesc ("MHD_get_daemon_info() failed");
  use_external_poll = (0 == (dinfo->flags
                             & MHD_USE_INTERNAL_POLLING_THREAD));

  if (NULL == p->queryPath)
    abort ();

  if (0 == p->queryPort)
    abort ();

  /* Test must not fail due to test's internal buffer shortage */
  dcbp.size = TEST_FAIL_SIZE * 2 + 1;
  dcbp.buf = malloc (dcbp.size);
  if (NULL == dcbp.buf)
    externalErrorExit ();
  dcbp.pos = 0;

  memset (hdr_res, 0, sizeof(*hdr_res));

  c = curlEasyInitForTest (p->queryPath, p->method, p->queryPort,
                           &dcbp, hdr_res, p->headers);

  if (! use_external_poll)
    errornum = curl_easy_perform (c);
  else
    errornum = performQueryExternal (d, c);

  if (CURLE_OK != curl_easy_getinfo (c, CURLINFO_RESPONSE_CODE, &resp_code))
    libcurlErrorExitDesc ("curl_easy_getinfo() failed");

  p->responseCode = (int) resp_code;
  if ((CURLE_OK == errornum) && (200 != resp_code))
  {
    fprintf (stderr,
             "Got reply with unexpected status code: %d\n",
             p->responseCode);
    mhdErrorExit ();
  }

  if (CURLE_OK != errornum)
  {
    if ((CURLE_GOT_NOTHING != errornum) && (CURLE_RECV_ERROR != errornum)
        && (CURLE_HTTP_RETURNED_ERROR != errornum))
    {
      if (CURLE_OPERATION_TIMEDOUT == errornum)
        mhdErrorExitDesc ("Request was aborted due to timeout");
      fprintf (stderr, "libcurl returned unexpected error: %s\n",
               curl_easy_strerror (errornum));
      mhdErrorExitDesc ("Request failed due to unexpected error");
    }
    p->queryError = 1;
    if ((0 != resp_code) &&
        ((499 < resp_code) || (400 > resp_code))) /* TODO: add all expected error codes */
    {
      fprintf (stderr,
               "Got reply with unexpected status code: %ld\n",
               resp_code);
      mhdErrorExit ();
    }
  }
  else
  {
    if (dcbp.pos != expected_data_size)
      mhdErrorExit ("libcurl reports wrong size of MHD reply body data");
    else if (0 != memcmp (expected_data, dcbp.buf,
                          expected_data_size))
      mhdErrorExit ("libcurl reports wrong MHD reply body data");
    else
      p->queryError = 0;
  }

  curl_easy_cleanup (c);
  free (dcbp.buf);

  return p->queryError;
}


/* Perform test queries, shut down MHD daemon, and free parameters */
static int
performTestQueries (struct MHD_Daemon *d, int d_port,
                    struct ahc_cls_type *ahc_param,
                    struct check_uri_cls *uri_cb_param)
{
  struct curlQueryParams qParam;
  int ret = 0;          /* Return value */
  struct headers_check_result rp_headers_check;
  char *buf;
  size_t i;
  size_t first_failed_at = 0;


  buf = malloc (TEST_FAIL_SIZE + 1 + strlen (URL_SCHEME_HOST));
  if (NULL == buf)
    externalErrorExit ();

  /* Common parameters, to be individually overridden by specific test cases */
  qParam.queryPort = d_port;
  qParam.method = NULL;  /* Use libcurl default: GET */
  qParam.queryPath = URL_SCHEME_HOST EXPECTED_URI_BASE_PATH;
  qParam.headers = NULL; /* No additional headers */
  uri_cb_param->uri = EXPECTED_URI_BASE_PATH;
  ahc_param->rq_url = EXPECTED_URI_BASE_PATH;
  ahc_param->rq_method = "GET"; /* Default expected method */

  ahc_param->rp_data = "~";
  ahc_param->rp_data_size = 1;
  ahc_param->rp_large_hdr_name_size = 0;
  ahc_param->rp_large_hdr_value_size = 0;
  ahc_param->rp_num_n1_hdrs = 0;

  if (large_req_method)
  {
    for (i = 0; i < TEST_START_SIZE; i++)
      buf[i] = 'A' + i % ('Z' - 'A' + 1);
    for (; i <= TEST_FAIL_SIZE; i++)
    {
      buf[i] = 0;

      qParam.method = buf;
      ahc_param->rq_method = buf;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != ahc_param->header_check_param.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large request header");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
      if (0 != rp_headers_check.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large reply header");

      buf[i] = 'A' + i % ('Z' - 'A' + 1);
    }
  }
  else if (large_req_url)
  {
    const size_t base_size = strlen (URL_SCHEME_HOST);
    char *const url = buf + base_size;

    memcpy (buf, URL_SCHEME_HOST, base_size);
    url[0] = '/';
    for (i = 1; i < TEST_START_SIZE; i++)
      url[i] = 'a' + i % ('z' - 'a' + 1);
    for (; i <= TEST_FAIL_SIZE; i++)
    {
      url[i] = 0;

      qParam.queryPath = buf;
      uri_cb_param->uri = url;
      ahc_param->rq_url = url;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != ahc_param->header_check_param.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large request header");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
      if (0 != rp_headers_check.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large reply header");

      url[i] = 'a' + i % ('z' - 'a' + 1);
    }
  }
  else if (large_req_header_name)
  {
    buf[0] = '0'; /* Name starts with zero for unique identification */
    for (i = 1; i < TEST_START_SIZE; i++)
      buf[i] = 'a' + i % ('z' - 'a' + 1);
    for (; i <= TEST_FAIL_SIZE; i++)
    {
      struct curl_slist *curl_headers;
      curl_headers = NULL;

      memcpy (buf + i, ": Z", 3); /* Note: strlen(": Z") is less than strlen(URL_SCHEME_HOST) */
      buf[i + 3] = 0;

      curl_headers = curl_slist_append (curl_headers, buf);
      if (NULL == curl_headers)
        externalErrorExit ();

      qParam.headers = curl_headers;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (0 != ahc_param->header_check_param.large_header_name_size)
        { /* If large header was processed, it must be valid */
          if (i != ahc_param->header_check_param.large_header_name_size)
            mhdErrorExitDesc ("Detected wrong large request header name size");
          if (1 != ahc_param->header_check_param.large_header_value_size)
            mhdErrorExitDesc ("Detected wrong large request header value size");
          if (0 == ahc_param->header_check_param.large_header_valid)
            mhdErrorExitDesc ("Detected wrong large request header");
        }
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (i != ahc_param->header_check_param.large_header_name_size)
          mhdErrorExitDesc ("Detected wrong large request header name size");
        if (1 != ahc_param->header_check_param.large_header_value_size)
          mhdErrorExitDesc ("Detected wrong large request header value size");
        if (0 == ahc_param->header_check_param.large_header_valid)
          mhdErrorExitDesc ("Detected wrong large request header");
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
      if (0 != rp_headers_check.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large reply header");

      curl_slist_free_all (curl_headers);
      buf[i] = 'a' + i % ('z' - 'a' + 1);
    }
  }
  else if (large_req_header_value)
  {
    char *const hdr_value = buf + 3;
    /* Name starts with zero for unique identification */
    memcpy (buf, "0: ", 3); /* Note: strlen(": Z") is less than strlen(URL_SCHEME_HOST) */
    for (i = 0; i < TEST_START_SIZE; i++)
      hdr_value[i] = 'Z' - i % ('Z' - 'A' + 1);
    for (; i <= TEST_FAIL_SIZE; i++)
    {
      struct curl_slist *curl_headers;
      curl_headers = NULL;

      hdr_value[i] = 0;

      curl_headers = curl_slist_append (curl_headers, buf);
      if (NULL == curl_headers)
        externalErrorExit ();

      qParam.headers = curl_headers;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (0 != ahc_param->header_check_param.large_header_name_size)
        { /* If large header was processed, it must be valid */
          if (1 != ahc_param->header_check_param.large_header_name_size)
            mhdErrorExitDesc ("Detected wrong large request header name size");
          if (i != ahc_param->header_check_param.large_header_value_size)
            mhdErrorExitDesc ("Detected wrong large request header value size");
          if (0 == ahc_param->header_check_param.large_header_valid)
            mhdErrorExitDesc ("Detected wrong large request header");
        }
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (1 != ahc_param->header_check_param.large_header_name_size)
          mhdErrorExitDesc ("Detected wrong large request header name size");
        if (i != ahc_param->header_check_param.large_header_value_size)
          mhdErrorExitDesc ("Detected wrong large request header value size");
        if (0 == ahc_param->header_check_param.large_header_valid)
          mhdErrorExitDesc ("Detected wrong large request header");
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
      if (0 != rp_headers_check.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large reply header");

      curl_slist_free_all (curl_headers);
      hdr_value[i] = 'Z' - i % ('Z' - 'A' + 1);
    }
  }
  else if (large_req_headers)
  {
    unsigned int num_hdrs = 0;
    struct curl_slist *curl_headers;
    const size_t hdr_size = strlen (N1_HEADER_CRLF);

    curl_headers = NULL;

    for (i = 0; i < TEST_RQ_N1_START_SIZE; i += hdr_size)
    {
      curl_headers = curl_slist_append (curl_headers, N1_HEADER);
      if (NULL == curl_headers)
        externalErrorExit ();
      num_hdrs++;
    }
    for (; i <= TEST_FAIL_SIZE; i += hdr_size)
    {
      qParam.headers = curl_headers;
      ahc_param->header_check_param.num_n1_headers = num_hdrs;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (0 != ahc_param->header_check_param.num_n1_headers)
        { /* If headers were processed, they must be valid */
          if (num_hdrs != ahc_param->header_check_param.num_n1_headers)
            mhdErrorExitDesc ("Detected wrong number of request headers");
        }
        if (TEST_RQ_N1_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (num_hdrs != ahc_param->header_check_param.num_n1_headers)
          mhdErrorExitDesc ("Detected wrong number of request headers");
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large request header");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
      if (0 != rp_headers_check.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large reply header");

      curl_headers = curl_slist_append (curl_headers, N1_HEADER);
      if (NULL == curl_headers)
        externalErrorExit ();
      num_hdrs++;
    }
    curl_slist_free_all (curl_headers);
  }
  else if (large_rsp_header_name)
  {
    for (i = TEST_START_SIZE; i <= TEST_FAIL_SIZE; i++)
    {
      ahc_param->rp_large_hdr_name_size = i;
      ahc_param->rp_large_hdr_value_size = 1;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (0 != rp_headers_check.large_header_name_size)
          mhdErrorExitDesc ("Detected unexpected large reply header");
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (i != rp_headers_check.large_header_name_size)
          mhdErrorExitDesc ("Detected wrong large reply header name size");
        if (1 != rp_headers_check.large_header_value_size)
          mhdErrorExitDesc ("Detected wrong large reply header value size");
        if (0 == rp_headers_check.large_header_valid)
          mhdErrorExitDesc ("Detected wrong large reply header");
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != ahc_param->header_check_param.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large request header");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
    }
  }
  else if (large_rsp_header_value)
  {
    for (i = TEST_START_SIZE; i <= TEST_FAIL_SIZE; i++)
    {
      ahc_param->rp_large_hdr_name_size = 1;
      ahc_param->rp_large_hdr_value_size = i;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (0 != rp_headers_check.large_header_name_size)
          mhdErrorExitDesc ("Detected unexpected large reply header");
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (1 != rp_headers_check.large_header_name_size)
          mhdErrorExitDesc ("Detected wrong large reply header name size");
        if (i != rp_headers_check.large_header_value_size)
          mhdErrorExitDesc ("Detected wrong large reply header value size");
        if (0 == rp_headers_check.large_header_valid)
          mhdErrorExitDesc ("Detected wrong large reply header");
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != ahc_param->header_check_param.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large request header");
      if (0 != rp_headers_check.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected reply headers");
    }
  }
  else if (large_rsp_headers)
  {
    unsigned int num_hrds;
    const size_t hdr_size = strlen (N1_HEADER_CRLF);

    for (num_hrds = TEST_START_SIZE / hdr_size;
         num_hrds * hdr_size <= TEST_FAIL_SIZE; num_hrds++)
    {
      i = num_hrds * hdr_size;
      ahc_param->rp_num_n1_hdrs = num_hrds;

      memset (&ahc_param->header_check_param, 0,
              sizeof (ahc_param->header_check_param));
      if (0 != doCurlQueryInThread (d, &qParam, &rp_headers_check,
                                    ahc_param->rp_data,
                                    ahc_param->rp_data_size))
      {
        (void) qParam.responseCode; /* TODO: check for the right response code */
        if (0 != rp_headers_check.num_n1_headers)
          mhdErrorExitDesc ("Detected unexpected reply headers");
        if (TEST_OK_SIZE >= i)
        {
          fprintf (stderr,
                   "Request failed when running with the valid value size.\n");
          ret = 1; /* Failed too early */
        }
        if (0 == first_failed_at)
        {
          if (verbose)
            fprintf (stderr, "First failed size is %u.\n", (unsigned int) i);
          first_failed_at = i;
        }
      }
      else
      {
        if (num_hrds != rp_headers_check.num_n1_headers)
          mhdErrorExitDesc ("Detected wrong number of reply headers");
        if (TEST_FAIL_SIZE == i)
        {
          fprintf (stderr, "Request succeed with the largest size.\n");
          ret = 1; /* Succeed with largest value */
        }
      }
      if (0 != ahc_param->header_check_param.num_n1_headers)
        mhdErrorExitDesc ("Detected unexpected request headers");
      if (0 != ahc_param->header_check_param.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large request header");
      if (0 != rp_headers_check.large_header_name_size)
        mhdErrorExitDesc ("Detected unexpected large reply header");
    }
  }
  else
    externalErrorExitDesc ("No valid test test was selected");

  MHD_stop_daemon (d);
  free (buf);
  free (uri_cb_param);
  free (ahc_param);

  return ret;
}


enum testMhdThreadsType
{
  testMhdThreadExternal              = 0,
  testMhdThreadInternal              = MHD_USE_INTERNAL_POLLING_THREAD,
  testMhdThreadInternalPerConnection = MHD_USE_THREAD_PER_CONNECTION
                                       | MHD_USE_INTERNAL_POLLING_THREAD,
  testMhdThreadInternalPool
};

enum testMhdPollType
{
  testMhdPollBySelect = 0,
  testMhdPollByPoll   = MHD_USE_POLL,
  testMhdPollByEpoll  = MHD_USE_EPOLL,
  testMhdPollAuto     = MHD_USE_AUTO
};

/* Get number of threads for thread pool depending
 * on used poll function and test type. */
static unsigned int
testNumThreadsForPool (enum testMhdPollType pollType)
{
  int numThreads = MHD_CPU_COUNT;
  (void) pollType; /* Don't care about pollType for this test */
  return numThreads; /* No practical limit for non-cleanup test */
}


static struct MHD_Daemon *
startTestMhdDaemon (enum testMhdThreadsType thrType,
                    enum testMhdPollType pollType, int *pport,
                    struct ahc_cls_type **ahc_param,
                    struct check_uri_cls **uri_cb_param)
{
  struct MHD_Daemon *d;
  const union MHD_DaemonInfo *dinfo;

  if ((NULL == ahc_param) || (NULL == uri_cb_param))
    abort ();

  *ahc_param = (struct ahc_cls_type *) malloc (sizeof(struct ahc_cls_type));
  if (NULL == *ahc_param)
    externalErrorExit ();
  *uri_cb_param =
    (struct check_uri_cls *) malloc (sizeof(struct check_uri_cls));
  if (NULL == *uri_cb_param)
    externalErrorExit ();

  if ( (0 == *pport) &&
       (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT)) )
  {
    *pport = 4100;
    if (large_req_method)
      *pport += 1;
    if (large_req_url)
      *pport += 2;
    if (large_req_header_name)
      *pport += 3;
    if (large_req_header_value)
      *pport += 4;
    if (large_req_headers)
      *pport += 5;
    if (large_rsp_header_name)
      *pport += 6;
    if (large_rsp_header_value)
      *pport += 7;
    if (large_rsp_headers)
      *pport += 8;
    if (! oneone)
      *pport += 16;
  }

  if (testMhdThreadInternalPool != thrType)
    d = MHD_start_daemon (((int) thrType) | ((int) pollType)
                          | (verbose ? MHD_USE_ERROR_LOG : 0),
                          *pport, NULL, NULL,
                          &ahcCheck, *ahc_param,
                          MHD_OPTION_URI_LOG_CALLBACK, &check_uri_cb,
                          *uri_cb_param,
                          MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                          (size_t) BUFFER_SIZE,
                          MHD_OPTION_END);
  else
    d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | ((int) pollType)
                          | (verbose ? MHD_USE_ERROR_LOG : 0),
                          *pport, NULL, NULL,
                          &ahcCheck, *ahc_param,
                          MHD_OPTION_THREAD_POOL_SIZE,
                          testNumThreadsForPool (pollType),
                          MHD_OPTION_URI_LOG_CALLBACK, &check_uri_cb,
                          *uri_cb_param,
                          MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                          (size_t) BUFFER_SIZE,
                          MHD_OPTION_END);

  if (NULL == d)
  {
    fprintf (stderr, "Failed to start MHD daemon, errno=%d.\n", errno);
    abort ();
  }

  if (0 == *pport)
  {
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      fprintf (stderr, "MHD_get_daemon_info() failed.\n");
      abort ();
    }
    *pport = (int) dinfo->port;
    if (0 == global_port)
      global_port = *pport; /* Reuse the same port for all tests */
  }

  return d;
}


/* Test runners */


static int
testExternalGet (void)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;

  d = startTestMhdDaemon (testMhdThreadExternal, testMhdPollBySelect, &d_port,
                          &ahc_param, &uri_cb_param);

  return performTestQueries (d, d_port, ahc_param, uri_cb_param);
}


static int
testInternalGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;

  d = startTestMhdDaemon (testMhdThreadInternal, pollType, &d_port,
                          &ahc_param, &uri_cb_param);

  return performTestQueries (d, d_port, ahc_param, uri_cb_param);
}


static int
testMultithreadedGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;

  d = startTestMhdDaemon (testMhdThreadInternalPerConnection, pollType, &d_port,
                          &ahc_param, &uri_cb_param);
  return performTestQueries (d, d_port, ahc_param, uri_cb_param);
}


static int
testMultithreadedPoolGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */
  struct ahc_cls_type *ahc_param;
  struct check_uri_cls *uri_cb_param;

  d = startTestMhdDaemon (testMhdThreadInternalPool, pollType, &d_port,
                          &ahc_param, &uri_cb_param);
  return performTestQueries (d, d_port, ahc_param, uri_cb_param);
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  unsigned int test_result = 0;
  verbose = 0;

  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  oneone = ! has_in_name (argv[0], "10");
  large_req_method = has_in_name (argv[0], "_method") ? 1 : 0;
  large_req_url = has_in_name (argv[0], "_url") ? 1 : 0;
  large_req_header_name = has_in_name (argv[0], "_request_header_name") ?
                          1 : 0;
  large_req_header_value = has_in_name (argv[0], "_request_header_value") ?
                           1 : 0;
  large_req_headers = has_in_name (argv[0], "_request_headers") ? 1 : 0;
  large_rsp_header_name = has_in_name (argv[0], "_reply_header_name") ?
                          1 : 0;
  large_rsp_header_value = has_in_name (argv[0], "_reply_header_value") ?
                           1 : 0;
  large_rsp_headers = has_in_name (argv[0], "_reply_headers") ? 1 : 0;
  if (large_req_method + large_req_url + large_req_header_name
      + large_req_header_value + large_req_headers + large_rsp_header_name
      + large_rsp_header_value + large_rsp_headers != 1)
    return 99;
  verbose = ! has_param (argc, argv, "-q") || has_param (argc, argv, "--quiet");

  test_global_init ();

  /* Could be set to non-zero value to enforce using specific port
   * in the test */
  global_port = 0;
  test_result = testExternalGet ();
  if (test_result)
    fprintf (stderr, "FAILED: testExternalGet () - %u.\n", test_result);
  else if (verbose)
    printf ("PASSED: testExternalGet ().\n");
  errorCount += test_result;
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    test_result = testInternalGet (testMhdPollAuto);
    if (test_result)
      fprintf (stderr, "FAILED: testInternalGet (testMhdPollAuto) - %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testInternalGet (testMhdPollBySelect).\n");
    errorCount += test_result;
#ifdef _MHD_HEAVY_TESTS
    /* Actually tests are not heavy, but took too long to complete while
     * not really provide any additional results. */
    test_result = testInternalGet (testMhdPollBySelect);
    if (test_result)
      fprintf (stderr, "FAILED: testInternalGet (testMhdPollBySelect) - %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testInternalGet (testMhdPollBySelect).\n");
    errorCount += test_result;
    test_result = testMultithreadedPoolGet (testMhdPollBySelect);
    if (test_result)
      fprintf (stderr,
               "FAILED: testMultithreadedPoolGet (testMhdPollBySelect) - %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testMultithreadedPoolGet (testMhdPollBySelect).\n");
    errorCount += test_result;
    test_result = testMultithreadedGet (testMhdPollBySelect);
    if (test_result)
      fprintf (stderr,
               "FAILED: testMultithreadedGet (testMhdPollBySelect) - %u.\n",
               test_result);
    else if (verbose)
      printf ("PASSED: testMultithreadedGet (testMhdPollBySelect).\n");
    errorCount += test_result;
    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_POLL))
    {
      test_result = testInternalGet (testMhdPollByPoll);
      if (test_result)
        fprintf (stderr, "FAILED: testInternalGet (testMhdPollByPoll) - %u.\n",
                 test_result);
      else if (verbose)
        printf ("PASSED: testInternalGet (testMhdPollByPoll).\n");
      errorCount += test_result;
    }
    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_EPOLL))
    {
      test_result = testInternalGet (testMhdPollByEpoll);
      if (test_result)
        fprintf (stderr, "FAILED: testInternalGet (testMhdPollByEpoll) - %u.\n",
                 test_result);
      else if (verbose)
        printf ("PASSED: testInternalGet (testMhdPollByEpoll).\n");
      errorCount += test_result;
    }
#else
    /* Mute compiler warnings */
    (void) testMultithreadedGet;
    (void) testMultithreadedPoolGet;
#endif /* _MHD_HEAVY_TESTS */
  }
  if (0 != errorCount)
    fprintf (stderr,
             "Error (code: %u)\n",
             errorCount);
  else if (verbose)
    printf ("All tests passed.\n");

  test_global_cleanup ();

  return (errorCount == 0) ? 0 : 1;       /* 0 == pass */
}
