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

#include <stdlib.h>
#include <string.h>
#include "nft.h"
#include "nft-cmd.h"

struct nft_cmd *nft_cmd_new(struct nft_handle *h, int command,
			    const char *table, const char *chain,
			    struct iptables_command_state *state,
			    int rulenum, bool verbose)
{
	struct nftnl_rule *rule;
	struct nft_cmd *cmd;

	cmd = calloc(1, sizeof(struct nft_cmd));
	if (!cmd)
		return NULL;

	cmd->command = command;
	cmd->table = strdup(table);
	if (chain)
		cmd->chain = strdup(chain);
	cmd->rulenum = rulenum;
	cmd->verbose = verbose;

	if (state) {
		rule = nft_rule_new(h, chain, table, state);
		if (!rule)
			return NULL;

		cmd->obj.rule = rule;

		if (!state->target && strlen(state->jumpto) > 0)
			cmd->jumpto = strdup(state->jumpto);
	}

	list_add_tail(&cmd->head, &h->cmd_list);

	return cmd;
}

void nft_cmd_free(struct nft_cmd *cmd)
{
	free((void *)cmd->table);
	free((void *)cmd->chain);
	free((void *)cmd->policy);
	free((void *)cmd->rename);
	free((void *)cmd->jumpto);

	switch (cmd->command) {
	case NFT_COMPAT_RULE_CHECK:
	case NFT_COMPAT_RULE_DELETE:
		if (cmd->obj.rule)
			nftnl_rule_free(cmd->obj.rule);
		break;
	default:
		break;
	}

	list_del(&cmd->head);
	free(cmd);
}

static void nft_cmd_rule_bridge(struct nft_handle *h, const struct nft_cmd *cmd)
{
	const struct builtin_table *t;

	t = nft_table_builtin_find(h, cmd->table);
	if (!t)
		return;

	/* Since ebtables user-defined chain policies are implemented as last
	 * rule in nftables, rule cache is required here to treat them right.
	 */
	if (h->family == NFPROTO_BRIDGE &&
	    !nft_chain_builtin_find(t, cmd->chain))
		nft_cache_level_set(h, NFT_CL_RULES, cmd);
	else
		nft_cache_level_set(h, NFT_CL_CHAINS, cmd);
}

int nft_cmd_rule_append(struct nft_handle *h, const char *chain,
			const char *table, struct iptables_command_state *state,
			void *ref, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_APPEND, table, chain, state, -1,
			  verbose);
	if (!cmd)
		return 0;

	nft_cmd_rule_bridge(h, cmd);

	return 1;
}

int nft_cmd_rule_insert(struct nft_handle *h, const char *chain,
			const char *table, struct iptables_command_state *state,
			int rulenum, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_INSERT, table, chain, state,
			  rulenum, verbose);
	if (!cmd)
		return 0;

	nft_cmd_rule_bridge(h, cmd);

	if (cmd->rulenum > 0)
		nft_cache_level_set(h, NFT_CL_RULES, cmd);
	else
		nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_rule_delete(struct nft_handle *h, const char *chain,
			const char *table, struct iptables_command_state *state,
			bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_DELETE, table, chain, state,
			  -1, verbose);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int nft_cmd_rule_delete_num(struct nft_handle *h, const char *chain,
			    const char *table, int rulenum, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_DELETE, table, chain, NULL,
			  rulenum, verbose);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int nft_cmd_rule_flush(struct nft_handle *h, const char *chain,
		       const char *table, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_FLUSH, table, chain, NULL, -1,
			  verbose);
	if (!cmd)
		return 0;

	if (chain || verbose)
		nft_cache_level_set(h, NFT_CL_CHAINS, cmd);
	else
		nft_cache_level_set(h, NFT_CL_TABLES, cmd);

	return 1;
}

int nft_cmd_chain_zero_counters(struct nft_handle *h, const char *chain,
				const char *table, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_CHAIN_ZERO, table, chain, NULL, -1,
			  verbose);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_chain_user_add(struct nft_handle *h, const char *chain,
			   const char *table)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_CHAIN_USER_ADD, table, chain, NULL, -1,
			  false);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_chain_user_del(struct nft_handle *h, const char *chain,
			   const char *table, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_CHAIN_USER_DEL, table, chain, NULL, -1,
			  verbose);
	if (!cmd)
		return 0;

	/* This triggers nft_bridge_chain_postprocess() when fetching the
	 * rule cache.
	 */
	if (h->family == NFPROTO_BRIDGE)
		nft_cache_level_set(h, NFT_CL_RULES, cmd);
	else
		nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_chain_user_rename(struct nft_handle *h,const char *chain,
			      const char *table, const char *newname)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_CHAIN_RENAME, table, chain, NULL, -1,
			  false);
	if (!cmd)
		return 0;

	cmd->rename = strdup(newname);

	nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_rule_list(struct nft_handle *h, const char *chain,
		      const char *table, int rulenum, unsigned int format)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_LIST, table, chain, NULL, rulenum,
			  false);
	if (!cmd)
		return 0;

	cmd->format = format;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int nft_cmd_rule_replace(struct nft_handle *h, const char *chain,
			 const char *table, void *data, int rulenum,
			 bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_REPLACE, table, chain, data,
			  rulenum, verbose);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int nft_cmd_rule_check(struct nft_handle *h, const char *chain,
		       const char *table, void *data, bool verbose)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_CHECK, table, chain, data, -1,
			  verbose);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int nft_cmd_chain_set(struct nft_handle *h, const char *table,
		      const char *chain, const char *policy,
		      const struct xt_counters *counters)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_CHAIN_UPDATE, table, chain, NULL, -1,
			  false);
	if (!cmd)
		return 0;

	cmd->policy = strdup(policy);
	if (counters)
		cmd->counters = *counters;

	nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_table_flush(struct nft_handle *h, const char *table)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_TABLE_FLUSH, table, NULL, NULL, -1,
			  false);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_TABLES, cmd);

	return 1;
}

int nft_cmd_chain_restore(struct nft_handle *h, const char *chain,
			  const char *table)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_CHAIN_RESTORE, table, chain, NULL, -1,
			  false);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_CHAINS, cmd);

	return 1;
}

int nft_cmd_rule_zero_counters(struct nft_handle *h, const char *chain,
			       const char *table, int rulenum)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_ZERO, table, chain, NULL, rulenum,
			  false);
	if (!cmd)
		return 0;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int nft_cmd_rule_list_save(struct nft_handle *h, const char *chain,
			   const char *table, int rulenum, int counters)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_RULE_SAVE, table, chain, NULL, rulenum,
			  false);
	if (!cmd)
		return 0;

	cmd->counters_save = counters;

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

int ebt_cmd_user_chain_policy(struct nft_handle *h, const char *table,
                              const char *chain, const char *policy)
{
	struct nft_cmd *cmd;

	cmd = nft_cmd_new(h, NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE, table, chain,
			  NULL, -1, false);
	if (!cmd)
		return 0;

	cmd->policy = strdup(policy);

	nft_cache_level_set(h, NFT_CL_RULES, cmd);

	return 1;
}

void nft_cmd_table_new(struct nft_handle *h, const char *table)
{
	nft_cmd_new(h, NFT_COMPAT_TABLE_NEW, table, NULL, NULL, -1, false);
}
