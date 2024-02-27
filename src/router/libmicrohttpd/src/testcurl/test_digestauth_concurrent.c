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
 * @file test_digestauth_concurrent.c
 * @brief  Testcase for libmicrohttpd concurrent Digest Authorisation
 * @author Amr Ali
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(MHD_HTTPS_REQUIRE_GCRYPT) && \
  (defined(MHD_SHA256_TLSLIB) || defined(MHD_MD5_TLSLIB))
#define NEED_GCRYP_INIT 1
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT && (MHD_SHA256_TLSLIB || MHD_MD5_TLSLIB) */
#include <errno.h>

#ifndef WINDOWS
#include <sys/socket.h>
#include <unistd.h>
#else
#include <wincrypt.h>
#endif

#include <pthread.h>

#include "mhd_has_param.h"

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
#define TIMEOUTS_VAL 5

#define MHD_URI_BASE_PATH "/bar%20foo?key=value"

#define PAGE \
  "<html><head><title>libmicrohttpd demo</title></head><body>Access granted</body></html>"

#define DENIED \
  "<html><head><title>libmicrohttpd demo</title></head><body>Access denied</body></html>"

#define MY_OPAQUE "11733b200778ce33060f31c9af70a870ba96ddd4"

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
  const char *password = "testpass";
  const char *realm = "test@example.com";
  enum MHD_Result ret;
  enum MHD_DigestAuthResult ret_e;
  static int already_called_marker;
  (void) cls; (void) url;                         /* Unused. Silent compiler warning. */
  (void) method; (void) version; (void) upload_data; /* Unused. Silent compiler warning. */
  (void) upload_data_size; (void) req_cls;        /* Unused. Silent compiler warning. */

  if (&already_called_marker != *req_cls)
  { /* Called for the first time, request not fully read yet */
    *req_cls = &already_called_marker;
    /* Wait for complete request */
    return MHD_YES;
  }

  username = MHD_digest_auth_get_username (connection);
  if ( (username == NULL) ||
       (0 != strcmp (username, "testuser")) )
  {
    response = MHD_create_response_from_buffer_static (strlen (DENIED),
                                                       DENIED);
    if (NULL == response)
      mhdErrorExitDesc ("MHD_create_response_from_buffer failed");
    ret = MHD_queue_auth_fail_response2 (connection,
                                         realm,
                                         MY_OPAQUE,
                                         response,
                                         MHD_NO,
                                         MHD_DIGEST_ALG_MD5);
    if (MHD_YES != ret)
      mhdErrorExitDesc ("MHD_queue_auth_fail_response2 failed");
    MHD_destroy_response (response);
    return ret;
  }
  ret_e = MHD_digest_auth_check3 (connection,
                                  realm,
                                  username,
                                  password,
                                  50 * TIMEOUTS_VAL,
                                  0, MHD_DIGEST_AUTH_MULT_QOP_AUTH,
                                  MHD_DIGEST_AUTH_MULT_ALGO3_MD5);
  MHD_free (username);
  if (ret_e != MHD_DAUTH_OK)
  {
    response = MHD_create_response_from_buffer_static (strlen (DENIED),
                                                       DENIED);
    if (NULL == response)
      mhdErrorExitDesc ("MHD_create_response_from_buffer() failed");
    ret = MHD_queue_auth_fail_response2 (connection,
                                         realm,
                                         MY_OPAQUE,
                                         response,
                                         (MHD_DAUTH_NONCE_STALE == ret_e) ?
                                         MHD_YES : MHD_NO,
                                         MHD_DIGEST_ALG_MD5);
    if (MHD_YES != ret)
      mhdErrorExitDesc ("MHD_queue_auth_fail_response2() failed");
    MHD_destroy_response (response);
    return ret;
  }
  response = MHD_create_response_from_buffer_static (strlen (PAGE),
                                                     PAGE);
  if (NULL == response)
    mhdErrorExitDesc ("MHD_create_response_from_buffer() failed");
  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  if (MHD_YES != ret)
    mhdErrorExitDesc ("MHD_queue_auth_fail_response2() failed");
  MHD_destroy_response (response);
  return ret;
}


static CURL *
setupCURL (void *cbc, uint16_t port, char *errbuf)
{
  CURL *c;
  char url[512];

  if (1)
  {
    int res;
    /* A workaround for some old libcurl versions, which ignore the specified
     * port by CURLOPT_PORT when digest authorisation is used. */
    res = snprintf (url, (sizeof(url) / sizeof(url[0])),
                    "http://127.0.0.1:%u%s", (unsigned int) port,
                    MHD_URI_BASE_PATH);
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
#ifdef _DEBUG
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_VERBOSE, 1L)) ||
#endif /* _DEBUG */
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
  if ((CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST)) ||
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_USERPWD,
                                     "testuser:testpass")))
    libcurlErrorExitDesc ("curl_easy_setopt() authorization options failed");
  return c;
}


static void
getRnd (void *buf, size_t size)
{
#ifndef WINDOWS
  int fd;
  size_t off = 0;

  fd = open ("/dev/urandom",
             O_RDONLY);
  if (-1 == fd)
    externalErrorExitDesc ("Failed to open '/dev/urandom'");

  do
  {
    ssize_t res;
    res = read (fd, ((uint8_t *) buf) + off, size - off);
    if (0 > res)
      externalErrorExitDesc ("Failed to read '/dev/urandom'");
    off += (size_t) res;
  } while (off < size);
  (void) close (fd);
#else
  HCRYPTPROV cc;
  BOOL b;

  b = CryptAcquireContext (&cc,
                           NULL,
                           NULL,
                           PROV_RSA_FULL,
                           CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
  if (b == 0)
    externalErrorExitDesc ("CryptAcquireContext() failed");
  b = CryptGenRandom (cc, (DWORD) size, (BYTE *) buf);
  if (b == 0)
    externalErrorExitDesc ("CryptGenRandom() failed");
  CryptReleaseContext (cc, 0);
#endif /* ! WINDOWS */
}


struct curlWokerInfo
{
  int workerNumber;
  struct CBC cbc;
  pthread_t tid;
  /**
   * The libcurl handle to run in thread
   */
  CURL *c;
  char *libcurl_errbuf;
  /**
   * Non-zero if worker is finished
   */
  volatile int finished;
  /**
   * The number of successful worker results
   */
  volatile unsigned int success;
};


static void *
worker_func (void *param)
{
  struct curlWokerInfo *const w = (struct curlWokerInfo *) param;
  CURLcode req_result;
  if (NULL == w)
    externalErrorExit ();

  req_result = curl_easy_perform (w->c);
  if (CURLE_OK != req_result)
  {
    if (0 != w->libcurl_errbuf[0])
      fprintf (stderr, "Worker %d: first request failed. "
               "libcurl error: '%s'.\n"
               "libcurl error description: '%s'.\n",
               w->workerNumber, curl_easy_strerror (req_result),
               w->libcurl_errbuf);
    else
      fprintf (stderr, "Worker %d: first request failed. "
               "libcurl error: '%s'.\n",
               w->workerNumber, curl_easy_strerror (req_result));
  }
  else
  {
    if (w->cbc.pos != strlen (PAGE))
    {
      fprintf (stderr, "Worker %d: Got %u bytes ('%.*s'), expected %u bytes. ",
               w->workerNumber,
               (unsigned) w->cbc.pos, (int) w->cbc.pos, w->cbc.buf,
               (unsigned) strlen (MHD_URI_BASE_PATH));
      mhdErrorExitDesc ("Wrong returned data length");
    }
    if (0 != strncmp (PAGE, w->cbc.buf, strlen (PAGE)))
    {
      fprintf (stderr, "Worker %d: Got invalid response '%.*s'. ",
               w->workerNumber,
               (int) w->cbc.pos, w->cbc.buf);
      mhdErrorExitDesc ("Wrong returned data");
    }
    if (verbose)
      printf ("Worker %d: first request successful.\n", w->workerNumber);
    w->success++;
  }
#ifdef _DEBUG
  fflush (stderr);
  fflush (stdout);
#endif /* _DEBUG */

  /* Second request */
  w->cbc.pos = 0;
  req_result = curl_easy_perform (w->c);
  if (CURLE_OK != req_result)
  {
    if (0 != w->libcurl_errbuf[0])
      fprintf (stderr, "Worker %d: second request failed. "
               "libcurl error: '%s'.\n"
               "libcurl error description: '%s'.\n",
               w->workerNumber, curl_easy_strerror (req_result),
               w->libcurl_errbuf);
    else
      fprintf (stderr, "Worker %d: second request failed. "
               "libcurl error: '%s'.\n",
               w->workerNumber, curl_easy_strerror (req_result));
  }
  else
  {
    if (w->cbc.pos != strlen (PAGE))
    {
      fprintf (stderr, "Worker %d: Got %u bytes ('%.*s'), expected %u bytes. ",
               w->workerNumber,
               (unsigned) w->cbc.pos, (int) w->cbc.pos, w->cbc.buf,
               (unsigned) strlen (MHD_URI_BASE_PATH));
      mhdErrorExitDesc ("Wrong returned data length");
    }
    if (0 != strncmp (PAGE, w->cbc.buf, strlen (PAGE)))
    {
      fprintf (stderr, "Worker %d: Got invalid response '%.*s'. ",
               w->workerNumber,
               (int) w->cbc.pos, w->cbc.buf);
      mhdErrorExitDesc ("Wrong returned data");
    }
    if (verbose)
      printf ("Worker %d: second request successful.\n", w->workerNumber);
    w->success++;
  }
#ifdef _DEBUG
  fflush (stderr);
  fflush (stdout);
#endif /* _DEBUG */

  w->finished = ! 0;
  return NULL;
}


#define CLIENT_BUF_SIZE 2048

static unsigned int
testDigestAuth (void)
{
  struct MHD_Daemon *d;
  char rnd[8];
  uint16_t port;
  size_t i;
  /* Run three workers in parallel so at least two workers would start within
   * the same monotonic clock second.*/
  struct curlWokerInfo workers[3];
  unsigned int ret;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 4200;

  getRnd (rnd, sizeof(rnd));

  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL,
                        &ahc_echo, NULL,
                        MHD_OPTION_DIGEST_AUTH_RANDOM, sizeof (rnd), rnd,
                        MHD_OPTION_NONCE_NC_SIZE, 300,
                        MHD_OPTION_THREAD_POOL_SIZE,
                        (unsigned int) (sizeof(workers) / sizeof(workers[0])),
                        MHD_OPTION_DIGEST_AUTH_DEFAULT_MAX_NC, (uint32_t) 999,
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

  /* Initialise all workers */
  for (i = 0; i < sizeof(workers) / sizeof(workers[0]); i++)
  {
    struct curlWokerInfo *const w = workers + i;
    w->workerNumber = (int) i + 1; /* Use 1-based numbering */
    w->cbc.buf = malloc (CLIENT_BUF_SIZE);
    if (NULL == w->cbc.buf)
      externalErrorExitDesc ("malloc() failed");
    w->cbc.size = CLIENT_BUF_SIZE;
    w->cbc.pos = 0;
    w->libcurl_errbuf = malloc (CURL_ERROR_SIZE);
    if (NULL == w->libcurl_errbuf)
      externalErrorExitDesc ("malloc() failed");
    w->libcurl_errbuf[0] = 0;
    w->c = setupCURL (&w->cbc, port, w->libcurl_errbuf);
    w->finished = 0;
    w->success = 0;
  }

  /* Fire already initialised workers */
  for (i = 0; i < sizeof(workers) / sizeof(workers[0]); i++)
  {
    struct curlWokerInfo *const w = workers + i;
    if (0 != pthread_create (&w->tid, NULL, &worker_func, w))
      externalErrorExitDesc ("pthread_create() failed");
  }

  /* Collect results, cleanup workers */
  ret = 0;
  for (i = 0; i < sizeof(workers) / sizeof(workers[0]); i++)
  {
    struct curlWokerInfo *const w = workers + i;
    if (0 != pthread_join (w->tid, NULL))
      externalErrorExitDesc ("pthread_join() failed");
    curl_easy_cleanup (w->c);
    free (w->libcurl_errbuf);
    free (w->cbc.buf);
    if (! w->finished)
      externalErrorExitDesc ("The worker thread did't signal 'finished' state");
    ret += 2 - w->success;
  }

  MHD_stop_daemon (d);
  return ret;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
#if (LIBCURL_VERSION_MAJOR == 7) && (LIBCURL_VERSION_MINOR == 62)
  if (1)
  {
    fprintf (stderr, "libcurl version 7.62.x has bug in processing "
             "URI with GET arguments for Digest Auth.\n");
    fprintf (stderr, "This test cannot be performed.\n");
    exit (77);
  }
#endif /* libcurl version 7.62.x */

  verbose = ! (has_param (argc, argv, "-q") ||
               has_param (argc, argv, "--quiet") ||
               has_param (argc, argv, "-s") ||
               has_param (argc, argv, "--silent"));

#ifdef NEED_GCRYP_INIT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif /* GCRYCTL_INITIALIZATION_FINISHED */
#endif /* NEED_GCRYP_INIT */
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  errorCount += testDigestAuth ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
