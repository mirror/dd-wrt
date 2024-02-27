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
 * @file microhttpd/test_str_quote.c
 * @brief  Unit tests for quoted strings processing
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


#define TEST_STR_MAX_LEN 1024

/* return zero if succeed, non-zero otherwise */
static unsigned int
expect_result_unquote_n (const char *const quoted, const size_t quoted_len,
                         const char *const unquoted, const size_t unquoted_len,
                         const unsigned int line_num)
{
  static char buf[TEST_STR_MAX_LEN];
  size_t res_len;
  unsigned int ret1;
  unsigned int ret2;
  unsigned int ret3;
  unsigned int ret4;

  mhd_assert (NULL != quoted);
  mhd_assert (NULL != unquoted);
  mhd_assert (TEST_STR_MAX_LEN > quoted_len);
  mhd_assert (quoted_len >= unquoted_len);

  /* First check: MHD_str_unquote () */
  ret1 = 0;
  memset (buf, '#', sizeof(buf)); /* Fill buffer with character unused in the check */
  res_len = MHD_str_unquote (quoted, quoted_len, buf);

  if (res_len != unquoted_len)
  {
    ret1 = 1;
    fprintf (stderr,
             "'MHD_str_unquote ()' FAILED: Wrong result size:\n");
  }
  else if ((0 != unquoted_len) && (0 != memcmp (buf, unquoted, unquoted_len)))
  {
    ret1 = 1;
    fprintf (stderr,
             "'MHD_str_unquote ()' FAILED: Wrong result string:\n");
  }
  if (0 != ret1)
  {
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_unquote('%.*s', %u, ->'%.*s') -> %u\n"
             "\tEXPECTED: MHD_str_unquote('%.*s', %u, ->'%.*s') -> %u\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) res_len, buf, (unsigned) res_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }

  /* Second check: MHD_str_equal_quoted_bin_n () */
  ret2 = 0;
  if (! MHD_str_equal_quoted_bin_n (quoted, quoted_len, unquoted, unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret2 = 1;
  }

  /* Third check: MHD_str_equal_caseless_quoted_bin_n () */
  ret3 = 0;
  if (! MHD_str_equal_caseless_quoted_bin_n (quoted, quoted_len, unquoted,
                                             unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_caseless_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret3 = 1;
  }

  /* Fourth check: MHD_str_unquote () */
  ret4 = 0;
  memset (buf, '#', sizeof(buf)); /* Fill buffer with character unused in the check */
  res_len = MHD_str_quote (unquoted, unquoted_len, buf, quoted_len);
  if (res_len != quoted_len)
  {
    ret4 = 1;
    fprintf (stderr,
             "'MHD_str_quote ()' FAILED: Wrong result size:\n");
  }
  else if ((0 != quoted_len) && (0 != memcmp (buf, quoted, quoted_len)))
  {
    ret4 = 1;
    fprintf (stderr,
             "'MHD_str_quote ()' FAILED: Wrong result string:\n");
  }
  if (0 != ret4)
  {
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_quote('%.*s', %u, ->'%.*s', %u) -> %u\n"
             "\tEXPECTED: MHD_str_quote('%.*s', %u, ->'%.*s', %u) -> %u\n",
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) res_len, buf, (unsigned) quoted_len, (unsigned) res_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }

  return ret1 + ret2 + ret3 + ret4;
}


#define expect_result_unquote(q,u) \
    expect_result_unquote_n(q,MHD_STATICSTR_LEN_(q),\
                            u,MHD_STATICSTR_LEN_(u),__LINE__)


static unsigned int
check_match (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_result_unquote ("", "");
  r += expect_result_unquote ("a", "a");
  r += expect_result_unquote ("abc", "abc");
  r += expect_result_unquote ("abcdef", "abcdef");
  r += expect_result_unquote ("a\0" "bc", "a\0" "bc");
  r += expect_result_unquote ("abc\\\"", "abc\"");
  r += expect_result_unquote ("\\\"", "\"");
  r += expect_result_unquote ("\\\"abc", "\"abc");
  r += expect_result_unquote ("abc\\\\", "abc\\");
  r += expect_result_unquote ("\\\\", "\\");
  r += expect_result_unquote ("\\\\abc", "\\abc");
  r += expect_result_unquote ("123\\\\\\\\\\\\\\\\", "123\\\\\\\\");
  r += expect_result_unquote ("\\\\\\\\\\\\\\\\", "\\\\\\\\");
  r += expect_result_unquote ("\\\\\\\\\\\\\\\\123", "\\\\\\\\123");
  r += expect_result_unquote ("\\\\\\\"\\\\\\\"\\\\\\\"\\\\\\\"\\\\\\\"" \
                              "\\\\\\\"\\\\\\\"\\\\\\\"\\\\\\\"\\\\\\\"", \
                              "\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"");

  return r;
}


/* return zero if succeed, non-zero otherwise */
static unsigned int
expect_result_quote_failed_n (const char *const unquoted,
                              const size_t unquoted_len,
                              const size_t buf_size,
                              const unsigned int line_num)
{
  static char buf[TEST_STR_MAX_LEN];
  size_t res_len;
  unsigned int ret4;

  mhd_assert (TEST_STR_MAX_LEN > buf_size);

  /* The check: MHD_str_unquote () */
  ret4 = 0;
  memset (buf, '#', sizeof(buf)); /* Fill buffer with character unused in the check */
  res_len = MHD_str_quote (unquoted, unquoted_len, buf, buf_size);
  if (0 != res_len)
  {
    ret4 = 1;
    fprintf (stderr,
             "'MHD_str_quote ()' FAILED: Wrong result size:\n");
  }
  if (0 != ret4)
  {
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_quote('%.*s', %u, ->'%.*s', %u) -> %u\n"
             "\tEXPECTED: MHD_str_quote('%.*s', %u, (not checked), %u) -> 0\n",
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) res_len, buf,
             (unsigned) buf_size, (unsigned) res_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (unsigned) buf_size);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }

  return ret4;
}


#define expect_result_quote_failed(q,s) \
    expect_result_quote_failed_n(q,MHD_STATICSTR_LEN_(q),\
                            s,__LINE__)


static unsigned int
check_quote_failed (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_result_quote_failed ("a", 0);
  r += expect_result_quote_failed ("aa", 1);
  r += expect_result_quote_failed ("abc\\", 4);
  r += expect_result_quote_failed ("abc\"", 4);
  r += expect_result_quote_failed ("abc\"\"\"\"", 6);
  r += expect_result_quote_failed ("abc\"\"\"\"", 7);
  r += expect_result_quote_failed ("abc\"\"\"\"", 8);
  r += expect_result_quote_failed ("abc\"\"\"\"", 9);
  r += expect_result_quote_failed ("abc\"\"\"\"", 10);
  r += expect_result_quote_failed ("abc\\\\\\\\", 9);
  r += expect_result_quote_failed ("abc\\\\\\\\", 10);
  r += expect_result_quote_failed ("abc\"\"\"\"", 9);
  r += expect_result_quote_failed ("abc\"\"\"\"", 10);
  r += expect_result_quote_failed ("abc\"\\\"\\", 9);
  r += expect_result_quote_failed ("abc\\\"\\\"", 10);
  r += expect_result_quote_failed ("\"\"\"\"abc", 6);
  r += expect_result_quote_failed ("\"\"\"\"abc", 7);
  r += expect_result_quote_failed ("\"\"\"\"abc", 8);
  r += expect_result_quote_failed ("\"\"\"\"abc", 9);
  r += expect_result_quote_failed ("\"\"\"\"abc", 10);
  r += expect_result_quote_failed ("\\\\\\\\abc", 9);
  r += expect_result_quote_failed ("\\\\\\\\abc", 10);
  r += expect_result_quote_failed ("\"\"\"\"abc", 9);
  r += expect_result_quote_failed ("\"\"\"\"abc", 10);
  r += expect_result_quote_failed ("\"\\\"\\abc", 9);
  r += expect_result_quote_failed ("\\\"\\\"abc", 10);

  return r;
}


/* return zero if succeed, one otherwise */
static unsigned int
expect_match_caseless_n (const char *const quoted, const size_t quoted_len,
                         const char *const unquoted, const size_t unquoted_len,
                         const unsigned int line_num)
{
  unsigned int ret3;

  mhd_assert (NULL != quoted);
  mhd_assert (NULL != unquoted);
  mhd_assert (TEST_STR_MAX_LEN > quoted_len);

  /* The check: MHD_str_equal_caseless_quoted_bin_n () */
  ret3 = 0;
  if (! MHD_str_equal_caseless_quoted_bin_n (quoted, quoted_len, unquoted,
                                             unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_caseless_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret3 = 1;
  }

  return ret3;
}


#define expect_match_caseless(q,u) \
    expect_match_caseless_n(q,MHD_STATICSTR_LEN_(q),\
                            u,MHD_STATICSTR_LEN_(u),__LINE__)

static unsigned int
check_match_caseless (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_match_caseless ("a", "A");
  r += expect_match_caseless ("abC", "aBc");
  r += expect_match_caseless ("AbCdeF", "aBCdEF");
  r += expect_match_caseless ("a\0" "Bc", "a\0" "bC");
  r += expect_match_caseless ("Abc\\\"", "abC\"");
  r += expect_match_caseless ("\\\"", "\"");
  r += expect_match_caseless ("\\\"aBc", "\"abc");
  r += expect_match_caseless ("abc\\\\", "ABC\\");
  r += expect_match_caseless ("\\\\", "\\");
  r += expect_match_caseless ("\\\\ABC", "\\abc");
  r += expect_match_caseless ("\\\\ZYX", "\\ZYX");
  r += expect_match_caseless ("abc", "ABC");
  r += expect_match_caseless ("ABCabc", "abcABC");
  r += expect_match_caseless ("abcXYZ", "ABCxyz");
  r += expect_match_caseless ("AbCdEfABCabc", "ABcdEFabcABC");
  r += expect_match_caseless ("a\\\\bc", "A\\BC");
  r += expect_match_caseless ("ABCa\\\\bc", "abcA\\BC");
  r += expect_match_caseless ("abcXYZ\\\\", "ABCxyz\\");
  r += expect_match_caseless ("\\\\AbCdEfABCabc", "\\ABcdEFabcABC");

  return r;
}


/* return zero if succeed, one otherwise */
static unsigned int
expect_result_invalid_n (const char *const quoted, const size_t quoted_len,
                         const unsigned int line_num)
{
  static char buf[TEST_STR_MAX_LEN];
  size_t res_len;
  unsigned int ret1;

  mhd_assert (NULL != quoted);
  mhd_assert (TEST_STR_MAX_LEN > quoted_len);

  /* The check: MHD_str_unquote () */
  ret1 = 0;
  memset (buf, '#', sizeof(buf)); /* Fill buffer with character unused in the check */
  res_len = MHD_str_unquote (quoted, quoted_len, buf);

  if (res_len != 0)
  {
    ret1 = 1;
    fprintf (stderr,
             "'MHD_str_unquote ()' FAILED: Wrong result size:\n");
  }
  if (0 != ret1)
  {
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_unquote('%.*s', %u, (not checked)) -> %u\n"
             "\tEXPECTED: MHD_str_unquote('%.*s', %u, (not checked)) -> 0\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (unsigned) res_len,
             (int) quoted_len, quoted, (unsigned) quoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
  }

  return ret1;
}


#define expect_result_invalid(q) \
    expect_result_invalid_n(q,MHD_STATICSTR_LEN_(q),__LINE__)


static unsigned int
check_invalid (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_result_invalid ("\\");
  r += expect_result_invalid ("\\\\\\");
  r += expect_result_invalid ("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\");
  r += expect_result_invalid ("xyz\\");
  r += expect_result_invalid ("\\\"\\");
  r += expect_result_invalid ("\\\"\\\"\\\"\\");

  return r;
}


/* return zero if succeed, non-zero otherwise */
static unsigned int
expect_result_unmatch_n (const char *const quoted, const size_t quoted_len,
                         const char *const unquoted,
                         const size_t unquoted_len,
                         const unsigned int line_num)
{
  unsigned int ret2;
  unsigned int ret3;

  mhd_assert (NULL != quoted);
  mhd_assert (NULL != unquoted);

  /* The check: MHD_str_equal_quoted_bin_n () */
  ret2 = 0;
  if (MHD_str_equal_quoted_bin_n (quoted, quoted_len, unquoted, unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret2 = 1;
  }

  /* The check: MHD_str_equal_quoted_bin_n () */
  ret3 = 0;
  if (MHD_str_equal_caseless_quoted_bin_n (quoted, quoted_len, unquoted,
                                           unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_caseless_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret3 = 1;
  }

  return ret2 + ret3;
}


#define expect_result_unmatch(q,u) \
    expect_result_unmatch_n(q,MHD_STATICSTR_LEN_(q),\
                            u,MHD_STATICSTR_LEN_(u),__LINE__)


static unsigned int
check_unmatch (void)
{
  unsigned int r = 0; /**< The number of errors */

  /* Matched sequence except invalid backslash at the end */
  r += expect_result_unmatch ("\\", "");
  r += expect_result_unmatch ("a\\", "a");
  r += expect_result_unmatch ("abc\\", "abc");
  r += expect_result_unmatch ("a\0" "bc\\", "a\0" "bc");
  r += expect_result_unmatch ("abc\\\"\\", "abc\"");
  r += expect_result_unmatch ("\\\"\\", "\"");
  r += expect_result_unmatch ("\\\"abc\\", "\"abc");
  r += expect_result_unmatch ("abc\\\\\\", "abc\\");
  r += expect_result_unmatch ("\\\\\\", "\\");
  r += expect_result_unmatch ("\\\\abc\\", "\\abc");
  r += expect_result_unmatch ("123\\\\\\\\\\\\\\\\\\", "123\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\\\", "\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\123\\", "\\\\\\\\123");
  /* Invalid backslash at the end and empty string */
  r += expect_result_unmatch ("\\", "");
  r += expect_result_unmatch ("a\\", "");
  r += expect_result_unmatch ("abc\\", "");
  r += expect_result_unmatch ("a\0" "bc\\", "");
  r += expect_result_unmatch ("abc\\\"\\", "");
  r += expect_result_unmatch ("\\\"\\", "");
  r += expect_result_unmatch ("\\\"abc\\", "");
  r += expect_result_unmatch ("abc\\\\\\", "");
  r += expect_result_unmatch ("\\\\\\", "");
  r += expect_result_unmatch ("\\\\abc\\", "");
  r += expect_result_unmatch ("123\\\\\\\\\\\\\\\\\\", "");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\\\", "");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\123\\", "");
  /* Difference at binary zero */
  r += expect_result_unmatch ("\0", "");
  r += expect_result_unmatch ("", "\0");
  r += expect_result_unmatch ("a\0", "a");
  r += expect_result_unmatch ("a", "a\0");
  r += expect_result_unmatch ("abc\0", "abc");
  r += expect_result_unmatch ("abc", "abc\0");
  r += expect_result_unmatch ("a\0" "bc\0", "a\0" "bc");
  r += expect_result_unmatch ("a\0" "bc", "a\0" "bc\0");
  r += expect_result_unmatch ("abc\\\"\0", "abc\"");
  r += expect_result_unmatch ("abc\\\"", "abc\"\0");
  r += expect_result_unmatch ("\\\"\0", "\"");
  r += expect_result_unmatch ("\\\"", "\"\0");
  r += expect_result_unmatch ("\\\"abc\0", "\"abc");
  r += expect_result_unmatch ("\\\"abc", "\"abc\0");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\\0", "\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\", "\\\\\\\\\0");
  r += expect_result_unmatch ("\\\\\\\\\\\\\0" "\\\\", "\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\", "\\\\\\\0" "\\");
  r += expect_result_unmatch ("\0" "abc", "abc");
  r += expect_result_unmatch ("abc", "\0" "abc");
  r += expect_result_unmatch ("\0" "abc", "0abc");
  r += expect_result_unmatch ("0abc", "\0" "abc");
  r += expect_result_unmatch ("xyz", "xy" "\0" "z");
  r += expect_result_unmatch ("xy" "\0" "z", "xyz");
  /* Difference after binary zero */
  r += expect_result_unmatch ("abc\0" "1", "abc\0" "2");
  r += expect_result_unmatch ("a\0" "bcx", "a\0" "bcy");
  r += expect_result_unmatch ("\0" "abc\\\"2", "\0" "abc\"1");
  r += expect_result_unmatch ("\0" "abc1\\\"", "\0" "abc2\"");
  r += expect_result_unmatch ("\0" "\\\"c", "\0" "\"d");
  r += expect_result_unmatch ("\\\"ab" "\0" "1c", "\"ab" "\0" "2c");
  r += expect_result_unmatch ("a\0" "bcdef2", "a\0" "bcdef1");
  r += expect_result_unmatch ("a\0" "bc2def", "a\0" "bc1def");
  r += expect_result_unmatch ("a\0" "1bcdef", "a\0" "2bcdef");
  r += expect_result_unmatch ("abcde\0" "f2", "abcde\0" "f1");
  r += expect_result_unmatch ("123\\\\\\\\\\\\\0" "\\\\1", "123\\\\\\\0" "\\2");
  r += expect_result_unmatch ("\\\\\\\\\\\\\0" "1\\\\", "\\\\\\" "2\\");
  /* One side is empty */
  r += expect_result_unmatch ("abc", "");
  r += expect_result_unmatch ("", "abc");
  r += expect_result_unmatch ("1234567890", "");
  r += expect_result_unmatch ("", "1234567890");
  r += expect_result_unmatch ("abc\\\"", "");
  r += expect_result_unmatch ("", "abc\"");
  r += expect_result_unmatch ("\\\"", "");
  r += expect_result_unmatch ("", "\"");
  r += expect_result_unmatch ("\\\"abc", "");
  r += expect_result_unmatch ("", "\"abc");
  r += expect_result_unmatch ("abc\\\\", "");
  r += expect_result_unmatch ("", "abc\\");
  r += expect_result_unmatch ("\\\\", "");
  r += expect_result_unmatch ("", "\\");
  r += expect_result_unmatch ("\\\\abc", "");
  r += expect_result_unmatch ("", "\\abc");
  r += expect_result_unmatch ("123\\\\\\\\\\\\\\\\", "");
  r += expect_result_unmatch ("", "123\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\", "");
  r += expect_result_unmatch ("", "\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\123", "");
  r += expect_result_unmatch ("", "\\\\\\\\123");
  /* Various unmatched strings */
  r += expect_result_unmatch ("a", "x");
  r += expect_result_unmatch ("abc", "abcabc");
  r += expect_result_unmatch ("abc", "abcabcabc");
  r += expect_result_unmatch ("abc", "abcabcabcabc");
  r += expect_result_unmatch ("ABCABC", "ABC");
  r += expect_result_unmatch ("ABCABCABC", "ABC");
  r += expect_result_unmatch ("ABCABCABCABC", "ABC");
  r += expect_result_unmatch ("123\\\\\\\\\\\\\\\\\\\\", "123\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\\\\\", "\\\\\\\\");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\123\\\\", "\\\\\\\\123");
  r += expect_result_unmatch ("\\\\\\\\\\\\\\\\", "\\\\\\\\\\");

  return r;
}


/* return zero if succeed, one otherwise */
static unsigned int
expect_result_case_unmatch_n (const char *const quoted,
                              const size_t quoted_len,
                              const char *const unquoted,
                              const size_t unquoted_len,
                              const unsigned int line_num)
{
  unsigned int ret2;

  mhd_assert (NULL != quoted);
  mhd_assert (NULL != unquoted);

  /* THe check: MHD_str_equal_quoted_bin_n () */
  ret2 = 0;
  if (MHD_str_equal_quoted_bin_n (quoted, quoted_len, unquoted, unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret2 = 1;
  }

  return ret2;
}


#define expect_result_case_unmatch(q,u) \
    expect_result_case_unmatch_n(q,MHD_STATICSTR_LEN_(q),\
                                 u,MHD_STATICSTR_LEN_(u),__LINE__)

static unsigned int
check_unmatch_case (void)
{
  unsigned int r = 0; /**< The number of errors */

  r += expect_result_case_unmatch ("a", "A");
  r += expect_result_case_unmatch ("abC", "aBc");
  r += expect_result_case_unmatch ("AbCdeF", "aBCdEF");
  r += expect_result_case_unmatch ("a\0" "Bc", "a\0" "bC");
  r += expect_result_case_unmatch ("Abc\\\"", "abC\"");
  r += expect_result_case_unmatch ("\\\"aBc", "\"abc");
  r += expect_result_case_unmatch ("abc\\\\", "ABC\\");
  r += expect_result_case_unmatch ("\\\\ABC", "\\abc");
  r += expect_result_case_unmatch ("\\\\ZYX", "\\ZYx");
  r += expect_result_case_unmatch ("abc", "ABC");
  r += expect_result_case_unmatch ("ABCabc", "abcABC");
  r += expect_result_case_unmatch ("abcXYZ", "ABCxyz");
  r += expect_result_case_unmatch ("AbCdEfABCabc", "ABcdEFabcABC");
  r += expect_result_case_unmatch ("a\\\\bc", "A\\BC");
  r += expect_result_case_unmatch ("ABCa\\\\bc", "abcA\\BC");
  r += expect_result_case_unmatch ("abcXYZ\\\\", "ABCxyz\\");
  r += expect_result_case_unmatch ("\\\\AbCdEfABCabc", "\\ABcdEFabcABC");

  return r;
}


/* return zero if succeed, one otherwise */
static unsigned int
expect_result_caseless_unmatch_n (const char *const quoted,
                                  const size_t quoted_len,
                                  const char *const unquoted,
                                  const size_t unquoted_len,
                                  const unsigned int line_num)
{
  unsigned int ret2;
  unsigned int ret3;

  mhd_assert (NULL != quoted);
  mhd_assert (NULL != unquoted);

  /* The check: MHD_str_equal_quoted_bin_n () */
  ret2 = 0;
  if (MHD_str_equal_quoted_bin_n (quoted, quoted_len, unquoted, unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret2 = 1;
  }

  /* The check: MHD_str_equal_quoted_bin_n () */
  ret3 = 0;
  if (MHD_str_equal_caseless_quoted_bin_n (quoted, quoted_len, unquoted,
                                           unquoted_len))
  {
    fprintf (stderr,
             "'MHD_str_equal_caseless_quoted_bin_n ()' FAILED: Wrong result:\n");
    /* This does NOT print part of the string after binary zero */
    fprintf (stderr,
             "\tRESULT  : MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> true\n"
             "\tEXPECTED: MHD_str_equal_caseless_quoted_bin_n('%.*s', %u, "
             "'%.*s', %u) -> false\n",
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len,
             (int) quoted_len, quoted, (unsigned) quoted_len,
             (int) unquoted_len, unquoted, (unsigned) unquoted_len);
    fprintf (stderr,
             "The check is at line: %u\n\n", line_num);
    ret3 = 1;
  }

  return ret2 + ret3;
}


#define expect_result_caseless_unmatch(q,u) \
    expect_result_caseless_unmatch_n(q,MHD_STATICSTR_LEN_(q),\
                                     u,MHD_STATICSTR_LEN_(u),__LINE__)


static unsigned int
check_unmatch_caseless (void)
{
  unsigned int r = 0; /**< The number of errors */

  /* Matched sequence except invalid backslash at the end */
  r += expect_result_caseless_unmatch ("a\\", "A");
  r += expect_result_caseless_unmatch ("abC\\", "abc");
  r += expect_result_caseless_unmatch ("a\0" "Bc\\", "a\0" "bc");
  r += expect_result_caseless_unmatch ("abc\\\"\\", "ABC\"");
  r += expect_result_caseless_unmatch ("\\\"\\", "\"");
  r += expect_result_caseless_unmatch ("\\\"ABC\\", "\"abc");
  r += expect_result_caseless_unmatch ("Abc\\\\\\", "abC\\");
  r += expect_result_caseless_unmatch ("\\\\\\", "\\");
  r += expect_result_caseless_unmatch ("\\\\aBc\\", "\\abC");
  /* Difference at binary zero */
  r += expect_result_caseless_unmatch ("a\0", "A");
  r += expect_result_caseless_unmatch ("A", "a\0");
  r += expect_result_caseless_unmatch ("abC\0", "abc");
  r += expect_result_caseless_unmatch ("abc", "ABc\0");
  r += expect_result_caseless_unmatch ("a\0" "bC\0", "a\0" "bc");
  r += expect_result_caseless_unmatch ("a\0" "bc", "A\0" "bc\0");
  r += expect_result_caseless_unmatch ("ABC\\\"\0", "abc\"");
  r += expect_result_caseless_unmatch ("abc\\\"", "ABC\"\0");
  r += expect_result_caseless_unmatch ("\\\"aBc\0", "\"abc");
  r += expect_result_caseless_unmatch ("\\\"Abc", "\"abc\0");
  r += expect_result_caseless_unmatch ("\\\\\\\\\\\\\\\\\0", "\\\\\\\\");
  r += expect_result_caseless_unmatch ("\\\\\\\\\\\\\\\\", "\\\\\\\\\0");
  r += expect_result_caseless_unmatch ("\\\\\\\\\\\\\0" "\\\\", "\\\\\\\\");
  r += expect_result_caseless_unmatch ("\\\\\\\\\\\\\\\\", "\\\\\\\0" "\\");
  r += expect_result_caseless_unmatch ("\0" "aBc", "abc");
  r += expect_result_caseless_unmatch ("abc", "\0" "abC");
  r += expect_result_caseless_unmatch ("\0" "abc", "0abc");
  r += expect_result_caseless_unmatch ("0abc", "\0" "aBc");
  r += expect_result_caseless_unmatch ("xyZ", "xy" "\0" "z");
  r += expect_result_caseless_unmatch ("Xy" "\0" "z", "xyz");
  /* Difference after binary zero */
  r += expect_result_caseless_unmatch ("abc\0" "1", "aBC\0" "2");
  r += expect_result_caseless_unmatch ("a\0" "bcX", "a\0" "bcy");
  r += expect_result_caseless_unmatch ("\0" "abc\\\"2", "\0" "Abc\"1");
  r += expect_result_caseless_unmatch ("\0" "Abc1\\\"", "\0" "abc2\"");
  r += expect_result_caseless_unmatch ("\0" "\\\"c", "\0" "\"d");
  r += expect_result_caseless_unmatch ("\\\"ab" "\0" "1C", "\"ab" "\0" "2C");
  r += expect_result_caseless_unmatch ("a\0" "BCDef2", "a\0" "bcdef1");
  r += expect_result_caseless_unmatch ("a\0" "bc2def", "a\0" "BC1def");
  r += expect_result_caseless_unmatch ("a\0" "1bcdeF", "a\0" "2bcdef");
  r += expect_result_caseless_unmatch ("abcde\0" "f2", "ABCDE\0" "f1");
  r += expect_result_caseless_unmatch ("\\\"ab" "\0" "XC", "\"ab" "\0" "yC");
  r += expect_result_caseless_unmatch ("a\0" "BCDefY", "a\0" "bcdefx");
  r += expect_result_caseless_unmatch ("a\0" "bczdef", "a\0" "BCXdef");
  r += expect_result_caseless_unmatch ("a\0" "YbcdeF", "a\0" "zbcdef");
  r += expect_result_caseless_unmatch ("abcde\0" "fy", "ABCDE\0" "fX");

  return r;
}


int
main (int argc, char *argv[])
{
  unsigned int errcount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
  errcount += check_match ();
  errcount += check_quote_failed ();
  errcount += check_match_caseless ();
  errcount += check_invalid ();
  errcount += check_unmatch ();
  errcount += check_unmatch_case ();
  errcount += check_unmatch_caseless ();
  if (0 == errcount)
    printf ("All tests were passed without errors.\n");
  return errcount == 0 ? 0 : 1;
}
