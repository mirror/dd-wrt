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

/* Table API tests */

#include "tests.h"

static pool *p = NULL;

/* Fixtures */

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

/* Helper functions */

static unsigned int b_val_count = 0;

static int do_cb(const void *key, size_t keysz, const void *value,
    size_t valuesz, void *user_data) {

  if (*((const char *) value) == 'b') {
    b_val_count++;
  }

  return -1;
}

static int do_with_remove_cb(const void *key, size_t keysz, const void *value,
    size_t valuesz, void *user_data) {
  pr_table_t *tab;
 
  tab = user_data;

  if (*((const char *) value) == 'b') {
    b_val_count++;
  }

  pr_table_kremove(tab, key, keysz, NULL);
  return 0;
}

static void table_dump(const char *fmt, ...) {
}

/* Tests */

START_TEST (table_alloc_test) {
  pr_table_t *tab;

  tab = pr_table_alloc(NULL, 0);
  ck_assert_msg(tab == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);
  ck_assert_msg(tab != NULL, "Failed to allocate table: %s", strerror(errno));
}
END_TEST

START_TEST (table_nalloc_test) {
  pr_table_t *tab;

  tab = pr_table_nalloc(NULL, 0, 0);
  ck_assert_msg(tab == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_nalloc(p, 0, 0);
  ck_assert_msg(tab == NULL, "Failed to handle zero chains");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_nalloc(p, 0, 1);
  ck_assert_msg(tab != NULL, "Failed to allocate table: %s", strerror(errno));
}
END_TEST

START_TEST (table_add_test) {
  int res;
  pr_table_t *tab;

  res = pr_table_add(NULL, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);
  ck_assert_msg(tab != NULL, "Failed to allocate table: %s", strerror(errno));

  res = pr_table_add(tab, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_add(tab, "", NULL, 0);
  ck_assert_msg(res == 0, "Failed to add null value (len 0) for empty key");

  res = pr_table_add(tab, "", NULL, 1);
  ck_assert_msg(res == -1, "Failed to handle null value (len 1) for empty key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_add(tab, "", NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle duplicate (empty) key");
  ck_assert_msg(errno == EEXIST, "Failed to set errno to EEXIST");
}
END_TEST

START_TEST (table_add_dup_test) {
  int res;
  pr_table_t *tab;

  res = pr_table_add_dup(NULL, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);
  ck_assert_msg(tab != NULL, "Failed to allocate table: %s", strerror(errno));

  res = pr_table_add_dup(tab, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_add_dup(tab, "", NULL, 0);
  ck_assert_msg(res == 0, "Failed to add null value (len 0) for empty key");

  res = pr_table_add_dup(tab, "", NULL, 1);
  ck_assert_msg(res == -1, "Failed to handle null value (len 1) for empty key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_add_dup(tab, "", NULL, 0);
  ck_assert_msg(res == 0, "Failed to handle empty value: %s", strerror(errno));

  mark_point();
  res = pr_table_add_dup(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo': %s", strerror(errno));
}
END_TEST

START_TEST (table_count_test) {
  int res, ok;
  pr_table_t *tab;

  res = pr_table_count(NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  ok = 0;
  res = pr_table_count(tab);
  ck_assert_msg(res == ok, "Expected count %d, got %d", ok, res);

  res = pr_table_add(tab, "foo", NULL, 0);
  ck_assert_msg(res == 0, "Failed to add item to table: %s", strerror(errno));

  ok = 1;
  res = pr_table_count(tab);
  ck_assert_msg(res == ok, "Expected count %d, got %d", ok, res);

  res = pr_table_add(tab, "bar", NULL, 0);
  ck_assert_msg(res == 0, "Failed to add item to table: %s", strerror(errno));

  ok = 2;
  res = pr_table_count(tab);
  ck_assert_msg(res == ok, "Expected count %d, got %d", ok, res);
}
END_TEST

START_TEST (table_exists_test) {
  int res, ok;
  pr_table_t *tab;

  res = pr_table_exists(NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, PR_TABLE_FL_MULTI_VALUE);
  ck_assert_msg(tab != NULL, "Failed to allocate table: %s", strerror(errno));

  res = pr_table_exists(tab, NULL);
  ck_assert_msg(res == -1, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_exists(NULL, "foo");
  ck_assert_msg(res == -1, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  ok = -1;
  res = pr_table_exists(tab, "foo");
  ck_assert_msg(res == ok, "Expected value count %d, got %d", ok, res);
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");

  res = pr_table_add(tab, "foo", "a", 0);
  ck_assert_msg(res == 0, "Failed to add key to table: %s", strerror(errno));

  ok = 1;
  res = pr_table_exists(tab, "foo");
  ck_assert_msg(res == ok, "Expected value count %d, got %d", ok, res);

  res = pr_table_add(tab, "foo", "b", 0);
  ck_assert_msg(res == 0, "Failed to add key to table: %s", strerror(errno));

  ok = 2;
  res = pr_table_exists(tab, "foo");
  ck_assert_msg(res == ok, "Expected value count %d, got %d", ok, res);

  mark_point();
  res = pr_table_kexists(NULL, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_table_kexists(tab, NULL, 0);
  ck_assert_msg(res < 0, "Failed to handle null key_data");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (table_empty_test) {
  int res;
  pr_table_t *tab;

  res = pr_table_empty(NULL);
  ck_assert_msg(res == -1, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_empty(tab);
  ck_assert_msg(res == 0, "Failed to empty table: %s", strerror(errno));

  res = pr_table_add(tab, "foo", NULL, 0);
  ck_assert_msg(res == 0, "Failed to add key to table: %s", strerror(errno));

  res = pr_table_count(tab);
  ck_assert_msg(res == 1, "Expected table item count of 1, got %d", res);

  res = pr_table_empty(tab);
  ck_assert_msg(res == 0, "Failed to empty table: %s", strerror(errno));

  res = pr_table_count(tab);
  ck_assert_msg(res == 0, "Expected table item count of 0, got %d", res);
}
END_TEST

START_TEST (table_free_test) {
  int res;
  pr_table_t *tab;

  res = pr_table_free(NULL);
  ck_assert_msg(res == -1, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_free(tab);
  ck_assert_msg(res == 0, "Failed to free table: %s", strerror(errno));

  tab = pr_table_alloc(p, 0);
  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add item to table: %s", strerror(errno));

  res = pr_table_free(tab);
  ck_assert_msg(res == -1, "Failed to handle non-empty table");
  ck_assert_msg(errno == EPERM, "Failed to set errno to EPERM");
}
END_TEST

START_TEST (table_get_test) {
  int ok, xerrno;
  const void *res;
  pr_table_t *tab;
  char *str;
  size_t sz;

  res = pr_table_get(NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_get(tab, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null key");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");

  ok = pr_table_add(tab, "foo", NULL, 0);
  ck_assert_msg(ok == 0, "Failed to add null value to table: %s",
    strerror(errno));

  errno = xerrno = 0;
  res = pr_table_get(tab, "foo", &sz);
  xerrno = errno;

  ck_assert_msg(res == NULL, "Failed to lookup null value: %s", strerror(errno));
  ck_assert_msg(xerrno == 0, "Expected errno 0, got %d (%s)", xerrno,
    strerror(xerrno));

  ok = pr_table_add(tab, "bar", "baz", 0);
  ck_assert_msg(ok == 0, "Failed to add 'bar' to table: %s", strerror(errno));

  res = pr_table_get(tab, "bar", &sz);
  ck_assert_msg(res != NULL, "Failed to lookup value for 'bar': %s",
    strerror(errno));
  ck_assert_msg(sz == 4, "Expected result len of 4, got %lu", (unsigned long)sz);

  str = pcalloc(p, sz);
  memcpy(str, res, sz);

  ck_assert_msg(strcmp(str, "baz") == 0,
    "Expected value '%s', got '%s'", "baz", str);

  mark_point();
  res = pr_table_kget(NULL, NULL, 0, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

static unsigned int cache_key_hash(const void *key, size_t keysz) {
  return 1;
}

START_TEST (table_get_use_cache_test) {
  int ok, xerrno;
  const void *res;
  pr_table_t *tab;
  const char *key = "bar";
  char *str;
  size_t sz;

  res = pr_table_get(NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, PR_TABLE_FL_USE_CACHE);

  /* We use this specific key hash function to ensure that all of the keys we
   * add to this table end up in the same linked-list chain.
   */

  ok = pr_table_ctl(tab, PR_TABLE_CTL_SET_KEY_HASH, cache_key_hash);
  ck_assert_msg(ok == 0, "Failed to set key hash function for table: %s",
    strerror(errno));

  res = pr_table_get(tab, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null key");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");

  ok = pr_table_add(tab, key, "baz", 0);
  ck_assert_msg(ok == 0, "Failed to add 'bar' to table: %s", strerror(errno));

  res = pr_table_get(tab, key, &sz);
  ck_assert_msg(res != NULL, "Failed to lookup value for 'bar': %s",
    strerror(errno));
  ck_assert_msg(sz == 4, "Expected result len of 4, got %lu", (unsigned long)sz);

  str = pcalloc(p, sz);
  memcpy(str, res, sz);

  ck_assert_msg(strcmp(str, "baz") == 0,
    "Expected value '%s', got '%s'", "baz", str);

  /* With the USE_CACHE flag on, we should still receive NULL here. */
  errno = xerrno = 0;
  res = pr_table_get(tab, key, &sz);
  ck_assert_msg(res == NULL, "Failed to return null next value: %s",
    strerror(errno));
  ck_assert_msg(xerrno == 0, "Expected errno 0, got %d (%s)", xerrno,
    strerror(xerrno));

}
END_TEST

START_TEST (table_next_test) {
  int ok;
  const char *res;
  size_t sz = 0;
  pr_table_t *tab;

  res = pr_table_next(NULL);
  ck_assert_msg(res == NULL, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_next(tab);
  ck_assert_msg(res == NULL, "Failed to handle empty table");
  ck_assert_msg(errno == EPERM, "Failed to set errno to EPERM");

  ok = pr_table_add(tab, "foo", NULL, 0);
  ck_assert_msg(ok == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  res = pr_table_next(tab);
  ck_assert_msg(res != NULL, "Failed to get next key: %s", strerror(errno));
  ck_assert_msg(strcmp(res, "foo") == 0,
    "Expected key '%s', got '%s'", "foo", res);

  res = pr_table_next(tab);
  ck_assert_msg(res == NULL, "Expected no more keys, got '%s'", res);

  pr_table_rewind(tab);

  res = pr_table_knext(tab, &sz);
  ck_assert_msg(res != NULL, "Failed to get next key: %s", strerror(errno));
  ck_assert_msg(sz == 4, "Expected 4, got %lu", (unsigned long) sz);
  ck_assert_msg(strcmp(res, "foo") == 0,
    "Expected key '%s', got '%s'", "foo", res);

  sz = 0;
  res = pr_table_knext(tab, &sz);
  ck_assert_msg(res == NULL, "Expected no more keys, got '%s'", res);
}
END_TEST

START_TEST (table_rewind_test) {
  int res;
  const char *key;
  pr_table_t *tab;

  res = pr_table_rewind(NULL);
  ck_assert_msg(res == -1, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_rewind(tab);
  ck_assert_msg(res == 0, "Failed to handle empty table");

  res = pr_table_add(tab, "foo", NULL, 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  key = pr_table_next(tab);
  ck_assert_msg(key != NULL, "Failed to get next key: %s", strerror(errno));
  ck_assert_msg(strcmp(key, "foo") == 0,
    "Expected key '%s', got '%s'", "foo", key);

  key = pr_table_next(tab);
  ck_assert_msg(key == NULL, "Expected no more keys, got '%s'", key);

  res = pr_table_rewind(tab);
  ck_assert_msg(res == 0, "Failed to rewind table: %s", strerror(errno));

  key = pr_table_next(tab);
  ck_assert_msg(key != NULL, "Failed to get next key: %s", strerror(errno));
  ck_assert_msg(strcmp(key, "foo") == 0,
    "Expected key '%s', got '%s'", "foo", key);

  key = pr_table_next(tab);
  ck_assert_msg(key == NULL, "Expected no more keys, got '%s'", key);
}
END_TEST

START_TEST (table_remove_test) {
  int ok;
  const char *res;
  char *str;
  pr_table_t *tab;
  size_t sz;

  res = pr_table_remove(NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_remove(tab, NULL, 0);
  ck_assert_msg(res == NULL, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_remove(tab, "foo", &sz);
  ck_assert_msg(res == NULL, "Failed to handle absent value");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");

  ok = pr_table_add(tab, "foo", "bar baz", 0);
  ck_assert_msg(ok == 0, "Failed to add key to table: %s", strerror(errno));

  res = pr_table_remove(tab, "foo", &sz);
  ck_assert_msg(res != NULL, "Failed to remove 'foo': %s", strerror(errno));
  ck_assert_msg(sz == 8, "Expected value len of 8, got %lu", (unsigned long)sz);

  str = pcalloc(p, sz);
  memcpy(str, res, sz);

  ck_assert_msg(strcmp(str, "bar baz") == 0,
    "Expected value of '%s', got '%s'", "bar baz", str);

  ok = pr_table_count(tab);
  ck_assert_msg(ok == 0, "Expected table count of 0, got %d", ok);

  res = pr_table_remove(tab, "foo", &sz);
  ck_assert_msg(res == NULL, "Failed to handle absent value");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");

  mark_point();
  res = pr_table_kremove(NULL, NULL, 0, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);

  mark_point();
  res = pr_table_kremove(tab, NULL, 0, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null key data");
  ck_assert_msg(errno == EINVAL, "Expected EINVAL (%d), got %s (%d)", EINVAL,
    strerror(errno), errno);
}
END_TEST

START_TEST (table_set_test) {
  int res;
  pr_table_t *tab;
  const void *v;
  char *str;
  size_t sz;

  res = pr_table_set(NULL, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_set(tab, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null key");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_set(tab, "foo", NULL, 1);
  ck_assert_msg(res == -1, "Failed to handle null value (len 1)");
  ck_assert_msg(errno == EINVAL, "Failed to handle null value (len 1)");

  mark_point();
  res = pr_table_set(tab, "foo", "bar", 1);
  ck_assert_msg(res < 0, "Failed to handle empty table");
  ck_assert_msg(errno == ENOENT, "Expected ENOENT (%d), got %s (%d)", ENOENT,
    strerror(errno), errno);

  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  res = pr_table_set(tab, "foo", "BAZ", 0);
  ck_assert_msg(res == 0, "Failed to set 'foo' in table: %s", strerror(errno));

  v = pr_table_get(tab, "foo", &sz);
  ck_assert_msg(v != NULL, "Failed to retrieve 'foo' from table: %s",
    strerror(errno));
  ck_assert_msg(sz == 4, "Expected len 4, got %lu", (unsigned long)sz);

  str = pcalloc(p, sz);
  memcpy(str, v, sz);

  ck_assert_msg(strcmp(str, "BAZ") == 0,
    "Expected value of '%s', got '%s'", "BAZ", str);
}
END_TEST

START_TEST (table_do_test) {
  int res;
  pr_table_t *tab;

  res = pr_table_do(NULL, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_do(tab, NULL, NULL, 0);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_do(tab, do_cb, NULL, 0);
  ck_assert_msg(res == 0, "Failed to handle empty table");

  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  res = pr_table_add(tab, "bar", "baz", 0);
  ck_assert_msg(res == 0, "Failed to add 'bar' to table: %s", strerror(errno));

  res = pr_table_do(tab, do_cb, NULL, 0);
  ck_assert_msg(res == -1, "Expected res %d, got %d", -1, res);
  ck_assert_msg(errno == EPERM, "Failed to set errno to EPERM");
  ck_assert_msg(b_val_count == 1, "Expected count %u, got %u", 1, b_val_count);

  b_val_count = 0;
  res = pr_table_do(tab, do_cb, NULL, PR_TABLE_DO_FL_ALL);
  ck_assert_msg(res == 0, "Failed to do table: %s", strerror(errno));
  ck_assert_msg(b_val_count == 2, "Expected count %u, got %u", 2, b_val_count);
}
END_TEST

START_TEST (table_do_with_remove_test) {
  int res;
  pr_table_t *tab;

  tab = pr_table_alloc(p, 0);

  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  res = pr_table_add(tab, "bar", "baz", 0);
  ck_assert_msg(res == 0, "Failed to add 'bar' to table: %s", strerror(errno));

  b_val_count = 0;
  res = pr_table_do(tab, do_with_remove_cb, tab, PR_TABLE_DO_FL_ALL);
  ck_assert_msg(res == 0, "Failed to do table: %s", strerror(errno));
  ck_assert_msg(b_val_count == 2, "Expected count %u, got %u", 2, b_val_count);
}
END_TEST

START_TEST (table_ctl_test) {
  int res;
  pr_table_t *tab;
  unsigned long flags = 0;
  unsigned int max_ents = 0, nchains = 0;

  res = pr_table_ctl(NULL, 0, NULL);
  ck_assert_msg(res == -1, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);
 
  mark_point();
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_ENT_INSERT, NULL);
  ck_assert_msg(res == 0, "Failed to set entry insert callback: %s",
    strerror(errno));

  mark_point();
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_ENT_REMOVE, NULL);
  ck_assert_msg(res == 0, "Failed to set entry removal callback: %s",
    strerror(errno));

  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  mark_point();
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_MAX_ENTS, 0);
  ck_assert_msg(res < 0, "Failed to handle SET_MAX_ENTS smaller than table");
  ck_assert_msg(errno == EPERM, "Expected EPERM (%d), got %s (%d)", EPERM,
    strerror(errno), errno);

  res = pr_table_ctl(tab, 0, NULL);
  ck_assert_msg(res == -1, "Failed to handle non-empty table");
  ck_assert_msg(errno == EPERM, "Failed to set errno to EPERM");

  res = pr_table_empty(tab);
  ck_assert_msg(res == 0, "Failed to empty table: %s", strerror(errno));

  res = pr_table_ctl(tab, 0, NULL);
  ck_assert_msg(res == -1, "Failed to handle unknown ctl");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_FLAGS, NULL);
  ck_assert_msg(res == -1, "Failed to handle SET_FLAGS, null args");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_FLAGS, &flags);
  ck_assert_msg(res == 0, "Failed to handle SET_FLAGS: %s", strerror(errno));

  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_NCHAINS, NULL);
  ck_assert_msg(res == -1, "Failed to handle SET_NCHAINS, null args");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");
  
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_NCHAINS, &nchains);
  ck_assert_msg(res == -1, "Failed to handle SET_NCHAINS, zero args");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");
 
  nchains = 1; 
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_NCHAINS, &nchains);
  ck_assert_msg(res == 0, "Failed to handle SET_NCHAINS: %s", strerror(errno));

  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_MAX_ENTS, &max_ents);
  ck_assert_msg(res == -1, "Failed to handle SET_MAX_ENTS, zero args");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  /* Add two entries, then try to set MAX_ENTS to one.  We should get an
   * EPERM back for that.
   */
  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  res = pr_table_add(tab, "baz", "quxx", 0);
  ck_assert_msg(res == 0, "Failed to add 'baz' to table: %s", strerror(errno));

  max_ents = 1;
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_MAX_ENTS, &max_ents);
  ck_assert_msg(res == -1, "Failed to handle SET_MAX_ENTS on non-empty table");
  ck_assert_msg(errno == EPERM, "Failed to set errno to EPERM");

  /* Now empty the table, set the MAX_ENTS to one, then try add two entries. */

  res = pr_table_empty(tab);
  ck_assert_msg(res == 0, "Failed to empty table: %s", strerror(errno));

  max_ents = 1;
  res = pr_table_ctl(tab, PR_TABLE_CTL_SET_MAX_ENTS, &max_ents);
  ck_assert_msg(res == 0, "Failed to handle SET_MAX_ENTS to %d: %s",
    max_ents, strerror(errno));

  res = pr_table_add(tab, "foo", "bar", 0);
  ck_assert_msg(res == 0, "Failed to add 'foo' to table: %s", strerror(errno));

  res = pr_table_add(tab, "baz", "quxx", 0);
  ck_assert_msg(res == -1, "Added second entry unexpectedly");
  ck_assert_msg(errno == ENOSPC,
    "Failed to set errno to ENOSPC, received %d (%s)", errno, strerror(errno));
}
END_TEST

START_TEST (table_load_test) {
  pr_table_t *tab = NULL;
  float load;

  load = pr_table_load(tab);
  ck_assert_msg(load < 0, "Failed to handle NULL table argument");
  ck_assert_msg(errno == EINVAL,
    "Failed to set errno to EINVAL; received %d (%s)", errno, strerror(errno));

  tab = pr_table_alloc(p, 0);
  load = pr_table_load(tab);
  ck_assert_msg(load >= 0.0, "Failed to calculate load properly; load = %0.3f",
    load);
}
END_TEST

START_TEST (table_dump_test) {
  pr_table_t *tab;

  pr_table_dump(NULL, NULL);

  tab = pr_table_alloc(p, 0);

  pr_table_dump(NULL, tab);
  pr_table_dump(table_dump, NULL);
}
END_TEST

START_TEST (table_pcalloc_test) {
  void *res;
  pr_table_t *tab;

  res = pr_table_pcalloc(NULL, 0);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  tab = pr_table_alloc(p, 0);

  res = pr_table_pcalloc(tab, 0);
  ck_assert_msg(res == NULL, "Failed to handle zero len argument");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_pcalloc(NULL, 1);
  ck_assert_msg(res == NULL, "Failed to handle null table");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = pr_table_pcalloc(tab, 2);
  ck_assert_msg(res != NULL, "Failed to allocate len 2 from table: %s",
    strerror(errno));
}
END_TEST

Suite *tests_get_table_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("table");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, table_alloc_test);
  tcase_add_test(testcase, table_nalloc_test);
  tcase_add_test(testcase, table_add_test);
  tcase_add_test(testcase, table_add_dup_test);
  tcase_add_test(testcase, table_count_test);
  tcase_add_test(testcase, table_exists_test);
  tcase_add_test(testcase, table_empty_test);
  tcase_add_test(testcase, table_free_test);
  tcase_add_test(testcase, table_get_test);
  tcase_add_test(testcase, table_get_use_cache_test);
  tcase_add_test(testcase, table_next_test);
  tcase_add_test(testcase, table_rewind_test);
  tcase_add_test(testcase, table_remove_test);
  tcase_add_test(testcase, table_set_test);
  tcase_add_test(testcase, table_do_test);
  tcase_add_test(testcase, table_do_with_remove_test);
  tcase_add_test(testcase, table_ctl_test);
  tcase_add_test(testcase, table_load_test);
  tcase_add_test(testcase, table_dump_test);
  tcase_add_test(testcase, table_pcalloc_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
