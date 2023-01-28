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

/* Sets API tests */

#include "tests.h"

static pool *p = NULL;

struct test_item {
  struct test_item *next, *prev;
  int num;
  char *str;
};

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

static int item_cmp(struct test_item *a, struct test_item *b) {

  if (a == NULL &&
      b == NULL) {
    return 0;
  }

  if (a == NULL) {
    return -1;
  }

  if (b == NULL) {
    return 1;
  }

  if (a->num > b->num) {
    return 1;
  }

  if (a->num < b->num) {
    return -1;
  }

  return 0;
}

static struct test_item *item_cpy(struct test_item *a) {
  struct test_item *a_dup;

  a_dup = pcalloc(p, sizeof(struct test_item));
  a_dup->num = a->num;
  a_dup->str = pstrdup(p, a->str);

  return a_dup;
}

/* Tests */

START_TEST (set_create_test) {
  xaset_t *res;

  res = xaset_create(NULL, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EPERM, "Failed to set errno to EPERM");

  res = xaset_create(p, NULL);
  ck_assert_msg(res != NULL, "Expected non-null result");
  ck_assert_msg(res->pool == p, "Expected %p, got %p", p, res->pool);

  permanent_pool = make_sub_pool(p);

  res = xaset_create(NULL, NULL);
  ck_assert_msg(res != NULL, "Expected non-null result");
  ck_assert_msg(res->pool == permanent_pool, "Expected %p, got %p",
    permanent_pool, res->pool);
  ck_assert_msg(res->xas_compare == NULL, "Expected NULL, got %p",
    res->xas_compare);

  res = xaset_create(p, (XASET_COMPARE) item_cmp);
  ck_assert_msg(res != NULL, "Expected non-null result");
  ck_assert_msg(res->pool == p, "Expected %p, got %p", p, res->pool);
  ck_assert_msg(res->xas_compare == (XASET_COMPARE) item_cmp,
    "Expected %p, got %p", item_cmp, res->xas_compare);

  permanent_pool = NULL;
}
END_TEST

START_TEST (set_insert_test) {
  int res;
  xaset_t *set;
  struct test_item *item1, *item2;
  xasetmember_t *member;
 
  res = xaset_insert(NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  set = xaset_create(p, NULL);
  ck_assert_msg(set != NULL, "Failed to create set: %s", strerror(errno));
  ck_assert_msg(set->xas_list == NULL, "New set has non-empty list");

  res = xaset_insert(set, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  item1 = pcalloc(p, sizeof(struct test_item));
  item1->num = 7;
  item1->str = pstrdup(p, "foo");

  res = xaset_insert(NULL, (xasetmember_t *) item1);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = xaset_insert(set, (xasetmember_t *) item1);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));
  ck_assert_msg(set->xas_list != NULL, "Set has empty list");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item1, "Expected %p, got %p", item1,
    member);

  item2 = pcalloc(p, sizeof(struct test_item));
  item2->num = 2;
  item2->str = pstrdup(p, "bar");

  res = xaset_insert(set, (xasetmember_t *) item2);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item2, "Expected %p, got %p", item2,
    member);
  ck_assert_msg(member->next == (xasetmember_t *) item1,
    "Next item in list does not point to item1");
}
END_TEST

START_TEST (set_insert_end_test) {
  int res;
  xaset_t *set;
  struct test_item *item1, *item2;
  xasetmember_t *member;

  mark_point();
  res = xaset_insert_end(NULL, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  set = xaset_create(p, NULL);
  ck_assert_msg(set != NULL, "Failed to create set: %s", strerror(errno));
  ck_assert_msg(set->xas_list == NULL, "New set has non-empty list");

  mark_point();
  res = xaset_insert_end(set, NULL);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  item1 = pcalloc(p, sizeof(struct test_item));
  item1->num = 7;
  item1->str = pstrdup(p, "foo");

  mark_point();
  res = xaset_insert_end(NULL, (xasetmember_t *) item1);
  ck_assert_msg(res < 0, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = xaset_insert_end(set, (xasetmember_t *) item1);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));
  ck_assert_msg(set->xas_list != NULL, "Set has empty list");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item1, "Expected %p, got %p", item1,
    member);

  item2 = pcalloc(p, sizeof(struct test_item));
  item2->num = 2;
  item2->str = pstrdup(p, "bar");

  res = xaset_insert_end(set, (xasetmember_t *) item2);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));

  member = set->xas_list;
  ck_assert_msg(member != (xasetmember_t *) item2, "Expected %p, got %p", item2,
    member);
  ck_assert_msg(member == (xasetmember_t *) item1, "Expected %p, got %p", item1,
    member);
  ck_assert_msg(member->next == (xasetmember_t *) item2,
    "Next item in list does not point to item2");
  ck_assert_msg(item2->prev == item1,
    "Previous item in list does not point to item1");
}
END_TEST

START_TEST (set_insert_sort_test) {
  int res;
  xaset_t *set;
  struct test_item *item1, *item2, *item3;
  xasetmember_t *member;
 
  res = xaset_insert_sort(NULL, NULL, FALSE);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  set = xaset_create(p, NULL);
  ck_assert_msg(set != NULL, "Failed to create set: %s", strerror(errno));
  ck_assert_msg(set->xas_list == NULL, "New set has non-empty list");

  res = xaset_insert_sort(set, NULL, FALSE);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  item1 = pcalloc(p, sizeof(struct test_item));
  item1->num = 7;
  item1->str = pstrdup(p, "foo");

  res = xaset_insert_sort(NULL, (xasetmember_t *) item1, FALSE);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  /* This should fail because we specified a NULL comparator callback. */
  res = xaset_insert_sort(set, (xasetmember_t *) item1, FALSE);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  set = xaset_create(p, (XASET_COMPARE) item_cmp);
  ck_assert_msg(set != NULL, "Failed to create set: %s", strerror(errno));
  ck_assert_msg(set->xas_list == NULL, "New set has non-empty list");

  res = xaset_insert_sort(set, (xasetmember_t *) item1, FALSE);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));
  ck_assert_msg(set->xas_list != NULL, "Set has empty list");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item1, "Expected %p, got %p", item1,
    member);

  /* Now lets try to add another item of the same value, not allowing for dups.
   */
  item2 = pcalloc(p, sizeof(struct test_item));
  item2->num = 7;
  item2->str = pstrdup(p, "bar");

  res = xaset_insert_sort(set, (xasetmember_t *) item2, FALSE);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));
  ck_assert_msg(set->xas_list != NULL, "Set has empty list");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item1, "Expected %p, got %p", item1,
    member);
  ck_assert_msg(member->next == NULL, "Expected only one item on the list");

  /* Add the same item again, this time allowing for dups. */
  res = xaset_insert_sort(set, (xasetmember_t *) item2, TRUE);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));
  ck_assert_msg(set->xas_list != NULL, "Set has empty list");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item2, "Expected %p, got %p", item2,
    member);
  ck_assert_msg(member->next != NULL, "Expected two items on the list");

  /* Add a new item, make sure it sorts properly. */
  item3 = pcalloc(p, sizeof(struct test_item));
  item3->num = 2;
  item3->str = pstrdup(p, "baz");

  res = xaset_insert_sort(set, (xasetmember_t *) item3, FALSE);
  ck_assert_msg(res == 0, "Failed to insert item to set: %s", strerror(errno));

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item3, "Expected %p, got %p", item3,
    member);
  ck_assert_msg(member->next != NULL, "Expected a second item on the list");

  member = member->next;
  ck_assert_msg(member->next != NULL, "Expected a third item on the list");

  member = member->next;
  ck_assert_msg(member->next == NULL, "Expected only three items on the list");
}
END_TEST

START_TEST (set_remove_test) {
  int res;
  xaset_t *set;
  struct test_item *item1, *item2;
  xasetmember_t *member;

  res = xaset_remove(NULL, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  set = xaset_create(p, NULL);
  ck_assert_msg(set != NULL, "Failed to create set: %s", strerror(errno));

  res = xaset_remove(set, NULL);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  item1 = pcalloc(p, sizeof(struct test_item));
  item1->num = 7;
  item1->str = pstrdup(p, "foo");

  res = xaset_remove(NULL, (xasetmember_t *) item1);
  ck_assert_msg(res == -1, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  res = xaset_remove(set, (xasetmember_t *) item1);
  ck_assert_msg(res == -1, "Failed to handle non-included item properly");
  ck_assert_msg(errno == ENOENT, "Failed to set errno to ENOENT");

  res = xaset_insert(set, (xasetmember_t *) item1);
  ck_assert_msg(res == 0, "Failed to insert item");

  res = xaset_remove(set, (xasetmember_t *) item1);
  ck_assert_msg(res == 0, "Failed to remove item");
  ck_assert_msg(set->xas_list == NULL, "Have non-empty list");

  item2 = pcalloc(p, sizeof(struct test_item));
  item2->num = 9;
  item2->str = pstrdup(p, "bar");

  res = xaset_insert(set, (xasetmember_t *) item1);
  ck_assert_msg(res == 0, "Failed to add item1");

  res = xaset_insert(set, (xasetmember_t *) item2);
  ck_assert_msg(res == 0, "Failed to add item2");

  member = (xasetmember_t *) item1;
  ck_assert_msg(member->next == NULL, "Expected member->next to be null");
  ck_assert_msg(member->prev != NULL, "Expected member->prev to not be null");

  member = (xasetmember_t *) item2;
  ck_assert_msg(member->next != NULL, "Expected member->next to not be null");
  ck_assert_msg(member->prev == NULL, "Expected member->prev to be null");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item2,
    "Expected head of list to be item2 (%p), got %p", item2, member);

  res = xaset_remove(set, (xasetmember_t *) item2);
  ck_assert_msg(res == 0, "Failed to remove item2 from set: %s",
    strerror(errno));

  member = (xasetmember_t *) item2;
  ck_assert_msg(member->next == NULL, "Expected member->next to be null");
  ck_assert_msg(member->prev == NULL, "Expected member->prev to be null");

  member = set->xas_list;
  ck_assert_msg(member == (xasetmember_t *) item1,
    "Expected head of list to be item1 (%p), got %p", item1, member);
  
  res = xaset_remove(set, (xasetmember_t *) item1);
  ck_assert_msg(res == 0, "Failed to remove item1 from set: %s",
    strerror(errno));

  member = (xasetmember_t *) item1;
  ck_assert_msg(member->next == NULL, "Expected member->next to be null");
  ck_assert_msg(member->prev == NULL, "Expected member->prev to be null");

  member = set->xas_list;
  ck_assert_msg(member == NULL, "Expected list to be empty, got %p", member);
}
END_TEST

START_TEST (set_copy_test) {
  xaset_t *res, *set;
  struct test_item *item1, *item2;

  res = xaset_copy(NULL, NULL, 0, NULL);
  ck_assert_msg(res == NULL, "Failed to handle null arguments");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  set = xaset_create(p, NULL);
  ck_assert_msg(set != NULL, "Failed to create set: %s", strerror(errno));

  res = xaset_copy(p, set, 0, NULL);
  ck_assert_msg(res == NULL, "Failed to detect zero-size and null copier");
  ck_assert_msg(errno == EINVAL, "Failed to set errno to EINVAL");

  item1 = pcalloc(p, sizeof(struct test_item));
  item1->num = 7;
  item1->str = pstrdup(p, "foo");

  xaset_insert(set, (xasetmember_t *) item1);

  res = xaset_copy(p, set, sizeof(struct test_item), NULL);
  ck_assert_msg(res != NULL, "Failed to copy set: %s", strerror(errno));

  item2 = (struct test_item *) res->xas_list;
  ck_assert_msg(item2->num == item1->num,
    "Expected copied item num of %d, got %d", item1->num, item2->num);
  ck_assert_msg(item2->str == item1->str,
    "Expected copied item str ptr of %p, got %p", item1->str, item2->str);

  /* Of course, we don't want the copied set's items to point to the
   * same memory as the pointers in the first set's items, otherwise
   * it would only be a shallow copy.  So provider a copier callback,
   * and make sure it works.
   */

  res = xaset_copy(p, set, 0, (XASET_MCOPY) item_cpy);
  ck_assert_msg(res != NULL, "Failed to copy set: %s", strerror(errno));

  item2 = (struct test_item *) res->xas_list;
  ck_assert_msg(item2->num == item1->num,
    "Expected copied item num of %d, got %d", item1->num, item2->num);

  ck_assert_msg(item2->str != item1->str,
    "Expected copied item str ptr of %p, got %p", item1->str, item2->str);

  ck_assert_msg(strcmp(item2->str, item1->str) == 0,
    "Expected copied item str of '%s', got '%s'", item1->str, item2->str);
}
END_TEST

Suite *tests_get_sets_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("sets");
  testcase = tcase_create("base");

  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, set_create_test);
  tcase_add_test(testcase, set_insert_test);
  tcase_add_test(testcase, set_insert_end_test);
  tcase_add_test(testcase, set_insert_sort_test);
  tcase_add_test(testcase, set_remove_test);
  tcase_add_test(testcase, set_copy_test);

  suite_add_tcase(suite, testcase);
  return suite;
}
