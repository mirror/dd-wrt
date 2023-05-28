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
 * @brief  Unit tests for MHD_str_remove_token_caseless_() function
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_options.h"
#include <string.h>
#include <stdio.h>
#include "mhd_str.h"
#include "mhd_assert.h"


static int
expect_result_n (const char *str, size_t str_len,
                 const char *token, size_t token_len,
                 const char *expected, size_t expected_len,
                 const bool expected_removed)
{
  char buf_in[1024];
  char buf_token[256];
  char buf_out[1024];
  size_t buf_len;

  mhd_assert (sizeof(buf_in) > str_len + 2);
  mhd_assert (sizeof(buf_token) > token_len + 2);
  mhd_assert (sizeof(buf_out) > expected_len + 2);

  memset (buf_in, '#', sizeof(buf_in));
  memset (buf_token, '#', sizeof(buf_token));
  memcpy (buf_in, str, str_len); /* Copy without zero-termination */
  memcpy (buf_token, token, token_len); /* Copy without zero-termination */

  for (buf_len = 0; buf_len <= expected_len + 3; ++buf_len)
  {
    bool res;
    ssize_t result_len;
    memset (buf_out, '$', sizeof(buf_out));

    result_len = buf_len;

    res = MHD_str_remove_token_caseless_ (buf_in, str_len, buf_token, token_len,
                                          buf_out, &result_len);
    if (buf_len < expected_len)
    { /* The result should not fit into the buffer */
      if (res || (0 < result_len))
      {
        fprintf (stderr,
                 "MHD_str_remove_token_caseless_() FAILED:\n"
                 "\tMHD_str_remove_token_caseless_(\"%.*s\", %lu,"
                 " \"%.*s\", %lu, buf, &(%ld->%ld)) returned %s\n",
                 (int) str_len + 2, buf_in, (unsigned long) str_len,
                 (int) token_len + 2, buf_token, (unsigned long) token_len,
                 (long) buf_len, (long) result_len, res ? "true" : "false");
        return 1;
      }
    }
    else
    { /* The result should fit into the buffer */
      if ( (expected_removed != res) ||
           (expected_len != (size_t) result_len) ||
           ((0 != result_len) && (0 != memcmp (expected, buf_out,
                                               result_len))) ||
           ('$' != buf_out[result_len]))
      {
        fprintf (stderr,
                 "MHD_str_remove_token_caseless_() FAILED:\n"
                 "\tMHD_str_remove_token_caseless_(\"%.*s\", %lu,"
                 " \"%.*s\", %lu, \"%.*s\", &(%ld->%ld)) returned %s\n",
                 (int) str_len + 2, buf_in, (unsigned long) str_len,
                 (int) token_len + 2, buf_token, (unsigned long) token_len,
                 (int) expected_len + 2, buf_out,
                 (long) buf_len, (long) result_len,
                 res ? "true" : "false");
        return 1;
      }
    }
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
  errcount += expect_result ("\t strinG", "String", "", true);
  errcount += expect_result ("strinG\t ", "String", "", true);
  errcount += expect_result (" \t tOkEn  ", "toKEN", "", true);
  errcount += expect_result ("not token\t,  tOkEn  ", "toKEN", "not token",
                             true);
  errcount += expect_result ("not token,\t  tOkEn, more token", "toKEN",
                             "not token, more token", true);
  errcount += expect_result ("not token,\t  tOkEn\t, more token", "toKEN",
                             "not token, more token", true);
  errcount += expect_result (",,,,,,test,,,,", "TESt", "", true);
  errcount += expect_result (",,,,,\t,test,,,,", "TESt", "", true);
  errcount += expect_result (",,,,,,test, ,,,", "TESt", "", true);
  errcount += expect_result (",,,,,, test,,,,", "TESt", "", true);
  errcount += expect_result (",,,,,, test not,test,,", "TESt", "test not",
                             true);
  errcount += expect_result (",,,,,, test not,,test,,", "TESt", "test not",
                             true);
  errcount += expect_result (",,,,,, test not ,test,,", "TESt", "test not",
                             true);
  errcount += expect_result (",,,,,, test", "TESt", "", true);
  errcount += expect_result (",,,,,, test      ", "TESt", "", true);
  errcount += expect_result ("no test,,,,,, test      ", "TESt", "no test",
                             true);
  errcount += expect_result ("the-token,, the-token , the-token" \
                             ",the-token ,the-token", "the-token", "", true);
  errcount += expect_result (" the-token,, the-token , the-token," \
                             "the-token ,the-token ", "the-token", "", true);
  errcount += expect_result (" the-token ,, the-token , the-token," \
                             "the-token , the-token ", "the-token", "", true);
  errcount += expect_result ("the-token,a, the-token , the-token,b," \
                             "the-token , c,the-token", "the-token", "a, b, c",
                             true);
  errcount += expect_result (" the-token, a, the-token , the-token, b," \
                             "the-token ,c ,the-token ", "the-token",
                             "a, b, c", true);
  errcount += expect_result (" the-token , a , the-token , the-token, b ," \
                             "the-token , c , the-token ", "the-token",
                             "a, b, c",true);
  errcount += expect_result ("the-token,aa, the-token , the-token,bb," \
                             "the-token , cc,the-token", "the-token",
                             "aa, bb, cc", true);
  errcount += expect_result (" the-token, aa, the-token , the-token, bb," \
                             "the-token ,cc ,the-token ", "the-token",
                             "aa, bb, cc", true);
  errcount += expect_result (" the-token , aa , the-token , the-token, bb ," \
                             "the-token , cc , the-token ", "the-token",
                             "aa, bb, cc", true);

  errcount += expect_result ("strin", "string", "strin", false);
  errcount += expect_result ("Stringer", "string", "Stringer", false);
  errcount += expect_result ("sstring", "String", "sstring", false);
  errcount += expect_result ("string", "Strin", "string", false);
  errcount += expect_result ("\t( strinG", "String", "( strinG", false);
  errcount += expect_result (")strinG\t ", "String", ")strinG", false);
  errcount += expect_result (" \t tOkEn t ", "toKEN", "tOkEn t", false);
  errcount += expect_result ("not token\t,  tOkEner  ", "toKEN",
                             "not token, tOkEner", false);
  errcount += expect_result ("not token,\t  tOkEns, more token", "toKEN",
                             "not token, tOkEns, more token", false);
  errcount += expect_result ("not token,\t  tOkEns\t, more token", "toKEN",
                             "not token, tOkEns, more token", false);
  errcount += expect_result (",,,,,,testing,,,,", "TESt", "testing", false);
  errcount += expect_result (",,,,,\t,test,,,,", "TESting", "test", false);
  errcount += expect_result ("tests,,,,,,quest, ,,,", "TESt", "tests, quest",
                             false);
  errcount += expect_result (",,,,,, testы,,,,", "TESt", "testы", false);
  errcount += expect_result (",,,,,, test not,хtest,,", "TESt",
                             "test not, хtest", false);
  errcount += expect_result ("testing,,,,,, test not,,test2,,", "TESt",
                             "testing, test not, test2", false);
  errcount += expect_result (",testi,,,,, test not ,test,,", "TESting",
                             "testi, test not, test", false);
  errcount += expect_result (",,,,,,2 test", "TESt", "2 test", false);
  errcount += expect_result (",,,,,,test test      ", "test", "test test",
                             false);
  errcount += expect_result ("no test,,,,,,test test", "test",
                             "no test, test test", false);
  errcount += expect_result (",,,,,,,,,,,,,,,,,,,", "the-token", "", false);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,", "the-token",
                             "a, b, c, d, e, f, g", false);
  errcount += expect_result (",,,,,,,,,,,,,,,,,,,", "", "", false);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,", "",
                             "a, b, c, d, e, f, g", false);
  errcount += expect_result ("a,b,c,d,e,f,g", "", "a, b, c, d, e, f, g",
                             false);
  errcount += expect_result ("a1,b1,c1,d1,e1,f1,g1", "",
                             "a1, b1, c1, d1, e1, f1, g1", false);

  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,the-token",
                             "the-token", "a, b, c, d, e, f, g", true);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,the-token,",
                             "the-token", "a, b, c, d, e, f, g", true);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,the-token,x",
                             "the-token", "a, b, c, d, e, f, g, x", true);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,the-token x",
                             "the-token", "a, b, c, d, e, f, g, the-token x",
                             false);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,the-token x,",
                             "the-token", "a, b, c, d, e, f, g, the-token x",
                             false);
  errcount += expect_result (",a,b,c,d,e,f,g,,,,,,,,,,,,the-token x,x",
                             "the-token", "a, b, c, d, e, f, g," \
                             " the-token x, x", false);
  errcount += expect_result ("the-token,a,b,c,d,e,f,g,,,,,,,,,,,,the-token",
                             "the-token", "a, b, c, d, e, f, g", true);
  errcount += expect_result ("the-token ,a,b,c,d,e,f,g,,,,,,,,,,,,the-token,",
                             "the-token", "a, b, c, d, e, f, g", true);
  errcount += expect_result ("the-token,a,b,c,d,e,f,g,,,,,,,,,,,,the-token,x",
                             "the-token", "a, b, c, d, e, f, g, x", true);
  errcount += expect_result ("the-token x,a,b,c,d,e,f,g,,,,,,,,,,,," \
                             "the-token x", "the-token",
                             "the-token x, a, b, c, d, e, f, g, the-token x",
                             false);
  errcount += expect_result ("the-token x,a,b,c,d,e,f,g,,,,,,,,,,,," \
                             "the-token x,", "the-token",
                             "the-token x, a, b, c, d, e, f, g, the-token x",
                             false);
  errcount += expect_result ("the-token x,a,b,c,d,e,f,g,,,,,,,,,,,," \
                             "the-token x,x", "the-token",
                             "the-token x, a, b, c, d, e, f, g, " \
                             "the-token x, x", false);

  return errcount;
}


int
main (int argc, char *argv[])
{
  int errcount = 0;
  (void) argc; (void) argv; /* Unused. Silent compiler warning. */
  errcount += check_result ();
  return errcount == 0 ? 0 : 1;
}
