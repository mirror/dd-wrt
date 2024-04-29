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
 * nss_tlsmgr_buf.c
 *	NSS TLS Manager buffer
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/tlshdr.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/crypto.h>
#include <linux/debugfs.h>
#include <linux/rtnetlink.h>
#include <net/ipv6.h>
#include <linux/if_arp.h>
#include <linux/etherdevice.h>
#include <linux/atomic.h>
#include <asm/cmpxchg.h>

#include <nss_api_if.h>
#include <nss_dynamic_interface.h>

#include <nss_tls.h>
#include <nss_tlsmgr.h>
#include "nss_tlsmgr_ctx.h"
#include "nss_tlsmgr_crypto.h"
#include "nss_tlsmgr_buf.h"
#include "nss_tlsmgr_mdata.h"
#include "nss_tlsmgr_tun.h"
#include "nss_tlsmgr_priv.h"

#define NSS_TLSMGR_REC_MAX_SIZE	(sizeof(struct nss_tlsmgr_rec) * NSS_TLSMGR_MDATA_REC_MAX)

/*
 * nss_tlsmgr_buf_set_rec()
 *	Reserve space for a record in a buffer.
 */
struct nss_tlsmgr_rec *nss_tlsmgr_buf_set_rec(struct nss_tlsmgr_buf *buf, uint8_t in_segs, uint8_t out_segs)
{
	struct nss_tlsmgr_rec *rec;
	uint16_t cnt;

	if ((in_segs > NSS_TLSMGR_FRAG_MAX) || (out_segs > NSS_TLSMGR_FRAG_MAX)) {
		nss_tlsmgr_warn("%px: Unsupported fragment count: in_segs(%d) out_segs(%d)", buf, in_segs, out_segs);
		return NULL;
	}

	BUG_ON(buf->magic != NSS_TLSMGR_BUF_MAGIC);

	if (buf->rec_cnt >= NSS_TLSMGR_MDATA_REC_MAX) {
		nss_tlsmgr_warn("%px: Maximum record count reached(%d)", buf, buf->rec_cnt);
		return NULL;
	}

	cnt = buf->rec_cnt;
	buf->rec_cnt++;

	rec = nss_tlsmgr_buf_get_rec_start(buf);
	rec = rec + cnt;

	sg_init_table(rec->in, in_segs);
	sg_init_table(rec->out, out_segs);

	return rec;
}
EXPORT_SYMBOL(nss_tlsmgr_buf_set_rec);

/*
 * nss_tlsmgr_buf_get_rec()
 *	Get a record for the given record index.
 */
struct nss_tlsmgr_rec *nss_tlsmgr_buf_get_rec(struct nss_tlsmgr_buf *buf, uint8_t rec_idx)
{
	BUG_ON(buf->magic != NSS_TLSMGR_BUF_MAGIC);

	if (rec_idx > NSS_TLSMGR_MDATA_REC_MAX) {
		nss_tlsmgr_warn("%px: Invalid record index(%d)", buf, rec_idx);
		return NULL;
	}

	return nss_tlsmgr_buf_get_rec_start(buf) + rec_idx;
}
EXPORT_SYMBOL(nss_tlsmgr_buf_get_rec);

/*
 * nss_tlsmgr_buf_rx()
 *	Transform done completion API
 */
void nss_tlsmgr_buf_rx(struct nss_tlsmgr_buf *buf, nss_tlsmgr_status_t status)
{
	BUG_ON(buf->magic != NSS_TLSMGR_BUF_MAGIC);

	/*
	 * Invoke user registered callback
	 */
	if (buf->cb) {
		buf->cb(buf->app_data, buf, status);
	}
}

/*
 * nss_tlsmgr_buf_encap()
 *	Encapsulation API() for TLS records
 */
nss_tlsmgr_status_t nss_tlsmgr_buf_encap(struct nss_tlsmgr_buf *buf, nss_tlsmgr_data_callback_t cb, void *app_data)
{
	struct nss_tlsmgr_tun *tun = buf->tun;
	struct net_device *dev = tun->dev;
	struct nss_tlsmgr_ctx *ctx;
	struct sk_buff *skb;
	struct nss_tlsmgr_rec *rec;
	uint8_t *mdata_start;

	BUG_ON(buf->magic != NSS_TLSMGR_BUF_MAGIC);

	if (unlikely(!cb)) {
		nss_tlsmgr_error("%px: no user callback registered\n", buf);
		return NSS_TLSMGR_FAIL_QUEUE_FULL;
	}

	if (!(dev->flags & IFF_RUNNING)) {
		return NSS_TLSMGR_FAIL_QUEUE_FULL;
	}

	/*
	 * Fill User callback and application data in buffer
	 */
	buf->cb = cb;
	buf->app_data = app_data;

	rec = nss_tlsmgr_buf_get_rec_start(buf);

	mdata_start = (uint8_t *)(rec + buf->rec_cnt);
	mdata_start = PTR_ALIGN(mdata_start, SMP_CACHE_BYTES);

	/*
	 * At this point the buf is removed from the active data, only
	 * metadata is left.
	 */
	skb = buf->skb;
	skb_pull(skb, mdata_start - skb->data);

	ctx = &tun->ctx_enc;

	return nss_tlsmgr_ctx_tx(ctx, skb, rec);
}
EXPORT_SYMBOL(nss_tlsmgr_buf_encap);

/*
 * nss_tlsmgr_buf_decap()
 *	Decapsulation API() for TLS records
 */
nss_tlsmgr_status_t nss_tlsmgr_buf_decap(struct nss_tlsmgr_buf *buf, nss_tlsmgr_data_callback_t cb, void *app_data)
{
	struct nss_tlsmgr_tun *tun;
	struct nss_tlsmgr_ctx *ctx;
	struct sk_buff *skb;
	struct nss_tlsmgr_rec *rec;
	uint8_t *mdata_start;

	BUG_ON(buf->magic != NSS_TLSMGR_BUF_MAGIC);

	if (unlikely(!cb)) {
		nss_tlsmgr_error("%px: no user callback registered\n", buf);
		return NSS_TLSMGR_FAIL_QUEUE_FULL;
	}

	if (!(tun->dev->flags & IFF_RUNNING)) {
		return NSS_TLSMGR_FAIL_QUEUE_FULL;
	}

	/*
	 * Fill User callback and application data in buffer
	 */
	buf->cb = cb;
	buf->app_data = app_data;

	rec = nss_tlsmgr_buf_get_rec_start(buf);

	mdata_start = (uint8_t *)(rec + buf->rec_cnt);
	mdata_start = PTR_ALIGN(mdata_start, SMP_CACHE_BYTES);

	/*
	 * At this point the buf is removed from the active data, only
	 * metadata is left.
	 */
	skb = buf->skb;
	skb_pull(skb, mdata_start - skb->data);

	tun = buf->tun;
	ctx = &tun->ctx_dec;

	return nss_tlsmgr_ctx_tx(ctx, skb, rec);
}
EXPORT_SYMBOL(nss_tlsmgr_buf_decap);

/*
 * nss_tlsmgr_buf_decap_skb2recs()
 *	Create recs from SKB
 */
nss_tlsmgr_status_t nss_tlsmgr_buf_decap_skb2recs(struct sk_buff *skb, struct nss_tlsmgr_buf *buf)
{
	struct nss_tlsmgr_rec *rec;
	uint16_t tls_len, version;
	struct tlshdr *hdr;
	int payload_len;
	uint8_t *data;

	if (skb_linearize(skb) < 0) {
		nss_tlsmgr_warn("%px: Failed to linearize SKB", skb);
		return NSS_TLSMGR_FAIL_LINEARIZE;
	}

	payload_len = skb->len;
	data = skb->data;

	/*
	 * Packet will be of the form
	 * TLS_HDR1 APP_DATA1 TLS_HDR2 APP_DATA2...
	 * Here we will parse the header and check for the application data length for that
	 * record and prepare TLS record.
	 */
	do {
		hdr = (struct tlshdr *)data;
		tls_len = ntohs(hdr->len) + sizeof(*hdr);
		payload_len = payload_len - tls_len;
		version = ntohs(hdr->version);

		/*
		 * Check if this payload is malformed or not
		 */
		if (payload_len < 0) {
			nss_tlsmgr_warn("%px: Payload length shorter than record length (%d)", skb, tls_len);
			return NSS_TLSMGR_FAIL_REC_LEN;
		}

		/*
		 * Check if the type is supported for offload, note we only decode data and CCS.
		 * Other types are skipped in processing
		 */
		if ((hdr->type != TLSHDR_REC_TYPE_DATA) && (hdr->type != TLSHDR_REC_TYPE_CCS)) {
			nss_tlsmgr_warn("%px: Skipping TLS type (%d)", buf, hdr->type);
			continue;
		}

		/*
		 * Ideally, it should not receive 1.0 version packet. This check is needed for
		 * decapsulation side
		 */
		if ((version != TLSHDR_VERSION_1_1) && (version != TLSHDR_VERSION_1_2)) {
			nss_tlsmgr_warn("%px: bad TLS version (0x%x)", buf, version);
			return NSS_TLSMGR_FAIL_REC_VERSION;
		}

		/*
		 * Decapsulation is only supported in non-SG mode.
		 */
		rec = nss_tlsmgr_buf_set_rec(buf, 1, 1);
		if (unlikely(!rec)) {
			nss_tlsmgr_warn("%px: Error setting record", buf);
			return NSS_TLSMGR_FAIL_REC_RANGE;
		}

		rec->rec_type = hdr->type;
		sg_set_buf(&rec->in[0], data, tls_len);
		sg_set_buf(&rec->out[0], data, tls_len);
		data += tls_len;
	} while(payload_len);

	nss_tlsmgr_info("%px: SKB decoded to recs(%d) of size( %d)", skb, buf->rec_cnt, skb->len);
	return NSS_TLSMGR_OK;
}
EXPORT_SYMBOL(nss_tlsmgr_buf_decap_skb2recs);

/*
 * nss_tlsmgr_buf_get_priv()
 *	Get User private data from buffer.
 */
void *nss_tlsmgr_buf_get_priv(struct nss_tlsmgr_buf *buf)
{
	return buf->priv;
}
EXPORT_SYMBOL(nss_tlsmgr_buf_get_priv);

/*
 * nss_tlsmgr_buf_get_rec_cnt()
 *	Total number of records in a buffer.
 */
uint8_t nss_tlsmgr_buf_get_rec_cnt(struct nss_tlsmgr_buf *buf)
{
	return buf->rec_cnt;
}
EXPORT_SYMBOL(nss_tlsmgr_buf_get_rec_cnt);

/*
 * nss_tlsmgr_buf2skb()
 *	Buffer to SKB conversion.
 */
struct sk_buff *nss_tlsmgr_buf2skb(struct nss_tlsmgr_buf *buf)
{
	return buf->skb;
}
EXPORT_SYMBOL(nss_tlsmgr_buf2skb);

/*
 * nss_tlsmgr_skb2buf()
 *	SKB to buffer conversion.
 */
struct nss_tlsmgr_buf *nss_tlsmgr_skb2buf(struct sk_buff *skb)
{
	return (struct nss_tlsmgr_buf *)skb->head;
}
EXPORT_SYMBOL(nss_tlsmgr_skb2buf);

/*
 * nss_tlsmgr_buf_alloc()
 *	TLS buffer allocation API()
 */
struct nss_tlsmgr_buf *nss_tlsmgr_buf_alloc(struct net_device *dev, void *priv)
{
	struct nss_tlsmgr_tun *tun = netdev_priv(dev);
	struct nss_tlsmgr_buf *buf;
	struct sk_buff *skb;
	size_t size;

	/*
	 * The metadata start needs to be aligned to the Cache line boundary
	 */
	size = sizeof(*buf) + (sizeof(struct nss_tlsmgr_rec) * NSS_TLSMGR_MDATA_REC_MAX);
	size += sizeof(struct nss_tlsmgr_mdata) + SMP_CACHE_BYTES;

	size = ALIGN(size, SMP_CACHE_BYTES);

	/*
	 * Allocate SKB for TLS buffer
	 */
	skb = netdev_alloc_skb(dev, size);
	if (unlikely(!skb)) {
		nss_tlsmgr_warn("%px:unable to allocate SKB\n", priv);
		return NULL;
	}

	/*
	 * The buf structure must start from head; hence reset the
	 * data back to head
	 */
	skb_push(skb, skb_headroom(skb));
	skb_trim(skb, 0);

	buf = (struct nss_tlsmgr_buf *)skb_put(skb, size);
	memset(buf, 0, skb->len);
	buf->tun = tun;
	buf->skb = skb;
	buf->priv = priv;
	buf->magic = NSS_TLSMGR_BUF_MAGIC;

	nss_tlsmgr_trace("%px: allocated buffer of size(%zu)", skb, size);
	return buf;
}
EXPORT_SYMBOL(nss_tlsmgr_buf_alloc);

/*
 * nss_tlsmgr_buf_free()
 *	TLS buffer free API()
 */
void nss_tlsmgr_buf_free(struct nss_tlsmgr_buf *buf)
{
	/*
	 * Free the SKB
	 */
	dev_kfree_skb_any(buf->skb);
}
EXPORT_SYMBOL(nss_tlsmgr_buf_free);
