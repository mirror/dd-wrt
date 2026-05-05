/*
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/rculist.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <net/addrconf.h>
#include <net/esp.h>
#include <net/protocol.h>
#include <net/xfrm.h>
#include <net/udp.h>

#include "eip_ipsec_priv.h"

/*
 * Original ESP ptotocol handlers
 */
static const struct net_protocol *linux_esp_handler;
static const struct inet6_protocol *linux_esp6_handler;

/*
 * eip_ipsec_rps_hash_get()
 *	Compute Hash from the 5-tuple.
 */
static uint32_t eip_ipsec_rps_hash_get(struct sk_buff *skb, uint8_t rps_num_cores, bool is_v6)
{
	uint32_t hash = 0;
	uint8_t proto;

	if (!is_v6) {
		struct iphdr *iph = ip_hdr(skb);

		proto = iph->protocol;
		hash = ntohl(iph->saddr ^ iph->daddr);
	} else {
		struct ipv6hdr *ip6h = ipv6_hdr(skb);
		__be32 *saddr;
		__be32 *daddr;

		proto = ip6h->nexthdr;
		saddr = ip6h->saddr.s6_addr32;
		daddr = ip6h->daddr.s6_addr32;

		hash ^= saddr[0] ^ daddr[0];
		hash ^= saddr[1] ^ daddr[1];
		hash ^= saddr[2] ^ daddr[2];
		hash ^= saddr[3] ^ daddr[3];
	}

	switch (proto) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
		__le16 *sport = (__le16 *)skb_transport_header(skb);
		__le16 *dport = (__le16 *)(skb_transport_header(skb) + sizeof(__le16));

		hash ^= ntohs (*sport ^ *dport);
		break;

	default:
		break;
	}

	hash ^= proto;
	return (hash % rps_num_cores);
}

/*
 * eip_ipsec_proto_dec_err()
 *	Decapsulation completion callback.
 */
void eip_ipsec_proto_dec_err(void *app_data, eip_req_t req, int err)
{
	struct eip_ipsec_sa *sa = eip_ipsec_sa_ref_unless_zero((struct eip_ipsec_sa *)app_data);
	struct sk_buff *skb = eip_req2skb(req);
	struct eip_ipsec_dev_stats *dev_stats;
	struct eip_ipsec_sa_stats *sa_stats;
	struct eip_ipsec_dev *eid;

	if (unlikely(!sa)) {
		pr_debug("%px: Failed to take reference on SA\n", sa);
		consume_skb(skb);
		return;
	}

	eid = sa->eid;
	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	dev_stats->rx_fail++;

	/*
	 * Update SA statistics.
	 */
	sa_stats = this_cpu_ptr(sa->stats_pcpu);
	sa_stats->rx_pkts++;
	sa_stats->rx_bytes += skb->len;
	sa_stats->fail_transform++;

	eip_ipsec_sa_deref(sa);
	consume_skb(skb);
}

/*
 * eip_ipsec_proto_forward()
 *	Forwards decapsulated packet through SFE/Linux.
 */
static void eip_ipsec_proto_forward(struct eip_ipsec_dev *eid, uint32_t spi, struct sk_buff *skb)
{
	struct eip_ipsec_dev_stats *dev_stats;
	int (*fast_recv)(struct sk_buff *skb);
	struct eip_ipsec_sa_stats *sa_stats;
	struct eip_ipsec_sa *sa;

	/*
	 * Try to send packet via SFE
	 */
	rcu_read_lock_bh();

	fast_recv = rcu_dereference(athrs_fast_nat_recv);
	if (likely(fast_recv) && likely(fast_recv(skb))) {
		rcu_read_unlock_bh();
		return;
	}

	/*
	 * Forwarding with SFE failed, possibly connection does not exist yet.
	 * We should not come here very often, update and send packet to Linux stack.
	 * Linux requires sp in SKB when xfrm is enabled.
	 */
	sa = eip_ipsec_sa_match_spi(&eid->dec_sa, htonl(spi));
	if (!sa) {
		pr_debug("%px: Failed to find a valid Decap SA pointer\n", sa);
		goto fail;
	}

	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	sa_stats = this_cpu_ptr(sa->stats_pcpu);
	sa_stats->fast_recv_miss++;

	if (sa->xs) {
		struct xfrm_state *x = sa->xs;
		struct sec_path *sp;

		sp = secpath_set(skb);
		if(!sp) {
			sa_stats->fail_sp_alloc++;
			goto fail;
		}

		/*
		 * TODO: Add API in linux xfrm_state_try_hold() ?
		 * This is needed to solve race condition with final ref_put() by Linux.
		 */
		if (unlikely(!refcount_inc_not_zero(&x->refcnt))) {
			goto fail;
		}

		sp->xvec[sp->len++] = x;
	}

	netif_receive_skb(skb);
	rcu_read_unlock_bh();
	return;

fail:
	rcu_read_unlock_bh();
	dev_stats->rx_fail++;
	consume_skb(skb);
	return;
}

/*
 * eip_ipsec_proto_except()
 *	Handle plain text(post decapsulation) packet.
 */
void eip_ipsec_proto_except(struct eip_ipsec_dev *eid, struct sk_buff *skb)
{
	struct net_device *ndev;
	struct eip_ipsec_sa *sa;
	unsigned int ip_hdr_sz;
	uint16_t proto;
	__be32 spi;

	/* Reset General SKB fields for further processing */
	ndev = eid->ndev;
	skb_scrub_packet(skb, false);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	skb->dev = ndev;
	skb->skb_iif = ndev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;

	/* Detect IPv4 / IPv6 */
	if (ip_hdr(skb)->version == IPVERSION) {
		proto = htons(ETH_P_IP);
		ip_hdr_sz = sizeof(struct iphdr);
	} else {
		proto = htons(ETH_P_IPV6);
		ip_hdr_sz = sizeof(struct ipv6hdr);
	}

	skb->protocol = proto;
	skb_set_transport_header(skb, ip_hdr_sz);

	/*
	 * We will pick whatever head SA available as
	 * we don't know which exact decap SA has been used by EIP for ESP decapsulation.
	 * (If SA has not been rekeyed during exception then this SA would be the correct one).
	 */
	sa = eip_ipsec_sa_ref_get_decap(ndev);
	if (!sa) {
		pr_debug("%px: Failed to get decap SA\n", skb);
		kfree_skb(skb);
		return;
	}

	spi = htonl(sa->spi);
	eip_ipsec_proto_forward(eid, spi, skb);
	eip_ipsec_sa_deref(sa);
}

/*
 * eip_ipsec_proto_rx_steer_cb()
 *	Rx data packet steering callback
 */
void eip_ipsec_proto_rx_steer_cb(struct netfn_pkt_steer *rx_steer, struct sk_buff_head *q_head)
{
	struct eip_ipsec_dev *eid = container_of(rx_steer, struct eip_ipsec_dev, rx_steer);
	struct sk_buff *skb, *nskb;
	uint32_t spi;

	skb_queue_walk_safe(q_head, skb, nskb) {
		/*
		 * Retrieve the spi.
		 */
		spi = EIP_IPSEC_PROTO_SPI_SKB_CB(skb)->spi;
		EIP_IPSEC_PROTO_SPI_SKB_CB(skb)->spi = 0;

		__skb_unlink(skb, q_head);
		eip_ipsec_proto_forward(eid, spi, skb);
	}
}

/*
 * eip_ipsec_proto_dec_done()
 *	Decapsulation completion callback.
 */
void eip_ipsec_proto_dec_done(void *app_data, eip_req_t req)
{
	struct eip_ipsec_sa *sa = eip_ipsec_sa_ref_unless_zero((struct eip_ipsec_sa *)app_data);
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct sk_buff *skb = eip_req2skb(req);
	struct eip_ipsec_dev_stats *dev_stats;
	int ip_hdr_sz = sizeof(struct iphdr);
	struct eip_ipsec_sa_stats *sa_stats;
	uint16_t proto = htons(ETH_P_IP);
	struct eip_ipsec_dev *eid;
	uint32_t hash = 0;
	bool is_v6 = false;
	int cpu;

	if (unlikely(!sa)) {
		pr_debug("%px: Failed to take reference on SA\n", sa);
		consume_skb(skb);
		return;
	}

	eid = sa->eid;
	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	sa_stats = this_cpu_ptr(sa->stats_pcpu);

	/*
	 * Update SA & dev statistics.
	 */
	sa_stats->rx_pkts++;
	sa_stats->rx_bytes += skb->len;
	dev_stats->rx_pkts++;
	dev_stats->rx_bytes += skb->len;

	/*
	 * Reset General SKB fields for further processing.
	 */
	skb_scrub_packet(skb, false);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	skb->dev = sa->ndev;
	skb->skb_iif = sa->ndev->ifindex;
	skb->ip_summed = CHECKSUM_NONE;

	if (unlikely(ip_hdr(skb)->version != IPVERSION)) {
		ip_hdr_sz = sizeof(struct ipv6hdr);
		proto = htons(ETH_P_IPV6);
		is_v6 = true;
	}

	skb->protocol = proto;
	skb_set_transport_header(skb, ip_hdr_sz);

	/*
	 * Do not steer flows if RPS is disabled.
	 */
	if (likely(!drv->rx_steer_en)) {
		dev_stats->bypass_rps++;
		eip_ipsec_proto_forward(eid, sa->tuple.esp_spi, skb);
		goto done;
	}

	/*
	 * Fetch the hash for RPS mapping.
	 */
	if (likely(eid->rps_num_cores > 1)) {
		hash = eip_ipsec_rps_hash_get(skb, eid->rps_num_cores, is_v6);
	}

	/*
	 * Do not steer flows if its already on any of the RPS CPU.
	 */
	cpu = eid->rps_map[hash];
	if (cpu == smp_processor_id()) {
		dev_stats->bypass_rps++;
		eip_ipsec_proto_forward(eid, sa->tuple.esp_spi, skb);
		goto done;
	}

	/*
	 * Map the packet to desired core.
	 */
	EIP_IPSEC_PROTO_SPI_SKB_CB(skb)->spi = sa->tuple.esp_spi;
	if (unlikely(!netfn_pkt_steer_send(&eid->rx_steer, skb, cpu))) {
		dev_stats->fail_rps++;
		goto drop;
	}

done:
	eip_ipsec_sa_deref(sa);
	return;

drop:
	dev_stats->rx_fail++;
	eip_ipsec_sa_deref(sa);
	consume_skb(skb);
	return;
}

/*
 * eip_ipsec_proto_esp_rx()
 *	ESP Protocol handler for IPsec encapsulated packets.
 */
static inline void eip_ipsec_proto_esp_rx(struct eip_ipsec_sa *sa, struct sk_buff *skb)
{
	struct eip_ipsec_dev *eid = netdev_priv(sa->ndev);
	struct eip_ipsec_dev_stats *dev_stats;
	struct eip_ipsec_sa_stats *sa_stats;
	unsigned int len;
	int error;

	sa_stats = this_cpu_ptr(sa->stats_pcpu);
	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	dev_stats->rx_host++;
	len = skb->len;

	/*
	 * skb->data points to ESP or UDP header. So, We need to push ip header.
	 */
	skb_push(skb, skb->data - skb_network_header(skb));

	error = eip_tr_ipsec_dec(sa->tr, skb);
	if (unlikely(error < 0)) {
		/*
		 * TODO: We need to reschedule packet during congestion.
		 */
		sa_stats->fail_enqueue++;
		goto fail;
	}

	/*
	 * Update SA statistics.
	 */
	sa_stats->tx_pkts++;
	sa_stats->tx_bytes += len;
	return;

fail:
	dev_stats->rx_fail++;
	consume_skb(skb);
	return;
}

/*
 * eip_ipsec_proto_esp4_rx()
 *	ESP Protocol handler for IPv4 IPsec encapsulated packets.
 */
static int eip_ipsec_proto_esp4_rx(struct sk_buff *skb)
{
	struct eip_ipsec_sa *sa;

	/*
	 * Lookup SA using <ESP SPI, IP Destination address>.
	 * We can't use skb->dev because ECM is only pushing rule for 3 tuple (Single rule for multiple decap SAs).
	 */
	sa = eip_ipsec_sa_ref_get_decap_v4(&ip_hdr(skb)->daddr, ip_esp_hdr(skb)->spi);
	if (!sa) {
		pr_debug("IPv4 SA not found %pI4n SPI is: %x\n", &(ip_hdr(skb)->daddr), ip_esp_hdr(skb)->spi);
		goto drop;
	}

	eip_ipsec_proto_esp_rx(sa, skb);
	eip_ipsec_sa_deref(sa);
	return 0;

drop:
	consume_skb(skb);
	return 0;
}

/*
 * eip_ipsec_proto_esp6_rx()
 *	ESP Protocol handler for IPv6 IPsec encapsulated packets.
 */
static int eip_ipsec_proto_esp6_rx(struct sk_buff *skb)
{
	struct eip_ipsec_sa *sa;

	/*
	 * Lookup SA using <ESP SPI, IP Destination address>.
	 * We can't use skb->dev because ECM is only pushing rule for 3 tuple (Single rule for multiple decap SAs).
	 */
	sa = eip_ipsec_sa_ref_get_decap_v6(ipv6_hdr(skb)->daddr.s6_addr32, ip_esp_hdr(skb)->spi);
	if (!sa) {
		pr_debug("IPv6 SA not found %pI6 %x\n", ipv6_hdr(skb)->daddr.s6_addr32, ip_esp_hdr(skb)->spi);
		goto drop;
	}

	eip_ipsec_proto_esp_rx(sa, skb);
	eip_ipsec_sa_deref(sa);
	return 0;

drop:
	consume_skb(skb);
	return 0;
}

/*
 * eip_ipsec_proto_udp_rx()
 *	Handle UDP encapsulated IPsec packets.
 *
 * Shell returns the following value:
 * =0 if SKB is consumed.
 * >0 if skb should be passed on to UDP socket.
 * <0 if skb should be resubmitted.
 */
static int eip_ipsec_proto_udp_rx(struct sock *sk, struct sk_buff *skb)
{
	size_t hdr_len = sizeof(struct udphdr) +  sizeof(struct ip_esp_hdr);
	struct ip_esp_hdr *esph;

	/*
	 * NAT-keepalive packet has udphdr & one byte payload (rfc3948).
	 */
	if (skb->len < hdr_len) {
		return 1;
	}

	/*
	 * In case of non-linear SKB we would like to ensure that
	 * all the required headers are present in the first segment
	 */
	if (skb_is_nonlinear(skb) && (skb_headlen(skb) < hdr_len)) {
		if (skb_linearize(skb)) {
			dev_kfree_skb_any(skb);
			return 0;
		}

		/*
		 * skb_linearize may change header. So, reload all required pointer.
		 */
		skb_reset_transport_header(skb);
		skb_set_network_header(skb, -(int)sizeof(struct iphdr));
	}

	/*
	 * Check if packet has non-ESP marker (rfc3948)
	 */
	esph = (struct ip_esp_hdr *)(skb_transport_header(skb) + sizeof(struct udphdr));
	if (ntohl(esph->spi) == EIP_IPSEC_PROTO_NON_ESP_MARKER) {
		return 1;
	}

	/*
	 * ESPinUDP, Make transport header to point ESP.
	 */
	skb_set_transport_header(skb, sizeof(struct udphdr));
	return eip_ipsec_proto_esp4_rx(skb);
}

/*
 * eip_ipsec_proto_vp_rx()
 * 	VP callback after PPE lookup.
 */
bool eip_ipsec_proto_vp_rx(struct ppe_vp_cb_info *info, void *cb_data)
{
	struct sk_buff *skb = info->skb;
	struct eip_ipsec_dev *eid = cb_data;
	struct eip_ipsec_dev_stats *dev_stats;
	struct iphdr *iph;

	skb_reset_network_header(skb);
	dev_stats = this_cpu_ptr(eid->stats_pcpu);
	iph = ip_hdr(skb);

	/*
	 * Update device stats
	 */
	dev_stats->rx_vp++;

	/*
	 * We need to pull ip header to point to ESP or UDP header and then
	 * send packet to the ESP packet handler
	 */
	if (iph->version != IPVERSION) {
		skb_pull(skb, sizeof(struct ipv6hdr));
		skb_reset_transport_header(skb);
		eip_ipsec_proto_esp6_rx(skb);
		return true;
	}

	skb_pull(skb, sizeof(struct iphdr));
	skb_reset_transport_header(skb);

	if (iph->protocol == IPPROTO_UDP) {
		int ret;

		ret = eip_ipsec_proto_udp_rx(NULL, skb);

		/*
		 * Send the NONESP packet to Linux
		 */
		if (unlikely(ret)) {
			skb_push(skb, sizeof(struct iphdr));
			skb->ip_summed = CHECKSUM_NONE;
			netif_receive_skb(skb);
		}

		return true;
	}

	eip_ipsec_proto_esp4_rx(skb);
	return true;
}

/*
 * IPv4 ESP handler
 */
static struct net_protocol esp_protocol = {
	.handler = eip_ipsec_proto_esp4_rx,
	.no_policy = 1,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	.netns_ok  = 1,
#endif
};

/*
 * IPv6 ESP handler
 */
static struct inet6_protocol esp6_protocol = {
	.handler = eip_ipsec_proto_esp6_rx,
	.flags = INET6_PROTO_NOPOLICY,
};

/*
 * eip_ipsec_proto_esp_init()
 *	Register ESP handler with the Linux.
 */
bool eip_ipsec_proto_esp_init(void)
{
	const struct net_protocol *ip_prot = &esp_protocol;

	if (!disable_v4_offload && inet_update_protocol(&esp_protocol, IPPROTO_ESP, &linux_esp_handler) < 0) {
		pr_err("Failed to update ESP protocol handler for IPv4\n");
		return false;
	}

	if (inet6_update_protocol(&esp6_protocol, IPPROTO_ESP, &linux_esp6_handler) < 0) {
		pr_err("Failed to update ESP protocol handler for IPv4\n");
		/*
		 * Revert v4 ESP handler to original handler.
		 */
		xchg(&linux_esp6_handler, NULL);
		if (disable_v4_offload)
			return false;

		inet_update_protocol(linux_esp_handler, IPPROTO_ESP, &ip_prot);
		xchg(&linux_esp_handler, NULL);
		return false;
	}

	return true;
}

/*
 * eip_ipsec_proto_udp_sock_override()
 */
bool eip_ipsec_proto_udp_sock_override(struct eip_ipsec_tuple *sa_tuple)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct udp_sock *up;
	struct sock *sk;

	rcu_read_lock();

	sk = __udp4_lib_lookup(&init_net, htonl(sa_tuple->src_ip[0]),  htons(sa_tuple->sport),
			htonl(sa_tuple->dest_ip[0]), htons(sa_tuple->dport), 0, 0, &udp_table, NULL);
	if (!sk) {
		rcu_read_unlock();
		pr_err("%px: Failed to lookup UDP socket dst(%pI4h) dport(0x%X)\n", drv, sa_tuple->dest_ip, sa_tuple->dport);
		return false;
	}

	up = udp_sk(sk);
	if (up->encap_type != UDP_ENCAP_ESPINUDP) {
		rcu_read_unlock();
		pr_err("%px: Socket type is not UDP_ENCAP_ESPINUDP (%u)\n", up, up->encap_type);
		return false;
	}

	if (READ_ONCE(up->encap_rcv) != eip_ipsec_proto_udp_rx) {
		xchg(&up->encap_rcv, eip_ipsec_proto_udp_rx);
		pr_debug("%px: Overriden socket encap handler\n", up);
	}

	rcu_read_unlock();

	return true;
}

/*
 * eip_ipsec_proto_esp_deinit()
 *	De-register ESP handler with the Linux.
 */
void eip_ipsec_proto_esp_deinit(void)
{
	const struct inet6_protocol *ip6_prot;
	const struct net_protocol *ip_prot;

	/*
	 * Revert v4 ESP handler to original handler.
	 */
	if (!disable_v4_offload) {
		inet_update_protocol(linux_esp_handler, IPPROTO_ESP, &ip_prot);
		xchg(&linux_esp_handler, NULL);
	}

	inet6_update_protocol(linux_esp6_handler, IPPROTO_ESP, &ip6_prot);
	xchg(&linux_esp6_handler, NULL);
}
