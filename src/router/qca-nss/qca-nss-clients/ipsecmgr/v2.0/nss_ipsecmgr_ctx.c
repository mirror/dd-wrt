/*
 * ********************************************************************************
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
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
 **********************************************************************************
 */
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <asm/atomic.h>
#include <linux/debugfs.h>
#include <linux/completion.h>
#include <linux/vmalloc.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/ip.h>
#include <net/ip6_route.h>

#include <nss_api_if.h>
#include <nss_ipsec_cmn.h>
#include <nss_ipsecmgr.h>

#include "nss_ipsecmgr_ref.h"
#include "nss_ipsecmgr_flow.h"
#include "nss_ipsecmgr_sa.h"
#include "nss_ipsecmgr_ctx.h"
#include "nss_ipsecmgr_tunnel.h"
#include "nss_ipsecmgr_priv.h"

/*
 * Context Host statistics print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_ctx_host_stats[] = {
	{"\tIPv4 notify", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv4 notify drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv4 route", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv4 route drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv6 notify", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv6 notify drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv6 route", NSS_IPSECMGR_PRINT_DWORD},
	{"\tIPv6 route drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tInner exp", NSS_IPSECMGR_PRINT_DWORD},
	{"\tInner exp drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tInner callbacks", NSS_IPSECMGR_PRINT_DWORD},
	{"\tInner fail dev", NSS_IPSECMGR_PRINT_DWORD},
	{"\tInner fail SA", NSS_IPSECMGR_PRINT_DWORD},
	{"\tInner fail flow", NSS_IPSECMGR_PRINT_DWORD},
	{"\tOuter exp", NSS_IPSECMGR_PRINT_DWORD},
	{"\tOuter exp drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tOuter callbacks", NSS_IPSECMGR_PRINT_DWORD},
	{"\tOuter fail dev", NSS_IPSECMGR_PRINT_DWORD},
	{"\tOuter fail SA", NSS_IPSECMGR_PRINT_DWORD},
	{"\tOuter fail flow", NSS_IPSECMGR_PRINT_DWORD},
	{"\tRedir exp", NSS_IPSECMGR_PRINT_DWORD},
	{"\tRedir exp drop", NSS_IPSECMGR_PRINT_DWORD},
	{"\tRedir callbacks", NSS_IPSECMGR_PRINT_DWORD},
	{"\tRedir fail dev", NSS_IPSECMGR_PRINT_DWORD},
	{"\tRedir fail SA", NSS_IPSECMGR_PRINT_DWORD},
	{"\tRedir fail flow", NSS_IPSECMGR_PRINT_DWORD},
};

/*
 * Context statistics print info
 */
static const struct nss_ipsecmgr_print ipsecmgr_print_ctx_stats[] = {
	{"\trx_packets", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_bytes", NSS_IPSECMGR_PRINT_DWORD},
	{"\ttx_packets", NSS_IPSECMGR_PRINT_DWORD},
	{"\ttx_bytes", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[0]", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[1]", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[2]", NSS_IPSECMGR_PRINT_DWORD},
	{"\trx_dropped[3]", NSS_IPSECMGR_PRINT_DWORD},
	{"\texceptioned", NSS_IPSECMGR_PRINT_DWORD},
	{"\tlinearized", NSS_IPSECMGR_PRINT_DWORD},
	{"\tredirected", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_sa", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_flow", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_stats", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_exception", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_transform", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_linearized", NSS_IPSECMGR_PRINT_DWORD},
	{"\tfail_mdata_ver", NSS_IPSECMGR_PRINT_DWORD},
};

/*
 * nss_ipsecmgr_ctx_stats_size()
 * 	Calculate size of context stats
 */
static ssize_t nss_ipsecmgr_ctx_stats_size(void)
{
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_ctx_stats;
	ssize_t len = NSS_IPSECMGR_CTX_PRINT_EXTRA;
	int i;

	for (i = 0; i < ARRAY_SIZE(ipsecmgr_print_ctx_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	prn = ipsecmgr_print_ctx_host_stats;

	len += NSS_IPSECMGR_CTX_PRINT_EXTRA;
	for (i = 0; i < ARRAY_SIZE(ipsecmgr_print_ctx_host_stats); i++, prn++)
		len += strlen(prn->str) + prn->var_size;

	return len;
}

/*
 * nss_ipsecmgr_ctx_print_len()
 * 	Return the length of context stats
 */
static ssize_t nss_ipsecmgr_ctx_print_len(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_ctx *ctx = container_of(ref, struct nss_ipsecmgr_ctx, ref);
	return ctx->state.print_len;
}

/*
 * nss_ipsecmgr_ctx_print()
 * 	Print context statistics
 */
static ssize_t nss_ipsecmgr_ctx_print(struct nss_ipsecmgr_ref *ref, char *buf)
{
	struct nss_ipsecmgr_ctx *ctx = container_of(ref, struct nss_ipsecmgr_ctx, ref);
	const struct nss_ipsecmgr_print *prn = ipsecmgr_print_ctx_stats;
	ssize_t max_len = ctx->state.print_len;
	uint64_t *stats_word = (uint64_t *)&ctx->stats;
	ssize_t len;
	int i;

	/*
	 * This expects a strict order as per the stats structure
	 */
	len = snprintf(buf, max_len, "---- Context -----\n");
	len += snprintf(buf + len, max_len - len, "stats: {\n");

	for (i = 0; i < ARRAY_SIZE(ipsecmgr_print_ctx_stats); i++, prn++)
		len += snprintf(buf + len, max_len - len, "%s: %llu\n", prn->str, *stats_word++);

	len += snprintf(buf + len, max_len - len, "}\n");

	stats_word = (uint64_t *)&ctx->hstats;
	len += snprintf(buf + len, max_len - len, "Host stats: {\n");
	prn = ipsecmgr_print_ctx_host_stats;

	for (i = 0; i < ARRAY_SIZE(ipsecmgr_print_ctx_host_stats); i++, prn++)
		len += snprintf(buf + len, max_len - len, "%s: %llu\n", prn->str, *stats_word++);

	len += snprintf(buf + len, max_len - len, "}\n");

	return len;
}

/*
 * nss_ipsecmgr_ctx_read()
 *	Read context info
 */
static ssize_t nss_ipsecmgr_ctx_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsecmgr_ctx *ctx = fp->private_data;
	ssize_t print_len = ctx->state.stats_len;
	ssize_t len = 0, max_len;
	char *buf;

	buf = vzalloc(print_len);
	if (!buf) {
		nss_ipsecmgr_warn("%px: failed to allocate print buffer (req:%zd)", ctx, print_len);
		return 0;
	}

	read_lock_bh(&ipsecmgr_drv->lock);
	max_len = nss_ipsecmgr_ref_print_len(&ctx->ref);
	if (max_len > print_len) {
		read_unlock_bh(&ipsecmgr_drv->lock);
		len += snprintf(buf, print_len, "print buffer size error (need:%zd), retry", max_len);
		ctx->state.stats_len = max_len;
		goto done;
	}

	/*
	 * Walk the context reference tree and retrieve stats
	 */
	len = nss_ipsecmgr_ref_print(&ctx->ref, buf);
	read_unlock_bh(&ipsecmgr_drv->lock);

done:
	len = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return len;
}

/*
 * nss_ipsecmgr_ctx_notify_ipv4()
 * 	Notify Linux by route lookup for destination
 */
static void nss_ipsecmgr_ctx_notify_ipv4(struct sk_buff *skb, struct nss_ipsecmgr_ctx *ctx)
{
	struct iphdr *iph = ip_hdr(skb);
	struct dst_entry *dst;
	struct rtable *rt;

	if (skb->dev != ipsecmgr_drv->dev)
		goto notify;

	/*
	 * If, we could not find the flow then this must be a new
	 * flow that coming in for the first time. We should query
	 * the Linux to see the associated NETDEV
	 */
	rt = ip_route_output(&init_net, iph->saddr, 0, 0, 0);
	if (IS_ERR(rt)) {
		dev_kfree_skb_any(skb);
		ctx->hstats.v4_notify_drop++;
		return;
	}

	dst = (struct dst_entry *)rt;
	skb->dev = dst->dev;
	dst_release(dst);

	skb->pkt_type = PACKET_HOST;
	skb->protocol = htons(ETH_P_IP);

notify:
	ctx->hstats.v4_notify++;
	netif_receive_skb(skb);
}

/*
 * nss_ipsecmgr_ctx_route_ipv4()
 *	Send IPv4 packet for routing
 */
static void nss_ipsecmgr_ctx_route_ipv4(struct sk_buff *skb, struct nss_ipsecmgr_ctx *ctx)
{
	struct iphdr *iph = ip_hdr(skb);
	struct rtable *rt;

	rt = ip_route_output(&init_net, iph->daddr, iph->saddr, 0, 0);
	if (unlikely(IS_ERR(rt))) {
		nss_ipsecmgr_warn("%pK: No route, drop packet.\n", skb);
		dev_kfree_skb_any(skb);
		ctx->hstats.v4_route_drop++;
		return;
	}

	/*
	 * Sets the 'dst' entry for SKB and sends the packet out directly to the physical
	 * device associated with the IPsec tunnel interface.
	 */
	skb_dst_set(skb, &rt->dst);
	skb->ip_summed = CHECKSUM_COMPLETE;

	ctx->hstats.v4_route++;

	ip_local_out(&init_net, NULL, skb);
}

/*
 * nss_ipsecmgr_ctx_notify_ipv6()
 * 	Notify Linux by route lookup for destination
 */
static void nss_ipsecmgr_ctx_notify_ipv6(struct sk_buff *skb, struct nss_ipsecmgr_ctx *ctx)
{
	struct ipv6hdr *ip6h = ipv6_hdr(skb);
	struct dst_entry *dst;
	struct flowi6 fl6;

	if (skb->dev != ipsecmgr_drv->dev)
		goto notify;

	/*
	 * If, we could not find the flow then this must be a new
	 * flow that coming in for the first time. We should query
	 * the Linux to see the associated NETDEV
	 */
	memset(&fl6, 0, sizeof(fl6));
	memcpy(&fl6.daddr, &ip6h->saddr, sizeof(fl6.daddr));

	dst = ip6_route_output(&init_net, NULL, &fl6);
	if (IS_ERR(dst)) {
		dev_kfree_skb_any(skb);
		ctx->hstats.v6_notify_drop++;
		return;
	}

	skb->dev = dst->dev;
	dst_release(dst);

	skb->pkt_type = PACKET_HOST;
	skb->protocol = htons(ETH_P_IPV6);

notify:
	ctx->hstats.v6_notify++;
	netif_receive_skb(skb);
}

/*
 * nss_ipsecmgr_ctx_route_ipv6()
 *	Send IPv6 packet for routing
 */
static void nss_ipsecmgr_ctx_route_ipv6(struct sk_buff *skb, struct nss_ipsecmgr_ctx *ctx)
{
	struct ipv6hdr *ip6h = ipv6_hdr(skb);
	struct dst_entry *dst;
	struct flowi6 fl6;

	memset(&fl6, 0, sizeof(fl6));
	memcpy(&fl6.daddr, &ip6h->daddr, sizeof(fl6.daddr));
	memcpy(&fl6.saddr, &ip6h->saddr, sizeof(fl6.saddr));

	dst = ip6_route_output(&init_net, NULL, &fl6);
	if (unlikely(IS_ERR(dst))) {
		nss_ipsecmgr_warn("%pK: No route, drop packet.\n", skb);
		dev_kfree_skb_any(skb);

		ctx->hstats.v6_notify_drop++;
		return;
	}

	/*
	 * Sets the 'dst' entry for SKB and sends the packet out directly to the physical
	 * device associated with the IPsec tunnel interface.
	 */
	skb_dst_set(skb, dst);
	skb->ip_summed = CHECKSUM_COMPLETE;

	ctx->hstats.v6_route++;

	ip6_local_out(&init_net, NULL, skb);
}

/*
 * nss_ipsecmgr_ctx_del_ref()
 * 	Delete context from context list
 */
static void nss_ipsecmgr_ctx_del_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_ctx *ctx = container_of(ref, struct nss_ipsecmgr_ctx, ref);
	list_del_init(&ctx->list);
}

/*
 * nss_ipsecmgr_ctx_free_ref()
 * 	Free context
 */
static void nss_ipsecmgr_ctx_free_ref(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_ctx *ctx = container_of(ref, struct nss_ipsecmgr_ctx, ref);
	enum nss_dynamic_interface_type di_type = ctx->state.di_type;
	bool status;

	status = nss_ipsec_cmn_unregister_if(ctx->ifnum);
	if (!status) {
		nss_ipsecmgr_warn("%px: Failed to unregister, di_type(%u), I/F(%u)", ctx, di_type, ctx->ifnum);
		return;
	}

	nss_ipsecmgr_ctx_free(ctx);
}

/*
 * file operation structure instance
 */
const struct file_operations ipsecmgr_ctx_file_ops = {
	.open = simple_open,
	.llseek = default_llseek,
	.read = nss_ipsecmgr_ctx_read,
};

/*
 * nss_ipsecmgr_ctx_rx_redir()
 *	NSS IPsec manager device receive function
 */
void nss_ipsecmgr_ctx_rx_redir(struct net_device *dev, struct sk_buff *skb,
				__attribute__((unused))struct napi_struct *napi)
{
	void (*forward_fn)(struct sk_buff *skb, struct nss_ipsecmgr_ctx *ctx) = NULL;
	struct nss_ipsec_cmn_sa_tuple sa_tuple = {0};
	struct nss_ipsecmgr_tunnel *tun;
	struct nss_ipsecmgr_sa *sa;
	struct nss_ipsecmgr_ctx *ctx;
	int tunnel_id;

	ctx = nss_ipsecmgr_ctx_find(netdev_priv(dev), NSS_IPSEC_CMN_CTX_TYPE_REDIR);
	if (!ctx) {
		dev_kfree_skb_any(skb);
		nss_ipsecmgr_warn("%px: ctx is NULL", dev);
		return;
	}

	ctx->hstats.redir_exp++;

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	/*
	 * Set the default values
	 */
	skb->dev = dev;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = skb->dev->ifindex;

	switch (ip_hdr(skb)->version) {
	case IPVERSION:
	{
		struct iphdr *iph = ip_hdr(skb);
		struct udphdr *udph;

		skb->protocol = ETH_P_IP;

		/*
		 * This will happen for exception after de-capsulation.
		 */
		if ((iph->protocol != IPPROTO_ESP) && (iph->protocol != IPPROTO_UDP)) {
			nss_ipsecmgr_ctx_notify_ipv4(skb, ctx);
			return;
		}

		/*
		 * Note: For outer flows check if the SA entry is present.
		 * This will happen for exception after encapsulation
		 */
		if (iph->protocol == IPPROTO_ESP) {
			nss_ipsecmgr_sa_ipv4_outer2tuple(iph, &sa_tuple);

			read_lock(&ipsecmgr_drv->lock);
			sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
			if (!sa) {
				read_unlock(&ipsecmgr_drv->lock);
				dev_kfree_skb_any(skb);

				ctx->hstats.redir_fail_sa++;
				return;
			}

			tunnel_id = sa->tunnel_id;
			read_unlock(&ipsecmgr_drv->lock);
			forward_fn = nss_ipsecmgr_ctx_route_ipv4;
			break;
		}

		/*
		 * UDP NAT-T is for outer processing otherwise inner processing
		 */
		udph = (struct udphdr *)((uint8_t *)iph + sizeof(*iph));
		if (udph->dest != ntohs(NSS_IPSECMGR_NATT_PORT_DATA)) {
			nss_ipsecmgr_ctx_notify_ipv4(skb, ctx);
			return;
		}

		nss_ipsecmgr_sa_ipv4_outer2tuple(iph, &sa_tuple);

		read_lock(&ipsecmgr_drv->lock);
		sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
		if (!sa) {
			read_unlock(&ipsecmgr_drv->lock);
			dev_kfree_skb_any(skb);

			write_lock_bh(&ipsecmgr_drv->lock);
			ctx->hstats.redir_fail_sa++;
			write_unlock_bh(&ipsecmgr_drv->lock);

			return;
		}

		tunnel_id = sa->tunnel_id;
		read_unlock(&ipsecmgr_drv->lock);
		forward_fn = nss_ipsecmgr_ctx_route_ipv4;
		break;
	}

	case 6:
	{
		struct ipv6hdr *ip6h = ipv6_hdr(skb);
		skb->protocol = ETH_P_IPV6;

		if (ip6h->nexthdr == IPPROTO_ESP) {
			nss_ipsecmgr_sa_ipv6_outer2tuple(ip6h, &sa_tuple);

			read_lock(&ipsecmgr_drv->lock);
			sa = nss_ipsecmgr_sa_find(ipsecmgr_drv->sa_db, &sa_tuple);
			if (!sa) {
				read_unlock(&ipsecmgr_drv->lock);
				dev_kfree_skb_any(skb);

				ctx->hstats.redir_fail_sa++;

				return;
			}

			tunnel_id = sa->tunnel_id;
			read_unlock(&ipsecmgr_drv->lock);
			forward_fn = nss_ipsecmgr_ctx_route_ipv6;
			break;
		}

		nss_ipsecmgr_ctx_notify_ipv6(skb, ctx);
		return;
	}

	default:
		nss_ipsecmgr_warn("%px: non IP packet received", dev);
		ctx->hstats.redir_exp_drop++;

		dev_kfree_skb_any(skb);
		return;
	}

	/*
	 * Use the tunnel-ID to find the associated device
	 */
	dev = dev_get_by_index(&init_net, tunnel_id);
	if (!dev) {
		dev_kfree_skb_any(skb);
		ctx->hstats.redir_fail_dev++;
		return;
	}

	/*
	 * Reset SKB fields with the actual values
	 */
	tun = netdev_priv(dev);
	skb->dev = tun->cb.skb_dev;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = skb->dev->ifindex;
	dev_put(dev);

	/*
	 * If, data callback is available then send the packet to the
	 * callback function
	 */
	if (tun->cb.data_cb) {
		tun->cb.data_cb(tun->cb.app_data, skb);
		ctx->hstats.redir_cb++;
		return;
	}

	/*
	 * Indicate up the stack
	 */
	forward_fn(skb, ctx);
	return;
}

/*
 * nss_ipsecmgr_ctx_rx_outer()
 *	Process outer exception from NSS
 */
void nss_ipsecmgr_ctx_rx_outer(struct net_device *dev, struct sk_buff *skb,
				__attribute__((unused)) struct napi_struct *napi)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsecmgr_ctx *ctx;
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	skb->dev = tun->cb.skb_dev;
	skb->skb_iif = skb->dev->ifindex;

	ctx = nss_ipsecmgr_ctx_find(netdev_priv(dev), NSS_IPSEC_CMN_CTX_TYPE_OUTER);
	if (!ctx) {
		nss_ipsecmgr_warn("%px: Could not find ctx", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	ctx->hstats.outer_exp++;

	/*
	 * Reaching this point means the outer ECM rule is non-existent
	 * whereas the IPsec rules are still present in FW. So, this is an ESP
	 * encapsulated packet that has been exception to host from NSS.
	 * We can send it to Linux IP stack for further routing.
	 */
	switch (ip_hdr(skb)->version) {
	case IPVERSION: {
		struct iphdr *iph = ip_hdr(skb);
		skb->protocol = cpu_to_be16(ETH_P_IP);

		if ((iph->protocol != IPPROTO_UDP) && (iph->protocol != IPPROTO_ESP)) {
			nss_ipsecmgr_warn("%px: Unsupported IPv4 protocol(%u)", dev, iph->protocol);
			dev_kfree_skb_any(skb);

			ctx->hstats.outer_exp_drop++;
			return;
		}

		skb_set_transport_header(skb, sizeof(*iph));
		nss_ipsecmgr_ctx_route_ipv4(skb, ctx);
		return;
	}

	case 6: {
		struct ipv6hdr *ip6h = ipv6_hdr(skb);
		skb->protocol = cpu_to_be16(ETH_P_IPV6);

		if (ip6h->nexthdr != IPPROTO_ESP) {
			nss_ipsecmgr_warn("%px: unsupported ipv6 next_hdr(%u)", dev, ip6h->nexthdr);
			dev_kfree_skb_any(skb);

			ctx->hstats.outer_exp_drop++;
			return;
		}

		skb_set_transport_header(skb, sizeof(*ip6h));
		nss_ipsecmgr_ctx_route_ipv6(skb, ctx);
		return;
	}

	default:
		nss_ipsecmgr_warn("%px: non ip packet received after decapsulation", dev);
		ctx->hstats.outer_exp_drop++;

		dev_kfree_skb_any(skb);
		return;
	}
}

/*
 * nss_ipsecmgr_ctx_rx_inner()
 * 	Process inner exception from NSS
 */
void nss_ipsecmgr_ctx_rx_inner(struct net_device *dev, struct sk_buff *skb,
				__attribute__((unused)) struct napi_struct *napi)
{
	struct nss_ipsecmgr_tunnel *tun = netdev_priv(dev);
	struct nss_ipsecmgr_ctx *ctx;

	ctx = nss_ipsecmgr_ctx_find(netdev_priv(dev), NSS_IPSEC_CMN_CTX_TYPE_INNER);
	if (!ctx) {
		nss_ipsecmgr_warn("%px: Could not find ctx", dev);
		dev_kfree_skb_any(skb);
		return;
	}

	ctx->hstats.inner_exp++;

	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);

	switch (ip_hdr(skb)->version) {
	case IPVERSION:
		skb->protocol = cpu_to_be16(ETH_P_IP);
		skb_set_transport_header(skb, sizeof(struct iphdr));
		break;

	case 6:
		skb->protocol = cpu_to_be16(ETH_P_IPV6);
		skb_set_transport_header(skb, sizeof(struct ipv6hdr));
		break;

	default:
		nss_ipsecmgr_warn("%px: Invalid IP header received for rx_inner", tun);
		dev_kfree_skb_any(skb);

		ctx->hstats.inner_exp_drop++;

		return;
	}

	skb->dev = tun->cb.skb_dev;
	skb->pkt_type = PACKET_HOST;
	skb->skb_iif = skb->dev->ifindex;

	/*
	 * If, data callback is available then send the packet to the
	 * callback funtion
	 */
	if (tun->cb.data_cb) {
		tun->cb.data_cb(tun->cb.app_data, skb);
		ctx->hstats.inner_cb++;
		return;
	}

	netif_receive_skb(skb);
}

/*
 * nss_ipsecmgr_ctx_rx_stats()
 *	Asynchronous event for reception of stat
 */
void nss_ipsecmgr_ctx_rx_stats(void *app_data, struct nss_cmn_msg *ncm)
{
	struct nss_ipsec_cmn_msg *nicm = (struct nss_ipsec_cmn_msg *)ncm;
	struct nss_ipsecmgr_tunnel *tun;
	struct nss_ipsecmgr_ctx *ctx;

	ctx = app_data;
	tun = ctx->tun;

	switch (ncm->type) {

	case NSS_IPSEC_CMN_MSG_TYPE_SA_SYNC: {
		struct nss_ipsec_cmn_sa_sync *sync = &nicm->msg.sa_sync;
		struct list_head *sa_db = ipsecmgr_drv->sa_db;
		struct nss_ipsecmgr_event event = {0};
		nss_ipsecmgr_event_callback_t ev_cb;
		struct nss_ipsecmgr_sa *sa;
		void *app_data;

		write_lock(&ipsecmgr_drv->lock);

		sa = nss_ipsecmgr_sa_find(sa_db, &sync->sa_tuple);
		if (!sa) {
			write_unlock(&ipsecmgr_drv->lock);
			break;
		}

		nss_ipsecmgr_sa_sync_state(sa, sync);

		ev_cb = sa->cb.event_cb;
		if (ev_cb) {
			nss_ipsecmgr_sa_sync2stats(sa, sync, &event.data.stats);
			app_data = sa->cb.app_data;
		}

		write_unlock(&ipsecmgr_drv->lock);

		if (ev_cb)
			ev_cb(app_data, &event);
		break;
	}

	case NSS_IPSEC_CMN_MSG_TYPE_CTX_SYNC: {
		struct nss_ipsec_cmn_ctx_sync *ctx_sync = &nicm->msg.ctx_sync;
		uint32_t *msg_stats = (uint32_t *)&ctx_sync->stats;
		uint64_t *ctx_stats = (uint64_t *)&ctx->stats;
		int num;

		write_lock(&ipsecmgr_drv->lock);

		for (num = 0; num < sizeof(ctx->stats)/sizeof(*ctx_stats); num++) {
			ctx_stats[num] += msg_stats[num];
		}

		write_unlock(&ipsecmgr_drv->lock);
		break;
	}

	default:
		nss_ipsecmgr_info("%px: unhandled ipsec message type(%u)", nicm, nicm->cm.type);
		break;
	}

}

/*
 * nss_ipsecmgr_ctx_stats_read()
 * 	Read context stats into Linux device stats
 */
void nss_ipsecmgr_ctx_stats_read(struct nss_ipsecmgr_ctx *ctx, struct rtnl_link_stats64 *dev_stats)
{
	struct nss_ipsecmgr_ctx_stats_priv *ctx_stats = &ctx->stats;
	uint64_t *packets, *bytes, *dropped;
	int i;

	switch (ctx->state.type) {
	case NSS_IPSEC_CMN_CTX_TYPE_INNER:
	case NSS_IPSEC_CMN_CTX_TYPE_MDATA_INNER:
		packets = &dev_stats->tx_packets;
		bytes = &dev_stats->tx_bytes;
		dropped = &dev_stats->tx_dropped;
		break;

	case NSS_IPSEC_CMN_CTX_TYPE_OUTER:
	case NSS_IPSEC_CMN_CTX_TYPE_MDATA_OUTER:
		packets = &dev_stats->rx_packets;
		bytes = &dev_stats->rx_bytes;
		dropped = &dev_stats->rx_dropped;
		break;
	default:
		return;
	}

	*packets += ctx_stats->rx_packets;
	*bytes += ctx_stats->rx_bytes;
	*dropped += (ctx_stats->rx_packets - ctx_stats->tx_packets);

	for (i = 0; i < ARRAY_SIZE(ctx_stats->rx_dropped); i++) {
		*dropped += ctx_stats->rx_dropped[i];
	}
}

/*
 * nss_ipsecmgr_ctx_find()
 * 	Find the context for the given type
 */
struct nss_ipsecmgr_ctx *nss_ipsecmgr_ctx_find(struct nss_ipsecmgr_tunnel *tun, enum nss_ipsec_cmn_ctx_type type)
{
	struct list_head *head = &tun->ctx_db;
	struct nss_ipsecmgr_ctx *ctx;

	/*
	 * Linux does not provide any specific API(s) to test for RW locks. The caller
	 * being internal is assumed to hold write lock before initiating this.
	 */
	list_for_each_entry(ctx, head, list) {
		if (ctx->state.type == type)
			return ctx;
	}

	return NULL;
}

/*
 * nss_ipsecmgr_ctx_find_by_sa()
 *	Find the context for the given type
 */
struct nss_ipsecmgr_ctx *nss_ipsecmgr_ctx_find_by_sa(struct nss_ipsecmgr_tunnel *tun, enum nss_ipsecmgr_sa_type sa_type)
{
	struct list_head *head = &tun->ctx_db;
	enum nss_ipsec_cmn_ctx_type ctx_type;
	struct nss_ipsecmgr_ctx *ctx;

	switch (sa_type) {
	case NSS_IPSECMGR_SA_TYPE_ENCAP:
		ctx_type = NSS_IPSEC_CMN_CTX_TYPE_INNER;
		break;

	case NSS_IPSECMGR_SA_TYPE_DECAP:
		ctx_type = NSS_IPSEC_CMN_CTX_TYPE_OUTER;
		break;

	default:
		nss_ipsecmgr_warn("%px: Unsupported SA type(%u)", tun, sa_type);
		return NULL;
	}

	/*
	 * Linux does not provide any specific API(s) to test for RW locks. The caller
	 * being internal is assumed to hold write lock before initiating this.
	 */
	list_for_each_entry(ctx, head, list) {
		if (ctx->state.type == ctx_type) {
			return ctx;
		}
	}

	return NULL;
}

/*
 * nss_ipsecmgr_ctx_config()
 * 	Configure context
 */
bool nss_ipsecmgr_ctx_config(struct nss_ipsecmgr_ctx *ctx)
{
	enum nss_ipsec_cmn_msg_type msg_type = NSS_IPSEC_CMN_MSG_TYPE_CTX_CONFIG;
	struct nss_ipsecmgr_tunnel *tun = ctx->tun;
	struct nss_ipsec_cmn_ctx *ctx_msg;
	struct nss_ipsec_cmn_msg nicm;
	nss_tx_status_t status;

	memset(&nicm, 0, sizeof(struct nss_ipsec_cmn_msg));

	ctx_msg = &nicm.msg.ctx;
	ctx_msg->type = ctx->state.type;
	ctx_msg->except_ifnum = ctx->state.except_ifnum;

	status = nss_ipsec_cmn_tx_msg_sync(ctx->nss_ctx, ctx->ifnum, msg_type, sizeof(*ctx_msg), &nicm);
	if (status != NSS_TX_SUCCESS) {
		nss_ipsecmgr_warn("%px: Failed to configure the context (ctx_type:%u),(tx_status:%d),(error:%x)",
				ctx, ctx->state.type, status, nicm.cm.error);
		return false;
	}

	write_lock_bh(&ipsecmgr_drv->lock);
	nss_ipsecmgr_ref_add(&ctx->ref, &tun->ref);
	write_unlock_bh(&ipsecmgr_drv->lock);

	return true;
}

/*
 * nss_ipsecmgr_ctx_free()
 * 	Free context
 */
void nss_ipsecmgr_ctx_free(struct nss_ipsecmgr_ctx *ctx)
{
	nss_dynamic_interface_dealloc_node(ctx->ifnum, ctx->state.di_type);
	kfree(ctx);
}

/*
 * nss_ipsecmgr_ctx_alloc()
 * 	Allocate context for type and dynamic interface(s)
 */
struct nss_ipsecmgr_ctx *nss_ipsecmgr_ctx_alloc(struct nss_ipsecmgr_tunnel *tun,
						enum nss_ipsec_cmn_ctx_type ctx_type,
						enum nss_dynamic_interface_type di_type,
						nss_ipsec_cmn_data_callback_t rx_data,
						nss_ipsec_cmn_msg_callback_t rx_stats,
						uint32_t features)
{
	struct nss_ipsecmgr_ctx *ctx;

	ctx = kzalloc(sizeof(*ctx), in_atomic() ? GFP_ATOMIC : GFP_KERNEL);
	if (!ctx) {
		nss_ipsecmgr_warn("%px: failed to allocate context memory", tun);
		return NULL;
	}

	nss_ipsecmgr_trace("%px: Allocating dynamic interface type(%d)", ctx, di_type);

	ctx->tun = tun;
	ctx->state.type = ctx_type;
	ctx->state.di_type = di_type;

	ctx->ifnum = nss_dynamic_interface_alloc_node(di_type);
	if (ctx->ifnum < 0) {
		nss_ipsecmgr_warn("%px: failed to allocate dynamic interface(%d)", tun, di_type);
		kfree(ctx);
		return NULL;
	}

	ctx->state.stats_len = ctx->state.print_len = nss_ipsecmgr_ctx_stats_size();
	nss_ipsecmgr_ref_init(&ctx->ref, nss_ipsecmgr_ctx_del_ref, nss_ipsecmgr_ctx_free_ref);
	nss_ipsecmgr_ref_init_print(&ctx->ref, nss_ipsecmgr_ctx_print_len, nss_ipsecmgr_ctx_print);

	INIT_LIST_HEAD(&ctx->list);

	ctx->nss_ctx = nss_ipsec_cmn_register_if(ctx->ifnum, tun->dev, rx_data, rx_stats, features, di_type, ctx);
	if (!ctx->nss_ctx) {
		nss_ipsecmgr_warn("%px: failed to register dynamic interface(%d, %d)", ctx, di_type, ctx->ifnum);
		nss_dynamic_interface_dealloc_node(ctx->ifnum, di_type);
		kfree(ctx);
		return NULL;
	}

	return ctx;
}
