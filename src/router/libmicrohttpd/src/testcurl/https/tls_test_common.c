/*
 This file is part of libmicrohttpd
 Copyright (C) 2007 Christian Grothoff

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
 */
#include "tls_test_common.h"
#include "tls_test_keys.h"


FILE *
setup_ca_cert ()
{
  FILE *cert_fd;

  if (NULL == (cert_fd = fopen (ca_cert_file_name, "wb+")))
  {
    fprintf (stderr, "Error: failed to open `%s': %s\n",
             ca_cert_file_name, strerror (errno));
    return NULL;
  }
  if (fwrite (ca_cert_pem, sizeof (char), strlen (ca_cert_pem) + 1, cert_fd)
      != strlen (ca_cert_pem) + 1)
  {
    fprintf (stderr, "Error: failed to write `%s. %s'\n",
             ca_cert_file_name, strerror (errno));
    fclose (cert_fd);
    return NULL;
  }
  if (fflush (cert_fd))
  {
    fprintf (stderr, "Error: failed to flush ca cert file stream. %s\n",
             strerror (errno));
    fclose (cert_fd);
    return NULL;
  }
  return cert_fd;
}


/*
 * test HTTPS transfer
 */
int
test_daemon_get (void *cls,
                 int port,
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
    return -1;
  }
  cbc.size = len;
  cbc.pos = 0;

  /* construct url - this might use doc_path */
  gen_test_file_url (url,
                     sizeof (url),
                     port);

  c = curl_easy_init ();
#if DEBUG_HTTPS_TEST
  curl_easy_setopt (c, CURLOPT_VERBOSE, 1L);
#endif
  if ((CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_URL, url))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                          CURL_HTTP_VERSION_1_0))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_TIMEOUT, 10L))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 10L))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                          &copyBuffer))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_FILE, &cbc))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L))))
  {
    fprintf (stderr, "curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return e;
  }

  /* TLS options */
  if ((CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSL_VERIFYPEER,
                                          ver_peer))) ||
      (CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_SSL_VERIFYHOST, 0L))))
  {
    fprintf (stderr, "HTTPS curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return e;
  }
  if (ver_peer &&
      (CURLE_OK != curl_easy_setopt (c, CURLOPT_CAINFO, ca_cert_file_name)))
  {
    fprintf (stderr, "HTTPS curl_easy_setopt failed: `%s'\n",
             curl_easy_strerror (e));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return e;
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr, "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    free (cbc.buf);
    return errornum;
  }

  curl_easy_cleanup (c);

  if (memcmp (cbc.buf, test_data, len) != 0)
  {
    fprintf (stderr, "Error: local file & received file differ.\n");
    free (cbc.buf);
    return -1;
  }

  free (cbc.buf);
  return 0;
}


void
print_test_result (int test_outcome,
                   char *test_name)
{
  if (test_outcome != 0)
    fprintf (stderr,
             "running test: %s [fail: %u]\n",
             test_name, (unsigned
                         int)
             test_outcome);
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
    return 0;                   /* overflow */
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
          void **ptr)
{
  static int aptr;
  struct MHD_Response *response;
  enum MHD_Result ret;
  (void) cls; (void) url; (void) version;          /* Unused. Silent compiler warning. */
  (void) upload_data; (void) upload_data_size;     /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, MHD_HTTP_METHOD_GET))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
  {
    /* do never respond on first call */
    *ptr = &aptr;
    return MHD_YES;
  }
  *ptr = NULL;                  /* reset when done */
  response = MHD_create_response_from_buffer (strlen (test_data),
                                              (void *) test_data,
                                              MHD_RESPMEM_PERSISTENT);
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
                void **ptr)
{
  (void) cls;
  (void) connection;
  (void) url;
  (void) method;
  (void) version;      /* Unused. Silent compiler warning. */
  (void) upload_data;
  (void) upload_data_size;
  (void) ptr;                   /* Unused. Silent compiler warning. */
  return 0;
}


/**
 * send a test http request to the daemon
 * @param url
 * @param cbc - may be null
 * @return
 */
/* TODO have test wrap consider a NULL cbc */
int
send_curl_req (char *url,
               struct CBC *cbc)
{
  CURL *c;
  CURLcode errornum;
  CURLcode e;
  c = curl_easy_init ();
#if DEBUG_HTTPS_TEST
  curl_easy_setopt (c, CURLOPT_VERBOSE, CURL_VERBOS_LEVEL);
#endif
  if ((CURLE_OK != (e = curl_easy_setopt (c, CURLOPT_URL, url))) ||
      (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                           CURL_HTTP_VERSION_1_0))) ||
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
        (CURLE_OK  != (e = curl_easy_setopt (c, CURLOPT_FILE, cbc))))
    {
      fprintf (stderr, "curl_easy_setopt failed: `%s'\n",
               curl_easy_strerror (e));
      curl_easy_cleanup (c);
      return e;
    }
  }

  /* TLS options */
  if (/* currently skip any peer authentication */
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
 * compile test file url pointing to the current running directory path
 *
 * @param[out] url - char buffer into which the url is compiled
 * @param url_len number of bytes available in url
 * @param port port to use for the test
 * @return -1 on error
 */
int
gen_test_file_url (char *url,
                   size_t url_len,
                   int port)
{
  int ret = 0;
  char *doc_path;
  size_t doc_path_len;
  /* setup test file path, url */
#ifdef PATH_MAX
  doc_path_len = PATH_MAX > 4096 ? 4096 : PATH_MAX;
#else  /* ! PATH_MAX */
  doc_path_len = 4096;
#endif /* ! PATH_MAX */
#ifdef WINDOWS
  size_t i;
#endif /* ! WINDOWS */
  if (NULL == (doc_path = malloc (doc_path_len)))
  {
    fprintf (stderr, MHD_E_MEM);
    return -1;
  }
  if (NULL == getcwd (doc_path, doc_path_len))
  {
    fprintf (stderr,
             "Error: failed to get working directory. %s\n",
             strerror (errno));
    free (doc_path);
    return -1;
  }
#ifdef WINDOWS
  for (i = 0; i < doc_path_len; i++)
  {
    if (doc_path[i] == 0)
      break;
    if (doc_path[i] == '\\')
    {
      doc_path[i] = '/';
    }
    if (doc_path[i] != ':')
      continue;
    if (i == 0)
      break;
    doc_path[i] = doc_path[i - 1];
    doc_path[i - 1] = '/';
  }
#endif
  /* construct url */
  if (snprintf (url,
                url_len,
                "%s:%d%s/%s",
                "https://127.0.0.1",
                port,
                doc_path,
                "urlpath") >= (long long) url_len)
    ret = -1;

  free (doc_path);
  return ret;
}


/**
 * test HTTPS file transfer
 */
int
test_https_transfer (void *cls,
                     int port)
{
  int len;
  int ret = 0;
  struct CBC cbc;
  char url[255];
  (void) cls;    /* Unused. Silent compiler warning. */

  len = strlen (test_data);
  if (NULL == (cbc.buf = malloc (sizeof (char) * len)))
  {
    fprintf (stderr, MHD_E_MEM);
    return -1;
  }
  cbc.size = len;
  cbc.pos = 0;

  if (gen_test_file_url (url,
                         sizeof (url),
                         port))
  {
    ret = -1;
    goto cleanup;
  }

  if (CURLE_OK !=
      send_curl_req (url, &cbc))
  {
    ret = -1;
    goto cleanup;
  }

  /* compare test file & daemon response */
  if ( (len != strlen (test_data)) ||
       (memcmp (cbc.buf,
                test_data,
                len) != 0) )
  {
    fprintf (stderr, "Error: local file & received file differ.\n");
    ret = -1;
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
int
setup_testcase (struct MHD_Daemon **d, int port, int daemon_flags, va_list
                arg_list)
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
    port = (int) dinfo->port;
  }

  return port;
}


void
teardown_testcase (struct MHD_Daemon *d)
{
  MHD_stop_daemon (d);
}


int
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
  return -1;
}


int
teardown_session (gnutls_session_t session,
                  gnutls_certificate_credentials_t xcred)
{
  gnutls_deinit (session);
  gnutls_certificate_free_credentials (xcred);
  return 0;
}


/* TODO test_wrap: change sig to (setup_func, test, va_list test_arg) */
int
test_wrap (const char *test_name, int
           (*test_function)(void *cls, int port), void *cls,
           int port,
           int daemon_flags, ...)
{
  int ret;
  va_list arg_list;
  struct MHD_Daemon *d;
  (void) cls;    /* Unused. Silent compiler warning. */

  va_start (arg_list, daemon_flags);
  port = setup_testcase (&d, port, daemon_flags, arg_list);
  if (0 == port)
  {
    va_end (arg_list);
    fprintf (stderr, "Failed to setup testcase %s\n", test_name);
    return -1;
  }
#if 0
  fprintf (stdout, "running test: %s ", test_name);
#endif
  ret = test_function (NULL, port);
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
    fprintf (stderr, "libcurl initialisation error: %s\n", curl_easy_strerror (
               res));
    return 0;
  }
  return 1;
}
