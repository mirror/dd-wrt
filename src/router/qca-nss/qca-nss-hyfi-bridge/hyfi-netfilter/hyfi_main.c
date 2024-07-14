/*
 *  QCA HyFi Netfilter
 *
 * Copyright (c) 2012-2016, The Linux Foundation. All rights reserved.
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

#define DEBUG_LEVEL HYFI_NF_DEBUG_LEVEL

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include "hyfi_netfilter.h"
#include "hyfi_bridge.h"
#include "hyfi_netlink.h"
#include "hyfi_notify.h"
#include "hyfi_fdb.h"

/* Macro definitions */
#define LKM_AUTHOR          "Hai Shalom and Miaoqing Pan"
#define LKM_DESCRIPTION     "QCA Hy-Fi Bridging"

extern int mc_init(void);
extern void mc_exit(void);

static int __init hyfi_init(void)
{
	int ret;

	/* Initialize the bridge device */
	if ((ret = hyfi_bridge_init())) {
		goto out;
	}

#ifdef HYFI_MULTICAST_SUPPORT
	if ((ret = mc_init())) {
		goto out;
	}
#endif

	if ((ret = hyfi_netfilter_init())) {
		goto out;
	}

	if ((ret = hyfi_notify_init())) {
		goto out;
	}

	if ((ret = hyfi_netlink_init())) {
		goto out;
	}

	out: DEBUG_INFO("QCA Hy-Fi netfilter installation: %s\n",
			ret ? "FAILED" : "OK");

	return ret;
}

static void __exit hyfi_exit(void)
{
	hyfi_netlink_fini();
	hyfi_notify_fini();
	hyfi_netfilter_fini();
#ifdef HYFI_MULTICAST_SUPPORT
	mc_exit();
#endif
	hyfi_bridge_fini();

	DEBUG_INFO("QCA Hy-Fi netfilter uninstalled\n");
}

module_init(hyfi_init);
module_exit(hyfi_exit);

/*
 * Define the module’s license. Important!
 */

MODULE_LICENSE("GPL v2");

/*
 * Optional definitions
 */

MODULE_AUTHOR(LKM_AUTHOR);
/* The author’s name */
MODULE_DESCRIPTION(LKM_DESCRIPTION);
/* The module description */

/*
 * API the module exports to other modules
 */
