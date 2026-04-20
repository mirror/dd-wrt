/* Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/xfrm.h>
#include <net/protocol.h>
#include <net/esp.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#include <linux/netfilter.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>

#include "nss_ipsec_xfrm_tunnel.h"
#include "nss_ipsec_xfrm_sa.h"
#include "nss_ipsec_xfrm_flow.h"
#include "nss_ipsec_xfrm.h"

/*
 * nss_ipsec_xfrm_flow_final()
 *	Frees the flow object.
 */
static void nss_ipsec_xfrm_flow_final(struct kref *kref)
{
	struct nss_ipsec_xfrm_flow *flow = container_of(kref, struct nss_ipsec_xfrm_flow, ref);
	struct nss_ipsec_xfrm_drv *drv = flow->drv;

	nss_ipsec_xfrm_info("%p:Flow released\n", flow);

	/*
	 * Release reference to the parent SA.
	 */
	if (READ_ONCE(flow->sa)) {
		nss_ipsec_xfrm_sa_deref(READ_ONCE(flow->sa));
		WRITE_ONCE(flow->sa, NULL);
	}

	if (flow->pol) {
		xfrm_pol_put(flow->pol);
		flow->pol = NULL;
	}

	atomic64_inc(&drv->stats.flow_freed);
	kfree(flow);
}

/*
 * nss_ipsec_xfrm_flow_hdr2tuple()
 *	Parse header and populate the flow tuple
 */
void nss_ipsec_xfrm_flow_hdr2tuple(struct sk_buff *skb, bool natt, struct nss_ipsecmgr_flow_tuple *tuple)
{
	memset(tuple, 0, sizeof(*tuple));

	if (ip_hdr(skb)->version == IPVERSION) {
		struct iphdr *iph = ip_hdr(skb);

		tuple->src_ip[0] = ntohl(iph->saddr);
		tuple->dest_ip[0] = ntohl(iph->daddr);
		tuple->proto_next_hdr = iph->protocol;
		tuple->ip_version = IPVERSION;
	} else {
		struct ipv6hdr *ip6h = ipv6_hdr(skb);

		tuple->src_ip[0] = ntohl(ip6h->saddr.s6_addr32[0]);
		tuple->src_ip[1] = ntohl(ip6h->saddr.s6_addr32[1]);
		tuple->src_ip[2] = ntohl(ip6h->saddr.s6_addr32[2]);
		tuple->src_ip[3] = ntohl(ip6h->saddr.s6_addr32[3]);

		tuple->dest_ip[0] = ntohl(ip6h->daddr.s6_addr32[0]);
		tuple->dest_ip[1] = ntohl(ip6h->daddr.s6_addr32[1]);
		tuple->dest_ip[2] = ntohl(ip6h->daddr.s6_addr32[2]);
		tuple->dest_ip[3] = ntohl(ip6h->daddr.s6_addr32[3]);
		tuple->proto_next_hdr = ip6h->nexthdr;
		tuple->ip_version = 6;
	}

	switch (tuple->proto_next_hdr) {
	case IPPROTO_UDP:
		tuple->sport = ntohs(udp_hdr(skb)->source);
		tuple->dport = ntohs(udp_hdr(skb)->dest);

		/*
		 * Check if this a NAT-T outer flow
		 */
		if (natt) {
			struct ip_esp_hdr *esph = (struct ip_esp_hdr *)(udp_hdr(skb) + 1);
			tuple->spi_index = ntohl(esph->spi);
		}
		break;

	case IPPROTO_TCP:
		tuple->sport = ntohs(tcp_hdr(skb)->source);
		tuple->dport = ntohs(tcp_hdr(skb)->dest);
		break;

	case IPPROTO_ESP:
		tuple->spi_index = ntohl(ip_esp_hdr(skb)->spi);
		break;

	default:
		break;
	}
}

/*
 * nss_ipsec_xfrm_flow_match()
 *	Returns true if the provided flow tuple matches the flow.
 */
bool nss_ipsec_xfrm_flow_match(struct nss_ipsec_xfrm_flow *flow, struct nss_ipsecmgr_flow_tuple *s)
{
	struct nss_ipsecmgr_flow_tuple *d = &flow->tuple;
	uint32_t status = 0;

	switch (d->ip_version) {
		case IPVERSION:
			status += d->dest_ip[0] ^ s->dest_ip[0];
			status += d->src_ip[0] ^ s->src_ip[0];
			status += d->spi_index ^ s->spi_index;

			status += d->sport ^ s->sport;
			status += d->dport ^ s->dport;

			status += d->ip_version ^ s->ip_version;
			status += d->proto_next_hdr ^ s->proto_next_hdr;
			return !status;

		case 6:
			status += d->dest_ip[0] ^ s->dest_ip[0];
			status += d->dest_ip[1] ^ s->dest_ip[1];
			status += d->dest_ip[2] ^ s->dest_ip[2];
			status += d->dest_ip[3] ^ s->dest_ip[3];
			status += d->src_ip[0] ^ s->src_ip[0];
			status += d->src_ip[1] ^ s->src_ip[1];
			status += d->src_ip[2] ^ s->src_ip[2];
			status += d->src_ip[3] ^ s->src_ip[3];
			status += d->spi_index ^ s->spi_index;

			status += d->sport ^ s->sport;
			status += d->dport ^ s->dport;

			status += d->ip_version ^ s->ip_version;
			status += d->proto_next_hdr ^ s->proto_next_hdr;
			return !status;
	}

	return false;
}

/*
 * nss_ipsec_xfrm_flow_deref()
 *	Put flow ref.
 */
void nss_ipsec_xfrm_flow_deref(struct nss_ipsec_xfrm_flow *flow)
{
	kref_put(&flow->ref, nss_ipsec_xfrm_flow_final);
}

/*
 * nss_ipsec_xfrm_flow_ref()
 *	Hold flow ref.
 */
struct nss_ipsec_xfrm_flow *nss_ipsec_xfrm_flow_ref(struct nss_ipsec_xfrm_flow *flow)
{
	kref_get(&flow->ref);
	return flow;
}

/*
 * nss_ipsec_xfrm_flow_update()
 *	Update Flow if the SA is same then this is duplicate otherwise update the SA.
 *
 * Note: The IPsec manager internally takes care of the migration for its flow object
 */
bool nss_ipsec_xfrm_flow_update(struct nss_ipsec_xfrm_flow *flow, struct nss_ipsec_xfrm_sa *sa)
{
	struct nss_ipsec_xfrm_sa *flow_sa = READ_ONCE(flow->sa);
	enum nss_ipsecmgr_status status;

	/*
	 * The SA is different it means flow is migrating to new SA
	 */
	status = nss_ipsecmgr_flow_add(sa->tun->dev, &flow->tuple, &sa->tuple);
	if ((status != NSS_IPSECMGR_DUPLICATE_FLOW) && (status != NSS_IPSECMGR_OK)) {
		nss_ipsec_xfrm_err("%p:Failed to add flow to ipsecmgr; status %d, sa %p\n", flow, status, sa);
		return false;
	}

	/*
	 * If the SA is same then the flow is already configured
	 */
	if (flow_sa == sa) {
		return true;
	}

	if (cmpxchg(&flow->sa, flow_sa, sa) != flow_sa) {
		nss_ipsec_xfrm_info("%p: Flow migrated to newer SA by other CPU\n", flow);
		return false;
	}

	nss_ipsec_xfrm_sa_ref(sa);
	nss_ipsec_xfrm_sa_deref(flow_sa);
	nss_ipsec_xfrm_info("%p: Flow migrated from SA %p to SA %p\n", flow, flow_sa, sa);
	return true;
}

/*
 * nss_ipsec_xfrm_flow_dealloc()
 * 	Deallocate an existing flow object
 */
void nss_ipsec_xfrm_flow_dealloc(struct nss_ipsec_xfrm_flow *flow)
{
	struct nss_ipsec_xfrm_drv *drv = flow->drv;
	struct nss_ipsec_xfrm_tunnel *tun;
	struct nss_ipsec_xfrm_sa *sa;

	nss_ipsec_xfrm_info("%p", flow);

	atomic64_inc(&drv->stats.flow_dealloced);

	sa = READ_ONCE(flow->sa);
	BUG_ON(!sa);

	tun = sa->tun;
	BUG_ON(!tun);

	/*
	 * Delete from IPSecmgr; release the reference acquired in alloc
	 */
	nss_ipsecmgr_flow_del(tun->dev, &flow->tuple, &sa->tuple);
	nss_ipsec_xfrm_flow_deref(flow);
}

/*
 * nss_ipsec_xfrm_flow_alloc()
 * 	Allocate a new flow object
 */
struct nss_ipsec_xfrm_flow *nss_ipsec_xfrm_flow_alloc(struct nss_ipsec_xfrm_drv *drv,
							struct nss_ipsecmgr_flow_tuple *tuple,
							struct nss_ipsec_xfrm_sa *sa)
{
	struct nss_ipsec_xfrm_flow *flow;
	enum nss_ipsecmgr_status status;

	flow = kzalloc(sizeof(struct nss_ipsec_xfrm_flow), GFP_ATOMIC);
	if (!flow) {
		nss_ipsec_xfrm_err("Failed to create flow; Out of memory\n");
		return NULL;
	}

	flow->drv = drv;
	memcpy(&flow->tuple, tuple, sizeof(struct nss_ipsecmgr_flow_tuple));
	kref_init(&flow->ref);

	atomic64_inc(&drv->stats.flow_alloced);

	if (flow->tuple.ip_version == IPVERSION) {
		nss_ipsec_xfrm_info("New IPv4 flow %p created; sip %pI4h dip %pI4h proto %d sport %d dport %d\n", flow,
				flow->tuple.src_ip, flow->tuple.dest_ip, flow->tuple.proto_next_hdr,
				flow->tuple.sport, flow->tuple.dport);
	} else {
		nss_ipsec_xfrm_info("New IPv6 flow %p created; sip %pI6h dip %pI6h proto %d sport %d dport %d\n", flow,
				flow->tuple.src_ip, flow->tuple.dest_ip, flow->tuple.proto_next_hdr,
				flow->tuple.sport, flow->tuple.dport);
	}

	WRITE_ONCE(flow->sa, nss_ipsec_xfrm_sa_ref(sa));

	status = nss_ipsecmgr_flow_add(sa->tun->dev, &flow->tuple, &sa->tuple);
	if ((status != NSS_IPSECMGR_DUPLICATE_FLOW) && (status != NSS_IPSECMGR_OK)) {
		nss_ipsec_xfrm_err("%p:Failed to add flow to ipsecmgr; status %d, sa %p\n", flow, status, sa);
		goto error;
	}

	nss_ipsec_xfrm_info("%p:Flow %p added to SA %p\n", drv, flow, sa);
	return flow;

error:
	nss_ipsec_xfrm_flow_deref(flow);
	return NULL;
}
