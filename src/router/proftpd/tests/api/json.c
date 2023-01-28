/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2017-2022 The ProFTPD Project team
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

/* JSON API tests */

#include <math.h>
#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = make_sub_pool(NULL);
  }

  init_json();

  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("json", 1, 20);
  }
}

static void tear_down(void) {
  if (getenv("TEST_VERBOSE") != NULL) {
    pr_trace_set_levels("json", 0, 0);
  }

  finish_json();

  if (p != NULL) {
    destroy_pool(p);
    p = NULL;
  }
}

START_TEST (json_object_free_test) {
  int res;

  mark_point();
  res = pr_json_object_free(NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (json_object_alloc_test) {
  int res;
  pr_json_object_t *json;

  mark_point();
  json = pr_json_object_alloc(NULL);
  ck_assert_msg(json == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  json = pr_json_object_alloc(p);
  ck_assert_msg(json != NULL, "Failed to allocate object: %s", strerror(errno));

  mark_point();
  res = pr_json_object_free(json);
  ck_assert_msg(res == 0, "Failed to free object: %s", strerror(errno));
}
END_TEST

START_TEST (json_object_from_text_test) {
  pr_json_object_t *json;
  const char *text;

  mark_point();
  json = pr_json_object_from_text(NULL, NULL);
  ck_assert_msg(json == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  json = pr_json_object_from_text(p, NULL);
  ck_assert_msg(json == NULL, "Failed to handle null text");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "foo bar";

  mark_point();
  json = pr_json_object_from_text(p, text);
  ck_assert_msg(json == NULL, "Failed to handle invalid text '%s'", text);
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  text = "[\"foo\",\"bar\"]";

  mark_point();
  json = pr_json_object_from_text(p, text);
  ck_assert_msg(json == NULL, "Failed to handle non-object text '%s'", text);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  text = "{\"foo\":\"bar\"}";

  mark_point();
  json = pr_json_object_from_text(p, text);
  ck_assert_msg(json != NULL, "Failed to handle text '%s': %s", text,
    strerror(errno));
  (void) pr_json_object_free(json);

  mark_point();
  text = "{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":{\"key\":null}}}}}}}}}}}}}}}}}";
  json = pr_json_object_from_text(p, text);
  ck_assert_msg(json == NULL, "Failed to handle depth-exceeding text '%s'", text);
}
END_TEST

START_TEST (json_object_to_text_test) {
  const char *text, *expected;
  pr_json_object_t *json;

  mark_point();
  text = pr_json_object_to_text(NULL, NULL, NULL);
  ck_assert_msg(text == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  text = pr_json_object_to_text(p, NULL, NULL);
  ck_assert_msg(text == NULL, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_object_alloc(p);

  mark_point();
  text = pr_json_object_to_text(p, json, NULL);
  ck_assert_msg(text == NULL, "Failed to handle null indent");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  expected = "{}";

  mark_point();
  text = pr_json_object_to_text(p, json, "");
  ck_assert_msg(text != NULL, "Failed to get text for object: %s",
    strerror(errno));
  ck_assert_msg(strcmp(text, expected) == 0, "Expected '%s', got '%s'", expected,
    text);

  (void) pr_json_object_set_string(p, json, "foo", "bar");
  expected = "{\"foo\":\"bar\"}";

  mark_point();
  text = pr_json_object_to_text(p, json, "");
  ck_assert_msg(text != NULL, "Failed to get text for object: %s",
    strerror(errno));
  ck_assert_msg(strcmp(text, expected) == 0, "Expected '%s', got '%s'", expected,
    text);

  (void) pr_json_object_free(json);
}
END_TEST

static int object_item_ok(const char *key, int val_type, const void *val,
    size_t valsz, void *user_data) {
  return 0;
}

static int object_item_fail(const char *key, int val_type, const void *val,
    size_t valsz, void *user_data) {
  errno = EPERM;
  return -1;
}

START_TEST(json_object_foreach_test) {
  int res;
  pr_json_object_t *json = NULL;
  const char *text;

  mark_point();
  res = pr_json_object_foreach(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_json_object_foreach(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null object");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_foreach(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null callback");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  (void) pr_json_object_free(json);

  text = "{\"foo\":true,\"bar\":false,\"baz\":{}}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_foreach(p, json, object_item_ok, NULL);
  ck_assert_msg(res == 0, "Failed to iterate over object: %s", strerror(errno));

  mark_point();
  res = pr_json_object_foreach(p, json, object_item_fail, NULL);
  ck_assert_msg(res < 0, "Failing callback succeeded unexpectedly");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  (void) pr_json_object_free(json);
}
END_TEST

START_TEST (json_object_count_test) {
  int res;
  pr_json_object_t *json;
  const char *text;

  mark_point();
  res = pr_json_object_count(NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_count(json);
  ck_assert_msg(res == 0, "Expected 0, got %d", res);

  (void) pr_json_object_free(json);

  text = "{\"foo\":true,\"bar\":false,\"baz\":1}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_count(json);
  ck_assert_msg(res == 3, "Expected 3, got %d", res);

  (void) pr_json_object_free(json);
}
END_TEST

START_TEST (json_object_exists_test) {
  int res;
  pr_json_object_t *json;
  const char *key, *text;

  mark_point();
  res = pr_json_object_exists(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_exists(json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "foo";

  mark_point();
  res = pr_json_object_exists(json, key);
  ck_assert_msg(res == FALSE, "Expected FALSE, got %d", res);

  (void) pr_json_object_free(json);

  text = "{\"foo\":true,\"bar\":false,\"baz\":1}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_exists(json, key);
  ck_assert_msg(res == TRUE, "Expected TRUE, got %d", res);

  (void) pr_json_object_free(json);
}
END_TEST

START_TEST (json_object_remove_test) {
  int res;
  pr_json_object_t *json;
  const char *key, *text;

  mark_point();
  res = pr_json_object_remove(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_remove(json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  key = "foo";

  mark_point();
  res = pr_json_object_remove(json, key);
  ck_assert_msg(res == 0, "Failed to remove nonexistent key '%s': %s", key,
    strerror(errno));

  res = pr_json_object_count(json);
  ck_assert_msg(res == 0, "Expected count 0, got %d", res);

  (void) pr_json_object_free(json);

  text = "{\"foo\":true,\"bar\":false,\"baz\":1}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_remove(json, key);
  ck_assert_msg(res == 0, "Failed to remove existing key '%s': %s", key,
    strerror(errno));
  
  res = pr_json_object_count(json);
  ck_assert_msg(res == 2, "Expected count 2, got %d", res);

  mark_point();
  res = pr_json_object_exists(json, key);
  ck_assert_msg(res == FALSE, "Expected FALSE, got %d", res);

  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_get_bool_test) {
  int res, val;
  const char *key, *text;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_get_bool(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_bool(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_get_bool(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_get_bool(p, json, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_bool(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":1,\"bar\":true}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_get_bool(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle non-boolean key '%s'", key);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  key = "bar";
 
  mark_point();
  res = pr_json_object_get_bool(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(val == TRUE, "Expected TRUE, got %d", val);
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_set_bool_test) {
  int res, val = TRUE;
  const char *key;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_set_bool(NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_set_bool(p, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_set_bool(p, json, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_set_bool(p, json, key, val);
  ck_assert_msg(res == 0, "Failed to set key '%s' to %d: %s", key, val,
    strerror(errno));
 
  val = FALSE;
 
  mark_point();
  res = pr_json_object_get_bool(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(val == TRUE, "Expected TRUE, got %d", val);
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_get_null_test) {
  int res;
  const char *key, *text;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_get_null(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_null(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_get_null(p, json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_get_null(p, json, key);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":1,\"bar\":null}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_get_null(p, json, key);
  ck_assert_msg(res < 0, "Failed to handle non-null key '%s'", key);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  key = "bar";
 
  mark_point();
  res = pr_json_object_get_null(p, json, key);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_set_null_test) {
  int res;
  const char *key;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_set_null(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_set_null(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_set_null(p, json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_set_null(p, json, key);
  ck_assert_msg(res == 0, "Failed to set key '%s': %s", key, strerror(errno));
 
  mark_point();
  res = pr_json_object_get_null(p, json, key);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_get_number_test) {
  int res;
  double val;
  const char *key, *text;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_get_number(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_number(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_get_number(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_get_number(p, json, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_number(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":false,\"bar\":7}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_get_number(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle non-number key '%s'", key);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  key = "bar";
 
  mark_point();
  res = pr_json_object_get_number(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(fabs(val) == fabs((double) 7.0), "Expected 7, got %e", val);
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_set_number_test) {
  int res;
  double val = 7;
  const char *key;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_set_number(NULL, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_set_number(p, NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_set_number(p, json, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_set_number(p, json, key, val);
  ck_assert_msg(res == 0, "Failed to set key '%s' to %f: %s", key, val,
    strerror(errno));
 
  val = 3;
 
  mark_point();
  res = pr_json_object_get_number(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(fabs(val) == fabs((double) 7.0), "Expected 7, got %e", val);
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_get_string_test) {
  int res;
  const char *key, *val, *text;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_get_string(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_string(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_get_string(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_get_string(p, json, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_string(p, json, key, (char **) &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":false,\"bar\":\"baz\"}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_get_string(p, json, key, (char **) &val);
  ck_assert_msg(res < 0, "Failed to handle non-string key '%s'", key);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  key = "bar";
 
  mark_point();
  res = pr_json_object_get_string(p, json, key, (char **) &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(strcmp(val, "baz") == 0, "Expected 'baz', got '%s'", val);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":\"\"}";
  json = pr_json_object_from_text(p, text);

  key = "foo";

  mark_point();
  res = pr_json_object_get_string(p, json, key, (char **) &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(strcmp(val, "") == 0, "Expected '', got '%s'", val);

  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_set_string_test) {
  int res;
  const char *key, *val;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_set_string(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_set_string(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_set_string(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";
 
  mark_point();
  res = pr_json_object_set_string(p, json, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  val = "Hello, World!";

  mark_point();
  res = pr_json_object_set_string(p, json, key, val);
  ck_assert_msg(res == 0, "Failed to set key '%s' to '%s': %s", key, val,
    strerror(errno));
 
  val = "glarg";
 
  mark_point();
  res = pr_json_object_get_string(p, json, key, (char **) &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(strcmp(val, "Hello, World!") == 0,
    "Expected 'Hello, World!', got '%s'", val);
 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_get_array_test) {
  int res;
  const char *key, *text;
  char *expected = NULL, *str = NULL;
  pr_json_array_t *val = NULL;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_get_array(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_array(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_get_array(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_get_array(p, json, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_array(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":false,\"bar\":[\"baz\"]}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_get_array(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle non-array key '%s'", key);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  key = "bar";
  val = NULL;
 
  mark_point();
  res = pr_json_object_get_array(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected array, got null");

  expected = "[\"baz\"]";
  str = pr_json_array_to_text(p, val, "");
  ck_assert_msg(strcmp(str, expected) == 0,
    "Expected '%s', got '%s'", expected, str);

  mark_point();
  (void) pr_json_array_free(val);

  mark_point();
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_set_array_test) {
  int res;
  const char *key, *text;
  pr_json_array_t *val = NULL;
  pr_json_object_t *json;

  mark_point();
  res = pr_json_object_set_array(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_set_array(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_set_array(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_set_array(p, json, key, val);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "[1, 1, 2, 3, 5, 8]";
  val = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_object_set_array(p, json, key, val);
  ck_assert_msg(res == 0, "Failed to set key '%s' to '%s': %s", key, text,
    strerror(errno));

  val = NULL;
 
  mark_point();
  res = pr_json_object_get_array(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected array, got null");

  mark_point();
  (void) pr_json_array_free(val); 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_get_object_test) {
  int res;
  const char *key, *text;
  char *expected = NULL, *str = NULL;
  pr_json_object_t *json, *val = NULL;

  mark_point();
  res = pr_json_object_get_object(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_object(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_get_object(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_get_object(p, json, key, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_get_object(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent key '%s'", key);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_object_free(json);

  text = "{\"foo\":false,\"bar\":{\"baz\":null}}";
  json = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_get_object(p, json, key, &val);
  ck_assert_msg(res < 0, "Failed to handle non-object key '%s'", key);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  key = "bar";
  val = NULL;
 
  mark_point();
  res = pr_json_object_get_object(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected object, got null");

  expected = "{\"baz\":null}";
  str = pr_json_object_to_text(p, val, "");
  ck_assert_msg(strcmp(str, expected) == 0,
    "Expected '%s', got '%s'", expected, str);

  mark_point();
  (void) pr_json_object_free(val);

  mark_point();
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_object_set_object_test) {
  int res;
  const char *key, *text;
  pr_json_object_t *json, *val = NULL;

  mark_point();
  res = pr_json_object_set_object(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_object_set_object(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_object_alloc(p);

  mark_point();
  res = pr_json_object_set_object(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  key = "foo";

  mark_point();
  res = pr_json_object_set_object(p, json, key, val);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "{\"eeny\":1,\"meeny\":2,\"miny\":3,\"moe\":false}";
  val = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_object_set_object(p, json, key, val);
  ck_assert_msg(res == 0, "Failed to set key '%s' to '%s': %s", key, text,
    strerror(errno));

  val = NULL;
 
  mark_point();
  res = pr_json_object_get_object(p, json, key, &val);
  ck_assert_msg(res == 0, "Failed to handle existing key '%s': %s", key,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected object, got null");

  mark_point();
  (void) pr_json_object_free(val); 
  (void) pr_json_object_free(json);
}
END_TEST

START_TEST(json_array_free_test) {
  int res;

  mark_point();
  res = pr_json_array_free(NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST(json_array_alloc_test) {
  int res;
  pr_json_array_t *json;

  mark_point();
  json = pr_json_array_alloc(NULL);
  ck_assert_msg(json == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  json = pr_json_array_alloc(p);
  ck_assert_msg(json != NULL, "Failed to allocate array: %s", strerror(errno));

  mark_point();
  res = pr_json_array_free(json);
  ck_assert_msg(res == 0, "Failed to free array: %s", strerror(errno));
}
END_TEST

START_TEST(json_array_from_text_test) {
  pr_json_array_t *json;
  const char *text;

  mark_point();
  json = pr_json_array_from_text(NULL, NULL);
  ck_assert_msg(json == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  json = pr_json_array_from_text(p, NULL);
  ck_assert_msg(json == NULL, "Failed to handle null text");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "foo bar";

  mark_point();
  json = pr_json_array_from_text(p, text);
  ck_assert_msg(json == NULL, "Failed to handle invalid text '%s'", text);
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  text = "{\"foo\":null,\"bar\":false}";

  mark_point();
  json = pr_json_array_from_text(p, text);
  ck_assert_msg(json == NULL, "Failed to handle non-array text '%s'", text);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  text = "[\"foo\",\"bar\"]";

  mark_point();
  json = pr_json_array_from_text(p, text);
  ck_assert_msg(json != NULL, "Failed to handle text '%s': %s", text,
    strerror(errno));
  (void) pr_json_array_free(json);

  mark_point();
  text = "[[[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]]]";
  json = pr_json_array_from_text(p, text);
  ck_assert_msg(json == NULL, "Failed to handle depth-exceeding text '%s'", text);
}
END_TEST

START_TEST(json_array_to_text_test) {
  const char *text, *expected;
  pr_json_array_t *json;

  mark_point();
  text = pr_json_array_to_text(NULL, NULL, NULL);
  ck_assert_msg(text == NULL, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  text = pr_json_array_to_text(p, NULL, NULL);
  ck_assert_msg(text == NULL, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_array_alloc(p);

  mark_point();
  text = pr_json_array_to_text(p, json, NULL);
  ck_assert_msg(text == NULL, "Failed to handle null indent");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  expected = "[]";

  mark_point();
  text = pr_json_array_to_text(p, json, "");
  ck_assert_msg(text != NULL, "Failed to get text for array: %s",
    strerror(errno));
  ck_assert_msg(strcmp(text, expected) == 0, "Expected '%s', got '%s'", expected,
    text);

  (void) pr_json_array_free(json);
}
END_TEST

static int array_item_ok(int val_type, const void *val, size_t valsz,
    void *user_data) {
  return 0;
}

static int array_item_fail(int val_type, const void *val, size_t valsz,
    void *user_data) {
  errno = EPERM;
  return -1;
}

START_TEST(json_array_foreach_test) {
  int res;
  pr_json_array_t *json = NULL;
  const char *text;

  mark_point();
  res = pr_json_array_foreach(NULL, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_json_array_foreach(p, NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null array");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_foreach(p, json, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null callback");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  (void) pr_json_array_free(json);

  text = "[\"foo\",true,\"bar\",false,\"baz\",1,{}]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_foreach(p, json, array_item_ok, NULL);
  ck_assert_msg(res == 0, "Failed to iterate over array: %s", strerror(errno));

  mark_point();
  res = pr_json_array_foreach(p, json, array_item_fail, NULL);
  ck_assert_msg(res < 0, "Failing callback succeeded unexpectedly");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_count_test) {
  int res;
  pr_json_array_t *json;
  const char *text;

  mark_point();
  res = pr_json_array_count(NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_count(json);
  ck_assert_msg(res == 0, "Expected 0, got %d", res);

  (void) pr_json_array_free(json);

  text = "[\"foo\",true,\"bar\",false,\"baz\",1]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_count(json);
  ck_assert_msg(res == 6, "Expected 6, got %d", res);

  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_exists_test) {
  int res;
  pr_json_array_t *json;
  unsigned int idx;
  const char *text;

  mark_point();
  res = pr_json_array_exists(NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_exists(json, 0);
  ck_assert_msg(res == FALSE, "Expected FALSE, got %d", res);

  (void) pr_json_array_free(json);

  text = "[\"foo\",true,\"bar\",false,\"baz\",1]";
  json = pr_json_array_from_text(p, text);

  idx = 3;

  mark_point();
  res = pr_json_array_exists(json, idx);
  ck_assert_msg(res == TRUE, "Expected TRUE, got %d", res);

  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_remove_test) {
  int res;
  pr_json_array_t *json;
  unsigned int idx;
  const char *text;

  mark_point();
  res = pr_json_array_remove(NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  json = pr_json_array_alloc(p);

  idx = 2;

  mark_point();
  res = pr_json_array_remove(json, idx);
  ck_assert_msg(res == 0, "Failed to remove nonexistent index %u: %s", idx,
    strerror(errno));

  res = pr_json_array_count(json);
  ck_assert_msg(res == 0, "Expected count 0, got %d", res);

  (void) pr_json_array_free(json);

  text = "[\"foo\",true,\"bar\",false,\"baz\",1]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_remove(json, idx);
  ck_assert_msg(res == 0, "Failed to remove existing index %u: %s", idx,
    strerror(errno));
  
  res = pr_json_array_count(json);
  ck_assert_msg(res == 5, "Expected count 5, got %d", res);

  mark_point();
  res = pr_json_array_exists(json, idx);
  ck_assert_msg(res == TRUE, "Expected TRUE, got %d", res);

  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_get_bool_test) {
  int res, val;
  unsigned int idx;
  const char *text;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_get_bool(NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_get_bool(p, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_get_bool(p, json, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  idx = 0;
 
  mark_point();
  res = pr_json_array_get_bool(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent index %u", idx);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_array_free(json);

  text = "[\"foo\",2,\"bar\",true]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_get_bool(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle non-boolean index %u", idx);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  idx = 3;

  mark_point();
  res = pr_json_array_get_bool(p, json, idx, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(val == TRUE, "Expected TRUE, got %d", val);
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_append_bool_test) {
  int res, val = TRUE;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_append_bool(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_append_bool(p, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_append_bool(p, json, val);
  ck_assert_msg(res == 0, "Failed to append val %d: %s", val, strerror(errno));
 
  val = FALSE;
 
  mark_point();
  res = pr_json_array_get_bool(p, json, 0, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index 0: %s",
    strerror(errno));
  ck_assert_msg(val == TRUE, "Expected TRUE, got %d", val);
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_get_null_test) {
  int res;
  unsigned int idx;
  const char *text;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_get_null(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_get_null(p, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  idx = 1;
 
  mark_point();
  res = pr_json_array_get_null(p, json, idx);
  ck_assert_msg(res < 0, "Failed to handle nonexistent index %u", idx);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_array_free(json);

  text = "[\"foo\",2,\"bar\",null]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_get_null(p, json, idx);
  ck_assert_msg(res < 0, "Failed to handle non-null index %u", idx);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  idx = 3;

  mark_point();
  res = pr_json_array_get_null(p, json, idx);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_append_null_test) {
  int res;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_append_null(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_append_null(p, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_append_null(p, json);
  ck_assert_msg(res == 0, "Failed to append null vall: %s", strerror(errno));
 
  mark_point();
  res = pr_json_array_get_null(p, json, 0);
  ck_assert_msg(res == 0, "Failed to handle existing index 0: %s",
    strerror(errno));
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_get_number_test) {
  int res;
  double val;
  unsigned int idx;
  const char *text;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_get_number(NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_get_number(p, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_get_number(p, json, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  idx = 3;
 
  mark_point();
  res = pr_json_array_get_number(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent index %u", idx);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_array_free(json);

  text = "[\"foo\",2,\"bar\",true]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_get_number(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle non-number index %u", idx);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  idx = 1;

  mark_point();
  res = pr_json_array_get_number(p, json, idx, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(fabs(val) == fabs((double) 2.0), "Expected 2, got '%e'", val);
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_append_number_test) {
  int res;
  double val = 7;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_append_number(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_append_number(p, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_append_number(p, json, val);
  ck_assert_msg(res == 0, "Failed to append val %e: %s", val, strerror(errno));
 
  val = 2;
 
  mark_point();
  res = pr_json_array_get_number(p, json, 0, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index 0: %s",
    strerror(errno));
  ck_assert_msg(fabs(val) == fabs((double) 7.0), "Expected 7, got %e", val);
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_get_string_test) {
  int res;
  unsigned int idx;
  const char *text, *val;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_get_string(NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_get_string(p, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_get_string(p, json, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  idx = 3;
 
  mark_point();
  res = pr_json_array_get_string(p, json, idx, (char **) &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent index %u", idx);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_array_free(json);

  text = "[\"foo\",2,\"bar\",true]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_get_string(p, json, idx, (char **) &val);
  ck_assert_msg(res < 0, "Failed to handle non-string index %u", idx);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  idx = 0;

  mark_point();
  res = pr_json_array_get_string(p, json, idx, (char **) &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(strcmp(val, "foo") == 0, "Expected 'foo', got '%s'", val);
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_append_string_test) {
  int res;
  const char *val;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_append_string(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_append_string(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_append_string(p, json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
 
  val = "foo!";
 
  mark_point();
  res = pr_json_array_append_string(p, json, val);
  ck_assert_msg(res == 0, "Failed to append val '%s': %s", val, strerror(errno));
 
  val = NULL;
 
  mark_point();
  res = pr_json_array_get_string(p, json, 0, (char **) &val);
  ck_assert_msg(res == 0, "Failed to handle existing index 0: %s",
    strerror(errno));
  ck_assert_msg(strcmp(val, "foo!") == 0, "Expected 'foo!', got '%s'", val);
 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_get_array_test) {
  int res;
  unsigned int idx;
  const char *text;
  char *expected = NULL, *str = NULL;
  pr_json_array_t *json, *val = NULL;

  mark_point();
  res = pr_json_array_get_array(NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_get_array(p, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_get_array(p, json, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  idx = 0;
 
  mark_point();
  res = pr_json_array_get_array(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent index %u", idx);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_array_free(json);

  text = "[\"foo\",false,\"bar\",[\"baz\"]]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_get_array(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle non-array index %u", idx);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  idx = 3;
  val = NULL;
 
  mark_point();
  res = pr_json_array_get_array(p, json, idx, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected array, got null");

  expected = "[\"baz\"]";
  str = pr_json_array_to_text(p, val, "");
  ck_assert_msg(strcmp(str, expected) == 0,
    "Expected '%s', got '%s'", expected, str);

  mark_point();
  (void) pr_json_array_free(val);

  mark_point();
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_append_array_test) {
  int res;
  unsigned int idx;
  const char *text;
  pr_json_array_t *json, *val = NULL;

  mark_point();
  res = pr_json_array_append_array(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_append_array(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_append_array(p, json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "[1, 1, 2, 3, 5, 8]";
  val = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_append_array(p, json, val);
  ck_assert_msg(res == 0, "Failed to append array: %s", strerror(errno));

  val = NULL;
  idx = 0;
 
  mark_point();
  res = pr_json_array_get_array(p, json, idx, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected array, got null");

  mark_point();
  (void) pr_json_array_free(val); 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_get_object_test) {
  int res;
  unsigned int idx;
  const char *text;
  char *expected = NULL, *str = NULL;
  pr_json_object_t *val = NULL;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_get_object(NULL, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_get_object(p, NULL, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_get_object(p, json, 0, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  idx = 0;
 
  mark_point();
  res = pr_json_array_get_object(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle nonexistent index %u", idx);
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);
 
  (void) pr_json_array_free(json);

  text = "[\"foo\",false,\"bar\",{}]";
  json = pr_json_array_from_text(p, text);

  mark_point();
  res = pr_json_array_get_object(p, json, idx, &val);
  ck_assert_msg(res < 0, "Failed to handle non-object index %u", idx);
  ck_assert_msg(errno == EEXIST, "Expected EEXIST (%d), got %s (%d)", EEXIST,
    strerror(errno), errno);

  idx = 3;
  val = NULL;
 
  mark_point();
  res = pr_json_array_get_object(p, json, idx, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected object, got null");

  expected = "{}";
  str = pr_json_object_to_text(p, val, "");
  ck_assert_msg(strcmp(str, expected) == 0,
    "Expected '%s', got '%s'", expected, str);

  mark_point();
  (void) pr_json_object_free(val);

  mark_point();
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_array_append_object_test) {
  int res;
  unsigned int idx;
  const char *text;
  pr_json_object_t *val = NULL;
  pr_json_array_t *json;

  mark_point();
  res = pr_json_array_append_object(NULL, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  mark_point();
  res = pr_json_array_append_object(p, NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null json");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
  
  json = pr_json_array_alloc(p);

  mark_point();
  res = pr_json_array_append_object(p, json, NULL);
  ck_assert_msg(res < 0, "Failed to handle null val");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "{\"foo\":1,\"bar\":2}";
  val = pr_json_object_from_text(p, text);

  mark_point();
  res = pr_json_array_append_object(p, json, val);
  ck_assert_msg(res == 0, "Failed to append object: %s", strerror(errno));

  val = NULL;
  idx = 0;

  mark_point();
  res = pr_json_array_get_object(p, json, idx, &val);
  ck_assert_msg(res == 0, "Failed to handle existing index %u: %s", idx,
    strerror(errno));
  ck_assert_msg(val != NULL, "Expected object, got null");

  mark_point();
  (void) pr_json_object_free(val); 
  (void) pr_json_array_free(json);
}
END_TEST

START_TEST(json_text_validate_test) {
  int res;
  const char *text;

  mark_point();
  res = pr_json_text_validate(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null pool");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_json_text_validate(p, NULL);
  ck_assert_msg(res < 0, "Failed to handle null text");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  text = "foo bar";

  mark_point();
  res = pr_json_text_validate(p, text);
  ck_assert_msg(res == FALSE, "Failed to handle invalid text '%s'", text);

  text = "[{}]";

  mark_point();
  res = pr_json_text_validate(p, text);
  ck_assert_msg(res == TRUE, "Failed to handle valid text '%s'", text);
}
END_TEST

START_TEST(json_type_name_test) {
  const char *res, *expected;

  res = pr_json_type_name(0);
  ck_assert_msg(res == NULL, "Failed to handle invalid JSON type ID 0");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  expected = "boolean";
  res = pr_json_type_name(PR_JSON_TYPE_BOOL);
  ck_assert_msg(res != NULL, "Failed to handle JSON_TYPE_BOOL: %s",
    strerror(errno));
  ck_assert_msg(strcmp(res, expected) == 0, "Expected '%s', got '%s'", expected,
    res);

  expected = "number";
  res = pr_json_type_name(PR_JSON_TYPE_NUMBER);
  ck_assert_msg(res != NULL, "Failed to handle JSON_TYPE_NUMBER: %s",
    strerror(errno));
  ck_assert_msg(strcmp(res, expected) == 0, "Expected '%s', got '%s'", expected,
    res);

  expected = "null";
  res = pr_json_type_name(PR_JSON_TYPE_NULL);
  ck_assert_msg(res != NULL, "Failed to handle JSON_TYPE_NULL: %s",
    strerror(errno));
  ck_assert_msg(strcmp(res, expected) == 0, "Expected '%s', got '%s'", expected,
    res);

  expected = "string";
  res = pr_json_type_name(PR_JSON_TYPE_STRING);
  ck_assert_msg(res != NULL, "Failed to handle JSON_TYPE_STRING: %s",
    strerror(errno));
  ck_assert_msg(strcmp(res, expected) == 0, "Expected '%s', got '%s'", expected,
    res);

  expected = "array";
  res = pr_json_type_name(PR_JSON_TYPE_ARRAY);
  ck_assert_msg(res != NULL, "Failed to handle JSON_TYPE_ARRAY: %s",
    strerror(errno));
  ck_assert_msg(strcmp(res, expected) == 0, "Expected '%s', got '%s'", expected,
    res);

  expected = "object";
  res = pr_json_type_name(PR_JSON_TYPE_OBJECT);
  ck_assert_msg(res != NULL, "Failed to handle JSON_TYPE_OBJECT: %s",
    strerror(errno));
  ck_assert_msg(strcmp(res, expected) == 0, "Expected '%s', got '%s'", expected,
    res);
}
END_TEST

Suite *tests_get_json_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("json");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, json_object_free_test);
  tcase_add_test(testcase, json_object_alloc_test);
  tcase_add_test(testcase, json_object_from_text_test);
  tcase_add_test(testcase, json_object_to_text_test);
  tcase_add_test(testcase, json_object_foreach_test);
  tcase_add_test(testcase, json_object_count_test);
  tcase_add_test(testcase, json_object_exists_test);
  tcase_add_test(testcase, json_object_remove_test);
  tcase_add_test(testcase, json_object_get_bool_test);
  tcase_add_test(testcase, json_object_set_bool_test);
  tcase_add_test(testcase, json_object_get_null_test);
  tcase_add_test(testcase, json_object_set_null_test);
  tcase_add_test(testcase, json_object_get_number_test);
  tcase_add_test(testcase, json_object_set_number_test);
  tcase_add_test(testcase, json_object_get_string_test);
  tcase_add_test(testcase, json_object_set_string_test);
  tcase_add_test(testcase, json_object_get_array_test);
  tcase_add_test(testcase, json_object_set_array_test);
  tcase_add_test(testcase, json_object_get_object_test);
  tcase_add_test(testcase, json_object_set_object_test);

  tcase_add_test(testcase, json_array_free_test);
  tcase_add_test(testcase, json_array_alloc_test);
  tcase_add_test(testcase, json_array_from_text_test);
  tcase_add_test(testcase, json_array_to_text_test);
  tcase_add_test(testcase, json_array_foreach_test);
  tcase_add_test(testcase, json_array_count_test);
  tcase_add_test(testcase, json_array_exists_test);
  tcase_add_test(testcase, json_array_remove_test);
  tcase_add_test(testcase, json_array_get_bool_test);
  tcase_add_test(testcase, json_array_append_bool_test);
  tcase_add_test(testcase, json_array_get_null_test);
  tcase_add_test(testcase, json_array_append_null_test);
  tcase_add_test(testcase, json_array_get_number_test);
  tcase_add_test(testcase, json_array_append_number_test);
  tcase_add_test(testcase, json_array_get_string_test);
  tcase_add_test(testcase, json_array_append_string_test);
  tcase_add_test(testcase, json_array_get_array_test);
  tcase_add_test(testcase, json_array_append_array_test);
  tcase_add_test(testcase, json_array_get_object_test);
  tcase_add_test(testcase, json_array_append_object_test);

  tcase_add_test(testcase, json_text_validate_test);
  tcase_add_test(testcase, json_type_name_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
