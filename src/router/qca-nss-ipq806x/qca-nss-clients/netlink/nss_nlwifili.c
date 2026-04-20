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
 * nss_nlwifili.c
 *	NSS Netlink wifili Handler
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
#include "nss_nlwifili_if.h"
#include "nss_wifili_if.h"

/*
 * prototypes
 */
static int nss_nlwifili_ops_get_stats(struct sk_buff *skb, struct genl_info *info);
static int nss_nlwifili_process_notify(struct notifier_block *nb, unsigned long val, void *data);

/*
 * multicast group for sending message status & events
 */
static const struct genl_multicast_group nss_nlwifili_mcgrp[] = {
	{.name = NSS_NLWIFILI_MCAST_GRP},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlwifili_ops[] = {
	{.cmd = NSS_STATS_EVENT_NOTIFY, .doit = nss_nlwifili_ops_get_stats},
};

/*
 * wifili family definition
 */
static struct genl_family nss_nlwifili_family = {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 9, 0))
	.id = GENL_ID_GENERATE,						/* Auto generate ID */
#endif
	.name = NSS_NLWIFILI_FAMILY,					/* family name string */
	.hdrsize = sizeof(struct nss_wifili_stats_notification),	/* NSS NETLINK wifili stats */
	.version = NSS_NL_VER,						/* Set it to NSS_NLWIFILI version */
	.maxattr = NSS_STATS_EVENT_MAX,					/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
	.ops = nss_nlwifili_ops,
	.n_ops = ARRAY_SIZE(nss_nlwifili_ops),
	.mcgrps = nss_nlwifili_mcgrp,
	.n_mcgrps = ARRAY_SIZE(nss_nlwifili_mcgrp)
};

/*
 * device call back handler for wifili from NSS
 */
static struct notifier_block nss_wifili_stats_notifier_nb = {
	.notifier_call = nss_nlwifili_process_notify,
};

/*
 * nss_nlwifili_ops_get_stats()
 *	get stats handler
 */
static int nss_nlwifili_ops_get_stats(struct sk_buff *skb, struct genl_info *info)
{
	return 0;
}

/*
 * nss_nlwifili_process_notify()
 *	process notification messages from NSS
 */
static int nss_nlwifili_process_notify(struct notifier_block *nb, unsigned long val, void *data)
{
	struct sk_buff *skb;
	struct nss_wifili_stats_notification *nss_stats, *nl_stats;

	nss_stats = (struct nss_wifili_stats_notification *)data;
	skb = nss_nl_new_msg(&nss_nlwifili_family, NSS_NLCMN_SUBSYS_WIFILI);
	if (!skb) {
		nss_nl_error("unable to allocate NSS_NLWIFILI event\n");
		return NOTIFY_DONE;
	}

	nl_stats = nss_nl_get_data(skb);
	memcpy(nl_stats, nss_stats, sizeof(struct nss_wifili_stats_notification));
	nss_nl_mcast_event(&nss_nlwifili_family, skb);

	return NOTIFY_DONE;
}

/*
 * nss_nlwifili_init()
 *	handler init
 */
bool nss_nlwifili_init(void)
{
	int error,ret;

	nss_nl_info_always("Init NSS netlink wifili handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	error = genl_register_family(&nss_nlwifili_family);
	if (error) {
		nss_nl_info_always("Error: unable to register wifili family\n");
		return false;
	}

	/*
	 * register device call back handler for wifili from NSS
	 */
	ret = nss_wifili_stats_register_notifier(&nss_wifili_stats_notifier_nb);
	if (ret) {
		nss_nl_info_always("Error: retreiving the NSS Context\n");
		genl_unregister_family(&nss_nlwifili_family);
		return false;
	}

	return true;
}

/*
 * nss_nlwifili_exit()
 *	handler exit
 */
bool nss_nlwifili_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink wifili handler\n");

	/*
	 * Unregister the device callback handler for wifili
	 */
	nss_wifili_stats_unregister_notifier(&nss_wifili_stats_notifier_nb);

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nlwifili_family);
	if (error) {
		nss_nl_info_always("unable to unregister wifili NETLINK family\n");
		return false;
	}

	return true;
}
