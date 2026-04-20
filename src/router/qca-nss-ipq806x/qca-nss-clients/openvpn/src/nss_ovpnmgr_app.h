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
 * nss_ovpnmgr_app.h
 */
#ifndef __NSS_OVPNMGR_APP__H
#define __NSS_OVPNMGR_APP__H

struct nss_ovpnmgr_tun;

/*
 * nss_ovpnmgr_app
 */
struct nss_ovpnmgr_app {
	struct list_head list;		/* List node to add into list of application. */
	struct list_head tun_list;	/* List head to maintain OVPN Tunnels. */
	enum nss_ovpnmgr_app_mode mode;	/* Application mode - server/client. */
	struct net_device *dev;		/* TUN/TAP netdevice. */
	void *app_data;			/* Application data. */
	struct dentry *dentry;		/* Debugfs entry for OVPN Tunnels created by the application. */
};

struct nss_ovpnmgr_app *nss_ovpnmgr_app_find(struct net_device *app_dev);
void nss_ovpnmgr_app_forward(struct nss_ovpnmgr_app *app, struct nss_ovpnmgr_tun *tun, struct sk_buff *skb);
#endif /* __NSS_OVPNMGR_APP__H */
