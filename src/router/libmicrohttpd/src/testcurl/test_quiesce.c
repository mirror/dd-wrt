/*
     This file is part of libmicrohttpd
     Copyright (C) 2013, 2015 Christian Grothoff
     Copyright (C) 2014-2022 Evgeny Grin (Karlson2k)

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
 * @file test_quiesce.c
 * @brief  Testcase for libmicrohttpd quiescing
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
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include "mhd_sockets.h" /* only macros used */
#include "mhd_has_in_name.h"
#include "mhd_has_param.h"


#ifndef WINDOWS
#include <unistd.h>
#include <sys/socket.h>
#endif

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif


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
#define checkCURLE_OK(libcurlcall) \
    _checkCURLE_OK_func((libcurlcall), _MHD_STRMACRO(libcurlcall), \
                        __func__, __LINE__)
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
#define checkCURLE_OK(libcurlcall) \
    _checkCURLE_OK_func((libcurlcall), _MHD_STRMACRO(libcurlcall), \
                        __FUNCTION__, __LINE__)
#else
#define externalErrorExit(ignore) _externalErrorExit_func(NULL, NULL, __LINE__)
#define externalErrorExitDesc(errDesc) \
  _externalErrorExit_func(errDesc, NULL, __LINE__)
#define libcurlErrorExit(ignore) _libcurlErrorExit_func(NULL, NULL, __LINE__)
#define libcurlErrorExitDesc(errDesc) \
  _libcurlErrorExit_func(errDesc, NULL, __LINE__)
#define mhdErrorExit(ignore) _mhdErrorExit_func(NULL, NULL, __LINE__)
#define mhdErrorExitDesc(errDesc) _mhdErrorExit_func(errDesc, NULL, __LINE__)
#define checkCURLE_OK(libcurlcall) \
  _checkCURLE_OK_func((libcurlcall), _MHD_STRMACRO(libcurlcall), NULL, __LINE__)
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


static void
_checkCURLE_OK_func (CURLcode code, const char *curlFunc,
                     const char *funcName, int lineNum)
{
  if (CURLE_OK == code)
    return;

  fflush (stdout);
  if ((NULL != curlFunc) && (0 != curlFunc[0]))
    fprintf (stderr, "'%s' resulted in '%s'", curlFunc,
             curl_easy_strerror (code));
  else
    fprintf (stderr, "libcurl function call resulted in '%s'",
             curl_easy_strerror (code));
  if ((NULL != funcName) && (0 != funcName[0]))
    fprintf (stderr, " in %s", funcName);
  if (0 < lineNum)
    fprintf (stderr, " at line %d", lineNum);

  fprintf (stderr, ".\nLast errno value: %d (%s)\n", (int) errno,
           strerror (errno));
  if (0 != libcurl_errbuf[0])
    fprintf (stderr, "Last libcurl error description: %s\n", libcurl_errbuf);

  fflush (stderr);
  exit (9);
}


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 4

#define MHD_URI_BASE_PATH "/hello_world"

/* Global parameters */
static int verbose;                 /**< Be verbose */
static int oneone;                  /**< If false use HTTP/1.0 for requests*/
static uint16_t global_port;        /**< MHD daemons listen port number */

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
    return 0;                   /* overflow */
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
          void **req_cls)
{
  static int ptr;
  struct MHD_Response *response;
  (void) cls;
  (void) version; (void) upload_data; (void) upload_data_size;       /* Unused. Silent compiler warning. */

  if (0 != strcmp (MHD_HTTP_METHOD_GET, method))
  {
    fprintf (stderr, "Unexpected HTTP method '%s'. ", method);
    externalErrorExit ();
  }
  if (&ptr != *req_cls)
  {
    *req_cls = &ptr;
    return MHD_YES;
  }
  *req_cls = NULL;
  response = MHD_create_response_from_buffer_copy (strlen (url),
                                                   (const void *) url);
  if (NULL == response)
    mhdErrorExitDesc ("MHD_create_response failed");
  /* Make sure that connection will not be reused */
  if (MHD_NO == MHD_add_response_header (response, MHD_HTTP_HEADER_CONNECTION,
                                         "close"))
    mhdErrorExitDesc ("MHD_add_response_header() failed");
  if (MHD_NO == MHD_queue_response (connection, MHD_HTTP_OK, response))
    mhdErrorExitDesc ("MHD_queue_response() failed");
  MHD_destroy_response (response);
  return MHD_YES;
}


static void
request_completed (void *cls, struct MHD_Connection *connection,
                   void **req_cls, enum MHD_RequestTerminationCode code)
{
  int *done = (int *) cls;
  (void) connection; (void) req_cls; (void) code;    /* Unused. Silent compiler warning. */
  if (MHD_REQUEST_TERMINATED_COMPLETED_OK != code)
  {
    fprintf (stderr, "Unexpected termination code: %d. ", (int) code);
    mhdErrorExit ();
  }
  *done = 1;
  if (verbose)
    printf ("Notify callback has been called with OK code.\n");
}


static void *
ServeOneRequest (void *param)
{
  struct MHD_Daemon *d;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket fd, max;
  time_t start;
  struct timeval tv;
  volatile int done = 0;

  if (NULL == param)
    externalErrorExit ();

  fd = *((MHD_socket *) param);

  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        0, NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_LISTEN_SOCKET, fd,
                        MHD_OPTION_NOTIFY_COMPLETED, &request_completed, &done,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);
  if (d == NULL)
    mhdErrorExit ();

  if (verbose)
    printf ("Started MHD daemon in ServeOneRequest().\n");

  start = time (NULL);
  while ((time (NULL) - start < TIMEOUTS_VAL * 2) && done == 0)
  {
    max = 0;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &max))
      mhdErrorExit ("MHD_get_fdset() failed");
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (-1 == MHD_SYS_select_ (max + 1, &rs, &ws, &es, &tv))
    {
#ifdef MHD_POSIX_SOCKETS
      if (EINTR != errno)
        externalErrorExitDesc ("Unexpected select() error");
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
        externalErrorExitDesc ("Unexpected select() error");
      Sleep ((DWORD) (tv.tv_sec * 1000 + tv.tv_usec / 1000));
#endif
    }
    MHD_run (d);
  }
  if (! done)
    mhdErrorExit ("ServeOneRequest() failed and finished by timeout");
  fd = MHD_quiesce_daemon (d);
  if (MHD_INVALID_SOCKET == fd)
    mhdErrorExit ("MHD_quiesce_daemon() failed in ServeOneRequest()");

  MHD_stop_daemon (d);
  return NULL;
}


static CURL *
setupCURL (void *cbc)
{
  CURL *c;

  c = curl_easy_init ();
  if (NULL == c)
    libcurlErrorExitDesc ("curl_easy_init() failed");

  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                     "http://127.0.0.1" MHD_URI_BASE_PATH)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, (long) global_port)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     (long) (TIMEOUTS_VAL / 2))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     (long) TIMEOUTS_VAL)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER,
                                     libcurl_errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                     (oneone) ?
                                     CURL_HTTP_VERSION_1_1 :
                                     CURL_HTTP_VERSION_1_0)))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");
  return c;
}


static unsigned int
testGet (unsigned int type, int pool_count, uint32_t poll_flag)
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  MHD_socket fd;
  pthread_t thrd;
  char *thrdRet;

  if (verbose)
    printf ("testGet(%u, %d, %u) test started.\n",
            type, pool_count, (unsigned int) poll_flag);

  cbc.buf = buf;
  cbc.size = sizeof(buf);
  cbc.pos = 0;
  if (pool_count > 0)
  {
    d = MHD_start_daemon (type | MHD_USE_ERROR_LOG | MHD_USE_ITC
                          | (enum MHD_FLAG) poll_flag,
                          global_port, NULL, NULL, &ahc_echo, NULL,
                          MHD_OPTION_THREAD_POOL_SIZE,
                          (unsigned int) pool_count,
                          MHD_OPTION_END);

  }
  else
  {
    d = MHD_start_daemon (type | MHD_USE_ERROR_LOG | MHD_USE_ITC
                          | (enum MHD_FLAG) poll_flag,
                          global_port, NULL, NULL, &ahc_echo, NULL,
                          MHD_OPTION_END);
  }
  if (d == NULL)
    mhdErrorExitDesc ("MHD_start_daemon() failed");
  if (0 == global_port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      mhdErrorExit ();
    global_port = dinfo->port;
  }

  c = setupCURL (&cbc);

  checkCURLE_OK (curl_easy_perform (c));

  if (cbc.pos != strlen (MHD_URI_BASE_PATH))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen (MHD_URI_BASE_PATH));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != strncmp (MHD_URI_BASE_PATH, cbc.buf, strlen (MHD_URI_BASE_PATH)))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos, cbc.buf);
    mhdErrorExitDesc ("Wrong returned data");
  }
  if (verbose)
    printf ("Received valid response data.\n");

  fd = MHD_quiesce_daemon (d);
  if (MHD_INVALID_SOCKET == fd)
    mhdErrorExitDesc ("MHD_quiesce_daemon failed");

  if (0 != pthread_create (&thrd, NULL, &ServeOneRequest,
                           (void *) &fd))
    externalErrorExitDesc ("pthread_create() failed");

  /* No need for the thread sync as socket is already listening,
   * so libcurl may start connecting before MHD is started in another thread */
  cbc.pos = 0;
  checkCURLE_OK (curl_easy_perform (c));

  if (cbc.pos != strlen (MHD_URI_BASE_PATH))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen (MHD_URI_BASE_PATH));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != strncmp (MHD_URI_BASE_PATH, cbc.buf, strlen (MHD_URI_BASE_PATH)))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos, cbc.buf);
    mhdErrorExitDesc ("Wrong returned data");
  }

  if (0 != pthread_join (thrd, (void **) &thrdRet))
    externalErrorExitDesc ("pthread_join() failed");
  if (NULL != thrdRet)
    externalErrorExitDesc ("ServeOneRequest() returned non-NULL result");

  if (verbose)
  {
    printf ("ServeOneRequest() thread was joined.\n");
    fflush (stdout);
  }

  /* at this point, the forked server quiesced and quit,
   * so new requests should fail
   */
  cbc.pos = 0;
  if (CURLE_OK == curl_easy_perform (c))
  {
    fprintf (stderr, "curl_easy_perform() succeed while it should fail. ");
    fprintf (stderr, "Got %u bytes ('%.*s'), "
             "valid data would be %u bytes (%s). ",
             (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
             (unsigned) strlen (MHD_URI_BASE_PATH), MHD_URI_BASE_PATH);
    mhdErrorExitDesc ("Unexpected succeed request");
  }
  if (verbose)
    printf ("curl_easy_perform() failed as expected.\n");
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  MHD_socket_close_chk_ (fd);

  if (verbose)
  {
    printf ("testGet(%u, %d, %u) test succeed.\n",
            type, pool_count, (unsigned int) poll_flag);
    fflush (stdout);
  }

  return 0;
}


static unsigned int
testExternalGet (void)
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
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
  int i;
  MHD_socket fd = MHD_INVALID_SOCKET;

  if (verbose)
    printf ("testExternalGet test started.\n");

  fd = MHD_INVALID_SOCKET;
  multi = NULL;
  cbc.buf = buf;
  cbc.size = sizeof(buf);
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        global_port,
                        NULL, NULL,
                        &ahc_echo, NULL,
                        MHD_OPTION_APP_FD_SETSIZE, (int) FD_SETSIZE,
                        MHD_OPTION_END);
  if (d == NULL)
    mhdErrorExitDesc ("Failed to start MHD daemon");
  if (0 == global_port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      mhdErrorExit ();
    global_port = dinfo->port;
  }

  for (i = 0; i < 2; i++)
  {
    c = setupCURL (&cbc);

    multi = curl_multi_init ();
    if (multi == NULL)
      libcurlErrorExit ();

    mret = curl_multi_add_handle (multi, c);
    if (mret != CURLM_OK)
      libcurlErrorExit ();

    start = time (NULL);
    while ( (time (NULL) - start < TIMEOUTS_VAL * 2) &&
            (NULL != multi) )
    {
      MHD_socket maxsock;
      int maxposixs;
      maxsock = MHD_INVALID_SOCKET;
      maxposixs = -1;
      FD_ZERO (&rs);
      FD_ZERO (&ws);
      FD_ZERO (&es);
      curl_multi_perform (multi, &running);
      mret = curl_multi_fdset (multi, &rs, &ws, &es, &maxposixs);
      if (mret != CURLM_OK)
        libcurlErrorExit ();
      if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxsock))
        mhdErrorExit ();
#ifndef MHD_WINSOCK_SOCKETS
      if (maxsock > maxposixs)
        maxposixs = maxsock;
#endif /* MHD_POSIX_SOCKETS */
      tv.tv_sec = 0;
      tv.tv_usec = 100000;
      if (-1 == select (maxposixs + 1, &rs, &ws, &es, &tv))
      {
#ifdef MHD_POSIX_SOCKETS
        if (EINTR != errno)
          externalErrorExitDesc ("Unexpected select() error");
#else
        if ((WSAEINVAL != WSAGetLastError ()) ||
            (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
          externalErrorExitDesc ("Unexpected select() error");
        Sleep ((DWORD) (tv.tv_sec * 1000 + tv.tv_usec / 1000));
#endif
      }
      curl_multi_perform (multi, &running);
      if (0 == running)
      {
        int pending;
        int curl_fine = 0;
        while (NULL != (msg = curl_multi_info_read (multi, &pending)))
        {
          if (msg->msg == CURLMSG_DONE)
          {
            if (msg->data.result == CURLE_OK)
            {
              curl_fine = 1;
              if (verbose)
                printf ("libcurl reported success.\n");
            }
            else if (i == 0)
            {
              fprintf (stderr,
                       "curl_multi_perform() failed with '%s'. ",
                       curl_easy_strerror (msg->data.result));
              mhdErrorExit ();
            }
          }
        }
        if (i == 0)
        {
          if (! curl_fine)
          {
            fprintf (stderr, "libcurl haven't returned OK code\n");
            mhdErrorExit ();
          }
          /* MHD is running, result should be correct */
          if (cbc.pos != strlen (MHD_URI_BASE_PATH))
          {
            fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
                     (unsigned) cbc.pos, (int) cbc.pos, cbc.buf,
                     (unsigned) strlen (MHD_URI_BASE_PATH));
            mhdErrorExitDesc ("Wrong returned data length");
          }
          if (0 != strncmp (MHD_URI_BASE_PATH, cbc.buf,
                            strlen (MHD_URI_BASE_PATH)))
          {
            fprintf (stderr, "Got invalid response '%.*s'. ", (int) cbc.pos,
                     cbc.buf);
            mhdErrorExitDesc ("Wrong returned data");
          }
          if (verbose)
          {
            printf ("First request was successful.\n");
            fflush (stdout);
          }
        }
        else if (i == 1)
        {
          if  (curl_fine)
          {
            fprintf (stderr, "libcurl returned OK code, while it shouldn't\n");
            mhdErrorExit ();
          }
          if (verbose)
            printf ("Second request failed as expected.\n");
        }
        curl_multi_remove_handle (multi, c);
        curl_multi_cleanup (multi);
        curl_easy_cleanup (c);
        c = NULL;
        multi = NULL;
        break;
      }
      MHD_run (d);
    }

    if (NULL != multi)
      mhdErrorExitDesc ("Test failed and finished by timeout");

    if (0 == i)
    {
      /* quiesce the daemon on the 1st iteration, so the 2nd should fail */
      fd = MHD_quiesce_daemon (d);
      if (MHD_INVALID_SOCKET == fd)
        mhdErrorExitDesc ("MHD_quiesce_daemon() failed");
    }
  }
  MHD_stop_daemon (d);
  if (MHD_INVALID_SOCKET == fd)
  {
    fprintf (stderr, "Failed to MHD_quiesce_daemon() at some point. ");
    externalErrorExit ();
  }
  MHD_socket_close_chk_ (fd);

  if (verbose)
  {
    printf ("testExternalGet succeed.\n");
    fflush (stdout);
  }

  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  oneone = ! has_in_name (argv[0], "10");
  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    global_port = 0;
  else
    global_port = 1480 + (oneone ? 1 : 0);

  errorCount += testExternalGet ();
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    errorCount += testGet (MHD_USE_INTERNAL_POLLING_THREAD, 0, 0);
    errorCount += testGet (MHD_USE_THREAD_PER_CONNECTION
                           | MHD_USE_INTERNAL_POLLING_THREAD, 0, 0);
    errorCount += testGet (MHD_USE_INTERNAL_POLLING_THREAD, MHD_CPU_COUNT, 0);
    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_POLL))
    {
      errorCount += testGet (MHD_USE_INTERNAL_POLLING_THREAD, 0, MHD_USE_POLL);
      errorCount += testGet (MHD_USE_THREAD_PER_CONNECTION
                             | MHD_USE_INTERNAL_POLLING_THREAD, 0,
                             MHD_USE_POLL);
      errorCount += testGet (MHD_USE_INTERNAL_POLLING_THREAD, MHD_CPU_COUNT,
                             MHD_USE_POLL);
    }
    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_EPOLL))
    {
      errorCount += testGet (MHD_USE_INTERNAL_POLLING_THREAD, 0, MHD_USE_EPOLL);
      errorCount += testGet (MHD_USE_INTERNAL_POLLING_THREAD, MHD_CPU_COUNT,
                             MHD_USE_EPOLL);
    }
  }
  if (0 != errorCount)
    fprintf (stderr,
             "Error (code: %u)\n",
             errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
