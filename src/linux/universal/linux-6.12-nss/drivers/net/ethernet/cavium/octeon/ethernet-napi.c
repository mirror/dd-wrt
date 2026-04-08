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

/* This file is included in ethernet-rx.c *twice* */

#undef CVM_OCT_NAPI_POLL
#undef CVM_OCT_NAPI_HAS_CN68XX_SSO

#ifdef CVM_OCT_NAPI_68
#define CVM_OCT_NAPI_POLL cvm_oct_napi_poll_68
#define CVM_OCT_NAPI_HAS_CN68XX_SSO 1
#else
#define CVM_OCT_NAPI_POLL cvm_oct_napi_poll_38
#define CVM_OCT_NAPI_HAS_CN68XX_SSO 0
#endif

/**
 * cvm_oct_napi_poll - the NAPI poll function.
 * @napi: The NAPI instance, or null if called from cvm_oct_poll_controller
 * @budget: Maximum number of packets to receive.
 *
 * Returns the number of packets processed.
 */
static int CVM_OCT_NAPI_POLL(struct napi_struct *napi, int budget)
{
	const int	coreid = cvmx_get_core_num();
	int		no_work_count = 0;
	u64		old_group_mask;
	u64		old_scratch;
	int		rx_count = 0;
	bool		did_work_request = false;
	bool		packet_copied;

	char		*p = (char *)cvm_oct_by_pkind;

	/* Prefetch cvm_oct_device since we know we need it soon */
	prefetch(&p[0]);
	prefetch(&p[SMP_CACHE_BYTES]);
	prefetch(&p[2 * SMP_CACHE_BYTES]);

	if (USE_ASYNC_IOBDMA) {
		/* Save scratch in case userspace is using it */
		CVMX_SYNCIOBDMA;
		old_scratch = cvmx_scratch_read64(CVMX_SCR_SCRATCH);
	}

	/* Only allow work for our group (and preserve priorities) */
	if (CVM_OCT_NAPI_HAS_CN68XX_SSO) {
		old_group_mask = cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
		cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(coreid),
			       1ull << pow_receive_group);
		/* Read it back so it takes effect before we request work */
		cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
	} else {
		old_group_mask = cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(coreid));
		cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid),
			       (old_group_mask & ~0xFFFFull) | 1 << pow_receive_group);
	}

	if (USE_ASYNC_IOBDMA) {
		cvmx_pow_work_request_async(CVMX_SCR_SCRATCH, CVMX_POW_NO_WAIT);
		did_work_request = true;
	}

	while (rx_count < budget) {
		struct sk_buff *skb = NULL;
		struct sk_buff **pskb = NULL;
		struct octeon_ethernet *priv;
		enum cvm_oct_callback_result callback_result;
		bool skb_in_hw;
		cvmx_wqe_t *work;
		int port;
		unsigned int segments;
		int packets_to_replace = 0;
		unsigned int packet_len;

		union cvmx_buf_ptr  packet_ptr;

		if (USE_ASYNC_IOBDMA && did_work_request)
			work = cvmx_pow_work_response_async(CVMX_SCR_SCRATCH);
		else
			work = cvmx_pow_work_request_sync(CVMX_POW_NO_WAIT);

		did_work_request = false;
		if (unlikely(work == NULL)) {
			/* It takes so long to get here, so lets wait
			 * around a little to see if another packet
			 * comes in.
			 */
			if (no_work_count >= 20)
				break;
			no_work_count++;
			continue;
		}
		no_work_count = 0;

		packet_ptr = work->packet_ptr;
		pskb = cvm_oct_packet_to_skb(cvm_oct_get_buffer_ptr(packet_ptr));
		prefetch(pskb);

		if (likely(USE_ASYNC_IOBDMA && rx_count < (budget - 1))) {
			cvmx_pow_work_request_async_nocheck(CVMX_SCR_SCRATCH, CVMX_POW_NO_WAIT);
			did_work_request = true;
		}

		// HY 07/12/2021: Do not let other CPU get napi irq, risk of disordering packets
		/* Check every 8th packet if there is enough work waiting to
		 * merit waking another CPU.
		 */
		/*
		if (unlikely((rx_count % 8) == 0)) {
			int backlog;
			int cores_in_use = core_state.active_cores;
			if (CVM_OCT_NAPI_HAS_CN68XX_SSO) {
				union cvmx_sso_wq_int_cntx counts;
				counts.u64 = cvmx_read_csr(CVMX_SSO_WQ_INT_CNTX(pow_receive_group));
				backlog = counts.s.iq_cnt + counts.s.ds_cnt;
			} else {
				union cvmx_pow_wq_int_cntx counts;
				counts.u64 = cvmx_read_csr(CVMX_POW_WQ_INT_CNTX(pow_receive_group));
				backlog = counts.s.iq_cnt + counts.s.ds_cnt;
			}
			if (backlog > rx_cpu_factor * cores_in_use &&
			    napi != NULL &&
			    cores_in_use < core_state.baseline_cores)
				cvm_oct_enable_one_cpu();
		}*/
#ifndef CAVIUM_NET_PACKET_FWD_OFFLOAD
		rx_count++;
#endif

		/* If WORD2[SOFTWARE] then this WQE is a complete for
		 * a TX packet.
		 */
		if (work->word2.s.software) {
			struct octeon_ethernet *priv;
			int packet_qos = work->word0.raw.unused;

			skb = (struct sk_buff *)packet_ptr.u64;
			priv = netdev_priv(skb->dev);
			if (netif_queue_stopped(skb->dev))
				netif_wake_queue(skb->dev);
			if (unlikely((skb_shinfo(skb)->tx_flags | SKBTX_IN_PROGRESS) != 0 &&
				     priv->tx_timestamp_hw)) {
					u64 ns = *(u64 *)work->packet_data;
					struct skb_shared_hwtstamps ts;
					memset(&ts, 0, sizeof(ts));
					ts.hwtstamp = ns_to_ktime(ns);
					skb_tstamp_tx(skb, &ts);
			}
			dev_kfree_skb_any(skb);

			cvmx_fpa1_free(work, wqe_pool, DONT_WRITEBACK(1));

			/* We are done with this one, adjust the queue
			 * depth.
			 */
			cvmx_hwfau_atomic_add32(priv->tx_queue[packet_qos].fau, -1);
			continue;
		}
		segments = work->word2.s.bufs;
		skb_in_hw = USE_SKBUFFS_IN_HW && segments > 0;
		if (likely(skb_in_hw)) {
			skb = *pskb;
			prefetch(&skb->head);
			prefetch(&skb->len);
		}

		if (CVM_OCT_NAPI_HAS_CN68XX_SSO)
			port = work->word0.pip.cn68xx.pknd;
		else
			port = work->word1.cn38xx.ipprt;

		prefetch(cvm_oct_by_pkind[port]);

		/* Immediately throw away all packets with receive errors */
		if (unlikely(work->word2.snoip.rcv_error)) {
			if (cvm_oct_check_rcv_error(work))
				continue;
		}

		if (CVM_OCT_NAPI_HAS_CN68XX_SSO) {
			if (unlikely(cvm_oct_by_pkind[port] == NULL))
				priv = cvm_oct_dev_for_port(work->word2.s_cn68xx.port);
			else
				priv = cvm_oct_by_pkind[port];
		} else {
			/* srio priv is based on mbox, not port */
			if (port >= 40 && port <= 47)
				priv = NULL;
			else
				priv = cvm_oct_by_pkind[port];
		}

		if (likely(priv) && priv->rx_strip_fcs)
			work->word1.len -= 4;

		packet_len = work->word1.len;
		/* We can only use the zero copy path if skbuffs are
		 * in the FPA pool and the packet fits in a single
		 * buffer.
		 */
		if (likely(skb_in_hw)) {
			skb->data = phys_to_virt(packet_ptr.s.addr);
			prefetch(skb->data);
			skb->len = packet_len;
			packets_to_replace = segments;
			if (likely(segments == 1)) {
				skb_set_tail_pointer(skb, skb->len);
			} else {
				struct sk_buff *current_skb = skb;
				struct sk_buff *next_skb = NULL;
				unsigned int segment_size;
				bool first_frag = true;

				skb_frag_list_init(skb);
				/* Multi-segment packet. */
				for (;;) {
					/* Octeon Errata PKI-100: The segment size is
					 * wrong. Until it is fixed, calculate the
					 * segment size based on the packet pool
					 * buffer size. When it is fixed, the
					 * following line should be replaced with this
					 * one: int segment_size =
					 * segment_ptr.s.size;
					 */
					segment_size = FPA_PACKET_POOL_SIZE -
						(packet_ptr.s.addr - (((packet_ptr.s.addr >> 7) - packet_ptr.s.back) << 7));
					if (segment_size > packet_len)
						segment_size = packet_len;
					if (!first_frag) {
						current_skb->len = segment_size;
						skb->data_len += segment_size;
						skb->truesize += current_skb->truesize;
					}
					skb_set_tail_pointer(current_skb, segment_size);
					packet_len -= segment_size;
					segments--;
					if (segments == 0)
						break;
					packet_ptr = *(union cvmx_buf_ptr *)phys_to_virt(packet_ptr.s.addr - 8);
					next_skb = *cvm_oct_packet_to_skb(cvm_oct_get_buffer_ptr(packet_ptr));
					if (first_frag) {
						next_skb->next = skb_shinfo(current_skb)->frag_list;
						skb_shinfo(current_skb)->frag_list = next_skb;
					} else {
						current_skb->next = next_skb;
						next_skb->next = NULL;
					}
					current_skb = next_skb;
					first_frag = false;
					current_skb->data = phys_to_virt(packet_ptr.s.addr);
				}
			}
			packet_copied = false;
		} else {
			/* We have to copy the packet. First allocate
			 * an skbuff for it.
			 */
			skb = dev_alloc_skb(packet_len);
			if (!skb) {
				printk_ratelimited("Port %d failed to allocate skbuff, packet dropped\n",
						   port);
				cvm_oct_free_work(work);
				continue;
			}

			/* Check if we've received a packet that was
			 * entirely stored in the work entry.
			 */
			if (unlikely(work->word2.s.bufs == 0)) {
				u8 *ptr = work->packet_data;

				if (likely(!work->word2.s.not_IP)) {
					/* The beginning of the packet
					 * moves for IP packets.
					 */
					if (work->word2.s.is_v6)
						ptr += 2;
					else
						ptr += 6;
				}
				skb_put_data(skb, ptr, packet_len);
				/* No packet buffers to free */
			} else {
				int segments = work->word2.s.bufs;
				union cvmx_buf_ptr segment_ptr = work->packet_ptr;

				while (segments--) {
					union cvmx_buf_ptr next_ptr =
					    *(union cvmx_buf_ptr *)phys_to_virt(segment_ptr.s.addr - 8);

			/* Octeon Errata PKI-100: The segment size is
			 * wrong. Until it is fixed, calculate the
			 * segment size based on the packet pool
			 * buffer size. When it is fixed, the
			 * following line should be replaced with this
			 * one: int segment_size =
			 * segment_ptr.s.size;
			 */
					int segment_size = FPA_PACKET_POOL_SIZE -
						(segment_ptr.s.addr - (((segment_ptr.s.addr >> 7) - segment_ptr.s.back) << 7));
					/* Don't copy more than what
					 * is left in the packet.
					 */
					if (segment_size > packet_len)
						segment_size = packet_len;
					/* Copy the data into the packet */
					skb_put_data(skb, phys_to_virt(segment_ptr.s.addr),
			    			segment_size);

					packet_len -= segment_size;
					segment_ptr = next_ptr;
				}
			}
			packet_copied = true;
		}
		/* srio priv is based on mbox, not port */
		if (!CVM_OCT_NAPI_HAS_CN68XX_SSO && unlikely(priv == NULL)) {
			const struct cvmx_srio_rx_message_header *rx_header =
				(const struct cvmx_srio_rx_message_header *)skb->data;
			*(u64 *)rx_header = be64_to_cpu(*(u64 *)rx_header);
			priv = cvm_oct_by_srio_mbox[(port - 40) >> 1][rx_header->word0.s.mbox];
		}

		if (likely(priv)) {
#ifdef CONFIG_RAPIDIO
			if (unlikely(priv->imode == CVMX_HELPER_INTERFACE_MODE_SRIO)) {
				__skb_pull(skb, sizeof(struct cvmx_srio_rx_message_header));

				atomic64_add(1, (atomic64_t *)&priv->netdev->stats.rx_packets);
				atomic64_add(skb->len, (atomic64_t *)&priv->netdev->stats.rx_bytes);
			}
#endif
			/* Only accept packets for devices that are
			 * currently up.
			 */
			if (likely(priv->netdev->flags & IFF_UP)) {
				if (priv->rx_timestamp_hw) {
					/* The first 8 bytes are the timestamp */
					u64 ns = *(u64 *)skb->data;
					struct skb_shared_hwtstamps *ts;
					memset(&ts, 0, sizeof(ts));
					ts = skb_hwtstamps(skb);
					ts->hwtstamp = ns_to_ktime(ns);
					__skb_pull(skb, 8);
				}
				skb->protocol = eth_type_trans(skb, priv->netdev);
				skb->dev = priv->netdev;

				if (unlikely(work->word2.s.not_IP || work->word2.s.IP_exc ||
					work->word2.s.L4_error || !work->word2.s.tcp_or_udp))
					skb->ip_summed = CHECKSUM_NONE;
				else
					skb->ip_summed = CHECKSUM_UNNECESSARY;

				/* Increment RX stats for virtual ports */
				if (port >= CVMX_PIP_NUM_INPUT_PORTS) {
					atomic64_add(1, (atomic64_t *)&priv->netdev->stats.rx_packets);
					atomic64_add(skb->len, (atomic64_t *)&priv->netdev->stats.rx_bytes);
				}
				if (priv->intercept_cb) {
					callback_result = priv->intercept_cb(priv->netdev, work, skb);
					switch (callback_result) {
					case CVM_OCT_PASS:
						netif_receive_skb(skb);
						break;
					case CVM_OCT_DROP:
						dev_kfree_skb_any(skb);
						atomic64_add(1, (atomic64_t *)&priv->netdev->stats.rx_dropped);
						break;
					case CVM_OCT_TAKE_OWNERSHIP_WORK:
						/*
						 * Interceptor took
						 * our work, but we
						 * need to free the
						 * skbuff
						 */
						if (USE_SKBUFFS_IN_HW && likely(!packet_copied)) {
							/*
							 * We can't free the skbuff since its data is
							 * the same as the work. In this case we don't
							 * do anything
							 */
						} else {
							dev_kfree_skb_any(skb);
						}
						break;
					case CVM_OCT_TAKE_OWNERSHIP_SKB:
						/* Interceptor took our packet */
						break;
					}
#ifdef CAVIUM_NET_PACKET_FWD_OFFLOAD
					rx_count++;
#endif
				} else {
					netif_receive_skb(skb);
					callback_result = CVM_OCT_PASS;
#ifdef CAVIUM_NET_PACKET_FWD_OFFLOAD
					rx_count++;
#endif
				}
			} else {
				/* Drop any packet received for a device that isn't up */
				atomic64_add(1, (atomic64_t *)&priv->netdev->stats.rx_dropped);
				dev_kfree_skb_any(skb);
				callback_result = CVM_OCT_DROP;

			}
		} else {
			/* Drop any packet received for a device that
			 * doesn't exist.
			 */
			printk_ratelimited("Port %d not controlled by Linux, packet dropped\n",
					   port);
			dev_kfree_skb_any(skb);
			callback_result = CVM_OCT_DROP;
		}
		/* We only need to free the work if the interceptor didn't
		   take over ownership of it */
		if (callback_result != CVM_OCT_TAKE_OWNERSHIP_WORK) {
			/* Check to see if the skbuff and work share the same
			 * packet buffer.
			 */
			if (USE_SKBUFFS_IN_HW && likely(!packet_copied)) {
				/* This buffer needs to be replaced, increment
				 * the number of buffers we need to free by
				 * one.
				 */
				cvmx_hwfau_atomic_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE,
						      packets_to_replace);

				cvmx_fpa1_free(work, wqe_pool, DONT_WRITEBACK(1));
			} else {
				cvm_oct_free_work(work);
			}
		}
	}
	/* Restore the original POW group mask */
	if (CVM_OCT_NAPI_HAS_CN68XX_SSO) {
		cvmx_write_csr(CVMX_SSO_PPX_GRP_MSK(coreid), old_group_mask);
		/* Read it back so it takes effect before ?? */
		cvmx_read_csr(CVMX_SSO_PPX_GRP_MSK(coreid));
	} else {
		cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid), old_group_mask);
	}
	if (USE_ASYNC_IOBDMA) {
		/* Restore the scratch area */
		cvmx_scratch_write64(CVMX_SCR_SCRATCH, old_scratch);
	}
	cvm_oct_rx_refill_pool(0);

	if (rx_count < budget && napi != NULL) {
		/* No more work */
		napi_complete_done(napi, rx_count);
		cvm_oct_no_more_work(napi);
	}
	return rx_count;
}
