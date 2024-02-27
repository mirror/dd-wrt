/*
 This file is part of libmicrohttpd
 Copyright (C) 2007, 2016 Christian Grothoff
 Copyright (C) 2016-2021 Evgeny Grin (Karlson2k)

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
 * @file test_https_session_info.c
 * @brief  Testcase for libmicrohttpd HTTPS connection querying operations
 * @author Sagie Amir
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include "microhttpd.h"
#include <curl/curl.h>
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
#include "tls_test_common.h"
#include "tls_test_keys.h"


static int test_append_prio;

/*
 * HTTP access handler call back
 * used to query negotiated security parameters
 */
static enum MHD_Result
query_info_ahc (void *cls, struct MHD_Connection *connection,
                const char *url, const char *method,
                const char *version, const char *upload_data,
                size_t *upload_data_size, void **req_cls)
{
  struct MHD_Response *response;
  enum MHD_Result ret;
  const union MHD_ConnectionInfo *conn_info;
  enum know_gnutls_tls_id *used_tls_ver;
  (void) url; (void) method; (void) version;   /* Unused. Silent compiler warning. */
  (void) upload_data; (void) upload_data_size; /* Unused. Silent compiler warning. */
  used_tls_ver = (enum know_gnutls_tls_id *) cls;

  if (NULL == *req_cls)
  {
    *req_cls = (void *) &query_info_ahc;
    return MHD_YES;
  }

  conn_info = MHD_get_connection_info (connection,
                                       MHD_CONNECTION_INFO_PROTOCOL);
  if (NULL == conn_info)
  {
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr, "MHD_get_connection_info() failed.\n");
    fflush (stderr);
    return MHD_NO;
  }
  if (0 == (unsigned int) conn_info->protocol)
  {
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr, "MHD_get_connection_info()->protocol has "
             "wrong zero value.\n");
    fflush (stderr);
    return MHD_NO;
  }
  *used_tls_ver = (enum know_gnutls_tls_id) conn_info->protocol;

  response = MHD_create_response_from_buffer_static (strlen (EMPTY_PAGE),
                                                     EMPTY_PAGE);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


/**
 * negotiate a secure connection with server & query negotiated security parameters
 */
static unsigned int
test_query_session (enum know_gnutls_tls_id tls_ver, uint16_t *pport)
{
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  char url[256];
  enum know_gnutls_tls_id found_tls_ver;
  struct MHD_Daemon *d;

  if (NULL == (cbc.buf = malloc (sizeof (char) * 255)))
    return 99;
  cbc.size = 255;
  cbc.pos = 0;

  /* setup test */
  found_tls_ver = KNOWN_BAD;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS
                        | MHD_USE_ERROR_LOG, *pport,
                        NULL, NULL,
                        &query_info_ahc, &found_tls_ver,
                        test_append_prio ?
                        MHD_OPTION_HTTPS_PRIORITIES_APPEND :
                        MHD_OPTION_HTTPS_PRIORITIES,
                        test_append_prio ?
                        priorities_append_map[tls_ver] :
                        priorities_map[tls_ver],
                        MHD_OPTION_HTTPS_MEM_KEY, srv_self_signed_key_pem,
                        MHD_OPTION_HTTPS_MEM_CERT, srv_self_signed_cert_pem,
                        MHD_OPTION_END);

  if (d == NULL)
  {
    free (cbc.buf);
    fprintf (stderr, "MHD_start_daemon() with %s failed.\n",
             tls_names[tls_ver]);
    fflush (stderr);
    return 77;
  }
  if (0 == *pport)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d);
      free (cbc.buf);
      fprintf (stderr, "MHD_get_daemon_info() failed.\n");
      fflush (stderr);
      return 10;
    }
    *pport = dinfo->port; /* Use the same port for rest of the checks */
  }

  gen_test_uri (url,
                sizeof (url),
                *pport);
  c = curl_easy_init ();
  fflush (stderr);
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    fflush (stderr);
    MHD_stop_daemon (d);
    free (cbc.buf);
    return 99;
  }
#ifdef _DEBUG
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1L);
#endif

  if ((CURLE_OK != (errornum = curl_easy_setopt (c, CURLOPT_URL, url))) ||
      (CURLE_OK != (errornum = curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                                 CURL_HTTP_VERSION_1_1))) ||
      (CURLE_OK != (errornum = curl_easy_setopt (c, CURLOPT_TIMEOUT, 10L))) ||
      (CURLE_OK !=
       (errornum = curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 10L))) ||
      (CURLE_OK !=
       (errornum = curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer))) ||
      (CURLE_OK != (errornum = curl_easy_setopt (c, CURLOPT_WRITEDATA,
                                                 &cbc))) ||
      /* TLS options */
      /* currently skip any peer authentication */
      (CURLE_OK !=
       (errornum = curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER, 0L))) ||
      (CURLE_OK !=
       (errornum = curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0L))) ||
      (CURLE_OK !=
       (errornum = curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L))) ||
      (CURLE_OK != (errornum = curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L))))
  {
    curl_easy_cleanup (c);
    free (cbc.buf);
    MHD_stop_daemon (d);
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr, "Error setting libcurl option: %s.\n",
             curl_easy_strerror (errornum));
    fflush (stderr);
    return 99;
  }

  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    unsigned int ret;
    curl_easy_cleanup (c);
    free (cbc.buf);
    MHD_stop_daemon (d);

    fflush (stderr);
    fflush (stdout);
    if ((CURLE_SSL_CONNECT_ERROR == errornum) ||
        (CURLE_SSL_CIPHER == errornum))
    {
      ret = 77;
      fprintf (stderr, "libcurl request failed due to TLS error: '%s'\n",
               curl_easy_strerror (errornum));

    }
    else
    {
      ret = 1;
      fprintf (stderr, "curl_easy_perform failed: '%s'\n",
               curl_easy_strerror (errornum));
    }
    fflush (stderr);

    return ret;
  }

  curl_easy_cleanup (c);
  free (cbc.buf);
  MHD_stop_daemon (d);

  if (tls_ver != found_tls_ver)
  {
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr, "MHD_get_connection_info (conn, "
             "MHD_CONNECTION_INFO_PROTOCOL) returned unexpected "
             "protocol version.\n"
             "\tReturned: %s (%u)\tExpected: %s (%u)\n",
             ((unsigned int) found_tls_ver) > KNOWN_TLS_MAX ?
             "[wrong value]" : tls_names[found_tls_ver],
             (unsigned int) found_tls_ver,
             tls_names[tls_ver], (unsigned int) tls_ver);
    fflush (stderr);
    return 2;
  }
  return 0;
}


static unsigned int
test_all_supported_versions (void)
{
  enum know_gnutls_tls_id ver_for_test; /**< TLS version used for test */
  const gnutls_protocol_t *vers_list;    /**< The list of GnuTLS supported TLS versions */
  uint16_t port;
  unsigned int num_success; /**< Number of tests succeeded */
  unsigned int num_failed;  /**< Number of tests failed */

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;     /* Use system automatic assignment */
  else
    port = 3060;  /* Use predefined port, may break parallel testing of another MHD build */

  vers_list = gnutls_protocol_list ();
  if (NULL == vers_list)
  {
    fprintf (stderr, "Error getting GnuTLS supported TLS versions");
    return 99;
  }
  num_success = 0;
  num_failed = 0;

  for (ver_for_test = KNOWN_TLS_MIN; KNOWN_TLS_MAX >= ver_for_test;
       ++ver_for_test)
  {
    const gnutls_protocol_t *ver_ptr;      /**< The pointer to the position on the @a vers_list */
    unsigned int res;
    for (ver_ptr = vers_list; 0 != *ver_ptr; ++ver_ptr)
    {
      if (ver_for_test == (enum know_gnutls_tls_id) *ver_ptr)
        break;
    }
    if (0 == *ver_ptr)
    {
      printf ("%s is not supported by GnuTLS, skipping.\n\n",
              tls_names[ver_for_test]);
      fflush (stdout);
      continue;
    }
    printf ("Starting check for %s...\n",
            tls_names[ver_for_test]);
    fflush (stdout);
    res = test_query_session (ver_for_test, &port);
    fflush (stderr);
    fflush (stdout);
    if (99 == res)
    {
      fprintf (stderr, "Hard error. Test stopped.\n");
      fflush (stderr);
      return 99;
    }
    else if (77 == res)
    {
      printf ("%s does not work with libcurl client and GnuTLS "
              "server combination, skipping.\n",
              tls_names[ver_for_test]);
      fflush (stdout);
    }
    else if (0 != res)
    {
      fprintf (stderr, "Check failed for %s.\n",
               tls_names[ver_for_test]);
      fflush (stderr);
      num_failed++;
    }
    else
    {
      printf ("Check succeeded for %s.\n",
              tls_names[ver_for_test]);
      fflush (stdout);
      num_success++;
    }
    printf ("\n");
    fflush (stdout);
  }

  if (0 == num_failed)
  {
    if (0 == num_success)
    {
      fprintf (stderr, "No supported TLS version was found.\n");
      fflush (stderr);
      return 77;
    }
    return 0;
  }
  return num_failed;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  const char *ssl_version;
  (void) argc;   /* Unused. Silent compiler warning. */

#ifdef MHD_HTTPS_REQUIRE_GCRYPT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
  test_append_prio = has_in_name (argv[0], "_append");
  if (! testsuite_curl_global_init ())
    return 99;

  ssl_version = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == ssl_version)
  {
    fprintf (stderr, "Curl does not support SSL.  Cannot run the test.\n");
    curl_global_cleanup ();
    return 77;
  }
  errorCount = test_all_supported_versions ();
  fflush (stderr);
  fflush (stdout);
  curl_global_cleanup ();
  if (77 == errorCount)
    return 77;
  else if (99 == errorCount)
    return 99;
  print_test_result (errorCount, argv[0]);
  return errorCount != 0 ? 1 : 0;
}
