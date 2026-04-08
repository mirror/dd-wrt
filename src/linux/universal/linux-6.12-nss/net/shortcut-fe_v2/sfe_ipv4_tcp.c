/*
 * sfe_ipv4_tcp.c
 *	Shortcut forwarding engine - IPv4 TCP implementation
 *
 * Copyright (c) 2013-2016, 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/skbuff.h>
#include <net/tcp.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv4.h"
#include "sfe_pppoe.h"
#include "sfe_vlan.h"
#include "sfe_trustsec.h"

static uint8_t tso_clear_fixed_id __read_mostly;		/* TSO clear fixed id */

/*
 * sfe_tso_clear_fixed_id_cfg_get()
 *	Get tso clear fixed id value
 */
uint8_t sfe_tso_clear_fixed_id_cfg_get(void)
{
	return tso_clear_fixed_id;
}

/*
 * sfe_tso_clear_fixed_id_cfg_set()
 *	Set tso clear fixed id value
 */
int sfe_tso_clear_fixed_id_cfg_set(uint8_t val)
{
	if (tso_clear_fixed_id && val) {
		pr_err("TSO clear fixed id is already enabled\n");
		return -EINVAL;
	}

	if (!tso_clear_fixed_id && !val) {
		pr_err("TSO clear fixed id is already disabled\n");
		return -EINVAL;
	}

	tso_clear_fixed_id = val;
	return 0;
}

/*
 * sfe_ipv4_process_tcp_option_sack()
 *	Parse TCP SACK option and update ack according
 */
static bool sfe_ipv4_process_tcp_option_sack(const struct tcphdr *th, const u32 data_offs,
					     u32 *ack)
{
	u32 length = sizeof(struct tcphdr);
	u8 *ptr = (u8 *)th + length;

	/*
	 * Ignore processing if TCP packet has only TIMESTAMP option.
	 */
	if (likely(data_offs == length + TCPOLEN_TIMESTAMP + 1 + 1)
	    && likely(ptr[0] == TCPOPT_NOP)
	    && likely(ptr[1] == TCPOPT_NOP)
	    && likely(ptr[2] == TCPOPT_TIMESTAMP)
	    && likely(ptr[3] == TCPOLEN_TIMESTAMP)) {
		return true;
	}

	/*
	 * TCP options. Parse SACK option.
	 */
	while (length < data_offs) {
		u8 size;
		u8 kind;

		ptr = (u8 *)th + length;
		kind = *ptr;

		/*
		 * NOP, for padding
		 * Not in the switch because to fast escape and to not calculate size
		 */
		if (kind == TCPOPT_NOP) {
			length++;
			continue;
		}

		if (kind == TCPOPT_SACK) {
			u32 sack = 0;
			u8 re = 1 + 1;

			size = *(ptr + 1);
			if ((size < (1 + 1 + TCPOLEN_SACK_PERBLOCK))
			    || ((size - (1 + 1)) % (TCPOLEN_SACK_PERBLOCK))
			    || (size > (data_offs - length))) {
				return false;
			}

			re += 4;
			while (re < size) {
				u32 sack_re;
				u8 *sptr = ptr + re;
				sack_re = (sptr[0] << 24) | (sptr[1] << 16) | (sptr[2] << 8) | sptr[3];
				if (sack_re > sack) {
					sack = sack_re;
				}
				re += TCPOLEN_SACK_PERBLOCK;
			}
			if (sack > *ack) {
				*ack = sack;
			}
			length += size;
			continue;
		}
		if (kind == TCPOPT_EOL) {
			return true;
		}
		size = *(ptr + 1);
		if (size < 2) {
			return false;
		}
		length += size;
	}

	return true;
}

/*
 * sfe_ipv4_recv_tcp()
 *	Handle TCP packet receives and forwarding.
 */
int sfe_ipv4_recv_tcp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct iphdr *iph, unsigned int ihl, bool sync_on_find, struct sfe_l2_info *l2_info)
{
	struct tcphdr *tcph;
	__be32 src_ip;
	__be32 dest_ip;
	__be16 src_port;
	__be16 dest_port;
	struct sfe_ipv4_connection_match *cm;
	struct sfe_ipv4_connection_match *counter_cm;
	u8 ttl;
	u32 flags;
	u32 service_class_id;
	struct net_device *xmit_dev;
	bool ret;
	bool hw_csum;
	bool bridge_flow;
	bool fast_xmit;
	netdev_features_t features;
	u8 ingress_flags = 0;
	sfe_fls_conn_stats_update_t update_cb;
	bool vlan_passthrough = false;

	/*
	 * Is our packet too short to contain a valid TCP header?
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct tcphdr) + ihl)))) {
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE);
		DEBUG_TRACE("packet too short for TCP header\n");
		return 0;
	}

	/*
	 * Read the IP address and port information.  Read the IP header data first
	 * because we've almost certainly got that in the cache.  We may not yet have
	 * the TCP header cached though so allow more time for any prefetching.
	 */
	src_ip = iph->saddr;
	dest_ip = iph->daddr;

	tcph = (struct tcphdr *)(skb->data + ihl);
	src_port = tcph->source;
	dest_port = tcph->dest;
	flags = tcp_flag_word(tcph);

	rcu_read_lock();

	/*
	 * Look for a connection match.
	 */
#ifdef CONFIG_NF_FLOW_COOKIE
	cm = si->sfe_flow_cookie_table[skb->flow_cookie & SFE_FLOW_COOKIE_MASK].match;
	if (unlikely(!cm)) {
		cm = sfe_ipv4_find_connection_match_rcu(si, dev, IPPROTO_TCP, src_ip, src_port, dest_ip, dest_port);
	}
#else
	/*
	 * 5-tuple lookup for TCP flow.
	 */
	cm = sfe_ipv4_find_connection_match_rcu(si, dev, IPPROTO_TCP, src_ip, src_port, dest_ip, dest_port);
#endif
	if (unlikely(!cm)) {
		/*
		 * We didn't get a connection but as TCP is connection-oriented that
		 * may be because this is a non-fast connection (not running established).
		 * For diagnostic purposes we differentiate this here.
		 */
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {

			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS);
			DEBUG_TRACE("no connection found - fast flags\n");
			return 0;
		}

		rcu_read_unlock();
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS);
		DEBUG_TRACE("no connection found - slow flags: 0x%x\n",
			    flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK));
		return 0;
	}

	/*
	 * Source interface validate.
	 */
	if (unlikely((cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK) && (cm->match_dev != dev))) {
		if (!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_SRC_INTERFACE_CHECK_NO_FLUSH)) {
			struct sfe_ipv4_connection *c = cm->connection;
			DEBUG_TRACE("flush on source interface check failure\n");
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
		}
		rcu_read_unlock();
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_INVALID_SRC_IFACE);
		DEBUG_TRACE("exception the packet on source interface check failure\n");
		return 0;
	}

	if (unlikely(cm->fls_conn && !(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FLS_DISABLED))){
		update_cb = rcu_dereference(sfe_fls_info.stats_update_cb);
		if (likely(update_cb && !update_cb(cm->fls_conn, skb))) {
			cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_FLS_DISABLED;
		}
	}

	/*
	 * If our packet has been marked as "sync on find" we can't actually
	 * forward it in the fast path, but now that we've found an associated
	 * connection we need sync its status before throw it slow path.
	 */
	if (unlikely(sync_on_find)) {
		sfe_ipv4_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();

		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT);
		DEBUG_TRACE("Sync on find\n");
		return 0;
	}

#ifdef CONFIG_XFRM
	/*
	 * We can't accelerate the flow on this direction, just let it go
	 * through the slow path.
	 */
	if (unlikely(!cm->flow_accel)) {
		rcu_read_unlock();
		this_cpu_inc(si->stats_pcpu->packets_not_forwarded64);
		return 0;
	}
#endif

	/*
	 * Do we expect an ingress VLAN tag for this flow?
	 */
#ifdef SFE_BRIDGE_VLAN_FILTERING_ENABLE
	ingress_flags = cm->vlan_filter_rule.ingress_flags;
#endif
	if (unlikely(!sfe_vlan_validate_ingress_tag(skb, cm->ingress_vlan_hdr_cnt, cm->ingress_vlan_hdr, l2_info, ingress_flags))) {
		if (!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_BRIDGE_VLAN_PASSTHROUGH)) {
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_INGRESS_VLAN_TAG_MISMATCH);
			DEBUG_TRACE("VLAN tag mismatch. skb=%px\n", skb);
			return 0;
		}
		vlan_passthrough = true;
		this_cpu_inc(si->stats_pcpu->bridge_vlan_passthorugh_forwarded64);
	}

	/*
	 * Do we expect a trustsec header for this flow ?
	 */
	if (unlikely(!sfe_trustsec_validate_ingress_sgt(skb, cm->ingress_trustsec_valid, &cm->ingress_trustsec_hdr, l2_info))) {
		rcu_read_unlock();
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_INGRESS_TRUSTSEC_SGT_MISMATCH);
		DEBUG_TRACE("Trustsec SGT mismatch. skb=%px\n", skb);
		return 0;
	}

	bridge_flow = !!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_BRIDGE_FLOW);

	/*
	 * Does our TTL allow forwarding?
	 */
	if (likely(!bridge_flow)) {
		ttl = iph->ttl;
		if (unlikely(ttl < 2)) {
			sfe_ipv4_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
			rcu_read_unlock();

			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL);
			DEBUG_TRACE("TTL too low\n");
			return 0;
		}
	}

	/*
	 * If our packet is larger than the MTU of the transmit interface then
	 * we can't forward it easily.
	 */
	if (unlikely((len > cm->xmit_dev_mtu) && !skb_is_gso(skb))) {
		sfe_ipv4_sync_status(si, cm->connection, SFE_SYNC_REASON_STATS);
		rcu_read_unlock();

		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT);
		DEBUG_TRACE("Larger than MTU\n");
		return 0;
	}

	/*
	 * Look at our TCP flags.  Anything missing an ACK or that has RST, SYN or FIN
	 * set is not a fast path packet.
	 */
	if (unlikely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) != TCP_FLAG_ACK)) {
		struct sfe_ipv4_connection *c = cm->connection;
		spin_lock_bh(&si->lock);
		ret = sfe_ipv4_remove_connection(si, c);
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("TCP flags: %#x are not fast. %u->%u\n",
				htonl(flags), htons(src_port), htons(dest_port));
		if (ret) {
			sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
		}
		rcu_read_unlock();
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS);
		return 0;
	}

	counter_cm = cm->counter_match;

	/*
	 * Are we doing sequence number checking?
	 */
	if (likely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK))) {
		u32 seq;
		u32 ack;
		u32 sack;
		u32 data_offs;
		u32 end;
		u32 left_edge;
		u32 scaled_win;
		u32 max_end;

		/*
		 * Is our sequence fully past the right hand edge of the window?
		 */
		seq = ntohl(tcph->seq);
		if (unlikely((s32)(seq - (cm->protocol_state.tcp.max_end + 1)) > 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("seq: %u exceeds right edge: %u\n",
				    seq, cm->protocol_state.tcp.max_end + 1);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE);
			return 0;
		}

		/*
		 * Check that our TCP data offset isn't too short.
		 */
		data_offs = tcph->doff << 2;
		if (unlikely(data_offs < sizeof(struct tcphdr))) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("TCP data offset: %u, too small\n", data_offs);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS);
			return 0;
		}

		/*
		 * Update ACK according to any SACK option.
		 */
		ack = ntohl(tcph->ack_seq);
		sack = ack;
		if (unlikely(!sfe_ipv4_process_tcp_option_sack(tcph, data_offs, &sack))) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("TCP option SACK size is wrong\n");
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK);
			return 0;
		}

		/*
		 * Check that our TCP data offset isn't past the end of the packet.
		 */
		data_offs += sizeof(struct iphdr);
		if (unlikely(len < data_offs)) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("TCP data offset: %u, past end of packet: %u\n",
				    data_offs, len);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS);
			return 0;
		}

		end = seq + len - data_offs;

		/*
		 * Is our sequence fully before the left hand edge of the window?
		 */
		if (unlikely((s32)(end - (cm->protocol_state.tcp.end
						- counter_cm->protocol_state.tcp.max_win - 1)) < 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("seq: %u before left edge: %u\n",
				    end, cm->protocol_state.tcp.end - counter_cm->protocol_state.tcp.max_win - 1);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE);
			return 0;
		}

		/*
		 * Are we acking data that is to the right of what has been sent?
		 */
		if (unlikely((s32)(sack - (counter_cm->protocol_state.tcp.end + 1)) > 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("ack: %u exceeds right edge: %u\n",
				    sack, counter_cm->protocol_state.tcp.end + 1);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE);
			return 0;
		}

		/*
		 * Is our ack too far before the left hand edge of the window?
		 */
		left_edge = counter_cm->protocol_state.tcp.end
			    - cm->protocol_state.tcp.max_win
			    - SFE_IPV4_TCP_MAX_ACK_WINDOW
			    - 1;
		if (unlikely((s32)(sack - left_edge) < 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("ack: %u before left edge: %u\n", sack, left_edge);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE);
			return 0;
		}

		/*
		 * Have we just seen the largest window size yet for this connection?  If yes
		 * then we need to record the new value.
		 */
		scaled_win = ntohs(tcph->window) << cm->protocol_state.tcp.win_scale;
		scaled_win += (sack - ack);
		if (unlikely(cm->protocol_state.tcp.max_win < scaled_win)) {
			cm->protocol_state.tcp.max_win = scaled_win;
		}

		/*
		 * If our sequence and/or ack numbers have advanced then record the new state.
		 */
		if (likely((s32)(end - cm->protocol_state.tcp.end) >= 0)) {
			cm->protocol_state.tcp.end = end;
		}

		max_end = sack + scaled_win;
		if (likely((s32)(max_end - counter_cm->protocol_state.tcp.max_end) >= 0)) {
			counter_cm->protocol_state.tcp.max_end = max_end;
		}
	}

	/*
	 * Check if skb was cloned. If it was, unclone it. Because
	 * the data area is going to be written in this path and we don't want to
	 * change the cloned skb's data section.
	 */
	if (unlikely(skb_cloned(skb))) {
		DEBUG_TRACE("%px: skb is a cloned skb\n", skb);

		if (unlikely(skb_shared(skb)) || unlikely(skb_unclone(skb, GFP_ATOMIC))) {
			rcu_read_unlock();
			DEBUG_WARN("Failed to unclone the cloned skb\n");
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_UNCLONE_FAILED);
			return 0;
		}

		/*
		 * Update the iph and tcph pointers with the uncloned skb's data area.
		 */
		iph = (struct iphdr *)skb->data;
		tcph = (struct tcphdr *)(skb->data + ihl);
	}

	/*
	 * Check if skb has enough headroom to write L2 headers
	 */
	if (unlikely(skb_headroom(skb) < cm->l2_hdr_size)) {
		rcu_read_unlock();
		DEBUG_WARN("%px: Not enough headroom: %u\n", skb, skb_headroom(skb));
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_NO_HEADROOM);
		return 0;
	}

	/*
	 * For PPPoE packets, match server MAC and session id
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PPPOE_DECAP)) {
		struct ethhdr *eth;
		bool pppoe_match;

		if (unlikely(!sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS))) {
			rcu_read_unlock();
			DEBUG_TRACE("%px: PPPoE header not present in packet for PPPoE rule\n", skb);
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_INCORRECT_PPPOE_PARSING);
			return 0;
		}

		eth = eth_hdr(skb);

		pppoe_match = (cm->pppoe_session_id == sfe_l2_pppoe_session_id_get(l2_info)) &&
				ether_addr_equal((u8*)cm->pppoe_remote_mac, (u8 *)eth->h_source);

		if (unlikely(!pppoe_match)) {
			DEBUG_TRACE("%px: PPPoE session ID %d and %d or MAC %pM and %pM did not match\n",
					skb, cm->pppoe_session_id, sfe_l2_pppoe_session_id_get(l2_info),
					cm->pppoe_remote_mac, eth->h_source);
			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_INVALID_PPPOE_SESSION);
			return 0;
		}

		skb->protocol = htons(l2_info->protocol);
		this_cpu_inc(si->stats_pcpu->pppoe_decap_packets_forwarded64);
	} else if (unlikely(sfe_l2_parse_flag_check(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS))) {

		/*
		 * If packet contains PPPoE header but CME doesn't contain PPPoE flag yet we are exceptioning
		 * the packet to linux
		 */
		if (unlikely(!bridge_flow)) {
			rcu_read_unlock();
			DEBUG_TRACE("%px: CME doesn't contain PPPoE flag but packet has PPPoE header\n", skb);
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_PPPOE_NOT_SET_IN_CME);
			return 0;
		}

		/*
		 * For bridged flows when packet contains PPPoE header, restore the header back and forward
		 * to xmit interface
		 */
		__skb_push(skb, PPPOE_SES_HLEN);
		this_cpu_inc(si->stats_pcpu->pppoe_bridge_packets_forwarded64);
	}

	/*
	 * For bridged flows when packet contains the VLan header, restore the header back and forward
	 * we do this here, to make sure PPPOE header is restored before VLAN header(s) is restored for pppoe over vlan passthrough use cases
	 */
	if (unlikely(vlan_passthrough)) {
		struct ethhdr *eth = eth_hdr(skb);
		__skb_push(skb, l2_info->vlan_hdr_cnt * VLAN_HLEN);
		skb_reset_network_header(skb);
		skb->protocol = eth->h_proto;
	}

	/*
	 * From this point on we're good to modify the packet.
	 */

	/*
	 * In case of TSO, when DF bit is set in TCP frames in gro path,
	 * it sets SKB_GSO_TCP_FIXEDID in gso_type but our HW does not
	 * support fixed ID. Hence, in normal SFE+TSO case it goes through
	 * GSO instead of TSO. In case of SFE+PPPOE+TSO if fixed id is
	 * set it can't do TSO as HW doesn't support it and also it can't
	 * do GSO because GSO currently does not support PPPoE header.
	 * In both the above cases, if TSO has to be used then clear the
	 * fixed id from skb gso type and it can be done using sys param.
	 */
	if (unlikely(tso_clear_fixed_id)) {
		skb_shinfo(skb)->gso_type &= ~(SKB_GSO_TCP_FIXEDID);
	}

	/*
	 * TSO is not supported for maple. Hence SFE_IPV4_CONNECTION_MATCH_FLAG_TSO_ENABLE will be disabled for maple.
	 * For SFE + PPPoE flow, GSO is not supported but TSO is supported. If TSO is enabled and if the number of segments is more than 32, flush the rule so that it goes throught host.
	 * For PPPoE flows, add PPPoE header before L2 header is added.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PPPOE_ENCAP)) {
#if defined(SFE_TSO_MAX_SEG_LIMIT_ENABLE)
		if (((!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_TSO_ENABLE)) && skb_shinfo(skb)->gso_segs) ||
			(skb_shinfo(skb)->gso_segs >= SFE_TSO_SEG_MAX) || (skb_shinfo(skb)->gso_type & SKB_GSO_TCP_FIXEDID)) {
			struct sfe_ipv4_connection *c = cm->connection;
			spin_lock_bh(&si->lock);
			ret = sfe_ipv4_remove_connection(si, c);
			spin_unlock_bh(&si->lock);
			if (ret) {
				sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
			}

			rcu_read_unlock();
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_TSO_SEG_MAX_NOT_SUPPORTED);
			return 0;
		}
#endif
		sfe_pppoe_add_header(skb, cm->pppoe_session_id, PPP_IP);
		this_cpu_inc(si->stats_pcpu->pppoe_encap_packets_forwarded64);
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		iph->tos = (iph->tos & SFE_IPV4_DSCP_MASK) | cm->dscp;
	}

	/*
	 * Set SKB packet type to PACKET_HOST
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PACKET_HOST)) {
		skb->pkt_type = PACKET_HOST;
	}

	/*
	 * Decrement our TTL.
	 */
	if (likely(!bridge_flow)) {
		iph->ttl = ttl - 1;
	}

	/*
	 * Enable HW csum if rx checksum is verified and xmit interface is CSUM offload capable.
	 * Note: If L4 csum at Rx was found to be incorrect, we (router) should use incremental L4 checksum here
	 * so that HW does not re-calculate/replace the L4 csum
	 */
	hw_csum = !!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_CSUM_OFFLOAD) && (skb->ip_summed == CHECKSUM_UNNECESSARY);

	/*
	 * Do we have to perform translations of the source address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
		u16 tcp_csum;
		u32 sum;

		iph->saddr = cm->xlate_src_ip;
		tcph->source = cm->xlate_src_port;

		if (unlikely(!hw_csum)) {
			tcp_csum = tcph->check;
			if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
				sum = tcp_csum + cm->xlate_src_partial_csum_adjustment;
			} else {
				sum = tcp_csum + cm->xlate_src_csum_adjustment;
			}

			sum = (sum & 0xffff) + (sum >> 16);
			tcph->check = (u16)sum;
		}
	}

	/*
	 * Do we have to perform translations of the destination address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
		u16 tcp_csum;
		u32 sum;

		iph->daddr = cm->xlate_dest_ip;
		tcph->dest = cm->xlate_dest_port;

		if (unlikely(!hw_csum)) {
			tcp_csum = tcph->check;
			if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
				sum = tcp_csum + cm->xlate_dest_partial_csum_adjustment;
			} else {
				sum = tcp_csum + cm->xlate_dest_csum_adjustment;
			}

			sum = (sum & 0xffff) + (sum >> 16);
			tcph->check = (u16)sum;
		}
	}

	/*
	 * If HW checksum offload is not possible, full L3 checksum and incremental L4 checksum
	 * are used to update the packet. Setting ip_summed to CHECKSUM_UNNECESSARY ensures checksum is
	 * not recalculated further in packet path.
	 */
	if (likely(hw_csum)) {
		skb->ip_summed = CHECKSUM_PARTIAL;
	} else {
		iph->check = sfe_ipv4_gen_ip_csum(iph);
		skb->ip_summed = CHECKSUM_UNNECESSARY;
	}

	/*
	 * Update traffic stats.
	 */
	atomic_inc(&cm->rx_packet_count);
	atomic_add(len, &cm->rx_byte_count);

	xmit_dev = cm->xmit_dev;
	skb->dev = xmit_dev;

	/*
	 * For trustsec flows, add trustsec header before L2 header is added.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_INSERT_EGRESS_TRUSTSEC_SGT)) {
		sfe_trustsec_add_sgt(skb, &cm->egress_trustsec_hdr);
	}

	/*
	 * Check to see if we need to add VLAN tags
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_INSERT_EGRESS_VLAN_TAG)) {
		sfe_vlan_add_tag(skb, cm->egress_vlan_hdr_cnt, cm->egress_vlan_hdr);
	}

	/*
	 * Check to see if we need to write an Ethernet header.
	 */
	if (likely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
		if (unlikely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
			dev_hard_header(skb, xmit_dev, ntohs(skb->protocol),
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
		} else {
			/*
			 * For the simple case we write this really fast.
			 */
			struct ethhdr *eth = (struct ethhdr *)__skb_push(skb, ETH_HLEN);

			eth->h_proto = skb->protocol;

			ether_addr_copy((u8 *)eth->h_dest, (u8 *)cm->xmit_dest_mac);
			ether_addr_copy((u8 *)eth->h_source, (u8 *)cm->xmit_src_mac);
		}
	}

	/*
	 * Update priority and int_pri of skb.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
		skb->priority = cm->priority;
#if defined(SFE_PPE_QOS_SUPPORTED)
		skb_set_int_pri(skb, cm->int_pri);
#endif
	}

	/*
	 * Mark outgoing packet
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_MARK)) {
		skb->mark = cm->mark;
		/*
		 * Update service class stats if SAWF is valid.
		 */
		if (likely(cm->sawf_valid)) {
			service_class_id = SFE_GET_SAWF_SERVICE_CLASS(cm->mark);
			sfe_ipv4_service_class_stats_inc(si, service_class_id, len);
		}
	}

	/*
	 * For the first packets, check if it could got fast xmit.
	 */
	if (unlikely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED)
				&& (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_DEV_ADMISSION))){
		cm->features = netif_skb_features(skb);
		if (likely(sfe_fast_xmit_check(skb, cm->features))) {
			cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT;
		}
		cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT_FLOW_CHECKED;
	}
	features = cm->features;
	fast_xmit = !!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT);

	/*
	 * In the map-t tunnel, it needs correct transport header.
	 * when l2 acceleration enabled, this header was not ever set.
	 */
	skb_set_transport_header(skb, ihl);

	rcu_read_unlock();

	this_cpu_inc(si->stats_pcpu->packets_forwarded64);

	/*
	 * We do per packet condition check before we could fast xmit the
	 * packet.
	 */
	if (likely(fast_xmit)) {
		if (likely(!skb_is_gso(skb))) {
			if (likely(dev_fast_xmit(skb, xmit_dev, features))) {
				this_cpu_inc(si->stats_pcpu->packets_fast_xmited64);
				return 1;
			}
		} else {
			cm->flags &= ~SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_XMIT;
			DEBUG_TRACE("%px: fast xmit disabled for xmit dev %s", skb, xmit_dev->name);
		}
	}

	/*
	 * We're going to check for GSO flags when we transmit the packet so
	 * start fetching the necessary cache line now.
	 */
	prefetch(skb_shinfo(skb));

	/*
	 * Mark that this packet has been fast forwarded.
	 */
	skb->fast_forwarded = 1;

	/*
	 * check if fast qdisc xmit is enabled and send the packet on its way.
	 */
        if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_FAST_QDISC_XMIT) {
		if (likely(dev_fast_xmit_qdisc(skb, xmit_dev, cm->qdisc_xmit_dev))) {
			this_cpu_inc(si->stats_pcpu->packets_fast_qdisc_xmited64);
			return 1;
		}
	}

	dev_queue_xmit(skb);

	return 1;
}
