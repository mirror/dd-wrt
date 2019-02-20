/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
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

#include "device_mapper/misc/dmlib.h"
#include "parse_rx.h"

#ifdef DEBUG
#include <ctype.h>

__attribute__ ((__unused__))
static void _regex_print(struct rx_node *rx, int depth, unsigned show_nodes)
{
	int i, numchars;

	if (rx->left) {
		if (rx->left->type != CHARSET && (show_nodes || (!((rx->type == CAT || rx->type == OR) && rx->left->type == CAT))))
			printf("(");

		_regex_print(rx->left, depth + 1, show_nodes);

		if (rx->left->type != CHARSET && (show_nodes || (!((rx->type == CAT || rx->type == OR) && rx->left->type == CAT))))
			printf(")");
	}

	/* display info about the node */
	switch (rx->type) {
	case CAT:
		break;

	case OR:
		printf("|");
		break;

	case STAR:
		printf("*");
		break;

	case PLUS:
		printf("+");
		break;

	case QUEST:
		printf("?");
		break;

	case CHARSET:
		numchars = 0;
		for (i = 0; i < 256; i++)
			if (dm_bit(rx->charset, i) && (isprint(i) || i == HAT_CHAR || i == DOLLAR_CHAR))
				numchars++;
		if (numchars == 97) {
			printf(".");
			break;
		}
		if (numchars > 1)
			printf("[");
		for (i = 0; i < 256; i++)
			if (dm_bit(rx->charset, i)) {
				if (isprint(i))
					printf("%c", (char) i);
				else if (i == HAT_CHAR)
					printf("^");
				else if (i == DOLLAR_CHAR)
					printf("$");
			}
		if (numchars > 1)
			printf("]");
		break;

	default:
		fprintf(stderr, "Unknown type");
	}

	if (rx->right) {
		if (rx->right->type != CHARSET && (show_nodes || (!(rx->type == CAT && rx->right->type == CAT) && rx->right->right)))
			printf("(");
		_regex_print(rx->right, depth + 1, show_nodes);
		if (rx->right->type != CHARSET && (show_nodes || (!(rx->type == CAT && rx->right->type == CAT) && rx->right->right)))
			printf(")");
	}

	if (!depth)
		printf("\n");
}
#endif /* DEBUG */

struct parse_sp {		/* scratch pad for the parsing process */
	struct dm_pool *mem;
	int type;		/* token type, 0 indicates a charset */
	dm_bitset_t charset;	/* The current charset */
	const char *cursor;	/* where we are in the regex */
	const char *rx_end;	/* 1pte for the expression being parsed */
};

static struct rx_node *_or_term(struct parse_sp *ps);

static void _single_char(struct parse_sp *ps, unsigned int c, const char *ptr)
{
	ps->type = 0;
	ps->cursor = ptr + 1;
	dm_bit_clear_all(ps->charset);
	dm_bit_set(ps->charset, c);
}

/*
 * Get the next token from the regular expression.
 * Returns: 1 success, 0 end of input, -1 error.
 */
static int _rx_get_token(struct parse_sp *ps)
{
	int neg = 0, range = 0;
	char c, lc = 0;
	const char *ptr = ps->cursor;
	if (ptr == ps->rx_end) {	/* end of input ? */
		ps->type = -1;
		return 0;
	}

	switch (*ptr) {
		/* charsets and ncharsets */
	case '[':
		ptr++;
		if (*ptr == '^') {
			dm_bit_set_all(ps->charset);

			/* never transition on zero */
			dm_bit_clear(ps->charset, 0);
			neg = 1;
			ptr++;

		} else
			dm_bit_clear_all(ps->charset);

		while ((ptr < ps->rx_end) && (*ptr != ']')) {
			if (*ptr == '\\') {
				/* an escaped character */
				ptr++;
				switch (*ptr) {
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 't':
					c = '\t';
					break;
				default:
					c = *ptr;
				}
			} else if (*ptr == '-' && lc) {
				/* we've got a range on our hands */
				range = 1;
				ptr++;
				if (ptr == ps->rx_end) {
					log_error("Incomplete range"
						  "specification");
					return -1;
				}
				c = *ptr;
			} else
				c = *ptr;

			if (range) {
				/* add lc - c into the bitset */
				if (lc > c) {
					char tmp = c;
					c = lc;
					lc = tmp;
				}

				for (; lc <= c; lc++) {
					if (neg)
						dm_bit_clear(ps->charset, lc);
					else
						dm_bit_set(ps->charset, lc);
				}
				range = 0;
			} else {
				/* add c into the bitset */
				if (neg)
					dm_bit_clear(ps->charset, c);
				else
					dm_bit_set(ps->charset, c);
			}
			ptr++;
			lc = c;
		}

		if (ptr >= ps->rx_end) {
			ps->type = -1;
			return -1;
		}

		ps->type = 0;
		ps->cursor = ptr + 1;
		break;

		/* These characters are special, we just return their ASCII
		   codes as the type.  Sorted into ascending order to help the
		   compiler */
	case '(':
	case ')':
	case '*':
	case '+':
	case '?':
	case '|':
		ps->type = (int) *ptr;
		ps->cursor = ptr + 1;
		break;

	case '^':
		_single_char(ps, HAT_CHAR, ptr);
		break;

	case '$':
		_single_char(ps, DOLLAR_CHAR, ptr);
		break;

	case '.':
		/* The 'all but newline' character set */
		ps->type = 0;
		ps->cursor = ptr + 1;
		dm_bit_set_all(ps->charset);
		dm_bit_clear(ps->charset, (int) '\n');
		dm_bit_clear(ps->charset, (int) '\r');
		dm_bit_clear(ps->charset, 0);
		break;

	case '\\':
		/* escaped character */
		ptr++;
		if (ptr >= ps->rx_end) {
			log_error("Badly quoted character at end "
				  "of expression");
			ps->type = -1;
			return -1;
		}

		ps->type = 0;
		ps->cursor = ptr + 1;
		dm_bit_clear_all(ps->charset);
		switch (*ptr) {
		case 'n':
			dm_bit_set(ps->charset, (int) '\n');
			break;
		case 'r':
			dm_bit_set(ps->charset, (int) '\r');
			break;
		case 't':
			dm_bit_set(ps->charset, (int) '\t');
			break;
		default:
			dm_bit_set(ps->charset, (int) *ptr);
		}
		break;

	default:
		/* add a single character to the bitset */
		ps->type = 0;
		ps->cursor = ptr + 1;
		dm_bit_clear_all(ps->charset);
		dm_bit_set(ps->charset, (int) (unsigned char) *ptr);
		break;
	}

	return 1;
}

static struct rx_node *_node(struct dm_pool *mem, int type,
			     struct rx_node *l, struct rx_node *r)
{
	struct rx_node *n = dm_pool_zalloc(mem, sizeof(*n));

	if (n) {
		if (type == CHARSET && !(n->charset = dm_bitset_create(mem, 256))) {
			dm_pool_free(mem, n);
			return NULL;
		}

		n->type = type;
		n->left = l;
		n->right = r;
	}

	return n;
}

static struct rx_node *_term(struct parse_sp *ps)
{
	struct rx_node *n;

	switch (ps->type) {
	case 0:
		if (!(n = _node(ps->mem, CHARSET, NULL, NULL)))
			return_NULL;

		dm_bit_copy(n->charset, ps->charset);
		_rx_get_token(ps);	/* match charset */
		break;

	case '(':
		_rx_get_token(ps);	/* match '(' */
		n = _or_term(ps);
		if (ps->type != ')') {
			log_error("missing ')' in regular expression");
			return 0;
		}
		_rx_get_token(ps);	/* match ')' */
		break;

	default:
		n = 0;
	}

	return n;
}

static struct rx_node *_closure_term(struct parse_sp *ps)
{
	struct rx_node *l, *n;

	if (!(l = _term(ps)))
		return NULL;

	for (;;) {
		switch (ps->type) {
		case '*':
			n = _node(ps->mem, STAR, l, NULL);
			break;

		case '+':
			n = _node(ps->mem, PLUS, l, NULL);
			break;

		case '?':
			n = _node(ps->mem, QUEST, l, NULL);
			break;

		default:
			return l;
		}

		if (!n)
			return_NULL;

		_rx_get_token(ps);
		l = n;
	}

	return n;
}

static struct rx_node *_cat_term(struct parse_sp *ps)
{
	struct rx_node *l, *r, *n;

	if (!(l = _closure_term(ps)))
		return NULL;

	if (ps->type == '|')
		return l;

	if (!(r = _cat_term(ps)))
		return l;

	if (!(n = _node(ps->mem, CAT, l, r)))
		stack;

	return n;
}

static struct rx_node *_or_term(struct parse_sp *ps)
{
	struct rx_node *l, *r, *n;

	if (!(l = _cat_term(ps)))
		return NULL;

	if (ps->type != '|')
		return l;

	_rx_get_token(ps);		/* match '|' */

	if (!(r = _or_term(ps))) {
		log_error("Badly formed 'or' expression");
		return NULL;
	}

	if (!(n = _node(ps->mem, OR, l, r)))
		stack;

	return n;
}

/*----------------------------------------------------------------*/

/* Macros for left and right nodes.  Inverted if 'leftmost' is set. */
#define LEFT(a) (leftmost ? (a)->left : (a)->right)
#define RIGHT(a) (leftmost ? (a)->right : (a)->left)

/*
 * The optimiser spots common prefixes on either side of an 'or' node, and
 * lifts them outside the 'or' with a 'cat'.
 */
static unsigned _depth(struct rx_node *r, unsigned leftmost)
{
	int count = 1;

	while (r->type != CHARSET && LEFT(r) && (leftmost || r->type != OR)) {
		count++;
		r = LEFT(r);
	}

	return count;
}

/*
 * FIXME: a unique key could be built up as part of the parse, to make the
 * comparison quick.  Alternatively we could use cons-hashing, and then
 * this would simply be a pointer comparison.
 */
static int _nodes_equal(struct rx_node *l, struct rx_node *r)
{
	if (l->type != r->type)
		return 0;

	switch (l->type) {
	case CAT:
	case OR:
		return _nodes_equal(l->left, r->left) &&
			_nodes_equal(l->right, r->right);

	case STAR:
	case PLUS:
	case QUEST:
		return _nodes_equal(l->left, r->left);

	case CHARSET:
		/*
		 * Never change anything containing TARGET_TRANS
		 * used by matcher as boundary marker between concatenated
		 * expressions.
		 */
		return (!dm_bit(l->charset, TARGET_TRANS) && dm_bitset_equal(l->charset, r->charset));
	}

	/* NOTREACHED */
	return_0;
}

static int _find_leftmost_common(struct rx_node *or,
                                 struct rx_node **l,
                                 struct rx_node **r,
				 unsigned leftmost)
{
	struct rx_node *left = or->left, *right = or->right;
	unsigned left_depth = _depth(left, leftmost);
	unsigned right_depth = _depth(right, leftmost);

	while (left_depth > right_depth && left->type != OR) {
		left = LEFT(left);
		left_depth--;
	}

	while (right_depth > left_depth && right->type != OR) {
		right = LEFT(right);
		right_depth--;
	}

	if (left_depth != right_depth)
		return 0;

	while (left_depth) {
		if (left->type == CAT && right->type == CAT) {
			if (_nodes_equal(LEFT(left), LEFT(right))) {
				*l = left;
				*r = right;
				return 1;
			}
		}
		if (left->type == OR || right->type == OR)
			break;
		left = LEFT(left);
		right = LEFT(right);
		left_depth--;
	}

	return 0;
}

/* If top node is OR, rotate (leftmost example) from ((ab)|((ac)|d)) to (((ab)|(ac))|d) */
static int _rotate_ors(struct rx_node *r, unsigned leftmost)
{
	struct rx_node *old_node;

	if (r->type != OR || RIGHT(r)->type != OR)
		return 0;

	old_node = RIGHT(r);

	if (leftmost) {
		r->right = RIGHT(old_node);
		old_node->right = LEFT(old_node);
		old_node->left = LEFT(r);
		r->left = old_node;
	} else {
		r->left = RIGHT(old_node);
		old_node->left = LEFT(old_node);
		old_node->right = LEFT(r);
		r->right = old_node;
	}

	return 1;
}

static struct rx_node *_exchange_nodes(struct dm_pool *mem, struct rx_node *r,
				       struct rx_node *left_cat, struct rx_node *right_cat,
				       unsigned leftmost)
{
	struct rx_node *new_r;

	if (leftmost)
		new_r = _node(mem, CAT, LEFT(left_cat), r);
	else
		new_r = _node(mem, CAT, r, LEFT(right_cat));

	if (!new_r)
		return_NULL;

	memcpy(left_cat, RIGHT(left_cat), sizeof(*left_cat));
	memcpy(right_cat, RIGHT(right_cat), sizeof(*right_cat));

	return new_r;
}

static struct rx_node *_pass(struct dm_pool *mem,
                             struct rx_node *r,
                             int *changed)
{
	struct rx_node *left, *right;

	/*
	 * walk the tree, optimising every 'or' node.
	 */
	switch (r->type) {
	case CAT:
		if (!(r->left = _pass(mem, r->left, changed)))
			return_NULL;

		if (!(r->right = _pass(mem, r->right, changed)))
			return_NULL;

		break;

	case STAR:
	case PLUS:
	case QUEST:
		if (!(r->left = _pass(mem, r->left, changed)))
			return_NULL;

		break;
	case OR:
		/* It's important we optimise sub nodes first */
		if (!(r->left = _pass(mem, r->left, changed)))
			return_NULL;

		if (!(r->right = _pass(mem, r->right, changed)))
			return_NULL;
		/*
		 * If rotate_ors changes the tree, left and right are stale,
		 * so just set 'changed' to repeat the search.
		 *
		 * FIXME Check we can't 'bounce' between left and right rotations here.
		 */
		if (_find_leftmost_common(r, &left, &right, 1)) {
			if (!_rotate_ors(r, 1))
				r = _exchange_nodes(mem, r, left, right, 1);
			*changed = 1;
		} else if (_find_leftmost_common(r, &left, &right, 0)) {
			if (!_rotate_ors(r, 0))
				r = _exchange_nodes(mem, r, left, right, 0);
			*changed = 1;
		}
		break;

	case CHARSET:
		break;
	}

	return r;
}

static struct rx_node *_optimise(struct dm_pool *mem, struct rx_node *r)
{
	/*
	 * We're looking for (or (... (cat <foo> a)) (... (cat <foo> b)))
	 * and want to turn it into (cat <foo> (or (... a) (... b)))
	 *
	 * (fa)|(fb) becomes f(a|b)
	 */

	/*
	 * Initially done as an inefficient multipass algorithm.
	 */
	int changed;

	do {
		changed = 0;
		r = _pass(mem, r, &changed);
	} while (r && changed);

	return r;
}

/*----------------------------------------------------------------*/

struct rx_node *rx_parse_tok(struct dm_pool *mem,
			     const char *begin, const char *end)
{
	struct rx_node *r;
	struct parse_sp *ps = dm_pool_zalloc(mem, sizeof(*ps));

	if (!ps)
		return_NULL;

	ps->mem = mem;
	if (!(ps->charset = dm_bitset_create(mem, 256))) {
		log_error("Regex charset allocation failed");
		dm_pool_free(mem, ps);
		return NULL;
	}
	ps->cursor = begin;
	ps->rx_end = end;
	_rx_get_token(ps);		/* load the first token */

	if (!(r = _or_term(ps))) {
		log_error("Parse error in regex");
		dm_pool_free(mem, ps);
		return NULL;
	}

	if (!(r = _optimise(mem, r))) {
		log_error("Regex optimisation error");
		dm_pool_free(mem, ps);
		return NULL;
	}

	return r;
}

struct rx_node *rx_parse_str(struct dm_pool *mem, const char *str)
{
	return rx_parse_tok(mem, str, str + strlen(str));
}
