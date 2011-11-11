/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2011 The ProFTPD Project team
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

/* Regexp API tests
 * $Id: regexp.c,v 1.4 2011/05/23 20:50:31 castaglia Exp $
 */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  init_regexp();
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
    permanent_pool = NULL;
  } 
}

START_TEST (regexp_alloc_test) {
  pr_regex_t *res;

  res = pr_regexp_alloc(NULL);
  fail_unless(res != NULL, "Failed to allocate regex");
}
END_TEST

START_TEST (regexp_free_test) {
  pr_regex_t *res = NULL;

  pr_regexp_free(NULL, res);

  res = pr_regexp_alloc(NULL);
  fail_unless(res != NULL, "Failed to allocate regex");

  pr_regexp_free(NULL, res);
}
END_TEST

START_TEST (regexp_compile) {
  pr_regex_t *pre = NULL;
  int res;
  char errstr[256], *pattern;
  size_t errstrlen;

  pre = pr_regexp_alloc(NULL);

  pattern = "[=foo";
  res = pr_regexp_compile(pre, pattern, 0); 
  fail_unless(res != 0, "Successfully compiled pattern unexpectedly"); 

  errstrlen = pr_regexp_error(res, pre, errstr, sizeof(errstr));
  fail_unless(errstrlen > 0, "Failed to get regex compilation error string");

  pattern = "foo";
  res = pr_regexp_compile(pre, pattern, 0);
  fail_unless(res == 0, "Failed to compile regex pattern");

  pr_regexp_free(NULL, pre);
}
END_TEST

START_TEST (regexp_exec) {
  pr_regex_t *pre = NULL;
  int res;
  char *pattern, *str;

  pre = pr_regexp_alloc(NULL);

  pattern = "^foo";
  res = pr_regexp_compile(pre, pattern, 0);
  fail_unless(res == 0, "Failed to compile regex pattern");

  str = "bar";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  fail_unless(res != 0, "Matched string unexpectedly");

  str = "foobar";
  res = pr_regexp_exec(pre, str, 0, NULL, 0, 0, 0);
  fail_unless(res == 0, "Failed to match string");

  pr_regexp_free(NULL, pre);
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
  tcase_add_test(testcase, regexp_compile);
  tcase_add_test(testcase, regexp_exec);

  suite_add_tcase(suite, testcase);

  return suite;
}
