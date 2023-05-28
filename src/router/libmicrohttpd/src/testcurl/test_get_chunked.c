/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Christian Grothoff
     Copyright (C) 2014-2021 Evgeny Grin (Karlson2k)

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
 * @file test_get_chunked.c
 * @brief  Testcase for libmicrohttpd GET operations with chunked content encoding
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

#ifndef WINDOWS
#include <unistd.h>
#endif

#include "mhd_has_in_name.h"

#if defined(MHD_CPU_COUNT) && (MHD_CPU_COUNT + 0) < 2
#undef MHD_CPU_COUNT
#endif
#if ! defined(MHD_CPU_COUNT)
#define MHD_CPU_COUNT 2
#endif

#define HDR_CHUNKED_ENCODING MHD_HTTP_HEADER_TRANSFER_ENCODING ": chunked"
#define RESP_FOOTER_NAME "Footer"
#define RESP_FOOTER_VALUE "working"
#define RESP_FOOTER RESP_FOOTER_NAME ": " RESP_FOOTER_VALUE

#define RESP_BLOCK_SIZE 128
#define RESP_BLOCK_QUANTIY 10
#define RESP_SIZE (RESP_BLOCK_SIZE * RESP_BLOCK_QUANTIY)

/**
 * Use "Connection: close" header?
 */
int conn_close;

/**
 * Use static string response instead of callback-generated?
 */
int resp_string;

/**
 * Use response with known size?
 */
int resp_sized;

/**
 * Use empty (zero-sized) response?
 */
int resp_empty;

/**
 * Force chunked response by response header?
 */
int chunked_forced;

/**
 * MHD port used for testing
 */
int port_global;


struct headers_check_result
{
  int found_chunked;
  int found_footer;
};

size_t
lcurl_hdr_callback (char *buffer, size_t size, size_t nitems,
                    void *userdata)
{
  const size_t data_size = size * nitems;
  struct headers_check_result *check_res =
    (struct headers_check_result *) userdata;

  if ((data_size == strlen (HDR_CHUNKED_ENCODING) + 2) &&
      (0 == memcmp (buffer, HDR_CHUNKED_ENCODING "\r\n", data_size)))
    check_res->found_chunked = 1;
  if ((data_size == strlen (RESP_FOOTER) + 2) &&
      (0 == memcmp (buffer, RESP_FOOTER "\r\n", data_size)))
    check_res->found_footer = 1;

  return data_size;
}


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


/**
 * MHD content reader callback that returns data in chunks.
 */
static ssize_t
crc (void *cls,
     uint64_t pos,
     char *buf,
     size_t max)
{
  struct MHD_Response **responseptr = cls;

  if (resp_empty || (pos == RESP_SIZE - RESP_BLOCK_SIZE))
  { /* Add footer with the last block */
    if (MHD_YES != MHD_add_response_footer (*responseptr,
                                            RESP_FOOTER_NAME,
                                            RESP_FOOTER_VALUE))
      abort ();
  }
  if (resp_empty || (pos == RESP_SIZE))
    return MHD_CONTENT_READER_END_OF_STREAM;

  if (max < RESP_BLOCK_SIZE)
    abort ();                   /* should not happen in this testcase... */
  memset (buf, 'A' + (pos / RESP_BLOCK_SIZE), RESP_BLOCK_SIZE);
  return RESP_BLOCK_SIZE;
}


/**
 * Dummy function that frees the "responseptr".
 */
static void
crcf (void *ptr)
{
  free (ptr);
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr)
{
  static int aptr;
  const char *me = cls;
  struct MHD_Response *response;
  enum MHD_Result ret;

  (void) url;
  (void) version;              /* Unused. Silent compiler warning. */
  (void) upload_data;
  (void) upload_data_size;     /* Unused. Silent compiler warning. */

  if (0 != strcmp (me, method))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
  {
    /* do never respond on first call */
    *ptr = &aptr;
    return MHD_YES;
  }
  if (! resp_string)
  {
    struct MHD_Response **responseptr;
    responseptr = malloc (sizeof (struct MHD_Response *));
    if (NULL == responseptr)
      _exit (99);

    response = MHD_create_response_from_callback (resp_sized ?
                                                  RESP_SIZE : MHD_SIZE_UNKNOWN,
                                                  1024,
                                                  &crc,
                                                  responseptr,
                                                  &crcf);
    *responseptr = response;
  }
  else
  {
    if (! resp_empty)
    {
      size_t pos;
      static const size_t resp_size = RESP_SIZE;
      char *buf = malloc (resp_size);
      if (NULL == buf)
        _exit (99);
      for (pos = 0; pos < resp_size; pos += RESP_BLOCK_SIZE)
        memset (buf + pos, 'A' + (pos / RESP_BLOCK_SIZE), RESP_BLOCK_SIZE);

      response = MHD_create_response_from_buffer (resp_size, buf,
                                                  MHD_RESPMEM_MUST_COPY);
      free (buf);
    }
    else
      response = MHD_create_response_from_buffer (0, NULL,
                                                  MHD_RESPMEM_PERSISTENT);
  }
  if (NULL == response)
    abort ();
  if (chunked_forced)
  {
    if (MHD_NO == MHD_add_response_header (response,
                                           MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                           "chunked"))
      abort ();
  }
  if (MHD_NO == MHD_add_response_header (response,
                                         MHD_HTTP_HEADER_TRAILER,
                                         RESP_FOOTER_NAME))
    abort ();

  if (resp_string || (resp_sized && resp_empty))
  {
    /* There is no chance to add footer later */
    if (MHD_YES != MHD_add_response_footer (response,
                                            RESP_FOOTER_NAME,
                                            RESP_FOOTER_VALUE))
      abort ();
  }

  ret = MHD_queue_response (connection,
                            MHD_HTTP_OK,
                            response);
  MHD_destroy_response (response);
  return ret;
}


static int
validate (struct CBC cbc, int ebase)
{
  int i;
  char buf[RESP_BLOCK_SIZE];

  if (resp_empty)
  {
    if (0 != cbc.pos)
    {
      fprintf (stderr,
               "Got %u bytes instead of zero!\n",
               (unsigned int) cbc.pos);
      return 1;
    }
    return 0;
  }

  if (cbc.pos != RESP_SIZE)
  {
    fprintf (stderr,
             "Got %u bytes instead of 1280!\n",
             (unsigned int) cbc.pos);
    return ebase;
  }

  for (i = 0; i < RESP_BLOCK_QUANTIY; i++)
  {
    memset (buf, 'A' + i, RESP_BLOCK_SIZE);
    if (0 != memcmp (buf, &cbc.buf[i * RESP_BLOCK_SIZE], RESP_BLOCK_SIZE))
    {
      fprintf (stderr,
               "Got  `%.*s'\nWant `%.*s'\n",
               RESP_BLOCK_SIZE, &cbc.buf[i * RESP_BLOCK_SIZE],
               RESP_BLOCK_SIZE, buf);
      return ebase * 2;
    }
  }
  return 0;
}


static int
testInternalGet ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  int port;
  struct curl_slist *h_list = NULL;
  struct headers_check_result hdr_check;

  port = port_global;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
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
    port = (int) dinfo->port;
    if (0 == port_global)
      port_global = port; /* Re-use the same port for all checks */
  }
  hdr_check.found_chunked = 0;
  hdr_check.found_footer = 0;
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt (c, CURLOPT_HEADERFUNCTION, lcurl_hdr_callback);
  curl_easy_setopt (c, CURLOPT_HEADERDATA, &hdr_check);
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  if (conn_close)
  {
    h_list = curl_slist_append (h_list, "Connection: close");
    if (NULL == h_list)
      abort ();
    curl_easy_setopt (c, CURLOPT_HTTPHEADER, h_list);
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    curl_slist_free_all (h_list);
    MHD_stop_daemon (d);
    return 2;
  }
  curl_easy_cleanup (c);
  curl_slist_free_all (h_list);
  MHD_stop_daemon (d);
  if (1 != hdr_check.found_chunked)
  {
    fprintf (stderr,
             "Chunked encoding header was not found in the response\n");
    return 8;
  }
  if (1 != hdr_check.found_footer)
  {
    fprintf (stderr,
             "The specified footer was not found in the response\n");
    return 16;
  }
  return validate (cbc, 4);
}


static int
testMultithreadedGet ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  int port;
  struct curl_slist *h_list = NULL;
  struct headers_check_result hdr_check;

  port = port_global;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_THREAD_PER_CONNECTION
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
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
    port = (int) dinfo->port;
    if (0 == port_global)
      port_global = port; /* Re-use the same port for all checks */
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  hdr_check.found_chunked = 0;
  hdr_check.found_footer = 0;
  curl_easy_setopt (c, CURLOPT_HEADERFUNCTION, lcurl_hdr_callback);
  curl_easy_setopt (c, CURLOPT_HEADERDATA, &hdr_check);
  if (conn_close)
  {
    h_list = curl_slist_append (h_list, "Connection: close");
    if (NULL == h_list)
      abort ();
    curl_easy_setopt (c, CURLOPT_HTTPHEADER, h_list);
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    curl_slist_free_all (h_list);
    MHD_stop_daemon (d);
    return 32;
  }
  curl_easy_cleanup (c);
  curl_slist_free_all (h_list);
  MHD_stop_daemon (d);
  if (1 != hdr_check.found_chunked)
  {
    fprintf (stderr,
             "Chunked encoding header was not found in the response\n");
    return 8;
  }
  if (1 != hdr_check.found_footer)
  {
    fprintf (stderr,
             "The specified footer was not found in the response\n");
    return 16;
  }
  return validate (cbc, 64);
}


static int
testMultithreadedPoolGet ()
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  CURLcode errornum;
  int port;
  struct curl_slist *h_list = NULL;
  struct headers_check_result hdr_check;

  port = port_global;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, "GET",
                        MHD_OPTION_THREAD_POOL_SIZE, MHD_CPU_COUNT,
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
    port = (int) dinfo->port;
    if (0 == port_global)
      port_global = port; /* Re-use the same port for all checks */
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  hdr_check.found_chunked = 0;
  hdr_check.found_footer = 0;
  curl_easy_setopt (c, CURLOPT_HEADERFUNCTION, lcurl_hdr_callback);
  curl_easy_setopt (c, CURLOPT_HEADERDATA, &hdr_check);
  if (conn_close)
  {
    h_list = curl_slist_append (h_list, "Connection: close");
    if (NULL == h_list)
      abort ();
    curl_easy_setopt (c, CURLOPT_HTTPHEADER, h_list);
  }
  if (CURLE_OK != (errornum = curl_easy_perform (c)))
  {
    fprintf (stderr,
             "curl_easy_perform failed: `%s'\n",
             curl_easy_strerror (errornum));
    curl_easy_cleanup (c);
    curl_slist_free_all (h_list);
    MHD_stop_daemon (d);
    return 32;
  }
  curl_easy_cleanup (c);
  curl_slist_free_all (h_list);
  MHD_stop_daemon (d);
  if (1 != hdr_check.found_chunked)
  {
    fprintf (stderr,
             "Chunked encoding header was not found in the response\n");
    return 8;
  }
  if (1 != hdr_check.found_footer)
  {
    fprintf (stderr,
             "The specified footer was not found in the response\n");
    return 16;
  }
  return validate (cbc, 64);
}


static int
testExternalGet ()
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
  int port;
  struct curl_slist *h_list = NULL;
  struct headers_check_result hdr_check;

  port = port_global;
  multi = NULL;
  cbc.buf = buf;
  cbc.size = 2048;
  cbc.pos = 0;
  d = MHD_start_daemon (MHD_USE_ERROR_LOG,
                        port, NULL, NULL, &ahc_echo, "GET", MHD_OPTION_END);
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
    port = (int) dinfo->port;
    if (0 == port_global)
      port_global = port; /* Re-use the same port for all checks */
  }
  c = curl_easy_init ();
  curl_easy_setopt (c, CURLOPT_URL, "http://127.0.0.1/hello_world");
  curl_easy_setopt (c, CURLOPT_PORT, (long) port);
  curl_easy_setopt (c, CURLOPT_WRITEFUNCTION, &copyBuffer);
  curl_easy_setopt (c, CURLOPT_WRITEDATA, &cbc);
  curl_easy_setopt (c, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt (c, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
  curl_easy_setopt (c, CURLOPT_TIMEOUT, 150L);
  curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT, 5L);
  curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L);
  hdr_check.found_chunked = 0;
  hdr_check.found_footer = 0;
  curl_easy_setopt (c, CURLOPT_HEADERFUNCTION, lcurl_hdr_callback);
  curl_easy_setopt (c, CURLOPT_HEADERDATA, &hdr_check);
  if (conn_close)
  {
    h_list = curl_slist_append (h_list, "Connection: close");
    if (NULL == h_list)
      abort ();
    curl_easy_setopt (c, CURLOPT_HTTPHEADER, h_list);
  }

  multi = curl_multi_init ();
  if (multi == NULL)
  {
    curl_easy_cleanup (c);
    curl_slist_free_all (h_list);
    MHD_stop_daemon (d);
    return 512;
  }
  mret = curl_multi_add_handle (multi, c);
  if (mret != CURLM_OK)
  {
    curl_multi_cleanup (multi);
    curl_easy_cleanup (c);
    curl_slist_free_all (h_list);
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
      curl_slist_free_all (h_list);
      MHD_stop_daemon (d);
      return 2048;
    }
    if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxsock))
    {
      curl_multi_remove_handle (multi, c);
      curl_multi_cleanup (multi);
      curl_easy_cleanup (c);
      curl_slist_free_all (h_list);
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
      curl_slist_free_all (h_list);
      h_list = NULL;
      c = NULL;
      multi = NULL;
    }
    MHD_run (d);
  }
  MHD_stop_daemon (d);
  if (multi != NULL)
  {
    curl_multi_remove_handle (multi, c);
    curl_easy_cleanup (c);
    curl_multi_cleanup (multi);
  }
  curl_slist_free_all (h_list);
  if (1 != hdr_check.found_chunked)
  {
    fprintf (stderr,
             "Chunked encoding header was not found in the response\n");
    return 8;
  }
  if (1 != hdr_check.found_footer)
  {
    fprintf (stderr,
             "The specified footer was not found in the response\n");
    return 16;
  }
  return validate (cbc, 8192);
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */

  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  conn_close = has_in_name (argv[0], "_close");
  resp_string = has_in_name (argv[0], "_string");
  resp_sized = has_in_name (argv[0], "_sized");
  resp_empty = has_in_name (argv[0], "_empty");
  chunked_forced = has_in_name (argv[0], "_forced");
  if (resp_string)
    resp_sized = ! 0;
  if (resp_sized)
    chunked_forced = ! 0;

  if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    port_global = 0;
  else
  {
    port_global = 4100;
    if (conn_close)
      port_global += 1 << 0;
    if (resp_string)
      port_global += 1 << 1;
    if (resp_sized)
      port_global += 1 << 2;
    if (resp_empty)
      port_global += 1 << 3;
    if (chunked_forced)
      port_global += 1 << 4;
  }

  if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS))
  {
    errorCount += testInternalGet ();
    errorCount += testMultithreadedGet ();
    errorCount += testMultithreadedPoolGet ();
  }
  errorCount += testExternalGet ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
