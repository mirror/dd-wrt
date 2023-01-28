/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2022 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/* Regexp API tests */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  init_regexp();

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("regexp", 1, 20);
  }
}

static void tear_down(void) {
  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("regexp", 0, 0);
  }

  if (p) {
    destroy_pool(p);
    p = permanent_pool = NULL;
  }
}

START_TEST (regexp_alloc_test) {
  pr_regex_t *res;

  res = pr_regexp_alloc(NULL);
  ck_assert_msg(res != NULL, "Failed to allocate regex: %s", strerror(errno));
  pr_regexp_free(NULL, res);
}
END_TEST

START_TEST (regexp_free_test) {
  mark_point();
  pr_regexp_free(NULL, NULL);
}
END_TEST

START_TEST (regexp_error_test) {
  size_t bufsz, res;
  const pr_regex_t *pre;
  char *buf;

  mark_point();
  res = pr_regexp_error(0, NULL, NULL, 0);
  ck_assert_msg(res == 0, "Failed to handle null regexp");

  pre = (const pr_regex_t *) 3;

  mark_point();
  res = pr_regexp_error(0, pre, NULL, 0);
  ck_assert_msg(res == 0, "Failed to handle null buf");

  bufsz = 256;
  buf = pcalloc(p, bufsz);

  mark_point();
  res = pr_regexp_error(0, pre, buf, 0);
  ck_assert_msg(res == 0, "Failed to handle zero bufsz");
}
END_TEST

START_TEST (regexp_compile_test) {
  pr_regex_t *pre = NULL;
  int res;
  char errstr[256], *pattern;
  size_t errstrlen;

  mark_point();
  res = pr_regexp_compile(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  pre = pr_regexp_alloc(NULL);
  res = pr_regexp_compile(pre, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pattern");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pattern = "[=foo";
  res = pr_regexp_compile(pre, pattern, 0); 
  ck_assert_msg(res != 0, "Successfully compiled pattern unexpectedly"); 

  errstrlen = pr_regexp_error(1, NULL, NULL, 0);
  ck_assert_msg(errstrlen == 0, "Failed to handle null arguments");

  errstrlen = pr_regexp_error(1, pre, NULL, 0);
  ck_assert_msg(errstrlen == 0, "Failed to handle null buffer");

  errstrlen = pr_regexp_error(1, pre, errstr, 0);
  ck_assert_msg(errstrlen == 0, "Failed to handle zero buffer length");

  errstrlen = pr_regexp_error(res, pre, errstr, sizeof(errstr));
  ck_assert_msg(errstrlen > 0, "Failed to get regex compilation error string");

  mark_point();
  pattern = "foo";
  res = pr_regexp_compile(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  mark_point();
  pr_regexp_free(NULL, pre);
  pre = pr_regexp_alloc(NULL);
  pattern = "foo";
  res = pr_regexp_compile(pre, pattern, REG_ICASE);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);
  pr_regexp_free(NULL, pre);
}
END_TEST

START_TEST (regexp_compile_posix_test) {
  pr_regex_t *pre = NULL;
  int res;
  char errstr[256], *pattern;
  size_t errstrlen;

  res = pr_regexp_compile_posix(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pre = pr_regexp_alloc(NULL);

  res = pr_regexp_compile_posix(pre, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pattern");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pattern = "[=foo";
  res = pr_regexp_compile_posix(pre, pattern, 0);
  ck_assert_msg(res != 0, "Successfully compiled pattern unexpectedly");

  errstrlen = pr_regexp_error(res, pre, errstr, sizeof(errstr));
  ck_assert_msg(errstrlen > 0, "Failed to get regex compilation error string");

  pattern = "foo";
  res = pr_regexp_compile_posix(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  pattern = "foo";
  res = pr_regexp_compile_posix(pre, pattern, REG_ICASE);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  pr_regexp_free(NULL, pre);
}
END_TEST

START_TEST (regexp_get_pattern_test) {
  pr_regex_t *pre = NULL;
  int res;
  const char *str;
  char *pattern;

  str = pr_regexp_get_pattern(NULL);
  ck_assert_msg(str == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pre = pr_regexp_alloc(NULL);

  str = pr_regexp_get_pattern(pre);
  ck_assert_msg(str == NULL, "Failed to handle null pattern");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  pattern = "^foo";
  res = pr_regexp_compile(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  str = pr_regexp_get_pattern(pre);
  ck_assert_msg(str != NULL, "Failed to get regex pattern: %s", strerror(errno));
  ck_assert_msg(strcmp(str, pattern) == 0, "Expected '%s', got '%s'", pattern,
    str);

  pr_regexp_free(NULL, pre);
}
END_TEST

START_TEST (regexp_set_limits_test) {
  int res;
  pr_regex_t *pre = NULL;
  const char *pattern, *str;

  res = pr_regexp_set_limits(0, 0);
  ck_assert_msg(res == 0, "Failed to set limits: %s", strerror(errno));

  /* Set the limits, and compile/execute a regex. */
  res = pr_regexp_set_limits(1, 1);
  ck_assert_msg(res == 0, "Failed to set limits: %s", strerror(errno));

  pre = pr_regexp_alloc(NULL);

  pattern = "^foo";
  res = pr_regexp_compile(pre, pattern, REG_ICASE);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  str = "fooBAR";
  (void) pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);

  pr_regexp_free(NULL, pre);
}
END_TEST

START_TEST (regexp_exec_test) {
  pr_regex_t *pre = NULL;
  int res;
  char *pattern, *str;

  res = pr_regexp_exec(NULL, NULL, 0, NULL, 0, 0, 0);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  pre = pr_regexp_alloc(NULL);

  pattern = "^foo";
  res = pr_regexp_compile(pre, pattern, REG_ICASE);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  res = pr_regexp_exec(pre, NULL, 0, NULL, 0, 0, 0);
  ck_assert_msg(res != 0, "Failed to handle null string");

  str = "bar";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  ck_assert_msg(res != 0, "Matched string unexpectedly");

  str = "foobar";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");

  str = "FOOBAR";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");

  pr_regexp_free(NULL, pre);
  pre = pr_regexp_alloc(NULL);

  pattern = "^foo";
  res = pr_regexp_compile_posix(pre, pattern, REG_ICASE);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  res = pr_regexp_exec(pre, NULL, 0, NULL, 0, 0, 0);
  ck_assert_msg(res != 0, "Failed to handle null string");

  str = "BAR";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  ck_assert_msg(res != 0, "Matched string unexpectedly");

  str = "foobar";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");

#if !defined(PR_USE_PCRE2) && \
    !defined(PR_USE_PCRE)
  /* Note that when PCRE support is used, behavior of POSIX matching may be
   * surprising; I suspect it relates to the overrides in <pcreposix.h>.
   */
  str = "FOOBAR";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");
#endif /* !PR_USE_PCRE2 and !PR_USE_PCRE */

  pr_regexp_free(NULL, pre);
}
END_TEST

#if !defined(PR_USE_PCRE2) && \
    !defined(PR_USE_PCRE)
START_TEST (regexp_capture_posix_test) {
  register unsigned int i;
  pr_regex_t *pre = NULL;
  int captured = FALSE, res;
  char *pattern, *str;
  size_t nmatches;
  regmatch_t *matches;

  pre = pr_regexp_alloc(NULL);

  pattern = "(.*)";
  res = pr_regexp_compile_posix(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  nmatches = 10;
  matches = pcalloc(p, sizeof(regmatch_t) * nmatches);

  str = "foobar";
  res = pr_regexp_exec(pre, str, nmatches, matches, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");

  for (i = 0; i < nmatches; i++) {
    int match_len;
    const char *match_text;

    if (matches[i].rm_so == -1 ||
        matches[i].rm_eo == -1) {
      break;
    }

    match_text = &(str[matches[i].rm_so]);
    match_len = matches[i].rm_eo - matches[i].rm_so;

    ck_assert_msg(strcmp(match_text, str) == 0,
      "Expected matched text '%s', got '%s'", str, match_text);
    ck_assert_msg(match_len == 6,
      "Expected match text len 6, got %d", match_len);

    captured = TRUE;
  }

  ck_assert_msg(captured == TRUE,
    "POSIX regex failed to capture expected groups");

  pr_regexp_free(NULL, pre);
}
END_TEST
#endif /* !PR_USE_PCRE2 and !PR_USE_PCRE */

#if defined(PR_USE_PCRE)
START_TEST (regexp_capture_pcre_test) {
  register unsigned int i;
  pr_regex_t *pre = NULL;
  int captured = FALSE, res;
  char *pattern, *str;
  size_t nmatches;
  regmatch_t *matches;

  pre = pr_regexp_alloc(NULL);

  pattern = "(.*)";
  res = pr_regexp_compile(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  nmatches = 10;
  matches = pcalloc(p, sizeof(regmatch_t) * nmatches);

  str = "foobar";
  res = pr_regexp_exec(pre, str, nmatches, matches, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");

  for (i = 0; i < nmatches; i++) {
    int match_len;
    const char *match_text;

    if (matches[i].rm_so == -1 ||
        matches[i].rm_eo == -1) {
      break;
    }

    match_text = &(str[matches[i].rm_so]);
    match_len = matches[i].rm_eo - matches[i].rm_so;

    ck_assert_msg(strcmp(match_text, str) == 0,
      "Expected matched text '%s', got '%s' (i = %u)", str, match_text, i);
    ck_assert_msg(match_len == 6,
      "Expected match text len 6, got %d (i = %u)", match_len, i);

    captured = TRUE;
  }

  ck_assert_msg(captured == TRUE,
    "PCRE regex failed to capture expected groups");

  pr_regexp_free(NULL, pre);
}
END_TEST
#endif /* PR_USE_PCRE */

#if defined(PR_USE_PCRE2)
START_TEST (regexp_capture_pcre2_test) {
  register unsigned int i;
  pr_regex_t *pre = NULL;
  int captured = FALSE, res;
  char *pattern, *str;
  size_t nmatches;
  regmatch_t *matches;

  pre = pr_regexp_alloc(NULL);

  pattern = "(.*)";
  res = pr_regexp_compile(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regex pattern '%s'", pattern);

  nmatches = 10;
  matches = pcalloc(p, sizeof(regmatch_t) * nmatches);

  str = "foobar";
  res = pr_regexp_exec(pre, str, nmatches, matches, 0, 0, 0);
  ck_assert_msg(res == 0, "Failed to match string");

  for (i = 0; i < nmatches; i++) {
    int match_len;
    const char *match_text;

    if (matches[i].rm_so == -1 ||
        matches[i].rm_eo == -1) {
      break;
    }

    match_text = &(str[matches[i].rm_so]);
    match_len = matches[i].rm_eo - matches[i].rm_so;

    ck_assert_msg(strcmp(match_text, str) == 0,
      "Expected matched text '%s', got '%s' (i = %u)", str, match_text, i);
    ck_assert_msg(match_len == 6,
      "Expected match text len 6, got %d (i = %u)", match_len, i);

    captured = TRUE;
  }

  ck_assert_msg(captured == TRUE,
    "PCRE2 regex failed to capture expected groups");

  pr_regexp_free(NULL, pre);
}
END_TEST
#endif /* PR_USE_PCRE2 */

START_TEST (regexp_cleanup_test) {
  pr_regex_t *pre, *pre2, *pre3;
  int res;
  char *pattern;

  pattern = "^foo";

  pre = pr_regexp_alloc(NULL);
  res = pr_regexp_compile(pre, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regexp pattern '%s'", pattern);

  pattern = "bar$";
  pre2 = pr_regexp_alloc(NULL);
  res = pr_regexp_compile(pre2, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile regexp pattern '%s'", pattern);

  pattern = "&baz$";
  pre3 = pr_regexp_alloc(NULL);
  res = pr_regexp_compile_posix(pre3, pattern, 0);
  ck_assert_msg(res == 0, "Failed to compile POSIX regexp pattern '%s'", pattern);

  mark_point();
  pr_event_generate("core.restart", NULL);

  mark_point();
  pr_event_generate("core.exit", NULL);

  mark_point();
  pr_regexp_free(NULL, pre);

  mark_point();
  pr_regexp_free(NULL, pre2);
}
END_TEST

START_TEST (regexp_set_engine_test) {
  int res;

  mark_point();
  res = pr_regexp_set_engine(NULL);
  ck_assert_msg(res == 0,
    "Failed to restore default engine: %s", strerror(errno));

  mark_point();
  res = pr_regexp_set_engine("foobar");
  ck_assert_msg(res < 0, "Failed to handle unknown engine");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_regexp_set_engine("posix");
  ck_assert_msg(res == 0, "Failed to handle POSIX engine: %s", strerror(errno));

  mark_point();
  res = pr_regexp_set_engine("pcre");
#if defined(PR_USE_PCRE)
  ck_assert_msg(res == 0, "Failed to handle PCRE engine: %s", strerror(errno));
#else
  ck_assert_msg(res < 0,
    "Failed to handle PCRE engine when lacking PCRE support");
  ck_assert_msg(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
    strerror(errno), errno);
#endif /* PR_USE_PCRE */

  mark_point();
  res = pr_regexp_set_engine("pcre2");
#if defined(PR_USE_PCRE2)
  ck_assert_msg(res == 0, "Failed to handle PCRE2 engine: %s", strerror(errno));
#else
  ck_assert_msg(res < 0,
    "Failed to handle PCRE2 engine when lacking PCRE2 support");
  ck_assert_msg(errno == ENOSYS, "Expected ENOSYS (%d), got %s (%d)", ENOSYS,
    strerror(errno), errno);
#endif /* PR_USE_PCRE2 */

  (void) pr_regexp_set_engine(NULL);
}
END_TEST

Suite *tests_get_regexp_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("regexp");

  testcase = tcase_create("base");
  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, regexp_alloc_test);
  tcase_add_test(testcase, regexp_free_test);
  tcase_add_test(testcase, regexp_error_test);
  tcase_add_test(testcase, regexp_compile_test);
  tcase_add_test(testcase, regexp_compile_posix_test);
  tcase_add_test(testcase, regexp_exec_test);
#if !defined(PR_USE_PCRE2) && \
    !defined(PR_USE_PCRE)
  tcase_add_test(testcase, regexp_capture_posix_test);
#endif /* !PR_USE_PCRE2 and !PR_USE_PCRE */
#if defined(PR_USE_PCRE)
  tcase_add_test(testcase, regexp_capture_pcre_test);
#endif /* PR_USE_PCRE */
#if defined(PR_USE_PCRE2)
  tcase_add_test(testcase, regexp_capture_pcre2_test);
#endif /* PR_USE_PCRE */
  tcase_add_test(testcase, regexp_get_pattern_test);
  tcase_add_test(testcase, regexp_set_limits_test);
  tcase_add_test(testcase, regexp_cleanup_test);
  tcase_add_test(testcase, regexp_set_engine_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
