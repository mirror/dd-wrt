/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/*
 * nss_nlqrfs.c
 *	NSS Netlink qrfs Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#include <net/genetlink.h>
#include <net/sock.h>
#include <net/arp.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nlqrfs_if.h"
#include <nss_qrfs.h>

/*
 * prototypes
 */
static int nss_nlqrfs_ops_add_rule(struct sk_buff *skb, struct genl_info *info);
static int nss_nlqrfs_ops_del_rule(struct sk_buff *skb, struct genl_info *info);

/*
 * multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlqrfs_mcgrp[] = {
	{.name = NSS_NLQRFS_MCAST_GRP},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlqrfs_ops[] = {
	{.cmd = NSS_QRFS_MSG_FLOW_ADD, .doit = nss_nlqrfs_ops_add_rule,},
	{.cmd = NSS_QRFS_MSG_FLOW_DELETE, .doit = nss_nlqrfs_ops_del_rule,},
};

/*
 * qrfs family definition
 */
static struct genl_family nss_nlqrfs_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,						/* Auto generate ID */
#endif
	.name = NSS_NLQRFS_FAMILY,					/* family name string */
	.hdrsize = sizeof(struct nss_nlqrfs_rule),			/* NSS NETLINK qrfs rule */
	.version = NSS_NL_VER,						/* Set it to NSS_NLQRFS version */
	.maxattr = NSS_QRFS_MSG_MAX,					/* Maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nlqrfs_ops,
	.n_ops = ARRAY_SIZE(nss_nlqrfs_ops),
	.mcgrps = nss_nlqrfs_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nlqrfs_mcgrp)
};

/*
 * nss_nlqrfs_ops_cfg_rule
 *	Handler for unconfiguring rules
 */
static int nss_nlqrfs_ops_cfg_rule(struct sk_buff *skb, struct genl_info *info, bool action)
{
	struct nss_nlqrfs_rule *nl_rule;
	struct nss_qrfs_flow_rule_msg *nrm;
	struct nss_nlcmn *nl_cm;
	int ret = 0;

	/*
	 * Extract the message payload
	 */
	if (action) {
		nl_cm = nss_nl_get_msg(&nss_nlqrfs_family, info, NSS_QRFS_MSG_FLOW_ADD);
		nss_nl_info("add flow rule\n");
	} else {
		nl_cm = nss_nl_get_msg(&nss_nlqrfs_family, info, NSS_QRFS_MSG_FLOW_DELETE);
		nss_nl_info("delete flow rule\n");
	}

	if (!nl_cm) {
		nss_nl_error("%px: Unable to extract configure rule data\n", skb);
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlqrfs_rule, cm);
	nrm = &nl_rule->msg;

	if (nrm->ip_version == 4) {
		nss_nl_trace("src_ip:%pl4h src_port:%u dst_ip:%pl4h dst_port:%u protocol:%u version:%u cpu:%u\n",
				nrm->src_addr, nrm->src_port, nrm->dst_addr, nrm->dst_port, nrm->protocol,
				nrm->ip_version, nrm->cpu);
	} else if (nrm->ip_version == 6) {
		nss_nl_trace("src_ip:%pl6 src_port:%u dst_ip:%pl6 dst_port:%u protocol:%u version:%u cpu:%u\n",
				nrm->src_addr, nrm->src_port, nrm->dst_addr, nrm->dst_port, nrm->protocol,
				nrm->ip_version, nrm->cpu);
	} else {
		nss_nl_trace("Unsupported IP version field\n");
		return -EINVAL;
	}

	if (action) {
		nss_qrfs_configure_flow_rule(nrm->dst_addr, nrm->src_addr, nrm->dst_port, nrm->src_port,
				nrm->ip_version, nrm->protocol, nrm->cpu, NSS_QRFS_MSG_FLOW_ADD);
	} else {
		nss_qrfs_configure_flow_rule(nrm->dst_addr, nrm->src_addr, nrm->dst_port, nrm->src_port,
				nrm->ip_version, nrm->protocol, nrm->cpu, NSS_QRFS_MSG_FLOW_DELETE);
	}

	nss_nl_trace("%s flow rule finished\n", action? "add" : "delete");

	return ret;
}

/*
 * nss_nlqrfs_ops_add_rule()
 *	Handler for Adding rules
 */
static int nss_nlqrfs_ops_add_rule(struct sk_buff *skb, struct genl_info *info)
{
	return nss_nlqrfs_ops_cfg_rule(skb, info, true);
}

/*
 * nss_nlqrfs_ops_del_rule()
 *	Handler for deleting rules
 */
static int nss_nlqrfs_ops_del_rule(struct sk_buff *skb, struct genl_info *info)
{
	return nss_nlqrfs_ops_cfg_rule(skb, info, false);
}

/*
 * nss_nlqrfs_exit()
 *	handler exit
 */
bool nss_nlqrfs_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink qrfs handler\n");

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nlqrfs_family);
	if (error) {
		nss_nl_info_always("unable to unregister qrfs NETLINK family\n");
		return false;
	}

	return true;
}

/*
 * nss_nlqrfs_init()
 *	handler init
 */
bool nss_nlqrfs_init(void)
{
	int error;

	nss_nl_info_always("Init NSS netlink qrfs handler\n");

	/*
	 * register Netlink ops with the family
	 */
	error = genl_register_family(&nss_nlqrfs_family);
	if (error) {
		nss_nl_info_always("Error: unable to register qrfs family\n");
		return false;
	}

	return true;
}
