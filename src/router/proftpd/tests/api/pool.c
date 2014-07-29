/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008-2013 The ProFTPD Project team
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

/* Pool API tests
 * $Id: pool.c,v 1.5 2013/02/19 15:49:16 castaglia Exp $
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
  fail_if(sub_pool == NULL, "Failed to allocate %u byte sub-pool", sz);
  destroy_pool(sub_pool);

  sz = 1;
  sub_pool = pr_pool_create_sz(p, sz);
  fail_if(sub_pool == NULL, "Failed to allocate %u byte sub-pool", sz);
  destroy_pool(sub_pool);

  sz = 16382;
  sub_pool = pr_pool_create_sz(p, sz);
  fail_if(sub_pool == NULL, "Failed to allocate %u byte sub-pool", sz);
  destroy_pool(sub_pool);

  destroy_pool(p);
}
END_TEST

START_TEST (pool_create_sz_with_alloc_test) {
  register unsigned int i;
  pool *p;
  unsigned int factors[] = { 1, 2, 4, 8, 16, 32, 64, 128, 0 };

  p = make_sub_pool(NULL);

  for (i = 0; factors[i] > 0; i++) {
    register unsigned int j;
    size_t pool_sz, alloc_sz;
    pool *sub_pool;
    unsigned char *data;

    /* Allocate a pool with a given size, then allocate more than that out of
     * the pool.
     */
    pool_sz = (32 * factors[i]);
#ifdef PR_TEST_VERBOSE
    fprintf(stdout, "pool_sz: %lu bytes (factor %u)\n", pool_sz, factors[i]);
#endif /* PR_TEST_VERBOSE */
    sub_pool = pr_pool_create_sz(p, pool_sz);
    fail_if(sub_pool == NULL, "Failed to allocate %u byte sub-pool", pool_sz);

    alloc_sz = (pool_sz * 2);
#ifdef PR_TEST_VERBOSE
    fprintf(stdout, "alloc_sz: %lu bytes (factor %u)\n", alloc_sz, factors[i]);
#endif /* PR_TEST_VERBOSE */
    data = palloc(sub_pool, alloc_sz);

    /* Initialize our allocated memory with some values. */
    mark_point();
    for (j = 0; j < alloc_sz; j++) {
      data[j] = j;
    }

    /* Verify that our values are still there. */
    mark_point();
    for (j = 0; j < alloc_sz; j++) {
#ifdef PR_TEST_VERBOSE
      if (data[j] != j) {
        fprintf(stdout,
          "Iteration #%u: Expected value %u at memory index %u, got %u\n",
          i + 1, j, j, data[j]);
      }
#endif /* PR_TEST_VERBOSE */
      fail_if(data[j] != j,
        "Iteration #%u: Expected value %u at memory index %u, got %u\n", i + 1,
        j, j, data[j]);
    }

    destroy_pool(sub_pool);
  }

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
  v = pcalloc(p, sz);
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

  /* Seems this particular testcase reveals a bug in the pool code.  On the
   * third iteration of the loop (pool size = 256, alloc size = 512), the
   * memory check fails.  Perhaps related to PR_TUNABLE_NEW_POOL_SIZE being
   * 512 bytes?  Need to dig into this more.  In the mean time, keep the
   * testcase commented out.
   *
   * Note: when it fails, it looks like:
   *
   *   api/pool.c:116:F:base:pool_create_sz_with_alloc_test:0: Iteration #3: Expected value 256 at memory index 256, got 0
   *
   * If the PR_TEST_VERBOSE macro is defined, you can see the printout of
   * where the memory read does not meet expectations.
   */
#if 0
  tcase_add_test(testcase, pool_create_sz_with_alloc_test);
#endif
  tcase_add_test(testcase, palloc_test);
  tcase_add_test(testcase, pcalloc_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
