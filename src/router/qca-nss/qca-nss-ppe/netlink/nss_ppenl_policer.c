/*
 * Copyright (c) 2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

/*
 * nss_ppenl_policer.c
 * NSS Netlink POLICER Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/netlink.h>
#include <linux/rcupdate.h>
#include <linux/etherdevice.h>
#include <linux/if_addr.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/if_vlan.h>
#include <linux/completion.h>
#include <linux/semaphore.h>
#include <linux/in.h>

#include <net/arp.h>
#include <net/genetlink.h>
#include <net/neighbour.h>
#include <net/net_namespace.h>
#include <net/route.h>
#include <net/sock.h>

#include <nss_ppenl_cmn_if.h>
#include <nss_ppenl_policer_if.h>
#include "nss_ppenl.h"
#include "nss_ppenl_policer.h"
#include <ppe_policer.h>

/*
 * prototypes
 */
static int nss_ppenl_policer_ops_create_rule(struct sk_buff *skb, struct genl_info *info);
static int nss_ppenl_policer_ops_destroy_rule(struct sk_buff *skb, struct genl_info *info);

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_ppenl_policer_ops[] = {
	{.cmd = NSS_PPE_POLICER_CREATE_RULE_MSG, .doit = nss_ppenl_policer_ops_create_rule,},	/* rule create */
	{.cmd = NSS_PPE_POLICER_DESTROY_RULE_MSG, .doit = nss_ppenl_policer_ops_destroy_rule,},	/* rule destroy */
};

/*
 * POLICER family definition
 */
static struct genl_family nss_ppenl_policer_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,	/* Auto generate ID */
#endif
	.name = NSS_PPENL_POLICER_FAMILY,	/* family name string */
	.hdrsize = sizeof(struct nss_ppenl_policer_rule),	/* NSS NETLINK Policer rule */
	.version = NSS_PPENL_VER,	/* Set it to NSS_PPENL_VER version */
	.maxattr = NSS_PPE_POLICER_MAX_MSG_TYPES,	/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_ppenl_policer_ops,
	.n_ops = ARRAY_SIZE(nss_ppenl_policer_ops),
};

#define NSS_PPENL_POLICER_OPS_SZ ARRAY_SIZE(nss_ppenl_policer_ops)

/*
 * nss_ppenl_policer_ops_create_rule()
 * rule create handler
 */
static int nss_ppenl_policer_ops_create_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_ppenl_policer_rule *nl_policer_rule;
	struct nss_ppenl_cmn *nl_cm;
	struct sk_buff *resp;
	uint32_t pid;
	int error;
	enum ppe_policer_ret pt;
	struct ppe_policer_create_info create = {0};

	/*
	 * extract the message payload
	 */
	nl_cm = nss_ppenl_get_msg(&nss_ppenl_policer_family, info, NSS_PPE_POLICER_CREATE_RULE_MSG);
	if (!nl_cm) {
		nss_ppenl_info("unable to extract rule create data\n");
		nss_ppenl_ucast_resp(skb);
		return -EINVAL;
	}

	/*
	 * Validate config message before calling rule API
	 */
	nl_policer_rule = container_of(nl_cm, struct nss_ppenl_policer_rule, cm);
	pid = nl_cm->pid;

	create.policer_type = nl_policer_rule->config.is_port_policer;
	create.rule_id = nl_policer_rule->config.policer_id;
	create.config.committed_rate = nl_policer_rule->config.committed_rate;
	create.config.committed_burst_size = nl_policer_rule->config.committed_burst_size;
	create.config.peak_rate = nl_policer_rule->config.peak_rate;
	create.config.peak_burst_size = nl_policer_rule->config.peak_burst_size;
	if (create.policer_type) {
		memcpy(&create.name, nl_policer_rule->config.dev, sizeof(nl_policer_rule->config.dev));
	}

	if (!nl_policer_rule->config.meter_enable) {
		create.config.meter_enable = false;
	} else {
		create.config.meter_enable = true;
	}

	if (!nl_policer_rule->config.couple_enable) {
		create.config.couple_enable = false;
	} else {
		create.config.couple_enable = true;
	}

	if (!nl_policer_rule->config.colour_aware) {
		create.config.colour_aware = true;
	} else {
		create.config.colour_aware = false;
	}

	create.config.action_info.yellow_pri = nl_policer_rule->config.action_info.yellow_int_pri;
	create.config.action_info.yellow_dp = nl_policer_rule->config.action_info.yellow_dp;
	create.config.action_info.yellow_pcp = nl_policer_rule->config.action_info.yellow_pcp;
	create.config.action_info.yellow_dei = nl_policer_rule->config.action_info.yellow_dei;
	create.config.mode = nl_policer_rule->config.meter_mode;
	create.config.meter_unit = nl_policer_rule->config.meter_unit;
	if(!create.policer_type) {
		create.config.action_info.yellow_dscp = nl_policer_rule->config.action_info.yellow_dscp;
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_ppenl_copy_msg(skb);
	if (!resp) {
		nss_ppenl_info("%d:unable to save response data from NL buffer\n", pid);
		error = -ENOMEM;
		nss_ppenl_ucast_resp(skb);
		return error;
	}

	pt = ppe_policer_create(&create);
	if (pt == PPE_POLICER_SUCCESS || pt == PPE_POLICER_ACL_RET_CREATE_SUCCESS) {
		nss_ppenl_info("PPE rule create success");
	} else {
		nss_ppenl_info("create rule in ppe driver failed, error = %d", pt);
	}

	nl_policer_rule = nss_ppenl_get_data(resp);
	nl_policer_rule->config.ret = pt;
	nss_ppenl_ucast_resp(resp);
	return 0;
}

/*
 * nss_ppenl_policer_ops_destroy_rule()
 * rule delete handler
 */
static int nss_ppenl_policer_ops_destroy_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_ppenl_policer_rule *nl_policer_rule;
	struct nss_ppenl_cmn *nl_cm;
	struct sk_buff *resp;
	uint32_t pid;
	int error;
	enum ppe_policer_ret pt;
	struct ppe_policer_destroy_info destroy = {0};

	/*
	 * extract the message payload
	 */
	nl_cm = nss_ppenl_get_msg(&nss_ppenl_policer_family, info, NSS_PPE_POLICER_DESTROY_RULE_MSG);
	if (!nl_cm) {
		nss_ppenl_info("unable to extract rule destroy data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_policer_rule = container_of(nl_cm, struct nss_ppenl_policer_rule, cm);
	pid = nl_cm->pid;

	destroy.rule_id = nl_policer_rule->config.policer_id;
	destroy.policer_type = nl_policer_rule->config.is_port_policer;
	if (destroy.policer_type) {
		memcpy(&destroy.name, nl_policer_rule->config.dev, sizeof(nl_policer_rule->config.dev));
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_ppenl_copy_msg(skb);
	if (!resp) {
		nss_ppenl_info("%d:unable to save response data from NL buffer\n", pid);
		error = -ENOMEM;
		goto done;
	}

	pt = ppe_policer_destroy(&destroy);
	if (pt == PPE_POLICER_SUCCESS) {
		nss_ppenl_info("PPE rule destroy success");
	} else {
		nss_ppenl_info("Delete rule in ppe driver failed, error = %d",pt);
	}

	nl_policer_rule = nss_ppenl_get_data(resp);
	nl_policer_rule->config.ret = pt;
	nss_ppenl_ucast_resp(resp);
	return 0;
done:
	return error;
}

/*
 * nss_ppenl_policer_init()
 *  handler init
 */
bool nss_ppenl_policer_init(void)
{
	int error;

	nss_ppenl_info("Init NSS PPE POLICER handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	error = genl_register_family(&nss_ppenl_policer_family);
	if (error != 0) {
		nss_ppenl_info_always("Error: unable to register ACL family\n");
		return false;
	}

	return true;
}

/*
 * nss_ppenl_policer_exit()
 * handler exit
 */
bool nss_ppenl_policer_exit(void)
{
	int error;

	nss_ppenl_info("Exit NSS netlink POLICER handler\n");

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_ppenl_policer_family);
	if (error != 0) {
		nss_ppenl_info_always("unable to unregister POLICER NETLINK family\n");
		return false;
	}

	return true;
}
