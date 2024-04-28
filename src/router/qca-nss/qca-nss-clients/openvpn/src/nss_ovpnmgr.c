/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
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
 * nss_ovpnmgr.c
 */
#include <linux/module.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/crypto.h>
#include <linux/if_tun.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include "nss_ovpnmgr.h"
#include "nss_ovpnmgr_crypto.h"
#include "nss_ovpnmgr_app.h"
#include "nss_ovpnmgr_tun.h"
#include "nss_ovpnmgr_debugfs.h"
#include "nss_ovpnmgr_priv.h"
#include "nss_ovpnmgr_route.h"
#include "nss_ovpn_sk_priv.h"

/*
 * OVPN manager context structure
 */
struct nss_ovpnmgr_context ovpnmgr_ctx;

/*
 * nss_ovpnmgr_init()
 *	Initialize NSS OVPN Manager
 */
int __init nss_ovpnmgr_init(void)
{
	if (!nss_cmn_get_nss_enabled()) {
		nss_ovpnmgr_warn("OVPN Manager is not compatible with this platform\n");
		return -1;
	}

	rwlock_init(&ovpnmgr_ctx.lock);
	INIT_LIST_HEAD(&ovpnmgr_ctx.app_list);

	if (nss_ovpn_sk_init()) {
		nss_ovpnmgr_warn("Failed to initialize socket interface\n");
		return -1;
	}

	if (nss_ovpnmgr_debugfs_init()) {
		nss_ovpnmgr_warn("Debugfs Initialization failed\n");
		nss_ovpn_sk_cleanup();
		return -1;
	}

	tun_register_offload_stats_callback(nss_ovpnmgr_tun_get_stats);
	nss_ovpnmgr_info("OVPN Init successful\n");
	return 0;
}

/*
 * nss_ovpnmgr_exit()
 *	Cleanup NSS OVPN Manager and exit
 */
void __exit nss_ovpnmgr_exit(void)
{
	nss_ovpnmgr_debugfs_cleanup();
	nss_ovpn_sk_cleanup();
	tun_unregister_offload_stats_callback();
	nss_ovpnmgr_info("OVPN Manager Removed\n");
}

module_init(nss_ovpnmgr_init);
module_exit(nss_ovpnmgr_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS OVPN Manager");
