/*
  This file is part of libmicrohttpd
  Copyright (C) 2007-2021 Christian Grothoff
  Copyright (C) 2016-2021 Evgeny Grin

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
 * @file test_https_get_iovec.c
 * @brief  Testcase for libmicrohttpd HTTPS GET operations using an iovec
 * @author Sagie Amir
 * @author Karlson2k (Evgeny Grin)
 * @author Lawrence Sebald
 */

/*
 * This testcase is derived from the test_https_get.c testcase. This version
 * adds the usage of a scatter/gather array for storing the response data.
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

/* Use large enough pieces (>16KB) to test partially consumed
 * data as TLS doesn't take more than 16KB by a single call. */
#define TESTSTR_IOVLEN 20480
#define TESTSTR_IOVCNT 30
#define TESTSTR_SIZE   (TESTSTR_IOVCNT * TESTSTR_IOVLEN)


static void
iov_free_callback (void *cls)
{
  free (cls);
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
iovec_ahc (void *cls,
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
  int *data;
  struct MHD_IoVec iov[TESTSTR_IOVCNT];
  int i;
  int j;
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

  /* Create some test data. */
  if (NULL == (data = malloc (TESTSTR_SIZE)))
    return MHD_NO;

  for (j = 0; j < TESTSTR_IOVCNT; ++j)
  {
    /* Assign chunks of memory area in the reverse order
     * to make non-continous set of data therefore
     * possible buffer overruns could be detected */
    iov[j].iov_base = data + (((TESTSTR_IOVCNT - 1) - j)
                              * (TESTSTR_SIZE / TESTSTR_IOVCNT
                                 / sizeof(int)));
    iov[j].iov_len = TESTSTR_SIZE / TESTSTR_IOVCNT;

    for (i = 0; i < (int) (TESTSTR_IOVLEN / sizeof(int)); ++i)
      ((int *) iov[j].iov_base)[i] = i + (j * TESTSTR_IOVLEN / sizeof(int));
  }

  response = MHD_create_response_from_iovec (iov,
                                             TESTSTR_IOVCNT,
                                             &iov_free_callback,
                                             data);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


static int
test_iovec_transfer (void *cls,
                     int port)
{
  int len;
  int ret = 0;
  struct CBC cbc;
  char url[255];
  (void) cls;    /* Unused. Silent compiler warning. */

  len = TESTSTR_SIZE;
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
  if ((cbc.pos != TESTSTR_SIZE) ||
      (0 != check_read_data (cbc.buf, cbc.pos)))
  {
    fprintf (stderr, "Error: local file & received file differ.\n");
    ret = -1;
  }
cleanup:
  free (cbc.buf);
  return ret;
}


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
                        &iovec_ahc, NULL,
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

  ret = test_iovec_transfer (test_fd,
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
  struct MHD_IoVec iov;
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

  iov.iov_base = NULL;
  iov.iov_len = 0;

  response = MHD_create_response_from_iovec (&iov,
                                             1,
                                             NULL,
                                             NULL);
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
