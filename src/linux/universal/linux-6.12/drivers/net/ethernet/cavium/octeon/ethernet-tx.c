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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/ratelimit.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <net/dst.h>

#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"

#include <asm/octeon/cvmx-wqe.h>
#include <asm/octeon/cvmx-hwfau.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-pip.h>
#include <asm/octeon/cvmx-hwpko.h>
#include <asm/octeon/cvmx-helper.h>

#include <asm/octeon/cvmx-gmxx-defs.h>

#ifdef CONFIG_CAVIUM_IPFWD_OFFLOAD
#include "ipfwd_config.h"
#endif

/*
 * You can define GET_SKBUFF_QOS() to override how the skbuff output
 * function determines which output queue is used. The default
 * implementation always uses the base queue for the port. If, for
 * example, you wanted to use the skb->priority fieid, define
 * GET_SKBUFF_QOS as: #define GET_SKBUFF_QOS(skb) ((skb)->priority)
 */
#ifndef GET_SKBUFF_QOS
#if defined(CONFIG_CAVIUM_IPFWD_OFFLOAD) && defined(IPFWD_OUTPUT_QOS)
#define GET_SKBUFF_QOS(skb) ((skb)->cvm_info.qos_level)
#else
#define GET_SKBUFF_QOS(skb) 0
#endif
#endif

static bool cvm_oct_skb_ok_for_reuse(struct sk_buff *skb)
{
	unsigned char *fpa_head = cvm_oct_get_fpa_head(skb);

	if (*(struct sk_buff **)(fpa_head - sizeof(void *)) != skb)
		return false;

	if (unlikely(skb->data < fpa_head))
		return false;

	if (unlikely(fpa_head - skb->head < sizeof(void *)))
		return false;

	if (unlikely((skb_end_pointer(skb) - fpa_head) < FPA_PACKET_POOL_SIZE))
		return false;

	if (unlikely(skb_shared(skb)) ||
	    unlikely(skb_cloned(skb)) ||
	    unlikely(skb->fclone != SKB_FCLONE_UNAVAILABLE))
		return false;

	return true;
}

static void skb_recycle(struct sk_buff *skb)
{
	struct skb_shared_info *shinfo;

	skb_release_head_state(skb);

	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->data = skb->head + NET_SKB_PAD;
	skb_reset_tail_pointer(skb);
}

static void cvm_oct_skb_prepare_for_reuse(struct sk_buff *skb)
{
	unsigned char *fpa_head = cvm_oct_get_fpa_head(skb);

	skb->data_len = 0;
	skb_frag_list_init(skb);

	/* The check also resets all the fields. */
	skb_recycle(skb);

	*(struct sk_buff **)(fpa_head - sizeof(void *)) = skb;
	skb->truesize = sizeof(*skb) + skb_end_pointer(skb) - skb->head;
}

static inline void cvm_oct_set_back(struct sk_buff *skb,
				    union cvmx_buf_ptr *hw_buffer)
{
	unsigned char *fpa_head = cvm_oct_get_fpa_head(skb);

	hw_buffer->s.back = ((unsigned long)skb->data >> 7) - ((unsigned long)fpa_head >> 7);
}

#define CVM_OCT_LOCKLESS 1
#include "ethernet-xmit.c"

#undef CVM_OCT_LOCKLESS
#include "ethernet-xmit.c"

/**
 * cvm_oct_transmit_qos - transmit a work queue entry out of the ethernet port.
 *
 * Both the work queue entry and the packet data can optionally be
 * freed. The work will be freed on error as well.
 *
 * @dev: Device to transmit out.
 * @work_queue_entry: Work queue entry to send
 * @do_free: True if the work queue entry and packet data should be
 *           freed. If false, neither will be freed.
 * @qos: Index into the queues for this port to transmit on. This is
 *       used to implement QoS if their are multiple queues per
 *       port. This parameter must be between 0 and the number of
 *       queues per port minus 1. Values outside of this range will be
 *       change to zero.
 *
 * Returns Zero on success, negative on failure.
 */
int cvm_oct_transmit_qos(struct net_device *dev,
			 void *work_queue_entry,
			 int do_free,
			 int qos)
{
	unsigned long			flags;
	cvmx_buf_ptr_t			hw_buffer;
	cvmx_pko_command_word0_t	pko_command;
	int				dropped;
	struct octeon_ethernet		*priv = netdev_priv(dev);
	cvmx_wqe_t			*work = work_queue_entry;
	cvmx_pko_lock_t lock_type;

	if (!(dev->flags & IFF_UP)) {
		netdev_err(dev, "Error: Device not up\n");
		if (do_free)
			cvm_oct_free_work(work);
		return -1;
	}

	if (priv->tx_lockless) {
		qos = cvmx_get_core_num();
		lock_type = CVMX_PKO_LOCK_NONE;
	} else {
		/*
		 * The check on CVMX_PKO_QUEUES_PER_PORT_* is designed to
		 * completely remove "qos" in the event neither interface
		 * supports multiple queues per port
		 */
		if (priv->tx_multiple_queues) {
			if (qos <= 0)
				qos = 0;
			else if (qos >= priv->num_tx_queues)
				qos = 0;
		} else
			qos = 0;
		lock_type = CVMX_PKO_LOCK_CMD_QUEUE;
	}

	/* Start off assuming no drop */
	dropped = 0;

	local_irq_save(flags);

	cvmx_pko_send_packet_prepare_pkoid(priv->pko_port, priv->tx_queue[qos].queue, lock_type);

	/* Build the PKO buffer pointer */
	hw_buffer.u64 = 0;
	hw_buffer.s.addr = work->packet_ptr.s.addr;
	hw_buffer.s.pool = packet_pool;
	hw_buffer.s.size = FPA_PACKET_POOL_SIZE;
	hw_buffer.s.back = work->packet_ptr.s.back;

	/* Build the PKO command */
	pko_command.u64 = 0;
#ifdef CONFIG_CAVIUM_NET_PACKET_FWD_OFFLOAD
	pko_command.s.n2 = 0; /* pollute L2 with the outgoing packet */
#else
	pko_command.s.n2 = 1; /* Don't pollute L2 with the outgoing packet */
#endif
	pko_command.s.dontfree = !do_free;
	pko_command.s.segs = work->word2.s.bufs;
	pko_command.s.total_bytes = work->word1.len;

	/* Check if we can use the hardware checksumming */
	if (unlikely(work->word2.s.not_IP || work->word2.s.IP_exc))
		pko_command.s.ipoffp1 = 0;
	else
#ifdef CONFIG_CAVIUM_NET_PACKET_FWD_OFFLOAD
		pko_command.s.ipoffp1 = sizeof(struct ethhdr)
					+ 1 + cvmx_wqe_get_unused8(work);
#else
		pko_command.s.ipoffp1 = sizeof(struct ethhdr) + 1;
#endif

	/* Send the packet to the output queue */
	if (unlikely(cvmx_hwpko_send_packet_finish_pkoid(priv->pko_port, priv->tx_queue[qos].queue, pko_command, hw_buffer, lock_type))) {
		netdev_err(dev, "Error: Failed to send the packet\n");
		dropped = -1;
	}
	local_irq_restore(flags);

	if (unlikely(dropped)) {
		if (do_free)
			cvm_oct_free_work(work);
		dev->stats.tx_dropped++;
	} else
	if (do_free)
		cvmx_fpa1_free(work, wqe_pool, DONT_WRITEBACK(1));

	return dropped;
}
EXPORT_SYMBOL(cvm_oct_transmit_qos);
#ifdef CONFIG_CAVIUM_NET_PACKET_FWD_OFFLOAD
/**
 * Transmit a work queue entry out of the ethernet port.
 *
 * Both the work queue entry and the packet data can optionally be
 * freed. The work will be freed on error as well.
 *
 * @dev: Device to transmit out.
 * @work_queue_entry: Work queue entry to send
 * @skb: socket buffer
 * @qos: Index into the queues for this port to transmit on. This is
 *       used to implement QoS if their are multiple queues per
 *       port. This parameter must be between 0 and the number of
 *       queues per port minus 1. Values outside of this range will be
 *       change to zero.
 *
 * Returns Zero on success, negative on failure.
 */
int cvm_oct_transmit_qos_not_free(struct net_device *dev,
				void *work_queue_entry,
				struct sk_buff *skb)
{
	unsigned long			flags;
	cvmx_buf_ptr_t			hw_buffer;
	cvmx_pko_command_word0_t	pko_command;
	int				dropped;
	struct octeon_ethernet		*priv = netdev_priv(dev);
	cvmx_wqe_t			*work = work_queue_entry;
	cvmx_wqe_t			*rcv_work = NULL;
	cvmx_pko_lock_t lock_type;
	u64 word2;
	int qos;

	if (!(dev->flags & IFF_UP)) {
		netdev_err(dev, "Error: Device not up\n");
		if (work)
			cvm_oct_free_work(work);
		return -1;
	}

	if (priv->tx_lockless) {
		qos = cvmx_get_core_num();
		lock_type = CVMX_PKO_LOCK_NONE;
	} else {
		/* The check on CVMX_PKO_QUEUES_PER_PORT_*
		 * is designed to completely remove "qos"
		 * in the event neither interface
		 * supports multiple queues per port.
		 */
		if (priv->tx_multiple_queues) {
			qos = GET_SKBUFF_QOS(skb);
			if (qos <= 0)
				qos = 0;
			else if (qos >= priv->num_tx_queues)
				qos = 0;
		} else {
			qos = 0;
		}
		lock_type = CVMX_PKO_LOCK_CMD_QUEUE;
	}

	/* Start off assuming no drop */
	dropped = 0;

	/* Build the PKO buffer pointer */
	hw_buffer.u64 = 0;
	hw_buffer.s.addr = virt_to_phys(skb->data);
	hw_buffer.s.pool = packet_pool;
	hw_buffer.s.size = FPA_PACKET_POOL_SIZE;
	hw_buffer.s.back = work->packet_ptr.s.back;

	/* Build the PKO command */
	pko_command.u64 = 0;
	pko_command.s.n2 = 0; /* pollute L2 with the outgoing packet */
	pko_command.s.dontfree = 1;
	pko_command.s.segs = work->word2.s.bufs;
	pko_command.s.total_bytes = skb->len;

	/* Use fau0 to decrement the number of packets queued */
	pko_command.s.size0 = CVMX_FAU_OP_SIZE_32;
	pko_command.s.subone0 = 1;

	/* Check if we can use the hardware checksumming */
	if (unlikely(work->word2.s.not_IP || work->word2.s.IP_exc))
		pko_command.s.ipoffp1 = 0;
	else
		pko_command.s.ipoffp1 = sizeof(struct ethhdr) +
						1 + cvmx_wqe_get_unused8(work);

	rcv_work = cvmx_fpa_alloc(wqe_pool);
	if (unlikely(!work)) {
		netdev_err(dev, "failed to allocate rcv work\n");
		dropped = -1;
		goto skip_xmit;
	}

	pko_command.s.rsp = 1;
	pko_command.s.wqp = 1;

	pko_command.s.reg0 = 0;
	rcv_work->word0.u64 = 0;
	rcv_work->word0.raw.unused = (u8)qos;

	rcv_work->word1.u64 = 0;
	rcv_work->word1.tag_type = CVMX_POW_TAG_TYPE_NULL;
	rcv_work->word1.tag = 0;
	rcv_work->word2.u64 = 0;
	rcv_work->word2.s.software = 1;
	cvmx_wqe_set_grp(rcv_work, pow_receive_group);
	rcv_work->packet_ptr.u64 = (unsigned long)skb;

	word2 = virt_to_phys(rcv_work);

	local_irq_save(flags);

	cvmx_pko_send_packet_prepare_pkoid(priv->pko_port,
					priv->tx_queue[qos].queue,
					lock_type);

	/* Send the packet to the output queue */
	if (unlikely(cvmx_hwpko_send_packet_finish3_pkoid(
					priv->pko_port,
					priv->tx_queue[qos].queue,
					pko_command,
					hw_buffer,
					word2,
					lock_type))) {
		netdev_err(dev, "Failed to send the packet\n");
		dropped = -1;
	}
	local_irq_restore(flags);

skip_xmit:
	if (unlikely(dropped)) {
		cvmx_hwfau_atomic_add32(priv->tx_queue[qos].fau, -1);
		dev_kfree_skb_any(skb);
		dev->stats.tx_dropped++;
		if (work)
			cvmx_fpa_free(work, packet_pool, DONT_WRITEBACK(1));
		if (rcv_work)
			cvmx_fpa_free(rcv_work, wqe_pool, DONT_WRITEBACK(1));
	}

	return dropped;
}
EXPORT_SYMBOL(cvm_oct_transmit_qos_not_free);
#endif
