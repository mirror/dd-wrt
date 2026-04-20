/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * nss_nledma.c
 *	NSS Netlink Edma Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netlink.h>
#include <linux/vmalloc.h>

#include <net/genetlink.h>
#include <net/sock.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nledma_if.h"
#include "nss_edma.h"

/*
 * prototypes
 */
static int nss_nledma_ops_get_stats(struct sk_buff *skb, struct genl_info *info);
static int nss_nledma_process_notify(struct notifier_block *nb, unsigned long val, void *data);

/*
 * multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nledma_mcgrp[] = {
	{.name = NSS_NLEDMA_MCAST_GRP},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nledma_ops[] = {
	{.cmd = NSS_STATS_EVENT_NOTIFY, .doit = nss_nledma_ops_get_stats,},
};

/*
 * Edma family definition
 */
static struct genl_family nss_nledma_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
#endif
	.name = NSS_NLEDMA_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nledma_stats),	/* NSS NETLINK Edma stats */
	.version = NSS_NL_VER,				/* Set it to NSS_NLEDMA version */
	.maxattr = NSS_STATS_EVENT_MAX,			/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nledma_ops,
	.n_ops = ARRAY_SIZE(nss_nledma_ops),
	.mcgrps = nss_nledma_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nledma_mcgrp)
};

/*
 * statistics call back handler for edma from NSS
 */
static struct notifier_block nss_edma_stats_notifier_nb = {
	.notifier_call = nss_nledma_process_notify,
};

#define NSS_NLEDMA_OPS_SZ ARRAY_SIZE(nss_nledma_ops)

/*
 * nss_nledma_ops_get_stats()
 *	get stats handler
 */
static int nss_nledma_ops_get_stats(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

/*
 * nss_nledma_process_notify()
 *	process notification messages from NSS
 */
static int nss_nledma_process_notify(struct notifier_block *nb, unsigned long val, void *data)
{
	struct nss_nledma_stats *stats;
	struct sk_buff *skb;
	int id;

	for (id = 0 ; id < NSS_EDMA_NUM_PORTS_MAX; id++) {
		skb = nss_nl_new_msg(&nss_nledma_family, NSS_NLCMN_SUBSYS_EDMA);
		if (!skb) {
			nss_nl_error("unable to allocate NSS_NLEDMA event\n");
			return NOTIFY_DONE;
		}

		stats = nss_nl_get_data(skb);
		nss_edma_get_stats(stats->cmn_node_stats,id);
		stats->core_id = *(uint32_t *)data;
		stats->port_id = id;
		nss_nl_mcast_event(&nss_nledma_family, skb);
	}

	return NOTIFY_DONE;
}

/*
 * nss_nledma_init()
 *	handler init
 */
bool nss_nledma_init(void)
{
	int error, ret;

	nss_nl_info_always("Init NSS netlink Edma handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	error = genl_register_family(&nss_nledma_family);
	if (error) {
		nss_nl_info_always("Error: unable to register Edma family\n");
		return false;
	}

	/*
	 * register device call back handler for edma from NSS
	 */
	ret = nss_edma_stats_register_notifier(&nss_edma_stats_notifier_nb);
	if (ret) {
		nss_nl_info_always("Error: retreiving the NSS Context\n");
		genl_unregister_family(&nss_nledma_family);
		return false;
	}

	return true;
}

/*
 * nss_nledma_exit()
 *	handler exit
 */
bool nss_nledma_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink Edma handler\n");

	/*
	 * Unregister the device callback handler for edma
	 */
	nss_edma_stats_unregister_notifier(&nss_edma_stats_notifier_nb);

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nledma_family);
	if (error) {
		nss_nl_info_always("unable to unregister Edma NETLINK family\n");
		return false;
	}

	return true;
}
