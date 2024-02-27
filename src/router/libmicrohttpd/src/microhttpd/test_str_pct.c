/*
  This file is part of libmicrohttpd
  Copyright (C) 2022 Karlson2k (Evgeny Grin)

  This test tool is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2, or
  (at your option) any later version.

  This test tool is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/test_str_pct.c
 * @brief  Unit tests for percent (URL) encoded strings processing
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <string.h>
#include <stdio.h>
#include "mhd_str.h"
#include "mhd_assert.h"

#ifndef MHD_STATICSTR_LEN_
/**
 * Determine length of static string / macro strings at compile time.
 */
#define MHD_STATICSTR_LEN_(macro) (sizeof(macro) / sizeof(char) - 1)
#endif /* ! MHD_STATICSTR_LEN_ */


static char tmp_bufs[4][4 * 1024]; /* should be enough for testing */
static size_t buf_idx = 0;

/* print non-printable chars as char codes */
static char *
n_prnt (const char *str, size_t len)
{
  static char *buf;  /* should be enough for testing */
  static const size_t buf_size = sizeof(tmp_bufs[0]);
  size_t r_pos = 0;
  size_t w_pos = 0;
  if (++buf_idx >= (sizeof(tmp_bufs) / sizeof(tmp_bufs[0])))
    buf_idx = 0;
  buf = tmp_bufs[buf_idx];

  while (len > r_pos && w_pos + 1 < buf_size)
  {
    const unsigned char c = (unsigned char) str[r_pos];
    if ((c == '\\') || (c == '"') )
    {
      if (w_pos + 2 >= buf_size)
        break;
      buf[w_pos++] = '\\';
      buf[w_pos++] = (char) c;
    }
    else if ((c >= 0x20) && (c <= 0x7E) )
      buf[w_pos++] = (char) c;
    else
    {
      if (w_pos + 4 >= buf_size)
        break;
      if (snprintf (buf + w_pos, buf_size - w_pos, "\\x%02hX", (short unsigned
                                                                int) c) != 4)
        break;
      w_pos += 4;
    }
    r_pos++;
  }

  if (len != r_pos)
  {   /* not full string is printed */
      /* enough space for "..." ? */
    if (w_pos + 3 > buf_size)
      w_pos = buf_size - 4;
    buf[w_pos++] = '.';
    buf[w_pos++] = '.';
    buf[w_pos++] = '.';
  }
  buf[w_pos] = 0;
  return buf;
}


#define TEST_BIN_MAX_SIZE 1024

/* return zero if succeed, number of failures otherwise */
static unsigned int
expect_decoded_n (const char *const encoded, const size_t encoded_len,
                  const char *const decoded, const size_t decoded_size,
                  const unsigned int line_num)
{
  static const char fill_chr = '#';
  static char buf[TEST_BIN_MAX_SIZE];
  size_t res_size;
  unsigned int ret;

  mhd_assert (NULL != encoded);
  mhd_assert (NULL != decoded);
  mhd_assert (TEST_BIN_MAX_SIZE > decoded_size + 1);
  mhd_assert (TEST_BIN_MAX_SIZE > encoded_len + 1);
  mhd_assert (encoded_len >= decoded_size);

  ret = 0;

  /* check MHD_str_pct_decode_strict_n_() with small out buffer */
  if (1)
  {
    unsigned int check_res = 0;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_strict_n_ (encoded, encoded_len, buf,
                                             decoded_size + 1);
    if (res_size != decoded_size)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    else
    {
      if (fill_chr != buf[res_size])
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
                 "A char written outside the buffer:\n");
      }
      else
      {
        memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
        res_size = MHD_str_pct_decode_strict_n_ (encoded, encoded_len, buf,
                                                 decoded_size);
        if (res_size != decoded_size)
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
                   "Wrong returned value:\n");
        }
      }
      if ((res_size == decoded_size) && (0 != decoded_size) &&
          (0 != memcmp (buf, decoded, decoded_size)))
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
                 "Wrong output string:\n");
      }
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->\"%s\", %u) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) decoded_size,
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->\"%s\", %u) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (decoded, decoded_size), (unsigned) decoded_size,
               (unsigned) decoded_size);
    }
  }

  /* check MHD_str_pct_decode_strict_n_() with large out buffer */
  if (1)
  {
    unsigned int check_res = 0;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_strict_n_ (encoded, encoded_len, buf,
                                             encoded_len + 1);
    if (res_size != decoded_size)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    else
    {
      if (fill_chr != buf[res_size])
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
                 "A char written outside the buffer:\n");
      }
      if ((res_size == decoded_size) && (0 != decoded_size) &&
          (0 != memcmp (buf, decoded, decoded_size)))
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
                 "Wrong output string:\n");
      }
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->\"%s\", %u) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) (encoded_len + 1),
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->\"%s\", %u) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (decoded, decoded_size), (unsigned) (encoded_len + 1),
               (unsigned) decoded_size);
    }
  }

  /* check MHD_str_pct_decode_lenient_n_() with small out buffer */
  if (1)
  {
    unsigned int check_res = 0;
    bool is_broken = true;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_lenient_n_ (encoded, encoded_len, buf,
                                              decoded_size + 1, &is_broken);
    if (res_size != decoded_size)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    else
    {
      if (fill_chr != buf[res_size])
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "A char written outside the buffer:\n");
      }
      else
      {
        is_broken = true;
        memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
        res_size = MHD_str_pct_decode_lenient_n_ (encoded, encoded_len, buf,
                                                  decoded_size, &is_broken);
        if (res_size != decoded_size)
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                   "Wrong returned value:\n");
        }
      }
      if (is_broken)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong 'broken_encoding' result:\n");
      }
      if ((res_size == decoded_size) && (0 != decoded_size) &&
          (0 != memcmp (buf, decoded, decoded_size)))
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong output string:\n");
      }
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->%s) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) decoded_size,
               is_broken ? "true" : "false",
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->false) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (decoded, decoded_size), (unsigned) decoded_size,
               (unsigned) decoded_size);
    }
  }

  /* check MHD_str_pct_decode_lenient_n_() with large out buffer */
  if (1)
  {
    unsigned int check_res = 0;
    bool is_broken = true;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_lenient_n_ (encoded, encoded_len, buf,
                                              encoded_len + 1, &is_broken);
    if (res_size != decoded_size)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    else
    {
      if (fill_chr != buf[res_size])
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "A char written outside the buffer:\n");
      }
      if (is_broken)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong 'broken_encoding' result:\n");
      }
      if ((res_size == decoded_size) && (0 != decoded_size) &&
          (0 != memcmp (buf, decoded, decoded_size)))
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong output string:\n");
      }
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->%s) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) (encoded_len + 1),
               is_broken ? "true" : "false",
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->false) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (decoded, decoded_size), (unsigned) (encoded_len + 1),
               (unsigned) decoded_size);
    }
  }

  if (strlen (encoded) == encoded_len)
  {
    /* check MHD_str_pct_decode_in_place_strict_() */
    if (1)
    {
      unsigned int check_res = 0;

      memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
      memcpy (buf, encoded, encoded_len);
      buf[encoded_len] = 0;
      res_size = MHD_str_pct_decode_in_place_strict_ (buf);
      if (res_size != decoded_size)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_in_place_strict_ ()' FAILED: "
                 "Wrong returned value:\n");
      }
      else
      {
        if (0 != buf[res_size])
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_strict_ ()' FAILED: "
                   "The result is not zero-terminated:\n");
        }
        if (((res_size + 1) < encoded_len) ?
            (encoded[res_size + 1] != buf[res_size + 1]) :
            (fill_chr != buf[res_size + 1]))
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_strict_ ()' FAILED: "
                   "A char written outside the buffer:\n");
        }
        if ((res_size == decoded_size) && (0 != decoded_size) &&
            (0 != memcmp (buf, decoded, decoded_size)))
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_strict_ ()' FAILED: "
                   "Wrong output string:\n");
        }
      }
      if (0 != check_res)
      {
        ret++;
        fprintf (stderr,
                 "\tRESULT  : MHD_str_pct_decode_in_place_strict_ (\"%s\" "
                 "-> \"%s\") -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (buf, res_size),
                 (unsigned) res_size);
        fprintf (stderr,
                 "\tEXPECTED: MHD_str_pct_decode_in_place_strict_ (\"%s\" "
                 "-> \"%s\") -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (decoded, decoded_size),
                 (unsigned) decoded_size);
      }
    }

    /* check MHD_str_pct_decode_in_place_lenient_() */
    if (1)
    {
      unsigned int check_res = 0;
      bool is_broken = true;

      memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
      memcpy (buf, encoded, encoded_len);
      buf[encoded_len] = 0;
      res_size = MHD_str_pct_decode_in_place_lenient_ (buf, &is_broken);
      if (res_size != decoded_size)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                 "Wrong returned value:\n");
      }
      else
      {
        if (0 != buf[res_size])
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "The result is not zero-terminated:\n");
        }
        if (((res_size + 1) < encoded_len) ?
            (encoded[res_size + 1] != buf[res_size + 1]) :
            (fill_chr != buf[res_size + 1]))
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "A char written outside the buffer:\n");
        }
        if (is_broken)
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "Wrong 'broken_encoding' result:\n");
        }
        if ((res_size == decoded_size) && (0 != decoded_size) &&
            (0 != memcmp (buf, decoded, decoded_size)))
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "Wrong output string:\n");
        }
      }
      if (0 != check_res)
      {
        ret++;
        fprintf (stderr,
                 "\tRESULT  : MHD_str_pct_decode_in_place_lenient_ (\"%s\" "
                 "-> \"%s\", ->%s) -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (buf, res_size),
                 is_broken ? "true" : "false",
                 (unsigned) res_size);
        fprintf (stderr,
                 "\tEXPECTED: MHD_str_pct_decode_in_place_lenient_ (\"%s\" "
                 "-> \"%s\", ->false) -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (decoded, decoded_size),
                 (unsigned) decoded_size);
      }
    }
  }

  if (0 != ret)
  {
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }
  return ret;
}


#define expect_decoded(e,d) \
        expect_decoded_n(e,MHD_STATICSTR_LEN_(e),\
                         d,MHD_STATICSTR_LEN_(d), \
                         __LINE__)

static unsigned int
check_decode_str (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_decoded ("", "");

  /* Base sequences without percent symbol */
  r += expect_decoded ("aaa", "aaa");
  r += expect_decoded ("bbb", "bbb");
  r += expect_decoded ("ccc", "ccc");
  r += expect_decoded ("ddd", "ddd");
  r += expect_decoded ("lll", "lll");
  r += expect_decoded ("mmm", "mmm");
  r += expect_decoded ("nnn", "nnn");
  r += expect_decoded ("ooo", "ooo");
  r += expect_decoded ("www", "www");
  r += expect_decoded ("xxx", "xxx");
  r += expect_decoded ("yyy", "yyy");
  r += expect_decoded ("zzz", "zzz");
  r += expect_decoded ("AAA", "AAA");
  r += expect_decoded ("GGG", "GGG");
  r += expect_decoded ("MMM", "MMM");
  r += expect_decoded ("TTT", "TTT");
  r += expect_decoded ("ZZZ", "ZZZ");
  r += expect_decoded ("012", "012");
  r += expect_decoded ("345", "345");
  r += expect_decoded ("678", "678");
  r += expect_decoded ("901", "901");
  r += expect_decoded ("aaaaaa", "aaaaaa");
  r += expect_decoded ("bbbbbb", "bbbbbb");
  r += expect_decoded ("cccccc", "cccccc");
  r += expect_decoded ("dddddd", "dddddd");
  r += expect_decoded ("llllll", "llllll");
  r += expect_decoded ("mmmmmm", "mmmmmm");
  r += expect_decoded ("nnnnnn", "nnnnnn");
  r += expect_decoded ("oooooo", "oooooo");
  r += expect_decoded ("wwwwww", "wwwwww");
  r += expect_decoded ("xxxxxx", "xxxxxx");
  r += expect_decoded ("yyyyyy", "yyyyyy");
  r += expect_decoded ("zzzzzz", "zzzzzz");
  r += expect_decoded ("AAAAAA", "AAAAAA");
  r += expect_decoded ("GGGGGG", "GGGGGG");
  r += expect_decoded ("MMMMMM", "MMMMMM");
  r += expect_decoded ("TTTTTT", "TTTTTT");
  r += expect_decoded ("ZZZZZZ", "ZZZZZZ");
  r += expect_decoded ("012012", "012012");
  r += expect_decoded ("345345", "345345");
  r += expect_decoded ("678678", "678678");
  r += expect_decoded ("901901", "901901");
  r += expect_decoded ("a", "a");
  r += expect_decoded ("bc", "bc");
  r += expect_decoded ("DEFG", "DEFG");
  r += expect_decoded ("123t", "123t");
  r += expect_decoded ("12345", "12345");
  r += expect_decoded ("TestStr", "TestStr");
  r += expect_decoded ("Teststring", "Teststring");
  r += expect_decoded ("Teststring.", "Teststring.");
  r += expect_decoded ("Longerstring", "Longerstring");
  r += expect_decoded ("Longerstring.", "Longerstring.");
  r += expect_decoded ("Longerstring2.", "Longerstring2.");

  /* Simple percent-encoded strings */
  r += expect_decoded ("Test%20string", "Test string");
  r += expect_decoded ("Test%3Fstring.", "Test?string.");
  r += expect_decoded ("100%25", "100%");
  r += expect_decoded ("a%2C%20b%3Dc%26e%3Dg", "a, b=c&e=g");
  r += expect_decoded ("%20%21%23%24%25%26%27%28%29%2A%2B%2C"
                       "%2F%3A%3B%3D%3F%40%5B%5D%09",
                       " !#$%&'()*+,/:;=?@[]\t");

  return r;
}


#define expect_decoded_arr(e,a) \
        expect_decoded_n(e,MHD_STATICSTR_LEN_(e),\
                         (const char *)a,(sizeof(a)/sizeof(a[0])), \
                         __LINE__)

static unsigned int
check_decode_bin (void)
{
  unsigned int r = 0; /**< The number of errors */

  if (1)
  {
    static const uint8_t bin[256] =
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe,
     0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
     0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
     0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
     0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
     0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
     0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
     0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62,
     0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
     0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
     0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
     0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92,
     0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e,
     0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
     0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
     0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2,
     0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce,
     0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
     0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
     0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2,
     0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe,
     0xff };
    /* The lower case */
    r += expect_decoded_arr ("%00%01%02%03%04%05%06%07%08%09%0a%0b%0c%0d%0e" \
                             "%0f%10%11%12%13%14%15%16%17%18%19%1a%1b%1c%1d" \
                             "%1e%1f%20%21%22%23%24%25%26%27%28%29%2a%2b%2c" \
                             "%2d%2e%2f%30%31%32%33%34%35%36%37%38%39%3a%3b" \
                             "%3c%3d%3e%3f%40%41%42%43%44%45%46%47%48%49%4a" \
                             "%4b%4c%4d%4e%4f%50%51%52%53%54%55%56%57%58%59" \
                             "%5a%5b%5c%5d%5e%5f%60%61%62%63%64%65%66%67%68" \
                             "%69%6a%6b%6c%6d%6e%6f%70%71%72%73%74%75%76%77" \
                             "%78%79%7a%7b%7c%7d%7e%7f%80%81%82%83%84%85%86" \
                             "%87%88%89%8a%8b%8c%8d%8e%8f%90%91%92%93%94%95" \
                             "%96%97%98%99%9a%9b%9c%9d%9e%9f%a0%a1%a2%a3%a4" \
                             "%a5%a6%a7%a8%a9%aa%ab%ac%ad%ae%af%b0%b1%b2%b3" \
                             "%b4%b5%b6%b7%b8%b9%ba%bb%bc%bd%be%bf%c0%c1%c2" \
                             "%c3%c4%c5%c6%c7%c8%c9%ca%cb%cc%cd%ce%cf%d0%d1" \
                             "%d2%d3%d4%d5%d6%d7%d8%d9%da%db%dc%dd%de%df%e0" \
                             "%e1%e2%e3%e4%e5%e6%e7%e8%e9%ea%eb%ec%ed%ee%ef" \
                             "%f0%f1%f2%f3%f4%f5%f6%f7%f8%f9%fa%fb%fc%fd%fe" \
                             "%ff", bin);
  }

  if (1)
  {
    static const uint8_t bin[256] =
    {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe,
     0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
     0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
     0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32,
     0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e,
     0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,
     0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56,
     0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x62,
     0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
     0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,
     0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
     0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92,
     0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e,
     0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
     0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
     0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2,
     0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce,
     0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
     0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
     0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2,
     0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe,
     0xff };
    /* The upper case */
    r += expect_decoded_arr ("%00%01%02%03%04%05%06%07%08%09%0A%0B%0C%0D%0E" \
                             "%0F%10%11%12%13%14%15%16%17%18%19%1A%1B%1C%1D" \
                             "%1E%1F%20%21%22%23%24%25%26%27%28%29%2A%2B%2C" \
                             "%2D%2E%2F%30%31%32%33%34%35%36%37%38%39%3A%3B" \
                             "%3C%3D%3E%3F%40%41%42%43%44%45%46%47%48%49%4A" \
                             "%4B%4C%4D%4E%4F%50%51%52%53%54%55%56%57%58%59" \
                             "%5A%5B%5C%5D%5E%5F%60%61%62%63%64%65%66%67%68" \
                             "%69%6A%6B%6C%6D%6E%6F%70%71%72%73%74%75%76%77" \
                             "%78%79%7A%7B%7C%7D%7E%7F%80%81%82%83%84%85%86" \
                             "%87%88%89%8A%8B%8C%8D%8E%8F%90%91%92%93%94%95" \
                             "%96%97%98%99%9A%9B%9C%9D%9E%9F%A0%A1%A2%A3%A4" \
                             "%A5%A6%A7%A8%A9%AA%AB%AC%AD%AE%AF%B0%B1%B2%B3" \
                             "%B4%B5%B6%B7%B8%B9%BA%BB%BC%BD%BE%BF%C0%C1%C2" \
                             "%C3%C4%C5%C6%C7%C8%C9%CA%CB%CC%CD%CE%CF%D0%D1" \
                             "%D2%D3%D4%D5%D6%D7%D8%D9%DA%DB%DC%DD%DE%DF%E0" \
                             "%E1%E2%E3%E4%E5%E6%E7%E8%E9%EA%EB%EC%ED%EE%EF" \
                             "%F0%F1%F2%F3%F4%F5%F6%F7%F8%F9%FA%FB%FC%FD%FE" \
                             "%FF", bin);
  }

  return r;
}


/* return zero if succeed, number of failures otherwise */
static unsigned int
expect_decoded_bad_n (const char *const encoded, const size_t encoded_len,
                      const char *const decoded, const size_t decoded_size,
                      const unsigned int line_num)
{
  static const char fill_chr = '#';
  static char buf[TEST_BIN_MAX_SIZE];
  size_t res_size;
  unsigned int ret;

  mhd_assert (NULL != encoded);
  mhd_assert (NULL != decoded);
  mhd_assert (TEST_BIN_MAX_SIZE > decoded_size + 1);
  mhd_assert (TEST_BIN_MAX_SIZE > encoded_len + 1);
  mhd_assert (encoded_len >= decoded_size);

  ret = 0;

  /* check MHD_str_pct_decode_strict_n_() with small out buffer */
  if (1)
  {
    unsigned int check_res = 0;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_strict_n_ (encoded, encoded_len, buf,
                                             decoded_size);
    if (res_size != 0)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->\"%s\", %u) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) decoded_size,
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->(not defined), %u) -> 0\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               (unsigned) decoded_size);
    }
  }

  /* check MHD_str_pct_decode_strict_n_() with large out buffer */
  if (1)
  {
    unsigned int check_res = 0;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_strict_n_ (encoded, encoded_len, buf,
                                             encoded_len + 1);
    if (res_size != 0)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_strict_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->\"%s\", %u) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) (encoded_len + 1),
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_strict_n_ (\"%s\", %u, "
               "->(not defined), %u) -> 0\n",
               n_prnt (encoded, encoded_len), (unsigned) (encoded_len + 1),
               (unsigned) decoded_size);
    }
  }

  /* check MHD_str_pct_decode_lenient_n_() with small out buffer */
  if (1)
  {
    unsigned int check_res = 0;
    bool is_broken = false;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_lenient_n_ (encoded, encoded_len, buf,
                                              decoded_size + 1, &is_broken);
    if (res_size != decoded_size)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    else
    {
      if (fill_chr != buf[res_size])
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "A char written outside the buffer:\n");
      }
      else
      {
        is_broken = false;
        memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
        res_size = MHD_str_pct_decode_lenient_n_ (encoded, encoded_len, buf,
                                                  decoded_size, &is_broken);
        if (res_size != decoded_size)
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                   "Wrong returned value:\n");
        }
      }
      if (! is_broken)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong 'broken_encoding' result:\n");
      }
      if ((res_size == decoded_size) && (0 != decoded_size) &&
          (0 != memcmp (buf, decoded, decoded_size)))
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong output string:\n");
      }
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->%s) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) decoded_size,
               is_broken ? "true" : "false",
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->true) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (decoded, decoded_size), (unsigned) decoded_size,
               (unsigned) decoded_size);
    }
  }

  /* check MHD_str_pct_decode_lenient_n_() with large out buffer */
  if (1)
  {
    unsigned int check_res = 0;
    bool is_broken = false;

    memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
    res_size = MHD_str_pct_decode_lenient_n_ (encoded, encoded_len, buf,
                                              encoded_len + 1, &is_broken);
    if (res_size != decoded_size)
    {
      check_res = 1;
      fprintf (stderr,
               "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
               "Wrong returned value:\n");
    }
    else
    {
      if (fill_chr != buf[res_size])
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "A char written outside the buffer:\n");
      }
      if (! is_broken)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong 'broken_encoding' result:\n");
      }
      if ((res_size == decoded_size) && (0 != decoded_size) &&
          (0 != memcmp (buf, decoded, decoded_size)))
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_lenient_n_ ()' FAILED: "
                 "Wrong output string:\n");
      }
    }
    if (0 != check_res)
    {
      ret++;
      fprintf (stderr,
               "\tRESULT  : MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->%s) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (buf, res_size), (unsigned) (encoded_len + 1),
               is_broken ? "true" : "false",
               (unsigned) res_size);
      fprintf (stderr,
               "\tEXPECTED: MHD_str_pct_decode_lenient_n_ (\"%s\", %u, "
               "->\"%s\", %u, ->true) -> %u\n",
               n_prnt (encoded, encoded_len), (unsigned) encoded_len,
               n_prnt (decoded, decoded_size), (unsigned) (encoded_len + 1),
               (unsigned) decoded_size);
    }
  }

  if (strlen (encoded) == encoded_len)
  {
    /* check MHD_str_pct_decode_in_place_strict_() */
    if (1)
    {
      unsigned int check_res = 0;

      memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
      memcpy (buf, encoded, encoded_len);
      buf[encoded_len] = 0;
      res_size = MHD_str_pct_decode_in_place_strict_ (buf);
      if (res_size != 0)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_in_place_strict_ ()' FAILED: "
                 "Wrong returned value:\n");
      }
      if (0 != check_res)
      {
        ret++;
        fprintf (stderr,
                 "\tRESULT  : MHD_str_pct_decode_in_place_strict_ (\"%s\" "
                 "-> \"%s\") -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (buf, res_size),
                 (unsigned) res_size);
        fprintf (stderr,
                 "\tEXPECTED: MHD_str_pct_decode_in_place_strict_ (\"%s\" "
                 "-> (not defined)) -> 0\n",
                 n_prnt (encoded, encoded_len));
      }
    }

    /* check MHD_str_pct_decode_in_place_lenient_() */
    if (1)
    {
      unsigned int check_res = 0;
      bool is_broken = false;

      memset (buf, fill_chr, sizeof(buf)); /* Fill buffer with some character */
      memcpy (buf, encoded, encoded_len);
      buf[encoded_len] = 0;
      res_size = MHD_str_pct_decode_in_place_lenient_ (buf, &is_broken);
      if (res_size != decoded_size)
      {
        check_res = 1;
        fprintf (stderr,
                 "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                 "Wrong returned value:\n");
      }
      else
      {
        if (0 != buf[res_size])
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "The result is not zero-terminated:\n");
        }
        if (((res_size + 1) < encoded_len) ?
            (encoded[res_size + 1] != buf[res_size + 1]) :
            (fill_chr != buf[res_size + 1]))
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "A char written outside the buffer:\n");
        }
        if (! is_broken)
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "Wrong 'broken_encoding' result:\n");
        }
        if ((res_size == decoded_size) && (0 != decoded_size) &&
            (0 != memcmp (buf, decoded, decoded_size)))
        {
          check_res = 1;
          fprintf (stderr,
                   "'MHD_str_pct_decode_in_place_lenient_ ()' FAILED: "
                   "Wrong output string:\n");
        }
      }
      if (0 != check_res)
      {
        ret++;
        fprintf (stderr,
                 "\tRESULT  : MHD_str_pct_decode_in_place_lenient_ (\"%s\" "
                 "-> \"%s\", ->%s) -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (buf, res_size),
                 is_broken ? "true" : "false",
                 (unsigned) res_size);
        fprintf (stderr,
                 "\tEXPECTED: MHD_str_pct_decode_in_place_lenient_ (\"%s\" "
                 "-> \"%s\", ->true) -> %u\n",
                 n_prnt (encoded, encoded_len),
                 n_prnt (decoded, decoded_size),
                 (unsigned) decoded_size);
      }
    }
  }

  if (0 != ret)
  {
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }
  return ret;
}


#define expect_decoded_bad(e,d) \
        expect_decoded_bad_n(e,MHD_STATICSTR_LEN_(e),\
                             d,MHD_STATICSTR_LEN_(d), \
                             __LINE__)

static unsigned int
check_decode_bad_str (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_decoded_bad ("50%/50%", "50%/50%");
  r += expect_decoded_bad ("This is 100% incorrect.",
                           "This is 100% incorrect.");
  r += expect_decoded_bad ("Some %%", "Some %%");
  r += expect_decoded_bad ("1 %", "1 %");
  r += expect_decoded_bad ("%", "%");
  r += expect_decoded_bad ("%a", "%a");
  r += expect_decoded_bad ("%0", "%0");
  r += expect_decoded_bad ("%0x", "%0x");
  r += expect_decoded_bad ("%FX", "%FX");
  r += expect_decoded_bad ("Valid%20and%2invalid", "Valid and%2invalid");

  return r;
}


int
main (int argc, char *argv[])
{
  unsigned int errcount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
  errcount += check_decode_str ();
  errcount += check_decode_bin ();
  errcount += check_decode_bad_str ();
  if (0 == errcount)
    printf ("All tests have been passed without errors.\n");
  return errcount == 0 ? 0 : 1;
}
