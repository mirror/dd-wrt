/*
 * Copyright (c) 2017-2018, 2020, The Linux Foundation. All rights reserved.
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
 */

#include <nss_crypto_hlos.h>
#include <nss_api_if.h>
#include <nss_crypto_hdr.h>
#include <nss_crypto_ctrl.h>
#include <nss_crypto_api.h>
#include <linux/scatterlist.h>

/*
 * nss_crypto_hdr_alloc()
 *	allocate a crypto header buf for the user to submit transform requests
 *
 * Note: The allocation happens from its corresponding user pool.
 * If, a user runs out buffer then it will only impact this particular user
 */
struct nss_crypto_hdr *nss_crypto_hdr_alloc(struct nss_crypto_user *user, uint32_t session,
						uint8_t src_frags, uint8_t dst_frags, uint8_t iv_len,
						uint8_t hmac_len, bool ahash)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_hdr *ch;
	struct nss_crypto_buf *buf;
	struct sk_buff *skb;
	size_t in_frag_len, out_frag_len;
	size_t size;

	in_frag_len = (src_frags + ahash) * sizeof(struct nss_crypto_frag);
	out_frag_len = (dst_frags + ahash) * sizeof(struct nss_crypto_frag);

	/*
	 * we are now going to consume a resource from the user
	 * hence we need to increment the reference count
	 */
	if (!kref_get_unless_zero(&user->ref))
		return NULL;

	/*
	 * Now add all the components that need to be filled in the crypto
	 * meta data, we will add all the len and used the final size as the size
	 * of skb_alloc. The input and output fragments are essentially pointing
	 * to the same memory. But, for crypto we need to indicate them as separate
	 * fragment structures.
	 */
	size = sizeof(struct nss_crypto_hdr) + sizeof(struct nss_crypto_buf);
	size = size + in_frag_len + out_frag_len + iv_len + hmac_len;

	/*
	 * If, total size requested exceeds the configured
	 * payload size for the particular user then we need
	 * to abort the allocation
	 */
	if (size > user->ctx.default_hdr_sz)
		return NULL;


	/*
	 * Get hold of a preallocated SKB
	 */
	spin_lock_bh(&user->lock);
	if (skb_queue_empty(&user->sk_head)) {
		spin_unlock_bh(&user->lock);
		kref_put(&user->ref, nss_crypto_user_free);
		return NULL;
	}

	skb = __skb_dequeue(&user->sk_head);
	/*
	 * Set skb->queue_mapping to the core on which this transformation is scheduled.
	 */
	skb_set_queue_mapping(skb, smp_processor_id());
	spin_unlock_bh(&user->lock);

	ch = (struct nss_crypto_hdr *)skb_put(skb, size);
	memset(ch, 0, skb->len);

	ch->version = NSS_CRYPTO_CMN_HDR_VERSION;
	ch->magic = NSS_CRYPTO_CMN_HDR_MAGIC;
	ch->index = NSS_CRYPTO_SESSION_INDEX(session, ctrl->max_contexts);

	ch->tot_len = size;
	ch->iv_len = iv_len;
	ch->hmac_len = hmac_len;
	ch->buf_len = sizeof(struct nss_crypto_buf);

	ch->in_frags = src_frags + ahash;
	ch->out_frags = dst_frags + ahash;

	ch->in_frag_start = sizeof(struct nss_crypto_hdr);
	ch->out_frag_start = ch->in_frag_start + in_frag_len;
	ch->iv_start = ch->out_frag_start + out_frag_len;
	ch->buf_start = ch->iv_start + iv_len;
	ch->hmac_start = ch->buf_start + sizeof(struct nss_crypto_buf);
	ch->priv_start = ch->hmac_start + hmac_len;
	ch->auth_end = 0;

	/*
	 * we need to store SKB in buf->skb
	 */
	buf = nss_crypto_hdr_get_buf(ch);
	buf->mapped = false;
	buf->skb = skb;

	return ch;
}
EXPORT_SYMBOL(nss_crypto_hdr_alloc);

/*
 * nss_crypto_hdr_free()
 * 	free the crypto buffer back to the user buf pool
 */
void nss_crypto_hdr_free(struct nss_crypto_user *user, struct nss_crypto_hdr *ch)
{
	struct nss_crypto_buf *buf = nss_crypto_hdr_get_buf(ch);
	struct sk_buff *skb = buf->skb;

	/*
	 * We will reset len and tail before putting
	 * skb back to skb queue head
	 */
	__skb_trim(skb, 0);

	spin_lock_bh(&user->lock);
	__skb_queue_head(&user->sk_head, skb);
	spin_unlock_bh(&user->lock);

	kref_put(&user->ref, nss_crypto_user_free);
}
EXPORT_SYMBOL(nss_crypto_hdr_free);

/*
 * nss_crypto_transform_payload()
 *	submit a transform for crypto operation to NSS
 */
int nss_crypto_transform_payload(struct nss_crypto_user *user, struct nss_crypto_hdr *ch,
			nss_crypto_req_callback_t cb, void *app_data)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_node *node;
	struct nss_crypto_buf *buf;
	struct nss_crypto_ctx *ctx;
	nss_tx_status_t error;
	struct sk_buff *skb;

	if (!cb)
		return -EPERM;

	/*
	 * increment the context reference so that session delete
	 * waits till all packets being processed for the given
	 * context have returned
	 */
	ctx = &ctrl->ctx_tbl[ch->index];
	if (!kref_get_unless_zero(&ctx->ref))
		return -ENOENT;

	if (!atomic_read(&ctx->active)) {
		kref_put(&ctx->ref, nss_crypto_ctx_free);
		return -EBUSY;
	}

	buf = nss_crypto_hdr_get_buf(ch);
	if (!buf->mapped) {
		kref_put(&ctx->ref, nss_crypto_ctx_free);
		return -EACCES;
	}

	node = ctx->node;
	skb = buf->skb;
	buf->comp = cb;
	buf->app_data = app_data;

	error = nss_crypto_cmn_tx_buf(node->nss_data_hdl, node->nss_ifnum, skb);
	if (error != NSS_TX_SUCCESS) {
		kref_put(&ctx->ref, nss_crypto_ctx_free);
		return (error == NSS_TX_FAILURE_QUEUE) ? -EBUSY : -ENODEV;
	}

	return 0;
}
EXPORT_SYMBOL(nss_crypto_transform_payload);

/*
 * nss_crypto_hdr_unmap_frag()
 *	Unmaps the fragments
 */
static inline void nss_crypto_unmap_frag(struct nss_crypto_node *node, struct nss_crypto_frag *frag, uint16_t frag_cnt)
{
	for (; frag_cnt--; frag++)
		dma_unmap_single(node->dev, frag->addr, frag->len, DMA_BIDIRECTIONAL);
}

/*
 * nss_crypto_transform_done()
 * 	completion callback for NSS HLOS driver when it receives a crypto buffer
 */
void nss_crypto_transform_done(struct net_device *net, struct sk_buff *skb,
			struct napi_struct *napi __attribute__((unused)))
{
	struct nss_crypto_hdr *ch = (struct nss_crypto_hdr *)skb->data;
	struct nss_crypto_buf *buf = nss_crypto_hdr_get_buf(ch);
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_node *node;
	struct nss_crypto_ctx *ctx;

	ctx = &ctrl->ctx_tbl[ch->index];
	node = ctx->node;

	/*
	 * Here, we will run through frags out and unmap the address.
	 * In case of out-of-place transformation, we will also run through
	 * frags in and unmap the address.
	 */
	nss_crypto_unmap_frag(node, nss_crypto_hdr_get_out_frag(ch), ch->out_frags);
	if (unlikely(!buf->in_place))
		nss_crypto_unmap_frag(node, nss_crypto_hdr_get_in_frag(ch), ch->in_frags);

	buf->mapped = false;
	buf->comp(buf->app_data, ch, ch->error);
	kref_put(&ctx->ref, nss_crypto_ctx_free);
}

/*
 * nss_crypto_hdr_map_src_sglist()
 *	Maps the linux SGLIST into the input crypto fragments
 */
static void nss_crypto_hdr_map_src_sglist(struct nss_crypto_hdr *ch, struct scatterlist *sg, int nsegs, uint16_t tot_len)
{
	struct scatterlist *sglist = sg;
	struct nss_crypto_frag *frag;
	uint32_t addr;
	uint16_t len;
	int i;

	frag = nss_crypto_hdr_get_in_frag(ch);
	for_each_sg(sg, sglist, nsegs, i) {
		/*
		 * get length for each segment
		 */
		len = sg_dma_len(sglist);
		addr = sg_dma_address(sglist);

		if (unlikely(tot_len < len))
			len = tot_len;

		tot_len -= len;

		/*
		 * load the physical address of the segment
		 */
		frag[i].addr = addr;
		frag[i].len = len;
	}

	/*
	 * Mark first descriptor and last descriptor
	 * with FIRST and LAST flags.
	 */
	frag[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	frag[nsegs - 1].flags |= NSS_CRYPTO_FRAG_FLAGS_LAST;
}

/*
 * nss_crypto_hdr_map_dst_sglist()
 *	Maps the Linux SGLIST into the output crypto fragments
 */
static void nss_crypto_hdr_map_dst_sglist(struct nss_crypto_hdr *ch, struct scatterlist *sg, int nsegs)
{
	struct scatterlist *sglist = sg;
	struct nss_crypto_frag *frag;
	uint32_t addr;
	uint16_t len;
	int i;

	frag = nss_crypto_hdr_get_out_frag(ch);
	for_each_sg(sg, sglist, nsegs, i) {
		/*
		 * get length for each segment
		 */
		len = sg_dma_len(sglist);
		addr = sg_dma_address(sglist);

		/*
		 * load the physical address of the segment
		 */
		frag[i].addr = addr;
		frag[i].len = len;
	}

	frag[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	frag[nsegs - 1].flags |= NSS_CRYPTO_FRAG_FLAGS_LAST;
}

/*
 * nss_crypto_hdr_map_sglist()
 * 	Map a Linux SGLIST into crypto fragments for IN & OUT
 *
 * Note:This API can be used in the following ways
 * - Encrypt + HMAC operation
 * - Decrypt + HMAC operation
 * - Encrypt operation
 * - Decrypt operation
 * - HMAC/HASH operation
 *
 * The intent is to allow the users to map there respective data payloads
 * single or multiple into crypto fragment structure. The crypto fragment
 * structure is uniform across all users. Thus using different API(s) a user
 * can maps its request data into a crypto specific request data. This particular
 * API expects that generated HMAC is at the end of the packet.
 *
 */
void nss_crypto_hdr_map_sglist(struct nss_crypto_hdr *ch, struct scatterlist *src, struct scatterlist *dst,
				uint16_t src_len, uint16_t dst_len, bool in_place)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_node *node;
	struct nss_crypto_buf *buf;
	struct nss_crypto_ctx *ctx;
	int src_nsegs, dst_nsegs;

	ctx = &ctrl->ctx_tbl[ch->index];
	if (!kref_get_unless_zero(&ctx->ref))
		return;

	node = ctx->node;
	kref_put(&ctx->ref, nss_crypto_ctx_free);

	buf = nss_crypto_hdr_get_buf(ch);
	buf->in_place = in_place;
	buf->mapped = true;

	/*
	 * use linux to map the SGLIST
	 */
	src_nsegs = dma_map_sg(node->dev, src, sg_nents(src), DMA_TO_DEVICE);
	dst_nsegs = in_place ? src_nsegs : dma_map_sg(node->dev, dst, sg_nents(dst), DMA_TO_DEVICE);

	/*
	 * Data length is same as source length
	 */
	ch->data_len = src_len;

	nss_crypto_hdr_map_src_sglist(ch, src, src_nsegs, src_len);
	nss_crypto_hdr_map_dst_sglist(ch, dst, dst_nsegs);
}
EXPORT_SYMBOL(nss_crypto_hdr_map_sglist);

/*
 * nss_crypto_hdr_map_sglist_ahash()
 * 	Map a Linux SGLIST into crypto fragments for ahash mode
 *
 * Note:This API can be used only for HMAC/HASH operation
 *
 * The intent is to allow the users to map their respective data payloads
 * single or multiple into crypto fragment structure. The crypto fragment
 * structure is uniform across all users. Thus using different API(s) a user
 * can maps its request data into a crypto specific request data.
 *
 */
void nss_crypto_hdr_map_sglist_ahash(struct nss_crypto_hdr *ch, struct scatterlist *sg, uint16_t auth_len)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_frag *in, *out;
	struct scatterlist *cur = sg;
	struct nss_crypto_node *node;
	struct nss_crypto_buf *buf;
	struct nss_crypto_ctx *ctx;
	uint32_t addr;
	uint16_t len;
	int nsegs, i;
	uint8_t *hmac;

	ctx = &ctrl->ctx_tbl[ch->index];
	if (!kref_get_unless_zero(&ctx->ref))
		return;

	node = ctx->node;
	kref_put(&ctx->ref, nss_crypto_ctx_free);

	/*
	 * use linux to map the SGLIST
	 */
	nsegs = dma_map_sg(node->dev, sg, sg_nents(sg), DMA_TO_DEVICE);
	buf = nss_crypto_hdr_get_buf(ch);
	buf->in_place = true;
	buf->mapped = true;

	/*
	 * find the start of the fragment structure in crypto header
	 */
	in = nss_crypto_hdr_get_in_frag(ch);
	out = nss_crypto_hdr_get_out_frag(ch);

	ch->data_len = 0;

	for_each_sg(sg, cur, nsegs, i) {
		/*
		 * get length for each segment
		 */
		len = sg_dma_len(cur);
		addr = sg_dma_address(cur);

		/*
		 * total length is the sum total of
		 * all segment data length
		 */
		ch->data_len += len;

		/*
		 * load the physical address of the segment
		 */
		in[i].addr = addr;
		in[i].len = len;

		out[i].addr = addr;
		out[i].len = len;
	}

	/*
	 * We need to reduce the auth_len from the payload if the HW
	 * is generates it. Otherwise, the user passes zero when
	 * it is already available. We also need to adjust the in and out
	 * length of 2nd last fragment.
	 */
	ch->data_len -= auth_len;
	in[nsegs - 1].len -= auth_len;
	out[nsegs - 1].len -= auth_len;

	/*
	 * Mark first descriptor and last descriptor
	 * with FIRST and LAST flags.
	 */
	in[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	in[nsegs].flags = NSS_CRYPTO_FRAG_FLAGS_LAST;
	out[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	out[nsegs].flags = NSS_CRYPTO_FRAG_FLAGS_LAST;

	/*
	 * Fill in and out addr
	 */
	hmac = nss_crypto_hdr_get_hmac(ch);
	in[nsegs].addr = (uint32_t)virt_to_phys(hmac);
	out[nsegs].addr = (uint32_t)virt_to_phys(hmac);

	/*
	 * HW is not expected to do anything with the last
	 * input fragment added. This is added just to match
	 * input fragment with output fragment.Hence, we
	 * program the input segment lenght as zero.
	 */
	in[nsegs].len = 0;
	out[nsegs].len = nss_crypto_hdr_get_hmac_len(ch);
}
EXPORT_SYMBOL(nss_crypto_hdr_map_sglist_ahash);

/*
 * nss_crypto_hdr_map_skb_frags()
 *	Map nr_frags into in and out descriptors
 */
static void nss_crypto_hdr_map_skb_frags(struct nss_crypto_hdr *ch, struct sk_buff *skb, struct device *dev)
{
	struct nss_crypto_frag *in, *out;
	uint32_t addr;
	uint16_t len;
	int nr_frags;
	int i;

	nr_frags = skb_shinfo(skb)->nr_frags;

	/*
	 * find the start of the fragment structure in crypto header
	 */
	in = nss_crypto_hdr_get_in_frag(ch);
	out = nss_crypto_hdr_get_out_frag(ch);

	ch->data_len = 0;

	for (i = 0; i < nr_frags; i++) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

		/*
		 * get length for each segment
		 */
		len = skb_frag_size(frag);
		addr = skb_frag_dma_map(dev, frag, 0, len, DMA_TO_DEVICE);

		/*
		 * total length is the sum total of
		 * all segment data length
		 */
		ch->data_len += len;

		/*
		 * load the physical address of the segment
		 */
		in[i].addr = addr;
		in[i].len = len;

		out[i].addr = addr;
		out[i].len = len;
	}

	/*
	 * Mark first descriptor and last descriptor
	 * with FIRST and LAST flags. Also, we need to
	 * adjust the hmac length in the last descriptor
	 */
	in[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	in[nr_frags - 1].flags = NSS_CRYPTO_FRAG_FLAGS_LAST;

	out[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	out[nr_frags - 1].flags = NSS_CRYPTO_FRAG_FLAGS_LAST;
	out[nr_frags - 1].len += ch->hmac_len;
}

/*
 * nss_crypto_hdr_map_skb_fraglist()
 *	Map skb fraglist into desscriptors
 */
static void nss_crypto_hdr_map_skb_fraglist(struct nss_crypto_hdr *ch, struct sk_buff *skb, struct device *dev)
{
	struct nss_crypto_frag *in, *out;
	struct sk_buff *iter;
	uint32_t addr;
	uint16_t len;
	int nr_skb = 0;

	/*
	 * find the start of the fragment structure in crypto header
	 */
	in = nss_crypto_hdr_get_in_frag(ch);
	out = nss_crypto_hdr_get_out_frag(ch);

	ch->data_len = 0;

	skb_walk_frags(skb, iter) {
		/*
		 * get length for each segment
		 */
		len = iter->len;
		addr = dma_map_single(dev, iter->data, len, DMA_TO_DEVICE);

		/*
		 * total length is the sum total of
		 * all segment data length
		 */
		ch->data_len += len;

		/*
		 * load the physical address of the segment
		 */
		in[nr_skb].addr = addr;
		in[nr_skb].len = len;

		out[nr_skb].addr = addr;
		out[nr_skb].len = len;

		nr_skb++;
	}

	/*
	 * Mark first descriptor and last descriptor
	 * with FIRST and LAST flags. Also, we need to
	 * adjust the hmac length in the last descriptor
	 */
	in[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	in[nr_skb - 1].flags = NSS_CRYPTO_FRAG_FLAGS_LAST;

	out[0].flags = NSS_CRYPTO_FRAG_FLAGS_FIRST;
	out[nr_skb - 1].flags = NSS_CRYPTO_FRAG_FLAGS_LAST;
	out[nr_skb - 1].len += ch->hmac_len;
}

/*
 * nss_crypto_hdr_map_skb()
 *	Map a Linux cryptoop data buffer into crypto fragments for IN & OUT
 *	This api is called by klips stack.
 *
 * Note:This API can be used in the following ways
 * - Encrypt + HMAC operation
 * - Decrypt + HMAC operation
 * - Encrypt operation
 * - Decrypt operation
 *
 * The intent is to allow the users to map there respective data payloads
 * single or multiple into crypto fragment structure. The crypto fragment
 * structure is uniform across all users. Thus using different API(s) a user
 * can maps its request data into a crypto specific request data. This particular
 * API expects ths generated HMAC as part of the packet.
 *
 */
void nss_crypto_hdr_map_skb(struct nss_crypto_hdr *ch, struct sk_buff *skb)
{
	struct nss_crypto_ctrl *ctrl = &g_control;
	struct nss_crypto_frag *in, *out;
	struct nss_crypto_node *node;
	struct nss_crypto_buf *buf;
	struct nss_crypto_ctx *ctx;
	uint8_t *data;
	uint32_t addr;
	uint16_t len;

	ctx = &ctrl->ctx_tbl[ch->index];
	if (!kref_get_unless_zero(&ctx->ref))
		return;

	node = ctx->node;
	kref_put(&ctx->ref, nss_crypto_ctx_free);

	/*
	 * We should have space for adding the hmac. It is typically expected the caller submitting
	 * for AEAD transform has enough tailroom to add the HMAC.
	 * In case there is not enough tailroom for HMAC, we will expand the tailroom
	 */
	if ((skb_tailroom(skb) < ch->hmac_len) && pskb_expand_head(skb, 0, ch->hmac_len, GFP_KERNEL))
		return;

	buf = nss_crypto_hdr_get_buf(ch);
	buf->mapped = true;

	/*
	 * If skb has multiple fragments with nr_frags
	 * set
	 */
	if (unlikely(skb_shinfo(skb)->nr_frags)) {
		nss_crypto_hdr_map_skb_frags(ch, skb, node->dev);
		return;
	}

	/*
	 * if skb is a fraglist
	 */
	if (unlikely(skb_has_frag_list(skb))) {
		nss_crypto_hdr_map_skb_fraglist(ch, skb, node->dev);
		return;
	}

	/*
	 * find the start of the fragment structure in crypto header
	 */
	in = nss_crypto_hdr_get_in_frag(ch);
	out = nss_crypto_hdr_get_out_frag(ch);

	/*
	 * Single SKB case (No Fragmentation Case)
	 */
	data = skb->data;
	len = skb->len;

	addr = dma_map_single(node->dev, data, len, DMA_TO_DEVICE);

	/*
	 * handle single fragment case separately
	 */
	ch->data_len = len;

	/*
	 * Fill in frag and out frag physical address and len
	 */
	in->addr = addr;
	in->len = len;

	/*
	 * Note: the output contains the HMAC if present
	 */
	out->addr = addr;
	out->len = len + ch->hmac_len;

	in->flags = NSS_CRYPTO_FRAG_FLAGS_FIRST | NSS_CRYPTO_FRAG_FLAGS_LAST;
	out->flags = NSS_CRYPTO_FRAG_FLAGS_FIRST | NSS_CRYPTO_FRAG_FLAGS_LAST;

	return;
}
EXPORT_SYMBOL(nss_crypto_hdr_map_skb);
