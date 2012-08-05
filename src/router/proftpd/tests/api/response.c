/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2011-2012 The ProFTPD Project team
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

/* Response API tests
 * $Id: response.c,v 1.2.2.2 2012/07/26 22:40:45 castaglia Exp $
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

START_TEST (response_pool_get_test) {
  pool *res;

  res = pr_response_get_pool();
  fail_unless(res == NULL, "Response pool not null as expected");
}
END_TEST

START_TEST (response_pool_set_test) {
  pool *res;

  pr_response_set_pool(p);
  res = pr_response_get_pool();
  fail_unless(res == p, "Response pool not %p as expected", p);
}
END_TEST

START_TEST (response_add_test) {
  int res;
  char *last_resp_code = NULL, *last_resp_msg = NULL;
  char *resp_code = R_200, *resp_msg = "OK";

  pr_response_set_pool(p);
  pr_response_add(resp_code, "%s", resp_msg);

  res = pr_response_get_last(p, &last_resp_code, &last_resp_msg);
  fail_unless(res == 0, "Failed to get last values: %d (%s)", errno,
    strerror(errno));

  fail_unless(last_resp_code != NULL, "Last response code unexpectedly null");
  fail_unless(strcmp(last_resp_code, resp_code) == 0,
    "Expected response code '%s', got '%s'", resp_code, last_resp_code);
  
  fail_unless(last_resp_msg != NULL, "Last response message unexpectedly null");
  fail_unless(strcmp(last_resp_msg, resp_msg) == 0,
    "Expected response message '%s', got '%s'", resp_msg, last_resp_msg);
}
END_TEST

START_TEST (response_add_err_test) {
  int res;
  char *last_resp_code = NULL, *last_resp_msg = NULL;
  char *resp_code = R_450, *resp_msg = "Busy";

  pr_response_set_pool(p);
  pr_response_add_err(resp_code, "%s", resp_msg);

  res = pr_response_get_last(p, &last_resp_code, &last_resp_msg);
  fail_unless(res == 0, "Failed to get last values: %d (%s)", errno,
    strerror(errno));

  fail_unless(last_resp_code != NULL, "Last response code unexpectedly null");
  fail_unless(strcmp(last_resp_code, resp_code) == 0,
    "Expected response code '%s', got '%s'", resp_code, last_resp_code);

  fail_unless(last_resp_msg != NULL, "Last response message unexpectedly null");
  fail_unless(strcmp(last_resp_msg, resp_msg) == 0,
    "Expected response message '%s', got '%s'", resp_msg, last_resp_msg);
}
END_TEST

START_TEST (response_get_last_test) {
  int res;
  char *resp_code = NULL, *resp_msg = NULL;

  res = pr_response_get_last(NULL, NULL, NULL);
  fail_unless(res == -1, "Failed to handle null pool");
  fail_unless(errno == EINVAL, "Expected errno %d (%s), got %d (%s)",
    EINVAL, strerror(EINVAL), errno, strerror(errno));

  res = pr_response_get_last(p, NULL, NULL);
  fail_unless(res == -1, "Failed to handle null argument");
  fail_unless(errno == EINVAL, "Expected errno %d (%s), got %d (%s)",
    EINVAL, strerror(EINVAL), errno, strerror(errno));

  res = pr_response_get_last(p, &resp_code, &resp_msg);
  fail_unless(res == 0, "Failed to get last values: %d (%s)", errno,
    strerror(errno));

  fail_unless(resp_code == NULL,
    "Last response code not null as expected: %s", resp_code);
  fail_unless(resp_msg == NULL,
    "Last response message not null as expected: %s", resp_msg);
}
END_TEST

START_TEST (response_pool_bug3711_test) {
  cmd_rec *cmd;
  pool *resp_pool, *cmd_pool;
  char *err_code = R_450, *err_msg = "Busy";

  resp_pool = make_sub_pool(p);
  cmd_pool = make_sub_pool(p);

  cmd = pr_cmd_alloc(cmd_pool, 1, "foo");

  pr_response_set_pool(cmd->pool);
  pr_response_add_err(err_code, "%s", err_msg);

  /* We expect segfaults here, so use the mark_point() function to get
   * more accurate reporting of the problematic line of code in the
   * error logs.
   */
  mark_point();

  /* We explicitly do NOT reset the Response API pool here, to emulate the
   * behavior of Bug#3711.
   *
   * In the future, we could address this by proving a Pool API function
   * that e.g. the Response API could use, to check whether the given
   * pool is still a valid pool.  To do this, the Pool API would keep a
   * list of allocated pools, which would then be scanned.  In practice such
   * a list is maintained, albeit in a tree form.  And there is tracking
   * of the root trees for pools; permanent_pool is not the only root pool
   * which can be created/used.
   */
  destroy_pool(cmd_pool);

  mark_point();
  pr_response_add_err(err_code, "%s", err_msg);

  mark_point();
  pr_response_add_err(err_code, "%s", err_msg);

  mark_point();
  pr_response_add_err(err_code, "%s", err_msg);
}
END_TEST


Suite *tests_get_response_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("response");

  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, response_pool_get_test);
  tcase_add_test(testcase, response_pool_set_test);
  tcase_add_test(testcase, response_add_test);
  tcase_add_test(testcase, response_add_err_test);
  tcase_add_test(testcase, response_get_last_test);

#if 0
  /* We expect this test to fail due to a segfault; see Bug#3711. */
  tcase_add_test_raise_signal(testcase, response_pool_bug3711_test, SIGSEGV);
#endif

  suite_add_tcase(suite, testcase);

  return suite;
}
