/*
 This file is part of libmicrohttpd
 Copyright (C) 2007 Christian Grothoff
 Copyright (C) 2017-2022 Evgeny Grin (Karlson2k)

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
 * @file tls_test_common.c
 * @brief  Common tls test functions
 * @author Sagie Amir
 * @author Karlson2k (Evgeny Grin)
 */
#include <string.h>
#include "tls_test_common.h"
#include "tls_test_keys.h"

/**
 * Map @a know_gnutls_tls_ids values to printable names.
 */
const char *tls_names[KNOW_TLS_IDS_COUNT] = {
  "Bad value",
  "SSL version 3",
  "TLS version 1.0",
  "TLS version 1.1",
  "TLS version 1.2",
  "TLS version 1.3"
};

/**
 * Map @a know_gnutls_tls_ids values to GnuTLS priorities strings.
 */
const char *priorities_map[KNOW_TLS_IDS_COUNT] = {
  "NONE",
  "NORMAL:!VERS-ALL:+VERS-SSL3.0",
  "NORMAL:!VERS-ALL:+VERS-TLS1.0",
  "NORMAL:!VERS-ALL:+VERS-TLS1.1",
  "NORMAL:!VERS-ALL:+VERS-TLS1.2",
  "NORMAL:!VERS-ALL:+VERS-TLS1.3"
};

/**
 * Map @a know_gnutls_tls_ids values to GnuTLS priorities append strings.
 */
const char *priorities_append_map[KNOW_TLS_IDS_COUNT] = {
  "NONE",
  "!VERS-ALL:+VERS-SSL3.0",
  "!VERS-ALL:+VERS-TLS1.0",
  "!VERS-ALL:+VERS-TLS1.1",
  "!VERS-ALL:+VERS-TLS1.2",
  "!VERS-ALL:+VERS-TLS1.3"
};


/**
 * Map @a know_gnutls_tls_ids values to libcurl @a CURLOPT_SSLVERSION value.
 */
const long libcurl_tls_vers_map[KNOW_TLS_IDS_COUNT] = {
  CURL_SSLVERSION_LAST, /* bad value */
  CURL_SSLVERSION_SSLv3,
#if CURL_AT_LEAST_VERSION (7,34,0)
  CURL_SSLVERSION_TLSv1_0,
#else  /* CURL VER < 7.34.0 */
  CURL_SSLVERSION_TLSv1, /* TLS 1.0 or later */
#endif /* CURL VER < 7.34.0 */
#if CURL_AT_LEAST_VERSION (7,34,0)
  CURL_SSLVERSION_TLSv1_1,
#else  /* CURL VER < 7.34.0 */
  CURL_SSLVERSION_LAST, /* bad value, not supported by this libcurl version */
#endif /* CURL VER < 7.34.0 */
#if CURL_AT_LEAST_VERSION (7,34,0)
  CURL_SSLVERSION_TLSv1_2,
#else  /* CURL VER < 7.34.0 */
  CURL_SSLVERSION_LAST, /* bad value, not supported by this libcurl version */
#endif /* CURL VER < 7.34.0 */
#if CURL_AT_LEAST_VERSION (7,52,0)
  CURL_SSLVERSION_TLSv1_3
#else  /* CURL VER < 7.34.0 */
  CURL_SSLVERSION_LAST /* bad value, not supported by this libcurl version */
#endif /* CURL VER < 7.34.0 */
};

#if CURL_AT_LEAST_VERSION (7,54,0)
/**
 * Map @a know_gnutls_tls_ids values to libcurl @a CURLOPT_SSLVERSION value
 * for maximum supported TLS version.
 */
const long libcurl_tls_max_vers_map[KNOW_TLS_IDS_COUNT]  = {
  CURL_SSLVERSION_MAX_DEFAULT, /* bad value */
  CURL_SSLVERSION_MAX_DEFAULT, /* SSLv3 */
  CURL_SSLVERSION_MAX_TLSv1_0,
  CURL_SSLVERSION_MAX_TLSv1_1,
  CURL_SSLVERSION_MAX_TLSv1_2,
  CURL_SSLVERSION_MAX_TLSv1_3
};
#endif /* CURL_AT_LEAST_VERSION(7,54,0) */

/*
 * test HTTPS transfer
 */
enum test_get_result
test_daemon_get (void *cls,
                 const char *cipher_suite,
                 int proto_version,
                 uint16_t port,
                 int ver_peer)
{
  CURL *c;
  struct CBC cbc;
  CURLcode errornum;
  CURLcode e;
  char url[255];
  size_t len;
  (void) cls;    /* Unused. Silence compiler warning. */

  len = strlen (test_data);
  if (NULL == (cbc.buf = malloc (sizeof (char) * len)))
  {
    fprintf (stderr, MHD_E_MEM);
    return TEST_GET_HARD_ERROR;
  }
  cbc.size = len;
  cbc.pos = 0;

  /* construct url - this might use doc_path */
  gen_test_uri (url,
                sizeof (url),
                port);

  c = curl_easy_init ();
#ifdef _DEBUG
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1L);
#endif
  if ((CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_URL, url))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                          CURL_HTTP_VERSION_1_1))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_TIMEOUT, 10L))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 10L))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                          &copyBuffer))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L))))
  {
    fprintf (stderr, "curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return TEST_GET_CURL_GEN_ERROR;
  }

  /* TLS options */
  if ((CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSLVERSION,
                                          proto_version))) ||
      ((NULL != cipher_suite) &&
       (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSL_CIPHER_LIST,
                                           cipher_suite)))) ||

      /* perform peer authentication */
      /* TODO merge into send_curl_req */
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER,
                                          ver_peer))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0L))))
  {
    fprintf (stderr, "HTTPS curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return TEST_GET_CURL_GEN_ERROR;
  }
  if (ver_peer &&
      (CURLE_OK !=
       (e = curl_easy_setopt (c, CURLOPT_CAINFO, ca_cert_file_name))))
  {
    fprintf (stderr, "HTTPS curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return TEST_GET_CURL_CA_ERROR;
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr, "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    free (cbc.buf);
    if ((CURLE_SSL_CACERT_BADFILE == errornum)
#if CURL_AT_LEAST_VERSION (7,21,5)
        || (CURLE_NOT_BUILT_IN == errornum)
#endif /* CURL_AT_LEAST_VERSION (7,21,5) */
        )
      return TEST_GET_CURL_CA_ERROR;
    if (CURLE_OUT_OF_MEMORY == errornum)
      return TEST_GET_HARD_ERROR;
    return TEST_GET_ERROR;
  }

  curl_easy_cleanup (c);

  if (memcmp (cbc.buf, test_data, len) != 0)
  {
    fprintf (stderr, "Error: local data & received data differ.\n");
    free (cbc.buf);
    return TEST_GET_TRANSFER_ERROR;
  }

  free (cbc.buf);
  return TEST_GET_OK;
}


void
print_test_result (unsigned int test_outcome,
                   const char *test_name)
{
  if (test_outcome != 0)
    fprintf (stderr,
             "running test: %s [fail: %u]\n",
             test_name, test_outcome);
#if 0
  else
    fprintf (stdout,
             "running test: %s [pass]\n",
             test_name);
#endif
}


size_t
copyBuffer (void *ptr,
            size_t size,
            size_t nmemb,
            void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
  {
    fprintf (stderr, "Server data does not fit buffer.\n");
    return 0;                   /* overflow */
  }
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


/**
 *  HTTP access handler call back
 */
enum MHD_Result
http_ahc (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **req_cls)
{
  static int aptr;
  struct MHD_Response *response;
  enum MHD_Result ret;
  (void) cls; (void) url; (void) version;          /* Unused. Silent compiler warning. */
  (void) upload_data; (void) upload_data_size;     /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *req_cls)
  {
    /* do never respond on first call */
    *req_cls = &aptr;
    return MHD_YES;
  }
  *req_cls = NULL;                  /* reset when done */
  response = MHD_create_response_from_buffer_static (strlen (test_data),
                                                     test_data);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


/* HTTP access handler call back */
enum MHD_Result
http_dummy_ahc (void *cls,
                struct MHD_Connection *connection,
                const char *url,
                const char *method,
                const char *version,
                const char *upload_data,
                size_t *upload_data_size,
                void **req_cls)
{
  (void) cls;
  (void) connection;
  (void) url;
  (void) method;
  (void) version;      /* Unused. Silent compiler warning. */
  (void) upload_data;
  (void) upload_data_size;
  (void) req_cls;                   /* Unused. Silent compiler warning. */
  return 0;
}


/**
 * send a test http request to the daemon
 * @param url
 * @param cbc - may be null
 * @param cipher_suite
 * @param proto_version
 * @return
 */
/* TODO have test wrap consider a NULL cbc */
CURLcode
send_curl_req (char *url,
               struct CBC *cbc,
               const char *cipher_suite,
               int proto_version)
{
  CURL *c;
  CURLcode errornum;
  CURLcode e;
  c = curl_easy_init ();
#ifdef _DEBUG
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1L);
#endif
  if ((CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_URL, url))) ||
      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                           CURL_HTTP_VERSION_1_1))) ||
      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_TIMEOUT, 60L))) ||
      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 60L))) ||

      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L))) ||

      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L))))
  {
    fprintf (stderr, "curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    return e;
  }
  if (cbc != NULL)
  {
    if ((CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                             &copyBuffer))) ||
        (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_WRITEDATA, cbc))))
    {
      fprintf (stderr, "curl_easy_setopt failed: `%s'\n",
               curl_easy_strerror (e));
      curl_easy_cleanup (c);
      return e;
    }
  }

  /* TLS options */
  if ((CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_SSLVERSION,
                                           proto_version))) ||
      ((NULL != cipher_suite) &&
       (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSL_CIPHER_LIST,
                                           cipher_suite)))) ||
      /* currently skip any peer authentication */
      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER, 0L))) ||
      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0L))))
  {
    fprintf (stderr, "HTTPS curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    return e;
  }

  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr, "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    return errornum;
  }
  curl_easy_cleanup (c);

  return CURLE_OK;
}


/**
 * compile test URI
 *
 * @param[out] uri - char buffer into which the url is compiled
 * @param uri_len number of bytes available in @a url
 * @param port port to use for the test
 * @return 1 on error
 */
unsigned int
gen_test_uri (char *uri,
              size_t uri_len,
              uint16_t port)
{
  int res;

  res = snprintf (uri,
                  uri_len,
                  "https://127.0.0.1:%u/urlpath",
                  (unsigned int) port);
  if (res <= 0)
    return 1;
  if ((size_t) res >= uri_len)
    return 1;

  return 0;
}


/**
 * test HTTPS data transfer
 */
unsigned int
test_https_transfer (void *cls,
                     uint16_t port,
                     const char *cipher_suite,
                     int proto_version)
{
  size_t len;
  unsigned int ret = 0;
  struct CBC cbc;
  char url[255];
  (void) cls;    /* Unused. Silent compiler warning. */

  len = strlen (test_data);
  if (NULL == (cbc.buf = malloc (sizeof (char) * len)))
  {
    fprintf (stderr, MHD_E_MEM);
    return 1;
  }
  cbc.size = len;
  cbc.pos = 0;

  if (gen_test_uri (url,
                    sizeof (url),
                    port))
  {
    ret = 1;
    goto cleanup;
  }

  if (CURLE_OK !=
      send_curl_req (url, &cbc, cipher_suite, proto_version))
  {
    ret = 1;
    goto cleanup;
  }

  /* compare test data & daemon response */
  if ( (len != strlen (test_data)) ||
       (memcmp (cbc.buf,
                test_data,
                len) != 0) )
  {
    fprintf (stderr, "Error: original data & received data differ.\n");
    ret = 1;
  }
cleanup:
  free (cbc.buf);
  return ret;
}


/**
 * setup test case
 *
 * @param d
 * @param daemon_flags
 * @param arg_list
 * @return port number on success or zero on failure
 */
static uint16_t
setup_testcase (struct MHD_Daemon **d, uint16_t port, unsigned int daemon_flags,
                va_list arg_list)
{
  *d = MHD_start_daemon_va (daemon_flags, port,
                            NULL, NULL, &http_ahc, NULL, arg_list);

  if (*d == NULL)
  {
    fprintf (stderr, MHD_E_SERVER_INIT);
    return 0;
  }

  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (*d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (*d);
      return 0;
    }
    port = dinfo->port;
  }

  return port;
}


static void
teardown_testcase (struct MHD_Daemon *d)
{
  MHD_stop_daemon (d);
}


unsigned int
setup_session (gnutls_session_t *session,
               gnutls_certificate_credentials_t *xcred)
{
  if (GNUTLS_E_SUCCESS == gnutls_init (session, GNUTLS_CLIENT))
  {
    if (GNUTLS_E_SUCCESS == gnutls_set_default_priority (*session))
    {
      if (GNUTLS_E_SUCCESS == gnutls_certificate_allocate_credentials (xcred))
      {
        if (GNUTLS_E_SUCCESS == gnutls_credentials_set (*session,
                                                        GNUTLS_CRD_CERTIFICATE,
                                                        *xcred))
        {
          return 0;
        }
        gnutls_certificate_free_credentials (*xcred);
      }
    }
    gnutls_deinit (*session);
  }
  return 1;
}


unsigned int
teardown_session (gnutls_session_t session,
                  gnutls_certificate_credentials_t xcred)
{
  gnutls_deinit (session);
  gnutls_certificate_free_credentials (xcred);
  return 0;
}


/* TODO test_wrap: change sig to (setup_func, test, va_list test_arg) */
unsigned int
test_wrap (const char *test_name, unsigned int
           (*test_function)(void *cls, uint16_t port, const char *cipher_suite,
                            int proto_version), void *cls,
           uint16_t port,
           unsigned int daemon_flags, const char *cipher_suite,
           int proto_version, ...)
{
  unsigned int ret;
  va_list arg_list;
  struct MHD_Daemon *d;
  (void) cls;    /* Unused. Silent compiler warning. */

  va_start (arg_list, proto_version);
  port = setup_testcase (&d, port, daemon_flags, arg_list);
  if (0 == port)
  {
    va_end (arg_list);
    fprintf (stderr, "Failed to setup testcase %s\n", test_name);
    return 1;
  }
#if 0
  fprintf (stdout, "running test: %s ", test_name);
#endif
  ret = test_function (NULL, port, cipher_suite, proto_version);
#if 0
  if (ret == 0)
  {
    fprintf (stdout, "[pass]\n");
  }
  else
  {
    fprintf (stdout, "[fail]\n");
  }
#endif
  teardown_testcase (d);
  va_end (arg_list);
  return ret;
}


static int inited_tls_is_gnutls = 0;
static int inited_tls_is_openssl = 0;

int
curl_tls_is_gnutls (void)
{
  const char *tlslib;
  if (inited_tls_is_gnutls)
    return 1;
  if (inited_tls_is_openssl)
    return 0;

  tlslib = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == tlslib)
    return 0;
  if (0 == strncmp (tlslib, "GnuTLS/", 7))
    return 1;

  /* Multi-backends handled during initialization by setting variable */
  return 0;
}


int
curl_tls_is_openssl (void)
{
  const char *tlslib;
  if (inited_tls_is_gnutls)
    return 0;
  if (inited_tls_is_openssl)
    return 1;

  tlslib = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == tlslib)
    return 0;
  if (0 == strncmp (tlslib, "OpenSSL/", 8))
    return 1;

  /* Multi-backends handled during initialization by setting variable */
  return 0;
}


int
curl_tls_is_nss (void)
{
  const char *tlslib;
  if (inited_tls_is_gnutls)
    return 0;
  if (inited_tls_is_openssl)
    return 0;

  tlslib = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == tlslib)
    return 0;
  if (0 == strncmp (tlslib, "NSS/", 4))
    return 1;

  /* Handle multi-backends with selected backend */
  if (NULL != strstr (tlslib," NSS/"))
    return 1;

  return 0;
}


int
curl_tls_is_schannel (void)
{
  const char *tlslib;
  if (inited_tls_is_gnutls)
    return 0;
  if (inited_tls_is_openssl)
    return 0;

  tlslib = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == tlslib)
    return 0;
  if ((0 == strncmp (tlslib, "Schannel", 8)) || (0 == strncmp (tlslib, "WinSSL",
                                                               6)))
    return 1;

  /* Handle multi-backends with selected backend */
  if ((NULL != strstr (tlslib," Schannel")) || (NULL != strstr (tlslib,
                                                                " WinSSL")))
    return 1;

  return 0;
}


int
curl_tls_is_sectransport (void)
{
  const char *tlslib;
  if (inited_tls_is_gnutls)
    return 0;
  if (inited_tls_is_openssl)
    return 0;

  tlslib = curl_version_info (CURLVERSION_NOW)->ssl_version;
  if (NULL == tlslib)
    return 0;
  if (0 == strncmp (tlslib, "SecureTransport", 15))
    return 1;

  /* Handle multi-backends with selected backend */
  if (NULL != strstr (tlslib," SecureTransport"))
    return 1;

  return 0;
}


int
testsuite_curl_global_init (void)
{
  CURLcode res;
#if LIBCURL_VERSION_NUM >= 0x073800
  if (CURLSSLSET_OK != curl_global_sslset (CURLSSLBACKEND_GNUTLS, NULL, NULL))
  {
    CURLsslset e;
    e = curl_global_sslset (CURLSSLBACKEND_OPENSSL, NULL, NULL);
    if (CURLSSLSET_TOO_LATE == e)
      fprintf (stderr, "WARNING: libcurl was already initialised.\n");
    else if (CURLSSLSET_OK == e)
      inited_tls_is_openssl = 1;
  }
  else
    inited_tls_is_gnutls = 1;
#endif /* LIBCURL_VERSION_NUM >= 0x07380 */
  res = curl_global_init (CURL_GLOBAL_ALL);
  if (CURLE_OK != res)
  {
    fprintf (stderr, "libcurl initialisation error: %s\n",
             curl_easy_strerror (res));
    return 0;
  }
  return 1;
}


/**
 * Check whether program name contains specific @a marker string.
 * Only last component in pathname is checked for marker presence,
 * all leading directories names (if any) are ignored. Directories
 * separators are handled correctly on both non-W32 and W32
 * platforms.
 * @param prog_name program name, may include path
 * @param marker    marker to look for.
 * @return zero if any parameter is NULL or empty string or
 *         @a prog_name ends with slash or @a marker is not found in
 *         program name, non-zero if @a maker is found in program
 *         name.
 */
int
has_in_name (const char *prog_name, const char *marker)
{
  size_t name_pos;
  size_t pos;

  if (! prog_name || ! marker || ! prog_name[0] || ! marker[0])
    return 0;

  pos = 0;
  name_pos = 0;
  while (prog_name[pos])
  {
    if ('/' == prog_name[pos])
      name_pos = pos + 1;
#if defined(_WIN32) || defined(__CYGWIN__)
    else if ('\\' == prog_name[pos])
      name_pos = pos + 1;
#endif /* _WIN32 || __CYGWIN__ */
    pos++;
  }
  if (name_pos == pos)
    return 0;
  return strstr (prog_name + name_pos, marker) != (char *) 0;
}
