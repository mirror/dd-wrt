/*
 * Annapurna Labs DMA Linux driver - SG Memory copy preparation
 * Copyright(c) 2013 Annapurna Labs.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 */
#include "al_dma.h"


/******************************************************************************
 *****************************************************************************/
struct dma_async_tx_descriptor *al_dma_prep_sg_lock(
	struct dma_chan *c,
	struct scatterlist *dst_sg, unsigned int dst_nents,
	struct scatterlist *src_sg, unsigned int src_nents,
	unsigned long	flags)
{
	struct al_dma_chan *chan = to_al_dma_chan(c);
	struct dma_async_tx_descriptor *txd = NULL;
	int idx;
	int32_t rc;
	int sw_desc_num_req = 1;
	struct scatterlist *sg;
	size_t total_src_len = 0;
	size_t total_dst_len = 0;

	dev_dbg(
		chan->device->common.dev,
		"%s: chan->idx = %d, dests = %u, srcs = %u, flags = %08lx\n",
		__func__,
		chan->idx,
		dst_nents,
		src_nents,
		flags);

	BUG_ON(src_nents > AL_SSM_MAX_SRC_DESCS);
	BUG_ON(dst_nents > AL_SSM_MAX_DST_DESCS);

	if (likely(al_dma_get_sw_desc_lock(chan, sw_desc_num_req) == 0))
		idx = chan->head;
	else {
		dev_dbg(
			chan->device->common.dev,
			"%s: al_dma_get_sw_desc_lock failed!\n",
			__func__);

		return NULL;
	}

	chan->sw_desc_num_locked = sw_desc_num_req;

	while (sw_desc_num_req) {
		struct al_raid_transaction *xaction;
		struct al_dma_sw_desc *desc = al_dma_get_ring_ent(chan, idx);
		int umap_ent_cnt = 0;
		int i;

		if (1 == sw_desc_num_req)
			txd = &desc->txd;

		desc->txd.flags = flags;
		/* prepare hal transaction */
		xaction = &desc->hal_xaction;
		xaction->op = AL_RAID_OP_MEM_CPY;
		if ((flags & DMA_PREP_INTERRUPT) && (1 == sw_desc_num_req))
			xaction->flags |= AL_SSM_INTERRUPT;
		if ((flags & DMA_PREP_FENCE) && (1 == sw_desc_num_req))
			xaction->flags |= AL_SSM_BARRIER;

		/* use bufs[0] and block[0] for source buffers/blocks */
		for_each_sg(src_sg, sg, src_nents, i) {
			desc->bufs[i].addr = sg_dma_address(sg);
			desc->bufs[i].len = sg_dma_len(sg);

			dma_descriptor_unmap(&desc->txd);

			total_src_len += desc->bufs[i].len;
			dev_dbg(chan->device->common.dev,
			"%s: src[%d] = %llx len 0x%x\n",
			__func__, i, (unsigned long long)desc->bufs[i].addr,
			desc->bufs[i].len);
		}
		desc->blocks[0].bufs = &desc->bufs[0];
		desc->blocks[0].num = src_nents;
		xaction->srcs_blocks = &desc->blocks[0];
		xaction->num_of_srcs = 1;
		xaction->total_src_bufs = src_nents;

		/* use next bufs and block for destination buffers/blocks */
		for_each_sg(dst_sg, sg, dst_nents, i) {
			desc->bufs[src_nents + i].addr = sg_dma_address(sg);
			desc->bufs[src_nents + i].len = sg_dma_len(sg);
			dma_descriptor_unmap(&desc->txd);
			total_dst_len += desc->bufs[src_nents + i].len;
			dev_dbg(chan->device->common.dev,
			"%s: dst[%d] = %llx len 0x%x\n",
			__func__, i,
			(unsigned long long)desc->bufs[src_nents + i].addr,
			desc->bufs[src_nents + i].len);
		}

		desc->umap_ent_cnt = umap_ent_cnt;

		desc->blocks[1].bufs = &desc->bufs[src_nents];
		desc->blocks[1].num = dst_nents;
		xaction->dsts_blocks = &desc->blocks[1];
		xaction->num_of_dsts = 1;
		xaction->total_dst_bufs = dst_nents;

		dev_dbg(
			chan->device->common.dev,
			"%s: xaction->flags = %08x\n",
			__func__,
			xaction->flags);

		if (total_src_len != total_dst_len) {
			dev_err(chan->device->common.dev,
				"%s: src len 0x%x doesn't match dst len 0x%x!\n",
				__func__, total_src_len, total_dst_len);
		}

		/* send raid transaction to engine */
		rc = al_raid_dma_prepare(chan->hal_raid, chan->idx,
					&desc->hal_xaction);
		if (unlikely(rc)) {
			dev_err(chan->device->common.dev,
				"%s: al_raid_dma_prepare failed!\n", __func__);
			spin_unlock_bh(&chan->prep_lock);
			return NULL;
		}

		chan->tx_desc_produced += desc->hal_xaction.tx_descs_count;

		idx++;
		sw_desc_num_req--;
	}
	AL_DMA_STATS_UPDATE(
		chan,
		chan->stats_prep.sg_memcpy_num,
		1,
		chan->stats_prep.sg_memcpy_size,
		total_src_len);


	al_dma_tx_submit_sw_cond_unlock(chan, txd);

	return txd;
}

