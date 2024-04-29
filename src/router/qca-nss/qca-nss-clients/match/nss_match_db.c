/*
 *******************************************************************************
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
 ********************************************************************************
 **/
#include "nss_match_db.h"
#include "nss_match_priv.h"
#include "nss_match_user.h"

struct nss_match_db match_db; /* Match DB */
struct match_profile_ops *profile_ops[NSS_MATCH_PROFILE_TYPE_MAX];

/*
 * nss_match_db_get_instance_by_table_id()
 *	Get match instance.
 */
static struct nss_match_instance *nss_match_db_get_instance_by_table_id(uint32_t table_id)
{
	assert_spin_locked(&match_db.db_lock);

	/*
	 * This assumes table_id is 0 based actual array index.
	 */
	if ((table_id >= NSS_MATCH_INSTANCE_MAX) || (!match_db.instance[table_id])) {
		nss_match_warn("Invalid table index: %d\n", table_id+1);
		return NULL;
	}

	return match_db.instance[table_id];
}

int nss_match_table_count_get(void)
{
	int index, count = 0;

	spin_lock_bh(&match_db.db_lock);

	for (index = 0; index < NSS_MATCH_INSTANCE_MAX;	index++) {
		if ((match_db.instance[index]) && (match_db.instance[index]->is_configured)) {
			count++;
		}
	}

	spin_unlock_bh(&match_db.db_lock);
	return count;
}

/*
 * nss_match_db_stats_get()
 * 	Get stats value to print.
 */
bool nss_match_db_stats_get(uint32_t table_id, struct nss_match_stats *match_stats)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);
	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	memcpy(match_stats, &db_instance->stats, sizeof(struct nss_match_stats));
	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_table_read()
 * 	Read match table information, including profile information and rules data.
 */
size_t nss_match_db_table_read(uint32_t table_id, size_t buflen, char *bufp)
{
	struct nss_match_instance *db_instance;
	size_t wr_len = 0;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return wr_len;
	}

	wr_len = db_instance->ops->nss_match_table_read(db_instance, buflen, bufp);
	spin_unlock_bh(&match_db.db_lock);
	return wr_len;
}

/*
 * nss_match_db_generate_rule_id()
 * 	Generates rule id for match rule.
 */
int nss_match_db_generate_rule_id(uint32_t table_id)
{
	int rule_id;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);
	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return -1;
	}

	/*
	 * Find an unused slot in the rule table.
	 */
	for (rule_id = 0; rule_id < NSS_MATCH_INSTANCE_RULE_MAX; rule_id++) {
		if (db_instance->rules[rule_id].valid_rule) {
			continue;
		}

		spin_unlock_bh(&match_db.db_lock);
		return (rule_id + 1);
	}

	nss_match_warn("Rule table full with NSS_MATCH_INSTANCE_RULE_MAX:%d entries.\n",
			NSS_MATCH_INSTANCE_RULE_MAX);

	spin_unlock_bh(&match_db.db_lock);
	return -1;
}

/*
 * nss_match_db_rule_find()
 * 	Finds if rule exists in db.
 */
bool nss_match_db_rule_find(struct nss_match_rule_info *rule, uint32_t table_id)
{
	bool result;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);
	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return true;
	}

	result = db_instance->ops->nss_match_rule_find(db_instance, rule);
	spin_unlock_bh(&match_db.db_lock);
	return result;
}

/*
 * nss_match_db_rule_add()
 * 	Adds the match rule in match db.
 */
bool nss_match_db_rule_add(struct nss_match_rule_info *rule, uint8_t table_id)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	if (!db_instance->ops->nss_match_rule_add(db_instance, rule)) {
		nss_match_warn("Unable to add rule to match database.\n");
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_rule_delete()
 * 	Deletes the match rule from match db.
 */
bool nss_match_db_rule_delete(uint32_t table_id, uint32_t rule_id)
{
	struct nss_match_instance *db_instance;
	int index;

	if (rule_id == 0 || rule_id > NSS_MATCH_INSTANCE_RULE_MAX) {
		nss_match_warn("Invalid rule ID: %d\n", rule_id);
		return false;
	}

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	if (!(db_instance->rules[rule_id - 1].valid_rule)) {
		nss_match_warn("Rule dosn't exist for rule id: %d\n", rule_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	db_instance->rule_count--;
	db_instance->stats.hit_count[rule_id - 1] = 0;

	for (index = 0; index < NSS_MATCH_MASK_MAX; index++) {
		db_instance->valid_rule_mask[index][rule_id - 1] = false;
	}
	memset(&(db_instance->rules[rule_id - 1]), 0, sizeof(struct nss_match_rule_info));

	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_rule_read()
 * 	Gets the match rule from match db.
 */
bool nss_match_db_rule_read(struct nss_match_rule_info *rule, uint32_t table_id, uint16_t rule_id)
{
	bool res;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	res = db_instance->ops->nss_match_rule_read(db_instance, rule, rule_id);
	spin_unlock_bh(&match_db.db_lock);
	return res;
}

/*
 * nss_match_db_parse_cmd()
 * 	Parses match commands and fills 'rule_msg' accordingly with the information.
 */
int nss_match_db_parse_cmd(uint32_t table_id, char *input_msg, struct nss_match_msg *rule_msg, nss_match_cmd_t type)
{
	int res;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return -1;
	}

	spin_unlock_bh(&match_db.db_lock);

	res = db_instance->ops->nss_match_cmd_parse(input_msg, rule_msg, type);
	return res;
}

/*
 * nss_match_db_table_validate()
 * 	Check if table is configured.
 */
bool nss_match_db_table_validate(int table_id)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	if (!db_instance->is_configured) {
		nss_match_warn("Table is not configured, table_id %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_get_profile_type()
 *	Returns the table index and updates profile type.
 */
bool nss_match_db_get_profile_type(uint32_t table_id, uint32_t *profile_type)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	*profile_type = db_instance->profile_type;

	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_get_ifnum_by_table_id()
 *	Returns interface number using table ID.
 */
int nss_match_db_get_ifnum_by_table_id(uint32_t table_id)
{
	int if_num;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return -1;
	}

	if_num = db_instance->if_num;
	spin_unlock_bh(&match_db.db_lock);
	return if_num;
}

/*
 * nss_match_get_table_id_by_ifnum()
 * 	Returns table index using interface number.
 *
 */
int nss_match_get_table_id_by_ifnum(int if_num)
{
	int index;

	spin_lock_bh(&match_db.db_lock);

	for (index = 0; index < NSS_MATCH_INSTANCE_MAX; index++) {
		if (!match_db.instance[index] || match_db.instance[index]->if_num != if_num) {
			continue;
		}

		spin_unlock_bh(&match_db.db_lock);
		return index + 1;
	}

	spin_unlock_bh(&match_db.db_lock);
	return -1;
}

/*
 * nss_match_db_table_destroy()
 *	Destroys table information from DB.
 */
bool nss_match_db_table_destroy(int table_id)
{
	/*
	 * TODO: Add clear table API.
	 */
	struct nss_match_instance *table_info;

	if (table_id <= 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		return false;
	}

	spin_lock_bh(&match_db.db_lock);

	table_info = match_db.instance[table_id - 1];
	if (!table_info) {
		nss_match_warn("Invalid table index: %d, table doesn't exist.\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	match_db.instance_count--;
	match_db.instance[table_id - 1] = NULL;

	spin_unlock_bh(&match_db.db_lock);
	kfree(table_info);
	return true;
}

/*
 * nss_match_db_table_create()
 * 	Create an instance in the DB for the new table.
 */
int nss_match_db_table_create(int if_num)
{
	int index, table_id = -1;
	struct nss_match_instance *mi;

	mi = (struct nss_match_instance *) kzalloc(sizeof(struct nss_match_instance), GFP_KERNEL);
	if (!mi) {
		nss_match_warn("Unable to allocate memory for new table.\n");
		return -1;
	}

	spin_lock_bh(&match_db.db_lock);
	for (index = 0; index < NSS_MATCH_INSTANCE_MAX; index++) {
		if (match_db.instance[index]) {
			continue;
		}

		match_db.instance[index] = mi;
		match_db.instance_count++;
		match_db.instance[index]->profile_type = 0;
		match_db.instance[index]->maskset[0][0] = 0;
		match_db.instance[index]->maskset[1][0] = 0;
		match_db.instance[index]->if_num = if_num;
		match_db.instance[index]->is_configured = false;
		match_db.instance[index]->valid_mask_flag = 0;
		table_id = index + 1;
		break;
	}

	spin_unlock_bh(&match_db.db_lock);
	return table_id;
}

/*
 * nss_match_db_profile_type_add()
 * 	Add profile type to match instance.
 */
bool nss_match_db_profile_type_add(uint32_t profile_type, uint8_t table_id)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	db_instance->profile_type = profile_type;
	db_instance->ops = profile_ops[profile_type];
	nss_match_info("Added ops %px for profile type: %d", db_instance->ops, profile_type);
	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_mask_add()
 * 	Add a mask to the profile.
 */
bool nss_match_db_mask_add(struct nss_match_profile_configure_msg *config_msg, uint8_t table_id)
{
	int valid, index;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	for (valid = 0; valid < NSS_MATCH_MASK_MAX; valid++) {
		if (config_msg->valid_mask_flag & (1 << valid)) {
			for (index = 0; index < NSS_MATCH_MASK_WORDS_MAX; index++) {
				db_instance->maskset[valid][index] = config_msg->maskset[valid][index];
			}
		}
	}
	db_instance->valid_mask_flag |= config_msg->valid_mask_flag;
	spin_unlock_bh(&match_db.db_lock);
	return true;
}

/*
 * nss_match_db_instance_config_get()
 * 	Get the profile config given the table ID.
 */
bool nss_match_db_instance_config_get(struct nss_match_profile_configure_msg *config_msg, int *if_num, uint8_t table_id)
{
	int valid, index;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	config_msg->profile_type = db_instance->profile_type;
	if (!(db_instance->valid_mask_flag)) {
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	config_msg->valid_mask_flag = db_instance->valid_mask_flag;

	for (valid = 0; valid < NSS_MATCH_MASK_MAX; valid++) {
		if (db_instance->valid_mask_flag & (1 << valid)) {
			for (index = 0; index < NSS_MATCH_MASK_WORDS_MAX; index++) {
				config_msg->maskset[valid][index] = db_instance->maskset[valid][index];
			}
		}
	}

	*if_num = db_instance->if_num;

	spin_unlock_bh(&match_db.db_lock);

	return true;
}

/*
 * nss_match_db_instance_count_get()
 * 	Get total number of existing match instance.
 */
int nss_match_db_instance_count_get(void)
{
	int count;

	spin_lock_bh(&match_db.db_lock);
	count = match_db.instance_count;
	spin_unlock_bh(&match_db.db_lock);

	return count;
}

/*
 * nss_match_db_rule_count_get()
 * 	Get total number of rule pr instance.
 */
int nss_match_db_rule_count_get(uint32_t table_id)
{
	int count;
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return -1;
	}

	count = db_instance->rule_count;
	spin_unlock_bh(&match_db.db_lock);
	return count;
}

/*
 * nss_match_stats_table_sync()
 *	Debug stats sync for match.
 */
void nss_match_stats_table_sync(struct nss_ctx_instance *nss_ctx, struct nss_match_stats_sync *stats_msg, uint16_t if_num)
{
	int index, table_id;
	struct nss_match_instance *db_instance;

	table_id = nss_match_get_table_id_by_ifnum(if_num);
	if (table_id <= 0 || table_id > NSS_MATCH_INSTANCE_MAX) {
		nss_match_warn("Invalid table id: %d\n", table_id);
		return;
	}

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid if_num: %d, table index: %d, failed to get DB instance. \n", if_num, table_id);
		spin_unlock_bh(&match_db.db_lock);
		return;
	}

	db_instance->stats.pstats.rx_packets += stats_msg->p_stats.rx_packets;
	db_instance->stats.pstats.rx_bytes += stats_msg->p_stats.rx_bytes;
	db_instance->stats.pstats.tx_packets += stats_msg->p_stats.tx_packets;
	db_instance->stats.pstats.tx_bytes += stats_msg->p_stats.tx_bytes;

	for (index = 0; index < NSS_MAX_NUM_PRI; index++) {
		db_instance->stats.pstats.rx_dropped[index] += stats_msg->p_stats.rx_dropped[index];
	}

	for (index = 0; index < NSS_MATCH_INSTANCE_RULE_MAX; index++) {
		/*
		 * Avoid sync for invalid rules.
		 */
		if (!db_instance->rules[index].valid_rule) {
			continue;
		}
		db_instance->stats.hit_count[index] += stats_msg->hit_count[index];
	}

	spin_unlock_bh(&match_db.db_lock);
}

/*
 * nss_match_db_instance_disable
 * 	Disable the match instance.
 */
bool nss_match_db_instance_disable(uint32_t table_id)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	db_instance->is_configured = false;
	spin_unlock_bh(&match_db.db_lock);

	return true;
}

/*
 * nss_match_db_instance_enable()
 * 	Enable the match instance.
 */
bool nss_match_db_instance_enable(uint32_t table_id)
{
	struct nss_match_instance *db_instance;

	spin_lock_bh(&match_db.db_lock);

	db_instance = nss_match_db_get_instance_by_table_id(table_id - 1);
	if (!db_instance) {
		nss_match_warn("Invalid table index, failed to get DB instance: %d\n", table_id);
		spin_unlock_bh(&match_db.db_lock);
		return false;
	}

	db_instance->is_configured = true;
	spin_unlock_bh(&match_db.db_lock);

	return true;
}

/*
 * nss_match_profile_ops_register()
 * 	Registers match ops according to profile type.
 */
bool nss_match_profile_ops_register(uint32_t type, struct match_profile_ops *mops)
{
	if (type >= NSS_MATCH_PROFILE_TYPE_MAX) {
		nss_match_warn("Invalid profile type: %d", type);
		return false;
	}

	profile_ops[type] = mops;
	nss_match_info("Match ops added for profile type: %d", type);

	return true;
}

/*
 * nss_match_db_init()
 * 	Initializes DB.
 */
void nss_match_db_init(void)
{
	match_db.instance_count = 0;
	spin_lock_init(&match_db.db_lock);
	nss_match_info("db init successful.\n");
}
