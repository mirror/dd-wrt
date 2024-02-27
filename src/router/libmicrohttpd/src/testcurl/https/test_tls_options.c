/*
  This file is part of libmicrohttpd
  Copyright (C) 2007, 2016 Christian Grothoff
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
 * @file test_tls_options.c
 * @brief  Testcase for libmicrohttpd HTTPS TLS version match/mismatch
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

/*
 * HTTP access handler call back
 * used to query negotiated security parameters
 */
static enum MHD_Result
simple_ahc (void *cls, struct MHD_Connection *connection,
            const char *url, const char *method,
            const char *version, const char *upload_data,
            size_t *upload_data_size, void **req_cls)
{
  struct MHD_Response *response;
  enum MHD_Result ret;
  (void) cls; (void) url; (void) method; (void) version;   /* Unused. Silent compiler warning. */
  (void) upload_data; (void) upload_data_size; /* Unused. Silent compiler warning. */

  if (NULL == *req_cls)
  {
    *req_cls = (void *) &simple_ahc;
    return MHD_YES;
  }

  response =
    MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (EMPTY_PAGE),
                                            EMPTY_PAGE);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


enum check_result
{
  CHECK_RES_OK = 0,
  CHECK_RES_ERR = 1,

  CHECK_RES_MHD_START_FAILED = 17,
  CHECK_RES_CURL_TLS_INIT_FAIL = 18,
  CHECK_RES_CURL_TLS_CONN_FAIL = 19,

  CHECK_RES_HARD_ERROR = 99
};

static enum check_result
check_tls_match_inner (enum know_gnutls_tls_id tls_ver_mhd,
                       enum know_gnutls_tls_id tls_ver_libcurl,
                       uint16_t *pport,
                       struct MHD_Daemon **d_ptr,
                       struct CBC *pcbc,
                       CURL **c_ptr)
{
  CURLcode errornum;
  char url[256];
  int libcurl_tls_set;
  CURL *c;
  struct MHD_Daemon *d;

  /* setup test */
  d =
    MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                      | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_TLS
                      | MHD_USE_ERROR_LOG, *pport,
                      NULL, NULL,
                      &simple_ahc, NULL,
                      MHD_OPTION_HTTPS_PRIORITIES, priorities_map[tls_ver_mhd],
                      MHD_OPTION_HTTPS_MEM_KEY, srv_self_signed_key_pem,
                      MHD_OPTION_HTTPS_MEM_CERT, srv_self_signed_cert_pem,
                      MHD_OPTION_END);
  fflush (stderr);
  fflush (stdout);
  *d_ptr = d;

  if (d == NULL)
  {
    fprintf (stderr, "MHD_start_daemon() with %s failed.\n",
             tls_names[tls_ver_mhd]);
    return CHECK_RES_MHD_START_FAILED;
  }
  if (0 == *pport)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      fprintf (stderr, "MHD_get_daemon_info() failed.\n");
      return CHECK_RES_ERR;
    }
    *pport = dinfo->port; /* Use the same port for rest of the checks */
  }

  if (0 != gen_test_uri (url,
                         sizeof (url),
                         *pport))
  {
    fprintf (stderr, "failed to generate URI.\n");
    return CHECK_RES_CURL_TLS_INIT_FAIL;
  }
  c = curl_easy_init ();
  fflush (stderr);
  fflush (stdout);
  *c_ptr = c;
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed.\n");
    return CHECK_RES_HARD_ERROR;
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
                                                 pcbc))) ||
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
    fflush (stderr);
    fflush (stdout);
    fprintf (stderr, "Error setting libcurl option: %s.\n",
             curl_easy_strerror (errornum));
    return CHECK_RES_HARD_ERROR;
  }
  libcurl_tls_set = 0;
#if CURL_AT_LEAST_VERSION (7,54,0)
  if (CURL_SSLVERSION_MAX_DEFAULT !=
      libcurl_tls_max_vers_map[tls_ver_libcurl])
  {
    errornum = curl_easy_setopt (c, CURLOPT_SSLVERSION,
                                 libcurl_tls_vers_map[tls_ver_libcurl]
                                 | libcurl_tls_max_vers_map[tls_ver_libcurl]);
    if (CURLE_OK == errornum)
      libcurl_tls_set = 1;
    else
    {
      fprintf (stderr, "Error setting libcurl TLS version range: "
               "%s.\nRetrying with minimum TLS version only.\n",
               curl_easy_strerror (errornum));
    }
  }
#endif /* CURL_AT_LEAST_VERSION(7,54,0) */
  if (! libcurl_tls_set &&
      (CURLE_OK !=
       (errornum = curl_easy_setopt (c, CURLOPT_SSLVERSION,
                                     libcurl_tls_vers_map[tls_ver_libcurl]))))
  {
    fprintf (stderr, "Error setting libcurl minimum TLS version: %s.\n",
             curl_easy_strerror (errornum));
    return CHECK_RES_CURL_TLS_INIT_FAIL;
  }

  errornum = curl_easy_perform (c);
  fflush (stderr);
  fflush (stdout);
  if (CURLE_OK != errornum)
  {
    if ((CURLE_SSL_CONNECT_ERROR == errornum) ||
        (CURLE_SSL_CIPHER == errornum))
    {
      fprintf (stderr, "libcurl request failed due to TLS error: '%s'\n",
               curl_easy_strerror (errornum));
      return CHECK_RES_CURL_TLS_CONN_FAIL;

    }
    else
    {
      fprintf (stderr, "curl_easy_perform failed: '%s'\n",
               curl_easy_strerror (errornum));
      return CHECK_RES_ERR;
    }
  }
  return CHECK_RES_OK;
}


/**
 * negotiate a secure connection with server with specific TLS versions
 * set for MHD and for libcurl
 */
static enum check_result
check_tls_match (enum know_gnutls_tls_id tls_ver_mhd,
                 enum know_gnutls_tls_id tls_ver_libcurl,
                 uint16_t *pport)
{
  CURL *c;
  struct CBC cbc;
  enum check_result ret;
  struct MHD_Daemon *d;

  if (NULL == (cbc.buf = malloc (sizeof (char) * 255)))
    return CHECK_RES_HARD_ERROR;
  cbc.size = 255;
  cbc.pos = 0;

  d = NULL;
  c = NULL;
  ret = check_tls_match_inner (tls_ver_mhd, tls_ver_libcurl, pport,
                               &d, &cbc, &c);
  fflush (stderr);
  fflush (stdout);
  if (NULL != d)
    MHD_stop_daemon (d);
  if (NULL != c)
    curl_easy_cleanup (c);
  free (cbc.buf);

  return ret;
}


static unsigned int
test_first_supported_versions (void)
{
  enum know_gnutls_tls_id ver_for_check; /**< TLS version used for test */
  const gnutls_protocol_t *vers_list;    /**< The list of GnuTLS supported TLS versions */
  uint16_t port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;     /* Use system automatic assignment */
  else
    port = 3080;  /* Use predefined port, may break parallel testing of another MHD build */

  vers_list = gnutls_protocol_list ();
  if (NULL == vers_list)
  {
    fprintf (stderr, "Error getting GnuTLS supported TLS versions");
    return 99;
  }

  for (ver_for_check = KNOWN_TLS_MIN; KNOWN_TLS_MAX >= ver_for_check;
       ++ver_for_check)
  {
    const gnutls_protocol_t *ver_ptr;      /**< The pointer to the position on the @a vers_list */
    enum check_result res;
    for (ver_ptr = vers_list; 0 != *ver_ptr; ++ver_ptr)
    {
      if (ver_for_check == (enum know_gnutls_tls_id) *ver_ptr)
        break;
    }
    if (0 == *ver_ptr)
    {
      printf ("%s is not supported by GnuTLS, skipping.\n\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      continue;
    }
    if (CURL_SSLVERSION_LAST == libcurl_tls_vers_map[ver_for_check])
    {
      printf ("%s is not supported by libcurl, skipping.\n\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      continue;
    }
    /* Found some TLS version that supported by GnuTLS and should be supported
       by libcurl (but in practice support depends on used TLS library) */

    if (KNOWN_TLS_MIN != ver_for_check)
      printf ("\n");
    printf ("Starting check with MHD set to '%s' and "
            "libcurl set to '%s' (successful connection is expected)...\n",
            tls_names[ver_for_check], tls_names[ver_for_check]);
    fflush (stdout);

    /* Check with MHD and libcurl set to the same TLS version */
    res = check_tls_match (ver_for_check, ver_for_check, &port);
    if (CHECK_RES_HARD_ERROR == res)
    {
      fprintf (stderr, "Hard error. Test stopped.\n");
      fflush (stderr);
      return 99;
    }
    else if (CHECK_RES_ERR == res)
    {
      printf ("Test failed.\n");
      fflush (stdout);
      return 2;
    }
    else if (CHECK_RES_MHD_START_FAILED == res)
    {
      printf ("Skipping '%s' as MHD cannot be started with this setting.\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      continue;
    }
    else if (CHECK_RES_CURL_TLS_INIT_FAIL == res)
    {
      printf ("Skipping '%s' as libcurl rejected this setting.\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      continue;
    }
    else if (CHECK_RES_CURL_TLS_CONN_FAIL == res)
    {
      printf ("Skipping '%s' as it is not supported by current libcurl "
              "and GnuTLS combination.\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      continue;
    }
    printf ("Connection succeeded for MHD set to '%s' and "
            "libcurl set to '%s'.\n\n",
            tls_names[ver_for_check], tls_names[ver_for_check]);

    /* Check with libcurl set to the next TLS version relative to MHD setting */
    if (KNOWN_TLS_MAX == ver_for_check)
    {
      printf ("Test is incomplete as the latest known TLS version ('%s') "
              "was found as minimum working version.\nThere is no space to "
              "advance to the next version.\nAssuming that test is fine.\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      return 0;
    }
    if (CURL_SSLVERSION_LAST == libcurl_tls_vers_map[ver_for_check + 1])
    {
      printf ("Test is incomplete as '%s' is the latest version supported "
              "by libcurl.\nThere is no space to "
              "advance to the next version.\nAssuming that test is fine.\n",
              tls_names[ver_for_check]);
      fflush (stdout);
      return 0;
    }
    printf ("Starting check with MHD set to '%s' and "
            "minimum libcurl TLS version set to '%s' "
            "(failed connection is expected)...\n",
            tls_names[ver_for_check], tls_names[ver_for_check + 1]);
    fflush (stdout);
    res = check_tls_match (ver_for_check, ver_for_check + 1,
                           &port);
    if (CHECK_RES_HARD_ERROR == res)
    {
      fprintf (stderr, "Hard error. Test stopped.\n");
      fflush (stderr);
      return 99;
    }
    else if (CHECK_RES_ERR == res)
    {
      printf ("Test failed.\n");
      fflush (stdout);
      return 2;
    }
    else if (CHECK_RES_MHD_START_FAILED == res)
    {
      printf ("MHD cannot be started for the second time with "
              "the same setting.\n");
      fflush (stdout);
      return 4;
    }
    else if (CHECK_RES_CURL_TLS_INIT_FAIL == res)
    {
      printf ("'%s' has been rejected by libcurl.\n"
              "Assuming that test is fine.\n",
              tls_names[ver_for_check + 1]);
      fflush (stdout);
      return 0;
    }
    else if (CHECK_RES_CURL_TLS_CONN_FAIL == res)
    {
      printf ("As expected, libcurl cannot connect to MHD when libcurl "
              "minimum TLS version is set to '%s' while MHD TLS version set "
              "to '%s'.\n"
              "Test succeeded.\n",
              tls_names[ver_for_check + 1], tls_names[ver_for_check]);
      fflush (stdout);
      return 0;
    }
  }

  fprintf (stderr, "The test skipped: No know TLS versions are supported by "
           "both MHD and libcurl.\n");
  fflush (stderr);
  return 77;
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
  if (! testsuite_curl_global_init ())
    return 99;

  ssl_version = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == ssl_version)
  {
    fprintf (stderr, "Curl does not support SSL.  Cannot run the test.\n");
    curl_global_cleanup ();
    return 77;
  }
  errorCount = test_first_supported_versions ();
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
