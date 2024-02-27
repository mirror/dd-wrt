/*
  This file is part of libmicrohttpd
  Copyright (C) 2007 Christian Grothoff
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
 * @file test_https_get_parallel_threads.c
 * @brief  Testcase for libmicrohttpd HTTPS GET operations with multi-threaded
 *         MHD daemon and several clients working in parallel
 * @author Sagie Amir
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 *
 * TODO: add test for external select!
 */

#include "platform.h"
#include "microhttpd.h"
#include <sys/stat.h>
#include <limits.h>
#include <curl/curl.h>
#include <pthread.h>
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
#include "tls_test_common.h"
#include "tls_test_keys.h"

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 4
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 4
#endif

/**
 * used when spawning multiple threads executing curl server requests
 *
 */
static void *
https_transfer_thread_adapter (void *args)
{
  static int nonnull;
  struct https_test_data *cargs = args;
  unsigned int ret;

  ret = test_https_transfer (cargs->cls, cargs->port,
                             cargs->cipher_suite, cargs->proto_version);
  if (ret == 0)
    return NULL;
  return &nonnull;
}


/**
 * Test non-parallel requests.
 *
 * @return: 0 upon all client requests returning '0', 1 otherwise.
 *
 * TODO : make client_count a parameter - number of curl client threads to spawn
 */
static unsigned int
test_single_client (void *cls, uint16_t port, const char *cipher_suite,
                    int curl_proto_version)
{
  void *client_thread_ret;
  struct https_test_data client_args =
  { NULL, port, cipher_suite, curl_proto_version };
  (void) cls;    /* Unused. Silent compiler warning. */

  client_thread_ret = https_transfer_thread_adapter (&client_args);
  if (client_thread_ret != NULL)
    return 1;
  return 0;
}


/**
 * Test parallel request handling.
 *
 * @return: 0 upon all client requests returning '0', 1 otherwise.
 *
 * TODO : make client_count a parameter - number of curl client threads to spawn
 */
static unsigned int
test_parallel_clients (void *cls, uint16_t port, const char *cipher_suite,
                       int curl_proto_version)
{
  int i;
  int client_count = (MHD_CPU_COUNT - 1);
  void *client_thread_ret;
  pthread_t client_arr[client_count];
  struct https_test_data client_args =
  { NULL, port, cipher_suite, curl_proto_version };
  (void) cls;    /* Unused. Silent compiler warning. */

  for (i = 0; i < client_count; ++i)
  {
    if (pthread_create (&client_arr[i], NULL,
                        &https_transfer_thread_adapter, &client_args) != 0)
    {
      fprintf (stderr, "Error: failed to spawn test client threads.\n");

      return 1;
    }
  }

  /* check all client requests fulfilled correctly */
  for (i = 0; i < client_count; ++i)
  {
    if ((pthread_join (client_arr[i], &client_thread_ret) != 0) ||
        (client_thread_ret != NULL))
      return 1;
  }

  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  const char *ssl_version;
  uint16_t port;
  unsigned int iseed;
  (void) argc;   /* Unused. Silent compiler warning. */

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 3010;

  /* initialize random seed used by curl clients */
  iseed = (unsigned int) time (NULL);

#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
  srand (iseed);
  if (! testsuite_curl_global_init ())
    return 99;
  ssl_version = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == ssl_version)
  {
    fprintf (stderr, "Curl does not support SSL.  Cannot run the test.\n");
    curl_global_cleanup ();
    return 77;
  }

  errorCount +=
    test_wrap ("multi threaded daemon, single client", &test_single_client,
               NULL, port,
               MHD_USE_TLS | MHD_USE_ERROR_LOG | MHD_USE_THREAD_PER_CONNECTION
               | MHD_USE_INTERNAL_POLLING_THREAD,
               NULL, CURL_SSLVERSION_DEFAULT, MHD_OPTION_HTTPS_MEM_KEY,
               srv_self_signed_key_pem, MHD_OPTION_HTTPS_MEM_CERT,
               srv_self_signed_cert_pem, MHD_OPTION_END);

  errorCount +=
    test_wrap ("multi threaded daemon, parallel client",
               &test_parallel_clients, NULL, port,
               MHD_USE_TLS | MHD_USE_ERROR_LOG | MHD_USE_THREAD_PER_CONNECTION
               | MHD_USE_INTERNAL_POLLING_THREAD,
               NULL, CURL_SSLVERSION_DEFAULT, MHD_OPTION_HTTPS_MEM_KEY,
               srv_self_signed_key_pem, MHD_OPTION_HTTPS_MEM_CERT,
               srv_self_signed_cert_pem, MHD_OPTION_END);

  if (errorCount != 0)
    fprintf (stderr, "Failed test: %s, error: %u.\n", argv[0], errorCount);

  curl_global_cleanup ();
  return errorCount != 0 ? 1 : 0;
}
