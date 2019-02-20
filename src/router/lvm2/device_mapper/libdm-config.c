/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "misc/dmlib.h"

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>

#define SECTION_B_CHAR '{'
#define SECTION_E_CHAR '}'

enum {
	TOK_INT,
	TOK_FLOAT,
	TOK_STRING,		/* Single quotes */
	TOK_STRING_ESCAPED,	/* Double quotes */
	TOK_STRING_BARE,	/* No quotes */
	TOK_EQ,
	TOK_SECTION_B,
	TOK_SECTION_E,
	TOK_ARRAY_B,
	TOK_ARRAY_E,
	TOK_IDENTIFIER,
	TOK_COMMA,
	TOK_EOF
};

struct parser {
	const char *fb, *fe;		/* file limits */

	int t;			/* token limits and type */
	const char *tb, *te;

	int line;		/* line number we are on */

	struct dm_pool *mem;
	int no_dup_node_check;	/* whether to disable dup node checking */
};

struct config_output {
	struct dm_pool *mem;
	dm_putline_fn putline;
	const struct dm_config_node_out_spec *spec;
	void *baton;
};

static void _get_token(struct parser *p, int tok_prev);
static void _eat_space(struct parser *p);
static struct dm_config_node *_file(struct parser *p);
static struct dm_config_node *_section(struct parser *p, struct dm_config_node *parent);
static struct dm_config_value *_value(struct parser *p);
static struct dm_config_value *_type(struct parser *p);
static int _match_aux(struct parser *p, int t);
static struct dm_config_value *_create_value(struct dm_pool *mem);
static struct dm_config_node *_create_node(struct dm_pool *mem);
static char *_dup_tok(struct parser *p);
static char *_dup_token(struct dm_pool *mem, const char *b, const char *e);

static const int _sep = '/';

#define MAX_INDENT 32

#define match(t) do {\
   if (!_match_aux(p, (t))) {\
	log_error("Parse error at byte %" PRIptrdiff_t " (line %d): unexpected token", \
		  p->tb - p->fb + 1, p->line); \
      return 0;\
   } \
} while(0)

static int _tok_match(const char *str, const char *b, const char *e)
{
	while (*str && (b != e)) {
		if (*str++ != *b++)
			return 0;
	}

	return !(*str || (b != e));
}

struct dm_config_tree *dm_config_create(void)
{
	struct dm_config_tree *cft;
	struct dm_pool *mem = dm_pool_create("config", 10 * 1024);

	if (!mem) {
		log_error("Failed to allocate config pool.");
		return 0;
	}

	if (!(cft = dm_pool_zalloc(mem, sizeof(*cft)))) {
		log_error("Failed to allocate config tree.");
		dm_pool_destroy(mem);
		return 0;
	}
	cft->mem = mem;

	return cft;
}

void dm_config_set_custom(struct dm_config_tree *cft, void *custom)
{
	cft->custom = custom;
}

void *dm_config_get_custom(struct dm_config_tree *cft)
{
	return cft->custom;
}

void dm_config_destroy(struct dm_config_tree *cft)
{
	dm_pool_destroy(cft->mem);
}

/*
 * If there's a cascaded dm_config_tree, remove and return it, otherwise
 * return NULL.
 */
struct dm_config_tree *dm_config_remove_cascaded_tree(struct dm_config_tree *cft)
{
	struct dm_config_tree *second_cft;

	if (!cft)
		return NULL;

	second_cft = cft->cascade;
	cft->cascade = NULL;

	return second_cft;
}

/*
 * When searching, first_cft is checked before second_cft.
 */
struct dm_config_tree *dm_config_insert_cascaded_tree(struct dm_config_tree *first_cft, struct dm_config_tree *second_cft)
{
	first_cft->cascade = second_cft;

	return first_cft;
}

static struct dm_config_node *_config_reverse(struct dm_config_node *head)
{
	struct dm_config_node *left = head, *middle = NULL, *right = NULL;

	while (left) {
		right = middle;
		middle = left;
		left = left->sib;
		middle->sib = right;
		middle->child = _config_reverse(middle->child);
	}

	return middle;
}

static int _do_dm_config_parse(struct dm_config_tree *cft, const char *start, const char *end, int no_dup_node_check)
{
	/* TODO? if (start == end) return 1; */

	struct parser *p;
	if (!(p = dm_pool_alloc(cft->mem, sizeof(*p))))
		return_0;

	p->mem = cft->mem;
	p->fb = start;
	p->fe = end;
	p->tb = p->te = p->fb;
	p->line = 1;
	p->no_dup_node_check = no_dup_node_check;

	_get_token(p, TOK_SECTION_E);
	if (!(cft->root = _file(p)))
		return_0;

	cft->root = _config_reverse(cft->root);

	return 1;
}

int dm_config_parse(struct dm_config_tree *cft, const char *start, const char *end)
{
	return _do_dm_config_parse(cft, start, end, 0);
}

int dm_config_parse_without_dup_node_check(struct dm_config_tree *cft, const char *start, const char *end)
{
	return _do_dm_config_parse(cft, start, end, 1);
}

struct dm_config_tree *dm_config_from_string(const char *config_settings)
{
	struct dm_config_tree *cft;

	if (!(cft = dm_config_create()))
		return_NULL;

	if (!dm_config_parse(cft, config_settings, config_settings + strlen(config_settings))) {
		dm_config_destroy(cft);
		return_NULL;
	}

	return cft;
}

static int _line_start(struct config_output *out)
{
	if (!dm_pool_begin_object(out->mem, 128)) {
		log_error("dm_pool_begin_object failed for config line");
		return 0;
	}

	return 1;
}

__attribute__ ((format(printf, 2, 3)))
static int _line_append(struct config_output *out, const char *fmt, ...)
{
	char buf[4096];
	char *dyn_buf = NULL;
	va_list ap;
	int n;

	/*
	 * We should be fine with the 4096 char buffer 99% of the time,
	 * but if we need to go beyond that, allocate the buffer dynamically.
	 */

	va_start(ap, fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (n < 0) {
		log_error("vsnprintf failed for config line");
		return 0;
	}

	if (n > (int) sizeof buf - 1) {
		/*
		 * Fixed size buffer with sizeof buf is not enough,
		 * so try dynamically allocated buffer now...
		 */
		va_start(ap, fmt);
		n = dm_vasprintf(&dyn_buf, fmt, ap);
		va_end(ap);

		if (n < 0) {
			log_error("dm_vasprintf failed for config line");
			return 0;
		}
	}

	if (!dm_pool_grow_object(out->mem, dyn_buf ? : buf, 0)) {
		log_error("dm_pool_grow_object failed for config line");
		free(dyn_buf);
		return 0;
	}

	free(dyn_buf);

	return 1;
}

#define line_append(args...) do {if (!_line_append(out, args)) {return_0;}} while (0)

static int _line_end(const struct dm_config_node *cn, struct config_output *out)
{
	const char *line;

	if (!dm_pool_grow_object(out->mem, "\0", 1)) {
		log_error("dm_pool_grow_object failed for config line");
		return 0;
	}

	line = dm_pool_end_object(out->mem);

	if (!out->putline && !out->spec)
		return 0;

	if (out->putline)
		out->putline(line, out->baton);

	if (out->spec && out->spec->line_fn)
		out->spec->line_fn(cn, line, out->baton);

	return 1;
}

static int _write_value(struct config_output *out, const struct dm_config_value *v)
{
	char *buf;
	const char *s;

	switch (v->type) {
	case DM_CFG_STRING:
		buf = alloca(dm_escaped_len(v->v.str));
		s = (v->format_flags & DM_CONFIG_VALUE_FMT_STRING_NO_QUOTES) ? "" : "\"";
		line_append("%s%s%s", s, dm_escape_double_quotes(buf, v->v.str), s);
		break;

	case DM_CFG_FLOAT:
		line_append("%f", v->v.f);
		break;

	case DM_CFG_INT:
		if (v->format_flags & DM_CONFIG_VALUE_FMT_INT_OCTAL)
			line_append("0%" PRIo64, v->v.i);
		else
			line_append(FMTd64, v->v.i);
		break;

	case DM_CFG_EMPTY_ARRAY:
		s = (v->format_flags & DM_CONFIG_VALUE_FMT_COMMON_EXTRA_SPACES) ? " " : "";
		line_append("[%s]", s);
		break;

	default:
		log_error("_write_value: Unknown value type: %d", v->type);

	}

	return 1;
}

static int _write_config(const struct dm_config_node *n, int only_one,
			 struct config_output *out, int level)
{
	const char *extra_space;
	int format_array;
	char space[MAX_INDENT + 1];
	int l = (level < MAX_INDENT) ? level : MAX_INDENT;
	int i;
	char *escaped_key = NULL;

	if (!n)
		return 1;

	for (i = 0; i < l; i++)
		space[i] = '\t';
	space[i] = '\0';

	do {
		extra_space = (n->v && (n->v->format_flags & DM_CONFIG_VALUE_FMT_COMMON_EXTRA_SPACES)) ? " " : "";
		format_array = (n->v && (n->v->format_flags & DM_CONFIG_VALUE_FMT_COMMON_ARRAY));

		if (out->spec && out->spec->prefix_fn)
			out->spec->prefix_fn(n, space, out->baton);

		if (!_line_start(out))
			return_0;
		if (strchr(n->key, '#') || strchr(n->key, '"') || strchr(n->key, '!')) {
			escaped_key = alloca(dm_escaped_len(n->key) + 2);
			*escaped_key = '"';
			dm_escape_double_quotes(escaped_key + 1, n->key);
			strcat(escaped_key, "\"");
		}
		line_append("%s%s", space, escaped_key ? escaped_key : n->key);
		escaped_key = NULL;
		if (!n->v) {
			/* it's a sub section */
			line_append(" {");
			if (!_line_end(n, out))
				return_0;
			if (!_write_config(n->child, 0, out, level + 1))
				return_0;
			if (!_line_start(out))
				return_0;
			line_append("%s}", space);
		} else {
			/* it's a value */
			const struct dm_config_value *v = n->v;
			line_append("%s=%s", extra_space, extra_space);
			if (v->next) {
				line_append("[%s", extra_space);
				while (v && v->type != DM_CFG_EMPTY_ARRAY) {
					if (!_write_value(out, v))
						return_0;
					v = v->next;
					if (v && v->type != DM_CFG_EMPTY_ARRAY)
						line_append(",%s", extra_space);
				}
				line_append("%s]", extra_space);
			} else {
				if (format_array && (v->type != DM_CFG_EMPTY_ARRAY))
					line_append("[%s", extra_space);
				if (!_write_value(out, v))
					return_0;
				if (format_array && (v->type != DM_CFG_EMPTY_ARRAY))
					line_append("%s]", extra_space);
			}
		}
		if (!_line_end(n, out))
			return_0;

		if (out->spec && out->spec->suffix_fn)
			out->spec->suffix_fn(n, space, out->baton);

		n = n->sib;
	} while (n && !only_one);
	/* FIXME: add error checking */
	return 1;
}

static int _write_node(const struct dm_config_node *cn, int only_one,
		       dm_putline_fn putline,
		       const struct dm_config_node_out_spec *out_spec,
		       void *baton)
{
	struct config_output out = {
		.mem = dm_pool_create("config_output", 1024),
		.putline = putline,
		.spec = out_spec,
		.baton = baton
	};

	if (!out.mem)
		return_0;

	if (!_write_config(cn, only_one, &out, 0)) {
		dm_pool_destroy(out.mem);
		return_0;
	}
	dm_pool_destroy(out.mem);
	return 1;
}

int dm_config_write_one_node(const struct dm_config_node *cn, dm_putline_fn putline, void *baton)
{
	return _write_node(cn, 1, putline, NULL, baton);
}

int dm_config_write_node(const struct dm_config_node *cn, dm_putline_fn putline, void *baton)
{
	return _write_node(cn, 0, putline, NULL, baton);
}

int dm_config_write_one_node_out(const struct dm_config_node *cn,
				 const struct dm_config_node_out_spec *out_spec,
				 void *baton)
{
	return _write_node(cn, 1, NULL, out_spec, baton);
}

int dm_config_write_node_out(const struct dm_config_node *cn,
			     const struct dm_config_node_out_spec *out_spec,
			     void *baton)
{
	return _write_node(cn, 0, NULL, out_spec, baton);
}

/*
 * parser
 */
static char *_dup_string_tok(struct parser *p)
{
	char *str;

	p->tb++, p->te--;	/* strip "'s */

	if (p->te < p->tb) {
		log_error("Parse error at byte %" PRIptrdiff_t " (line %d): "
			  "expected a string token.",
			  p->tb - p->fb + 1, p->line);
		return NULL;
	}

	if (!(str = _dup_tok(p)))
		return_NULL;

	p->te++;

	return str;
}

static struct dm_config_node *_file(struct parser *p)
{
	struct dm_config_node root = { 0 };
	root.key = "<root>";

	while (p->t != TOK_EOF)
		if (!_section(p, &root))
			return_NULL;
	return root.child;
}

static struct dm_config_node *_make_node(struct dm_pool *mem,
					 const char *key_b, const char *key_e,
					 struct dm_config_node *parent)
{
	struct dm_config_node *n;

	if (!(n = _create_node(mem)))
		return_NULL;

	n->key = _dup_token(mem, key_b, key_e);
	if (parent) {
		n->parent = parent;
		n->sib = parent->child;
		parent->child = n;
	}
	return n;
}

/* when mem is not NULL, we create the path if it doesn't exist yet */
static struct dm_config_node *_find_or_make_node(struct dm_pool *mem,
						 struct dm_config_node *parent,
						 const char *path,
						 int no_dup_node_check)
{
	const char *e;
	struct dm_config_node *cn = parent ? parent->child : NULL;
	struct dm_config_node *cn_found = NULL;

	while (cn || mem) {
		/* trim any leading slashes */
		while (*path && (*path == _sep))
			path++;

		/* find the end of this segment */
		for (e = path; *e && (*e != _sep); e++) ;

		/* hunt for the node */
		cn_found = NULL;

		if (!no_dup_node_check) {
			while (cn) {
				if (_tok_match(cn->key, path, e)) {
					/* Inefficient */
					if (!cn_found)
						cn_found = cn;
					else
						log_warn("WARNING: Ignoring duplicate"
							 " config node: %s ("
							 "seeking %s)", cn->key, path);
				}

				cn = cn->sib;
			}
		}

		if (!cn_found && mem) {
			if (!(cn_found = _make_node(mem, path, e, parent)))
				return_NULL;
		}

		if (cn_found && *e) {
			parent = cn_found;
			cn = cn_found->child;
		} else
			return cn_found;
		path = e;
	}

	return NULL;
}

static struct dm_config_node *_section(struct parser *p, struct dm_config_node *parent)
{
	/* IDENTIFIER SECTION_B_CHAR VALUE* SECTION_E_CHAR */

	struct dm_config_node *root;
	struct dm_config_value *value;
	char *str;

	if (p->t == TOK_STRING_ESCAPED) {
		if (!(str = _dup_string_tok(p)))
			return_NULL;
		dm_unescape_double_quotes(str);

		match(TOK_STRING_ESCAPED);
	} else if (p->t == TOK_STRING) {
		if (!(str = _dup_string_tok(p)))
			return_NULL;

		match(TOK_STRING);
	} else {
		if (!(str = _dup_tok(p)))
			return_NULL;

		match(TOK_IDENTIFIER);
	}

	if (!strlen(str)) {
		log_error("Parse error at byte %" PRIptrdiff_t " (line %d): empty section identifier",
			  p->tb - p->fb + 1, p->line);
		return NULL;
	}

	if (!(root = _find_or_make_node(p->mem, parent, str, p->no_dup_node_check)))
		return_NULL;

	if (p->t == TOK_SECTION_B) {
		match(TOK_SECTION_B);
		while (p->t != TOK_SECTION_E) {
			if (!(_section(p, root)))
				return_NULL;
		}
		match(TOK_SECTION_E);
	} else {
		match(TOK_EQ);
		if (!(value = _value(p)))
			return_NULL;
		if (root->v)
			log_warn("WARNING: Ignoring duplicate"
				 " config value: %s", str);
		root->v = value;
	}

	return root;
}

static struct dm_config_value *_value(struct parser *p)
{
	/* '[' TYPE* ']' | TYPE */
	struct dm_config_value *h = NULL, *l, *ll = NULL;
	if (p->t == TOK_ARRAY_B) {
		match(TOK_ARRAY_B);
		while (p->t != TOK_ARRAY_E) {
			if (!(l = _type(p)))
				return_NULL;

			if (!h)
				h = l;
			else
				ll->next = l;
			ll = l;

			if (p->t == TOK_COMMA)
				match(TOK_COMMA);
		}
		match(TOK_ARRAY_E);
		/*
		 * Special case for an empty array.
		 */
		if (!h) {
			if (!(h = _create_value(p->mem))) {
				log_error("Failed to allocate value");
				return NULL;
			}

			h->type = DM_CFG_EMPTY_ARRAY;
		}

	} else
		if (!(h = _type(p)))
			return_NULL;

	return h;
}

static struct dm_config_value *_type(struct parser *p)
{
	/* [+-]{0,1}[0-9]+ | [0-9]*\.[0-9]* | ".*" */
	struct dm_config_value *v = _create_value(p->mem);
	char *str;

	if (!v) {
		log_error("Failed to allocate type value");
		return NULL;
	}

	switch (p->t) {
	case TOK_INT:
		v->type = DM_CFG_INT;
		errno = 0;
		v->v.i = strtoll(p->tb, NULL, 0);	/* FIXME: check error */
		if (errno) {
			log_error("Failed to read int token.");
			return NULL;
		}
		match(TOK_INT);
		break;

	case TOK_FLOAT:
		v->type = DM_CFG_FLOAT;
		errno = 0;
		v->v.f = strtod(p->tb, NULL);	/* FIXME: check error */
		if (errno) {
			log_error("Failed to read float token.");
			return NULL;
		}
		match(TOK_FLOAT);
		break;

	case TOK_STRING:
		v->type = DM_CFG_STRING;

		if (!(v->v.str = _dup_string_tok(p)))
			return_NULL;

		match(TOK_STRING);
		break;

	case TOK_STRING_BARE:
		v->type = DM_CFG_STRING;

		if (!(v->v.str = _dup_tok(p)))
			return_NULL;

		match(TOK_STRING_BARE);
		break;

	case TOK_STRING_ESCAPED:
		v->type = DM_CFG_STRING;

		if (!(str = _dup_string_tok(p)))
			return_NULL;
		dm_unescape_double_quotes(str);
		v->v.str = str;
		match(TOK_STRING_ESCAPED);
		break;

	default:
		log_error("Parse error at byte %" PRIptrdiff_t " (line %d): expected a value",
			  p->tb - p->fb + 1, p->line);
		return NULL;
	}
	return v;
}

static int _match_aux(struct parser *p, int t)
{
	if (p->t != t)
		return 0;

	_get_token(p, t);
	return 1;
}

/*
 * tokeniser
 */
static void _get_token(struct parser *p, int tok_prev)
{
	int values_allowed = 0;

	const char *te;

	p->tb = p->te;
	_eat_space(p);
	if (p->tb == p->fe || !*p->tb) {
		p->t = TOK_EOF;
		return;
	}

	/* Should next token be interpreted as value instead of identifier? */
	if (tok_prev == TOK_EQ || tok_prev == TOK_ARRAY_B ||
	    tok_prev == TOK_COMMA)
		values_allowed = 1;

	p->t = TOK_INT;		/* fudge so the fall through for
				   floats works */

	te = p->te;
	switch (*te) {
	case SECTION_B_CHAR:
		p->t = TOK_SECTION_B;
		te++;
		break;

	case SECTION_E_CHAR:
		p->t = TOK_SECTION_E;
		te++;
		break;

	case '[':
		p->t = TOK_ARRAY_B;
		te++;
		break;

	case ']':
		p->t = TOK_ARRAY_E;
		te++;
		break;

	case ',':
		p->t = TOK_COMMA;
		te++;
		break;

	case '=':
		p->t = TOK_EQ;
		te++;
		break;

	case '"':
		p->t = TOK_STRING_ESCAPED;
		te++;
		while ((te != p->fe) && (*te) && (*te != '"')) {
			if ((*te == '\\') && (te + 1 != p->fe) &&
			    *(te + 1))
				te++;
			te++;
		}

		if ((te != p->fe) && (*te))
			te++;
		break;

	case '\'':
		p->t = TOK_STRING;
		te++;
		while ((te != p->fe) && (*te) && (*te != '\''))
			te++;

		if ((te != p->fe) && (*te))
			te++;
		break;

	case '.':
		p->t = TOK_FLOAT;
		/* Fall through */
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '+':
	case '-':
		if (values_allowed) {
			while (++te != p->fe) {
				if (!isdigit((int) *te)) {
					if (*te == '.') {
						if (p->t != TOK_FLOAT) {
							p->t = TOK_FLOAT;
							continue;
						}
					}
					break;
				}
			}
			break;
		}
		/* fall through */

	default:
		p->t = TOK_IDENTIFIER;
		while ((te != p->fe) && (*te) && !isspace(*te) &&
		       (*te != '#') && (*te != '=') &&
		       (*te != SECTION_B_CHAR) &&
		       (*te != SECTION_E_CHAR))
			te++;
		if (values_allowed)
			p->t = TOK_STRING_BARE;
		break;
	}

	p->te = te;
}

static void _eat_space(struct parser *p)
{
	while (p->tb != p->fe) {
		if (*p->te == '#')
			while ((p->te != p->fe) && (*p->te != '\n') && (*p->te))
				++p->te;

		else if (!isspace(*p->te))
			break;

		while ((p->te != p->fe) && isspace(*p->te)) {
			if (*p->te == '\n')
				++p->line;
			++p->te;
		}

		p->tb = p->te;
	}
}

/*
 * memory management
 */
static struct dm_config_value *_create_value(struct dm_pool *mem)
{
	return dm_pool_zalloc(mem, sizeof(struct dm_config_value));
}

static struct dm_config_node *_create_node(struct dm_pool *mem)
{
	return dm_pool_zalloc(mem, sizeof(struct dm_config_node));
}

static char *_dup_token(struct dm_pool *mem, const char *b, const char *e)
{
	size_t len = e - b;
	char *str = dm_pool_alloc(mem, len + 1);
	if (!str) {
		log_error("Failed to duplicate token.");
		return 0;
	}
	memcpy(str, b, len);
	str[len] = '\0';
	return str;
}

static char *_dup_tok(struct parser *p)
{
	return _dup_token(p->mem, p->tb, p->te);
}

/*
 * Utility functions
 */

/*
 * node_lookup_fn is either:
 *   _find_config_node to perform a lookup starting from a given config_node 
 *   in a config_tree;
 * or
 *   _find_first_config_node to find the first config_node in a set of 
 *   cascaded trees.
 */
typedef const struct dm_config_node *node_lookup_fn(const void *start, const char *path);

static const struct dm_config_node *_find_config_node(const void *start, const char *path) {
	struct dm_config_node dummy = { .child = (void *) start };
	return _find_or_make_node(NULL, &dummy, path, 0);
}

static const struct dm_config_node *_find_first_config_node(const void *start, const char *path)
{
	const struct dm_config_tree *cft = start;
	const struct dm_config_node *cn = NULL;

	while (cft) {
		if ((cn = _find_config_node(cft->root, path)))
			return cn;
		cft = cft->cascade;
	}

	return NULL;
}

static const char *_find_config_str(const void *start, node_lookup_fn find_fn,
				    const char *path, const char *fail, int allow_empty)
{
	const struct dm_config_node *n = find_fn(start, path);

	/* Empty strings are ignored if allow_empty is set */
	if (n && n->v) {
		if ((n->v->type == DM_CFG_STRING) &&
		    (allow_empty || (*n->v->v.str))) {
			/* log_very_verbose("Setting %s to %s", path, n->v->v.str); */
			return n->v->v.str;
		}
		if ((n->v->type != DM_CFG_STRING) || (!allow_empty && fail))
			log_warn("WARNING: Ignoring unsupported value for %s.", path);
	}

	if (fail)
		log_very_verbose("%s not found in config: defaulting to %s",
				 path, fail);
	return fail;
}

const char *dm_config_find_str(const struct dm_config_node *cn,
			       const char *path, const char *fail)
{
	return _find_config_str(cn, _find_config_node, path, fail, 0);
}

const char *dm_config_find_str_allow_empty(const struct dm_config_node *cn,
					   const char *path, const char *fail)
{
	return _find_config_str(cn, _find_config_node, path, fail, 1);
}

static int64_t _find_config_int64(const void *start, node_lookup_fn find,
				  const char *path, int64_t fail)
{
	const struct dm_config_node *n = find(start, path);

	if (n && n->v && n->v->type == DM_CFG_INT) {
		/* log_very_verbose("Setting %s to %" PRId64, path, n->v->v.i); */
		return n->v->v.i;
	}

	log_very_verbose("%s not found in config: defaulting to %" PRId64,
			 path, fail);
	return fail;
}

static float _find_config_float(const void *start, node_lookup_fn find,
				const char *path, float fail)
{
	const struct dm_config_node *n = find(start, path);

	if (n && n->v && n->v->type == DM_CFG_FLOAT) {
		/* log_very_verbose("Setting %s to %f", path, n->v->v.f); */
		return n->v->v.f;
	}

	log_very_verbose("%s not found in config: defaulting to %f",
			 path, fail);

	return fail;

}

static int _str_in_array(const char *str, const char * const values[])
{
	int i;

	for (i = 0; values[i]; i++)
		if (!strcasecmp(str, values[i]))
			return 1;

	return 0;
}

static int _str_to_bool(const char *str, int fail)
{
	const char * const _true_values[]  = { "y", "yes", "on", "true", NULL };
	const char * const _false_values[] = { "n", "no", "off", "false", NULL };

	if (_str_in_array(str, _true_values))
		return 1;

	if (_str_in_array(str, _false_values))
		return 0;

	return fail;
}

static int _find_config_bool(const void *start, node_lookup_fn find,
			     const char *path, int fail)
{
	const struct dm_config_node *n = find(start, path);
	const struct dm_config_value *v;
	int b;

	if (n) {
		v = n->v;

		switch (v->type) {
		case DM_CFG_INT:
			b = v->v.i ? 1 : 0;
			/* log_very_verbose("Setting %s to %d", path, b); */
			return b;

		case DM_CFG_STRING:
			b = _str_to_bool(v->v.str, fail);
			/* log_very_verbose("Setting %s to %d", path, b); */
			return b;
		default:
			;
		}
	}

	log_very_verbose("%s not found in config: defaulting to %d",
			 path, fail);

	return fail;
}

/***********************************
 * node-based lookup
 **/

struct dm_config_node *dm_config_find_node(const struct dm_config_node *cn,
					   const char *path)
{
	return (struct dm_config_node *) _find_config_node(cn, path);
}

int dm_config_find_int(const struct dm_config_node *cn, const char *path, int fail)
{
	/* FIXME Add log_error message on overflow */
	return (int) _find_config_int64(cn, _find_config_node, path, (int64_t) fail);
}

int64_t dm_config_find_int64(const struct dm_config_node *cn, const char *path, int64_t fail)
{
	return _find_config_int64(cn, _find_config_node, path, fail);
}

float dm_config_find_float(const struct dm_config_node *cn, const char *path,
			   float fail)
{
	return _find_config_float(cn, _find_config_node, path, fail);
}

int dm_config_find_bool(const struct dm_config_node *cn, const char *path, int fail)
{
	return _find_config_bool(cn, _find_config_node, path, fail);
}

int dm_config_value_is_bool(const struct dm_config_value *v) {
	if (!v)
		return 0;

	switch(v->type) {
		case DM_CFG_INT:
			return 1;
		case DM_CFG_STRING:
			return _str_to_bool(v->v.str, -1) != -1;
		default:
			return 0;
	}
}

/***********************************
 * tree-based lookup
 **/

const struct dm_config_node *dm_config_tree_find_node(const struct dm_config_tree *cft,
						      const char *path)
{
	return _find_first_config_node(cft, path);
}

const char *dm_config_tree_find_str(const struct dm_config_tree *cft, const char *path,
				    const char *fail)
{
	return _find_config_str(cft, _find_first_config_node, path, fail, 0);
}

const char *dm_config_tree_find_str_allow_empty(const struct dm_config_tree *cft, const char *path,
						const char *fail)
{
	return _find_config_str(cft, _find_first_config_node, path, fail, 1);
}

int dm_config_tree_find_int(const struct dm_config_tree *cft, const char *path, int fail)
{
	/* FIXME Add log_error message on overflow */
	return (int) _find_config_int64(cft, _find_first_config_node, path, (int64_t) fail);
}

int64_t dm_config_tree_find_int64(const struct dm_config_tree *cft, const char *path, int64_t fail)
{
	return _find_config_int64(cft, _find_first_config_node, path, fail);
}

float dm_config_tree_find_float(const struct dm_config_tree *cft, const char *path,
				float fail)
{
	return _find_config_float(cft, _find_first_config_node, path, fail);
}

int dm_config_tree_find_bool(const struct dm_config_tree *cft, const char *path, int fail)
{
	return _find_config_bool(cft, _find_first_config_node, path, fail);
}

/************************************/


int dm_config_get_uint32(const struct dm_config_node *cn, const char *path,
			 uint32_t *result)
{
	const struct dm_config_node *n;

	n = _find_config_node(cn, path);

	if (!n || !n->v || n->v->type != DM_CFG_INT)
		return 0;

	if (result)
		*result = n->v->v.i;
	return 1;
}

int dm_config_get_uint64(const struct dm_config_node *cn, const char *path,
			 uint64_t *result)
{
	const struct dm_config_node *n;

	n = _find_config_node(cn, path);

	if (!n || !n->v || n->v->type != DM_CFG_INT)
		return 0;

	if (result)
		*result = (uint64_t) n->v->v.i;
	return 1;
}

int dm_config_get_str(const struct dm_config_node *cn, const char *path,
		      const char **result)
{
	const struct dm_config_node *n;

	n = _find_config_node(cn, path);

	if (!n || !n->v || n->v->type != DM_CFG_STRING)
		return 0;

	if (result)
		*result = n->v->v.str;
	return 1;
}

int dm_config_get_list(const struct dm_config_node *cn, const char *path,
		       const struct dm_config_value **result)
{
	const struct dm_config_node *n;

	n = _find_config_node(cn, path);
	/* TODO when we represent single-item lists consistently, add a check
	 * for n->v->next != NULL */
	if (!n || !n->v)
		return 0;

	if (result)
		*result = n->v;
	return 1;
}

int dm_config_get_section(const struct dm_config_node *cn, const char *path,
			  const struct dm_config_node **result)
{
	const struct dm_config_node *n;

	n = _find_config_node(cn, path);
	if (!n || n->v)
		return 0;

	if (result)
		*result = n;
	return 1;
}

int dm_config_has_node(const struct dm_config_node *cn, const char *path)
{
	return _find_config_node(cn, path) ? 1 : 0;
}

/*
 * Convert a token type to the char it represents.
 */
static char _token_type_to_char(int type)
{
	switch (type) {
		case TOK_SECTION_B:
			return SECTION_B_CHAR;
		case TOK_SECTION_E:
			return SECTION_E_CHAR;
		default:
			return 0;
	}
}

/*
 * Returns:
 *  # of 'type' tokens in 'str'.
 */
static unsigned _count_tokens(const char *str, unsigned len, int type)
{
	char c;

	c = _token_type_to_char(type);

	return dm_count_chars(str, len, c);
}

const char *dm_config_parent_name(const struct dm_config_node *n)
{
	return (n->parent ? n->parent->key : "(root)");
}
/*
 * Heuristic function to make a quick guess as to whether a text
 * region probably contains a valid config "section".  (Useful for
 * scanning areas of the disk for old metadata.)
 * Config sections contain various tokens, may contain other sections
 * and strings, and are delimited by begin (type 'TOK_SECTION_B') and
 * end (type 'TOK_SECTION_E') tokens.  As a quick heuristic, we just
 * count the number of begin and end tokens, and see if they are
 * non-zero and the counts match.
 * Full validation of the section should be done with another function
 * (for example, read_config_fd).
 *
 * Returns:
 *  0 - probably is not a valid config section
 *  1 - probably _is_ a valid config section
 */
unsigned dm_config_maybe_section(const char *str, unsigned len)
{
	int begin_count;
	int end_count;

	begin_count = _count_tokens(str, len, TOK_SECTION_B);
	end_count = _count_tokens(str, len, TOK_SECTION_E);

	if (begin_count && end_count && (begin_count == end_count))
		return 1;
	else
		return 0;
}

__attribute__((nonnull(1, 2)))
static struct dm_config_value *_clone_config_value(struct dm_pool *mem,
						   const struct dm_config_value *v)
{
	struct dm_config_value *new_cv;

	if (!(new_cv = _create_value(mem))) {
		log_error("Failed to clone config value.");
		return NULL;
	}

	new_cv->type = v->type;
	if (v->type == DM_CFG_STRING) {
		if (!(new_cv->v.str = dm_pool_strdup(mem, v->v.str))) {
			log_error("Failed to clone config string value.");
			return NULL;
		}
	} else
		new_cv->v = v->v;

	if (v->next && !(new_cv->next = _clone_config_value(mem, v->next)))
		return_NULL;

	return new_cv;
}

struct dm_config_node *dm_config_clone_node_with_mem(struct dm_pool *mem, const struct dm_config_node *cn, int siblings)
{
	struct dm_config_node *new_cn;

	if (!cn) {
		log_error("Cannot clone NULL config node.");
		return NULL;
	}

	if (!(new_cn = _create_node(mem))) {
		log_error("Failed to clone config node.");
		return NULL;
	}

	if ((cn->key && !(new_cn->key = dm_pool_strdup(mem, cn->key)))) {
		log_error("Failed to clone config node key.");
		return NULL;
	}

	new_cn->id = cn->id;

	if ((cn->v && !(new_cn->v = _clone_config_value(mem, cn->v))) ||
	    (cn->child && !(new_cn->child = dm_config_clone_node_with_mem(mem, cn->child, 1))) ||
	    (siblings && cn->sib && !(new_cn->sib = dm_config_clone_node_with_mem(mem, cn->sib, siblings))))
		return_NULL; /* 'new_cn' released with mem pool */

	return new_cn;
}

struct dm_config_node *dm_config_clone_node(struct dm_config_tree *cft, const struct dm_config_node *node, int sib)
{
	return dm_config_clone_node_with_mem(cft->mem, node, sib);
}

struct dm_config_node *dm_config_create_node(struct dm_config_tree *cft, const char *key)
{
	struct dm_config_node *cn;

	if (!(cn = _create_node(cft->mem))) {
		log_error("Failed to create config node.");
		return NULL;
	}
	if (!(cn->key = dm_pool_strdup(cft->mem, key))) {
		log_error("Failed to create config node's key.");
		return NULL;
	}
	cn->parent = NULL;
	cn->v = NULL;

	return cn;
}

struct dm_config_value *dm_config_create_value(struct dm_config_tree *cft)
{
	return _create_value(cft->mem);
}

void dm_config_value_set_format_flags(struct dm_config_value *cv, uint32_t format_flags)
{
	if (!cv)
		return;

	cv->format_flags = format_flags;
}

uint32_t dm_config_value_get_format_flags(struct dm_config_value *cv)
{
	if (!cv)
		return 0;

	return cv->format_flags;
}

struct dm_pool *dm_config_memory(struct dm_config_tree *cft)
{
	return cft->mem;
}

static int _override_path(const char *path, struct dm_config_node *node, void *baton)
{
	struct dm_config_tree *cft = baton;
	struct dm_config_node dummy, *target;
	dummy.child = cft->root;
	if (!(target = _find_or_make_node(cft->mem, &dummy, path, 0)))
		return_0;
	if (!(target->v = _clone_config_value(cft->mem, node->v)))
		return_0;
	cft->root = dummy.child;
	return 1;
}

static int _enumerate(const char *path, struct dm_config_node *cn, int (*cb)(const char *, struct dm_config_node *, void *), void *baton)
{
	char *sub = NULL;

	while (cn) {
		if (dm_asprintf(&sub, "%s/%s", path, cn->key) < 0)
			return_0;
		if (cn->child) {
			if (!_enumerate(sub, cn->child, cb, baton))
				goto_bad;
		} else
			if (!cb(sub, cn, baton))
				goto_bad;
		free(sub);
		cn = cn->sib;
	}
	return 1;
bad:
	free(sub);
	return 0;
}

struct dm_config_tree *dm_config_flatten(struct dm_config_tree *cft)
{
	struct dm_config_tree *res = dm_config_create(), *done = NULL, *current = NULL;

	if (!res)
		return_NULL;

	while (done != cft) {
		current = cft;
		while (current->cascade != done)
			current = current->cascade;
		_enumerate("", current->root, _override_path, res);
		done = current;
	}

	return res;
}

int dm_config_remove_node(struct dm_config_node *parent, struct dm_config_node *rem_node)
{
	struct dm_config_node *cn = parent->child, *last = NULL;
	while (cn) {
		if (cn == rem_node) {
			if (last)
				last->sib = cn->sib;
			else
				parent->child = cn->sib;
			return 1;
		}
		last = cn;
		cn = cn->sib;
	}
	return 0;
}
