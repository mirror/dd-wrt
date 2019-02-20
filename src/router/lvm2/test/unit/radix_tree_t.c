// Copyright (C) 2018 Red Hat, Inc. All rights reserved.
// 
// This file is part of LVM2.
//
// This copyrighted material is made available to anyone wishing to use,
// modify, copy, or redistribute it subject to the terms and conditions
// of the GNU Lesser General Public License v.2.1.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
#include "units.h"
#include "base/data-struct/radix-tree.h"
#include "base/memory/container_of.h"

#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------

static void *rt_init(void)
{
	struct radix_tree *rt = radix_tree_create(NULL, NULL);
	T_ASSERT(rt);
	return rt;
}

static void rt_exit(void *fixture)
{
	radix_tree_destroy(fixture);
}

static void test_create_destroy(void *fixture)
{
	T_ASSERT(fixture);
}

static void test_insert_one(void *fixture)
{
	struct radix_tree *rt = fixture;
	union radix_value v;
	unsigned char k = 'a';
	v.n = 65;
	T_ASSERT(radix_tree_insert(rt, &k, &k + 1, v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	v.n = 0;
	T_ASSERT(radix_tree_lookup(rt, &k, &k + 1, &v));
	T_ASSERT_EQUAL(v.n, 65);
}

static void test_single_byte_keys(void *fixture)
{
	unsigned i, count = 256;
	struct radix_tree *rt = fixture;
	union radix_value v;
	uint8_t k;

	for (i = 0; i < count; i++) {
		k = i;
		v.n = 100 + i;
		T_ASSERT(radix_tree_insert(rt, &k, &k + 1, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));

	for (i = 0; i < count; i++) {
		k = i;
		T_ASSERT(radix_tree_lookup(rt, &k, &k + 1, &v));
		T_ASSERT_EQUAL(v.n, 100 + i);
	}
}

static void test_overwrite_single_byte_keys(void *fixture)
{
	unsigned i, count = 256;
	struct radix_tree *rt = fixture;
	union radix_value v;
	uint8_t k;

	for (i = 0; i < count; i++) {
		k = i;
		v.n = 100 + i;
		T_ASSERT(radix_tree_insert(rt, &k, &k + 1, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));

	for (i = 0; i < count; i++) {
		k = i;
		v.n = 1000 + i;
		T_ASSERT(radix_tree_insert(rt, &k, &k + 1, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));

	for (i = 0; i < count; i++) {
		k = i;
		T_ASSERT(radix_tree_lookup(rt, &k, &k + 1, &v));
		T_ASSERT_EQUAL(v.n, 1000 + i);
	}
}

static void test_16_bit_keys(void *fixture)
{
	unsigned i, count = 1 << 16;
	struct radix_tree *rt = fixture;
	union radix_value v;
	uint8_t k[2];

	for (i = 0; i < count; i++) {
		k[0] = i / 256;
		k[1] = i % 256;
		v.n = 100 + i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));

	for (i = 0; i < count; i++) {
		k[0] = i / 256;
		k[1] = i % 256;
		T_ASSERT(radix_tree_lookup(rt, k, k + sizeof(k), &v));
		T_ASSERT_EQUAL(v.n, 100 + i);
	}
}

static void test_prefix_keys(void *fixture)
{
	struct radix_tree *rt = fixture;
	union radix_value v;
	uint8_t k[2];

	k[0] = 100;
	k[1] = 200;
	v.n = 1024;
	T_ASSERT(radix_tree_insert(rt, k, k + 1, v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	v.n = 2345;
	T_ASSERT(radix_tree_insert(rt, k, k + 2, v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	T_ASSERT(radix_tree_lookup(rt, k, k + 1, &v));
	T_ASSERT_EQUAL(v.n, 1024);
	T_ASSERT(radix_tree_lookup(rt, k, k + 2, &v));
	T_ASSERT_EQUAL(v.n, 2345);
}

static void test_prefix_keys_reversed(void *fixture)
{
	struct radix_tree *rt = fixture;
	union radix_value v;
	uint8_t k[2];

	k[0] = 100;
	k[1] = 200;
	v.n = 1024;
	T_ASSERT(radix_tree_insert(rt, k, k + 2, v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	v.n = 2345;
	T_ASSERT(radix_tree_insert(rt, k, k + 1, v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	T_ASSERT(radix_tree_lookup(rt, k, k + 2, &v));
	T_ASSERT_EQUAL(v.n, 1024);
	T_ASSERT(radix_tree_lookup(rt, k, k + 1, &v));
	T_ASSERT_EQUAL(v.n, 2345);
}

static void _gen_key(uint8_t *b, uint8_t *e)
{
	for (; b != e; b++)
		*b = rand() % 256;
}

static void test_sparse_keys(void *fixture)
{
	unsigned n;
	struct radix_tree *rt = fixture;
	union radix_value v;
	uint8_t k[32];

	for (n = 0; n < 100000; n++) {
		_gen_key(k, k + sizeof(k));
		v.n = 1234;
		T_ASSERT(radix_tree_insert(rt, k, k + 32, v));
		// FIXME: remove
		//T_ASSERT(radix_tree_is_well_formed(rt));
	}
	T_ASSERT(radix_tree_is_well_formed(rt));
}

static void test_remove_one(void *fixture)
{
	struct radix_tree *rt = fixture;
	uint8_t k[4];
	union radix_value v;

	_gen_key(k, k + sizeof(k));
	v.n = 1234;
	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	T_ASSERT(radix_tree_remove(rt, k, k + sizeof(k)));
	T_ASSERT(radix_tree_is_well_formed(rt));
	T_ASSERT(!radix_tree_lookup(rt, k, k + sizeof(k), &v));
}

static void test_remove_one_byte_keys(void *fixture)
{
        struct radix_tree *rt = fixture;
        unsigned i, j;
	uint8_t k[1];
	union radix_value v;

	for (i = 0; i < 256; i++) {
        	k[0] = i;
        	v.n = i + 1000;
		T_ASSERT(radix_tree_insert(rt, k, k + 1, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	for (i = 0; i < 256; i++) {
        	k[0] = i;
		T_ASSERT(radix_tree_remove(rt, k, k + 1));
		T_ASSERT(radix_tree_is_well_formed(rt));

		for (j = i + 1; j < 256; j++) {
        		k[0] = j;
			T_ASSERT(radix_tree_lookup(rt, k, k + 1, &v));
			if (v.n != j + 1000)
				test_fail("v.n (%u) != j + 1000 (%u)\n",
                                          (unsigned) v.n,
                                          (unsigned) j + 1000);
		}
	}

	for (i = 0; i < 256; i++) {
        	k[0] = i;
		T_ASSERT(!radix_tree_lookup(rt, k, k + 1, &v));
	}
}

static void test_remove_one_byte_keys_reversed(void *fixture)
{
        struct radix_tree *rt = fixture;
        unsigned i, j;
	uint8_t k[1];
	union radix_value v;

	for (i = 0; i < 256; i++) {
        	k[0] = i;
        	v.n = i + 1000;
		T_ASSERT(radix_tree_insert(rt, k, k + 1, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	for (i = 256; i; i--) {
        	k[0] = i - 1;
		T_ASSERT(radix_tree_remove(rt, k, k + 1));
		T_ASSERT(radix_tree_is_well_formed(rt));

		for (j = 0; j < i - 1; j++) {
        		k[0] = j;
			T_ASSERT(radix_tree_lookup(rt, k, k + 1, &v));
			if (v.n != j + 1000)
				test_fail("v.n (%u) != j + 1000 (%u)\n",
                                          (unsigned) v.n,
                                          (unsigned) j + 1000);
		}
	}

	for (i = 0; i < 256; i++) {
        	k[0] = i;
		T_ASSERT(!radix_tree_lookup(rt, k, k + 1, &v));
	}
}
static void test_remove_prefix_keys(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i, j;
	uint8_t k[32];
	union radix_value v;

	_gen_key(k, k + sizeof(k));

	for (i = 0; i < 32; i++) {
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + i, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	for (i = 0; i < 32; i++) {
        	T_ASSERT(radix_tree_remove(rt, k, k + i));
		T_ASSERT(radix_tree_is_well_formed(rt));
        	for (j = i + 1; j < 32; j++) {
                	T_ASSERT(radix_tree_lookup(rt, k, k + j, &v));
                	T_ASSERT_EQUAL(v.n, j);
        	}
	}

        for (i = 0; i < 32; i++)
                T_ASSERT(!radix_tree_lookup(rt, k, k + i, &v));
}

static void test_remove_prefix_keys_reversed(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i, j;
	uint8_t k[32];
	union radix_value v;

	_gen_key(k, k + sizeof(k));

	for (i = 0; i < 32; i++) {
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + i, v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	for (i = 0; i < 32; i++) {
        	T_ASSERT(radix_tree_remove(rt, k, k + (31 - i)));
		T_ASSERT(radix_tree_is_well_formed(rt));
        	for (j = 0; j < 31 - i; j++) {
                	T_ASSERT(radix_tree_lookup(rt, k, k + j, &v));
                	T_ASSERT_EQUAL(v.n, j);
        	}
	}

        for (i = 0; i < 32; i++)
                T_ASSERT(!radix_tree_lookup(rt, k, k + i, &v));
}

static void test_remove_prefix(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i, count = 0;
	uint8_t k[4];
	union radix_value v;

	// populate some random 32bit keys
	for (i = 0; i < 100000; i++) {
        	_gen_key(k, k + sizeof(k));
        	if (k[0] == 21)
                	count++;
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));

	// remove keys in a sub range 
	k[0] = 21;
	T_ASSERT_EQUAL(radix_tree_remove_prefix(rt, k, k + 1), count);
	T_ASSERT(radix_tree_is_well_formed(rt));
}

static void test_remove_prefix_single(void *fixture)
{
	struct radix_tree *rt = fixture;
	uint8_t k[4];
	union radix_value v;

	_gen_key(k, k + sizeof(k));
	v.n = 1234;
	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	T_ASSERT(radix_tree_is_well_formed(rt));
	T_ASSERT_EQUAL(radix_tree_remove_prefix(rt, k, k + 2), 1);
	T_ASSERT(radix_tree_is_well_formed(rt));
}

static void test_size(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i, dup_count = 0;
	uint8_t k[2];
	union radix_value v;

	// populate some random 16bit keys
	for (i = 0; i < 10000; i++) {
        	_gen_key(k, k + sizeof(k));
        	if (radix_tree_lookup(rt, k, k + sizeof(k), &v))
                	dup_count++;
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}

	T_ASSERT_EQUAL(radix_tree_size(rt), 10000 - dup_count);
	T_ASSERT(radix_tree_is_well_formed(rt));
}

struct visitor {
	struct radix_tree_iterator it;
	unsigned count;
};

static bool _visit(struct radix_tree_iterator *it,
                   uint8_t *kb, uint8_t *ke, union radix_value v)
{
	struct visitor *vt = container_of(it, struct visitor, it);
	vt->count++;
	return true;
}

static void test_iterate_all(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i;
	uint8_t k[4];
	union radix_value v;
	struct visitor vt;

	// populate some random 32bit keys
	for (i = 0; i < 100000; i++) {
        	_gen_key(k, k + sizeof(k));
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	vt.count = 0;
	vt.it.visit = _visit;
	radix_tree_iterate(rt, NULL, NULL, &vt.it);
	T_ASSERT_EQUAL(vt.count, radix_tree_size(rt));
}

static void test_iterate_subset(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i, subset_count = 0;
	uint8_t k[3];
	union radix_value v;
	struct visitor vt;

	// populate some random 32bit keys
	for (i = 0; i < 100000; i++) {
        	_gen_key(k, k + sizeof(k));
        	if (k[0] == 21 && k[1] == 12)
                	subset_count++;
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	vt.count = 0;
	vt.it.visit = _visit;
	k[0] = 21;
	k[1] = 12;
	radix_tree_iterate(rt, k, k + 2, &vt.it);
	T_ASSERT_EQUAL(vt.count, subset_count);
}

static void test_iterate_single(void *fixture)
{
	struct radix_tree *rt = fixture;
	uint8_t k[6];
	union radix_value v;
	struct visitor vt;

	_gen_key(k, k + sizeof(k));
	v.n = 1234;
	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));

	T_ASSERT(radix_tree_is_well_formed(rt));
	vt.count = 0;
	vt.it.visit = _visit;
	radix_tree_iterate(rt, k, k + 3, &vt.it);
	T_ASSERT_EQUAL(vt.count, 1);
}

static void test_iterate_vary_middle(void *fixture)
{
	struct radix_tree *rt = fixture;
	unsigned i;
	uint8_t k[6];
	union radix_value v;
	struct visitor vt;

	_gen_key(k, k + sizeof(k));
	for (i = 0; i < 16; i++) {
        	k[3] = i;
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}

	T_ASSERT(radix_tree_is_well_formed(rt));
	vt.it.visit = _visit;
	for (i = 0; i < 16; i++) {
        	vt.count = 0;
        	k[3] = i;
        	radix_tree_iterate(rt, k, k + 4, &vt.it);
        	T_ASSERT_EQUAL(vt.count, 1);
	}
}

//----------------------------------------------------------------

#define DTR_COUNT 100

struct counter {
	unsigned c;
	uint8_t present[DTR_COUNT];
};

static void _counting_dtr(void *context, union radix_value v)
{
	struct counter *c = context;
	c->c++;
	T_ASSERT(v.n < DTR_COUNT);
	c->present[v.n] = 0;
}

static void test_remove_calls_dtr(void *fixture)
{
	struct counter c;
	struct radix_tree *rt = radix_tree_create(_counting_dtr, &c);
	T_ASSERT(rt);

	// Bug hunting, so I need the keys to be deterministic
	srand(0);

	c.c = 0;
	memset(c.present, 1, sizeof(c.present));

	{
		unsigned i;
		uint8_t keys[DTR_COUNT * 3];
		union radix_value v;

		// generate and insert a lot of keys
		for (i = 0; i < DTR_COUNT; i++) {
			bool found = false;
			do {
				v.n = i;
				uint8_t *k = keys + (i * 3);
				_gen_key(k, k + 3);
				if (!radix_tree_lookup(rt, k, k + 3, &v)) {
					T_ASSERT(radix_tree_insert(rt, k, k + 3, v));
					found = true;
				}

			} while (!found);
		}

		T_ASSERT(radix_tree_is_well_formed(rt));
		
		// double check
		for (i = 0; i < DTR_COUNT; i++) {
			uint8_t *k = keys + (i * 3);
			T_ASSERT(radix_tree_lookup(rt, k, k + 3, &v));
		}

		for (i = 0; i < DTR_COUNT; i++) {
			uint8_t *k = keys + (i * 3);
			// FIXME: check the values get passed to the dtr
			T_ASSERT(radix_tree_remove(rt, k, k + 3));
		}

		T_ASSERT(c.c == DTR_COUNT);
		for (i = 0; i < DTR_COUNT; i++)
			T_ASSERT(!c.present[i]);
	}

	radix_tree_destroy(rt);
}

static void test_destroy_calls_dtr(void *fixture)
{
	unsigned i;
	struct counter c;
	struct radix_tree *rt = radix_tree_create(_counting_dtr, &c);
	T_ASSERT(rt);

	// Bug hunting, so I need the keys to be deterministic
	srand(0);

	c.c = 0;
	memset(c.present, 1, sizeof(c.present));

	{
		uint8_t keys[DTR_COUNT * 3];
		union radix_value v;

		// generate and insert a lot of keys
		for (i = 0; i < DTR_COUNT; i++) {
			bool found = false;
			do {
				v.n = i;
				uint8_t *k = keys + (i * 3);
				_gen_key(k, k + 3);
				if (!radix_tree_lookup(rt, k, k + 3, &v)) {
					T_ASSERT(radix_tree_insert(rt, k, k + 3, v));
					found = true;
				}

			} while (!found);
		}

		T_ASSERT(radix_tree_is_well_formed(rt));
	}
		
	radix_tree_destroy(rt);
	T_ASSERT(c.c == DTR_COUNT);
	for (i = 0; i < DTR_COUNT; i++)
		T_ASSERT(!c.present[i]);
}

//----------------------------------------------------------------

static void test_bcache_scenario(void *fixture)
{
	struct radix_tree *rt = fixture;

    	unsigned i;
    	uint8_t k[6];
	union radix_value v;

    	memset(k, 0, sizeof(k));

    	for (i = 0; i < 3; i++) {
	    	// it has to be the 4th byte that varies to
	    	// trigger the bug.
	    	k[4] = i;
	    	v.n = i;
	    	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
    	}
	T_ASSERT(radix_tree_is_well_formed(rt));

	k[4] = 0;
    	T_ASSERT(radix_tree_remove(rt, k, k + sizeof(k)));
	T_ASSERT(radix_tree_is_well_formed(rt));

    	k[4] = i;
    	v.n = i;
    	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	T_ASSERT(radix_tree_is_well_formed(rt));
}

//----------------------------------------------------------------

static void _bcs2_step1(struct radix_tree *rt)
{
	unsigned i;
	uint8_t k[12];
	union radix_value v;

	memset(k, 0, sizeof(k));
	for (i = 0x6; i < 0x69; i++) {
		k[0] = i;
		v.n = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}
	T_ASSERT(radix_tree_is_well_formed(rt));
}

static void _bcs2_step2(struct radix_tree *rt)
{
	unsigned i;
	uint8_t k[12];

	memset(k, 0, sizeof(k));
	for (i = 0x6; i < 0x69; i++) {
		k[0] = i;
		radix_tree_remove_prefix(rt, k, k + 4);
	}
	T_ASSERT(radix_tree_is_well_formed(rt));
}

static void test_bcache_scenario2(void *fixture)
{
	unsigned i;
	struct radix_tree *rt = fixture;
	uint8_t k[12];
	union radix_value v;

	_bcs2_step1(rt);
	_bcs2_step2(rt);

	memset(k, 0, sizeof(k));
        for (i = 0; i < 50; i++) {
	        k[0] = 0x6;
	        v.n = 0x6;
	        T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	        radix_tree_remove_prefix(rt, k, k + 4);
        }
        T_ASSERT(radix_tree_is_well_formed(rt));

	_bcs2_step1(rt);
	_bcs2_step2(rt);
	_bcs2_step1(rt);
	_bcs2_step2(rt);

	memset(k, 0, sizeof(k));
	for(i = 0x6; i < 0x37; i++) {
		k[0] = i;
		k[4] = 0xf;
		k[5] = 0x1;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
		k[4] = 0;
		k[5] = 0;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}
	T_ASSERT(radix_tree_is_well_formed(rt));

	memset(k, 0, sizeof(k));
	for (i = 0x38; i < 0x69; i++) {
		k[0] = i - 0x32;
		k[4] = 0xf;
		k[5] = 1;
		T_ASSERT(radix_tree_remove(rt, k, k + sizeof(k)));

		k[0] = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));

		k[0] = i - 0x32;
		k[4] = 0;
		k[5] = 0;
		T_ASSERT(radix_tree_remove(rt, k, k + sizeof(k)));

		k[0] = i;
		T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	}
	T_ASSERT(radix_tree_is_well_formed(rt));

	memset(k, 0, sizeof(k));
	k[0] = 0x6;
	radix_tree_remove_prefix(rt, k, k + 4);
	T_ASSERT(radix_tree_is_well_formed(rt));

	k[0] = 0x38;
	k[4] = 0xf;
	k[5] = 0x1;
	T_ASSERT(radix_tree_remove(rt, k, k + sizeof(k)));
	T_ASSERT(radix_tree_is_well_formed(rt));

	memset(k, 0, sizeof(k));
	k[0] = 0x6;
	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	T_ASSERT(radix_tree_is_well_formed(rt));

	k[0] = 0x7;
	radix_tree_remove_prefix(rt, k, k + 4);
	T_ASSERT(radix_tree_is_well_formed(rt));

	k[0] = 0x38;
	T_ASSERT(radix_tree_remove(rt, k, k + sizeof(k)));
	T_ASSERT(radix_tree_is_well_formed(rt));

	k[0] = 7;
	T_ASSERT(radix_tree_insert(rt, k, k + sizeof(k), v));
	T_ASSERT(radix_tree_is_well_formed(rt));
}

//----------------------------------------------------------------

struct key_parts {
	uint32_t fd;
	uint64_t b;
} __attribute__ ((packed));

union key {
	struct key_parts parts;
        uint8_t bytes[12];
};

static void __lookup_matches(struct radix_tree *rt, int fd, uint64_t b, uint64_t expected)
{
	union key k;
	union radix_value v;

	k.parts.fd = fd;
	k.parts.b = b;
	T_ASSERT(radix_tree_lookup(rt, k.bytes, k.bytes + sizeof(k.bytes), &v));
	T_ASSERT(v.n == expected);
}

static void __lookup_fails(struct radix_tree *rt, int fd, uint64_t b)
{
	union key k;
	union radix_value v;

	k.parts.fd = fd;
	k.parts.b = b;
	T_ASSERT(!radix_tree_lookup(rt, k.bytes, k.bytes + sizeof(k.bytes), &v));
}

static void __insert(struct radix_tree *rt, int fd, uint64_t b, uint64_t n)
{
	union key k;
	union radix_value v;

	k.parts.fd = fd;
	k.parts.b = b;
	v.n = n;
	T_ASSERT(radix_tree_insert(rt, k.bytes, k.bytes + sizeof(k.bytes), v));
}

static void __invalidate(struct radix_tree *rt, int fd)
{
	union key k;

	k.parts.fd = fd;
	radix_tree_remove_prefix(rt, k.bytes, k.bytes + sizeof(k.parts.fd));
	radix_tree_is_well_formed(rt);
}

static void test_bcache_scenario3(void *fixture)
{
	struct radix_tree *rt = fixture;

	#include "test/unit/rt_case1.c"
}

//----------------------------------------------------------------
#define T(path, desc, fn) register_test(ts, "/base/data-struct/radix-tree/" path, desc, fn)

void radix_tree_tests(struct dm_list *all_tests)
{
	struct test_suite *ts = test_suite_create(rt_init, rt_exit);
	if (!ts) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

	T("create-destroy", "create and destroy an empty tree", test_create_destroy);
	T("insert-one", "insert one trivial trivial key", test_insert_one);
	T("insert-single-byte-keys", "inserts many single byte keys", test_single_byte_keys);
	T("overwrite-single-byte-keys", "overwrite many single byte keys", test_overwrite_single_byte_keys);
	T("insert-16-bit-keys", "insert many 16bit keys", test_16_bit_keys);
	T("prefix-keys", "prefixes of other keys are valid keys", test_prefix_keys);
	T("prefix-keys-reversed", "prefixes of other keys are valid keys", test_prefix_keys_reversed);
	T("sparse-keys", "see what the memory usage is for sparsely distributed keys", test_sparse_keys);
	T("remove-one", "remove one entry", test_remove_one);
	T("remove-one-byte-keys", "remove many one byte keys", test_remove_one_byte_keys);
	T("remove-one-byte-keys-reversed", "remove many one byte keys reversed", test_remove_one_byte_keys_reversed);
	T("remove-prefix-keys", "remove a set of keys that have common prefixes", test_remove_prefix_keys);
	T("remove-prefix-keys-reversed", "remove a set of keys that have common prefixes (reversed)", test_remove_prefix_keys_reversed);
	T("remove-prefix", "remove a subrange", test_remove_prefix);
	T("remove-prefix-single", "remove a subrange with a single entry", test_remove_prefix_single);
	T("size-spots-duplicates", "duplicate entries aren't counted twice", test_size);
	T("iterate-all", "iterate all entries in tree", test_iterate_all);
	T("iterate-subset", "iterate a subset of entries in tree", test_iterate_subset);
	T("iterate-single", "iterate a subset that contains a single entry", test_iterate_single);
	T("iterate-vary-middle", "iterate keys that vary in the middle", test_iterate_vary_middle);
	T("remove-calls-dtr", "remove should call the dtr for the value", test_remove_calls_dtr);
	T("destroy-calls-dtr", "destroy should call the dtr for all values", test_destroy_calls_dtr);
	T("bcache-scenario", "A specific series of keys from a bcache scenario", test_bcache_scenario);
	T("bcache-scenario-2", "A second series of keys from a bcache scenario", test_bcache_scenario2);
	T("bcache-scenario-3", "A third series of keys from a bcache scenario", test_bcache_scenario3);

	dm_list_add(all_tests, &ts->list);
}
//----------------------------------------------------------------
