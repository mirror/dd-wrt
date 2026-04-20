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
 * nss_tlsmgr_ctx.c
 *	NSS TLS Manager context
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/tlshdr.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>
#include <linux/delay.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include <nss_tls.h>
#include <nss_tlsmgr.h>
#include "nss_tlsmgr_buf.h"
#include "nss_tlsmgr_ctx.h"
#include "nss_tlsmgr_crypto.h"
#include "nss_tlsmgr_mdata.h"
#include "nss_tlsmgr_tun.h"
#include "nss_tlsmgr_priv.h"

/*
 * Context Host statistics print info
 */
static const struct nss_tlsmgr_print tlsmgr_print_ctx_host_stats[] = {
	{"\ttx_packets", NSS_TLSMGR_PRINT_DWORD},
	{"\ttx_bytes", NSS_TLSMGR_PRINT_DWORD},
	{"\ttx_error", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_packets", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_bytes", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_error", NSS_TLSMGR_PRINT_DWORD},
};

/*
 * Context statistics print info
 */
static const struct nss_tlsmgr_print tlsmgr_print_ctx_fw_stats[] = {
	{"\trx_packets", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_bytes", NSS_TLSMGR_PRINT_DWORD},
	{"\ttx_packets", NSS_TLSMGR_PRINT_DWORD},
	{"\ttx_bytes", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_dropped[0]", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_dropped[1]", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_dropped[2]", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_dropped[3]", NSS_TLSMGR_PRINT_DWORD},

	{"\tsingle_rec", NSS_TLSMGR_PRINT_DWORD},
	{"\tmulti_rec", NSS_TLSMGR_PRINT_DWORD},
	{"\ttx_inval_reqs", NSS_TLSMGR_PRINT_DWORD},
	{"\trx_ccs_rec", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_ccs", NSS_TLSMGR_PRINT_DWORD},
	{"\teth_node_deactive", NSS_TLSMGR_PRINT_DWORD},
	{"\tcrypto_alloc_success", NSS_TLSMGR_PRINT_DWORD},
	{"\tcrypto_free_req", NSS_TLSMGR_PRINT_DWORD},
	{"\tcrypto_free_success", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_crypto_alloc", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_crypto_lookup", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_req_alloc", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_pbuf_stats", NSS_TLSMGR_PRINT_DWORD},
	{"\tfail_ctx_active", NSS_TLSMGR_PRINT_DWORD},

	{"\thw_len_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_token_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_bypass_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_crypto_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_hash_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_config_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_algo_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_hash_ovf_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_auth_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_pad_verify_error", NSS_TLSMGR_PRINT_DWORD},
	{"\thw_timeout_error", NSS_TLSMGR_PRINT_DWORD},

	{"\tno_desc_in", NSS_TLSMGR_PRINT_DWORD},
	{"\tno_desc_out", NSS_TLSMGR_PRINT_DWORD},
	{"\tno_reqs", NSS_TLSMGR_PRINT_DWORD},
};

#ifdef NSS_TLSMGR_DEBUG_DUMP
/*
 * nss_tlsmgr_ctx_dump_mdata()
 *	Dump metadata
 */
static void nss_tlsmgr_ctx_dump_mdata(struct nss_tlsmgr_mdata *mdata)
{
	int i;

	nss_tlsmgr_trace("mdata_ver (%x) mdata_rec_cnt (%x) in_frag_cnt (%x) out_frag_cnt (%x) flags (%x)\n",
			mdata->ver, mdata->rec_cnt, mdata->in_frag_cnt, mdata->out_frag_cnt, mdata->flags);

	for (i = 0; i < 4; i++) {
		nss_tlsmgr_trace("rec_len (%x) rec_type (%x) in_frags (%x) out_frags (%x)\n",
			mdata->rec[i].rec_len, mdata->rec[i].rec_type, mdata->rec[i].in_frags, mdata->rec[i].out_frags);
	}
}

#define NSS_TLSMGR_DUMP_MDATA(mdata) nss_tlsmgr_ctx_dump_mdata(mdata);
#else
#define NSS_TLSMGR_DUMP_MDATA(mdata)
#endif

/*
 * nss_tlsmgr_ctx_stats_size()
 *	Calculate size of context stats
 */
static ssize_t nss_tlsmgr_ctx_stats_size(void)
{
	const struct nss_tlsmgr_print *prn = tlsmgr_print_ctx_fw_stats;
	ssize_t len = NSS_TLSMGR_CTX_PRINT_EXTRA;
	int i;

	for (i = 0; i < ARRAY_SIZE(tlsmgr_print_ctx_fw_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	prn = tlsmgr_print_ctx_host_stats;

	len += NSS_TLSMGR_CTX_PRINT_EXTRA;

	for (i = 0; i < ARRAY_SIZE(tlsmgr_print_ctx_host_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	return len;
}

/*
 * nss_tlsmgr_ctx_print()
 *	Print context statistics
 */
static ssize_t nss_tlsmgr_ctx_print(struct nss_tlsmgr_ctx *ctx, char *buf)
{
	const struct nss_tlsmgr_print *prn = tlsmgr_print_ctx_fw_stats;
	ssize_t max_len = ctx->print_len;
	uint64_t *stats_dword = (uint64_t *)&ctx->fw_stats;
	ssize_t len;
	int i;

	/*
	 * This expects a strict order as per the stats structure
	 */
	len = snprintf(buf, max_len, "---- Context -----\n");
	len += snprintf(buf + len, max_len - len, "stats: {\n");

	for (i = 0; i < ARRAY_SIZE(tlsmgr_print_ctx_fw_stats); i++, prn++)
		len += snprintf(buf + len, max_len - len, "%s: %llu\n", prn->str, *stats_dword++);

	len += snprintf(buf + len, max_len - len, "}\n");

	stats_dword = (uint64_t *)&ctx->host_stats;

	len += snprintf(buf + len, max_len - len, "Host stats: {\n");
	prn = tlsmgr_print_ctx_host_stats;

	for (i = 0; i < ARRAY_SIZE(tlsmgr_print_ctx_host_stats); i++, prn++)
		len += snprintf(buf + len, max_len - len, "%s: %llu\n", prn->str, *stats_dword++);

	len += snprintf(buf + len, max_len - len, "}\n");

	return len;
}

/*
 * nss_tlsmgr_ctx_map_frag()
 *	map fragments
 */
static inline uint16_t nss_tlsmgr_ctx_map_frag(struct nss_tlsmgr_ctx *ctx, struct scatterlist *sg,
					struct nss_tlsmgr_mdata_frag *frag, uint8_t *frag_cnt)
{
	struct scatterlist *cur = sg;
	uint16_t frag_len = 0;
	uint8_t frag_count;
	int i = 0;

	frag_count = dma_map_sg(ctx->nss_dev, sg, sg_nents(sg), DMA_TO_DEVICE);
	for_each_sg(sg, cur, frag_count, i) {
		frag[i].len = sg_dma_len(cur);
		frag[i].addr = sg_dma_address(cur);
		frag_len += frag[i].len;
	}

	frag[0].flags |= NSS_TLSMGR_MDATA_FRAG_FLAG_FIRST;
	frag[frag_count - 1].flags |= NSS_TLSMGR_MDATA_FRAG_FLAG_LAST;

	*frag_cnt = frag_count;
	return frag_len;
}

/*
 * nss_tlsmgr_ctx_unmap_frag()
 *	Unmap fragments
 */
static inline void nss_tlsmgr_ctx_unmap_frag(struct nss_tlsmgr_ctx *ctx, struct nss_tlsmgr_mdata_frag *frag_ptr, uint8_t frag_cnt)
{
	uint8_t frag_idx;

	for (frag_idx = 0; frag_idx < frag_cnt; frag_ptr++, frag_idx++) {
		dma_unmap_single(ctx->nss_dev, frag_ptr->addr, frag_ptr->len, DMA_BIDIRECTIONAL);
	}
}

/*
 * nss_tlsmgr_ctx_read_stats()
 *	Read context info
 */
ssize_t nss_tlsmgr_ctx_read_stats(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_tlsmgr_ctx *ctx = fp->private_data;
	struct nss_tlsmgr_tun *tun = netdev_priv(ctx->dev);
	ssize_t print_len = ctx->print_len;
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
	read_lock_bh(&tun->lock);
	len = nss_tlsmgr_ctx_print(ctx, buf);
	read_unlock_bh(&tun->lock);

	len = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return len;
}

/*
 * nss_tlsmgr_ctx_rx_stats()
 *	Asynchronous event for reception of stats
 */
void nss_tlsmgr_ctx_rx_stats(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_tls_msg *ntcm = (struct nss_tls_msg *)ncm;
	struct nss_tlsmgr_ctx *ctx = app_data;
	struct nss_tlsmgr_tun *tun = netdev_priv(ctx->dev);

	switch (ncm->type) {
        case NSS_TLS_MSG_TYPE_CTX_SYNC: {
		struct nss_tls_ctx_stats *sync = &ntcm->msg.stats;
		uint64_t *fw_stats = (uint64_t *)&ctx->fw_stats;
		uint32_t *msg_stats = (uint32_t *)sync;
		int num;

		write_lock(&tun->lock);
		for (num = 0; num < sizeof(ctx->fw_stats)/sizeof(*fw_stats); num++) {
			fw_stats[num] += msg_stats[num];
		}

		write_unlock(&tun->lock);
		break;
	}

	default:
		nss_tlsmgr_info("%px: unhandled tls message type(%u)", ctx, ntcm->cm.type);
		break;
	}
}

/*
 * nss_tlsmgr_ctx_stats_copy()
 *	Read context stats into Linux device stats
 */
void nss_tlsmgr_ctx_stats_copy(struct nss_tlsmgr_ctx *ctx, struct rtnl_link_stats64 *dev_stats)
{
	struct nss_tlsmgr_pkt_stats *stats = &ctx->host_stats;
	uint64_t *packets, *bytes, *error;

	switch (ctx->di_type) {
	case NSS_DYNAMIC_INTERFACE_TYPE_TLS_INNER:
		packets = &dev_stats->tx_packets;
		bytes = &dev_stats->tx_bytes;
		error = &dev_stats->rx_errors;
		break;

	case NSS_DYNAMIC_INTERFACE_TYPE_TLS_OUTER:
		packets = &dev_stats->rx_packets;
		bytes = &dev_stats->rx_bytes;
		error = &dev_stats->rx_errors;
		break;

	default:
		return;
	}

	*packets += stats->rx_packets;
	*bytes += stats->rx_bytes;
	*error += stats->rx_errors;
}

/*
 * nss_tlsmgr_ctx_rx_inner()
 *	Process inner from NSS
 */
void nss_tlsmgr_ctx_rx_inner(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_tlsmgr_mdata *mdata = (struct nss_tlsmgr_mdata *)skb->data;
	struct nss_tlsmgr_mdata_rec *rec_mdata = nss_tlsmgr_mdata_get_rec(mdata, 0);
	struct nss_tlsmgr_buf *buf = (struct nss_tlsmgr_buf *)skb->head;
	struct nss_tlsmgr_rec *rec = nss_tlsmgr_buf_get_rec_start(buf);
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	struct nss_tlsmgr_ctx *ctx = &tun->ctx_enc;
	struct nss_tlsmgr_mdata_frag *frag_ptr;
	struct nss_tlsmgr_crypto *crypto;
	nss_tlsmgr_status_t status;
	uint8_t frag_cnt;
	int rec_idx;

	ctx->host_stats.rx_packets++;
	atomic_dec(&tun->pkt_pending);

	BUG_ON(nss_tlsmgr_mdata_get_magic(mdata) != NSS_TLSMGR_MDATA_MAGIC);
	BUG_ON(nss_tlsmgr_mdata_get_ver(mdata) != NSS_TLSMGR_MDATA_VER);
	BUG_ON(nss_tlsmgr_mdata_get_rec_cnt(mdata) > NSS_TLSMGR_MDATA_REC_MAX);

	for (rec_idx = 0; rec_idx < mdata->rec_cnt; rec_idx++, rec_mdata++, rec++) {
		frag_ptr = nss_tlsmgr_mdata_rec_get_in_frag(rec_mdata);
		frag_cnt = nss_tlsmgr_mdata_rec_get_in_frags(rec_mdata);
		nss_tlsmgr_ctx_unmap_frag(ctx, frag_ptr, frag_cnt);

		frag_ptr = nss_tlsmgr_mdata_rec_get_out_frag(rec_mdata);
		frag_cnt = nss_tlsmgr_mdata_rec_get_out_frags(rec_mdata);
		nss_tlsmgr_ctx_unmap_frag(ctx, frag_ptr, frag_cnt);

		rec->error = rec_mdata->error;
	}

	/*
	 * Peek into crypto active list and schedule detach and free
	 */
	if (mdata->flags & NSS_TLSMGR_MDATA_FLAG_CCS) {
		/*
		 * Detach from head of an active list and attach to tail of inactive list
		 * and schedule a delayed free.
		 */
		write_lock(&tun->lock);
		crypto = list_first_entry_or_null(&ctx->crypto_active, struct nss_tlsmgr_crypto, list);
		if (crypto) {
			list_del(&crypto->list);
			list_add_tail(&crypto->list, &tun->free_list);
		}

		write_unlock(&tun->lock);

		schedule_work(&tun->free_work);
	}

	if (unlikely(mdata->flags & NSS_TLSMGR_MDATA_FLAG_ERROR)) {
		status = NSS_TLSMGR_FAIL_TRANSFORM;
		ctx->host_stats.rx_errors++;
	}

	nss_tlsmgr_buf_rx(buf, status);
}

/*
 * nss_tlsmgr_ctx_rx_outer()
 *	Process outer from NSS
 */
void nss_tlsmgr_ctx_rx_outer(struct net_device *dev, struct sk_buff *skb, struct napi_struct *napi)
{
	struct nss_tlsmgr_mdata *mdata = (struct nss_tlsmgr_mdata *)skb->data;
	struct nss_tlsmgr_mdata_rec *rec_mdata = nss_tlsmgr_mdata_get_rec(mdata, 0);
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	nss_tlsmgr_status_t status = NSS_TLSMGR_OK;
	struct nss_tlsmgr_buf *buf = (struct nss_tlsmgr_buf *)skb->head;
	struct nss_tlsmgr_rec *rec = nss_tlsmgr_buf_get_rec_start(buf);
	struct nss_tlsmgr_ctx *ctx = &tun->ctx_dec;
	struct nss_tlsmgr_mdata_frag *frag_ptr;
	struct nss_tlsmgr_crypto *crypto;
	uint8_t frag_cnt;
	int rec_idx;

	BUG_ON(nss_tlsmgr_mdata_get_magic(mdata) != NSS_TLSMGR_MDATA_MAGIC);
	BUG_ON(nss_tlsmgr_mdata_get_ver(mdata) != NSS_TLSMGR_MDATA_VER);
	BUG_ON(nss_tlsmgr_mdata_get_rec_cnt(mdata) > NSS_TLSMGR_MDATA_REC_MAX);

	ctx->host_stats.rx_packets++;
	atomic_dec(&tun->pkt_pending);

	for (rec_idx = 0; rec_idx < mdata->rec_cnt; rec_idx++, rec_mdata++, rec++) {
		frag_ptr = nss_tlsmgr_mdata_rec_get_in_frag(rec_mdata);
		frag_cnt = nss_tlsmgr_mdata_rec_get_in_frags(rec_mdata);
		nss_tlsmgr_ctx_unmap_frag(ctx, frag_ptr, frag_cnt);

		frag_ptr = nss_tlsmgr_mdata_rec_get_out_frag(rec_mdata);
		frag_cnt = nss_tlsmgr_mdata_rec_get_out_frags(rec_mdata);
		nss_tlsmgr_ctx_unmap_frag(ctx, frag_ptr, frag_cnt);

		rec->error = rec_mdata->error;
	}

	/*
	 * Peek into crypto active list and schedule detach and free
	 */
	if (mdata->flags & NSS_TLSMGR_MDATA_FLAG_CCS) {
		/*
		 * Detach from head of an active list and attach to tail of inactive list
		 * and schedule a delayed free.
		 */
		write_lock(&tun->lock);
		crypto = list_first_entry_or_null(&ctx->crypto_active, struct nss_tlsmgr_crypto, list);
		if (crypto) {
			list_del(&crypto->list);
			list_add_tail(&crypto->list, &tun->free_list);
		}

		write_unlock(&tun->lock);

		schedule_work(&tun->free_work);
	}

	/*
	 * check if there is any error reported in mdata.
	 */
	if (unlikely(mdata->flags & NSS_TLSMGR_MDATA_FLAG_ERROR)) {
		status = NSS_TLSMGR_FAIL_TRANSFORM;
		ctx->host_stats.rx_errors++;
	}

	nss_tlsmgr_buf_rx(buf, status);
}

/*
 * nss_tlsmgr_ctx_fill_mdata()
 *	Prepare metadata records and fragments
 */
static void nss_tlsmgr_ctx_fill_mdata(struct nss_tlsmgr_ctx *ctx, struct nss_tlsmgr_rec *rec,
					uint8_t *data, uint8_t rec_cnt)
{
	struct nss_tlsmgr_mdata *mdata = (struct nss_tlsmgr_mdata *)data;
	struct nss_tlsmgr_mdata_rec *rec_mdata = nss_tlsmgr_mdata_get_rec(mdata, 0);
	struct nss_tlsmgr_mdata_frag *infrag_ptr, *outfrag_ptr;
	uint8_t rec_idx;

	mdata->ver = NSS_TLSMGR_MDATA_VER;
	mdata->rec_cnt = rec_cnt;
	mdata->magic = NSS_TLSMGR_MDATA_MAGIC;

	for (rec_idx = 0; rec_idx < mdata->rec_cnt; rec++, rec_mdata++, rec_idx++) {
		rec_mdata->rec_type = rec->rec_type;

		if (unlikely(rec->rec_type == NSS_TLSMGR_REC_TYPE_CCS))
			mdata->flags |= NSS_TLSMGR_MDATA_FLAG_CCS;

		infrag_ptr = nss_tlsmgr_mdata_rec_get_in_frag(rec_mdata);
		outfrag_ptr = nss_tlsmgr_mdata_rec_get_out_frag(rec_mdata);
		rec_mdata->rec_len = nss_tlsmgr_ctx_map_frag(ctx, rec->in, infrag_ptr, &rec_mdata->in_frags);
		nss_tlsmgr_ctx_map_frag(ctx, rec->out, outfrag_ptr, &rec_mdata->out_frags);

		mdata->in_frag_cnt += rec_mdata->in_frags;
		mdata->out_frag_cnt += rec_mdata->out_frags;
	}

	NSS_TLSMGR_DUMP_MDATA(mdata);
}

/*
 * nss_tlsmgr_ctx_tx()
 *	Transmit API
 */
nss_tlsmgr_status_t nss_tlsmgr_ctx_tx(struct nss_tlsmgr_ctx *ctx, struct sk_buff *skb, struct nss_tlsmgr_rec *rec)
{
	struct nss_tlsmgr_buf *buf = (struct nss_tlsmgr_buf *)skb->head;
	struct net_device *dev = ctx->dev;
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	nss_tx_status_t status;

	nss_tlsmgr_ctx_fill_mdata(ctx, rec, skb->data, buf->rec_cnt);

	nss_tlsmgr_trace("%px: Enqueueing buffer to ctx with interface num(%d)", ctx, ctx->ifnum);
	status = nss_tls_tx_buf(skb, ctx->ifnum, ctx->nss_ctx);
	if (status != NSS_TX_SUCCESS) {
		nss_tlsmgr_warn("%px: Buffer transmission to NSS FW failed, status=%d", buf, status);
		ctx->host_stats.tx_error++;

		/*
		 * Reset Device state to be only UP and not RUNNING
		 */
		dev->flags &= ~IFF_RUNNING;
		mod_timer(&tun->decongest.timer, jiffies + tun->decongest.ticks);
		return (status == NSS_TX_FAILURE_QUEUE) ? NSS_TLSMGR_FAIL_QUEUE_FULL : NSS_TLSMGR_FAIL;
	}

	ctx->host_stats.tx_packets++;

	atomic_inc(&buf->tun->pkt_pending);
	return NSS_TLSMGR_OK;
}

/*
 * nss_tlsmgr_ctx_deconfig()
 *	Deconfigure TLS context
 */
void nss_tlsmgr_ctx_deconfig(struct nss_tlsmgr_ctx *ctx)
{
	struct nss_tlsmgr_tun *tun = netdev_priv(ctx->dev);

	nss_tls_unregister_if(ctx->ifnum);
	nss_dynamic_interface_dealloc_node(ctx->ifnum, ctx->di_type);

	/*
	 * Move all active crypto sessions to free list and schedule
	 * the free work function
	 */
	write_lock_bh(&tun->lock);
	list_splice_tail_init(&ctx->crypto_active, &tun->free_list);
	write_unlock_bh(&tun->lock);

	schedule_work(&tun->free_work);
}
EXPORT_SYMBOL(nss_tlsmgr_ctx_deconfig);

/*
 * nss_tlsmgr_ctx_config_inner()
 *	Create TLS Encapsulation context
 */
int nss_tlsmgr_ctx_config_inner(struct nss_tlsmgr_ctx *ctx, struct net_device *dev)
{
	enum nss_tls_msg_type msg_type = NSS_TLS_MSG_TYPE_CTX_CONFIG;
	struct nss_tls_ctx_config *ctx_msg;
	struct nss_tls_msg ntcm;
	nss_tx_status_t status;
	int32_t ifnum;

	INIT_LIST_HEAD(&ctx->crypto_active);

	ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_TLS_INNER);
	if (ifnum < 0) {
		nss_tlsmgr_warn("%px: failed to allocate encap dynamic interface(%u)", tlsmgr_drv, ifnum);
		return -EINVAL;
	}

	ctx->di_type = NSS_DYNAMIC_INTERFACE_TYPE_TLS_INNER;
	ctx->ifnum = ifnum;
	ctx->dev = dev;
	ctx->print_len = nss_tlsmgr_ctx_stats_size();

	/*
	 * Register NSS TLS Encap I/F
	 */
	ctx->nss_ctx = nss_tls_register_if(ctx->ifnum, nss_tlsmgr_ctx_rx_inner,
					nss_tlsmgr_ctx_rx_stats, ctx->dev,
					0, ctx->di_type, (void *)ctx);
	if (!ctx->nss_ctx) {
		nss_tlsmgr_warn("%px: NSS register interface(%u) failed", ctx, ctx->ifnum);
		goto fail_register;
	}

	ctx->nss_dev = nss_tls_get_dev(ctx->nss_ctx);

	memset(&ctx->fw_stats, 0, sizeof(ctx->fw_stats));
	memset(&ntcm, 0, sizeof(struct nss_tls_msg));

	ctx_msg = &ntcm.msg.ctx_cfg;
	ctx_msg->except_ifnum = ifnum;

	status = nss_tls_tx_msg_sync(ctx->nss_ctx, ctx->ifnum, msg_type, sizeof(*ctx_msg), &ntcm);
	if (status != NSS_TX_SUCCESS) {
		nss_tlsmgr_warn("%px: Failed to configure the context (ctx_type:%u),(tx_status:%d),(error:%x)",
				ctx, ctx->di_type, status, ntcm.cm.error);
		goto fail_msg;
	}

	/*
	 * This is needed because the NSS firmware can have
	 * different requirements for headroom and tailroom
	 * based on chipsets. Hence, load the value supported
	 * by NSS.
	 */
	dev->needed_headroom = ctx_msg->headroom;
	dev->needed_tailroom = ctx_msg->tailroom;

	/*
	 * Allocate dummy crypto context in active list
	 */
	nss_tlsmgr_crypto_update_null(dev, ctx);

	return 0;

fail_msg:
	nss_tls_unregister_if(ctx->ifnum);
fail_register:
	nss_dynamic_interface_dealloc_node(ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TLS_INNER);
	return -EINVAL;
}
EXPORT_SYMBOL(nss_tlsmgr_ctx_config_inner);

/*
 * nss_tlsmgr_ctx_config_outer()
 *	Create TLS Decapsulation context
 */
int nss_tlsmgr_ctx_config_outer(struct nss_tlsmgr_ctx *ctx, struct net_device *dev)
{
	enum nss_tls_msg_type msg_type = NSS_TLS_MSG_TYPE_CTX_CONFIG;
	struct nss_tls_ctx_config *ctx_msg;
	struct nss_tls_msg ntcm;
	nss_tx_status_t status;
	int32_t ifnum;

	INIT_LIST_HEAD(&ctx->crypto_active);

	ifnum = nss_dynamic_interface_alloc_node(NSS_DYNAMIC_INTERFACE_TYPE_TLS_OUTER);
	if (ifnum < 0) {
		nss_tlsmgr_warn("%px: failed to allocate decap dynamic interface(%u)", tlsmgr_drv, ifnum);
		return -EINVAL;
	}

	ctx->di_type = NSS_DYNAMIC_INTERFACE_TYPE_TLS_OUTER;
	ctx->ifnum = ifnum;
	ctx->dev = dev;
	ctx->print_len = nss_tlsmgr_ctx_stats_size();

	/*
	 * Register NSS TLS Decap I/F
	 */
	ctx->nss_ctx = nss_tls_register_if(ctx->ifnum, nss_tlsmgr_ctx_rx_outer,
					nss_tlsmgr_ctx_rx_stats, ctx->dev,
					0, ctx->di_type, (void *)ctx);
	if (!ctx->nss_ctx) {
		nss_tlsmgr_warn("%px: NSS register interface(%u) failed", ctx, ctx->ifnum);
		goto fail_register;
	}

	ctx->nss_dev = nss_tls_get_dev(ctx->nss_ctx);

	memset(&ctx->fw_stats, 0, sizeof(ctx->fw_stats));
	memset(&ntcm, 0, sizeof(struct nss_tls_msg));

	ctx_msg = &ntcm.msg.ctx_cfg;
	ctx_msg->except_ifnum = ifnum;

	status = nss_tls_tx_msg_sync(ctx->nss_ctx, ctx->ifnum, msg_type, sizeof(*ctx_msg), &ntcm);
	if (status != NSS_TX_SUCCESS) {
		nss_tlsmgr_warn("%px: Failed to configure the context (ctx_type:%u),(tx_status:%d),(error:%x)",
				ctx, ctx->di_type, status, ntcm.cm.error);
		goto fail_msg;
	}

	/*
	 * Allocate dummy crypto context in active list
	 */
	nss_tlsmgr_crypto_update_null(dev, ctx);

	return 0;

fail_msg:
	nss_tls_unregister_if(ctx->ifnum);
fail_register:
	nss_dynamic_interface_dealloc_node(ifnum, NSS_DYNAMIC_INTERFACE_TYPE_TLS_INNER);
	return -EINVAL;
}
EXPORT_SYMBOL(nss_tlsmgr_ctx_config_outer);
