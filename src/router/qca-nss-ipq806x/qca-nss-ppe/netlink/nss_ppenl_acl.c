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
 * nss_ppenl_acl.c
 *	NSS Netlink ACL Handler
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
#include <nss_ppenl_acl_if.h>
#include "nss_ppenl.h"
#include "nss_ppenl_acl.h"

/*
 * prototypes
 */
static int nss_ppenl_acl_ops_create_rule(struct sk_buff *skb, struct genl_info *info);
static int nss_ppenl_acl_ops_destroy_rule(struct sk_buff *skb, struct genl_info *info);

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_ppenl_acl_ops[] = {
	{.cmd = NSS_PPE_ACL_CREATE_RULE_MSG, .doit = nss_ppenl_acl_ops_create_rule,},	/* rule create */
	{.cmd = NSS_PPE_ACL_DESTROY_RULE_MSG, .doit = nss_ppenl_acl_ops_destroy_rule,},	/* rule destroy */
};

/*
 * ACL family definition
 */
static struct genl_family nss_ppenl_acl_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
#endif
	.name = NSS_PPENL_ACL_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_ppenl_acl_rule),	/* NSS NETLINK ACL rule */
	.version = NSS_PPENL_VER,				/* Set it to NSS_PPENL_VER version */
	.maxattr = NSS_PPE_ACL_MAX_MSG_TYPES,		/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_ppenl_acl_ops,
	.n_ops = ARRAY_SIZE(nss_ppenl_acl_ops),
};

#define NSS_PPENL_ACL_OPS_SZ ARRAY_SIZE(nss_ppenl_acl_ops)

static void ppe_acl_rule_dump_rule(struct ppe_acl_rule *rule) {
	nss_ppenl_info("%px rule dump from netlink\n"
			"valid_flag: %d"
			"dev_name: %s\n"
			"smac rule_flags: %d"
			"stype: %d"
			"smac.mac : %pM\n"
			"smac.mac_mask : %pM\n"
			"dmac rule_flags: %d"
			"dmac.mac : %pM\n"
			"dmac.mac_mask : %pM\n"
			"svid.tag_fmt : %d\n"
			"svid.vid_min : %d\n"
			"svid.vid_mask_max : %d\n"
			"cvid.tag_fmt : %d\n"
			"cvid.vid_min : %d\n"
			"cvid.vid_mask_max : %d\n"
			"cpcp.pcp : %d\n"
			"cpcp.pcp_mask : %d\n"
			"spcp.pcp : %d\n"
			"spcp.pcp_mask : %d\n"
			"pppoe_sess_id : %d\n"
			"pppoe_sess_id_mask : %d\n"
			"SIP IPv4 : %pI4\n"
			"SIP IPv6 : %pI6\n",
			rule, rule->valid_flags,
			rule->src.dev_name,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SMAC].rule_flags, rule->stype,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SMAC].rule.smac.mac,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SMAC].rule.smac.mac_mask,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_DMAC].rule_flags,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_DMAC].rule.dmac.mac,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_DMAC].rule.dmac.mac_mask,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SVID].rule.svid.tag_fmt,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SVID].rule.svid.vid_min,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SVID].rule.svid.vid_mask_max,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_CVID].rule.cvid.tag_fmt,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_CVID].rule.cvid.vid_min,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_CVID].rule.cvid.vid_mask_max,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_CPCP].rule.cpcp.pcp,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_CPCP].rule.cpcp.pcp_mask,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SPCP].rule.spcp.pcp,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_SPCP].rule.spcp.pcp_mask,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS].rule.pppoe_sess.pppoe_session_id,
			rule->rules[PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS].rule.pppoe_sess.pppoe_session_id_mask,
			&rule->rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip[0],
			&rule->rules[PPE_ACL_RULE_MATCH_TYPE_SIP].rule.sip.ip);

	nss_ppenl_info("%px: action dump: service_code %d, qid %d\n"
			"enqueue_pri %d, ctag_pcp %d, stag_pcp %d\n"
			"dscp_tc %d, cvid %d, svid %d\n"
			"redir_core %d\n"
			"fwd_cmd %d, flags %d\n",
			rule, rule->action.service_code,
			rule->action.qid,
			rule->action.enqueue_pri,
			rule->action.ctag_pcp,
			rule->action.stag_pcp,
			rule->action.dscp_tc,
			rule->action.cvid,
			rule->action.svid,
			rule->action.redir_core,
			rule->action.fwd_cmd,
			rule->action.flags);
}

/*
 * nss_ippenl_acl_ops_create_rule()
 * 	rule create handler
 */
static int nss_ppenl_acl_ops_create_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_ppenl_acl_rule *nl_acl_rule;
	struct nss_ppenl_cmn *nl_cm;
	struct sk_buff *resp;
	uint32_t pid;
	int error, status = 0;
        ppe_acl_ret_t ret;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_ppenl_get_msg(&nss_ppenl_acl_family, info, NSS_PPE_ACL_CREATE_RULE_MSG);
	if (!nl_cm) {
		nss_ppenl_info("unable to extract rule create data\n");
		nss_ppenl_ucast_resp(skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_acl_rule = container_of(nl_cm, struct nss_ppenl_acl_rule, cm);
	pid = nl_cm->pid;
	nss_ppenl_info("%s: pid: %d\n", __func__, pid);

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

	ppe_acl_rule_dump_rule(&nl_acl_rule->rule);
	status = ppe_acl_rule_create(&nl_acl_rule->rule);
	if (status == PPE_ACL_RET_SUCCESS) {
		nss_ppenl_info("%s: PPE rule create success\n", __func__);
	} else {
		nss_ppenl_info("create rule in ppe driver failed, error = %d\n", status);
	}

	ret = nl_acl_rule->rule.ret;

	/*
	 * Send the response code to user application
	 */
	nl_acl_rule = nss_ppenl_get_data(resp);
	nl_acl_rule->rule.ret = ret;

	nss_ppenl_trace("Sending response to userspace: rule_id %d, ret %d\n", nl_acl_rule->rule.rule_id, nl_acl_rule->rule.ret);
	nss_ppenl_ucast_resp(resp);
	return 0;
}

/*
 * nss_ppenl_ppe_ops_destroy_rule()
 * 	rule delete handler
 */
static int nss_ppenl_acl_ops_destroy_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_ppenl_acl_rule *nl_acl_rule;
	struct nss_ppenl_cmn *nl_cm;
	struct sk_buff *resp;
	uint32_t pid;
	int error, status;
	ppe_acl_ret_t ret;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_ppenl_get_msg(&nss_ppenl_acl_family, info, NSS_PPE_ACL_DESTROY_RULE_MSG);
	if (!nl_cm) {
		nss_ppenl_warn("unable to extract rule destroy data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_acl_rule = container_of(nl_cm, struct nss_ppenl_acl_rule, cm);
	pid = nl_cm->pid;

	/*
	 * copy the NL message for response
	 */
	resp = nss_ppenl_copy_msg(skb);
	if (!resp) {
		nss_ppenl_warn("%d:unable to save response data from NL buffer\n", pid);
		error = -ENOMEM;
		goto done;
	}

	status = ppe_acl_rule_destroy(nl_acl_rule->rule.rule_id);
	if (status != PPE_ACL_RET_SUCCESS) {
		nss_ppenl_warn("unable to create rule in ppe driver, error = %d\n", status);
		return -EINVAL;
	}

	ret = nl_acl_rule->rule.ret;

	/*
	 * Send the response back to user application
	 */
	nl_acl_rule = nss_ppenl_get_data(resp);
	nl_acl_rule->rule.ret = ret;

	nss_ppenl_trace("Sending response to userspace: rule_id %d, ret %d\n", nl_acl_rule->rule.rule_id, nl_acl_rule->rule.ret);
	nss_ppenl_ucast_resp(resp);
	return 0;
done:
	return error;
}

/*
 * nss_ppenl_acl_init()
 * 	handler init
 */
bool nss_ppenl_acl_init(void)
{
	int error;

	nss_ppenl_info("Init NSS PPE ACL handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	error = genl_register_family(&nss_ppenl_acl_family);
	if (error != 0) {
		nss_ppenl_info_always("Error: unable to register ACL family\n");
		return false;
	}

	nss_ppenl_info("size of the msg in netlink = %d\n", (uint32_t)sizeof(struct nss_ppenl_acl_rule));
	return true;
}

/*
 * nss_ppenl_acl_exit()
 *	handler exit
 */
bool nss_ppenl_acl_exit(void)
{
	int error;

	nss_ppenl_info("Exit NSS netlink ACL handler\n");

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_ppenl_acl_family);
	if (error != 0) {
		nss_ppenl_info_always("unable to unregister ACL NETLINK family\n");
		return false;
	}

	return true;
}
