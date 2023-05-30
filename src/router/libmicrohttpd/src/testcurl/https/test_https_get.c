/*
  This file is part of libmicrohttpd
  Copyright (C) 2007 Christian Grothoff

  libmicrohttpd is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published
  by the Free Software Foundation; either version 3, or (at your
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
 * @file test_https_get.c
 * @brief  Testcase for libmicrohttpd HTTPS GET operations
 * @author Sagie Amir
 */

#include "platform.h"
#include "microhttpd.h"
#include <limits.h>
#include <sys/stat.h>
#include <curl/curl.h>
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
#include "tls_test_common.h"


static int global_port;


/* perform a HTTP GET request via SSL/TLS */
static int
test_secure_get (FILE *test_fd)
{
  int ret;
  struct MHD_Daemon *d;
  int port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 3041;

  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS
                        | MHD_USE_ERROR_LOG, port,
                        NULL, NULL,
                        &http_ahc, NULL,
                        MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
                        MHD_OPTION_END);

  if (d == NULL)
  {
    fprintf (stderr, MHD_E_SERVER_INIT);
    return -1;
  }
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return -1;
    }
    port = (int) dinfo->port;
  }

  ret = test_https_transfer (test_fd,
                             port);

  MHD_stop_daemon (d);
  return ret;
}


static enum MHD_Result
ahc_empty (void *cls,
           struct MHD_Connection *connection,
           const char *url,
           const char *method,
           const char *version,
           const char *upload_data,
           size_t *upload_data_size,
           void **unused)
{
  static int ptr;
  struct MHD_Response *response;
  enum MHD_Result ret;
  (void) cls;
  (void) url;
  (void) url;
  (void) version;          /* Unused. Silent compiler warning. */
  (void) upload_data;
  (void) upload_data_size; /* Unused. Silent compiler warning. */

  if (0 != strcmp ("GET",
                   method))
    return MHD_NO;              /* unexpected method */
  if (&ptr != *unused)
  {
    *unused = &ptr;
    return MHD_YES;
  }
  *unused = NULL;
  response = MHD_create_response_from_buffer (0,
                                              NULL,
                                              MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  if (ret == MHD_NO)
  {
    fprintf (stderr, "Failed to queue response.\n");
    _exit (20);
  }
  return ret;
}


static int
curlExcessFound (CURL *c,
                 curl_infotype type,
                 char *data,
                 size_t size,
                 void *cls)
{
  static const char *excess_found = "Excess found";
  const size_t str_size = strlen (excess_found);
  (void) c;      /* Unused. Silence compiler warning. */

  if ((CURLINFO_TEXT == type)
      && (size >= str_size)
      && (0 == strncmp (excess_found, data, str_size)))
    *(int *) cls = 1;
  return 0;
}


static int
testEmptyGet (int poll_flag)
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  int excess_found = 0;


  if ( (0 == global_port) &&
       (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT)) )
  {
    global_port = 1225;

  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                        | poll_flag | MHD_USE_TLS,
                        global_port, NULL, NULL,
                        &ahc_empty, NULL,
                        MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
                        MHD_OPTION_END);
  if (d == NULL)
    return 4194304;
  if (0 == global_port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    global_port = (int) dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "https://127.0.0.1/");
  curl_easy_setopt (c, CURLOPT_PORT, (long) global_port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_DEBUGFUNCTION, &curlExcessFound);
  curl_easy_setopt (c, CURLOPT_DEBUGDATA, &excess_found);
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0L);
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
    return 8388608;
  }
  curl_easy_cleanup (c);
  MHD_stop_daemon (d);
  if (cbc.pos != 0)
    return 16777216;
  if (excess_found)
    return 33554432;
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv;   /* Unused. Silent compiler warning. */

#ifdef MHD_HTTPS_REQUIRE_GCRYPT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
  if (! testsuite_curl_global_init ())
    return 99;
  if (NULL == curl_version_info (CURLVERSION_NOW)->ssl_version)
  {
    fprintf (stderr, "Curl does not support SSL.  Cannot run the test.\n");
    curl_global_cleanup ();
    return 77;
  }

  errorCount +=
    test_secure_get (NULL);
  errorCount += testEmptyGet (0);
  curl_global_cleanup ();

  return errorCount != 0 ? 1 : 0;
}
