/*
 ***************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ***************************************************************************
 */

/*
 * nss_match.c
 */

#include "nss_match_cmd.h"
#include "nss_match_db.h"
#include "nss_match_vow.h"
#include "nss_match_l2.h"
#include "nss_match_priv.h"
#include <linux/types.h>
#include <nss_api_if.h>
#include <linux/debugfs.h>
#include <linux/of.h>

/*
 * nss_match_verify_config_msg()
 * 	Verify configuration message.
 */
static nss_match_status_t nss_match_verify_config_msg(struct nss_match_profile_configure_msg *config_msg)
{
	int mask, valid_mask_word;
	int index, invalid_word_count = 0;

	switch (config_msg->profile_type) {
	case NSS_MATCH_PROFILE_TYPE_VOW:
		valid_mask_word = 1;
		break;
	case NSS_MATCH_PROFILE_TYPE_L2:
		valid_mask_word = 4;
		break;
	default:
		nss_match_warn("Invalid profile type: %d.\n", config_msg->profile_type);
		return NSS_MATCH_ERROR_PROFILE_CONFIG_INVALID;
	}

	for (mask = 0; mask < NSS_MATCH_MASK_MAX; mask++) {
		if ((config_msg->valid_mask_flag) & (1 << mask)) {
			for (index = 0; index < valid_mask_word; index++) {
				if (!config_msg->maskset[mask][index]) {
					invalid_word_count++;
				}
			}

			/*
			 * If all words of mask is null.
			 */
			if (invalid_word_count == valid_mask_word) {
				nss_match_warn("Invalid mask for valid_mask_flag: %d\n", config_msg->valid_mask_flag);
				return NSS_MATCH_ERROR_PROFILE_CONFIG_INVALID;
			}
		}
	}

	return NSS_MATCH_SUCCESS;
}

/*
 * nss_match_sync_callback()
 * 	Sync callback for syncing stats.
 */
static void nss_match_sync_callback(void *app_data, struct nss_match_msg *nmm)
{
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();

	switch (nmm->cm.type) {
	case NSS_MATCH_STATS_SYNC:
		nss_match_stats_table_sync(nss_ctx, &nmm->msg.stats, nmm->cm.interface);
		return;
	default:
		nss_match_warn("%px: Unknown Event from NSS", nmm);
		return;
	}
}

/*
 * nss_match_rule_delete()
 * 	User API to delete the match rule.
 */
nss_match_status_t nss_match_rule_delete(struct nss_ctx_instance *nss_ctx, uint32_t rule_id, uint32_t table_id)
{
	struct nss_match_msg matchm;
	struct nss_match_rule_info rule;
	uint32_t profile_type, len;
	int if_num;
	enum nss_match_msg_types type;
	nss_tx_status_t nss_tx_status;

	if ((rule_id == 0) || (rule_id > NSS_MATCH_INSTANCE_RULE_MAX)) {
		nss_match_warn("%px: rule_id doesnot exist, rule_id = %d", nss_ctx, rule_id);
		return NSS_MATCH_ERROR_RULE_ID_OUTOFBOUND;
	}

	if (!nss_match_db_table_validate(table_id)) {
		nss_match_warn("%px: Invalid table_id %d", nss_ctx, table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	if_num = nss_match_get_ifnum_by_table_id(table_id);
	if (if_num < 0) {
		nss_match_warn("%px: Invalid table_id %d", nss_ctx, table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	/*
	 * Read the rule information
	 */
	if (!nss_match_db_rule_read(&rule, table_id, rule_id)) {
		nss_match_warn("%px: rule_id does not exist, rule_id = %d", nss_ctx, rule_id);
		return NSS_MATCH_ERROR_RULE_ID_OUTOFBOUND;
	}

	nss_match_db_get_profile_type(table_id, &profile_type);

	switch (profile_type) {
	case NSS_MATCH_PROFILE_TYPE_VOW:
		type = NSS_MATCH_DELETE_VOW_RULE_MSG;
		len = sizeof(struct nss_match_rule_vow_msg);
		matchm.msg.vow_rule = rule.profile.vow;
		break;
	case NSS_MATCH_PROFILE_TYPE_L2:
		type = NSS_MATCH_DELETE_L2_RULE_MSG;
		len = sizeof(struct nss_match_rule_l2_msg);
		matchm.msg.l2_rule = rule.profile.l2;
		break;
	default:
		nss_match_warn("%px: Unknown profile type: %d", nss_ctx, profile_type);
		return NSS_MATCH_ERROR_UNKNOWN_MSG;
	}

	nss_match_msg_init(&matchm, if_num, type, len, NULL, NULL);

	nss_tx_status = nss_match_msg_tx_sync(nss_ctx, &matchm);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		nss_match_warn("%px: Sending delete rule failed, rule_id = %d, status = %d", nss_ctx, rule_id, nss_tx_status);
		return NSS_MATCH_ERROR_RULE_DELETE;
	}

	nss_match_db_rule_delete(table_id, rule_id);
	return NSS_MATCH_SUCCESS;
}
EXPORT_SYMBOL(nss_match_rule_delete);

/*
 * nss_match_get_ifnum_by_table_id()
 *	Returns interface number using table ID.
 */
int nss_match_get_ifnum_by_table_id(uint32_t table_id)
{
	if (table_id == 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
		nss_match_warn("Invalid table_id %d.\n", table_id);
		return -1;
	}

	return nss_match_db_get_ifnum_by_table_id(table_id);
}
EXPORT_SYMBOL(nss_match_get_ifnum_by_table_id);

/*
 * nss_match_vow_rule_add()
 * 	User API to add VoW rule.
 */
int nss_match_vow_rule_add(struct nss_ctx_instance *nss_ctx, struct nss_match_rule_vow_msg *rule_msg, uint32_t table_id)
{
	int rule_id = -1, if_num;
	struct nss_match_msg nmm;
	struct nss_match_rule_info rule;
	enum nss_match_msg_types type = NSS_MATCH_ADD_VOW_RULE_MSG;
	nss_tx_status_t nss_tx_status;

	if (!nss_match_db_table_validate(table_id)) {
		nss_match_warn("%px: Invalid table_id %d, table is not configured.\n", nss_ctx, table_id);
		return rule_id;
	}

	if_num = nss_match_get_ifnum_by_table_id(table_id);
	if (if_num < 0) {
		nss_match_warn("%px: Cannot add the rule, table doesnot exist", nss_ctx);
		return rule_id;
	}

	rule.profile.vow = *rule_msg;

	if (nss_match_db_rule_find(&rule, table_id)) {
		nss_match_warn("%px: Rule exists already. \n", nss_ctx);
		return -1;
	}

	rule_id = nss_match_db_generate_rule_id(table_id);
	if (rule_id <= 0 ) {
		nss_match_warn("%px: Reached limit, New rule can't be added. \n", nss_ctx);
		return -1;
	}

	rule.profile.vow.rule_id = rule_id;
	nss_match_db_rule_add(&rule, table_id);

	nss_match_msg_init(&nmm, if_num, type, sizeof(struct nss_match_rule_vow_msg), NULL, NULL);

	nmm.msg.vow_rule = rule.profile.vow;
	nmm.cm.type = type;

	/*
	 * Send the message to FW to add the VoW rule into the rule table.
	 */
	nss_tx_status = nss_match_msg_tx_sync(nss_ctx, &nmm);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		nss_match_warn("%px:add rule failed from NSS, rule_id = %d, nss_tx_status = %d\n",
				nss_ctx, rule_id, nss_tx_status);
		nss_match_db_rule_delete(table_id, rule_id);
		return -1;
	}

	return rule_id;
}
EXPORT_SYMBOL(nss_match_vow_rule_add);

/*
 * nss_match_l2_rule_add()
 * 	User API to add rule for L2 profile.
 */
int nss_match_l2_rule_add(struct nss_ctx_instance *nss_ctx, struct nss_match_rule_l2_msg *rule_msg, uint32_t table_id)
{
	int rule_id = -1, if_num;
	enum nss_match_msg_types type = NSS_MATCH_ADD_L2_RULE_MSG;
	struct nss_match_msg nmm;
	struct nss_match_rule_info rule;
	nss_tx_status_t nss_tx_status;

	if (!nss_match_db_table_validate(table_id)) {
		nss_match_warn("%px: Cannot insert rule, table: %d is not configured. \n", nss_ctx, table_id);
		return -1;
	}

	if_num = nss_match_get_ifnum_by_table_id(table_id);
	if (if_num < 0) {
		nss_match_warn("%px: Cannot add the rule, invalid table ID: %d", nss_ctx, table_id);
		return rule_id;
	}

	rule.profile.l2 = *rule_msg;

	if (nss_match_db_rule_find(&rule, table_id)) {
		nss_match_warn("%px: Rule exists already \n", nss_ctx);
		return -1;
	}

	rule_id = nss_match_db_generate_rule_id(table_id);
	if (rule_id <= 0) {
		nss_match_warn("%px: New rule can't be added, Reached limit. \n", nss_ctx);
		return -1;
	}

	rule.profile.l2.rule_id = rule_id;
	nss_match_db_rule_add(&rule, table_id);

	nss_match_msg_init(&nmm, if_num, type, sizeof(struct nss_match_rule_l2_msg), NULL, NULL);

	nmm.msg.l2_rule = rule.profile.l2;
	nmm.cm.type = type;

	/*
	 * Send the message to FW to add the L2 rule into the rule table.
	 */
	nss_tx_status = nss_match_msg_tx_sync(nss_ctx, &nmm);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		nss_match_warn("%px:add rule failed from NSS, rule_id = %d, nss_tx_status = %d\n",
				nss_ctx, rule_id, nss_tx_status);
		nss_match_db_rule_delete(table_id, rule_id);
		return -1;
	}

	return rule_id;
}
EXPORT_SYMBOL(nss_match_l2_rule_add);

/*
 * nss_match_profile_configure()
 * 	User API to configure match profile.
 */
nss_match_status_t nss_match_profile_configure(struct nss_ctx_instance *nss_ctx, struct nss_match_profile_configure_msg *config_msg, uint32_t table_id)
{
	struct nss_match_msg matchm;
	int32_t if_num;
	nss_tx_status_t nss_tx_status;
	nss_match_status_t status;

	status = nss_match_verify_config_msg(config_msg);
        if (status != NSS_MATCH_SUCCESS) {
		nss_match_warn("%px: Invalid config message.", nss_ctx);
		return status;
	}

	if ((table_id == 0) || (table_id > NSS_MATCH_INSTANCE_MAX)) {
		nss_match_warn("%px: Cannot configure table, table_id %d is not valid.\n", nss_ctx, table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	if (nss_match_db_table_validate(table_id)) {
		nss_match_warn("Table %d is already configured.\n", table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	if_num = nss_match_get_ifnum_by_table_id(table_id);
	if (if_num < 0) {
		nss_match_warn("%px: Invalid table ID: %d, if_num %d", nss_ctx, table_id, if_num);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	/*
	 * Configuring the profile will enable sync from NSS. So add the instance into the DB first.
	 */
	nss_match_db_profile_type_add(config_msg->profile_type, table_id);
	nss_match_db_mask_add(config_msg, table_id);
	nss_match_db_instance_enable(table_id);

	nss_match_msg_init(&matchm, if_num, NSS_MATCH_TABLE_CONFIGURE_MSG,
			sizeof(struct nss_match_profile_configure_msg), NULL, NULL);
	matchm.msg.configure_msg = *config_msg;

	nss_tx_status = nss_match_msg_tx_sync(nss_ctx, &matchm);
	if (nss_tx_status != NSS_TX_SUCCESS) {
		nss_match_warn("%px: Profile configuration failed for table_id = %d\n, nss_tx_status = %d\n",
				nss_ctx, if_num, nss_tx_status);
		nss_match_db_instance_disable(table_id);
		return NSS_MATCH_ERROR_INSTANCE_CONFIGURED;
	}

	return NSS_MATCH_SUCCESS;
}
EXPORT_SYMBOL(nss_match_profile_configure);

/*
 * nss_match_instance_destroy()
 * 	User API to destroy the match instance.
 */
nss_match_status_t nss_match_instance_destroy(uint32_t table_id)
{
	nss_tx_status_t status;
	int if_num;

	if (table_id == 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
		nss_match_warn("Cannot delete the rule, table = %d, does not exist", table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	if_num = nss_match_get_ifnum_by_table_id(table_id);
	if (if_num < 0) {
		nss_match_warn("Cannot delete the rule, if_num = %d, table = %d, does not exist", if_num, table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	if (!nss_match_unregister_instance(if_num)) {
		nss_match_warn("Cannot unregister the instance: %d\n", table_id);
		return NSS_MATCH_ERROR_TABLE_ID_OUTOFBOUND;
	}

	/*
	 * Dealloc the interface first, so that stats syncs are disabled.
	 */
	status = nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_MATCH);
	if (status != NSS_TX_SUCCESS) {
		nss_match_warn("Table dealloc node failure for if_num =%d\n", if_num);
		return NSS_MATCH_ERROR_TABLE_DELETE;
	}

	nss_match_db_table_destroy(table_id);
	return NSS_MATCH_SUCCESS;
}
EXPORT_SYMBOL(nss_match_instance_destroy);

/*
 * nss_match_instance_create()
 * 	User API to create the match instance.
 */
int32_t nss_match_instance_create(void)
{
	int32_t if_num, table_id;
	struct nss_ctx_instance *nss_ctx;

	if (nss_match_db_instance_count_get() == NSS_MATCH_INSTANCE_MAX) {
		nss_match_warn("Match instance limit exceeded.\n");
		return -1;
	}

	if_num = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_MATCH);
	if (if_num < 0) {
		nss_match_warn("Failed to allocate a node.\n");
		return if_num;
	}

	nss_ctx = nss_match_register_instance(if_num, nss_match_sync_callback);
	if (!nss_ctx) {
		nss_match_warn("%px, Failed to register node : %d.\n", nss_ctx, if_num);
		goto dealloc_node;
	}

	/*
	 * Create the table entry in the DB.
	 */
	table_id = nss_match_db_table_create(if_num);
	if (table_id < 0) {
		nss_match_warn("%px: Memory allocation failed for match DB array\n", nss_ctx);
		goto unregister_node;
	}

	return table_id;

unregister_node:
	nss_match_unregister_instance(if_num);

dealloc_node:
	nss_dynamic_interface_dealloc_node(if_num, NSS_DYNAMIC_INTERFACE_TYPE_MATCH);

	return -1;

}
EXPORT_SYMBOL(nss_match_instance_create);

static struct dentry *match_config;

/*
 * nss_match_exit_module()
 */
static void __exit nss_match_exit_module(void)
{
	int table, status;

	for (table = 1; table <= NSS_MATCH_INSTANCE_MAX; table++) {

		/*
		 * Skip invalid table ID.
		 */
		if (nss_match_get_ifnum_by_table_id(table) < 0) {
			continue;
		}

		status = nss_match_instance_destroy(table);
		if (status != NSS_MATCH_SUCCESS) {
			nss_match_warn("NSS match client destroy failed for table ID=%d with err=%d \n", table, status);
		}
	}

	nss_match_ctl_unregister();
	debugfs_remove_recursive(match_config);
	nss_match_info("NSS match client destroyed\n");
}

/*
 * nss_match_init_module()
 */
static int __init nss_match_init_module(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_match_db_init();

	/*
	 * Register VoW profile ops
	 */
	nss_match_vow_init();

	/*
	 * Register L2 profile ops
	 */
	nss_match_l2_init();

	/*
	 * Register command line interface for match
	 */
	if (!nss_match_ctl_register()) {
		nss_match_warn("Can't create Match directory in procfs");
		return -1;
	}

	match_config = debugfs_create_dir("match", NULL);

	if (!match_config) {
		nss_match_warn("Cannot create MATCH directory");
		nss_match_ctl_unregister();
		return -1;
	}

	/*
	 * Register stats CLI for match
	 */
	if (!nss_match_stats_debugfs_create(match_config)) {
		nss_match_warn("Cannot create MATCH node stats dentry file");
		debugfs_remove_recursive(match_config);
		nss_match_ctl_unregister();
		return -1;
	}

	nss_match_info("NSS match client initialized\n");
	return 0;
}
module_init(nss_match_init_module);
module_exit(nss_match_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS match client");
