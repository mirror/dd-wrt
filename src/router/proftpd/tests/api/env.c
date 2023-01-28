/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2020 The ProFTPD Project team
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

/* Env API tests */

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

START_TEST (env_get_test) {
  const char *key = "foo";
  char *res;

  res = pr_env_get(NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_get(p, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_get(NULL, key);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

#if defined(HAVE_GETENV)
  pr_env_unset(p, key);

  res = pr_env_get(p, key);
  ck_assert_msg(res == NULL, "Unexpectedly found value '%s' for key '%s'",
    res, key);

  /* XXX PATH should always be set in the environment, right? */
  res = pr_env_get(p, "PATH");
  ck_assert_msg(res != NULL, "Failed to get value for 'PATH': %s",
    strerror(errno));

#else
  res = pr_env_get(p, key);
  ck_assert_msg(errno == ENOSYS, "Failed to set errno to ENOSYS");
  ck_assert_msg(res == NULL);
#endif
}
END_TEST

START_TEST (env_set_test) {
  const char *key = "PR_TEST_FOO", *value = "bar";
  char *v;
  int res;
 
  res = pr_env_set(NULL, NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(p, NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(NULL, key, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(NULL, NULL, value);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(p, key, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(p, NULL, value);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(NULL, key, value);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(p, key, value);
  ck_assert_msg(res == 0, "Failed to handle set '%s': %s", key, strerror(errno));

  v = pr_env_get(p, key);
  ck_assert_msg(strcmp(v, value) == 0, "Expected '%s', got '%s'", value, v);
}
END_TEST

START_TEST (env_unset_test) {
  const char *key = "PR_TEST_FOO", *value = "bar";
  char *v;
  int res;

  res = pr_env_unset(NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_unset(p, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_unset(NULL, key);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_env_set(p, key, value);
  ck_assert_msg(res == 0, "Failed to set '%s': %s", key, strerror(errno));

  v = pr_env_get(p, key);
  ck_assert_msg(strcmp(v, value) == 0, "Expected '%s', got '%s'", value, v);

#if defined(HAVE_UNSETENV)
  res = pr_env_unset(p, key);
  ck_assert_msg(res == 0, "Failed to unset '%s': %s", key, strerror(errno));

  v = pr_env_get(p, key);
  ck_assert_msg(v == NULL, "Expected null, got '%s'", v);
#else
  res = pr_env_unset(p, key);
  ck_assert_msg(errno == ENOSYS, "Failed to set errno to ENOSYS");
  ck_assert_msg(res == -1);
#endif
}
END_TEST

Suite *tests_get_env_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("env");

  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, env_get_test);
  tcase_add_test(testcase, env_set_test);
  tcase_add_test(testcase, env_unset_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
