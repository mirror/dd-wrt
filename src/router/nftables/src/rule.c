/*
 * Copyright (c) 2008-2012 Patrick McHardy <kaber@trash.net>
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
#include <inttypes.h>
#include <errno.h>

#include <statement.h>
#include <rule.h>
#include <utils.h>
#include <netdb.h>
#include <netlink.h>
#include <mnl.h>
#include <misspell.h>
#include <json.h>
#include <cache.h>
#include <owner.h>
#include <intervals.h>
#include "nftutils.h"

#include <libnftnl/common.h>
#include <libnftnl/ruleset.h>
#include <netinet/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nf_synproxy.h>
#include <net/if.h>
#include <linux/netfilter_bridge.h>

static const char *const tcp_state_to_name[] = {
	[NFTNL_CTTIMEOUT_TCP_SYN_SENT]		= "syn_sent",
	[NFTNL_CTTIMEOUT_TCP_SYN_RECV]		= "syn_recv",
	[NFTNL_CTTIMEOUT_TCP_ESTABLISHED]	= "established",
	[NFTNL_CTTIMEOUT_TCP_FIN_WAIT]		= "fin_wait",
	[NFTNL_CTTIMEOUT_TCP_CLOSE_WAIT]	= "close_wait",
	[NFTNL_CTTIMEOUT_TCP_LAST_ACK]		= "last_ack",
	[NFTNL_CTTIMEOUT_TCP_TIME_WAIT]		= "time_wait",
	[NFTNL_CTTIMEOUT_TCP_CLOSE]		= "close",
	[NFTNL_CTTIMEOUT_TCP_SYN_SENT2]		= "syn_sent2",
	[NFTNL_CTTIMEOUT_TCP_RETRANS]		= "retrans",
	[NFTNL_CTTIMEOUT_TCP_UNACK]		= "unack",
};

static const char *const udp_state_to_name[] = {
	[NFTNL_CTTIMEOUT_UDP_UNREPLIED]		= "unreplied",
	[NFTNL_CTTIMEOUT_UDP_REPLIED]		= "replied",
};

static uint32_t tcp_dflt_timeout[] = {
	[NFTNL_CTTIMEOUT_TCP_SYN_SENT]		= 120,
	[NFTNL_CTTIMEOUT_TCP_SYN_RECV]		= 60,
	[NFTNL_CTTIMEOUT_TCP_ESTABLISHED]	= 432000,
	[NFTNL_CTTIMEOUT_TCP_FIN_WAIT]		= 120,
	[NFTNL_CTTIMEOUT_TCP_CLOSE_WAIT]	= 60,
	[NFTNL_CTTIMEOUT_TCP_LAST_ACK]		= 30,
	[NFTNL_CTTIMEOUT_TCP_TIME_WAIT]		= 120,
	[NFTNL_CTTIMEOUT_TCP_CLOSE]		= 10,
	[NFTNL_CTTIMEOUT_TCP_SYN_SENT2]		= 120,
	[NFTNL_CTTIMEOUT_TCP_RETRANS]		= 300,
	[NFTNL_CTTIMEOUT_TCP_UNACK]		= 300,
};

static uint32_t udp_dflt_timeout[] = {
	[NFTNL_CTTIMEOUT_UDP_UNREPLIED]		= 30,
	[NFTNL_CTTIMEOUT_UDP_REPLIED]		= 120,
};

struct timeout_protocol timeout_protocol[UINT8_MAX + 1] = {
	[IPPROTO_TCP]	= {
		.array_size	= NFTNL_CTTIMEOUT_TCP_MAX,
		.state_to_name	= tcp_state_to_name,
		.dflt_timeout	= tcp_dflt_timeout,
	},
	[IPPROTO_UDP]	= {
		.array_size	= NFTNL_CTTIMEOUT_UDP_MAX,
		.state_to_name	= udp_state_to_name,
		.dflt_timeout	= udp_dflt_timeout,
	},
};

int timeout_str2num(uint16_t l4proto, struct timeout_state *ts)
{
	unsigned int i;

	for (i = 0; i < timeout_protocol[l4proto].array_size; i++) {
		if (!strcmp(timeout_protocol[l4proto].state_to_name[i], ts->timeout_str)) {
			ts->timeout_index = i;
			return 0;
		}
	}
	return -1;
}

void handle_free(struct handle *h)
{
	free_const(h->table.name);
	free_const(h->chain.name);
	free_const(h->set.name);
	free_const(h->flowtable.name);
	free_const(h->obj.name);
}

void handle_merge(struct handle *dst, const struct handle *src)
{
	if (dst->family == 0)
		dst->family = src->family;
	if (dst->table.name == NULL && src->table.name != NULL) {
		dst->table.name = xstrdup(src->table.name);
		dst->table.location = src->table.location;
	}
	if (dst->chain.name == NULL && src->chain.name != NULL) {
		dst->chain.name = xstrdup(src->chain.name);
		dst->chain.location = src->chain.location;
	}
	if (dst->set.name == NULL && src->set.name != NULL) {
		dst->set.name = xstrdup(src->set.name);
		dst->set.location = src->set.location;
	}
	if (dst->flowtable.name == NULL && src->flowtable.name != NULL)
		dst->flowtable.name = xstrdup(src->flowtable.name);
	if (dst->obj.name == NULL && src->obj.name != NULL)
		dst->obj.name = xstrdup(src->obj.name);
	if (dst->handle.id == 0)
		dst->handle = src->handle;
	if (dst->position.id == 0)
		dst->position = src->position;
	if (dst->index.id == 0)
		dst->index = src->index;
}

/* internal ID to uniquely identify a set in the batch */
static uint32_t set_id;

struct set *set_alloc(const struct location *loc)
{
	struct set *set;

	assert(loc);

	set = xzalloc(sizeof(*set));
	set->refcnt = 1;
	set->handle.set_id = ++set_id;
	set->location = *loc;

	init_list_head(&set->stmt_list);

	return set;
}

struct set *set_clone(const struct set *set)
{
	struct set *new_set;

	new_set			= set_alloc(&internal_location);
	handle_merge(&new_set->handle, &set->handle);
	new_set->flags		= set->flags;
	new_set->gc_int		= set->gc_int;
	new_set->timeout	= set->timeout;
	new_set->key		= expr_clone(set->key);
	if (set->data)
		new_set->data	= expr_clone(set->data);
	new_set->objtype	= set->objtype;
	new_set->policy		= set->policy;
	new_set->automerge	= set->automerge;
	new_set->desc		= set->desc;
	init_list_head(&new_set->stmt_list);

	return new_set;
}

struct set *set_get(struct set *set)
{
	assert_refcount_safe(set->refcnt);
	set->refcnt++;
	return set;
}

void set_free(struct set *set)
{
	struct stmt *stmt, *next;

	assert_refcount_safe(set->refcnt);
	if (--set->refcnt > 0)
		return;

	expr_free(set->init);
	if (set->comment)
		free_const(set->comment);
	handle_free(&set->handle);
	list_for_each_entry_safe(stmt, next, &set->stmt_list, list)
		stmt_free(stmt);
	expr_free(set->key);
	expr_free(set->data);
	free(set);
}

struct set *set_lookup_fuzzy(const char *set_name,
			     const struct nft_cache *cache,
			     const struct table **t)
{
	struct string_misspell_state st;
	struct table *table;
	struct set *set;

	if (!set_name)
		return NULL;

	string_misspell_init(&st);

	list_for_each_entry(table, &cache->table_cache.list, cache.list) {
		list_for_each_entry(set, &table->set_cache.list, cache.list) {
			if (set_is_anonymous(set->flags))
				continue;
			if (string_misspell_update(set->handle.set.name,
						   set_name, set, &st))
				*t = table;
		}
	}
	return st.obj;
}

struct set *set_lookup_global(uint32_t family, const char *table,
			      const char *name, struct nft_cache *cache)
{
	struct table *t;

	t = table_cache_find(&cache->table_cache, table, family);
	if (t == NULL)
		return NULL;

	return set_cache_find(t, name);
}

struct print_fmt_options {
	const char	*tab;
	const char	*nl;
	const char	*table;
	const char	*family;
	const char	*stmt_separator;
};

const char *set_policy2str(uint32_t policy)
{
	switch (policy) {
	case NFT_SET_POL_PERFORMANCE:
		return "performance";
	case NFT_SET_POL_MEMORY:
		return "memory";
	default:
		return "unknown";
	}
}

static void set_print_key(const struct expr *expr, struct output_ctx *octx)
{
	const struct datatype *dtype = expr->dtype;

	if (dtype->size || dtype->type == TYPE_VERDICT)
		nft_print(octx, "%s", dtype->name);
	else
		expr_print(expr, octx);
}

static void set_print_key_and_data(const struct set *set, struct output_ctx *octx)
{
	bool use_typeof = set->key_typeof_valid;

	nft_print(octx, "%s ", use_typeof ? "typeof" : "type");

	if (use_typeof)
		expr_print(set->key, octx);
	else
		set_print_key(set->key, octx);

	if (set_is_datamap(set->flags)) {
		nft_print(octx, " : ");
		if (set->data->flags & EXPR_F_INTERVAL)
			nft_print(octx, "interval ");

		if (use_typeof)
			expr_print(set->data, octx);
		else
			set_print_key(set->data, octx);
	} else if (set_is_objmap(set->flags)) {
		nft_print(octx, " : %s", obj_type_name(set->objtype));
	}
}

static void set_print_declaration(const struct set *set,
				  struct print_fmt_options *opts,
				  struct output_ctx *octx)
{
	const char *delim = "";
	struct stmt *stmt;
	const char *type;
	uint32_t flags;

	if (set_is_meter(set->flags))
		type = "meter";
	else if (set_is_map(set->flags))
		type = "map";
	else
		type = "set";

	nft_print(octx, "%s%s", opts->tab, type);

	if (opts->family != NULL)
		nft_print(octx, " %s", opts->family);

	if (opts->table != NULL)
		nft_print(octx, " %s", opts->table);

	nft_print(octx, " %s {", set->handle.set.name);

	if (nft_output_handle(octx))
		nft_print(octx, " # handle %" PRIu64, set->handle.handle.id);
	nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);

	set_print_key_and_data(set, octx);

	nft_print(octx, "%s", opts->stmt_separator);

	if (!(set->flags & (NFT_SET_CONSTANT))) {
		if (set->policy != NFT_SET_POL_PERFORMANCE) {
			nft_print(octx, "%s%spolicy %s%s",
				  opts->tab, opts->tab,
				  set_policy2str(set->policy),
				  opts->stmt_separator);
		}

		if (set->desc.size > 0) {
			nft_print(octx, "%s%ssize %u",
				  opts->tab, opts->tab,
				  set->desc.size);
			if (set->count > 0)
				nft_print(octx, "%s# count %u", opts->tab,
					  set->count);
			nft_print(octx, "%s", opts->stmt_separator);
		}
	}

	flags = set->flags;
	/* "timeout" flag is redundant if a default timeout exists */
	if (set->timeout)
		flags &= ~NFT_SET_TIMEOUT;

	if (flags & (NFT_SET_CONSTANT | NFT_SET_INTERVAL | NFT_SET_TIMEOUT | NFT_SET_EVAL)) {
		nft_print(octx, "%s%sflags ", opts->tab, opts->tab);
		if (set->flags & NFT_SET_CONSTANT) {
			nft_print(octx, "%sconstant", delim);
			delim = ",";
		}
		if (set->flags & NFT_SET_EVAL) {
			nft_print(octx, "%sdynamic", delim);
			delim = ",";
		}
		if (set->flags & NFT_SET_INTERVAL) {
			nft_print(octx, "%sinterval", delim);
			delim = ",";
		}
		if (set->flags & NFT_SET_TIMEOUT) {
			nft_print(octx, "%stimeout", delim);
			delim = ",";
		}
		nft_print(octx, "%s", opts->stmt_separator);
	}

	if (!list_empty(&set->stmt_list)) {
		unsigned int flags = octx->flags;

		nft_print(octx, "%s%s", opts->tab, opts->tab);

		octx->flags |= NFT_CTX_OUTPUT_STATELESS;
		list_for_each_entry(stmt, &set->stmt_list, list) {
			stmt_print(stmt, octx);
			if (!list_is_last(&stmt->list, &set->stmt_list))
				nft_print(octx, " ");
		}
		octx->flags = flags;

		nft_print(octx, "%s", opts->stmt_separator);
	}

	if (set->automerge)
		nft_print(octx, "%s%sauto-merge%s", opts->tab, opts->tab,
			  opts->stmt_separator);

	if (set->timeout) {
		nft_print(octx, "%s%stimeout ", opts->tab, opts->tab);
		time_print(set->timeout, octx);
		nft_print(octx, "%s", opts->stmt_separator);
	}
	if (set->gc_int) {
		nft_print(octx, "%s%sgc-interval ", opts->tab, opts->tab);
		time_print(set->gc_int, octx);
		nft_print(octx, "%s", opts->stmt_separator);
	}

	if (set->comment) {
		nft_print(octx, "%s%scomment \"%s\"%s",
			  opts->tab, opts->tab,
			  set->comment,
			  opts->stmt_separator);
	}
}

static void do_set_print(const struct set *set, struct print_fmt_options *opts,
			 struct output_ctx *octx)
{
	set_print_declaration(set, opts, octx);

	if ((set_is_meter(set->flags) && nft_output_stateless(octx)) ||
	    nft_output_terse(octx)) {
		nft_print(octx, "%s}%s", opts->tab, opts->nl);
		return;
	}

	if (set->init != NULL && expr_set(set->init)->size > 0) {
		nft_print(octx, "%s%selements = ", opts->tab, opts->tab);

		if (set->timeout || set->elem_has_comment ||
		    (set->flags & (NFT_SET_MAP | NFT_SET_OBJECT |
				   NFT_SET_TIMEOUT | NFT_SET_CONCAT)) ||
		    !list_empty(&set->stmt_list))
			octx->force_newline = true;

		expr_print(set->init, octx);
		octx->force_newline = false;

		nft_print(octx, "%s", opts->nl);
	}
	nft_print(octx, "%s}%s", opts->tab, opts->nl);
}

void set_print(const struct set *s, struct output_ctx *octx)
{
	struct print_fmt_options opts = {
		.tab		= "\t",
		.nl		= "\n",
		.stmt_separator	= "\n",
	};

	do_set_print(s, &opts, octx);
}

void set_print_plain(const struct set *s, struct output_ctx *octx)
{
	struct print_fmt_options opts = {
		.tab		= "",
		.nl		= " ",
		.table		= s->handle.table.name,
		.family		= family2str(s->handle.family),
		.stmt_separator	= "; ",
	};

	do_set_print(s, &opts, octx);
}

struct rule *rule_alloc(const struct location *loc, const struct handle *h)
{
	struct rule *rule;

	assert(loc);

	rule = xzalloc(sizeof(*rule));
	rule->location = *loc;
	init_list_head(&rule->list);
	init_list_head(&rule->stmts);
	rule->refcnt = 1;
	if (h != NULL)
		rule->handle = *h;

	return rule;
}

struct rule *rule_get(struct rule *rule)
{
	assert_refcount_safe(rule->refcnt);
	rule->refcnt++;
	return rule;
}

void rule_free(struct rule *rule)
{
	assert_refcount_safe(rule->refcnt);
	if (--rule->refcnt > 0)
		return;
	stmt_list_free(&rule->stmts);
	handle_free(&rule->handle);
	free_const(rule->comment);
	free(rule);
}

void rule_print(const struct rule *rule, struct output_ctx *octx)
{
	const struct stmt_ops *ops;
	const struct stmt *stmt;

	list_for_each_entry(stmt, &rule->stmts, list) {
		ops = stmt_ops(stmt);
		ops->print(stmt, octx);
		if (!list_is_last(&stmt->list, &rule->stmts))
			nft_print(octx, " ");
	}

	if (rule->comment)
		nft_print(octx, " comment \"%s\"", rule->comment);

	if (nft_output_handle(octx))
		nft_print(octx, " # handle %" PRIu64, rule->handle.handle.id);
}

struct rule *rule_lookup(const struct chain *chain, uint64_t handle)
{
	struct rule *rule;

	list_for_each_entry(rule, &chain->rules, list) {
		if (rule->handle.handle.id == handle)
			return rule;
	}
	return NULL;
}

struct rule *rule_lookup_by_index(const struct chain *chain, uint64_t index)
{
	struct rule *rule;

	list_for_each_entry(rule, &chain->rules, list) {
		if (!--index)
			return rule;
	}
	return NULL;
}

void rule_stmt_append(struct rule *rule, struct stmt *stmt)
{
	list_add_tail(&stmt->list, &rule->stmts);
	rule->num_stmts++;
}

void rule_stmt_insert_at(struct rule *rule, struct stmt *nstmt,
			 struct stmt *stmt)
{
	list_add_tail(&nstmt->list, &stmt->list);
	rule->num_stmts++;
}

struct scope *scope_alloc(void)
{
	struct scope *scope = xzalloc(sizeof(struct scope));

	init_list_head(&scope->symbols);

	return scope;
}

struct scope *scope_init(struct scope *scope, const struct scope *parent)
{
	scope->parent = parent;
	return scope;
}

void scope_release(const struct scope *scope)
{
	struct symbol *sym, *next;

	list_for_each_entry_safe(sym, next, &scope->symbols, list) {
		assert(sym->refcnt == 1);
		list_del(&sym->list);
		free_const(sym->identifier);
		expr_free(sym->expr);
		free(sym);
	}
}

void scope_free(struct scope *scope)
{
	scope_release(scope);
	free(scope);
}

void symbol_bind(struct scope *scope, const char *identifier, struct expr *expr)
{
	struct symbol *sym;

	sym = xzalloc(sizeof(*sym));
	sym->identifier = xstrdup(identifier);
	sym->expr = expr;
	sym->refcnt = 1;

	list_add(&sym->list, &scope->symbols);
}

struct symbol *symbol_get(const struct scope *scope, const char *identifier)
{
	struct symbol *sym;

	sym = symbol_lookup(scope, identifier);
	if (!sym)
		return NULL;

	assert_refcount_safe(sym->refcnt);
	sym->refcnt++;

	return sym;
}

static void symbol_put(struct symbol *sym)
{
	assert_refcount_safe(sym->refcnt);
	if (--sym->refcnt == 0) {
		free_const(sym->identifier);
		expr_free(sym->expr);
		free(sym);
	}
}

static void symbol_remove(struct symbol *sym)
{
	list_del(&sym->list);
	symbol_put(sym);
}

int symbol_unbind(const struct scope *scope, const char *identifier)
{
	struct symbol *sym, *next;

	list_for_each_entry_safe(sym, next, &scope->symbols, list) {
		if (!strcmp(sym->identifier, identifier))
			symbol_remove(sym);
	}

	return 0;
}

struct symbol *symbol_lookup(const struct scope *scope, const char *identifier)
{
	struct symbol *sym;

	while (scope != NULL) {
		list_for_each_entry(sym, &scope->symbols, list) {
			if (!strcmp(sym->identifier, identifier))
				return sym;
		}
		scope = scope->parent;
	}
	return NULL;
}

struct symbol *symbol_lookup_fuzzy(const struct scope *scope,
				   const char *identifier)
{
	struct string_misspell_state st;
	struct symbol *sym;

	string_misspell_init(&st);

	while (scope != NULL) {
		list_for_each_entry(sym, &scope->symbols, list)
			string_misspell_update(sym->identifier, identifier,
					       sym, &st);

		scope = scope->parent;
	}
	return st.obj;
}

static const char * const chain_type_str_array[] = {
	"filter",
	"nat",
	"route",
	NULL,
};

const char *chain_type_name_lookup(const char *name)
{
	int i;

	for (i = 0; chain_type_str_array[i]; i++) {
		if (!strcmp(name, chain_type_str_array[i]))
			return chain_type_str_array[i];
	}

	return NULL;
}

static const char * const chain_hookname_str_array[] = {
	"prerouting",
	"input",
	"forward",
	"postrouting",
	"output",
	"ingress",
	"egress",
	NULL,
};

const char *chain_hookname_lookup(const char *name)
{
	int i;

	for (i = 0; chain_hookname_str_array[i]; i++) {
		if (!strcmp(name, chain_hookname_str_array[i]))
			return chain_hookname_str_array[i];
	}

	return NULL;
}

/* internal ID to uniquely identify a set in the batch */
static uint32_t chain_id;

struct chain *chain_alloc(void)
{
	struct chain *chain;

	chain = xzalloc(sizeof(*chain));
	chain->location = internal_location;
	chain->refcnt = 1;
	chain->handle.chain_id = ++chain_id;
	init_list_head(&chain->rules);
	init_list_head(&chain->scope.symbols);

	chain->policy = NULL;
	return chain;
}

struct chain *chain_get(struct chain *chain)
{
	assert_refcount_safe(chain->refcnt);
	chain->refcnt++;
	return chain;
}

void chain_free(struct chain *chain)
{
	struct rule *rule, *next;
	int i;

	assert_refcount_safe(chain->refcnt);
	if (--chain->refcnt > 0)
		return;
	list_for_each_entry_safe(rule, next, &chain->rules, list)
		rule_free(rule);
	handle_free(&chain->handle);
	free_const(chain->type.str);
	expr_free(chain->dev_expr);
	for (i = 0; i < chain->dev_array_len; i++)
		free_const(chain->dev_array[i]);
	free(chain->dev_array);
	expr_free(chain->priority.expr);
	expr_free(chain->policy);
	free_const(chain->comment);

	/* MUST be released after all expressions, they could
	 * hold refcounts.
	 */
	scope_release(&chain->scope);
	free(chain);
}

struct chain *chain_binding_lookup(const struct table *table,
				   const char *chain_name)
{
	struct chain *chain;

	list_for_each_entry(chain, &table->chain_bindings, cache.list) {
		if (!strcmp(chain->handle.chain.name, chain_name))
			return chain;
	}
	return NULL;
}

struct chain *chain_lookup_fuzzy(const struct handle *h,
				 const struct nft_cache *cache,
				 const struct table **t)
{
	struct string_misspell_state st;
	struct table *table;
	struct chain *chain;

	if (!h->chain.name)
		return NULL;

	string_misspell_init(&st);

	list_for_each_entry(table, &cache->table_cache.list, cache.list) {
		list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
			if (string_misspell_update(chain->handle.chain.name,
						   h->chain.name, chain, &st))
				*t = table;
		}
	}
	return st.obj;
}

const char *family2str(unsigned int family)
{
	switch (family) {
		case NFPROTO_IPV4:
			return "ip";
		case NFPROTO_IPV6:
			return "ip6";
		case NFPROTO_INET:
			return "inet";
		case NFPROTO_NETDEV:
			return "netdev";
		case NFPROTO_ARP:
			return "arp";
		case NFPROTO_BRIDGE:
			return "bridge";
		default:
			break;
	}
	return "unknown";
}

const char *hooknum2str(unsigned int family, unsigned int hooknum)
{
	switch (family) {
	case NFPROTO_IPV4:
	case NFPROTO_BRIDGE:
	case NFPROTO_IPV6:
	case NFPROTO_INET:
		switch (hooknum) {
		case NF_INET_PRE_ROUTING:
			return "prerouting";
		case NF_INET_LOCAL_IN:
			return "input";
		case NF_INET_FORWARD:
			return "forward";
		case NF_INET_POST_ROUTING:
			return "postrouting";
		case NF_INET_LOCAL_OUT:
			return "output";
		case NF_INET_INGRESS:
			return "ingress";
		default:
			break;
		};
		break;
	case NFPROTO_ARP:
		switch (hooknum) {
		case NF_ARP_IN:
			return "input";
		case NF_ARP_FORWARD:
			return "forward";
		case NF_ARP_OUT:
			return "output";
		case __NF_ARP_INGRESS:
			return "ingress";
		default:
			break;
		}
		break;
	case NFPROTO_NETDEV:
		switch (hooknum) {
		case NF_NETDEV_INGRESS:
			return "ingress";
		case NF_NETDEV_EGRESS:
			return "egress";
		}
		break;
	default:
		break;
	};

	return "unknown";
}

const char *chain_policy2str(uint32_t policy)
{
	switch (policy) {
	case NF_DROP:
		return "drop";
	case NF_ACCEPT:
		return "accept";
	}
	return "unknown";
}

struct prio_tag {
	int val;
	const char *str;
};

static const struct prio_tag std_prios[] = {
	{ NF_IP_PRI_RAW,      "raw" },
	{ NF_IP_PRI_MANGLE,   "mangle" },
	{ NF_IP_PRI_NAT_DST,  "dstnat" },
	{ NF_IP_PRI_FILTER,   "filter" },
	{ NF_IP_PRI_SECURITY, "security" },
	{ NF_IP_PRI_NAT_SRC,  "srcnat" },
};

static const struct prio_tag bridge_std_prios[] = {
	{ NF_BR_PRI_NAT_DST_BRIDGED,  "dstnat" },
	{ NF_BR_PRI_FILTER_BRIDGED,   "filter" },
	{ NF_BR_PRI_NAT_DST_OTHER,    "out" },
	{ NF_BR_PRI_NAT_SRC,          "srcnat" },
};

static bool std_prio_family_hook_compat(int prio, int family, int hook)
{
	/* bridge family has different values */
	if (family == NFPROTO_BRIDGE) {
		switch (prio) {
		case NF_BR_PRI_NAT_DST_BRIDGED:
			if (hook == NF_BR_PRE_ROUTING)
				return true;
			break;
		case NF_BR_PRI_FILTER_BRIDGED:
			return true;
		case NF_BR_PRI_NAT_DST_OTHER:
			if (hook == NF_BR_LOCAL_OUT)
				return true;
			break;
		case NF_BR_PRI_NAT_SRC:
			if (hook == NF_BR_POST_ROUTING)
				return true;
		}
		return false;
	}
	switch(prio) {
	case NF_IP_PRI_FILTER:
		switch (family) {
		case NFPROTO_INET:
		case NFPROTO_IPV4:
		case NFPROTO_IPV6:
		case NFPROTO_ARP:
		case NFPROTO_NETDEV:
			return true;
		}
		break;
	case NF_IP_PRI_RAW:
	case NF_IP_PRI_MANGLE:
	case NF_IP_PRI_SECURITY:
		switch (family) {
		case NFPROTO_INET:
		case NFPROTO_IPV4:
		case NFPROTO_IPV6:
			return true;
		}
		break;
	case NF_IP_PRI_NAT_DST:
		switch(family) {
		case NFPROTO_INET:
		case NFPROTO_IPV4:
		case NFPROTO_IPV6:
			if (hook == NF_INET_PRE_ROUTING ||
			    hook == NF_INET_LOCAL_OUT)
				return true;
		}
		break;
	case NF_IP_PRI_NAT_SRC:
		switch(family) {
		case NFPROTO_INET:
		case NFPROTO_IPV4:
		case NFPROTO_IPV6:
			if (hook == NF_INET_LOCAL_IN ||
			    hook == NF_INET_POST_ROUTING)
				return true;
		}
	}
	return false;
}

int std_prio_lookup(const char *std_prio_name, int family, int hook)
{
	const struct prio_tag *prio_arr;
	size_t i, arr_size;

	if (family == NFPROTO_BRIDGE) {
		prio_arr = bridge_std_prios;
		arr_size = array_size(bridge_std_prios);
	} else {
		prio_arr = std_prios;
		arr_size = array_size(std_prios);
	}

	for (i = 0; i < arr_size; ++i) {
		if (strcmp(prio_arr[i].str, std_prio_name) == 0 &&
		    std_prio_family_hook_compat(prio_arr[i].val, family, hook))
			return prio_arr[i].val;
	}
	return NF_IP_PRI_LAST;
}

static const char *prio2str(const struct output_ctx *octx,
			    char *buf, size_t bufsize, int family, int hook,
			    const struct expr *expr)
{
	const struct prio_tag *prio_arr;
	const uint32_t reach = 10;
	const char *std_prio_str;
	int std_prio, prio;
	size_t i, arr_size;
	int64_t offset;

	mpz_export_data(&prio, expr->value, BYTEORDER_HOST_ENDIAN, sizeof(int));
	if (family == NFPROTO_BRIDGE) {
		prio_arr = bridge_std_prios;
		arr_size = array_size(bridge_std_prios);
	} else {
		prio_arr = std_prios;
		arr_size = array_size(std_prios);
	}

	if (!nft_output_numeric_prio(octx)) {
		for (i = 0; i < arr_size; ++i) {
			std_prio = prio_arr[i].val;
			std_prio_str = prio_arr[i].str;

			offset = (int64_t)prio - std_prio;
			if (llabs(offset) <= reach) {
				if (!std_prio_family_hook_compat(std_prio,
								 family, hook))
					break;

				strncpy(buf, std_prio_str, bufsize);
				if (offset > 0)
					snprintf(buf + strlen(buf),
						 bufsize - strlen(buf), " + %" PRIu64,
						 offset);
				else if (offset < 0)
					snprintf(buf + strlen(buf),
						 bufsize - strlen(buf), " - %" PRIu64,
						 -offset);
				return buf;
			}
		}
	}
	snprintf(buf, bufsize, "%d", prio);
	return buf;
}

static void chain_print_declaration(const struct chain *chain,
				    struct output_ctx *octx)
{
	char priobuf[STD_PRIO_BUFSIZE];
	int policy, i;

	if (chain->flags & CHAIN_F_BINDING)
		return;

	nft_print(octx, "\tchain %s {", chain->handle.chain.name);
	if (nft_output_handle(octx))
		nft_print(octx, " # handle %" PRIu64, chain->handle.handle.id);
	if (chain->comment)
		nft_print(octx, "\n\t\tcomment \"%s\"", chain->comment);
	nft_print(octx, "\n");
	if (chain->flags & CHAIN_F_BASECHAIN) {
		if (chain->type.str)
			nft_print(octx, "\t\ttype %s hook %s", chain->type.str,
				  hooknum2str(chain->handle.family, chain->hook.num));

		if (chain->dev_array_len == 1) {
			nft_print(octx, " device \"%s\"", chain->dev_array[0]);
		} else if (chain->dev_array_len > 1) {
			nft_print(octx, " devices = { ");
			for (i = 0; i < chain->dev_array_len; i++) {
				nft_print(octx, "\"%s\"", chain->dev_array[i]);
					if (i + 1 != chain->dev_array_len)
						nft_print(octx, ", ");
			}
			nft_print(octx, " }");
		}

		if (chain->priority.expr)
			nft_print(octx, " priority %s;",
				  prio2str(octx, priobuf, sizeof(priobuf),
					   chain->handle.family, chain->hook.num,
					   chain->priority.expr));
		if (chain->policy) {
			mpz_export_data(&policy, chain->policy->value,
					BYTEORDER_HOST_ENDIAN, sizeof(int));
			nft_print(octx, " policy %s;",
				  chain_policy2str(policy));
		}
		if (chain->flags & CHAIN_F_HW_OFFLOAD)
			nft_print(octx, " flags offload;");

		nft_print(octx, "\n");
	}
}

void chain_rules_print(const struct chain *chain, struct output_ctx *octx,
		       const char *indent)
{
	unsigned int flags = octx->flags;
	struct rule *rule;

	if (chain->flags & CHAIN_F_BINDING)
		octx->flags &= ~NFT_CTX_OUTPUT_HANDLE;

	list_for_each_entry(rule, &chain->rules, list) {
		nft_print(octx, "\t\t%s", indent ? : "");
		rule_print(rule, octx);
		nft_print(octx, "\n");
	}

	octx->flags = flags;
}

static void chain_print(const struct chain *chain, struct output_ctx *octx)
{
	chain_print_declaration(chain, octx);
	chain_rules_print(chain, octx, NULL);
	nft_print(octx, "\t}\n");
}

void chain_print_plain(const struct chain *chain, struct output_ctx *octx)
{
	char priobuf[STD_PRIO_BUFSIZE];
	int policy;

	nft_print(octx, "chain %s %s %s", family2str(chain->handle.family),
		  chain->handle.table.name, chain->handle.chain.name);

	if (chain->flags & CHAIN_F_BASECHAIN) {
		mpz_export_data(&policy, chain->policy->value,
				BYTEORDER_HOST_ENDIAN, sizeof(int));
		nft_print(octx, " { type %s hook %s ",
			  chain->type.str, chain->hook.name);

		if (chain->dev_array_len > 0) {
			int i;

			nft_print(octx, "devices = { ");
			for (i = 0; i < chain->dev_array_len; i++) {
				nft_print(octx, "\"%s\"", chain->dev_array[i]);
				if (i + 1 != chain->dev_array_len)
					nft_print(octx, ", ");
			}
			nft_print(octx, " } ");
		}
		nft_print(octx, "priority %s; policy %s; }",
			  prio2str(octx, priobuf, sizeof(priobuf),
				   chain->handle.family, chain->hook.num,
				   chain->priority.expr),
			  chain_policy2str(policy));
	}
	if (nft_output_handle(octx))
		nft_print(octx, " # handle %" PRIu64, chain->handle.handle.id);
}

struct table *table_alloc(void)
{
	struct table *table;

	table = xzalloc(sizeof(*table));
	table->location = internal_location;
	init_list_head(&table->chains);
	init_list_head(&table->sets);
	init_list_head(&table->objs);
	init_list_head(&table->flowtables);
	init_list_head(&table->chain_bindings);
	init_list_head(&table->scope.symbols);
	table->refcnt = 1;

	cache_init(&table->chain_cache);
	cache_init(&table->set_cache);
	cache_init(&table->obj_cache);
	cache_init(&table->ft_cache);

	return table;
}

void table_free(struct table *table)
{
	struct chain *chain, *next;
	struct flowtable *ft, *nft;
	struct set *set, *nset;
	struct obj *obj, *nobj;

	assert_refcount_safe(table->refcnt);
	if (--table->refcnt > 0)
		return;
	if (table->comment)
		free_const(table->comment);
	list_for_each_entry_safe(chain, next, &table->chains, list)
		chain_free(chain);
	list_for_each_entry_safe(chain, next, &table->chain_bindings, cache.list)
		chain_free(chain);
	/* this is implicitly releasing chains in the hashtable cache */
	list_for_each_entry_safe(chain, next, &table->chain_cache.list, cache.list)
		chain_free(chain);
	list_for_each_entry_safe(set, nset, &table->sets, list)
		set_free(set);
	/* this is implicitly releasing sets in the hashtable cache */
	list_for_each_entry_safe(set, nset, &table->set_cache.list, cache.list)
		set_free(set);
	list_for_each_entry_safe(ft, nft, &table->flowtables, list)
		flowtable_free(ft);
	/* this is implicitly releasing flowtables in the hashtable cache */
	list_for_each_entry_safe(ft, nft, &table->ft_cache.list, cache.list)
		flowtable_free(ft);
	list_for_each_entry_safe(obj, nobj, &table->objs, list)
		obj_free(obj);
	/* this is implicitly releasing objs in the hashtable cache */
	list_for_each_entry_safe(obj, nobj, &table->obj_cache.list, cache.list)
		obj_free(obj);

	handle_free(&table->handle);
	scope_release(&table->scope);
	cache_free(&table->chain_cache);
	cache_free(&table->set_cache);
	cache_free(&table->obj_cache);
	cache_free(&table->ft_cache);
	free(table);
}

struct table *table_get(struct table *table)
{
	assert_refcount_safe(table->refcnt);
	table->refcnt++;
	return table;
}

struct table *table_lookup_fuzzy(const struct handle *h,
				 const struct nft_cache *cache)
{
	struct string_misspell_state st;
	struct table *table;

	if (!h->table.name)
		return NULL;

	string_misspell_init(&st);

	list_for_each_entry(table, &cache->table_cache.list, cache.list) {
		string_misspell_update(table->handle.table.name,
				       h->table.name, table, &st);
	}
	return st.obj;
}

static const char *table_flags_name[TABLE_FLAGS_MAX] = {
	"dormant",
	"owner",
	"persist",
};

const char *table_flag_name(uint32_t flag)
{
	if (flag >= TABLE_FLAGS_MAX)
		return "unknown";

	return table_flags_name[flag];
}

unsigned int parse_table_flag(const char *name)
{
	int i;

	for (i = 0; i < TABLE_FLAGS_MAX; i++) {
		if (!strcmp(name, table_flags_name[i]))
			return 1 << i;
	}
	return 0;
}

static void table_print_flags(const struct table *table, const char **delim,
			      struct output_ctx *octx)
{
	uint32_t flags = table->flags;
	bool comma = false;
	int i;

	if (!table->flags)
		return;

	nft_print(octx, "\tflags ");
	for (i = 0; i < TABLE_FLAGS_MAX; i++) {
		if (flags & (1 << i)) {
			if (comma)
				nft_print(octx, ",");

			nft_print(octx, "%s", table_flag_name(i));
			comma = true;
		}
	}
	nft_print(octx, "\n");
	*delim = "\n";
}

static void table_print(const struct table *table, struct output_ctx *octx)
{
	struct flowtable *flowtable;
	struct chain *chain;
	struct obj *obj;
	struct set *set;
	const char *delim = "";
	const char *family = family2str(table->handle.family);

	if (table->has_xt_stmts)
		fprintf(octx->error_fp,
			"# Warning: table %s %s is managed by iptables-nft, do not touch!\n",
			family, table->handle.table.name);
	if (table->is_from_future)
		fprintf(octx->error_fp,
			"# Warning: table %s %s was created by a newer version of nftables? Content may be incomplete!\n",
			family, table->handle.table.name);

	nft_print(octx, "table %s %s {", family, table->handle.table.name);
	if (nft_output_handle(octx) || table->flags & TABLE_F_OWNER)
		nft_print(octx, " #");
	if (nft_output_handle(octx))
		nft_print(octx, " handle %" PRIu64, table->handle.handle.id);
	if (table->flags & TABLE_F_OWNER)
		nft_print(octx, " progname %s", get_progname(table->owner));

	nft_print(octx, "\n");
	table_print_flags(table, &delim, octx);

	if (table->comment)
		nft_print(octx, "\tcomment \"%s\"\n", table->comment);

	list_for_each_entry(obj, &table->obj_cache.list, cache.list) {
		nft_print(octx, "%s", delim);
		obj_print(obj, octx);
		delim = "\n";
	}
	list_for_each_entry(set, &table->set_cache.list, cache.list) {
		if (set_is_anonymous(set->flags))
			continue;
		nft_print(octx, "%s", delim);
		set_print(set, octx);
		delim = "\n";
	}
	list_for_each_entry(flowtable, &table->ft_cache.list, cache.list) {
		nft_print(octx, "%s", delim);
		flowtable_print(flowtable, octx);
		delim = "\n";
	}
	list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
		nft_print(octx, "%s", delim);
		chain_print(chain, octx);
		delim = "\n";
	}
	nft_print(octx, "}\n");
}

struct cmd *cmd_alloc(enum cmd_ops op, enum cmd_obj obj,
		      const struct handle *h, const struct location *loc,
		      void *data)
{
	struct cmd *cmd;

	assert(loc);

	cmd = xzalloc(sizeof(*cmd));
	init_list_head(&cmd->list);
	cmd->op       = op;
	cmd->obj      = obj;
	cmd->handle   = *h;
	cmd->location = *loc;
	cmd->data     = data;
	cmd->attr     = xzalloc_array(NFT_NLATTR_LOC_MAX,
				      sizeof(struct nlerr_loc));
	cmd->attr_array_len = NFT_NLATTR_LOC_MAX;

	return cmd;
}

struct markup *markup_alloc(uint32_t format)
{
	struct markup *markup;

	markup = xmalloc(sizeof(struct markup));
	markup->format = format;

	return markup;
}

void markup_free(struct markup *m)
{
	free(m);
}

struct monitor *monitor_alloc(uint32_t format, uint32_t type, const char *event)
{
	struct monitor *mon;

	mon = xmalloc(sizeof(struct monitor));
	mon->format = format;
	mon->type = type;
	mon->event = event;
	mon->flags = 0;

	return mon;
}

void monitor_free(struct monitor *m)
{
	free_const(m->event);
	free(m);
}

void cmd_free(struct cmd *cmd)
{
	if (cmd == NULL)
		return;

	handle_free(&cmd->handle);
	if (cmd->data != NULL) {
		switch (cmd->obj) {
		case CMD_OBJ_ELEMENTS:
			expr_free(cmd->expr);
			if (cmd->elem.set)
				set_free(cmd->elem.set);
			break;
		case CMD_OBJ_SET:
		case CMD_OBJ_MAP:
		case CMD_OBJ_METER:
		case CMD_OBJ_SETELEMS:
			set_free(cmd->set);
			break;
		case CMD_OBJ_RULE:
			rule_free(cmd->rule);
			break;
		case CMD_OBJ_CHAIN:
			chain_free(cmd->chain);
			break;
		case CMD_OBJ_TABLE:
			table_free(cmd->table);
			break;
		case CMD_OBJ_EXPR:
			expr_free(cmd->expr);
			break;
		case CMD_OBJ_MONITOR:
			monitor_free(cmd->monitor);
			break;
		case CMD_OBJ_MARKUP:
			markup_free(cmd->markup);
			break;
		case CMD_OBJ_COUNTER:
		case CMD_OBJ_QUOTA:
		case CMD_OBJ_CT_HELPER:
		case CMD_OBJ_CT_TIMEOUT:
		case CMD_OBJ_CT_EXPECT:
		case CMD_OBJ_LIMIT:
		case CMD_OBJ_SECMARK:
		case CMD_OBJ_SYNPROXY:
		case CMD_OBJ_TUNNEL:
			obj_free(cmd->object);
			break;
		case CMD_OBJ_FLOWTABLE:
			flowtable_free(cmd->flowtable);
			break;
		default:
			BUG("invalid command object type %u", cmd->obj);
		}
	}
	free(cmd->attr);
	free_const(cmd->arg);
	free(cmd);
}

#include <netlink.h>
#include <mnl.h>

static int __do_add_elements(struct netlink_ctx *ctx, struct cmd *cmd,
			     struct set *set, struct expr *expr, uint32_t flags)
{
	expr_set(expr)->set_flags |= set->flags;
	if (mnl_nft_setelem_add(ctx, cmd, set, expr, flags) < 0)
		return -1;

	return 0;
}

static int do_add_elements(struct netlink_ctx *ctx, struct cmd *cmd,
			   uint32_t flags)
{
	struct expr *init = cmd->expr;
	struct set *set = cmd->elem.set;

	if (set_is_non_concat_range(set) &&
	    set_to_intervals(set, init, true) < 0)
		return -1;

	return __do_add_elements(ctx, cmd, set, init, flags);
}

static int do_add_setelems(struct netlink_ctx *ctx, struct cmd *cmd,
			   uint32_t flags)
{
	struct set *set = cmd->set;

	if (!set->init)
		return 0;

	return __do_add_elements(ctx, cmd, set, set->init, flags);
}

static int do_add_set(struct netlink_ctx *ctx, struct cmd *cmd,
		      uint32_t flags)
{
	struct set *set = cmd->set;

	if (set->init != NULL) {
		/* Update set->init->size (NFTNL_SET_DESC_SIZE) before adding
		 * the set to the kernel. Calling this from do_add_setelems()
		 * comes too late which might result in spurious ENFILE errors.
		 */
		if (set_is_non_concat_range(set) &&
		    set_to_intervals(set, set->init, true) < 0)
			return -1;
	}

	if (mnl_nft_set_add(ctx, cmd, flags) < 0)
		return -1;

	if (set_is_anonymous(set->flags))
		return __do_add_elements(ctx, cmd, set, set->init, flags);

	return 0;
}

static int do_command_add(struct netlink_ctx *ctx, struct cmd *cmd, bool excl)
{
	uint32_t flags = excl ? NLM_F_EXCL : 0;

	if (nft_output_echo(&ctx->nft->output))
		flags |= NLM_F_ECHO;

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		return mnl_nft_table_add(ctx, cmd, flags);
	case CMD_OBJ_CHAIN:
		return mnl_nft_chain_add(ctx, cmd, flags);
	case CMD_OBJ_RULE:
		return mnl_nft_rule_add(ctx, cmd, flags | NLM_F_APPEND);
	case CMD_OBJ_SET:
		return do_add_set(ctx, cmd, flags);
	case CMD_OBJ_SETELEMS:
		return do_add_setelems(ctx, cmd, flags);
	case CMD_OBJ_ELEMENTS:
		return do_add_elements(ctx, cmd, flags);
	case CMD_OBJ_COUNTER:
	case CMD_OBJ_QUOTA:
	case CMD_OBJ_CT_HELPER:
	case CMD_OBJ_CT_TIMEOUT:
	case CMD_OBJ_CT_EXPECT:
	case CMD_OBJ_LIMIT:
	case CMD_OBJ_SECMARK:
	case CMD_OBJ_SYNPROXY:
	case CMD_OBJ_TUNNEL:
		return mnl_nft_obj_add(ctx, cmd, flags);
	case CMD_OBJ_FLOWTABLE:
		return mnl_nft_flowtable_add(ctx, cmd, flags);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

static int do_command_replace(struct netlink_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_RULE:
		return mnl_nft_rule_replace(ctx, cmd);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

static int do_command_insert(struct netlink_ctx *ctx, struct cmd *cmd)
{
	uint32_t flags = 0;

	if (nft_output_echo(&ctx->nft->output))
		flags |= NLM_F_ECHO;

	switch (cmd->obj) {
	case CMD_OBJ_RULE:
		return mnl_nft_rule_add(ctx, cmd, flags);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

static int do_delete_setelems(struct netlink_ctx *ctx, struct cmd *cmd)
{
	const struct set *set = cmd->elem.set;
	struct expr *expr = cmd->elem.expr;

	if (set_is_non_concat_range(set) &&
	    set_to_intervals(set, expr, false) < 0)
		return -1;

	if (mnl_nft_setelem_del(ctx, cmd, &cmd->handle, set, cmd->elem.expr) < 0)
		return -1;

	return 0;
}

static int do_command_delete(struct netlink_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		return mnl_nft_table_del(ctx, cmd);
	case CMD_OBJ_CHAIN:
		return mnl_nft_chain_del(ctx, cmd);
	case CMD_OBJ_RULE:
		return mnl_nft_rule_del(ctx, cmd);
	case CMD_OBJ_SET:
		return mnl_nft_set_del(ctx, cmd);
	case CMD_OBJ_ELEMENTS:
		return do_delete_setelems(ctx, cmd);
	case CMD_OBJ_COUNTER:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_COUNTER);
	case CMD_OBJ_QUOTA:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_QUOTA);
	case CMD_OBJ_CT_HELPER:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_CT_HELPER);
	case CMD_OBJ_CT_TIMEOUT:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_CT_TIMEOUT);
	case CMD_OBJ_CT_EXPECT:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_CT_EXPECT);
	case CMD_OBJ_LIMIT:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_LIMIT);
	case CMD_OBJ_SECMARK:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_SECMARK);
	case CMD_OBJ_SYNPROXY:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_SYNPROXY);
	case CMD_OBJ_TUNNEL:
		return mnl_nft_obj_del(ctx, cmd, NFT_OBJECT_TUNNEL);
	case CMD_OBJ_FLOWTABLE:
		return mnl_nft_flowtable_del(ctx, cmd);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static int do_list_table(struct netlink_ctx *ctx, struct table *table)
{
	table_print(table, &ctx->nft->output);
	return 0;
}

static int do_list_sets(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct table *table;
	struct set *set;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		nft_print(&ctx->nft->output, "table %s %s {\n",
			  family2str(table->handle.family),
			  table->handle.table.name);

		list_for_each_entry(set, &table->set_cache.list, cache.list) {
			if (cmd->obj == CMD_OBJ_SETS &&
			    !set_is_literal(set->flags))
				continue;
			if (cmd->obj == CMD_OBJ_METERS &&
			    !set_is_meter_compat(set->flags))
				continue;
			if (cmd->obj == CMD_OBJ_MAPS &&
			    !map_is_literal(set->flags))
				continue;
			set_print(set, &ctx->nft->output);
		}

		nft_print(&ctx->nft->output, "}\n");
	}
	return 0;
}

struct obj *obj_alloc(const struct location *loc)
{
	struct obj *obj;

	assert(loc);

	obj = xzalloc(sizeof(*obj));
	obj->location = *loc;

	obj->refcnt = 1;
	return obj;
}

struct obj *obj_get(struct obj *obj)
{
	assert_refcount_safe(obj->refcnt);
	obj->refcnt++;
	return obj;
}

void obj_free(struct obj *obj)
{
	assert_refcount_safe(obj->refcnt);
	if (--obj->refcnt > 0)
		return;
	free_const(obj->comment);
	handle_free(&obj->handle);
	switch (obj->type) {
	case NFT_OBJECT_CT_TIMEOUT: {
		struct timeout_state *ts, *next;

		list_for_each_entry_safe(ts, next, &obj->ct_timeout.timeout_list, head) {
			list_del(&ts->head);
			free_const(ts->timeout_str);
			free(ts);
		}
		}
		break;
	case NFT_OBJECT_TUNNEL:
		expr_free(obj->tunnel.src);
		expr_free(obj->tunnel.dst);
		if (obj->tunnel.type == TUNNEL_GENEVE) {
			struct tunnel_geneve *geneve, *next;

			list_for_each_entry_safe(geneve, next, &obj->tunnel.geneve_opts, list) {
				list_del(&geneve->list);
				free(geneve);
			}
		}
		break;
	default:
		break;
	}
	free(obj);
}

struct obj *obj_lookup_fuzzy(const char *obj_name,
			     const struct nft_cache *cache,
			     const struct table **t)
{
	struct string_misspell_state st;
	struct table *table;
	struct obj *obj;

	if (!obj_name)
		return NULL;

	string_misspell_init(&st);

	list_for_each_entry(table, &cache->table_cache.list, cache.list) {
		list_for_each_entry(obj, &table->obj_cache.list, cache.list) {
			if (string_misspell_update(obj->handle.obj.name,
						   obj_name, obj, &st))
				*t = table;
		}
	}
	return st.obj;
}

static void print_proto_name_proto(uint8_t l4, struct output_ctx *octx)
{
	char name[NFT_PROTONAME_MAXSIZE];

	if (nft_getprotobynumber(l4, name, sizeof(name)))
		nft_print(octx, "%s", name);
	else
		nft_print(octx, "%d", l4);
}

static void print_proto_timeout_policy(uint8_t l4, const uint32_t *timeout,
				       struct print_fmt_options *opts,
				       struct output_ctx *octx)
{
	bool comma = false;
	unsigned int i;

	nft_print(octx, "%s%spolicy = { ", opts->tab, opts->tab);
	for (i = 0; i < timeout_protocol[l4].array_size; i++) {
		if (timeout[i] != timeout_protocol[l4].dflt_timeout[i]) {
			uint64_t timeout_ms;

			if (comma)
				nft_print(octx, ", ");
			timeout_ms = timeout[i] * 1000u;
			nft_print(octx, "%s : ",
				  timeout_protocol[l4].state_to_name[i]);
			time_print(timeout_ms, octx);
			comma = true;
		}
	}
	nft_print(octx, " }%s", opts->stmt_separator);
}

static const char *synproxy_sack_to_str(const uint32_t flags)
{
        if (flags & NF_SYNPROXY_OPT_SACK_PERM)
                return "sack-perm";

        return "";
}

static const char *synproxy_timestamp_to_str(const uint32_t flags)
{
        if (flags & NF_SYNPROXY_OPT_TIMESTAMP)
                return "timestamp";

        return "";
}

int tunnel_geneve_data_str2array(const char *hexstr,
				 uint8_t *out_data,
				 uint32_t *out_len)
{
	char bytestr[3] = {0};
	size_t len;

	if (hexstr[0] == '0' && (hexstr[1] == 'x' || hexstr[1] == 'X'))
		hexstr += 2;
	else
		return -1;

	len = strlen(hexstr);
	if (len % 4 != 0)
		return -1;

	len = len / 2;
	if (len > NFTNL_TUNNEL_GENEVE_DATA_MAXLEN)
		return -1;

	for (size_t i = 0; i < len; i++) {
		uint32_t value;
		char *endptr;

		bytestr[0] = hexstr[i * 2];
		bytestr[1] = hexstr[i * 2 + 1];

		value = strtoul(bytestr, &endptr, 16);
		if (*endptr != '\0')
			return -1;

		out_data[i] = (uint8_t) value;
	}
	*out_len = (uint8_t) len;

	return 0;
}

static void obj_print_comment(const struct obj *obj,
			      struct print_fmt_options *opts,
			      struct output_ctx *octx)
{
	if (obj->comment)
		nft_print(octx, "%s%s%scomment \"%s\"",
			  opts->nl, opts->tab, opts->tab,
			  obj->comment);
}

static void obj_print_data(const struct obj *obj,
			   struct print_fmt_options *opts,
			   struct output_ctx *octx)
{
	switch (obj->type) {
	case NFT_OBJECT_COUNTER:
		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		if (nft_output_stateless(octx))
			nft_print(octx, "%s", opts->nl);
		else
			nft_print(octx, "%s%s%spackets %" PRIu64 " bytes %" PRIu64 "%s",
				  opts->nl, opts->tab, opts->tab,
				  obj->counter.packets, obj->counter.bytes, opts->nl);
		break;
	case NFT_OBJECT_QUOTA: {
		const char *data_unit;
		uint64_t bytes;

		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);
		data_unit = get_rate(obj->quota.bytes, &bytes);
		nft_print(octx, "%s%" PRIu64 " %s",
			  obj->quota.flags & NFT_QUOTA_F_INV ? "over " : "",
			  bytes, data_unit);
		if (!nft_output_stateless(octx) && obj->quota.used) {
			data_unit = get_rate(obj->quota.used, &bytes);
			nft_print(octx, " used %" PRIu64 " %s",
				  bytes, data_unit);
		}
		nft_print(octx, "%s", opts->nl);
		}
		break;
	case NFT_OBJECT_SECMARK:
		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);
		nft_print(octx, "\"%s\"%s", obj->secmark.ctx, opts->nl);
		break;
	case NFT_OBJECT_CT_HELPER:
		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		nft_print(octx, "%s", opts->nl);
		nft_print(octx, "%s%stype \"%s\" protocol ",
			  opts->tab, opts->tab, obj->ct_helper.name);
		print_proto_name_proto(obj->ct_helper.l4proto, octx);
		nft_print(octx, "%s", opts->stmt_separator);
		nft_print(octx, "%s%sl3proto %s%s",
			  opts->tab, opts->tab,
			  family2str(obj->ct_helper.l3proto),
			  opts->stmt_separator);
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		nft_print(octx, "%s", opts->nl);
		nft_print(octx, "%s%sprotocol ", opts->tab, opts->tab);
		print_proto_name_proto(obj->ct_timeout.l4proto, octx);
		nft_print(octx, "%s", opts->stmt_separator);
		nft_print(octx, "%s%sl3proto %s%s",
			  opts->tab, opts->tab,
			  family2str(obj->ct_timeout.l3proto),
			  opts->stmt_separator);
		print_proto_timeout_policy(obj->ct_timeout.l4proto,
					   obj->ct_timeout.timeout, opts, octx);
		break;
	case NFT_OBJECT_CT_EXPECT:
		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		nft_print(octx, "%s", opts->nl);
		nft_print(octx, "%s%sprotocol ", opts->tab, opts->tab);
		print_proto_name_proto(obj->ct_expect.l4proto, octx);
		nft_print(octx, "%s", opts->stmt_separator);
		nft_print(octx, "%s%sdport %d%s",
			  opts->tab, opts->tab,
			  obj->ct_expect.dport,
			  opts->stmt_separator);
		nft_print(octx, "%s%stimeout ", opts->tab, opts->tab);
		time_print(obj->ct_expect.timeout, octx);
		nft_print(octx, "%s", opts->stmt_separator);
		nft_print(octx, "%s%ssize %d%s",
			  opts->tab, opts->tab,
			  obj->ct_expect.size,
			  opts->stmt_separator);
		nft_print(octx, "%s%sl3proto %s%s",
			  opts->tab, opts->tab,
			  family2str(obj->ct_expect.l3proto),
			  opts->stmt_separator);
		break;
	case NFT_OBJECT_LIMIT: {
		bool inv = obj->limit.flags & NFT_LIMIT_F_INV;
		const char *data_unit;
		uint64_t rate;

		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);
		nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);
		switch (obj->limit.type) {
		case NFT_LIMIT_PKTS:
			nft_print(octx, "rate %s%" PRIu64 "/%s",
				  inv ? "over " : "", obj->limit.rate,
				  get_unit(obj->limit.unit));
			if (obj->limit.burst > 0 && obj->limit.burst != 5)
				nft_print(octx, " burst %u packets",
					  obj->limit.burst);
			break;
		case NFT_LIMIT_PKT_BYTES:
			data_unit = get_rate(obj->limit.rate, &rate);

			nft_print(octx, "rate %s%" PRIu64 " %s/%s",
				  inv ? "over " : "", rate, data_unit,
				  get_unit(obj->limit.unit));
			if (obj->limit.burst > 0) {
				uint64_t burst;

				data_unit = get_rate(obj->limit.burst, &burst);
				nft_print(octx, " burst %"PRIu64" %s",
					  burst, data_unit);
			}
			break;
		}
		nft_print(octx, "%s", opts->nl);
		}
		break;
	case NFT_OBJECT_SYNPROXY: {
		uint32_t flags = obj->synproxy.flags;
		const char *sack_str = synproxy_sack_to_str(flags);
		const char *ts_str = synproxy_timestamp_to_str(flags);

		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);

		if (flags & NF_SYNPROXY_OPT_MSS) {
			nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);
			nft_print(octx, "mss %u", obj->synproxy.mss);
		}
		if (flags & NF_SYNPROXY_OPT_WSCALE) {
			nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);
			nft_print(octx, "wscale %u", obj->synproxy.wscale);
		}
		if (flags & (NF_SYNPROXY_OPT_TIMESTAMP | NF_SYNPROXY_OPT_SACK_PERM)) {
			nft_print(octx, "%s%s%s", opts->nl, opts->tab, opts->tab);
			nft_print(octx, "%s %s", ts_str, sack_str);
		}
		nft_print(octx, "%s", opts->stmt_separator);
		}
		break;
	case NFT_OBJECT_TUNNEL:
		nft_print(octx, " %s {", obj->handle.obj.name);
		if (nft_output_handle(octx))
			nft_print(octx, " # handle %" PRIu64, obj->handle.handle.id);

		obj_print_comment(obj, opts, octx);

		nft_print(octx, "%s%s%sid %u",
			  opts->nl, opts->tab, opts->tab, obj->tunnel.id);

		if (obj->tunnel.src) {
			if (obj->tunnel.src->len == 32) {
				nft_print(octx, "%s%s%sip saddr ",
					  opts->nl, opts->tab, opts->tab);
				expr_print(obj->tunnel.src, octx);
			} else if (obj->tunnel.src->len == 128) {
				nft_print(octx, "%s%s%sip6 saddr ",
					  opts->nl, opts->tab, opts->tab);
				expr_print(obj->tunnel.src, octx);
			}
		}
		if (obj->tunnel.dst) {
			if (obj->tunnel.dst->len == 32) {
				nft_print(octx, "%s%s%sip daddr ",
					  opts->nl, opts->tab, opts->tab);
				expr_print(obj->tunnel.dst, octx);
			} else if (obj->tunnel.dst->len == 128) {
				nft_print(octx, "%s%s%sip6 daddr ",
					  opts->nl, opts->tab, opts->tab);
				expr_print(obj->tunnel.dst, octx);
			}
		}
		if (obj->tunnel.sport) {
			nft_print(octx, "%s%s%ssport %u",
				  opts->nl, opts->tab, opts->tab,
				  obj->tunnel.sport);
		}
		if (obj->tunnel.dport) {
			nft_print(octx, "%s%s%sdport %u",
				  opts->nl, opts->tab, opts->tab,
				  obj->tunnel.dport);
		}
		if (obj->tunnel.tos) {
			nft_print(octx, "%s%s%stos %u",
				  opts->nl, opts->tab, opts->tab,
				  obj->tunnel.tos);
		}
		if (obj->tunnel.ttl) {
			nft_print(octx, "%s%s%sttl %u",
				  opts->nl, opts->tab, opts->tab,
				  obj->tunnel.ttl);
		}
		switch (obj->tunnel.type) {
		case TUNNEL_ERSPAN:
			nft_print(octx, "%s%s%serspan {",
				  opts->nl, opts->tab, opts->tab);
			nft_print(octx, "%s%s%s%sversion %u",
				  opts->nl, opts->tab, opts->tab, opts->tab,
				  obj->tunnel.erspan.version);
			if (obj->tunnel.erspan.version == 1) {
				nft_print(octx, "%s%s%s%sindex %u",
					  opts->nl, opts->tab, opts->tab, opts->tab,
					  obj->tunnel.erspan.v1.index);
			} else {
				nft_print(octx, "%s%s%s%sdirection %s",
					  opts->nl, opts->tab, opts->tab, opts->tab,
					  obj->tunnel.erspan.v2.direction ? "egress"
									  : "ingress");
				nft_print(octx, "%s%s%s%sid %u",
					  opts->nl, opts->tab, opts->tab, opts->tab,
					  obj->tunnel.erspan.v2.hwid);
			}
			nft_print(octx, "%s%s%s}",
				  opts->nl, opts->tab, opts->tab);
			break;
		case TUNNEL_VXLAN:
			nft_print(octx, "%s%s%svxlan {",
				  opts->nl, opts->tab, opts->tab);
			nft_print(octx, "%s%s%s%sgbp %u",
				  opts->nl, opts->tab, opts->tab, opts->tab,
				  obj->tunnel.vxlan.gbp);
			nft_print(octx, "%s%s%s}",
				  opts->nl, opts->tab, opts->tab);
			break;
		case TUNNEL_GENEVE:
			struct tunnel_geneve *geneve;

			nft_print(octx, "%s%s%sgeneve {", opts->nl, opts->tab, opts->tab);
			list_for_each_entry(geneve, &obj->tunnel.geneve_opts, list) {
				char data_str[256];
				int offset = 0;

				for (uint32_t i = 0; i < geneve->data_len; i++) {
					offset += snprintf(data_str + offset,
							   geneve->data_len,
							   "%x",
							   geneve->data[i]);
				}
				nft_print(octx, "%s%s%s%sclass 0x%x opt-type 0x%x data \"0x%s\"",
					  opts->nl, opts->tab, opts->tab, opts->tab,
					  geneve->geneve_class, geneve->type, data_str);

			}
			nft_print(octx, "%s%s%s}", opts->nl, opts->tab, opts->tab);
			break;
		default:
			break;
		}

		nft_print(octx, "%s", opts->stmt_separator);
		break;
	default:
		nft_print(octx, " unknown {%s", opts->nl);
		break;
	}
}

static const char * const obj_type_name_array[] = {
	[NFT_OBJECT_COUNTER]	= "counter",
	[NFT_OBJECT_QUOTA]	= "quota",
	[NFT_OBJECT_CT_HELPER]	= "ct helper",
	[NFT_OBJECT_LIMIT]	= "limit",
	[NFT_OBJECT_CT_TIMEOUT] = "ct timeout",
	[NFT_OBJECT_SECMARK]	= "secmark",
	[NFT_OBJECT_SYNPROXY]	= "synproxy",
	[NFT_OBJECT_CT_EXPECT]	= "ct expectation",
	[NFT_OBJECT_TUNNEL]	= "tunnel",
};

const char *obj_type_name(unsigned int type)
{
	assert(type <= NFT_OBJECT_MAX && obj_type_name_array[type]);

	return obj_type_name_array[type];
}

static uint32_t obj_type_cmd_array[NFT_OBJECT_MAX + 1] = {
	[NFT_OBJECT_COUNTER]	= CMD_OBJ_COUNTER,
	[NFT_OBJECT_QUOTA]	= CMD_OBJ_QUOTA,
	[NFT_OBJECT_CT_HELPER]	= CMD_OBJ_CT_HELPER,
	[NFT_OBJECT_LIMIT]	= CMD_OBJ_LIMIT,
	[NFT_OBJECT_CT_TIMEOUT] = CMD_OBJ_CT_TIMEOUT,
	[NFT_OBJECT_SECMARK]	= CMD_OBJ_SECMARK,
	[NFT_OBJECT_SYNPROXY]	= CMD_OBJ_SYNPROXY,
	[NFT_OBJECT_CT_EXPECT]	= CMD_OBJ_CT_EXPECT,
	[NFT_OBJECT_TUNNEL]	= CMD_OBJ_TUNNEL,
};

enum cmd_obj obj_type_to_cmd(uint32_t type)
{
	assert(type <= NFT_OBJECT_MAX && obj_type_cmd_array[type]);

	return obj_type_cmd_array[type];
}

static void obj_print_declaration(const struct obj *obj,
				  struct print_fmt_options *opts,
				  struct output_ctx *octx)
{
	nft_print(octx, "%s%s", opts->tab, obj_type_name(obj->type));

	if (opts->family != NULL)
		nft_print(octx, " %s", opts->family);

	if (opts->table != NULL)
		nft_print(octx, " %s", opts->table);

	obj_print_data(obj, opts, octx);

	nft_print(octx, "%s}%s", opts->tab, opts->nl);
}

void obj_print(const struct obj *obj, struct output_ctx *octx)
{
	struct print_fmt_options opts = {
		.tab		= "\t",
		.nl		= "\n",
		.stmt_separator	= "\n",
	};

	obj_print_declaration(obj, &opts, octx);
}

void obj_print_plain(const struct obj *obj, struct output_ctx *octx)
{
	struct print_fmt_options opts = {
		.tab		= "",
		.nl		= " ",
		.table		= obj->handle.table.name,
		.family		= family2str(obj->handle.family),
		.stmt_separator = "; ",
	};

	obj_print_declaration(obj, &opts, octx);
}

static int do_list_obj(struct netlink_ctx *ctx, struct cmd *cmd, uint32_t type)
{
	struct print_fmt_options opts = {
		.tab		= "\t",
		.nl		= "\n",
		.stmt_separator	= "\n",
	};
	struct table *table;
	struct obj *obj;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		if (cmd->handle.table.name != NULL &&
		    strcmp(cmd->handle.table.name, table->handle.table.name))
			continue;

		if (list_empty(&table->obj_cache.list))
			continue;

		nft_print(&ctx->nft->output, "table %s %s {\n",
			  family2str(table->handle.family),
			  table->handle.table.name);

		list_for_each_entry(obj, &table->obj_cache.list, cache.list) {
			if (obj->type != type ||
			    (cmd->handle.obj.name != NULL &&
			     strcmp(cmd->handle.obj.name, obj->handle.obj.name)))
				continue;

			obj_print_declaration(obj, &opts, &ctx->nft->output);
		}

		nft_print(&ctx->nft->output, "}\n");
	}
	return 0;
}

struct flowtable *flowtable_alloc(const struct location *loc)
{
	struct flowtable *flowtable;

	assert(loc);

	flowtable = xzalloc(sizeof(*flowtable));
	flowtable->location = *loc;

	flowtable->refcnt = 1;
	return flowtable;
}

struct flowtable *flowtable_get(struct flowtable *flowtable)
{
	assert_refcount_safe(flowtable->refcnt);
	flowtable->refcnt++;
	return flowtable;
}

void flowtable_free(struct flowtable *flowtable)
{
	int i;

	assert_refcount_safe(flowtable->refcnt);
	if (--flowtable->refcnt > 0)
		return;
	handle_free(&flowtable->handle);
	expr_free(flowtable->priority.expr);
	expr_free(flowtable->dev_expr);

	if (flowtable->dev_array != NULL) {
		for (i = 0; i < flowtable->dev_array_len; i++)
			free_const(flowtable->dev_array[i]);
		free(flowtable->dev_array);
	}
	free(flowtable);
}

static void flowtable_print_declaration(const struct flowtable *flowtable,
					struct print_fmt_options *opts,
					struct output_ctx *octx)
{
	char priobuf[STD_PRIO_BUFSIZE];
	int i;

	nft_print(octx, "%sflowtable", opts->tab);

	if (opts->family != NULL)
		nft_print(octx, " %s", opts->family);

	if (opts->table != NULL)
		nft_print(octx, " %s", opts->table);

	nft_print(octx, " %s {", flowtable->handle.flowtable.name);

	if (nft_output_handle(octx))
		nft_print(octx, " # handle %" PRIu64, flowtable->handle.handle.id);
	nft_print(octx, "%s", opts->nl);

	if (flowtable->priority.expr) {
		nft_print(octx, "%s%shook %s priority %s%s",
			  opts->tab, opts->tab,
			  hooknum2str(NFPROTO_NETDEV, flowtable->hook.num),
			  prio2str(octx, priobuf, sizeof(priobuf), NFPROTO_NETDEV,
				   flowtable->hook.num, flowtable->priority.expr),
			  opts->stmt_separator);
	}

	if (flowtable->dev_array_len > 0) {
		nft_print(octx, "%s%sdevices = { ", opts->tab, opts->tab);
		for (i = 0; i < flowtable->dev_array_len; i++) {
			nft_print(octx, "\"%s\"", flowtable->dev_array[i]);
			if (i + 1 != flowtable->dev_array_len)
				nft_print(octx, ", ");
		}
		nft_print(octx, " }%s", opts->stmt_separator);
	}

	if (flowtable->flags & NFT_FLOWTABLE_HW_OFFLOAD)
		nft_print(octx, "%s%sflags offload%s", opts->tab, opts->tab,
			  opts->stmt_separator);

	if (flowtable->flags & NFT_FLOWTABLE_COUNTER)
		nft_print(octx, "%s%scounter%s", opts->tab, opts->tab,
			  opts->stmt_separator);
}

static void do_flowtable_print(const struct flowtable *flowtable,
			       struct print_fmt_options *opts,
			       struct output_ctx *octx)
{
	flowtable_print_declaration(flowtable, opts, octx);
	nft_print(octx, "%s}%s", opts->tab, opts->nl);
}

void flowtable_print(const struct flowtable *s, struct output_ctx *octx)
{
	struct print_fmt_options opts = {
		.tab		= "\t",
		.nl		= "\n",
		.stmt_separator	= "\n",
	};

	do_flowtable_print(s, &opts, octx);
}

void flowtable_print_plain(const struct flowtable *ft, struct output_ctx *octx)
{
	struct print_fmt_options opts = {
		.tab		= "",
		.nl		= " ",
		.table		= ft->handle.table.name,
		.family		= family2str(ft->handle.family),
		.stmt_separator = "; ",
	};

	flowtable_print_declaration(ft, &opts, octx);
	nft_print(octx, "}");
}


struct flowtable *flowtable_lookup_fuzzy(const char *ft_name,
					 const struct nft_cache *cache,
					 const struct table **t)
{
	struct string_misspell_state st;
	struct table *table;
	struct flowtable *ft;

	if (!ft_name)
		return NULL;

	string_misspell_init(&st);

	list_for_each_entry(table, &cache->table_cache.list, cache.list) {
		list_for_each_entry(ft, &table->ft_cache.list, cache.list) {
			if (string_misspell_update(ft->handle.flowtable.name,
						   ft_name, ft, &st))
				*t = table;
		}
	}
	return st.obj;
}

static int do_list_flowtable(struct netlink_ctx *ctx, struct cmd *cmd,
			     struct table *table)
{
	struct flowtable *ft;

	ft = ft_cache_find(table, cmd->handle.flowtable.name);
	if (!ft)
		return -1;

	nft_print(&ctx->nft->output, "table %s %s {\n",
		  family2str(table->handle.family),
		  table->handle.table.name);

	flowtable_print(ft, &ctx->nft->output);
	nft_print(&ctx->nft->output, "}\n");

	return 0;
}

static int do_list_flowtables(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct print_fmt_options opts = {
		.tab		= "\t",
		.nl		= "\n",
		.stmt_separator	= "\n",
	};
	struct flowtable *flowtable;
	struct table *table;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		nft_print(&ctx->nft->output, "table %s %s {\n",
			  family2str(table->handle.family),
			  table->handle.table.name);

		list_for_each_entry(flowtable, &table->ft_cache.list, cache.list) {
			flowtable_print_declaration(flowtable, &opts, &ctx->nft->output);
			nft_print(&ctx->nft->output, "%s}%s", opts.tab, opts.nl);
		}

		nft_print(&ctx->nft->output, "}\n");
	}
	return 0;
}

static int do_list_ruleset(struct netlink_ctx *ctx, struct cmd *cmd)
{
	unsigned int family = cmd->handle.family;
	struct table *table;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (family != NFPROTO_UNSPEC &&
		    table->handle.family != family)
			continue;

		if (do_list_table(ctx, table) < 0)
			return -1;
	}

	return 0;
}

static int do_list_tables(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct table *table;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		nft_print(&ctx->nft->output, "table %s %s\n",
			  family2str(table->handle.family),
			  table->handle.table.name);
	}

	return 0;
}

static void table_print_declaration(struct table *table,
				    struct output_ctx *octx)
{
	const char *family = family2str(table->handle.family);

	if (table->has_xt_stmts)
		fprintf(octx->error_fp,
			"# Warning: table %s %s is managed by iptables-nft, do not touch!\n",
			family, table->handle.table.name);

	nft_print(octx, "table %s %s {\n", family, table->handle.table.name);
}

static int do_list_chain(struct netlink_ctx *ctx, struct cmd *cmd,
			 struct table *table)
{
	struct chain *chain;

	table_print_declaration(table, &ctx->nft->output);

	chain = chain_cache_find(table, cmd->handle.chain.name);
	if (chain)
		chain_print(chain, &ctx->nft->output);

	nft_print(&ctx->nft->output, "}\n");

	return 0;
}

static int do_list_chains(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct table *table;
	struct chain *chain;

	list_for_each_entry(table, &ctx->nft->cache.table_cache.list, cache.list) {
		if (cmd->handle.family != NFPROTO_UNSPEC &&
		    cmd->handle.family != table->handle.family)
			continue;

		table_print_declaration(table, &ctx->nft->output);

		list_for_each_entry(chain, &table->chain_cache.list, cache.list) {
			chain_print_declaration(chain, &ctx->nft->output);
			nft_print(&ctx->nft->output, "\t}\n");
		}
		nft_print(&ctx->nft->output, "}\n");
	}

	return 0;
}

static void __do_list_set(struct netlink_ctx *ctx, struct cmd *cmd,
			  struct set *set)
{
	struct table *table = table_alloc();

	table->handle.table.name = xstrdup(cmd->handle.table.name);
	table->handle.family = cmd->handle.family;
	table_print_declaration(table, &ctx->nft->output);
	table_free(table);

	set_print(set, &ctx->nft->output);
	nft_print(&ctx->nft->output, "}\n");
}

static int do_list_set(struct netlink_ctx *ctx, struct cmd *cmd,
		       struct table *table)
{
	struct set *set = cmd->set;

	if (!set) {
		set = set_cache_find(table, cmd->handle.set.name);
		if (set == NULL)
			return -1;
	}

	__do_list_set(ctx, cmd, set);

	return 0;
}

static int do_list_hooks(struct netlink_ctx *ctx, struct cmd *cmd)
{
	const char *devname = cmd->handle.obj.name;

	return mnl_nft_dump_nf_hooks(ctx, cmd->handle.family, devname);
}

static int do_command_list(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct table *table = NULL;

	if (nft_output_json(&ctx->nft->output))
		return do_command_list_json(ctx, cmd);

	if (cmd->handle.table.name != NULL) {
		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name,
					 cmd->handle.family);
		if (!table) {
			errno = ENOENT;
			return -1;
		}
	}

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		if (!cmd->handle.table.name)
			return do_list_tables(ctx, cmd);
		return do_list_table(ctx, table);
	case CMD_OBJ_CHAIN:
		return do_list_chain(ctx, cmd, table);
	case CMD_OBJ_CHAINS:
		return do_list_chains(ctx, cmd);
	case CMD_OBJ_SETS:
		return do_list_sets(ctx, cmd);
	case CMD_OBJ_SET:
		return do_list_set(ctx, cmd, table);
	case CMD_OBJ_RULESET:
	case CMD_OBJ_RULES:
	case CMD_OBJ_RULE:
		return do_list_ruleset(ctx, cmd);
	case CMD_OBJ_METERS:
		return do_list_sets(ctx, cmd);
	case CMD_OBJ_METER:
		return do_list_set(ctx, cmd, table);
	case CMD_OBJ_MAPS:
		return do_list_sets(ctx, cmd);
	case CMD_OBJ_MAP:
		return do_list_set(ctx, cmd, table);
	case CMD_OBJ_COUNTER:
	case CMD_OBJ_COUNTERS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_COUNTER);
	case CMD_OBJ_QUOTA:
	case CMD_OBJ_QUOTAS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_QUOTA);
	case CMD_OBJ_CT_HELPER:
	case CMD_OBJ_CT_HELPERS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_CT_HELPER);
	case CMD_OBJ_CT_TIMEOUT:
	case CMD_OBJ_CT_TIMEOUTS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_CT_TIMEOUT);
	case CMD_OBJ_CT_EXPECT:
	case CMD_OBJ_CT_EXPECTATIONS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_CT_EXPECT);
	case CMD_OBJ_LIMIT:
	case CMD_OBJ_LIMITS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_LIMIT);
	case CMD_OBJ_SECMARK:
	case CMD_OBJ_SECMARKS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_SECMARK);
	case CMD_OBJ_SYNPROXY:
	case CMD_OBJ_SYNPROXYS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_SYNPROXY);
	case CMD_OBJ_TUNNEL:
	case CMD_OBJ_TUNNELS:
		return do_list_obj(ctx, cmd, NFT_OBJECT_TUNNEL);
	case CMD_OBJ_FLOWTABLE:
		return do_list_flowtable(ctx, cmd, table);
	case CMD_OBJ_FLOWTABLES:
		return do_list_flowtables(ctx, cmd);
	case CMD_OBJ_HOOKS:
		return do_list_hooks(ctx, cmd);
	case CMD_OBJ_MONITOR:
	case CMD_OBJ_MARKUP:
	case CMD_OBJ_SETELEMS:
	case CMD_OBJ_EXPR:
	case CMD_OBJ_ELEMENTS:
		errno = EOPNOTSUPP;
		return -1;
	case CMD_OBJ_INVALID:
		break;
	}

	BUG("invalid command object type %u", cmd->obj);
	return 0;
}

static int do_get_setelems(struct netlink_ctx *ctx, struct cmd *cmd, bool reset)
{
	struct set *set, *new_set;
	struct expr *init;
	int err;

	set = cmd->elem.set;

	/* Create a list of elements based of what we got from command line. */
	if (set_is_non_concat_range(set))
		init = get_set_intervals(set, cmd->expr);
	else
		init = cmd->expr;

	new_set = set_clone(set);

	/* Fetch from kernel the elements that have been requested .*/
	err = netlink_get_setelem(ctx, &cmd->handle, &cmd->location,
				  cmd->elem.set, new_set, init, reset);
	if (err >= 0)
		__do_list_set(ctx, cmd, new_set);

	if (set_is_non_concat_range(set))
		expr_free(init);

	set_free(new_set);

	return err;
}

static int do_command_get(struct netlink_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_ELEMENTS:
		return do_get_setelems(ctx, cmd, false);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}

	return 0;
}

static int do_command_reset(struct netlink_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_ELEMENTS:
		return do_get_setelems(ctx, cmd, true);
	default:
		break;
	}

	return do_command_list(ctx, cmd);
}

static int do_command_flush(struct netlink_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
	case CMD_OBJ_CHAIN:
		return mnl_nft_rule_del(ctx, cmd);
	case CMD_OBJ_SET:
	case CMD_OBJ_MAP:
	case CMD_OBJ_METER:
		return mnl_nft_setelem_flush(ctx, cmd);
	case CMD_OBJ_RULESET:
		return mnl_nft_table_del(ctx, cmd);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

static int do_command_rename(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct table *table = table_cache_find(&ctx->nft->cache.table_cache,
					       cmd->handle.table.name,
					       cmd->handle.family);
	const struct chain *chain;

	switch (cmd->obj) {
	case CMD_OBJ_CHAIN:
		chain = chain_cache_find(table, cmd->handle.chain.name);

		return mnl_nft_chain_rename(ctx, cmd, chain);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
	return 0;
}

static int do_command_monitor(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct netlink_mon_handler monhandler = {
		.monitor_flags	= cmd->monitor->flags,
		.format		= cmd->monitor->format,
		.ctx		= ctx,
		.loc		= &cmd->location,
		.cache		= &ctx->nft->cache,
		.debug_mask	= ctx->nft->debug_mask,
	};

	if (nft_output_json(&ctx->nft->output))
		monhandler.format = NFTNL_OUTPUT_JSON;

	return netlink_monitor(&monhandler, ctx->nft->nf_sock);
}

static int do_command_describe(struct netlink_ctx *ctx, struct cmd *cmd,
			       struct output_ctx *octx)
{
	expr_describe(cmd->expr, octx);
	return 0;
}

struct cmd *cmd_alloc_obj_ct(enum cmd_ops op, int type, const struct handle *h,
			     const struct location *loc, struct obj *obj)
{
	enum cmd_obj cmd_obj;

	if (obj)
		obj->type = type;

	switch (type) {
	case NFT_OBJECT_CT_HELPER:
		cmd_obj = CMD_OBJ_CT_HELPER;
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		cmd_obj = CMD_OBJ_CT_TIMEOUT;
		break;
	case NFT_OBJECT_CT_EXPECT:
		cmd_obj = CMD_OBJ_CT_EXPECT;
		break;
	default:
		BUG("missing type mapping");
	}

	return cmd_alloc(op, cmd_obj, h, loc, obj);
}

int do_command(struct netlink_ctx *ctx, struct cmd *cmd)
{
	switch (cmd->op) {
	case CMD_ADD:
		return do_command_add(ctx, cmd, false);
	case CMD_CREATE:
		return do_command_add(ctx, cmd, true);
	case CMD_INSERT:
		return do_command_insert(ctx, cmd);
	case CMD_REPLACE:
		return do_command_replace(ctx, cmd);
	case CMD_DELETE:
	case CMD_DESTROY:
		return do_command_delete(ctx, cmd);
	case CMD_GET:
		return do_command_get(ctx, cmd);
	case CMD_LIST:
		return do_command_list(ctx, cmd);
	case CMD_RESET:
		return do_command_reset(ctx, cmd);
	case CMD_FLUSH:
		return do_command_flush(ctx, cmd);
	case CMD_RENAME:
		return do_command_rename(ctx, cmd);
	case CMD_IMPORT:
	case CMD_EXPORT:
		errno = EOPNOTSUPP;
		return -1;
	case CMD_MONITOR:
		return do_command_monitor(ctx, cmd);
	case CMD_DESCRIBE:
		return do_command_describe(ctx, cmd, &ctx->nft->output);
	default:
		BUG("invalid command object type %u", cmd->obj);
	}
}

static int payload_match_stmt_cmp(const void *p1, const void *p2)
{
	const struct stmt *s1 = *(struct stmt * const *)p1;
	const struct stmt *s2 = *(struct stmt * const *)p2;
	const struct expr *e1 = s1->expr, *e2 = s2->expr;
	int d;

	d = e1->left->payload.base - e2->left->payload.base;
	if (d != 0)
		return d;
	return e1->left->payload.offset - e2->left->payload.offset;
}

static bool relational_ops_match(const struct expr *e1, const struct expr *e2)
{
	enum ops op1, op2;

	op1 = e1->op == OP_IMPLICIT ? OP_EQ : e1->op;
	op2 = e2->op == OP_IMPLICIT ? OP_EQ : e2->op;

	return op1 == op2;
}

static void payload_do_merge(struct stmt *sa[], unsigned int n)
{
	struct expr *last, *this, *expr1, *expr2;
	struct stmt *stmt;
	unsigned int i, j;

	qsort(sa, n, sizeof(sa[0]), payload_match_stmt_cmp);

	last = sa[0]->expr;
	for (j = 0, i = 1; i < n; i++) {
		stmt = sa[i];
		this = stmt->expr;

		if (!payload_can_merge(last->left, this->left) ||
		    !relational_ops_match(last, this)) {
			last = this;
			j = i;
			continue;
		}

		expr1 = payload_expr_join(last->left, this->left);
		expr2 = constant_expr_join(last->right, this->right);

		/* We can merge last into this, but we can't replace
		 * the statement associated with this if it does contain
		 * a higher level protocol.
		 *
		 * ether type ip ip saddr X ether saddr Y
		 * ... can be changed to
		 * ether type ip ether saddr Y ip saddr X
		 * ... but not
		 * ip saddr X ether type ip ether saddr Y
		 *
		 * The latter form means we perform ip saddr test before
		 * ensuring ip dependency, plus it makes decoding harder
		 * since we don't know the type of the network header
		 * right away.
		 *
		 * So, if we're about to replace a statement
		 * containing a protocol identifier, just swap this and last
		 * and replace the other one (i.e., replace 'load ether type ip'
		 * with the combined 'load both ether type and saddr') and not
		 * the other way around.
		 */
		if (this->left->flags & EXPR_F_PROTOCOL) {
			struct expr *tmp = last;

			last = this;
			this = tmp;

			expr1->flags |= EXPR_F_PROTOCOL;
			stmt = sa[j];
			assert(stmt->expr == this);
			j = i;
		}

		expr_free(last->left);
		last->left = expr1;

		expr_free(last->right);
		last->right = expr2;

		list_del(&stmt->list);
		stmt_free(stmt);
	}
}

/**
 * stmt_reduce - reduce statements in rule
 *
 * @rule:	nftables rule
 *
 * This function aims to:
 *
 * - remove redundant statement, e.g. remove 'meta protocol ip' if family is ip
 * - merge consecutive payload match statements
 *
 * Locate sequences of payload match statements referring to adjacent
 * header locations and merge those using only equality relations.
 *
 * As a side-effect, payload match statements are ordered in ascending
 * order according to the location of the payload.
 */
static void stmt_reduce(const struct rule *rule)
{
	struct stmt *stmt, *dstmt = NULL, *next;
	struct stmt *sa[rule->num_stmts];
	unsigned int idx = 0;

	list_for_each_entry_safe(stmt, next, &rule->stmts, list) {
		/* delete this redundant statement */
		if (dstmt) {
			list_del(&dstmt->list);
			stmt_free(dstmt);
			dstmt = NULL;
		}

		/* Must not merge across other statements */
		if (stmt->type != STMT_EXPRESSION) {
			if (idx >= 2)
				payload_do_merge(sa, idx);
			idx = 0;
			continue;
		}

		if (stmt->expr->etype != EXPR_RELATIONAL)
			continue;
		if (stmt->expr->right->etype != EXPR_VALUE)
			continue;

		if (stmt->expr->left->etype == EXPR_PAYLOAD) {
			switch (stmt->expr->op) {
			case OP_EQ:
			case OP_IMPLICIT:
				break;
			default:
				continue;
			}

			sa[idx++] = stmt;
		} else if (stmt->expr->left->etype == EXPR_META) {
			switch (stmt->expr->op) {
			case OP_EQ:
			case OP_IMPLICIT:
				if (stmt->expr->left->meta.key == NFT_META_PROTOCOL &&
				    !stmt->expr->left->meta.inner_desc) {
					uint16_t protocol;

					protocol = mpz_get_uint16(stmt->expr->right->value);
					if ((rule->handle.family == NFPROTO_IPV4 &&
					     protocol == ETH_P_IP) ||
					    (rule->handle.family == NFPROTO_IPV6 &&
					     protocol == ETH_P_IPV6))
						dstmt = stmt;
				}
				break;
			default:
				break;
			}
		}
	}

	if (idx > 1)
		payload_do_merge(sa, idx);
}

struct error_record *rule_postprocess(struct rule *rule)
{
	stmt_reduce(rule);
	return NULL;
}
