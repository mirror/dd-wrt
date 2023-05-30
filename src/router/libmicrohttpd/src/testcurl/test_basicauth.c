/*
     This file is part of libmicrohttpd
     Copyright (C) 2010 Christian Grothoff
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

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
 * @file test_basicauth.c
 * @brief  Testcase for libmicrohttpd concurrent Basic Authorisation
 * @author Amr Ali
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
#include <sys/socket.h>
#include <unistd.h>
#else
#include <wincrypt.h>
#endif

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */

#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x)<<16|(y)<<8|(z))
#endif /* ! CURL_VERSION_BITS */
#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(x, y, z))
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


/* Not actually used in this test */
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


#if 0
/* Function unused in this test */
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


#endif


/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 10

#define MHD_URI_BASE_PATH "/bar%20foo%3Fkey%3Dvalue"

#define REALM "TestRealm"
#define USERNAME "Aladdin"
#define PASSWORD "open sesame"


#define PAGE \
  "<html><head><title>libmicrohttpd demo page</title>" \
  "</head><body>Access granted</body></html>"

#define DENIED \
  "<html><head><title>libmicrohttpd - Access denied</title>" \
  "</head><body>Access denied</body></html>"

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

static int verbose;

static size_t
copyBuffer (void *ptr,
            size_t size,
            size_t nmemb,
            void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    mhdErrorExitDesc ("Wrong too large data");       /* overflow */
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
          const char *upload_data,
          size_t *upload_data_size,
          void **req_cls)
{
  struct MHD_Response *response;
  char *username;
  char *password;
  enum MHD_Result ret;
  static int already_called_marker;
  (void) cls; (void) url;         /* Unused. Silent compiler warning. */
  (void) method; (void) version; (void) upload_data; /* Unused. Silent compiler warning. */
  (void) upload_data_size;        /* Unused. Silent compiler warning. */

  if (&already_called_marker != *req_cls)
  { /* Called for the first time, request not fully read yet */
    *req_cls = &already_called_marker;
    /* Wait for complete request */
    return MHD_YES;
  }

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    mhdErrorExitDesc ("Unexpected HTTP method");

  /* require: USERNAME with password PASSWORD */
  password = NULL;
  username = MHD_basic_auth_get_username_password (connection,
                                                   &password);
  if (NULL != username)
  {
    if (0 != strcmp (username, USERNAME))
    {
      fprintf (stderr, "'username' does not match.\n"
               "Expected: '%s'\tRecieved: '%s'. ", USERNAME, username);
      mhdErrorExitDesc ("Wrong 'username'");
    }
    if (NULL == password)
      mhdErrorExitDesc ("The password pointer is NULL");
    if (0 != strcmp (password, PASSWORD))
      fprintf (stderr, "'password' does not match.\n"
               "Expected: '%s'\tRecieved: '%s'. ", PASSWORD, password);
    response =
      MHD_create_response_from_buffer (MHD_STATICSTR_LEN_ (PAGE),
                                       (void *) PAGE,
                                       MHD_RESPMEM_PERSISTENT);
    if (NULL == response)
      mhdErrorExitDesc ("Response creation failed");
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    if (MHD_YES != ret)
      mhdErrorExitDesc ("'MHD_queue_response()' failed");
  }
  else
  {
    if (NULL != password)
      mhdErrorExitDesc ("The password pointer is NOT NULL");
    response =
      MHD_create_response_from_buffer (MHD_STATICSTR_LEN_ (DENIED),
                                       (void *) DENIED,
                                       MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_basic_auth_fail_response (connection, REALM,
                                              response);
  }

  if (NULL != username)
    MHD_free (username);
  if (NULL != password)
    MHD_free (password);
  MHD_destroy_response (response);
  return ret;
}


static CURL *
setupCURL (void *cbc, int port, char *errbuf)
{
  CURL *c;
  char url[512];

  if (1)
  {
    int res;
    /* A workaround for some old libcurl versions, which ignore the specified
     * port by CURLOPT_PORT when authorisation is used. */
    res = snprintf (url, (sizeof(url) / sizeof(url[0])),
                    "http://127.0.0.1:%d%s", port, MHD_URI_BASE_PATH);
    if ((0 >= res) || ((sizeof(url) / sizeof(url[0])) <= (size_t) res))
      externalErrorExitDesc ("Cannot form request URL");
  }

  c = curl_easy_init ();
  if (NULL == c)
    libcurlErrorExitDesc ("curl_easy_init() failed");

  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER,
                                     errbuf)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                     &copyBuffer)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, cbc)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                     ((long) TIMEOUTS_VAL))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                     CURL_HTTP_VERSION_1_1)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L)) ||
#if CURL_AT_LEAST_VERSION (7, 85, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS_STR, "http")) ||
#elif CURL_AT_LEAST_VERSION (7, 19, 4)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS, CURLPROTO_HTTP)) ||
#endif /* CURL_AT_LEAST_VERSION (7, 19, 4) */
#if CURL_AT_LEAST_VERSION (7, 45, 0)
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEFAULT_PROTOCOL, "http")) ||
#endif /* CURL_AT_LEAST_VERSION (7, 45, 0) */
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, ((long) port))) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL, url)))
    libcurlErrorExitDesc ("curl_easy_setopt() failed");
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPAUTH, CURLAUTH_BASIC)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_USERPWD,
                                     USERNAME ":" PASSWORD)))
    libcurlErrorExitDesc ("curl_easy_setopt() authorization options failed");
  return c;
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
check_result (CURLcode curl_code, struct CBC *pcbc)
{
  if (CURLE_OK != curl_code)
  {
    fflush (stdout);
    if (0 != libcurl_errbuf[0])
      fprintf (stderr, "First request failed. "
               "libcurl error: '%s'.\n"
               "libcurl error description: '%s'.\n",
               curl_easy_strerror (curl_code),
               libcurl_errbuf);
    else
      fprintf (stderr, "First request failed. "
               "libcurl error: '%s'.\n",
               curl_easy_strerror (curl_code));
    fflush (stderr);
    return 0;
  }

  if (pcbc->pos != strlen (PAGE))
  {
    fprintf (stderr, "Got %u bytes ('%.*s'), expected %u bytes. ",
             (unsigned) pcbc->pos, (int) pcbc->pos, pcbc->buf,
             (unsigned) strlen (PAGE));
    mhdErrorExitDesc ("Wrong returned data length");
  }
  if (0 != memcmp (PAGE, pcbc->buf, pcbc->pos))
  {
    fprintf (stderr, "Got invalid response '%.*s'. ",
             (int) pcbc->pos, pcbc->buf);
    mhdErrorExitDesc ("Wrong returned data");
  }
  return 1;
}


static unsigned int
testBasicAuth (void)
{
  struct MHD_Daemon *d;
  uint16_t port;
  struct CBC cbc;
  char buf[2048];
  CURL *c;
  CURLM *multi_reuse;
  int failed = 0;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 4210;

  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        port, NULL, NULL,
                        &ahc_echo, NULL,
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
    port = (uint16_t) dinfo->port;
  }

  /* First request */
  cbc.buf = buf;
  cbc.size = sizeof (buf);
  cbc.pos = 0;
  memset (cbc.buf, 0, cbc.size);
  c = setupCURL (&cbc, port, libcurl_errbuf);
  multi_reuse = NULL;
  if (check_result (performQueryExternal (d, c, &multi_reuse), &cbc))
  {
    if (verbose)
      printf ("First request successful.\n");
  }
  else
  {
    fprintf (stderr, "First request FAILED.\n");
    failed = 1;
  }
  curl_easy_cleanup (c);

  /* Second request */
  cbc.buf = buf;
  cbc.size = sizeof (buf);
  cbc.pos = 0;
  memset (cbc.buf, 0, cbc.size);
  c = setupCURL (&cbc, port, libcurl_errbuf);
  if (check_result (performQueryExternal (d, c, &multi_reuse), &cbc))
  {
    if (verbose)
      printf ("Second request successful.\n");
  }
  else
  {
    fprintf (stderr, "Second request FAILED.\n");
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
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */

  verbose = ! 0;

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  errorCount += testBasicAuth ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
