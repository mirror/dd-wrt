/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/*
 * String API tests
 * $Id: str.c,v 1.1 2008/10/06 18:16:50 castaglia Exp $
 */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = make_sub_pool(NULL);
  }
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
  }
}

START_TEST (sstrncpy_test) {
  char *res, *ok, *dst;
  size_t len, sz = 32;

  res = sstrncpy(NULL, NULL, 0);
  fail_unless(res == NULL, "Failed to handle null arguments");

  dst = "";
  res = sstrncpy(dst, "foo", 0);
  fail_unless(res == NULL, "Failed to handle zero length");

  dst = pcalloc(p, sz);
  memset(dst, 'A', sz);

  res = sstrncpy(dst, NULL, 1);
  fail_unless(res == dst, "Expected %p, got %p", dst, res);
  fail_unless(*res == '\0', "Expected NUL, got '%c'", *res);

  ok = "Therefore, all progress depends on the unreasonable man";

  memset(dst, 'A', sz);
  len = 1;

  res = sstrncpy(dst, ok, len);
  fail_unless(res == dst, "Expected %p, got %p", dst, res);
  fail_unless(strlen(res) == (len - 1), "Expected len %u, got len %u", len - 1,
    strlen(res));
  fail_unless(res[len-1] == '\0', "Expected NUL, got '%c'", res[len-1]);

  memset(dst, 'A', sz);
  len = 7;

  res = sstrncpy(dst, ok, len);
  fail_unless(res == dst, "Expected %p, got %p", dst, res);
  fail_unless(strlen(res) == (len - 1), "Expected len %u, got len %u", len - 1,
    strlen(res));
  fail_unless(res[len-1] == '\0', "Expected NUL, got '%c'", res[len-1]);

  memset(dst, 'A', sz);
  len = sz;

  res = sstrncpy(dst, ok, len);
  fail_unless(res == dst, "Expected %p, got %p", dst, res);
  fail_unless(strlen(res) == (len - 1), "Expected len %u, got len %u", len - 1,
    strlen(res));
  fail_unless(res[len-1] == '\0', "Expected NUL, got '%c'", res[len-1]);

  memset(dst, 'A', sz);
  len = sz;

  res = sstrncpy(dst, "", len);
  fail_unless(res == dst, "Expected %p, got %p", dst, res);
  fail_unless(strlen(res) == 0, "Expected len %u, got len %u", 0, strlen(res));
  fail_unless(*res == '\0', "Expected NUL, got '%c'", *res);
}
END_TEST

START_TEST (sstrcat_test) {
  register unsigned int i;
  char c = 'A', src[1024], dst[1024], *res;

  res = sstrcat(dst, src, 0);
  fail_unless(res == NULL, "Non-null result for zero-length strcat");

  src[0] = 'f';
  src[1] = '\0';
  dst[0] = 'e';
  dst[1] = '\0';
  res = sstrcat(dst, src, 1);
  fail_unless(res == dst, "Returned wrong destination buffer");

  /* In this case, we told sstrcat() that dst is len 1, which means that
   * sstrcat() should set dst[0] to NUL.
   */
  fail_unless(dst[0] == 0, "Failed to terminate destination buffer");

  src[0] = 'f';
  src[1] = '\0';
  dst[0] = 'e';
  dst[1] = '\0';
  res = sstrcat(dst, src, 2);
  fail_unless(res == dst, "Returned wrong destination buffer");

  /* In this case, we told sstrcat() that dst is len 2, which means that
   * sstrcat() should preserve the value at 0, and set dst[1] to NUL.
   */
  fail_unless(dst[0] == 'e',
    "Failed to preserve destination buffer (expected '%c' at index 0, "
    "got '%c')", 'e', dst[0]);

  fail_unless(dst[1] == 0, "Failed to terminate destination buffer");

  src[0] = 'f';
  src[1] = '\0';
  dst[0] = 'e';
  dst[1] = '\0';
  res = sstrcat(dst, src, 3);
  fail_unless(res == dst, "Returned wrong destination buffer");

  fail_unless(dst[0] == 'e',
    "Failed to preserve destination buffer (expected '%c' at index 0, "
    "got '%c')", 'e', dst[0]);

  fail_unless(dst[1] == 'f',
    "Failed to copy source buffer (expected '%c' at index 1, got '%c')",
    'f', dst[1]);

  fail_unless(dst[2] == 0, "Failed to terminate destination buffer");

  memset(src, c, sizeof(src));

  dst[0] = '\0';
  res = sstrcat(dst, src, sizeof(dst));
  fail_unless(res == dst, "Returned wrong destination buffer");
  fail_unless(dst[sizeof(dst)-1] == 0,
    "Failed to terminate destination buffer");

  fail_unless(strlen(dst) == (sizeof(dst)-1),
    "Failed to copy all the data (expected len %u, got len %u)",
    sizeof(dst)-1, strlen(dst));

  for (i = 0; i < sizeof(dst)-1; i++) {
    fail_unless(dst[i] == c, "Copied wrong value (expected '%c', got '%c')",
      c, dst[i]);
  }
}
END_TEST

START_TEST (sreplace_test) {
  char *fmt = NULL, *res, *ok;

  res = sreplace(NULL, NULL, 0);
  fail_unless(res == NULL, "Failed to handle invalid arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = sreplace(NULL, "", 0);
  fail_unless(res == NULL, "Failed to handle invalid arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = sreplace(p, NULL, 0);
  fail_unless(res == NULL, "Failed to handle invalid arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  fmt = "%a";
  res = sreplace(p, fmt, "foo", NULL);
  fail_unless(strcmp(res, fmt) == 0, "Expected '%s', got '%s'", fmt, res);

  fmt = "foo %a";
  res = sreplace(p, fmt, "%b", NULL);
  fail_unless(strcmp(res, fmt) == 0, "Expected '%s', got '%s'", fmt, res);

  fmt = "foo %a";
  res = sreplace(p, fmt, "%a", "bar", NULL);
  fail_unless(strcmp(res, "foo bar") == 0, "Expected '%s', got '%s'", fmt, res);

  fmt = "foo %a %a";
  ok = "foo bar bar";
  res = sreplace(p, fmt, "%a", "bar", NULL);
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  fmt = "foo %a %a %a %a %a %a %a %a";
  ok = "foo bar bar bar bar bar bar bar bar";
  
  res = sreplace(p, fmt, "%a", "bar", NULL);
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  /* sreplace() will not handle more than 8 occurrences of the same escape
   * sequence in the same line.  Make sure this happens.
   */
  fmt = "foo %a %a %a %a %a %a %a %a %a";
  ok = "foo bar bar bar bar bar bar bar bar bar";

  res = sreplace(p, fmt, "%a", "bar", NULL);
  fail_unless(strcmp(res, fmt) == 0, "Expected '%s', got '%s'", fmt, res);
}
END_TEST

START_TEST (pdircat_test) {
  char *res, *ok;

  res = pdircat(NULL, 0);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pdircat(p, 0);
  fail_unless(res != NULL,
    "Failed to handle empty arguments (expected '', got '%s')", res);
  fail_unless(strcmp(res, "") == 0, "Expected '%s', got '%s'", "", res);

  /* Comments in the pdircat() function suggest that an empty string
   * should be treated as a leading slash.  However, that never got
   * implemented.  Is this a bug, or just an artifact?  I doubt that it
   * is causing problems at present.
   */
  res = pdircat(p, "", NULL);
  ok = "";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pdircat(p, "foo", "bar", NULL);
  ok = "foo/bar";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pdircat(p, "", "foo", "bar", NULL);
  ok = "foo/bar";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pdircat(p, "/", "/foo/", "/bar/", NULL);
  ok = "/foo/bar/";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  /* Sadly, pdircat() only handles single leading/trailing slashes, not
   * an arbitrary number of leading/trailing slashes.
   */
  res = pdircat(p, "//", "//foo//", "//bar//", NULL);
  ok = "///foo///bar//";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (pstrcat_test) {
  char *res, *ok;

  res = pstrcat(NULL, 0);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrcat(p, 0);
  fail_unless(res != NULL,
    "Failed to handle empty arguments (expected '', got '%s')", res);
  fail_unless(strcmp(res, "") == 0, "Expected '%s', got '%s'", "", res);

  res = pstrcat(p, "", NULL);
  ok = "";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pstrcat(p, "foo", "bar", NULL);
  ok = "foobar";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pstrcat(p, "", "foo", "bar", NULL);
  ok = "foobar";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pstrcat(p, "/", "/foo/", "/bar/", NULL);
  ok = "//foo//bar/";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pdircat(p, "//", "//foo//", NULL, "//bar//", NULL);
  ok = "///foo//";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (pstrdup_test) {
  char *res, *ok;

  res = pstrdup(NULL, NULL);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrdup(p, NULL);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrdup(NULL, "");
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrdup(p, "foo");
  ok = "foo";
  fail_unless(strlen(res) == strlen(ok), "Expected len %u, got len %u",
    strlen(ok), strlen(res));
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (pstrndup_test) {
  char *res, *ok;

  res = pstrndup(NULL, NULL, 0);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrndup(p, NULL, 0);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrndup(NULL, "", 0);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pstrndup(p, "foo", 0);
  ok = "";
  fail_unless(strlen(res) == strlen(ok), "Expected len %u, got len %u",
    strlen(ok), strlen(res));
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pstrndup(p, "foo", 1);
  ok = "f";
  fail_unless(strlen(res) == strlen(ok), "Expected len %u, got len %u",
    strlen(ok), strlen(res));
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pstrndup(p, "foo", 10);
  ok = "foo";
  fail_unless(strlen(res) == strlen(ok), "Expected len %u, got len %u",
    strlen(ok), strlen(res));
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (strip_test) {
  char *ok, *res, *str;

  res = pr_str_strip(NULL, NULL);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_str_strip(p, NULL);
  fail_unless(res == NULL, "Failed to handle null str argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_str_strip(NULL, "foo");
  fail_unless(res == NULL, "Failed to handle null pool argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  str = pstrdup(p, "foo");
  res = pr_str_strip(p, str);
  fail_unless(res != NULL, "Failed to strip '%s': %s", str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, " \n \t foo");
  res = pr_str_strip(p, str);
  fail_unless(res != NULL, "Failed to strip '%s': %s", str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, "foo  \n \t \r");
  res = pr_str_strip(p, str);
  fail_unless(res != NULL, "Failed to strip '%s': %s", str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, "\r \n\n\t    foo  \n \t \r");
  res = pr_str_strip(p, str);
  fail_unless(res != NULL, "Failed to strip '%s': %s", str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (strip_end_test) {
  char *ch, *ok, *res, *str;

  res = pr_str_strip_end(NULL, NULL);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  str = pstrdup(p, "foo");

  res = pr_str_strip_end(str, NULL);
  fail_unless(res == NULL, "Failed to handle null char argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  ch = "\r\n";

  res = pr_str_strip_end(NULL, ch);
  fail_unless(res == NULL, "Failed to handle null str argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_str_strip_end(str, ch);
  fail_unless(res != NULL, "Failed to strip '%s' from end of '%s': %s",
    ch, str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, "foo\r\n");
  res = pr_str_strip_end(str, ch);
  fail_unless(res != NULL, "Failed to strip '%s' from end of '%s': %s",
    ch, str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, "foo\r\n\r\n\r\n");
  res = pr_str_strip_end(str, ch);
  fail_unless(res != NULL, "Failed to strip '%s' from end of '%s': %s",
    ch, str, strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (get_token_test) {
  char *ok, *res, *str;

  res = pr_str_get_token(NULL, NULL);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  str = NULL;
  res = pr_str_get_token(&str, NULL);
  fail_unless(res == NULL, "Failed to handle null str argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  str = pstrdup(p, "foo,bar,baz");
  res = pr_str_get_token(&str, NULL);
  fail_unless(res == NULL, "Failed to handle null sep argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_str_get_token(&str, ",");
  fail_unless(res != NULL, "Failed to get token from '%s': %s", str,
    strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_token(&str, ",");
  fail_unless(res != NULL, "Failed to get token from '%s': %s", str,
    strerror(errno));

  ok = "bar";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_token(&str, ",");
  fail_unless(res != NULL, "Failed to get token from '%s': %s", str,
    strerror(errno));

  ok = "baz";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_token(&str, ",");
  fail_unless(res == NULL, "Unexpectedly got token '%s'", res);
}
END_TEST

START_TEST (get_word_test) {
  char *ok, *res, *str;

  res = pr_str_get_word(NULL, 0);
  fail_unless(res == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  str = NULL;
  res = pr_str_get_word(&str, 0);
  fail_unless(res == NULL, "Failed to handle null str argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL");

  str = pstrdup(p, "  ");
  res = pr_str_get_word(&str, 0);
  fail_unless(res == NULL, "Failed to handle whitespace argument");

  str = pstrdup(p, " foo");
  res = pr_str_get_word(&str, PR_STR_FL_PRESERVE_WHITESPACE);
  fail_unless(res != NULL, "Failed to handle whitespace argument: %s",
    strerror(errno));

  ok = "";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_word(&str, PR_STR_FL_PRESERVE_WHITESPACE);
  fail_unless(res != NULL, "Failed to handle whitespace argument: %s",
    strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, "  # foo");
  res = pr_str_get_word(&str, 0);
  fail_unless(res == NULL, "Failed to handle commented argument");

  res = pr_str_get_word(&str, PR_STR_FL_PRESERVE_COMMENTS);
  fail_unless(res != NULL, "Failed to handle commented argument: %s",
    strerror(errno));

  ok = "#";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_word(&str, PR_STR_FL_PRESERVE_COMMENTS);
  fail_unless(res != NULL, "Failed to handle commented argument: %s",
    strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  str = pstrdup(p, "foo \"bar\" baz");
  res = pr_str_get_word(&str, 0);
  fail_unless(res != NULL, "Failed to handle quoted argument: %s",
    strerror(errno));

  ok = "foo";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_word(&str, 0);
  fail_unless(res != NULL, "Failed to handle quoted argument: %s",
    strerror(errno));

  ok = "bar";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);

  res = pr_str_get_word(&str, 0);
  fail_unless(res != NULL, "Failed to handle quoted argument: %s",
    strerror(errno));

  ok = "baz";
  fail_unless(strcmp(res, ok) == 0, "Expected '%s', got '%s'", ok, res);
}
END_TEST

START_TEST (is_boolean_test) {
  int res;

  res = pr_str_is_boolean(NULL);
  fail_unless(res == -1, "Failed to handle null argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);

  res = pr_str_is_boolean("on");
  fail_unless(res == TRUE, "Expected TRUE, got FALSE");

  res = pr_str_is_boolean("Yes");
  fail_unless(res == TRUE, "Expected TRUE, got FALSE");

  res = pr_str_is_boolean("TrUe");
  fail_unless(res == TRUE, "Expected TRUE, got FALSE");

  res = pr_str_is_boolean("1");
  fail_unless(res == TRUE, "Expected TRUE, got FALSE");

  res = pr_str_is_boolean("oFF");
  fail_unless(res == FALSE, "Expected FALSE, got TRUE");

  res = pr_str_is_boolean("no");
  fail_unless(res == FALSE, "Expected FALSE, got TRUE");

  res = pr_str_is_boolean("false");
  fail_unless(res == FALSE, "Expected FALSE, got TRUE");

  res = pr_str_is_boolean("0");
  fail_unless(res == FALSE, "Expected FALSE, got TRUE");

  res = pr_str_is_boolean("foo");
  fail_unless(res == -1, "Failed to handle null argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);
}
END_TEST

Suite *tests_get_str_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("str");

  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, sstrncpy_test);
  tcase_add_test(testcase, sstrcat_test);
  tcase_add_test(testcase, sreplace_test);
  tcase_add_test(testcase, pdircat_test);
  tcase_add_test(testcase, pstrcat_test);
  tcase_add_test(testcase, pstrdup_test);
  tcase_add_test(testcase, pstrndup_test);
  tcase_add_test(testcase, strip_test);
  tcase_add_test(testcase, strip_end_test);
  tcase_add_test(testcase, get_token_test);
  tcase_add_test(testcase, get_word_test);
  tcase_add_test(testcase, is_boolean_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
