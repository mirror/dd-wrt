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
 
#ifndef BASE_DATA_STRUCT_RADIX_TREE_H
#define BASE_DATA_STRUCT_RADIX_TREE_H

#include "configure.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

//----------------------------------------------------------------

struct radix_tree;

union radix_value {
	void *ptr;
	uint64_t n;
};

typedef void (*radix_value_dtr)(void *context, union radix_value v);

// dtr will be called on any deleted entries.  dtr may be NULL.
struct radix_tree *radix_tree_create(radix_value_dtr dtr, void *dtr_context);
void radix_tree_destroy(struct radix_tree *rt);

unsigned radix_tree_size(struct radix_tree *rt);
bool radix_tree_insert(struct radix_tree *rt, uint8_t *kb, uint8_t *ke, union radix_value v);
bool radix_tree_remove(struct radix_tree *rt, uint8_t *kb, uint8_t *ke);

// Returns the number of values removed
unsigned radix_tree_remove_prefix(struct radix_tree *rt, uint8_t *prefix_b, uint8_t *prefix_e);

bool radix_tree_lookup(struct radix_tree *rt,
		       uint8_t *kb, uint8_t *ke, union radix_value *result);

// The radix tree stores entries in lexicographical order.  Which means
// we can iterate entries, in order.  Or iterate entries with a particular
// prefix.
struct radix_tree_iterator {
        // Returns false if the iteration should end.
	bool (*visit)(struct radix_tree_iterator *it,
                      uint8_t *kb, uint8_t *ke, union radix_value v);
};

void radix_tree_iterate(struct radix_tree *rt, uint8_t *kb, uint8_t *ke,
                        struct radix_tree_iterator *it);

// Checks that some constraints on the shape of the tree are
// being held.  For debug only.
bool radix_tree_is_well_formed(struct radix_tree *rt);
void radix_tree_dump(struct radix_tree *rt, FILE *out);

//----------------------------------------------------------------

#endif
