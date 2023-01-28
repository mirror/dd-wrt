/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2021 The ProFTPD Project team
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

/* Array API tests */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = make_sub_pool(NULL);
  }
}

static void tear_down(void) {
  if (p != NULL) {
    destroy_pool(p);
    p = NULL;
  }
}

START_TEST (make_array_test) {
  array_header *list;

  list = make_array(NULL, 0, 0);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = make_array(p, 0, 0);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = make_array(p, 1, 0);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = make_array(p, 0, 1);
  ck_assert_msg(list != NULL, "Failed to create an array_header: %s",
    strerror(errno));
  ck_assert_msg(list->nalloc == 1, "Expected list->nalloc of %u, got %d",
    1, list->nalloc);

  list = make_array(p, 3, 1);
  ck_assert_msg(list != NULL, "Failed to create an array_header: %s",
    strerror(errno));

  ck_assert_msg(list->pool == p, "List pool doesn't given pool; "
    "expected %p, got %p", p, list->pool);
  ck_assert_msg(list->elts != NULL, "Expected non-null elements pointer");
  ck_assert_msg(list->nalloc == 3, "Expected list->nalloc of %u, got %d",
    3, list->nalloc);
  ck_assert_msg(list->nelts == 0, "Expected list->nelts of %u, got %d",
    0, list->nelts);
  ck_assert_msg(list->elt_size == 1, "Expect list element size of %u, got %lu",
    1, (unsigned long)list->elt_size);
}
END_TEST

START_TEST (push_array_test) {
  array_header *list;
  void *res;

  res = push_array(NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL for null args");

  list = make_array(p, 0, 1);

  res = push_array(list);
  ck_assert_msg(res != NULL, "Failed to allocate new list element");
  ck_assert_msg(list->nalloc == 1, "Incremented alloc elements needlessly ("
    "expected %u, got %d)", 1, list->nalloc);
  ck_assert_msg(list->nelts == 1, "Failed to increment element count "
    "(expected %u, got %d)", 1, list->nelts);

  res = push_array(list);
  ck_assert_msg(res != NULL, "Failed to allocate new list element");
  ck_assert_msg(list->nalloc == 2, "Incremented alloc elements needlessly "
    "(expected %u, got %d)", 2, list->nalloc);
  ck_assert_msg(list->nelts == 2, "Failed to increment element count "
    "(expected %u, got %d)", 2, list->nelts);

  res = push_array(list);
  ck_assert_msg(res != NULL, "Failed to allocate new list element");
  ck_assert_msg(list->nalloc == 4, "Incremented alloc elements needlessly "
    "(expected %u, got %d)", 4, list->nalloc);
  ck_assert_msg(list->nelts == 3, "Failed to increment element count "
    "(expected %u, got %d)", 3, list->nelts);

  res = push_array(list);
  ck_assert_msg(res != NULL, "Failed to allocate new list element");
  ck_assert_msg(list->nalloc == 4, "Incremented alloc elements needlessly "
    "(expected %u, got %d)", 4, list->nalloc);
  ck_assert_msg(list->nelts == 4, "Failed to increment element count "
    "(expected %u, got %d)", 4, list->nelts);
}
END_TEST

START_TEST (array_cat_test) {
  array_header *src, *dst;

  mark_point();

  /* This should not segfault. */
  array_cat(NULL, NULL);

  dst = make_array(p, 0, 1);
  mark_point();
  array_cat(dst, NULL);

  src = make_array(p, 0, 1);
  mark_point();
  array_cat(NULL, src);

  mark_point();
  array_cat(dst, src);

  ck_assert_msg(dst->nalloc == 1, "Wrong dst alloc count (expected %u, got %d)",
    1, dst->nalloc);
  ck_assert_msg(dst->nelts == 0, "Wrong dst item count (expected %u, got %d)",
    0, dst->nelts);

  push_array(src);
  array_cat(dst, src);

  ck_assert_msg(dst->nalloc == 1, "Wrong dst alloc count (expected %u, got %d)",
    1, dst->nalloc);
  ck_assert_msg(dst->nelts == 1, "Wrong dst item count (expected %u, got %d)",
    1, dst->nelts);

  push_array(src);
  push_array(src);
  push_array(src);
  array_cat(dst, src);

  ck_assert_msg(dst->nalloc == 8, "Wrong dst alloc count (expected %u, got %d)",
    8, dst->nalloc);
  ck_assert_msg(dst->nelts == 5, "Wrong dst item count (expected %u, got %d)",
    5, dst->nelts);
}
END_TEST

START_TEST (array_cat2_test) {
  array_header *src, *dst;
  int res;

  mark_point();

  res = array_cat2(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected errno EINVAL, got '%s' (%d)",
    strerror(errno), errno);

  dst = make_array(p, 0, 1);
  mark_point();
  res = array_cat2(dst, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected errno EINVAL, got '%s' (%d)",
    strerror(errno), errno);

  src = make_array(p, 0, 1);
  mark_point();
  res = array_cat2(NULL, src);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Expected errno EINVAL, got '%s' (%d)",
    strerror(errno), errno);

  mark_point();
  res = array_cat2(dst, src);
  ck_assert_msg(res == 0, "Failed to concatenate arrays: %s", strerror(errno));

  ck_assert_msg(dst->nalloc == 1, "Wrong dst alloc count (expected %u, got %d)",
    1, dst->nalloc);
  ck_assert_msg(dst->nelts == 0, "Wrong dst item count (expected %u, got %d)",
    0, dst->nelts);

  push_array(src);
  res = array_cat2(dst, src);
  ck_assert_msg(res == 0, "Failed to concatenate arrays: %s", strerror(errno));

  ck_assert_msg(dst->nalloc == 1, "Wrong dst alloc count (expected %u, got %d)",
    1, dst->nalloc);
  ck_assert_msg(dst->nelts == 1, "Wrong dst item count (expected %u, got %d)",
    1, dst->nelts);

  push_array(src);
  push_array(src);
  push_array(src);
  res = array_cat2(dst, src);
  ck_assert_msg(res == 0, "Failed to concatenate arrays: %s", strerror(errno));

  ck_assert_msg(dst->nalloc == 8, "Wrong dst alloc count (expected %u, got %d)",
    8, dst->nalloc);
  ck_assert_msg(dst->nelts == 5, "Wrong dst item count (expected %u, got %d)",
    5, dst->nelts);
}
END_TEST

START_TEST (clear_array_test) {
  array_header *list;

  mark_point();

  /* This should not segfault. */
  clear_array(NULL);

  list = make_array(p, 0, 1);
  push_array(list);
  push_array(list);

  ck_assert_msg(list->nalloc == 2, "Wrong list alloc count (expected %u, got %d)",
    2, list->nalloc);
  ck_assert_msg(list->nelts == 2, "Wrong list item count (expected %u, got %d)",
    2, list->nelts);

  clear_array(list);

  ck_assert_msg(list->nalloc == 2, "Wrong list alloc count (expected %u, got %d)",
    2, list->nalloc);
  ck_assert_msg(list->nelts == 0, "Wrong list item count (expected %u, got %d)",
    0, list->nelts);
}
END_TEST

START_TEST (copy_array_test) {
  array_header *list, *src;

  list = copy_array(NULL, NULL);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  src = make_array(p, 0, 1);

  list = copy_array(NULL, src);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = copy_array(p, NULL);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  push_array(src);

  list = copy_array(p, src);
  ck_assert_msg(list != NULL, "Failed to copy list");
  ck_assert_msg(list->elt_size == src->elt_size,
    "Copy item size wrong (expected %lu, got %lu)",
    (unsigned long)src->elt_size, (unsigned long)list->elt_size);
  ck_assert_msg(list->nalloc == src->nalloc,
    "Copy nalloc wrong (expected %d, got %d)", src->nalloc, list->nalloc);
  ck_assert_msg(list->nelts == src->nelts,
    "Copy nelts wrong (expected %d, got %d)", src->nelts, list->nelts);
}
END_TEST

START_TEST (copy_array_str_test) {
  array_header *list, *src;
  char *elt, **elts;

  src = make_array(p, 0, sizeof(char *));

  list = copy_array_str(NULL, NULL);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = copy_array_str(NULL, src);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = copy_array_str(p, NULL);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  *((char **) push_array(src)) = pstrdup(p, "foo");
  *((char **) push_array(src)) = pstrdup(p, "bar");

  list = copy_array_str(p, src);

  ck_assert_msg(list->elt_size == src->elt_size,
    "Copy item size wrong (expected %lu, got %lu)",
    (unsigned long)src->elt_size, (unsigned long)list->elt_size);
  ck_assert_msg(list->nalloc == src->nalloc,
    "Copy nalloc wrong (expected %d, got %d)", src->nalloc, list->nalloc);
  ck_assert_msg(list->nelts == src->nelts,
    "Copy nelts wrong (expected %d, got %d)", src->nelts, list->nelts);

  elts = list->elts;

  elt = elts[0];
  ck_assert_msg(strcmp(elt, "foo") == 0,
    "Improper copy (expected '%s', got '%s')", "foo", elt);

  elt = elts[1];
  ck_assert_msg(strcmp(elt, "bar") == 0,
    "Improper copy (expected '%s', got '%s')", "bar", elt);
}
END_TEST

START_TEST (copy_array_hdr_test) {
  array_header *list, *src;
  int elt, *elts;

  src = make_array(p, 0, sizeof(int));

  list = copy_array_hdr(NULL, NULL);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = copy_array_hdr(NULL, src);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  list = copy_array_hdr(p, NULL);
  ck_assert_msg(list == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  *((int *) push_array(src)) = -1;
  *((int *) push_array(src)) = 2;
  *((int *) push_array(src)) = 2476;

  list = copy_array_hdr(p, src);

  ck_assert_msg(list->elt_size == src->elt_size,
    "Copy item size wrong (expected %lu, got %lu)",
    (unsigned long)src->elt_size, (unsigned long)list->elt_size);
  ck_assert_msg(list->elts == src->elts,
    "Copy elts wrong (expected %p, got %p)", src->elts, list->elts);
  ck_assert_msg(list->nelts == src->nelts,
    "Copy nelts wrong (expected %d, got %d)", src->nelts, list->nelts);

  ck_assert_msg(list->nalloc != src->nalloc,
    "Copy nalloc wrong (expected %d, got %d)", src->nalloc, list->nalloc);
  ck_assert_msg(list->nalloc == 3,
    "Copy nalloc wrong (expected %d, got %d)", 3, list->nalloc);

  elts = list->elts;

  elt = elts[0];
  ck_assert_msg(elt == -1, "Improper copy (expected %d, got %d)", -1, elt);

  elt = elts[1];
  ck_assert_msg(elt == 2, "Improper copy (expected %d, got %d)", 2, elt);

  elt = elts[2];
  ck_assert_msg(elt == 2476, "Improper copy (expected %d, got %d)", 2476, elt);
}
END_TEST

START_TEST (append_arrays_test) {
  array_header *a, *b, *res;
  int elt, *elts;

  mark_point();
  a = make_array(p, 0, sizeof(int));
  b = make_array(p, 0, sizeof(int));

  mark_point();
  res = append_arrays(NULL, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  res = append_arrays(p, NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  res = append_arrays(NULL, a, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  res = append_arrays(NULL, NULL, b);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  res = append_arrays(p, a, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  res = append_arrays(p, NULL, b);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  mark_point();
  res = append_arrays(NULL, a, b);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  *((int *) push_array(a)) = -1;
  *((int *) push_array(a)) = 2;
  *((int *) push_array(b)) = 2476;
  *((int *) push_array(b)) = 4762;
  *((int *) push_array(b)) = 7642;

  mark_point();
  res = append_arrays(p, a, b);

  ck_assert_msg(res->elt_size == a->elt_size,
    "Append item size wrong (expected %lu, got %lu)",
    (unsigned long)a->elt_size, (unsigned long)res->elt_size);
  ck_assert_msg(res->nelts == 5,
    "Append nelts wrong (expected %d, got %d)", 5, res->nelts);

  ck_assert_msg(res->nalloc == 8,
    "Append nalloc wrong (expected %d, got %d)", 8, res->nalloc);

  elts = res->elts;

  elt = elts[0];
  ck_assert_msg(elt == -1, "Improper append (expected %d, got %d)", -1, elt);

  elt = elts[1];
  ck_assert_msg(elt == 2, "Improper append (expected %d, got %d)", 2, elt);

  elt = elts[2];
  ck_assert_msg(elt == 2476, "Improper append (expected %d, got %d)", 2476, elt);

  elt = elts[3];
  ck_assert_msg(elt == 4762, "Improper append (expected %d, got %d)", 4762, elt);

  elt = elts[4];
  ck_assert_msg(elt == 7642, "Improper append (expected %d, got %d)", 7642, elt);
}
END_TEST

Suite *tests_get_array_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("array");

  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, make_array_test);
  tcase_add_test(testcase, push_array_test);
  tcase_add_test(testcase, array_cat_test);
  tcase_add_test(testcase, array_cat2_test);
  tcase_add_test(testcase, clear_array_test);
  tcase_add_test(testcase, copy_array_test);
  tcase_add_test(testcase, copy_array_str_test);
  tcase_add_test(testcase, copy_array_hdr_test);
  tcase_add_test(testcase, append_arrays_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
