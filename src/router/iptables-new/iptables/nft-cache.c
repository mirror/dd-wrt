/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <xtables.h>

#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/gen.h>
#include <libnftnl/set.h>
#include <libnftnl/table.h>

#include "nft.h"
#include "nft-cache.h"

static void cache_chain_list_insert(struct list_head *list, const char *name)
{
	struct cache_chain *pos = NULL, *new;

	list_for_each_entry(pos, list, head) {
		int cmp = strcmp(pos->name, name);

		if (!cmp)
			return;
		if (cmp > 0)
			break;
	}

	new = xtables_malloc(sizeof(*new));
	new->name = strdup(name);
	list_add_tail(&new->head, pos ? &pos->head : list);
}

void nft_cache_level_set(struct nft_handle *h, int level,
			 const struct nft_cmd *cmd)
{
	struct nft_cache_req *req = &h->cache_req;

	if (level > req->level)
		req->level = level;

	if (!cmd || !cmd->table || req->all_chains)
		return;

	if (!req->table)
		req->table = strdup(cmd->table);
	else
		assert(!strcmp(req->table, cmd->table));

	if (!cmd->chain) {
		req->all_chains = true;
		return;
	}

	cache_chain_list_insert(&req->chain_list, cmd->chain);
	if (cmd->rename)
		cache_chain_list_insert(&req->chain_list, cmd->rename);
	if (cmd->jumpto)
		cache_chain_list_insert(&req->chain_list, cmd->jumpto);
}

static int genid_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t *genid = data;
	struct nftnl_gen *gen;

	gen = nftnl_gen_alloc();
	if (!gen)
		return MNL_CB_ERROR;

	if (nftnl_gen_nlmsg_parse(nlh, gen) < 0)
		goto out;

	*genid = nftnl_gen_get_u32(gen, NFTNL_GEN_ID);

	nftnl_gen_free(gen);
	return MNL_CB_STOP;
out:
	nftnl_gen_free(gen);
	return MNL_CB_ERROR;
}

static void mnl_genid_get(struct nft_handle *h, uint32_t *genid)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	int ret;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETGEN, 0, 0, h->seq);
	ret = mnl_talk(h, nlh, genid_cb, genid);
	if (ret == 0)
		return;

	xtables_error(RESOURCE_PROBLEM,
		      "Could not fetch rule set generation id: %s\n", nft_strerror(errno));
}

static int nftnl_table_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_table *t;
	struct nftnl_table_list *list = data;

	t = nftnl_table_alloc();
	if (t == NULL)
		goto err;

	if (nftnl_table_nlmsg_parse(nlh, t) < 0)
		goto out;

	nftnl_table_list_add_tail(t, list);

	return MNL_CB_OK;
out:
	nftnl_table_free(t);
err:
	return MNL_CB_OK;
}

static int fetch_table_cache(struct nft_handle *h)
{
	char buf[16536];
	struct nlmsghdr *nlh;
	struct nftnl_table_list *list;
	int i, ret;

	if (h->cache->tables)
		return 0;

	list = nftnl_table_list_alloc();
	if (list == NULL)
		return 0;

	nlh = nftnl_rule_nlmsg_build_hdr(buf, NFT_MSG_GETTABLE, h->family,
					NLM_F_DUMP, h->seq);

	ret = mnl_talk(h, nlh, nftnl_table_list_cb, list);
	if (ret < 0 && errno == EINTR)
		assert(nft_restart(h) >= 0);

	h->cache->tables = list;

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		enum nft_table_type type = h->tables[i].type;

		if (!h->tables[i].name)
			continue;

		h->cache->table[type].chains = nftnl_chain_list_alloc();
		if (!h->cache->table[type].chains)
			return 0;

		h->cache->table[type].sets = nftnl_set_list_alloc();
		if (!h->cache->table[type].sets)
			return 0;
	}

	return 1;
}

struct nftnl_chain_list_cb_data {
	struct nft_handle *h;
	const struct builtin_table *t;
};

static int nftnl_chain_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_chain_list_cb_data *d = data;
	const struct builtin_table *t = d->t;
	struct nftnl_chain_list *list;
	struct nft_handle *h = d->h;
	const char *tname, *cname;
	struct nftnl_chain *c;

	c = nftnl_chain_alloc();
	if (c == NULL)
		goto err;

	if (nftnl_chain_nlmsg_parse(nlh, c) < 0)
		goto out;

	tname = nftnl_chain_get_str(c, NFTNL_CHAIN_TABLE);

	if (!t) {
		t = nft_table_builtin_find(h, tname);
		if (!t)
			goto out;
	} else if (strcmp(t->name, tname)) {
		goto out;
	}

	list = h->cache->table[t->type].chains;
	cname = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);

	if (nftnl_chain_list_lookup_byname(list, cname))
		goto out;

	nftnl_chain_list_add_tail(c, list);

	return MNL_CB_OK;
out:
	nftnl_chain_free(c);
err:
	return MNL_CB_OK;
}

struct nftnl_set_list_cb_data {
	struct nft_handle *h;
	const struct builtin_table *t;
};

static int nftnl_set_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_set_list_cb_data *d = data;
	const struct builtin_table *t = d->t;
	struct nftnl_set_list *list;
	struct nft_handle *h = d->h;
	const char *tname, *sname;
	struct nftnl_set *s;

	s = nftnl_set_alloc();
	if (s == NULL)
		return MNL_CB_OK;

	if (nftnl_set_nlmsg_parse(nlh, s) < 0)
		goto out_free;

	tname = nftnl_set_get_str(s, NFTNL_SET_TABLE);

	if (!t)
		t = nft_table_builtin_find(h, tname);
	else if (strcmp(t->name, tname))
		goto out_free;

	if (!t)
		goto out_free;

	list = h->cache->table[t->type].sets;
	sname = nftnl_set_get_str(s, NFTNL_SET_NAME);

	if (nftnl_set_list_lookup_byname(list, sname))
		goto out_free;

	nftnl_set_list_add_tail(s, list);

	return MNL_CB_OK;
out_free:
	nftnl_set_free(s);
	return MNL_CB_OK;
}

static int set_elem_cb(const struct nlmsghdr *nlh, void *data)
{
	return nftnl_set_elems_nlmsg_parse(nlh, data) ? -1 : MNL_CB_OK;
}

static bool set_has_elements(struct nftnl_set *s)
{
	struct nftnl_set_elems_iter *iter;
	bool ret = false;

	iter = nftnl_set_elems_iter_create(s);
	if (iter) {
		ret = !!nftnl_set_elems_iter_cur(iter);
		nftnl_set_elems_iter_destroy(iter);
	}
	return ret;
}

static int set_fetch_elem_cb(struct nftnl_set *s, void *data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nft_handle *h = data;
	struct nlmsghdr *nlh;

	if (set_has_elements(s))
		return 0;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETSETELEM, h->family,
				    NLM_F_DUMP, h->seq);
	nftnl_set_elems_nlmsg_build_payload(nlh, s);

	return mnl_talk(h, nlh, set_elem_cb, s);
}

static int fetch_set_cache(struct nft_handle *h,
			   const struct builtin_table *t, const char *set)
{
	struct nftnl_set_list_cb_data d = {
		.h = h,
		.t = t,
	};
	uint16_t flags = NLM_F_DUMP;
	struct nftnl_set *s = NULL;
	struct nlmsghdr *nlh;
	char buf[16536];
	int i, ret;

	if (t) {
		s = nftnl_set_alloc();
		if (!s)
			return -1;

		nftnl_set_set_str(s, NFTNL_SET_TABLE, t->name);

		if (set) {
			nftnl_set_set_str(s, NFTNL_SET_NAME, set);
			flags = NLM_F_ACK;
		}
	}

	nlh = nftnl_set_nlmsg_build_hdr(buf, NFT_MSG_GETSET,
					h->family, flags, h->seq);

	if (s) {
		nftnl_set_nlmsg_build_payload(nlh, s);
		nftnl_set_free(s);
	}

	ret = mnl_talk(h, nlh, nftnl_set_list_cb, &d);
	if (ret < 0 && errno == EINTR) {
		assert(nft_restart(h) >= 0);
		return ret;
	}

	if (t) {
		nftnl_set_list_foreach(h->cache->table[t->type].sets,
				       set_fetch_elem_cb, h);
	} else {
		for (i = 0; i < NFT_TABLE_MAX; i++) {
			enum nft_table_type type = h->tables[i].type;

			if (!h->tables[i].name)
				continue;

			nftnl_set_list_foreach(h->cache->table[type].sets,
					       set_fetch_elem_cb, h);
		}
	}
	return ret;
}

static int __fetch_chain_cache(struct nft_handle *h,
			       const struct builtin_table *t,
			       const struct nftnl_chain *c)
{
	struct nftnl_chain_list_cb_data d = {
		.h = h,
		.t = t,
	};
	char buf[16536];
	struct nlmsghdr *nlh;
	int ret;

	nlh = nftnl_chain_nlmsg_build_hdr(buf, NFT_MSG_GETCHAIN, h->family,
					  c ? NLM_F_ACK : NLM_F_DUMP, h->seq);
	if (c)
		nftnl_chain_nlmsg_build_payload(nlh, c);

	ret = mnl_talk(h, nlh, nftnl_chain_list_cb, &d);
	if (ret < 0 && errno == EINTR)
		assert(nft_restart(h) >= 0);

	return ret;
}

static int fetch_chain_cache(struct nft_handle *h,
			     const struct builtin_table *t,
			     struct list_head *chains)
{
	struct cache_chain *cc;
	struct nftnl_chain *c;
	int rc, ret = 0;

	if (!chains)
		return __fetch_chain_cache(h, t, NULL);

	assert(t);

	c = nftnl_chain_alloc();
	if (!c)
		return -1;

	nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, t->name);

	list_for_each_entry(cc, chains, head) {
		nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, cc->name);
		rc = __fetch_chain_cache(h, t, c);
		if (rc)
			ret = rc;
	}

	nftnl_chain_free(c);
	return ret;
}

static int nftnl_rule_list_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_chain *c = data;
	struct nftnl_rule *r;

	r = nftnl_rule_alloc();
	if (r == NULL)
		return MNL_CB_OK;

	if (nftnl_rule_nlmsg_parse(nlh, r) < 0) {
		nftnl_rule_free(r);
		return MNL_CB_OK;
	}

	nftnl_chain_rule_add_tail(r, c);
	return MNL_CB_OK;
}

static int nft_rule_list_update(struct nftnl_chain *c, void *data)
{
	struct nft_handle *h = data;
	char buf[16536];
	struct nlmsghdr *nlh;
	struct nftnl_rule *rule;
	int ret;

	if (nftnl_rule_lookup_byindex(c, 0))
		return 0;

	rule = nftnl_rule_alloc();
	if (!rule)
		return -1;

	nftnl_rule_set_str(rule, NFTNL_RULE_TABLE,
			   nftnl_chain_get_str(c, NFTNL_CHAIN_TABLE));
	nftnl_rule_set_str(rule, NFTNL_RULE_CHAIN,
			   nftnl_chain_get_str(c, NFTNL_CHAIN_NAME));

	nlh = nftnl_rule_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, h->family,
					NLM_F_DUMP, h->seq);
	nftnl_rule_nlmsg_build_payload(nlh, rule);

	ret = mnl_talk(h, nlh, nftnl_rule_list_cb, c);
	if (ret < 0 && errno == EINTR)
		assert(nft_restart(h) >= 0);

	nftnl_rule_free(rule);

	if (h->family == NFPROTO_BRIDGE)
		nft_bridge_chain_postprocess(h, c);

	return 0;
}

static int fetch_rule_cache(struct nft_handle *h,
			    const struct builtin_table *t)
{
	int i;

	if (t) {
		struct nftnl_chain_list *list =
			h->cache->table[t->type].chains;

		return nftnl_chain_list_foreach(list, nft_rule_list_update, h);
	}

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		enum nft_table_type type = h->tables[i].type;

		if (!h->tables[i].name)
			continue;

		if (nftnl_chain_list_foreach(h->cache->table[type].chains,
					     nft_rule_list_update, h))
			return -1;
	}
	return 0;
}

static int flush_cache(struct nft_handle *h, struct nft_cache *c,
		       const char *tablename);

static void
__nft_build_cache(struct nft_handle *h)
{
	struct nft_cache_req *req = &h->cache_req;
	const struct builtin_table *t = NULL;
	struct list_head *chains = NULL;
	uint32_t genid_check;

	if (h->cache_init)
		return;

	if (req->table) {
		t = nft_table_builtin_find(h, req->table);
		if (!req->all_chains)
			chains = &req->chain_list;
	}

	h->cache_init = true;
retry:
	mnl_genid_get(h, &h->nft_genid);

	if (req->level >= NFT_CL_TABLES)
		fetch_table_cache(h);
	if (req->level == NFT_CL_FAKE)
		return;
	if (req->level >= NFT_CL_CHAINS)
		fetch_chain_cache(h, t, chains);
	if (req->level >= NFT_CL_SETS)
		fetch_set_cache(h, t, NULL);
	if (req->level >= NFT_CL_RULES)
		fetch_rule_cache(h, t);

	mnl_genid_get(h, &genid_check);
	if (h->nft_genid != genid_check) {
		flush_cache(h, h->cache, NULL);
		goto retry;
	}
}

static void __nft_flush_cache(struct nft_handle *h)
{
	if (!h->cache_index) {
		h->cache_index++;
		h->cache = &h->__cache[h->cache_index];
	} else {
		flush_chain_cache(h, NULL);
	}
}

static int ____flush_rule_cache(struct nftnl_rule *r, void *data)
{
	nftnl_rule_list_del(r);
	nftnl_rule_free(r);

	return 0;
}

static int __flush_rule_cache(struct nftnl_chain *c, void *data)
{
	return nftnl_rule_foreach(c, ____flush_rule_cache, NULL);
}

int flush_rule_cache(struct nft_handle *h, const char *table,
		     struct nftnl_chain *c)
{
	const struct builtin_table *t;

	if (c)
		return __flush_rule_cache(c, NULL);

	t = nft_table_builtin_find(h, table);
	if (!t || !h->cache->table[t->type].chains)
		return 0;

	return nftnl_chain_list_foreach(h->cache->table[t->type].chains,
					__flush_rule_cache, NULL);
}

static int __flush_chain_cache(struct nftnl_chain *c, void *data)
{
	nftnl_chain_list_del(c);
	nftnl_chain_free(c);

	return 0;
}

static int __flush_set_cache(struct nftnl_set *s, void *data)
{
	nftnl_set_list_del(s);
	nftnl_set_free(s);

	return 0;
}

static int flush_cache(struct nft_handle *h, struct nft_cache *c,
		       const char *tablename)
{
	const struct builtin_table *table;
	int i;

	if (tablename) {
		table = nft_table_builtin_find(h, tablename);
		if (!table)
			return 0;
		if (c->table[table->type].chains)
			nftnl_chain_list_foreach(c->table[table->type].chains,
						 __flush_chain_cache, NULL);
		if (c->table[table->type].sets)
			nftnl_set_list_foreach(c->table[table->type].sets,
					       __flush_set_cache, NULL);
		return 0;
	}

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		if (h->tables[i].name == NULL)
			continue;

		if (c->table[i].chains) {
			nftnl_chain_list_free(c->table[i].chains);
			c->table[i].chains = NULL;
		}
		if (c->table[i].sets) {
			nftnl_set_list_free(c->table[i].sets);
			c->table[i].sets = NULL;
		}
	}
	if (c->tables) {
		nftnl_table_list_free(c->tables);
		c->tables = NULL;
	}

	return 1;
}

void flush_chain_cache(struct nft_handle *h, const char *tablename)
{
	if (!h->cache_init)
		return;

	if (flush_cache(h, h->cache, tablename))
		h->cache_init = false;
}

void nft_rebuild_cache(struct nft_handle *h)
{
	if (h->cache_init) {
		__nft_flush_cache(h);
		h->cache_init = false;
	}

	__nft_build_cache(h);
}

void nft_cache_build(struct nft_handle *h)
{
	struct nft_cache_req *req = &h->cache_req;
	const struct builtin_table *t = NULL;
	int i;

	if (req->table)
		t = nft_table_builtin_find(h, req->table);

	/* fetch builtin chains as well (if existing) so nft_xt_builtin_init()
	 * doesn't override policies by accident */
	if (t && !req->all_chains) {
		for (i = 0; i < NF_INET_NUMHOOKS; i++) {
			const char *cname = t->chains[i].name;

			if (!cname)
				break;
			cache_chain_list_insert(&req->chain_list, cname);
		}
	}

	__nft_build_cache(h);
}

void nft_release_cache(struct nft_handle *h)
{
	struct nft_cache_req *req = &h->cache_req;
	struct cache_chain *cc, *cc_tmp;

	while (h->cache_index)
		flush_cache(h, &h->__cache[h->cache_index--], NULL);
	flush_cache(h, &h->__cache[0], NULL);
	h->cache = &h->__cache[0];
	h->cache_init = false;

	if (req->level != NFT_CL_FAKE)
		req->level = NFT_CL_TABLES;
	if (req->table) {
		free(req->table);
		req->table = NULL;
	}
	req->all_chains = false;
	list_for_each_entry_safe(cc, cc_tmp, &req->chain_list, head) {
		list_del(&cc->head);
		free(cc->name);
		free(cc);
	}
}

struct nftnl_table_list *nftnl_table_list_get(struct nft_handle *h)
{
	return h->cache->tables;
}

struct nftnl_set_list *
nft_set_list_get(struct nft_handle *h, const char *table, const char *set)
{
	const struct builtin_table *t;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return NULL;

	return h->cache->table[t->type].sets;
}

struct nftnl_chain_list *
nft_chain_list_get(struct nft_handle *h, const char *table, const char *chain)
{
	const struct builtin_table *t;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return NULL;

	return h->cache->table[t->type].chains;
}

