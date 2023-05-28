/*
  This file is part of libmicrohttpd
  Copyright (C) 2017-2021 Karlson2k (Evgeny Grin)

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
 * @file microhttpd/test_str_token.c
 * @brief  Unit tests for MHD_str_remove_tokens_caseless_() function
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <string.h>
#include <stdio.h>
#include "mhd_str.h"
#include "mhd_assert.h"


static int
expect_result_n (const char *str, size_t str_len,
                 const char *tokens, size_t tokens_len,
                 const char *expected, size_t expected_len,
                 const bool expected_removed)
{
  char buf_in[1024];
  char buf_tokens[256];
  bool res;
  size_t result_len;

  mhd_assert (sizeof(buf_in) > str_len + 2);
  mhd_assert (sizeof(buf_tokens) > tokens_len + 2);

  memset (buf_tokens, '#', sizeof(buf_tokens));
  memcpy (buf_tokens, tokens, tokens_len); /* Copy without zero-termination */
  memset (buf_in, '$', sizeof(buf_in));
  memcpy (buf_in, str, str_len); /* Copy without zero-termination */

  result_len = str_len;

  res = MHD_str_remove_tokens_caseless_ (buf_in, &result_len,
                                         buf_tokens, tokens_len);

  if ( (expected_removed != res) ||
       (expected_len != result_len) ||
       ((0 != result_len) && (0 != memcmp (expected, buf_in, result_len))) ||
       ('$' != buf_in[str_len]))
  {
    fprintf (stderr,
             "MHD_str_remove_tokens_caseless_() FAILED:\n"
             "\tRESULT: "
             "\tMHD_str_remove_token_caseless_(\"%s\"->\"%.*s\", &(%lu->%lu),"
             " \"%.*s\", %lu) returned %s\n",
             str,
             (int) result_len, buf_in,
             (unsigned long) str_len, (unsigned long) result_len,
             (int) tokens_len, buf_tokens, (unsigned long) tokens_len,
             res ? "true" : "false");
    fprintf (stderr,
             "\tEXPECTED: "
             "\tMHD_str_remove_token_caseless_(\"%s\"->\"%s\", &(%lu->%lu),"
             " \"%.*s\", %lu) returned %s\n",
             str,
             expected,
             (unsigned long) str_len, (unsigned long) expected_len,
             (int) tokens_len, buf_tokens, (unsigned long) tokens_len,
             expected_removed ? "true" : "false");
    return 1;
  }
  return 0;
}


#define expect_result(s,t,e,found) \
  expect_result_n ((s),MHD_STATICSTR_LEN_ (s), \
                   (t),MHD_STATICSTR_LEN_ (t), \
                   (e),MHD_STATICSTR_LEN_ (e), found)

int
check_result (void)
{
  int errcount = 0;
  errcount += expect_result ("string", "string", "", true);
  errcount += expect_result ("String", "string", "", true);
  errcount += expect_result ("string", "String", "", true);
  errcount += expect_result ("strinG", "String", "", true);
  errcount += expect_result ("strinG", "String\t", "", true);
  errcount += expect_result ("strinG", "\tString", "", true);
  errcount += expect_result ("tOkEn", " \t toKEN  ", "", true);
  errcount += expect_result ("not-token, tOkEn", "token", "not-token",
                             true);
  errcount += expect_result ("not-token1, tOkEn1, token", "token1",
                             "not-token1, token",
                             true);
  errcount += expect_result ("token, tOkEn1", "token1", "token",
                             true);
  errcount += expect_result ("not-token, tOkEn", " \t toKEN", "not-token",
                             true);
  errcount += expect_result ("not-token, tOkEn, more-token", "toKEN\t",
                             "not-token, more-token", true);
  errcount += expect_result ("not-token, tOkEn, more-token", "\t  toKEN,,,,,",
                             "not-token, more-token", true);
  errcount += expect_result ("a, b, c, d", ",,,,,a", "b, c, d", true);
  errcount += expect_result ("a, b, c, d", "a,,,,,,", "b, c, d", true);
  errcount += expect_result ("a, b, c, d", ",,,,a,,,,,,", "b, c, d", true);
  errcount += expect_result ("a, b, c, d", "\t \t,,,,a,,   ,   ,,,\t",
                             "b, c, d", true);
  errcount += expect_result ("a, b, c, d", "b, c, d", "a", true);
  errcount += expect_result ("a, b, c, d", "a, b, c, d", "", true);
  errcount += expect_result ("a, b, c, d", "d, c, b, a", "", true);
  errcount += expect_result ("a, b, c, d", "b, d, a, c", "", true);
  errcount += expect_result ("a, b, c, d, e", "b, d, a, c", "e", true);
  errcount += expect_result ("e, a, b, c, d", "b, d, a, c", "e", true);
  errcount += expect_result ("e, a, b, c, d, e", "b, d, a, c", "e, e", true);
  errcount += expect_result ("a, b, c, d", "b,c,d", "a", true);
  errcount += expect_result ("a, b, c, d", "a,b,c,d", "", true);
  errcount += expect_result ("a, b, c, d", "d,c,b,a", "", true);
  errcount += expect_result ("a, b, c, d", "b,d,a,c", "", true);
  errcount += expect_result ("a, b, c, d, e", "b,d,a,c", "e", true);
  errcount += expect_result ("e, a, b, c, d", "b,d,a,c", "e", true);
  errcount += expect_result ("e, a, b, c, d, e", "b,d,a,c", "e, e", true);
  errcount += expect_result ("a, b, c, d", "d,,,,,,,,,c,b,a", "", true);
  errcount += expect_result ("a, b, c, d", "b,d,a,c,,,,,,,,,,", "", true);
  errcount += expect_result ("a, b, c, d, e", ",,,,\t,,,,b,d,a,c,\t", "e",
                             true);
  errcount += expect_result ("e, a, b, c, d", "b,d,a,c", "e", true);
  errcount += expect_result ("token, a, b, c, d", "token", "a, b, c, d", true);
  errcount += expect_result ("token1, a, b, c, d", "token1", "a, b, c, d",
                             true);
  errcount += expect_result ("token12, a, b, c, d", "token12", "a, b, c, d",
                             true);
  errcount += expect_result ("token123, a, b, c, d", "token123", "a, b, c, d",
                             true);
  errcount += expect_result ("token1234, a, b, c, d", "token1234", "a, b, c, d",
                             true);
  errcount += expect_result ("token12345, a, b, c, d", "token12345",
                             "a, b, c, d", true);
  errcount += expect_result ("token123456, a, b, c, d", "token123456",
                             "a, b, c, d", true);
  errcount += expect_result ("token1234567, a, b, c, d", "token1234567",
                             "a, b, c, d", true);
  errcount += expect_result ("token12345678, a, b, c, d", "token12345678",
                             "a, b, c, d", true);

  errcount += expect_result ("", "a", "", false);
  errcount += expect_result ("", "", "", false);
  errcount += expect_result ("a, b, c, d", "bb, dd, aa, cc", "a, b, c, d",
                             false);
  errcount += expect_result ("a, b, c, d, e", "bb, dd, aa, cc", "a, b, c, d, e",
                             false);
  errcount += expect_result ("e, a, b, c, d", "bb, dd, aa, cc", "e, a, b, c, d",
                             false);
  errcount += expect_result ("e, a, b, c, d, e", "bb, dd, aa, cc",
                             "e, a, b, c, d, e", false);
  errcount += expect_result ("aa, bb, cc, dd", "b, d, a, c", "aa, bb, cc, dd",
                             false);
  errcount += expect_result ("aa, bb, cc, dd, ee", "b, d, a, c",
                             "aa, bb, cc, dd, ee", false);
  errcount += expect_result ("ee, aa, bb, cc, dd", "b, d, a, c",
                             "ee, aa, bb, cc, dd", false);
  errcount += expect_result ("ee, aa, bb, cc, dd, ee", "b, d, a, c",
                             "ee, aa, bb, cc, dd, ee", false);

  errcount += expect_result ("TESt", ",,,,,,test,,,,", "", true);
  errcount += expect_result ("TESt", ",,,,,\t,test,,,,", "", true);
  errcount += expect_result ("TESt", ",,,,,,test, ,,,", "", true);
  errcount += expect_result ("TESt", ",,,,,, test,,,,", "", true);
  errcount += expect_result ("TESt", ",,,,,, test-not,test,,", "",
                             true);
  errcount += expect_result ("TESt", ",,,,,, test-not,,test,,", "",
                             true);
  errcount += expect_result ("TESt", ",,,,,, test-not ,test,,", "",
                             true);
  errcount += expect_result ("TESt", ",,,,,, test", "", true);
  errcount += expect_result ("TESt", ",,,,,, test      ", "", true);
  errcount += expect_result ("TESt", "no-test,,,,,, test      ", "",
                             true);

  errcount += expect_result ("the-token, a, the-token, b, the-token, " \
                             "the-token, c, the-token", "the-token", "a, b, c",
                             true);
  errcount += expect_result ("aa, the-token, bb, the-token, cc, the-token, " \
                             "the-token, dd, the-token", "the-token",
                             "aa, bb, cc, dd", true);
  errcount += expect_result ("the-token, a, the-token, b, the-token, " \
                             "the-token, c, the-token, e", "the-token",
                             "a, b, c, e", true);
  errcount += expect_result ("aa, the-token, bb, the-token, cc, the-token, " \
                             "the-token, dd, the-token, ee", "the-token",
                             "aa, bb, cc, dd, ee", true);
  errcount += expect_result ("the-token, the-token, the-token, " \
                             "the-token, the-token", "the-token", "", true);
  errcount += expect_result ("the-token, a, the-token, the-token, b, " \
                             "the-token, c, the-token, a", "c,a,b",
                             "the-token, the-token, the-token, the-token, the-token",
                             true);
  errcount += expect_result ("the-token, xx, the-token, the-token, zz, " \
                             "the-token, yy, the-token, ww", "ww,zz,yy",
                             "the-token, xx, the-token, the-token, the-token, the-token",
                             true);
  errcount += expect_result ("the-token, a, the-token, the-token, b, " \
                             "the-token, c, the-token, a", " c,\t a,b,,,",
                             "the-token, the-token, the-token, the-token, the-token",
                             true);
  errcount += expect_result ("the-token, xx, the-token, the-token, zz, " \
                             "the-token, yy, the-token, ww",
                             ",,,,ww,\t zz,  yy",
                             "the-token, xx, the-token, the-token, the-token, the-token",
                             true);
  errcount += expect_result ("the-token, a, the-token, the-token, b, " \
                             "the-token, c, the-token, a", ",,,,c,\t a,b",
                             "the-token, the-token, the-token, the-token, the-token",
                             true);
  errcount += expect_result ("the-token, xx, the-token, the-token, zz, " \
                             "the-token, yy, the-token, ww", " ww,\t zz,yy,,,,",
                             "the-token, xx, the-token, the-token, the-token, the-token",
                             true);
  errcount += expect_result ("close, 2", "close",
                             "2", true);
  errcount += expect_result ("close, 22", "close",
                             "22", true);
  errcount += expect_result ("close, nothing", "close",
                             "nothing", true);
  errcount += expect_result ("close, 2", "2",
                             "close", true);
  errcount += expect_result ("close", "close",
                             "", true);
  errcount += expect_result ("close, nothing", "close, token",
                             "nothing", true);
  errcount += expect_result ("close, nothing", "nothing, token",
                             "close", true);
  errcount += expect_result ("close, 2", "close, 10, 12, 22, nothing",
                             "2", true);

  errcount += expect_result ("strin", "string", "strin", false);
  errcount += expect_result ("Stringer", "string", "Stringer", false);
  errcount += expect_result ("sstring", "String", "sstring", false);
  errcount += expect_result ("string", "Strin", "string", false);
  errcount += expect_result ("String", "\t(-strinG", "String", false);
  errcount += expect_result ("String", ")strinG\t ", "String", false);
  errcount += expect_result ("not-token, tOkEner", "toKEN",
                             "not-token, tOkEner", false);
  errcount += expect_result ("not-token, tOkEns, more-token", "toKEN",
                             "not-token, tOkEns, more-token", false);
  errcount += expect_result ("tests, quest", "TESt", "tests, quest",
                             false);
  errcount += expect_result ("testы", "TESt", "testы", false);
  errcount += expect_result ("test-not, хtest", "TESt",
                             "test-not, хtest", false);
  errcount += expect_result ("testing, test not, test2", "TESt",
                             "testing, test not, test2", false);
  errcount += expect_result ("", ",,,,,,,,,,,,,,,,,,,the-token", "", false);
  errcount += expect_result ("a1, b1, c1, d1, e1, f1, g1", "",
                             "a1, b1, c1, d1, e1, f1, g1", false);

  return errcount;
}


int
main (int argc, char *argv[])
{
  int errcount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
  errcount += check_result ();
  if (0 == errcount)
    printf ("All tests were passed without errors.\n");
  return errcount == 0 ? 0 : 1;
}
