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

#include "radix-tree.h"

#include "base/memory/container_of.h"
#include "base/memory/zalloc.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

//----------------------------------------------------------------
// This implementation is based around nested binary trees.  Very
// simple (and hopefully correct).

struct node {
	struct node *left;
	struct node *right;

	uint8_t key;
	struct node *center;

	bool has_value;
	union radix_value value;
};

struct radix_tree {
	radix_value_dtr dtr;
	void *dtr_context;

	struct node *root;
};

struct radix_tree *
radix_tree_create(radix_value_dtr dtr, void *dtr_context)
{
	struct radix_tree *rt = zalloc(sizeof(*rt));

	if (rt) {
		rt->dtr = dtr;
		rt->dtr_context = dtr_context;
	}

	return rt;
}

// Returns the number of entries in the tree
static unsigned _destroy_tree(struct node *n, radix_value_dtr dtr, void *context)
{
	unsigned r;

	if (!n)
		return 0;

	r = _destroy_tree(n->left, dtr, context);
	r += _destroy_tree(n->right, dtr, context);
	r += _destroy_tree(n->center, dtr, context);

	if (n->has_value) {
		if (dtr)
			dtr(context, n->value);
		r++;
	}

	free(n);

	return r;
}

void radix_tree_destroy(struct radix_tree *rt)
{
	_destroy_tree(rt->root, rt->dtr, rt->dtr_context);
	free(rt);
}

static unsigned _count(struct node *n)
{
	unsigned r;

	if (!n)
		return 0;

	r = _count(n->left);
	r += _count(n->right);
	r += _count(n->center);

	if (n->has_value)
		r++;

	return r;
}

unsigned radix_tree_size(struct radix_tree *rt)
{
	return _count(rt->root);
}

static struct node **_lookup(struct node **pn, uint8_t *kb, uint8_t *ke)
{
	struct node *n = *pn;

	if (!n || (kb == ke))
		return pn;

	if (*kb < n->key)
		return _lookup(&n->left, kb, ke);

	else if (*kb > n->key)
		return _lookup(&n->right, kb, ke);

	else
		return _lookup(&n->center, kb + 1, ke);
}

static bool _insert(struct node **pn, uint8_t *kb, uint8_t *ke, union radix_value v)
{
	struct node *n = *pn;

	if (!n) {
		n = zalloc(sizeof(*n));
		if (!n)
			return false;

		n->key = *kb;
		*pn = n;
	}

	if (kb == ke) {
		n->has_value = true;
		n->value = v;
		return true;
	}

	if (*kb < n->key)
		return _insert(&n->left, kb, ke, v);

	else if (*kb > n->key)
		return _insert(&n->right, kb, ke, v);

	else
		return _insert(&n->center, kb + 1, ke, v);
}

bool radix_tree_insert(struct radix_tree *rt, uint8_t *kb, uint8_t *ke, union radix_value v)
{
	return _insert(&rt->root, kb, ke, v);
}

bool radix_tree_remove(struct radix_tree *rt, uint8_t *kb, uint8_t *ke)
{
	struct node **pn = _lookup(&rt->root, kb, ke);
	struct node *n = *pn;

	if (!n || !n->has_value)
		return false;

	else {
		if (rt->dtr)
			rt->dtr(rt->dtr_context, n->value);

		if (n->left || n->center || n->right) {
			n->has_value = false;
			return true;

		} else {
			// FIXME: delete parent if this was the last entry
			free(n);
			*pn = NULL;
		}

		return true;
	}
}

unsigned radix_tree_remove_prefix(struct radix_tree *rt, uint8_t *kb, uint8_t *ke)
{
	struct node **pn;
	unsigned count;

	pn = _lookup(&rt->root, kb, ke);

	if (*pn) {
		count = _destroy_tree(*pn, rt->dtr, rt->dtr_context);
		*pn = NULL;
	}

	return count;
}

bool
radix_tree_lookup(struct radix_tree *rt, uint8_t *kb, uint8_t *ke, union radix_value *result)
{
	struct node **pn = _lookup(&rt->root, kb, ke);
	struct node *n = *pn;

	if (n && n->has_value) {
		*result = n->value;
		return true;
	} else
		return false;
}

static void _iterate(struct node *n, struct radix_tree_iterator *it)
{
	if (!n)
		return;

	_iterate(n->left, it);

	if (n->has_value)
		// FIXME: fill out the key
		it->visit(it, NULL, NULL, n->value);

	_iterate(n->center, it);
	_iterate(n->right, it);
}

void radix_tree_iterate(struct radix_tree *rt, uint8_t *kb, uint8_t *ke,
                        struct radix_tree_iterator *it)
{
	if (kb == ke)
		_iterate(rt->root, it);

	else {
		struct node **pn = _lookup(&rt->root, kb, ke);
		struct node *n = *pn;

		if (n) {
			if (n->has_value)
				it->visit(it, NULL, NULL, n->value);
			_iterate(n->center, it);
		}
	}
}

bool radix_tree_is_well_formed(struct radix_tree *rt)
{
	return true;
}

void radix_tree_dump(struct radix_tree *rt, FILE *out)
{
}

//----------------------------------------------------------------

