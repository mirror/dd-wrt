/*
 *  QCA HyFi Packet Aggregation
 *
 * Copyright (c) 2013-2014, 2016 The Linux Foundation. All rights reserved.
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
 */

#define DEBUG_LEVEL HYFI_NF_DEBUG_LEVEL

#include "hyfi_aggr.h"

/* Be careful, noisy and damages high rate flows when defined */
#undef HYFI_AGGR_NOISY_DEBUG

static struct net_bridge_port *hyfi_aggr_dequeue_pkts(
		struct net_hatbl_entry *ha, struct sk_buff **skb)
{
	u_int32_t i;
	u_int32_t quota = HYFI_AGGR_REORD_FLUSH_QUOTA;

	/* Look for other queued skbs with the current sequence */
	while (quota) {
		u_int32_t pkt_found = 0;

		for (i = 0; i < ha->aggr_rx_entry->num_ifs; i++) {
			if (ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt
					&& ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid
					&& ha->aggr_rx_entry->hyfi_iface_info[i].seq
							== ha->aggr_rx_entry->aggr_next_seq) {
				struct hyfi_skb_aggr_q *skb_aggr_q;
				struct hyfi_aggr_skb_buffer *hyfi_aggr_skb_buffer;

				pkt_found = 1;

				/* Found an skb. Pop it from the queue and return to the bridge */
				skb_aggr_q = &ha->aggr_rx_entry->hyfi_iface_info[i].skb_aggr_q;
				hyfi_aggr_skb_buffer = TAILQ_FIRST( skb_aggr_q );

				if (likely(hyfi_aggr_skb_buffer && hyfi_aggr_skb_buffer->skb)) {
					u_int32_t more_packets = 0;

					TAILQ_REMOVE( skb_aggr_q, hyfi_aggr_skb_buffer,
							skb_aggr_qelem);
					*skb = hyfi_aggr_skb_buffer->skb;

#ifdef HYFI_AGGR_NOISY_DEBUG /* Be careful, noisy and damages high rate flows */
					DEBUG_TRACE("%s: iface = %s, seq = %d\n", __func__, (*skb)->dev->name, hyfi_aggr_skb_buffer->pkt_seq);
#endif

					ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt--;

					hyfi_aggr_skb_buffer = TAILQ_FIRST( skb_aggr_q );

					if (hyfi_aggr_skb_buffer && hyfi_aggr_skb_buffer->skb) {
						/* That's the next sequence in that queue */
						ha->aggr_rx_entry->hyfi_iface_info[i].seq =
								hyfi_aggr_skb_buffer->pkt_seq;
					} else {
						/* No more skbs here, clear the queue */
						ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt = 0;
						ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid = 0;
						ha->aggr_rx_entry->hyfi_iface_info[i].ifindex = 0;
					}

					ha->aggr_rx_entry->aggr_next_seq++;
					ha->aggr_rx_entry->time_stamp = jiffies;

					quota--;

					if (quota) {
						u_int32_t j;

						for (j = 0; j < ha->aggr_rx_entry->num_ifs; j++) {
							if (ha->aggr_rx_entry->hyfi_iface_info[j].pkt_cnt
									&& ha->aggr_rx_entry->hyfi_iface_info[j].seq_valid) {
								more_packets = 1;
								break;
							}
						}
					}

					if (!quota || !more_packets) {
						return ha->dst;
					} else {
						hyfi_br_forward(ha->dst, *skb);
					}
				} else {
					/* Unlikely to happen, something's wrong */
					ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt = 0;
					ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid = 0;
					ha->aggr_rx_entry->hyfi_iface_info[i].ifindex = 0;

					*skb = NULL;
					return (struct net_bridge_port *) -1;
				}
			} else {
				continue;
			}
		}

		if (!pkt_found)
			break;
	}

	*skb = NULL;
	return (struct net_bridge_port *) -1;
}

static void hyfi_aggr_reset_queues(struct net_hatbl_entry *ha, u_int16_t seq)
{
	u_int32_t i;

	for (i = 0; i < ha->aggr_rx_entry->num_ifs; i++) {
		struct hyfi_skb_aggr_q *skb_aggr_q;
		struct hyfi_aggr_skb_buffer *hyfi_aggr_skb_buffer;

		/* Found an skb. Pop it from the queue and free it */
		skb_aggr_q = &ha->aggr_rx_entry->hyfi_iface_info[i].skb_aggr_q;
		hyfi_aggr_skb_buffer = TAILQ_FIRST( skb_aggr_q );

		while (hyfi_aggr_skb_buffer) {
			if (((ha->aggr_rx_entry->aggr_next_seq > hyfi_aggr_skb_buffer->pkt_seq)
					&& ((ha->aggr_rx_entry->aggr_next_seq - hyfi_aggr_skb_buffer->pkt_seq) < (1 << 13)))
					|| ((hyfi_aggr_skb_buffer->pkt_seq	> ha->aggr_rx_entry->aggr_next_seq)
							&& (hyfi_aggr_skb_buffer->pkt_seq - ha->aggr_rx_entry->aggr_next_seq > (1 << 13)))) {
				TAILQ_REMOVE( skb_aggr_q, hyfi_aggr_skb_buffer, skb_aggr_qelem);

				if (hyfi_aggr_skb_buffer->skb) {
					kfree_skb(hyfi_aggr_skb_buffer->skb);
				}

				hyfi_aggr_skb_buffer = TAILQ_FIRST( skb_aggr_q );
				ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt--;
			} else {
				break;
			}
		}

		if (!hyfi_aggr_skb_buffer)
			ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt = 0;

		if (!ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt) {
			ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid = 0;
		}
	}

}

static int hyfi_aggr_queue_pkt(struct net_hatbl_entry *ha, struct sk_buff **skb,
		u_int16_t seq)
{
	u_int32_t i, idx = HYFI_AGGR_MAX_IFACES;

	/* Find the queue for that interface */
	for (i = 0; i < ha->aggr_rx_entry->num_ifs; i++) {
		if (ha->aggr_rx_entry->hyfi_iface_info[i].ifindex
				== (*skb)->dev->ifindex) {
			/* Found */
			idx = i;
			break;
		}
	}

	/* If no queue exists, assign an empty queue to that interface */
	if (idx == HYFI_AGGR_MAX_IFACES) {
		for (i = 0; i < ha->aggr_rx_entry->num_ifs; i++) {
			if (!ha->aggr_rx_entry->hyfi_iface_info[i].ifindex) {
				/* Assign to the interface */
				ha->aggr_rx_entry->hyfi_iface_info[i].ifindex =
						(*skb)->dev->ifindex;
				ha->aggr_rx_entry->hyfi_iface_info[i].pkt_cnt = 0;
				ha->aggr_rx_entry->hyfi_iface_info[i].seq_valid = 0;
				idx = i;
				break;
			}
		}
	}

	if (unlikely(idx >= HYFI_AGGR_MAX_IFACES)) {
		/* Defensive check, in case there is no available queue (unlikely)
		 * we should just finish here.
		 */
		return -1;
	} else {
		struct hyfi_skb_aggr_q *skb_aggr_q;
		struct hyfi_aggr_skb_buffer *hyfi_aggr_skb_buffer =
				(struct hyfi_aggr_skb_buffer *) (*skb)->head;

#ifdef HYFI_AGGR_NOISY_DEBUG /* Be careful, noisy and damages high rate flows */
		DEBUG_TRACE("%s: Queue future packet, ha = %p, iface = %s, seq = %d, next_seq = %d\n",
				__func__, ha, (*skb)->dev->name, (seq & 0x3FFF), ha->aggr_rx_entry->aggr_next_seq);
#endif
		/* Push the skb in the queue */
		skb_aggr_q = &ha->aggr_rx_entry->hyfi_iface_info[idx].skb_aggr_q;
		hyfi_aggr_skb_buffer->pkt_seq = (seq & 0x3FFF);
		hyfi_aggr_skb_buffer->skb = *skb;

		TAILQ_INSERT_TAIL( skb_aggr_q, hyfi_aggr_skb_buffer, skb_aggr_qelem);
		ha->aggr_rx_entry->hyfi_iface_info[idx].pkt_cnt++;

		if (ha->aggr_rx_entry->hyfi_iface_info[idx].pkt_cnt == 1) {
			ha->aggr_rx_entry->hyfi_iface_info[idx].seq = (seq & 0x3FFF);
			ha->aggr_rx_entry->hyfi_iface_info[idx].seq_valid = 1;
		}

		*skb = NULL;
	}

	/* Sanity check: if we reached the maximum packets quota per queue, then
	 * something is really off. flush everything and restart (may be blunt, could
	 * be revisited in the future).
	 */
	if (unlikely(
			ha->aggr_rx_entry->hyfi_iface_info[idx].pkt_cnt
					> HYFI_AGGR_MAX_QUEUE_LEN)) {
		u_int16_t next_seq;

		DEBUG_TRACE("%s: Queue %d is full, flush and recover\n", __func__, idx);

		hyfi_aggr_reset_queues(ha, seq);

		ha->aggr_rx_entry->time_stamp = jiffies;

		/* Handle the gap, dequeue the next available packets */
		next_seq = hyfi_aggr_find_next_seq(ha);

		if (likely(next_seq != (u_int16_t) ~0)) {
			/* Update our next sequence variable to the closest
			 * sequence number we have in the queues.
			 */
			ha->aggr_rx_entry->aggr_next_seq = next_seq;
			DEBUG_TRACE("%s: Next sequence to dequeue: %d\n", __func__, next_seq);
		} else {
			ha->aggr_rx_entry->next_seq_valid = 0;
			DEBUG_TRACE("%s: Next sequence is unavailable, forward current packet\n",
					__func__);
			return -1;
		}
	}

	return 0;
}

struct net_bridge_port *hyfi_aggr_process_pkt(struct net_hatbl_entry *ha,
		struct sk_buff **skb, u_int16_t seq)
{
	spin_lock(&ha->aggr_lock);

	if (likely(ha->aggr_rx_entry->next_seq_valid)) {
		struct net_bridge_port *dst;

		/* Check if the current packet sequence matches our next expected sequence */
		if (likely(ha->aggr_rx_entry->aggr_next_seq == (seq & 0x3FFF))) {
			ha->aggr_rx_entry->aggr_next_seq++;
			ha->aggr_rx_entry->time_stamp = jiffies;
			ha->aggr_rx_entry->aggr_new_flow = 0;
#ifdef HYFI_AGGR_NOISY_DEBUG /* Be careful, noisy and damages high rate flows */
			DEBUG_TRACE("%s: pkt OK, seq = %d, iface = %s\n", __func__, seq & 0x3fff, (*skb)->dev->name);
#endif
			spin_unlock(&ha->aggr_lock);
			return ha->dst;
		} else if (unlikely(
				(ha->aggr_rx_entry->aggr_next_seq > (seq & 0x3FFF))
						&& ((ha->aggr_rx_entry->aggr_next_seq - (seq & 0x3FFF))
								< (1 << 13)))) {

#ifdef HYFI_AGGR_NOISY_DEBUG /* Be careful, noisy and damages high rate flows */
			DEBUG_TRACE("%s: drop old packet, seq = %d, next_seq = %d\n",
					__func__, (seq & 0x3FFF), ha->aggr_rx_entry->aggr_next_seq);
#endif
			/* This is an expired sequence, drop (TBD: option to forward) */
			kfree_skb(*skb);
			*skb = NULL;
			spin_unlock(&ha->aggr_lock);
			return (struct net_bridge_port *) -1;
		}

		/* Received a future sequence, queue it for later reordering */
		if (unlikely(hyfi_aggr_queue_pkt(ha, skb, seq) < 0)) {
			/* We cannot queue this packet (unlikely) */
			spin_unlock(&ha->aggr_lock);
			return ha->dst;
		}

		/* Dequeue any queued skbs with the current sequence, up to the quota */
		dst = hyfi_aggr_dequeue_pkts(ha, skb);

		if (dst != (struct net_bridge_port *) -1) {
			/* Last packet in the queues or max quota */
			spin_unlock(&ha->aggr_lock);
			return dst;
		}

		if (hyfi_aggr_timeout(ha)) {
			u_int16_t next_seq;

			hyfi_aggr_reset_queues(ha, seq);

			ha->aggr_rx_entry->time_stamp = jiffies;

			/* Handle the gap, dequeue the next available packets */
			next_seq = hyfi_aggr_find_next_seq(ha);

			if (likely(next_seq != (u_int16_t) ~0)) {
				/* Update our next sequence variable to the closest
				 * sequence number we have in the queues.
				 */
				ha->aggr_rx_entry->aggr_next_seq = next_seq;
#ifdef HYFI_AGGR_NOISY_DEBUG /* Be careful, noisy and damages high rate flows */
				DEBUG_TRACE("%s: Next sequence to dequeue: %d\n", __func__, next_seq);
#endif
			} else {
				ha->aggr_rx_entry->next_seq_valid = 0;
#ifdef HYFI_AGGR_NOISY_DEBUG /* Be careful, noisy and damages high rate flows */
				DEBUG_TRACE("%s: Next sequence is unavailable, forward next incoming packet\n", __func__);
#endif
			}
		}

		/* In case we have nothing to forward: current skb was queued,
		 * no other matching skbs were found in queues, and no timeout
		 * has occured yet.
		 */
		*skb = NULL;
		spin_unlock(&ha->aggr_lock);
		return (struct net_bridge_port *) -1;
	} else {
		DEBUG_TRACE("%s: new entry, seq = %d\n", __func__, seq & 0x3fff);

		/* Init a new entry */
		ha->aggr_rx_entry->next_seq_valid = 1;
		ha->aggr_rx_entry->aggr_next_seq = seq & 0x3FFF;
		ha->aggr_rx_entry->aggr_next_seq++;
		ha->aggr_rx_entry->num_ifs = (seq >> 14) & 3;
		ha->aggr_rx_entry->time_stamp = jiffies;

		spin_unlock(&ha->aggr_lock);
		return ha->dst;
	}
}

int hyfi_aggr_end(struct net_hatbl_entry *ha)
{
	u_int32_t i;
	void *ptr;

	DEBUG_INFO("hyfi: Cleaning up aggregated flow 0x%02x\n", ha->hash);

    	spin_lock(&ha->aggr_lock);

	if (unlikely(!ha->aggr_rx_entry)) {
		spin_unlock(&ha->aggr_lock);
		return 0;
	}

	for (i = 0; i < HYFI_AGGR_MAX_IFACES; i++) {
		struct hyfi_skb_aggr_q *skb_aggr_q;
		struct hyfi_aggr_skb_buffer *hyfi_aggr_skb_buffer;

		/* Flush any leftover skbs */
		skb_aggr_q = &ha->aggr_rx_entry->hyfi_iface_info[i].skb_aggr_q;
		hyfi_aggr_skb_buffer = TAILQ_FIRST( skb_aggr_q );

		while (hyfi_aggr_skb_buffer) {
			TAILQ_REMOVE( skb_aggr_q, hyfi_aggr_skb_buffer, skb_aggr_qelem);

			if (hyfi_aggr_skb_buffer->skb) {
				kfree_skb(hyfi_aggr_skb_buffer->skb);
			}

			hyfi_aggr_skb_buffer = TAILQ_FIRST( skb_aggr_q );
		}
	}

	hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_AGGR_RX_ENTRY);
	ptr = ha->aggr_rx_entry;
	ha->aggr_rx_entry = NULL;
	spin_unlock(&ha->aggr_lock);

	kfree(ptr);
	return 0;
}

struct net_bridge_port *hyfi_aggr_handle_tx_path(struct net_hatbl_entry *ha,
		struct sk_buff **skb)
{
	struct hyfi_iface_info *iface_info;

	spin_lock(&ha->aggr_lock);

	if (unlikely(ha->aggr_seq_data.aggr_cur_iface == HYFI_AGGR_MAX_IFACE)) {
		if (unlikely(ha->iface_info[0].packet_count)) {
			/* Aggregation tail */
			hyfi_aggr_tag_packet(ha, *skb);
			ha->iface_info[0].packet_count--;
		} else {
			/* No more aggregation for this flow */
			hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY);
		}

		spin_unlock(&ha->aggr_lock);
		return ha->iface_info[0].dst;
	}

	iface_info = &ha->iface_info[ha->aggr_seq_data.aggr_cur_iface];

	hyfi_aggr_tag_packet(ha, *skb);
	iface_info->packet_count--;

	if (!iface_info->packet_count) {
		u_int32_t aggr_end = 0;

		iface_info->packet_count = iface_info->packet_quota;
		ha->aggr_seq_data.aggr_cur_iface++;
		if (ha->aggr_seq_data.aggr_cur_iface >= HYFI_AGGR_MAX_IFACE)
			ha->aggr_seq_data.aggr_cur_iface = 0;

		while (ha->iface_info[ha->aggr_seq_data.aggr_cur_iface].packet_count == 0) {
			ha->aggr_seq_data.aggr_cur_iface++;
			if (ha->aggr_seq_data.aggr_cur_iface >= HYFI_AGGR_MAX_IFACE) {
				ha->aggr_seq_data.aggr_cur_iface = 0;

				if (!aggr_end) {
					aggr_end = 1;
				} else {
					/* No more aggregation for this flow */
					hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY);
					break;
				}
			}
		}
	}

	spin_unlock(&ha->aggr_lock);
	return iface_info->dst;
}

int hyfi_aggr_init_entry(struct net_hatbl_entry *ha, u_int16_t seq)
{
	u_int32_t i;

	spin_lock(&ha->aggr_lock);

	if (hyfi_ha_has_flag(ha, HYFI_HACTIVE_TBL_AGGR_RX_ENTRY)) {
		spin_unlock(&ha->aggr_lock);
		return 0;
	}

	ha->aggr_rx_entry = kzalloc(sizeof(struct hyfi_aggr_rx_entry), GFP_ATOMIC );
	if (!ha->aggr_rx_entry) {
		spin_unlock(&ha->aggr_lock);
		return -1;
	}

	hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_AGGR_RX_ENTRY);

	for (i = 0; i < HYFI_AGGR_MAX_IFACES; i++) {
		TAILQ_INIT(&ha->aggr_rx_entry->hyfi_iface_info[ i ].skb_aggr_q);
	}

	ha->aggr_rx_entry->next_seq_valid = 1;
	ha->aggr_rx_entry->aggr_next_seq = 0; /*seq & 0x3FFF;*/
	ha->aggr_rx_entry->num_ifs = (seq >> 14) & 3;
	ha->aggr_rx_entry->time_stamp = jiffies;
	ha->aggr_rx_entry->aggr_new_flow = 1;

	spin_unlock(&ha->aggr_lock);

	DEBUG_INFO("hyfi: Detected an aggregated flow 0x%02x, splitted to %d interfaces\n",
			ha->hash, ha->aggr_rx_entry->num_ifs);

	return 0;
}

int hyfi_aggr_update_flow(struct hyfi_net_bridge *br, struct __hatbl_entry *hae,
		struct net_hatbl_entry *ha)
{
	u_int32_t i, cur_iface, update = 0;
	u_int32_t old_port;
	u_int8_t old_packet_count;
	u_int8_t old_packet_quota;
	struct net_device *dev = NULL;
	u_int16_t num_ifs = 0;

	spin_lock_bh(&ha->aggr_lock);

	/* First, find out if all given ports are either NULLs
	 * or have 0 quota. It means that we should terminate
	 * the aggregation, else, need to update.
	 */
	for (i = 0; i < HYFI_AGGR_MAX_IFACE; i++) {
		if (!hae->port_list[i].port)
			break;

		if (hae->port_list[i].quota) {
			update = 1;
			break;
		}
	}

	/* First case: stop aggregation */
	if (!hae->aggr_entry || !update) {
		DEBUG_TRACE("hyfi: Terminating aggregated flow 0x%02x\n", ha->hash);

		/* Clear all interfaces */
		for (i = 0; i < HYFI_AGGR_MAX_IFACE; i++) {
			if (!ha->iface_info[i].dst)
				break;

			ha->iface_info[i].packet_count = 0;
			ha->iface_info[i].packet_quota = 0;
		}

		/* Setup the default port to the current port */
		ha->iface_info[0].dst =
				ha->iface_info[ha->aggr_seq_data.aggr_cur_iface].dst;
		ha->aggr_seq_data.aggr_cur_iface = HYFI_AGGR_MAX_IFACE;

		/* Setup tail */
		ha->iface_info[0].packet_count = HYFI_AGGR_TAIL_COUNT;
		spin_unlock_bh(&ha->aggr_lock);

		return 0;
	}

	DEBUG_TRACE("hyfi: Updating aggregated flow 0x%02x\n", ha->hash);

	if (ha->aggr_seq_data.aggr_cur_iface < HYFI_AGGR_MAX_IFACE) {
		cur_iface = ha->aggr_seq_data.aggr_cur_iface;
	} else {
		cur_iface = 0;
	}

	old_packet_count = ha->iface_info[cur_iface].packet_count;
	old_packet_quota = ha->iface_info[cur_iface].packet_quota;
	old_port = ha->iface_info[cur_iface].dst->dev->ifindex;

	/* Clear all interfaces */
	for (i = 0; i < HYFI_AGGR_MAX_IFACE; i++) {
		if (!ha->iface_info[i].dst)
			break;

		ha->iface_info[i].packet_count = 0;
		ha->iface_info[i].packet_quota = 0;
	}

	/* Second case: update existing or new interfaces */
	for (i = 0; i < HYFI_AGGR_MAX_IFACE; i++) {
		u_int32_t j;

		if (!hae->port_list[i].port)
			break;

		if (hae->port_list[i].quota > HYFI_AGGR_MAX_PACKETS)
			hae->port_list[i].quota = HYFI_AGGR_MAX_PACKETS;

		for (j = 0; j < HYFI_AGGR_MAX_IFACE; j++) {
			/* Update quota on existing port */
			if (ha->iface_info[j].dst
					&& hae->port_list[i].port
							== ha->iface_info[j].dst->dev->ifindex) {
				num_ifs++;
				ha->iface_info[j].packet_quota = hae->port_list[i].quota;

				DEBUG_TRACE("hyfi: Updating %s quota to %d packets\n",
						ha->iface_info[j].dst->dev->name,
						ha->iface_info[j].packet_quota);

				if (old_port != hae->port_list[i].port) {
					ha->iface_info[j].packet_count = hae->port_list[i].quota;
				} else {
					if (ha->iface_info[j].packet_quota > old_packet_quota) {
						ha->iface_info[j].packet_count =
								ha->iface_info[j].packet_quota
										- (old_packet_quota - old_packet_count);
					} else {
						ha->iface_info[j].packet_count =
								ha->iface_info[j].packet_quota;
					}
				}
				break;
			}
		}

		/* Check if we need to create a new entry */
		if (j == HYFI_AGGR_MAX_IFACE) {
			struct net_bridge_port *br_port = hyfi_br_port_get(dev);
			for (j = 0; j < HYFI_AGGR_MAX_IFACE; j++) {
				if (!ha->iface_info[j].dst) {
					num_ifs++;
					ha->iface_info[j].dst = br_port;
					ha->iface_info[j].packet_quota = hae->port_list[i].quota;
					ha->iface_info[j].packet_count = hae->port_list[i].quota;
					break;
				}
			}
		}

		ha->aggr_seq_data.num_ifs = num_ifs;
	}

	spin_unlock_bh(&ha->aggr_lock);
	return 0;
}

int hyfi_aggr_new_flow(struct hyfi_net_bridge *br, struct __hatbl_entry *hae,
		struct net_hatbl_entry *ha)
{
	u_int32_t i, j = 0, aggr_port_idx = HYFI_AGGR_MAX_IFACE;
	struct net_device *dev = NULL;

	DEBUG_INFO("hyfi: Creating a new aggregated flow 0x%02x\n", ha->hash);

	spin_lock_bh(&ha->aggr_lock);
	hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY);

	/* Clear information structure */
	memset(ha->iface_info, 0, sizeof(ha->iface_info));

	/* Populate it */
	for (i = 0; i < HYFI_AGGR_MAX_IFACE; i++) {
		struct net_bridge_port *br_port;

		if (!hae->port_list[i].port)
			break;

		dev = dev_get_by_index(dev_net(br->dev), hae->port_list[i].port);
		br_port = hyfi_br_port_get(dev);

		if (likely(dev && br_port && br_port->br->dev == br->dev)) {
			if (hae->port_list[i].quota > HYFI_AGGR_MAX_PACKETS)
				hae->port_list[i].quota = HYFI_AGGR_MAX_PACKETS;

			ha->iface_info[j].dst = br_port;
			ha->iface_info[j].packet_quota = hae->port_list[i].quota;
			ha->iface_info[j].packet_count = hae->port_list[i].quota;

			DEBUG_INFO("hyfi: Adding interface %s, quota %d\n", dev->name, ha->iface_info[ j ].packet_quota);

			if (ha->dst->dev == dev) {
				/* Start aggregation on the current interface */
				aggr_port_idx = j;
				if (ha->iface_info[j].packet_count < HYFI_AGGR_HEAD_COUNT)
					ha->iface_info[j].packet_count = HYFI_AGGR_HEAD_COUNT;
			}

			j++;
		}

		if (likely(dev))
			dev_put(dev);
	}

	if (unlikely(!j)) {
		hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY);
		spin_unlock_bh(&ha->aggr_lock);

		/* Unlikely, no interface was found */
		return -1;
	} else {
		if (aggr_port_idx == HYFI_AGGR_MAX_IFACE) {
			ha->aggr_seq_data.aggr_cur_iface = 0;

			if (ha->iface_info[0].packet_count < HYFI_AGGR_HEAD_COUNT)
				ha->iface_info[0].packet_count = HYFI_AGGR_HEAD_COUNT;
		} else {
			ha->aggr_seq_data.aggr_cur_iface = aggr_port_idx & 3;
		}

		hyfi_ha_set_flag(ha, HYFI_HACTIVE_TBL_AGGR_TX_ENTRY);
		ha->aggr_seq_data.aggr_cur_seq = 0;
		ha->aggr_seq_data.num_ifs = j;
	}

	spin_unlock_bh(&ha->aggr_lock);
	return 0;
}

int hyfi_aggr_flush(struct net_hatbl_entry *ha)
{
	u_int32_t i;

	if (!ha)
		return 0;

	spin_lock_bh(&ha->aggr_lock);

	if (!ha->aggr_rx_entry)
		goto out;

	for (i = 0; i < HYFI_AGGR_MAX_IFACES; i++) {
		struct hyfi_skb_aggr_q *skb_aggr_q;
		struct hyfi_aggr_skb_buffer *hyfi_aggr_skb_buffer;

		/* Found an skb. Pop it from the queue and free it */
		skb_aggr_q = &ha->aggr_rx_entry->hyfi_iface_info[i].skb_aggr_q;
		hyfi_aggr_skb_buffer = TAILQ_FIRST(skb_aggr_q);

		while (hyfi_aggr_skb_buffer) {
			TAILQ_REMOVE(skb_aggr_q, hyfi_aggr_skb_buffer, skb_aggr_qelem);

			if (hyfi_aggr_skb_buffer->skb) {
				kfree_skb(hyfi_aggr_skb_buffer->skb);
			}

			hyfi_aggr_skb_buffer = TAILQ_FIRST(skb_aggr_q);
		}
	}

	kfree(ha->aggr_rx_entry);
	ha->aggr_rx_entry = NULL;
	hyfi_ha_clear_flag(ha, HYFI_HACTIVE_TBL_AGGR_RX_ENTRY);

	out:
	spin_unlock_bh( &ha->aggr_lock);

	return 0;
}
