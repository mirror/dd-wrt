/*
 *******************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

#define NSS_MATCH_MAX_DSCP 63
#define NSS_MATCH_MAX_8021P 7
#define NSS_MATCH_VOW_KEY_IFNUM_SHIFT 0
#define NSS_MATCH_VOW_KEY_DSCP_SHIFT 16
#define NSS_MATCH_VOW_KEY_INNER_8021P_SHIFT 22
#define NSS_MATCH_VOW_KEY_OUTER_8021P_SHIFT 25

/*
 * nss_match_vow_rule_find()
 * 	Check if rule exists already.
 */
static bool nss_match_vow_rule_find(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule)
{
	uint16_t index;

	/*
	 * Check if entry is present already.
	 */
	for (index = 0; index < NSS_MATCH_INSTANCE_RULE_MAX; index++) {
		struct nss_match_rule_info rule_info = db_instance->rules[index];

		if (rule->valid_rule &&
				rule_info.profile.vow.if_num == rule->profile.vow.if_num &&
				rule_info.profile.vow.dscp == rule->profile.vow.dscp &&
				rule_info.profile.vow.inner_8021p == rule->profile.vow.inner_8021p &&
				rule_info.profile.vow.outer_8021p == rule->profile.vow.outer_8021p &&
				rule_info.profile.vow.mask_id == rule->profile.vow.mask_id) {
			nss_match_info("Rule matched.\n");
			return true;
		}
	}

	return false;
}

/*
 * nss_match_vow_db_rule_add()
 * 	Store VoW rule information.
 */
static bool nss_match_vow_db_rule_add(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule)
{
	uint8_t rule_id = rule->profile.vow.rule_id;
	uint8_t mask_id = rule->profile.vow.mask_id;

	if (rule_id == 0 || rule_id > NSS_MATCH_INSTANCE_RULE_MAX) {
		nss_match_info("Invalid rule id: %d\n", rule_id);
		return false;
	}

	if (db_instance->rules[rule_id - 1].valid_rule) {
		nss_match_info("Rule exists for rule id: %d\n", rule_id);
		return false;
	}

	db_instance->rules[rule_id - 1].profile.vow.if_num = rule->profile.vow.if_num;
	db_instance->rules[rule_id - 1].profile.vow.dscp = rule->profile.vow.dscp;
	db_instance->rules[rule_id - 1].profile.vow.inner_8021p = rule->profile.vow.inner_8021p;
	db_instance->rules[rule_id - 1].profile.vow.outer_8021p = rule->profile.vow.outer_8021p;
	db_instance->rules[rule_id - 1].profile.vow.mask_id = mask_id;
	db_instance->rules[rule_id - 1].profile.vow.action.action_flag = rule->profile.vow.action.action_flag;
	db_instance->rules[rule_id - 1].profile.vow.action.setprio = rule->profile.vow.action.setprio;
	db_instance->rules[rule_id - 1].profile.vow.action.forward_ifnum = rule->profile.vow.action.forward_ifnum;
	db_instance->rules[rule_id - 1].valid_rule = true;
	db_instance->rules[rule_id - 1].profile.vow.rule_id = rule_id;
	db_instance->valid_rule_mask[mask_id - 1][rule_id - 1] = true;
	db_instance->rule_count++;

	return true;
}

/*
 * nss_match_vow_rule_read()
 *	Reads rule parameters by rule id.
 */
static bool nss_match_vow_rule_read(struct nss_match_instance *db_instance, struct nss_match_rule_info *rule, uint16_t rule_id)
{
	if (rule_id == 0 || rule_id > NSS_MATCH_INSTANCE_RULE_MAX) {
		nss_match_info("Invalid rule id: %d\n", rule_id);
		return false;
	}

	if (!db_instance->rules[rule_id - 1].valid_rule) {
		nss_match_info("rule_id doesnot exist, rule_id = %d", rule_id);
		return false;
	}

	rule->profile.vow.if_num = db_instance->rules[rule_id - 1].profile.vow.if_num;
	rule->profile.vow.dscp = db_instance->rules[rule_id - 1].profile.vow.dscp;
	rule->profile.vow.outer_8021p = db_instance->rules[rule_id - 1].profile.vow.outer_8021p;
	rule->profile.vow.inner_8021p = db_instance->rules[rule_id - 1].profile.vow.inner_8021p;
	rule->profile.vow.mask_id = db_instance->rules[rule_id - 1].profile.vow.mask_id;
	rule->profile.vow.rule_id = db_instance->rules[rule_id - 1].profile.vow.rule_id;
	return true;
}

/*
 * nss_match_vow_cmd_parse()
 *	Adds new rules to the list
 */
static int nss_match_vow_cmd_parse(char *input_msg, struct nss_match_msg *rule_msg, nss_match_cmd_t type)
{
	char *token, *param, *value;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	int ret = 0;
	uint32_t actions = 0, if_num = 0, dscp = 0, outer_prio = 0, inner_prio = 0, setprio = NSS_MAX_NUM_PRI, nexthop = 0;
	uint16_t mask_id = 0;
	uint32_t mask_val = 0;

	/*
	 * Parse the user input.
	 */
	while (input_msg != NULL) {
		token = strsep(&input_msg, " ");
		param = strsep(&token, "=");
		value = token;
		if (!param || !value) {
			goto fail;
		}

		/*
		 * Parsing mask ID from the message.
		 */
		if (!(strncasecmp(param, "mask", strlen("mask")))) {
			if (!sscanf(value, "%hu", &mask_id)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing interface name from the message.
		 */
		if (!(strncasecmp(param, "ifname", strlen("ifname")))) {
			struct net_device *dev;

			if (type == NSS_MATCH_ADD_MASK) {
				if (!sscanf(value, "%x", &if_num)) {
					pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
					return -EINVAL;
				}
				continue;
			}

			if (type == NSS_MATCH_ADD_RULE) {
				dev = dev_get_by_name(&init_net, value);
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
		 * Parsing Dscp value from the message.
		 */
		if (!(strncasecmp(param, "dscp", strlen("dscp")))) {

			if (type == NSS_MATCH_ADD_RULE) {
				ret = sscanf(value, "%u", &dscp);
			} else if (type == NSS_MATCH_ADD_MASK) {
				ret = sscanf(value, "%x", &dscp);
			}

			if (!ret) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing 8021.p inner from the message given by host.
		 */
		if (!(strncasecmp(param, "802.1p_inner", strlen("802.1p_inner")))) {
			if (type == NSS_MATCH_ADD_RULE) {
				ret = sscanf(value, "%u", &inner_prio);
			} else if (type == NSS_MATCH_ADD_MASK) {
				ret = sscanf(value, "%x", &inner_prio);
			}

			if (!ret) {
				pr_info("%px: Cannot convert to integer. Wrong input!!\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing 8021.p outer from the message given by host.
		 */
		if (!(strncasecmp(param, "802.1p_outer", strlen("802.1p_outer")))) {

			if (type == NSS_MATCH_ADD_RULE) {
				ret = sscanf(value, "%u", &outer_prio);
			} else if (type == NSS_MATCH_ADD_MASK) {
				ret = sscanf(value, "%x", &outer_prio);
			}

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
			if (!sscanf(value, "%u", &actions)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing priority for action provided by user.
		 */
		if (!(strncasecmp(param, "priority", strlen("priority")))) {
			if (!sscanf(value, "%u", &setprio)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		/*
		 * Parsing nexthop for action provided by user.
		 */
		if (!(strncasecmp(param, "nexthop", strlen("nexthop")))) {
			if (!sscanf(value, "%u", &nexthop)) {
				pr_info("%px: Cannot convert to integer. Wrong input\n", nss_ctx);
				return -EINVAL;
			}

			continue;
		}

		pr_info("%px: Not a valid input\n", nss_ctx);
		return -EINVAL;
	}

	/*
	 * Validate user input values.
	 */
	if (mask_id > NSS_MATCH_MASK_MAX) {
		pr_info("%px: Maskset num exceeds allowed value: %d\n", nss_ctx, mask_id);
		return -EINVAL;
	}

	if (dscp > NSS_MATCH_MAX_DSCP) {
		pr_info("%px: Dscp value %d cannot go beyond %d\n", nss_ctx, dscp, NSS_MATCH_MAX_DSCP);
		return -EINVAL;
	}

	if (inner_prio > NSS_MATCH_MAX_8021P || outer_prio > NSS_MATCH_MAX_8021P) {
		pr_info("%px: Priority inner:%d outer:%d value cannot go beyond 7.\n", nss_ctx, inner_prio, outer_prio);
		return -EINVAL;
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

		if (!outer_prio && inner_prio) {
			pr_info("Wrong config for two layer of vlan, inner = %u outer = %u", inner_prio, outer_prio);
			goto fail;
		}

		rule_msg->msg.vow_rule.if_num = if_num;
		rule_msg->msg.vow_rule.dscp = dscp;
		rule_msg->msg.vow_rule.outer_8021p = outer_prio;
		rule_msg->msg.vow_rule.inner_8021p = inner_prio;
		rule_msg->msg.vow_rule.mask_id = mask_id;
		rule_msg->msg.vow_rule.action.setprio = setprio;
		rule_msg->msg.vow_rule.action.action_flag = actions;
		rule_msg->msg.vow_rule.action.forward_ifnum = nexthop;
		break;
	case NSS_MATCH_ADD_MASK:
		if (!mask_id) {
			pr_info("Missing mandatory field, mask ID.\n");
			goto fail;
		}

		mask_val = (if_num << NSS_MATCH_VOW_KEY_IFNUM_SHIFT);
	 	mask_val |= (dscp << NSS_MATCH_VOW_KEY_DSCP_SHIFT);
	 	mask_val |= (inner_prio << NSS_MATCH_VOW_KEY_INNER_8021P_SHIFT);
	 	mask_val |= (outer_prio << NSS_MATCH_VOW_KEY_OUTER_8021P_SHIFT);

		rule_msg->msg.configure_msg.profile_type = NSS_MATCH_PROFILE_TYPE_VOW;
		rule_msg->msg.configure_msg.maskset[mask_id-1][0] = mask_val;
		rule_msg->msg.configure_msg.valid_mask_flag = mask_id;
		break;
	default:
		pr_info("Invalid parse type: %d", type);
		goto fail;
	}

	return 0;

fail:
	pr_warn("Invalid input, Check help.(cat /proc/sys/dev/nss/match/help)");
	return -EINVAL;
}

/*
 * nss_match_vow_table_read()
 * 	Reads stats for VoW profile.
 */
static size_t nss_match_vow_table_read(struct nss_match_instance *db_instance, size_t buflen, char *bufp)
{
	int i, j;
	struct nss_ctx_instance *nss_ctx = nss_match_get_context();
	struct net_device *net_dev;
	uint64_t mask_hit_count = 0;
	size_t size_wr = 0;
	char *dev_name;

	size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "Match if_num = %d\n", db_instance->if_num);
	size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "Profile Type = %d\n", db_instance->profile_type);

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

		size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "Mask %d = %x\t Rule hits using mask %d = %llu\n",
				i+1, db_instance->maskset[i][0], i+1, mask_hit_count);
		mask_hit_count = 0;
	}

	size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "%8s %20s  %8s  %8s  %8s  %9s  %8s  %8s  %8s  %9s\n\n",
		"rule_id", "hit count per rule", "mask_id", "if_name", "dscp", "out_vlan", "in_vlan", "action", "prio", "nh_ifnum");

	for (j = 0; j < NSS_MATCH_INSTANCE_RULE_MAX; j++) {
		if (!db_instance->rules[j].valid_rule)
			continue;

		dev_name = "N/A";
		net_dev = nss_cmn_get_interface_dev(nss_ctx, db_instance->rules[j].profile.vow.if_num);
		if (net_dev) {
			dev_name = net_dev->name;
		}

		size_wr += scnprintf(bufp + size_wr, buflen - size_wr, "%8d %20llu  %8d  %8s  %8d  %8d  %8d  %8u  %8d %8d\n",
				j+1,
				db_instance->stats.hit_count[j],
				db_instance->rules[j].profile.vow.mask_id,
				dev_name,
				db_instance->rules[j].profile.vow.dscp,
				db_instance->rules[j].profile.vow.outer_8021p,
				db_instance->rules[j].profile.vow.inner_8021p,
				db_instance->rules[j].profile.vow.action.action_flag,
				db_instance->rules[j].profile.vow.action.setprio,
				db_instance->rules[j].profile.vow.action.forward_ifnum);
	}

	return size_wr;
}

/*
 * Match ops for VoW profile.
 */
static struct match_profile_ops match_profile_vow_ops = {
	nss_match_vow_rule_find,
	nss_match_vow_db_rule_add,
	nss_match_vow_rule_read,
	nss_match_vow_table_read,
	nss_match_vow_cmd_parse,
};

/*
 * nss_match_vow_init()
 * 	Initializes the VoW profile.
 */
void nss_match_vow_init(void) {

	/*
	 * Register the VoW profile ops.
	 */
	nss_match_profile_ops_register(NSS_MATCH_PROFILE_TYPE_VOW, &match_profile_vow_ops);
	nss_match_info("VoW profile initialization done\n");
}
