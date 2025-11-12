/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Arturo Borrero Gonzalez <arturo@debian.org>
 * (C) 2013 by Alvaro Neira Ayuso <alvaroneay@gmail.com>
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <errno.h>

#include "internal.h"
#include <stdlib.h>

#include <libmnl/libmnl.h>
#include <libnftnl/ruleset.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/set.h>
#include <libnftnl/rule.h>

struct nftnl_ruleset {
	struct nftnl_table_list	*table_list;
	struct nftnl_chain_list	*chain_list;
	struct nftnl_set_list	*set_list;
	struct nftnl_rule_list	*rule_list;

	uint16_t		flags;
};

struct nftnl_parse_ctx {
	enum nftnl_cmd_type cmd;
	enum nftnl_ruleset_type type;
	union {
		struct nftnl_table	*table;
		struct nftnl_chain	*chain;
		struct nftnl_rule		*rule;
		struct nftnl_set		*set;
		struct nftnl_set_elem	*set_elem;
	};
	void *data;

	/* These fields below are not exposed to the user */
	uint32_t format;
	uint32_t set_id;
	struct nftnl_set_list *set_list;

	int (*cb)(const struct nftnl_parse_ctx *ctx);
	uint16_t flags;
};

EXPORT_SYMBOL(nftnl_ruleset_alloc);
struct nftnl_ruleset *nftnl_ruleset_alloc(void)
{
	return calloc(1, sizeof(struct nftnl_ruleset));
}

EXPORT_SYMBOL(nftnl_ruleset_free);
void nftnl_ruleset_free(const struct nftnl_ruleset *r)
{
	if (r->flags & (1 << NFTNL_RULESET_TABLELIST))
		nftnl_table_list_free(r->table_list);
	if (r->flags & (1 << NFTNL_RULESET_CHAINLIST))
		nftnl_chain_list_free(r->chain_list);
	if (r->flags & (1 << NFTNL_RULESET_SETLIST))
		nftnl_set_list_free(r->set_list);
	if (r->flags & (1 << NFTNL_RULESET_RULELIST))
		nftnl_rule_list_free(r->rule_list);
	xfree(r);
}

EXPORT_SYMBOL(nftnl_ruleset_is_set);
bool nftnl_ruleset_is_set(const struct nftnl_ruleset *r, uint16_t attr)
{
	return r->flags & (1 << attr);
}

EXPORT_SYMBOL(nftnl_ruleset_unset);
void nftnl_ruleset_unset(struct nftnl_ruleset *r, uint16_t attr)
{
	if (!(r->flags & (1 << attr)))
		return;

	switch (attr) {
	case NFTNL_RULESET_TABLELIST:
		nftnl_table_list_free(r->table_list);
		break;
	case NFTNL_RULESET_CHAINLIST:
		nftnl_chain_list_free(r->chain_list);
		break;
	case NFTNL_RULESET_SETLIST:
		nftnl_set_list_free(r->set_list);
		break;
	case NFTNL_RULESET_RULELIST:
		nftnl_rule_list_free(r->rule_list);
		break;
	}
	r->flags &= ~(1 << attr);
}

EXPORT_SYMBOL(nftnl_ruleset_set);
void nftnl_ruleset_set(struct nftnl_ruleset *r, uint16_t attr, void *data)
{
	switch (attr) {
	case NFTNL_RULESET_TABLELIST:
		nftnl_ruleset_unset(r, NFTNL_RULESET_TABLELIST);
		r->table_list = data;
		break;
	case NFTNL_RULESET_CHAINLIST:
		nftnl_ruleset_unset(r, NFTNL_RULESET_CHAINLIST);
		r->chain_list = data;
		break;
	case NFTNL_RULESET_SETLIST:
		nftnl_ruleset_unset(r, NFTNL_RULESET_SETLIST);
		r->set_list = data;
		break;
	case NFTNL_RULESET_RULELIST:
		nftnl_ruleset_unset(r, NFTNL_RULESET_RULELIST);
		r->rule_list = data;
		break;
	default:
		return;
	}
	r->flags |= (1 << attr);
}

EXPORT_SYMBOL(nftnl_ruleset_get);
void *nftnl_ruleset_get(const struct nftnl_ruleset *r, uint16_t attr)
{
	if (!(r->flags & (1 << attr)))
		return NULL;

	switch (attr) {
	case NFTNL_RULESET_TABLELIST:
		return r->table_list;
	case NFTNL_RULESET_CHAINLIST:
		return r->chain_list;
	case NFTNL_RULESET_SETLIST:
		return r->set_list;
	case NFTNL_RULESET_RULELIST:
		return r->rule_list;
	default:
		return NULL;
	}
}

EXPORT_SYMBOL(nftnl_ruleset_ctx_free);
void nftnl_ruleset_ctx_free(const struct nftnl_parse_ctx *ctx)
{
	switch (ctx->type) {
	case NFTNL_RULESET_TABLE:
		nftnl_table_free(ctx->table);
		break;
	case NFTNL_RULESET_CHAIN:
		nftnl_chain_free(ctx->chain);
		break;
	case NFTNL_RULESET_RULE:
		nftnl_rule_free(ctx->rule);
		break;
	case NFTNL_RULESET_SET:
	case NFTNL_RULESET_SET_ELEMS:
		nftnl_set_free(ctx->set);
		break;
	case NFTNL_RULESET_RULESET:
	case NFTNL_RULESET_UNSPEC:
		break;
	}
}

EXPORT_SYMBOL(nftnl_ruleset_ctx_is_set);
bool nftnl_ruleset_ctx_is_set(const struct nftnl_parse_ctx *ctx, uint16_t attr)
{
	return ctx->flags & (1 << attr);
}

EXPORT_SYMBOL(nftnl_ruleset_ctx_get);
void *nftnl_ruleset_ctx_get(const struct nftnl_parse_ctx *ctx, uint16_t attr)
{
	if (!(ctx->flags & (1 << attr)))
		return NULL;

	switch (attr) {
	case NFTNL_RULESET_CTX_CMD:
		return (void *)&ctx->cmd;
	case NFTNL_RULESET_CTX_TYPE:
		return (void *)&ctx->type;
	case NFTNL_RULESET_CTX_TABLE:
		return ctx->table;
	case NFTNL_RULESET_CTX_CHAIN:
		return ctx->chain;
	case NFTNL_RULESET_CTX_RULE:
		return ctx->rule;
	case NFTNL_RULESET_CTX_SET:
		return ctx->set;
	case NFTNL_RULESET_CTX_DATA:
		return ctx->data;
	default:
		return NULL;
	}
}

EXPORT_SYMBOL(nftnl_ruleset_ctx_get_u32);
uint32_t nftnl_ruleset_ctx_get_u32(const struct nftnl_parse_ctx *ctx, uint16_t attr)
{
	const void *ret = nftnl_ruleset_ctx_get(ctx, attr);
	return ret == NULL ? 0 : *((uint32_t *)ret);
}


EXPORT_SYMBOL(nftnl_ruleset_parse_file_cb);
int nftnl_ruleset_parse_file_cb(enum nftnl_parse_type type, FILE *fp,
			      struct nftnl_parse_err *err, void *data,
			      int (*cb)(const struct nftnl_parse_ctx *ctx))
{
	errno = EOPNOTSUPP;
	return -1;
}

EXPORT_SYMBOL(nftnl_ruleset_parse_buffer_cb);
int nftnl_ruleset_parse_buffer_cb(enum nftnl_parse_type type, const char *buffer,
				struct nftnl_parse_err *err, void *data,
				int (*cb)(const struct nftnl_parse_ctx *ctx))
{
	errno = EOPNOTSUPP;
	return -1;
}

static int nftnl_ruleset_cb(const struct nftnl_parse_ctx *ctx)
{
	struct nftnl_ruleset *r = ctx->data;

	if (ctx->cmd != NFTNL_CMD_ADD)
		return -1;

	switch (ctx->type) {
	case NFTNL_RULESET_TABLE:
		if (r->table_list == NULL) {
			r->table_list = nftnl_table_list_alloc();
			if (r->table_list == NULL)
				return -1;

			nftnl_ruleset_set(r, NFTNL_RULESET_TABLELIST,
					     r->table_list);
		}
		nftnl_table_list_add_tail(ctx->table, r->table_list);
		break;
	case NFTNL_RULESET_CHAIN:
		if (r->chain_list == NULL) {
			r->chain_list = nftnl_chain_list_alloc();
			if (r->chain_list == NULL)
				return -1;

			nftnl_ruleset_set(r, NFTNL_RULESET_CHAINLIST,
					     r->chain_list);
		}
		nftnl_chain_list_add_tail(ctx->chain, r->chain_list);
		break;
	case NFTNL_RULESET_SET:
		if (r->set_list == NULL) {
			r->set_list = nftnl_set_list_alloc();
			if (r->set_list == NULL)
				return -1;

			nftnl_ruleset_set(r, NFTNL_RULESET_SETLIST,
					     r->set_list);
		}
		nftnl_set_list_add_tail(ctx->set, r->set_list);
		break;
	case NFTNL_RULESET_RULE:
		if (r->rule_list == NULL) {
			r->rule_list = nftnl_rule_list_alloc();
			if (r->rule_list == NULL)
				return -1;

			nftnl_ruleset_set(r, NFTNL_RULESET_RULELIST,
					     r->rule_list);
		}
		nftnl_rule_list_add_tail(ctx->rule, r->rule_list);
		break;
	case NFTNL_RULESET_RULESET:
		break;
	default:
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(nftnl_ruleset_parse);
int nftnl_ruleset_parse(struct nftnl_ruleset *r, enum nftnl_parse_type type,
		      const char *data, struct nftnl_parse_err *err)
{
	errno = EOPNOTSUPP;
	return -1;
}

EXPORT_SYMBOL(nftnl_ruleset_parse_file);
int nftnl_ruleset_parse_file(struct nftnl_ruleset *rs, enum nftnl_parse_type type,
			   FILE *fp, struct nftnl_parse_err *err)
{
	return nftnl_ruleset_parse_file_cb(type, fp, err, rs, nftnl_ruleset_cb);
}

static int
nftnl_ruleset_snprintf_table(char *buf, size_t remain,
			   const struct nftnl_ruleset *rs, uint32_t type,
			   uint32_t flags)
{
	struct nftnl_table *t;
	struct nftnl_table_list_iter *ti;
	const char *sep = "";
	int ret, offset = 0;

	ti = nftnl_table_list_iter_create(rs->table_list);
	if (ti == NULL)
		return 0;

	t = nftnl_table_list_iter_next(ti);
	while (t != NULL) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_table_snprintf(buf + offset, remain, t, type, flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		t = nftnl_table_list_iter_next(ti);
		sep = "\n";
	}
	nftnl_table_list_iter_destroy(ti);

	return offset;
}

static int
nftnl_ruleset_snprintf_chain(char *buf, size_t remain,
			   const struct nftnl_ruleset *rs, uint32_t type,
			   uint32_t flags)
{
	struct nftnl_chain *c;
	struct nftnl_chain_list_iter *ci;
	const char *sep = "";
	int ret, offset = 0;

	ci = nftnl_chain_list_iter_create(rs->chain_list);
	if (ci == NULL)
		return 0;

	c = nftnl_chain_list_iter_next(ci);
	while (c != NULL) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_chain_snprintf(buf + offset, remain, c, type, flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		c = nftnl_chain_list_iter_next(ci);
		sep = "\n";
	}
	nftnl_chain_list_iter_destroy(ci);

	return offset;
}

static int
nftnl_ruleset_snprintf_set(char *buf, size_t remain,
			 const struct nftnl_ruleset *rs, uint32_t type,
			 uint32_t flags)
{
	struct nftnl_set *s;
	struct nftnl_set_list_iter *si;
	const char *sep = "";
	int ret, offset = 0;

	si = nftnl_set_list_iter_create(rs->set_list);
	if (si == NULL)
		return 0;

	s = nftnl_set_list_iter_next(si);
	while (s != NULL) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_set_snprintf(buf + offset, remain, s, type, flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		s = nftnl_set_list_iter_next(si);
		sep = "\n";
	}
	nftnl_set_list_iter_destroy(si);

	return offset;
}

static int
nftnl_ruleset_snprintf_rule(char *buf, size_t remain,
			  const struct nftnl_ruleset *rs, uint32_t type,
			  uint32_t flags)
{
	struct nftnl_rule *r;
	struct nftnl_rule_list_iter *ri;
	const char *sep = "";
	int ret, offset = 0;

	ri = nftnl_rule_list_iter_create(rs->rule_list);
	if (ri == NULL)
		return 0;

	r = nftnl_rule_list_iter_next(ri);
	while (r != NULL) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_rule_snprintf(buf + offset, remain, r, type, flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		r = nftnl_rule_list_iter_next(ri);
		sep = "\n";
	}
	nftnl_rule_list_iter_destroy(ri);

	return offset;
}

static int
nftnl_ruleset_do_snprintf(char *buf, size_t remain,
			  const struct nftnl_ruleset *rs,
			  uint32_t cmd, uint32_t type, uint32_t flags)
{
	uint32_t inner_flags = flags;
	const char *sep = "";
	int ret, offset = 0;

	/* dont pass events flags to child calls of _snprintf() */
	inner_flags &= ~NFTNL_OF_EVENT_ANY;

	if (nftnl_ruleset_is_set(rs, NFTNL_RULESET_TABLELIST) &&
	    (!nftnl_table_list_is_empty(rs->table_list))) {
		ret = nftnl_ruleset_snprintf_table(buf + offset, remain, rs,
						 type, inner_flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		if (ret > 0)
			sep = "\n";
	}

	if (nftnl_ruleset_is_set(rs, NFTNL_RULESET_CHAINLIST) &&
	    (!nftnl_chain_list_is_empty(rs->chain_list))) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_ruleset_snprintf_chain(buf + offset, remain, rs,
						 type, inner_flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		if (ret > 0)
			sep = "\n";
	}

	if (nftnl_ruleset_is_set(rs, NFTNL_RULESET_SETLIST) &&
	    (!nftnl_set_list_is_empty(rs->set_list))) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_ruleset_snprintf_set(buf + offset, remain, rs,
					       type, inner_flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		if (ret > 0)
			sep = "\n";
	}

	if (nftnl_ruleset_is_set(rs, NFTNL_RULESET_RULELIST) &&
	    (!nftnl_rule_list_is_empty(rs->rule_list))) {
		ret = snprintf(buf + offset, remain, "%s", sep);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		ret = nftnl_ruleset_snprintf_rule(buf + offset, remain, rs,
						type, inner_flags);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	return offset;
}

EXPORT_SYMBOL(nftnl_ruleset_snprintf);
int nftnl_ruleset_snprintf(char *buf, size_t size, const struct nftnl_ruleset *r,
			 uint32_t type, uint32_t flags)
{
	if (size)
		buf[0] = '\0';

	if (type != NFTNL_OUTPUT_DEFAULT) {
		errno = EOPNOTSUPP;
		return -1;
	}
	return nftnl_ruleset_do_snprintf(buf, size, r, nftnl_flag2cmd(flags),
					 type, flags);
}

static int nftnl_ruleset_fprintf_tables(FILE *fp, const struct nftnl_ruleset *rs,
				      uint32_t type, uint32_t flags)
{
	int len = 0, ret = 0;
	struct nftnl_table *t;
	struct nftnl_table_list_iter *ti;
	const char *sep = "";

	ti = nftnl_table_list_iter_create(rs->table_list);
	if (ti == NULL)
		return -1;

	t = nftnl_table_list_iter_next(ti);
	while (t != NULL) {
		ret = fprintf(fp, "%s", sep);
		if (ret < 0)
			goto err;

		len += ret;

		ret = nftnl_table_fprintf(fp, t, type, flags);
		if (ret < 0)
			goto err;

		len += ret;

		t = nftnl_table_list_iter_next(ti);
		sep = "\n";

	}
	nftnl_table_list_iter_destroy(ti);

	return len;
err:
	nftnl_table_list_iter_destroy(ti);
	return -1;
}

static int nftnl_ruleset_fprintf_chains(FILE *fp, const struct nftnl_ruleset *rs,
				      uint32_t type, uint32_t flags)
{
	int len = 0, ret = 0;
	struct nftnl_chain *o;
	struct nftnl_chain_list_iter *i;
	const char *sep = "";

	i = nftnl_chain_list_iter_create(rs->chain_list);
	if (i == NULL)
		return -1;

	o = nftnl_chain_list_iter_next(i);
	while (o != NULL) {
		ret = fprintf(fp, "%s", sep);
		if (ret < 0)
			goto err;

		len += ret;

		ret = nftnl_chain_fprintf(fp, o, type, flags);
		if (ret < 0)
			goto err;

		len += ret;

		o = nftnl_chain_list_iter_next(i);
		sep = "\n";
	}
	nftnl_chain_list_iter_destroy(i);

	return len;
err:
	nftnl_chain_list_iter_destroy(i);
	return -1;
}

static int nftnl_ruleset_fprintf_sets(FILE *fp, const struct nftnl_ruleset *rs,
				    uint32_t type, uint32_t flags)
{
	int len = 0, ret = 0;
	struct nftnl_set *o;
	struct nftnl_set_list_iter *i;
	const char *sep = "";

	i = nftnl_set_list_iter_create(rs->set_list);
	if (i == NULL)
		return -1;

	o = nftnl_set_list_iter_next(i);
	while (o != NULL) {
		ret = fprintf(fp, "%s", sep);
		if (ret < 0)
			goto err;

		len += ret;

		ret = nftnl_set_fprintf(fp, o, type, flags);
		if (ret < 0)
			goto err;

		len += ret;

		o = nftnl_set_list_iter_next(i);
		sep = "\n";
	}
	nftnl_set_list_iter_destroy(i);

	return len;
err:
	nftnl_set_list_iter_destroy(i);
	return -1;
}

static int nftnl_ruleset_fprintf_rules(FILE *fp, const struct nftnl_ruleset *rs,
				    uint32_t type, uint32_t flags)
{
	int len = 0, ret = 0;
	struct nftnl_rule *o;
	struct nftnl_rule_list_iter *i;
	const char *sep = "";

	i = nftnl_rule_list_iter_create(rs->rule_list);
	if (i == NULL)
		return -1;

	o = nftnl_rule_list_iter_next(i);
	while (o != NULL) {
		ret = fprintf(fp, "%s", sep);
		if (ret < 0)
			goto err;

		len += ret;

		ret = nftnl_rule_fprintf(fp, o, type, flags);
		if (ret < 0)
			goto err;

		len += ret;

		o = nftnl_rule_list_iter_next(i);
		sep = "\n";
	}
	nftnl_rule_list_iter_destroy(i);

	return len;
err:
	nftnl_rule_list_iter_destroy(i);
	return -1;
}

#define NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len)	\
	if (ret < 0)				\
		return -1;			\
	len += ret;

static int nftnl_ruleset_cmd_fprintf(FILE *fp, const struct nftnl_ruleset *rs,
				   uint32_t cmd, uint32_t type, uint32_t flags)
{
	int len = 0, ret = 0;
	uint32_t inner_flags = flags;
	const char *sep = "";

	/* dont pass events flags to child calls of _snprintf() */
	inner_flags &= ~NFTNL_OF_EVENT_ANY;

	if ((nftnl_ruleset_is_set(rs, NFTNL_RULESET_TABLELIST)) &&
	    (!nftnl_table_list_is_empty(rs->table_list))) {
		ret = nftnl_ruleset_fprintf_tables(fp, rs, type, inner_flags);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);

		if (ret > 0)
			sep = "\n";
	}

	if ((nftnl_ruleset_is_set(rs, NFTNL_RULESET_CHAINLIST)) &&
	    (!nftnl_chain_list_is_empty(rs->chain_list))) {
		ret = fprintf(fp, "%s", sep);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);

		ret = nftnl_ruleset_fprintf_chains(fp, rs, type, inner_flags);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);

		if (ret > 0)
			sep = "\n";
	}

	if ((nftnl_ruleset_is_set(rs, NFTNL_RULESET_SETLIST)) &&
	    (!nftnl_set_list_is_empty(rs->set_list))) {
		ret = fprintf(fp, "%s", sep);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);

		ret = nftnl_ruleset_fprintf_sets(fp, rs, type, inner_flags);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);

		if (ret > 0)
			sep = "\n";
	}

	if ((nftnl_ruleset_is_set(rs, NFTNL_RULESET_RULELIST)) &&
	    (!nftnl_rule_list_is_empty(rs->rule_list))) {
		ret = fprintf(fp, "%s", sep);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);

		ret = nftnl_ruleset_fprintf_rules(fp, rs, type, inner_flags);
		NFTNL_FPRINTF_RETURN_OR_FIXLEN(ret, len);
	}

	return len;
}

EXPORT_SYMBOL(nftnl_ruleset_fprintf);
int nftnl_ruleset_fprintf(FILE *fp, const struct nftnl_ruleset *rs, uint32_t type,
			uint32_t flags)
{
	return nftnl_ruleset_cmd_fprintf(fp, rs, nftnl_flag2cmd(flags), type,
				       flags);
}
