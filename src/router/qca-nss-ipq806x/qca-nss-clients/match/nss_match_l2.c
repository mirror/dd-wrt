/*
 *******************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#define MATCH_L2_KEY0_IFNUM_SHIFT 0
#define MATCH_L2_KEY0_DMAC_HW0_SHIFT 16
#define MATCH_L2_KEY1_DMAC_HW1_SHIFT 0
#define MATCH_L2_KEY1_DMAC_HW2_SHIFT 16
#define MATCH_L2_KEY2_SMAC_HW0_SHIFT 0
#define MATCH_L2_KEY2_SMAC_HW1_SHIFT 16
#define MATCH_L2_KEY3_SMAC_HW2_SHIFT 0
#define MATCH_L2_KEY3_ETHERTYPE_SHIFT 16

/*
 * nss_match_l2_rule_find()
 *	Check if any slot is available for new entry.
 */
static bool nss_match_l2_rule_find(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule)
{
	uint16_t index;

	/*
	 * Check if entry is duplicate.
	 */
	for (index = 0; index < NSS_MATCH_INSTANCE_RULE_MAX; index++) {
		struct nss_match_rule_info rule_info = db_instance->rules[index];

		if (rule_info.valid_rule &&
			rule_info.profile.l2.if_num == rule->profile.l2.if_num &&
			rule_info.profile.l2.ethertype == rule->profile.l2.ethertype &&
			rule_info.profile.l2.smac[0] == htons(rule->profile.l2.smac[0]) &&
			rule_info.profile.l2.smac[1] == htons(rule->profile.l2.smac[1]) &&
			rule_info.profile.l2.smac[2] == htons(rule->profile.l2.smac[2]) &&
			rule_info.profile.l2.dmac[0] == htons(rule->profile.l2.dmac[0]) &&
			rule_info.profile.l2.dmac[1] == htons(rule->profile.l2.dmac[1]) &&
			rule_info.profile.l2.dmac[2] == htons(rule->profile.l2.dmac[2]) &&
			rule_info.profile.l2.mask_id == rule->profile.l2.mask_id) {
			nss_match_info("Rule matched\n");
			return true;
		}
	}

	return false;
}

/*
 * nss_match_l2_db_rule_add()
 * 	Store L2 rule information.
 */
static bool nss_match_l2_db_rule_add(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule)
{
	uint8_t rule_id = rule->profile.l2.rule_id;
	uint8_t mask_id = rule->profile.l2.mask_id;

	if (rule_id == 0 || rule_id > NSS_MATCH_INSTANCE_RULE_MAX) {
		nss_match_warn("Invalid rule id: %d\n", rule_id);
		return false;
	}

	if (db_instance->rules[rule_id - 1].valid_rule) {
		nss_match_warn("Rule exists for rule id: %d\n", rule_id);
		return false;
	}

	db_instance->rules[rule_id - 1].profile.l2.if_num =  rule->profile.l2.if_num;
	db_instance->rules[rule_id - 1].profile.l2.smac[0] = ntohs(rule->profile.l2.smac[0]);
	db_instance->rules[rule_id - 1].profile.l2.smac[1] = ntohs(rule->profile.l2.smac[1]);
	db_instance->rules[rule_id - 1].profile.l2.smac[2] = ntohs(rule->profile.l2.smac[2]);
	db_instance->rules[rule_id - 1].profile.l2.dmac[0] = ntohs(rule->profile.l2.dmac[0]);
	db_instance->rules[rule_id - 1].profile.l2.dmac[1] = ntohs(rule->profile.l2.dmac[1]);
	db_instance->rules[rule_id - 1].profile.l2.dmac[2] = ntohs(rule->profile.l2.dmac[2]);
	db_instance->rules[rule_id - 1].profile.l2.ethertype = rule->profile.l2.ethertype;
	db_instance->rules[rule_id - 1].profile.l2.mask_id = mask_id;
	db_instance->rules[rule_id - 1].profile.l2.action.action_flag = rule->profile.l2.action.action_flag;
	db_instance->rules[rule_id - 1].profile.l2.action.setprio = rule->profile.l2.action.setprio;
	db_instance->rules[rule_id - 1].profile.l2.action.forward_ifnum = rule->profile.l2.action.forward_ifnum;
	db_instance->rules[rule_id - 1].valid_rule = true;
	db_instance->rules[rule_id - 1].profile.l2.rule_id = rule_id;
	db_instance->valid_rule_mask[mask_id - 1][rule_id - 1] = true;
	db_instance->rule_count++;

	return true;
}

/*
 * nss_match_l2_rule_read()
 *	Reads rule parameters by rule id.
 */
static bool nss_match_l2_rule_read(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule, uint16_t rule_id)
{
	if (!db_instance->rules[rule_id - 1].valid_rule) {
		nss_match_warn("rule_id does not exist, rule_id = %d", rule_id);
		return false;
	}

	rule->profile.l2.if_num = db_instance->rules[rule_id - 1].profile.l2.if_num;
	rule->profile.l2.smac[0] = htons(db_instance->rules[rule_id - 1].profile.l2.smac[0]);
	rule->profile.l2.smac[1] = htons(db_instance->rules[rule_id - 1].profile.l2.smac[1]);
	rule->profile.l2.smac[2] = htons(db_instance->rules[rule_id - 1].profile.l2.smac[2]);
	rule->profile.l2.dmac[0] = htons(db_instance->rules[rule_id - 1].profile.l2.dmac[0]);
	rule->profile.l2.dmac[1] = htons(db_instance->rules[rule_id - 1].profile.l2.dmac[1]);
	rule->profile.l2.dmac[2] = htons(db_instance->rules[rule_id - 1].profile.l2.dmac[2]);
	rule->profile.l2.ethertype = db_instance->rules[rule_id - 1].profile.l2.ethertype;
	rule->profile.l2.mask_id = db_instance->rules[rule_id - 1].profile.l2.mask_id;
	rule->profile.l2.rule_id = db_instance->rules[rule_id - 1].profile.l2.rule_id;

	return true;
}

/*
 * nss_match_l2_cmd_parse()
 *	Parse the L2 command
 */
static int nss_match_l2_cmd_parse(char *input_msg, struct nss_match_msg *rule_msg, nss_match_cmd_t type)
{
	char *token, *param;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	int ret = 0;
	uint32_t mask_val[4] = {0};
	uint32_t actions = 0, if_num = 0, setprio = NSS_MAX_NUM_PRI, nexthop = 0;
	uint16_t smac[3] = {0}, dmac[3] = {0}, mask_id = 0, ethertype = 0;
	uint8_t mac_addr_tmp[6];
	char tmp[4];

	/*
	 * Parse User input.
	 */
	while (input_msg != NULL) {
		token = strsep(&input_msg, " ");
		param = strsep(&token, "=");
		if (!param || !token) {
			goto fail;
		}

		/*
		 * Parse mask ID of the message.
		 */
		if (!(strncasecmp(param, "mask", strlen("mask")))) {
			if (!sscanf(token, "%hu", &mask_id)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			if (mask_id > NSS_MATCH_MASK_MAX) {
				pr_info("%px: Maskset num %d, exceeds max allowed value %d\n", nss_ctx, mask_id, NSS_MATCH_MASK_MAX);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parse interface name of the message.
		 */
		if (!(strncasecmp(param, "ifname", strlen("ifname")))) {
			struct net_device *dev;
			if (type == NSS_MATCH_ADD_MASK) {
				if (!sscanf(token, "%x", &if_num)) {
					pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
					return -EINVAL;
				}

				continue;
			}

			if (type == NSS_MATCH_ADD_RULE) {
				dev = dev_get_by_name(&init_net, token);
				if (!dev) {
					pr_info("%px: Cannot find the net device\n", nss_ctx);
					return -ENODEV;
				}

				if_num = nss_cmn_get_interface_number_by_dev(dev);
				dev_put(dev);
				continue;
			}
		}

		/*
		 * Parse soure mac address of the message.
		 */
		if (!(strncasecmp(param, "smac", strlen("smac")))) {

			/*
			 * Parse the 48bit mask input in hex. For ex, smac=FFFF0000FFFF
			 */
			if (type == NSS_MATCH_ADD_MASK) {
				tmp[0] = token[0];
				tmp[1] = token[1];
				tmp[2] = token[2];
				tmp[3] = token[3];
				sscanf(tmp, "%hx", &smac[0]);

				tmp[0] = token[4];
				tmp[1] = token[5];
				tmp[2] = token[6];
				tmp[3] = token[7];
				sscanf(tmp, "%hx", &smac[1]);

				tmp[0] = token[8];
				tmp[1] = token[9];
				tmp[2] = token[10];
				tmp[3] = token[11];
				sscanf(tmp, "%hx", &smac[2]);
			}

			/*
			 * Parse the 6 byte MAC address delimited by ':'. For ex, smac=01:00:25:ff:45:a1
			 */
			if (type == NSS_MATCH_ADD_RULE) {
				sscanf(token, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		&mac_addr_tmp[0], &mac_addr_tmp[1], &mac_addr_tmp[2], &mac_addr_tmp[3], &mac_addr_tmp[4], &mac_addr_tmp[5]);

				memcpy((uint8_t *)smac, mac_addr_tmp, 6);
			}

			nss_match_info("%px: src mac %x %x %x ", nss_ctx, smac[0], smac[1], smac[2]);
			continue;
		}

		/*
		 * Parse the destination mac address of the message.
		 */
		if (!(strncasecmp(param, "dmac", strlen("dmac")))) {

			/*
			 * Parse the 48bit mask input in hex. For ex, dmac=FFFF0000FFFF
			 */
			if (type == NSS_MATCH_ADD_MASK) {

				tmp[0] = token[0];
				tmp[1] = token[1];
				tmp[2] = token[2];
				tmp[3] = token[3];
				sscanf(tmp, "%hx", &dmac[0]);

				tmp[0] = token[4];
				tmp[1] = token[5];
				tmp[2] = token[6];
				tmp[3] = token[7];
				sscanf(tmp, "%hx", &dmac[1]);

				tmp[0] = token[8];
				tmp[1] = token[9];
				tmp[2] = token[10];
				tmp[3] = token[11];
				sscanf(tmp, "%hx", &dmac[2]);
			}

			/*
			 * Parse the 6 byte MAC address delimited by ':'. For ex, dmac=01:00:5e:ff:45:a1
			 */
			if (type == NSS_MATCH_ADD_RULE) {
				sscanf(token, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		&mac_addr_tmp[0], &mac_addr_tmp[1], &mac_addr_tmp[2], &mac_addr_tmp[3], &mac_addr_tmp[4], &mac_addr_tmp[5]);
				memcpy((uint8_t *)dmac, mac_addr_tmp, 6);
			}

			nss_match_info("%px: dest mac %x %x %x ", nss_ctx, dmac[0], dmac[1], dmac[2]);
			continue;
		}

		/*
		 * Parse ethertype of the message.
		 */
		if (!(strncasecmp(param, "ethertype", strlen("ethertype")))) {
			ret = sscanf(token, "%hx", &ethertype);
			if (!ret) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing action from the message provided by user.
		 */
		if (!(strncasecmp(param, "action", strlen("action")))) {
			if (!sscanf(token, "%u", &actions)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing priority for action provided by user.
		 */
		if (!(strncasecmp(param, "priority", strlen("priority")))) {
			if (!sscanf(token, "%u", &setprio)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing nexthop for action provided by user.
		 */
		if (!(strncasecmp(param, "nexthop", strlen("nexthop")))) {
			if (!sscanf(token, "%u", &nexthop)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		pr_info("%px: Not a valid input\n", nss_ctx);
		goto fail;
	}

	/*
	 * Verify correctness of field combination.
	 */
	switch (type) {
	case NSS_MATCH_ADD_RULE:
		if (!mask_id || !actions) {
			goto fail;
		}

		switch(actions) {
		case NSS_MATCH_ACTION_SETPRIO:
			if (nexthop || setprio >= NSS_MAX_NUM_PRI) {
				goto fail;
			}
			break;
		case NSS_MATCH_ACTION_FORWARD:
			if (!(setprio == NSS_MAX_NUM_PRI) || !nexthop) {
				goto fail;
			}
			break;
		case NSS_MATCH_ACTION_SETPRIO | NSS_MATCH_ACTION_FORWARD:
			if (!nexthop || setprio >= NSS_MAX_NUM_PRI) {
				goto fail;
			}
			break;
		case NSS_MATCH_ACTION_DROP:
			if (!(setprio == NSS_MAX_NUM_PRI) || nexthop) {
				goto fail;
			}
			break;
		default:
			goto fail;
		}

		rule_msg->msg.l2_rule.if_num = if_num;

		/*
		 * Smac, dmac are in host order, these should be converted to network order, before sending to FW.
		 */
		rule_msg->msg.l2_rule.smac[0] = htons(smac[0]);
		rule_msg->msg.l2_rule.smac[1] = htons(smac[1]);
		rule_msg->msg.l2_rule.smac[2] = htons(smac[2]);
		rule_msg->msg.l2_rule.dmac[0] = htons(dmac[0]);
		rule_msg->msg.l2_rule.dmac[1] = htons(dmac[1]);
		rule_msg->msg.l2_rule.dmac[2] = htons(dmac[2]);
		rule_msg->msg.l2_rule.ethertype = ethertype;
		rule_msg->msg.l2_rule.mask_id = mask_id;
		rule_msg->msg.l2_rule.action.setprio = setprio;
		rule_msg->msg.l2_rule.action.action_flag = actions;
		rule_msg->msg.l2_rule.action.forward_ifnum = nexthop;
		break;
	case NSS_MATCH_ADD_MASK:
		if (!mask_id) {
			pr_info("Missing mandatory field, Mask ID.\n");
			goto fail;
		}

		mask_val[0] = (if_num << MATCH_L2_KEY0_IFNUM_SHIFT);
		mask_val[0] |= (dmac[0] << MATCH_L2_KEY0_DMAC_HW0_SHIFT);
		mask_val[1] = (dmac[1] << MATCH_L2_KEY1_DMAC_HW1_SHIFT);
		mask_val[1] |= (dmac[2] << MATCH_L2_KEY1_DMAC_HW2_SHIFT);
		mask_val[2] = (smac[0] << MATCH_L2_KEY2_SMAC_HW0_SHIFT);
		mask_val[2] |= (smac[1] << MATCH_L2_KEY2_SMAC_HW1_SHIFT);
		mask_val[3] = (smac[2] << MATCH_L2_KEY3_SMAC_HW2_SHIFT);
		mask_val[3] |= (ethertype << MATCH_L2_KEY3_ETHERTYPE_SHIFT);

		rule_msg->msg.configure_msg.maskset[mask_id-1][0] = mask_val[0];
		rule_msg->msg.configure_msg.maskset[mask_id-1][1] = mask_val[1];
		rule_msg->msg.configure_msg.maskset[mask_id-1][2] = mask_val[2];
		rule_msg->msg.configure_msg.maskset[mask_id-1][3] = mask_val[3];
		rule_msg->msg.configure_msg.valid_mask_flag = mask_id;
		break;
	default:
		pr_info("Invalid parse type: %d", type);
		goto fail;
	}

	return 0;

fail:
	pr_warn("Invalid input, Check help: cat /proc/sys/dev/nss/match/help");
	return -EINVAL;
}

/*
 * nss_match_l2_table_read()
 * 	Reads stats for l2 profile.
 */
static size_t nss_match_l2_table_read(struct nss_match_instance *db_instance, size_t buflen, char *bufp) {
	int i, j;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	struct net_device *net_dev;
	uint64_t mask_hit_count = 0;
	size_t size_wr = 0;
	char *dev_name;

	size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "\nProfile Type = %d\n", db_instance->profile_type);

	for (i = 0; i < NSS_MATCH_MASK_MAX; i++) {
		if (!(db_instance->valid_mask_flag & (1 << i))) {
			size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "Mask %d = Invalid\n", i+1);
			continue;
		}

		for (j = 0; j < NSS_MATCH_INSTANCE_RULE_MAX; j++) {
			if (!db_instance->valid_rule_mask[i][j]) {
				continue;
			}

			mask_hit_count += db_instance->stats.hit_count[j];
		}

		size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "Mask %d =%x %x %x %x\t\t Rule hits using mask %d = %llu\n\n",
				i+1, db_instance->maskset[i][3], db_instance->maskset[i][2], db_instance->maskset[i][1], db_instance->maskset[i][0], i+1, mask_hit_count);
		mask_hit_count = 0;
	}

	size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "%8s  %20s  %8s  %8s  %18s  %18s  %10s  %8s  %8s  %9s\n\n",
		                "rule_id", "hit_count_per_rule", "mask_id", "if_name", "dest_mac_addr", "src_mac_addr", "ethertype", "action", "prio", "nh_ifnum");

	for (j = 0; j < NSS_MATCH_INSTANCE_RULE_MAX; j++) {
		if (!db_instance->rules[j].valid_rule)
			continue;

		dev_name = "N/A";
		net_dev = nss_cmn_get_interface_dev(nss_ctx, db_instance->rules[j].profile.l2.if_num);
		if (net_dev) {
			dev_name = net_dev->name;
		}

		size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "%8d  %20llu  %8d  %8s  %18pM  %18pM  %10hx  %8u  %8d  %9d\n",
				j + 1,
				db_instance->stats.hit_count[j],
				db_instance->rules[j].profile.l2.mask_id,
				dev_name,
				&db_instance->rules[j].profile.l2.dmac,
				&db_instance->rules[j].profile.l2.smac,
				db_instance->rules[j].profile.l2.ethertype,
				db_instance->rules[j].profile.l2.action.action_flag,
				db_instance->rules[j].profile.l2.action.setprio,
				db_instance->rules[j].profile.l2.action.forward_ifnum);
	}

	return size_wr;
}

/*
 * Match ops for L2 profile.
 */
static struct match_profile_ops match_profile_ops_l2 = {
	nss_match_l2_rule_find,
	nss_match_l2_db_rule_add,
	nss_match_l2_rule_read,
	nss_match_l2_table_read,
	nss_match_l2_cmd_parse,
};

void nss_match_l2_init(void) {
	/*
	 * Register the L2 profile ops.
	 */
	nss_match_profile_ops_register(NSS_MATCH_PROFILE_TYPE_L2, &match_profile_ops_l2);
	nss_match_info("L2 profile initialization done\n");
}
