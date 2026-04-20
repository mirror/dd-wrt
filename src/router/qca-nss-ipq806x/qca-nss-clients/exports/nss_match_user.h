/*
 **************************************************************************
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
 **************************************************************************
 */

/**
 * nss_match_get_ifnum_by_table_index
 * 	Returns match interface number given table index.
 *
 * @param[in] table_id  Table ID of match instance.
 *
 * @return
 * Interface number of match instance, return -1 on failure.
 */
extern int nss_match_get_ifnum_by_table_id(uint32_t table_id);

/**
 * nss_match_vow_rule_add
 * 	Add the VoW rule to the configured match instance.
 *
 * @datatype
 * nss_ctx_instance \n
 * nss_match_rule_vow_msg
 *
 * @param[in] nss_ctx   Pointer to NSS core context.
 * @param[in] rule_msg  Pointer to VoW rule message.
 * @param[in] table_id  Table ID of match instance.
 *
 * @return
 * Rule ID of rule added, returns -1 on failure.
 */
extern int nss_match_vow_rule_add(struct nss_ctx_instance *nss_ctx, struct nss_match_rule_vow_msg *rule_msg, uint32_t table_id);

/**
 * nss_match_l2_rule_add
 * 	Add the l2 rule to the configured match instance.
 *
 * @datatype
 * nss_ctx_instance \n
 * nss_match_rule_l2_msg
 *
 * @param[in] nss_ctx   Pointer to NSS core context.
 * @param[in] rule_msg  Pointer to L2 rule message.
 * @param[in] table_id  Table ID of match instance.
 *
 * @return
 * Rule ID of rule added, returns -1 on failure.
 */
extern int nss_match_l2_rule_add(struct nss_ctx_instance *nss_ctx, struct nss_match_rule_l2_msg *rule_msg, uint32_t table_id);

/**
 * nss_match_rule_delete
 * 	Deletes the rule from match instance.
 *
 * @datatype
 * nss_ctx_instance
 *
 * @param[in] nss_ctx  Pointer to NSS core context.
 * @param[in] rule_id  Rule ID of match rule.
 * @param[in] table_id Table ID of match instance.
 *
 * @return
 * Error status of rule deletion.
 */
extern nss_match_status_t nss_match_rule_delete(struct nss_ctx_instance *nss_ctx, uint32_t rule_id, uint32_t table_id);

/**
 * nss_match_profile_configure
 * 	Configures the match instance.
 *
 * @datatype
 * nss_ctx_instance \n
 * nss_match_profile_configure_msg
 *
 * @param[in] nss_ctx    Pointer to NSS core context.
 * @param[in] config_msg Match message for configuration.
 * @param[in] table_id   Table ID of match instance.
 *
 * @return
 * Error status of configuration.
 */
extern nss_match_status_t nss_match_profile_configure(struct nss_ctx_instance *nss_ctx, struct nss_match_profile_configure_msg *config_msg, uint32_t table_id);

/**
 * nss_match_instance_create
 * 	Creates the match instance.
 *
 * @return
 * Table ID of match instance created, returns -1 on failure.
 */
extern int32_t nss_match_instance_create(void);

/**
 * nss_match_instance_destroy
 * 	Deletes the match instance.
 *
 * @param[in] table_id  Table ID of match instance.
 *
 * @return
 * Error status of deletion.
 */
extern nss_match_status_t nss_match_instance_destroy(uint32_t table_id);
