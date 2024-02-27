/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2008 Christian Grothoff
     Copyright (C) 2016-2022 Evgeny Grin (Karlson2k)

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
 * @file testzzuf/test_get.c
 * @brief  Several testcases for libmicrohttpd with input fuzzing
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "platform.h"
#include <curl/curl.h>
#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#ifndef WINDOWS
#include <unistd.h>
#endif

#include "mhd_debug_funcs.h"
#include "test_helpers.h"

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
 * A larger loop count will run more random tests --
 * which would be good, except that it may take too
 * long for most user's patience.  So this small
 * value is the default.
 * Can be redefined by CPPFLAGS=-DLOOP_COUNT=123
 */
#ifndef LOOP_COUNT
#ifndef _MHD_VHEAVY_TESTS
#define LOOP_COUNT 10
#else  /* ! _MHD_HEAVY_TESTS */
#define LOOP_COUNT 200
#endif /* ! _MHD_HEAVY_TESTS */
#endif /* LOOP_COUNT */

#ifdef _DEBUG
/* Uncomment the next line (or use CPPFLAGS) to see all request and response bodies in log */
/* #define TEST_PRINT_BODY */
/* Uncomment the next line (or use CPPFLAGS) to see all request bodies as they are sent by libcurl */
/* #define TEST_PRINT_BODY_RQ 1 */
/* Uncomment the next line (or use CPPFLAGS) to see all request bodies as they are received by libcurl */
/* #define TEST_PRINT_BODY_RP 1 */
#endif /* _DEBUG */

#define MHD_TIMEOUT 2

#define CURL_TIMEOUT 5

/* Global test parameters */
static int oneone;
static int dry_run;
static int use_get;
static int use_get_chunked;
static int use_put;
static int use_put_large;
static int use_put_chunked;
static int use_post;
static int use_post_form;
static int use_long_header;
static int use_long_uri;
static int use_close;
static int run_with_socat;

#define TEST_BASE_URI "http:/" "/127.0.0.1/test_uri"
#define TEST_BASE_URI_SOCAT "http:/" "/127.0.0.121/test_uri"

#define SOCAT_PORT 10121

#define TEST_BASE_PORT 4010

#define EMPTY_PAGE "Empty page."
#define EMPTY_PAGE_ALT "Alternative empty page."
#define METHOD_NOT_SUPPORTED "HTTP method is not supported."
#define POST_DATA_BROKEN "The POST request is ill-formed."

#define POST_KEY1 "test"
#define POST_VALUE1 "test_post"
#define POST_KEY2 "library"
#define POST_VALUE2 "GNU libmicrohttpd"
#define POST_URLENC_DATA \
  POST_KEY1 "=" POST_VALUE1 "&" POST_KEY2 "=" "GNU%20libmicrohttpd"

#define PUT_NORMAL_SIZE 11
/* Does not need to be very large as MHD buffer will be made smaller anyway */
#define PUT_LARGE_SIZE (4 * 1024)
/* The length of "very long" URI and header strings. MHD uses smaller buffer. */
#define TEST_STRING_VLONG_LEN 8 * 1024


#if ! CURL_AT_LEAST_VERSION (7,56,0)
#define TEST_USE_STATIC_POST_DATA 1
static struct curl_httppost *post_first;
static struct curl_httppost *post_last;
#endif /* ! CURL_AT_LEAST_VERSION(7,56,0) */

static struct curl_slist *libcurl_long_header;

/**
 * Initialise long header for libcurl
 *
 * @return non-zero if succeed,
 *         zero if failed
 */
static int
long_header_init (void)
{
  char *buf;

  buf = malloc (TEST_STRING_VLONG_LEN + 1);
  if (NULL == buf)
  {
    fprintf (stderr, "malloc() failed "
             "at line %d.\n", (int) __LINE__);
    return 0;
  }
  buf[TEST_STRING_VLONG_LEN] = 0;
  buf[0] = 'A';
  memset (buf + 1, 'a', TEST_STRING_VLONG_LEN / 2 - 2);
  buf[TEST_STRING_VLONG_LEN / 2 - 1] = ':';
  buf[TEST_STRING_VLONG_LEN / 2] = ' ';
  memset (buf + TEST_STRING_VLONG_LEN / 2 + 1, 'c',
          TEST_STRING_VLONG_LEN / 2 - 1);
  libcurl_long_header = curl_slist_append (NULL, buf);
  free (buf);
  if (NULL != libcurl_long_header)
    return ! 0; /* Success exit point */

  fprintf (stderr, "curl_slist_append() failed "
           "at line %d.\n", (int) __LINE__);
  return 0; /* Failure exit point */
}


/**
 * Globally initialise test environment
 * @return non-zero if succeed,
 *         zero if failed
 */
static int
test_global_init (void)
{
  libcurl_long_header = NULL;
  if (CURLE_OK != curl_global_init (CURL_GLOBAL_WIN32))
  {
    fprintf (stderr, "curl_global_init() failed "
             "at line %d.\n", (int) __LINE__);
    return 0;
  }

  if (long_header_init ())
  {
#ifndef TEST_USE_STATIC_POST_DATA
    return 1; /* Success exit point */
#else  /* ! TEST_USE_STATIC_POST_DATA */
    post_first = NULL;
    post_last = NULL;
    if ((CURL_FORMADD_OK !=
         curl_formadd (&post_first, &post_last,
                       CURLFORM_PTRNAME, POST_KEY1,
                       CURLFORM_NAMELENGTH,
                       (long) MHD_STATICSTR_LEN_ (POST_KEY1),
                       CURLFORM_PTRCONTENTS, POST_VALUE1,
#if CURL_AT_LEAST_VERSION (7,46,0)
                       CURLFORM_CONTENTLEN,
                       (curl_off_t) MHD_STATICSTR_LEN_ (POST_VALUE1),
#else  /* ! CURL_AT_LEAST_VERSION(7,46,0) */
                       CURLFORM_CONTENTSLENGTH,
                       (long) MHD_STATICSTR_LEN_ (POST_VALUE1),
#endif /* ! CURL_AT_LEAST_VERSION(7,46,0) */
                       CURLFORM_END)) ||
        (CURL_FORMADD_OK !=
         curl_formadd (&post_first, &post_last,
                       CURLFORM_PTRNAME, POST_KEY2,
                       CURLFORM_NAMELENGTH,
                       (long) MHD_STATICSTR_LEN_ (POST_KEY2),
                       CURLFORM_PTRCONTENTS, POST_VALUE2,
#if CURL_AT_LEAST_VERSION (7,46,0)
                       CURLFORM_CONTENTLEN,
                       (curl_off_t) MHD_STATICSTR_LEN_ (POST_VALUE2),
#else  /* ! CURL_AT_LEAST_VERSION(7,46,0) */
                       CURLFORM_CONTENTSLENGTH,
                       (long) MHD_STATICSTR_LEN_ (POST_VALUE2),
#endif /* ! CURL_AT_LEAST_VERSION(7,46,0) */
                       CURLFORM_END)))
      fprintf (stderr, "curl_formadd() failed "
               "at line %d.\n", (int) __LINE__);
    else
      return 1; /* Success exit point */

    if (NULL != post_first)
      curl_formfree (post_first);
    curl_slist_free_all (libcurl_long_header);
#endif /* ! CURL_AT_LEAST_VERSION(7,56,0) */
  }
  curl_global_cleanup ();
  return 0; /* Failure exit point */
}


/**
 * Globally de-initialise test environment
 */
static void
test_global_deinit (void)
{
#ifdef TEST_USE_STATIC_POST_DATA
  curl_formfree (post_first);
#endif /* TEST_USE_STATIC_POST_DATA */
  curl_global_cleanup ();
  if (NULL != libcurl_long_header)
    curl_slist_free_all (libcurl_long_header);
}


/**
 * libcurl callback parameters for uploads, downloads and debug callbacks
 */
struct CBC
{
  /* Upload members */
  size_t up_pos;
  size_t up_size;
  /* Download members */
  char *dn_buf;
  size_t dn_pos;
  size_t dn_buf_size;
  /* Debug callback members */
  unsigned int excess_found;
};

static void
initCBC (struct CBC *libcurlcbc, char *dn_buf, size_t dn_buf_size)
{
  libcurlcbc->up_pos = 0;
  if (use_put_large)
    libcurlcbc->up_size = PUT_LARGE_SIZE;
  else if (use_put)
    libcurlcbc->up_size = PUT_NORMAL_SIZE;
  else
    libcurlcbc->up_size = 0;
  libcurlcbc->dn_buf = dn_buf;
  libcurlcbc->dn_pos = 0;
  libcurlcbc->dn_buf_size = dn_buf_size;
  libcurlcbc->excess_found = 0;
}


static void
resetCBC (struct CBC *libcurlcbc)
{
  libcurlcbc->up_pos = 0;
  libcurlcbc->dn_pos = 0;
}


static size_t
putBuffer (void *stream, size_t item_size, size_t nitems, void *ctx)
{
  size_t to_fill;
  size_t i;
  struct CBC *cbc = ctx;

  to_fill = cbc->up_size - cbc->up_pos;
  /* Skip overflow check as the return value is valid anyway */
  if (use_put_chunked)
  {
    /* Send data as several chunks */
    if (to_fill > cbc->up_size / 3)
      to_fill = cbc->up_size / 3;
  }
  if (to_fill > item_size * nitems)
    to_fill = item_size * nitems;

  /* Avoid libcurl magic numbers */
#ifdef CURL_READFUNC_PAUSE
  if (CURL_READFUNC_ABORT == to_fill)
    to_fill -= 2;
#endif /* CURL_READFUNC_PAUSE */
#ifdef CURL_READFUNC_ABORT
  if (CURL_READFUNC_ABORT == to_fill)
    --to_fill;
#endif /* CURL_READFUNC_ABORT */
  for (i = 0; i < to_fill; ++i)
    ((char *) stream)[i] = 'a' + (char) ((cbc->up_pos + i)
                                         % (unsigned char) ('z' - 'a' + 1));

  cbc->up_pos += to_fill;
  return to_fill;
}


static size_t
copyBuffer (void *ptr, size_t size, size_t nmemb, void *ctx)
{
  struct CBC *cbc = ctx;

  if (cbc->dn_pos + size * nmemb > cbc->dn_buf_size)
    return 0;                   /* overflow */
  memcpy (&cbc->dn_buf[cbc->dn_pos], ptr, size * nmemb);
  cbc->dn_pos += size * nmemb;
  return size * nmemb;
}


#define TEST_MAGIC_MARKER0 0xFEE1C0DE
#define TEST_MAGIC_MARKER1 (TEST_MAGIC_MARKER0 + 1)
#define TEST_MAGIC_MARKER2 (TEST_MAGIC_MARKER0 + 2)

struct content_cb_param_strct
{
  unsigned int magic0;    /**< Must have TEST_MAGIC_MARKER0 value */
  struct MHD_Response *response; /**< The pointer to the response structure */
};

/**
 * MHD content reader callback that returns
 * data in chunks.
 */
static ssize_t
content_cb (void *cls, uint64_t pos, char *buf, size_t max)
{
  struct content_cb_param_strct *param = (struct content_cb_param_strct *) cls;
  size_t fill_size;

  if ((unsigned int) TEST_MAGIC_MARKER0 != param->magic0)
  {
    fprintf (stderr, "Wrong cls pointer "
             "at line %d.\n", (int) __LINE__);
    fflush (stderr);
    abort ();
  }

  if (pos >= 128 * 10)
  {
    if (MHD_YES !=
        MHD_add_response_footer (param->response, "Footer", "working"))
    {
      fprintf (stderr, "MHD_add_response_footer() failed "
               "at line %d.\n", (int) __LINE__);
      fflush (stderr);
      abort ();
    }
    return MHD_CONTENT_READER_END_OF_STREAM;
  }

  if (128 > max)
    fill_size = 128;
  else
    fill_size = max;
  memset (buf, 'A' + (char) (unsigned int) (pos / 128), fill_size);

  return (ssize_t) fill_size;
}


/**
 * Deallocate memory for callback cls.
 */
static void
crcf (void *ptr)
{
  free (ptr);
}


struct req_process_strct
{
  unsigned int magic2;   /**< Must have TEST_MAGIC_MARKER2 value */
  int is_static;         /**< Non-zero if statically allocated, zero if malloc()'ed */
  struct MHD_PostProcessor *postprocsr;
  unsigned int post_data_sum;
};

static enum MHD_Result
post_iterator (void *cls,
               enum MHD_ValueKind kind,
               const char *key,
               const char *filename,
               const char *content_type,
               const char *transfer_encoding,
               const char *value, uint64_t off, size_t size)
{
  struct req_process_strct *param = (struct req_process_strct *) cls;
  size_t i;

  (void) filename; (void) content_type; (void) transfer_encoding;
  (void) off; /* Unused. Mute compiler warnings. */

  if (TEST_MAGIC_MARKER2 != param->magic2)
  {
    fprintf (stderr, "The 'param->magic2' has wrong value "
             "at line %d.\n", (int) __LINE__);
    abort ();
  }

  if (MHD_POSTDATA_KIND != kind)
  {
    fprintf (stderr, "The 'kind' parameter has wrong value "
             "at line %d.\n", (int) __LINE__);
    abort ();
  }

  if (NULL != key)
    param->post_data_sum += (unsigned int) strlen (key);

  for (i = 0; size > i; ++i)
    param->post_data_sum += (unsigned int) (unsigned char) value[i];

  return MHD_YES;
}


static void
free_req_pr_data (struct req_process_strct *pr_data)
{
  if (NULL == pr_data)
    return;
  if (TEST_MAGIC_MARKER2 != pr_data->magic2)
  {
    fprintf (stderr, "The 'pr_data->magic2' has wrong value "
             "at line %d.\n", (int) __LINE__);
    abort ();
  }
  if (pr_data->is_static)
  {
    if (NULL != pr_data->postprocsr)
    {
      fprintf (stderr, "The 'pr_data->postprocsr' has wrong value "
               "at line %d.\n", (int) __LINE__);
      abort ();
    }
    return;
  }
  if (NULL != pr_data->postprocsr)
    MHD_destroy_post_processor (pr_data->postprocsr);
  pr_data->postprocsr = NULL;
  free (pr_data);
}


struct ahc_param_strct
{
  unsigned int magic1;   /**< Must have TEST_MAGIC_MARKER1 value */
  unsigned int err_flag; /**< Non-zero if any error is encountered */
  unsigned int num_replies; /**< The number of replies sent for the current request */
};

static enum MHD_Result
send_error_response (struct MHD_Connection *connection,
                     struct ahc_param_strct *param,
                     unsigned int status_code,
                     const char *static_text,
                     const size_t static_text_len)
{
  struct MHD_Response *response;
  response =
    MHD_create_response_from_buffer_static (static_text_len,
                                            static_text);
  if (NULL != response)
  {
    if (MHD_YES == MHD_add_response_header (response,
                                            MHD_HTTP_HEADER_CONNECTION,
                                            "close"))
    {
      if (MHD_YES == MHD_queue_response (connection, status_code, response))
      {
        MHD_destroy_response (response);
        return MHD_YES; /* Success exit point */
      }
      else
        fprintf (stderr, "MHD_queue_response() failed "
                 "at line %d.\n", (int) __LINE__);
    }
    else
      fprintf (stderr, "MHD_add_response_header() failed "
               "at line %d.\n", (int) __LINE__);
    MHD_destroy_response (response);
  }
  else
    fprintf (stderr, "MHD_create_response_from_callback() failed "
             "at line %d.\n", (int) __LINE__);

  param->err_flag = 1;
  return MHD_NO; /* Failure exit point */
}


static enum MHD_Result
ahc_check (void *cls,
           struct MHD_Connection *connection,
           const char *url,
           const char *method,
           const char *version,
           const char *upload_data, size_t *upload_data_size,
           void **req_cls)
{
  static struct req_process_strct static_req_pr_data = {
    TEST_MAGIC_MARKER2, ! 0, NULL, 0
  };
  struct req_process_strct *req_pr_data;
  struct ahc_param_strct *param = (struct ahc_param_strct *) cls;
  struct MHD_Response *response;
  enum MHD_Result ret;
  unsigned char data_sum;
  int is_post_req;

  if (NULL == cls)
  {
    fprintf (stderr, "The 'cls' parameter is NULL "
             "at line %d.\n", (int) __LINE__);
    fflush (stderr);
    abort ();
  }
  if ((unsigned int) TEST_MAGIC_MARKER1 != param->magic1)
  {
    fprintf (stderr, "The 'param->magic1' has wrong value "
             "at line %d.\n", (int) __LINE__);
    fflush (stderr);
    abort ();
  }
  if (NULL == connection)
  {
    fprintf (stderr, "The 'connection' parameter is NULL "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
    return MHD_NO; /* Should not reply */
  }
  if (1)
  { /* Simple check for 'connection' parameter validity */
    const union MHD_ConnectionInfo *conn_info;
    conn_info =
      MHD_get_connection_info (connection,
                               MHD_CONNECTION_INFO_CONNECTION_TIMEOUT);
    if (NULL == conn_info)
    {
      fprintf (stderr, "The 'MHD_get_connection_info' has returned NULL "
               "at line %d.\n", (int) __LINE__);
      param->err_flag = 1;
    }
    else if (MHD_TIMEOUT != conn_info->connection_timeout)
    {
      fprintf (stderr, "The 'MHD_get_connection_info' has returned "
               "unexpected timeout value "
               "at line %d.\n", (int) __LINE__);
      param->err_flag = 1;
    }
  }
  if (NULL == url)
  {
    fprintf (stderr, "The 'url' parameter is NULL "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
  }
  if (NULL == method)
  {
    fprintf (stderr, "The 'method' parameter is NULL "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
    return MHD_NO; /* Should not reply */
  }
  if (NULL == version)
  {
    fprintf (stderr, "The 'version' parameter is NULL "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
    return MHD_NO; /* Should not reply */
  }
  if (NULL == upload_data_size)
  {
    fprintf (stderr, "The 'upload_data_size' parameter is NULL "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
    return MHD_NO; /* Should not reply */
  }
  if ((0 != *upload_data_size) && (NULL == upload_data))
  {
    fprintf (stderr, "The 'upload_data' parameter is NULL "
             "while '*upload_data_size' is not zero "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
    return MHD_NO; /* Should not reply */
  }
  if ((NULL != upload_data) && (0 == *upload_data_size))
  {
    fprintf (stderr, "The 'upload_data' parameter is NOT NULL "
             "while '*upload_data_size' is zero "
             "at line %d.\n", (int) __LINE__);
    param->err_flag = 1;
    return MHD_NO; /* Should not reply */
  }

  if (0 != param->num_replies)
  {
    /* Phantom "second" request due to the fuzzing of the input. Refuse. */
    return MHD_NO;
  }

  is_post_req = (0 == strcmp (method, MHD_HTTP_METHOD_POST));
  if ((0 != strcmp (method, MHD_HTTP_METHOD_GET))
      && (0 != strcmp (method, MHD_HTTP_METHOD_HEAD))
      && (0 != strcmp (method, MHD_HTTP_METHOD_PUT))
      && (! is_post_req))
  {
    /* Unsupported method for this callback */
    return send_error_response (connection, param, MHD_HTTP_NOT_IMPLEMENTED,
                                METHOD_NOT_SUPPORTED,
                                MHD_STATICSTR_LEN_ (METHOD_NOT_SUPPORTED));
  }

  if (NULL == *req_cls)
  {
    if (! is_post_req)
    { /* Use static memory */
      *req_cls = &static_req_pr_data;
    }
    else
    { /* POST request, use PostProcessor */
      req_pr_data =
        (struct req_process_strct *) malloc (sizeof (struct req_process_strct));
      if (NULL == req_pr_data)
      {
        fprintf (stderr, "malloc() failed "
                 "at line %d.\n", (int) __LINE__);
        return MHD_NO;
      }
      req_pr_data->magic2 = TEST_MAGIC_MARKER2;
      req_pr_data->is_static = 0;
      req_pr_data->post_data_sum = 0;
      req_pr_data->postprocsr = MHD_create_post_processor (connection, 1024,
                                                           &post_iterator,
                                                           req_pr_data);
      if (NULL == req_pr_data->postprocsr)
      {
        free (req_pr_data);
        if (NULL == upload_data)
          return send_error_response (connection, param, MHD_HTTP_BAD_REQUEST,
                                      POST_DATA_BROKEN,
                                      MHD_STATICSTR_LEN_ (POST_DATA_BROKEN));
        else
          return MHD_NO; /* Cannot handle request, broken POST */
      }
      *req_cls = req_pr_data;
    }
    if (NULL == upload_data)
      return MHD_YES;
  }
  req_pr_data = (struct req_process_strct *) *req_cls;

  data_sum = 0;
  if (NULL != upload_data)
  {
    if (is_post_req)
    {
      if (MHD_YES != MHD_post_process (req_pr_data->postprocsr,
                                       upload_data, *upload_data_size))
      {
        free_req_pr_data (req_pr_data);
        *req_cls = NULL;
        /* Processing upload body (context), error reply cannot be queued here */
        return MHD_NO;
      }
      *upload_data_size = 0; /* All data have been processed */
    }
    else
    {
      /* Check that all 'upload_data' is addressable */
      size_t pos;
      for (pos = 0; pos < *upload_data_size; ++pos)
        data_sum =
          (unsigned char) (data_sum + (unsigned char) upload_data[pos]);
      if (0 != *upload_data_size)
      {
        if (3 >= *upload_data_size)
          *upload_data_size = 0;                             /* Consume all incoming data */
        else
          *upload_data_size = data_sum % *upload_data_size;  /* Pseudo-random */
      }
    }
    return MHD_YES;
  }
  if (is_post_req)
  {
    if (MHD_YES != MHD_destroy_post_processor (req_pr_data->postprocsr))
    {
      free (req_pr_data);
      *req_cls = NULL;
      return send_error_response (connection, param, MHD_HTTP_BAD_REQUEST,
                                  POST_DATA_BROKEN,
                                  MHD_STATICSTR_LEN_ (POST_DATA_BROKEN));
    }
    req_pr_data->postprocsr = NULL;
  }
  data_sum += (unsigned char) req_pr_data->post_data_sum;
  free_req_pr_data (req_pr_data);
  *req_cls = NULL;

  ret = MHD_YES;
  if (use_get_chunked)
  {
    struct content_cb_param_strct *cnt_cb_param;
    cnt_cb_param = malloc (sizeof (struct content_cb_param_strct));
    if (NULL == cnt_cb_param)
    {
      fprintf (stderr, "malloc() failed "
               "at line %d.\n", (int) __LINE__);
      /* External error, do not rise the error flag */
      return MHD_NO;
    }
    cnt_cb_param->magic0 = (unsigned int) TEST_MAGIC_MARKER0;
    response = MHD_create_response_from_callback (MHD_SIZE_UNKNOWN,
                                                  1024,
                                                  &content_cb, cnt_cb_param,
                                                  &crcf);
    if (NULL == response)
    {
      fprintf (stderr, "MHD_create_response_from_callback() failed "
               "at line %d.\n", (int) __LINE__);
      free (cnt_cb_param);
      param->err_flag = 1;
      ret = MHD_NO;
    }
    else
      cnt_cb_param->response = response;
  }
  else if (use_get || use_put || use_post)
  {
    /* Randomly choose the response page for the POST requests */
    if (0 == data_sum % 2)
      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ (EMPTY_PAGE),
                                                EMPTY_PAGE);
    else
      response =
        MHD_create_response_from_buffer_static (MHD_STATICSTR_LEN_ ( \
                                                  EMPTY_PAGE_ALT),
                                                EMPTY_PAGE_ALT);

    if (NULL == response)
    {
      fprintf (stderr, "MHD_create_response_from_buffer_static() failed "
               "at line %d.\n", (int) __LINE__);
      param->err_flag = 1;
      ret = MHD_NO;
    }
  }
  else
  {
    fprintf (stderr, "Response is not implemented for this test. "
             "Internal logic is broken. "
             "At line %d.\n", (int) __LINE__);
    abort ();
  }

  if (NULL != response)
  {
    if ((MHD_YES == ret) &&
        (use_close || (! oneone && (0 != strcmp (version,
                                                 MHD_HTTP_VERSION_1_0)))))
    {
      ret = MHD_add_response_header (response,
                                     MHD_HTTP_HEADER_CONNECTION,
                                     "close");
      if (MHD_YES != ret)
      {
        fprintf (stderr, "MHD_add_response_header() failed "
                 "at line %d.\n", (int) __LINE__);
        param->err_flag = 1;
      }
    }
    if (MHD_YES == ret)
    {
      ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
      if (MHD_YES != ret)
      {
        fprintf (stderr, "MHD_queue_response() failed "
                 "at line %d.\n", (int) __LINE__);
        param->err_flag = 1;
      }
    }
    else
      param->num_replies++;

    MHD_destroy_response (response);
  }
  else
  {
    fprintf (stderr, "MHD_create_response_from_buffer_static() failed "
             "at line %d.\n", (int) __LINE__);
    ret = MHD_NO;
  }
  return ret;
}


static void
req_completed_cleanup (void *cls,
                       struct MHD_Connection *connection,
                       void **req_cls,
                       enum MHD_RequestTerminationCode toe)
{
  struct ahc_param_strct *param = (struct ahc_param_strct *) cls;
  struct req_process_strct *req_pr_data = (struct req_process_strct *) *req_cls;
  (void) connection; /* Unused. Mute compiler warning. */

  if (NULL == param)
  {
    fprintf (stderr, "The 'cls' parameter is NULL at line %d.\n",
             (int) __LINE__);
    fflush (stderr);
    abort ();
  }
  if ((unsigned int) TEST_MAGIC_MARKER1 != param->magic1)
  {
    fprintf (stderr, "The 'param->magic1' has wrong value at line %d.\n",
             (int) __LINE__);
    fflush (stderr);
    abort ();
  }
  if (NULL == req_pr_data)
    return; /* The data have been freed */
  if ((unsigned int) TEST_MAGIC_MARKER2 != req_pr_data->magic2)
  {
    fprintf (stderr, "The 'req_pr_data->magic2' has wrong value at line %d.\n",
             (int) __LINE__);
    fflush (stderr);
    abort ();
  }
  if (MHD_REQUEST_TERMINATED_COMPLETED_OK == toe)
  {
    fprintf (stderr, "The request completed successful, but request cls has"
             "not been cleared. "
             "At line %d.\n", (int) __LINE__);
    param->err_flag = 1;
  }
  if (req_pr_data->is_static)
    return;
  if (NULL != req_pr_data->postprocsr)
    MHD_destroy_post_processor (req_pr_data->postprocsr);
  req_pr_data->postprocsr = NULL;
  free (req_pr_data);
  *req_cls = NULL;
}


/* Un-comment the next line (or use CPPFLAGS) to avoid
   logging of the traffic with debug builds */
/* #define TEST_NO_PRINT_TRAFFIC 1 */

#ifdef _DEBUG
#ifdef TEST_PRINT_BODY
#ifndef TEST_PRINT_BODY_RQ
#define TEST_PRINT_BODY_RQ 1
#endif /* TEST_PRINT_BODY_RQ */
#ifndef TEST_PRINT_BODY_RP
#define TEST_PRINT_BODY_RP 1
#endif /* TEST_PRINT_BODY_RP */
#endif /* TEST_PRINT_BODY */
#endif /* _DEBUG */

static int
libcurl_debug_cb (CURL *handle,
                  curl_infotype type,
                  char *data,
                  size_t size,
                  void *ctx)
{
  static const char excess_mark[] = "Excess found";
  static const size_t excess_mark_len = MHD_STATICSTR_LEN_ (excess_mark);
  struct CBC *cbc = ctx;
  (void) handle;

#if defined(_DEBUG) && ! defined(TEST_NO_PRINT_TRAFFIC)
  switch (type)
  {
  case CURLINFO_TEXT:
    fprintf (stderr, "* %.*s", (int) size, data);
    break;
  case CURLINFO_HEADER_IN:
    fprintf (stderr, "< %.*s", (int) size, data);
    break;
  case CURLINFO_HEADER_OUT:
    fprintf (stderr, "> %.*s", (int) size, data);
    break;
  case CURLINFO_DATA_IN:
#ifdef TEST_PRINT_BODY_RP
    fprintf (stderr, "<| %.*s\n", (int) size, data);
#endif /* TEST_PRINT_BODY_RP */
    break;
  case CURLINFO_DATA_OUT:
#ifdef TEST_PRINT_BODY_RQ
    fprintf (stderr, ">| %.*s\n", (int) size, data);
#endif /* TEST_PRINT_BODY_RQ */
    break;
  case CURLINFO_SSL_DATA_IN:
  case CURLINFO_SSL_DATA_OUT:
  case CURLINFO_END:
  default:
    break;
  }
#endif /* _DEBUG  && ! TEST_NO_PRINT_TRAFFIC */
  if (use_close || ! oneone)
  {
    /* Check for extra data only if every connection is terminated by MHD
       after one request, otherwise MHD may react on garbage after request
       data. */
    if (CURLINFO_TEXT == type)
    {
      if ((size >= excess_mark_len) &&
          (0 == memcmp (data, excess_mark, excess_mark_len)))
      {
        fprintf (stderr, "Extra data has been detected in MHD reply "
                 "at line %d.\n", (int) __LINE__);
        cbc->excess_found++;
      }
    }
  }
  return 0;
}


static CURL *
setupCURL (struct CBC *cbc, uint16_t port
#ifndef TEST_USE_STATIC_POST_DATA
           , curl_mime **mime
#endif /* ! TEST_USE_STATIC_POST_DATA */
           )
{
  CURL *c;
  CURLcode e;
  char *buf;
  const char *uri_to_use;
  const char *base_uri;

#ifndef TEST_USE_STATIC_POST_DATA
  *mime = NULL;
#endif /* ! TEST_USE_STATIC_POST_DATA */

  base_uri = run_with_socat ? TEST_BASE_URI_SOCAT : TEST_BASE_URI;
  if (! use_long_uri)
  {
    uri_to_use = base_uri;
    buf = NULL;
  }
  else
  {
    size_t pos;
    size_t base_uri_len;

    base_uri_len = strlen (base_uri);
    buf = malloc (TEST_STRING_VLONG_LEN + 1);
    if (NULL == buf)
    {
      fprintf (stderr, "malloc() failed "
               "at line %d.\n", (int) __LINE__);
      return NULL;
    }
    memcpy (buf, base_uri, base_uri_len);
    for (pos = base_uri_len;
         pos < TEST_STRING_VLONG_LEN;
         ++pos)
    {
      if (0 == pos % 9)
        buf[pos] = '/';
      else
        buf[pos] = 'a' + (char) (unsigned char) (pos % ((unsigned char)
                                                        ('z' - 'a' + 1)));
    }
    buf[TEST_STRING_VLONG_LEN] = 0;
    uri_to_use = buf;
  }
  if (run_with_socat)
    port = SOCAT_PORT;

  c = curl_easy_init ();
  if (NULL == c)
  {
    fprintf (stderr, "curl_easy_init() failed "
             "at line %d.\n", (int) __LINE__);
    return NULL;
  }

  if ((CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_URL,
                                          uri_to_use))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_NOSIGNAL, 1L))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_WRITEFUNCTION,
                                          &copyBuffer))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_WRITEDATA, cbc))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_CONNECTTIMEOUT,
                                          ((long) CURL_TIMEOUT)))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_TIMEOUT,
                                          ((long) CURL_TIMEOUT)))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_FAILONERROR, 0L))) &&
#ifdef _DEBUG
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_VERBOSE, 1L))) &&
#endif /* _DEBUG */
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_DEBUGFUNCTION,
                                          &libcurl_debug_cb))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_DEBUGDATA,
                                          cbc))) &&
#if CURL_AT_LEAST_VERSION (7, 45, 0)
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_DEFAULT_PROTOCOL,
                                          "http"))) &&
#endif /* CURL_AT_LEAST_VERSION (7, 45, 0) */
#if CURL_AT_LEAST_VERSION (7, 85, 0)
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_PROTOCOLS_STR,
                                          "http"))) &&
#elif CURL_AT_LEAST_VERSION (7, 19, 4)
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_PROTOCOLS,
                                          CURLPROTO_HTTP))) &&
#endif /* CURL_AT_LEAST_VERSION (7, 19, 4) */
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_HTTP_VERSION,
                                          oneone ?
                                          CURL_HTTP_VERSION_1_1 :
                                          CURL_HTTP_VERSION_1_0))) &&
#if CURL_AT_LEAST_VERSION (7, 24, 0)
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_INTERFACE,
                                          "host!127.0.0.101"))) &&
#else  /* ! CURL_AT_LEAST_VERSION (7, 24, 0) */
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_INTERFACE,
                                          "127.0.0.101"))) &&
#endif /* ! CURL_AT_LEAST_VERSION (7, 24, 0) */
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_PORT, ((long) port)))) &&
      (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_HTTPHEADER,
                                          use_long_header ?
                                          libcurl_long_header : NULL)))
      )
  {
    if (NULL != buf)
    {
      free (buf);
      buf = NULL;
    }
    if (use_put)
    {
      if ((CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_READFUNCTION,
                                              &putBuffer))) &&
          (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_READDATA, cbc))) &&
          (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_UPLOAD, (long) 1))) &&
          (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_INFILESIZE_LARGE,
                                              use_put_chunked ?
                                              ((curl_off_t) -1) :
                                              ((curl_off_t) cbc->up_size)))))
      {
        return c; /* Success exit point for 'use_put' */
      }
      else
        fprintf (stderr, "PUT-related curl_easy_setopt() failed at line %d, "
                 "error: %s\n", (int) __LINE__,
                 curl_easy_strerror (e));
    }
    else if (use_post)
    {
      if (! use_post_form)
      {
        if ((CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_POST, (long) 1))) &&
            (CURLE_OK == (e = curl_easy_setopt (c, CURLOPT_POSTFIELDS,
                                                POST_URLENC_DATA))) &&
            (CURLE_OK ==
             (e = curl_easy_setopt (c, CURLOPT_POSTFIELDSIZE,
                                    MHD_STATICSTR_LEN_ (POST_URLENC_DATA)))))
        {
          return c; /* Success exit point for 'use_post' */
        }
        else
          fprintf (stderr,
                   "POST-related curl_easy_setopt() failed at line %d, "
                   "error: %s\n", (int) __LINE__,
                   curl_easy_strerror (e));
      }
      else
      {
#ifndef TEST_USE_STATIC_POST_DATA
        *mime = curl_mime_init (c);
        if (NULL != *mime)
        {
          curl_mimepart *part;
          if ((NULL != (part = curl_mime_addpart (*mime))) &&
              (CURLE_OK == curl_mime_name (part, POST_KEY1)) &&
              (CURLE_OK == curl_mime_data (part, POST_VALUE1,
                                           MHD_STATICSTR_LEN_ (POST_VALUE1))) &&
              (NULL != (part = curl_mime_addpart (*mime))) &&
              (CURLE_OK == curl_mime_name (part, POST_KEY2)) &&
              (CURLE_OK == curl_mime_data (part, POST_VALUE2,
                                           MHD_STATICSTR_LEN_ (POST_VALUE2))))
          {
            if (CURLE_OK ==
                (e = curl_easy_setopt (c, CURLOPT_MIMEPOST, *mime)))
              return c; /* Success exit point for 'use_post' */
            else
              fprintf (stderr, "curl_easy_setopt(c, CURLOPT_MIMEPOST, mime) "
                       "failed at line %d, error: %s\n",
                       (int) __LINE__, curl_easy_strerror (e));
          }
          else
            fprintf (stderr, "curl_mime_addpart(), curl_mime_name() or "
                     "curl_mime_data() failed.\n");
        }
        else
          fprintf (stderr, "curl_mime_init() failed.\n");

#else  /* TEST_USE_STATIC_POST_DATA */
        if (CURLE_OK == (e = curl_easy_setopt (c,
                                               CURLOPT_HTTPPOST, post_first)))
        {
          return c; /* Success exit point for 'use_post' */
        }
        else
          fprintf (stderr, "POST form-related curl_easy_setopt() failed, "
                   "error: %s\n", curl_easy_strerror (e));
#endif /* TEST_USE_STATIC_POST_DATA */
      }
    }
    else
      return c; /* Success exit point */
  }
  else
    fprintf (stderr, "curl_easy_setopt() failed at line %d, "
             "error: %s\n", (int) __LINE__,
             curl_easy_strerror (e));

  curl_easy_cleanup (c);
#ifndef TEST_USE_STATIC_POST_DATA
  if (NULL != *mime)
    curl_mime_free (*mime);
#endif /* ! TEST_USE_STATIC_POST_DATA */

  if (NULL != buf)
    free (buf);

  return NULL; /* Failure exit point */
}


static struct MHD_Daemon *
start_daemon_for_test (unsigned int daemon_flags, uint16_t *pport,
                       struct ahc_param_strct *callback_param)
{
  struct MHD_Daemon *d;
  struct MHD_OptionItem ops[] = {
    { MHD_OPTION_END, 0, NULL },
    { MHD_OPTION_END, 0, NULL },
    { MHD_OPTION_END, 0, NULL }
  };
  size_t num_opt;

  num_opt = 0;

  callback_param->magic1 = (unsigned int) TEST_MAGIC_MARKER1;
  callback_param->err_flag = 0;
  callback_param->num_replies = 0;

  if (use_put_large)
  {
    ops[num_opt].option = MHD_OPTION_CONNECTION_MEMORY_LIMIT;
    ops[num_opt].value = (intptr_t) (PUT_LARGE_SIZE / 4);
    ++num_opt;
  }
  else if (use_long_header || use_long_uri)
  {
    ops[num_opt].option = MHD_OPTION_CONNECTION_MEMORY_LIMIT;
    ops[num_opt].value = (intptr_t) (TEST_STRING_VLONG_LEN / 2);
    ++num_opt;
  }
  if (0 == (MHD_USE_INTERNAL_POLLING_THREAD & daemon_flags))
  {
    ops[num_opt].option = MHD_OPTION_APP_FD_SETSIZE;
    ops[num_opt].value = (intptr_t) (FD_SETSIZE);
    ++num_opt;
  }
  d = MHD_start_daemon (daemon_flags /* | MHD_USE_ERROR_LOG */,
                        *pport, NULL, NULL,
                        &ahc_check, callback_param,
                        MHD_OPTION_CONNECTION_TIMEOUT,
                        (unsigned int) MHD_TIMEOUT,
                        MHD_OPTION_NOTIFY_COMPLETED,
                        &req_completed_cleanup, callback_param,
                        MHD_OPTION_ARRAY, ops,
                        MHD_OPTION_END);
  if (NULL == d)
  {
    fprintf (stderr, "MHD_start_daemon() failed "
             "at line %d.\n", (int) __LINE__);
    return NULL;
  }

  /* Do not use accept4() as only accept() is intercepted by zzuf */
  if (! run_with_socat)
    MHD_avoid_accept4_ (d);

  if (0 == *pport)
  {
    const union MHD_DaemonInfo *dinfo;

    dinfo = MHD_get_daemon_info (d,
                                 MHD_DAEMON_INFO_BIND_PORT);
    if ( (NULL == dinfo) ||
         (0 == dinfo->port) )
    {
      fprintf (stderr, "MHD_get_daemon_info() failed "
               "at line %d.\n", (int) __LINE__);
      MHD_stop_daemon (d);
      return NULL;
    }
    *pport = dinfo->port;
  }
  return d;
}


static void
print_test_starting (unsigned int daemon_flags)
{
  fflush (stderr);
  if (0 != (MHD_USE_INTERNAL_POLLING_THREAD & daemon_flags))
  {
    if (0 != (MHD_USE_THREAD_PER_CONNECTION & daemon_flags))
    {
      if (0 != (MHD_USE_POLL & daemon_flags))
        printf ("\nStarting test with internal polling by poll() and "
                "thread-per-connection.\n");
      else
        printf ("\nStarting test with internal polling by select() and "
                "thread-per-connection.\n");
    }
    else
    {
      if (0 != (MHD_USE_POLL & daemon_flags))
        printf ("\nStarting test with internal polling by poll().\n");
      else if (0 != (MHD_USE_EPOLL & daemon_flags))
        printf ("\nStarting test with internal polling by 'epoll'.\n");
      else
        printf ("\nStarting test with internal polling by select().\n");
    }
  }
  else
  {
    if (0 != (MHD_USE_EPOLL & daemon_flags))
      printf ("\nStarting test with%s thread safety with external polling "
              "and internal 'epoll'.\n",
              ((0 != (MHD_USE_NO_THREAD_SAFETY & daemon_flags)) ? "out" : ""));
    else
      printf ("\nStarting test with%s thread safety with external polling.\n",
              ((0 != (MHD_USE_NO_THREAD_SAFETY & daemon_flags)) ? "out" : ""));
  }
  fflush (stdout);
}


static unsigned int
testInternalPolling (uint16_t *pport, unsigned int daemon_flags)
{
  struct MHD_Daemon *d;
  CURL *c;
  char buf[2048];
  struct CBC cbc;
  struct ahc_param_strct callback_param;
  unsigned int ret;
#ifndef TEST_USE_STATIC_POST_DATA
  curl_mime *mime;
#endif /* ! TEST_USE_STATIC_POST_DATA */

  if (0 == (MHD_USE_INTERNAL_POLLING_THREAD & daemon_flags))
  {
    fprintf (stderr, "Wrong internal flags, the test is broken. "
             "At line %d.\n", (int) __LINE__);
    abort (); /* Wrong flags, error in code */
  }

  print_test_starting (daemon_flags);
  initCBC (&cbc, buf, sizeof(buf));
  d = start_daemon_for_test (daemon_flags, pport, &callback_param);
  if (d == NULL)
    return 1;

  ret = 0;
  c = setupCURL (&cbc, *pport
#ifndef TEST_USE_STATIC_POST_DATA
                 , &mime
#endif /* ! TEST_USE_STATIC_POST_DATA */
                 );
  if (NULL != c)
  {
    int i;

    for (i = dry_run ? LOOP_COUNT : 0; i < LOOP_COUNT; i++)
    {
      fprintf (stderr, ".");
      callback_param.num_replies = 0;
      resetCBC (&cbc);
      /* Run libcurl without checking the result */
      curl_easy_perform (c);
      fflush (stderr);
    }
    curl_easy_cleanup (c);
#ifndef TEST_USE_STATIC_POST_DATA
    if (NULL != mime)
      curl_mime_free (mime);
#endif /* ! TEST_USE_STATIC_POST_DATA */
  }
  else
    ret = 99; /* Not an MHD error */

  if ((0 == ret) && callback_param.err_flag)
  {
    fprintf (stderr, "One or more errors have been detected by "
             "access handler callback function. "
             "At line %d.\n", (int) __LINE__);
    ret = 1;
  }
  else if ((0 == ret) && cbc.excess_found)
  {
    fprintf (stderr, "The extra reply data have been detected one "
             "or more times. "
             "At line %d.\n", (int) __LINE__);
    ret = 1;
  }

  fprintf (stderr, "\n");
  MHD_stop_daemon (d);
  fflush (stderr);
  return ret;
}


static unsigned int
testExternalPolling (uint16_t *pport, unsigned int daemon_flags)
{
  struct MHD_Daemon *d;
  CURLM *multi;
  char buf[2048];
  struct CBC cbc;
  struct ahc_param_strct callback_param;
  unsigned int ret;
#ifndef TEST_USE_STATIC_POST_DATA
  curl_mime *mime;
#endif /* ! TEST_USE_STATIC_POST_DATA */

  if (0 != (MHD_USE_INTERNAL_POLLING_THREAD & daemon_flags))
  {
    fprintf (stderr, "Wrong internal flags, the test is broken. "
             "At line %d.\n", (int) __LINE__);
    abort (); /* Wrong flags, error in code */
  }

  print_test_starting (daemon_flags);
  initCBC (&cbc, buf, sizeof(buf));
  d = start_daemon_for_test (daemon_flags, pport, &callback_param);
  if (d == NULL)
    return 1;

  ret = 0;
  multi = curl_multi_init ();
  if (multi == NULL)
  {
    fprintf (stderr, "curl_multi_init() failed "
             "at line %d.\n", (int) __LINE__);
    ret = 99; /* Not an MHD error */
  }
  else
  {
    CURL *c;
    c = setupCURL (&cbc, *pport
#ifndef TEST_USE_STATIC_POST_DATA
                   , &mime
#endif /* ! TEST_USE_STATIC_POST_DATA */
                   );

    if (NULL == c)
      ret = 99; /* Not an MHD error */
    else
    {
      int i;

      for (i = dry_run ? LOOP_COUNT : 0;
           (i < LOOP_COUNT) && (0 == ret); i++)
      {
        CURLMcode mret;

        /* The same 'multi' handle will be used in transfers so
           connection will be reused.
           The same 'easy' handle is added (and removed later) to (re-)start
           the same transfer. */
        mret = curl_multi_add_handle (multi, c);
        if (CURLM_OK != mret)
        {
          fprintf (stderr, "curl_multi_add_handle() failed at %d, "
                   "error: %s\n", (int) __LINE__,
                   curl_multi_strerror (mret));
          ret = 99; /* Not an MHD error */
        }
        else
        {
          time_t start;

          fprintf (stderr, ".");
          callback_param.num_replies = 0;
          resetCBC (&cbc);
          start = time (NULL);
          do
          {
            fd_set rs;
            fd_set ws;
            fd_set es;
            int maxfd_curl;
            MHD_socket maxfd_mhd;
            int maxfd;
            int running;
            struct timeval tv;

            maxfd_curl = 0;
            maxfd_mhd = MHD_INVALID_SOCKET;
            FD_ZERO (&rs);
            FD_ZERO (&ws);
            FD_ZERO (&es);
            curl_multi_perform (multi, &running);
            if (0 == running)
            {
              int msgs_left;
              do
              {
                (void) curl_multi_info_read (multi, &msgs_left);
              } while (0 != msgs_left);
              break; /* The transfer has been finished */
            }
            mret = curl_multi_fdset (multi, &rs, &ws, &es, &maxfd_curl);
            if (CURLM_OK != mret)
            {
              fprintf (stderr, "curl_multi_fdset() failed at line %d, "
                       "error: %s\n", (int) __LINE__,
                       curl_multi_strerror (mret));
              ret = 99; /* Not an MHD error */
              break;
            }
            if (MHD_YES != MHD_get_fdset (d, &rs, &ws, &es, &maxfd_mhd))
            {
              fprintf (stderr, "MHD_get_fdset() failed "
                       "at line %d.\n", (int) __LINE__);
              ret = 1;
              break;
            }
#ifndef MHD_WINSOCK_SOCKETS
            if ((int) maxfd_mhd > maxfd_curl)
              maxfd = (int) maxfd_mhd;
            else
#endif /* ! MHD_WINSOCK_SOCKETS */
            maxfd = maxfd_curl;
            tv.tv_sec = 0;
            tv.tv_usec = 100 * 1000;
            if (0 == MHD_get_timeout64s (d))
              tv.tv_usec = 0;
            else
            {
              long curl_to = -1;
              curl_multi_timeout (multi, &curl_to);
              if (0 == curl_to)
                tv.tv_usec = 0;
            }
            if (-1 == select (maxfd + 1, &rs, &ws, &es, &tv))
            {
#ifdef MHD_POSIX_SOCKETS
              if (EINTR != errno)
                fprintf (stderr, "Unexpected select() error "
                         "at line %d.\n", (int) __LINE__);
#else  /* ! MHD_POSIX_SOCKETS */
              if ((WSAEINVAL != WSAGetLastError ()) ||
                  (0 != rs.fd_count) || (0 != ws.fd_count) ||
                  (0 != es.fd_count))
                fprintf (stderr, "Unexpected select() error "
                         "at line %d.\n", (int) __LINE__);
              Sleep ((unsigned long) tv.tv_usec / 1000);
#endif /* ! MHD_POSIX_SOCKETS */
            }
            MHD_run (d);
          } while (time (NULL) - start <= MHD_TIMEOUT);
          /* Remove 'easy' handle from 'multi' handle to
           * restart the transfer or to finish. */
          curl_multi_remove_handle (multi, c);
        }
      }
      curl_easy_cleanup (c);
    }
    curl_multi_cleanup (multi);
#ifndef TEST_USE_STATIC_POST_DATA
    if (NULL != mime)
      curl_mime_free (mime);
#endif /* ! TEST_USE_STATIC_POST_DATA */
  }

  if ((0 == ret) && callback_param.err_flag)
  {
    fprintf (stderr, "One or more errors have been detected by "
             "access handler callback function. "
             "At line %d.\n", (int) __LINE__);
    ret = 1;
  }
  else if ((0 == ret) && cbc.excess_found)
  {
    fprintf (stderr, "The extra reply data have been detected one "
             "or more times. "
             "At line %d.\n", (int) __LINE__);
    ret = 1;
  }

  fprintf (stderr, "\n");
  MHD_stop_daemon (d);
  return 0;
}


static unsigned int
run_all_checks (void)
{
  uint16_t port;
  unsigned int testRes;
  unsigned int ret = 0;

  if (! run_with_socat)
  {
    if (MHD_are_sanitizers_enabled_ ())
    {
      fprintf (stderr, "Direct run with zzuf does not work with sanitizers. "
               "At line %d.\n", (int) __LINE__);
      return 77;
    }
    if (! MHD_is_avoid_accept4_possible_ ())
    {
      fprintf (stderr,
               "Non-debug build of MHD on this platform use accept4() function. "
               "Direct run with zzuf is not possible. "
               "At line %d.\n", (int) __LINE__);
      return 77;
    }
    if (MHD_NO != MHD_is_feature_supported (MHD_FEATURE_AUTODETECT_BIND_PORT))
      port = 0;     /* Use system automatic assignment */
    else
    {
      port = TEST_BASE_PORT;  /* Use predefined port, may break parallel testing of another MHD build */
      if (oneone)
        port += 100;
      if (use_long_uri)
        port += 30;
      else if (use_long_header)
        port += 35;
      else if (use_get_chunked)
        port += 0;
      else if (use_get)
        port += 5;
      else if (use_post_form)
        port += 10;
      else if (use_post)
        port += 15;
      else if (use_put_large)
        port += 20;
      else if (use_put_chunked)
        port += 25;
    }
  }
  else
    port = TEST_BASE_PORT;  /* Use predefined port, may break parallel testing of another MHD build */

  if (! dry_run && (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_THREADS)))
  {
    testRes = testInternalPolling (&port, MHD_USE_SELECT_INTERNALLY);
    if ((77 == testRes) || (99 == testRes))
      return testRes;
    ret += testRes;
    testRes = testInternalPolling (&port, MHD_USE_SELECT_INTERNALLY
                                   | MHD_USE_THREAD_PER_CONNECTION);
    if ((77 == testRes) || (99 == testRes))
      return testRes;
    ret += testRes;

    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_POLL))
    {
      testRes = testInternalPolling (&port, MHD_USE_POLL_INTERNALLY);
      if ((77 == testRes) || (99 == testRes))
        return testRes;
      ret += testRes;
      testRes = testInternalPolling (&port, MHD_USE_POLL_INTERNALLY
                                     | MHD_USE_THREAD_PER_CONNECTION);
      if ((77 == testRes) || (99 == testRes))
        return testRes;
      ret += testRes;
    }

    if (MHD_YES == MHD_is_feature_supported (MHD_FEATURE_EPOLL))
    {
      testRes = testInternalPolling (&port, MHD_USE_EPOLL_INTERNALLY);
      if ((77 == testRes) || (99 == testRes))
        return testRes;
    }
    testRes = testExternalPolling (&port, MHD_NO_FLAG);
  }
  testRes = testExternalPolling (&port, MHD_USE_NO_THREAD_SAFETY);
  if ((77 == testRes) || (99 == testRes))
    return testRes;
  ret += testRes;

  return ret;
}


int
main (int argc, char *const *argv)
{
  unsigned int res;
  int use_magic_exit_codes;

  oneone = ! has_in_name (argv[0], "10");
  use_get = has_in_name (argv[0], "_get");
  use_get_chunked = has_in_name (argv[0], "_get_chunked");
  use_put = has_in_name (argv[0], "_put");
  use_put_large = has_in_name (argv[0], "_put_large");
  use_put_chunked = has_in_name (argv[0], "_put_chunked");
  use_post = has_in_name (argv[0], "_post");
  use_post_form = has_in_name (argv[0], "_post_form");
  use_long_header = has_in_name (argv[0], "_long_header");
  use_long_uri = has_in_name (argv[0], "_long_uri");
  use_close = has_in_name (argv[0], "_close");

  run_with_socat = has_param (argc, argv, "--with-socat");
  dry_run = has_param (argc, argv, "--dry-run") ||
            has_param (argc, argv, "-n");

  if (1 !=
      ((use_get ? 1 : 0) + (use_put ? 1 : 0) + (use_post ? 1 : 0)))
  {
    fprintf (stderr, "Wrong test name '%s': no or multiple indications "
             "for the test type.\n", argv[0] ? argv[0] : "(NULL)");
    return 99;
  }
  use_magic_exit_codes = run_with_socat || dry_run;

  /* zzuf cannot bypass exit values.
     Unless 'dry run' is used, do not return errors for external error
     conditions (like out-of-memory) as they will be reported as test failures. */
  if (! test_global_init ())
    return use_magic_exit_codes ? 99 : 0;
  res = run_all_checks ();
  test_global_deinit ();
  if (99 == res)
    return use_magic_exit_codes ? 99 : 0;
  if (77 == res)
    return use_magic_exit_codes ? 77 : 0;
  return (0 == res) ? 0 : 1;       /* 0 == pass */
}
