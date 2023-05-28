/*
     This file is part of libmicrohttpd
     Copyright (C) 2007, 2013, 2019, 2020 Christian Grothoff
     Copyright (C) 2021 Evgeny Grin (Karlson2k)

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
 * @file test_postprocessor.c
 * @brief  Testcase for postprocessor
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */
#include "platform.h"
#include "microhttpd.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mhd_compat.h"

#ifndef WINDOWS
#include <unistd.h>
#endif

#ifndef MHD_DEBUG_PP
#define MHD_DEBUG_PP 0
#endif /* MHD_DEBUG_PP */

struct expResult
{
  const char *key;
  const char *fname;
  const char *cnt_type;
  const char *tr_enc;
  const char *data;
};

/**
 * Array of values that the value checker "wants".
 * Each series of checks should be terminated by
 * five NULL-entries.
 */
struct expResult exp_results[] = {
#define URL_NOVALUE1_DATA "abc&x=5"
#define URL_NOVALUE1_START 0
  {"abc", NULL, NULL, NULL, /* NULL */ ""}, /* change after API update */
  {"x", NULL, NULL, NULL, "5"},
#define URL_NOVALUE1_END (URL_NOVALUE1_START + 2)
#define URL_NOVALUE2_DATA "abc=&x=5"
#define URL_NOVALUE2_START URL_NOVALUE1_END
  {"abc", NULL, NULL, NULL, ""},
  {"x", NULL, NULL, NULL, "5"},
#define URL_NOVALUE2_END (URL_NOVALUE2_START + 2)
#define URL_NOVALUE3_DATA "xyz="
#define URL_NOVALUE3_START URL_NOVALUE2_END
  {"xyz", NULL, NULL, NULL, ""},
#define URL_NOVALUE3_END (URL_NOVALUE3_START + 1)
#define URL_NOVALUE4_DATA "xyz"
#define URL_NOVALUE4_START URL_NOVALUE3_END
  {"xyz", NULL, NULL, NULL, /* NULL */ ""}, /* change after API update */
#define URL_NOVALUE4_END (URL_NOVALUE4_START + 1)
#define URL_DATA "abc=def&x=5"
#define URL_START URL_NOVALUE4_END
  {"abc", NULL, NULL, NULL, "def"},
  {"x", NULL, NULL, NULL, "5"},
#define URL_END (URL_START + 2)
#define URL_ENC_DATA "space=%20&key%201=&crlf=%0D%0a&mix%09ed=%2001%0d%0A"
#define URL_ENC_START URL_END
  {"space", NULL, NULL, NULL, " "},
  {"key 1", NULL, NULL, NULL, ""},
  {"crlf", NULL, NULL, NULL, "\r\n"},
  {"mix\ted", NULL, NULL, NULL, " 01\r\n"},
#define URL_ENC_END (URL_ENC_START + 4)
  {NULL, NULL, NULL, NULL, NULL},
#define FORM_DATA \
  "--AaB03x\r\ncontent-disposition: form-data; name=\"field1\"\r\n\r\nJoe Blow\r\n--AaB03x\r\ncontent-disposition: form-data; name=\"pics\"; filename=\"file1.txt\"\r\nContent-Type: text/plain\r\nContent-Transfer-Encoding: binary\r\n\r\nfiledata\r\n--AaB03x--\r\n"
#define FORM_START (URL_ENC_END + 1)
  {"field1", NULL, NULL, NULL, "Joe Blow"},
  {"pics", "file1.txt", "text/plain", "binary", "filedata"},
#define FORM_END (FORM_START + 2)
  {NULL, NULL, NULL, NULL, NULL},
#define FORM_NESTED_DATA \
  "--AaB03x\r\ncontent-disposition: form-data; name=\"field1\"\r\n\r\nJane Blow\r\n--AaB03x\r\ncontent-disposition: form-data; name=\"pics\"\r\nContent-type: multipart/mixed, boundary=BbC04y\r\n\r\n--BbC04y\r\nContent-disposition: attachment; filename=\"file1.txt\"\r\nContent-Type: text/plain\r\n\r\nfiledata1\r\n--BbC04y\r\nContent-disposition: attachment; filename=\"file2.gif\"\r\nContent-type: image/gif\r\nContent-Transfer-Encoding: binary\r\n\r\nfiledata2\r\n--BbC04y--\r\n--AaB03x--"
#define FORM_NESTED_START (FORM_END + 1)
  {"field1", NULL, NULL, NULL, "Jane Blow"},
  {"pics", "file1.txt", "text/plain", NULL, "filedata1"},
  {"pics", "file2.gif", "image/gif", "binary", "filedata2"},
#define FORM_NESTED_END (FORM_NESTED_START + 3)
  {NULL, NULL, NULL, NULL, NULL},
#define URL_EMPTY_VALUE_DATA "key1=value1&key2=&key3="
#define URL_EMPTY_VALUE_START (FORM_NESTED_END + 1)
  {"key1", NULL, NULL, NULL, "value1"},
  {"key2", NULL, NULL, NULL, ""},
  {"key3", NULL, NULL, NULL, ""},
#define URL_EMPTY_VALUE_END (URL_EMPTY_VALUE_START + 3)
  {NULL, NULL, NULL, NULL, NULL}
};


static int
mismatch (const char *a, const char *b)
{
  if (a == b)
    return 0;
  if ((a == NULL) || (b == NULL))
    return 1;
  return 0 != strcmp (a, b);
}


static int
mismatch2 (const char *data, const char *expected, size_t offset, size_t size)
{
  if (data == expected)
    return 0;
  if ((data == NULL) || (expected == NULL))
    return 1;
  return 0 != memcmp (data, expected + offset, size);
}


static enum MHD_Result
value_checker (void *cls,
               enum MHD_ValueKind kind,
               const char *key,
               const char *filename,
               const char *content_type,
               const char *transfer_encoding,
               const char *data,
               uint64_t off,
               size_t size)
{
  unsigned int *idxp = cls;
  struct expResult *expect = exp_results + *idxp;
  (void) kind;  /* Unused. Silent compiler warning. */

#if MHD_DEBUG_PP
  fprintf (stderr,
           "VC: `%s' `%s' `%s' `%s' (+%u)`%.*s' (%d)\n",
           key ? key : "(NULL)",
           filename ? filename : "(NULL)",
           content_type ? content_type : "(NULL)",
           transfer_encoding ? transfer_encoding : "(NULL)",
           (unsigned int) off,
           (int) (data ? size : 6),
           data ? data : "(NULL)",
           (int) size);
#endif
  if (*idxp == (unsigned int) -1)
    exit (99);
  if ( (0 != off) && (0 == size) )
  {
    if (NULL == expect->data)
      *idxp += 1;
    return MHD_YES;
  }
  if ((expect->key == NULL) ||
      (0 != strcmp (key, expect->key)) ||
      (mismatch (filename, expect->fname)) ||
      (mismatch (content_type, expect->cnt_type)) ||
      (mismatch (transfer_encoding, expect->tr_enc)) ||
      (mismatch2 (data, expect->data, off, size)))
  {
    *idxp = (unsigned int) -1;
    fprintf (stderr,
             "Failed with: `%s' `%s' `%s' `%s' `%.*s'\n",
             key ? key : "(NULL)",
             filename ? filename : "(NULL)",
             content_type ? content_type : "(NULL)",
             transfer_encoding ? transfer_encoding : "(NULL)",
             (int) (data ? size : 6),
             data ? data : "(NULL)");
    fprintf (stderr,
             "Wanted: `%s' `%s' `%s' `%s' `%s'\n",
             expect->key ? expect->key : "(NULL)",
             expect->fname ? expect->fname : "(NULL)",
             expect->cnt_type ? expect->cnt_type : "(NULL)",
             expect->tr_enc ? expect->tr_enc : "(NULL)",
             expect->data ? expect->data : "(NULL)");
    fprintf (stderr,
             "Unexpected result: %d/%d/%d/%d/%d/%d\n",
             (expect->key == NULL),
             (NULL != expect->key) && (0 != strcmp (key, expect->key)),
             (mismatch (filename, expect->fname)),
             (mismatch (content_type, expect->cnt_type)),
             (mismatch (transfer_encoding, expect->tr_enc)),
             (mismatch2 (data, expect->data, off, size)));
    return MHD_NO;
  }
  if ( ( (NULL == expect->data) &&
         (0 == off + size) ) ||
       ( (NULL != expect->data) &&
         (off + size == strlen (expect->data)) ) )
    *idxp += 1;
  return MHD_YES;
}


static int
test_urlencoding_case (unsigned int want_start,
                       unsigned int want_end,
                       const char *url_data)
{
  size_t step;
  int errors = 0;
  const size_t size = strlen (url_data);

  for (step = 1; size >= step; ++step)
  {
    struct MHD_Connection connection;
    struct MHD_HTTP_Header header;
    struct MHD_PostProcessor *pp;
    unsigned int want_off = want_start;
    size_t i;

    memset (&connection, 0, sizeof (struct MHD_Connection));
    memset (&header, 0, sizeof (struct MHD_HTTP_Header));
    connection.headers_received = &header;
    header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
    header.value = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    header.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_TYPE);
    header.value_size =
      MHD_STATICSTR_LEN_ (MHD_HTTP_POST_ENCODING_FORM_URLENCODED);
    header.kind = MHD_HEADER_KIND;
    pp = MHD_create_post_processor (&connection,
                                    1024,
                                    &value_checker,
                                    &want_off);
    if (NULL == pp)
    {
      fprintf (stderr, "Failed to create post processor.\n"
               "Line: %u\n", (unsigned int) __LINE__);
      exit (50);
    }
    for (i = 0; size > i; i += step)
    {
      size_t left = size - i;
      if (MHD_YES != MHD_post_process (pp,
                                       &url_data[i],
                                       (left > step) ? step : left))
      {
        fprintf (stderr, "Failed to process the data.\n"
                 "i: %u. step: %u.\n"
                 "Line: %u\n", (unsigned) i, (unsigned) step,
                 (unsigned int) __LINE__);
        exit (49);
      }
    }
    MHD_destroy_post_processor (pp);
    if (want_off != want_end)
    {
      fprintf (stderr,
               "Test failed in line %u.\tStep: %u.\tData: \"%s\"\n" \
               " Got: %u\tExpected: %u\n",
               (unsigned int) __LINE__,
               (unsigned int) step,
               url_data,
               want_off,
               want_end);
      errors++;
    }
  }
  return errors;
}


static int
test_urlencoding (void)
{
  unsigned int errorCount = 0;

  errorCount += test_urlencoding_case (URL_START,
                                       URL_END,
                                       URL_DATA);
  errorCount += test_urlencoding_case (URL_ENC_START,
                                       URL_ENC_END,
                                       URL_ENC_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE1_START,
                                       URL_NOVALUE1_END,
                                       URL_NOVALUE1_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE2_START,
                                       URL_NOVALUE2_END,
                                       URL_NOVALUE2_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE3_START,
                                       URL_NOVALUE3_END,
                                       URL_NOVALUE3_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE4_START,
                                       URL_NOVALUE4_START, /* No advance */
                                       URL_NOVALUE4_DATA);
  errorCount += test_urlencoding_case (URL_EMPTY_VALUE_START,
                                       URL_EMPTY_VALUE_END,
                                       URL_EMPTY_VALUE_DATA);

  errorCount += test_urlencoding_case (URL_START,
                                       URL_END,
                                       URL_DATA "\n");
  errorCount += test_urlencoding_case (URL_ENC_START,
                                       URL_ENC_END,
                                       URL_ENC_DATA "\n");
  errorCount += test_urlencoding_case (URL_NOVALUE1_START,
                                       URL_NOVALUE1_END,
                                       URL_NOVALUE1_DATA "\n");
  errorCount += test_urlencoding_case (URL_NOVALUE2_START,
                                       URL_NOVALUE2_END,
                                       URL_NOVALUE2_DATA "\n");
  errorCount += test_urlencoding_case (URL_NOVALUE3_START,
                                       URL_NOVALUE3_END,
                                       URL_NOVALUE3_DATA "\n");
  errorCount += test_urlencoding_case (URL_NOVALUE4_START,
                                       URL_NOVALUE4_END, /* With advance */
                                       URL_NOVALUE4_DATA "\n");
  errorCount += test_urlencoding_case (URL_EMPTY_VALUE_START,
                                       URL_EMPTY_VALUE_END,
                                       URL_EMPTY_VALUE_DATA "\n");

  errorCount += test_urlencoding_case (URL_START,
                                       URL_END,
                                       "&&" URL_DATA);
  errorCount += test_urlencoding_case (URL_ENC_START,
                                       URL_ENC_END,
                                       "&&" URL_ENC_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE1_START,
                                       URL_NOVALUE1_END,
                                       "&&" URL_NOVALUE1_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE2_START,
                                       URL_NOVALUE2_END,
                                       "&&" URL_NOVALUE2_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE3_START,
                                       URL_NOVALUE3_END,
                                       "&&" URL_NOVALUE3_DATA);
  errorCount += test_urlencoding_case (URL_NOVALUE4_START,
                                       URL_NOVALUE4_START, /* No advance */
                                       "&&" URL_NOVALUE4_DATA);
  errorCount += test_urlencoding_case (URL_EMPTY_VALUE_START,
                                       URL_EMPTY_VALUE_END,
                                       "&&" URL_EMPTY_VALUE_DATA);
  if (0 != errorCount)
    fprintf (stderr,
             "Test failed in line %u with %u errors\n",
             (unsigned int) __LINE__,
             errorCount);
  return errorCount;
}


static int
test_multipart_garbage (void)
{
  struct MHD_Connection connection;
  struct MHD_HTTP_Header header;
  struct MHD_PostProcessor *pp;
  unsigned int want_off;
  size_t size = MHD_STATICSTR_LEN_ (FORM_DATA);
  size_t splitpoint;
  char xdata[MHD_STATICSTR_LEN_ (FORM_DATA) + 3];

  /* fill in evil garbage at the beginning */
  xdata[0] = '-';
  xdata[1] = 'x';
  xdata[2] = '\r';
  memcpy (&xdata[3], FORM_DATA, size);
  size += 3;
  for (splitpoint = 1; splitpoint < size; splitpoint++)
  {
    want_off = FORM_START;
    memset (&connection, 0, sizeof (struct MHD_Connection));
    memset (&header, 0, sizeof (struct MHD_HTTP_Header));
    connection.headers_received = &header;
    header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
    header.value =
      MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA ", boundary=AaB03x";
    header.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_TYPE);
    header.value_size = MHD_STATICSTR_LEN_ (
      MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA ", boundary=AaB03x");
    header.kind = MHD_HEADER_KIND;
    pp = MHD_create_post_processor (&connection,
                                    1024, &value_checker, &want_off);
    if (NULL == pp)
    {
      fprintf (stderr, "Failed to create post processor.\n"
               "Line: %u\n", (unsigned int) __LINE__);
      exit (50);
    }
    if (MHD_YES != MHD_post_process (pp, xdata, splitpoint))
    {
      fprintf (stderr,
               "Test failed in line %u at point %d\n",
               (unsigned int) __LINE__,
               (int) splitpoint);
      exit (49);
    }
    if (MHD_YES != MHD_post_process (pp, &xdata[splitpoint], size - splitpoint))
    {
      fprintf (stderr,
               "Test failed in line %u at point %d\n",
               (unsigned int) __LINE__,
               (int) splitpoint);
      exit (49);
    }
    MHD_destroy_post_processor (pp);
    if (want_off != FORM_END)
    {
      fprintf (stderr,
               "Test failed in line %u at point %d\n",
               (unsigned int) __LINE__,
               (int) splitpoint);
      return (int) splitpoint;
    }
  }
  return 0;
}


static int
test_multipart_splits (void)
{
  struct MHD_Connection connection;
  struct MHD_HTTP_Header header;
  struct MHD_PostProcessor *pp;
  unsigned int want_off;
  size_t size;
  size_t splitpoint;

  size = strlen (FORM_DATA);
  for (splitpoint = 1; splitpoint < size; splitpoint++)
  {
    want_off = FORM_START;
    memset (&connection, 0, sizeof (struct MHD_Connection));
    memset (&header, 0, sizeof (struct MHD_HTTP_Header));
    connection.headers_received = &header;
    header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
    header.value =
      MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA ", boundary=AaB03x";
    header.header_size = strlen (header.header);
    header.value_size = strlen (header.value);
    header.kind = MHD_HEADER_KIND;
    pp = MHD_create_post_processor (&connection,
                                    1024, &value_checker, &want_off);
    if (NULL == pp)
    {
      fprintf (stderr, "Failed to create post processor.\n"
               "Line: %u\n", (unsigned int) __LINE__);
      exit (50);
    }
    if (MHD_YES != MHD_post_process (pp, FORM_DATA, splitpoint))
    {
      fprintf (stderr,
               "Test failed in line %u at point %d\n",
               (unsigned int) __LINE__,
               (int) splitpoint);
      exit (49);
    }
    if (MHD_YES != MHD_post_process (pp, &FORM_DATA[splitpoint],
                                     size - splitpoint))
    {
      fprintf (stderr,
               "Test failed in line %u at point %d\n",
               (unsigned int) __LINE__,
               (int) splitpoint);
      exit (49);
    }
    MHD_destroy_post_processor (pp);
    if (want_off != FORM_END)
    {
      fprintf (stderr,
               "Test failed in line %u at point %d\n",
               (unsigned int) __LINE__,
               (int) splitpoint);
      return (int) splitpoint;
    }
  }
  return 0;
}


static int
test_multipart (void)
{
  struct MHD_Connection connection;
  struct MHD_HTTP_Header header;
  struct MHD_PostProcessor *pp;
  unsigned int want_off = FORM_START;
  size_t i;
  size_t delta;
  size_t size;

  memset (&connection, 0, sizeof (struct MHD_Connection));
  memset (&header, 0, sizeof (struct MHD_HTTP_Header));
  connection.headers_received = &header;
  header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
  header.value =
    MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA ", boundary=AaB03x";
  header.kind = MHD_HEADER_KIND;
  header.header_size = strlen (header.header);
  header.value_size = strlen (header.value);
  pp = MHD_create_post_processor (&connection,
                                  1024, &value_checker, &want_off);
  if (NULL == pp)
  {
    fprintf (stderr, "Failed to create post processor.\n"
             "Line: %u\n", (unsigned int) __LINE__);
    exit (50);
  }
  i = 0;
  size = strlen (FORM_DATA);
  while (i < size)
  {
    delta = 1 + MHD_random_ () % (size - i);
    if (MHD_YES != MHD_post_process (pp,
                                     &FORM_DATA[i],
                                     delta))
    {
      fprintf (stderr, "Failed to process the data.\n"
               "i: %u. delta: %u.\n"
               "Line: %u\n", (unsigned) i, (unsigned) delta,
               (unsigned int) __LINE__);
      exit (49);
    }
    i += delta;
  }
  MHD_destroy_post_processor (pp);
  if (want_off != FORM_END)
  {
    fprintf (stderr,
             "Test failed in line %u\n",
             (unsigned int) __LINE__);
    return 2;
  }
  return 0;
}


static int
test_nested_multipart (void)
{
  struct MHD_Connection connection;
  struct MHD_HTTP_Header header;
  struct MHD_PostProcessor *pp;
  unsigned int want_off = FORM_NESTED_START;
  size_t i;
  size_t delta;
  size_t size;

  memset (&connection, 0, sizeof (struct MHD_Connection));
  memset (&header, 0, sizeof (struct MHD_HTTP_Header));
  connection.headers_received = &header;
  header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
  header.value =
    MHD_HTTP_POST_ENCODING_MULTIPART_FORMDATA ", boundary=AaB03x";
  header.kind = MHD_HEADER_KIND;
  header.header_size = strlen (header.header);
  header.value_size = strlen (header.value);
  pp = MHD_create_post_processor (&connection,
                                  1024, &value_checker, &want_off);
  if (NULL == pp)
  {
    fprintf (stderr, "Failed to create post processor.\n"
             "Line: %u\n", (unsigned int) __LINE__);
    exit (50);
  }
  i = 0;
  size = strlen (FORM_NESTED_DATA);
  while (i < size)
  {
    delta = 1 + MHD_random_ () % (size - i);
    if (MHD_YES != MHD_post_process (pp,
                                     &FORM_NESTED_DATA[i],
                                     delta))
    {
      fprintf (stderr, "Failed to process the data.\n"
               "i: %u. delta: %u.\n"
               "Line: %u\n", (unsigned) i, (unsigned) delta,
               (unsigned int) __LINE__);
      exit (49);
    }
    i += delta;
  }
  MHD_destroy_post_processor (pp);
  if (want_off != FORM_NESTED_END)
  {
    fprintf (stderr,
             "Test failed in line %u\n",
             (unsigned int) __LINE__);
    return 4;
  }
  return 0;
}


static enum MHD_Result
value_checker2 (void *cls,
                enum MHD_ValueKind kind,
                const char *key,
                const char *filename,
                const char *content_type,
                const char *transfer_encoding,
                const char *data,
                uint64_t off,
                size_t size)
{
  (void) cls; (void) kind; (void) key; /* Mute compiler warnings */
  (void) filename; (void) content_type; (void) transfer_encoding;
  (void) data; (void) off; (void) size;
  return MHD_YES;
}


static int
test_overflow ()
{
  struct MHD_Connection connection;
  struct MHD_HTTP_Header header;
  struct MHD_PostProcessor *pp;
  size_t i;
  size_t j;
  size_t delta;
  char *buf;

  memset (&connection, 0, sizeof (struct MHD_Connection));
  memset (&header, 0, sizeof (struct MHD_HTTP_Header));
  connection.headers_received = &header;
  header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
  header.value = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
  header.header_size = strlen (header.header);
  header.value_size = strlen (header.value);
  header.kind = MHD_HEADER_KIND;
  for (i = 128; i < 1024 * 1024; i += 1024)
  {
    pp = MHD_create_post_processor (&connection,
                                    1024,
                                    &value_checker2,
                                    NULL);
    if (NULL == pp)
    {
      fprintf (stderr, "Failed to create post processor.\n"
               "Line: %u\n", (unsigned int) __LINE__);
      exit (50);
    }
    buf = malloc (i);
    if (NULL == buf)
      return 1;
    memset (buf, 'A', i);
    buf[i / 2] = '=';
    delta = 1 + (MHD_random_ () % (i - 1));
    j = 0;
    while (j < i)
    {
      if (j + delta > i)
        delta = i - j;
      if (MHD_NO ==
          MHD_post_process (pp,
                            &buf[j],
                            delta))
        break;
      j += delta;
    }
    free (buf);
    MHD_destroy_post_processor (pp);
  }
  return 0;
}


static int
test_empty_key (void)
{
  const char form_data[] = "=abcdef";
  size_t step;
  const size_t size = MHD_STATICSTR_LEN_ (form_data);

  for (step = 1; size >= step; ++step)
  {
    size_t i;
    struct MHD_Connection connection;
    struct MHD_HTTP_Header header;
    struct MHD_PostProcessor *pp;
    memset (&connection, 0, sizeof (struct MHD_Connection));
    memset (&header, 0, sizeof (struct MHD_HTTP_Header));

    connection.headers_received = &header;
    connection.headers_received_tail = &header;
    header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
    header.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_TYPE);
    header.value = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    header.value_size =
      MHD_STATICSTR_LEN_ (MHD_HTTP_POST_ENCODING_FORM_URLENCODED);
    header.kind = MHD_HEADER_KIND;
    pp = MHD_create_post_processor (&connection,
                                    1024, &value_checker2, NULL);
    if (NULL == pp)
    {
      fprintf (stderr, "Failed to create post processor.\n"
               "Line: %u\n", (unsigned int) __LINE__);
      exit (50);
    }
    for (i = 0; size > i; i += step)
    {
      if (MHD_NO != MHD_post_process (pp,
                                      form_data + i,
                                      (step > size - i) ? (size - i) : step))
      {
        fprintf (stderr, "Succeed to process the broken data.\n"
                 "i: %u. step: %u.\n"
                 "Line: %u\n", (unsigned) i, (unsigned) step,
                 (unsigned int) __LINE__);
        exit (49);
      }
    }
    MHD_destroy_post_processor (pp);
  }
  return 0;
}


static int
test_double_value (void)
{
  const char form_data[] = URL_DATA "=abcdef";
  size_t step;
  const size_t size = MHD_STATICSTR_LEN_ (form_data);
  const size_t safe_size = MHD_STATICSTR_LEN_ (URL_DATA);

  for (step = 1; size >= step; ++step)
  {
    size_t i;
    struct MHD_Connection connection;
    struct MHD_HTTP_Header header;
    struct MHD_PostProcessor *pp;
    unsigned int results_off = URL_START;
    unsigned int results_final = results_off + 1; /* First value is correct */
    memset (&connection, 0, sizeof (struct MHD_Connection));
    memset (&header, 0, sizeof (struct MHD_HTTP_Header));

    connection.headers_received = &header;
    connection.headers_received_tail = &header;
    header.header = MHD_HTTP_HEADER_CONTENT_TYPE;
    header.header_size = MHD_STATICSTR_LEN_ (MHD_HTTP_HEADER_CONTENT_TYPE);
    header.value = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    header.value_size =
      MHD_STATICSTR_LEN_ (MHD_HTTP_POST_ENCODING_FORM_URLENCODED);
    header.kind = MHD_HEADER_KIND;
    pp = MHD_create_post_processor (&connection,
                                    1024, &value_checker, &results_off);
    if (NULL == pp)
    {
      fprintf (stderr, "Failed to create post processor.\n"
               "Line: %u\n", (unsigned int) __LINE__);
      exit (50);
    }
    for (i = 0; size > i; i += step)
    {
      if (MHD_NO != MHD_post_process (pp,
                                      form_data + i,
                                      (step > size - i) ? (size - i) : step))
      {
        if (safe_size == i + step)
          results_final = URL_END;
        if (safe_size < i + step)
        {
          fprintf (stderr, "Succeed to process the broken data.\n"
                   "i: %u. step: %u.\n"
                   "Line: %u\n", (unsigned) i, (unsigned) step,
                   (unsigned int) __LINE__);
          exit (49);
        }
      }
      else
      {
        if (safe_size >= i + step)
        {
          fprintf (stderr, "Failed to process the data.\n"
                   "i: %u. step: %u.\n"
                   "Line: %u\n", (unsigned) i, (unsigned) step,
                   (unsigned int) __LINE__);
          exit (49);
        }
      }
    }
    MHD_destroy_post_processor (pp);
    if (results_final != results_off)
    {
      fprintf (stderr,
               "Test failed in line %u.\tStep:%u\n Got: %u\tExpected: %u\n",
               (unsigned int) __LINE__,
               (unsigned int) step,
               results_off,
               results_final);
      return 1;
    }
  }
  return 0;
}


int
main (int argc, char *const *argv)
{
  unsigned int errorCount = 0;
  (void) argc; (void) argv;  /* Unused. Silent compiler warning. */

  errorCount += test_multipart_splits ();
  errorCount += test_multipart_garbage ();
  errorCount += test_urlencoding ();
  errorCount += test_multipart ();
  errorCount += test_nested_multipart ();
  errorCount += test_empty_key ();
  errorCount += test_double_value ();
  errorCount += test_overflow ();
  if (errorCount != 0)
    fprintf (stderr, "Error (code: %u)\n", errorCount);
  return errorCount != 0;       /* 0 == pass */
}
