/*
     This file is part of libmicrohttpd
     Copyright (C) 2007 Christian Grothoff
     Copyright (C) 2016-2023 Evgeny Grin (Karlson2k)

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
 * @file test_long_header.c
 * @brief  Testcase for libmicrohttpd handling of very long headers
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mhd_has_in_name.h"

#ifndef WINDOWS
#include <unistd.h>
#endif

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */

#ifndef CURL_VERSION_BITS
#define CURL_VERSION_BITS(x,y,z) ((x) << 16 | (y) << 8 | (z))
#endif /* ! CURL_VERSION_BITS */
#ifndef CURL_AT_LEAST_VERSION
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS (x, y, z))
#endif /* ! CURL_AT_LEAST_VERSION */

/**
 * We will set the memory available per connection to
 * half of this value, so the actual value does not have
 * to be big at all...
 */
#define VERY_LONG (1024 * 8)

/* Could be increased to facilitate debugging */
#define TIMEOUTS_VAL 5

#define EXPECTED_URI_BASE_PATH  "/"

#define URL_SCHEME "http:/" "/"

#define URL_HOST "127.0.0.1"

#define URL_SCHEME_HOST_PATH URL_SCHEME URL_HOST EXPECTED_URI_BASE_PATH


static int oneone;

static uint16_t daemon_port;


static char libcurl_err_buf[CURL_ERROR_SIZE];

struct CBC
{
  char *buf;
  size_t pos;
  size_t size;
};

static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  (void) ptr; (void) ctx;  /* Unused. Silent compiler warning. */
  return size * nmemb;
}


/* Return non-zero on success, zero on failure */
static int
setup_easy_handler_params (CURL *c, struct CBC *pcbc, const char *url,
                           struct curl_slist *header)
{
  libcurl_err_buf[0] = 0; /* Reset error message */

  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_ERRORBUFFER, libcurl_err_buf))
  {
    fprintf (stderr, "Failed to set CURLOPT_ERRORBUFFER option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L))
  {
    fprintf (stderr, "Failed to set CURLOPT_NOSIGNAL option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                    &copyBuffer))
  {
    fprintf (stderr, "Failed to set CURLOPT_WRITEFUNCTION option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_WRITEDATA, pcbc))
  {
    fprintf (stderr, "Failed to set CURLOPT_WRITEDATA option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                    ((long) TIMEOUTS_VAL)))
  {
    fprintf (stderr, "Failed to set CURLOPT_CONNECTTIMEOUT option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                    ((long) TIMEOUTS_VAL)))
  {
    fprintf (stderr, "Failed to set CURLOPT_TIMEOUT option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                    (oneone) ?
                                    CURL_HTTP_VERSION_1_1 :
                                    CURL_HTTP_VERSION_1_0))
  {
    fprintf (stderr, "Failed to set CURLOPT_HTTP_VERSION option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_FAILONERROR, 0L))
  {
    fprintf (stderr, "Failed to set CURLOPT_FAILONERROR option.\n");
    return 0;
  }
#ifdef _DEBUG
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_VERBOSE, 1L))
  {
    fprintf (stderr, "Failed to set CURLOPT_VERBOSE option.\n");
    return 0;
  }
#endif /* _DEBUG */
#if CURL_AT_LEAST_VERSION (7, 45, 0)
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_DEFAULT_PROTOCOL, "http"))
  {
    fprintf (stderr, "Failed to set CURLOPT_DEFAULT_PROTOCOL option.\n");
    return 0;
  }
#endif /* CURL_AT_LEAST_VERSION (7, 45, 0) */
#if CURL_AT_LEAST_VERSION (7, 85, 0)
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS_STR, "http"))
  {
    fprintf (stderr, "Failed to set CURLOPT_PROTOCOLS_STR option.\n");
    return 0;
  }
#elif CURL_AT_LEAST_VERSION (7, 19, 4)
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_PROTOCOLS, CURLPROTO_HTTP))
  {
    fprintf (stderr, "Failed to set CURLOPT_PROTOCOLS option.\n");
    return 0;
  }
#endif /* CURL_AT_LEAST_VERSION (7, 19, 4) */
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_URL,
                                    (NULL != url) ?
                                    url : URL_SCHEME_HOST_PATH))
  {
    fprintf (stderr, "Failed to set CURLOPT_PROTOCOLS option.\n");
    return 0;
  }
  if (CURLE_OK != curl_easy_setopt (c, CURLOPT_PORT, ((long) daemon_port)))
  {
    fprintf (stderr, "Failed to set CURLOPT_PROTOCOLS option.\n");
    return 0;
  }

  if (NULL != header)
  {
    if (CURLE_OK != curl_easy_setopt (c, CURLOPT_HTTPHEADER, header))
    {
      fprintf (stderr, "Failed to set CURLOPT_HTTPHEADER option.\n");
      return 0;
    }
  }
  return ! 0;
}


static CURL *
setup_easy_handler (struct CBC *pcbc, const char *url, struct
                    curl_slist *header)
{
  CURL *c;

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() error.\n");
    return NULL;
  }
  if (setup_easy_handler_params (c, pcbc, url, header))
  {
    return c; /* Success exit point */
  }
  curl_easy_cleanup (c);
  return NULL;
}


static enum MHD_Result
ahc_echo (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size,
          void **req_cls)
{
  struct MHD_Response *response;
  enum MHD_Result ret;
  static int marker;

  (void) cls;
  (void) version; (void) upload_data;      /* Unused. Silent compiler warning. */
  (void) upload_data_size; (void) req_cls; /* Unused. Silent compiler warning. */

  if (&marker != *req_cls)
  {
    *req_cls = &marker;
    return MHD_YES;
  }
  if (0 != strcmp (MHD_HTTP_METHOD_GET, method))
    return MHD_NO;              /* unexpected method */
  response = MHD_create_response_from_buffer_copy (strlen (url),
                                                   (const void *) url);
  ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
  MHD_destroy_response (response);
  return ret;
}


static unsigned int
testLongUrlGet (size_t buff_size)
{
  struct MHD_Daemon *d;
  unsigned int ret;

  ret = 1; /* Error value, shall be reset to zero if succeed */
  d =
    MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD /* | MHD_USE_ERROR_LOG */,
                      daemon_port,
                      NULL,
                      NULL,
                      &ahc_echo, NULL,
                      MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                      (size_t) buff_size, MHD_OPTION_END);
  if (d == NULL)
  {
    fprintf (stderr, "MHD_start_daemon() failed.\n");
    return 16;
  }
  if (0 == daemon_port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      fprintf (stderr, "MHD_get_daemon_info(d, MHD_DAEMON_INFO_BIND_PORT) " \
               "failed.\n");
    else
      daemon_port = dinfo->port;
  }
  if (0 != daemon_port)
  {
    char *url_str;
    url_str = malloc (VERY_LONG);
    if (NULL == url_str)
      fprintf (stderr, "malloc (VERY_LONG) failed.\n");
    else
    {
      CURL *c;
      char buf[2048];
      struct CBC cbc;

      memset (url_str, 'a', VERY_LONG);
      url_str[VERY_LONG - 1] = '\0';
      memcpy (url_str, URL_SCHEME_HOST_PATH,
              MHD_STATICSTR_LEN_ (URL_SCHEME_HOST_PATH));

      cbc.buf = buf;
      cbc.size = sizeof (buf);
      cbc.pos = 0;

      c = setup_easy_handler (&cbc, url_str, NULL);
      if (NULL != c)
      {
        CURLcode r;
        r = curl_easy_perform (c);
        if (CURLE_OK != r)
        {
          fprintf (stderr, "curl_easy_perform() failed. Error message: %s\n",
                   curl_easy_strerror (r));
          if (0 != libcurl_err_buf[0])
            fprintf (stderr, "Detailed error message: %s\n", libcurl_err_buf);
        }
        else
        {
          long code;

          r = curl_easy_getinfo (c, CURLINFO_RESPONSE_CODE, &code);
          if (CURLE_OK != r)
          {
            fprintf (stderr, "curl_easy_getinfo() failed. "
                     "Error message: %s\n",
                     curl_easy_strerror (r));
            if (0 != libcurl_err_buf[0])
              fprintf (stderr, "Detailed error message: %s\n",
                       libcurl_err_buf);
          }
          else
          {
            if (code != MHD_HTTP_URI_TOO_LONG)
            {
              fprintf (stderr, "testLongHeaderGet(%lu) failed. HTTP "
                       "response code is %ld, while it should be %ld.\n",
                       (unsigned long) buff_size,
                       code,
                       (long) MHD_HTTP_URI_TOO_LONG);
            }
            else
            {
              printf ("testLongHeaderGet(%lu) succeed. HTTP "
                      "response code is %ld, as expected.\n",
                      (unsigned long) buff_size,
                      (long) MHD_HTTP_URI_TOO_LONG);
              ret = 0; /* Success */
            }
          }
        }
        curl_easy_cleanup (c);
      }
      free (url_str);
    }
  }
  MHD_stop_daemon (d);
  return ret;
}


static unsigned int
testLongHeaderGet (size_t buff_size)
{
  struct MHD_Daemon *d;
  unsigned int ret;

  ret = 1; /* Error value, shall be reset to zero if succeed */
  d =
    MHD_start_daemon (MHD_USE_INTERNAL_POLLING_THREAD /* | MHD_USE_ERROR_LOG */,
                      daemon_port,
                      NULL,
                      NULL,
                      &ahc_echo, NULL,
                      MHD_OPTION_CONNECTION_MEMORY_LIMIT,
                      (size_t) buff_size, MHD_OPTION_END);
  if (d == NULL)
  {
    fprintf (stderr, "MHD_start_daemon() failed.\n");
    return 16;
  }
  if (0 == daemon_port)
  {
    const union MHD_DaemonInfo *dinfo;
    dinfo = MHD_get_daemon_info (d, MHD_DAEMON_INFO_BIND_PORT);
    if ((NULL == dinfo) || (0 == dinfo->port) )
      fprintf (stderr, "MHD_get_daemon_info(d, MHD_DAEMON_INFO_BIND_PORT) " \
               "failed.\n");
    else
      daemon_port = dinfo->port;
  }
  if (0 != daemon_port)
  {
    char *header_str;
    header_str = malloc (VERY_LONG);
    if (NULL == header_str)
      fprintf (stderr, "malloc (VERY_LONG) failed.\n");
    else
    {
      struct curl_slist *header = NULL;

      memset (header_str, 'a', VERY_LONG);
      header_str[VERY_LONG - 1] = '\0';
      header_str[VERY_LONG / 2] = ':';
      header_str[VERY_LONG / 2 + 1] = ' ';

      header = curl_slist_append (header, header_str);
      if (NULL == header)
        fprintf (stderr, "curl_slist_append () failed.\n");
      else
      {
        CURL *c;
        char buf[2048];
        struct CBC cbc;

        cbc.buf = buf;
        cbc.size = sizeof (buf);
        cbc.pos = 0;

        c = setup_easy_handler (&cbc, NULL, header);
        if (NULL != c)
        {
          CURLcode r;
          r = curl_easy_perform (c);
          if (CURLE_OK != r)
          {
            fprintf (stderr, "curl_easy_perform() failed. Error message: %s\n",
                     curl_easy_strerror (r));
            if (0 != libcurl_err_buf[0])
              fprintf (stderr, "Detailed error message: %s\n", libcurl_err_buf);
          }
          else
          {
            long code;

            r = curl_easy_getinfo (c, CURLINFO_RESPONSE_CODE, &code);
            if (CURLE_OK != r)
            {
              fprintf (stderr, "curl_easy_getinfo() failed. "
                       "Error message: %s\n",
                       curl_easy_strerror (r));
              if (0 != libcurl_err_buf[0])
                fprintf (stderr, "Detailed error message: %s\n",
                         libcurl_err_buf);
            }
            else
            {
              if (code != MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE)
              {
                fprintf (stderr, "testLongHeaderGet(%lu) failed. HTTP "
                         "response code is %ld, while it should be %ld.\n",
                         (unsigned long) buff_size,
                         code,
                         (long) MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE);
              }
              else
              {
                printf ("testLongHeaderGet(%lu) succeed. HTTP "
                        "response code is %ld, as expected.\n",
                        (unsigned long) buff_size,
                        (long) MHD_HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE);
                ret = 0; /* Success */
              }
            }
          }
          curl_easy_cleanup (c);
        }
        curl_slist_free_all (header);
      }
      free (header_str);
    }
  }
  MHD_stop_daemon (d);
  return ret;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc;   /* Unused. Silent compiler warning. */

  if ((NULL == argv) || (0 == argv[0]))
    return 99;
  oneone = has_in_name (argv[0], "11");
  if (0 != curl_global_init (CURL_GLOBAL_WIN32))
    return 2;
  if (MHD_NO == MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
    daemon_port = 0;
  else
    daemon_port = oneone ? 1336 : 1331;

  errorCount += testLongUrlGet (VERY_LONG / 2);
  errorCount += testLongUrlGet (VERY_LONG / 2 + 978);
  errorCount += testLongHeaderGet (VERY_LONG / 2);
  errorCount += testLongHeaderGet (VERY_LONG / 2 + 1893);
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  else
    printf ("Test succeed.\n");
  curl_global_cleanup ();
  return (0 == errorCount) ? 0 : 1;       /* 0 == pass */
}
