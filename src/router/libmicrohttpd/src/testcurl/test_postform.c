/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Christian Grothoff
     Copyright (C) 2014-2023 Evgeny Grin (Karlson2k)

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
 * @file test_postform.c
 * @brief  Testcase for libmicrohttpd POST operations using multipart/postform data
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
#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#ifdef HAVE_GCRYPT_H
#include <gcrypt.h>
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */

#ifndef WINDOWS
#include <unistd.h>
#endif

#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x) << 16 | (y) << 8 | (z))
#endif /* ! CURL_VERSION_BITS */
#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS (x, y, z))
#endif /* ! CURL_AT_LEAST_VERSION */

#if CURL_AT_LEAST_VERSION (7,56,0)
#define HAS_CURL_MIME 1
#endif /* CURL_AT_LEAST_VERSION(7,56,0) */

#include "mhd_has_in_name.h"

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif

static int oneone;

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};


static void
completed_cb (void *cls,
              struct MHD_Connection *connection,
              void **con_cls,
              enum MHD_RequestTerminationCode toe)
{
  struct MHD_PostProcessor *pp = *con_cls;
  (void) cls; (void) connection; (void) toe;            /* Unused. Silent compiler warning. */

  if (NULL != pp)
    MHD_destroy_post_processor (pp);
  *con_cls = NULL;
}


static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->pos + size * nmemb > cbc->size)
    return 0;                   /* overflow */
  memcpy (&cbc->buf[cbc->pos], ptr, size * nmemb);
  cbc->pos += size * nmemb;
  return size * nmemb;
}


/**
 * Note that this post_iterator is not perfect
 * in that it fails to support incremental processing.
 * (to be fixed in the future)
 */
static enum MHD_Result
post_iterator (void *cls,
               enum MHD_ValueKind kind,
               const char *key,
               const char *filename,
               const char *content_type,
               const char *transfer_encoding,
               const char *value, uint64_t off, size_t size)
{
  int *eok = cls;
  (void) kind; (void) filename; (void) content_type; /* Unused. Silent compiler warning. */
  (void) transfer_encoding; (void) off;            /* Unused. Silent compiler warning. */

#if 0
  fprintf (stderr, "PI sees %s-%.*s\n", key, size, value);
#endif
  if ((0 == strcmp (key, "name")) &&
      (size == strlen ("daniel")) && (0 == strncmp (value, "daniel", size)))
    (*eok) |= 1;
  if ((0 == strcmp (key, "project")) &&
      (size == strlen ("curl")) && (0 == strncmp (value, "curl", size)))
    (*eok) |= 2;
  return MHD_YES;
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
  static int eok;
  struct MHD_Response *response;
  struct MHD_PostProcessor *pp;
  enum MHD_Result ret;
  (void) cls; (void) version;      /* Unused. Silent compiler warning. */

  if (0 != strcmp ("POST", method))
  {
    printf ("METHOD: %s\n", method);
    return MHD_NO;              /* unexpected method */
  }
  pp = *unused;
  if (pp == NULL)
  {
    eok = 0;
    pp = MHD_create_post_processor (connection, 1024, &post_iterator, &eok);
    if (pp == NULL)
      abort ();
    *unused = pp;
  }
  MHD_post_process (pp, upload_data, *upload_data_size);
  if ((eok == 3) && (0 == *upload_data_size))
  {
    response = MHD_create_response_from_buffer (strlen (url),
                                                (void *) url,
                                                MHD_RESPMEM_MUST_COPY);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    MHD_destroy_post_processor (pp);
    *unused = NULL;
    return ret;
  }
  *upload_data_size = 0;
  return MHD_YES;
}


struct mhd_test_postdata
{
#if defined(HAS_CURL_MIME)
  curl_mime *mime;
#else  /* ! HAS_CURL_MIME */
  struct curl_httppost *post;
#endif /* ! HAS_CURL_MIME */
};

/* Return non-zero if succeed */
static int
add_test_form (CURL *handle, struct mhd_test_postdata *postdata)
{
#if defined(HAS_CURL_MIME)
  postdata->mime = curl_mime_init (handle);
  if (NULL == postdata->mime)
    return 0;
  else
  {
    curl_mimepart *part;
    part = curl_mime_addpart (postdata->mime);
    if (NULL != part)
    {
      if ( (CURLE_OK == curl_mime_data (part, "daniel",
                                        CURL_ZERO_TERMINATED)) &&
           (CURLE_OK == curl_mime_name (part, "name")) )
      {
        part = curl_mime_addpart (postdata->mime);
        if (NULL != part)
        {
          if ( (CURLE_OK == curl_mime_data (part, "curl",
                                            CURL_ZERO_TERMINATED)) &&
               (CURLE_OK == curl_mime_name (part, "project")) )
          {
            if (CURLE_OK == curl_easy_setopt (handle,
                                              CURLOPT_MIMEPOST, postdata->mime))
            {
              return ! 0;
            }
          }
        }
      }
    }
  }
  curl_mime_free (postdata->mime);
  postdata->mime = NULL;
  return 0;
#else  /* ! HAS_CURL_MIME */
  postdata->post = NULL;
  struct curl_httppost *last = NULL;

  if (0 == curl_formadd (&postdata->post, &last,
                         CURLFORM_COPYNAME, "name",
                         CURLFORM_COPYCONTENTS, "daniel", CURLFORM_END))
  {
    if (0 == curl_formadd (&postdata->post, &last,
                           CURLFORM_COPYNAME, "project",
                           CURLFORM_COPYCONTENTS, "curl", CURLFORM_END))
    {
      if (CURLE_OK == curl_easy_setopt (handle,
                                        CURLOPT_HTTPPOST, postdata->post))
      {
        return ! 0;
      }
    }
  }
  curl_formfree (postdata->post);
  return 0;
#endif /* ! HAS_CURL_MIME */
}


static void
free_test_form (struct mhd_test_postdata *postdata)
{
#if defined(HAS_CURL_MIME)
  if (NULL != postdata->mime)
    curl_mime_free (postdata->mime);
#else  /* ! HAS_CURL_MIME */
  if (NULL != postdata->post)
    curl_formfree (postdata->post);
#endif /* ! HAS_CURL_MIME */
}


static int
testInternalPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  struct mhd_test_postdata form;
  uint16_t port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1390;
    if (oneone)
      port += 10;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,
                        MHD_OPTION_END);
  if (d == NULL)
    return 1;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  /* NOTE: use of CONNECTTIMEOUT without also
   *   setting NOSIGNAL results in really weird
   *   crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  if (! add_test_form (c, &form))
  {
    fprintf (stderr, "libcurl form initialisation error.\n");
    curl_easy_cleanup (c);
    return 2;
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    free_test_form (&form);
    MHD_stop_daemon (d);
    return 2;
  }
  curl_easy_cleanup (c);
  free_test_form (&form);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 4;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 8;
  return 0;
}


static int
testMultithreadedPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  struct mhd_test_postdata form;
  uint16_t port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1390;
    if (oneone)
      port += 10;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,
                        MHD_OPTION_END);
  if (d == NULL)
    return 16;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 5L);
  /* NOTE: use of CONNECTTIMEOUT without also
   *   setting NOSIGNAL results in really weird
   *   crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  if (! add_test_form (c, &form))
  {
    fprintf (stderr, "libcurl form initialisation error.\n");
    curl_easy_cleanup (c);
    return 2;
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    free_test_form (&form);
    MHD_stop_daemon (d);
    return 32;
  }
  curl_easy_cleanup (c);
  free_test_form (&form);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 64;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 128;
  return 0;
}


static int
testMultithreadedPoolPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  struct mhd_test_postdata form;
  uint16_t port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1391;
    if (oneone)
      port += 10;
  }

  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_THREAD_POOL_SIZE, MHD_CPU_COUNT,
                        MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,
                        MHD_OPTION_END);
  if (d == NULL)
    return 16;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 5L);
  /* NOTE: use of CONNECTTIMEOUT without also
   *   setting NOSIGNAL results in really weird
   *   crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  if (! add_test_form (c, &form))
  {
    fprintf (stderr, "libcurl form initialisation error.\n");
    curl_easy_cleanup (c);
    return 2;
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    free_test_form (&form);
    MHD_stop_daemon (d);
    return 32;
  }
  curl_easy_cleanup (c);
  free_test_form (&form);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 64;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 128;
  return 0;
}


static int
testExternalPost ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLM *multi;
  CURLMcode mret;
  fd_set rs;
  fd_set ws;
  fd_set es;
  MHD_socket maxsock;
#ifdef MHD_WINSOCK_SOCKETS
  int maxposixs; /* Max socket number unused on W32 */
#else  /* MHD_POSIX_SOCKETS */
#define maxposixs maxsock
#endif /* MHD_POSIX_SOCKETS */
  int running;
  struct CURLMsg *msg;
  time_t start;
  struct timeval tv;
  struct mhd_test_postdata form;
  uint16_t port;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port = 0;
  else
  {
    port = 1392;
    if (oneone)
      port += 10;
  }

  multi = NULL;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, NULL,
                        MHD_OPTION_NOTIFY_COMPLETED, &completed_cb, NULL,
                        MHD_OPTION_END);
  if (d == NULL)
    return 256;
  if (0 == port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
    {
      MHD_stop_daemon (d); return 32;
    }
    port = dinfo->port;
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  if (oneone)
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  else
    curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  /* NOTE: use of CONNECTTIMEOUT without also
   *   setting NOSIGNAL results in really weird
   *   crashes on my system! */
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  if (! add_test_form (c, &form))
  {
    fprintf (stderr, "libcurl form initialisation error.\n");
    curl_easy_cleanup (c);
    return 2;
  }

  multi = curl_multi_init ();
  if (multi == NULL)
  {
    curl_easy_cleanup (c);
    free_test_form (&form);
    MHD_stop_daemon (d);
    return 512;
  }
  mret = curl_multi_add_handle (multi, c);
  if (mret != CURLM_OK)
  {
    curl_multi_cleanup (multi);
    free_test_form (&form);
    curl_easy_cleanup (c);
    MHD_stop_daemon (d);
    return 1024;
  }
  start = time (NULL);
  while ((time (NULL) - start < 5) && (multi != NULL))
  {
    maxsock = MHD_INVALID_SOCKET;
    maxposixs = -1;
    FD_ZERO (&rs);
    FD_ZERO (&ws);
    FD_ZERO (&es);
    curl_multi_perform (multi, &running);
    mret = curl_multi_fdset (multi, &rs, &ws, &es, &maxposixs);
    if (mret != CURLM_OK)
    {
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      MHD_stop_daemon (d);
      free_test_form (&form);
      return 2048;
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxsock))
    {
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      free_test_form (&form);
      MHD_stop_daemon (d);
      return 4096;
    }
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    if (-1 == select (maxposixs + 1, &rs, &ws, &es, &tv))
    {
#ifdef MHD_POSIX_SOCKETS
      if (EINTR != errno)
      {
        fprintf (stderr, "Unexpected select() error: %d. Line: %d\n",
                 (int) errno, __LINE__);
        fflush (stderr);
        exit (99);
      }
#else
      if ((WSAEINVAL != WSAGetLastError ()) ||
          (0 != rs.fd_count) || (0 != ws.fd_count) || (0 != es.fd_count) )
      {
        fprintf (stderr, "Unexpected select() error: %d. Line: %d\n",
                 (int) WSAGetLastError (), __LINE__);
        fflush (stderr);
        exit (99);
      }
      Sleep (1);
#endif
    }
    curl_multi_perform (multi, &running);
    if (0 == running)
    {
      int pending;
      int curl_fine = 0;
      while (NULL != (msg = curl_multi_info_read (multi, &pending)))
      {
        if (msg->msg == CURLMSG_DONE)
        {
          if (msg->data.result == CURLE_OK)
            curl_fine = 1;
          else
          {
            fprintf (stderr,
                     "%s failed at %s:%d: `%s'\n",
                     "curl_multi_perform",
                     __FILE__,
                     __LINE__, curl_easy_strerror (msg->data.result));
            abort ();
          }
        }
      }
      if (! curl_fine)
      {
        fprintf (stderr, "libcurl haven't returned OK code\n");
        abort ();
      }
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      c = NULL;
      multi = NULL;
    }
    MHD_run (d);
  }
  if (multi != NULL)
  {
    curl_multi_remove_handle (multi, c);
    curl_easy_cleanup (c);
    curl_multi_cleanup (multi);
  }
  free_test_form (&form);
  MHD_stop_daemon (d);
  if (cbc.pos != strlen ("/hello_world"))
    return 8192;
  if (0 != strncmp ("/hello_world", cbc.buf, strlen ("/hello_world")))
    return 16384;
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc;   /* Unused. Silent compiler warning. */

#ifdef MHD_HTTPS_REQUIRE_GCRYPT
#ifdef HAVE_GCRYPT_H
  gcry_control (GCRYCTL_ENABLE_QUICK_RANDOM, 0);
#ifdef GCRYCTL_INITIALIZATION_FINISHED
  gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);
#endif
#endif
#endif /* MHD_HTTPS_REQUIRE_GCRYPT */
  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  oneone = has_in_name (argv[0], "11");
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    errorCount += testInternalPost ();
    errorCount += testMultithreadedPost ();
    errorCount += testMultithreadedPoolPost ();
  }
  errorCount += testExternalPost ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
