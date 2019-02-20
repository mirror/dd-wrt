/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2010 Red Hat, Inc. All rights reserved.
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

#include "matcher_data.h"

static void *_mem_init(void)
{
	struct dm_pool *mem = dm_pool_create("bitset test", 1024);
	if (!mem) {
		fprintf(stderr, "out of memory");
		exit(1);
	}

	return mem;
}

static void _mem_exit(void *mem)
{
	dm_pool_destroy(mem);
}

static struct dm_regex *make_scanner(struct dm_pool *mem, const char **rx)
{
	struct dm_regex *scanner;
	int nrx = 0;
	for (; rx[nrx]; ++nrx);

	scanner = dm_regex_create(mem, rx, nrx);
	T_ASSERT(scanner != NULL);
	return scanner;
}

static void test_fingerprints(void *fixture)
{
	struct dm_pool *mem = fixture;
	struct dm_regex *scanner;

	scanner = make_scanner(mem, dev_patterns);
	T_ASSERT_EQUAL(dm_regex_fingerprint(scanner), 0x7f556c09);

	scanner = make_scanner(mem, random_patterns);
	T_ASSERT_EQUAL(dm_regex_fingerprint(scanner), 0x9f11076c);
}

static void test_matching(void *fixture)
{
	struct dm_pool *mem = fixture;
	struct dm_regex *scanner;
	int i;

	scanner = make_scanner(mem, dev_patterns);
	for (i = 0; devices[i].str; ++i)
		T_ASSERT_EQUAL(dm_regex_match(scanner, devices[i].str), devices[i].expected - 1);

	scanner = make_scanner(mem, nonprint_patterns);
	for (i = 0; nonprint[i].str; ++i)
		T_ASSERT_EQUAL(dm_regex_match(scanner, nonprint[i].str), nonprint[i].expected - 1);
}

static void test_kabi_query(void *fixture)
{
        // Remember, matches regexes from last to first.
        static const char *_patterns[] = {
                ".*", ".*/dev/md.*", "loop"
        };

        static struct {
                const char *input;
                int r;
        } _cases[] = {
		{"foo", 0},
		{"/dev/mapper/vg-lvol1", 0},
		{"/dev/mapper/vglvol1", 0},
		{"/dev/md1", 1},
		{"loop", 2},
        };

	int r;
	unsigned i;
	struct dm_pool *mem = fixture;
  	struct dm_regex *scanner;

	scanner = dm_regex_create(mem, _patterns, DM_ARRAY_SIZE(_patterns));
	T_ASSERT(scanner != NULL);

	for (i = 0; i < DM_ARRAY_SIZE(_cases); i++) {
        	r = dm_regex_match(scanner, _cases[i].input);
        	if (r != _cases[i].r) {
                	test_fail("'%s' expected to match '%s', but matched %s",
                                  _cases[i].input,
                                  _cases[i].r >= DM_ARRAY_SIZE(_patterns) ? "<nothing>" : _patterns[_cases[i].r],
                                  r >= DM_ARRAY_SIZE(_patterns) ? "<nothing>" : _patterns[r]);
        	}
	}


}

#define T(path, desc, fn) register_test(ts, "/base/regex/" path, desc, fn)

void regex_tests(struct dm_list *all_tests)
{
	struct test_suite *ts = test_suite_create(_mem_init, _mem_exit);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("fingerprints", "not sure", test_fingerprints);
	T("matching", "test the matcher with a variety of regexes", test_matching);
	T("kabi-query", "test the matcher with some specific patterns", test_kabi_query);

	dm_list_add(all_tests, &ts->list);
}
