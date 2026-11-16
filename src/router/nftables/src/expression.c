/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>

#include <stddef.h>
#include <stdio.h>
#include <limits.h>

#include <expression.h>
#include <statement.h>
#include <datatype.h>
#include <netlink.h>
#include <rule.h>
#include <gmputil.h>
#include <utils.h>
#include <list.h>
#include <erec.h>
#include <json.h>

extern const struct expr_ops ct_expr_ops;
extern const struct expr_ops fib_expr_ops;
extern const struct expr_ops hash_expr_ops;
extern const struct expr_ops inner_expr_ops;
extern const struct expr_ops meta_expr_ops;
extern const struct expr_ops numgen_expr_ops;
extern const struct expr_ops osf_expr_ops;
extern const struct expr_ops payload_expr_ops;
extern const struct expr_ops rt_expr_ops;
extern const struct expr_ops socket_expr_ops;
extern const struct expr_ops xfrm_expr_ops;

struct expr *expr_alloc(const struct location *loc, enum expr_types etype,
			const struct datatype *dtype, enum byteorder byteorder,
			unsigned int len)
{
	struct expr *expr;

	expr = xzalloc(sizeof(*expr));
	expr->location  = *loc;
	expr->dtype	= datatype_get(dtype);
	expr->etype	= etype;
	expr->byteorder	= byteorder;
	expr->len	= len;
	expr->refcnt	= 1;
	init_list_head(&expr->list);
	return expr;
}

struct expr *expr_clone(const struct expr *expr)
{
	struct expr *new;

	new = expr_alloc(&expr->location, expr->etype,
			 expr->dtype, expr->byteorder, expr->len);
	new->flags = expr->flags;
	new->op    = expr->op;
	expr_ops(expr)->clone(new, expr);
	return new;
}

struct expr *expr_get(struct expr *expr)
{
	assert_refcount_safe(expr->refcnt);
	expr->refcnt++;
	return expr;
}

static void expr_destroy(struct expr *e)
{
	const struct expr_ops *ops = expr_ops(e);

	if (ops->destroy)
		ops->destroy(e);
}

void expr_free(struct expr *expr)
{
	if (expr == NULL)
		return;

	assert_refcount_safe(expr->refcnt);
	if (--expr->refcnt > 0)
		return;

	datatype_free(expr->dtype);

	/* EXPR_INVALID expressions lack ->ops structure.
	 * This happens for set, list and concat types.
	 */
	if (expr->etype != EXPR_INVALID)
		expr_destroy(expr);
	free(expr);
}

void expr_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct expr_ops *ops = expr_ops(expr);

	if (ops->print)
		ops->print(expr, octx);
}

bool expr_cmp(const struct expr *e1, const struct expr *e2)
{
	assert(e1->flags & EXPR_F_SINGLETON);
	assert(e2->flags & EXPR_F_SINGLETON);

	if (e1->etype != e2->etype)
		return false;

	return expr_ops(e1)->cmp(e1, e2);
}

const char *expr_name(const struct expr *e)
{
	return expr_ops(e)->name;
}

void expr_describe(const struct expr *expr, struct output_ctx *octx)
{
	const struct datatype *dtype = expr->dtype, *edtype = NULL;
	unsigned int len = expr->len;
	const char *delim = "";

	if (dtype == &invalid_type &&
	    expr->etype == EXPR_SYMBOL)
		edtype = datatype_lookup_byname(expr->identifier);

	if (edtype) {
		dtype = edtype;
		nft_print(octx, "datatype %s (%s)",
			  dtype->name, dtype->desc);
		len = dtype->size;
	} else {
		nft_print(octx, "%s expression, datatype %s (%s)",
			  expr_name(expr), dtype->name, dtype->desc);

		if (dtype == &invalid_type) {
			nft_print(octx, "\n");
			return;
		}
	}

	if (dtype->basetype != NULL) {
		nft_print(octx, " (basetype ");
		for (dtype = dtype->basetype; dtype != NULL;
		     dtype = dtype->basetype) {
			nft_print(octx, "%s%s", delim, dtype->desc);
			delim = ", ";
		}
		nft_print(octx, ")");
	}

	if (expr_basetype(expr)->type == TYPE_STRING) {
		if (len)
			nft_print(octx, ", %u characters",
				  len / BITS_PER_BYTE);
		else
			nft_print(octx, ", dynamic length");
	} else
		nft_print(octx, ", %u bits", len);

	if (!edtype)
		edtype = expr->dtype;

	nft_print(octx, "\n");

	if (edtype->sym_tbl != NULL) {
		nft_print(octx, "\npre-defined symbolic constants ");
		if (edtype->sym_tbl->base == BASE_DECIMAL)
			nft_print(octx, "(in decimal):\n");
		else
			nft_print(octx, "(in hexadecimal):\n");
		symbol_table_print(edtype->sym_tbl, edtype,
				   expr->byteorder, octx);
	} else if (edtype->describe) {
		edtype->describe(octx);
	}
}

void expr_set_type(struct expr *expr, const struct datatype *dtype,
		   enum byteorder byteorder)
{
	const struct expr_ops *ops = expr_ops(expr);

	if (ops->set_type)
		ops->set_type(expr, dtype, byteorder);
	else {
		datatype_set(expr, dtype);
		expr->byteorder	= byteorder;
	}
}

const struct datatype *expr_basetype(const struct expr *expr)
{
	const struct datatype *type = expr->dtype;

	while (type->basetype != NULL)
		type = type->basetype;
	return type;
}

int __fmtstring(4, 5) expr_binary_error(struct list_head *msgs,
					const struct expr *e1, const struct expr *e2,
					const char *fmt, ...)
{
	struct error_record *erec;
	va_list ap;

	va_start(ap, fmt);
	erec = erec_vcreate(EREC_ERROR, &e1->location, fmt, ap);
	if (e2 != NULL)
		erec_add_location(erec, &e2->location);
	va_end(ap);
	erec_queue(erec, msgs);
	return -1;
}

static void verdict_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	datatype_print(expr, octx);
}

static bool verdict_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	if (e1->verdict != e2->verdict)
		return false;

	if ((e1->verdict == NFT_JUMP ||
	     e1->verdict == NFT_GOTO) &&
	     expr_cmp(e1->chain, e2->chain))
		return true;

	return false;
}

static void verdict_expr_clone(struct expr *new, const struct expr *expr)
{
	new->verdict = expr->verdict;
	if (expr->chain != NULL)
		new->chain = expr_clone(expr->chain);
}

static void verdict_expr_destroy(struct expr *expr)
{
	expr_free(expr->chain);
}

static int verdict_expr_build_udata(struct nftnl_udata_buf *udbuf,
				    const struct expr *expr)
{
	return 0;
}

static struct expr *verdict_expr_parse_udata(const struct nftnl_udata *attr)
{
	struct expr *e;

	e = symbol_expr_alloc(&internal_location, SYMBOL_VALUE, NULL, "verdict");
	e->dtype = &verdict_type;
	e->len = NFT_REG_SIZE * BITS_PER_BYTE;
	return e;
}

static const struct expr_ops verdict_expr_ops = {
	.type		= EXPR_VERDICT,
	.name		= "verdict",
	.print		= verdict_expr_print,
	.json		= verdict_expr_json,
	.cmp		= verdict_expr_cmp,
	.clone		= verdict_expr_clone,
	.destroy	= verdict_expr_destroy,
	.build_udata	= verdict_expr_build_udata,
	.parse_udata	= verdict_expr_parse_udata,
};

struct expr *verdict_expr_alloc(const struct location *loc,
				int verdict, struct expr *chain)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_VERDICT, &verdict_type,
			  BYTEORDER_INVALID, 0);
	expr->verdict = verdict;
	if (chain != NULL)
		expr->chain = chain;
	expr->flags = EXPR_F_CONSTANT | EXPR_F_SINGLETON;
	return expr;
}

static void symbol_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_print(octx, "%s", expr->identifier);
}

static void symbol_expr_clone(struct expr *new, const struct expr *expr)
{
	new->symtype	= expr->symtype;
	new->scope      = expr->scope;
	new->identifier = xstrdup(expr->identifier);
}

static void symbol_expr_destroy(struct expr *expr)
{
	free_const(expr->identifier);
}

static const struct expr_ops symbol_expr_ops = {
	.type		= EXPR_SYMBOL,
	.name		= "symbol",
	.print		= symbol_expr_print,
	.clone		= symbol_expr_clone,
	.destroy	= symbol_expr_destroy,
};

struct expr *symbol_expr_alloc(const struct location *loc,
			       enum symbol_types type, struct scope *scope,
			       const char *identifier)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_SYMBOL, &invalid_type,
			  BYTEORDER_INVALID, 0);
	expr->symtype	 = type;
	expr->scope	 = scope;
	expr->identifier = xstrdup(identifier);
	return expr;
}

static void variable_expr_print(const struct expr *expr,
				struct output_ctx *octx)
{
	nft_print(octx, "$%s", expr->sym->identifier);
}

static void variable_expr_clone(struct expr *new, const struct expr *expr)
{
	new->scope      = expr->scope;
	new->sym	= expr->sym;

	assert_refcount_safe(expr->sym->refcnt);
	expr->sym->refcnt++;
}

static void variable_expr_destroy(struct expr *expr)
{
	assert_refcount_safe(expr->sym->refcnt);
	expr->sym->refcnt--;
}

static const struct expr_ops variable_expr_ops = {
	.type		= EXPR_VARIABLE,
	.name		= "variable",
	.print		= variable_expr_print,
	.clone		= variable_expr_clone,
	.destroy	= variable_expr_destroy,
};

struct expr *variable_expr_alloc(const struct location *loc,
				 struct scope *scope, struct symbol *sym)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_VARIABLE, &invalid_type,
			  BYTEORDER_INVALID, 0);
	expr->scope	 = scope;
	expr->sym	 = sym;
	return expr;
}

#define NFTNL_UDATA_CONSTANT_TYPE 0
#define NFTNL_UDATA_CONSTANT_MAX NFTNL_UDATA_CONSTANT_TYPE

#define CONSTANT_EXPR_NFQUEUE_ID 0

static int constant_expr_build_udata(struct nftnl_udata_buf *udbuf,
				     const struct expr *expr)
{
	uint32_t type;

	if (expr->dtype == &queue_type)
		type = CONSTANT_EXPR_NFQUEUE_ID;
	else
		return -1;

	if (!nftnl_udata_put_u32(udbuf, NFTNL_UDATA_CONSTANT_TYPE, type))
		return -1;

	return 0;
}

static int constant_parse_udata(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);
	uint32_t value;

	switch (type) {
	case NFTNL_UDATA_CONSTANT_TYPE:
		if (len != sizeof(uint32_t))
			return -1;

		value = nftnl_udata_get_u32(attr);
		switch (value) {
		case CONSTANT_EXPR_NFQUEUE_ID:
			break;
		default:
			return -1;
		}
		break;
	default:
		return 0;
	}

	ud[type] = attr;

	return 0;
}

static struct expr *constant_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_CONSTANT_MAX + 1] = {};
	const struct datatype *dtype = NULL;
	uint32_t type;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				constant_parse_udata, ud);
	if (err < 0)
		return NULL;

	if (!ud[NFTNL_UDATA_CONSTANT_TYPE])
		return NULL;

	type = nftnl_udata_get_u32(ud[NFTNL_UDATA_CONSTANT_TYPE]);
	switch (type) {
	case CONSTANT_EXPR_NFQUEUE_ID:
		dtype = &queue_type;
		break;
	default:
		break;
	}

	return constant_expr_alloc(&internal_location, dtype, BYTEORDER_HOST_ENDIAN,
				   16, NULL);
}

static void constant_expr_print(const struct expr *expr,
				 struct output_ctx *octx)
{
	datatype_print(expr, octx);
}

static bool constant_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return expr_basetype(e1) == expr_basetype(e2) &&
	       !mpz_cmp(e1->value, e2->value);
}

static void constant_expr_clone(struct expr *new, const struct expr *expr)
{
	mpz_init_set(new->value, expr->value);
}

static void constant_expr_destroy(struct expr *expr)
{
	mpz_clear(expr->value);
}

static const struct expr_ops constant_expr_ops = {
	.type		= EXPR_VALUE,
	.name		= "value",
	.print		= constant_expr_print,
	.json		= constant_expr_json,
	.cmp		= constant_expr_cmp,
	.clone		= constant_expr_clone,
	.destroy	= constant_expr_destroy,
	.build_udata	= constant_expr_build_udata,
	.parse_udata	= constant_expr_parse_udata,
};

struct expr *constant_expr_alloc(const struct location *loc,
				 const struct datatype *dtype,
				 enum byteorder byteorder,
				 unsigned int len, const void *data)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_VALUE, dtype, byteorder, len);
	expr->flags = EXPR_F_CONSTANT | EXPR_F_SINGLETON;

	mpz_init2(expr->value, len);
	if (data != NULL && len)
		mpz_import_data(expr->value, data, byteorder,
				div_round_up(len, BITS_PER_BYTE));

	return expr;
}

struct expr *constant_expr_join(const struct expr *e1, const struct expr *e2)
{
	unsigned int len = (e1->len + e2->len) / BITS_PER_BYTE, tmp;
	unsigned char data[len];

	assert(e1->etype == EXPR_VALUE);
	assert(e2->etype == EXPR_VALUE);

	tmp = e1->len / BITS_PER_BYTE;
	mpz_export_data(data, e1->value, e1->byteorder, tmp);
	mpz_export_data(data + tmp, e2->value, e2->byteorder,
			e2->len / BITS_PER_BYTE);

	return constant_expr_alloc(&e1->location, &invalid_type,
				   BYTEORDER_INVALID, len * BITS_PER_BYTE,
				   data);
}

struct expr *constant_expr_splice(struct expr *expr, unsigned int len)
{
	struct expr *slice;
	mpz_t mask;

	assert(expr->etype == EXPR_VALUE);
	assert(len <= expr->len);

	slice = constant_expr_alloc(&expr->location, &invalid_type,
				    BYTEORDER_INVALID, len, NULL);
	mpz_init2(mask, len);
	mpz_bitmask(mask, len);
	mpz_lshift_ui(mask, expr->len - len);

	mpz_set(slice->value, expr->value);
	mpz_and(slice->value, slice->value, mask);
	mpz_rshift_ui(slice->value, expr->len - len);
	mpz_clear(mask);

	expr->len -= len;
	return slice;
}

static void constant_range_expr_print_one(const struct expr *expr,
					  const mpz_t value,
					  struct output_ctx *octx)
{
	unsigned int len = div_round_up(expr->len, BITS_PER_BYTE);
	unsigned char data[len];
	struct expr *dummy;

	/* create dummy temporary constant expression to print range. */
	mpz_export_data(data, value, expr->byteorder, len);
	dummy = constant_expr_alloc(&expr->location, expr->dtype,
				    expr->byteorder, expr->len, data);
	expr_print(dummy, octx);
	expr_free(dummy);
}

static void constant_range_expr_print(const struct expr *expr,
				      struct output_ctx *octx)
{
	unsigned int flags = octx->flags;

	/* similar to range_expr_print(). */
	octx->flags &= ~(NFT_CTX_OUTPUT_SERVICE |
			 NFT_CTX_OUTPUT_REVERSEDNS |
			 NFT_CTX_OUTPUT_GUID);
	octx->flags |= NFT_CTX_OUTPUT_NUMERIC_ALL;

	constant_range_expr_print_one(expr, expr->range.low, octx);
	nft_print(octx, "-");
	constant_range_expr_print_one(expr, expr->range.high, octx);

	octx->flags = flags;
}

static bool constant_range_expr_cmp(const struct expr *e1, const struct expr *e2)
{
	return expr_basetype(e1) == expr_basetype(e2) &&
	       !mpz_cmp(e1->range.low, e2->range.low) &&
	       !mpz_cmp(e1->range.high, e2->range.high);
}

static void constant_range_expr_clone(struct expr *new, const struct expr *expr)
{
	mpz_init_set(new->range.low, expr->range.low);
	mpz_init_set(new->range.high, expr->range.high);
}

static void constant_range_expr_destroy(struct expr *expr)
{
	mpz_clear(expr->range.low);
	mpz_clear(expr->range.high);
}

static const struct expr_ops constant_range_expr_ops = {
	.type		= EXPR_RANGE_VALUE,
	.name		= "range_value",
	.print		= constant_range_expr_print,
	.cmp		= constant_range_expr_cmp,
	.clone		= constant_range_expr_clone,
	.destroy	= constant_range_expr_destroy,
};

struct expr *constant_range_expr_alloc(const struct location *loc,
				       const struct datatype *dtype,
				       enum byteorder byteorder,
				       unsigned int len, mpz_t low, mpz_t high)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_RANGE_VALUE, dtype, byteorder, len);
	expr->flags = EXPR_F_CONSTANT;

	mpz_init_set(expr->range.low, low);
	mpz_init_set(expr->range.high, high);

	return expr;
}

static void symbol_range_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	nft_print(octx, "%s", expr->identifier_range[0]);
	nft_print(octx, "-");
	nft_print(octx, "%s", expr->identifier_range[1]);
}

static void symbol_range_expr_clone(struct expr *new, const struct expr *expr)
{
	new->symtype	= expr->symtype;
	new->scope      = expr->scope;
	new->identifier_range[0] = xstrdup(expr->identifier_range[0]);
	new->identifier_range[1] = xstrdup(expr->identifier_range[1]);
}

static void symbol_range_expr_destroy(struct expr *expr)
{
	free_const(expr->identifier_range[0]);
	free_const(expr->identifier_range[1]);
}

static const struct expr_ops symbol_range_expr_ops = {
	.type		= EXPR_RANGE_SYMBOL,
	.name		= "range_symbol",
	.print		= symbol_range_expr_print,
	.clone		= symbol_range_expr_clone,
	.destroy	= symbol_range_expr_destroy,
};

struct expr *symbol_range_expr_alloc(const struct location *loc,
				     enum symbol_types type, const struct scope *scope,
				     const char *identifier_low, const char *identifier_high)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_RANGE_SYMBOL, &invalid_type,
			  BYTEORDER_INVALID, 0);
	expr->symtype	 = type;
	expr->scope	 = scope;
	expr->identifier_range[0] = xstrdup(identifier_low);
	expr->identifier_range[1] = xstrdup(identifier_high);
	return expr;
}

/*
 * Allocate a constant expression with a single bit set at position n.
 */
struct expr *flag_expr_alloc(const struct location *loc,
			     const struct datatype *dtype,
			     enum byteorder byteorder,
			     unsigned int len, unsigned long n)
{
	struct expr *expr;

	assert(n < len);

	expr = constant_expr_alloc(loc, dtype, byteorder, len, NULL);
	mpz_set_ui(expr->value, 1);
	mpz_lshift_ui(expr->value, n);

	return expr;
}

/*
 * Convert an expression of basetype TYPE_BITMASK into a series of inclusive
 * OR binop expressions of the individual flag values.
 */
struct expr *bitmask_expr_to_binops(struct expr *expr)
{
	struct expr *binop, *flag;
	unsigned long n;

	assert(expr->etype == EXPR_VALUE);
	assert(expr->dtype->basetype->type == TYPE_BITMASK);

	n = mpz_popcount(expr->value);
	if (n == 0 || n == 1)
		return expr;

	binop = NULL;
	n = 0;
	while ((n = mpz_scan1(expr->value, n)) != ULONG_MAX) {
		flag = flag_expr_alloc(&expr->location, expr->dtype,
				       expr->byteorder, expr->len, n);
		if (binop != NULL)
			binop = binop_expr_alloc(&expr->location,
						 OP_OR, binop, flag);
		else
			binop = flag;

		n++;
	}

	expr_free(expr);
	return binop;
}

static void prefix_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	expr_print(expr->prefix, octx);
	nft_print(octx, "/%u", expr->prefix_len);
}

static void prefix_expr_set_type(const struct expr *expr,
				 const struct datatype *type,
				 enum byteorder byteorder)
{
	expr_set_type(expr->prefix, type, byteorder);
}

static void prefix_expr_clone(struct expr *new, const struct expr *expr)
{
	new->prefix     = expr_clone(expr->prefix);
	new->prefix_len = expr->prefix_len;
}

static void prefix_expr_destroy(struct expr *expr)
{
	expr_free(expr->prefix);
}

static const struct expr_ops prefix_expr_ops = {
	.type		= EXPR_PREFIX,
	.name		= "prefix",
	.print		= prefix_expr_print,
	.json		= prefix_expr_json,
	.set_type	= prefix_expr_set_type,
	.clone		= prefix_expr_clone,
	.destroy	= prefix_expr_destroy,
};

struct expr *prefix_expr_alloc(const struct location *loc,
			       struct expr *expr, unsigned int prefix_len)
{
	struct expr *prefix;

	prefix = expr_alloc(loc, EXPR_PREFIX, &invalid_type,
			    BYTEORDER_INVALID, 0);
	prefix->prefix     = expr;
	prefix->prefix_len = prefix_len;
	return prefix;
}

const char *expr_op_symbols[] = {
	[OP_INVALID]	= "invalid",
	[OP_HTON]	= "hton",
	[OP_NTOH]	= "ntoh",
	[OP_AND]	= "&",
	[OP_OR]		= "|",
	[OP_XOR]	= "^",
	[OP_LSHIFT]	= "<<",
	[OP_RSHIFT]	= ">>",
	[OP_EQ]		= "==",
	[OP_NEQ]	= "!=",
	[OP_LT]		= "<",
	[OP_GT]		= ">",
	[OP_LTE]	= "<=",
	[OP_GTE]	= ">=",
	[OP_NEG]	= "!",
};

static void unary_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	expr_print(expr->arg, octx);
}

static void unary_expr_clone(struct expr *new, const struct expr *expr)
{
	new->arg = expr_clone(expr->arg);
}

static void unary_expr_destroy(struct expr *expr)
{
	expr_free(expr->arg);
}

static const struct expr_ops unary_expr_ops = {
	.type		= EXPR_UNARY,
	.name		= "unary",
	.print		= unary_expr_print,
	.json		= unary_expr_json,
	.clone		= unary_expr_clone,
	.destroy	= unary_expr_destroy,
};

struct expr *unary_expr_alloc(const struct location *loc,
			      enum ops op, struct expr *arg)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_UNARY, &invalid_type,
			  BYTEORDER_INVALID, 0);
	expr->op  = op;
	expr->arg = arg;
	return expr;
}

static uint8_t expr_binop_precedence[OP_MAX + 1] = {
	[OP_LSHIFT]	= 1,
	[OP_RSHIFT]	= 1,
	[OP_AND]	= 2,
	[OP_XOR]	= 3,
	[OP_OR]		= 4,
};

static void binop_arg_print(const struct expr *op, const struct expr *arg,
			     struct output_ctx *octx)
{
	bool prec = false;

	if (arg->etype == EXPR_BINOP &&
	    expr_binop_precedence[op->op] != 0 &&
	    expr_binop_precedence[op->op] < expr_binop_precedence[arg->op])
		prec = 1;

	if (prec)
		nft_print(octx, "(");
	expr_print(arg, octx);
	if (prec)
		nft_print(octx, ")");
}

bool must_print_eq_op(const struct expr *expr)
{
	if (expr->right->dtype->basetype != NULL &&
	    expr->right->dtype->basetype->type == TYPE_BITMASK &&
	    expr->right->etype == EXPR_VALUE)
		return true;

	return expr->left->etype == EXPR_BINOP;
}

static void binop_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	binop_arg_print(expr, expr->left, octx);

	if (expr_op_symbols[expr->op] &&
	    (expr->op != OP_EQ || must_print_eq_op(expr)))
		nft_print(octx, " %s ", expr_op_symbols[expr->op]);
	else
		nft_print(octx, " ");

	binop_arg_print(expr, expr->right, octx);
}

static void binop_expr_clone(struct expr *new, const struct expr *expr)
{
	new->left  = expr_clone(expr->left);
	new->right = expr_clone(expr->right);
}

static void binop_expr_destroy(struct expr *expr)
{
	expr_free(expr->left);
	expr_free(expr->right);
}

static const struct expr_ops binop_expr_ops = {
	.type		= EXPR_BINOP,
	.name		= "binop",
	.print		= binop_expr_print,
	.json		= binop_expr_json,
	.clone		= binop_expr_clone,
	.destroy	= binop_expr_destroy,
};

struct expr *binop_expr_alloc(const struct location *loc, enum ops op,
			      struct expr *left, struct expr *right)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_BINOP, left->dtype,
			  left->byteorder, 0);
	expr->left  = left;
	expr->op    = op;
	expr->right = right;
	return expr;
}

static const struct expr_ops relational_expr_ops = {
	.type		= EXPR_RELATIONAL,
	.name		= "relational",
	.print		= binop_expr_print,
	.json		= relational_expr_json,
	.destroy	= binop_expr_destroy,
};

struct expr *relational_expr_alloc(const struct location *loc, enum ops op,
				   struct expr *left, struct expr *right)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_RELATIONAL, &verdict_type,
			  BYTEORDER_INVALID, 0);
	expr->left  = left;
	expr->op    = op;
	expr->right = right;

	if (right->dtype == &boolean_type)
		left->flags |= EXPR_F_BOOLEAN;

	return expr;
}

void relational_expr_pctx_update(struct proto_ctx *ctx,
				 const struct expr *expr)
{
	const struct expr *left = expr->left, *right = expr->right;
	const struct expr_ops *ops;
	const struct expr *i;

	assert(expr->etype == EXPR_RELATIONAL);
	assert(expr->op == OP_EQ || expr->op == OP_IMPLICIT);

	ops = expr_ops(left);
	if (ops->pctx_update &&
	    (left->flags & EXPR_F_PROTOCOL)) {
		if (expr_is_singleton(right))
			ops->pctx_update(ctx, &expr->location, left, right);
		else if (right->etype == EXPR_SET) {
			list_for_each_entry(i, &expr_set(right)->expressions, list) {
				assert(i->etype == EXPR_SET_ELEM);

				if (i->key->etype == EXPR_VALUE)
					ops->pctx_update(ctx, &expr->location, left, i->key);
			}
		} else if (ops == &meta_expr_ops &&
			   right->etype == EXPR_SET_REF) {
			const struct expr *key = right->set->key;
			struct expr *tmp;

			tmp = constant_expr_alloc(&expr->location, key->dtype,
						  key->byteorder, key->len,
						  NULL);

			ops->pctx_update(ctx, &expr->location, left, tmp);
			expr_free(tmp);
		}
	}
}

static void range_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	unsigned int flags = octx->flags;

	octx->flags &= ~(NFT_CTX_OUTPUT_SERVICE |
			 NFT_CTX_OUTPUT_REVERSEDNS |
			 NFT_CTX_OUTPUT_GUID);
	octx->flags |= NFT_CTX_OUTPUT_NUMERIC_ALL;
	expr_print(expr->left, octx);
	nft_print(octx, "-");
	expr_print(expr->right, octx);
	octx->flags = flags;
}

static void range_expr_clone(struct expr *new, const struct expr *expr)
{
	new->left  = expr_clone(expr->left);
	new->right = expr_clone(expr->right);
}

static void range_expr_destroy(struct expr *expr)
{
	expr_free(expr->left);
	expr_free(expr->right);
}

static void range_expr_set_type(const struct expr *expr,
				const struct datatype *type,
				enum byteorder byteorder)
{
	expr_set_type(expr->left, type, byteorder);
	expr_set_type(expr->right, type, byteorder);
}

static const struct expr_ops range_expr_ops = {
	.type		= EXPR_RANGE,
	.name		= "range",
	.print		= range_expr_print,
	.json		= range_expr_json,
	.clone		= range_expr_clone,
	.destroy	= range_expr_destroy,
	.set_type	= range_expr_set_type,
};

struct expr *range_expr_alloc(const struct location *loc,
			      struct expr *left, struct expr *right)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_RANGE, &invalid_type,
			  BYTEORDER_INVALID, 0);
	expr->left  = left;
	expr->right = right;
	expr->len = left->len;
	return expr;
}

static void concat_expr_destroy(struct expr *expr)
{
	struct expr *i, *next;

	list_for_each_entry_safe(i, next, &expr_concat(expr)->expressions, list)
		expr_free(i);
}

static void concat_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct expr *i;
	const char *d = "";

	list_for_each_entry(i, &expr_concat(expr)->expressions, list) {
		nft_print(octx, "%s", d);
		expr_print(i, octx);
		d = " . ";
	}
}

static void concat_expr_clone(struct expr *new, const struct expr *expr)
{
	struct expr *i;

	init_list_head(&expr_concat(new)->expressions);
	list_for_each_entry(i, &expr_concat(expr)->expressions, list)
		concat_expr_add(new, expr_clone(i));
}

#define NFTNL_UDATA_SET_KEY_CONCAT_NEST 0
#define NFTNL_UDATA_SET_KEY_CONCAT_NEST_MAX  NFT_REG32_SIZE

#define NFTNL_UDATA_SET_KEY_CONCAT_SUB_TYPE 0
#define NFTNL_UDATA_SET_KEY_CONCAT_SUB_DATA 1
#define NFTNL_UDATA_SET_KEY_CONCAT_SUB_MAX  2

static struct expr *expr_build_udata_recurse(struct expr *e)
{
	switch (e->etype) {
	case EXPR_BINOP:
		return expr_build_udata_recurse(e->left);
	default:
		break;
	}

	return e;
}

static int concat_expr_build_udata(struct nftnl_udata_buf *udbuf,
				    const struct expr *concat_expr)
{
	struct nftnl_udata *nest;
	struct expr *expr, *tmp;
	unsigned int i = 0;

	list_for_each_entry_safe(expr, tmp, &expr_concat(concat_expr)->expressions, list) {
		struct nftnl_udata *nest_expr;
		int err;

		expr = expr_build_udata_recurse(expr);
		if (!expr_ops(expr)->build_udata || i >= NFT_REG32_SIZE)
			return -1;

		nest = nftnl_udata_nest_start(udbuf, NFTNL_UDATA_SET_KEY_CONCAT_NEST + i);
		nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEY_CONCAT_SUB_TYPE, expr->etype);
		nest_expr = nftnl_udata_nest_start(udbuf, NFTNL_UDATA_SET_KEY_CONCAT_SUB_DATA);
		err = expr_ops(expr)->build_udata(udbuf, expr);
		if (err < 0)
			return err;
		nftnl_udata_nest_end(udbuf, nest_expr);
		nftnl_udata_nest_end(udbuf, nest);
		i++;
	}

	return 0;
}

static int concat_parse_udata_nest(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	if (type >= NFTNL_UDATA_SET_KEY_CONCAT_NEST_MAX)
		return -1;

	if (len <= sizeof(uint32_t))
		return -1;

	ud[type] = attr;
	return 0;
}

static int concat_parse_udata_nested(const struct nftnl_udata *attr, void *data)
{
	const struct nftnl_udata **ud = data;
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);

	switch (type) {
	case NFTNL_UDATA_SET_KEY_CONCAT_SUB_TYPE:
		if (len != sizeof(uint32_t))
			return -1;
		break;
	case NFTNL_UDATA_SET_KEY_CONCAT_SUB_DATA:
		if (len <= sizeof(uint32_t))
			return -1;
		break;
	default:
		return 0;
	}

	ud[type] = attr;
	return 0;
}

static struct expr *concat_expr_parse_udata(const struct nftnl_udata *attr)
{
	const struct nftnl_udata *ud[NFTNL_UDATA_SET_KEY_CONCAT_NEST_MAX] = {};
	const struct datatype *dtype;
	struct expr *concat_expr;
	uint32_t dt = 0, len = 0;
	unsigned int i;
	int err;

	err = nftnl_udata_parse(nftnl_udata_get(attr), nftnl_udata_len(attr),
				concat_parse_udata_nest, ud);
	if (err < 0)
		return NULL;

	concat_expr = concat_expr_alloc(&internal_location);
	if (!concat_expr)
		return NULL;

	for (i = 0; i < array_size(ud); i++) {
		const struct nftnl_udata *nest_ud[NFTNL_UDATA_SET_KEY_CONCAT_SUB_MAX];
		const struct nftnl_udata *nested, *subdata;
		const struct expr_ops *ops;
		struct expr *expr;
		uint32_t etype;

		if (ud[NFTNL_UDATA_SET_KEY_CONCAT_NEST + i] == NULL)
			break;

		nested = ud[NFTNL_UDATA_SET_KEY_CONCAT_NEST + i];
		err = nftnl_udata_parse(nftnl_udata_get(nested), nftnl_udata_len(nested),
					concat_parse_udata_nested, nest_ud);
		if (err < 0)
			goto err_free;

		etype = nftnl_udata_get_u32(nest_ud[NFTNL_UDATA_SET_KEY_CONCAT_SUB_TYPE]);
		ops = expr_ops_by_type_u32(etype);
		if (!ops || !ops->parse_udata)
			goto err_free;

		subdata = nest_ud[NFTNL_UDATA_SET_KEY_CONCAT_SUB_DATA];
		expr = ops->parse_udata(subdata);
		if (!expr)
			goto err_free;

		dt = concat_subtype_add(dt, expr->dtype->type);
		concat_expr_add(concat_expr, expr);
		len += netlink_padded_len(expr->len);
	}

	dtype = concat_type_alloc(dt);
	if (!dtype)
		goto err_free;

	__datatype_set(concat_expr, dtype);
	concat_expr->len = len;

	return concat_expr;

err_free:
	expr_free(concat_expr);
	return NULL;
}

static const struct expr_ops concat_expr_ops = {
	.type		= EXPR_CONCAT,
	.name		= "concat",
	.print		= concat_expr_print,
	.json		= concat_expr_json,
	.clone		= concat_expr_clone,
	.destroy	= concat_expr_destroy,
	.build_udata	= concat_expr_build_udata,
	.parse_udata	= concat_expr_parse_udata,
};

struct expr *concat_expr_alloc(const struct location *loc)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_CONCAT, &invalid_type, BYTEORDER_INVALID, 0);
	init_list_head(&expr_concat(expr)->expressions);

	return expr;
}

void concat_expr_add(struct expr *concat, struct expr *item)
{
	struct expr_concat *expr_concat = expr_concat(concat);

	list_add_tail(&item->list, &expr_concat->expressions);
	expr_concat->size++;
}

void concat_expr_remove(struct expr *concat, struct expr *expr)
{
	expr_concat(concat)->size--;
	list_del(&expr->list);
}

static void list_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct expr *i;
	const char *d = "";

	list_for_each_entry(i, &expr_list(expr)->expressions, list) {
		nft_print(octx, "%s", d);
		expr_print(i, octx);
		d = ",";
	}
}

static void list_expr_clone(struct expr *new, const struct expr *expr)
{
	struct expr *i;

	init_list_head(&expr_list(new)->expressions);
	list_for_each_entry(i, &expr_list(expr)->expressions, list)
		list_expr_add(new, expr_clone(i));
}

static void list_expr_destroy(struct expr *expr)
{
	struct expr *i, *next;

	list_for_each_entry_safe(i, next, &expr_list(expr)->expressions, list)
		expr_free(i);
}

static const struct expr_ops list_expr_ops = {
	.type		= EXPR_LIST,
	.name		= "list",
	.print		= list_expr_print,
	.json		= list_expr_json,
	.clone		= list_expr_clone,
	.destroy	= list_expr_destroy,
};

struct expr *list_expr_alloc(const struct location *loc)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_LIST, &invalid_type, BYTEORDER_INVALID, 0);
	init_list_head(&expr_list(expr)->expressions);

	return expr;
}

void list_expr_add(struct expr *expr, struct expr *item)
{
	struct expr_list *expr_list = expr_list(expr);

	list_add_tail(&item->list, &expr_list->expressions);
	expr_list->size++;
}

void list_expr_remove(struct expr *list, struct expr *expr)
{
	expr_list(list)->size--;
	list_del(&expr->list);
}

/* list is assumed to have two items at least, otherwise extend this! */
struct expr *list_expr_to_binop(struct expr *expr)
{
	struct expr *first, *last = NULL, *i;

	assert(!list_empty(&expr_list(expr)->expressions));

	first = list_first_entry(&expr_list(expr)->expressions, struct expr, list);
	i = first;

	list_for_each_entry_continue(i, &expr_list(expr)->expressions, list) {
		if (first) {
			last = binop_expr_alloc(&expr->location, OP_OR, first, i);
			first = NULL;
		} else {
			last = binop_expr_alloc(&expr->location, OP_OR, i, last);
		}
	}
	/* list with one single item only, this should not happen. */
	assert(!first);

	/* zap list expressions, they have been moved to binop expression. */
	init_list_head(&expr_list(expr)->expressions);
	expr_free(expr);

	return last;
}

static const char *calculate_delim(const struct expr *expr, int *count,
				   struct output_ctx *octx)
{
	const char *newline = ",\n\t\t\t     ";
	const char *singleline = ", ";

	if (octx->force_newline)
		return newline;

	if (set_is_anonymous(expr_set(expr)->set_flags))
		return singleline;

	if (!expr->dtype)
		return newline;

	switch (expr->dtype->type) {
	case TYPE_NFPROTO:
	case TYPE_INTEGER:
	case TYPE_ARPOP:
	case TYPE_INET_PROTOCOL:
	case TYPE_INET_SERVICE:
	case TYPE_TCP_FLAG:
	case TYPE_DCCP_PKTTYPE:
	case TYPE_MARK:
	case TYPE_IFINDEX:
	case TYPE_CLASSID:
	case TYPE_UID:
	case TYPE_GID:
	case TYPE_CT_DIR:
		if (*count < 5)
			return singleline;
		*count = 0;
		break;
	case TYPE_IPADDR:
	case TYPE_CT_STATE:
	case TYPE_CT_STATUS:
	case TYPE_PKTTYPE:
		if (*count < 2)
			return singleline;
		*count = 0;
		break;

	default:
		break;
	}

	return newline;
}

static void set_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	const struct expr *i;
	const char *d = "";
	int count = 0;

	nft_print(octx, "{ ");

	list_for_each_entry(i, &expr_set(expr)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		nft_print(octx, "%s", d);
		expr_print(i, octx);
		count++;
		d = calculate_delim(expr, &count, octx);
	}

	nft_print(octx, " }");
}

static void set_expr_clone(struct expr *new, const struct expr *expr)
{
	struct expr *i;

	init_list_head(&expr_set(new)->expressions);
	list_for_each_entry(i, &expr_set(expr)->expressions, list)
		set_expr_add(new, expr_clone(i));
}

static void set_expr_destroy(struct expr *expr)
{
	struct expr *i, *next;

	list_for_each_entry_safe(i, next, &expr_set(expr)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);
		expr_free(i);
	}
}

static void set_expr_set_type(const struct expr *expr,
			      const struct datatype *dtype,
			      enum byteorder byteorder)
{
	struct expr *i;

	list_for_each_entry(i, &expr_set(expr)->expressions, list) {
		assert(i->etype == EXPR_SET_ELEM);

		expr_set_type(i, dtype, byteorder);
	}
}

static const struct expr_ops set_expr_ops = {
	.type		= EXPR_SET,
	.name		= "set",
	.print		= set_expr_print,
	.json		= set_expr_json,
	.set_type	= set_expr_set_type,
	.clone		= set_expr_clone,
	.destroy	= set_expr_destroy,
};

struct expr *set_expr_alloc(const struct location *loc, const struct set *set)
{
	struct expr *set_expr;

	set_expr = expr_alloc(loc, EXPR_SET, &invalid_type, BYTEORDER_INVALID, 0);
	init_list_head(&expr_set(set_expr)->expressions);

	if (!set)
		return set_expr;

	expr_set(set_expr)->set_flags = set->flags;
	datatype_set(set_expr, set->key->dtype);

	return set_expr;
}

void set_expr_add(struct expr *set, struct expr *elem)
{
	struct expr_set *expr_set = expr_set(set);

	assert(elem->etype == EXPR_SET_ELEM);

	list_add_tail(&elem->list, &expr_set->expressions);
	expr_set->size++;
}

void set_expr_remove(struct expr *set, struct expr *expr)
{
	assert(expr->etype == EXPR_SET_ELEM);

	expr_set(set)->size--;
	list_del(&expr->list);
}

static void mapping_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	expr_print(expr->left, octx);
	nft_print(octx, " : ");
	expr_print(expr->right, octx);
}

static void mapping_expr_set_type(const struct expr *expr,
				  const struct datatype *dtype,
				  enum byteorder byteorder)
{
	expr_set_type(expr->left, dtype, byteorder);
}

static void mapping_expr_clone(struct expr *new, const struct expr *expr)
{
	new->left  = expr_clone(expr->left);
	new->right = expr_clone(expr->right);
}

static void mapping_expr_destroy(struct expr *expr)
{
	expr_free(expr->left);
	expr_free(expr->right);
}

static const struct expr_ops mapping_expr_ops = {
	.type		= EXPR_MAPPING,
	.name		= "mapping",
	.print		= mapping_expr_print,
	.json		= mapping_expr_json,
	.set_type	= mapping_expr_set_type,
	.clone		= mapping_expr_clone,
	.destroy	= mapping_expr_destroy,
};

struct expr *mapping_expr_alloc(const struct location *loc,
				struct expr *from, struct expr *to)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_MAPPING, from->dtype,
			  from->byteorder, 0);
	expr->left  = from;
	expr->right = to;
	return expr;
}

static bool __set_expr_is_vmap(const struct expr *mappings)
{
	const struct expr *elem;

	if (list_empty(&expr_set(mappings)->expressions))
		return false;

	elem = list_first_entry(&expr_set(mappings)->expressions, struct expr, list);
	assert(elem->etype == EXPR_SET_ELEM);

	if (elem->key->etype == EXPR_MAPPING &&
	    elem->key->right->etype == EXPR_VERDICT)
		return true;

	return false;
}

static bool set_expr_is_vmap(const struct expr *expr)
{

	if (expr->mappings->etype == EXPR_SET)
		return __set_expr_is_vmap(expr->mappings);

	return false;
}

static void map_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	expr_print(expr->map, octx);
	if ((expr->mappings->etype == EXPR_SET_REF &&
	     expr->mappings->set->data->dtype->type == TYPE_VERDICT) ||
	    set_expr_is_vmap(expr))
		nft_print(octx, " vmap ");
	else
		nft_print(octx, " map ");

	expr_print(expr->mappings, octx);
}

static void map_expr_clone(struct expr *new, const struct expr *expr)
{
	new->map      = expr_clone(expr->map);
	new->mappings = expr_clone(expr->mappings);
}

static void map_expr_destroy(struct expr *expr)
{
	expr_free(expr->map);
	expr_free(expr->mappings);
}

static const struct expr_ops map_expr_ops = {
	.type		= EXPR_MAP,
	.name		= "map",
	.print		= map_expr_print,
	.json		= map_expr_json,
	.clone		= map_expr_clone,
	.destroy	= map_expr_destroy,
};

struct expr *map_expr_alloc(const struct location *loc, struct expr *arg,
			    struct expr *mappings)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_MAP, &invalid_type, BYTEORDER_INVALID, 0);
	expr->map      = arg;
	expr->mappings = mappings;
	return expr;
}

static void set_ref_expr_print(const struct expr *expr, struct output_ctx *octx)
{
	if (set_is_meter(expr->set->flags))
		nft_print(octx, "%s", expr->set->handle.set.name);
	else if (set_is_anonymous(expr->set->flags))
		expr_print(expr->set->init, octx);
	else
		nft_print(octx, "@%s", expr->set->handle.set.name);
}

static void set_ref_expr_clone(struct expr *new, const struct expr *expr)
{
	new->set = set_get(expr->set);
}

static void set_ref_expr_destroy(struct expr *expr)
{
	set_free(expr->set);
}

static void set_ref_expr_set_type(const struct expr *expr,
				  const struct datatype *dtype,
				  enum byteorder byteorder)
{
	const struct set *s = expr->set;

	/* normal sets already have a precise datatype that is given in
	 * the set definition via type foo.
	 *
	 * Anon sets do not have this, and need to rely on type info
	 * generated at rule creation time.
	 *
	 * For most cases, the type info is correct.
	 * In some cases however, the kernel only stores TYPE_INTEGER.
	 *
	 * This happens with expressions that only use an integer alias
	 * type, e.g. the mptcpopt_subtype datatype.
	 *
	 * In this case nft will print the elements as numerical values
	 * because the base type lacks the ->sym_tbl information of the
	 * subtypes.
	 */
	if (s->init && set_is_anonymous(s->flags))
		expr_set_type(s->init, dtype, byteorder);
}

static const struct expr_ops set_ref_expr_ops = {
	.type		= EXPR_SET_REF,
	.name		= "set reference",
	.print		= set_ref_expr_print,
	.json		= set_ref_expr_json,
	.clone		= set_ref_expr_clone,
	.destroy	= set_ref_expr_destroy,
	.set_type	= set_ref_expr_set_type,
};

struct expr *set_ref_expr_alloc(const struct location *loc, struct set *set)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_SET_REF, set->key->dtype, 0, 0);
	expr->set = set_get(set);
	expr->flags |= EXPR_F_CONSTANT;
	return expr;
}

static void set_elem_expr_print(const struct expr *expr,
				 struct output_ctx *octx)
{
	struct stmt *stmt;

	/* The mapping output needs to print lhs first, then timeout, expires,
	 * comment and list of statements and finally rhs.
	 *
	 * Because EXPR_SET_ELEM always comes before EXPR_MAPPING, add this
	 * special handling to print the output accordingly.
	 */
	if (expr->key->etype == EXPR_MAPPING)
		expr_print(expr->key->left, octx);
	else
		expr_print(expr->key, octx);

	list_for_each_entry(stmt, &expr->stmt_list, list) {
		nft_print(octx, " ");
		stmt_print(stmt, octx);
	}
	if (expr->timeout) {
		nft_print(octx, " timeout ");
		if (expr->timeout == NFT_NEVER_TIMEOUT)
			nft_print(octx, "never");
		else
			time_print(expr->timeout, octx);
	}
	if (!nft_output_stateless(octx) &&
	    expr->timeout != NFT_NEVER_TIMEOUT &&
	    expr->expiration) {
		nft_print(octx, " expires ");
		time_print(expr->expiration, octx);
	}
	if (expr->comment)
		nft_print(octx, " comment \"%s\"", expr->comment);

	if (expr->key->etype == EXPR_MAPPING) {
		nft_print(octx, " : ");
		expr_print(expr->key->right, octx);
	}
}

static void set_elem_expr_destroy(struct expr *expr)
{
	struct stmt *stmt, *next;

	assert(expr->flags == 0);

	free_const(expr->comment);
	expr_free(expr->key);
	list_for_each_entry_safe(stmt, next, &expr->stmt_list, list)
		stmt_free(stmt);
}

static void __set_elem_expr_clone(struct expr *new, const struct expr *expr)
{
	new->expiration = expr->expiration;
	new->timeout = expr->timeout;
	if (expr->comment)
		new->comment = xstrdup(expr->comment);
	init_list_head(&new->stmt_list);
}

static void set_elem_expr_clone(struct expr *new, const struct expr *expr)
{
	new->key = expr_clone(expr->key);
	__set_elem_expr_clone(new, expr);
}

static void set_elem_expr_set_type(const struct expr *expr,
				   const struct datatype *dtype,
				   enum byteorder byteorder)
{
       expr_set_type(expr->key, dtype, byteorder);
}

static const struct expr_ops set_elem_expr_ops = {
	.type		= EXPR_SET_ELEM,
	.name		= "set element",
	.clone		= set_elem_expr_clone,
	.print		= set_elem_expr_print,
	.json		= set_elem_expr_json,
	.destroy	= set_elem_expr_destroy,
	.set_type	= set_elem_expr_set_type,
};

struct expr *set_elem_expr_alloc(const struct location *loc, struct expr *key)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_SET_ELEM, key->dtype,
			  key->byteorder, key->len);
	expr->key = key;
	init_list_head(&expr->stmt_list);

	return expr;
}

static void set_elem_catchall_expr_print(const struct expr *expr,
					 struct output_ctx *octx)
{
	nft_print(octx, "*");
}

static void set_elem_catchall_expr_clone(struct expr *new, const struct expr *expr)
{
	__set_elem_expr_clone(new, expr);
}

static const struct expr_ops set_elem_catchall_expr_ops = {
	.type		= EXPR_SET_ELEM_CATCHALL,
	.name		= "catch-all set element",
	.print		= set_elem_catchall_expr_print,
	.json		= set_elem_catchall_expr_json,
	.clone		= set_elem_catchall_expr_clone,
};

struct expr *set_elem_catchall_expr_alloc(const struct location *loc)
{
	struct expr *expr;

	expr = expr_alloc(loc, EXPR_SET_ELEM_CATCHALL, &invalid_type,
			  BYTEORDER_INVALID, 0);
	expr->flags = EXPR_F_CONSTANT | EXPR_F_SINGLETON;

	return expr;
}

void range_expr_value_low(mpz_t rop, const struct expr *expr)
{
	switch (expr->etype) {
	case EXPR_VALUE:
		return mpz_set(rop, expr->value);
	case EXPR_RANGE_VALUE:
		return mpz_set(rop, expr->range.low);
	case EXPR_PREFIX:
		return range_expr_value_low(rop, expr->prefix);
	case EXPR_RANGE:
		return range_expr_value_low(rop, expr->left);
	case EXPR_MAPPING:
		return range_expr_value_low(rop, expr->left);
	default:
		BUG("invalid range expression type %s", expr_name(expr));
	}
}

void range_expr_value_high(mpz_t rop, const struct expr *expr)
{
	mpz_t tmp;

	switch (expr->etype) {
	case EXPR_VALUE:
		return mpz_set(rop, expr->value);
	case EXPR_RANGE_VALUE:
		return mpz_set(rop, expr->range.high);
	case EXPR_PREFIX:
		range_expr_value_low(rop, expr->prefix);
		assert(expr->len >= expr->prefix_len);
		mpz_init_bitmask(tmp, expr->len - expr->prefix_len);
		mpz_add(rop, rop, tmp);
		mpz_clear(tmp);
		return;
	case EXPR_RANGE:
		return range_expr_value_high(rop, expr->right);
	case EXPR_MAPPING:
		return range_expr_value_high(rop, expr->left);
	default:
		BUG("invalid range expression type %s", expr_name(expr));
	}
}

static const struct expr_ops *__expr_ops_by_type(enum expr_types etype)
{
	switch (etype) {
	case EXPR_INVALID: break;
	case EXPR_VERDICT: return &verdict_expr_ops;
	case EXPR_SYMBOL: return &symbol_expr_ops;
	case EXPR_VARIABLE: return &variable_expr_ops;
	case EXPR_VALUE: return &constant_expr_ops;
	case EXPR_PREFIX: return &prefix_expr_ops;
	case EXPR_RANGE: return &range_expr_ops;
	case EXPR_PAYLOAD: return &payload_expr_ops;
	case EXPR_EXTHDR: return &exthdr_expr_ops;
	case EXPR_META: return &meta_expr_ops;
	case EXPR_SOCKET: return &socket_expr_ops;
	case EXPR_OSF: return &osf_expr_ops;
	case EXPR_CT: return &ct_expr_ops;
	case EXPR_CONCAT: return &concat_expr_ops;
	case EXPR_LIST: return &list_expr_ops;
	case EXPR_SET: return &set_expr_ops;
	case EXPR_SET_REF: return &set_ref_expr_ops;
	case EXPR_SET_ELEM: return &set_elem_expr_ops;
	case EXPR_MAPPING: return &mapping_expr_ops;
	case EXPR_MAP: return &map_expr_ops;
	case EXPR_UNARY: return &unary_expr_ops;
	case EXPR_BINOP: return &binop_expr_ops;
	case EXPR_RELATIONAL: return &relational_expr_ops;
	case EXPR_NUMGEN: return &numgen_expr_ops;
	case EXPR_HASH: return &hash_expr_ops;
	case EXPR_RT: return &rt_expr_ops;
	case EXPR_TUNNEL: return &tunnel_expr_ops;
	case EXPR_FIB: return &fib_expr_ops;
	case EXPR_XFRM: return &xfrm_expr_ops;
	case EXPR_SET_ELEM_CATCHALL: return &set_elem_catchall_expr_ops;
	case EXPR_RANGE_VALUE: return &constant_range_expr_ops;
	case EXPR_RANGE_SYMBOL: return &symbol_range_expr_ops;
	case __EXPR_MAX: break;
	}

	return NULL;
}

const struct expr_ops *expr_ops(const struct expr *e)
{
	const struct expr_ops *ops;

	ops = __expr_ops_by_type(e->etype);
	if (!ops)
		BUG("Unknown expression type %d", e->etype);

	return ops;
}

const struct expr_ops *expr_ops_by_type_u32(uint32_t value)
{
	if (value > EXPR_MAX)
		return NULL;

	return __expr_ops_by_type(value);
}
