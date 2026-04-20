/*
 * Copyright (c) 2013-2017, 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
#include <nss_crypto_hlos.h>
#include <nss_api_if.h>
#include <nss_crypto.h>
#include <nss_crypto_if.h>
#include <nss_crypto_hw.h>
#include <nss_crypto_ctrl.h>
#include <nss_crypto_dbg.h>
#include <nss_crypto_debugfs.h>

#define NSS_CRYPTO_DEBUGFS_PERM_RO 0444
#define NSS_CRYPTO_DEBUGFS_PERM_RW 0666
#define NSS_CRYPTO_MSG_LEN (sizeof(struct nss_crypto_msg) - sizeof(struct nss_cmn_msg))

#define NSS_CRYPTO_ZONE_NAME_LEN	64
#define NSS_CRYPTO_ZONE_DEFAULT_NAME	"crypto_buf-"

/*
 * global control component
 */
extern struct nss_crypto_ctrl gbl_crypto_ctrl;

struct nss_ctx_instance *nss_drv_hdl;

struct nss_crypto_drv_ctx gbl_ctx = {0};

/*
 * internal structure for a buffer node
 */
struct nss_crypto_buf_node {
	struct llist_node node;			/* lockless node */
	struct sk_buff *skb;			/* SKB for holding the Crypto buffer */
	uint8_t results[NSS_CRYPTO_RESULTS_SZ] __attribute__((aligned(L1_CACHE_BYTES)));
};

#define NSS_CRYPTO_BUF_MAP_LEN \
	(offsetof(struct nss_crypto_buf_node, results) - offsetof(struct nss_crypto_buf_node, buf))

#define NSS_CRYPTO_RESULTS_MAP_LEN L1_CACHE_BYTES

/*
 * users of crypto driver
 */
struct nss_crypto_user {
	struct list_head  node;			/* user list */
	struct llist_head pool_head;	/* buffer pool lockless list */

	nss_crypto_user_ctx_t ctx;		/* user specific context*/

	nss_crypto_attach_t attach;		/* attach function*/
	nss_crypto_detach_t detach;		/* detach function*/

	struct kmem_cache *zone;
	uint8_t zone_name[NSS_CRYPTO_ZONE_NAME_LEN];
};

LIST_HEAD(nss_crypto_user_head);

/*
 * This pool seed indicates that we have 1024 SKBs per
 * user. These 1024 SKBs are preallocated per user and
 * maintained in a list. If a particular user does not
 * find a free SKB from this preallocated pool, it will
 * try to allocate a new one.
 */
static uint32_t nss_crypto_pool_seed = 1024;

/*
 * nss_crypto_skb_to_buf
 *	Initialize the skb with crypto buffer
 */
static inline struct nss_crypto_buf *nss_crypto_skb_to_buf(struct nss_crypto_buf_node *entry, struct sk_buff *skb)
{
	struct nss_crypto_buf *buf = (struct nss_crypto_buf *)skb_put(skb, sizeof(struct nss_crypto_buf));

	BUG_ON(((uint32_t)entry->results % L1_CACHE_BYTES));

	memset(entry->results, 0, NSS_CRYPTO_MAX_HASHLEN);

	buf->ctx_0 = (uint32_t)entry;
	buf->origin = NSS_CRYPTO_BUF_ORIGIN_HOST;

	buf->hash_addr = (uint32_t)entry->results;
	buf->iv_addr = (uint32_t)entry->results;

	return buf;
}

/*
 * nss_crypto_user_attach_all()
 *	Helper API for user to attach with crypto
 */
void nss_crypto_user_attach_all(struct nss_crypto_ctrl *ctrl)
{
	nss_crypto_user_ctx_t *ctx = NULL;
	struct nss_crypto_user *user, *tmp;

	BUG_ON(!(nss_crypto_check_state(ctrl, NSS_CRYPTO_STATE_INITIALIZED)));

	/*
	 * Walk the list of users and call the attach if they are not called yet
	 */
	mutex_lock(&ctrl->mutex);
	list_for_each_entry_safe(user, tmp, &nss_crypto_user_head, node) {
		if (user->ctx) {
			continue;
		}

		mutex_unlock(&ctrl->mutex);
		ctx = user->attach(user);
		mutex_lock(&ctrl->mutex);

		if (ctx) {
			user->ctx = ctx;
		}

	}

	mutex_unlock(&ctrl->mutex);
}

/*
 * nss_crypto_register_user()
 * 	register a new user of the crypto driver
 */
void nss_crypto_register_user(nss_crypto_attach_t attach, nss_crypto_detach_t detach, uint8_t *user_name)
{
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	nss_crypto_user_ctx_t *ctx = NULL;
	struct nss_crypto_buf_node *entry;
	struct nss_crypto_user *user;
	int i;

	if (nss_crypto_check_state(ctrl, NSS_CRYPTO_STATE_NOT_READY)) {
		nss_crypto_warn("%px: Crypto Device is not ready\n", ctrl);
		return;
	}

	user = vzalloc(sizeof(struct nss_crypto_user));
	if (!user) {
		nss_crypto_warn("%px:memory allocation fails for user(%s)\n", ctrl, user_name);
		return;
	}

	user->attach = attach;
	user->detach = detach;
	user->ctx = NULL;

	strlcpy(user->zone_name, NSS_CRYPTO_ZONE_DEFAULT_NAME, NSS_CRYPTO_ZONE_NAME_LEN);

	/*
	 * initialize list elements
	 */
	INIT_LIST_HEAD(&user->node);
	init_llist_head(&user->pool_head);

	/*
	 * Allocated the kmem_cache pool of crypto_bufs
	 * XXX: we can use the constructor
	 */
	strlcat(user->zone_name, user_name, NSS_CRYPTO_ZONE_NAME_LEN);
	user->zone = kmem_cache_create(user->zone_name, sizeof(struct nss_crypto_buf_node), 0, SLAB_HWCACHE_ALIGN, NULL);
	if (!user->zone) {
		nss_crypto_info_always("%px:(%s)failed to create crypto_buf for user\n", user, user_name);
		goto fail;
	}

	/*
	 * Try allocating till the seed value. If, the
	 * system returned less buffers then it will be
	 * taken care by the alloc routine by allocating
	 * the addtional buffers.
	 */
	for (i = 0; i < nss_crypto_pool_seed; i++) {
		entry = kmem_cache_alloc(user->zone, GFP_KERNEL);
		if (!entry) {
			nss_crypto_info_always("%px:failed to allocate memory\n", user);
			break;
		}

		/*
		 * We do not want to fail in case we are unable to allocate the
		 * seed amount of SKBs. We would like to continue with whatever
		 * SKBs that were allocated successfully.
		 */
		entry->skb = dev_alloc_skb(sizeof(struct nss_crypto_buf) + L1_CACHE_BYTES);
		if (!entry->skb) {
			kmem_cache_free(user->zone, entry);
			break;
		}

		/*
		 * Add to user local list.
		 */
		llist_add(&entry->node, &user->pool_head);
	}

	mutex_lock(&ctrl->mutex);
	list_add_tail(&user->node, &nss_crypto_user_head);

	/*
	 * this is required; if the crypto has not probed but a new
	 * user comes and registers itself. In that case we add the
	 * user to the 'user_head' and wait for the probe to complete.
	 * Once the probe completes the 'user_attach_all' gets called
	 * which initiates the attach for all users
	 */
	if (!nss_crypto_check_state(ctrl, NSS_CRYPTO_STATE_INITIALIZED)) {
		mutex_unlock(&ctrl->mutex);
		return;
	}

	/*
	 * release the mutex before calling; if the attach tries to unregister
	 * in the same call it will not deadlock
	 */
	mutex_unlock(&ctrl->mutex);
	ctx = attach(user);
	mutex_lock(&ctrl->mutex);

	if (ctx) {
		user->ctx = ctx;
	}

	mutex_unlock(&ctrl->mutex);
	return;
fail:
	nss_crypto_unregister_user(user);
	return;
}
EXPORT_SYMBOL(nss_crypto_register_user);

/*
 * nss_crypto_unregister_user()
 * 	unregister a user from the crypto driver
 */
void nss_crypto_unregister_user(nss_crypto_handle_t crypto)
{
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	struct nss_crypto_buf_node *entry;
	struct nss_crypto_user *user;
	struct llist_node *node;

	user = (struct nss_crypto_user *)crypto;

	mutex_lock(&ctrl->mutex);
	if (user->ctx && user->detach) {
		user->detach(user->ctx);
	}

	list_del(&user->node);
	mutex_unlock(&ctrl->mutex);

	/*
	 * The skb pool is a lockless list
	 */
	while (!llist_empty(&user->pool_head)) {
		node = llist_del_first(&user->pool_head);
		entry = container_of(node, struct nss_crypto_buf_node, node);

		dev_kfree_skb_any(entry->skb);
		kmem_cache_free(user->zone, entry);
	}

	/*
	 * this will happen if we are unwinding in a error
	 * path. Remove the user zone if it was successfully
	 * allocated.
	 */
	if (user->zone) {
		kmem_cache_destroy(user->zone);
	}

	vfree(user);
}
EXPORT_SYMBOL(nss_crypto_unregister_user);

/*
 * nss_crypto_buf_alloc()
 * 	allocate a crypto buffer for its user
 *
 * the allocation happens from its user pool. If, a user runs out its pool
 * then it will only be affected. Also, this function is lockless
 */
struct nss_crypto_buf *nss_crypto_buf_alloc(nss_crypto_handle_t hdl)
{
	struct nss_crypto_buf_node *entry;
	struct nss_crypto_user *user;
	struct llist_node *node;

	user = (struct nss_crypto_user *)hdl;

	node = llist_del_first(&user->pool_head);
	if (likely(node)) {
		entry = container_of(node, struct nss_crypto_buf_node, node);
		return nss_crypto_skb_to_buf(entry, entry->skb);
	}

	/*
	 * Note: this condition is hit when there are more than 'seed' worth
	 * of crypto buffers outstanding with the system. Instead of failing
	 * allocation attempt allocating buffers so that pool grows itself
	 * to the right amount needed to sustain the traffic without the need
	 * for dynamic allocation in future requests
	 */
	entry = kmem_cache_alloc(user->zone, GFP_KERNEL);
	if (!entry) {
		nss_crypto_info("%px:(%s)Unable to allocate crypto buffer from cache\n", user, user->zone_name);
		return NULL;
	}

	entry->skb = dev_alloc_skb(sizeof(struct nss_crypto_buf) + L1_CACHE_BYTES);
	if (!entry->skb) {
		nss_crypto_info("%px:Unable to allocate skb\n", entry);
		kmem_cache_free(user->zone, entry);
		return NULL;
	}

	return nss_crypto_skb_to_buf(entry, entry->skb);
}
EXPORT_SYMBOL(nss_crypto_buf_alloc);

/*
 * nss_crypto_buf_free()
 * 	free the crypto buffer back to the user buf pool
 */
void nss_crypto_buf_free(nss_crypto_handle_t hdl, struct nss_crypto_buf *buf)
{
	struct nss_crypto_buf_node *entry;
	struct nss_crypto_user *user;

	user = (struct nss_crypto_user *)hdl;
	entry = (struct nss_crypto_buf_node *)buf->ctx_0;

	/*
	 * trim the skb of the crypto_buf
	 */
	skb_trim(entry->skb, 0);
	llist_add(&entry->node, &user->pool_head);

}
EXPORT_SYMBOL(nss_crypto_buf_free);

/*
 * nss_crypto_transform_done()
 * 	completion callback for NSS HLOS driver when it receives a crypto buffer
 *
 * this function assumes packets arriving from host are transform buffers that
 * have been completed by the NSS crypto. It needs to have a switch case for
 * detecting control packets also
 */
void nss_crypto_transform_done(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_crypto_buf *buf = (struct nss_crypto_buf *)skb->data;
	struct nss_crypto_buf_node *entry;
	void *addr;
	struct device *cdev = gbl_crypto_ctrl.eng[0].dev;

	if (likely(buf->data_in == buf->data_out)) {
		dma_unmap_single(cdev, buf->data_in, buf->data_len, DMA_BIDIRECTIONAL);
	} else {
		dma_unmap_single(cdev, buf->data_in, buf->data_len, DMA_TO_DEVICE);
		dma_unmap_single(cdev, buf->data_out, buf->data_len, DMA_FROM_DEVICE);
	}

	dma_unmap_single(cdev, buf->iv_addr,  L1_CACHE_BYTES, DMA_BIDIRECTIONAL);

	addr = phys_to_virt(buf->iv_addr);
	entry = container_of(addr, struct nss_crypto_buf_node, results);

	buf->hash_addr = (uint32_t)addr;
	buf->iv_addr = (uint32_t)addr;

	buf->ctx_0 = (uint32_t)entry;

	buf->cb_fn(buf);
}

/*
 * nss_crypto_copy_stats()
 * 	copy stats from msg to local copy.
 */
static void nss_crypto_copy_stats(void *dst, void *src)
{
	memcpy(dst, src, sizeof(struct nss_crypto_stats));
}

/*
 * nss_crypto_process_sync()
 *	callback function for sync messages.
 */
void nss_crypto_process_event(void *app_data, struct nss_crypto_msg *nim)
{
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	struct nss_crypto_ctrl_eng *e_ctrl;
	struct nss_crypto_idx_info *idx;
	struct nss_crypto_sync_stats *stats;
	int i;

	switch (nim->cm.type) {
	case NSS_CRYPTO_MSG_TYPE_STATS:

		stats = &nim->msg.stats;

		for (i = 0; i < ctrl->num_eng; i++) {
			if (!ctrl->eng) {
				return;
			}

			e_ctrl = &ctrl->eng[i];
			nss_crypto_copy_stats(&e_ctrl->stats, &stats->eng_stats[i]);
		}

		for (i = 0; i < NSS_CRYPTO_MAX_IDXS; i++) {
			idx = &ctrl->idx_info[i];

			/*
			 * Copy statistics only if session is active
			 */
			if (nss_crypto_chk_idx_isfree(idx) == true) {
				continue;
			}

			nss_crypto_copy_stats(&idx->stats, &stats->idx_stats[i]);
		}

		nss_crypto_copy_stats(&ctrl->total_stats, &stats->total);

		break;

	default:
		nss_crypto_err("unsupported sync type %d\n", nim->cm.type);
		return;
	}
}

/*
 * nss_crypto_msg_sync_cb()
 * 	callback handler for for NSS synchronous messages
 */
void nss_crypto_msg_sync_cb(void *app_data, struct nss_crypto_msg *nim)
{
	struct nss_crypto_msg *nim_resp = (struct nss_crypto_msg *)app_data;
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;

	/*
	 * make sure there was no timeout
	 */
	if (atomic_read(&ctrl->complete_timeo)) {
		nss_crypto_dbg("response received after timeout (type - %d)\n", cm->type);
		return;
	}

	memcpy(nim_resp, nim, sizeof(struct nss_crypto_msg));

	complete(&ctrl->complete);
}

/*
 * nss_crypto_send_msg_sync
 * 	Send synchronous message to NSS.
 */
nss_crypto_status_t nss_crypto_send_msg_sync(struct nss_crypto_msg *nim, enum nss_crypto_msg_type type)
{
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	int ret;

	/*
	 * only one caller will be allowed to send a message
	 */
	if (down_interruptible(&ctrl->sem)) {
		nss_crypto_dbg("failed to acquire semaphore\n");
		return NSS_CRYPTO_STATUS_FAIL;
	}

	nss_cmn_msg_init(&nim->cm, NSS_CRYPTO_INTERFACE, type, NSS_CRYPTO_MSG_LEN, nss_crypto_msg_sync_cb, nim);

	if (nss_crypto_tx_msg(nss_drv_hdl, nim) != NSS_TX_SUCCESS) {
		nss_crypto_dbg("failed to send message to NSS(type - %d)\n", type);
		goto fail;
	}

	atomic_set(&ctrl->complete_timeo, 0);

	ret = wait_for_completion_timeout(&ctrl->complete, NSS_CRYPTO_RESP_TIMEO_TICKS);
	if (!ret) {
		atomic_inc(&ctrl->complete_timeo);
		nss_crypto_err("no response received from NSS(type - %d)\n", type);
		goto fail;
	}

	/*
	 * need to ensure that the response data has correctly arrived in
	 * current CPU cache
	 */
	smp_rmb();

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_crypto_err("Error from NSS: resp code (%d) error code (%d) \n",
				nim->cm.response, nim->cm.error);
		goto fail;
	}

	up(&ctrl->sem);
	return NSS_CRYPTO_STATUS_OK;

fail:
	up(&ctrl->sem);
	return NSS_CRYPTO_STATUS_FAIL;
}

/*
 * nss_crypto_transform_payload()
 *	submit a transform for crypto operation to NSS
 */
nss_crypto_status_t nss_crypto_transform_payload(nss_crypto_handle_t crypto, struct nss_crypto_buf *buf)
{
	struct nss_crypto_buf_node *entry;
	nss_tx_status_t nss_status;
	uint32_t paddr;
	void *vaddr;
	size_t len;
	struct device *cdev = gbl_crypto_ctrl.eng[0].dev;

	if (!buf->cb_fn) {
		nss_crypto_warn("%px:no buffer(%px) callback present\n", crypto, buf);
		return NSS_CRYPTO_STATUS_FAIL;
	}

	entry = (struct nss_crypto_buf_node *)buf->ctx_0;

	/*
	 * map data IN address
	 */
	vaddr = (void *)buf->data_in;
	len = buf->data_len;
	paddr = dma_map_single(cdev, vaddr, len, DMA_TO_DEVICE);
	buf->data_in = paddr;

	if (vaddr == (void *)buf->data_out) {
		buf->data_out = buf->data_in;
	} else {
		/*
		 * map data OUT address
		 */
		vaddr = (void *)buf->data_out;
		len = buf->data_len;
		paddr = dma_map_single(cdev, vaddr, len, DMA_FROM_DEVICE);
		buf->data_out = paddr;
	}

	/*
	 * We need to map the results into IV
	 */
	paddr = dma_map_single(cdev, entry->results, L1_CACHE_BYTES, DMA_BIDIRECTIONAL);
	buf->hash_addr = paddr;
	buf->iv_addr = paddr;

	/*
	 * Crypto buffer is essentially sitting inside the "skb->data". So, there
	 * is no need to MAP it here as it will be taken care by NSS driver
	 */
	nss_status = nss_crypto_tx_buf(nss_drv_hdl, NSS_CRYPTO_INTERFACE, entry->skb);
	if (nss_status != NSS_TX_SUCCESS) {
		nss_crypto_dbg("Not able to send crypto buf to NSS\n");
		return NSS_CRYPTO_STATUS_FAIL;
	}

	return NSS_CRYPTO_STATUS_OK;
}
EXPORT_SYMBOL(nss_crypto_transform_payload);

/*
 * nss_crypto_init()
 * 	initialize the crypto driver
 *
 * this will do the following
 * - Bring Power management perf level to TURBO
 * - register itself to the NSS HLOS driver
 * - wait for the NSS to be ready
 * - initialize the control component
 */
void nss_crypto_init(void)
{
	nss_crypto_ctrl_init();

	gbl_ctx.pm_hdl = nss_pm_client_register(NSS_PM_CLIENT_CRYPTO);

	/*
	 * Initialize debugfs entries
	 */
	nss_crypto_debugfs_init(&gbl_crypto_ctrl);

	nss_drv_hdl = nss_crypto_notify_register(nss_crypto_process_event, &nss_crypto_user_head);
	nss_drv_hdl = nss_crypto_data_register(NSS_CRYPTO_INTERFACE, nss_crypto_transform_done, NULL, 0);
}

/*
 * nss_crypto_engine_init()
 * 	initialize the crypto interface for each engine
 *
 * this will do the following
 * - prepare the open message for the engine
 * - initialize the control component for all pipes in that engine
 * - send the open message to the NSS crypto
 */
int nss_crypto_engine_init(uint32_t eng_num)
{
	struct nss_crypto_msg nim;
	struct nss_crypto_config_eng *open = &nim.msg.eng;
	struct nss_crypto_ctrl_eng *e_ctrl;
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	int i;

	e_ctrl = &ctrl->eng[eng_num];

	/*
	 * prepare the open config message
	 */
	open->eng_id = eng_num;
	open->bam_pbase = e_ctrl->bam_pbase;

	for (i = 0; i < NSS_CRYPTO_BAM_PP; i++) {
		nss_crypto_pipe_init(e_ctrl, i, &open->desc_paddr[i], &e_ctrl->hw_desc[i]);
	}

	if (nss_crypto_idx_init(e_ctrl, open->idx) != NSS_CRYPTO_STATUS_OK) {
		nss_crypto_err("failed to initiallize\n");
		return NSS_CRYPTO_STATUS_FAIL;
	}

	/*
	 * send open config message to NSS crypto
	 */
	return nss_crypto_send_msg_sync(&nim, NSS_CRYPTO_MSG_TYPE_OPEN_ENG);
}

/*
 * nss_crypto_send_session_update()
 * 	reset session specific state (alloc or free)
 */
nss_crypto_status_t nss_crypto_send_session_update(uint32_t session_idx, enum nss_crypto_session_state state, enum nss_crypto_cipher algo)
{
	struct nss_crypto_msg nim;
	struct nss_crypto_config_session *session = &nim.msg.session;
	struct nss_crypto_ctrl *ctrl = &gbl_crypto_ctrl;
	nss_crypto_status_t status;
	uint32_t iv_len = 0;

	switch (state) {
	case NSS_CRYPTO_SESSION_STATE_ACTIVE:
		nss_crypto_debugfs_add_session(ctrl, session_idx);
		break;

	case NSS_CRYPTO_SESSION_STATE_FREE:
		nss_crypto_debugfs_del_session(ctrl, session_idx);
		break;

	default:
		nss_crypto_err("incorrect session state = %d\n", state);
		return NSS_CRYPTO_STATUS_FAIL;
	}

	switch (algo) {
	case NSS_CRYPTO_CIPHER_AES_CBC:
	case NSS_CRYPTO_CIPHER_AES_CTR:
		iv_len = NSS_CRYPTO_MAX_IVLEN_AES;
		break;

	case NSS_CRYPTO_CIPHER_DES:
		iv_len = NSS_CRYPTO_MAX_IVLEN_DES;
		break;

	case NSS_CRYPTO_CIPHER_NULL:
		iv_len = NSS_CRYPTO_MAX_IVLEN_NULL;
		break;

	default:
		nss_crypto_err("invalid cipher\n");
		return NSS_CRYPTO_STATUS_FAIL;
	}

	session->idx = session_idx;
	session->state = state;
	session->iv_len = iv_len;

	/*
	 * send reset stats config message to NSS crypto
	 */
	status = nss_crypto_send_msg_sync(&nim, NSS_CRYPTO_MSG_TYPE_UPDATE_SESSION);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_crypto_info_always("session(%d) update failed\n", session->idx);
		return status;
	}

	/*
	 * If NSS state has changed to free. Delete session resources
	 */
	if (session->state == NSS_CRYPTO_SESSION_STATE_FREE) {
		nss_crypto_idx_free(session->idx);
	}

	return status;
}

/**
 * @brief crypto buf get api for IV address
 */
uint8_t *nss_crypto_get_ivaddr(struct nss_crypto_buf *buf)
{
	return (uint8_t *)buf->iv_addr;
}
EXPORT_SYMBOL(nss_crypto_get_ivaddr);

/**
 * @brief crypto buf get api for hash address
 */
uint8_t *nss_crypto_get_hash_addr(struct nss_crypto_buf *buf)
{
	return (uint8_t *)buf->hash_addr;
}
EXPORT_SYMBOL(nss_crypto_get_hash_addr);
