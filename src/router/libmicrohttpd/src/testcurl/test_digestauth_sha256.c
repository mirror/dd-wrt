/*
     This file is part of libmicrohttpd
     Copyright (C) 2010, 2018 Christian Grothoff
     Copyright (C) 2019-2022 Evgeny Grin (Karlson2k)

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
 * @file daemontest_digestauth_sha256.c
 * @brief  Testcase for libmicrohttpd Digest Auth with SHA256
 * @author Amr Ali
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#if defined(MHD_HTTPS_REQUIRE_GCRYPT) && \
  (defined(MHD_SHA256_TLSLIB) || defined(MHD_MD5_TLSLIB))
#define NEED_GCRYP_INIT 1
#include <gcrypt.h>
#endif /* MHD_HTTPS_REQUIRE_GCRYPT && (MHD_SHA256_TLSLIB || MHD_MD5_TLSLIB) */

#ifndef WINDOWS
#include <sys/socket.h>
#include <unistd.h>
#else
#include <wincrypt.h>
#endif

#define PAGE \
  "<html><head><title>libmicrohttpd demo</title></head><body>Access granted</body></html>"

#define DENIED \
  "<html><head><title>libmicrohttpd demo</title></head><body>Access denied</body></html>"

#define MY_OPAQUE "11733b200778ce33060f31c9af70a870ba96ddd4"

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};


static size_t
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


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data,
          size_t *upload_data_size,
          void **req_cls)
{
  struct MHD_Response *response;
  char *username;
  const char *password = "testpass";
  const char *realm = "test@example.com";
  enum MHD_Result ret;
  int ret_i;
  static int already_called_marker;
  (void) cls; (void) url;                         /* Unused. Silent compiler warning. */
  (void) method; (void) version; (void) upload_data; /* Unused. Silent compiler warning. */
  (void) upload_data_size; (void) req_cls;        /* Unused. Silent compiler warning. */

  if (&already_called_marker != *req_cls)
  { /* Called for the first time, request not fully read yet */
    *req_cls = &already_called_marker;
    /* Wait for complete request */
    return MHD_YES;
  }

  username = MHD_digest_auth_get_username (connection);
  if ( (username == NULL) ||
       (0 != strcmp (username, "testuser")) )
  {
    response = MHD_create_response_from_buffer_static (strlen (DENIED),
                                                       DENIED);
    ret = MHD_queue_auth_fail_response2 (connection,
                                         realm,
                                         MY_OPAQUE,
                                         response,
                                         MHD_NO,
                                         MHD_DIGEST_ALG_SHA256);
    MHD_destroy_response (response);
    return ret;
  }
  ret_i = MHD_digest_auth_check2 (connection,
                                  realm,
                                  username,
                                  password,
                                  300,
                                  MHD_DIGEST_ALG_SHA256);
  MHD_free (username);
  if (ret_i != MHD_YES)
  {
    response = MHD_create_response_from_buffer_static (strlen (DENIED),
                                                       DENIED);
    if (NULL == response)
      return MHD_NO;
    ret = MHD_queue_auth_fail_response2 (connection,
                                         realm,
                                         MY_OPAQUE,
                                         response,
                                         (MHD_INVALID_NONCE == ret_i) ?
                                         MHD_YES : MHD_NO,
                                         MHD_DIGEST_ALG_SHA256);
    MHD_destroy_response (response);
    return ret;
  }
  response = MHD_create_response_from_buffer_static (strlen (PAGE),
                                                     PAGE);
  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  return ret;
}


static unsigned int
testDigestAuth (void)
{
  CURL *c;
  CURLcode errornum;
  struct MHD_Daemon *d;
  struct CBC cbc;
  char buf[2048];
  char rnd[8];
  uint16_t port;
  char url[128];
#ifndef WINDOWS
  int fd;
  size_t len;
  size_t off = 0;
#endif /* ! WINDOWS */

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
    port = 1167;

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
#ifndef WINDOWS
  fd = open ("/dev/urandom",
             O_RDONLY);
  if (-1 == fd)
  {
    fprintf (stderr,
             "Failed to open `%s': %s\n",
             "/dev/urandom",
             strerror (errno));
    return 1;
  }
  while (off < 8)
  {
    len = (size_t) read (fd,
                         rnd + off,
                         8 - off);
    if (len == (size_t) -1)
    {
      fprintf (stderr,
               "Failed to read `%s': %s\n",
               "/dev/urandom",
               strerror (errno));
      (void) close (fd);
      return 1;
    }
    off += len;
  }
  (void) close (fd);
#else
  {
    HCRYPTPROV cc;
    BOOL b;

    b = CryptAcquireContext (&cc,
                             NULL,
                             NULL,
                             PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT);
    if (b == 0)
    {
      fprintf (stderr,
               "Failed to acquire crypto provider context: %lu\n",
               GetLastError ());
      return 1;
    }
    b = CryptGenRandom (cc, 8, (BYTE *) rnd);
    if (b == 0)
    {
      fprintf (stderr,
               "Failed to generate 8 random bytes: %lu\n",
               GetLastError ());
    }
    CryptReleaseContext (cc, 0);
    if (b == 0)
      return 1;
  }
#endif
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL,
                        &ahc_echo, NULL,
                        MHD_OPTION_DIGEST_AUTH_RANDOM, sizeof (rnd), rnd,
                        MHD_OPTION_NONCE_NC_SIZE, 300,
                        MHD_OPTION_DIGEST_AUTH_DEFAULT_MAX_NC, (uint32_t) 999,
                        MHD_OPTION_END);
  if (d == NULL)
    return 1;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;

    dinfo = MHD_get_daemon_info (d,
                                 MHD_DAEMON_INFO_BIND_PORT);
    if ( (NULL == dinfo) ||
         (0 == dinfo->port) )
    {
      MHD_stop_daemon (d);
      return 32;
    }
    port = dinfo->port;
  }
  snprintf (url,
            sizeof (url),
            "http://127.0.0.1:%u/bar%%20foo?key=value",
            (unsigned int) port);
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, url);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
  curl_easy_setopt (c, CURLOPT_USERPWD, "testuser:testpass");
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
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
  if (cbc.pos != strlen (PAGE))
    return 4;
  if (0 != strncmp (PAGE, cbc.buf, strlen (PAGE)))
    return 8;
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  curl_version_info_data *d = curl_version_info (CURLVERSION_NOW);
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
#if (LIBCURL_VERSION_MAJOR == 7) && (LIBCURL_VERSION_MINOR == 62)
  if (1)
  {
    fprintf (stderr, "libcurl version 7.62.x has bug in processing"
             "URI with GET arguments for Digest Auth.\n");
    fprintf (stderr, "This test cannot be performed.\n");
    exit (77);
  }
#endif /* libcurl version 7.62.x */

#ifdef CURL_VERSION_SSPI
  if (0 != (d->features & CURL_VERSION_SSPI))
    return 77; /* Skip test, W32 SSPI doesn't support sha256 digest */
#endif /* CURL_VERSION_SSPI */

  /* curl added SHA256 support in 7.57 = 7.0x39 */
  if (d->version_num < 0x073900)
    return 77; /* skip test, curl is too old */
#ifdef NEED_GCRYP_INIT
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif /* GCRYCTL_INITIALIZATION_FINISHED */
#endif /* NEED_GCRYP_INIT */
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  errorCount += testDigestAuth ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
