/*
 **************************************************************************
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

/*
 * nss_dtlsmgr.c
 *	NSS DTLS Manager
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>

#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/aes.h>
#include <crypto/authenc.h>
#include <crypto/des.h>
#include <crypto/sha.h>
#include <crypto/skcipher.h>
#include <crypto/hash.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include <nss_cryptoapi.h>
#include <nss_dtls_cmn.h>
#include <nss_dtlsmgr.h>

#include "nss_dtlsmgr_private.h"

/*
 * Global DTLS context
 */
struct nss_dtlsmgr g_dtls = {0};

/*
 * nss_dtlsmgr_node_configure_done()
 *	Check and set the configured flag if the DTLS firmware package is successfully configured.
 */
static void nss_dtlsmgr_node_configure_done(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_dtlsmgr *drv = app_data;

	nss_dtlsmgr_info("%px: configure node(%u) response(%d) error(%d)\n", drv,
			 ncm->interface, ncm->response, ncm->error);
	atomic_cmpxchg(&drv->is_configured, false, (ncm->response == NSS_CMN_RESPONSE_ACK) ||
			(ncm->error == NSS_DTLS_CMN_ERROR_ALREADY_CONFIGURED));
}

/*
 * nss_dtlsmgr_node_configure()
 *	Send a configure message to the DTLS firmware package.
 */
static void nss_dtlsmgr_node_configure(struct nss_dtlsmgr *drv, uint32_t if_num)
{
	struct nss_dtls_cmn_msg ndcm = {0};
	nss_tx_status_t nss_status;

	/*
	 * Send DTLS configure message to NSS
	 */
	nss_dtls_cmn_msg_init(&ndcm, if_num, NSS_DTLS_CMN_MSG_TYPE_CONFIGURE_NODE, 0,
			      nss_dtlsmgr_node_configure_done, drv);

	nss_status = nss_dtls_cmn_tx_msg(drv->nss_ctx, &ndcm);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_dtlsmgr_warn("%px: unable to send node configure (%u)\n", drv, if_num);
		return;
	}
}

/*
 * nss_dtlsmgr_rx_event()
 *	Handle the response notification from the firmware.
 */
static void nss_dtlsmgr_rx_event(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_dtlsmgr *drv = app_data;

	nss_dtlsmgr_trace("%px: received Node stats sync:%u\n", drv, ncm->interface);

	/*
	 * The firmware DTLS should not configure its DMA rings till it
	 * knows that the HW is fully configured by host crypto driver.
	 * Since, the firmware boots independent of the host. There is no
	 * guarantee that the DMA will be ready for configuration when the
	 * firmware is configuring itself. Thus our approach is to notify
	 * the firmware to setup its DMA after the host is ready. Here we
	 * use a simple approach of module load order where the DTLS Manager
	 * loads after the crypto driver. The expectation is that the crypto
	 * driver would configure the HW correctly and other modules dependent
	 * upon it would get the chance to load. Once, the configuration is
	 * done for DTLS when would like to avoid further configuration of the
	 * DMA.
	 *
	 * TODO: Multiple items
	 * - Use a nss_cryptoapi_xxx to detect whether the crypto is ready.
	 * - Add a mechanism to switch to pointer for first time configuration
	 * - Add support for node stats. Eventhough the node stats is already
	 * captured in NSS driver.
	 */
	if (!atomic_read(&drv->is_configured)) {
		nss_dtlsmgr_node_configure(drv, ncm->interface);
		return;
	}
}

/*
 * nss_dtlsmgr_init_module()
 *	Initialize the DTLS manager module.
 */
int __init nss_dtlsmgr_init_module(void)
{
	struct nss_dtlsmgr *drv = &g_dtls;

	atomic_set(&drv->is_configured, false);

	nss_dtlsmgr_trace("registering for base interface(%u)", NSS_DTLS_INTERFACE);
	drv->nss_ctx = nss_dtls_cmn_notify_register(NSS_DTLS_INTERFACE, nss_dtlsmgr_rx_event, drv);
	if (!drv->nss_ctx) {
		nss_dtlsmgr_warn("%px: DTLS NSS context instance is NULL", drv);
		return -ENODEV;
	}

	drv->root_dir = debugfs_create_dir("qca-nss-dtlsmgr", NULL);
	if (!drv->root_dir)
		nss_dtlsmgr_warn("Failed to create debugfs directory");

	pr_info("qca-nss-dtlsmgr module loaded (%s)\n", NSS_PPE_BUILD_ID);
	return 0;
}

/*
 * nss_dtlsmgr_exit_module()
 *	Remove the DTLS manager module.
 */
void __exit nss_dtlsmgr_exit_module(void)
{
	struct nss_dtlsmgr *drv = &g_dtls;

	nss_dtls_cmn_notify_unregister(NSS_DTLS_INTERFACE);

	debugfs_remove_recursive(drv->root_dir);

	atomic_set(&drv->is_configured, false);

	nss_dtlsmgr_info("dtls manger is unloaded");
}

module_init(nss_dtlsmgr_init_module);
module_exit(nss_dtlsmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS DTLS manager");
