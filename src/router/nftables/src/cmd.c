/*
 * Copyright (c) 2020 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 */

#include <nft.h>

#include <erec.h>
#include <mnl.h>
#include <cmd.h>
#include <parser.h>
#include <utils.h>
#include <iface.h>
#include <errno.h>
#include <cache.h>

void cmd_add_loc(struct cmd *cmd, const struct nlmsghdr *nlh, const struct location *loc)
{
	if (cmd->num_attrs >= cmd->attr_array_len) {
		cmd->attr_array_len *= 2;
		cmd->attr = xrealloc(cmd->attr, sizeof(struct nlerr_loc) * cmd->attr_array_len);
	}

	cmd->attr[cmd->num_attrs].seqnum = nlh->nlmsg_seq;
	cmd->attr[cmd->num_attrs].offset = nlh->nlmsg_len;
	cmd->attr[cmd->num_attrs].location = loc;
	cmd->num_attrs++;
}

static int nft_cmd_enoent_table(struct netlink_ctx *ctx, const struct cmd *cmd,
				const struct location *loc)
{
	struct table *table;

	if (!cmd->handle.table.name)
		return 0;

	table = table_lookup_fuzzy(&cmd->handle, &ctx->nft->cache);
	if (!table)
		return 0;

	netlink_io_error(ctx, loc, "%s; did you mean table '%s' in family %s?",
			 strerror(ENOENT), table->handle.table.name,
			 family2str(table->handle.family));
	return 1;
}

static int table_fuzzy_check(struct netlink_ctx *ctx, const struct cmd *cmd,
			     const struct table *table)
{
	if (table_cache_find(&ctx->nft->cache.table_cache,
			     cmd->handle.table.name, cmd->handle.family))
		return 0;

	if (strcmp(cmd->handle.table.name, table->handle.table.name) ||
	    cmd->handle.family != table->handle.family) {
		netlink_io_error(ctx, &cmd->handle.table.location,
				 "%s; did you mean table '%s' in family %s?",
				 strerror(ENOENT), table->handle.table.name,
				 family2str(table->handle.family));
		return 1;
	}

	return 0;
}

static int nft_cmd_enoent_chain(struct netlink_ctx *ctx, const struct cmd *cmd,
				const struct location *loc)
{
	const struct table *table = NULL;
	struct chain *chain;

	if (!cmd->handle.chain.name)
		return 0;

	if (nft_cache_update(ctx->nft, NFT_CACHE_TABLE | NFT_CACHE_CHAIN,
			     ctx->msgs, NULL) < 0)
		return 0;

	chain = chain_lookup_fuzzy(&cmd->handle, &ctx->nft->cache, &table);
	/* check table first. */
	if (!table)
		return 0;

	if (table_fuzzy_check(ctx, cmd, table))
		return 1;

	if (!chain)
		return 0;

	netlink_io_error(ctx, loc, "%s; did you mean chain '%s' in table %s '%s'?",
			 strerror(ENOENT), chain->handle.chain.name,
			 family2str(table->handle.family),
			 table->handle.table.name);
	return 1;
}

static int nft_cmd_enoent_rule(struct netlink_ctx *ctx, const struct cmd *cmd,
			       const struct location *loc)
{
	unsigned int flags = NFT_CACHE_TABLE |
			     NFT_CACHE_CHAIN;
	const struct table *table = NULL;
	struct chain *chain;

	if (nft_cache_update(ctx->nft, flags, ctx->msgs, NULL) < 0)
		return 0;

	chain = chain_lookup_fuzzy(&cmd->handle, &ctx->nft->cache, &table);
	/* check table first. */
	if (!table)
		return 0;

	if (table_fuzzy_check(ctx, cmd, table))
		return 1;

	if (!chain)
		return 0;

	if (strcmp(cmd->handle.chain.name, chain->handle.chain.name)) {
		netlink_io_error(ctx, loc, "%s; did you mean chain '%s' in table %s '%s'?",
				 strerror(ENOENT),
				 chain->handle.chain.name,
				 family2str(table->handle.family),
				 table->handle.table.name);
		return 1;
	}

	return 0;
}

static int nft_cmd_enoent_set(struct netlink_ctx *ctx, const struct cmd *cmd,
			      const struct location *loc)
{
	const struct table *table = NULL;
	struct set *set;

	if (!cmd->handle.set.name)
		return 0;

	if (nft_cache_update(ctx->nft, NFT_CACHE_TABLE | NFT_CACHE_SET,
			     ctx->msgs, NULL) < 0)
		return 0;

	set = set_lookup_fuzzy(cmd->handle.set.name, &ctx->nft->cache, &table);
	/* check table first. */
	if (!table)
		return 0;

	if (table_fuzzy_check(ctx, cmd, table))
		return 1;

	if (!set)
		return 0;

	netlink_io_error(ctx, loc, "%s; did you mean %s '%s' in table %s '%s'?",
			 strerror(ENOENT),
			 set_is_map(set->flags) ? "map" : "set",
			 set->handle.set.name,
			 family2str(set->handle.family),
			 table->handle.table.name);
	return 1;
}

static int nft_cmd_enoent_obj(struct netlink_ctx *ctx, const struct cmd *cmd,
			      const struct location *loc)
{
	const struct table *table = NULL;
	struct obj *obj;

	if (!cmd->handle.obj.name)
		return 0;

	if (nft_cache_update(ctx->nft, NFT_CACHE_TABLE | NFT_CACHE_OBJECT,
			     ctx->msgs, NULL) < 0)
		return 0;

	obj = obj_lookup_fuzzy(cmd->handle.obj.name, &ctx->nft->cache, &table);
	/* check table first. */
	if (!table)
		return 0;

	if (table_fuzzy_check(ctx, cmd, table))
		return 1;

	if (!obj)
		return 0;

	netlink_io_error(ctx, loc, "%s; did you mean obj '%s' in table %s '%s'?",
			 strerror(ENOENT), obj->handle.obj.name,
			 family2str(obj->handle.family),
			 table->handle.table.name);
	return 1;
}

static int nft_cmd_enoent_flowtable(struct netlink_ctx *ctx,
				    const struct cmd *cmd,
				    const struct location *loc)
{
	const struct table *table = NULL;
	struct flowtable *ft;

	if (!cmd->handle.flowtable.name)
		return 0;

	if (nft_cache_update(ctx->nft, NFT_CACHE_TABLE | NFT_CACHE_FLOWTABLE,
			     ctx->msgs, NULL) < 0)
		return 0;

	ft = flowtable_lookup_fuzzy(cmd->handle.flowtable.name,
				    &ctx->nft->cache, &table);
	/* check table first. */
	if (!table)
		return 0;

	if (table_fuzzy_check(ctx, cmd, table))
		return 1;

	if (!ft)
		return 0;

	netlink_io_error(ctx, loc, "%s; did you mean flowtable '%s' in table %s '%s'?",
			 strerror(ENOENT), ft->handle.flowtable.name,
			 family2str(ft->handle.family),
			 table->handle.table.name);
	return 1;
}

static void nft_cmd_enoent(struct netlink_ctx *ctx, const struct cmd *cmd,
			   const struct location *loc, int err)
{
	int ret = 0;

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		ret = nft_cmd_enoent_table(ctx, cmd, loc);
		break;
	case CMD_OBJ_CHAIN:
		ret = nft_cmd_enoent_chain(ctx, cmd, loc);
		break;
	case CMD_OBJ_SET:
		ret = nft_cmd_enoent_set(ctx, cmd, loc);
		break;
	case CMD_OBJ_RULE:
		ret = nft_cmd_enoent_rule(ctx, cmd, loc);
		break;
	case CMD_OBJ_COUNTER:
	case CMD_OBJ_QUOTA:
	case CMD_OBJ_CT_HELPER:
	case CMD_OBJ_CT_TIMEOUT:
	case CMD_OBJ_LIMIT:
	case CMD_OBJ_SECMARK:
	case CMD_OBJ_CT_EXPECT:
	case CMD_OBJ_SYNPROXY:
		ret = nft_cmd_enoent_obj(ctx, cmd, loc);
		break;
	case CMD_OBJ_FLOWTABLE:
		ret = nft_cmd_enoent_flowtable(ctx, cmd, loc);
		break;
	default:
		break;
	}

	if (ret)
		return;

	netlink_io_error(ctx, loc, "Could not process rule: %s", strerror(err));
}

static int nft_cmd_chain_error(struct netlink_ctx *ctx, struct cmd *cmd,
			       struct mnl_err *err)
{
	struct chain *chain = cmd->chain, *existing_chain;
	const struct table *table;
	int priority;

	switch (err->err) {
	case EOPNOTSUPP:
		if (!(chain->flags & CHAIN_F_BASECHAIN))
			break;

		if (!chain->priority.expr || !chain->type.str)
			break;

		mpz_export_data(&priority, chain->priority.expr->value,
				BYTEORDER_HOST_ENDIAN, sizeof(int));
		if (priority <= -200 && !strcmp(chain->type.str, "nat"))
			return netlink_io_error(ctx, &chain->priority.loc,
						"Chains of type \"nat\" must have a priority value above -200");

		if (nft_cache_update(ctx->nft, NFT_CACHE_TABLE | NFT_CACHE_CHAIN,
				     ctx->msgs, NULL) < 0) {
			return netlink_io_error(ctx, &chain->loc,
						"Chain of type \"%s\" is not supported, perhaps kernel support is missing?",
						chain->type.str);
		}

		table = table_cache_find(&ctx->nft->cache.table_cache,
					 cmd->handle.table.name, cmd->handle.family);
		if (table) {
			existing_chain = chain_cache_find(table, cmd->handle.chain.name);
			if (existing_chain && existing_chain != chain &&
			    !strcmp(existing_chain->handle.chain.name, chain->handle.chain.name))
				return netlink_io_error(ctx, &chain->loc,
							"Chain \"%s\" already exists in table %s '%s' with different declaration",
							chain->handle.chain.name,
							family2str(table->handle.family), table->handle.table.name);
		}

		return netlink_io_error(ctx, &chain->loc,
					"Chain of type \"%s\" is not supported, perhaps kernel support is missing?",
					chain->type.str);
	default:
		break;
	}

	return 0;
}

void nft_cmd_error(struct netlink_ctx *ctx, struct cmd *cmd,
		   struct mnl_err *err)
{
	const struct location *loc = NULL;
	uint32_t i;

	for (i = 0; i < cmd->num_attrs; i++) {
		if (cmd->attr[i].seqnum == err->seqnum &&
		    cmd->attr[i].offset == err->offset)
			loc = cmd->attr[i].location;
	}

	if (loc) {
		if (err->err == ENOENT) {
			nft_cmd_enoent(ctx, cmd, loc, err->err);
			return;
		}
	} else {
		loc = &cmd->location;
	}

	switch (cmd->obj) {
	case CMD_OBJ_CHAIN:
		if (nft_cmd_chain_error(ctx, cmd, err) < 0)
			return;
		break;
	default:
		break;
	}

	if (cmd->op == CMD_DESTROY && err->err == EINVAL) {
		netlink_io_error(ctx, loc,
				 "\"destroy\" command is not supported, perhaps kernel support is missing?");
		return;
	}

	netlink_io_error(ctx, loc, "Could not process rule: %s",
			 strerror(err->err));
}

static void nft_cmd_expand_chain(struct chain *chain, struct list_head *new_cmds)
{
	struct rule *rule, *next;
	struct handle h;
	struct cmd *new;

	list_for_each_entry_safe(rule, next, &chain->rules, list) {
		list_del(&rule->list);
		handle_merge(&rule->handle, &chain->handle);
		memset(&h, 0, sizeof(h));
		handle_merge(&h, &chain->handle);
		if (chain->flags & CHAIN_F_BINDING) {
			rule->handle.chain_id = chain->handle.chain_id;
			rule->handle.chain.location = chain->location;
		}
		new = cmd_alloc(CMD_ADD, CMD_OBJ_RULE, &h,
				&rule->location, rule);
		list_add_tail(&new->list, new_cmds);
	}
}

bool nft_cmd_collapse_elems(enum cmd_ops op, struct list_head *cmds,
			    struct handle *handle, struct expr *init)
{
	struct cmd *last_cmd;

	if (list_empty(cmds))
		return false;

	if (init->etype == EXPR_VARIABLE)
		return false;

	last_cmd = list_last_entry(cmds, struct cmd, list);
	if (last_cmd->op != op ||
	    last_cmd->obj != CMD_OBJ_ELEMENTS ||
	    last_cmd->expr->etype == EXPR_VARIABLE ||
	    last_cmd->handle.family != handle->family ||
	    strcmp(last_cmd->handle.table.name, handle->table.name) ||
	    strcmp(last_cmd->handle.set.name, handle->set.name))
		return false;

	list_splice_tail_init(&expr_set(init)->expressions,
			      &expr_set(last_cmd->expr)->expressions);
	expr_set(last_cmd->expr)->size += expr_set(init)->size;

	return true;
}

void nft_cmd_expand(struct cmd *cmd)
{
	struct list_head new_cmds;
	struct flowtable *ft;
	struct table *table;
	struct chain *chain;
	struct set *set;
	struct obj *obj;
	struct cmd *new;
	struct handle h;

	init_list_head(&new_cmds);

	switch (cmd->obj) {
	case CMD_OBJ_TABLE:
		table = cmd->table;
		if (!table)
			return;

		list_for_each_entry(chain, &table->chains, list) {
			handle_merge(&chain->handle, &table->handle);
			memset(&h, 0, sizeof(h));
			handle_merge(&h, &chain->handle);
			h.chain_id = chain->handle.chain_id;
			new = cmd_alloc(CMD_ADD, CMD_OBJ_CHAIN, &h,
					&chain->location, chain_get(chain));
			list_add_tail(&new->list, &new_cmds);
		}
		list_for_each_entry(obj, &table->objs, list) {
			handle_merge(&obj->handle, &table->handle);
			memset(&h, 0, sizeof(h));
			handle_merge(&h, &obj->handle);
			new = cmd_alloc(CMD_ADD, obj_type_to_cmd(obj->type), &h,
					&obj->location, obj_get(obj));
			list_add_tail(&new->list, &new_cmds);
		}
		list_for_each_entry(set, &table->sets, list) {
			handle_merge(&set->handle, &table->handle);
			memset(&h, 0, sizeof(h));
			handle_merge(&h, &set->handle);
			new = cmd_alloc(CMD_ADD, CMD_OBJ_SET, &h,
					&set->location, set_get(set));
			list_add_tail(&new->list, &new_cmds);
		}
		list_for_each_entry(ft, &table->flowtables, list) {
			handle_merge(&ft->handle, &table->handle);
			memset(&h, 0, sizeof(h));
			handle_merge(&h, &ft->handle);
			new = cmd_alloc(CMD_ADD, CMD_OBJ_FLOWTABLE, &h,
					&ft->location, flowtable_get(ft));
			list_add_tail(&new->list, &new_cmds);
		}
		list_for_each_entry(chain, &table->chains, list)
			nft_cmd_expand_chain(chain, &new_cmds);

		list_splice(&new_cmds, &cmd->list);
		break;
	case CMD_OBJ_CHAIN:
		chain = cmd->chain;
		if (!chain || list_empty(&chain->rules))
			break;

		nft_cmd_expand_chain(chain, &new_cmds);
		list_splice(&new_cmds, &cmd->list);
		break;
	case CMD_OBJ_SET:
	case CMD_OBJ_MAP:
		set = cmd->set;
		if (!set->init)
			break;

		memset(&h, 0, sizeof(h));
		handle_merge(&h, &set->handle);
		new = cmd_alloc(CMD_ADD, CMD_OBJ_SETELEMS, &h,
				&set->location, set_get(set));
		list_add(&new->list, &cmd->list);
		break;
	default:
		break;
	}
}
