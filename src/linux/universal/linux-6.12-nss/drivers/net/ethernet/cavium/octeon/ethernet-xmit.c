/**********************************************************************
 * Author: Cavium, Inc.
 *
 * Contact: support@cavium.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2012 Cavium, Inc.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium, Inc. for more information
 **********************************************************************/

#undef CVM_OCT_XMIT
#undef CVM_OCT_PKO_LOCK_TYPE

#ifdef CVM_OCT_LOCKLESS
#define CVM_OCT_XMIT cvm_oct_xmit_lockless
#define CVM_OCT_PKO_LOCK_TYPE CVMX_PKO_LOCK_NONE
#else
#define CVM_OCT_XMIT cvm_oct_xmit
#define CVM_OCT_PKO_LOCK_TYPE CVMX_PKO_LOCK_CMD_QUEUE
#endif

/**
 * cvm_oct_xmit - transmit a packet
 * @skb:    Packet to send
 * @dev:    Device info structure
 *
 * Returns Always returns NETDEV_TX_OK
 */
int
CVM_OCT_XMIT
(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *skb_tmp;
	cvmx_pko_command_word0_t pko_command;
	union cvmx_buf_ptr hw_buffer;
	u64 old_scratch;
	u64 old_scratch2;
	int qos;
	int i;
	int frag_count;
	enum {QUEUE_HW, QUEUE_WQE, QUEUE_DROP, QUEUE_DROP_NO_DEC} queue_type;
	struct octeon_ethernet *priv = netdev_priv(dev);
	s32 queue_depth;
	s32 buffers_to_free;
	s32 buffers_being_recycled;
	unsigned long flags;
	cvmx_wqe_t *work = NULL;
	bool timestamp_this_skb = false;
	bool can_do_reuse = true;
	int ret = NETDEV_TX_OK;

	/* Prefetch the private data structure.  It is larger than one
	 * cache line.
	 */
	prefetch(priv);

	if (USE_ASYNC_IOBDMA) {
		/* Save scratch in case userspace is using it */
		CVMX_SYNCIOBDMA;
		old_scratch = cvmx_scratch_read64(CVMX_SCR_SCRATCH);
		old_scratch2 = cvmx_scratch_read64(CVMX_SCR_SCRATCH + 8);

		/* Fetch and increment the number of packets to be
		 * freed.
		 */
		cvmx_hwfau_async_fetch_and_add32(CVMX_SCR_SCRATCH + 8,
					       FAU_NUM_PACKET_BUFFERS_TO_FREE,
					       0);
	}

	frag_count = 0;
	if (skb_has_frag_list(skb))
		skb_walk_frags(skb, skb_tmp)
			frag_count++;
	/* We have space for 12 segment pointers, If there will be
	 * more than that, we must linearize.  The count is: 1 (base
	 * SKB) + frag_count + nr_frags.
	 */
	if (unlikely(skb_shinfo(skb)->nr_frags + frag_count > 11)) {
		if (unlikely(__skb_linearize(skb))) {
			dev_kfree_skb_any(skb);
			dev->stats.tx_dropped++;
			goto post_preempt_out;
		}
		frag_count = 0;
	}

	/* We cannot move to a different CPU once we determine our
	 * queue number/qos
	 */
	preempt_disable();
#ifdef CVM_OCT_LOCKLESS
	qos = cvmx_get_core_num();
#else
	/* The check on CVMX_PKO_QUEUES_PER_PORT_* is designed to
	 * completely remove "qos" in the event neither interface
	 * supports multiple queues per port.
	 */
	if (priv->tx_multiple_queues) {
		qos = GET_SKBUFF_QOS(skb);
		if (qos <= 0)
			qos = 0;
		else if (qos >= priv->num_tx_queues)
			qos = 0;
	} else
		qos = 0;
#endif
	if (USE_ASYNC_IOBDMA) {
		cvmx_hwfau_async_fetch_and_add32(CVMX_SCR_SCRATCH,
					       priv->tx_queue[qos].fau, 1);
	}

#ifndef CVM_OCT_LOCKLESS
	/* The CN3XXX series of parts has an errata (GMX-401) which
	 * causes the GMX block to hang if a collision occurs towards
	 * the end of a <68 byte packet. As a workaround for this, we
	 * pad packets to be 68 bytes whenever we are in half duplex
	 * mode. We don't handle the case of having a small packet but
	 * no room to add the padding.  The kernel should always give
	 * us at least a cache line
	 */
	if ((skb->len < 64) && OCTEON_IS_MODEL(OCTEON_CN3XXX)) {
		union cvmx_gmxx_prtx_cfg gmx_prt_cfg;

		if (priv->interface < 2) {
			/* We only need to pad packet in half duplex mode */
			gmx_prt_cfg.u64 = cvmx_read_csr(CVMX_GMXX_PRTX_CFG(priv->interface_port, priv->interface));
			if (gmx_prt_cfg.s.duplex == 0) {
				int add_bytes = 64 - skb->len;
				if ((skb_tail_pointer(skb) + add_bytes) <= skb_end_pointer(skb))
					__skb_put_zero(skb, add_bytes);
			}
		}
	}
#endif
	/* Build the PKO command */
	pko_command.u64 = 0;
#ifdef __LITTLE_ENDIAN
	pko_command.s.le = 1;
#endif
	/* Don't pollute L2 with the outgoing packet */
#ifdef CONFIG_CAVIUM_NET_PACKET_FWD_OFFLOAD
	pko_command.s.n2 = 0;
#else
	pko_command.s.n2 = 1;
#endif
	pko_command.s.segs = 1;
	pko_command.s.total_bytes = skb->len;
	/* Use fau0 to decrement the number of packets queued */
	pko_command.s.size0 = CVMX_FAU_OP_SIZE_32;
	pko_command.s.subone0 = 1;
	pko_command.s.reg0 = priv->tx_queue[qos].fau;
	pko_command.s.dontfree = 1;

	/* Build the PKO buffer pointer */
	hw_buffer.u64 = 0; /* Implies pool == 0, i == 0 */
	if (skb_shinfo(skb)->nr_frags == 0 && frag_count == 0) {
		hw_buffer.s.addr = virt_to_phys(skb->data);
		hw_buffer.s.size = skb->len;
		cvm_oct_set_back(skb, &hw_buffer);
		buffers_being_recycled = 1;
	} else {
		u64 *hw_buffer_list;

		work = cvmx_fpa1_alloc(wqe_pool);
		if (unlikely(!work)) {
			netdev_err(dev, "Failed WQE allocate\n");
			queue_type = USE_ASYNC_IOBDMA ? QUEUE_DROP : QUEUE_DROP_NO_DEC;
			goto skip_xmit;
		}
		hw_buffer_list = (u64 *)work->packet_data;
		hw_buffer.s.addr = virt_to_phys(skb->data);
		hw_buffer.s.size = skb_headlen(skb);
		if (skb_shinfo(skb)->nr_frags == 0 && cvm_oct_skb_ok_for_reuse(skb)) {
			cvm_oct_set_back(skb, &hw_buffer);
		} else {
			hw_buffer.s.back = 0;
			can_do_reuse = false;
		}
		hw_buffer_list[0] = hw_buffer.u64;
		for (i = 1; i <= skb_shinfo(skb)->nr_frags; i++) {
			skb_frag_t *fs = skb_shinfo(skb)->frags + i - 1;
			hw_buffer.s.addr = XKPHYS_TO_PHYS((uintptr_t)skb_frag_address(fs));
			hw_buffer.s.size = skb_frag_size(fs);
			hw_buffer_list[i] = hw_buffer.u64;
			can_do_reuse = false;
		}
		skb_walk_frags(skb, skb_tmp) {
			hw_buffer.s.addr = virt_to_phys(skb_tmp->data);
			hw_buffer.s.size = skb_tmp->len;
			if (cvm_oct_skb_ok_for_reuse(skb_tmp)) {
				cvm_oct_set_back(skb_tmp, &hw_buffer);
			} else {
				hw_buffer.s.back = 0;
				can_do_reuse = false;
			}
			hw_buffer_list[i] = hw_buffer.u64;
			i++;
		}
		hw_buffer.s.addr = virt_to_phys(hw_buffer_list);
		hw_buffer.s.size = i;
		hw_buffer.s.back = 0;
		hw_buffer.s.pool = wqe_pool;
		buffers_being_recycled = i;
		pko_command.s.segs = hw_buffer.s.size;
		pko_command.s.gather = 1;
	}

	if (unlikely(priv->tx_timestamp_hw) &&
	    (skb_shinfo(skb)->tx_flags & SKBTX_HW_TSTAMP)) {
		timestamp_this_skb = true;
		skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
		can_do_reuse = false;
	}

	/* See if we can put this skb in the FPA pool. Any strange
	 * behavior from the Linux networking stack will most likely
	 * be caused by a bug in the following code. If some field is
	 * in use by the network stack and get carried over when a
	 * buffer is reused, bad thing may happen.  If in doubt and
	 * you dont need the absolute best performance, set recycle_tx
	 * to zero . The reuse of buffers has shown a 25% increase in
	 * performance under some loads.
	 */
	if (octeon_recycle_tx && can_do_reuse &&
	    cvm_oct_skb_ok_for_reuse(skb) &&
	    likely(!skb_header_cloned(skb)) &&
	    likely(!skb->destructor))
		/* We can use this buffer in the FPA.  We don't need
		 * the FAU update anymore
		 */
		pko_command.s.dontfree = 0;

	/* Check if we can use the hardware checksumming */
	if (USE_HW_TCPUDP_CHECKSUM && skb->ip_summed != CHECKSUM_NONE &&
	    skb->ip_summed != CHECKSUM_UNNECESSARY) {
		/* Use hardware checksum calc */
		pko_command.s.ipoffp1 = sizeof(struct ethhdr) + 1;
		if (unlikely(priv->imode == CVMX_HELPER_INTERFACE_MODE_SRIO))
			pko_command.s.ipoffp1 += 8;
	}

	if (USE_ASYNC_IOBDMA) {
		/* Get the number of skbuffs in use by the hardware */
		CVMX_SYNCIOBDMA;
		queue_depth = cvmx_scratch_read64(CVMX_SCR_SCRATCH);
		buffers_to_free = cvmx_scratch_read64(CVMX_SCR_SCRATCH + 8);
	} else {
		/* Get the number of skbuffs in use by the hardware */
		queue_depth = cvmx_hwfau_fetch_and_add32(priv->tx_queue[qos].fau, 1);
		buffers_to_free = cvmx_hwfau_fetch_and_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE, 0);
	}

	/* If we're sending faster than the receive can free them then
	 * don't do the HW free.
	 */
	if (unlikely(buffers_to_free < -100))
		pko_command.s.dontfree = 1;

	/* Drop this packet if we have too many already queued to the HW */
	if (unlikely(queue_depth >= MAX_OUT_QUEUE_DEPTH)) {
		if (dev->priv_flags & IFF_NO_QUEUE) {
			/* not using normal queueing.  */
			queue_type = QUEUE_DROP;
			goto skip_xmit;
		} else {
			netif_stop_queue(dev);
		}
	}

	if (pko_command.s.dontfree) {
		queue_type = QUEUE_WQE;
	} else {
		queue_type = QUEUE_HW;
		if (buffers_being_recycled > 1) {
			struct sk_buff *tskb, *nskb;
			/* We are committed to use hardware free, restore the
			 * frag list to empty on the first SKB
			 */
			tskb = skb_shinfo(skb)->frag_list;
			while (tskb) {
				nskb = tskb->next;
				cvm_oct_skb_prepare_for_reuse(tskb);
				tskb = nskb;
			}
		}
		cvm_oct_skb_prepare_for_reuse(skb);
	}

	if (queue_type == QUEUE_WQE) {
		if (!work) {
			work = cvmx_fpa1_alloc(wqe_pool);
			if (unlikely(!work)) {
				netdev_err(dev, "Failed WQE allocate\n");
				queue_type = QUEUE_DROP;
				goto skip_xmit;
			}
		}

		pko_command.s.rsp = 1;
		pko_command.s.wqp = 1;
		/* work->unused will carry the qos for this packet,
		 * this allows us to find the proper FAU when freeing
		 * the packet.  We decrement the FAU when the WQE is
		 * replaced in the pool.
		 */
		pko_command.s.reg0 = 0;
		work->word0.u64 = 0;
		work->word0.raw.unused = (u8)qos;

		work->word1.u64 = 0;
		work->word1.tag_type = CVMX_POW_TAG_TYPE_NULL;
		work->word1.tag = 0;
		work->word2.u64 = 0;
		work->word2.s.software = 1;
		cvmx_wqe_set_grp(work, pow_receive_group);
		work->packet_ptr.u64 = (unsigned long)skb;
	}

	local_irq_save(flags);

	cvmx_pko_send_packet_prepare_pkoid(priv->pko_port,
					   priv->tx_queue[qos].queue,
					   CVM_OCT_PKO_LOCK_TYPE);

	/* Send the packet to the output queue */
	if (queue_type == QUEUE_WQE) {
		u64 word2 = virt_to_phys(work);
		if (timestamp_this_skb)
			word2 |= 1ull << 40; /* Bit 40 controls timestamps */

		if (unlikely(cvmx_hwpko_send_packet_finish3_pkoid(priv->pko_port,
							  priv->tx_queue[qos].queue, pko_command, hw_buffer,
							  word2, CVM_OCT_PKO_LOCK_TYPE))) {
				queue_type = QUEUE_DROP;
				netdev_err(dev, "Failed to send the packet with wqe\n");
		}
	} else {
		if (unlikely(cvmx_hwpko_send_packet_finish_pkoid(priv->pko_port,
							 priv->tx_queue[qos].queue,
							 pko_command, hw_buffer,
							 CVM_OCT_PKO_LOCK_TYPE))) {
			netdev_err(dev, "Failed to send the packet\n");
			queue_type = QUEUE_DROP;
		}
	}
	local_irq_restore(flags);

skip_xmit:
	switch (queue_type) {
	case QUEUE_DROP:
		cvmx_hwfau_atomic_add32(priv->tx_queue[qos].fau, -1);
		/* Fall through */
	case QUEUE_DROP_NO_DEC:
		ret = NETDEV_TX_BUSY; // inform QoS layer
		if (work)
			cvmx_fpa1_free(work, wqe_pool, DONT_WRITEBACK(1));
		break;
	case QUEUE_HW:
		cvmx_hwfau_atomic_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE, -buffers_being_recycled);
		break;
	case QUEUE_WQE:
		/* Cleanup is done on the RX path when the WQE returns */
		break;
	default:
		BUG();
	}
	preempt_enable();
post_preempt_out:
	if (USE_ASYNC_IOBDMA) {
		CVMX_SYNCIOBDMA;
		/* Restore the scratch area */
		cvmx_scratch_write64(CVMX_SCR_SCRATCH, old_scratch);
		cvmx_scratch_write64(CVMX_SCR_SCRATCH + 8, old_scratch2);
	}

	return ret;
}
