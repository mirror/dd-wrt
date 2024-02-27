/*
 This file is part of libmicrohttpd
 Copyright (C) 2007 Christian Grothoff
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
 * @file test_tls_authentication.c
 * @brief  Testcase for libmicrohttpd HTTPS GET operations with CA-signed TLS server certificate
 * @author Sagie Amir
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include "microhttpd.h"
#include <curl/curl.h>
#include <limits.h>
#include <sys/stat.h>
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
#include "tls_test_common.h"
#include "tls_test_keys.h"


/* perform a HTTP GET request via SSL/TLS */
static unsigned int
test_secure_get (void *cls, const char *cipher_suite, int proto_version)
{
  enum test_get_result ret;
  struct MHD_Daemon *d;
  uint16_t port;
  (void) cls;    /* Unused. Silent compiler warning. */

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 3075;

  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS
                        | MHD_USE_ERROR_LOG, port,
                        NULL, NULL, &http_ahc, NULL,
                        MHD_OPTION_HTTPS_MEM_KEY, srv_signed_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_signed_cert_pem,
                        MHD_OPTION_END);

  if (d == NULL)
  {
    fprintf (stderr, MHD_E_SERVER_INIT);
    return 1;
  }
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d);
      return 1;
    }
    port = dinfo->port;
  }

  ret = test_daemon_get (NULL, cipher_suite, proto_version, port, 1);

  MHD_stop_daemon (d);
  if (TEST_GET_HARD_ERROR == ret)
    return 99;
  if (TEST_GET_CURL_GEN_ERROR == ret)
  {
    fprintf (stderr, "libcurl error.\nTest aborted.\n");
    return 99;
  }
  if ((TEST_GET_CURL_CA_ERROR == ret) ||
      (TEST_GET_CURL_NOT_IMPLT == ret))
  {
    fprintf (stderr, "libcurl TLS backend does not support custom CA.\n"
             "Test skipped.\n");
    return 77;
  }
  return TEST_GET_OK == ret ? 0 : 1;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount;
  (void) argc;
  (void) argv;       /* Unused. Silent compiler warning. */

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
#if ! CURL_AT_LEAST_VERSION (7,60,0)
  if (curl_tls_is_schannel ())
  {
    fprintf (stderr, "libcurl before version 7.60.0 does not support "
             "custom CA with Schannel backend.\nTest skipped.\n");
    curl_global_cleanup ();
    return 77;
  }
#endif /* ! CURL_AT_LEAST_VERSION(7,60,0) */

  errorCount =
    test_secure_get (NULL, NULL, CURL_SSLVERSION_DEFAULT);

  print_test_result (errorCount, argv[0]);

  curl_global_cleanup ();
  if (77 == errorCount)
    return 77;
  if (99 == errorCount)
    return 77;
  return errorCount != 0 ? 1 : 0;
}
