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
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include "nss_ppe_vp.h"
#include "nss_ppe_vp_stats.h"

#define NSS_PPE_VP_TX_TIMEOUT 1000 /* 1 Second */

/*
 * Private data structure
 */
static struct nss_ppe_vp_pvt {
	struct semaphore sem;
	struct completion complete;
	int response;
	void *cb;
	void *app_data;
} ppe_vp_pvt;

int nss_ppe_vp_enable __read_mostly = 0;
int nss_ppe_vp_disable __read_mostly = 0;

DEFINE_SPINLOCK(nss_ppe_vp_stats_lock);

struct nss_ppe_vp_stats_debug nss_ppe_vp_debug_stats;
static struct dentry *nss_ppe_vp_dentry;

/*
 * nss_ppe_vp_verify_ifnum()
 *	Verify PPE VP interface number.
 */
static inline bool nss_ppe_vp_verify_ifnum(int if_num)
{
	return (if_num == NSS_PPE_VP_INTERFACE);
}

/*
 * nss_ppe_vp_get_context()
 *	Get NSS context instance for ppe_vp
 */
struct nss_ctx_instance *nss_ppe_vp_get_context(void)
{
	return (struct nss_ctx_instance *)&nss_top_main.nss[nss_top_main.ppe_handler_id];
}

/*
 * nss_ppe_vp_callback()
 *	Callback to handle the completion of NSS->HLOS messages.
 */
static void nss_ppe_vp_callback(void *app_data, struct nss_ppe_vp_msg *npvm)
{
	if (npvm->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_warning("ppe_vp error response %d\n", npvm->cm.response);
		ppe_vp_pvt.response = npvm->cm.response;
	}

	ppe_vp_pvt.response = NSS_TX_SUCCESS;
	complete(&ppe_vp_pvt.complete);
}

/*
 * nss_ppe_vp_tx_msg()
 *	Transmit a ppe_vp message to NSS FW
 */
nss_tx_status_t nss_ppe_vp_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_ppe_vp_msg *msg)
{
	struct nss_cmn_msg *ncm = &msg->cm;

	/*
	 * Trace messages.
	 */
	nss_ppe_vp_log_tx_msg(msg);

	/*
	 * Sanity check the message
	 */
	if (ncm->type >= NSS_PPE_VP_MSG_MAX) {
		nss_warning("%px: message type out of range: %d\n", nss_ctx, ncm->type);
		return NSS_TX_FAILURE;
	}

	if (!nss_ppe_vp_verify_ifnum(ncm->interface)) {
		nss_warning("%px: invalid interface %d\n", nss_ctx, ncm->interface);
		return NSS_TX_FAILURE;
	}

	return nss_core_send_cmd(nss_ctx, msg, sizeof(*msg), NSS_NBUF_PAYLOAD_SIZE);
}

/*
 * nss_ppe_vp_tx_msg_sync()
 *	Transmit a ppe_vp message to NSS firmware synchronously.
 */
nss_tx_status_t nss_ppe_vp_tx_msg_sync(struct nss_ctx_instance *nss_ctx, struct nss_ppe_vp_msg *npvm)
{
	nss_tx_status_t status;
	int ret = 0;

	down(&ppe_vp_pvt.sem);
	npvm->cm.cb = (nss_ptr_t)nss_ppe_vp_callback;
	npvm->cm.app_data = (nss_ptr_t)NULL;

	status = nss_ppe_vp_tx_msg(nss_ctx, npvm);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: ppe_tx_msg failed\n", nss_ctx);
		up(&ppe_vp_pvt.sem);
		return status;
	}

	ret = wait_for_completion_timeout(&ppe_vp_pvt.complete, msecs_to_jiffies(NSS_PPE_VP_TX_TIMEOUT));
	if (!ret) {
		nss_warning("%px: ppe_vp msg tx failed due to timeout\n", nss_ctx);
		ppe_vp_pvt.response = NSS_TX_FAILURE;
	}

	status = ppe_vp_pvt.response;
	up(&ppe_vp_pvt.sem);
	return status;
}

/*
 * nss_ppe_vp_tx_config_msg
 *	API to send ppe_vp support configure message to NSS FW
 */
nss_tx_status_t nss_ppe_vp_tx_config_msg(enum nss_dynamic_interface_type type, bool enable)
{
	struct nss_ctx_instance *nss_ctx = nss_ppe_vp_get_context();
	struct nss_ppe_vp_config_msg *npvcm;
	struct nss_ppe_vp_msg *npvm;
	nss_tx_status_t status;

	if (type >= NSS_DYNAMIC_INTERFACE_TYPE_MAX) {
		nss_warning("%px: Dynamic if msg drooped as type is wrong:%d\n", nss_ctx, type);
		return -1;
	}

	npvm = kzalloc(sizeof(struct nss_ppe_vp_msg), GFP_KERNEL);
	if (!npvm) {
		nss_warning("%px: Unable to allocate message\n", nss_ctx);
		return -1;
	}

	nss_cmn_msg_init(&npvm->cm, NSS_PPE_VP_INTERFACE, NSS_PPE_VP_MSG_CONFIG,
				sizeof(struct nss_ppe_vp_config_msg), NULL, NULL);

	npvcm = &npvm->msg.vp_config;
	npvcm->type = type;
	npvcm->vp_enable = enable;

	status = nss_ppe_vp_tx_msg_sync(nss_ctx, npvm);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("%px: Unable to send ppe_vp config message for type:%d\n", nss_ctx, type);
	}

	kfree(npvm);
	return status;
}

/*
 * nss_ppe_vp_handler()
 *	Handle NSS -> HLOS messages for ppe
 */
static void nss_ppe_vp_handler(struct nss_ctx_instance *nss_ctx, struct nss_cmn_msg *ncm, __attribute__((unused))void *app_data)
{
	struct nss_ppe_vp_msg *msg = (struct nss_ppe_vp_msg *)ncm;
	nss_ppe_vp_msg_callback_t cb;
	void *ctx;

	nss_trace("nss_ctx: %px ppe_vp msg: %px\n", nss_ctx, msg);
	BUG_ON(!nss_ppe_vp_verify_ifnum(ncm->interface));

	/*
	 * Trace messages.
	 */
	nss_ppe_vp_log_rx_msg(msg);

	/*
	 * Is this a valid request/response packet?
	 */
	if (ncm->type >= NSS_PPE_VP_MSG_MAX) {
		nss_warning("%px: received invalid message %d for PPE_VP interface\n", nss_ctx, ncm->type);
		return;
	}

	if (nss_cmn_get_msg_len(ncm) > sizeof(struct nss_ppe_vp_msg)) {
		nss_warning("%px: Length of message is greater than required: %d\n", nss_ctx, nss_cmn_get_msg_len(ncm));
		return;
	}

	switch (msg->cm.type) {
	case NSS_PPE_VP_MSG_SYNC_STATS:
		/*
		 * Per VP stats msg
		 */
		nss_ppe_vp_stats_sync(nss_ctx, &msg->msg.stats, ncm->interface);
		return;
	}

	/*
	 * Log failures
	 */
	nss_core_log_msg_failures(nss_ctx, ncm);

	/*
	 * Do we have a call back
	 */
	if (!ncm->cb) {
		return;
	}

	/*
	 * Callback
	 */
	cb = (nss_ppe_vp_msg_callback_t)ncm->cb;
	ctx = (void *)ncm->app_data;

	cb(ctx, msg);
}

/*
 * nss_ppe_vp_proc_help
 *	Print usage information for ppe_vp configure sysctl.
 */
void nss_ppe_vp_proc_help(void)
{
	printk("== for dynamic interface types read following file ==\n");
	printk("/sys/kernel/debug/qca-nss-drv/stats/dynamic_if/type_names\n");
}

/*
 * nss_ppe_vp_enable_handler
 *	Enable VP support for specfic dynamic interface type.
 */
static int nss_ppe_vp_enable_handler(struct ctl_table *table, int write, void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	nss_tx_status_t status;
	enum nss_dynamic_interface_type type;

	int ret = proc_dointvec(table, write, buffer, lenp, ppos);
	if (ret)
		return ret;

	nss_info("%s:%d start\n", __func__, __LINE__);

	if (!write) {
		nss_info("print dynamic interface type table\n");
		nss_ppe_vp_proc_help();
		return ret;
	}

	type = nss_ppe_vp_enable;
	if ((type <= NSS_DYNAMIC_INTERFACE_TYPE_NONE) || (type >= NSS_DYNAMIC_INTERFACE_TYPE_MAX)) {
		nss_warning("incorrect type: %u", nss_ppe_vp_enable);
		nss_ppe_vp_proc_help();
		return -EINVAL;
	}

	status = nss_ppe_vp_tx_config_msg(type, true);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("failed to enable VP support for type: %u", type);
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_ppe_vp_disable_handler
 *	Disable VP support for a given dynamic interface type.
 */
static int nss_ppe_vp_disable_handler(struct ctl_table *table, int write, void __user *buffer,
					size_t *lenp, loff_t *ppos)
{
	nss_tx_status_t status;
	enum nss_dynamic_interface_type type;

	int ret = proc_dointvec(table, write, buffer, lenp, ppos);
	if (ret)
		return ret;

	if (!write) {
		nss_ppe_vp_proc_help();
		return ret;
	}

	type = nss_ppe_vp_disable;
	if ((type <= NSS_DYNAMIC_INTERFACE_TYPE_NONE) || (type >= NSS_DYNAMIC_INTERFACE_TYPE_MAX)) {
		nss_warning("incorrect type: %u", nss_ppe_vp_enable);
		nss_ppe_vp_proc_help();
		return -EINVAL;
	}

	status = nss_ppe_vp_tx_config_msg(type, false);
	if (status != NSS_TX_SUCCESS) {
		nss_warning("failed to disable VP support for type: %u", type);
		return -EINVAL;
	}

	return 0;
}

static struct ctl_table nss_ppe_vp_table[] = {
	{
		.procname	= "enable",
		.data           = &nss_ppe_vp_enable,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = &nss_ppe_vp_enable_handler,
	},
	{
		.procname	= "disable",
		.data           = &nss_ppe_vp_disable,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = &nss_ppe_vp_disable_handler,
	},
	{ }
};

static struct ctl_table nss_ppe_vp_dir[] = {
	{
		.procname	= "ppe_vp",
		.mode		= 0555,
		.child		= nss_ppe_vp_table,
	},
	{ }
};

static struct ctl_table nss_ppe_vp_root_dir[] = {
	{
		.procname	= "nss",
		.mode		= 0555,
		.child		= nss_ppe_vp_dir,
	},
	{ }
};

static struct ctl_table_header *nss_ppe_vp_procfs_header;

/*
 * nss_ppe_vp_procfs_register()
 *	Register sysctl specific to ppe_vp
 */
void nss_ppe_vp_procfs_register(void)
{
	/*
	 * Register sysctl table.
	 */
	nss_ppe_vp_procfs_header = register_sysctl_table(nss_ppe_vp_root_dir);
}

/*
 * uss_ppe_vp_procfs_unregister()
 *	Unregister sysctl specific for ppe_vp
 */
void nss_ppe_vp_procfs_unregister(void)
{
	/*
	 * Unregister sysctl table.
	 */
	if (nss_ppe_vp_procfs_header) {
		unregister_sysctl_table(nss_ppe_vp_procfs_header);
	}
}

/*
 * nss_ppe_vp_register_handler()
 *
 */
void nss_ppe_vp_register_handler(void)
{
	struct nss_ctx_instance *nss_ctx = nss_ppe_vp_get_context();

	nss_ppe_vp_dentry = nss_ppe_vp_stats_dentry_create();
	if (nss_ppe_vp_dentry == NULL) {
		nss_warning("%px: Not able to create debugfs entry\n", nss_ctx);
		return;
	}

	nss_core_register_handler(nss_ctx, NSS_PPE_VP_INTERFACE, nss_ppe_vp_handler, NULL);
	nss_ppe_vp_procfs_register();

	sema_init(&ppe_vp_pvt.sem, 1);
	init_completion(&ppe_vp_pvt.complete);
}

/*
 * nss_ppe_vp_unregister_handler()
 *
 */
void nss_ppe_vp_unregister_handler(void)
{
	struct nss_ctx_instance *nss_ctx = nss_ppe_vp_get_context();

	debugfs_remove_recursive(nss_ppe_vp_dentry);
	nss_ppe_vp_procfs_unregister();
	nss_core_unregister_handler(nss_ctx, NSS_PPE_VP_INTERFACE);
}
