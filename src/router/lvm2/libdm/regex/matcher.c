/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libdm/misc/dmlib.h"
#include "parse_rx.h"
#include "ttree.h"
#include "assert.h"

struct dfa_state {
	struct dfa_state *next;
	int final;
	dm_bitset_t bits;
	struct dfa_state *lookup[256];
};

struct dm_regex {		/* Instance variables for the lexer */
	struct dfa_state *start;
	unsigned num_nodes;
        unsigned num_charsets;
	int nodes_entered;
	struct rx_node **nodes;
        int charsets_entered;
        struct rx_node **charsets;
	struct dm_pool *scratch, *mem;

        /* stuff for on the fly dfa calculation */
        dm_bitset_t charmap[256];
        dm_bitset_t dfa_copy;
        struct ttree *tt;
        dm_bitset_t bs;
        struct dfa_state *h, *t;
};

static int _count_nodes(struct rx_node *rx)
{
	int r = 1;

	if (rx->left)
		r += _count_nodes(rx->left);

	if (rx->right)
		r += _count_nodes(rx->right);

	return r;
}

static unsigned _count_charsets(struct rx_node *rx)
{
        if (rx->type == CHARSET)
                return 1;

        return (rx->left ? _count_charsets(rx->left) : 0) +
                (rx->right ? _count_charsets(rx->right) : 0);
}

static void _enumerate_charsets_internal(struct rx_node *rx, unsigned *i)
{
        if (rx->type == CHARSET)
                rx->charset_index = (*i)++;
        else {
                if (rx->left)
                        _enumerate_charsets_internal(rx->left, i);
                if (rx->right)
                        _enumerate_charsets_internal(rx->right, i);
        }
}

static void _enumerate_charsets(struct rx_node *rx)
{
        unsigned i = 0;
        _enumerate_charsets_internal(rx, &i);
}

static void _fill_table(struct dm_regex *m, struct rx_node *rx)
{
	assert((rx->type != OR) || (rx->left && rx->right));

	if (rx->left)
		_fill_table(m, rx->left);

	if (rx->right)
		_fill_table(m, rx->right);

	m->nodes[m->nodes_entered++] = rx;
        if (rx->type == CHARSET)
                m->charsets[m->charsets_entered++] = rx;
}

static int _create_bitsets(struct dm_regex *m)
{
	unsigned i;
	struct rx_node *n;

	for (i = 0; i < m->num_nodes; i++) {
		n = m->nodes[i];
		if (!(n->firstpos = dm_bitset_create(m->scratch, m->num_charsets)))
			return_0;
		if (!(n->lastpos = dm_bitset_create(m->scratch, m->num_charsets)))
			return_0;
		if (!(n->followpos = dm_bitset_create(m->scratch, m->num_charsets)))
			return_0;
	}

	return 1;
}

static void _calc_functions(struct dm_regex *m)
{
	unsigned i, j, final = 1;
	struct rx_node *rx, *c1, *c2;

	for (i = 0; i < m->num_nodes; i++) {
		rx = m->nodes[i];
		c1 = rx->left;
		c2 = rx->right;

		if (rx->type == CHARSET && dm_bit(rx->charset, TARGET_TRANS))
			rx->final = final++;

		switch (rx->type) {
		case CAT:
			if (c1->nullable)
				dm_bit_union(rx->firstpos,
					  c1->firstpos, c2->firstpos);
			else
				dm_bit_copy(rx->firstpos, c1->firstpos);

			if (c2->nullable)
				dm_bit_union(rx->lastpos,
					  c1->lastpos, c2->lastpos);
			else
				dm_bit_copy(rx->lastpos, c2->lastpos);

			rx->nullable = c1->nullable && c2->nullable;
			break;

		case PLUS:
			dm_bit_copy(rx->firstpos, c1->firstpos);
			dm_bit_copy(rx->lastpos, c1->lastpos);
			rx->nullable = c1->nullable;
			break;

		case OR:
			dm_bit_union(rx->firstpos, c1->firstpos, c2->firstpos);
			dm_bit_union(rx->lastpos, c1->lastpos, c2->lastpos);
			rx->nullable = c1->nullable || c2->nullable;
			break;

		case QUEST:
		case STAR:
			dm_bit_copy(rx->firstpos, c1->firstpos);
			dm_bit_copy(rx->lastpos, c1->lastpos);
			rx->nullable = 1;
			break;

		case CHARSET:
			dm_bit_set(rx->firstpos, rx->charset_index);
			dm_bit_set(rx->lastpos, rx->charset_index);
			rx->nullable = 0;
			break;

		default:
			log_error(INTERNAL_ERROR "Unknown calc node type");
		}

		/*
		 * followpos has it's own switch
		 * because PLUS and STAR do the
		 * same thing.
		 */
		switch (rx->type) {
		case CAT:
			for (j = 0; j < m->num_charsets; j++) {
                                struct rx_node *n = m->charsets[j];
				if (dm_bit(c1->lastpos, j))
					dm_bit_union(n->followpos,
                                                     n->followpos, c2->firstpos);
			}
			break;

		case PLUS:
		case STAR:
			for (j = 0; j < m->num_charsets; j++) {
                                struct rx_node *n = m->charsets[j];
				if (dm_bit(rx->lastpos, j))
					dm_bit_union(n->followpos,
                                                     n->followpos, rx->firstpos);
			}
			break;
		}
	}
}

static struct dfa_state *_create_dfa_state(struct dm_pool *mem)
{
	return dm_pool_zalloc(mem, sizeof(struct dfa_state));
}

static struct dfa_state *_create_state_queue(struct dm_pool *mem,
                                             struct dfa_state *dfa,
                                             dm_bitset_t bits)
{
	if (!(dfa->bits = dm_bitset_create(mem, bits[0])))  /* first element is the size */
		return_NULL;

	dm_bit_copy(dfa->bits, bits);
	dfa->next = 0;
	dfa->final = -1;

	return dfa;
}

static int _calc_state(struct dm_regex *m, struct dfa_state *dfa, int a)
{
        int set_bits = 0, i;
        dm_bitset_t dfa_bits = dfa->bits;
        dm_bit_and(m->dfa_copy, m->charmap[a], dfa_bits);

        /* iterate through all the states in firstpos */
        for (i = dm_bit_get_first(m->dfa_copy); i >= 0; i = dm_bit_get_next(m->dfa_copy, i)) {
                if (a == TARGET_TRANS)
                        dfa->final = m->charsets[i]->final;

                dm_bit_union(m->bs, m->bs, m->charsets[i]->followpos);
                set_bits = 1;
        }

        if (set_bits) {
                struct dfa_state *tmp;
                struct dfa_state *ldfa = ttree_lookup(m->tt, m->bs + 1);
                if (!ldfa) {
                        /* push */
			if (!(ldfa = _create_dfa_state(m->mem)))
				return_0;

			ttree_insert(m->tt, m->bs + 1, ldfa);
			if (!(tmp = _create_state_queue(m->scratch, ldfa, m->bs)))
				return_0;
                        if (!m->h)
                                m->h = m->t = tmp;
                        else {
                                m->t->next = tmp;
                                m->t = tmp;
                        }
                }

                dfa->lookup[a] = ldfa;
                dm_bit_clear_all(m->bs);
        }

	return 1;
}

static int _calc_states(struct dm_regex *m, struct rx_node *rx)
{
	unsigned iwidth = (m->num_charsets / DM_BITS_PER_INT) + 1;
	struct dfa_state *dfa;
	struct rx_node *n;
	unsigned i;
	int a;

	if (!(m->tt = ttree_create(m->scratch, iwidth)))
		return_0;

	if (!(m->bs = dm_bitset_create(m->scratch, m->num_charsets)))
		return_0;

        /* build some char maps */
        for (a = 0; a < 256; a++)
		if (!(m->charmap[a] = dm_bitset_create(m->scratch, m->num_charsets)))
			return_0;

        for (i = 0; i < m->num_nodes; i++) {
		n = m->nodes[i];
                        if (n->type == CHARSET) {
                        for (a = dm_bit_get_first(n->charset);
                             a >= 0; a = dm_bit_get_next(n->charset, a))
                                dm_bit_set(m->charmap[a], n->charset_index);
                }
        }

	/* create first state */
	if (!(dfa = _create_dfa_state(m->mem)))
		return_0;

	m->start = dfa;
	ttree_insert(m->tt, rx->firstpos + 1, dfa);

	/* prime the queue */
	if (!(m->h = m->t = _create_state_queue(m->scratch, dfa, rx->firstpos)))
		return_0;

	if (!(m->dfa_copy = dm_bitset_create(m->scratch, m->num_charsets)))
		return_0;

	return 1;
}

/*
 * Forces all the dfa states to be calculated up front, ie. what
 * _calc_states() used to do before we switched to calculating on demand.
 */
static int _force_states(struct dm_regex *m)
{
        int a;

        /* keep processing until there's nothing in the queue */
        struct dfa_state *s;
        while ((s = m->h)) {
                /* pop state off front of the queue */
                m->h = m->h->next;

                /* iterate through all the inputs for this state */
                dm_bit_clear_all(m->bs);
                for (a = 0; a < 256; a++)
			if (!_calc_state(m, s, a))
				return_0;
        }

        return 1;
}

struct dm_regex *dm_regex_create(struct dm_pool *mem, const char * const *patterns,
				 unsigned num_patterns)
{
	char *all, *ptr;
	unsigned i;
	size_t len = 0;
	struct rx_node *rx;
	struct dm_regex *m;
	struct dm_pool *scratch = mem;

	if (!(m = dm_pool_zalloc(mem, sizeof(*m))))
		return_NULL;

	/* join the regexps together, delimiting with zero */
	for (i = 0; i < num_patterns; i++)
		len += strlen(patterns[i]) + 8;

	ptr = all = dm_pool_alloc(scratch, len + 1);

	if (!all)
		goto_bad;

	for (i = 0; i < num_patterns; i++) {
		ptr += sprintf(ptr, "(.*(%s)%c)", patterns[i], TARGET_TRANS);
		if (i < (num_patterns - 1))
			*ptr++ = '|';
	}

	/* parse this expression */
	if (!(rx = rx_parse_tok(scratch, all, ptr))) {
		log_error("Couldn't parse regex");
		goto bad;
	}

	m->mem = mem;
	m->scratch = scratch;
	m->num_nodes = _count_nodes(rx);
	m->num_charsets = _count_charsets(rx);
	_enumerate_charsets(rx);
	if (!(m->nodes = dm_pool_alloc(scratch, sizeof(*m->nodes) * m->num_nodes)))
		goto_bad;

	if (!(m->charsets = dm_pool_alloc(scratch, sizeof(*m->charsets) * m->num_charsets)))
		goto_bad;

	_fill_table(m, rx);

	if (!_create_bitsets(m))
		goto_bad;

	_calc_functions(m);

	if (!_calc_states(m, rx))
		goto_bad;

	return m;

      bad:
	dm_pool_free(mem, m);

	return NULL;
}

static struct dfa_state *_step_matcher(struct dm_regex *m, int c, struct dfa_state *cs, int *r)
{
        struct dfa_state *ns;

	if (!(ns = cs->lookup[(unsigned char) c])) {
		if (!_calc_state(m, cs, (unsigned char) c))
                        return_NULL;

		if (!(ns = cs->lookup[(unsigned char) c]))
			return NULL;
	}

        // yuck, we have to special case the target trans
	if ((ns->final == -1) &&
	    !_calc_state(m, ns, TARGET_TRANS))
                return_NULL;

	if (ns->final && (ns->final > *r))
		*r = ns->final;

	return ns;
}

int dm_regex_match(struct dm_regex *regex, const char *s)
{
	struct dfa_state *cs = regex->start;
	int r = 0;

        dm_bit_clear_all(regex->bs);
	if (!(cs = _step_matcher(regex, HAT_CHAR, cs, &r)))
		goto out;

	for (; *s; s++)
		if (!(cs = _step_matcher(regex, *s, cs, &r)))
			goto out;

	_step_matcher(regex, DOLLAR_CHAR, cs, &r);

      out:
	/* subtract 1 to get back to zero index */
	return r - 1;
}

/*
 * The next block of code concerns calculating a fingerprint for the dfa.
 *
 * We're not calculating a minimal dfa in _calculate_state (maybe a future
 * improvement).  As such it's possible that two non-isomorphic dfas
 * recognise the same language.  This can only really happen if you start
 * with equivalent, but different regexes (for example the simplifier in
 * parse_rx.c may have changed).
 *
 * The code is inefficient; repeatedly searching a singly linked list for
 * previously seen nodes.  Not worried since this is test code.
 */
struct node_list {
        unsigned node_id;
        struct dfa_state *node;
        struct node_list *next;
};

struct printer {
        struct dm_pool *mem;
        struct node_list *pending;
        struct node_list *processed;
        unsigned next_index;
};

static uint32_t _randomise(uint32_t n)
{
        /* 2^32 - 5 */
        uint32_t const prime = (~0) - 4;
        return n * prime;
}

static int _seen(struct node_list *n, struct dfa_state *node, uint32_t *i)
{
        while (n) {
                if (n->node == node) {
                        *i = n->node_id;
                        return 1;
                }
                n = n->next;
        }

        return 0;
}

/*
 * Push node if it's not been seen before, returning a unique index.
 */
static uint32_t _push_node(struct printer *p, struct dfa_state *node)
{
        uint32_t i;
	struct node_list *n;

        if (_seen(p->pending, node, &i) ||
            _seen(p->processed, node, &i))
                return i;

	if (!(n = dm_pool_alloc(p->mem, sizeof(*n))))
		return_0;

	n->node_id = ++p->next_index; /* start from 1, keep 0 as error code */
	n->node = node;
	n->next = p->pending;
	p->pending = n;

	return n->node_id;
}

/*
 * Pop the front node, and fill out it's previously assigned index.
 */
static struct dfa_state *_pop_node(struct printer *p)
{
        struct dfa_state *node = NULL;
	struct node_list *n;

	if (p->pending) {
		n = p->pending;
                p->pending = n->next;
                n->next = p->processed;
                p->processed = n;

                node = n->node;
        }

        return node;
}

static uint32_t _combine(uint32_t n1, uint32_t n2)
{
        return ((n1 << 8) | (n1 >> 24)) ^ _randomise(n2);
}

static uint32_t _fingerprint(struct printer *p)
{
        int c;
        uint32_t result = 0;
        struct dfa_state *node;

        while ((node = _pop_node(p))) {
                result = _combine(result, (node->final < 0) ? 0 : node->final);
                for (c = 0; c < 256; c++)
                        result = _combine(result,
                                          _push_node(p, node->lookup[c]));
        }

        return result;
}

uint32_t dm_regex_fingerprint(struct dm_regex *regex)
{
        struct printer p;
        uint32_t result = 0;
        struct dm_pool *mem = dm_pool_create("regex fingerprint", 1024);

	if (!mem)
		return_0;

	if (!_force_states(regex))
		goto_out;

        p.mem = mem;
        p.pending = NULL;
        p.processed = NULL;
        p.next_index = 0;

	if (!_push_node(&p, regex->start))
		goto_out;

	result = _fingerprint(&p);
out:
        dm_pool_destroy(mem);

        return result;
}
