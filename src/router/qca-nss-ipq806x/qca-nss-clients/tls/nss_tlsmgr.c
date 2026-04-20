/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE
 **************************************************************************
 */

/*
 * nss_tlsmgr.c
 *	NSS TLS Manager
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/if_arp.h>
#include <linux/atomic.h>
#include <linux/debugfs.h>

#include <nss_api_if.h>

#include <nss_tls.h>
#include "nss_tlsmgr_priv.h"

/*
 * Global TLS context
 */
struct nss_tlsmgr *tlsmgr_drv;

/*
 * nss_tlsmgr_add_node_stats()
 *	Creates entries for node statistics
 *
 * Node statistics
 */
static const struct nss_tlsmgr_print tlsmgr_print_node_stats[] = {
	{"\tfail_ctx_alloc", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_ctx_free", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_pbuf_stats", NSS_TLSMGR_PRINT_DWORD},
};

/*
 * nss_tlsmgr_stats_size()
 *	Calculate size of NODE stats
 */
static ssize_t nss_tlsmgr_stats_size(void)
{
	const struct nss_tlsmgr_print *prn = tlsmgr_print_node_stats;
	ssize_t len = NSS_TLSMGR_PRINT_EXTRA;
	int i;

	for (i = 0; i < ARRAY_SIZE(tlsmgr_print_node_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	return len;
}

/*
 * nss_tlsmgr_print()
 *	Print context statistics
 */
static ssize_t nss_tlsmgr_print(char *buf)
{
	const struct nss_tlsmgr_print *prn = tlsmgr_print_node_stats;
	ssize_t max_len = tlsmgr_drv->print_len;
	uint32_t *stats_word = (uint32_t *)&tlsmgr_drv->stats;
	ssize_t len;
	int i;

	/*
	 * This expects a strict order as per the stats structure
	 */
	len = snprintf(buf, max_len, "---- NODE -----\n");
	len += snprintf(buf + len, max_len - len, "stats: {\n");

	for (i = 0; i < ARRAY_SIZE(tlsmgr_print_node_stats); i++, prn++)
		len += snprintf(buf + len, max_len - len, "%s: %u\n", prn->str, *stats_word++);

	len += snprintf(buf + len, max_len - len, "}\n");

	return len;
}

/*
 * nss_tlsmgr_read()
 *	Read NODE info
 */
static ssize_t nss_tlsmgr_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_tlsmgr_ctx *ctx = fp->private_data;
	ssize_t print_len = tlsmgr_drv->stats_len;
	ssize_t len = 0;
	char *buf;

	buf = vzalloc(print_len);
	if (!buf) {
		nss_tlsmgr_warn("%px: failed to allocate print buffer (req:%zd)", ctx, print_len);
		return 0;
	}

	/*
	 * Walk the context reference tree and retrieve stats
	 */
	len = nss_tlsmgr_print(buf);

	len = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return len;
}

/*
 * file operation structure instance
 */
const struct file_operations tlsmgr_file_ops = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_tlsmgr_read,
};

/*
 * nss_tlsmgr_node_config_done()
 *	Check and set the configured flag if the TLS firmware package is successfully configured.
 */
static void nss_tlsmgr_node_config_done(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_tlsmgr *drv = app_data;

	nss_tlsmgr_info("%px: configure node(%u) response(%d) error(%d)\n", drv,
			 ncm->interface, ncm->response, ncm->error);

	if ((ncm->response == NSS_CMN_RESPONSE_ACK) || (ncm->error == NSS_TLS_ERROR_ALREADY_CONFIGURE)) {
		   atomic_set(&drv->is_configured, true);
	}
}

/*
 * nss_tlsmgr_node_config()
 *	Send a configure message to the TLS firmware package.
 */
static void nss_tlsmgr_node_config(struct nss_tlsmgr *drv)
{
	struct nss_tls_msg ntcm = {0};
	nss_tx_status_t nss_status;

	/*
	 * Send TLS node configure message to NSS
	 */
	nss_tls_msg_init(&ntcm, NSS_TLS_INTERFACE, NSS_TLS_MSG_TYPE_NODE_CONFIG, 0, nss_tlsmgr_node_config_done, drv);

	nss_status = nss_tls_tx_msg(drv->nss_ctx, &ntcm);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_tlsmgr_warn("%px: unable to send node configure \n", drv);
		return;
	}
}

/*
 * nss_tlsmgr_rx_event()
 *	Handle the response notification from the firmware.
 */
static void nss_tlsmgr_rx_event(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_tlsmgr *drv = app_data;
	struct nss_tls_msg *ntcm = (struct nss_tls_msg *)ncm;

	nss_tlsmgr_trace("%px: received Node stats sync:%u\n", drv, ncm->interface);

	WARN_ON(ncm->interface != NSS_TLS_INTERFACE);

	/*
	 * The firmware TLS should not configure its DMA rings till it
	 * knows that the HW is fully configured by host crypto driver.
	 * Since, the firmware boots independent of the host. There is no
	 * guarantee that the DMA will be ready for configuration when the
	 * firmware is configuring itself. Thus our approach is to notify
	 * the firmware to setup its DMA after the host is ready. Here we
	 * use a simple approach of module load order where the TLS Manager
	 * loads after the crypto driver. The expectation is that the crypto
	 * driver would configure the HW correctly and other modules dependent
	 * upon it would get the chance to load. Once, the configuration is
	 * done for TLS when would like to avoid further configuration of the
	 * DMA.
	 */
	if (!atomic_read(&drv->is_configured)) {
		nss_tlsmgr_node_config(drv);
		return;
	}

	switch (ncm->type) {
        case NSS_TLS_MSG_TYPE_NODE_SYNC: {
		struct nss_tls_node_stats *sync = &ntcm->msg.node_stats;
		uint64_t *node_stats = (uint64_t *)&drv->stats;
		uint32_t *msg_stats = (uint32_t *)sync;
		int num;

		write_lock(&drv->lock);
		for (num = 0; num < sizeof(drv->stats)/sizeof(*node_stats); num++) {
			node_stats[num] += msg_stats[num];
		}

		write_unlock(&drv->lock);
		break;
	}

	default:
		nss_tlsmgr_info("%px: unhandled tls message type(%u)", drv, ntcm->cm.type);
		break;
	}
}

/*
 * nss_tlsmgr_init_module()
 *	Initialize the TLS manager module.
 */
int __init nss_tlsmgr_init_module(void)
{
	tlsmgr_drv = vzalloc(sizeof(*tlsmgr_drv));
	if (!tlsmgr_drv) {
		nss_tlsmgr_warn("Failed to allocate TLS manager context");
		return -1;
	}

	atomic_set(&tlsmgr_drv->is_configured, false);

	nss_tlsmgr_trace("registering for base interface(%u)", NSS_TLS_INTERFACE);

	tlsmgr_drv->nss_ctx = nss_tls_notify_register(NSS_TLS_INTERFACE, nss_tlsmgr_rx_event, tlsmgr_drv);
	if (unlikely(!tlsmgr_drv->nss_ctx)) {
		nss_tlsmgr_warn("%px: TLS NSS context instance is NULL", tlsmgr_drv);
		return -ENODEV;
	}

	tlsmgr_drv->dentry = debugfs_create_dir("qca-nss-tlsmgr", NULL);
	if (unlikely(!tlsmgr_drv->dentry)) {
		nss_tlsmgr_warn("Failed to create debugfs directory");
		return -ENOENT;
	}

	rwlock_init(&tlsmgr_drv->lock);

	debugfs_create_file("node", S_IRUGO, tlsmgr_drv->dentry, NULL, &tlsmgr_file_ops);
	tlsmgr_drv->stats_len = tlsmgr_drv->print_len = nss_tlsmgr_stats_size();

	nss_tlsmgr_info_always("qca-nss-tlsmgr module loaded (%s)\n", NSS_TLSMGR_BUILD_ID);
	return 0;
}

/*
 * nss_tlsmgr_exit_module()
 *	Remove the TLS manager module.
 */
void __exit nss_tlsmgr_exit_module(void)
{
	debugfs_remove_recursive(tlsmgr_drv->dentry);
	nss_tls_notify_unregister(NSS_TLS_INTERFACE);
	atomic_set(&tlsmgr_drv->is_configured, false);

	/*
	 * Free the tlsmgr ctx
	 */
	vfree(tlsmgr_drv);
	tlsmgr_drv = NULL;

	nss_tlsmgr_info_always("tls manger is unloaded");
}

module_init(nss_tlsmgr_init_module);
module_exit(nss_tlsmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS TLS manager");
