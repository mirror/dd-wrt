/*
 **************************************************************************
 * Copyright (c) 2015-2018, 2021, The Linux Foundation.  All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#include <linux/module.h>
#include <linux/of.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <asm/unaligned.h>
#include <linux/uaccess.h>	/* for put_user */
#include <net/ipv6.h>
#include <linux/inet.h>
#include <linux/in.h>
#include <linux/etherdevice.h>

#include "ecm_classifier_pcc_public.h"

#define MAC_FMT "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx"

#define RULE_FIELDS 15

/*
 * With feature_flags_support parameter enabled, registrant can provide
 * additional feature requests (like mirroring) to ECM.
 * It is disabled by default.
 */
bool feature_flags_support = false;
module_param(feature_flags_support, bool, S_IRUGO);
MODULE_PARM_DESC(feature_flags_support, "Enable feature flags support");

/*
 * DebugFS entry object.
 */
static struct dentry *ecm_pcc_test_dentry;

/*
 * Registration
 */
struct ecm_classifier_pcc_registrant *ecm_pcc_test_registrant = NULL;

/*
 * ecm_pcc_test_ap_info
 *	Descriptor for acl-policer feature.
 */
struct ecm_pcc_test_ap_info {
	int flow_ap_index;
	int return_ap_index;
};

/*
 * ecm_pcc_test_mirror_info
 *	Descriptor for mirror feature.
 */
struct ecm_pcc_test_mirror_info {
	char tuple_mirror_dev[IFNAMSIZ];
	char tuple_ret_mirror_dev[IFNAMSIZ];
};

/*
 * Rule table
 */
struct ecm_pcc_test_rule {
	struct list_head list;
	char name[50];
	ecm_classifier_pcc_result_t accel;
	unsigned int proto;
	uint8_t src_mac[ETH_ALEN];
	uint8_t dest_mac[ETH_ALEN];
	int src_port;
	int dest_port;
	struct in6_addr src_addr;
	struct in6_addr dest_addr;
	unsigned int ipv;
	unsigned int feature_flags;
	struct ecm_pcc_test_mirror_info mirror_info;
	struct ecm_pcc_test_ap_info ap_info;
};
LIST_HEAD(ecm_pcc_test_rules);
DEFINE_SPINLOCK(ecm_pcc_test_rules_lock);

/*
 * ecm_pcc_test_registrant_ref()
 *	Invoked when an additional hold is kept upon the registrant
 */
static void ecm_pcc_test_registrant_ref(struct ecm_classifier_pcc_registrant *r)
{
	int remain;

	/*
	 * Increment the ref count by 1.
	 * This causes the registrant structure to remain in existance until
	 * released (deref).
	 * By definition the caller of this method has a hold on the registrant
	 * already so it cannot 'go away'.
	 * This is because either:
	 * 1. The caller itself has been passed it in a function parameter;
	 * 2. It has its own explicit hold.
	 */
	remain = atomic_inc_return(&r->ref_count);
	if (remain <= 0)
		pr_debug("REFERENCE COUNT WRAP!\n");
	else
		pr_debug("ECM PCC Registrant ref: %d\n", remain);
}

/*
 * ecm_pcc_test_registrant_deref()
 *	Caller is releasing its hold upon the registrant.
 */
static void
ecm_pcc_test_registrant_deref(struct ecm_classifier_pcc_registrant *r)
{
	int remain;
	struct ecm_pcc_test_rule *tmp = NULL;
	struct list_head *pos, *q;

	/*
	 * Decrement the reference count
	 */
	remain = atomic_dec_return(&r->ref_count);
	if (remain > 0) {
		/*
		 * Something still holds a reference
		 */
		pr_debug("ECM PCC Registrant deref: %d\n", remain);
		return;
	}

	/*
	 * Last hold upon the registrant is released and so we can now
	 * destroy it.
	 */
	if (remain < 0)
		pr_info("REFERENCE COUNT WRAP!\n");

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	list_for_each_safe(pos, q , &ecm_pcc_test_rules) {
		tmp = list_entry(pos, struct ecm_pcc_test_rule, list);
		list_del(pos);
		kfree(tmp);
	}
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	pr_info("ECM PCC Registrant DESTROYED\n");
	kfree(r);
}

/*
 * __ecm_pcc_test_rule_find()
 *	Return matching rule
 */
static struct
ecm_pcc_test_rule *__ecm_pcc_test_rule_find(unsigned int proto,
					    uint8_t *src_mac,
					    uint8_t *dest_mac,
					    struct in6_addr *src_addr,
					    struct in6_addr *dest_addr,
					    int src_port, int dest_port,
					    bool *is_reverse)
{
	struct ecm_pcc_test_rule *rule = NULL;

	list_for_each_entry(rule , &ecm_pcc_test_rules, list) {
		if (rule->proto != proto)
			continue;

		if (rule->src_port != src_port)
			goto try_reverse;
		if (rule->dest_port != dest_port)
			goto try_reverse;
		if (!ipv6_addr_equal(&rule->src_addr, src_addr))
			goto try_reverse;
		if (!ipv6_addr_equal(&rule->dest_addr, dest_addr))
			goto try_reverse;

		return rule;

try_reverse:

		if (rule->src_port != dest_port)
			continue;
		if (rule->dest_port != src_port)
			continue;
		if (!ipv6_addr_equal(&rule->src_addr, dest_addr))
			continue;
		if (!ipv6_addr_equal(&rule->dest_addr, src_addr))
			continue;

		if (is_reverse) {
			*is_reverse = true;
		}

		return rule;
	}
	return NULL;
}

/*
 * ecm_pcc_test_fill_ap_info()
 *	Fill policing feature related information.
 */
static void ecm_pcc_test_fill_ap_info(struct ecm_pcc_test_rule *rule, struct ecm_classifier_pcc_info *cinfo)
{
	cinfo->ap_info.flow_ap_index = rule->ap_info.flow_ap_index;
	cinfo->ap_info.return_ap_index = rule->ap_info.return_ap_index;
}

/*
 * ecm_pcc_test_fill_mirror_info()
 *	Fill mirror feature related information.
 */
static int ecm_pcc_test_fill_mirror_info(struct ecm_pcc_test_rule *rule,
		struct ecm_classifier_pcc_info *cinfo, bool is_reverse)
{
	struct net_device *tuple_mirror_netdev = NULL, *tuple_ret_mirror_netdev = NULL;

	/*
	 * Get the name of the mirror device from the rule and fetch
	 * its netdevice structure.
	 */
	if (strlen(rule->mirror_info.tuple_mirror_dev)) {
		tuple_mirror_netdev = dev_get_by_name(&init_net, rule->mirror_info.tuple_mirror_dev);
		if (!tuple_mirror_netdev) {
			pr_warn("Cannot find flow mirror device (%s)\n", rule->mirror_info.tuple_mirror_dev);
			return -1;
		}
		pr_info("Tuple mirror device: %s\n", tuple_mirror_netdev->name);
		dev_put(tuple_mirror_netdev);
	}

	if (strlen(rule->mirror_info.tuple_ret_mirror_dev)) {
		tuple_ret_mirror_netdev = dev_get_by_name(&init_net, rule->mirror_info.tuple_ret_mirror_dev);
		if (!tuple_ret_mirror_netdev) {
			pr_warn("Cannot find return mirror device (%s)\n", rule->mirror_info.tuple_ret_mirror_dev);
			return -1;
		}
		pr_info("Tuple return mirror device: %s\n", tuple_ret_mirror_netdev->name);
		dev_put(tuple_ret_mirror_netdev);
	}

	/*
	 * Pass mirror netdevices of both direction to ECM.
	 */
	if (!is_reverse) {
		cinfo->mirror.tuple_mirror_dev = tuple_mirror_netdev;
		cinfo->mirror.tuple_ret_mirror_dev = tuple_ret_mirror_netdev;
	} else {
		cinfo->mirror.tuple_mirror_dev = tuple_ret_mirror_netdev;
		cinfo->mirror.tuple_ret_mirror_dev = tuple_mirror_netdev;
	}

	return 0;
}

/*
 * ecm_pcc_test_get_accel_info_v4()
 *	Invoked by the ECM to query if the given connection may be accelerated
 *	and to get the set of features to be enabled on it.
 */
static ecm_classifier_pcc_result_t
ecm_pcc_test_get_accel_info_v4(struct ecm_classifier_pcc_registrant *r,
			      uint8_t *src_mac, __be32 src_ip, int src_port,
			      uint8_t *dest_mac, __be32 dest_ip, int dest_port,
			      int protocol, struct ecm_classifier_pcc_info *cinfo)
{
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t accel;
	struct in6_addr src_addr = IN6ADDR_ANY_INIT;
	struct in6_addr dest_addr = IN6ADDR_ANY_INIT;
	unsigned int feature_flags;
	bool is_reverse = false;

	src_addr.s6_addr32[0] = src_ip;
	dest_addr.s6_addr32[0] = dest_ip;

	if (!cinfo) {
		pr_err("Invalid input parameter\n");
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(protocol, src_mac, dest_mac,
			&src_addr, &dest_addr,
			src_port, dest_port, &is_reverse);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		pr_debug("Rule not found\n");
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}
	accel = rule->accel;
	feature_flags= rule->feature_flags;

	/*
	 * Check if mirror feature is enabled on the rule.
	 * if enabled, then pass mirror netdevs for both the direction
	 * to ECM.
	 */
	if (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
		if (ecm_pcc_test_fill_mirror_info(rule, cinfo, is_reverse)) {
			spin_unlock_bh(&ecm_pcc_test_rules_lock);
			return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
		}
	}

	if ((rule->feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (rule->feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER)) {
		ecm_pcc_test_fill_ap_info(rule, cinfo);
	}

	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	cinfo->feature_flags |= feature_flags;
	return accel;
}

/*
 * ecm_pcc_test_get_accel_info_v6()
 *	Invoked by the ECM to query if the given connection may be accelerated
 *	and to get the set of features to be enabled on it.
 */
static ecm_classifier_pcc_result_t
ecm_pcc_test_get_accel_info_v6(struct ecm_classifier_pcc_registrant *r,
			      uint8_t *src_mac, struct in6_addr *src_addr,
			      int src_port, uint8_t *dest_mac,
			      struct in6_addr *dest_addr, int dest_port,
			      int protocol, struct ecm_classifier_pcc_info *cinfo)
{
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t accel;
	unsigned int feature_flags;
	bool is_reverse = false;

	if (!cinfo) {
		pr_err("Invalid input parameter\n");
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(protocol, src_mac, dest_mac,
			src_addr, dest_addr,
			src_port, dest_port, &is_reverse);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}
	accel = rule->accel;
	feature_flags= rule->feature_flags;

	/*
	 * Check if mirror feature is enabled on the rule.
	 * if enabled, then pass mirror netdevs for both the direction
	 * to ECM.
	 */
	if (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
		if (ecm_pcc_test_fill_mirror_info(rule, cinfo, is_reverse)) {
			spin_unlock_bh(&ecm_pcc_test_rules_lock);
			return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
		}
	}

	if ((feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER))  {
		ecm_pcc_test_fill_ap_info(rule, cinfo);
	}

	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	cinfo->feature_flags = feature_flags;
	return accel;
}

/*
 * ecm_pcc_test_okay_to_accel_v4()
 *	Invoked by the ECM to query if the given connection may be accelerated
 */
static ecm_classifier_pcc_result_t
ecm_pcc_test_okay_to_accel_v4(struct ecm_classifier_pcc_registrant *r,
			      uint8_t *src_mac, __be32 src_ip, int src_port,
			      uint8_t *dest_mac, __be32 dest_ip, int dest_port,
			      int protocol)
{
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t accel;
	struct in6_addr src_addr = IN6ADDR_ANY_INIT;
	struct in6_addr dest_addr = IN6ADDR_ANY_INIT;

	src_addr.s6_addr32[0] = src_ip;
	dest_addr.s6_addr32[0] = dest_ip;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(protocol, src_mac, dest_mac,
			&src_addr, &dest_addr,
			src_port, dest_port, NULL);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}
	accel = rule->accel;
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	return accel;
}

/*
 * ecm_pcc_test_okay_to_accel_v6()
 *	Invoked by the ECM to query if the given connection may be accelerated
 */
static ecm_classifier_pcc_result_t
ecm_pcc_test_okay_to_accel_v6(struct ecm_classifier_pcc_registrant *r,
			      uint8_t *src_mac, struct in6_addr *src_addr,
			      int src_port, uint8_t *dest_mac,
			      struct in6_addr *dest_addr, int dest_port,
			      int protocol)
{
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t accel;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(protocol, src_mac, dest_mac,
			src_addr, dest_addr,
			src_port, dest_port, NULL);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return ECM_CLASSIFIER_PCC_RESULT_NOT_YET;
	}
	accel = rule->accel;
	spin_unlock_bh(&ecm_pcc_test_rules_lock);
	return accel;
}

/*
 * ecm_pcc_test_unregister_get_unregister()
 */
static int ecm_pcc_test_unregister_get_unregister(void *data, u64 *val)
{
	*val = 0;
	return 0;
}

/*
 * ecm_pcc_test_unregister_set_unregister()
 */
static int ecm_pcc_test_unregister_set_unregister(void *data, u64 val)
{
	if (ecm_pcc_test_registrant) {
		pr_info("ECM PCC Test unregister\n");
		ecm_classifier_pcc_unregister_begin(ecm_pcc_test_registrant);
		ecm_pcc_test_registrant = NULL;
	}
	return 0;
}

/*
 * ecm_pcc_test_str_to_ip()
 *	Convert string IP to in6_addr.  Return 4 for IPv4 or 6 for IPv6.
	Return something else in error.
 *
 * NOTE: When string is IPv4 the lower 32 bit word of the in6 address
 * contains the address. Network order.
 */
static unsigned int ecm_pcc_test_str_to_ip(char *ip_str, struct in6_addr *addr)
{
	uint8_t *ptr = (uint8_t *)(addr->s6_addr);

	/*
	 * IPv4 address in addr->s6_addr32[0]
	 */
	if (in4_pton(ip_str, -1, ptr, '\0', NULL) > 0)
		return 4;

	/*
	 * IPv6
	 */
	if (in6_pton(ip_str, -1, ptr, '\0', NULL) > 0)
		return 6;

	return 0;
}

/*
 * ecm_pcc_test_decel()
 *	Decelerate the flow.
 */
static void ecm_pcc_test_decel(unsigned int proto,
		uint8_t *src_mac, uint8_t *dest_mac,
		struct in6_addr *src_addr, struct in6_addr *dest_addr,
		int src_port, int dest_port,
		unsigned int ipv)
{
	bool ret;

	if (ipv == 4) {
		ret = ecm_classifier_pcc_decel_v4(src_mac,
						src_addr->s6_addr32[0],
						src_port, dest_mac,
						dest_addr->s6_addr32[0],
						dest_port, proto);
	} else {
		ret = ecm_classifier_pcc_decel_v6(src_mac, src_addr, src_port,
						dest_mac, dest_addr, dest_port,
						proto);
	}

	if (!ret) {
		pr_warn("Decel operation failed, ret: %d\n", ret);
	}
}

/*
 * ecm_pcc_test_deny_accel()
 *	Deny acceleration
 */
static void ecm_pcc_test_deny_accel(unsigned int proto,
		uint8_t *src_mac, uint8_t *dest_mac,
		struct in6_addr *src_addr, struct in6_addr *dest_addr,
		int src_port, int dest_port,
		unsigned int ipv)
{
	if (ipv == 4)
		ecm_classifier_pcc_deny_accel_v4(src_mac,
						src_addr->s6_addr32[0],
						src_port, dest_mac,
						dest_addr->s6_addr32[0],
						dest_port, proto);
	else
		ecm_classifier_pcc_deny_accel_v6(src_mac, src_addr, src_port,
						dest_mac, dest_addr, dest_port,
						proto);
}

/*
 * ecm_pcc_test_permit_accel()
 *	Permit acceleration
 */
static void ecm_pcc_test_permit_accel(unsigned int proto,
		uint8_t *src_mac, uint8_t *dest_mac,
		struct in6_addr *src_addr, struct in6_addr *dest_addr,
		int src_port, int dest_port,
		unsigned int ipv)
{
	if (ipv == 4)
		ecm_classifier_pcc_permit_accel_v4(src_mac,
						src_addr->s6_addr32[0],
						src_port, dest_mac,
						dest_addr->s6_addr32[0],
						dest_port, proto);
	else
		ecm_classifier_pcc_permit_accel_v6(src_mac, src_addr, src_port,
						dest_mac, dest_addr, dest_port,
						proto);

}

/*
 * ecm_pcc_test_update_rule()
 *	Update a rule, return 0 on failure. If change in accel this will
 *	inform ECM.
 */
static unsigned int ecm_pcc_test_update_rule(char *name,
				ecm_classifier_pcc_result_t accel,
				unsigned int proto,
				uint8_t *src_mac, uint8_t *dest_mac,
				struct in6_addr *src_addr,
				struct in6_addr *dest_addr,
				int src_port, int dest_port, unsigned int feature_flags,
				char *tuple_mirror_dev, char *tuple_ret_mirror_dev, int flow_ap_index, int return_ap_index)
{
	unsigned int ipv, o_feature_flags;
	struct ecm_pcc_test_rule *rule;
	ecm_classifier_pcc_result_t oaccel;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, NULL);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return 0;
	}
	ipv = rule->ipv;
	oaccel = rule->accel;
	o_feature_flags = rule->feature_flags;
	rule->accel = accel;
	rule->feature_flags = feature_flags;
	strlcpy(rule->name, name, sizeof(rule->name));
	strlcpy(rule->mirror_info.tuple_mirror_dev, tuple_mirror_dev, IFNAMSIZ);
	strlcpy(rule->mirror_info.tuple_ret_mirror_dev, tuple_ret_mirror_dev, IFNAMSIZ);
	rule->ap_info.flow_ap_index = flow_ap_index;
	rule->ap_info.return_ap_index = return_ap_index;
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	if (feature_flags_support) {
		/*
		 * To have things simple, for now the per field check on the old
		 * and new rule to figure out whether the rule has actually changed
		 * or not is being avoided.
		 *
		 * If the old or new rule has the mirror flag set, then decelerate
		 * the connection to have in effect the updated values.
		 */
		if ((o_feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) ||
			(feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR)) {
			ecm_pcc_test_decel(proto, src_mac, dest_mac,
					src_addr, dest_addr, src_port, dest_port, ipv);
			return 1;
		}

		if ((o_feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (o_feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER)) {
			ecm_pcc_test_decel(proto, src_mac, dest_mac,
					src_addr, dest_addr, src_port, dest_port, ipv);
			return 1;
		}
	}

	if (oaccel == accel)
		return 1;

	if (accel == ECM_CLASSIFIER_PCC_RESULT_DENIED)
		ecm_pcc_test_deny_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);
	else
		ecm_pcc_test_permit_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);

	return 1;
}

/*
 * ecm_pcc_test_delete_rule()
 *	Delete a rule, return 0 on failure.  If deleted connection rule denied
 *	accel this will permit accel.
 */
static unsigned int ecm_pcc_test_delete_rule(char *name,
				ecm_classifier_pcc_result_t accel,
				unsigned int proto,
				uint8_t *src_mac, uint8_t *dest_mac,
				struct in6_addr *src_addr,
				struct in6_addr *dest_addr,
				int src_port, int dest_port)
{
	unsigned int ipv, feature_flags;
	struct ecm_pcc_test_rule *rule;

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	rule = __ecm_pcc_test_rule_find(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, NULL);
	if (!rule) {
		spin_unlock_bh(&ecm_pcc_test_rules_lock);
		return 0;
	}
	ipv = rule->ipv;
	accel = rule->accel;
	feature_flags = rule->feature_flags;
	list_del(&rule->list);
	spin_unlock_bh(&ecm_pcc_test_rules_lock);
	kfree(rule);

	if (feature_flags_support) {
		/*
		 * If mirror flag is set in the rule, then decelerate the
		 * existing connection, since the rule is getting deleted.
		 * The new acceleration decision for this flow should be
		 * taken by the get_accel_info_v4/v6 APIs.
		 */
		if (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
			ecm_pcc_test_decel(proto, src_mac, dest_mac,
					src_addr, dest_addr, src_port, dest_port, ipv);
			return 1;
		}

		if ((feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER))  {
			ecm_pcc_test_decel(proto, src_mac, dest_mac,
					src_addr, dest_addr, src_port, dest_port, ipv);
			return 1;
		}
	}

	if (accel == ECM_CLASSIFIER_PCC_RESULT_DENIED)
		ecm_pcc_test_permit_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);

	return 1;
}

/*
 * ecm_pcc_test_add_rule()
 *	Add a new rule, return 0 on failure.  Given accel the ecm is informed
 *	of permit/deny accel status.
 */
static unsigned int ecm_pcc_test_add_rule(char *name,
				ecm_classifier_pcc_result_t accel,
				unsigned int proto,
				uint8_t *src_mac, uint8_t *dest_mac,
				struct in6_addr *src_addr,
				struct in6_addr *dest_addr,
				int src_port, int dest_port,
				unsigned int ipv, unsigned int feature_flags,
				char *tuple_mirror_dev, char *tuple_ret_mirror_dev,
				int flow_ap_index, int return_ap_index)
{
	struct ecm_pcc_test_rule *new_rule;
	new_rule = kzalloc(sizeof(struct ecm_pcc_test_rule), GFP_ATOMIC);
	if (!new_rule)
		return 0;

	strlcpy(new_rule->name, name, sizeof(new_rule->name));
	new_rule->accel = accel;
	new_rule->proto = proto;
	new_rule->src_port = src_port;
	new_rule->dest_port = dest_port;
	ether_addr_copy(new_rule->src_mac, src_mac);
	ether_addr_copy(new_rule->dest_mac, dest_mac);
	new_rule->src_addr = *src_addr;
	new_rule->dest_addr = *dest_addr;
	new_rule->ipv = ipv;
	new_rule->feature_flags = feature_flags;
	strlcpy(new_rule->mirror_info.tuple_mirror_dev, tuple_mirror_dev, IFNAMSIZ);
	strlcpy(new_rule->mirror_info.tuple_ret_mirror_dev, tuple_ret_mirror_dev, IFNAMSIZ);
	new_rule->ap_info.flow_ap_index = flow_ap_index;
	new_rule->ap_info.return_ap_index = return_ap_index;
	INIT_LIST_HEAD(&new_rule->list);

	spin_lock_bh(&ecm_pcc_test_rules_lock);
	list_add(&new_rule->list, &ecm_pcc_test_rules);
	spin_unlock_bh(&ecm_pcc_test_rules_lock);

	if (feature_flags_support) {
		/*
		 * Return as we don't want to set the accel permit state to
		 * permit or deny. We want our registrant's callback to be get
		 * called so that it can fetch the mirror netdev and pass it to
		 * ECM.
		 */
		if (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
			return 1;
		}

		if (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) {
			return 1;
		}
		if (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER) {
			return 1;
		}
	}

	if (accel == ECM_CLASSIFIER_PCC_RESULT_DENIED)
		ecm_pcc_test_deny_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);
	else
		ecm_pcc_test_permit_accel(proto, src_mac, dest_mac,
			src_addr, dest_addr, src_port, dest_port, ipv);

	return 1;
}

/*
 * ecm_pcc_test_rule_write()
 *	Write a rule
 */
static ssize_t ecm_pcc_test_rule_write(struct file *file,
		const char __user *user_buf, size_t count, loff_t *ppos)
{
	char *rule_buf;
	int field_count;
	char *field_ptr;
	char *fields[RULE_FIELDS];
	char name[50];
	char tuple_mirror_dev[IFNAMSIZ] = {0};
	char tuple_ret_mirror_dev[IFNAMSIZ] = {0};
	uint8_t src_mac[ETH_ALEN];
	uint8_t dest_mac[ETH_ALEN];
	unsigned int oper;
	unsigned int feature_flags;
	ecm_classifier_pcc_result_t accel;
	unsigned int proto;
	int src_port;
	int dest_port;
	struct in6_addr src_addr = IN6ADDR_ANY_INIT;
	struct in6_addr dest_addr = IN6ADDR_ANY_INIT;
	unsigned int ipv;
	int flow_ap_index = 0;
	int return_ap_index = 0;

	/*
	 * buf is formed as:
	 * [0]    [1]                 [2]                           [3]     [4]       [5]        [6]        [7]        [8]         [9]         [10]            [11]               [12]
	 * <name>/<0=del,1=add,2=upd>/<1=denied, 2=accel_permitted>/<proto>/<src_mac>/<src_addr>/<src_port>/<dest mac>/<dest_addr>/<dest_port>/<feature_flags>/<tuple_mirror_dev>/<tuple_ret_mirror_dev/flow_ap_index/return_ap_index>
	 * e.g.:
	 * echo "my_rule/1/2/6/00:12:12:34:56:78/192.168.1.33/1234/00:12:12:34:56:22/10.10.10.10/80/1/mirror.0/mirror.1/flow_ap_index/return_ap_index" > /sys/kernel/debug/ecm_pcc_test/rule
	 * cat /sys/kernel/debug/ecm_pcc_test/rule (shows all rules)
	 * In the above rule, flow_ap_index and return_ap_index are either both represented as ACL indexes or Policer indexes based on the usecase.
	 */
	rule_buf = kzalloc(count + 100, GFP_ATOMIC);
	if (!rule_buf)
		return -EINVAL;

	count = simple_write_to_buffer(rule_buf, count, ppos, user_buf, count);

	/*
	 * Split the buffer into its fields
	 */
	field_count = 0;
	field_ptr = rule_buf;
	fields[field_count] = strsep(&field_ptr, "/");
	while (fields[field_count] != NULL) {
		pr_info("Field %d:\n", field_count);
		field_count++;
		if (field_count == RULE_FIELDS)
			break;

		fields[field_count] = strsep(&field_ptr, "/ \n");
	}

	if (field_count != RULE_FIELDS) {
		pr_info("Invalid field count %d\n", field_count);
		kfree(rule_buf);
		return -EINVAL;
	}

	/*
	 * Convert fields
	 */
	strlcpy(name, fields[0], sizeof(name));

	name[sizeof(name) - 1] = 0;
	if (sscanf(fields[1], "%u", &oper) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[2], "%d", (int *)&accel) != 1)
		goto sscanf_read_error;

	switch (accel) {
	case ECM_CLASSIFIER_PCC_RESULT_DENIED:
	case ECM_CLASSIFIER_PCC_RESULT_PERMITTED:
		break;
	default:
		pr_info("Bad accel: %u\n", accel);
		kfree(rule_buf);
		return -EINVAL;
	}

	if (sscanf(fields[3], "%u", &proto) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[6], "%d", &src_port) != 1)
		goto sscanf_read_error;
	src_port = htons(src_port);

	if (sscanf(fields[9], "%d", &dest_port) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[10], "%u", &feature_flags) != 1) {
		goto sscanf_read_error;
	}

	if ((feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) && (feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER)) {
		pr_info("ACL and POLICER both are enabled\n");
		kfree(rule_buf);
		return -EINVAL;
	}

	strlcpy(tuple_mirror_dev, fields[11], IFNAMSIZ);

	strlcpy(tuple_ret_mirror_dev, fields[12], IFNAMSIZ);

	if (sscanf(fields[13], "%d", &flow_ap_index) != 1)
		goto sscanf_read_error;

	if (sscanf(fields[14], "%d", &return_ap_index) != 1)
		goto sscanf_read_error;

	if (flow_ap_index == 0 && return_ap_index == 0) {
		pr_info("Flow and return AP index both are 0\n");
		kfree(rule_buf);
		return -EINVAL;
	}

	dest_port = htons(dest_port);

	if (sscanf(fields[4], MAC_FMT, src_mac, src_mac + 1, src_mac + 2,
		src_mac + 3, src_mac + 4, src_mac + 5) != 6)
		goto sscanf_read_error;

	if (sscanf(fields[7], MAC_FMT, dest_mac, dest_mac + 1, dest_mac + 2,
			dest_mac + 3, dest_mac + 4, dest_mac + 5) != 6)
		goto sscanf_read_error;

	ipv = ecm_pcc_test_str_to_ip(fields[5], &src_addr);
	if (ipv != ecm_pcc_test_str_to_ip(fields[8], &dest_addr)) {
		pr_info("Conflicting IP address types\n");
		kfree(rule_buf);
		return -EINVAL;
	}
	kfree(rule_buf);

	pr_info("name: %s\n"
			"oper: %u\n"
			"accel: %d\n"
			"proto: %u\n"
			"%pM\n"
			"%pI6:%d\n"
			"%pM\n"
			"%pI6:%d\n"
			"ipv: %u\n"
			"feature_flags: %0x\n"
			"tuple_mirror_dev: %s\n"
			"tuple_ret_mirror_dev: %s\n"
			"acl_index: %d\n"
			"policer_index: %d\n",
			name,
			oper,
			(int)accel,
			proto,
			src_mac,
			&src_addr,
			ntohs(src_port),
			dest_mac,
			&dest_addr,
			ntohs(dest_port),
			ipv,
			feature_flags,
			tuple_mirror_dev,
			tuple_ret_mirror_dev,
			flow_ap_index,
			return_ap_index);

	if (oper == 0) {
		pr_info("Delete\n");
		if (!ecm_pcc_test_delete_rule(name, accel, proto, src_mac,
						dest_mac, &src_addr, &dest_addr,
						src_port, dest_port))
			return -EINVAL;

	} else if (oper == 1) {
		pr_info("Add\n");
		if (!ecm_pcc_test_add_rule(name, accel, proto, src_mac,
						dest_mac, &src_addr, &dest_addr,
						src_port, dest_port, ipv,
						feature_flags, tuple_mirror_dev,
						tuple_ret_mirror_dev, flow_ap_index,
						return_ap_index))
			return -EINVAL;

	} else if (oper == 2) {
		pr_info("Update\n");
		if (!ecm_pcc_test_update_rule(name, accel, proto, src_mac,
						dest_mac, &src_addr, &dest_addr,
						src_port, dest_port, feature_flags,
						tuple_mirror_dev, tuple_ret_mirror_dev,
						flow_ap_index, return_ap_index))
			return -EINVAL;

	} else {
		pr_info("Unknown operation: %u\n", oper);
		return -EINVAL;
	}

	return count;

sscanf_read_error:
	pr_info("sscanf read error\n");
	kfree(rule_buf);
	return -EINVAL;
}

/*
 * ecm_pcc_test_rule_seq_show()
 */
static int ecm_pcc_test_rule_seq_show(struct seq_file *m, void *v)
{
	struct ecm_pcc_test_rule *rule;

	rule = list_entry(v, struct ecm_pcc_test_rule, list);

	seq_printf(m, "RULE:\n"
			"\tname: %s\n"
			"\taccel: %d\n"
			"\tproto: %u\n"
			"\t"
			"%pM\n"
			"\t%pI6:%u\n"
			"\t"
			"%pM\n"
			"\t%pI6:%d\n"
			"\tipv: %u\n"
			"\tfeature_flags: %u\n",
			rule->name,
			(int)(rule->accel),
			rule->proto,
			rule->src_mac,
			&rule->src_addr,
			ntohs(rule->src_port),
			rule->dest_mac,
			&rule->dest_addr,
			ntohs(rule->dest_port),
			rule->ipv,
			rule->feature_flags
			);

	if (feature_flags_support) {
		if (rule->feature_flags & ECM_CLASSIFIER_PCC_FEATURE_MIRROR) {
			if (strlen(rule->mirror_info.tuple_mirror_dev)) {
				seq_printf(m, "mirror_tuple_dev: %s\n",
						rule->mirror_info.tuple_mirror_dev);
			}
			if (strlen(rule->mirror_info.tuple_ret_mirror_dev)) {
				seq_printf(m, "mirror_ret_tuple_dev: %s\n",
						rule->mirror_info.tuple_ret_mirror_dev);
			}
		}

		if ((rule->feature_flags & ECM_CLASSIFIER_PCC_FEATURE_ACL) || (rule->feature_flags & ECM_CLASSIFIER_PCC_FEATURE_POLICER)) {
			if (rule->ap_info.flow_ap_index) {
				seq_printf(m, "flow_ap_index: %d\n", rule->ap_info.flow_ap_index);
			}

			if (rule->ap_info.return_ap_index) {
				seq_printf(m, "return_ap_index: %d\n", rule->ap_info.return_ap_index);
			}
		}
	}

	return 0;
}

/*
 * ecm_pcc_test_rule_seq_stop()
 */
static void ecm_pcc_test_rule_seq_stop(struct seq_file *p, void *v)
{
	spin_unlock_bh(&ecm_pcc_test_rules_lock);
}

/*
 * ecm_pcc_test_rule_seq_start()
 */
static void *ecm_pcc_test_rule_seq_start(struct seq_file *m, loff_t *_pos)
{
	spin_lock_bh(&ecm_pcc_test_rules_lock);
	return seq_list_start(&ecm_pcc_test_rules, *_pos);
}

/*
 * ecm_pcc_test_rule_seq_next()
 */
static void *ecm_pcc_test_rule_seq_next(struct seq_file *p, void *v,
					loff_t *pos)
{
	return seq_list_next(v, &ecm_pcc_test_rules, pos);
}

/*
 * DebugFS attributes.
 */
DEFINE_SIMPLE_ATTRIBUTE(ecm_pcc_test_unregister_fops,
			ecm_pcc_test_unregister_get_unregister,
			ecm_pcc_test_unregister_set_unregister, "%llu\n");

static const struct seq_operations ecm_pcc_test_rule_seq_ops = {
	.start = ecm_pcc_test_rule_seq_start,
	.next  = ecm_pcc_test_rule_seq_next,
	.stop  = ecm_pcc_test_rule_seq_stop,
	.show  = ecm_pcc_test_rule_seq_show,
};

/*
 * ecm_pcc_test_rule_open()
 */
static int ecm_pcc_test_rule_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ecm_pcc_test_rule_seq_ops);
}

/*
 * ecm_pcc_test_rule_release()
 */
static int ecm_pcc_test_rule_release(struct inode *inode, struct file *file)
{
	return seq_release(inode, file);
}

static const struct file_operations ecm_pcc_test_rule_fops = {
	.owner		= THIS_MODULE,
	.open		= ecm_pcc_test_rule_open,
	.write		= ecm_pcc_test_rule_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= ecm_pcc_test_rule_release,
};

/*
 * ecm_pcc_test_init()
 */
static int __init ecm_pcc_test_init(void)
{
	int result;

	pr_info("ECM PCC Test INIT\n");

	/*
	 * Create entries in DebugFS for control functions
	 */
	ecm_pcc_test_dentry = debugfs_create_dir("ecm_pcc_test", NULL);
	if (!ecm_pcc_test_dentry) {
		pr_info("Failed to create PCC directory entry\n");
		return -1;
	}
	if (!debugfs_create_file("unregister",
			S_IRUGO | S_IWUSR, ecm_pcc_test_dentry,
			NULL, &ecm_pcc_test_unregister_fops)) {
		pr_info("Failed to create ecm_pcc_test_unregister_fops\n");
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -2;
	}
	if (!debugfs_create_file("rule",
			S_IRUGO | S_IWUSR, ecm_pcc_test_dentry,
			NULL, &ecm_pcc_test_rule_fops)) {
		pr_info("Failed to create ecm_pcc_test_rule_fops\n");
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -3;
	}

	/*
	 * Create our registrant structure
	 */
	ecm_pcc_test_registrant = (struct ecm_classifier_pcc_registrant *)
			kzalloc(sizeof(struct ecm_classifier_pcc_registrant),
				GFP_ATOMIC | __GFP_NOWARN);
	if (!ecm_pcc_test_registrant) {
		pr_info("ECM PCC Failed to alloc registrant\n");
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -4;
	}
	ecm_pcc_test_registrant->version = 1;
	ecm_pcc_test_registrant->this_module = THIS_MODULE;
	ecm_pcc_test_registrant->ref = ecm_pcc_test_registrant_ref;
	ecm_pcc_test_registrant->deref = ecm_pcc_test_registrant_deref;

	/*
	 * If feature flag support is not enabled then fill
	 * okay_to_accel_v4/okay_to_accel_v6 callbacks instead
	 * of get_accel_info_v4/get_accel_info_v6 callbacks in the
	 * registrant's descriptor.
	 */
	if (!feature_flags_support) {
		ecm_pcc_test_registrant->okay_to_accel_v4 =
			ecm_pcc_test_okay_to_accel_v4;
		ecm_pcc_test_registrant->okay_to_accel_v6 =
			ecm_pcc_test_okay_to_accel_v6;
	} else {
		ecm_pcc_test_registrant->get_accel_info_v4 =
			ecm_pcc_test_get_accel_info_v4;
		ecm_pcc_test_registrant->get_accel_info_v6 =
			ecm_pcc_test_get_accel_info_v6;
	}

	/*
	 * Register with the PCC Classifier. ECM classifier will take a ref for
	 * registrant.
	 */
	result = ecm_classifier_pcc_register(ecm_pcc_test_registrant);
	if (result != 0) {
		pr_info("ECM PCC registrant failed to register: %d\n", result);
		kfree(ecm_pcc_test_registrant);
		debugfs_remove_recursive(ecm_pcc_test_dentry);
		return -5;
	}

	pr_info("ECM PCC registrant REGISTERED\n");

	return 0;
}

/*
 * ecm_pcc_test_exit()
 */
static void __exit ecm_pcc_test_exit(void)
{
	pr_info("ECM PCC Test EXIT\n");
	debugfs_remove_recursive(ecm_pcc_test_dentry);
}

module_init(ecm_pcc_test_init)
module_exit(ecm_pcc_test_exit)

MODULE_DESCRIPTION("ECM PCC Test");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

