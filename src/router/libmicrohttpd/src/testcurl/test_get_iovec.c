/*
     This file is part of libmicrohttpd
     Copyright (C) 2007-2021 Christian Grothoff
     Copyright (C) 2014-2021 Evgeny Grin

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
 * @file test_get_iovec.c
 * @brief  Testcase for libmicrohttpd response from scatter/gather array
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 * @author Lawrence Sebald
 */

/*
 * This test is largely derived from the test_get_sendfile.c file, with the
 * daemon using MHD_create_response_from_iovec instead of working from an fd.
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#endif
#include "mhd_sockets.h"
#include "mhd_has_in_name.h"

#ifndef WINDOWS
#include <sys/socket.h>
#include <unistd.h>
#endif

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif

#define TESTSTR_IOVLEN 20480
#define TESTSTR_IOVCNT 20
#define TESTSTR_SIZE   (TESTSTR_IOVCNT * TESTSTR_IOVLEN)

static int oneone;

static int readbuf[TESTSTR_SIZE * 2 / sizeof(int)];

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
    _exit (7);                   /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


static void
iov_free_callback (void *cls)
{
  free (cls);
}


static void
iovncont_free_callback (void *cls)
{
  struct MHD_IoVec *iov = cls;
  unsigned int i;

  for (i = 0; i < TESTSTR_IOVCNT; ++i)
    free ((void *) iov[i].iov_base);
  free (iov);
}


static int
check_read_data (const void *ptr, size_t len)
{
  const int *buf;
  size_t i;

  if (len % sizeof(int))
    return -1;

  buf = (const int *) ptr;

  for (i = 0; i < len / sizeof(int); ++i)
  {
    if (buf[i] != (int) i)
      return -1;
  }

  return 0;
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
  int *data;
  struct MHD_IoVec iov[TESTSTR_IOVCNT];
  int i;
  (void) url; (void) version;                      /* Unused. Silent compiler warning. */
  (void) upload_data; (void) upload_data_size;     /* Unused. Silent compiler warning. */

  if (0 != strcmp (me, method))
    return MHD_NO;              /* unexpected method */
  if (&ptr != *unused)
  {
    *unused = &ptr;
    return MHD_YES;
  }
  *unused = NULL;

  /* Create some test data. */
  if (NULL == (data = malloc (TESTSTR_SIZE)))
    return MHD_NO;

  for (i = 0; i < (int) (TESTSTR_SIZE / sizeof(int)); ++i)
  {
    data[i] = i;
  }

  for (i = 0; i < TESTSTR_IOVCNT; ++i)
  {
    iov[i].iov_base = data + (i * (TESTSTR_SIZE / TESTSTR_IOVCNT
                                   / sizeof(int)));
    iov[i].iov_len = TESTSTR_SIZE / TESTSTR_IOVCNT;
  }

  response = MHD_create_response_from_iovec (iov,
                                             TESTSTR_IOVCNT,
                                             &iov_free_callback,
                                             data);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  if (ret == MHD_NO)
    abort ();
  return ret;
}


static enum MHD_Result
ncont_echo (void *cls,
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
  int *data;
  struct MHD_IoVec *iov;
  int i, j;
  (void) url; (void) version;                      /* Unused. Silent compiler warning. */
  (void) upload_data; (void) upload_data_size;     /* Unused. Silent compiler warning. */

  if (0 != strcmp (me, method))
    return MHD_NO;              /* unexpected method */
  if (&ptr != *unused)
  {
    *unused = &ptr;
    return MHD_YES;
  }
  *unused = NULL;

  if (NULL == (iov = malloc (sizeof(struct MHD_IoVec) * TESTSTR_IOVCNT)))
    return MHD_NO;

  memset (iov, 0, sizeof(struct MHD_IoVec) * TESTSTR_IOVCNT);

  /* Create some test data. */
  for (j = TESTSTR_IOVCNT - 1; j >= 0; --j)
  {
    if (NULL == (data = malloc (TESTSTR_IOVLEN)))
      goto err_out;

    iov[j].iov_base = data;
    iov[j].iov_len = TESTSTR_IOVLEN;

    for (i = 0; i < (int) (TESTSTR_IOVLEN / sizeof(int)); ++i)
    {
      data[i] = i + (j * TESTSTR_IOVLEN / sizeof(int));
    }
  }

  response = MHD_create_response_from_iovec (iov,
                                             TESTSTR_IOVCNT,
                                             &iovncont_free_callback,
                                             iov);
  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (ret == MHD_NO)
    abort ();
  return ret;

err_out:
  for (j = 0; j < TESTSTR_IOVCNT; ++j)
  {
    if (NULL != iov[j].iov_base)
      free ((void *) iov[j].iov_base);
  }
  free (iov);
  return MHD_NO;
}


static int
testInternalGet (bool contiguous)
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1200;
    if (oneone)
      port += 10;
  }

  cbc.buf = (char *) readbuf;
  cbc.size = sizeof(readbuf);
  cbc.pos = 0;

  if (contiguous)
  {
    d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                          port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
  }
  else
  {
    d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                          port, NULL, NULL, &ncont_echo, "GET", MHD_OPTION_END);
  }

  if (d == NULL)
    return 1;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = (int) dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system!*/
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
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
  if (cbc.pos != TESTSTR_SIZE)
    return 4;
  if (0 != check_read_data (cbc.buf, cbc.pos))
    return 8;
  return 0;
}


static int
testMultithreadedGet ()
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1201;
    if (oneone)
      port += 10;
  }

  cbc.buf = (char *) readbuf;
  cbc.size = sizeof(readbuf);
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                        | MHD_USE_AUTO,
                        port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
  if (d == NULL)
    return 16;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = (int) dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
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
  if (cbc.pos != TESTSTR_SIZE)
    return 64;
  if (0 != check_read_data (cbc.buf, cbc.pos))
    return 128;
  return 0;
}


static int
testMultithreadedPoolGet ()
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1202;
    if (oneone)
      port += 10;
  }

  cbc.buf = (char *) readbuf;
  cbc.size = sizeof(readbuf);
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                        | MHD_USE_AUTO,
                        port, NULL, NULL, &ahc_echo, "GET",
                        MHD_OPTION_THREAD_POOL_SIZE, MHD_CPU_COUNT,
                        MHD_OPTION_END);
  if (d == NULL)
    return 16;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = (int) dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system!*/
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
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
  if (cbc.pos != TESTSTR_SIZE)
    return 64;
  if (0 != check_read_data (cbc.buf, cbc.pos))
    return 128;
  return 0;
}


static int
testExternalGet ()
{
  struct MHD_Daemon *d;
  CURL *c;
  struct CBC cbc;
  CURLM *multi;
  CURLMcode mret;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket maxsock;
#ifdef MHD_WINSOCK_SOCKETS
  int maxposixs; /* Max socket number unused on W32 */
#else  /* MHD_POSIX_SOCKETS */
#define maxposixs maxsock
#endif /* MHD_POSIX_SOCKETS */
  int running;
  struct CURLMsg *msg;
  time_t start;
  struct timeval tv;
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1203;
    if (oneone)
      port += 10;
  }

  multi = NULL;
  cbc.buf = (char *) readbuf;
  cbc.size = sizeof(readbuf);
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
  if (d == NULL)
    return 256;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = (int) dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);


  multi = curl_multi_init ();
  if (multi == NULL)
  {
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 512;
  }
  mret = curl_multi_add_handle (multi, c);
  if (mret != CURLM_OK)
  {
    curl_multi_cleanup (multi);
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 1024;
  }
  start = time (NULL);
  while ((time (NULL) - start < 5) && (multi != NULL))
  {
    maxsock = MHD_INVALID_SOCKET;
    maxposixs = -1;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    curl_multi_perform (multi, &running);
    mret = curl_multi_fdset (multi, &rs, &ws, &es, &maxposixs);
    if (mret != CURLM_OK)
    {
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 2048;
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxsock))
    {
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      return 4096;
    }
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    if (-1 == select (maxposixs + 1, &rs, &ws, &es, &tv))
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
            curl_fine = 1;
          else
          {
            fprintf (stderr,
                     "%s failed at %s:%d: `%s'\n",
                     "curl_multi_perform",
                     __FILE__,
                     __LINE__, curl_easy_strerror (msg->data.result));
            abort ();
          }
        }
      }
      if (! curl_fine)
      {
        fprintf (stderr, "libcurl haven't returned OK code\n");
        abort ();
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
  {
    curl_multi_remove_handle (multi, c);
    curl_easy_cleanup (c);
    curl_multi_cleanup (multi);
  }
  MHD_stop_daemon (d);
  if (cbc.pos != TESTSTR_SIZE)
    return 8192;
  if (0 != check_read_data (cbc.buf, cbc.pos))
    return 16384;
  return 0;
}


static int
testUnknownPortGet ()
{
  struct MHD_Daemon *d;
  const union MHD_DaemonInfo *di;
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  int port;
  char buf[2048];

  struct sockaddr_in addr;
  socklen_t addr_len = sizeof(addr);
  memset (&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = 0;
  addr.sin_addr.s_addr = INADDR_ANY;

  cbc.buf = (char *) readbuf;
  cbc.size = sizeof(readbuf);
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        0, NULL, NULL, &ahc_echo, "GET",
                        MHD_OPTION_SOCK_ADDR, &addr,
                        MHD_OPTION_END);
  if (d == NULL)
    return 32768;

  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
  {
    di = MHD_get_daemon_info (d, MHD_DAEMON_INFO_LISTEN_FD);
    if (di == NULL)
      return 65536;

    if (0 != getsockname (di->listen_fd, (struct sockaddr *) &addr, &addr_len))
      return 131072;

    if (addr.sin_family != AF_INET)
      return 26214;
    port = (int) ntohs (addr.sin_port);
  }
  else
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = (int) dinfo->port;
  }

  snprintf (buf, sizeof(buf), "http://127.0.0.1:%d/",
            port);

  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, buf);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  /* NOTE: use of CONNECTTIMEOUT without also
     setting NOSIGNAL results in really weird
     crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 524288;
  }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != TESTSTR_SIZE)
    return 1048576;
  if (0 != check_read_data (cbc.buf, cbc.pos))
    return 2097152;
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc;   /* Unused. Silent compiler warning. */

  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  oneone = has_in_name (argv[0], "11");

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    errorCount += testInternalGet (true);
    errorCount += testInternalGet (false);
    errorCount += testMultithreadedGet ();
    errorCount += testMultithreadedPoolGet ();
    errorCount += testUnknownPortGet ();
  }
  errorCount += testExternalGet ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
