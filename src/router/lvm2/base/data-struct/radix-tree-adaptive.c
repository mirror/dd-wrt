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
#include <string.h>

//----------------------------------------------------------------

enum node_type {
	UNSET = 0,
	VALUE,
	VALUE_CHAIN,
	PREFIX_CHAIN,
	NODE4,
	NODE16,
	NODE48,
	NODE256
};

struct value {
	enum node_type type;
	union radix_value value;
};

// This is used for entries that have a key which is a prefix of another key.
struct value_chain {
	union radix_value value;
	struct value child;
};

struct prefix_chain {
	struct value child;
	unsigned len;
	uint8_t prefix[0];
};

struct node4 {
	uint32_t nr_entries;
	uint8_t keys[4];
	struct value values[4];
};

struct node16 {
	uint32_t nr_entries;
	uint8_t keys[16];
	struct value values[16];
};

struct node48 {
	uint32_t nr_entries;
	uint8_t keys[256];
	struct value values[48];
};

struct node256 {
        uint32_t nr_entries;
	struct value values[256];
};

struct radix_tree {
	unsigned nr_entries;
	struct value root;
	radix_value_dtr dtr;
	void *dtr_context;
};

//----------------------------------------------------------------

struct radix_tree *radix_tree_create(radix_value_dtr dtr, void *dtr_context)
{
	struct radix_tree *rt = malloc(sizeof(*rt));

	if (rt) {
		rt->nr_entries = 0;
		rt->root.type = UNSET;
		rt->dtr = dtr;
		rt->dtr_context = dtr_context;
	}

	return rt;
}

static inline void _dtr(struct radix_tree *rt, union radix_value v)
{
	if (rt->dtr)
        	rt->dtr(rt->dtr_context, v);
}

// Returns the number of values removed
static unsigned _free_node(struct radix_tree *rt, struct value v)
{
	unsigned i, nr = 0;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	switch (v.type) {
	case UNSET:
		break;

	case VALUE:
        	_dtr(rt, v.value);
        	nr = 1;
		break;

	case VALUE_CHAIN:
		vc = v.value.ptr;
		_dtr(rt, vc->value);
		nr = 1 + _free_node(rt, vc->child);
		free(vc);
		break;

	case PREFIX_CHAIN:
		pc = v.value.ptr;
		nr = _free_node(rt, pc->child);
		free(pc);
		break;

	case NODE4:
		n4 = (struct node4 *) v.value.ptr;
		for (i = 0; i < n4->nr_entries; i++)
			nr += _free_node(rt, n4->values[i]);
		free(n4);
		break;

	case NODE16:
		n16 = (struct node16 *) v.value.ptr;
		for (i = 0; i < n16->nr_entries; i++)
			nr += _free_node(rt, n16->values[i]);
		free(n16);
		break;

	case NODE48:
		n48 = (struct node48 *) v.value.ptr;
		for (i = 0; i < n48->nr_entries; i++)
			nr += _free_node(rt, n48->values[i]);
		free(n48);
		break;

	case NODE256:
		n256 = (struct node256 *) v.value.ptr;
		for (i = 0; i < 256; i++)
			nr += _free_node(rt, n256->values[i]);
		free(n256);
		break;
	}

	return nr;
}

void radix_tree_destroy(struct radix_tree *rt)
{
	_free_node(rt, rt->root);
	free(rt);
}

unsigned radix_tree_size(struct radix_tree *rt)
{
	return rt->nr_entries;
}

static bool _insert(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv);

static bool _insert_unset(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	unsigned len = ke - kb;

	if (!len) {
		// value
		v->type = VALUE;
		v->value = rv;
		rt->nr_entries++;
	} else {
		// prefix -> value
		struct prefix_chain *pc = zalloc(sizeof(*pc) + len);
		if (!pc)
			return false;

		pc->child.type = VALUE;
		pc->child.value = rv;
		pc->len = len;
		memcpy(pc->prefix, kb, len);
		v->type = PREFIX_CHAIN;
		v->value.ptr = pc;
		rt->nr_entries++;
	}

	return true;
}

static bool _insert_value(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	unsigned len = ke - kb;

	if (!len)
		// overwrite
		v->value = rv;

	else {
		// value_chain -> value
		struct value_chain *vc = zalloc(sizeof(*vc));
		if (!vc)
			return false;

		vc->value = v->value;
		if (!_insert(rt, &vc->child, kb, ke, rv)) {
			free(vc);
			return false;
		}

		v->type = VALUE_CHAIN;
		v->value.ptr = vc;
	}

	return true;
}

static bool _insert_value_chain(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct value_chain *vc = v->value.ptr;
	return _insert(rt, &vc->child, kb, ke, rv);
}

static unsigned min(unsigned lhs, unsigned rhs)
{
	if (lhs <= rhs)
		return lhs;
	else
		return rhs;
}

static bool _insert_prefix_chain(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct prefix_chain *pc = v->value.ptr;

	if (!pc->len) {
		v->type = VALUE;
		v->value = rv;

	} else if (*kb == pc->prefix[0]) {
		// There's a common prefix let's split the chain into two and
		// recurse.
		struct prefix_chain *pc2;
		unsigned i, len = min(pc->len, ke - kb);

		for (i = 0; i < len; i++)
			if (kb[i] != pc->prefix[i])
				break;

		if (!(pc2 = zalloc(sizeof(*pc2) + pc->len - i)))
			return false;
		pc2->len = pc->len - i;
		memmove(pc2->prefix, pc->prefix + i, pc2->len);
		pc2->child = pc->child;

		// FIXME: this trashes pc so we can't back out
		pc->child.type = PREFIX_CHAIN;
		pc->child.value.ptr = pc2;
		pc->len = i;

		if (!_insert(rt, &pc->child, kb + i, ke, rv)) {
			free(pc2);
			return false;
		}

	} else {
		// Stick an n4 in front.
		struct node4 *n4 = zalloc(sizeof(*n4));
		if (!n4)
			return false;

		n4->keys[0] = pc->prefix[0];
		if (pc->len == 1) {
			n4->values[0] = pc->child;
			free(pc);
		} else {
			memmove(pc->prefix, pc->prefix + 1, pc->len - 1);
			pc->len--;
			n4->values[0] = *v;
		}

		n4->keys[1] = *kb;
		if (!_insert(rt, n4->values + 1, kb + 1, ke, rv)) {
			free(n4);
			return false;
		}

		n4->nr_entries = 2;

		v->type = NODE4;
		v->value.ptr = n4;
	}

	return true;
}

static bool _insert_node4(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct node4 *n4 = v->value.ptr;
	if (n4->nr_entries == 4) {
		struct node16 *n16 = zalloc(sizeof(*n16));
		if (!n16)
			return false;

		n16->nr_entries = 5;
		memcpy(n16->keys, n4->keys, sizeof(n4->keys));
		memcpy(n16->values, n4->values, sizeof(n4->values));

		n16->keys[4] = *kb;
		if (!_insert(rt, n16->values + 4, kb + 1, ke, rv)) {
			free(n16);
			return false;
		}
		free(n4);
		v->type = NODE16;
		v->value.ptr = n16;
	} else {
		if (!_insert(rt, n4->values + n4->nr_entries, kb + 1, ke, rv))
			return false;

		n4->keys[n4->nr_entries] = *kb;
		n4->nr_entries++;
	}
	return true;
}

static bool _insert_node16(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct node16 *n16 = v->value.ptr;

	if (n16->nr_entries == 16) {
		unsigned i;
		struct node48 *n48 = zalloc(sizeof(*n48));

		if (!n48)
			return false;

		n48->nr_entries = 17;
		/* coverity[bad_memset] intentional use of '0' */
		memset(n48->keys, 48, sizeof(n48->keys));

		for (i = 0; i < 16; i++) {
			n48->keys[n16->keys[i]] = i;
			n48->values[i] = n16->values[i];
		}

		n48->keys[*kb] = 16;
		if (!_insert(rt, n48->values + 16, kb + 1, ke, rv)) {
			free(n48);
			return false;
		}

		free(n16);
		v->type = NODE48;
		v->value.ptr = n48;
	} else {
		if (!_insert(rt, n16->values + n16->nr_entries, kb + 1, ke, rv))
			return false;
		n16->keys[n16->nr_entries] = *kb;
		n16->nr_entries++;
	}

	return true;
}

static bool _insert_node48(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct node48 *n48 = v->value.ptr;
	if (n48->nr_entries == 48) {
		unsigned i;
		struct node256 *n256 = zalloc(sizeof(*n256));
		if (!n256)
			return false;

		n256->nr_entries = 49;
		for (i = 0; i < 256; i++) {
			if (n48->keys[i] < 48)
				n256->values[i] = n48->values[n48->keys[i]];
		}

		if (!_insert(rt, n256->values + *kb, kb + 1, ke, rv)) {
			free(n256);
			return false;
		}

		free(n48);
		v->type = NODE256;
		v->value.ptr = n256;

	} else {
		if (!_insert(rt, n48->values + n48->nr_entries, kb + 1, ke, rv))
			return false;

		n48->keys[*kb] = n48->nr_entries;
		n48->nr_entries++;
	}

	return true;
}

static bool _insert_node256(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct node256 *n256 = v->value.ptr;
	bool r, was_unset = n256->values[*kb].type == UNSET;

	r = _insert(rt, n256->values + *kb, kb + 1, ke, rv);
	if (r && was_unset)
        	n256->nr_entries++;

	return r;
}

// FIXME: the tree should not be touched if insert fails (eg, OOM)
static bool _insert(struct radix_tree *rt, struct value *v, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	if (kb == ke) {
		if (v->type == UNSET) {
			v->type = VALUE;
			v->value = rv;
			rt->nr_entries++;

		} else if (v->type == VALUE) {
			v->value = rv;

		} else {
			struct value_chain *vc = zalloc(sizeof(*vc));
			if (!vc)
				return false;

			vc->value = rv;
			vc->child = *v;
			v->type = VALUE_CHAIN;
			v->value.ptr = vc;
			rt->nr_entries++;
		}
		return true;
	}

	switch (v->type) {
	case UNSET:
		return _insert_unset(rt, v, kb, ke, rv);

	case VALUE:
		return _insert_value(rt, v, kb, ke, rv);

	case VALUE_CHAIN:
		return _insert_value_chain(rt, v, kb, ke, rv);

	case PREFIX_CHAIN:
		return _insert_prefix_chain(rt, v, kb, ke, rv);

	case NODE4:
		return _insert_node4(rt, v, kb, ke, rv);

	case NODE16:
		return _insert_node16(rt, v, kb, ke, rv);

	case NODE48:
		return _insert_node48(rt, v, kb, ke, rv);

	case NODE256:
		return _insert_node256(rt, v, kb, ke, rv);
	}

	// can't get here
	return false;
}

struct lookup_result {
	struct value *v;
	uint8_t *kb;
};

static struct lookup_result _lookup_prefix(struct value *v, uint8_t *kb, uint8_t *ke)
{
	unsigned i;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	if (kb == ke)
		return (struct lookup_result) {.v = v, .kb = kb};

	switch (v->type) {
	case UNSET:
	case VALUE:
		break;

	case VALUE_CHAIN:
		vc = v->value.ptr;
		return _lookup_prefix(&vc->child, kb, ke);

	case PREFIX_CHAIN:
		pc = v->value.ptr;
		if (ke - kb < pc->len)
			return (struct lookup_result) {.v = v, .kb = kb};

		for (i = 0; i < pc->len; i++)
			if (kb[i] != pc->prefix[i])
				return (struct lookup_result) {.v = v, .kb = kb};

		return _lookup_prefix(&pc->child, kb + pc->len, ke);

	case NODE4:
		n4 = v->value.ptr;
		for (i = 0; i < n4->nr_entries; i++)
			if (n4->keys[i] == *kb)
				return _lookup_prefix(n4->values + i, kb + 1, ke);
		break;

	case NODE16:
		// FIXME: use binary search or simd?
		n16 = v->value.ptr;
		for (i = 0; i < n16->nr_entries; i++)
			if (n16->keys[i] == *kb)
				return _lookup_prefix(n16->values + i, kb + 1, ke);
		break;

	case NODE48:
		n48 = v->value.ptr;
		i = n48->keys[*kb];
		if (i < 48)
			return _lookup_prefix(n48->values + i, kb + 1, ke);
		break;

	case NODE256:
		n256 = v->value.ptr;
		if (n256->values[*kb].type != UNSET)
			return _lookup_prefix(n256->values + *kb, kb + 1, ke);
		break;
	}

	return (struct lookup_result) {.v = v, .kb = kb};
}

bool radix_tree_insert(struct radix_tree *rt, uint8_t *kb, uint8_t *ke, union radix_value rv)
{
	struct lookup_result lr = _lookup_prefix(&rt->root, kb, ke);
	return _insert(rt, lr.v, lr.kb, ke, rv);
}

// Note the degrade functions also free the original node.
static void _degrade_to_n4(struct node16 *n16, struct value *result)
{
        struct node4 *n4 = zalloc(sizeof(*n4));

	assert(n4 != NULL);

        n4->nr_entries = n16->nr_entries;
        memcpy(n4->keys, n16->keys, n16->nr_entries * sizeof(*n4->keys));
        memcpy(n4->values, n16->values, n16->nr_entries * sizeof(*n4->values));
        free(n16);

	result->type = NODE4;
	result->value.ptr = n4;
}

static void _degrade_to_n16(struct node48 *n48, struct value *result)
{
	unsigned i, count = 0;
        struct node16 *n16 = zalloc(sizeof(*n16));

	assert(n16 != NULL);

        n16->nr_entries = n48->nr_entries;
        for (i = 0; i < 256; i++) {
	        if (n48->keys[i] < 48) {
		        n16->keys[count] = i;
		        n16->values[count] = n48->values[n48->keys[i]];
		        count++;
	        }
        }

        free(n48);

	result->type = NODE16;
	result->value.ptr = n16;
}

static void _degrade_to_n48(struct node256 *n256, struct value *result)
{
        unsigned i, count = 0;
        struct node48 *n48 = zalloc(sizeof(*n48));

	assert(n48 != NULL);

        n48->nr_entries = n256->nr_entries;
        for (i = 0; i < 256; i++) {
		if (n256->values[i].type == UNSET)
			n48->keys[i] = 48;

		else {
			n48->keys[i] = count;
			n48->values[count] = n256->values[i];
			count++;
		}
        }

        free(n256);

	result->type = NODE48;
	result->value.ptr = n48;
}

// Removes an entry in an array by sliding the values above it down.
static void _erase_elt(void *array, size_t obj_size, unsigned count, unsigned idx)
{
	if (idx == (count - 1))
		// The simple case
		return;

	memmove(((uint8_t *) array) + (obj_size * idx),
                ((uint8_t *) array) + (obj_size * (idx + 1)),
                obj_size * (count - idx - 1));

	// Zero the now unused last elt (set's v.type to UNSET)
	memset(((uint8_t *) array) + (count - 1) * obj_size, 0, obj_size);
}

static bool _remove(struct radix_tree *rt, struct value *root, uint8_t *kb, uint8_t *ke)
{
	bool r;
	unsigned i, j;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	if (kb == ke) {
        	if (root->type == VALUE) {
                	root->type = UNSET;
                	_dtr(rt, root->value);
                	return true;

                } else if (root->type == VALUE_CHAIN) {
			vc = root->value.ptr;
			_dtr(rt, vc->value);
			memcpy(root, &vc->child, sizeof(*root));
			free(vc);
			return true;

                } else
			return false;
	}

	switch (root->type) {
	case UNSET:
	case VALUE:
        	// this is a value for a prefix of the key
        	return false;

	case VALUE_CHAIN:
		vc = root->value.ptr;
		r = _remove(rt, &vc->child, kb, ke);
		if (r && (vc->child.type == UNSET)) {
			root->type = VALUE;
			root->value = vc->value;
			free(vc);
		}
		return r;

	case PREFIX_CHAIN:
		pc = root->value.ptr;
		if (ke - kb < pc->len)
        		return false;

		for (i = 0; i < pc->len; i++)
			if (kb[i] != pc->prefix[i])
        			return false;

		r = _remove(rt, &pc->child, kb + pc->len, ke);
		if (r && pc->child.type == UNSET) {
			root->type = UNSET;
			free(pc);
		}
		return r;

	case NODE4:
		n4 = root->value.ptr;
		for (i = 0; i < n4->nr_entries; i++) {
			if (n4->keys[i] == *kb) {
				r = _remove(rt, n4->values + i, kb + 1, ke);
				if (r && n4->values[i].type == UNSET) {
        				if (i < n4->nr_entries) {
	        				_erase_elt(n4->keys, sizeof(*n4->keys), n4->nr_entries, i);
	        				_erase_elt(n4->values, sizeof(*n4->values), n4->nr_entries, i);
        				}

        				n4->nr_entries--;
					if (!n4->nr_entries) {
						free(n4);
						root->type = UNSET;
					}
				}
				return r;
			}
		}
		return false;

	case NODE16:
        	n16 = root->value.ptr;
		for (i = 0; i < n16->nr_entries; i++) {
			if (n16->keys[i] == *kb) {
				r = _remove(rt, n16->values + i, kb + 1, ke);
				if (r && n16->values[i].type == UNSET) {
        				if (i < n16->nr_entries) {
	        				_erase_elt(n16->keys, sizeof(*n16->keys), n16->nr_entries, i);
	        				_erase_elt(n16->values, sizeof(*n16->values), n16->nr_entries, i);
        				}

        				n16->nr_entries--;
					if (n16->nr_entries <= 4) {
        					_degrade_to_n4(n16, root);
					}
				}
				return r;
			}
		}
		return false;

	case NODE48:
		n48 = root->value.ptr;
		i = n48->keys[*kb];
		if (i < 48) {
        		r = _remove(rt, n48->values + i, kb + 1, ke);
        		if (r && n48->values[i].type == UNSET) {
                		n48->keys[*kb] = 48;
                		for (j = 0; j < 256; j++)
	                		if (n48->keys[j] < 48 && n48->keys[j] > i)
		                		n48->keys[j]--;
				_erase_elt(n48->values, sizeof(*n48->values), n48->nr_entries, i);
				n48->nr_entries--;
				if (n48->nr_entries <= 16)
        				_degrade_to_n16(n48, root);
        		}
        		return r;
		}
		return false;

	case NODE256:
		n256 = root->value.ptr;
		r = _remove(rt, n256->values + (*kb), kb + 1, ke);
		if (r && n256->values[*kb].type == UNSET) {
			n256->nr_entries--;
			if (n256->nr_entries <= 48)
        			_degrade_to_n48(n256, root);
		}
		return r;
	}

	return false;
}

bool radix_tree_remove(struct radix_tree *rt, uint8_t *key_begin, uint8_t *key_end)
{
	if (_remove(rt, &rt->root, key_begin, key_end)) {
        	rt->nr_entries--;
        	return true;
	}

	return false;
}

//----------------------------------------------------------------

static bool _prefix_chain_matches(struct lookup_result *lr, uint8_t *ke)
{
        // It's possible the top node is a prefix chain, and
        // the remaining key matches part of it.
        if (lr->v->type == PREFIX_CHAIN) {
                unsigned i, rlen = ke - lr->kb;
                struct prefix_chain *pc = lr->v->value.ptr;
                if (rlen < pc->len) {
                        for (i = 0; i < rlen; i++)
                                if (pc->prefix[i] != lr->kb[i])
                                        return false;
                        return true;
		}
        }

        return false;
}

static bool _remove_subtree(struct radix_tree *rt, struct value *root, uint8_t *kb, uint8_t *ke, unsigned *count)
{
	bool r;
	unsigned i, j, len;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	if (kb == ke) {
		*count += _free_node(rt, *root);
		root->type = UNSET;
		return true;
	}

	switch (root->type) {
	case UNSET:
	case VALUE:
		// No entries with the given prefix
        	return true;

	case VALUE_CHAIN:
		vc = root->value.ptr;
		r = _remove_subtree(rt, &vc->child, kb, ke, count);
		if (r && (vc->child.type == UNSET)) {
			root->type = VALUE;
			root->value = vc->value;
			free(vc);
		}
		return r;

	case PREFIX_CHAIN:
		pc = root->value.ptr;
		len = min(pc->len, ke - kb);
		for (i = 0; i < len; i++)
			if (kb[i] != pc->prefix[i])
        			return true;

		r = _remove_subtree(rt, &pc->child, len < pc->len ? ke : (kb + pc->len), ke, count);
		if (r && pc->child.type == UNSET) {
			root->type = UNSET;
			free(pc);
		}
		return r;

	case NODE4:
		n4 = root->value.ptr;
		for (i = 0; i < n4->nr_entries; i++) {
			if (n4->keys[i] == *kb) {
				r = _remove_subtree(rt, n4->values + i, kb + 1, ke, count);
				if (r && n4->values[i].type == UNSET) {
        				if (i < n4->nr_entries) {
	        				_erase_elt(n4->keys, sizeof(*n4->keys), n4->nr_entries, i);
	        				_erase_elt(n4->values, sizeof(*n4->values), n4->nr_entries, i);
        				}

        				n4->nr_entries--;
					if (!n4->nr_entries) {
						free(n4);
						root->type = UNSET;
					}
				}
				return r;
			}
		}
		return true;

	case NODE16:
        	n16 = root->value.ptr;
		for (i = 0; i < n16->nr_entries; i++) {
			if (n16->keys[i] == *kb) {
				r = _remove_subtree(rt, n16->values + i, kb + 1, ke, count);
				if (r && n16->values[i].type == UNSET) {
        				if (i < n16->nr_entries) {
	        				_erase_elt(n16->keys, sizeof(*n16->keys), n16->nr_entries, i);
	        				_erase_elt(n16->values, sizeof(*n16->values), n16->nr_entries, i);
        				}

        				n16->nr_entries--;
					if (n16->nr_entries <= 4)
        					_degrade_to_n4(n16, root);
				}
				return r;
			}
		}
		return true;

	case NODE48:
		n48 = root->value.ptr;
		i = n48->keys[*kb];
		if (i < 48) {
        		r = _remove_subtree(rt, n48->values + i, kb + 1, ke, count);
        		if (r && n48->values[i].type == UNSET) {
                		n48->keys[*kb] = 48;
                		for (j = 0; j < 256; j++)
	                		if (n48->keys[j] < 48 && n48->keys[j] > i)
		                		n48->keys[j]--;
				_erase_elt(n48->values, sizeof(*n48->values), n48->nr_entries, i);
				n48->nr_entries--;
				if (n48->nr_entries <= 16)
        				_degrade_to_n16(n48, root);
        		}
        		return r;
		}
		return true;

	case NODE256:
		n256 = root->value.ptr;
		if (n256->values[*kb].type == UNSET)
			return true;  // No entries

		r = _remove_subtree(rt, n256->values + (*kb), kb + 1, ke, count);
		if (r && n256->values[*kb].type == UNSET) {
			n256->nr_entries--;
			if (n256->nr_entries <= 48)
        			_degrade_to_n48(n256, root);
		}
		return r;
	}

	// Shouldn't get here
	return false;
}

unsigned radix_tree_remove_prefix(struct radix_tree *rt, uint8_t *kb, uint8_t *ke)
{
        unsigned count = 0;

        if (_remove_subtree(rt, &rt->root, kb, ke, &count))
		rt->nr_entries -= count;

	return count;
}

//----------------------------------------------------------------

bool radix_tree_lookup(struct radix_tree *rt,
		       uint8_t *kb, uint8_t *ke, union radix_value *result)
{
	struct value_chain *vc;
	struct lookup_result lr = _lookup_prefix(&rt->root, kb, ke);
	if (lr.kb == ke) {
		switch (lr.v->type) {
		case VALUE:
			*result = lr.v->value;
			return true;

		case VALUE_CHAIN:
			vc = lr.v->value.ptr;
			*result = vc->value;
			return true;

		default:
			return false;
		}
	}

	return false;
}

// FIXME: build up the keys too
static bool _iterate(struct value *v, struct radix_tree_iterator *it)
{
	unsigned i;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	switch (v->type) {
	case UNSET:
        	// can't happen
		break;

	case VALUE:
        	return it->visit(it, NULL, NULL, v->value);

	case VALUE_CHAIN:
		vc = v->value.ptr;
		return it->visit(it, NULL, NULL, vc->value) && _iterate(&vc->child, it);

	case PREFIX_CHAIN:
		pc = v->value.ptr;
		return _iterate(&pc->child, it);

	case NODE4:
		n4 = (struct node4 *) v->value.ptr;
		for (i = 0; i < n4->nr_entries; i++)
			if (!_iterate(n4->values + i, it))
        			return false;
        	return true;

	case NODE16:
		n16 = (struct node16 *) v->value.ptr;
		for (i = 0; i < n16->nr_entries; i++)
        		if (!_iterate(n16->values + i, it))
        			return false;
		return true;

	case NODE48:
		n48 = (struct node48 *) v->value.ptr;
		for (i = 0; i < n48->nr_entries; i++)
        		if (!_iterate(n48->values + i, it))
        			return false;
		return true;

	case NODE256:
		n256 = (struct node256 *) v->value.ptr;
		for (i = 0; i < 256; i++)
        		if (n256->values[i].type != UNSET && !_iterate(n256->values + i, it))
        			return false;
		return true;
	}

	// can't get here
	return false;
}

void radix_tree_iterate(struct radix_tree *rt, uint8_t *kb, uint8_t *ke,
                        struct radix_tree_iterator *it)
{
	struct lookup_result lr = _lookup_prefix(&rt->root, kb, ke);
	if (lr.kb == ke || _prefix_chain_matches(&lr, ke))
        	_iterate(lr.v, it);
}

//----------------------------------------------------------------
// Checks:
// 1) The number of entries matches rt->nr_entries
// 2) The number of entries is correct in each node
// 3) prefix chain len > 0
// 4) all unused values are UNSET

static bool _check_nodes(struct value *v, unsigned *count)
{
	uint64_t bits;
	unsigned i, ncount;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	switch (v->type) {
	case UNSET:
		return true;

	case VALUE:
		(*count)++;
		return true;

	case VALUE_CHAIN:
		(*count)++;
		vc = v->value.ptr;
		return _check_nodes(&vc->child, count);

	case PREFIX_CHAIN:
		pc = v->value.ptr;
		return _check_nodes(&pc->child, count);

	case NODE4:
		n4 = v->value.ptr;
		for (i = 0; i < n4->nr_entries; i++)
			if (!_check_nodes(n4->values + i, count))
				return false;

		for (i = n4->nr_entries; i < 4; i++)
			if (n4->values[i].type != UNSET) {
				fprintf(stderr, "unused value is not UNSET (n4)\n");
				return false;
			}

		return true;

	case NODE16:
		n16 = v->value.ptr;
		for (i = 0; i < n16->nr_entries; i++)
			if (!_check_nodes(n16->values + i, count))
				return false;

		for (i = n16->nr_entries; i < 16; i++)
			if (n16->values[i].type != UNSET) {
				fprintf(stderr, "unused value is not UNSET (n16)\n");
				return false;
			}

		return true;

	case NODE48:
		bits = 0;
		n48 = v->value.ptr;
		ncount = 0;
		for (i = 0; i < 256; i++) {
			if (n48->keys[i] < 48) {
				if (n48->keys[i] >= n48->nr_entries) {
					fprintf(stderr, "referencing value past nr_entries (n48)\n");
					return false;
				}

				if (bits & (1ull << n48->keys[i])) {
					fprintf(stderr, "duplicate entry (n48) %u\n", (unsigned) n48->keys[i]);
					return false;
				}
				bits = bits | (1ull << n48->keys[i]);
				ncount++;

				if (!_check_nodes(n48->values + n48->keys[i], count))
					return false;
			}
		}

		for (i = 0; i < n48->nr_entries; i++) {
			if (!(bits & (1ull << i))) {
				fprintf(stderr, "not all values are referenced (n48)\n");
				return false;
			}
		}

		if (ncount != n48->nr_entries) {
			fprintf(stderr, "incorrect number of entries in n48, n48->nr_entries = %u, actual = %u\n",
                                n48->nr_entries, ncount);
			return false;
		}

		for (i = 0; i < n48->nr_entries; i++)
			if (n48->values[i].type == UNSET) {
				fprintf(stderr, "value in UNSET (n48)\n");
				return false;
			}

		for (i = n48->nr_entries; i < 48; i++)
			if (n48->values[i].type != UNSET) {
				fprintf(stderr, "unused value is not UNSET (n48)\n");
				return false;
			}

		return true;

	case NODE256:
		n256 = v->value.ptr;

		ncount = 0;
		for (i = 0; i < 256; i++) {
			struct value *v2 = n256->values + i;

			if (v2->type == UNSET)
				continue;

			if (!_check_nodes(v2, count))
				return false;

			ncount++;
		}

		if (ncount != n256->nr_entries) {
			fprintf(stderr, "incorrect number of entries in n256, n256->nr_entries = %u, actual = %u\n",
                                n256->nr_entries, ncount);
			return false;
		}

		return true;

	default:
		fprintf(stderr, "unknown value type: %u\n", v->type);
	}

	fprintf(stderr, "shouldn't get here\n");
	return false;
}

bool radix_tree_is_well_formed(struct radix_tree *rt)
{
	unsigned count = 0;

	if (!_check_nodes(&rt->root, &count))
		return false;

	if (rt->nr_entries != count) {
		fprintf(stderr, "incorrect entry count: rt->nr_entries = %u, actual = %u\n",
                        rt->nr_entries, count);
		return false;
	}

	return true;
}

//----------------------------------------------------------------

static void _dump(FILE *out, struct value v, unsigned indent)
{
	unsigned i;
	struct value_chain *vc;
	struct prefix_chain *pc;
	struct node4 *n4;
	struct node16 *n16;
	struct node48 *n48;
	struct node256 *n256;

	if (v.type == UNSET)
		return;

	for (i = 0; i < 2 * indent; i++)
		fprintf(out, " ");

	switch (v.type) {
	case UNSET:
		// can't happen
		break;

	case VALUE:
		fprintf(out, "<val: %llu>\n", (unsigned long long) v.value.n);
		break;

	case VALUE_CHAIN:
		vc = v.value.ptr;
		fprintf(out, "<val_chain: %llu>\n", (unsigned long long) vc->value.n);
		_dump(out, vc->child, indent + 1);
		break;

	case PREFIX_CHAIN:
		pc = v.value.ptr;
		fprintf(out, "<prefix: ");
		for (i = 0; i < pc->len; i++)
			fprintf(out, "%x.", (unsigned) *(pc->prefix + i));
		fprintf(out, ">\n");
		_dump(out, pc->child, indent + 1);
		break;

	case NODE4:
		n4 = v.value.ptr;
		fprintf(out, "<n4: ");
		for (i = 0; i < n4->nr_entries; i++)
			fprintf(out, "%x ", (unsigned) n4->keys[i]);
		fprintf(out, ">\n");

		for (i = 0; i < n4->nr_entries; i++)
			_dump(out, n4->values[i], indent + 1);
		break;

	case NODE16:
		n16 = v.value.ptr;
		fprintf(out, "<n16: ");
		for (i = 0; i < n16->nr_entries; i++)
			fprintf(out, "%x ", (unsigned) n16->keys[i]);
		fprintf(out, ">\n");

		for (i = 0; i < n16->nr_entries; i++)
			_dump(out, n16->values[i], indent + 1);
		break;

	case NODE48:
		n48 = v.value.ptr;
		fprintf(out, "<n48: ");
		for (i = 0; i < 256; i++)
			if (n48->keys[i] < 48)
				fprintf(out, "%x ", i);
		fprintf(out, ">\n");

		for (i = 0; i < n48->nr_entries; i++) {
			assert(n48->values[i].type != UNSET);
			_dump(out, n48->values[i], indent + 1);
		}
		break;

	case NODE256:
		n256 = v.value.ptr;
		fprintf(out, "<n256: ");
		for (i = 0; i < 256; i++)
			if (n256->values[i].type != UNSET)
				fprintf(out, "%x ", i);
		fprintf(out, ">\n");

		for (i = 0; i < 256; i++)
			if (n256->values[i].type != UNSET)
				_dump(out, n256->values[i], indent + 1);
		break;
	}
}

void radix_tree_dump(struct radix_tree *rt, FILE *out)
{
	_dump(out, rt->root, 0);
}

//----------------------------------------------------------------
