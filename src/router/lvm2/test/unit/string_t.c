/*
 * Copyright (C) 2012 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "units.h"
#include "device_mapper/all.h"

#include <stdio.h>
#include <string.h>

#if 0
static int _mem_init(void)
{
	struct dm_pool *mem = dm_pool_create("string test", 1024);
	if (!mem) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	return mem;
}

static void _mem_exit(void *mem)
{
	dm_pool_destroy(mem);
}

/* TODO: Add more string unit tests here */
#endif

static void test_strncpy(void *fixture)
{
	const char st[] = "1234567890";
	char buf[sizeof(st)];

	T_ASSERT_EQUAL(dm_strncpy(buf, st, sizeof(buf)), 1);
	T_ASSERT_EQUAL(strcmp(buf, st), 0);

	T_ASSERT_EQUAL(dm_strncpy(buf, st, sizeof(buf) - 1), 0);
	T_ASSERT_EQUAL(strlen(buf) + 1, sizeof(buf) - 1);
}

static void test_asprint(void *fixture)
{
	const char st0[] = "";
	const char st1[] = "12345678901";
	const char st2[] = "1234567890123456789012345678901234567890123456789012345678901234567";
	char *buf;
	int a;

	a = dm_asprintf(&buf, "%s", st0);
	T_ASSERT_EQUAL(strcmp(buf, st0), 0);
	T_ASSERT_EQUAL(a, sizeof(st0));
	free(buf);

	a = dm_asprintf(&buf, "%s", st1);
	T_ASSERT_EQUAL(strcmp(buf, st1), 0);
	T_ASSERT_EQUAL(a, sizeof(st1));
	free(buf);

	a = dm_asprintf(&buf, "%s", st2);
	T_ASSERT_EQUAL(a, sizeof(st2));
	T_ASSERT_EQUAL(strcmp(buf, st2), 0);
	free(buf);
}

#define T(path, desc, fn) register_test(ts, "/base/data-struct/string/" path, desc, fn)

void string_tests(struct dm_list *all_tests)
{
	struct test_suite *ts = test_suite_create(NULL, NULL);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("asprint", "tests asprint", test_asprint);
	T("strncpy", "tests string copying", test_strncpy);

	dm_list_add(all_tests, &ts->list);
}
