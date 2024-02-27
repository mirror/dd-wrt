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
 * @file testcurl/test_head.c
 * @brief  Testcase for HEAD requests
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <unistd.h>
#else
#include <wincrypt.h>
#endif

#include "mhd_has_param.h"
#include "mhd_has_in_name.h"

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

#define EXPECTED_URI_BASE_PATH  "/"

#define EXISTING_URI  EXPECTED_URI_BASE_PATH

#define EXPECTED_URI_BASE_PATH_MISSING  "/wrong_uri"

#define URL_SCHEME "http:/" "/"

#define URL_HOST "127.0.0.1"

#define URL_SCHEME_HOST URL_SCHEME URL_HOST

#define HEADER1_NAME "First"
#define HEADER1_VALUE "1st"
#define HEADER1 HEADER1_NAME ": " HEADER1_VALUE
#define HEADER1_CRLF HEADER1 "\r\n"
#define HEADER2_NAME "Normal"
#define HEADER2_VALUE "it's fine"
#define HEADER2 HEADER2_NAME ": " HEADER2_VALUE
#define HEADER2_CRLF HEADER2 "\r\n"

#define PAGE \
  "<html><head><title>libmicrohttpd demo page</title></head>" \
  "<body>Success!</body></html>"

#define PAGE_404 \
  "<html><head><title>404 error</title></head>" \
  "<body>Error 404: The requested URI does not exist</body></html>"

/* Global parameters */
static int verbose;
static int oneone;                  /**< If false use HTTP/1.0 for requests*/

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
  unsigned int expected_size;
  int header1_found;
  int header2_found;
  int size_found;
};

static size_t
lcurl_hdr_callback (char *buffer, size_t size, size_t nitems,
                    void *userdata)
{
  const size_t data_size = size * nitems;
  struct headers_check_result *check_res =
    (struct headers_check_result *) userdata;

  if ((MHD_STATICSTR_LEN_ (HEADER1_CRLF) == data_size) &&
      (0 == memcmp (HEADER1_CRLF, buffer, data_size)))
    check_res->header1_found++;
  else if ((MHD_STATICSTR_LEN_ (HEADER2_CRLF) == data_size) &&
           (0 == memcmp (HEADER2_CRLF, buffer, data_size)))
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
      mhdErrorExitDesc ("Broken Content-Length");
    else if ((((size_t) res + 2) != data_size - numbers_pos) ||
             (0 != memcmp (buffer + numbers_pos, cmpbuf, (size_t) res)))
    {
      fprintf (stderr, "Wrong Content-Length.\n"
               "Expected:\n%u\n"
               "Received:\n%s", check_res->expected_size,
               buffer + numbers_pos);
      mhdErrorExitDesc ("Wrong Content-Length");
    }
    else if (0 != memcmp ("\r\n", buffer + data_size - 2, 2))
    {
      mhdErrorExitDesc ("The Content-Length header is not " \
                        "terminated by CRLF");
    }
    check_res->size_found++;
  }

  return data_size;
}


struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};


static size_t
copyBuffer (void *ptr,
            size_t size,
            size_t nmemb,
            void *ctx)
{
  (void) ptr; /* Unused, mute compiler warning */
  (void) ctx; /* Unused, mute compiler warning */
  if ((0 != size) && (0 != nmemb))
    libcurlErrorExitDesc ("Received unexpected body data");
  return size * nmemb;
}


struct ahc_cls_type
{
  const char *rq_method;
  const char *rq_url;
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
  unsigned int http_code;

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

  if (NULL != upload_data)
    mhdErrorExitDesc ("'upload_data' is not NULL");

  if (NULL == upload_data_size)
    mhdErrorExitDesc ("'upload_data_size' pointer is NULL");

  if (0 != *upload_data_size)
    mhdErrorExitDesc ("'*upload_data_size' value is not zero");

  if (0 != strcmp (param->rq_method, method))
    mhdErrorExitDesc ("Unexpected request method");

  if (&marker != *req_cls)
  {
    *req_cls = &marker;
    return MHD_YES;
  }
  *req_cls = NULL;

  if (0 == strcmp (url, EXISTING_URI))
  {
    response =
      MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE),
                                              PAGE);
    http_code = MHD_HTTP_OK;
  }
  else
  {
    response =
      MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (PAGE_404),
                                              PAGE_404);
    http_code = MHD_HTTP_NOT_FOUND;
  }
  if (NULL == response)
    mhdErrorExitDesc ("Failed to create response");

  if (MHD_YES != MHD_add_response_header (response,
                                          HEADER1_NAME,
                                          HEADER1_VALUE))
    mhdErrorExitDesc ("Cannot add header1");
  if (MHD_YES != MHD_add_response_header (response,
                                          HEADER2_NAME,
                                          HEADER2_VALUE))
    mhdErrorExitDesc ("Cannot add header2");

  ret = MHD_queue_response (connection,
                            http_code,
                            response);
  MHD_destroy_response (response);
  if (MHD_YES != ret)
    mhdErrorExitDesc ("Failed to queue response");

  return ret;
}


/**
 * Set required URI for the request
 * @param c the CURL handle to use
 * @param uri_exist if non-zero use request for "existing" URI
 */
static void
setCURL_rq_path (CURL *c, int uri_exist)
{
  if (uri_exist)
  {
    if (CURLE_OK !=
        curl_easy_setopt (c, CURLOPT_URL,
                          URL_SCHEME_HOST EXPECTED_URI_BASE_PATH))
      libcurlErrorExitDesc ("Cannot set request URL");
  }
  else
  {
    if (CURLE_OK !=
        curl_easy_setopt (c, CURLOPT_URL,
                          URL_SCHEME_HOST EXPECTED_URI_BASE_PATH_MISSING))
      libcurlErrorExitDesc ("Cannot set request URL");
  }
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
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, ((long) port))))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  /* When 'CURLOPT_NOBODY' is set, libcurl should use HEAD request. */
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_NOBODY, (long) 1))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");

  return c;
}


static CURLcode
performQueryExternal (struct MHD_Daemon *d, CURL *c, CURLM **multi_reuse)
{
  CURLM *multi;
  time_t start;
  struct timeval tv;
  CURLcode ret;

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
      if (0 != MHD_get_timeout64s (d))
        break; /* MHD finished as well */
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxMhdSk))
      mhdErrorExitDesc ("MHD_get_fdset() failed");
    tv.tv_sec = 0;
    tv.tv_usec = 200000;
    if (0 == MHD_get_timeout64s (d))
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
              struct headers_check_result *hdr_res)
{
  long code;
  unsigned int ret;

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
    fflush (stderr);
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
      fprintf (stderr, "Response 'Content-Length' header was not found.\n");
    else
      fprintf (stderr, "Response 'Content-Length' header was found %d times "
               "instead of one time only.\n", hdr_res->size_found);
    ret = 0;
  }
  else if (verbose)
    printf ("'Content-Length' header with correct value "
            "is present in the response.\n");

  return ret;
}


static unsigned int
testHead (void)
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
    port = 4220 + oneone ? 0 : 1;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG | MHD_USE_NO_THREAD_SAFETY,
                        port, NULL, NULL,
                        &ahcCheck, &ahc_param,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);
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
  ahc_param.rq_method = MHD_HTTP_METHOD_HEAD;
  ahc_param.rq_url = EXPECTED_URI_BASE_PATH;
  rp_headers_check.expected_size = MHD_STATICSTR_LEN_ (PAGE);
  rp_headers_check.header1_found = 0;
  rp_headers_check.header2_found = 0;
  rp_headers_check.size_found = 0;
  cbc.buf = buf;
  cbc.size = sizeof (buf);
  cbc.pos = 0;
  memset (cbc.buf, 0, cbc.size);
  c = setupCURL (&cbc, port, &rp_headers_check);
  setCURL_rq_path (c, 1);
  multi_reuse = NULL;
  /* First request */
  if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                    MHD_HTTP_OK, &rp_headers_check))
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
  rp_headers_check.expected_size = MHD_STATICSTR_LEN_ (PAGE_404);
  rp_headers_check.header1_found = 0;
  rp_headers_check.header2_found = 0;
  rp_headers_check.size_found = 0;
  cbc.pos = 0; /* Reset buffer position */
  ahc_param.rq_url = EXPECTED_URI_BASE_PATH_MISSING;
  setCURL_rq_path (c, 0);
  if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                    MHD_HTTP_NOT_FOUND, &rp_headers_check))
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
  rp_headers_check.header1_found = 0;
  rp_headers_check.header2_found = 0;
  rp_headers_check.size_found = 0;
  cbc.pos = 0; /* Reset buffer position */
  if (NULL != multi_reuse)
    curl_multi_cleanup (multi_reuse);
  multi_reuse = NULL; /* Force new connection */
  if (check_result (performQueryExternal (d, c, &multi_reuse), c,
                    MHD_HTTP_NOT_FOUND, &rp_headers_check))
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

  test_global_init ();

  errorCount += testHead ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  test_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
