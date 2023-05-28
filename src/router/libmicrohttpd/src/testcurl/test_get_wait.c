/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2009, 2011 Christian Grothoff
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
 * @file test_get_wait.c
 * @brief Test 'MHD_run_wait()' function.
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
#include <pthread.h>
#include "mhd_has_in_name.h"

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif

/**
 * How many rounds of operations do we do for each
 * test.
 * Check all three types of requests for HTTP/1.1:
 * * first request, new connection;
 * * "middle" request, existing connection with stay-alive;
 * * final request, no data processed after.
 */
#define ROUNDS 3

/**
 * Do we use HTTP 1.1?
 */
static int oneone;

/**
 * Response to return (re-used).
 */
static struct MHD_Response *response;

/**
 * Set to 1 if the worker threads are done.
 */
static volatile int signal_done;


static size_t
copyBuffer (void *ptr,
            size_t size, size_t nmemb,
            void *ctx)
{
  (void) ptr; (void) ctx;          /* Unused. Silent compiler warning. */
  return size * nmemb;
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
  enum MHD_Result ret;
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
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  if (ret == MHD_NO)
    abort ();
  return ret;
}


static void *
thread_gets (void *param)
{
  CURL *c;
  CURLcode errornum;
  unsigned int i;
  char url[64];
  int port = (int) (intptr_t) param;

  snprintf (url,
            sizeof (url),
            "http://127.0.0.1:%d/hello_world",
            port);

  c = curl_easy_init ();
  if (NULL == c)
    _exit (99);
  curl_easy_setopt (c, CURLOPT_URL, url);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, NULL);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 15L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 15L);
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  for (i = 0; i < ROUNDS; i++)
  {
    if (CURLE_OK != (errornum = curl_easy_perform (c)))
    {
      signal_done = 1;
      fprintf (stderr,
               "curl_easy_perform failed: `%s'\n",
               curl_easy_strerror (errornum));
      curl_easy_cleanup (c);
      abort ();
    }
  }
  curl_easy_cleanup (c);
  signal_done = 1;

  return NULL;
}


static int
testRunWaitGet (int port, int poll_flag)
{
  pthread_t get_tid;
  struct MHD_Daemon *d;
  const char *const test_desc = ((poll_flag & MHD_USE_AUTO) ?
                                 "MHD_USE_AUTO" :
                                 (poll_flag & MHD_USE_POLL) ?
                                 "MHD_USE_POLL" :
                                 (poll_flag & MHD_USE_EPOLL) ?
                                 "MHD_USE_EPOLL" :
                                 "select()");

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;

  printf ("Starting MHD_run_wait() test with MHD in %s polling mode.\n",
          test_desc);
  signal_done = 0;
  d = MHD_start_daemon (MHD_USE_ERROR_LOG | poll_flag,
                        port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
  if (d == NULL)
    abort ();
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      abort ();
    port = (int) dinfo->port;
  }

  if (0 != pthread_create (&get_tid, NULL,
                           &thread_gets, (void *) (intptr_t) port))
    _exit (99);

  /* As another thread sets "done" flag after ending of network
   * activity, it's required to set positive timeout value for MHD_run_wait().
   * Alternatively, to use timeout value "-1" here, another thread should start
   * additional connection to wake MHD after setting "done" flag. */
  do
  {
    if (MHD_NO == MHD_run_wait (d, 50))
      abort ();
  } while (0 == signal_done);

  if (0 != pthread_join (get_tid, NULL))
    _exit (99);

  MHD_stop_daemon (d);
  printf ("Test succeeded.\n");
  return 0;
}


int
main (int argc, char *const *argv)
{
  int port = 1675;
  (void) argc;   /* Unused. Silent compiler warning. */

  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  oneone = has_in_name (argv[0], "11");
  if (oneone)
    port += 5;
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  response = MHD_create_response_from_buffer (strlen ("/hello_world"),
                                              "/hello_world",
                                              MHD_RESPMEM_MUST_COPY);
  testRunWaitGet (port++, 0);
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_EPOLL))
    testRunWaitGet (port++, MHD_USE_EPOLL);
  testRunWaitGet (port++, MHD_USE_AUTO);

  MHD_destroy_response (response);
  curl_global_cleanup ();
  return 0; /* Errors produce abort() or _exit() */
}
