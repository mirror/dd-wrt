/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * nss_wifi_meshmgr.c
 *	NSS to HLOS WiFi-Mesh manager
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <nss_api_if.h>
#include <nss_wifi_meshmgr.h>
#include "nss_wifi_mesh_priv.h"

/*
 * WiFi-Mesh context
 */
static struct nss_wifi_meshmgr_ctx wmgr_ctx;

/*
 * nss_wifi_meshmgr_verify_if_num()
 *	Verify interface number.
 */
static bool nss_wifi_meshmgr_verify_if_num(int32_t if_num, enum nss_dynamic_interface_type di_type)
{
	return (nss_is_dynamic_interface(if_num) &&
		(nss_dynamic_interface_get_type(wmgr_ctx.nss_ctx, if_num) == di_type));
}

/*
 * nss_wifi_meshmgr_tx_msg()
 *	WiFi-Mesh message send API
 */
static nss_wifi_meshmgr_status_t nss_wifi_meshmgr_tx_msg(struct nss_wifi_mesh_msg *msg)
{
	return (nss_wifi_meshmgr_status_t)nss_wifi_mesh_tx_msg(wmgr_ctx.nss_ctx, msg);
}

/*
 * nss_wifi_meshmgr_ctx_insert()
 *	Insert the mesh context into global table
 */
static int32_t nss_wifi_meshmgr_ctx_insert(struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx)
{
	int32_t i, idx = -1;
	struct net_device *dev;

	assert_spin_locked(&wmgr_ctx.ref_lock);

	/*
	 * Check if the context already exist for the same netdev.
	 */
	for (i = 0; i < NSS_WIFI_MESH_MAX; i++) {
		if (!wmgr_ctx.mesh_ctx[i]) {
			idx = i;
			continue;
		}

		dev = (wmgr_ctx.mesh_ctx[i])->dev;
		if (dev == wmesh_ctx->dev) {
			nss_wifi_meshmgr_warn("%px: The mesh context already exist for dev:%s",
						&wmgr_ctx, wmesh_ctx->dev->name);
			return -1;
		}
	}

	if (idx == -1) {
		return idx;
	}

	wmgr_ctx.mesh_ctx[idx] = wmesh_ctx;
	return idx;
}

/*
 * nss_wifi_meshmgr_ctx_remove()
 *	Remove the mesh context from global table
 */
static void nss_wifi_meshmgr_ctx_remove(struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx)
{
	int32_t i;

	assert_spin_locked(&wmgr_ctx.ref_lock);

	for (i = 0; i < NSS_WIFI_MESH_MAX; i++) {
		if (wmgr_ctx.mesh_ctx[i] != wmesh_ctx) {
			continue;
		}

		wmgr_ctx.mesh_ctx[i] = NULL;
		return;
	}
}

/*
 * nss_wifi_meshmgr_cleanup()
 *	Clean up the mesh context.
 */
static void nss_wifi_meshmgr_cleanup(struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx)
{
	int32_t encap_ifnum, decap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	nss_wifi_meshmgr_trace("%px: Mesh handle cleanup called for: %s\n", &wmgr_ctx, wmesh_ctx->dev->name);

	encap_ifnum = wmesh_ctx->encap_ifnum;
	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Unregister and dealloc decap DI.
	 */
	nss_unregister_wifi_mesh_if(decap_ifnum);
	nss_status = (nss_wifi_meshmgr_status_t)nss_dynamic_interface_dealloc_node(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Failed to dealloc decap: %d\n", &wmgr_ctx, nss_status);
	}

	/*
	 * Unregister and dealloc encap DI.
	 */
	nss_unregister_wifi_mesh_if(encap_ifnum);
	nss_status = (nss_wifi_meshmgr_status_t)nss_dynamic_interface_dealloc_node(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Failed to dealloc encap: %d\n", &wmgr_ctx, nss_status);
	}

	dev_put(wmesh_ctx->dev);
	kfree(wmesh_ctx);
}

/*
 * nss_wifi_meshmgr_ref_dec()
 *	Find and decrement the reference counter.
 */
static void nss_wifi_meshmgr_ref_dec(struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx)
{
	if (atomic_dec_and_test(&wmesh_ctx->ref)) {
		nss_wifi_meshmgr_cleanup(wmesh_ctx);
	}
}

/*
 * nss_wifi_meshmgr_find_and_ref_inc()
 *	Find and inreae the reference counter.
 */
static struct nss_wifi_meshmgr_mesh_ctx *nss_wifi_meshmgr_find_and_ref_inc(nss_wifi_mesh_handle_t mesh_handle)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;

	if ((mesh_handle < 0) || (mesh_handle >= NSS_WIFI_MESH_MAX)) {
		nss_wifi_meshmgr_warn("%px: Invalid mesh handle: %d\n", &wmgr_ctx, mesh_handle);
		return NULL;
	}

	spin_lock_bh(&wmgr_ctx.ref_lock);
	wmesh_ctx = wmgr_ctx.mesh_ctx[mesh_handle];
	if (wmesh_ctx && atomic_inc_and_test(&wmesh_ctx->ref)) {
		BUG_ON(1);
	}
	spin_unlock_bh(&wmgr_ctx.ref_lock);
	return wmesh_ctx;
}

/*
 * nss_wifi_meshmgr_remap_error()
 *	Remap the error code.
 */
static nss_wifi_meshmgr_status_t nss_wifi_meshmgr_remap_error(enum nss_wifi_mesh_error_types error)
{
	switch (error) {
	case NSS_WIFI_MESH_ERROR_UNKNOWN_MSG:
		return NSS_WIFI_MESHMGR_FAILURE_UNKNOWN_MSG;
	case NSS_WIFI_MESH_ERROR_TTL_CONFIG:
		return NSS_WIFI_MESHMGR_FAILURE_TTL_CONFIG;
	case NSS_WIFI_MESH_ERROR_REFRESH_TIME_CONFIG:
		return NSS_WIFI_MESHMGR_FAILURE_REFRESH_TIME_CONFIG;
	case NSS_WIFI_MESH_ERROR_MPP_LEARNING_MODE_CONFIG:
		return NSS_WIFI_MESHMGR_FAILURE_MPP_LEARNING_MODE_CONFIG;
	case NSS_WIFI_MESH_ERROR_PATH_ADD_MAX_RADIO_CNT:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_ADD_MAX_RADIO_CNT;
	case NSS_WIFI_MESH_ERROR_PATH_ADD_INVALID_INTERFACE_NUM:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_ADD_INVALID_INTERFACE_NUM;
	case NSS_WIFI_MESH_ERROR_PATH_ADD_INTERFACE_NUM_NOT_FOUND:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_ADD_INTERFACE_NUM_NOT_FOUND;
	case NSS_WIFI_MESH_ERROR_PATH_TABLE_FULL:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_TABLE_FULL;
	case NSS_WIFI_MESH_ERROR_PATH_ALLOC_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_ALLOC_FAIL;
	case NSS_WIFI_MESH_ERROR_PATH_INSERT_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_INSERT_FAIL;
	case NSS_WIFI_MESH_ERROR_PATH_NOT_FOUND:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_NOT_FOUND;
	case NSS_WIFI_MESH_ERROR_PATH_UNHASHED:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_UNHASHED;
	case NSS_WIFI_MESH_ERROR_PATH_DELETE_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PATH_DELETE_FAIL;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_NOT_FOUND:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_NOT_FOUND;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_UNHASHED:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_UNHASHED;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_DELETE_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_DELETE_FAIL;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_EXISTS:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_EXISTS;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_ALLOC_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_ALLOC_FAIL;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_INSERT_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_INSERT_FAIL;
	case NSS_WIFI_MESH_ERROR_PROXY_PATH_TABLE_FULL:
		return NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_TABLE_FULL;
	case NSS_WIFI_MESH_ERROR_PB_ALLOC_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_PB_ALLOC_FAIL;
	case NSS_WIFI_MESH_ERROR_ENQUEUE_TO_HOST_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_ENQUEUE_TO_HOST_FAIL;
	case NSS_WIFI_MESH_ERROR_ENABLE_INTERFACE_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_ENABLE_INTERFACE_FAIL;
	case NSS_WIFI_MESH_ERROR_DISABLE_INTERFACE_FAIL:
		return NSS_WIFI_MESHMGR_FAILURE_DISABLE_INTERFACE_FAIL;
	case NSS_WIFI_MESH_ERROR_INVALID_EXCEPTION_NUM:
		return NSS_WIFI_MESHMGR_FAILURE_INVALID_EXCEPTION_NUM;
	case NSS_WIFI_MESH_ERROR_ONESHOT_ALREADY_ATTACHED:
		return NSS_WIFI_MESHMGR_FAILURE_ONESHOT_ALREADY_ATTACHED;
	case NSS_WIFI_MESH_ERROR_DUMMY_PATH_ADD_FAILED:
		return NSS_WIFI_MESHMGR_FAILURE_DUMMY_PATH_ADD;
	case NSS_WIFI_MESH_ERROR_DUMMY_PROXY_PATH_ADD_FAILED:
		return NSS_WIFI_MESHMGR_FAILURE_DUMMY_PROXY_PATH_ADD;
	default:
		return NSS_WIFI_MESHMGR_FAILURE;
	};
}

/*
 * nss_wifi_meshmgr_tx_msg_cb()
 *	Callback to handle the completion of NSS->HLOS messages.
 */
static void nss_wifi_meshmgr_tx_msg_cb(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx = (struct nss_wifi_meshmgr_mesh_ctx *)app_data;
	uint32_t error_code = ncm->error;

	/*
	 * FIXME: The wmesh_ctx can be invalid if the memory goes away with the caller being timedout.
	 */
	wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_SUCCESS;
	if (ncm->response != NSS_CMN_RESPONSE_ACK) {
		nss_wifi_meshmgr_warn("%px: WiFi-Mesh error response %d error_code: %u\n", &wmgr_ctx, ncm->response, error_code);
		wmesh_ctx->response = (nss_tx_status_t)nss_wifi_meshmgr_remap_error(error_code);
	}

	complete(&wmesh_ctx->complete);
}

 /*
 * nss_wifi_meshmgr_tx_msg_sync()
 *     Transmit a WiFi mesh message to NSS firmware synchronously.
 */
static nss_wifi_meshmgr_status_t nss_wifi_meshmgr_tx_msg_sync(struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx, struct nss_wifi_mesh_msg *wmesh_msg)
{
	nss_wifi_meshmgr_status_t status;
	int ret;

	down(&wmesh_ctx->sem);
	status = nss_wifi_meshmgr_tx_msg(wmesh_msg);
	if (status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh tx sync msg failed: %d\n", &wmgr_ctx, status);
		up(&wmesh_ctx->sem);
		return status;
	}

	/*
	 * Wait for the acknowledgement.
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);
	return status;
}

/*
 * nss_wifi_meshmgr_tx_buf()
 *	Send packets to mesh I/F
 */
nss_wifi_meshmgr_status_t nss_wifi_meshmgr_tx_buf(nss_wifi_mesh_handle_t mesh_handle, struct sk_buff *os_buf)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	nss_status = (nss_wifi_meshmgr_status_t)nss_wifi_mesh_tx_buf(wmgr_ctx.nss_ctx, os_buf, encap_ifnum);
	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_tx_buf);

/*
 * nss_wifi_meshmgr_if_down()
 *	Make the NSS interface down synchronously.
 */
nss_wifi_meshmgr_status_t nss_wifi_meshmgr_if_down(nss_wifi_mesh_handle_t mesh_handle)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t encap_ifnum, decap_ifnum;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;
	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Verify the I/F encap and decap number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER) ||
		nss_wifi_meshmgr_verify_if_num(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER))) {
		nss_wifi_meshmgr_warn("%px: Interface verification failed\n", &wmgr_ctx);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the encap I/F down message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_IF_CLOSE,
			sizeof(struct nss_if_close), nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);

	/*
	 * Send the I/F down message to the encap I/F.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg_sync(wmesh_ctx, &wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link encap I/F down failed: %d.\n", &wmgr_ctx, nss_status);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Initialize the decap I/F down message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nss_wifi_mesh_msg_init(&wmesh_msg, decap_ifnum, NSS_IF_CLOSE,
			sizeof(struct nss_if_close), nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);

	/*
	 * Send the I/F down message to the decap I/F.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg_sync(wmesh_ctx, &wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link decap I/F down failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_if_down);

/*
 * nss_wifi_meshmgr_if_up()
 *	Make the NSS interface up synchronously.
 */
nss_wifi_meshmgr_status_t nss_wifi_meshmgr_if_up(nss_wifi_mesh_handle_t mesh_handle)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t encap_ifnum, decap_ifnum;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;
	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Verify the I/F encap and decap number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER) ||
		nss_wifi_meshmgr_verify_if_num(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER))) {
		nss_wifi_meshmgr_warn("%px: Interface verification failed\n", &wmgr_ctx);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the encap I/F up message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_IF_OPEN,
			sizeof(struct nss_if_open), nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);

	/*
	 * Send the I/F up message to the encap I/F.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg_sync(wmesh_ctx, &wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link encap I/F up failed: %d.\n", &wmgr_ctx, nss_status);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Initialize the I/F decap up message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nss_wifi_mesh_msg_init(&wmesh_msg, decap_ifnum, NSS_IF_OPEN,
			sizeof(struct nss_if_open), nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);

	/*
	 * Send the I/F up message to the decap interface.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg_sync(wmesh_ctx, &wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link decap I/F up failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_if_up);

/*
 * nss_wifi_meshmgr_dump_mesh_path()
 *	Dump mesh path table request asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_dump_mesh_path(nss_wifi_mesh_handle_t mesh_handle, nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t encap_ifnum;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the path dump request message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_PATH_TABLE_DUMP,
			sizeof(struct nss_wifi_mesh_path_table_dump), msg_cb, app_data);

	/*
	 * Send the path dump request mesage to the NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh dump mesh path failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_dump_mesh_path);

/*
 * nss_wifi_meshmgr_dump_mesh_path_sync()
 *	Dump mesh path table request synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_dump_mesh_path_sync(nss_wifi_mesh_handle_t mesh_handle)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the path dump request mesage to the NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_dump_mesh_path(mesh_handle, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh path dump msg failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_dump_mesh_path_sync);

/*
 * nss_wifi_meshmgr_dump_mesh_proxy_path()
 *	Dump mesh proxy path table request asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_dump_mesh_proxy_path(nss_wifi_mesh_handle_t mesh_handle, nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t encap_ifnum;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the proxy path table message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_PROXY_PATH_TABLE_DUMP,
			sizeof(struct nss_wifi_mesh_proxy_path_table_dump), msg_cb, app_data);

	/*
	 * Send the proxy path dump message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link vap proxy path dump failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_dump_mesh_proxy_path);

/*
 * nss_wifi_meshmgr_dump_mesh_path_sync()
 *	Dump mesh proxy path table request synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_dump_mesh_proxy_path_sync(nss_wifi_mesh_handle_t mesh_handle)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the path dump request to the NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_dump_mesh_proxy_path(mesh_handle, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh proxy path dump msg failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_dump_mesh_proxy_path_sync);

/*
 * nss_wifi_meshmgr_assoc_link_vap()
 *	Associate the link interface to the mesh I/F asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_assoc_link_vap(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_assoc_link_vap *wmalv,
				nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_vdev_msg *wifivdevmsg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	struct nss_wifi_vdev_set_next_hop_msg *next_hop_msg = NULL;
	int32_t decap_ifnum, link_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	wifivdevmsg = kzalloc(sizeof(*wifivdevmsg), GFP_ATOMIC);
	if (!wifivdevmsg) {
		nss_wifi_meshmgr_warn("%px: Failed to allocate message memmory", &wmgr_ctx);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Verify the decap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, decap_ifnum);
		kfree(wifivdevmsg);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}


	link_ifnum = wmalv->link_vap_id;

	/*
	 * Verify the link VAP I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(link_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_VAP))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, link_ifnum);
		kfree(wifivdevmsg);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}


	next_hop_msg = &wifivdevmsg->msg.next_hop;
	next_hop_msg->ifnumber = decap_ifnum;
	nss_cmn_msg_init(&wifivdevmsg->cm, link_ifnum, NSS_WIFI_VDEV_SET_NEXT_HOP, sizeof(*next_hop_msg),
				msg_cb, app_data);

	/*
	 * Send the link vap mesage to the NSS synchronously.
	 */
	nss_status = (nss_wifi_meshmgr_status_t)nss_wifi_vdev_tx_msg(wmgr_ctx.nss_ctx, wifivdevmsg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link vap association failed: %d.\n", &wmgr_ctx, nss_status);
	}

	kfree(wifivdevmsg);
	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_assoc_link_vap);

/*
 * nss_wifi_meshmgr_assoc_link_vap_sync()
 *	Associate the link VAP to the mesh I/F synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_assoc_link_vap_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_assoc_link_vap *wmalv)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the link vap mesage to the NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_assoc_link_vap(mesh_handle, wmalv, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh link vap association failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_assoc_link_vap_sync);

/*
 * nss_wifi_meshmgr_mesh_config_update()
 *	Update mesh configuration message asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_config_update(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_config_msg *wmcum,
				    nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	struct nss_wifi_mesh_config_msg *nwmcum;
	int32_t encap_ifnum, decap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;
	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Verify the decap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER))) {
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, decap_ifnum);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the mesh configuration messsage.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmcum = &wmesh_msg.msg.mesh_config;
	memcpy(nwmcum, wmcum, sizeof(*nwmcum));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_INTERFACE_CONFIGURE,
			sizeof(*nwmcum), msg_cb, app_data);

	/*
	 * Send the configuration message to encap I/F.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh configuration message failed: %d.\n", &wmgr_ctx, nss_status);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Initialize the mesh configuration messsage.
	 */
	nss_wifi_mesh_msg_init(&wmesh_msg, decap_ifnum, NSS_WIFI_MESH_MSG_INTERFACE_CONFIGURE,
			sizeof(*nwmcum), msg_cb, app_data);

	/*
	 * Send the configuration message decap I/F.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh configuration message failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_config_update);

/*
 * nss_wifi_meshmgr_mesh_config_update_sync()
 *	Update mesh configuration message synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_config_update_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_config_msg *wmcum)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_config_update(mesh_handle, wmcum, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path update failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_config_update_sync);

/*
 * nss_wifi_meshmgr_mesh_proxy_path_delete()
 *	Delete the mesh proxy path asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_proxy_path_delete(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_proxy_path_del_msg *wmppdm,
					nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	struct nss_wifi_mesh_proxy_path_del_msg *nwmppdm;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmppdm = &wmesh_msg.msg.proxy_del_msg;
	memcpy(nwmppdm, wmppdm, sizeof(*nwmppdm));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_PROXY_PATH_DELETE,
			sizeof(*nwmppdm), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path delete failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_proxy_path_delete);

/*
 * nss_wifi_meshmgr_mesh_proxy_path_delete_sync()
 *	Delete the mesh proxy path synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_proxy_path_delete_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_proxy_path_del_msg *wmppdm)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_proxy_path_delete(mesh_handle, wmppdm, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path delete failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_proxy_path_delete_sync);

/*
 * nss_wifi_meshmgr_mesh_proxy_path_update()
 *	Mesh prpxy path update message asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_proxy_path_update(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_proxy_path_update_msg *wmppum,
					nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	struct nss_wifi_mesh_proxy_path_update_msg *nwmppum;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmppum = &wmesh_msg.msg.proxy_update_msg;
	memcpy(nwmppum, wmppum, sizeof(*nwmppum));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_PROXY_PATH_UPDATE,
			sizeof(*nwmppum), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path update failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_proxy_path_update);

/*
 * nss_wifi_meshmgr_mesh_proxy_path_update_sync()
 *	Send proxy update message synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_proxy_path_update_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_proxy_path_update_msg *wmppum)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_proxy_path_update(mesh_handle, wmppum, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path update failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_proxy_path_update_sync);

/*
 * nss_wifi_meshmgr_mesh_proxy_path_add()
 *	Send mesh proxy add message asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_proxy_path_add(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_proxy_path_add_msg *wmppam,
				     nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	struct nss_wifi_mesh_proxy_path_add_msg *nwmppam;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the message
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmppam = &wmesh_msg.msg.proxy_add_msg;
	memcpy(nwmppam, wmppam, sizeof(*nwmppam));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_PROXY_PATH_ADD,
			sizeof(*nwmppam), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path add  failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_proxy_path_add);

/*
 * nss_wifi_meshmgr_mesh_proxy_path_add_sync()
 *	Send mesh proxy add message synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_proxy_path_add_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_proxy_path_add_msg *wmppam)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t ret;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_proxy_path_add(mesh_handle, wmppam, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh proxy path add failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_proxy_path_add_sync);

/*
 * nss_wifi_meshmgr_mesh_path_delete()
 *	Send the mesh path delete message asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_delete(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_mpath_del_msg *wmpdm,
				  nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	struct nss_wifi_mesh_mpath_del_msg *nwmpdm;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmpdm = &wmesh_msg.msg.mpath_del;
	memcpy(nwmpdm, wmpdm, sizeof(*nwmpdm));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_MPATH_DELETE,
			sizeof(*nwmpdm), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh path delete failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_delete);

/*
 * nss_wifi_meshmgr_mesh_path_delete_sync()
 *	Send the mesh path delete message synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_delete_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_mpath_del_msg *wmpdm)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t ret;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_path_delete(mesh_handle, wmpdm, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh path delete failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_delete_sync);

/*
 * nss_wifi_meshmgr_mesh_path_add()
 *	Mesh path add message asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_add(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_mpath_add_msg *wmpam,
			       nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_mesh_mpath_add_msg *nwmpam;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmpam = &wmesh_msg.msg.mpath_add;
	memcpy(nwmpam, wmpam, sizeof(*nwmpam));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_MPATH_ADD,
			sizeof(*nwmpam), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh path addition failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_add);

/*
 * nss_wifi_meshmgr_mesh_path_add_sync()
 *	Mesh path add message synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_add_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_mpath_add_msg *wmpam)
{
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t ret;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_path_add(mesh_handle, wmpam, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh path addition failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);
	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_add_sync);

/*
 * nss_wifi_meshmgr_mpath_update()
 *	Send mesh path update message asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_update(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_mpath_update_msg *wmpum,
				  nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_mesh_mpath_update_msg *nwmpum;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the mesh path update message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmpum = &wmesh_msg.msg.mpath_update;
	memcpy(nwmpum, wmpum, sizeof(*nwmpum));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_MPATH_UPDATE,
			sizeof(*nwmpum), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh MPath update failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_update);

/*
 * nss_wifi_meshmgr_mpath_update_sync()
 *	Send mesh path update message sychronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_update_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_mpath_update_msg *wmpum)
{
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_path_update(mesh_handle, wmpum, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh MPath update failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_update_sync);

/*
 * nss_wifi_meshmgr_mesh_path_exception()
 *	Mesh path exception msg asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_exception(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_exception_flag_msg *wmefm,
			       nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_mesh_exception_flag_msg *nwmefm;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t encap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Initialize the message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmefm = &wmesh_msg.msg.exception_msg;
	memcpy(nwmefm, wmefm, sizeof(*nwmefm));
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_EXCEPTION_FLAG,
			sizeof(*nwmefm), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh path exception message failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_exception);

/*
 * nss_wifi_meshmgr_mesh_path_exception_sync()
 *	Send mesh path exception message sychronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_mesh_path_exception_sync(nss_wifi_mesh_handle_t mesh_handle,struct nss_wifi_mesh_exception_flag_msg *wmefm)
{
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_mesh_path_exception(mesh_handle, wmefm, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh path exception message failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_mesh_path_exception_sync);

/*
 * nss_wifi_meshmgr_config_mesh_exception()
 *	Configure mesh exception asynchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_config_mesh_exception(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_rate_limit_config *wmrlc,
			       nss_wifi_mesh_msg_callback_t msg_cb, void *app_data)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_mesh_rate_limit_config *nwmrlc;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;
	int32_t encap_ifnum, decap_ifnum, ifnum;
	nss_wifi_meshmgr_status_t nss_status;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Verify the decap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, decap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	switch(wmrlc->exception_num) {
	case NSS_WIFI_MESH_DS_MESH_PATH_NOT_FOUND:
		ifnum = encap_ifnum;
		break;

	case NSS_WIFI_MESH_US_MESH_PROXY_NOT_FOUND:
		ifnum = decap_ifnum;
		break;

	case NSS_WIFI_MESH_US_MESH_PATH_NOT_FOUND:
		ifnum = decap_ifnum;
		break;
	}

	/*
	 * Initialize the message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmrlc = &wmesh_msg.msg.exc_cfg;
	memcpy(nwmrlc, wmrlc, sizeof(*nwmrlc));
	nss_wifi_mesh_msg_init(&wmesh_msg, ifnum, NSS_WIFI_MESH_CONFIG_EXCEPTION,
			sizeof(*nwmrlc), msg_cb, app_data);

	/*
	 * Send the message to NSS asynchronously.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg(&wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh config exception message failed: %d.\n", &wmgr_ctx, nss_status);
	}

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_config_mesh_exception);

/*
 * nss_wifi_meshmgr_config_mesh_exception_sync()
 *	Configure mesh exception synchronously.
 */
nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_config_mesh_exception_sync(nss_wifi_mesh_handle_t mesh_handle,struct nss_wifi_mesh_rate_limit_config *wmrlc)
{
	nss_wifi_meshmgr_status_t nss_status;
	int32_t ret;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX;
	}

	/*
	 * Send the message to NSS synchronously.
	 */
	down(&wmesh_ctx->sem);
	nss_status = nss_wifi_meshmgr_config_mesh_exception(mesh_handle, wmrlc, nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Mesh config exception message failed: %d.\n", &wmgr_ctx, nss_status);
		up(&wmesh_ctx->sem);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return nss_status;
	}

	/*
	 * Wait for the acknowledgement
	 */
	ret = wait_for_completion_timeout(&wmesh_ctx->complete, msecs_to_jiffies(NSS_WIFI_MESH_TX_TIMEOUT));
	if (!ret) {
		nss_wifi_meshmgr_warn("%px: WiFi mesh msg tx failed due to timeout\n", &wmgr_ctx);
		wmesh_ctx->response = (nss_tx_status_t)NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT;
	}

	nss_status = (nss_wifi_meshmgr_status_t)wmesh_ctx->response;
	up(&wmesh_ctx->sem);

	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_config_mesh_exception_sync);

/*
 * nss_wifi_meshmgr_if_destroy_sync()
 *	Function to unregister and destroy dynamic interfaces synchronously.
 */
nss_wifi_meshmgr_status_t nss_wifi_meshmgr_if_destroy_sync(nss_wifi_mesh_handle_t mesh_handle)
{
	int32_t encap_ifnum, decap_ifnum;
	nss_wifi_meshmgr_status_t nss_status;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;

	wmesh_ctx = nss_wifi_meshmgr_find_and_ref_inc(mesh_handle);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Mesh context is null\n", &wmgr_ctx);
		return NSS_WIFI_MESHMGR_FAILURE_BAD_PARAM;
	}

	encap_ifnum = wmesh_ctx->encap_ifnum;
	decap_ifnum = wmesh_ctx->decap_ifnum;

	/*
	 * Verify the encap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, encap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Verify the decap I/F number against it types.
	 */
	if (!(nss_wifi_meshmgr_verify_if_num(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER))) {
		nss_wifi_meshmgr_warn("%px: I/F num: 0x%x verification failed\n", &wmgr_ctx, decap_ifnum);
		nss_wifi_meshmgr_ref_dec(wmesh_ctx);
		return NSS_WIFI_MESHMGR_FAILURE;
	}

	/*
	 * Send the I/F down message to NSS.
	 */
	nss_status = nss_wifi_meshmgr_if_down(mesh_handle);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Sending mesh I/F down to NSS failed: %d\n", &wmgr_ctx, nss_status);
	}

	/*
	 * Remove mesh context from the table.
	 */
	spin_lock_bh(&wmgr_ctx.ref_lock);
	nss_wifi_meshmgr_ctx_remove(wmesh_ctx);
	wmgr_ctx.mesh_count--;
	spin_unlock_bh(&wmgr_ctx.ref_lock);

	/*
	 * Release the reference taken during alloc.
	 */
	nss_wifi_meshmgr_ref_dec(wmesh_ctx);

	/*
	 * Release the reference for the find taken at the beginning of the function.
	 */
	nss_wifi_meshmgr_ref_dec(wmesh_ctx);
	return nss_status;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_if_destroy_sync);

/*
 * nss_wifi_meshmgr_if_create_sync()
 *	Create and register dynamic interface synchronously.
 *
 * Note: Synchronous create message, callbacks are used for driver registration not message handling.
 */
nss_wifi_mesh_handle_t nss_wifi_meshmgr_if_create_sync(struct net_device *dev, struct nss_wifi_mesh_config_msg *wmcm,
						       nss_wifi_mesh_data_callback_t data_cb,
						       nss_wifi_mesh_ext_data_callback_t ext_data_cb,
						       nss_wifi_mesh_msg_callback_t event_cb)
{
	struct nss_wifi_mesh_msg wmesh_msg;
	struct nss_wifi_mesh_config_msg *nwmcm;
	int32_t encap_ifnum, decap_ifnum;
	uint32_t features = 0;
	nss_wifi_mesh_handle_t mesh_handle;
	nss_wifi_meshmgr_status_t nss_status = NSS_WIFI_MESHMGR_SUCCESS;
	struct nss_wifi_meshmgr_mesh_ctx *wmesh_ctx;

	spin_lock_bh(&wmgr_ctx.ref_lock);
	if (wmgr_ctx.mesh_count == NSS_WIFI_MESH_MAX) {
		nss_wifi_meshmgr_warn("%px: Reached maxed number of mesh interface\n", dev);
		spin_unlock_bh(&wmgr_ctx.ref_lock);
		return -1;
	}

	wmgr_ctx.mesh_count++;
	spin_unlock_bh(&wmgr_ctx.ref_lock);

	dev_hold(dev);
	wmesh_ctx = kzalloc(sizeof(*wmesh_ctx), GFP_ATOMIC);
	if (!wmesh_ctx) {
		nss_wifi_meshmgr_warn("%px: Failed to allocate memory for mesh context\n", dev);
		goto ctx_alloc_fail;
	}

	wmesh_ctx->dev = dev;
	sema_init(&wmesh_ctx->sem, 1);
	init_completion(&wmesh_ctx->complete);

	/*
	 * Alloc the encap dynamic interface node.
	 */
	encap_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER);
	if (encap_ifnum < 0) {
		nss_wifi_meshmgr_warn("%px: Encap allocation failed.\n", dev);
		goto encap_alloc_fail;
	}

	if (nss_register_wifi_mesh_if(encap_ifnum, data_cb, ext_data_cb, event_cb,
					NSS_WIFI_MESH_DP_INNER, dev, features) != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Encap registration failed.\n", dev);
		goto encap_reg_fail;
	}

	/*
	 * Allocate and register decap interface.
	 */
	decap_ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER);
	if (decap_ifnum < 0) {
		nss_wifi_meshmgr_warn("%px: Decap allocation failed.\n", dev);
		goto decap_alloc_fail;
	}


	if (nss_register_wifi_mesh_if(decap_ifnum, data_cb, ext_data_cb, event_cb,
					NSS_WIFI_MESH_DP_OUTER, dev, features) != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Decap registration failed.\n", dev);
		goto decap_reg_fail;
	}


	wmesh_ctx->encap_ifnum = encap_ifnum;
	wmesh_ctx->decap_ifnum = decap_ifnum;

	nss_wifi_meshmgr_trace("%px: Successfully registered encap and decap iface for Mesh\n", dev);

	/*
	 * Initialize the encap configuration message.
	 */
	memset(&wmesh_msg, 0, sizeof(struct nss_wifi_mesh_msg));
	nwmcm = &wmesh_msg.msg.mesh_config;
	nwmcm->ttl = wmcm->ttl;
	nwmcm->mesh_path_refresh_time = wmcm->mesh_path_refresh_time;
	nwmcm->mpp_learning_mode = wmcm->mpp_learning_mode;
	nwmcm->config_flags = wmcm->config_flags | NSS_WIFI_MESH_CONFIG_FLAG_SIBLING_IF_NUM_VALID;
	nwmcm->sibling_ifnum = decap_ifnum;

	ether_addr_copy(nwmcm->local_mac_addr, wmcm->local_mac_addr);
	nss_wifi_mesh_msg_init(&wmesh_msg, encap_ifnum, NSS_WIFI_MESH_MSG_INTERFACE_CONFIGURE,
			sizeof(*nwmcm), nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);

	/*
	 * Send the encap configuration message.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg_sync(wmesh_ctx, &wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Encap configuration message failed: %d.\n", dev, nss_status);
		goto config_failed;
	}

	/*
	 * Initialize the decap configuration message.
	 */
	nwmcm->sibling_ifnum = encap_ifnum;
	nwmcm->block_mesh_forwarding = wmcm->block_mesh_forwarding;
	nss_wifi_mesh_msg_init(&wmesh_msg, decap_ifnum, NSS_WIFI_MESH_MSG_INTERFACE_CONFIGURE,
			       sizeof(*nwmcm), nss_wifi_meshmgr_tx_msg_cb, wmesh_ctx);

	/*
	 * Send the decap configuration message.
	 */
	nss_status = nss_wifi_meshmgr_tx_msg_sync(wmesh_ctx, &wmesh_msg);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Decap configuration message failed: %d.\n", dev, nss_status);
		goto config_failed;
	}

	/*
	 * Take the self reference on the mesh context.
	 */
	atomic_set(&wmesh_ctx->ref, 1);

	/*
	 * Add the Mesh context to the mesh manager's list.
	 */
	spin_lock_bh(&wmgr_ctx.ref_lock);
	mesh_handle = nss_wifi_meshmgr_ctx_insert(wmesh_ctx);
	if (mesh_handle < 0) {
		spin_unlock_bh(&wmgr_ctx.ref_lock);
		nss_wifi_meshmgr_warn("%px: Insertion for mesh context failed", &wmgr_ctx);
		goto config_failed;
	}

	spin_unlock_bh(&wmgr_ctx.ref_lock);

	nss_wifi_meshmgr_trace("%px: WiFi Mesh interface count:%d.\n", dev, wmgr_ctx.mesh_count);
	return mesh_handle;

config_failed:
	nss_unregister_wifi_mesh_if(decap_ifnum);
decap_reg_fail:
	nss_dynamic_interface_dealloc_node(decap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_OUTER);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Decap interface dealloc failed: %d\n", dev, nss_status);
	}
decap_alloc_fail:
	nss_unregister_wifi_mesh_if(encap_ifnum);
encap_reg_fail:
	nss_dynamic_interface_dealloc_node(encap_ifnum, NSS_DYNAMIC_INTERFACE_TYPE_WIFI_MESH_INNER);
	if (nss_status != NSS_WIFI_MESHMGR_SUCCESS) {
		nss_wifi_meshmgr_warn("%px: Encap interface dealloc failed: %d\n", dev, nss_status);
	}
encap_alloc_fail:
	kfree(wmesh_ctx);
ctx_alloc_fail:
	spin_lock_bh(&wmgr_ctx.ref_lock);
	wmgr_ctx.mesh_count--;
	spin_unlock_bh(&wmgr_ctx.ref_lock);
	dev_put(dev);
	return -1;
}
EXPORT_SYMBOL(nss_wifi_meshmgr_if_create_sync);

/*
 * nss_wifi_meshmgr_exit_module()
 *	WiFi-Mesh module exit function
 */
static void __exit nss_wifi_meshmgr_exit_module(void)
{
	int32_t i;

	/*
	 * Check if there are any mesh I/F. Delete all the mesh I/F from NSS FW and free.
	 */
	for (i = 0; i < NSS_WIFI_MESH_MAX; i++) {
		if (nss_wifi_meshmgr_if_destroy_sync(i) != NSS_WIFI_MESHMGR_SUCCESS) {
			nss_wifi_meshmgr_warn("%px Destroy failed or context does not exist", &wmgr_ctx);
		}
	}
	nss_wifi_meshmgr_info("Module %s unloaded\n", NSS_CLIENT_BUILD_ID);
}

/*
 * nss_wifi_meshmgr_init_module()
 *	Wi-Fi mesh manager module init function
 */
static int __init nss_wifi_meshmgr_init_module(void)
{
	struct nss_ctx_instance *nss_ctx;
	int32_t idx;

#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		nss_wifi_meshmgr_warn("NSS common not found.\n");
		return -1;
	}
#endif

	nss_ctx = nss_wifi_mesh_get_context();
	if (!nss_ctx) {
		nss_wifi_meshmgr_warn("NSS SoC context is NULL.\n");
		return -1;
	}

	wmgr_ctx.nss_ctx = nss_ctx;
	for (idx = 0; idx < NSS_WIFI_MESH_MAX; idx++) {
		wmgr_ctx.mesh_ctx[idx] = NULL;
	}

	wmgr_ctx.mesh_count = 0;
	spin_lock_init(&wmgr_ctx.ref_lock);

	nss_wifi_meshmgr_info("Module %s loaded\n", NSS_CLIENT_BUILD_ID);
	return 0;
}

module_init(nss_wifi_meshmgr_init_module);
module_exit(nss_wifi_meshmgr_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS WiFi-Mesh manager");
