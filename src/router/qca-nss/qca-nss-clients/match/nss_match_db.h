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

#ifndef __NSS_MATCH_DB_H
#define __NSS_MATCH_DB_H

#include "nss_match_stats.h"
#include "nss_match_cmd.h"

/*
 * nss_match_rule_info
 *	Rule information.
 */
struct nss_match_rule_info {
	union {
		struct nss_match_rule_vow_msg vow;
		struct nss_match_rule_l2_msg l2;
	} profile;
	bool valid_rule;
};

/*
 * nss_match_instance
 *	Match instance information.
 */
struct nss_match_instance {
	struct nss_match_rule_info rules[NSS_MATCH_INSTANCE_RULE_MAX];
	struct nss_match_stats stats;
	struct match_profile_ops *ops;
	uint32_t valid_mask_flag;
	uint32_t maskset[NSS_MATCH_MASK_MAX][NSS_MATCH_MASK_WORDS_MAX];	/* Maskset. */
	bool valid_rule_mask[NSS_MATCH_MASK_MAX][NSS_MATCH_INSTANCE_RULE_MAX];
	uint32_t profile_type;
	uint32_t if_num;
	uint16_t rule_count;
	bool is_configured;
};

/*
 * nss_match_db
 * 	Structure to store all match instance information.
 */
struct nss_match_db {
	struct nss_match_instance *instance[NSS_MATCH_INSTANCE_MAX];		/* Pointer to each match instance database. */
	spinlock_t db_lock;		/* Spin lock to protect database. */
	int8_t instance_count;		/* Match instance count. */
};

/*
 * match_profile_ops
 * 	Operations to perform on match db.
 */
struct match_profile_ops {

	/* Check if rule exists already in database. */
	bool (*nss_match_rule_find)(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule);

	/* Add match rule into database. */
	bool (*nss_match_rule_add)(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule);

	/* Get match rules details from database. */
	bool (*nss_match_rule_read)(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule, uint16_t rule_id);

	/* Read table details. */
	size_t (*nss_match_table_read)(struct nss_match_instance *db_instance, size_t buflen, char *bufp);

	/* Parse command line arguements. */
	int (*nss_match_cmd_parse)(char *input_msg, struct nss_match_msg *rule_msg, nss_match_cmd_t type);
};

int nss_match_table_count_get(void);
int nss_match_db_rule_count_get(uint32_t table_id);
int nss_match_get_table_id_by_ifnum(int if_num);
bool nss_match_db_table_validate(int table_id);
bool nss_match_db_get_profile_type(uint32_t table_id, uint32_t *profile_type);
bool nss_match_db_table_destroy(int table_id);
int nss_match_db_table_create(int if_num);
bool nss_match_db_mask_add(struct nss_match_profile_configure_msg *config_msg, uint8_t table_id);
bool nss_match_db_instance_config_get(struct nss_match_profile_configure_msg *config_msg, int *if_num, uint8_t table_id);
bool nss_match_db_instance_enable(uint32_t table_id);
bool nss_match_db_instance_disable(uint32_t table_id);
int nss_match_db_instance_count_get(void);
bool nss_match_db_profile_type_add(uint32_t profile_type, uint8_t table_id);
int nss_match_db_generate_rule_id(uint32_t table_id);
bool nss_match_db_rule_find(struct nss_match_rule_info *rule, uint32_t table_id);
bool nss_match_db_rule_add(struct nss_match_rule_info *rule, uint8_t table_id);
bool nss_match_db_rule_delete(uint32_t table_id, uint32_t rule_id);
bool nss_match_db_rule_read(struct nss_match_rule_info *rule, uint32_t table_id, uint16_t rule_id);
int nss_match_db_parse_cmd(uint32_t table_id, char *input_msg, struct nss_match_msg *rule_msg, nss_match_cmd_t type);
void nss_match_stats_table_sync(struct nss_ctx_instance *nss_ctx, struct nss_match_stats_sync *stats_msg, uint16_t if_num);
size_t nss_match_db_table_read(uint32_t table_id, size_t buflen, char *bufp);
bool nss_match_db_stats_get(uint32_t table_id, struct nss_match_stats *match_stats);
void nss_match_db_init(void);
bool nss_match_profile_ops_register(uint32_t type, struct match_profile_ops *mops);
int nss_match_db_get_ifnum_by_table_id(uint32_t table_id);

#endif /* __NSS_MATCH_DB_H */
