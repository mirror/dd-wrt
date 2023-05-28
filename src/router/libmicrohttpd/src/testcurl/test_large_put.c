/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2008 Christian Grothoff
     Copyright (C) 2014-2021 Evgeny Grin (Karlson2k)

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
 * @file test_large_put.c
 * @brief  Testcase for libmicrohttpd PUT operations
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "MHD_config.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef WINDOWS
#include <unistd.h>
#endif

#include "test_helpers.h"

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif


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


static int oneone;
static int incr_read; /* Use incremental read */
static int verbose; /* Be verbose */

#define PUT_SIZE (256 * 1024)

static char *put_buffer;

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

char *
alloc_init (size_t buf_size)
{
  static const char template[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz";
  static const size_t templ_size = sizeof(template) / sizeof(char) - 1;
  char *buf;
  char *fill_ptr;
  size_t to_fill;

  buf = malloc (buf_size);
  if (NULL == buf)
    externalErrorExit ();

  fill_ptr = buf;
  to_fill = buf_size;
  while (to_fill > 0)
  {
    const size_t to_copy = to_fill > templ_size ? templ_size : to_fill;
    memcpy (fill_ptr, template, to_copy);
    fill_ptr += to_copy;
    to_fill -= to_copy;
  }
  return buf;
}


static size_t
putBuffer (void *stream, size_t size, size_t nmemb, void *ptr)
{
  size_t *pos = (size_t *) ptr;
  size_t wrt;

  wrt = size * nmemb;
  /* Check for overflow. */
  if (wrt / size != nmemb)
    libcurlErrorExitDesc ("Too large buffer size");
  if (wrt > PUT_SIZE - (*pos))
    wrt = PUT_SIZE - (*pos);
  memcpy (stream, &put_buffer[*pos], wrt);
  (*pos) += wrt;
  return wrt;
}


static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    libcurlErrorExitDesc ("Too large buffer size");
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **pparam)
{
  int *done = cls;
  struct MHD_Response *response;
  enum MHD_Result ret;
  static size_t processed;

  if (NULL == cls)
    mhdErrorExitDesc ("cls parameter is NULL");

  if (0 != strcmp (version, oneone ?
                   MHD_HTTP_VERSION_1_1 : MHD_HTTP_VERSION_1_0))
    mhdErrorExitDesc ("Unexpected HTTP version");

  if (NULL == url)
    mhdErrorExitDesc ("url parameter is NULL");

  if (NULL == upload_data_size)
    mhdErrorExitDesc ("'upload_data_size' pointer is NULL");

  if (0 != strcmp ("PUT", method))
    mhdErrorExitDesc ("Unexpected request method");   /* unexpected method */

  if ((*done) == 0)
  {
    size_t *pproc;
    if (NULL == *pparam)
    {
      processed = 0;
      /* Safe as long as only one parallel request served. */
      *pparam = &processed;
    }
    pproc = (size_t *) *pparam;

    if (0 == *upload_data_size)
      return MHD_YES;   /* No data to process. */

    if (*pproc + *upload_data_size > PUT_SIZE)
      mhdErrorExitDesc ("Incoming data larger than expected");

    if ( (! incr_read) && (*upload_data_size != PUT_SIZE) )
      return MHD_YES;   /* Wait until whole request is received. */

    if (0 != memcmp (upload_data, put_buffer + (*pproc), *upload_data_size))
      mhdErrorExitDesc ("Incoming data does not match sent data");

    *pproc += *upload_data_size;
    *upload_data_size = 0;   /* Current block of data is fully processed. */

    if (PUT_SIZE == *pproc)
      *done = 1;   /* Whole request is processed. */
    return MHD_YES;
  }
  response = MHD_create_response_from_buffer (strlen (url),
                                              (void *) url,
                                              MHD_RESPMEM_MUST_COPY);
  if (NULL == response)
    mhdErrorExitDesc ("Failed to create response");
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


static int
testPutInternalThread (unsigned int add_flag)
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  size_t pos = 0;
  int done_flag = 0;
  CURLcode errornum;
  char buf[2048];
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1270;
    if (oneone)
      port += 10;
    if (incr_read)
      port += 20;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                        | add_flag,
                        port,
                        NULL, NULL, &ahc_echo, &done_flag,
                        MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                        (size_t) (incr_read ? 1024 : (PUT_SIZE * 4 / 3)),
                        MHD_OPTION_END);
  if (d == NULL)
    mhdErrorExit ();
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      mhdErrorExit ();
    port = (int) dinfo->port;
  }

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    externalErrorExit ();
  }
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                     "http://127.0.0.1/hello_world")) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READFUNCTION,
                                     &putBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READDATA, &pos)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_UPLOAD, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_INFILESIZE, (long) PUT_SIZE)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER, libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     (long) 150)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     (long) 150)) ||
      ((oneone) ?
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_1)) :
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_0))))
  {
    fprintf (stderr, "curl_easy_setopt() failed.\n");
    externalErrorExit ();
  }

  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 2;
  }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen ("/hello_world"));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos, cbc.buf);
    mhdErrorExitDesc ("Wrong returned data length");
  }
  return 0;
}


static int
testPutThreadPerConn (unsigned int add_flag)
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  size_t pos = 0;
  int done_flag = 0;
  CURLcode errornum;
  char buf[2048];
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1271;
    if (oneone)
      port += 10;
    if (incr_read)
      port += 20;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD
                        | MHD_USE_ERROR_LOG | add_flag,
                        port,
                        NULL, NULL, &ahc_echo, &done_flag,
                        MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                        (size_t) (incr_read ? 1024 : (PUT_SIZE * 4)),
                        MHD_OPTION_END);
  if (d == NULL)
    mhdErrorExit ();
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      mhdErrorExit ();
    port = (int) dinfo->port;
  }

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    externalErrorExit ();
  }
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                     "http://127.0.0.1/hello_world")) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READFUNCTION,
                                     &putBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READDATA, &pos)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_UPLOAD, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_INFILESIZE, (long) PUT_SIZE)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER, libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     (long) 150)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     (long) 150)) ||
      ((oneone) ?
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_1)) :
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_0))))
  {
    fprintf (stderr, "curl_easy_setopt() failed.\n");
    externalErrorExit ();
  }

  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 32;
  }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen ("/hello_world"));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos, cbc.buf);
    mhdErrorExitDesc ("Wrong returned data length");
  }
  return 0;
}


static int
testPutThreadPool (unsigned int add_flag)
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  size_t pos = 0;
  int done_flag = 0;
  CURLcode errornum;
  char buf[2048];
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1272;
    if (oneone)
      port += 10;
    if (incr_read)
      port += 20;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                        | add_flag,
                        port,
                        NULL, NULL, &ahc_echo, &done_flag,
                        MHD_OPTION_THREAD_POOL_SIZE, MHD_CPU_COUNT,
                        MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                        (size_t) (incr_read ? 1024 : (PUT_SIZE * 4)),
                        MHD_OPTION_END);
  if (d == NULL)
    mhdErrorExit ();
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      mhdErrorExit ();
    port = (int) dinfo->port;
  }

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    externalErrorExit ();
  }
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                     "http://127.0.0.1/hello_world")) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READFUNCTION,
                                     &putBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READDATA, &pos)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_UPLOAD, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_INFILESIZE, (long) PUT_SIZE)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER, libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     (long) 150)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     (long) 150)) ||
      ((oneone) ?
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_1)) :
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_0))))
  {
    fprintf (stderr, "curl_easy_setopt() failed.\n");
    externalErrorExit ();
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 32;
  }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen ("/hello_world"));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos, cbc.buf);
    mhdErrorExitDesc ("Wrong returned data length");
  }
  return 0;
}


static int
testPutExternal (void)
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  CURLM *multi;
  CURLMcode mret;
  fd_set rs;
  fd_set ws;
  fd_set es;
  int running;
  struct CURLMsg *msg;
  time_t start;
  struct timeval tv;
  size_t pos = 0;
  int done_flag = 0;
  char buf[2048];
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1273;
    if (oneone)
      port += 10;
    if (incr_read)
      port += 20;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  multi = NULL;
  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        port,
                        NULL, NULL, &ahc_echo, &done_flag,
                        MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                        (size_t) (incr_read ? 1024 : (PUT_SIZE * 4)),
                        MHD_OPTION_END);
  if (d == NULL)
    mhdErrorExit ();
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      mhdErrorExit ();
    port = (int) dinfo->port;
  }

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    externalErrorExit ();
  }
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                     "http://127.0.0.1/hello_world")) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READFUNCTION,
                                     &putBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_READDATA, &pos)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_UPLOAD, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_INFILESIZE, (long) PUT_SIZE)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER, libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     (long) 150)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     (long) 150)) ||
      ((oneone) ?
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_1)) :
       (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                      CURL_HTTP_VERSION_1_0))))
  {
    fprintf (stderr, "curl_easy_setopt() failed.\n");
    externalErrorExit ();
  }

  multi = curl_multi_init ();
  if (multi == NULL)
    libcurlErrorExit ();
  mret = curl_multi_add_handle (multi, c);
  if (mret != CURLM_OK)
    libcurlErrorExit ();

  start = time (NULL);
  while ((time (NULL) - start < 45) && (multi != NULL))
  {
    MHD_socket maxMHDsock;
    int maxcurlsock;
    maxMHDsock = MHD_INVALID_SOCKET;
    maxcurlsock = -1;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    mret = curl_multi_perform (multi, &running);
    if ((CURLM_OK != mret) && (CURLM_CALL_MULTI_PERFORM != mret))
    {
      fprintf (stderr, "curl_multi_perform() failed. Error: '%s'. ",
               curl_multi_strerror (mret));
      libcurlErrorExit ();
    }
    if (CURLM_OK != curl_multi_fdset (multi, &rs, &ws, &es, &maxcurlsock))
      libcurlErrorExitDesc ("curl_multi_fdset() failed");
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxMHDsock))
      mhdErrorExit ();

    tv.tv_sec = 0;
    tv.tv_usec = 1000;
#ifndef MHD_WINSOCK_SOCKETS
    if (maxMHDsock > maxcurlsock)
      maxcurlsock = maxMHDsock;
#endif /* MHD_WINSOCK_SOCKETS */
    if (-1 == select (maxcurlsock + 1, &rs, &ws, &es, &tv))
    {
#ifdef MHD_POSIX_SOCKETS
      if (EINTR != errno)
        externalErrorExitDesc ("Unexpected select() error");
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
        externalErrorExitDesc ("Unexpected select() error");
      Sleep (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
    }

    mret = curl_multi_perform (multi, &running);
    if ((CURLM_OK != mret) && (CURLM_CALL_MULTI_PERFORM != mret))
    {
      fprintf (stderr, "curl_multi_perform() failed. Error: '%s'. ",
               curl_multi_strerror (mret));
      libcurlErrorExit ();
    }
    if (0 == running)
    {
      int pending;
      int curl_fine = 0;
      while (NULL != (msg = curl_multi_info_read (multi, &pending)))
      {
        if (msg->msg == CURLMSG_DONE)
        {
          if (msg->data.result == CURLE_OK)
            curl_fine = 1;
          else
          {
            fprintf (stderr,
                     "curl_multi_perform() failed: '%s' ",
                     curl_easy_strerror (msg->data.result));
            libcurlErrorExit ();
          }
        }
      }
      if (! curl_fine)
      {
        fprintf (stderr, "libcurl haven't returned OK code ");
        mhdErrorExit ();
      }
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      c = NULL;
      multi = NULL;
    }
    MHD_run (d);
  }
  if (multi != NULL)
    mhdErrorExitDesc ("Request has been aborted by timeout");

  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen ("/hello_world"));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos, cbc.buf);
    mhdErrorExitDesc ("Wrong returned data length");
  }
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  unsigned int lastErr;

  oneone = has_in_name (argv[0], "11");
  incr_read = has_in_name (argv[0], "_inc");
  verbose = has_param (argc, argv, "-v");
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 99;
  put_buffer = alloc_init (PUT_SIZE);
  if (NULL == put_buffer)
    return 99;
  lastErr = testPutExternal ();
  if (verbose && (0 != lastErr))
    fprintf (stderr, "Error during testing with external select().\n");
  errorCount += lastErr;
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    lastErr = testPutInternalThread (0);
    if (verbose && (0 != lastErr) )
      fprintf (stderr,
               "Error during testing with internal thread with select().\n");
    errorCount += lastErr;
    lastErr = testPutThreadPerConn (0);
    if (verbose && (0 != lastErr) )
      fprintf (stderr,
               "Error during testing with internal thread per connection with select().\n");
    errorCount += lastErr;
    lastErr = testPutThreadPool (0);
    if (verbose && (0 != lastErr) )
      fprintf (stderr,
               "Error during testing with thread pool per connection with select().\n");
    errorCount += lastErr;
    if (MHD_is_feature_supported (MHD_FEATURE_POLL))
    {
      lastErr = testPutInternalThread (MHD_USE_POLL);
      if (verbose && (0 != lastErr) )
        fprintf (stderr,
                 "Error during testing with internal thread with poll().\n");
      errorCount += lastErr;
      lastErr = testPutThreadPerConn (MHD_USE_POLL);
      if (verbose && (0 != lastErr) )
        fprintf (stderr,
                 "Error during testing with internal thread per connection with poll().\n");
      errorCount += lastErr;
      lastErr = testPutThreadPool (MHD_USE_POLL);
      if (verbose && (0 != lastErr) )
        fprintf (stderr,
                 "Error during testing with thread pool per connection with poll().\n");
      errorCount += lastErr;
    }
    if (MHD_is_feature_supported (MHD_FEATURE_EPOLL))
    {
      lastErr = testPutInternalThread (MHD_USE_EPOLL);
      if (verbose && (0 != lastErr) )
        fprintf (stderr,
                 "Error during testing with internal thread with epoll.\n");
      errorCount += lastErr;
      lastErr = testPutThreadPool (MHD_USE_EPOLL);
      if (verbose && (0 != lastErr) )
        fprintf (stderr,
                 "Error during testing with thread pool per connection with epoll.\n");
      errorCount += lastErr;
    }
  }
  free (put_buffer);
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  else if (verbose)
    printf ("All checks passed successfully.\n");
  curl_global_cleanup ();
  return (errorCount == 0) ? 0 : 1;
}
