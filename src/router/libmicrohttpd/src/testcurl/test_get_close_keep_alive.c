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
 * @file test_get_close_keep_alive.c
 * @brief  Testcase for libmicrohttpd "Close" and "Keep-Alive" connection.
 * @details Testcases for testing of MHD automatic choice between "Close" and
 *          "Keep-Alive" connections. Also tested selected HTTP version and
 *          "Connection:" headers.
 * @author Karlson2k (Evgeny Grin)
 * @author Christian Grothoff
 */
#include "MHD_config.h"
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

#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x)<<16|(y)<<8|(z))
#endif /* ! CURL_VERSION_BITS */
#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(x, y, z))
#endif /* ! CURL_AT_LEAST_VERSION */

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
#elif defined(HAVE___FUNCTION__)
#define externalErrorExit(ignore) \
    _externalErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define externalErrorExitDesc(errDesc) \
    _externalErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#define libcurlErrorExit(ignore) \
    _libcurlErrorExit_func(NULL, __FUNCTION__, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
    _libcurlErrorExit_func(errDesc, __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func(NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func(errDesc, NULL, __LINE__)
#define libcurlErrorExit(ignore) _externalErrorExit_func(NULL, NULL, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _externalErrorExit_func(errDesc, NULL, __LINE__)
#endif


static void
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

static void
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


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 5

#define EXPECTED_URI_BASE_PATH  "/hello_world"
#define EXPECTED_URI_QUERY      "a=%26&b=c"
#define EXPECTED_URI_FULL_PATH  EXPECTED_URI_BASE_PATH "?" EXPECTED_URI_QUERY
#define HDR_CONN_CLOSE_VALUE      "close"
#define HDR_CONN_CLOSE            MHD_HTTP_HEADER_CONNECTION ": " \
                                  HDR_CONN_CLOSE_VALUE
#define HDR_CONN_KEEP_ALIVE_VALUE "Keep-Alive"
#define HDR_CONN_KEEP_ALIVE       MHD_HTTP_HEADER_CONNECTION ": " \
                                  HDR_CONN_KEEP_ALIVE_VALUE

/* Global parameters */
static int oneone;         /**< Use HTTP/1.1 instead of HTTP/1.0 for requests*/
static int conn_close;     /**< Don't use Keep-Alive */
static int global_port;    /**< MHD daemons listen port number */
static int slow_reply = 0; /**< Slowdown MHD replies */
static int ignore_response_errors = 0; /**< Do not fail test if CURL
                                            returns error */
static int response_timeout_val = TIMEOUTS_VAL;

/* Current test parameters */
/* Poor thread sync, but enough for the testing */
static volatile int mhd_add_close; /**< Add "Connection: close" header by MHD */
static volatile int mhd_set_10_cmptbl; /**< Set MHD_RF_HTTP_1_0_COMPATIBLE_STRICT response flag */
static volatile int mhd_set_10_server; /**< Set MHD_RF_HTTP_1_0_SERVER response flag */
static volatile int mhd_set_k_a_send; /**< Set MHD_RF_SEND_KEEP_ALIVE_HEADER response flag */

/* Static helper variables */
static struct curl_slist *curl_close_hdr;   /**< CURL "Connection: close" header */
static struct curl_slist *curl_k_alive_hdr; /**< CURL "Connection: keep-alive" header */
static struct curl_slist *curl_both_hdrs;   /**< CURL both "Connection: keep-alive" and "close" headers */

static void
test_global_init (void)
{
  libcurl_errbuf[0] = 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    externalErrorExit ();

  curl_close_hdr = NULL;
  curl_close_hdr = curl_slist_append (curl_close_hdr,
                                      HDR_CONN_CLOSE);
  if (NULL == curl_close_hdr)
    externalErrorExit ();

  curl_k_alive_hdr = NULL;
  curl_k_alive_hdr = curl_slist_append (curl_k_alive_hdr,
                                        HDR_CONN_KEEP_ALIVE);
  if (NULL == curl_k_alive_hdr)
    externalErrorExit ();

  curl_both_hdrs = NULL;
  curl_both_hdrs = curl_slist_append (curl_both_hdrs,
                                      HDR_CONN_KEEP_ALIVE);
  if (NULL == curl_both_hdrs)
    externalErrorExit ();
  curl_both_hdrs = curl_slist_append (curl_both_hdrs,
                                      HDR_CONN_CLOSE);
  if (NULL == curl_both_hdrs)
    externalErrorExit ();
}


static void
test_global_cleanup (void)
{
  curl_slist_free_all (curl_both_hdrs);
  curl_slist_free_all (curl_k_alive_hdr);
  curl_slist_free_all (curl_close_hdr);

  curl_global_cleanup ();
}


struct headers_check_result
{
  int found_http11;
  int found_http10;
  int found_conn_close;
  int found_conn_keep_alive;
};

size_t
lcurl_hdr_callback (char *buffer, size_t size, size_t nitems,
                    void *userdata)
{
  const size_t data_size = size * nitems;
  struct headers_check_result *check_res =
    (struct headers_check_result *) userdata;

  if ((strlen (MHD_HTTP_VERSION_1_1) < data_size) &&
      (0 == memcmp (MHD_HTTP_VERSION_1_1, buffer,
                    strlen (MHD_HTTP_VERSION_1_1))))
    check_res->found_http11 = 1;
  else if ((strlen (MHD_HTTP_VERSION_1_0) < data_size) &&
           (0 == memcmp (MHD_HTTP_VERSION_1_0, buffer,
                         strlen (MHD_HTTP_VERSION_1_0))))
    check_res->found_http10 = 1;
  else if ((data_size == strlen (HDR_CONN_CLOSE) + 2) &&
           (0 == memcmp (buffer, HDR_CONN_CLOSE "\r\n", data_size)))
    check_res->found_conn_close = 1;
  else if ((data_size == strlen (HDR_CONN_KEEP_ALIVE) + 2) &&
           (0 == memcmp (buffer, HDR_CONN_KEEP_ALIVE "\r\n", data_size)))
    check_res->found_conn_keep_alive = 1;

  return data_size;
}


struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};


static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    externalErrorExit ();  /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


static void *
log_cb (void *cls,
        const char *uri,
        struct MHD_Connection *con)
{
  (void) cls;
  (void) con;
  if (0 != strcmp (uri,
                   EXPECTED_URI_FULL_PATH))
  {
    fprintf (stderr,
             "Wrong URI: `%s', line: %d\n",
             uri, __LINE__);
    exit (22);
  }
  return NULL;
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **unused)
{
  static int ptr;
  const char *me = cls;
  struct MHD_Response *response;
  enum MHD_Result ret;
  (void) version;
  (void) upload_data;
  (void) upload_data_size;       /* Unused. Silence compiler warning. */

  if (0 != strcmp (me, method))
    return MHD_NO;              /* unexpected method */
  if (&ptr != *unused)
  {
    *unused = &ptr;
    return MHD_YES;
  }
  *unused = NULL;
  if (slow_reply)
    usleep (200000);

  response = MHD_create_response_from_buffer (strlen (url),
                                              (void *) url,
                                              MHD_RESPMEM_MUST_COPY);
  if (NULL == response)
  {
    fprintf (stderr, "Failed to create response. Line: %d\n", __LINE__);
    exit (19);
  }
  if (mhd_add_close)
  {
    if (MHD_YES != MHD_add_response_header (response,
                                            MHD_HTTP_HEADER_CONNECTION,
                                            HDR_CONN_CLOSE_VALUE))
    {
      fprintf (stderr, "Failed to add header. Line: %d\n", __LINE__);
      exit (19);
    }
  }
  if (MHD_YES != MHD_set_response_options (response,
                                           (mhd_set_10_cmptbl ?
                                            MHD_RF_HTTP_1_0_COMPATIBLE_STRICT
                                               : 0)
                                           | (mhd_set_10_server ?
                                              MHD_RF_HTTP_1_0_SERVER
                                               : 0)
                                           | (mhd_set_k_a_send ?
                                              MHD_RF_SEND_KEEP_ALIVE_HEADER
                                               : 0), MHD_RO_END))
  {
    fprintf (stderr, "Failed to set response flags. Line: %d\n", __LINE__);
    exit (19);
  }

  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (MHD_YES != ret)
  {
    fprintf (stderr, "Failed to queue response. Line: %d\n", __LINE__);
    exit (19);
  }
  return ret;
}


struct curlQueryParams
{
  /* Destination path for CURL query */
  const char *queryPath;

  /* Destination port for CURL query */
  int queryPort;

  /* CURL query result error flag */
  volatile int queryError;
};


static CURL *
curlEasyInitForTest (const char *queryPath, int port, struct CBC *pcbc,
                     struct headers_check_result *hdr_chk_result,
                     int add_hdr_close, int add_hdr_k_alive)
{
  CURL *c;

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    externalErrorExit ();
  }
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL, queryPath)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, pcbc)) ||
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
  {
    fprintf (stderr, "curl_easy_setopt() failed.\n");
    externalErrorExit ();
  }
  if (add_hdr_close && add_hdr_k_alive)
  { /* This combination is actually incorrect */
    if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, curl_both_hdrs))
    {
      fprintf (stderr, "Set libcurl HTTP header failed.\n");
      externalErrorExit ();
    }
  }
  else if (add_hdr_close)
  {
    if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, curl_close_hdr))
    {
      fprintf (stderr, "Set libcurl HTTP header failed.\n");
      externalErrorExit ();
    }
  }
  else if (add_hdr_k_alive)
  {
    if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, curl_k_alive_hdr))
    {
      fprintf (stderr, "Set libcurl HTTP header failed.\n");
      externalErrorExit ();
    }
  }

  return c;
}


static void
print_test_params (int add_hdr_close,
                   int add_hdr_k_alive)
{
  fprintf (stderr, "Request HTTP/%s| ", oneone ? "1.1" : "1.0");
  fprintf (stderr, "Connection must be: %s| ",
           conn_close ? "close" : "keep-alive");
  fprintf (stderr, "Request \"close\": %s| ",
           add_hdr_close ? "    used" : "NOT used");
  fprintf (stderr, "Request \"keep-alive\": %s| ",
           add_hdr_k_alive ? "    used" : "NOT used");
  fprintf (stderr, "MHD response \"close\": %s| ",
           mhd_add_close ? "    used" : "NOT used");
  fprintf (stderr, "MHD response 1.0 strict compatible: %s| ",
           mhd_set_10_cmptbl ? "yes" : " NO");
  fprintf (stderr, "MHD response 1.0 server: %s| ",
           mhd_set_10_server ? "yes" : " NO");
  fprintf (stderr, "MHD response send \"Keep-Alive\": %s|",
           mhd_set_k_a_send ? "yes" : " NO");
  fprintf (stderr, "\n*** ");
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
  {
    fprintf (stderr, "curl_multi_init() failed.\n");
    externalErrorExit ();
  }
  if (CURLM_OK != curl_multi_add_handle (multi, c))
  {
    fprintf (stderr, "curl_multi_add_handle() failed.\n");
    externalErrorExit ();
  }

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
          {
            fprintf (stderr, "curl_multi_info_read failed, NULL returned.\n");
            externalErrorExit ();
          }
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
        {
          fprintf (stderr, "curl_multi_fdset() failed.\n");
          externalErrorExit ();
        }
      }
    }
    if (NULL == multi)
    { /* libcurl has finished, check whether MHD still needs to perform cleanup */
      unsigned long long to;
      if ((MHD_YES != MHD_get_timeout (d, &to)) || (0 != to))
        break; /* MHD finished as well */
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxMhdSk))
    {
      fprintf (stderr, "MHD_get_fdset() failed. Line: %d\n", __LINE__);
      exit (11);
      break;
    }
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
      {
        fprintf (stderr, "Unexpected select() error: %d. Line: %d\n",
                 (int) errno, __LINE__);
        fflush (stderr);
        exit (99);
      }
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
      {
        fprintf (stderr, "Unexpected select() error: %d. Line: %d\n",
                 (int) WSAGetLastError (), __LINE__);
        fflush (stderr);
        exit (99);
      }
      Sleep (1);
#endif
    }
    if (MHD_YES != MHD_run_from_select (d, &rs, &ws, &es))
    {
      fprintf (stderr, "MHD_run_from_select() failed. Line: %d\n", __LINE__);
      exit (11);
    }
  }

  return ret;
}


static unsigned int
getMhdActiveConnections (struct MHD_Daemon *d)
{
  const union MHD_DaemonInfo *dinfo;
  /* The next method is unreliable unless it's known that no
   * connections are started or finished in parallel */
  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_CURRENT_CONNECTIONS);
  if (NULL == dinfo)
  {
    fprintf (stderr, "MHD_get_daemon_info() failed.\n");
    abort ();
  }
  return dinfo->num_connections;
}


static int
doCurlQueryInThread (struct MHD_Daemon *d,
                     struct curlQueryParams *p,
                     int add_hdr_close,
                     int add_hdr_k_alive)
{
  const union MHD_DaemonInfo *dinfo;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  struct headers_check_result hdr_res;
  CURLcode errornum;
  int use_external_poll;

  dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_FLAGS);
  if (NULL == dinfo)
  {
    fprintf (stderr, "MHD_get_daemon_info() failed.\n");
    abort ();
  }
  use_external_poll = (0 == (dinfo->flags
                             & MHD_USE_INTERNAL_POLLING_THREAD));

  if (NULL == p->queryPath)
    abort ();

  if (0 == p->queryPort)
    abort ();

  cbc.buf = buf;
  cbc.size = sizeof(buf);
  cbc.pos = 0;

  hdr_res.found_http11 = 0;
  hdr_res.found_http10 = 0;
  hdr_res.found_conn_close = 0;
  hdr_res.found_conn_keep_alive = 0;

  c = curlEasyInitForTest (p->queryPath, p->queryPort, &cbc, &hdr_res,
                           add_hdr_close, add_hdr_k_alive);

  if (! use_external_poll)
    errornum = curl_easy_perform (c);
  else
    errornum = performQueryExternal (d, c);
  if (ignore_response_errors)
  {
    p->queryError = 0;
    curl_easy_cleanup (c);

    return p->queryError;
  }
  if (CURLE_OK != errornum)
  {
    p->queryError = 1;
    fprintf (stderr,
             "libcurl query failed: `%s'\n",
             curl_easy_strerror (errornum));
    libcurlErrorExit ();
  }
  else
  {
    if (cbc.pos != strlen (EXPECTED_URI_BASE_PATH))
    {
      fprintf (stderr, "curl reports wrong size of MHD reply body data.\n");
      p->queryError = 1;
    }
    else if (0 != strncmp (EXPECTED_URI_BASE_PATH, cbc.buf,
                           strlen (EXPECTED_URI_BASE_PATH)))
    {
      fprintf (stderr, "curl reports wrong MHD reply body data.\n");
      p->queryError = 1;
    }
    else
      p->queryError = 0;
  }

  if (! hdr_res.found_http11 && ! hdr_res.found_http10)
  {
    print_test_params (add_hdr_close, add_hdr_k_alive);
    fprintf (stderr, "No know HTTP versions were found in the "
             "reply header. Line: %d\n", __LINE__);
    exit (24);
  }
  else if (hdr_res.found_http11 && hdr_res.found_http10)
  {
    print_test_params (add_hdr_close, add_hdr_k_alive);
    fprintf (stderr, "Both HTTP/1.1 and HTTP/1.0 were found in the "
             "reply header. Line: %d\n", __LINE__);
    exit (24);
  }

  if (conn_close)
  {
    if (! hdr_res.found_conn_close)
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "\"Connection: close\" was not found in"
               " MHD reply headers.\n");
      p->queryError |= 2;
    }
    if (hdr_res.found_conn_keep_alive)
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "\"Connection: keep-alive\" was found in"
               " MHD reply headers.\n");
      p->queryError |= 2;
    }
    if (use_external_poll)
    { /* The number of MHD connection can queried only with external poll.
       * otherwise it creates a race condition. */
      if (0 != getMhdActiveConnections (d))
      {
        print_test_params (add_hdr_close, add_hdr_k_alive);
        fprintf (stderr, "MHD still has active connection "
                 "after response has been sent.\n");
        p->queryError |= 2;
      }
    }
  }
  else
  { /* Keep-Alive */
    if (! oneone || mhd_set_10_server || mhd_set_k_a_send)
    { /* Should have "Connection: Keep-Alive" */
      if (! hdr_res.found_conn_keep_alive)
      {
        print_test_params (add_hdr_close, add_hdr_k_alive);
        fprintf (stderr, "\"Connection: keep-alive\" was not found in"
                 " MHD reply headers.\n");
        p->queryError |= 2;
      }
    }
    else
    { /* Should NOT have "Connection: Keep-Alive" */
      if (hdr_res.found_conn_keep_alive)
      {
        print_test_params (add_hdr_close, add_hdr_k_alive);
        fprintf (stderr, "\"Connection: keep-alive\" was found in"
                 " MHD reply headers.\n");
        p->queryError |= 2;
      }
    }
    if (hdr_res.found_conn_close)
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "\"Connection: close\" was found in"
               " MHD reply headers.\n");
      p->queryError |= 2;
    }
    if (use_external_poll)
    { /* The number of MHD connection can be queried only with external poll.
       * otherwise it creates a race condition. */
      unsigned int num_conn = getMhdActiveConnections (d);
      if (0 == num_conn)
      {
        print_test_params (add_hdr_close, add_hdr_k_alive);
        fprintf (stderr, "MHD has no active connection "
                 "after response has been sent.\n");
        p->queryError |= 2;
      }
      else if (1 != num_conn)
      {
        print_test_params (add_hdr_close, add_hdr_k_alive);
        fprintf (stderr, "MHD has wrong number of active connection (%u) "
                 "after response has been sent. Line: %d\n", num_conn,
                 __LINE__);
        exit (23);
      }
    }
  }

#if defined(CURL_AT_LEAST_VERSION) && CURL_AT_LEAST_VERSION (7, 45, 0)
  if (! use_external_poll)
  {
    /* libcurl closes connection socket with curl_multi_remove_handle () /
       curl_multi_cleanup() */
    curl_socket_t curl_sckt;
    if (CURLE_OK !=  curl_easy_getinfo (c, CURLINFO_ACTIVESOCKET, &curl_sckt))
    {
      fprintf (stderr,
               "Failed to get libcurl active socket.\n");
      libcurlErrorExit ();
    }

    if (conn_close && (CURL_SOCKET_BAD != curl_sckt))
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "libcurl still has active connection "
               "after performing the test query.\n");
      p->queryError |= 2;
    }
    else if (! conn_close && (CURL_SOCKET_BAD == curl_sckt))
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "libcurl has no active connection "
               "after performing the test query.\n");
      p->queryError |= 2;
    }
  }
#endif

  if (! mhd_set_10_server)
  { /* Response must be HTTP/1.1 */
    if (hdr_res.found_http10)
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "Reply has HTTP/1.0 version, while it "
               "must be HTTP/1.1.\n");
      p->queryError |= 4;
    }
  }
  else
  { /* Response must be HTTP/1.0 */
    if (hdr_res.found_http11)
    {
      print_test_params (add_hdr_close, add_hdr_k_alive);
      fprintf (stderr, "Reply has HTTP/1.1 version, while it "
               "must be HTTP/1.0.\n");
      p->queryError |= 4;
    }
  }
  curl_easy_cleanup (c);

  return p->queryError;
}


/* Perform test queries and shut down MHD daemon */
static int
performTestQueries (struct MHD_Daemon *d, int d_port)
{
  struct curlQueryParams qParam;
  int ret = 0;          /* Return value */
  int i = 0;
  /* masks */
  const int m_mhd_close = 1 << (i++);
  const int m_10_cmptbl = 1 << (i++);
  const int m_10_server = 1 << (i++);
  const int m_k_a_send = 1 << (i++);
  const int m_client_close = 1 << (i++);
  const int m_client_k_alive = 1 << (i++);

  qParam.queryPath = "http://127.0.0.1" EXPECTED_URI_FULL_PATH;
  qParam.queryPort = d_port;   /* Connect to the daemon */

  for (i = (1 << i) - 1; 0 <= i; i--)
  {
    const int f_mhd_close = (0 == (i & m_mhd_close)); /**< Use MHD "close" header */
    const int f_10_cmptbl = (0 == (i & m_10_cmptbl)); /**< Use MHD MHD_RF_HTTP_1_0_COMPATIBLE_STRICT flag */
    const int f_10_server = (0 == (i & m_10_server)); /**< Use MHD MHD_RF_HTTP_1_0_SERVER flag */
    const int f_k_a_send = (0 == (i & m_k_a_send));   /**< Use MHD MHD_RF_SEND_KEEP_ALIVE_HEADER flag */
    const int f_client_close = (0 == (i & m_client_close)); /**< Use libcurl "close" header */
    const int f_client_k_alive = (0 == (i & m_client_k_alive)); /**< Use libcurl "Keep-Alive" header */
    int res_close; /**< Indicate the result of the test query should be closed connection */

    if (f_mhd_close)         /* Connection with server's "close" header must be always closed */
      res_close = 1;
    else if (f_client_close) /* Connection with client's "close" header must be always closed */
      res_close = 1;
    else if (f_10_cmptbl)    /* Connection in strict HTTP/1.0 compatible mode must be always closed */
      res_close = 1;
    else if (! oneone || f_10_server)
      /* With HTTP/1.0 client or server connection must be close unless client use "keep-alive" header */
      res_close = ! f_client_k_alive;
    else
      res_close = 0; /* HTTP/1.1 is "keep-alive" by default */

    if ((! ! res_close) != (! ! conn_close))
      continue; /* Another mode is in test currently */

    mhd_add_close = f_mhd_close;
    mhd_set_10_cmptbl = f_10_cmptbl;
    mhd_set_10_server = f_10_server;
    mhd_set_k_a_send = f_k_a_send;

    ret <<= 3;                   /* Remember errors for each step */
    ret |= doCurlQueryInThread (d, &qParam, f_client_close, f_client_k_alive);
  }

  MHD_stop_daemon (d);

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
                    enum testMhdPollType pollType, int *pport)
{
  struct MHD_Daemon *d;
  const union MHD_DaemonInfo *dinfo;

  if ( (0 == *pport) &&
       (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT)) )
  {
    *pport = 4050;
    if (oneone)
      *pport += 1;
    if (! conn_close)
      *pport += 2;
  }

  if (testMhdThreadInternalPool != thrType)
    d = MHD_start_daemon (((int) thrType) | ((int) pollType)
                          | MHD_USE_ERROR_LOG,
                          *pport, NULL, NULL,
                          &ahc_echo, "GET",
                          MHD_OPTION_URI_LOG_CALLBACK, &log_cb, NULL,
                          MHD_OPTION_END);
  else
    d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | ((int) pollType)
                          | MHD_USE_ERROR_LOG,
                          *pport, NULL, NULL,
                          &ahc_echo, "GET",
                          MHD_OPTION_THREAD_POOL_SIZE,
                          testNumThreadsForPool (pollType),
                          MHD_OPTION_URI_LOG_CALLBACK, &log_cb, NULL,
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

  d = startTestMhdDaemon (testMhdThreadExternal, testMhdPollBySelect, &d_port);

  return performTestQueries (d, d_port);
}


static int
testInternalGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */

  d = startTestMhdDaemon (testMhdThreadInternal, pollType,
                          &d_port);

  return performTestQueries (d, d_port);
}


static int
testMultithreadedGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */

  d = startTestMhdDaemon (testMhdThreadInternalPerConnection, pollType,
                          &d_port);
  return performTestQueries (d, d_port);
}


static int
testMultithreadedPoolGet (enum testMhdPollType pollType)
{
  struct MHD_Daemon *d;
  int d_port = global_port; /* Daemon's port */

  d = startTestMhdDaemon (testMhdThreadInternalPool, pollType,
                          &d_port);
  return performTestQueries (d, d_port);
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  unsigned int test_result = 0;
  int verbose = 0;

  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  oneone = ! has_in_name (argv[0], "10");
  conn_close = has_in_name (argv[0], "_close");
  if (! conn_close && ! has_in_name (argv[0], "_keep_alive"))
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
