/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2014 The ProFTPD Project team
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

/* FSIO API tests
 * $Id: fsio.c,v 1.3 2014/01/20 19:36:27 castaglia Exp $
 */

#include "tests.h"

static pool *p = NULL;

/* Fixtures */

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
    permanent_pool = NULL;
  } 
}

/* Tests */

START_TEST (fs_clean_path_test) {
  char res[PR_TUNABLE_PATH_MAX+1], *path, *expected;

  res[sizeof(res)-1] = '\0';
  path = "/test.txt";
  pr_fs_clean_path(path, res, sizeof(res)-1);
  fail_unless(strcmp(res, path) == 0, "Expected cleaned path '%s', got '%s'",
    path, res);

  res[sizeof(res)-1] = '\0';
  path = "/./test.txt";
  pr_fs_clean_path(path, res, sizeof(res)-1);

  expected = "/test.txt";
  fail_unless(strcmp(res, expected) == 0,
    "Expected cleaned path '%s', got '%s'", expected, res);

  res[sizeof(res)-1] = '\0';
  path = "test.txt";
  pr_fs_clean_path(path, res, sizeof(res)-1);

  expected = "/test.txt";
  fail_unless(strcmp(res, expected) == 0,
    "Expected cleaned path '%s', got '%s'", path, res);
}
END_TEST

START_TEST (fs_clean_path2_test) {
  char res[PR_TUNABLE_PATH_MAX+1], *path, *expected;

  res[sizeof(res)-1] = '\0';
  path = "test.txt";
  pr_fs_clean_path2(path, res, sizeof(res)-1, 0);
  fail_unless(strcmp(res, path) == 0, "Expected cleaned path '%s', got '%s'",
    path, res);

  res[sizeof(res)-1] = '\0';
  path = "/./test.txt";
  pr_fs_clean_path2(path, res, sizeof(res)-1, 0);

  expected = "/test.txt";
  fail_unless(strcmp(res, expected) == 0,
    "Expected cleaned path '%s', got '%s'", expected, res);

  res[sizeof(res)-1] = '\0';
  path = "test.d///test.txt";
  pr_fs_clean_path2(path, res, sizeof(res)-1, 0);

  expected = "test.d/test.txt";
  fail_unless(strcmp(res, expected) == 0,
    "Expected cleaned path '%s', got '%s'", expected, res);

  res[sizeof(res)-1] = '\0';
  path = "/test.d///test.txt";
  pr_fs_clean_path2(path, res, sizeof(res)-1,
    PR_FSIO_CLEAN_PATH_FL_MAKE_ABS_PATH);

  expected = "/test.d/test.txt";
  fail_unless(strcmp(res, expected) == 0,
    "Expected cleaned path '%s', got '%s'", expected, res);

}
END_TEST

START_TEST (fs_dircat_test) {
  char buf[PR_TUNABLE_PATH_MAX+1], *a, *b, *ok;
  int res;

  res = pr_fs_dircat(NULL, 0, NULL, NULL);
  fail_unless(res == -1, "Failed to handle null arguments");
  fail_unless(errno == EINVAL,
    "Failed to set errno to EINVAL for null arguments");

  res = pr_fs_dircat(buf, 0, "foo", "bar");
  fail_unless(res == -1, "Failed to handle zero-length buffer");
  fail_unless(errno == EINVAL,
    "Failed to set errno to EINVAL for zero-length buffer");

  res = pr_fs_dircat(buf, -1, "foo", "bar");
  fail_unless(res == -1, "Failed to handle negative-length buffer");
  fail_unless(errno == EINVAL,
    "Failed to set errno to EINVAL for negative-length buffer");

  a = pcalloc(p, PR_TUNABLE_PATH_MAX);
  memset(a, 'A', PR_TUNABLE_PATH_MAX-1);

  b = "foo";

  res = pr_fs_dircat(buf, sizeof(buf)-1, a, b);
  fail_unless(res == -1, "Failed to handle too-long paths");
  fail_unless(errno == ENAMETOOLONG,
    "Failed to set errno to ENAMETOOLONG for too-long paths");

  a = "foo";
  b = "/bar";
  ok = b;
  res = pr_fs_dircat(buf, sizeof(buf)-1, a, b);
  fail_unless(res == 0, "Failed to concatenate abs-path path second dir");
  fail_unless(strcmp(buf, ok) == 0, "Expected concatenated dir '%s', got '%s'",
    ok, buf);
 
  a = "foo";
  b = "bar";
  ok = "foo/bar";
  res = pr_fs_dircat(buf, sizeof(buf)-1, a, b);
  fail_unless(res == 0, "Failed to concatenate two normal paths");
  fail_unless(strcmp(buf, ok) == 0, "Expected concatenated dir '%s', got '%s'",
    ok, buf);
 
  a = "foo/";
  b = "bar";
  ok = "foo/bar";
  res = pr_fs_dircat(buf, sizeof(buf)-1, a, b);
  fail_unless(res == 0, "Failed to concatenate first dir with trailing slash");
  fail_unless(strcmp(buf, ok) == 0, "Expected concatenated dir '%s', got '%s'",
    ok, buf);

  a = "";
  b = "";
  ok = "/";
  res = pr_fs_dircat(buf, sizeof(buf)-1, a, b);
  fail_unless(res == 0, "Failed to concatenate two empty paths");
  fail_unless(strcmp(buf, ok) == 0, "Expected concatenated dir '%s', got '%s'",
    ok, buf);
}
END_TEST

Suite *tests_get_fsio_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("fsio");

  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, fs_clean_path_test);
  tcase_add_test(testcase, fs_clean_path2_test);
  tcase_add_test(testcase, fs_dircat_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
