/*
     This file is part of libmicrohttpd
     Copyright (C) 2021 David Gausmann

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
 * @file test_websocket.c
 * @brief  Testcase for WebSocket decoding/encoding
 * @author David Gausmann
 */
#include "microhttpd.h"
#include "microhttpd_ws.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#if SIZE_MAX >= 0x100000000
  #define ENABLE_64BIT_TESTS 1
#endif

int disable_alloc = 0;
size_t open_allocs = 0;

/**
 * Custom `malloc()` function used for memory tests
 */
static void *
test_malloc (size_t buf_len)
{
  if (0 != disable_alloc)
    return NULL;
  void *result = malloc (buf_len);
  if (NULL != result)
    ++open_allocs;
  return result;
}


/**
 * Custom `realloc()` function used for memory tests
 */
static void *
test_realloc (void *buf, size_t buf_len)
{
  if (0 != disable_alloc)
    return NULL;
  void *result = realloc (buf, buf_len);
  if ((NULL != result) && (NULL == buf))
    ++open_allocs;
  return result;
}


/**
 * Custom `free()` function used for memory tests
 */
static void
test_free (void *buf)
{
  if (NULL != buf)
    --open_allocs;
  free (buf);
}


/**
 * Custom `rng()` function used for client mode tests
 */
static size_t
test_rng (void *cls, void *buf, size_t buf_len)
{
  for (size_t i = 0; i < buf_len; ++i)
  {
    ((char *) buf) [i] = (char) (rand () % 0xFF);
  }

  return buf_len;
}


/**
 * Helper function which allocates a big amount of data
 */
static void
allocate_length_test_data (char **buf1,
                           char **buf2,
                           size_t buf_len,
                           const char *buf1_prefix,
                           size_t buf1_prefix_len)
{
  if (NULL != *buf1)
    free (*buf1);
  if (NULL != *buf2)
    free (*buf2);
  *buf1 = (char *) malloc (buf_len + buf1_prefix_len);
  *buf2 = (char *) malloc (buf_len);
  if ((NULL == buf1) || (NULL == buf2))
    return;
  memcpy (*buf1,
          buf1_prefix,
          buf1_prefix_len);
  for (size_t i = 0; i < buf_len; i += 64)
  {
    size_t bytes_to_copy = buf_len - i;
    if (64 < bytes_to_copy)
      bytes_to_copy = 64;
    memcpy (*buf1 + i + buf1_prefix_len,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-",
            bytes_to_copy);
    memcpy (*buf2 + i,
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-",
            bytes_to_copy);
  }
}


/**
 * Helper function which performs a single decoder test
 */
static int
test_decode_single (unsigned int test_line,
                    int flags, size_t max_payload_size, size_t decode_count,
                    size_t buf_step,
                    const char *buf, size_t buf_len,
                    const char *expected_payload, size_t expected_payload_len,
                    int expected_return, int expected_valid, size_t
                    expected_streambuf_read_len)
{
  struct MHD_WebSocketStream *ws = NULL;
  int ret = MHD_WEBSOCKET_STATUS_OK;

  /* initialize stream */
  ret = MHD_websocket_stream_init2 (&ws,
                                    flags,
                                    max_payload_size,
                                    malloc,
                                    realloc,
                                    free,
                                    NULL,
                                    test_rng);
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "Allocation failed for decode test in line %u.\n",
             (unsigned int) test_line);
    return 1;
  }

  /* perform decoding in a loop */
  size_t streambuf_read_len = 0;
  size_t payload_len = 0;
  char *payload = NULL;
  for (size_t i = 0; i < decode_count; ++i)
  {
    size_t streambuf_read_len_ = 0;
    size_t bytes_to_take = buf_len - streambuf_read_len;
    if ((0 != buf_step) && (buf_step < bytes_to_take))
      bytes_to_take = buf_step;
    ret = MHD_websocket_decode (ws, buf + streambuf_read_len, bytes_to_take,
                                &streambuf_read_len_, &payload, &payload_len);
    streambuf_read_len += streambuf_read_len_;
    if (i + 1 < decode_count)
    {
      if (payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
        payload_len = 0;
      }
    }
  }

  /* check the (last) result */
  if (ret != expected_return)
  {
    fprintf (stderr,
             "Decode test failed in line %u: The return value should be %d, but is %d\n",
             (unsigned int) test_line,
             (int) expected_return,
             (int) ret);
    MHD_websocket_free (ws, payload);
    MHD_websocket_stream_free (ws);
    return 1;
  }
  if (payload_len != expected_payload_len)
  {
    fprintf (stderr,
             "Decode test failed in line %u: The payload_len should be %u, but is %u\n",
             (unsigned int) test_line,
             (unsigned int) expected_payload_len,
             (unsigned int) payload_len);
    MHD_websocket_free (ws, payload);
    MHD_websocket_stream_free (ws);
    return 1;
  }
  if (0 != payload_len)
  {
    if (NULL == payload)
    {
      fprintf (stderr,
               "Decode test failed in line %u: The payload is NULL\n",
               (unsigned int) test_line);
      MHD_websocket_free (ws, payload);
      MHD_websocket_stream_free (ws);
      return 1;
    }
    else if (NULL == expected_payload)
    {
      fprintf (stderr,
               "Decode test failed in line %u: The expected_payload is NULL (wrong test declaration)\n",
               (unsigned int) test_line);
      MHD_websocket_free (ws, payload);
      MHD_websocket_stream_free (ws);
      return 1;
    }
    else if (0 != memcmp (payload, expected_payload, payload_len))
    {
      fprintf (stderr,
               "Decode test failed in line %u: The payload differs from the expected_payload\n",
               (unsigned int) test_line);
      MHD_websocket_free (ws, payload);
      MHD_websocket_stream_free (ws);
      return 1;
    }
  }
  else
  {
    if (NULL != payload)
    {
      fprintf (stderr,
               "Decode test failed in line %u: The payload is not NULL, but payload_len is 0\n",
               (unsigned int) test_line);
      MHD_websocket_free (ws, payload);
      MHD_websocket_stream_free (ws);
      return 1;
    }
    else if (NULL != expected_payload)
    {
      fprintf (stderr,
               "Decode test failed in line %u: The expected_payload is not NULL, but expected_payload_len is 0 (wrong test declaration)\n",
               (unsigned int) test_line);
      MHD_websocket_free (ws, payload);
      MHD_websocket_stream_free (ws);
      return 1;
    }
  }
  if (streambuf_read_len != expected_streambuf_read_len)
  {
    fprintf (stderr,
             "Decode test failed in line %u: The streambuf_read_len should be %u, but is %u\n",
             (unsigned int) test_line,
             (unsigned int) expected_streambuf_read_len,
             (unsigned int) streambuf_read_len);
    MHD_websocket_free (ws, payload);
    MHD_websocket_stream_free (ws);
    return 1;
  }
  ret = MHD_websocket_stream_is_valid (ws);
  if (ret != expected_valid)
  {
    fprintf (stderr,
             "Decode test failed in line %u: The stream validity should be %u, but is %u\n",
             (unsigned int) test_line,
             (int) expected_valid,
             (int) ret);
    MHD_websocket_free (ws, payload);
    MHD_websocket_stream_free (ws);
    return 1;
  }

  /* cleanup */
  MHD_websocket_free (ws, payload);
  MHD_websocket_stream_free (ws);

  return 0;
}


/**
 * Test procedure for `MHD_websocket_stream_init()` and
 * `MHD_websocket_stream_init2()`
 */
int
test_inits ()
{
  int failed = 0;
  struct MHD_WebSocketStream *ws;
  int ret;

  /*
  ------------------------------------------------------------------------------
    All valid flags
  ------------------------------------------------------------------------------
  */
  /* Regular test: all valid flags for init (only the even ones work) */
  for (int i = 0; i < 7; ++i)
  {
    ws = NULL;
    ret = MHD_websocket_stream_init (&ws,
                                     i,
                                     0);
    if (((0 == (i & MHD_WEBSOCKET_FLAG_CLIENT)) &&
         ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (NULL == ws))) ||
        ((0 != (i & MHD_WEBSOCKET_FLAG_CLIENT)) &&
         ((MHD_WEBSOCKET_STATUS_OK == ret) ||
          (NULL != ws))))
    {
      fprintf (stderr,
               "Init test failed in line %u for flags %d.\n",
               (unsigned int) __LINE__,
               (int) i);
      ++failed;
    }
    if (NULL != ws)
    {
      MHD_websocket_stream_free (ws);
      ws = NULL;
    }
  }
  /* Regular test: all valid flags for init2 */
  for (int i = 0; i < 7; ++i)
  {
    ws = NULL;
    ret = MHD_websocket_stream_init2 (&ws,
                                      i,
                                      0,
                                      test_malloc,
                                      test_realloc,
                                      test_free,
                                      NULL,
                                      test_rng);
    if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
        (NULL == ws) )
    {
      fprintf (stderr,
               "Init test failed in line %u for flags %d.\n",
               (unsigned int) __LINE__,
               (int) i);
      ++failed;
    }
    if (NULL != ws)
    {
      MHD_websocket_stream_free (ws);
      ws = NULL;
    }
  }
  /* Fail test: Invalid flags for init */
  for (int i = 4; i < 32; ++i)
  {
    int flags = 1 << i;
    ws = NULL;
    ret = MHD_websocket_stream_init (&ws,
                                     flags,
                                     0);
    if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
        (NULL != ws) )
    {
      fprintf (stderr,
               "Init test failed in line %u for invalid flags %d.\n",
               (unsigned int) __LINE__,
               (int) flags);
      ++failed;
    }
    if (NULL != ws)
    {
      MHD_websocket_stream_free (ws);
      ws = NULL;
    }
  }
  /* Fail test: Invalid flag for init2 */
  for (int i = 4; i < 32; ++i)
  {
    int flags = 1 << i;
    ws = NULL;
    ret = MHD_websocket_stream_init2 (&ws,
                                      flags,
                                      0,
                                      test_malloc,
                                      test_realloc,
                                      test_free,
                                      NULL,
                                      NULL);
    if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
        (NULL != ws) )
    {
      fprintf (stderr,
               "Init test failed in line %u for invalid flags %d.\n",
               (unsigned int) __LINE__,
               (int) flags);
      ++failed;
    }
    if (NULL != ws)
    {
      MHD_websocket_stream_free (ws);
      ws = NULL;
    }
  }

  /*
  ------------------------------------------------------------------------------
    max_payload_size
  ------------------------------------------------------------------------------
  */
  /* Regular test: max_payload_size = 0 for init */
  ws = NULL;
  ret = MHD_websocket_stream_init (&ws,
                                   MHD_WEBSOCKET_FLAG_SERVER
                                   | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                   0);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 0.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Regular test: max_payload_size = 0 for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 0.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Edge test (success): max_payload_size = 1 for init */
  ws = NULL;
  ret = MHD_websocket_stream_init (&ws,
                                   MHD_WEBSOCKET_FLAG_SERVER
                                   | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                   1);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 1.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Edge test (success): max_payload_size = 1 for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    1,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 1.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Regular test: max_payload_size = 1000 for init */
  ws = NULL;
  ret = MHD_websocket_stream_init (&ws,
                                   MHD_WEBSOCKET_FLAG_SERVER
                                   | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                   1000);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 1000.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Regular test: max_payload_size = 1000 for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    1000,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 1000.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
#ifdef ENABLE_64BIT_TESTS
  /* Edge test (success): max_payload_size = 0x7FFFFFFFFFFFFFFF for init */
  ws = NULL;
  ret = MHD_websocket_stream_init (&ws,
                                   MHD_WEBSOCKET_FLAG_SERVER
                                   | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                   (uint64_t) 0x7FFFFFFFFFFFFFFF);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 0x7FFFFFFFFFFFFFFF.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Edge test (success): max_payload_size = 0x7FFFFFFFFFFFFFFF for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    (uint64_t) 0x7FFFFFFFFFFFFFFF,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 0x7FFFFFFFFFFFFFFF.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Edge test (fail): max_payload_size = 0x8000000000000000 for init */
  ws = NULL;
  ret = MHD_websocket_stream_init (&ws,
                                   MHD_WEBSOCKET_FLAG_SERVER
                                   | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                   (uint64_t) 0x8000000000000000);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 0x8000000000000000.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Edge test (fail): max_payload_size = 0x8000000000000000 for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    (uint64_t) 0x8000000000000000,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u for max_payload_size 0x8000000000000000.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
#endif

  /*
  ------------------------------------------------------------------------------
    Missing parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: websocket stream variable missing for init */
  ws = NULL;
  ret = MHD_websocket_stream_init (NULL,
                                   MHD_WEBSOCKET_FLAG_SERVER
                                   | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                   0);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Fail test: websocket stream variable missing for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (NULL,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Fail test: malloc missing for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    NULL,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Fail test: realloc missing for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    NULL,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Fail test: free missing for init2 */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    NULL,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Regular test: rng given for server mode (will be ignored) */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    test_rng);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Regular test: cls_rng given for server mode (will be ignored) */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_SERVER
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    (void *) 12345,
                                    test_rng);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Regular test: rng given for client mode */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_CLIENT
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    test_rng);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (NULL == ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }
  /* Fail test: rng not given for client mode */
  ws = NULL;
  ret = MHD_websocket_stream_init2 (&ws,
                                    MHD_WEBSOCKET_FLAG_CLIENT
                                    | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                    0,
                                    test_malloc,
                                    test_realloc,
                                    test_free,
                                    NULL,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != ws) )
  {
    fprintf (stderr,
             "Init test failed in line %u %u.\n",
             (unsigned int) __LINE__, ret);
    ++failed;
  }
  if (NULL != ws)
  {
    MHD_websocket_stream_free (ws);
    ws = NULL;
  }

  return failed != 0 ? 0x01 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_create_accept_header()`
 */
int
test_accept ()
{
  int failed = 0;
  char accept_key[29];
  int ret;

  /*
  ------------------------------------------------------------------------------
    accepting
  ------------------------------------------------------------------------------
  */
  /* Regular test: Test case from RFC6455 4.2.2 */
  memset (accept_key, 0, 29);
  ret = MHD_websocket_create_accept_header ("dGhlIHNhbXBsZSBub25jZQ==",
                                            accept_key);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (0 != memcmp (accept_key, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", 29)))
  {
    fprintf (stderr,
             "Accept test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Missing parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: missing sec-key value */
  memset (accept_key, 0, 29);
  ret = MHD_websocket_create_accept_header (NULL,
                                            accept_key);
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "Accept test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: missing accept variable */
  memset (accept_key, 0, 29);
  ret = MHD_websocket_create_accept_header ("dGhlIHNhbXBsZSBub25jZQ==",
                                            NULL);
  if (MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret)
  {
    fprintf (stderr,
             "Accept test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  return failed != 0 ? 0x02 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_decode()`
 */
int
test_decodes ()
{
  int failed = 0;
  char *buf1 = NULL, *buf2 = NULL;

  /*
  ------------------------------------------------------------------------------
    text frame
  ------------------------------------------------------------------------------
   */
  /* Regular test: Masked text frame from RFC 6455, must succeed for server */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58",
                                11,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Unmasked text frame from RFC 6455, must succeed for client */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x05\x48\x65\x6c\x6c\x6f",
                                7,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                7);
  /* Fail test: Unmasked text frame from RFC 6455, must fail for server */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x05\x48\x65\x6c\x6c\x6f",
                                7,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Masked text frame from RFC 6455, must fail for client */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Text frame with UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x90\x00\x00\x00\x00" "This is my n"
                                "\xC3\xB6" "te",
                                22,
                                "This is my n" "\xC3\xB6" "te",
                                16,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                22);
  /* Fail test: Text frame with with invalid UTF-8 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8F\x00\x00\x00\x00" "This is my n" "\xFF"
                                "te",
                                21,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                18);
  /* Fail test: Text frame with broken UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8F\x00\x00\x00\x00" "This is my n" "\xC3"
                                "te",
                                21,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                19);
  /* Regular test: Text frame without payload and mask (caller = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x80\x01\x02\x03\x04",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                6);
  /* Fail test: Text frame without payload and no mask (caller = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Text frame without payload and mask (caller = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                2);
  /* Fail test: Text frame without payload and no mask (caller = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x80\x01\x02\x03\x04",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);

  /*
  ------------------------------------------------------------------------------
    binary frame
  ------------------------------------------------------------------------------
  */
  /* Regular test: Masked binary frame (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58",
                                11,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Unmasked binary frame (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x05\x48\x65\x6c\x6c\x6f",
                                7,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                7);
  /* Fail test: Unmasked binary frame (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x05\x48\x65\x6c\x6c\x6f",
                                7,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Masked binary frame (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x85\x37\xfa\x21\x3d\x7f\x9f\x4d\x51\x58",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Binary frame without payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                6);
  /* Regular test: Fragmented binary frame without payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x80\x00\x00\x00\x00\x80\x80\x00\x00\x00\x00",
                                12,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                12);
  /* Regular test: Fragmented binary frame without payload, fragments to the caller, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x80\x00\x00\x00\x00\x80\x80\x00\x00\x00\x00",
                                12,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_BINARY_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                6);
  /* Regular test: Fragmented binary frame without payload, fragments to the caller, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x02\x80\x00\x00\x00\x00\x80\x80\x00\x00\x00\x00",
                                12,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_BINARY_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                12);
  /* Regular test: Fragmented binary frame with payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x80\x83\x00\x00\x00\x00\x04\x05\x06",
                                18,
                                "\x01\x02\x03\x04\x05\x06",
                                6,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                18);
  /* Regular test: Fragmented binary frame with payload, fragments to the caller, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x80\x83\x00\x00\x00\x00\x04\x05\x06",
                                18,
                                "\x01\x02\x03",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                9);
  /* Regular test: Fragmented binary frame without payload, fragments to the caller, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x80\x83\x00\x00\x00\x00\x04\x05\x06",
                                18,
                                "\x04\x05\x06",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                18);
  /* Regular test: Fragmented binary frame with payload, fragments to the caller, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x00\x83\x00\x00\x00\x00\x04\x05\x06\x00\x83\x00\x00\x00\x00\x07\x08\x09\x80\x83\x00\x00\x00\x00\x0A\x0B\x0C",
                                36,
                                "\x01\x02\x03",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                9);
  /* Regular test: Fragmented binary frame without payload, fragments to the caller, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x00\x83\x00\x00\x00\x00\x04\x05\x06\x00\x83\x00\x00\x00\x00\x07\x08\x09\x80\x83\x00\x00\x00\x00\x0A\x0B\x0C",
                                36,
                                "\x04\x05\x06",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_NEXT_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                18);
  /* Regular test: Fragmented binary frame without payload, fragments to the caller, 3rd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                3,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x00\x83\x00\x00\x00\x00\x04\x05\x06\x00\x83\x00\x00\x00\x00\x07\x08\x09\x80\x83\x00\x00\x00\x00\x0A\x0B\x0C",
                                36,
                                "\x07\x08\x09",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_NEXT_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                27);
  /* Regular test: Fragmented binary frame without payload, fragments to the caller, 4th call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                4,
                                0,
                                "\x02\x83\x00\x00\x00\x00\x01\x02\x03\x00\x83\x00\x00\x00\x00\x04\x05\x06\x00\x83\x00\x00\x00\x00\x07\x08\x09\x80\x83\x00\x00\x00\x00\x0A\x0B\x0C",
                                36,
                                "\x0A\x0B\x0C",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                36);
  /* Regular test: Binary frame with bytes which look like invalid UTF-8 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x85\x00\x00\x00\x00" "Hell\xf6",
                                11,
                                "Hell\xf6",
                                5,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Binary frame with bytes which look like broken UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x85\x00\x00\x00\x00" "H\xC3llo",
                                11,
                                "H\xC3llo",
                                5,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Binary frame with bytes which look like valid UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x82\x85\x00\x00\x00\x00" "H\xC3\xA4lo",
                                11,
                                "H\xC3\xA4lo",
                                5,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Fragmented binary frame with bytes which look like valid UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x82\x00\x00\x00\x00" "H\xC3"
                                "\x80\x83\x00\x00\x00\x00" "\xA4lo",
                                17,
                                "H\xC3\xA4lo",
                                5,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented binary frame with bytes which look like valid UTF-8 sequence,
     fragments to the caller, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x02\x82\x00\x00\x00\x00" "H\xC3"
                                "\x80\x83\x00\x00\x00\x00" "\xA4lo",
                                17,
                                "H\xC3",
                                2,
                                MHD_WEBSOCKET_STATUS_BINARY_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                8);
  /* Regular test: Fragmented binary frame with bytes which look like valid UTF-8 sequence,
     fragments to the caller, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x02\x82\x00\x00\x00\x00" "H\xC3"
                                "\x80\x83\x00\x00\x00\x00" "\xA4lo",
                                17,
                                "\xA4lo",
                                3,
                                MHD_WEBSOCKET_STATUS_BINARY_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);

  /*
  ------------------------------------------------------------------------------
    close frame
  ------------------------------------------------------------------------------
  */
  /* Regular test: Close frame with no payload but with mask (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                6);
  /* Regular test: Close frame with no payload (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                2);
  /* Fail test: Close frame with no payload and no mask (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Close frame with no payload but with mask (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Close frame with 2 byte payload for close reason */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x82\x00\x00\x00\x00\x03\xEB",
                                8,
                                "\x03\xEB",
                                2,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                8);
  /* Fail test: Close frame with 1 byte payload (no valid close reason) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x81\x00\x00\x00\x00\x03",
                                7,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Close frame with close reason and UTF-8 description */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x95\x00\x00\x00\x00\x03\xEB"
                                "Something was wrong",
                                27,
                                "\x03\xEB" "Something was wrong",
                                21,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                27);
  /* Regular test: Close frame with close reason and UTF-8 description (with UTF-8 sequence) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x96\x00\x00\x00\x00\x03\xEB"
                                "Something was wr" "\xC3\xB6" "ng",
                                28,
                                "\x03\xEB" "Something was wr" "\xC3\xB6" "ng",
                                22,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                28);
  /* Fail test: Close frame with close reason and invalid UTF-8 in description */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x95\x00\x00\x00\x00\x03\xEB"
                                "Something was wr" "\xFF" "ng",
                                27,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                24);
  /* Fail test: Close frame with close reason and broken UTF-8 sequence in description */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x95\x00\x00\x00\x00\x03\xEB"
                                "Something was wr" "\xC3" "ng",
                                27,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                25);
  /* Edge test (success): Close frame with 125 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\xFD\x00\x00\x00\x00\x03\xEB"
                                "Something was wrong, so I decided to close this websocket. I hope you are not angry. But this is also the 123 cap test. :-)",
                                131,
                                "\x03\xEB"
                                "Something was wrong, so I decided to close this websocket. I hope you are not angry. But this is also the 123 cap test. :-)",
                                125,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                131);
  /* Edge test (failure): Close frame with 126 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\xFE\x00\x7e\x00\x00\x00\x00\x03\xEB"
                                "Something was wrong, so I decided to close this websocket. I hope you are not angry. But this is also the 123 cap test. >:-)",
                                134,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Close frame with 500 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\xFE\x01\xf4\x00\x00\x00\x00\x03\xEB"
                                "The payload of this test isn't parsed.",
                                49,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Edge test (failure): Close frame with 65535 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\xFE\xff\xff\x00\x00\x00\x00\x03\xEB"
                                "The payload of this test isn't parsed.",
                                49,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Edge test (failure): Close frame with 65536 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\xFF\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x03\xEB"
                                "The payload of this test isn't parsed.",
                                54,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Close frame with 1000000 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\xFF\x00\x00\x00\x00\x00\x0F\x42\x40\x00\x00\x00\x00\x03\xEB"
                                "The payload of this test isn't parsed.",
                                54,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);

  /*
  ------------------------------------------------------------------------------
    ping frame
  ------------------------------------------------------------------------------
  */
  /* Regular test: Ping frame with no payload but with mask (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                6);
  /* Regular test: Ping frame with no payload (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                2);
  /* Fail test: Ping frame with no payload and no mask (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Ping frame with no payload but with mask (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Ping frame with some (masked) payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x88\x01\x20\x03\x40\xFF\xFF\xFF\xFF\x00\x00\x00\x00",
                                14,
                                "\xFE\xDF\xFC\xBF\x01\x20\x03\x40",
                                8,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                14);
  /* Edge test (success): Ping frame with one byte of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x81\x00\x00\x00\x00" "a",
                                7,
                                "a",
                                1,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                7);
  /* Edge test (success): Ping frame with 125 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\xFD\x00\x00\x00\x00"
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                131,
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                125,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                131);
  /* Edge test (fail): Ping frame with 126 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\xFE\x00\x7E\x00\x00\x00\x00"
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                134,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Ping frame with UTF-8 data */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x90\x00\x00\x00\x00" "Ping is bin"
                                "\xC3\xA4" "ry.",
                                22,
                                "Ping is bin" "\xC3\xA4" "ry.",
                                16,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                22);
  /* Regular test: Ping frame with invalid UTF-8 data */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x8F\x00\x00\x00\x00" "Ping is bin" "\xFF"
                                "ry.",
                                21,
                                "Ping is bin" "\xFF" "ry.",
                                15,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                21);
  /* Regular test: Ping frame with broken UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x8F\x00\x00\x00\x00" "Ping is bin" "\xC3"
                                "ry.",
                                21,
                                "Ping is bin" "\xC3" "ry.",
                                15,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                21);

  /*
  ------------------------------------------------------------------------------
    pong frame
  ------------------------------------------------------------------------------
  */
  /* Regular test: Pong frame with no payload but with mask (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                6);
  /* Regular test: Pong frame with no payload (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                2);
  /* Fail test: Pong frame with no payload and no mask (decoder = server) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x00",
                                2,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Fail test: Pong frame with no payload but with mask (decoder = client) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_CLIENT
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Pong frame with some (masked) payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x88\x01\x20\x03\x40\xFF\xFF\xFF\xFF\x00\x00\x00\x00",
                                14,
                                "\xFE\xDF\xFC\xBF\x01\x20\x03\x40",
                                8,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                14);
  /* Edge test (success): Pong frame with one byte of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x81\x00\x00\x00\x00" "a",
                                7,
                                "a",
                                1,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                7);
  /* Edge test (success): Pong frame with 125 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\xFD\x00\x00\x00\x00"
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                131,
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                125,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                131);
  /* Edge test (fail): Pong frame with 126 bytes of payload */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\xFE\x00\x7E\x00\x00\x00\x00"
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                134,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                1);
  /* Regular test: Pong frame with UTF-8 data */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x90\x00\x00\x00\x00" "Pong is bin"
                                "\xC3\xA4" "ry.",
                                22,
                                "Pong is bin" "\xC3\xA4" "ry.",
                                16,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                22);
  /* Regular test: Pong frame with invalid UTF-8 data */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x8F\x00\x00\x00\x00" "Pong is bin" "\xFF"
                                "ry.",
                                21,
                                "Pong is bin" "\xFF" "ry.",
                                15,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                21);
  /* Regular test: Pong frame with broken UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8A\x8F\x00\x00\x00\x00" "Pong is bin" "\xC3"
                                "ry.",
                                21,
                                "Pong is bin" "\xC3" "ry.",
                                15,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                21);

  /*
  ------------------------------------------------------------------------------
    fragmentation
  ------------------------------------------------------------------------------
  */
  /* Regular test: Fragmented, masked text frame, we are the server and don't want fragments as caller */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x80\x82\x3d\x37\xfa\x21\x51\x58",
                                17,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented, masked text frame, we are the server and don't want fragments as caller, but call decode two times */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x80\x82\x3d\x37\xfa\x21\x51\x58",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented, masked text frame, we are the server and want fragments, one call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x80\x82\x3d\x37\xfa\x21\x51\x58",
                                17,
                                "Hel",
                                3,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                9);
  /* Regular test: Fragmented, masked text frame, we are the server and want fragments, second call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x80\x82\x3d\x37\xfa\x21\x51\x58",
                                17,
                                "lo",
                                2,
                                MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented, masked text frame, we are the server and want fragments, third call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                3,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x80\x82\x3d\x37\xfa\x21\x51\x58",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented, masked text frame, we are the server and want fragments, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x00\x81\x3d\x37\xfa\x21\x51\x80\x81\x37\x37\xfa\x21\x58",
                                23,
                                "Hel",
                                3,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                9);
  /* Regular test: Fragmented, masked text frame, we are the server and want fragments, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x00\x81\x3d\x37\xfa\x21\x51\x80\x81\x37\x37\xfa\x21\x58",
                                23,
                                "l",
                                1,
                                MHD_WEBSOCKET_STATUS_TEXT_NEXT_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Regular test: Fragmented, masked text frame, we are the server and want fragments, 3rd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                3,
                                0,
                                "\x01\x83\x37\xfa\x21\x3d\x7f\x9f\x4d\x00\x81\x3d\x37\xfa\x21\x51\x80\x81\x37\x37\xfa\x21\x58",
                                23,
                                "o",
                                1,
                                MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                23);


  /*
  ------------------------------------------------------------------------------
    invalid flags
  ------------------------------------------------------------------------------
  */
  /* Regular test: Template with valid data for the next tests (this one must succeed) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00Hello",
                                11,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Fail test: RSV1 flag set */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x91\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: RSV2 flag set */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\xA1\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: RSV3 flag set */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\xC1\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);

  /*
  ------------------------------------------------------------------------------
    invalid opcodes
  ------------------------------------------------------------------------------
  */
  /* Fail test: Invalid opcode 0 (0 is usually valid, but only if there was a data frame before) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x80\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 3 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x83\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 4 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x84\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 5 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x85\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 6 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x86\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 7 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x87\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 0x0B */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8B\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 0x0C */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8c\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 0x0D */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8d\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 0x0E */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8e\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Invalid opcode 0x0F */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x8f\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);


  /*
  ------------------------------------------------------------------------------
    control frames without FIN flag
  ------------------------------------------------------------------------------
  */
  /* Fail test: Close frame without FIN flag */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x08\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Ping frame without FIN flag */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x09\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Fail test: Pong frame without FIN flag */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x0a\x85\x00\x00\x00\x00Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);

  /*
  ------------------------------------------------------------------------------
    length checks (without max_payload_len)
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): 0 bytes of payload (requires 1 byte length) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x80\x00\x00\x00\x00",
                                6,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                6);
  /* Edge test (success): 1 byte of payload (requires 1 byte length) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x81\x00\x00\x00\x00" "a",
                                7,
                                "a",
                                1,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                7);
  /* Edge test (success): 125 bytes of payload (requires 1 byte length) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xfd\x00\x00\x00\x00"
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                131,
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                125,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                131);
  /* Edge test (success): 126 bytes of payload (requires 2 byte length) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xfe\x00\x7e\x00\x00\x00\x00"
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                134,
                                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                126,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                134);
  /* Edge test (success): 65535 bytes of payload (requires 2 byte length) */
  allocate_length_test_data (&buf1,
                             &buf2,
                             65535,
                             "\x81\xfe\xff\xff\x00\x00\x00\x00",
                             8);
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                buf1,
                                65535 + 8,
                                buf2,
                                65535,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                65535 + 8);
  /* Edge test (success): 65536 bytes of payload (requires 8 byte length) */
  allocate_length_test_data (&buf1,
                             &buf2,
                             65536,
                             "\x81\xff\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00",
                             14);
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                buf1,
                                65536 + 14,
                                buf2,
                                65536,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                65536 + 14);
  /* Regular test: 1 MB of payload */
  allocate_length_test_data (&buf1,
                             &buf2,
                             1048576,
                             "\x81\xff\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00",
                             14);
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                buf1,
                                1048576 + 14,
                                buf2,
                                1048576,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                1048576 + 14);
  /* Regular test: 100 MB of payload */
  allocate_length_test_data (&buf1,
                             &buf2,
                             104857600,
                             "\x81\xff\x00\x00\x00\x00\x06\x40\x00\x00\x00\x00\x00\x00",
                             14);
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                buf1,
                                104857600 + 14,
                                buf2,
                                104857600,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                104857600 + 14);
  if (NULL != buf1)
  {
    free (buf1);
    buf1 = NULL;
  }
  if (NULL != buf2)
  {
    free (buf2);
    buf2 = NULL;
  }
#ifdef ENABLE_64BIT_TESTS
  /* Edge test (success): Maximum allowed length (here is only the header checked) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xff\x7f\xff\xff\xff\xff\xff\xff\xff",
                                10,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                10);
#else
  /* Edge test (fail): Maximum allowed length
     (the size is allowed, but the system cannot handle this amount of memory) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xff\x7f\xff\xff\xff\xff\xff\xff\xff",
                                10,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);
#endif
  /* Edge test (fail): Too big payload length */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xff\x80\x00\x00\x00\x00\x00\x00\x00",
                                10,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);
  /* Edge test (fail): Too big payload length */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xff\xff\xff\xff\xff\xff\xff\xff\xff",
                                10,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);
  /* Fail test: Not the smallest payload length syntax used (2 byte instead of 1 byte) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xfe\x00\x05\x00\x00\x00\x00" "abcde",
                                13,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                4);
  /* Fail test: Not the smallest payload length syntax used (8 byte instead of 1 byte) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xff\x00\x00\x00\x00\x00\x00\x00\x05\x00\x00\x00\x00"
                                "abcde",
                                13,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);
  /* Fail test: Not the smallest payload length syntax used (8 byte instead of 2 byte) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\xff\x00\x00\x00\x00\x00\x00\xff\xff\x00\x00\x00\x00"
                                "abcde",
                                13,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);

  /*
  ------------------------------------------------------------------------------
    length checks (with max_payload_len)
  ------------------------------------------------------------------------------
  */
  /* Regular test: Frame with less payload than specified as limit */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                100,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00" "Hello",
                                11,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Edge test (success): Frame with the same payload as the specified limit */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                5,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00" "Hello",
                                11,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Edge test (fail): Frame with more payload than specified as limit */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                4,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00" "Hello",
                                11,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                2);
  /* Regular test: Fragmented frames with the sum of payload less than specified as limit */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                100,
                                1,
                                0,
                                "\x01\x83\x00\x00\x00\x00"
                                "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                17,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Edge test (success): Fragmented frames with the sum of payload equal to the specified limit */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                5,
                                1,
                                0,
                                "\x01\x83\x00\x00\x00\x00"
                                "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                17,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Edge test (fail): Fragmented frames with the sum of payload more than specified as limit */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                4,
                                1,
                                0,
                                "\x01\x83\x00\x00\x00\x00"
                                "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                15);
  /* Edge test (success): Fragmented frames with the sum of payload greater than
     the specified limit, but we take fragments (one call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                5,
                                1,
                                0,
                                "\x01\x83\x00\x00\x00\x00"
                                "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                17,
                                "Hel",
                                3,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                9);
  /* Edge test (success): Fragmented frames with the sum of payload greater than
     the specified limit, but we take fragments (two calls) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                5,
                                2,
                                0,
                                "\x01\x83\x00\x00\x00\x00"
                                "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                17,
                                "lo",
                                2,
                                MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);

  /*
  ------------------------------------------------------------------------------
    UTF-8 sequences
  ------------------------------------------------------------------------------
  */
  /* Regular test: No UTF-8 characters  */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 a        ",
                                16,
                                " a        ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Fail test: A UTF-8 tail character without sequence start character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xA4        ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Regular test: A two byte UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xC3\xA4       ",
                                16,
                                " \xC3\xA4       ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Fail test: A broken two byte UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xC3        ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Fail test: A two byte UTF-8 sequence with one UTF-8 tail too much */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xC3\xA4\xA4      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                9);
  /* Regular test: A three byte UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF\x8F\x8F      ",
                                16,
                                " \xEF\x8F\x8F      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Fail test: A broken byte UTF-8 sequence (two of three bytes) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF\x8F       ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                9);
  /* Fail test: A broken byte UTF-8 sequence (one of three bytes) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF        ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Fail test: A three byte UTF-8 sequence followed by one UTF-8 tail byte */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF\x8F\x8F\x8F     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);
  /* Regular test: A four byte UTF-8 sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF2\x8F\x8F\x8F     ",
                                16,
                                " \xF2\x8F\x8F\x8F     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Fail test: A broken four byte UTF-8 sequence (three of four bytes) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF2\x8F\x8F      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                10);
  /* Fail test: A broken four byte UTF-8 sequence (two of four bytes) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF2\x8F       ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                9);
  /* Fail test: A broken four byte UTF-8 sequence (one of four bytes) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF2        ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Fail test: A four byte UTF-8 sequence followed by UTF-8 tail */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF2\x8F\x8F\x8F\x8F    ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                11);
  /* Fail test: A five byte UTF-8 sequence (only up to four bytes allowed) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xFB\x8F\x8F\x8F\x8F    ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Fail test: A six byte UTF-8 sequence (only up to four bytes allowed) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xFD\x8F\x8F\x8F\x8F\x8F   ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Fail test: A seven byte UTF-8 sequence (only up to four bytes allowed) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xFE\x8F\x8F\x8F\x8F\x8F\x8F  ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Fail test: A eight byte UTF-8 sequence (only up to four bytes allowed) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xFF\x8F\x8F\x8F\x8F\x8F\x8F\x8F ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Edge test (success): The maximum allowed UTF-8 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF4\x8F\xBF\xBF     ",
                                16,
                                " \xF4\x8F\xBF\xBF     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The maximum allowed UTF-8 character + 1 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF4\x90\x80\x80     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The last valid UTF8-1 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \x7F        ",
                                16,
                                " \x7F        ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the last valid UTF8-1 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \x80        ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Edge test (fail): The value before the first valid UTF8-2 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xC1\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Edge test (success): The first valid UTF8-2 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xC2\x80       ",
                                16,
                                " \xC2\x80       ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-2 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xDF\xBF       ",
                                16,
                                " \xDF\xBF       ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the lst valid UTF8-2 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xE0\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (fail): The value before the first valid UTF8-3 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xE0\x9F\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The first valid UTF8-3 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xE0\xA0\x80      ",
                                16,
                                " \xE0\xA0\x80      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-3 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xE0\xBF\xBF      ",
                                16,
                                " \xE0\xBF\xBF      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the first valid UTF8-3 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xE0\xC0\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The first valid UTF8-3 character (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xE1\x80\x80      ",
                                16,
                                " \xE1\x80\x80      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-3 character (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEC\xBF\xBF      ",
                                16,
                                " \xEC\xBF\xBF      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the last valid UTF8-3 character (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEC\xC0\xBF      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (fail): The value before the first valid UTF8-3 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xED\x7F\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The first valid UTF8-3 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xED\x80\x80      ",
                                16,
                                " \xED\x80\x80      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-3 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xED\x9F\xBF      ",
                                16,
                                " \xED\x9F\xBF      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the last valid UTF8-3 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xED\xA0\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (fail): The value before the first valid UTF8-3 character (tail 4) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEE\x7F\x80      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The first valid UTF8-3 character (tail 4) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEE\x80\x80      ",
                                16,
                                " \xEE\x80\x80      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-3 character (tail 4) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF\xBF\xBF      ",
                                16,
                                " \xEF\xBF\xBF      ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the last valid UTF8-3 character (tail 4) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF\xBF\xC0      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                9);
  /* Edge test (fail): The value after the last valid UTF8-3 character (tail 4) #2 */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xEF\xC0\xBF      ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (fail): The value before the first valid UTF8-4 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF0\x8F\x80\x80     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The first valid UTF8-4 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF0\x90\x80\x80     ",
                                16,
                                " \xF0\x90\x80\x80     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-4 character (tail 1) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF0\xBF\xBF\xBF     ",
                                16,
                                " \xF0\xBF\xBF\xBF     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The first valid UTF8-4 character (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF1\x80\x80\x80     ",
                                16,
                                " \xF1\x80\x80\x80     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-4 character (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF3\xBF\xBF\xBF     ",
                                16,
                                " \xF3\xBF\xBF\xBF     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): A value before the last valid UTF8-4 character in the second byte (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF3\x7F\x80\x80     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (fail): A value after the last valid UTF8-4 character in the second byte (tail 2) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF3\xC0\x80\x80     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (success): The first valid UTF8-4 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF4\x80\x80\x80     ",
                                16,
                                " \xF4\x80\x80\x80     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (success): The last valid UTF8-4 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF4\x8F\xBF\xBF     ",
                                16,
                                " \xF4\x8F\xBF\xBF     ",
                                10,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                16);
  /* Edge test (fail): The value after the last valid UTF8-4 character (tail 3) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF4\x90\x80\x80     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                8);
  /* Edge test (fail): The first byte value the last valid UTF8-4 character */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x8A\x00\x00\x00\x00 \xF5\x90\x80\x80     ",
                                16,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);

  /*
  ------------------------------------------------------------------------------
    Unfinished UTF-8 sequence between fragmented text frame
  ------------------------------------------------------------------------------
  */
  /* Regular test: UTF-8 sequence between fragments, no fragmentation for the caller */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x8D\x00\x00\x00\x00" "This is my n"
                                "\xC3\x80\x83\x00\x00\x00\x00\xB6" "te",
                                28,
                                "This is my n" "\xC3\xB6" "te",
                                16,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                28);
  /* Regular test: UTF-8 sequence between fragments, fragmentation for the caller, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x8D\x00\x00\x00\x00" "This is my n"
                                "\xC3\x80\x83\x00\x00\x00\x00\xB6" "te",
                                28,
                                "This is my n",
                                12,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                19);
  /* Regular test: UTF-8 sequence between fragments, fragmentation for the caller, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x8D\x00\x00\x00\x00" "This is my n"
                                "\xC3\x80\x83\x00\x00\x00\x00\xB6" "te",
                                28,
                                "\xC3\xB6" "te",
                                4,
                                MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                28);
  /* Edge test (success): UTF-8 sequence between fragments, but nothing before, fragmentation for the caller, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x81\x00\x00\x00\x00\xC3\x80\x81\x00\x00\x00\x00\xB6",
                                14,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                7);
  /* Edge test (success): UTF-8 sequence between fragments, but nothing before, fragmentation for the caller, 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x81\x00\x00\x00\x00\xC3\x80\x81\x00\x00\x00\x00\xB6",
                                14,
                                "\xC3\xB6",
                                2,
                                MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                14);

  /*
  ------------------------------------------------------------------------------
    Decoding with broken stream
  ------------------------------------------------------------------------------
  */
  /* Failure test: Invalid sequence */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\xFF\x81\x85\x00\x00\x00\x00" "Hello",
                                12,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Failure test: Call after invalidated stream */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\xFF\x81\x85\x00\x00\x00\x00" "Hello",
                                12,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_STREAM_BROKEN,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Failure test: Call after invalidated stream (but with different buffer) */
  {
    struct MHD_WebSocketStream *ws;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init (&ws,
                                                              MHD_WEBSOCKET_FLAG_SERVER
                                                              |
                                                              MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                              0))
    {
      size_t streambuf_read_len = 0;
      char *payload = NULL;
      size_t payload_len = 0;
      int ret = 0;
      ret = MHD_websocket_decode (ws,
                                  "\xFF",
                                  1,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if (MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR != ret)
      {
        fprintf (stderr,
                 "Test failed in line %u: The return value should be -1, but is %d\n",
                 (unsigned int) __LINE__,
                 (int) ret);
        ++failed;
      }
      else
      {
        ret = MHD_websocket_decode (ws,
                                    "\x81\x85\x00\x00\x00\x00" "Hello",
                                    11,
                                    &streambuf_read_len,
                                    &payload,
                                    &payload_len);
        if (MHD_WEBSOCKET_STATUS_STREAM_BROKEN != ret)
        {
          fprintf (stderr,
                   "Test failed in line %u: The return value should be -2, but is %d\n",
                   (unsigned int) __LINE__,
                   (int) ret);
          ++failed;
        }
      }
      MHD_websocket_stream_free (ws);
    }
    else
    {
      fprintf (stderr,
               "Individual test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  /*
  ------------------------------------------------------------------------------
    frame after close frame
  ------------------------------------------------------------------------------
  */
  /* Regular test: Close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x81\x85\x00\x00\x00\x00"
                                "Hello",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                6);
  /* Failure test: Text frame after close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x81\x85\x00\x00\x00\x00"
                                "Hello",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                6);
  /* Failure test: Binary frame after close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x82\x85\x00\x00\x00\x00"
                                "Hello",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                6);
  /* Failure test: Continue frame after close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x80\x85\x00\x00\x00\x00"
                                "Hello",
                                17,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                6);
  /* Regular test: Ping frame after close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x89\x85\x00\x00\x00\x00"
                                "Hello",
                                17,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                17);
  /* Regular test: Pong frame after close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x8A\x85\x00\x00\x00\x00"
                                "Hello",
                                17,
                                "Hello",
                                5,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                17);
  /* Regular test: Close frame after close frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x88\x80\x00\x00\x00\x00\x88\x80\x00\x00\x00\x00",
                                12,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                12);

  /*
  ------------------------------------------------------------------------------
    decoding byte-by-byte
  ------------------------------------------------------------------------------
  */
  /* Regular test: Text frame, 2 bytes per loop, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                2,
                                "\x81\x91\x01\x02\x04\x08" "Ujm{!kw(uja(ugw|/",
                                23,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                2);
  /* Regular test: Text frame, 2 bytes per loop, 11th call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                11,
                                2,
                                "\x81\x91\x01\x02\x04\x08" "Ujm{!kw(uja(ugw|/",
                                23,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                22);
  /* Regular test: Text frame, 2 bytes per loop, 12th call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                12,
                                2,
                                "\x81\x91\x01\x02\x04\x08" "Ujm{!kw(uja(ugw|/",
                                23,
                                "This is the test.",
                                17,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                23);
  /* Regular test: Text frame, 1 byte per loop, 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                1,
                                "\x81\x91\x01\x02\x04\x08" "Ujm{!kw(uja(ugw|/",
                                23,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                1);
  /* Regular test: Text frame, 1 byte per loop, 22nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                22,
                                1,
                                "\x81\x91\x01\x02\x04\x08" "Ujm{!kw(uja(ugw|/",
                                23,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_OK,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                22);
  /* Regular test: Text frame, 1 byte per loop, 23rd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                23,
                                1,
                                "\x81\x91\x01\x02\x04\x08" "Ujm{!kw(uja(ugw|/",
                                23,
                                "This is the test.",
                                17,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                23);

  /*
  ------------------------------------------------------------------------------
    mix of fragmented data frames and control frames
  ------------------------------------------------------------------------------
  */
  /* Regular test: Fragmented text frame mixed with one ping frame (1st call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x89\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented text frame mixed with one ping frame (2nd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x89\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                "This is the test.",
                                17,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                35);
  /* Regular test: Fragmented text frame mixed with one close frame (1st call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x88\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                17);
  /* Fail test: Fragmented text frame mixed with one ping frame (2nd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x88\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                17);
  /* Regular test: Fragmented text frame mixed with one ping frame, the caller wants fragments (1st call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x89\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                "This ",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Fragmented text frame mixed with one ping frame, the caller wants fragments (2nd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x89\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                17);
  /* Regular test: Fragmented text frame mixed with one ping frame, the caller wants fragments (3rd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                3,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x89\x80\x00\x00\x00\x00"
                                "\x80\x8C\x00\x00\x00\x00" "is the test.",
                                35,
                                "is the test.",
                                12,
                                MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                35);

  /*
  ------------------------------------------------------------------------------
    mix of fragmented data frames and data frames
  ------------------------------------------------------------------------------
  */
  /* Fail test: Fragmented text frame mixed with one non-fragmented binary frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x82\x81\x00\x00\x00\x00"
                                "a\x80\x8C\x00\x00\x00\x00" "is the test.",
                                36,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                11);
  /* Regular test: Fragmented text frame mixed with one non-fragmented binary frame; the caller wants fragments; 1st call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x82\x81\x00\x00\x00\x00"
                                "a\x80\x8C\x00\x00\x00\x00" "is the test.",
                                36,
                                "This ",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Fail test: Fragmented text frame mixed with one non-fragmented binary frame; the caller wants fragments; 2nd call */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x82\x81\x00\x00\x00\x00"
                                "a\x80\x8C\x00\x00\x00\x00" "is the test.",
                                36,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                11);
  /* Fail test: Fragmented text frame mixed with one fragmented binary frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x02\x81\x00\x00\x00\x00"
                                "a\x80\x8C\x00\x00\x00\x00" "is the test.",
                                36,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                11);
  /* Fail test: Fragmented text frame, continue frame, non-fragmented binary frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x00\x8C\x00\x00\x00\x00"
                                "is the test.\x82\x81\x00\x00\x00\x00" "a",
                                36,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                29);
  /* Fail test: Fragmented text frame, continue frame, fragmented binary frame */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x01\x85\x00\x00\x00\x00"
                                "This \x00\x8C\x00\x00\x00\x00"
                                "is the test.\x02\x81\x00\x00\x00\x00" "a",
                                36,
                                NULL,
                                0,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                29);

  /*
  ------------------------------------------------------------------------------
    multiple data frames
  ------------------------------------------------------------------------------
  */
  /* Regular test: Text frame, binary frame, text frame (1st call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00"
                                "This \x82\x87\x00\x00\x00\x00"
                                "is the \x81\x85\x00\x00\x00\x00" "test.",
                                35,
                                "This ",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Text frame, binary frame, text frame (2nd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x81\x85\x00\x00\x00\x00"
                                "This \x82\x87\x00\x00\x00\x00"
                                "is the \x81\x85\x00\x00\x00\x00" "test.",
                                35,
                                "is the ",
                                7,
                                MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                24);
  /* Regular test: Text frame, binary frame, text frame (3rd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                3,
                                0,
                                "\x81\x85\x00\x00\x00\x00"
                                "This \x82\x87\x00\x00\x00\x00"
                                "is the \x81\x85\x00\x00\x00\x00" "test.",
                                35,
                                "test.",
                                5,
                                MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                35);
  /*
  ------------------------------------------------------------------------------
    multiple control frames
  ------------------------------------------------------------------------------
  */
  /* Regular test: Ping frame, pong frame, close frame (1st call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                1,
                                0,
                                "\x89\x85\x00\x00\x00\x00"
                                "This \x8A\x87\x00\x00\x00\x00"
                                "is the \x88\x85\x00\x00\x00\x00" "test.",
                                35,
                                "This ",
                                5,
                                MHD_WEBSOCKET_STATUS_PING_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                11);
  /* Regular test: Ping frame, pong frame, close frame (2nd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                2,
                                0,
                                "\x89\x85\x00\x00\x00\x00"
                                "This \x8A\x87\x00\x00\x00\x00"
                                "is the \x88\x85\x00\x00\x00\x00" "test.",
                                35,
                                "is the ",
                                7,
                                MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                MHD_WEBSOCKET_VALIDITY_VALID,
                                24);
  /* Regular test: Ping frame, pong frame, close frame (3rd call) */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                0,
                                3,
                                0,
                                "\x89\x85\x00\x00\x00\x00"
                                "This \x8A\x87\x00\x00\x00\x00"
                                "is the \x88\x85\x00\x00\x00\x00" "test.",
                                35,
                                "test.",
                                5,
                                MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                35);

  /*
  ------------------------------------------------------------------------------
    generated close frames for errors
  ------------------------------------------------------------------------------
  */
  /* Regular test: Close frame generated for protocol error */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS
                                |
                                MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR,
                                0,
                                1,
                                0,
                                "\xFF",
                                1,
                                "\x88\x02\x03\xEA",
                                4,
                                MHD_WEBSOCKET_STATUS_PROTOCOL_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                0);
  /* Regular test: Close frame generated for UTF-8 sequence error */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS
                                |
                                MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR,
                                0,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00T\xFFst.",
                                11,
                                "\x88\x02\x03\xEF",
                                4,
                                MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                7);
  /* Regular test: Close frame generated for message size exceeded */
  failed += test_decode_single (__LINE__,
                                MHD_WEBSOCKET_FLAG_SERVER
                                | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS
                                |
                                MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR,
                                3,
                                1,
                                0,
                                "\x81\x85\x00\x00\x00\x00T\xFFst.",
                                11,
                                "\x88\x02\x03\xF1",
                                4,
                                MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED,
                                MHD_WEBSOCKET_VALIDITY_INVALID,
                                2);

  /*
  ------------------------------------------------------------------------------
    terminating NUL character
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *ws;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init (&ws,
                                                              MHD_WEBSOCKET_FLAG_SERVER
                                                              |
                                                              MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                              0))
    {
      size_t streambuf_read_len = 0;
      char *payload = NULL;
      size_t payload_len = 0;
      int ret = 0;

      /* Regular test: text frame */
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00" "Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_TEXT_FRAME != ret) ||
          (5 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("Hello", payload, 5 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      /* Regular test: text frame fragment */
      ret = MHD_websocket_decode (ws,
                                  "\x01\x83\x00\x00\x00\x00"
                                  "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                  17,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_TEXT_FRAME != ret) ||
          (5 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("Hello", payload, 5 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      /* Regular test: binary frame */
      ret = MHD_websocket_decode (ws,
                                  "\x82\x85\x00\x00\x00\x00" "Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_BINARY_FRAME != ret) ||
          (5 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("Hello", payload, 5 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      /* Regular test: binary frame fragment */
      ret = MHD_websocket_decode (ws,
                                  "\x02\x83\x00\x00\x00\x00"
                                  "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                  17,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_BINARY_FRAME != ret) ||
          (5 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("Hello", payload, 5 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      MHD_websocket_stream_free (ws);
    }
    else
    {
      fprintf (stderr,
               "Individual decode test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }
  {
    struct MHD_WebSocketStream *ws;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init (&ws,
                                                              MHD_WEBSOCKET_FLAG_SERVER
                                                              |
                                                              MHD_WEBSOCKET_FLAG_WANT_FRAGMENTS,
                                                              0))
    {
      size_t streambuf_read_len = 0;
      char *payload = NULL;
      size_t payload_len = 0;
      int ret = 0;

      /* Regular test: text frame fragment (caller wants fragment, 1st call) */
      ret = MHD_websocket_decode (ws,
                                  "\x01\x83\x00\x00\x00\x00"
                                  "Hel\x80\x82\x00\x00\x00\x00" "lo",
                                  17,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT != ret) ||
          (3 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("Hel", payload, 3 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      /* Regular test: text frame fragment (caller wants fragment, 2nd call) */
      ret = MHD_websocket_decode (ws,
                                  "\x01\x83\x00\x00\x00\x00"
                                  "Hel\x80\x82\x00\x00\x00\x00" "lo"
                                  + streambuf_read_len,
                                  17 - streambuf_read_len,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT != ret) ||
          (2 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("lo", payload, 2 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      /* Regular test: text frame fragment with broken UTF-8 sequence (caller wants fragment, 1st call) */
      ret = MHD_websocket_decode (ws,
                                  "\x01\x83\x00\x00\x00\x00"
                                  "He\xC3\x80\x82\x00\x00\x00\x00" "\xB6o",
                                  17,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_TEXT_FIRST_FRAGMENT != ret) ||
          (2 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("He", payload, 2 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      /* Regular test: text frame fragment with broken UTF-8 sequence (caller wants fragment, 2nd call) */
      ret = MHD_websocket_decode (ws,
                                  "\x01\x83\x00\x00\x00\x00"
                                  "He\xC3\x80\x82\x00\x00\x00\x00" "\xB6o"
                                  + streambuf_read_len,
                                  17 - streambuf_read_len,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_TEXT_LAST_FRAGMENT != ret) ||
          (3 != payload_len) ||
          (NULL == payload) ||
          (0 != memcmp ("\xC3\xB6o", payload, 3 + 1)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
        payload = NULL;
      }

      MHD_websocket_stream_free (ws);
    }
    else
    {
      fprintf (stderr,
               "Individual decode test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }


  /*
  ------------------------------------------------------------------------------
    invalid parameters
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *ws;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init (&ws,
                                                              MHD_WEBSOCKET_FLAG_SERVER
                                                              |
                                                              MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                              0))
    {
      size_t streambuf_read_len = 0;
      char *payload = NULL;
      size_t payload_len = 0;
      int ret = 0;

      /* Failure test: `ws` is NULL */
      payload = (char *) (uintptr_t) 0xBAADF00D;
      payload_len = 0x87654321;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (NULL,
                                  "\x81\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
          (NULL != payload) ||
          (0 != payload_len) ||
          (0 != streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }
      /* Failure test: `buf` is NULL, while `buf_len` != 0 */
      payload = (char *) (uintptr_t) 0xBAADF00D;
      payload_len = 0x87654321;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (ws,
                                  NULL,
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
          (NULL != payload) ||
          (0 != payload_len) ||
          (0 != streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }
      /* Failure test: `streambuf_read_len` is NULL */
      payload = (char *) (uintptr_t) 0xBAADF00D;
      payload_len = 0x87654321;
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00Hello",
                                  11,
                                  NULL,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
          (NULL != payload) ||
          (0 != payload_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }
      /* Failure test: `payload` is NULL */
      payload_len = 0x87654321;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  NULL,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
          (0 != payload_len) ||
          (0 != streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      /* Failure test: `payload_len` is NULL */
      payload = (char *) (uintptr_t) 0xBAADF00D;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  NULL);
      if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
          (NULL != payload) ||
          (0 != streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }
      /* Regular test: `buf` is NULL and `buf_len` is 0 */
      payload = (char *) (uintptr_t) 0xBAADF00D;
      payload_len = 0x87654321;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (ws,
                                  NULL,
                                  0,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (NULL != payload) ||
          (0 != payload_len) ||
          (0 != streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }
      /* Regular test: `buf` is not NULL and `buf_len` is 0 */
      payload = (char *) (uintptr_t) 0xBAADF00D;
      payload_len = 0x87654321;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00Hello",
                                  0,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (NULL != payload) ||
          (0 != payload_len) ||
          (0 != streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }

      MHD_websocket_stream_free (ws);
    }
    else
    {
      fprintf (stderr,
               "Parameter decode tests failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  /*
  ------------------------------------------------------------------------------
    validity after temporary out-of-memory
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *ws;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                               MHD_WEBSOCKET_FLAG_SERVER
                                                               |
                                                               MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      size_t streambuf_read_len = 0;
      char *payload = NULL;
      size_t payload_len = 0;
      int ret = 0;

      /* Failure test: No memory allocation at the start */
      disable_alloc = 1;
      payload = (char *) (uintptr_t) 0xBAADF00D;
      payload_len = 0x87654321;
      streambuf_read_len = 1000;
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
          (NULL != payload) ||
          (0 != payload_len) ||
          (1000 == streambuf_read_len) ||
          (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (ws)))
      {
        fprintf (stderr,
                 "Decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (((char *) (uintptr_t) 0xBAADF00D) == payload)
      {
        payload = NULL;
      }
      if (NULL != payload)
      {
        MHD_websocket_free (ws, payload);
      }
      MHD_websocket_stream_free (ws);
      if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                                 MHD_WEBSOCKET_FLAG_SERVER
                                                                 |
                                                                 MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                                 0,
                                                                 test_malloc,
                                                                 test_realloc,
                                                                 test_free,
                                                                 NULL,
                                                                 NULL))
      {
        /* Failure test: No memory allocation after fragmented frame */
        disable_alloc = 0;
        payload = (char *) (uintptr_t) 0xBAADF00D;
        payload_len = 0x87654321;
        streambuf_read_len = 1000;
        ret = MHD_websocket_decode (ws,
                                    "\x01\x83\x00\x00\x00\x00" "Hel",
                                    9,
                                    &streambuf_read_len,
                                    &payload,
                                    &payload_len);
        if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
            (NULL != payload) ||
            (0 != payload_len) ||
            (9 != streambuf_read_len) ||
            (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (
               ws)))
        {
          fprintf (stderr,
                   "Decode test failed in line %u\n",
                   (unsigned int) __LINE__);
          ++failed;
        }
        if (((char *) (uintptr_t) 0xBAADF00D) == payload)
        {
          payload = NULL;
        }
        if (NULL != payload)
        {
          MHD_websocket_free (ws, payload);
        }
        disable_alloc = 1;
        payload = (char *) (uintptr_t) 0xBAADF00D;
        payload_len = 0x87654321;
        streambuf_read_len = 1000;
        ret = MHD_websocket_decode (ws,
                                    "\x80\x82\x00\x00\x00\x00" "lo",
                                    8,
                                    &streambuf_read_len,
                                    &payload,
                                    &payload_len);
        if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
            (NULL != payload) ||
            (0 != payload_len) ||
            (1000 == streambuf_read_len) ||
            (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (
               ws)))
        {
          fprintf (stderr,
                   "Decode test failed in line %u\n",
                   (unsigned int) __LINE__);
          ++failed;
        }
        if (((char *) (uintptr_t) 0xBAADF00D) == payload)
        {
          payload = NULL;
        }
        if (NULL != payload)
        {
          MHD_websocket_free (ws, payload);
        }
        /* Regular test: Success after memory allocation ok again */
        /* (streambuf_read_len may not be overwritten for this test) */
        disable_alloc = 0;
        payload = (char *) (uintptr_t) 0xBAADF00D;
        payload_len = 0x87654321;
        size_t old_streambuf_read_len = streambuf_read_len;
        ret = MHD_websocket_decode (ws,
                                    "\x80\x82\x00\x00\x00\x00lo"
                                    + old_streambuf_read_len,
                                    8 - old_streambuf_read_len,
                                    &streambuf_read_len,
                                    &payload,
                                    &payload_len);
        if ((MHD_WEBSOCKET_STATUS_TEXT_FRAME != ret) ||
            (NULL == payload) ||
            (5 != payload_len) ||
            (8 != streambuf_read_len + old_streambuf_read_len) ||
            (MHD_WEBSOCKET_VALIDITY_VALID != MHD_websocket_stream_is_valid (
               ws)) ||
            (0 != memcmp ("Hello", payload, 5)))
        {
          fprintf (stderr,
                   "Decode test failed in line %u\n",
                   (unsigned int) __LINE__);
          ++failed;
        }
        if (((char *) (uintptr_t) 0xBAADF00D) == payload)
        {
          payload = NULL;
        }
        if (NULL != payload)
        {
          MHD_websocket_free (ws, payload);
        }

        MHD_websocket_stream_free (ws);
      }
      else
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
    }
    else
    {
      fprintf (stderr,
               "Memory decode tests failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  /*
  ------------------------------------------------------------------------------
    memory leak test, when freeing while decoding
  ------------------------------------------------------------------------------
  */
  {
    disable_alloc = 0;
    struct MHD_WebSocketStream *ws;
    size_t streambuf_read_len = 0;
    char *payload = NULL;
    size_t payload_len = 0;
    int ret = 0;

    /* Regular test: Free while decoding of data frame */
    open_allocs = 0;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                               MHD_WEBSOCKET_FLAG_SERVER
                                                               |
                                                               MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      ret = MHD_websocket_decode (ws,
                                  "\x81\x85\x00\x00\x00\x00Hel",
                                  9,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (9 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_stream_free (ws);
      if (MHD_WEBSOCKET_STATUS_OK != ret)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (0 != open_allocs)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u (memory leak detected)\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
    }
    else
    {
      fprintf (stderr,
               "Memory test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }

    /* Regular test: Free while decoding of control frame */
    open_allocs = 0;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                               MHD_WEBSOCKET_FLAG_SERVER
                                                               |
                                                               MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      ret = MHD_websocket_decode (ws,
                                  "\x88\x85\x00\x00\x00\x00Hel",
                                  9,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (9 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_stream_free (ws);
      if (MHD_WEBSOCKET_STATUS_OK != ret)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (0 != open_allocs)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u (memory leak detected)\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
    }
    else
    {
      fprintf (stderr,
               "Memory test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }

    /* Regular test: Free while decoding of fragmented data frame */
    open_allocs = 0;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                               MHD_WEBSOCKET_FLAG_SERVER
                                                               |
                                                               MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      ret = MHD_websocket_decode (ws,
                                  "\x01\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (11 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_stream_free (ws);
      if (MHD_WEBSOCKET_STATUS_OK != ret)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (0 != open_allocs)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u (memory leak detected)\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
    }
    else
    {
      fprintf (stderr,
               "Memory test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
    /* Regular test: Free while decoding of continued fragmented data frame */
    open_allocs = 0;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                               MHD_WEBSOCKET_FLAG_SERVER
                                                               |
                                                               MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      ret = MHD_websocket_decode (ws,
                                  "\x01\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (11 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_decode (ws,
                                  "\x80\x85\x00\x00\x00\x00Hel",
                                  9,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (9 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_stream_free (ws);
      if (MHD_WEBSOCKET_STATUS_OK != ret)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (0 != open_allocs)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u (memory leak detected)\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
    }
    else
    {
      fprintf (stderr,
               "Memory test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
    /* Regular test: Free while decoding of control frame during fragmented data frame */
    open_allocs = 0;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&ws,
                                                               MHD_WEBSOCKET_FLAG_SERVER
                                                               |
                                                               MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      ret = MHD_websocket_decode (ws,
                                  "\x01\x85\x00\x00\x00\x00Hello",
                                  11,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (11 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_decode (ws,
                                  "\x88\x85\x00\x00\x00\x00Hel",
                                  9,
                                  &streambuf_read_len,
                                  &payload,
                                  &payload_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (0 != payload_len) ||
          (NULL != payload) ||
          (9 != streambuf_read_len) )
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      ret = MHD_websocket_stream_free (ws);
      if (MHD_WEBSOCKET_STATUS_OK != ret)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (0 != open_allocs)
      {
        fprintf (stderr,
                 "Memory decode test failed in line %u (memory leak detected)\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
    }
    else
    {
      fprintf (stderr,
               "Memory test failed in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  if (NULL != buf1)
  {
    free (buf1);
    buf1 = NULL;
  }
  if (NULL != buf2)
  {
    free (buf2);
    buf2 = NULL;
  }
  return failed != 0 ? 0x04 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_encode_text()`
 */
int
test_encodes_text ()
{
  int failed = 0;
  struct MHD_WebSocketStream *wss;
  struct MHD_WebSocketStream *wsc;
  int ret;
  char *buf1 = NULL, *buf2 = NULL;
  char *frame = NULL;
  size_t frame_len = 0;
  int utf8_step = 0;

  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init2 (&wsc,
                                                             MHD_WEBSOCKET_FLAG_CLIENT,
                                                             0,
                                                             malloc,
                                                             realloc,
                                                             free,
                                                             NULL,
                                                             test_rng))
  {
    fprintf (stderr,
             "No encode text tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    return 0x08;
  }
  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init (&wss,
                                                            MHD_WEBSOCKET_FLAG_SERVER,
                                                            0))
  {
    fprintf (stderr,
             "No encode text tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    if (NULL != wsc)
      MHD_websocket_stream_free (wsc);
    return 0x08;
  }

  /*
  ------------------------------------------------------------------------------
    Encoding
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data without UTF-8, we are server */
  ret = MHD_websocket_encode_text (wss,
                                   "blablabla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data without UTF-8, we are client */
  ret = MHD_websocket_encode_text (wsc,
                                   "blablabla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (15 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "blablabla",
                                  9,
                                  MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Some data with UTF-8, we are server */
  ret = MHD_websocket_encode_text (wss,
                                   "bla" "\xC3\xA4" "blabla",
                                   11,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x0B" "bla" "\xC3\xA4" "blabla", 13)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data with UTF-8, we are client */
  ret = MHD_websocket_encode_text (wsc,
                                   "bla" "\xC3\xA4" "blabla",
                                   11,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (17 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "bla" "\xC3\xA4" "blabla",
                                  11,
                                  MHD_WEBSOCKET_STATUS_TEXT_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Edge test (success): Some data with NUL characters, we are server */
  ret = MHD_websocket_encode_text (wss,
                                   "bla" "\0\0\0" "bla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x09" "bla" "\0\0\0" "bla", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: Some data with broken UTF-8, we are server */
  ret = MHD_websocket_encode_text (wss,
                                   "bla" "\xC3" "blabla",
                                   10,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Fragmentation
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data without UTF-8 */
  ret = MHD_websocket_encode_text (wss,
                                   "blablabla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x81\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: First fragment without UTF-8 */
  ret = MHD_websocket_encode_text (wss,
                                   "blablabla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_FIRST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x01\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Middle fragment without UTF-8 */
  ret = MHD_websocket_encode_text (wss,
                                   "blablabla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_FOLLOWING,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x00\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment without UTF-8 */
  ret = MHD_websocket_encode_text (wss,
                                   "blablabla",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): First fragment with UTF-8 on the edge */
  ret = MHD_websocket_encode_text (wss,
                                   "blablabl\xC3",
                                   9,
                                   MHD_WEBSOCKET_FRAGMENTATION_FIRST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_UTF2TAIL_1OF1 != utf8_step) ||
      (0 != memcmp (frame, "\x01\x09" "blablabl\xC3", 11)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Last fragment with UTF-8 on the edge */
  ret = MHD_websocket_encode_text (wss,
                                   "\xA4" "blablabla",
                                   10,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (12 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0A" "\xA4" "blablabla", 12)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: Last fragment with UTF-8 on the edge (here with wrong old utf8_step) */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_NORMAL;
  ret = MHD_websocket_encode_text (wss,
                                   "\xA4" "blablabla",
                                   10,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF2TAIL_1OF1 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF2TAIL_1OF1;
  ret = MHD_websocket_encode_text (wss,
                                   "\xA4" "blablabla",
                                   10,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (12 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0A" "\xA4" "blablabla", 12)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF3TAIL1_1OF2 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL1_1OF2;
  ret = MHD_websocket_encode_text (wss,
                                   "\xA0\x80" "blablabla",
                                   11,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0B" "\xA0\x80" "blablabla", 13)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF3TAIL2_1OF2 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL2_1OF2;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80\x80" "blablabla",
                                   11,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0B" "\x80\x80" "blablabla", 13)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF3TAIL_1OF2 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_1OF2;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80\x80" "blablabla",
                                   11,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0B" "\x80\x80" "blablabla", 13)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF3TAIL_2OF2 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF3TAIL_2OF2;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80" " blablabla",
                                   11,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0B" "\x80" " blablabla", 13)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF4TAIL1_1OF3 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL1_1OF3;
  ret = MHD_websocket_encode_text (wss,
                                   "\x90\x80\x80" "blablabla",
                                   12,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (14 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0C" "\x90\x80\x80" "blablabla", 14)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF4TAIL2_1OF3 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL2_1OF3;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80\x80\x80" "blablabla",
                                   12,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (14 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0C" "\x80\x80\x80" "blablabla", 14)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF4TAIL_1OF3 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_1OF3;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80\x80\x80" "blablabla",
                                   12,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (14 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0C" "\x80\x80\x80" "blablabla", 14)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF4TAIL_2OF3 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_2OF3;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80\x80" " blablabla",
                                   12,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (14 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0C" "\x80\x80" " blablabla", 14)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment with UTF-8 on the edge for UTF4TAIL_3OF3 */
  utf8_step = MHD_WEBSOCKET_UTF8STEP_UTF4TAIL_3OF3;
  ret = MHD_websocket_encode_text (wss,
                                   "\x80" "  blablabla",
                                   12,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (14 != frame_len) ||
      (NULL == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x0C" "\x80" "  blablabla", 14)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Length checks
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): Text frame without data */
  ret = MHD_websocket_encode_text (wss,
                                   NULL,
                                   0,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x00", 2)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Text frame with 1 byte of data */
  ret = MHD_websocket_encode_text (wss,
                                   "a",
                                   1,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (3 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x01" "a", 3)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Text frame with 125 bytes of data */
  ret = MHD_websocket_encode_text (wss,
                                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                   125,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (127 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x7D"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                    127)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Text frame with 126 bytes of data */
  ret = MHD_websocket_encode_text (wss,
                                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                   126,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (130 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x81\x7E\x00\x7E"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                    130)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Text frame with 65535 bytes of data */
  allocate_length_test_data (&buf1,
                             &buf2,
                             65535,
                             "\x81\x7E\xFF\xFF",
                             4);
  ret = MHD_websocket_encode_text (wss,
                                   buf2,
                                   65535,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (65535 + 4 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, buf1, 65535 + 4)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Text frame with 65536 bytes of data */
  allocate_length_test_data (&buf1,
                             &buf2,
                             65536,
                             "\x81\x7F\x00\x00\x00\x00\x00\x01\x00\x00",
                             10);
  ret = MHD_websocket_encode_text (wss,
                                   buf2,
                                   65536,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (65536 + 10 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, buf1, 65536 + 10)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Text frame with 100 MB of data */
  allocate_length_test_data (&buf1,
                             &buf2,
                             104857600,
                             "\x81\x7F\x00\x00\x00\x00\x06\x40\x00\x00",
                             10);
  ret = MHD_websocket_encode_text (wss,
                                   buf2,
                                   104857600,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (104857600 + 10 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, buf1, 104857600 + 10)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  if (NULL != buf1)
  {
    free (buf1);
    buf1 = NULL;
  }
  if (NULL != buf2)
  {
    free (buf2);
    buf2 = NULL;
  }
#ifdef ENABLE_64BIT_TESTS
  /* Fail test: frame_len is greater than 0x7FFFFFFFFFFFFFFF
     (this is the maximum allowed payload size) */
  frame_len = 0;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   (uint64_t) 0x8000000000000000,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
#endif

  /*
  ------------------------------------------------------------------------------
    Wrong parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: `ws` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (NULL,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `payload_utf8` not passed, but `payload_utf8_len` != 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (wss,
                                   NULL,
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `payload_utf8` passed, but `payload_utf8_len` == 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   0,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (0 != memcmp (frame, "\x81\x00", 2)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `frame` not passed */
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   NULL,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: `frame_len` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   NULL,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `utf8_step` passed for non-fragmentation
     (is allowed and `utf8_step` will be filled then) */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  utf8_step = -99;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (5 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x81\x03" "abc", 5)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `utf8_step` passed for non-fragmentation with invalid UTF-8
     (is allowed and `utf8_step` will be filled then) */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  utf8_step = -99;
  ret = MHD_websocket_encode_text (wss,
                                   "ab\xC3",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) ||
      (MHD_WEBSOCKET_UTF8STEP_UTF2TAIL_1OF1 != utf8_step) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `utf8_step` not passed for fragmentation #1 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_FIRST,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `utf8_step` not passed for fragmentation #2 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_FOLLOWING,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `utf8_step` not passed for fragmentation #3 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `utf8_step` passed for fragmentation #1 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  utf8_step = -99;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_FIRST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (5 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x01\x03" "abc", 5)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `utf8_step` passed for fragmentation #2 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  utf8_step = MHD_WEBSOCKET_UTF8STEP_NORMAL;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_FOLLOWING,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (5 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x00\x03" "abc", 5)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `utf8_step` passed for fragmentation #3 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  utf8_step = MHD_WEBSOCKET_UTF8STEP_NORMAL;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (5 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (MHD_WEBSOCKET_UTF8STEP_NORMAL != utf8_step) ||
      (0 != memcmp (frame, "\x80\x03" "abc", 5)))
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `fragmentation` has an invalid value */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  utf8_step = -99;
  ret = MHD_websocket_encode_text (wss,
                                   "abc",
                                   3,
                                   MHD_WEBSOCKET_FRAGMENTATION_LAST + 1,
                                   &frame,
                                   &frame_len,
                                   &utf8_step);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) ||
      (-99 != utf8_step) )
  {
    fprintf (stderr,
             "Encode text test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    validity after temporary out-of-memory
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *wsx;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&wsx,
                                                               MHD_WEBSOCKET_FLAG_SERVER,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      /* Fail test: allocation while no memory available */
      disable_alloc = 1;
      ret = MHD_websocket_encode_text (wsx,
                                       "abc",
                                       3,
                                       MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                       &frame,
                                       &frame_len,
                                       NULL);
      if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
          (0 != frame_len) ||
          (NULL != frame) )
      {
        fprintf (stderr,
                 "Encode text test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }
      /* Regular test: allocation while memory is available again */
      disable_alloc = 0;
      ret = MHD_websocket_encode_text (wsx,
                                       "abc",
                                       3,
                                       MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                       &frame,
                                       &frame_len,
                                       NULL);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (5 != frame_len) ||
          (NULL == frame) ||
          (0 != memcmp (frame, "\x81\x03" "abc", 5)))
      {
        fprintf (stderr,
                 "Encode text test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }

      MHD_websocket_stream_free (wsx);
    }
    else
    {
      fprintf (stderr,
               "Couldn't perform memory test for text encoding in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  if (NULL != buf1)
    free (buf1);
  if (NULL != buf2)
    free (buf2);
  if (NULL != wsc)
    MHD_websocket_stream_free (wsc);
  if (NULL != wss)
    MHD_websocket_stream_free (wss);

  return failed != 0 ? 0x08 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_encode_binary()`
 */
int
test_encodes_binary ()
{
  int failed = 0;
  struct MHD_WebSocketStream *wss;
  struct MHD_WebSocketStream *wsc;
  int ret;
  char *buf1 = NULL, *buf2 = NULL;
  char *frame = NULL;
  size_t frame_len = 0;

  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init2 (&wsc,
                                                             MHD_WEBSOCKET_FLAG_CLIENT,
                                                             0,
                                                             malloc,
                                                             realloc,
                                                             free,
                                                             NULL,
                                                             test_rng))
  {
    fprintf (stderr,
             "No encode binary tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    return 0x10;
  }
  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init (&wss,
                                                            MHD_WEBSOCKET_FLAG_SERVER,
                                                            0))
  {
    fprintf (stderr,
             "No encode binary tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    if (NULL != wsc)
      MHD_websocket_stream_free (wsc);
    return 0x10;
  }

  /*
  ------------------------------------------------------------------------------
    Encoding
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data, we are server */
  ret = MHD_websocket_encode_binary (wss,
                                     "blablabla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data, we are client */
  ret = MHD_websocket_encode_binary (wsc,
                                     "blablabla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (15 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "blablabla",
                                  9,
                                  MHD_WEBSOCKET_STATUS_BINARY_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Edge test (success): Some data with NUL characters, we are server */
  ret = MHD_websocket_encode_binary (wss,
                                     "bla" "\0\0\0" "bla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x09" "bla" "\0\0\0" "bla", 11)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data which looks like broken UTF-8, we are server */
  ret = MHD_websocket_encode_binary (wss,
                                     "bla" "\xC3" "blabla",
                                     10,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (12 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x0A" "bla" "\xC3" "blabla", 12)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Fragmentation
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data */
  ret = MHD_websocket_encode_binary (wss,
                                     "blablabla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: First fragment */
  ret = MHD_websocket_encode_binary (wss,
                                     "blablabla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_FIRST,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x02\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Middle fragment */
  ret = MHD_websocket_encode_binary (wss,
                                     "blablabla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_FOLLOWING,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x00\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Last fragment  */
  ret = MHD_websocket_encode_binary (wss,
                                     "blablabla",
                                     9,
                                     MHD_WEBSOCKET_FRAGMENTATION_LAST,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x80\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Length checks
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): Binary frame without data */
  ret = MHD_websocket_encode_binary (wss,
                                     NULL,
                                     0,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x00", 2)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Binary frame with 1 byte of data */
  ret = MHD_websocket_encode_binary (wss,
                                     "a",
                                     1,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (3 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x01" "a", 3)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Binary frame with 125 bytes of data */
  ret = MHD_websocket_encode_binary (wss,
                                     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                     125,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (127 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x7D"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                    127)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Binary frame with 126 bytes of data */
  ret = MHD_websocket_encode_binary (wss,
                                     "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                     126,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (130 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x82\x7E\x00\x7E"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                    130)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Binary frame with 65535 bytes of data */
  allocate_length_test_data (&buf1,
                             &buf2,
                             65535,
                             "\x82\x7E\xFF\xFF",
                             4);
  ret = MHD_websocket_encode_binary (wss,
                                     buf2,
                                     65535,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (65535 + 4 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, buf1, 65535 + 4)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Binary frame with 65536 bytes of data */
  allocate_length_test_data (&buf1,
                             &buf2,
                             65536,
                             "\x82\x7F\x00\x00\x00\x00\x00\x01\x00\x00",
                             10);
  ret = MHD_websocket_encode_binary (wss,
                                     buf2,
                                     65536,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (65536 + 10 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, buf1, 65536 + 10)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Binary frame with 100 MB of data */
  allocate_length_test_data (&buf1,
                             &buf2,
                             104857600,
                             "\x82\x7F\x00\x00\x00\x00\x06\x40\x00\x00",
                             10);
  ret = MHD_websocket_encode_binary (wss,
                                     buf2,
                                     104857600,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (104857600 + 10 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, buf1, 104857600 + 10)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  if (NULL != buf1)
  {
    free (buf1);
    buf1 = NULL;
  }
  if (NULL != buf2)
  {
    free (buf2);
    buf2 = NULL;
  }
#ifdef ENABLE_64BIT_TESTS
  /* Fail test: `frame_len` is greater than 0x7FFFFFFFFFFFFFFF
     (this is the maximum allowed payload size) */
  frame_len = 0;
  ret = MHD_websocket_encode_binary (wss,
                                     "abc",
                                     (uint64_t) 0x8000000000000000,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
#endif

  /*
  ------------------------------------------------------------------------------
    Wrong parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: `ws` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_binary (NULL,
                                     "abc",
                                     3,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `payload` not passed, but `payload_len` != 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_binary (wss,
                                     NULL,
                                     3,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `payload` passed, but `payload_len` == 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_binary (wss,
                                     "abc",
                                     0,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (0 != memcmp (frame, "\x82\x00", 2)))
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `frame` not passed */
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_binary (wss,
                                     "abc",
                                     3,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     NULL,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: `frame_len` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  ret = MHD_websocket_encode_binary (wss,
                                     "abc",
                                     3,
                                     MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                     &frame,
                                     NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `fragmentation` has an invalid value */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_binary (wss,
                                     "abc",
                                     3,
                                     MHD_WEBSOCKET_FRAGMENTATION_LAST + 1,
                                     &frame,
                                     &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode binary test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    validity after temporary out-of-memory
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *wsx;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&wsx,
                                                               MHD_WEBSOCKET_FLAG_SERVER,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      /* Fail test: allocation while no memory available */
      disable_alloc = 1;
      ret = MHD_websocket_encode_binary (wsx,
                                         "abc",
                                         3,
                                         MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                         &frame,
                                         &frame_len);
      if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
          (0 != frame_len) ||
          (NULL != frame) )
      {
        fprintf (stderr,
                 "Encode binary test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }
      /* Regular test: allocation while memory is available again */
      disable_alloc = 0;
      ret = MHD_websocket_encode_binary (wsx,
                                         "abc",
                                         3,
                                         MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                         &frame,
                                         &frame_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (5 != frame_len) ||
          (NULL == frame) ||
          (0 != memcmp (frame, "\x82\x03" "abc", 5)))
      {
        fprintf (stderr,
                 "Encode binary test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }

      MHD_websocket_stream_free (wsx);
    }
    else
    {
      fprintf (stderr,
               "Couldn't perform memory test for binary encoding in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  if (NULL != buf1)
    free (buf1);
  if (NULL != buf2)
    free (buf2);
  if (NULL != wsc)
    MHD_websocket_stream_free (wsc);
  if (NULL != wss)
    MHD_websocket_stream_free (wss);

  return failed != 0 ? 0x10 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_encode_close()`
 */
int
test_encodes_close ()
{
  int failed = 0;
  struct MHD_WebSocketStream *wss;
  struct MHD_WebSocketStream *wsc;
  int ret;
  char *buf1 = NULL, *buf2 = NULL;
  char *frame = NULL;
  size_t frame_len = 0;

  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init2 (&wsc,
                                                             MHD_WEBSOCKET_FLAG_CLIENT,
                                                             0,
                                                             malloc,
                                                             realloc,
                                                             free,
                                                             NULL,
                                                             test_rng))
  {
    fprintf (stderr,
             "No encode close tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    return 0x10;
  }
  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init2 (&wss,
                                                             MHD_WEBSOCKET_FLAG_SERVER,
                                                             0,
                                                             malloc,
                                                             realloc,
                                                             free,
                                                             NULL,
                                                             test_rng))
  {
    fprintf (stderr,
             "No encode close tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    if (NULL != wsc)
      MHD_websocket_stream_free (wsc);
    return 0x10;
  }

  /*
  ------------------------------------------------------------------------------
    Encoding
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data, we are server */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "blablabla",
                                    9,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x0B\x03\xE8" "blablabla", 13)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data, we are client */
  ret = MHD_websocket_encode_close (wsc,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "blablabla",
                                    9,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (17 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "\x03\xE8" "blablabla",
                                  11,
                                  MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Close reason without text, we are server */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    NULL,
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (4 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x02\x03\xE8", 4)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Close reason without text, we are client */
  ret = MHD_websocket_encode_close (wsc,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    NULL,
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (8 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "\x03\xE8",
                                  2,
                                  MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Close without reason, we are server */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_NO_REASON,
                                    NULL,
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x00", 2)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Close without reason, we are client */
  ret = MHD_websocket_encode_close (wsc,
                                    MHD_WEBSOCKET_CLOSEREASON_NO_REASON,
                                    NULL,
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (6 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  NULL,
                                  0,
                                  MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Close with UTF-8 sequence in reason, we are client */
  ret = MHD_websocket_encode_close (wsc,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "bla" "\xC3\xA4" "blabla",
                                    11,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (19 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "\x03\xE8" "bla" "\xC3\xA4" "blabla",
                                  13,
                                  MHD_WEBSOCKET_STATUS_CLOSE_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_ONLY_VALID_FOR_CONTROL_FRAMES,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Edge test (success): Close reason with NUL characters, we are server */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_GOING_AWAY,
                                    "bla" "\0\0\0" "bla",
                                    9,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (13 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x0B\x03\xE9" "bla" "\0\0\0" "bla", 13)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: Some data with broken UTF-8, we are server */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "bla" "\xC3" "blabla",
                                    10,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_UTF8_ENCODING_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Length checks
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): Close frame without payload */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_NO_REASON,
                                    NULL,
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x00", 2)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Close frame only reason code */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    NULL,
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (4 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x02\x03\xE8", 4)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Close frame with 1 bytes of reason text */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "a",
                                    1,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (5 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x03\x03\xE8" "a", 5)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Close frame with 123 bytes of reason text */
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456",
                                    123,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (127 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x88\x7D\x03\xE8"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456",
                    127)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (fail): Close frame with 124 bytes of reason text*/
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567",
                                    124,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Wrong parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: `ws` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (NULL,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "abc",
                                    3,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `payload` not passed, but `payload_len` != 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    NULL,
                                    3,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `payload` passed, but `payload_len` == 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "abc",
                                    0,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (4 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (0 != memcmp (frame, "\x88\x02\x03\xE8", 4)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `frame` not passed */
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "abc",
                                    3,
                                    NULL,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: `frame_len` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                    "abc",
                                    3,
                                    &frame,
                                    NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: no reason code passed, but reason text */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    MHD_WEBSOCKET_CLOSEREASON_NO_REASON,
                                    "abc",
                                    3,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (fail): Invalid reason code */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    1,
                                    "abc",
                                    3,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (fail): Invalid reason code */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    999,
                                    "abc",
                                    3,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Custom reason code */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_close (wss,
                                    2000,
                                    "abc",
                                    3,
                                    &frame,
                                    &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (7 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (0 != memcmp (frame, "\x88\x05\x07\xD0" "abc", 7)))
  {
    fprintf (stderr,
             "Encode close test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    validity after temporary out-of-memory
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *wsx;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&wsx,
                                                               MHD_WEBSOCKET_FLAG_SERVER,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      /* Fail test: allocation while no memory available */
      disable_alloc = 1;
      ret = MHD_websocket_encode_close (wsx,
                                        MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                        "abc",
                                        3,
                                        &frame,
                                        &frame_len);
      if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
          (0 != frame_len) ||
          (NULL != frame) )
      {
        fprintf (stderr,
                 "Encode close test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }
      /* Regular test: allocation while memory is available again */
      disable_alloc = 0;
      ret = MHD_websocket_encode_close (wsx,
                                        MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                        "abc",
                                        3,
                                        &frame,
                                        &frame_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (7 != frame_len) ||
          (NULL == frame) ||
          (0 != memcmp (frame, "\x88\x05\x03\xE8" "abc", 7)))
      {
        fprintf (stderr,
                 "Encode close test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }

      MHD_websocket_stream_free (wsx);
    }
    else
    {
      fprintf (stderr,
               "Couldn't perform memory test for close encoding in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  if (NULL != buf1)
    free (buf1);
  if (NULL != buf2)
    free (buf2);
  if (NULL != wsc)
    MHD_websocket_stream_free (wsc);
  if (NULL != wss)
    MHD_websocket_stream_free (wss);

  return failed != 0 ? 0x20 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_encode_ping()`
 */
int
test_encodes_ping ()
{
  int failed = 0;
  struct MHD_WebSocketStream *wss;
  struct MHD_WebSocketStream *wsc;
  int ret;
  char *buf1 = NULL, *buf2 = NULL;
  char *frame = NULL;
  size_t frame_len = 0;

  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init2 (&wsc,
                                                             MHD_WEBSOCKET_FLAG_CLIENT,
                                                             0,
                                                             malloc,
                                                             realloc,
                                                             free,
                                                             NULL,
                                                             test_rng))
  {
    fprintf (stderr,
             "No encode ping tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    return 0x10;
  }
  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init (&wss,
                                                            MHD_WEBSOCKET_FLAG_SERVER,
                                                            0))
  {
    fprintf (stderr,
             "No encode ping tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    if (NULL != wsc)
      MHD_websocket_stream_free (wsc);
    return 0x10;
  }

  /*
  ------------------------------------------------------------------------------
    Encoding
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data, we are server */
  ret = MHD_websocket_encode_ping (wss,
                                   "blablabla",
                                   9,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data, we are client */
  ret = MHD_websocket_encode_ping (wsc,
                                   "blablabla",
                                   9,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (15 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "blablabla",
                                  9,
                                  MHD_WEBSOCKET_STATUS_PING_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Ping without payload, we are server */
  ret = MHD_websocket_encode_ping (wss,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x00", 2)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Ping without payload, we are client */
  ret = MHD_websocket_encode_ping (wsc,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (6 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  NULL,
                                  0,
                                  MHD_WEBSOCKET_STATUS_PING_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Ping with something like UTF-8 sequence in payload, we are client */
  ret = MHD_websocket_encode_ping (wsc,
                                   "bla" "\xC3\xA4" "blabla",
                                   11,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (17 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "bla" "\xC3\xA4" "blabla",
                                  11,
                                  MHD_WEBSOCKET_STATUS_PING_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Edge test (success): Ping payload with NUL characters, we are server */
  ret = MHD_websocket_encode_ping (wss,
                                   "bla" "\0\0\0" "bla",
                                   9,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x09" "bla" "\0\0\0" "bla", 11)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Ping payload with with something which looks like broken UTF-8, we are server */
  ret = MHD_websocket_encode_ping (wss,
                                   "bla" "\xC3" "blabla",
                                   10,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (12 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x0A" "bla" "\xC3" "blabla", 12)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Length checks
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): Ping frame without payload */
  ret = MHD_websocket_encode_ping (wss,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x00", 2)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Ping frame with one byte of payload */
  ret = MHD_websocket_encode_ping (wss,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x00", 2)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Ping frame with 125 bytes of payload */
  ret = MHD_websocket_encode_ping (wss,
                                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                   125,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (127 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x89\x7D"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                    127)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (fail): Ping frame with 126 bytes of payload */
  ret = MHD_websocket_encode_ping (wss,
                                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                   126,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Wrong parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: `ws` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_ping (NULL,
                                   "abc",
                                   3,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `payload` not passed, but `payload_len` != 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_ping (wss,
                                   NULL,
                                   3,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `payload` passed, but `payload_len` == 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_ping (wss,
                                   "abc",
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (0 != memcmp (frame, "\x89\x00", 2)))
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `frame` not passed */
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_ping (wss,
                                   "abc",
                                   3,
                                   NULL,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: `frame_len` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  ret = MHD_websocket_encode_ping (wss,
                                   "abc",
                                   3,
                                   &frame,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode ping test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    validity after temporary out-of-memory
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *wsx;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&wsx,
                                                               MHD_WEBSOCKET_FLAG_SERVER,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      /* Fail test: allocation while no memory available */
      disable_alloc = 1;
      ret = MHD_websocket_encode_ping (wsx,
                                       "abc",
                                       3,
                                       &frame,
                                       &frame_len);
      if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
          (0 != frame_len) ||
          (NULL != frame) )
      {
        fprintf (stderr,
                 "Encode ping test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }
      /* Regular test: allocation while memory is available again */
      disable_alloc = 0;
      ret = MHD_websocket_encode_ping (wsx,
                                       "abc",
                                       3,
                                       &frame,
                                       &frame_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (5 != frame_len) ||
          (NULL == frame) ||
          (0 != memcmp (frame, "\x89\x03" "abc", 5)))
      {
        fprintf (stderr,
                 "Encode ping test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }

      MHD_websocket_stream_free (wsx);
    }
    else
    {
      fprintf (stderr,
               "Couldn't perform memory test for ping encoding in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  if (NULL != buf1)
    free (buf1);
  if (NULL != buf2)
    free (buf2);
  if (NULL != wsc)
    MHD_websocket_stream_free (wsc);
  if (NULL != wss)
    MHD_websocket_stream_free (wss);

  return failed != 0 ? 0x40 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_encode_pong()`
 */
int
test_encodes_pong ()
{
  int failed = 0;
  struct MHD_WebSocketStream *wss;
  struct MHD_WebSocketStream *wsc;
  int ret;
  char *buf1 = NULL, *buf2 = NULL;
  char *frame = NULL;
  size_t frame_len = 0;

  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init2 (&wsc,
                                                             MHD_WEBSOCKET_FLAG_CLIENT,
                                                             0,
                                                             malloc,
                                                             realloc,
                                                             free,
                                                             NULL,
                                                             test_rng))
  {
    fprintf (stderr,
             "No encode pong tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    return 0x10;
  }
  if (MHD_WEBSOCKET_STATUS_OK != MHD_websocket_stream_init (&wss,
                                                            MHD_WEBSOCKET_FLAG_SERVER,
                                                            0))
  {
    fprintf (stderr,
             "No encode pong tests possible due to failed stream init in line %u\n",
             (unsigned int) __LINE__);
    if (NULL != wsc)
      MHD_websocket_stream_free (wsc);
    return 0x10;
  }

  /*
  ------------------------------------------------------------------------------
    Encoding
  ------------------------------------------------------------------------------
  */
  /* Regular test: Some data, we are server */
  ret = MHD_websocket_encode_pong (wss,
                                   "blablabla",
                                   9,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x09" "blablabla", 11)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Some data, we are client */
  ret = MHD_websocket_encode_pong (wsc,
                                   "blablabla",
                                   9,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (15 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "blablabla",
                                  9,
                                  MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Pong without payload, we are server */
  ret = MHD_websocket_encode_pong (wss,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x00", 2)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Pong without payload, we are client */
  ret = MHD_websocket_encode_pong (wsc,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (6 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  NULL,
                                  0,
                                  MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Regular test: Pong with something like UTF-8 sequence in payload, we are client */
  ret = MHD_websocket_encode_pong (wsc,
                                   "bla" "\xC3\xA4" "blabla",
                                   11,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (17 != frame_len) ||
      (NULL == frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  else
  {
    failed += test_decode_single (__LINE__,
                                  MHD_WEBSOCKET_FLAG_SERVER
                                  | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                  0,
                                  1,
                                  0,
                                  frame,
                                  frame_len,
                                  "bla" "\xC3\xA4" "blabla",
                                  11,
                                  MHD_WEBSOCKET_STATUS_PONG_FRAME,
                                  MHD_WEBSOCKET_VALIDITY_VALID,
                                  frame_len);
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wsc, frame);
    frame = NULL;
  }
  /* Edge test (success): Pong payload with NUL characters, we are server */
  ret = MHD_websocket_encode_pong (wss,
                                   "bla" "\0\0\0" "bla",
                                   9,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (11 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x09" "bla" "\0\0\0" "bla", 11)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: Pong payload with with something which looks like broken UTF-8, we are server */
  ret = MHD_websocket_encode_pong (wss,
                                   "bla" "\xC3" "blabla",
                                   10,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (12 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x0A" "bla" "\xC3" "blabla", 12)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Length checks
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): Pong frame without payload */
  ret = MHD_websocket_encode_pong (wss,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x00", 2)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Pong frame with one byte of payload */
  ret = MHD_websocket_encode_pong (wss,
                                   NULL,
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x00", 2)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (success): Pong frame with 125 bytes of payload */
  ret = MHD_websocket_encode_pong (wss,
                                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                                   125,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (127 != frame_len) ||
      (NULL == frame) ||
      (0 != memcmp (frame, "\x8A\x7D"
                    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678",
                    127)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Edge test (fail): Pong frame with 126 bytes of payload */
  ret = MHD_websocket_encode_pong (wss,
                                   "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
                                   126,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_MAXIMUM_SIZE_EXCEEDED != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    Wrong parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: `ws` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_pong (NULL,
                                   "abc",
                                   3,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `payload` not passed, but `payload_len` != 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_pong (wss,
                                   NULL,
                                   3,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Regular test: `payload` passed, but `payload_len` == 0 */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_pong (wss,
                                   "abc",
                                   0,
                                   &frame,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (2 != frame_len) ||
      (NULL == frame) ||
      (((char *) (uintptr_t) 0xBAADF00D) == frame) ||
      (0 != memcmp (frame, "\x8A\x00", 2)))
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }
  /* Fail test: `frame` not passed */
  frame_len = 0x87654321;
  ret = MHD_websocket_encode_pong (wss,
                                   "abc",
                                   3,
                                   NULL,
                                   &frame_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (0 != frame_len) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: `frame_len` not passed */
  frame = (char *) (uintptr_t) 0xBAADF00D;
  ret = MHD_websocket_encode_pong (wss,
                                   "abc",
                                   3,
                                   &frame,
                                   NULL);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (NULL != frame) )
  {
    fprintf (stderr,
             "Encode pong test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  if (((char *) (uintptr_t) 0xBAADF00D) == frame)
  {
    frame = NULL;
  }
  if (NULL != frame)
  {
    MHD_websocket_free (wss, frame);
    frame = NULL;
  }

  /*
  ------------------------------------------------------------------------------
    validity after temporary out-of-memory
  ------------------------------------------------------------------------------
  */
  {
    struct MHD_WebSocketStream *wsx;
    if (MHD_WEBSOCKET_STATUS_OK == MHD_websocket_stream_init2 (&wsx,
                                                               MHD_WEBSOCKET_FLAG_SERVER,
                                                               0,
                                                               test_malloc,
                                                               test_realloc,
                                                               test_free,
                                                               NULL,
                                                               NULL))
    {
      /* Fail test: allocation while no memory available */
      disable_alloc = 1;
      ret = MHD_websocket_encode_pong (wsx,
                                       "abc",
                                       3,
                                       &frame,
                                       &frame_len);
      if ((MHD_WEBSOCKET_STATUS_MEMORY_ERROR != ret) ||
          (0 != frame_len) ||
          (NULL != frame) )
      {
        fprintf (stderr,
                 "Encode pong test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }
      /* Regular test: allocation while memory is available again */
      disable_alloc = 0;
      ret = MHD_websocket_encode_pong (wsx,
                                       "abc",
                                       3,
                                       &frame,
                                       &frame_len);
      if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
          (5 != frame_len) ||
          (NULL == frame) ||
          (0 != memcmp (frame, "\x8A\x03" "abc", 5)))
      {
        fprintf (stderr,
                 "Encode pong test failed in line %u\n",
                 (unsigned int) __LINE__);
        ++failed;
      }
      if (NULL != frame)
      {
        MHD_websocket_free (wsx, frame);
        frame = NULL;
      }

      MHD_websocket_stream_free (wsx);
    }
    else
    {
      fprintf (stderr,
               "Couldn't perform memory test for pong encoding in line %u\n",
               (unsigned int) __LINE__);
      ++failed;
    }
  }

  if (NULL != buf1)
    free (buf1);
  if (NULL != buf2)
    free (buf2);
  if (NULL != wsc)
    MHD_websocket_stream_free (wsc);
  if (NULL != wss)
    MHD_websocket_stream_free (wss);

  return failed != 0 ? 0x80 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_split_close_reason()`
 */
int
test_split_close_reason ()
{
  int failed = 0;
  const char *payload;
  unsigned short reason_code;
  const char *reason_utf8;
  size_t reason_utf8_len;
  int ret;

  /*
  ------------------------------------------------------------------------------
    Normal splits
  ------------------------------------------------------------------------------
  */
  /* Regular test: Reason code + Reason text */
  reason_code = 9999;
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  reason_utf8_len = 12345;
  payload = "\x03\xE8" "abc";
  ret = MHD_websocket_split_close_reason (payload,
                                          5,
                                          &reason_code,
                                          &reason_utf8,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_REGULAR != reason_code) ||
      (3 != reason_utf8_len) ||
      (payload + 2 != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: Reason code */
  reason_code = 9999;
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  reason_utf8_len = 12345;
  payload = "\x03\xE8";
  ret = MHD_websocket_split_close_reason (payload,
                                          2,
                                          &reason_code,
                                          &reason_utf8,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_REGULAR != reason_code) ||
      (0 != reason_utf8_len) ||
      (NULL != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: No payload */
  reason_code = 9999;
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  reason_utf8_len = 12345;
  payload = NULL;
  ret = MHD_websocket_split_close_reason (payload,
                                          0,
                                          &reason_code,
                                          &reason_utf8,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_NO_REASON != reason_code) ||
      (0 != reason_utf8_len) ||
      (NULL != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: `payload` is not NULL given, but `payload_len` == 0 */
  reason_code = 9999;
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  reason_utf8_len = 12345;
  payload = "abc";
  ret = MHD_websocket_split_close_reason (payload,
                                          0,
                                          &reason_code,
                                          &reason_utf8,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_NO_REASON != reason_code) ||
      (0 != reason_utf8_len) ||
      (NULL != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Wrong parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: `payload` not passed, but `payload_len` != 0 */
  reason_code = 9999;
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  reason_utf8_len = 12345;
  payload = NULL;
  ret = MHD_websocket_split_close_reason (payload,
                                          3,
                                          &reason_code,
                                          &reason_utf8,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_PARAMETER_ERROR != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_NO_REASON != reason_code) ||
      (0 != reason_utf8_len) ||
      (NULL != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: `reason_code` not passed */
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  reason_utf8_len = 12345;
  payload = "\x03\xE8" "abc";
  ret = MHD_websocket_split_close_reason (payload,
                                          5,
                                          NULL,
                                          &reason_utf8,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (3 != reason_utf8_len) ||
      (payload + 2 != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: `reason_utf8` not passed */
  reason_code = 9999;
  reason_utf8_len = 12345;
  payload = "\x03\xE8" "abc";
  ret = MHD_websocket_split_close_reason (payload,
                                          5,
                                          &reason_code,
                                          NULL,
                                          &reason_utf8_len);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_REGULAR != reason_code) ||
      (3 != reason_utf8_len) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: `reason_utf8_len` not passed */
  reason_code = 9999;
  reason_utf8 = (const char *) (intptr_t) 0xBAADF00D;
  payload = "\x03\xE8" "abc";
  ret = MHD_websocket_split_close_reason (payload,
                                          5,
                                          &reason_code,
                                          &reason_utf8,
                                          NULL);
  if ((MHD_WEBSOCKET_STATUS_OK != ret) ||
      (MHD_WEBSOCKET_CLOSEREASON_REGULAR != reason_code) ||
      (payload + 2 != reason_utf8) )
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: `reason_code`, `reason_utf8` and `reason_utf8_len` not passed */
  /* (this is not prohibited, although it doesn't really make sense) */
  payload = "\x03\xE8" "abc";
  ret = MHD_websocket_split_close_reason (payload,
                                          5,
                                          NULL,
                                          NULL,
                                          NULL);
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "split close reason test failed in line %u\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  return failed != 0 ? 0x100 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_check_http_version()`
 */
int
test_check_http_version ()
{
  int failed = 0;
  int ret;

  /*
  ------------------------------------------------------------------------------
    Version check with valid HTTP version syntax
  ------------------------------------------------------------------------------
  */
  /* Regular test: HTTP/1.1 */
  ret = MHD_websocket_check_http_version ("HTTP/1.1");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: HTTP/1.2 */
  ret = MHD_websocket_check_http_version ("HTTP/1.2");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: HTTP/1.10 */
  ret = MHD_websocket_check_http_version ("HTTP/1.10");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: HTTP/2.0 */
  ret = MHD_websocket_check_http_version ("HTTP/2.0");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: HTTP/3.0 */
  ret = MHD_websocket_check_http_version ("HTTP/3.0");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: HTTP/1.0 */
  ret = MHD_websocket_check_http_version ("HTTP/1.0");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: HTTP/0.9 */
  ret = MHD_websocket_check_http_version ("HTTP/0.9");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Version check edge cases
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): HTTP/123.45 */
  ret = MHD_websocket_check_http_version ("HTTP/123.45");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/1.45 */
  ret = MHD_websocket_check_http_version ("HTTP/1.45");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/01.1 */
  ret = MHD_websocket_check_http_version ("HTTP/01.1");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/0001.1 */
  ret = MHD_websocket_check_http_version ("HTTP/0001.1");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/1.01 */
  ret = MHD_websocket_check_http_version ("HTTP/1.01");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/1.0001 */
  ret = MHD_websocket_check_http_version ("HTTP/1.0001");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/0001.0001 */
  ret = MHD_websocket_check_http_version ("HTTP/0001.0001");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/2.000 */
  ret = MHD_websocket_check_http_version ("HTTP/2.000");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): HTTP/0.0 */
  ret = MHD_websocket_check_http_version ("HTTP/0.0");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): HTTP/00.0 */
  ret = MHD_websocket_check_http_version ("HTTP/00.0");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): HTTP/00.0 */
  ret = MHD_websocket_check_http_version ("HTTP/0.00");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Invalid version syntax
  ------------------------------------------------------------------------------
  */
  /* Fail test: (empty string) */
  ret = MHD_websocket_check_http_version ("");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: http/1.1 */
  ret = MHD_websocket_check_http_version ("http/1.1");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: "HTTP / 1.1" */
  ret = MHD_websocket_check_http_version ("HTTP / 1.1");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Missing parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: NULL as version */
  ret = MHD_websocket_check_http_version (NULL);
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_http_version test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  return failed != 0 ? 0x200 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_check_connection_header()`
 */
int
test_check_connection_header ()
{
  int failed = 0;
  int ret;

  /*
  ------------------------------------------------------------------------------
    Check with valid Connection header syntax
  ------------------------------------------------------------------------------
  */
  /* Regular test: Upgrade */
  ret = MHD_websocket_check_connection_header ("Upgrade");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Regular test: keep-alive, Upgrade */
  ret = MHD_websocket_check_connection_header ("keep-alive, Upgrade");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: keep-alive */
  ret = MHD_websocket_check_connection_header ("keep-alive");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: close */
  ret = MHD_websocket_check_connection_header ("close");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Connection check edge cases
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): keep-alive,Upgrade */
  ret = MHD_websocket_check_connection_header ("keep-alive,Upgrade");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): Upgrade, keep-alive */
  ret = MHD_websocket_check_connection_header ("Upgrade, keep-alive");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): Upgrade,keep-alive */
  ret = MHD_websocket_check_connection_header ("Upgrade,keep-alive");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): Transfer-Encoding,Upgrade,keep-alive */
  ret = MHD_websocket_check_connection_header (
    "Transfer-Encoding,Upgrade,keep-alive");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): Transfer-Encoding  ,  Upgrade  ,  keep-alive */
  ret = MHD_websocket_check_connection_header (
    "Transfer-Encoding  ,  Upgrade  ,  keep-alive");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): upgrade */
  ret = MHD_websocket_check_connection_header ("upgrade");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): UPGRADE */
  ret = MHD_websocket_check_connection_header ("UPGRADE");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): All allowed token characters, then upgrade token */
  ret = MHD_websocket_check_connection_header (
    "!#$%&'*+-.^_`|~0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz,Upgrade");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): Different, allowed whitespaces */
  ret = MHD_websocket_check_connection_header (" \tUpgrade \t");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_connection_header ("\rUpgrade");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_connection_header ("\nUpgrade");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_connection_header ("\vUpgrade");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_connection_header ("\fUpgrade");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Invalid header syntax
  ------------------------------------------------------------------------------
  */
  /* Fail test: (empty string) */
  ret = MHD_websocket_check_connection_header ("");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: (Disallowed) multiple word token with the term "Upgrade" in it */
  ret = MHD_websocket_check_connection_header ("Upgrade or Downgrade");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: Invalid characters */
  ret = MHD_websocket_check_connection_header ("\"Upgrade\"");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Missing parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: NULL as connection */
  ret = MHD_websocket_check_connection_header (NULL);
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_connection_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  return failed != 0 ? 0x400 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_check_upgrade_header()`
 */
int
test_check_upgrade_header ()
{
  int failed = 0;
  int ret;

  /*
  ------------------------------------------------------------------------------
    Check with valid Upgrade header syntax
  ------------------------------------------------------------------------------
  */
  /* Regular test: websocket */
  ret = MHD_websocket_check_upgrade_header ("websocket");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: HTTP/2.0 */
  ret = MHD_websocket_check_upgrade_header ("HTTP/2.0");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Upgrade check edge cases
  ------------------------------------------------------------------------------
  */
  /* Edge test (success): websocket,HTTP/2.0 */
  ret = MHD_websocket_check_upgrade_header ("websocket,HTTP/2.0");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): websocket ,HTTP/2.0 */
  ret = MHD_websocket_check_upgrade_header (" websocket ,HTTP/2.0");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): HTTP/2.0, websocket */
  ret = MHD_websocket_check_upgrade_header ("HTTP/2.0, websocket ");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): websocket/13 */
  ret = MHD_websocket_check_upgrade_header ("websocket/13");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): WeBsOcKeT */
  ret = MHD_websocket_check_upgrade_header ("WeBsOcKeT");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): WEBSOCKET */
  ret = MHD_websocket_check_upgrade_header ("WEBSOCKET");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): All allowed token characters plus /, then websocket keyowrd */
  ret = MHD_websocket_check_upgrade_header (
    "!#$%&'*+-.^_`|~0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz/,websocket");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (success): Different, allowed whitespaces */
  ret = MHD_websocket_check_upgrade_header (" \twebsocket \t");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_upgrade_header ("\rwebsocket");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_upgrade_header ("\nwebsocket");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_upgrade_header ("\vwebsocket");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): Different, disallowed whitespaces */
  ret = MHD_websocket_check_upgrade_header ("\fwebsocket");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Invalid header syntax
  ------------------------------------------------------------------------------
  */
  /* Fail test: (empty string) */
  ret = MHD_websocket_check_upgrade_header ("");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: (Disallowed) multiple word token with the term "websocket" in it */
  ret = MHD_websocket_check_upgrade_header ("websocket or something");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: Invalid characters */
  ret = MHD_websocket_check_upgrade_header ("\"websocket\"");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Missing parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: NULL as upgrade */
  ret = MHD_websocket_check_upgrade_header (NULL);
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_upgrade_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  return failed != 0 ? 0x800 : 0x00;
}


/**
 * Test procedure for `MHD_websocket_check_version_header()`
 */
int
test_check_version_header ()
{
  int failed = 0;
  int ret;

  /*
  ------------------------------------------------------------------------------
    Check with valid Upgrade header syntax
  ------------------------------------------------------------------------------
  */
  /* Regular test: 13 */
  ret = MHD_websocket_check_version_header ("13");
  if (MHD_WEBSOCKET_STATUS_OK != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Version check edge cases
  ------------------------------------------------------------------------------
  */
  /* Edge test (fail): 14 */
  ret = MHD_websocket_check_version_header ("14");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): 12 */
  ret = MHD_websocket_check_version_header ("12");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): 0 */
  ret = MHD_websocket_check_version_header ("1");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): 1 */
  ret = MHD_websocket_check_version_header ("1");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): 130 */
  ret = MHD_websocket_check_version_header ("130");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Edge test (fail): " 13" */
  ret = MHD_websocket_check_version_header (" 13");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Invalid header syntax
  ------------------------------------------------------------------------------
  */
  /* Fail test: (empty string) */
  ret = MHD_websocket_check_version_header ("");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }
  /* Fail test: Invalid characters */
  ret = MHD_websocket_check_version_header ("abc");
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  /*
  ------------------------------------------------------------------------------
    Missing parameters
  ------------------------------------------------------------------------------
  */
  /* Fail test: NULL as version */
  ret = MHD_websocket_check_version_header (NULL);
  if (MHD_WEBSOCKET_STATUS_NO_WEBSOCKET_HANDSHAKE_HEADER != ret)
  {
    fprintf (stderr,
             "check_version_header test failed in line %u.\n",
             (unsigned int) __LINE__);
    ++failed;
  }

  return failed != 0 ? 0x1000 : 0x00;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv;  /* Unused. Silent compiler warning. */

  /* seed random number generator */
  srand ((unsigned long) time (NULL));

  /* perform tests */
  errorCount += test_inits ();
  errorCount += test_accept ();
  errorCount += test_decodes ();
  errorCount += test_encodes_text ();
  errorCount += test_encodes_binary ();
  errorCount += test_encodes_close ();
  errorCount += test_encodes_ping ();
  errorCount += test_encodes_pong ();
  errorCount += test_split_close_reason ();
  errorCount += test_check_http_version ();
  errorCount += test_check_connection_header ();
  errorCount += test_check_upgrade_header ();
  errorCount += test_check_version_header ();

  /* output result */
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);

  return errorCount != 0;       /* 0 == pass */
}
