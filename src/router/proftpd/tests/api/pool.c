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
 * Pool API tests
 * $Id: pool.c,v 1.2 2009/01/30 00:14:16 castaglia Exp $
 */

#include "tests.h"

START_TEST (parent_pool_test) {
  pool *p;

  p = make_sub_pool(NULL);
  fail_if(p == NULL, "Failed to allocate parent pool");

  destroy_pool(p);
}
END_TEST

START_TEST (parent_sub_pool_test) {
  pool *p, *sub_pool;

  p = make_sub_pool(NULL);
  fail_if(p == NULL, "Failed to allocate parent pool");

  sub_pool = make_sub_pool(p);
  fail_if(sub_pool == NULL, "Failed to allocate sub pool");

  destroy_pool(p);
}
END_TEST

START_TEST (pool_create_sz_test) {
  pool *p, *sub_pool;
  size_t sz;

  p = make_sub_pool(NULL);
  fail_if(p == NULL, "Failed to allocate parent pool");

  sz = 0;
  sub_pool = pr_pool_create_sz(p, sz);
  fail_if(sub_pool == NULL, "Failed to allocate %u-size sub pool", sz);
  destroy_pool(sub_pool);

  sz = 1;
  sub_pool = pr_pool_create_sz(p, sz);
  fail_if(sub_pool == NULL, "Failed to allocate %u-size sub pool", sz);
  destroy_pool(sub_pool);

  sz = 16382;
  sub_pool = pr_pool_create_sz(p, sz);
  fail_if(sub_pool == NULL, "Failed to allocate %u-size sub pool", sz);
  destroy_pool(sub_pool);

  destroy_pool(p);
}
END_TEST

START_TEST (palloc_test) {
  pool *p;
  char *v;
  size_t sz;

  p = make_sub_pool(NULL);
  fail_if(p == NULL, "Failed to allocate parent pool");

  sz = 0;
  v = palloc(p, sz);
  fail_unless(v == NULL, "Allocated %u-len memory", sz);

  sz = 1;
  v = palloc(p, sz);
  fail_if(v == NULL, "Failed to allocate %u-len memory", sz);

  sz = 16382;
  v = palloc(p, sz);
  fail_if(v == NULL, "Failed to allocate %u-len memory", sz);

  destroy_pool(p);
}
END_TEST

START_TEST (pcalloc_test) {
  register unsigned int i;
  pool *p;
  char *v;
  size_t sz;

  p = make_sub_pool(NULL);
  fail_if(p == NULL, "Failed to allocate parent pool");

  sz = 0;
  v = pcalloc(p, sz);
  fail_unless(v == NULL, "Allocated %u-len memory", sz);

  sz = 1;
  v = palloc(p, sz);
  fail_if(v == NULL, "Failed to allocate %u-len memory", sz);
  for (i = 0; i < sz; i++) {
    fail_unless(v[i] == 0, "Allocated non-zero memory at position %u", i);
  }

  sz = 16382;
  v = pcalloc(p, sz);
  fail_if(v == NULL, "Failed to allocate %u-len memory", sz);
  for (i = 0; i < sz; i++) {
    fail_unless(v[i] == 0, "Allocated non-zero memory at position %u", i);
  }

  destroy_pool(p);
}
END_TEST

Suite *tests_get_pool_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("pool");

  testcase = tcase_create("base");
  tcase_add_test(testcase, parent_pool_test);
  tcase_add_test(testcase, parent_sub_pool_test);
  tcase_add_test(testcase, pool_create_sz_test);
  tcase_add_test(testcase, palloc_test);
  tcase_add_test(testcase, pcalloc_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
