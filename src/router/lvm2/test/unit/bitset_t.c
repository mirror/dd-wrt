/*
 * Copyright (C) 2010 Red Hat, Inc. All rights reserved.
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

enum {
        NR_BITS = 137
};

static void *_mem_init(void) {
	struct dm_pool *mem = dm_pool_create("bitset test", 1024);
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

static void test_get_next(void *fixture)
{
	struct dm_pool *mem = fixture;

        int i, j, last = 0, first;
        dm_bitset_t bs = dm_bitset_create(mem, NR_BITS);

        for (i = 0; i < NR_BITS; i++)
                T_ASSERT(!dm_bit(bs, i));

        for (i = 0, j = 1; i < NR_BITS; i += j, j++)
                dm_bit_set(bs, i);

        first = 1;
        for (i = 0, j = 1; i < NR_BITS; i += j, j++) {
                if (first) {
                        last = dm_bit_get_first(bs);
                        first = 0;
                } else
                        last = dm_bit_get_next(bs, last);

                T_ASSERT(last == i);
        }

        T_ASSERT(dm_bit_get_next(bs, last) == -1);
}

static void bit_flip(dm_bitset_t bs, int bit)
{
        int old = dm_bit(bs, bit);
        if (old)
                dm_bit_clear(bs, bit);
        else
                dm_bit_set(bs, bit);
}

static void test_equal(void *fixture)
{
	struct dm_pool *mem = fixture;
        dm_bitset_t bs1 = dm_bitset_create(mem, NR_BITS);
        dm_bitset_t bs2 = dm_bitset_create(mem, NR_BITS);

        int i, j;
        for (i = 0, j = 1; i < NR_BITS; i += j, j++) {
                dm_bit_set(bs1, i);
                dm_bit_set(bs2, i);
        }

        T_ASSERT(dm_bitset_equal(bs1, bs2));
        T_ASSERT(dm_bitset_equal(bs2, bs1));

        for (i = 0; i < NR_BITS; i++) {
                bit_flip(bs1, i);
                T_ASSERT(!dm_bitset_equal(bs1, bs2));
                T_ASSERT(!dm_bitset_equal(bs2, bs1));

                T_ASSERT(dm_bitset_equal(bs1, bs1)); /* comparing with self */
                bit_flip(bs1, i);
        }
}

static void test_and(void *fixture)
{
	struct dm_pool *mem = fixture;
        dm_bitset_t bs1 = dm_bitset_create(mem, NR_BITS);
        dm_bitset_t bs2 = dm_bitset_create(mem, NR_BITS);
        dm_bitset_t bs3 = dm_bitset_create(mem, NR_BITS);

        int i, j;
        for (i = 0, j = 1; i < NR_BITS; i += j, j++) {
                dm_bit_set(bs1, i);
                dm_bit_set(bs2, i);
        }

        dm_bit_and(bs3, bs1, bs2);

        T_ASSERT(dm_bitset_equal(bs1, bs2));
        T_ASSERT(dm_bitset_equal(bs1, bs3));
        T_ASSERT(dm_bitset_equal(bs2, bs3));

        dm_bit_clear_all(bs1);
        dm_bit_clear_all(bs2);

        for (i = 0; i < NR_BITS; i++) {
                if (i % 2)
                        dm_bit_set(bs1, i);
                else
                        dm_bit_set(bs2, i);
        }

        dm_bit_and(bs3, bs1, bs2);
        for (i = 0; i < NR_BITS; i++)
                T_ASSERT(!dm_bit(bs3, i));
}

#define T(path, desc, fn) register_test(ts, "/base/data-struct/bitset/" path, desc, fn)

void bitset_tests(struct dm_list *all_tests)
{
	struct test_suite *ts = test_suite_create(_mem_init, _mem_exit);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("get_next", "get next set bit", test_get_next);
	T("equal", "equality", test_equal);
	T("and", "and all bits", test_and);

	dm_list_add(all_tests, &ts->list);
}

