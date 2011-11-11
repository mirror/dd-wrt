/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2010-2011 The ProFTPD Project team
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

/* Stash API tests
 * $Id: stash.c,v 1.2 2011/05/23 20:50:31 castaglia Exp $
 */

#include "tests.h"

static pool *p = NULL;

static void set_up(void) {
  if (p == NULL) {
    p = permanent_pool = make_sub_pool(NULL);
  }

  init_stash();
}

static void tear_down(void) {
  if (p) {
    destroy_pool(p);
    p = NULL;
    permanent_pool = NULL;
  } 
}

START_TEST (stash_add_symbol_test) {
  int res;
  conftable conftab;
  cmdtable cmdtab, hooktab;
  authtable authtab;
  
  res = pr_stash_add_symbol(0, NULL);
  fail_unless(res == -1, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);

  res = pr_stash_add_symbol(0, "Foo");
  fail_unless(res == -1, "Failed to handle bad type");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  memset(&conftab, 0, sizeof(conftab));
  res = pr_stash_add_symbol(PR_SYM_CONF, &conftab);
  fail_unless(res == -1, "Failed to handle null conf name");
  fail_unless(errno == EPERM, "Failed to set errno to EPERM (got %d)",
    errno);

  memset(&cmdtab, 0, sizeof(cmdtab));
  res = pr_stash_add_symbol(PR_SYM_CMD, &cmdtab);
  fail_unless(res == -1, "Failed to handle null cmd name");
  fail_unless(errno == EPERM, "Failed to set errno to EPERM (got %d)",
    errno);

  memset(&authtab, 0, sizeof(authtab));
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  fail_unless(res == -1, "Failed to handle null auth name");
  fail_unless(errno == EPERM, "Failed to set errno to EPERM (got %d)",
    errno);

  memset(&hooktab, 0, sizeof(hooktab));
  res = pr_stash_add_symbol(PR_SYM_HOOK, &hooktab);
  fail_unless(res == -1, "Failed to handle null hook name");
  fail_unless(errno == EPERM, "Failed to set errno to EPERM (got %d)",
    errno);

  memset(&conftab, 0, sizeof(conftab));
  conftab.directive = pstrdup(p, "Foo");
  res = pr_stash_add_symbol(PR_SYM_CONF, &conftab);
  fail_unless(res == 0, "Failed to add CONF symbol: %s", strerror(errno));

  memset(&cmdtab, 0, sizeof(cmdtab));
  cmdtab.command = pstrdup(p, "Foo");
  res = pr_stash_add_symbol(PR_SYM_CMD, &cmdtab);
  fail_unless(res == 0, "Failed to add CMD symbol: %s", strerror(errno));

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = pstrdup(p, "Foo");
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  fail_unless(res == 0, "Failed to add AUTH symbol: %s", strerror(errno));

  memset(&hooktab, 0, sizeof(hooktab));
  hooktab.command = pstrdup(p, "Foo");
  res = pr_stash_add_symbol(PR_SYM_HOOK, &hooktab);
  fail_unless(res == 0, "Failed to add HOOK symbol: %s", strerror(errno));
}
END_TEST

START_TEST (stash_get_symbol_test) {
  int res;
  void *sym;
  conftable conftab;
  cmdtable cmdtab, hooktab;
  authtable authtab;

  sym = pr_stash_get_symbol(0, NULL, NULL, NULL);
  fail_unless(sym == NULL, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);

  sym = pr_stash_get_symbol(0, "foo", NULL, NULL);
  fail_unless(sym == NULL, "Failed to handle bad type argument");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);

  sym = pr_stash_get_symbol(PR_SYM_CONF, "foo", NULL, NULL);
  fail_unless(sym == NULL, "Failed to handle nonexistent CONF symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  sym = pr_stash_get_symbol(PR_SYM_CMD, "foo", NULL, NULL);
  fail_unless(sym == NULL, "Failed to handle nonexistent CMD symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  sym = pr_stash_get_symbol(PR_SYM_AUTH, "foo", NULL, NULL);
  fail_unless(sym == NULL, "Failed to handle nonexistent AUTH symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  sym = pr_stash_get_symbol(PR_SYM_HOOK, "foo", NULL, NULL);
  fail_unless(sym == NULL, "Failed to handle nonexistent HOOK symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  memset(&conftab, 0, sizeof(conftab));
  conftab.directive = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_CONF, &conftab);
  fail_unless(res == 0, "Failed to add CONF symbol: %s", strerror(errno));

  sym = pr_stash_get_symbol(PR_SYM_CONF, "foo", NULL, NULL);
  fail_unless(sym != NULL, "Failed to get CONF symbol: %s", strerror(errno));
  fail_unless(sym == &conftab, "Expected %p, got %p", &conftab, sym);

  sym = pr_stash_get_symbol(PR_SYM_CONF, "foo", sym, NULL);
  fail_unless(sym == NULL, "Unexpectedly found CONF symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  memset(&cmdtab, 0, sizeof(cmdtab));
  cmdtab.command = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_CMD, &cmdtab);
  fail_unless(res == 0, "Failed to add CMD symbol: %s", strerror(errno));

  sym = pr_stash_get_symbol(PR_SYM_CMD, "foo", NULL, NULL);
  fail_unless(sym != NULL, "Failed to get CMD symbol: %s", strerror(errno));
  fail_unless(sym == &cmdtab, "Expected %p, got %p", &cmdtab, sym);

  sym = pr_stash_get_symbol(PR_SYM_CMD, "foo", sym, NULL);
  fail_unless(sym == NULL, "Unexpectedly found CMD symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  fail_unless(res == 0, "Failed to add AUTH symbol: %s", strerror(errno));

  sym = pr_stash_get_symbol(PR_SYM_AUTH, "foo", NULL, NULL);
  fail_unless(sym != NULL, "Failed to get AUTH symbol: %s", strerror(errno));
  fail_unless(sym == &authtab, "Expected %p, got %p", &authtab, sym);

  sym = pr_stash_get_symbol(PR_SYM_AUTH, "foo", sym, NULL);
  fail_unless(sym == NULL, "Unexpectedly found AUTH symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);

  memset(&hooktab, 0, sizeof(hooktab));
  hooktab.command = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_HOOK, &hooktab);
  fail_unless(res == 0, "Failed to add HOOK symbol: %s", strerror(errno));

  sym = pr_stash_get_symbol(PR_SYM_HOOK, "foo", NULL, NULL);
  fail_unless(sym != NULL, "Failed to get HOOK symbol: %s", strerror(errno));
  fail_unless(sym == &hooktab, "Expected %p, got %p", &hooktab, sym);

  sym = pr_stash_get_symbol(PR_SYM_HOOK, "foo", sym, NULL);
  fail_unless(sym == NULL, "Unexpectedly found HOOK symbol");
  fail_unless(errno == ENOENT, "Failed to set errno to ENOENT (got %d)",
    errno);
}
END_TEST

START_TEST (stash_remove_symbol_test) {
  int res;
  conftable conftab;
  cmdtable cmdtab, hooktab;
  authtable authtab;

  res = pr_stash_remove_symbol(0, NULL, NULL);
  fail_unless(res == -1, "Failed to handle null arguments");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);

  res = pr_stash_remove_symbol(0, "foo", NULL);
  fail_unless(res == -1, "Failed to handle bad symbol type");
  fail_unless(errno == EINVAL, "Failed to set errno to EINVAL (got %d)",
    errno);

  res = pr_stash_remove_symbol(PR_SYM_CONF, "foo", NULL);
  fail_unless(res == 0, "Expected %d, got %d", 0, res);

  memset(&conftab, 0, sizeof(conftab));
  conftab.directive = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_CONF, &conftab);
  fail_unless(res == 0, "Failed to add CONF symbol: %s", strerror(errno));

  res = pr_stash_add_symbol(PR_SYM_CONF, &conftab);
  fail_unless(res == 0, "Failed to add CONF symbol: %s", strerror(errno));

  res = pr_stash_remove_symbol(PR_SYM_CONF, "foo", NULL);
  fail_unless(res == 2, "Expected %d, got %d", 2, res);

  memset(&cmdtab, 0, sizeof(cmdtab));
  cmdtab.command = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_CMD, &cmdtab);
  fail_unless(res == 0, "Failed to add CMD symbol: %s", strerror(errno));

  res = pr_stash_add_symbol(PR_SYM_CMD, &cmdtab);
  fail_unless(res == 0, "Failed to add CMD symbol: %s", strerror(errno));

  res = pr_stash_remove_symbol(PR_SYM_CMD, "foo", NULL);
  fail_unless(res == 2, "Expected %d, got %d", 2, res);

  memset(&authtab, 0, sizeof(authtab));
  authtab.name = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  fail_unless(res == 0, "Failed to add AUTH symbol: %s", strerror(errno));

  res = pr_stash_add_symbol(PR_SYM_AUTH, &authtab);
  fail_unless(res == 0, "Failed to add AUTH symbol: %s", strerror(errno));

  res = pr_stash_remove_symbol(PR_SYM_AUTH, "foo", NULL);
  fail_unless(res == 2, "Expected %d, got %d", 2, res);

  memset(&hooktab, 0, sizeof(hooktab));
  hooktab.command = pstrdup(p, "foo");
  res = pr_stash_add_symbol(PR_SYM_HOOK, &hooktab);
  fail_unless(res == 0, "Failed to add HOOK symbol: %s", strerror(errno));

  res = pr_stash_add_symbol(PR_SYM_HOOK, &hooktab);
  fail_unless(res == 0, "Failed to add HOOK symbol: %s", strerror(errno));

  res = pr_stash_remove_symbol(PR_SYM_HOOK, "foo", NULL);
  fail_unless(res == 2, "Expected %d, got %d", 2, res);
}
END_TEST

Suite *tests_get_stash_suite(void) {
  Suite *suite;
  TCase *testcase;

  suite = suite_create("stash");

  testcase = tcase_create("stash");
  tcase_add_checked_fixture(testcase, set_up, tear_down);

  tcase_add_test(testcase, stash_add_symbol_test);
  tcase_add_test(testcase, stash_get_symbol_test);
  tcase_add_test(testcase, stash_remove_symbol_test);

  suite_add_tcase(suite, testcase);

  return suite;
}
