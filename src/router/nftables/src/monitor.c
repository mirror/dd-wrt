/*
 * Copyright (c) 2015 Arturo Borrero Gonzalez <arturo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <fcntl.h>
#include <errno.h>
#include <libmnl/libmnl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <inttypes.h>

#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/expr.h>
#include <libnftnl/object.h>
#include <libnftnl/set.h>
#include <libnftnl/flowtable.h>
#include <libnftnl/udata.h>
#include <libnftnl/ruleset.h>
#include <libnftnl/common.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter.h>

#include <nftables.h>
#include <netlink.h>
#include <mnl.h>
#include <trace.h>
#include <expression.h>
#include <statement.h>
#include <gmputil.h>
#include <utils.h>
#include <erec.h>
#include <iface.h>
#include <json.h>

enum {
	NFT_OF_EVENT_ADD,
	NFT_OF_EVENT_INSERT,
	NFT_OF_EVENT_DEL,
	NFT_OF_EVENT_CREATE,
};

#define nft_mon_print(monh, ...) nft_print(&monh->ctx->nft->output, __VA_ARGS__)

struct nftnl_table *netlink_table_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_table *nlt;

	nlt = nftnl_table_alloc();
	if (nlt == NULL)
		memory_allocation_error();
	if (nftnl_table_nlmsg_parse(nlh, nlt) < 0)
		netlink_abi_error();

	return nlt;
}

struct nftnl_chain *netlink_chain_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_chain *nlc;

	nlc = nftnl_chain_alloc();
	if (nlc == NULL)
		memory_allocation_error();
	if (nftnl_chain_nlmsg_parse(nlh, nlc) < 0)
		netlink_abi_error();

	return nlc;
}

struct nftnl_set *netlink_set_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_set *nls;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();
	if (nftnl_set_nlmsg_parse(nlh, nls) < 0)
		netlink_abi_error();

	return nls;
}

static struct nftnl_set *netlink_setelem_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_set *nls;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();
	if (nftnl_set_elems_nlmsg_parse(nlh, nls) < 0)
		netlink_abi_error();

	return nls;
}

struct nftnl_rule *netlink_rule_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_rule *nlr;

	nlr = nftnl_rule_alloc();
	if (nlr == NULL)
		memory_allocation_error();
	if (nftnl_rule_nlmsg_parse(nlh, nlr) < 0)
		netlink_abi_error();

	return nlr;
}

struct nftnl_obj *netlink_obj_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_obj *nlo;

	nlo = nftnl_obj_alloc();
	if (nlo == NULL)
		memory_allocation_error();
	if (nftnl_obj_nlmsg_parse(nlh, nlo) < 0)
		netlink_abi_error();

	return nlo;
}

struct nftnl_flowtable *netlink_flowtable_alloc(const struct nlmsghdr *nlh)
{
	struct nftnl_flowtable *nlf;

	nlf = nftnl_flowtable_alloc();
	if (nlf == NULL)
		memory_allocation_error();
	if (nftnl_flowtable_nlmsg_parse(nlh, nlf) < 0)
		netlink_abi_error();

	return nlf;
}

static uint32_t netlink_msg2nftnl_of(uint32_t type, uint16_t flags)
{
	switch (type) {
	case NFT_MSG_NEWRULE:
		if (flags & NLM_F_APPEND)
			return NFT_OF_EVENT_ADD;
		else
			return NFT_OF_EVENT_INSERT;
	case NFT_MSG_NEWTABLE:
	case NFT_MSG_NEWCHAIN:
	case NFT_MSG_NEWSET:
	case NFT_MSG_NEWSETELEM:
	case NFT_MSG_NEWOBJ:
	case NFT_MSG_NEWFLOWTABLE:
		if (flags & NLM_F_EXCL)
			return NFT_OF_EVENT_CREATE;
		else
			return NFT_OF_EVENT_ADD;
	case NFT_MSG_DELTABLE:
	case NFT_MSG_DELCHAIN:
	case NFT_MSG_DELSET:
	case NFT_MSG_DELSETELEM:
	case NFT_MSG_DELRULE:
	case NFT_MSG_DELOBJ:
	case NFT_MSG_DELFLOWTABLE:
		return NFTNL_OF_EVENT_DEL;
	}

	return 0;
}

static const char *nftnl_of2cmd(uint32_t of)
{
	switch (of) {
	case NFT_OF_EVENT_ADD:
		return "add";
	case NFT_OF_EVENT_CREATE:
		return "create";
	case NFT_OF_EVENT_INSERT:
		return "insert";
	case NFT_OF_EVENT_DEL:
		return "delete";
	default:
		return "???";
	}
}

static const char *netlink_msg2cmd(uint32_t type, uint16_t flags)
{
	return nftnl_of2cmd(netlink_msg2nftnl_of(type, flags));
}

static void nlr_for_each_set(struct nftnl_rule *nlr,
			     void (*cb)(struct set *s, void *data),
			     void *data, struct nft_cache *cache)
{
	struct nftnl_expr_iter *nlrei;
	struct nftnl_expr *nlre;
	const char *set_name, *table;
	const char *name;
	struct set *s;
	uint32_t family;

	nlrei = nftnl_expr_iter_create(nlr);
	if (nlrei == NULL)
		memory_allocation_error();

	family = nftnl_rule_get_u32(nlr, NFTNL_RULE_FAMILY);
	table = nftnl_rule_get_str(nlr, NFTNL_RULE_TABLE);

	nlre = nftnl_expr_iter_next(nlrei);
	while (nlre != NULL) {
		name = nftnl_expr_get_str(nlre, NFTNL_EXPR_NAME);
		if (strcmp(name, "lookup") != 0)
			goto next;

		set_name = nftnl_expr_get_str(nlre, NFTNL_EXPR_LOOKUP_SET);
		s = set_lookup_global(family, table, set_name, cache);
		if (s == NULL)
			goto next;

		cb(s, data);
next:
		nlre = nftnl_expr_iter_next(nlrei);
	}
	nftnl_expr_iter_destroy(nlrei);
}

static int netlink_events_table_cb(const struct nlmsghdr *nlh, int type,
				   struct netlink_mon_handler *monh)
{
	struct nftnl_table *nlt;
	struct table *t;
	const char *cmd;

	nlt = netlink_table_alloc(nlh);
	t = netlink_delinearize_table(monh->ctx, nlt);
	if (!t) {
		nftnl_table_free(nlt);
		return MNL_CB_ERROR;
	}
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		nft_mon_print(monh, "%s table ", cmd);

		nft_mon_print(monh, "%s %s", family2str(t->handle.family),
			      t->handle.table.name);

		if (t->flags & TABLE_F_DORMANT)
			nft_mon_print(monh, " { flags dormant; }");

		if (nft_output_handle(&monh->ctx->nft->output))
			nft_mon_print(monh, " # handle %" PRIu64 "",
				      t->handle.handle.id);
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_table_json(monh, cmd, t);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	table_free(t);
	nftnl_table_free(nlt);
	return MNL_CB_OK;
}

static int netlink_events_chain_cb(const struct nlmsghdr *nlh, int type,
				   struct netlink_mon_handler *monh)
{
	struct nftnl_chain *nlc;
	struct chain *c;
	const char *cmd;

	nlc = netlink_chain_alloc(nlh);
	c = netlink_delinearize_chain(monh->ctx, nlc);
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		nft_mon_print(monh, "%s ", cmd);

		switch (type) {
		case NFT_MSG_NEWCHAIN:
			chain_print_plain(c, &monh->ctx->nft->output);
			break;
		case NFT_MSG_DELCHAIN:
			if (c->dev_array_len > 0)
				chain_print_plain(c, &monh->ctx->nft->output);
			else
				nft_mon_print(monh, "chain %s %s %s",
					      family2str(c->handle.family),
					      c->handle.table.name,
					      c->handle.chain.name);
			break;
		}
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_chain_json(monh, cmd, c);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	chain_free(c);
	nftnl_chain_free(nlc);
	return MNL_CB_OK;
}

static int netlink_events_set_cb(const struct nlmsghdr *nlh, int type,
				 struct netlink_mon_handler *monh)
{
	struct nftnl_set *nls;
	const char *family, *cmd;
	struct set *set;
	uint32_t flags;

	nls = netlink_set_alloc(nlh);
	flags = nftnl_set_get_u32(nls, NFTNL_SET_FLAGS);
	if (set_is_anonymous(flags))
		goto out;

	set = netlink_delinearize_set(monh->ctx, nls);
	if (set == NULL) {
		nftnl_set_free(nls);
		return MNL_CB_ERROR;
	}
	family = family2str(set->handle.family);
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		nft_mon_print(monh, "%s ", cmd);

		switch (type) {
		case NFT_MSG_NEWSET:
			set_print_plain(set, &monh->ctx->nft->output);
			break;
		case NFT_MSG_DELSET:
			nft_mon_print(monh, "set %s %s %s", family,
				      set->handle.table.name,
				      set->handle.set.name);
			break;
		}
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_set_json(monh, cmd, set);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	set_free(set);
out:
	nftnl_set_free(nls);
	return MNL_CB_OK;
}

/* returns true if the event should be ignored (i.e. null element) */
static bool netlink_event_ignore_range_event(struct nftnl_set_elem *nlse)
{
        uint32_t flags = 0;

	if (nftnl_set_elem_is_set(nlse, NFTNL_SET_ELEM_FLAGS))
		flags = nftnl_set_elem_get_u32(nlse, NFTNL_SET_ELEM_FLAGS);
	if (!(flags & NFT_SET_ELEM_INTERVAL_END))
		return false;

	if (nftnl_set_elem_get_u32(nlse, NFTNL_SET_ELEM_KEY) != 0)
		return false;

	return true;
}

static bool set_elem_is_open_interval(struct expr *elem)
{
	switch (elem->etype) {
	case EXPR_SET_ELEM:
		return elem->key->flags & EXPR_F_INTERVAL_OPEN;
	case EXPR_MAPPING:
		return set_elem_is_open_interval(elem->left);
	default:
		return false;
	}
}

/* returns true if the we cached the range element */
static bool netlink_event_range_cache(struct set *cached_set,
				      struct set *dummyset)
{
	struct expr *elem;

	/* not an interval ? */
	if (!(cached_set->flags & NFT_SET_INTERVAL))
		return false;

	/* if cache exists, dummyset must contain the other end of the range */
	if (cached_set->rg_cache) {
		set_expr_add(dummyset->init, cached_set->rg_cache);
		cached_set->rg_cache = NULL;
		goto out_decompose;
	}

	/* don't cache half-open range elements */
	elem = list_entry(expr_set(dummyset->init)->expressions.prev, struct expr, list);
	if (!set_elem_is_open_interval(elem) &&
	    dummyset->desc.field_count <= 1) {
		cached_set->rg_cache = expr_clone(elem);
		return true;
	}

out_decompose:
	if (dummyset->flags & NFT_SET_INTERVAL &&
	    dummyset->desc.field_count > 1)
		concat_range_aggregate(dummyset->init);
	else
		interval_map_decompose(dummyset->init);

	return false;
}

static int netlink_events_setelem_cb(const struct nlmsghdr *nlh, int type,
				     struct netlink_mon_handler *monh)
{
	struct nftnl_set_elems_iter *nlsei;
	struct nftnl_set_elem *nlse;
	struct nftnl_set *nls;
	struct set *dummyset;
	struct set *set;
	const char *setname, *table, *cmd;
	uint32_t family;

	nls = netlink_setelem_alloc(nlh);
	table = nftnl_set_get_str(nls, NFTNL_SET_TABLE);
	setname = nftnl_set_get_str(nls, NFTNL_SET_NAME);
	family = nftnl_set_get_u32(nls, NFTNL_SET_FAMILY);
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	set = set_lookup_global(family, table, setname, &monh->ctx->nft->cache);
	if (set == NULL) {
		fprintf(stderr, "W: Received event for an unknown set.\n");
		goto out;
	}

	if (set_is_anonymous(set->flags))
		goto out;

	/* we want to 'delinearize' the set_elem, but don't
	 * modify the original cached set. This path is only
	 * used by named sets, so use a dummy set.
	 */
	dummyset = set_alloc(monh->loc);
	handle_merge(&dummyset->handle, &set->handle);
	dummyset->key = expr_clone(set->key);
	if (set->data)
		dummyset->data = expr_clone(set->data);
	dummyset->flags = set->flags;
	dummyset->init = set_expr_alloc(monh->loc, set);
	dummyset->desc.field_count = set->desc.field_count;

	nlsei = nftnl_set_elems_iter_create(nls);
	if (nlsei == NULL)
		memory_allocation_error();

	nlse = nftnl_set_elems_iter_next(nlsei);
	while (nlse != NULL) {
		if (netlink_event_ignore_range_event(nlse)) {
			set_free(dummyset);
			nftnl_set_elems_iter_destroy(nlsei);
			goto out;
		}
		if (netlink_delinearize_setelem(monh->ctx,
						nlse, dummyset) < 0) {
			set_free(dummyset);
			nftnl_set_elems_iter_destroy(nlsei);
			goto out;
		}
		nlse = nftnl_set_elems_iter_next(nlsei);
	}
	nftnl_set_elems_iter_destroy(nlsei);

	if (netlink_event_range_cache(set, dummyset)) {
		set_free(dummyset);
		goto out;
	}

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		nft_mon_print(monh, "%s element %s %s %s ",
			      cmd, family2str(family), table, setname);
		expr_print(dummyset->init, &monh->ctx->nft->output);
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_element_json(monh, cmd, dummyset);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	set_free(dummyset);
out:
	nftnl_set_free(nls);
	return MNL_CB_OK;
}

static int netlink_events_obj_cb(const struct nlmsghdr *nlh, int type,
				 struct netlink_mon_handler *monh)
{
	const char *family, *cmd;
	struct nftnl_obj *nlo;
	struct obj *obj;

	nlo = netlink_obj_alloc(nlh);

	obj = netlink_delinearize_obj(monh->ctx, nlo);
	if (!obj) {
		nftnl_obj_free(nlo);
		return MNL_CB_ERROR;
	}
	family = family2str(obj->handle.family);
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		nft_mon_print(monh, "%s ", cmd);

		switch (type) {
		case NFT_MSG_NEWOBJ:
			obj_print_plain(obj, &monh->ctx->nft->output);
			break;
		case NFT_MSG_DELOBJ:
			nft_mon_print(monh, "%s %s %s %s",
			       obj_type_name(obj->type),
			       family,
			       obj->handle.table.name,
			       obj->handle.obj.name);
			break;
		}
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_obj_json(monh, cmd, obj, type == NFT_MSG_DELOBJ);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	obj_free(obj);
	nftnl_obj_free(nlo);
	return MNL_CB_OK;
}

static int netlink_events_flowtable_cb(const struct nlmsghdr *nlh, int type,
				       struct netlink_mon_handler *monh)
{
	const char *family, *cmd;
	struct nftnl_flowtable *nlf;
	struct flowtable *ft;

	nlf = netlink_flowtable_alloc(nlh);

	ft = netlink_delinearize_flowtable(monh->ctx, nlf);
	if (!ft) {
		nftnl_flowtable_free(nlf);
		return MNL_CB_ERROR;
	}
	family = family2str(ft->handle.family);
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		nft_mon_print(monh, "%s ", cmd);

		switch (type) {
		case NFT_MSG_DELFLOWTABLE:
			if (!ft->dev_array_len) {
				nft_mon_print(monh, "flowtable %s %s %s",
					      family,
					      ft->handle.table.name,
					      ft->handle.flowtable.name);
				break;
			}
			/* fall through */
		case NFT_MSG_NEWFLOWTABLE:
			flowtable_print_plain(ft, &monh->ctx->nft->output);
			break;
		}
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_flowtable_json(monh, cmd, ft);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	flowtable_free(ft);
	nftnl_flowtable_free(nlf);
	return MNL_CB_OK;
}

static void rule_map_decompose_cb(struct set *s, void *data)
{
	if (!set_is_anonymous(s->flags))
		return;

	if (set_is_non_concat_range(s))
		interval_map_decompose(s->init);
	else if (set_is_interval(s->flags))
		concat_range_aggregate(s->init);
}

static int netlink_events_rule_cb(const struct nlmsghdr *nlh, int type,
				  struct netlink_mon_handler *monh)
{
	const char *family, *cmd;
	struct nftnl_rule *nlr;
	struct rule *r;

	nlr = netlink_rule_alloc(nlh);
	r = netlink_delinearize_rule(monh->ctx, nlr);
	if (!r) {
		fprintf(stderr, "W: Received event for an unknown table.\n");
		goto out_free_nlr;
	}
	nlr_for_each_set(nlr, rule_map_decompose_cb, NULL,
			 &monh->ctx->nft->cache);
	cmd = netlink_msg2cmd(type, nlh->nlmsg_flags);

	switch (monh->format) {
	case NFTNL_OUTPUT_DEFAULT:
		family = family2str(r->handle.family);

		nft_mon_print(monh, "%s rule %s %s %s ",
			      cmd,
			      family,
			      r->handle.table.name,
			      r->handle.chain.name);
		if (r->handle.position.id) {
			nft_mon_print(monh, "handle %" PRIu64" ",
				      r->handle.position.id);
		}
		switch (type) {
		case NFT_MSG_NEWRULE:
			rule_print(r, &monh->ctx->nft->output);

			break;
		case NFT_MSG_DELRULE:
			nft_mon_print(monh, "handle %" PRIu64,
				      r->handle.handle.id);
			break;
		}
		nft_mon_print(monh, "\n");
		break;
	case NFTNL_OUTPUT_JSON:
		monitor_print_rule_json(monh, cmd, r);
		if (!nft_output_echo(&monh->ctx->nft->output))
			nft_mon_print(monh, "\n");
		break;
	}
	rule_free(r);
out_free_nlr:
	nftnl_rule_free(nlr);
	return MNL_CB_OK;
}

static void netlink_events_cache_addtable(struct netlink_mon_handler *monh,
					  const struct nlmsghdr *nlh)
{
	struct nftnl_table *nlt;
	struct table *t;

	nlt = netlink_table_alloc(nlh);
	t = netlink_delinearize_table(monh->ctx, nlt);
	nftnl_table_free(nlt);

	table_cache_add(t, &monh->ctx->nft->cache);
}

static void netlink_events_cache_deltable(struct netlink_mon_handler *monh,
					  const struct nlmsghdr *nlh)
{
	struct nftnl_table *nlt;
	struct table *t;
	struct handle h;

	nlt      = netlink_table_alloc(nlh);
	h.family = nftnl_table_get_u32(nlt, NFTNL_TABLE_FAMILY);
	h.table.name  = nftnl_table_get_str(nlt, NFTNL_TABLE_NAME);

	t = table_cache_find(&monh->ctx->nft->cache.table_cache,
			     h.table.name, h.family);
	if (t == NULL)
		goto out;

	table_cache_del(t);
	table_free(t);
out:
	nftnl_table_free(nlt);
}

static void netlink_events_cache_addset(struct netlink_mon_handler *monh,
					const struct nlmsghdr *nlh)
{
	struct netlink_ctx set_tmpctx;
	struct nftnl_set *nls;
	struct table *t;
	struct set *s;
	LIST_HEAD(msgs);

	memset(&set_tmpctx, 0, sizeof(set_tmpctx));
	init_list_head(&set_tmpctx.list);
	init_list_head(&msgs);
	set_tmpctx.nft = monh->ctx->nft;
	set_tmpctx.msgs = &msgs;

	nls = netlink_set_alloc(nlh);
	s = netlink_delinearize_set(&set_tmpctx, nls);
	if (s == NULL)
		goto out;
	s->init = set_expr_alloc(monh->loc, s);

	t = table_cache_find(&monh->ctx->nft->cache.table_cache,
			     s->handle.table.name, s->handle.family);
	if (t == NULL) {
		fprintf(stderr, "W: Unable to cache set: table not found.\n");
		set_free(s);
		goto out;
	}

	if (nft_output_echo(&monh->ctx->nft->output) &&
	    !set_is_anonymous(s->flags)) {
		set_free(s);
		goto out;
	}

	set_cache_add(s, t);
out:
	nftnl_set_free(nls);
}

static void netlink_events_cache_addsetelem(struct netlink_mon_handler *monh,
					    const struct nlmsghdr *nlh)
{
	struct nftnl_set_elems_iter *nlsei;
	struct nftnl_set_elem *nlse;
	struct nftnl_set *nls;
	struct set *set;
	const char *table, *setname;
	uint32_t family;

	nls     = netlink_setelem_alloc(nlh);
	family  = nftnl_set_get_u32(nls, NFTNL_SET_FAMILY);
	table   = nftnl_set_get_str(nls, NFTNL_SET_TABLE);
	setname = nftnl_set_get_str(nls, NFTNL_SET_NAME);

	set = set_lookup_global(family, table, setname, &monh->ctx->nft->cache);
	if (set == NULL) {
		fprintf(stderr,
			"W: Unable to cache set_elem. Set not found.\n");
		goto out;
	}

	if (nft_output_echo(&monh->ctx->nft->output) &&
	    !set_is_anonymous(set->flags))
		goto out;

	nlsei = nftnl_set_elems_iter_create(nls);
	if (nlsei == NULL)
		memory_allocation_error();

	nlse = nftnl_set_elems_iter_next(nlsei);
	while (nlse != NULL) {
		if (netlink_delinearize_setelem(monh->ctx, nlse, set) < 0) {
			fprintf(stderr,
				"W: Unable to cache set_elem. "
				"Delinearize failed.\n");
			nftnl_set_elems_iter_destroy(nlsei);
			goto out;
		}
		nlse = nftnl_set_elems_iter_next(nlsei);
	}
	nftnl_set_elems_iter_destroy(nlsei);
out:
	nftnl_set_free(nls);
}

static void netlink_events_cache_delset_cb(struct set *s,
					   void *data)
{
	set_cache_del(s);
	set_free(s);
}

static void netlink_events_cache_delsets(struct netlink_mon_handler *monh,
					 const struct nlmsghdr *nlh)
{
	struct nftnl_rule *nlr = netlink_rule_alloc(nlh);

	nlr_for_each_set(nlr, netlink_events_cache_delset_cb, NULL,
			 &monh->ctx->nft->cache);
	nftnl_rule_free(nlr);
}

static void netlink_events_cache_addobj(struct netlink_mon_handler *monh,
					const struct nlmsghdr *nlh)
{
	struct netlink_ctx obj_tmpctx;
	struct nftnl_obj *nlo;
	struct table *t;
	struct obj *obj;
	LIST_HEAD(msgs);

	memset(&obj_tmpctx, 0, sizeof(obj_tmpctx));
	init_list_head(&obj_tmpctx.list);
	init_list_head(&msgs);
	obj_tmpctx.msgs = &msgs;

	nlo = netlink_obj_alloc(nlh);
	obj = netlink_delinearize_obj(&obj_tmpctx, nlo);
	if (obj == NULL)
		goto out;

	t = table_cache_find(&monh->ctx->nft->cache.table_cache,
			     obj->handle.table.name, obj->handle.family);
	if (t == NULL) {
		fprintf(stderr, "W: Unable to cache object: table not found.\n");
		obj_free(obj);
		goto out;
	}

	obj_cache_add(obj, t);
out:
	nftnl_obj_free(nlo);
}

static void netlink_events_cache_delobj(struct netlink_mon_handler *monh,
					const struct nlmsghdr *nlh)
{
	struct nftnl_obj *nlo;
	const char *name;
	struct obj *obj;
	struct handle h;
	struct table *t;
	uint32_t type;

	nlo      = netlink_obj_alloc(nlh);
	h.family = nftnl_obj_get_u32(nlo, NFTNL_OBJ_FAMILY);
	h.table.name  = nftnl_obj_get_str(nlo, NFTNL_OBJ_TABLE);

	name     = nftnl_obj_get_str(nlo, NFTNL_OBJ_NAME);
	type	 = nftnl_obj_get_u32(nlo, NFTNL_OBJ_TYPE);
	h.handle.id	= nftnl_obj_get_u64(nlo, NFTNL_OBJ_HANDLE);

	t = table_cache_find(&monh->ctx->nft->cache.table_cache,
			     h.table.name, h.family);
	if (t == NULL) {
		fprintf(stderr, "W: Unable to cache object: table not found.\n");
		goto out;
	}

	obj = obj_cache_find(t, name, type);
	if (obj == NULL) {
		fprintf(stderr, "W: Unable to find object in cache\n");
		goto out;
	}

	obj_cache_del(obj);
	obj_free(obj);
out:
	nftnl_obj_free(nlo);
}

static void netlink_events_cache_update(struct netlink_mon_handler *monh,
					const struct nlmsghdr *nlh, int type)
{
	if (nft_output_echo(&monh->ctx->nft->output) &&
	    type != NFT_MSG_NEWSET && type != NFT_MSG_NEWSETELEM)
		return;

	switch (type) {
	case NFT_MSG_NEWTABLE:
		netlink_events_cache_addtable(monh, nlh);
		break;
	case NFT_MSG_DELTABLE:
		netlink_events_cache_deltable(monh, nlh);
		break;
	case NFT_MSG_NEWSET:
		netlink_events_cache_addset(monh, nlh);
		break;
	case NFT_MSG_NEWSETELEM:
		netlink_events_cache_addsetelem(monh, nlh);
		break;
	case NFT_MSG_DELRULE:
		/* there are no notification for anon-set deletion */
		netlink_events_cache_delsets(monh, nlh);
		break;
	case NFT_MSG_NEWOBJ:
		netlink_events_cache_addobj(monh, nlh);
		break;
	case NFT_MSG_DELOBJ:
		netlink_events_cache_delobj(monh, nlh);
		break;
	}
}

/* only those which could be useful listening to events */
static const char *const nftnl_msg_types[NFT_MSG_MAX] = {
	[NFT_MSG_NEWTABLE]	= "NFT_MSG_NEWTABLE",
	[NFT_MSG_DELTABLE]	= "NFT_MSG_DELTABLE",
	[NFT_MSG_NEWCHAIN]	= "NFT_MSG_NEWCHAIN",
	[NFT_MSG_DELCHAIN]	= "NFT_MSG_DELCHAIN",
	[NFT_MSG_NEWSET]	= "NFT_MSG_NEWSET",
	[NFT_MSG_DELSET]	= "NFT_MSG_DELSET",
	[NFT_MSG_NEWSETELEM]	= "NFT_MSG_NEWSETELEM",
	[NFT_MSG_DELSETELEM]	= "NFT_MSG_DELSETELEM",
	[NFT_MSG_NEWRULE]	= "NFT_MSG_NEWRULE",
	[NFT_MSG_DELRULE]	= "NFT_MSG_DELRULE",
	[NFT_MSG_TRACE]		= "NFT_MSG_TRACE",
	[NFT_MSG_NEWGEN]	= "NFT_MSG_NEWGEN",
	[NFT_MSG_NEWOBJ]	= "NFT_MSG_NEWOBJ",
	[NFT_MSG_DELOBJ]	= "NFT_MSG_DELOBJ",
};

static const char *nftnl_msgtype2str(uint16_t type)
{
	if (type >= NFT_MSG_MAX || !nftnl_msg_types[type])
		return "unknown";

	return nftnl_msg_types[type];
}

static void netlink_events_debug(uint16_t type, unsigned int debug_mask)
{
	if (!(debug_mask & NFT_DEBUG_NETLINK))
		return;

	printf("netlink event: %s\n", nftnl_msgtype2str(type));
}

static int netlink_events_newgen_cb(const struct nlmsghdr *nlh, int type,
				    struct netlink_mon_handler *monh)
{
	const struct nlattr *attr;
	char name[256] = "";
	int genid = -1, pid = -1;

	if (monh->format != NFTNL_OUTPUT_DEFAULT)
		return MNL_CB_OK;

	mnl_attr_for_each(attr, nlh, sizeof(struct nfgenmsg)) {
		switch (mnl_attr_get_type(attr)) {
		case NFTA_GEN_ID:
			if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
				break;
			genid = ntohl(mnl_attr_get_u32(attr));
			break;
		case NFTA_GEN_PROC_NAME:
			if (mnl_attr_validate(attr, MNL_TYPE_NUL_STRING) < 0)
				break;
			snprintf(name, sizeof(name), "%s", mnl_attr_get_str(attr));
			break;
		case NFTA_GEN_PROC_PID:
			if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
				break;
			pid = ntohl(mnl_attr_get_u32(attr));
			break;
		}
	}
	if (genid >= 0) {
		nft_mon_print(monh, "# new generation %d", genid);
		if (pid >= 0)
			nft_mon_print(monh, " by process %d (%s)", pid, name);

		nft_mon_print(monh, "\n");
	}

	return MNL_CB_OK;
}

static int netlink_events_cb(const struct nlmsghdr *nlh, void *data)
{
	int ret = MNL_CB_OK;
	uint16_t type = NFNL_MSG_TYPE(nlh->nlmsg_type);
	struct netlink_mon_handler *monh = (struct netlink_mon_handler *)data;

	netlink_events_debug(type, monh->ctx->nft->debug_mask);
	netlink_events_cache_update(monh, nlh, type);

	if (!(monh->monitor_flags & (1 << type)))
		return ret;

	switch (type) {
	case NFT_MSG_NEWTABLE:
	case NFT_MSG_DELTABLE:
		ret = netlink_events_table_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWCHAIN:
	case NFT_MSG_DELCHAIN:
		ret = netlink_events_chain_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWSET:
	case NFT_MSG_DELSET:		/* nft {add|delete} set */
		ret = netlink_events_set_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWSETELEM:
	case NFT_MSG_DELSETELEM:	/* nft {add|delete} element */
		ret = netlink_events_setelem_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWRULE:
	case NFT_MSG_DELRULE:
		ret = netlink_events_rule_cb(nlh, type, monh);
		break;
	case NFT_MSG_TRACE:
		ret = netlink_events_trace_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWOBJ:
	case NFT_MSG_DELOBJ:
		ret = netlink_events_obj_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWFLOWTABLE:
	case NFT_MSG_DELFLOWTABLE:
		ret = netlink_events_flowtable_cb(nlh, type, monh);
		break;
	case NFT_MSG_NEWGEN:
		ret = netlink_events_newgen_cb(nlh, type, monh);
		break;
	}

	return ret;
}

int netlink_echo_callback(const struct nlmsghdr *nlh, void *data)
{
	struct netlink_cb_data *nl_cb_data = data;
	struct netlink_ctx *ctx = nl_cb_data->nl_ctx;
	struct nft_ctx *nft = ctx->nft;

	struct netlink_mon_handler echo_monh = {
		.format = NFTNL_OUTPUT_DEFAULT,
		.ctx = ctx,
		.loc = &netlink_location,
		.monitor_flags = 0xffffffff,
	};

	if (!nft_output_echo(&echo_monh.ctx->nft->output))
		return MNL_CB_OK;

	if (nft_output_json(&nft->output)) {
		if (nft->json_root)
			return json_events_cb(nlh, &echo_monh);
		if (!nft->json_echo)
			json_alloc_echo(nft);
		echo_monh.format = NFTNL_OUTPUT_JSON;
	}

	return netlink_events_cb(nlh, &echo_monh);
}

int netlink_monitor(struct netlink_mon_handler *monhandler,
		    struct mnl_socket *nf_sock)
{
	int group;

	if (monhandler->monitor_flags & (1 << NFT_MSG_TRACE)) {
		group = NFNLGRP_NFTRACE;
		if (mnl_socket_setsockopt(nf_sock, NETLINK_ADD_MEMBERSHIP,
					  &group, sizeof(int)) < 0)
			return -1;
	}
	if (monhandler->monitor_flags & ~(1 << NFT_MSG_TRACE)) {
		group = NFNLGRP_NFTABLES;
		if (mnl_socket_setsockopt(nf_sock, NETLINK_ADD_MEMBERSHIP,
					  &group, sizeof(int)) < 0)
			return -1;
	}

	return mnl_nft_event_listener(nf_sock, monhandler->ctx->nft->debug_mask,
				      &monhandler->ctx->nft->output,
				      netlink_events_cb, monhandler);
}
