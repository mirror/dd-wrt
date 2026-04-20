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
 * nss_nlipv6_reasm.c
 *	NSS Netlink ipv6_reasm Handler
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/netlink.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/notifier.h>

#include <net/genetlink.h>
#include <net/sock.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nlcmn_if.h"
#include "nss_nlipv6_reasm_if.h"
#include "nss_ipv6_reasm.h"

/*
 * prototypes
 */
static int nss_nlipv6_reasm_ops_get_stats(struct sk_buff *skb, struct genl_info *info);
static int nss_nlipv6_reasm_process_notify(struct notifier_block *nb,  unsigned long val, void *data);

/*
 * multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlipv6_reasm_mcgrp[] = {
	{.name = NSS_NLIPV6_REASM_MCAST_GRP},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlipv6_reasm_ops[] = {
	{.cmd = NSS_STATS_EVENT_NOTIFY, .doit = nss_nlipv6_reasm_ops_get_stats},
};

/*
 * ipv6_reasm family definition
 */
static struct genl_family nss_nlipv6_reasm_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,						/* Auto generate ID */
#endif
	.name = NSS_NLIPV6_REASM_FAMILY,				/* family name string */
	.hdrsize = sizeof(struct nss_ipv6_reasm_stats_notification),	/* NSS NETLINK ipv6_reasm stats */
	.version = NSS_NL_VER,						/* Set it to NSS_NLIPV6_REASM version */
	.maxattr = NSS_STATS_EVENT_MAX,					/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nlipv6_reasm_ops,
	.n_ops = ARRAY_SIZE(nss_nlipv6_reasm_ops),
	.mcgrps = nss_nlipv6_reasm_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nlipv6_reasm_mcgrp)
};

/*
 * stats call back handler for ipv6_reasm from NSS
 */
static struct notifier_block nss_ipv6_reasm_stats_notifier_nb = {
	.notifier_call = nss_nlipv6_reasm_process_notify,
};

/*
 * nss_nlipv6_reasm_ops_get_stats()
 *	get stats handler
 */
static int nss_nlipv6_reasm_ops_get_stats(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

/*
 * nss_nlipv6_reasm_process_notify()
 *	process notification messages from NSS
 */
static int nss_nlipv6_reasm_process_notify(struct notifier_block *nb, unsigned long val, void *data)
{
	struct sk_buff *skb;
	struct nss_ipv6_reasm_stats_notification *nss_stats, *nl_stats;

	skb = nss_nl_new_msg(&nss_nlipv6_reasm_family, NSS_NLCMN_SUBSYS_IPV6_REASM);
	if (!skb) {
		nss_nl_error("unable to allocate NSS_NLIPV6_REASM event\n");
		return NOTIFY_DONE;
	}

	nl_stats = nss_nl_get_data(skb);
	nss_stats = (struct nss_ipv6_reasm_stats_notification *)data;
	memcpy(nl_stats, nss_stats, sizeof(struct nss_ipv6_reasm_stats_notification));
	nss_nl_mcast_event(&nss_nlipv6_reasm_family, skb);

	return NOTIFY_DONE;
}

/*
 * nss_nlipv6_reasm_init()
 *	handler init
 */
bool nss_nlipv6_reasm_init(void)
{
	int error,ret;

	nss_nl_info_always("Init NSS netlink ipv6_reasm handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	error = genl_register_family(&nss_nlipv6_reasm_family);
	if (error) {
		nss_nl_info_always("Error: unable to register ipv6_reasm family\n");
		return false;
	}

	/*
	 * register device call back handler for ipv6_reasm from NSS
	 */
	ret = nss_ipv6_reasm_stats_register_notifier(&nss_ipv6_reasm_stats_notifier_nb);
	if (ret) {
		nss_nl_info_always("Error: retreiving the NSS Context\n");
		genl_unregister_family(&nss_nlipv6_reasm_family);
		return false;
	}

	return true;
}

/*
 * nss_nlipv6_reasm_exit()
 *	handler exit
 */
bool nss_nlipv6_reasm_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink ipv6_reasm handler\n");

	/*
	 * Unregister the device callback handler for ipv6_reasm
	 */
	nss_ipv6_reasm_stats_unregister_notifier(&nss_ipv6_reasm_stats_notifier_nb);

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nlipv6_reasm_family);
	if (error) {
		nss_nl_info_always("unable to unregister ipv6_reasm NETLINK family\n");
		return false;
	}

	return true;
}
