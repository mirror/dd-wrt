/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * nss_ovpn_link.c
 *	Interface between NSS OVPN manager and ECM.
 */
#include <linux/module.h>
#include <linux/ip.h>
#include <net/ip.h>
#include <linux/ipv6.h>
#include <linux/ipv6.h>
#include <linux/netdevice.h>
#include <linux/crypto.h>

#include <nss_api_if.h>
#include <nss_qvpn.h>
#include <nss_ovpnmgr.h>
#include <ecm_interface_ovpn.h>

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ovpn_link_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ovpn_link_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_OVPN_LINK_DEBUG_LEVEL < 1)
#define nss_ovpn_link_info(s, ...)
#else
#define nss_ovpn_link_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_OVPN_LINK_DEBUG_LEVEL < 2)
#define nss_ovpn_link_warn(s, ...)
#else
#define nss_ovpn_link_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * nss_ovpn_link_update_route()
 *	Update OVPN tunnel route.
 */
static void nss_ovpn_link_update_route(struct net_device *tun_dev, uint32_t *from_addr, uint32_t *to_addr, int version)
{
	BUG_ON(!tun_dev);

	nss_ovpnmgr_tun_route_update(tun_dev->ifindex, from_addr, to_addr, version);
}

/*
 * nss_ovpn_link_get_ifnum()
 *	Return NSS ifnum from OVPN manager.
 */
static int nss_ovpn_link_get_ifnum(struct net_device *dev, struct sk_buff *skb, struct net_device **tun_dev)
{
	struct nss_ovpnmgr_route_tuple rt={0};
	struct ipv6hdr *ipv6;
	uint32_t ifnum;

	/*
	 * skb start with L3 header
	 */
	if ((skb->protocol != htons(ETH_P_IP)) && (skb->protocol != htons(ETH_P_IPV6))) {
		nss_ovpn_link_warn("%px: Protocol=%x is not supported\n", dev, skb->protocol);
		return -1;
	}

	if (ip_hdr(skb)->version == IPVERSION) {	/* IPv4 Packet */
		rt.ip_addr[0] = ip_hdr(skb)->saddr;
		rt.ip_version = IPVERSION;

		*tun_dev = nss_ovpnmgr_app_find_tun(dev, &rt, &ifnum);
		if (*tun_dev) {
			return (int)ifnum;
		}

		rt.ip_addr[0] = ip_hdr(skb)->daddr;

		*tun_dev = nss_ovpnmgr_app_find_tun(dev, &rt, &ifnum);
		if (*tun_dev) {
			return (int)ifnum;
		}

		nss_ovpn_link_warn("%px: Failed to find tunnel device.\n", dev);
		return -ENOENT;
	}

	/* IPv6 Packet */
	ipv6 = ipv6_hdr(skb);

	memcpy(&rt.ip_addr[0], &ipv6->saddr, sizeof(ipv6->saddr));
	rt.ip_version = 6;

	*tun_dev = nss_ovpnmgr_app_find_tun(dev, &rt, &ifnum);
	if (*tun_dev) {
		return (int)ifnum;
	}

	memcpy(&rt.ip_addr[0], &ipv6->daddr, sizeof(ipv6->daddr));

	*tun_dev = nss_ovpnmgr_app_find_tun(dev, &rt, &ifnum);
	if (*tun_dev) {
		return (int)ifnum;
	}

	nss_ovpn_link_warn("%px: Failed to find tunnel device.\n", dev);
	return -ENOENT;
}

/*
 * nss_ovpn_ecm
 *	Register Front End interface with ECM to support OVPN offload.
 */
static struct ecm_interface_ovpn nss_ovpn_ecm = {
	.ovpn_update_route = nss_ovpn_link_update_route,
	.ovpn_get_ifnum = nss_ovpn_link_get_ifnum
};

/*
 * nss_ovpn_link_init()
 *	Initialize NSS OVPN Manager
 */
int __init nss_ovpn_link_init(void)
{
	if (!nss_cmn_get_nss_enabled()) {
		nss_ovpn_link_warn("OVPN Manager is not compatible with this Platform\n");
		return -1;
	}

	if (ecm_interface_ovpn_register(&nss_ovpn_ecm)) {
		nss_ovpn_link_warn("ecm fe registration failed.\n");
		return -1;
	}

	nss_ovpn_link_info("NSS OVPN Link loaded: %s\n", NSS_CLIENT_BUILD_ID);

	return 0;
}

/*
 * nss_ovpn_link_exit()
 *	Cleanup NSS OVPN Manager and exit
 */
void __exit nss_ovpn_link_exit(void)
{
	ecm_interface_ovpn_unregister();
	nss_ovpn_link_info("OVPN Link unloaded\n");
}

module_init(nss_ovpn_link_init);
module_exit(nss_ovpn_link_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS OVPN");
