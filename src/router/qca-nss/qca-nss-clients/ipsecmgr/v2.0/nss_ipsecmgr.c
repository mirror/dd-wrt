/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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

/* nss_ipsecmgr.c
 *	IPSec Manager
 */
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/bitops.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/etherdevice.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <linux/atomic.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <net/esp.h>
#include <net/xfrm.h>
#include <net/icmp.h>

#include <crypto/aead.h>
#include <crypto/skcipher.h>
#include <crypto/internal/hash.h>

#include <nss_api_if.h>
#include <nss_ipsec_cmn.h>
#include <nss_ipsecmgr.h>
#include <nss_cryptoapi.h>

#ifdef NSS_IPSECMGR_PPE_SUPPORT
#include <ref/ref_vsi.h>
#endif

#include "nss_ipsecmgr_ref.h"
#include "nss_ipsecmgr_flow.h"
#include "nss_ipsecmgr_sa.h"
#include "nss_ipsecmgr_ctx.h"
#include "nss_ipsecmgr_tunnel.h"
#include "nss_ipsecmgr_priv.h"

bool enable_ipsec_inline = false;
module_param(enable_ipsec_inline, bool, S_IRUGO);
MODULE_PARM_DESC(enable_ipsec_inline, "Enable IPsec Inline mode");

struct nss_ipsecmgr_drv *ipsecmgr_drv;

static const struct net_device_ops nss_ipsecmgr_dummy_ndev_ops;

/*
 * nss_ipsecmgr_dummy_setup()
 *	Setup function for dummy netdevice.
 */
static void nss_ipsecmgr_dummy_setup(struct net_device *dev)
{
	/*
	 * Since, we want to start with fragmentation post IPsec
	 * transform.
	 */
	dev->mtu = ETH_DATA_LEN;
}

/*
 * nss_ipsecmgr_rx_notify()
 *	RX notification on base node
 */
static void nss_ipsecmgr_rx_notify(void *app_data, struct nss_cmn_msg *ncm)
{
	struct net_device *dev = app_data;
	nss_ipsecmgr_warn("%px: Notification received on base node", netdev_priv(dev));
}

/*
 * nss_ipsecmgr_configure()
 *	Send the configure node message
 */
static void nss_ipsecmgr_configure(struct work_struct *work)
{
	enum nss_ipsec_cmn_msg_type type = NSS_IPSEC_CMN_MSG_TYPE_NODE_CONFIG;
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(ipsecmgr_drv->dev);
	uint32_t ifnum = ipsecmgr_drv->ifnum;
	struct nss_ipsec_cmn_msg nicm = {0};
	struct nss_ipsecmgr_ctx *redir;
	nss_tx_status_t status;
	uint32_t vsi_num = 0;

	/*
	 * By making sure that cryptoapi is registered,
	 * we are confirming that IPsec FW is initialized
	 * and ready to be configured.
	 */
	if (!nss_cryptoapi_is_registered()) {
		schedule_delayed_work(&ipsecmgr_drv->cfg_work, NSS_IPSECMGR_NODE_CONFIG_RETRY_TIMEOUT);
		return;
	}

#ifdef NSS_IPSECMGR_DMA_SUPPORT
	nicm.msg.node.dma_lookaside = true;
	nicm.msg.node.dma_redirect = ipsecmgr_drv->ipsec_inline;
#else
	nicm.msg.node.dma_lookaside = false;
	nicm.msg.node.dma_redirect = false;
#endif

	/*
	 * Send DMA IPsec message to initialize the DMA rings.
	 */
	status = nss_ipsec_cmn_tx_msg_sync(ipsecmgr_drv->nss_ctx, ifnum, type, sizeof(nicm.msg.node), &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_trace("%px: failed to configure IPsec in NSS(%u)", ipsecmgr_drv, status);
		/*
		 * TODO: We need unwind in case of failure
		 */
		return;
	}

	/*
	 * Program PPE for inline mode; if inline is enabled.
	 */
	if (ipsecmgr_drv->ipsec_inline) {

#ifdef NSS_IPSECMGR_PPE_SUPPORT
		redir = nss_ipsecmgr_ctx_alloc(tun,
						NSS_IPSEC_CMN_CTX_TYPE_REDIR,
						NSS_DYNAMIC_INTERFACE_TYPE_IPSEC_CMN_REDIRECT,
						nss_ipsecmgr_ctx_rx_redir,
						nss_ipsecmgr_ctx_rx_stats,
						0);
		if (!redir) {
			nss_ipsecmgr_warn("%px: failed to allocate redirect context; disabling inline", tun);
			ipsecmgr_drv->ipsec_inline = false;
			return;
		}

		nss_ipsecmgr_ctx_set_except(redir, redir->ifnum);

		if (!nss_ipsecmgr_ctx_config(redir)) {
			nss_ipsecmgr_warn("%px: failed to configure redirect context; disabling inline", tun);
			ipsecmgr_drv->ipsec_inline = false;
			nss_ipsecmgr_ctx_free(redir);
			return;
		}

		/*
		 * Get port's default VSI.
		 */
		if (ppe_port_vsi_get(0, NSS_PPE_PORT_IPSEC, &vsi_num)) {
			nss_ipsecmgr_warn("%px: Failed to get port VSI", ipsecmgr_drv);
			nss_ipsecmgr_ctx_free(redir);
			ipsecmgr_drv->ipsec_inline = false;
			return;
		}

		/*
		 * Configure PPE's inline port; Configure the redirect interface to receive
		 * exception packets from inline path
		 */
		if (!nss_ipsec_cmn_ppe_port_config(ipsecmgr_drv->nss_ctx, ipsecmgr_drv->dev, redir->ifnum, vsi_num)) {
			nss_ipsecmgr_warn("%px: Failed to configure PPE inline mode", ipsecmgr_drv);
			nss_ipsecmgr_ctx_free(redir);
			ipsecmgr_drv->ipsec_inline = false;
			return;
		}

		nss_ipsecmgr_ctx_attach(&tun->ctx_db, redir);

		debugfs_create_file("redir", S_IRUGO, tun->dentry, redir, &ipsecmgr_ctx_file_ops);
#endif
	}

	nss_ipsecmgr_trace("%px: Configure node msg successful", ipsecmgr_drv);
	return;
}

/*
 * nss_ipsecmgr_init()
 *	module init
 */
static int __init nss_ipsecmgr_init(void)
{
	struct nss_ipsecmgr_tunnel *tun;
	struct net_device *dev;
	int status;

	ipsecmgr_drv = vzalloc(sizeof(*ipsecmgr_drv));
	if (!ipsecmgr_drv) {
		nss_ipsecmgr_warn("Failed to allocate IPsec manager context");
		return -1;
	}

	ipsecmgr_drv->nss_ctx = nss_ipsec_cmn_get_context();
	if (!ipsecmgr_drv->nss_ctx) {
		nss_ipsecmgr_warn("%px: Failed to retrieve NSS context", ipsecmgr_drv);
		goto free;
	}

#ifdef NSS_IPSECMGR_PPE_SUPPORT
	ipsecmgr_drv->ipsec_inline = enable_ipsec_inline;
#endif

	dev = alloc_netdev(sizeof(*tun), NSS_IPSECMGR_DEFAULT_TUN_NAME, NET_NAME_UNKNOWN, nss_ipsecmgr_dummy_setup);
	if (!dev) {
		nss_ipsecmgr_warn("%px: Failed to allocate dummy netdevice", ipsecmgr_drv);
		goto free;
	}

	tun = netdev_priv(dev);
	tun->dev = dev;

	nss_ipsecmgr_ref_init(&tun->ref, NULL, NULL);
	INIT_LIST_HEAD(&tun->list);
	INIT_LIST_HEAD(&tun->free_refs);

	dev->netdev_ops = &nss_ipsecmgr_dummy_ndev_ops;
	nss_ipsecmgr_db_init(&tun->ctx_db);

	status = register_netdev(dev);
	if (status) {
		nss_ipsecmgr_info("%px: Failed to register dummy netdevice(%px)", ipsecmgr_drv, dev);
		goto netdev_free;
	}

	ipsecmgr_drv->dev = dev;
	ipsecmgr_drv->ifnum = NSS_IPSEC_CMN_INTERFACE;

	rwlock_init(&ipsecmgr_drv->lock);
	nss_ipsecmgr_db_init_entries(ipsecmgr_drv->sa_db, NSS_IPSECMGR_SA_MAX);
	nss_ipsecmgr_db_init_entries(ipsecmgr_drv->flow_db, NSS_IPSECMGR_FLOW_MAX);
	nss_ipsecmgr_db_init(&ipsecmgr_drv->tun_db);

	nss_ipsec_cmn_notify_register(ipsecmgr_drv->ifnum, nss_ipsecmgr_rx_notify, ipsecmgr_drv->dev);
	INIT_DELAYED_WORK(&ipsecmgr_drv->cfg_work, nss_ipsecmgr_configure);

	/*
	 * Initialize debugfs.
	 */
	ipsecmgr_drv->dentry = debugfs_create_dir("qca-nss-ipsecmgr", NULL);
	if (!ipsecmgr_drv->dentry) {
		nss_ipsecmgr_warn("%px: Failed to create root debugfs entry", ipsecmgr_drv);
		nss_ipsec_cmn_notify_unregister(ipsecmgr_drv->nss_ctx, ipsecmgr_drv->ifnum);
		goto unregister_dev;
	}

	/*
	 * Create debugfs entry for tunnel
	 */
	tun->dentry = debugfs_create_dir(dev->name, ipsecmgr_drv->dentry);

	/*
	 * Configure inline mode and the DMA rings.
	 */
	nss_ipsecmgr_configure(&ipsecmgr_drv->cfg_work.work);

	write_lock(&ipsecmgr_drv->lock);
	list_add(&tun->list, &ipsecmgr_drv->tun_db);

	ipsecmgr_drv->max_mtu = dev->mtu;
	write_unlock(&ipsecmgr_drv->lock);

	nss_ipsecmgr_info("NSS IPsec manager loaded: %s\n", NSS_CLIENT_BUILD_ID);
	return 0;

unregister_dev:
	unregister_netdev(ipsecmgr_drv->dev);

netdev_free:
	free_netdev(ipsecmgr_drv->dev);

free:
	vfree(ipsecmgr_drv);
	ipsecmgr_drv = NULL;

	return -1;
}

/*
 * nss_ipsecmgr_exit()
 *	module exit
 */
static void __exit nss_ipsecmgr_exit(void)
{
	struct nss_ipsecmgr_tunnel *tun;

	if (!ipsecmgr_drv) {
		nss_ipsecmgr_warn("IPsec manager driver context empty");
		return;
	}

	if (!ipsecmgr_drv->nss_ctx) {
		nss_ipsecmgr_warn("%px: NSS Context empty", ipsecmgr_drv);
		goto free;
	}

	tun = netdev_priv(ipsecmgr_drv->dev);

	write_lock(&ipsecmgr_drv->lock);
	list_del(&tun->list);

	ipsecmgr_drv->max_mtu = U16_MAX;
	write_unlock(&ipsecmgr_drv->lock);

	BUG_ON(!list_empty(&ipsecmgr_drv->tun_db));

	/*
	 * Unregister the callbacks from the HLOS as we are no longer
	 * interested in exception data & async messages
	 */
	nss_ipsec_cmn_notify_unregister(ipsecmgr_drv->nss_ctx, ipsecmgr_drv->ifnum);
	unregister_netdev(ipsecmgr_drv->dev);

	/*
	 * Remove debugfs directory and entries below that.
	 */
	debugfs_remove_recursive(ipsecmgr_drv->dentry);

free:
	/*
	 * Free the ipsecmgr ctx
	 */
	vfree(ipsecmgr_drv);
	ipsecmgr_drv = NULL;

	nss_ipsecmgr_info("NSS IPsec manager unloaded\n");

}

MODULE_LICENSE("Dual BSD/GPL");

module_init(nss_ipsecmgr_init);
module_exit(nss_ipsecmgr_exit);
