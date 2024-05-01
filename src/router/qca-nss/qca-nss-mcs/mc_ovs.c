/* Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
#include <linux/kernel.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <ovsmgr.h>
#include "mc_private.h"
#include "mc_snooping.h"

/*
 * mc_ovs_dp_process
 *	do a basic check then hand it over to real snooping handler:mc_rcv
 */
static unsigned int mc_ovs_dp_process(struct sk_buff *skb, struct net_device *out)
{
	struct mc_struct *mc;
	struct net_device *ovs_br;
	struct net_device *in = skb->dev;
	struct ethhdr *skb_eth_hdr = eth_hdr(skb);

	if (!in)
		return 0;

	ovs_br = ovsmgr_dev_get_master(in);
	if (!ovs_br || in == ovs_br)
		return 0;

	mc = MC_DEV(ovs_br);
	if (!mc)
		return 0;

	mc_rcv(mc, skb, skb_eth_hdr->h_source, in);
	return 0;
}

/*
 * mc_ipv4_ovs_dp_process
 *	entry for ovs filtered ipv4 packets
 */
static unsigned int mc_ipv4_ovs_dp_process(struct sk_buff *skb, struct net_device *out)
{
	if (skb->protocol != ntohs(ETH_P_IP))
		return 0;
	/*
	 * MC handling must be rcu protected
	 */
	rcu_read_lock();
	mc_ovs_dp_process(skb, out);
	rcu_read_unlock();
	return 0;
}

#ifdef MC_SUPPORT_MLD
/*
 * mc_ipv6_ovs_dp_process
 *	entry for ovs filtered ipv6 packets
 */
static unsigned int mc_ipv6_ovs_dp_process(struct sk_buff *skb, struct net_device *out)
{
	if (skb->protocol != ntohs(ETH_P_IPV6))
		return 0;
	/*
	 * MC handling must be rcu protected
	 */
	rcu_read_lock();
	mc_ovs_dp_process(skb, out);
	rcu_read_unlock();
	return 0;
}

struct ovsmgr_dp_hook_ops mc_ipv6_dh_hooks = {
	.protocol = IPPROTO_ICMPV6,
	.hook_num = OVSMGR_DP_HOOK_PRE_FLOW_PROC,
	.hook = mc_ipv6_ovs_dp_process,

};
#endif

struct ovsmgr_dp_hook_ops mc_ipv4_dh_hooks = {
	.protocol = IPPROTO_IGMP,
	.hook_num = OVSMGR_DP_HOOK_PRE_FLOW_PROC,
	.hook = mc_ipv4_ovs_dp_process,

};

/* mc_ovs_event
 *	handle event from ovsmgr
 */
static int mc_ovs_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct ovsmgr_notifiers_info *ovs_info = (struct ovsmgr_notifiers_info *)ptr;
	struct ovsmgr_dp_port_info *port = NULL;
	struct mc_struct *mc;

	switch (event) {
	case OVSMGR_DP_PORT_DEL:
		if (ovs_info)
			port = ovs_info->port;
		if (!port || !(port->master) || !(port->dev))
			break;

		rcu_read_lock();
		mc = MC_DEV(port->master);
		if (mc && mc->started)
			mc_nbp_change(mc, port->dev, RTM_DELLINK);
		rcu_read_unlock();
		break;
	default:
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block mc_ovs_notifier = {
	.notifier_call = mc_ovs_event,
	.priority = 1,
};

/* mc_ovs_init
 *	module init
 */
int __init mc_ovs_init(void)
{
	ovsmgr_dp_hook_register(&mc_ipv4_dh_hooks);
#ifdef MC_SUPPORT_MLD
	ovsmgr_dp_hook_register(&mc_ipv6_dh_hooks);
#endif
	ovsmgr_notifier_register(&mc_ovs_notifier);
	return 0;
}

/* mc_ovs_init
 *	cleanup the module
 */
void mc_ovs_exit(void)
{
	ovsmgr_notifier_unregister(&mc_ovs_notifier);
#ifdef MC_SUPPORT_MLD
	ovsmgr_dp_hook_unregister(&mc_ipv6_dh_hooks);
#endif
	ovsmgr_dp_hook_unregister(&mc_ipv4_dh_hooks);
}
