/*
 **************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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
 * nss_nlipv6.c
 *	NSS Netlink IPv6 Handler
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rcupdate.h>
#include <linux/etherdevice.h>
#include <linux/if_addr.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/if_vlan.h>
#include <linux/completion.h>
#include <linux/semaphore.h>
#include <net/addrconf.h>

#include <net/genetlink.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <net/arp.h>
#include <net/neighbour.h>
#include <net/genetlink.h>
#include <net/net_namespace.h>
#include <net/sock.h>

#include <nss_api_if.h>
#include <nss_cmn.h>
#include <nss_ipsec.h>
#include <nss_nl_if.h>
#include "nss_nl.h"
#include "nss_nlipv6.h"
#include "nss_nlcmn_if.h"
#include "nss_nlipv6_if.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0))
#define DST_NEIGH_LOOKUP(dst, ip_addr) dst_neigh_lookup(dst, ip_addr)
#else
#define DST_NEIGH_LOOKUP(dst, ip_addr) dst_get_neighbour_noref(dst)
#endif

/*
 * NSS NETLINK IPv6 context
 */
struct nss_nlipv6_ctx {
	void *nss;
};

/*
 * prototypes
 */

static int nss_nlipv6_ops_create_rule(struct sk_buff *skb, struct genl_info *info);
static int nss_nlipv6_ops_destroy_rule(struct sk_buff *skb, struct genl_info *info);

/*
 * IPV6 family definition
 */
static struct genl_family nss_nlipv6_family = {
	.id = GENL_ID_GENERATE,				/* Auto generate ID */
	.name = NSS_NLIPV6_FAMILY,			/* family name string */
	.hdrsize = sizeof(struct nss_nlipv6_rule),	/* NSS NETLINK IPV6 rule */
	.version = NSS_NL_VER,				/* Set it to NSS_NLIPV6 version */
	.maxattr = NSS_IPV6_MAX_MSG_TYPES,		/* maximum commands supported */
	.netnsok = true,
	.pre_doit = NULL,
	.post_doit = NULL,
};

/*
 * multicast group for sending message status & events
 */
static struct genl_multicast_group nss_nlipv6_mcgrp[] = {
	{.name = NSS_NLIPV6_FAMILY},
};

/*
 * operation table called by the generic netlink layer based on the command
 */
static struct genl_ops nss_nlipv6_ops[] = {
	{.cmd = NSS_IPV6_TX_CREATE_RULE_MSG, .doit = nss_nlipv6_ops_create_rule,},	/* rule create */
	{.cmd = NSS_IPV6_TX_DESTROY_RULE_MSG, .doit = nss_nlipv6_ops_destroy_rule,},	/* rule destroy */
};

#define NSS_NLIPV6_OPS_SZ ARRAY_SIZE(nss_nlipv6_ops)

static struct nss_nlipv6_ctx gbl_ctx;

/*
 * nss_nlipv6_get_neigh()
 * 	Returns neighbour reference for a given IP address
 */
static struct neighbour *nss_nlipv6_get_neigh(uint32_t dst_addr[4])
{
	struct neighbour *neigh;
	struct dst_entry *dst;
	struct rt6_info *rt;
	struct in6_addr daddr;

	IPV6_ADDR_TO_IN6_ADDR(daddr, dst_addr);

	rt = rt6_lookup(&init_net, &daddr, NULL, 0, 0);
	if (!rt) {
		return NULL;
	}

	dst = (struct dst_entry *)rt;

	/*
	 * neighbour lookup using IP address in the route table
	 */
	neigh = DST_NEIGH_LOOKUP(dst, &daddr);
	if (likely(neigh)) {
		neigh_hold(neigh);
		dst_release(dst);

		return neigh;
	}

	return NULL;
}

/*
 * nss_nlipv6_get_addr_hton()
 * 	Convert the ipv6 address from host order to network order.
 */
static inline void nss_nlipv6_get_addr_hton(uint32_t src[4], uint32_t dst[4])
{
	nss_nlipv6_swap_addr(src, dst);

	dst[0] = htonl(dst[0]);
	dst[1] = htonl(dst[1]);
	dst[2] = htonl(dst[2]);
	dst[3] = htonl(dst[3]);
}

/*
 * nss_nlipv6_mac_addr_get()
 * 	Return the hardware (MAC) address of the given ipv6 address, if any.
 *
 * Returns 0 on success or a negative result on failure.
 * We look up the rtable entry for the address and,
 * from its neighbour structure,obtain the hardware address.
 * This means we will also work if the neighbours are routers too.
 */
static int nss_nlipv6_get_macaddr(uint32_t ip_addr[4], uint8_t mac_addr[])
{
	struct neighbour *neigh;
	struct  in6_addr addr;

	nss_nlipv6_get_addr_hton(ip_addr, addr.s6_addr32);

	if (ipv6_addr_is_multicast(&addr)) {
		/*
		 * fixed
		 */
		mac_addr[0] = 0x33;
		mac_addr[1] = 0x33;

		/*
		 * Last word of the IPv6 address.
		 */
		memcpy(&mac_addr[2], &addr.s6_addr32[3], sizeof(uint32_t));
		return 0;

	}

	rcu_read_lock();

	/*
	 * retrieve the neighbour
	 */
	neigh = nss_nlipv6_get_neigh(addr.s6_addr32);
	if (!neigh) {
		rcu_read_unlock();
		nss_nl_info("neighbour lookup failed for %pI6c\n", addr.s6_addr32);
		return -ENODEV;
	}

	rcu_read_unlock();

	if ((neigh->nud_state & NUD_VALID) == 0) {
		nss_nl_info("neighbour state is invalid for %pI6c\n", addr.s6_addr32);
		goto fail;
	}

	if (!neigh->dev) {
		nss_nl_info("neighbour device not found for %pI6c\n", addr.s6_addr32);
		goto fail;
	}

	if (is_multicast_ether_addr(neigh->ha)) {
		nss_nl_info("neighbour MAC address is multicast or broadcast\n");
		goto fail;
	}

	memcpy(mac_addr, neigh->ha, (size_t)neigh->dev->addr_len);
	neigh_release(neigh);
	return 0;
fail:

	neigh_release(neigh);
	return -ENODEV;
}

/*
 * nss_nlipv6_verify_5tuple()
 * 	verify and override 5-tuple entries
 */
static int nss_nlipv6_verify_5tuple(struct nss_ipv6_5tuple *tuple)
{
	/*
	 * protocol must be provided
	 */
	if (!tuple->protocol) {
		nss_nl_info("Empty protocol for 5-tuple\n");
		return -EINVAL;
	}

	if (bitmap_empty((unsigned long *)tuple->flow_ip, NSS_NLIPV6_ADDR_BITS)) {
		nss_nl_info("Empty flow IP\n");
		return -EINVAL;
	}

	if (bitmap_empty((unsigned long *)tuple->return_ip, NSS_NLIPV6_ADDR_BITS)) {
		nss_nl_info("Empty return IP\n");
		return -EINVAL;
	}

	/* Validate the port number */
	switch (tuple->protocol) {
	case NSS_NLIPV6_UDP:
	case NSS_NLIPV6_TCP:
	case NSS_NLIPV6_SCTP:
		if (!tuple->flow_ident) {
			nss_nl_info("Empty flow ident\n");
			return -EINVAL;
		}

		if (!tuple->return_ident) {
			nss_nl_info("Empty return ident\n");
			return -EINVAL;
		}
		break;
	}

	return 0;
}

/*
 * nss_nlipv6_verify_conn_rule()
 * 	verify and override connection rule entries
 */
static int nss_nlipv6_verify_conn_rule(struct nss_ipv6_rule_create_msg *msg, struct net_device *flow_dev,
					struct net_device *return_dev, uint16_t flow_dev_type, uint16_t return_dev_type)
{
	struct nss_ipv6_connection_rule *conn = &msg->conn_rule;
	struct nss_ipv6_nexthop *nexthop = &msg->nexthop_rule;
	const size_t rule_sz = sizeof(struct nss_ipv6_connection_rule);
	bool valid;

	/*
	 * connection rule is not valid ignore rest of the checks
	 */
	valid = msg->valid_flags & NSS_IPV6_RULE_CREATE_CONN_VALID;
	if (!valid) {
		memset(conn, 0, rule_sz);
		return -EINVAL;
	}

	/*
	 * update the flow  & return MAC address
	 */
	if (nss_nlipv6_get_macaddr(msg->tuple.flow_ip, (uint8_t *)conn->flow_mac)) {
		nss_nl_info("Error in Updating the Flow MAC Address \n");
		return -EINVAL;
	}

	if (nss_nlipv6_get_macaddr(msg->tuple.return_ip, (uint8_t *)conn->return_mac)) {
		nss_nl_info("Error in Updating the Return MAC Address \n");
		return -EINVAL;
	}

	/*
	 * update flow and return interface numbers. Handle Ipsec and vlan interfaces seperately.
	 */
	if (flow_dev->type == NSS_IPSEC_ARPHRD_IPSEC)
		conn->flow_interface_num = nss_ipsec_get_ifnum(nss_ipsec_get_data_interface());
	else if (is_vlan_dev(flow_dev))
		conn->flow_interface_num = nss_cmn_get_interface_number_by_dev(vlan_dev_real_dev(flow_dev));
	else {
		if (!flow_dev_type)
			conn->flow_interface_num = nss_cmn_get_interface_number_by_dev(flow_dev);
		else {
			conn->flow_interface_num = nss_cmn_get_interface_number_by_dev_and_type(flow_dev, flow_dev_type);
		}
			nss_nl_info("flow_interface_num: %d flow_interface_type: %d\n", conn->flow_interface_num, flow_dev_type);
}

	if (return_dev->type == NSS_IPSEC_ARPHRD_IPSEC)
		conn->return_interface_num = nss_ipsec_get_ifnum(nss_ipsec_get_data_interface());
	else if (is_vlan_dev(return_dev))
		conn->return_interface_num = nss_cmn_get_interface_number_by_dev(vlan_dev_real_dev(return_dev));
	else {
		if (!return_dev_type)
			conn->return_interface_num = nss_cmn_get_interface_number_by_dev(return_dev);
		else {
			conn->return_interface_num = nss_cmn_get_interface_number_by_dev_and_type(return_dev, return_dev_type);
		}
                        nss_nl_info("return_interface_num: %d return_interface_type: %d\n", conn->return_interface_num, return_dev_type);
	}

	nexthop->flow_nexthop = conn->flow_interface_num;
	nexthop->return_nexthop = conn->return_interface_num;

	nss_nl_info("flow_nexthop:%d return_nexthop:%d\n", nexthop->flow_nexthop, nexthop->return_nexthop);

	/*
	 * update the flow & return MTU(s)
	 */
	conn->flow_mtu = flow_dev->mtu;
	conn->return_mtu = return_dev->mtu;

	return 0;
}

/*
 * nss_nlipv6_verify_tcp_rule()
 * 	verify and override TCP protocol rule entries
 */
static int nss_nlipv6_verify_tcp_rule(struct nss_ipv6_rule_create_msg *msg)
{
	struct nss_ipv6_protocol_tcp_rule *tcp = &msg->tcp_rule;
	const size_t rule_sz = sizeof(struct nss_ipv6_protocol_tcp_rule);
	bool valid;

	/*
	 * tcp rule is not valid ignore rest of the checks
	 */
	valid = msg->valid_flags & NSS_IPV6_RULE_CREATE_TCP_VALID;
	if (!valid) {
		memset(tcp, 0, rule_sz);
		return 0;
	}

	/*
	 * XXX: add addtional checks as required
	 */

	return 0;
}

/*
 * nss_nlipv6_verify_pppoe_rule()
 * 	verify and override pppoe rule entries
 */
static int nss_nlipv6_verify_pppoe_rule(struct nss_ipv6_rule_create_msg *msg)
{
	struct nss_ipv6_pppoe_rule *pppoe = &msg->pppoe_rule;
	const size_t rule_sz = sizeof(struct nss_ipv6_pppoe_rule);
	bool valid;

	/*
	 * pppoe rule is not valid ignore rest of the checks
	 */
	valid = msg->valid_flags & NSS_IPV6_RULE_CREATE_PPPOE_VALID;
	if (!valid) {
		memset(pppoe, 0, rule_sz);
		return 0;
	}

	/*
	 * XXX: add addtional checks as required
	 */
	return 0;
}

/*
 * nss_nlipv6_verify_qos_rule()
 * 	verify and override qos rule entries
 */
static int nss_nlipv6_verify_qos_rule(struct nss_ipv6_rule_create_msg *msg)
{
	struct nss_ipv6_qos_rule *qos = &msg->qos_rule;
	const size_t rule_sz = sizeof(struct nss_ipv6_qos_rule);
	bool valid;

	/*
	 * qos rule is not valid ignore rest of the checks
	 */
	valid = msg->valid_flags & NSS_IPV6_RULE_CREATE_QOS_VALID;
	if (!valid) {
		memset(qos, 0, rule_sz);
		return 0;
	}

	/*
	 * XXX: add addtional checks as required
	 */
	return 0;
}

/*
 * nss_nlipv6_verify_dscp_rule()
 * 	verify and override dscp rule entries
 */
static int nss_nlipv6_verify_dscp_rule(struct nss_ipv6_rule_create_msg *msg)
{
	struct nss_ipv6_dscp_rule *dscp = &msg->dscp_rule;
	const size_t rule_sz = sizeof(struct nss_ipv6_dscp_rule);
	bool valid;

	/*
	 * dscp rule is not valid ignore rest of the checks
	 */
	valid = msg->valid_flags & NSS_IPV6_RULE_CREATE_DSCP_MARKING_VALID;
	if (!valid) {
		memset(dscp, 0, rule_sz);
		return 0;
	}

	/*
	 * XXX: add addtional checks as required
	 */
	return 0;
}

/*
 * nss_nlipv6_verify_vlan_rule()
 * 	verify and override vlan rule entries
 */
static int nss_nlipv6_verify_vlan_rule(struct nss_ipv6_rule_create_msg *msg,
		struct net_device *flow_dev, struct net_device *return_dev)
{
	struct nss_ipv6_vlan_rule *vlan_primary = &msg->vlan_primary_rule;
	struct nss_ipv6_vlan_rule *vlan_secondary = &msg->vlan_secondary_rule;
	bool flow_vlan = is_vlan_dev(flow_dev);
	bool return_vlan = is_vlan_dev(return_dev);

	/*
	 * Fill all with default values.
	 */
	vlan_primary->ingress_vlan_tag = NSS_NLIPV6_VLAN_ID_NOT_CONFIGURED;
	vlan_primary->egress_vlan_tag = NSS_NLIPV6_VLAN_ID_NOT_CONFIGURED;

	vlan_secondary->ingress_vlan_tag = NSS_NLIPV6_VLAN_ID_NOT_CONFIGURED;
	vlan_secondary->egress_vlan_tag = NSS_NLIPV6_VLAN_ID_NOT_CONFIGURED;

	if (!flow_vlan && !return_vlan)
		return 0;

	msg->valid_flags |= NSS_IPV6_RULE_CREATE_VLAN_VALID;

	/*
	 * Add single vlan
	 */
	if (flow_vlan)
		vlan_primary->ingress_vlan_tag = ETH_P_8021Q << 16 | vlan_dev_vlan_id(flow_dev);

	if (return_vlan)
		vlan_primary->egress_vlan_tag = ETH_P_8021Q << 16 | vlan_dev_vlan_id(return_dev);

	/*
	 * XXX: add addtional checks as required
	 */
	return 0;
}

/*
 * nss_nlipv6_process_notify()
 * 	process notification messages from NSS
 */
static void nss_nlipv6_process_notify(void *app_data, struct nss_ipv6_msg *nim)
{
	struct nss_nlipv6_rule *nl_rule;
	struct nss_ipv6_msg *nl_nim;
	struct sk_buff *skb;

	skb = nss_nl_new_msg(&nss_nlipv6_family, nim->cm.type);
	if (!skb) {
		nss_nl_error("unable to allocate NSS_NLIPV6 event\n");
		return;
	}

	nl_rule = nss_nl_get_data(skb);
	nl_nim = &nl_rule->nim;

	/*
	 * initialize the NETLINK common header
	 */
	nss_nlipv6_rule_init(nl_rule, nim->cm.type);

	/*
	 * clear NSS common message items that are not useful to uspace
	 */
	nim->cm.interface = 0;
	nim->cm.cb = (nss_ptr_t)NULL;
	nim->cm.app_data = (nss_ptr_t)NULL;

	/*
	 * copy the contents of the sync message into the NETLINK message
	 */
	memcpy(nl_nim, nim, sizeof(struct nss_ipv6_msg));

	nss_nl_mcast_event(&nss_nlipv6_family, skb);
}

/*
 * nss_nlipv6_resp_create_rule()
 * 	handle response for create rule
 */
static void nss_nlipv6_process_resp(void *app_data, struct nss_ipv6_msg *nim)
{
	struct sk_buff *resp = (struct sk_buff *)app_data;
	struct nss_nlipv6_rule *nl_rule;
	struct nss_ipv6_msg *nl_nim;

	nl_rule = nss_nl_get_data(resp);
	nl_nim = &nl_rule->nim;

	/*
	 * copy the message response data into the NL message buffer. If, FW
	 * has updated the message then we must updated the same into the NL
	 * message as the NL message buffer is different from what was sent
	 * to FW
	 */
	memcpy(nl_nim, nim, sizeof(struct nss_ipv6_msg));

	nss_nl_ucast_resp(resp);
}

/*
 * nss_nlipv6_ops_create_rule()
 * 	rule create handler
 */
static int nss_nlipv6_ops_create_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct net_device *return_dev;
	struct net_device *flow_dev;
	struct nss_nlipv6_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ipv6_msg *nim;
	nss_tx_status_t tx_status;
	struct sk_buff *resp;
	uint32_t pid;
	int error;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlipv6_family, info, NSS_IPV6_TX_CREATE_RULE_MSG);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule create data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlipv6_rule, cm);
	nim = &nl_rule->nim;
	pid = nl_cm->pid;

	/*
	 * extract netdevices for flow and return
	 */
	flow_dev = dev_get_by_name(&init_net, nl_rule->flow_ifname);
	if (!flow_dev) {
		nss_nl_error("%d:flow interface is not available\n", pid);
		return -EINVAL;
	}

	return_dev = dev_get_by_name(&init_net, nl_rule->return_ifname);
	if (!return_dev) {
		dev_put(flow_dev);
		nss_nl_error("%d:return interface is not available\n", pid);
		return -EINVAL;
	}

	/*
	 * check 5-tuple
	 */
	error = nss_nlipv6_verify_5tuple(&nim->msg.rule_create.tuple);
	if (error < 0) {
		nss_nl_error("%d:invalid 5-tuple information passed\n", pid);
		goto done;
	}

	/*
	 * check connection rule
	 */
	error = nss_nlipv6_verify_conn_rule(&nim->msg.rule_create, flow_dev, return_dev,
				nl_rule->flow_iftype, nl_rule->return_iftype);
	if (error < 0) {
		nss_nl_error("%d:invalid conn rule information passed\n", pid);
		goto done;
	}

	/*
	 * check tcp protocol rule
	 */
	error = nss_nlipv6_verify_tcp_rule(&nim->msg.rule_create);
	if (error < 0) {
		nss_nl_error("%d:invalid tcp rule information passed\n", pid);
		goto done;
	}

	/*
	 * check pppoe rule
	 */
	error = nss_nlipv6_verify_pppoe_rule(&nim->msg.rule_create);
	if (error < 0) {
		nss_nl_error("%d:invalid pppoe rule information passed\n", pid);
		goto done;
	}

	/*
	 * check qos rule
	 */
	error = nss_nlipv6_verify_qos_rule(&nim->msg.rule_create);
	if (error < 0) {
		nss_nl_error("%d:invalid qos rule information passed\n", pid);
		goto done;
	}

	/*
	 * check dscp rule
	 */
	error = nss_nlipv6_verify_dscp_rule(&nim->msg.rule_create);
	if (error < 0) {
		nss_nl_error("%d:invalid dscp rule information passed\n", pid);
		goto done;
	}

	/*
	 * check vlan rule
	 */
	error = nss_nlipv6_verify_vlan_rule(&nim->msg.rule_create, flow_dev, return_dev);
	if (error < 0) {
		nss_nl_error("%d:invalid vlan rule information passed\n", pid);
		goto done;
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%d:unable to save response data from NL buffer\n", pid);
		error = -ENOMEM;
		goto done;
	}

	/*
	 * Initialize the common message
	 */
	nss_ipv6_msg_init(nim,					/* ipv6 message */
			NSS_IPV6_RX_INTERFACE,			/* interface number */
			NSS_IPV6_TX_CREATE_RULE_MSG,		/* rule */
			sizeof(struct nss_ipv6_rule_create_msg),/* message size */
			nss_nlipv6_process_resp,		/* response callback */
			(void *)resp);				/* app context */

	/*
	 * Conver the IP addresses to NSS format
	 */
	nss_nlipv6_swap_addr(nim->msg.rule_create.tuple.flow_ip, nim->msg.rule_create.tuple.flow_ip);
	nss_nlipv6_swap_addr(nim->msg.rule_create.tuple.return_ip, nim->msg.rule_create.tuple.return_ip);

	/*
	 * Push Rule to NSS
	 */
	tx_status = nss_ipv6_tx(gbl_ctx.nss, nim);
	if (tx_status != NSS_TX_SUCCESS) {
		nlmsg_free(resp);
		nss_nl_error("%d:unable to send IPV6 rule create, status(%d)\n", pid, tx_status);
		error = -EBUSY;
		goto done;
	}

done:
	dev_put(flow_dev);
	dev_put(return_dev);

	return error;
}

/*
 * nss_nlipv6_ops_destroy_rule()
 * 	rule delete handler
 */
static int nss_nlipv6_ops_destroy_rule(struct sk_buff *skb, struct genl_info *info)
{
	struct nss_nlipv6_rule *nl_rule;
	struct nss_nlcmn *nl_cm;
	struct nss_ipv6_msg *nim;
	nss_tx_status_t tx_status;
	struct sk_buff *resp;
	uint32_t pid;
	int error;

	/*
	 * extract the message payload
	 */
	nl_cm = nss_nl_get_msg(&nss_nlipv6_family, info, NSS_IPV6_TX_DESTROY_RULE_MSG);
	if (!nl_cm) {
		nss_nl_error("unable to extract rule destroy data\n");
		return -EINVAL;
	}

	/*
	 * Message validation required before accepting the configuration
	 */
	nl_rule = container_of(nl_cm, struct nss_nlipv6_rule, cm);
	nim = &nl_rule->nim;
	pid = nl_cm->pid;

	/*
	 * check 5-tuple
	 */
	error = nss_nlipv6_verify_5tuple(&nim->msg.rule_destroy.tuple);
	if (error < 0) {
		nss_nl_error("%d:invalid 5-tuple information passed\n", pid);
		goto done;
	}

	/*
	 * copy the NL message for response
	 */
	resp = nss_nl_copy_msg(skb);
	if (!resp) {
		nss_nl_error("%d:unable to save response data from NL buffer\n", pid);
		error = -ENOMEM;
		goto done;
	}
	/*
	 * Initialize the common message
	 */
	nss_ipv6_msg_init(nim,						/* ipv6 message */
			NSS_IPV6_RX_INTERFACE,				/* interface number */
			NSS_IPV6_TX_DESTROY_RULE_MSG,			/* rule */
			sizeof(struct nss_ipv6_rule_destroy_msg),	/* message size */
			nss_nlipv6_process_resp,			/* response callback */
			(void *)resp);					/* app context */
	/*
	 * Conver the IP addresses to NSS format
	 */
	nss_nlipv6_swap_addr(nim->msg.rule_destroy.tuple.flow_ip, nim->msg.rule_destroy.tuple.flow_ip);
	nss_nlipv6_swap_addr(nim->msg.rule_destroy.tuple.return_ip, nim->msg.rule_destroy.tuple.return_ip);

	/*
	 * Push rule to NSS
	 */
	tx_status = nss_ipv6_tx(gbl_ctx.nss, nim);
	if (tx_status != NSS_TX_SUCCESS) {
		nlmsg_free(resp);
		nss_nl_error("%d:unable to send IPV6 rule delete, status(%d)\n", pid, tx_status);
		return -EBUSY;
	}

done:
	return error;
}

/*
 * nss_nlipv6_init()
 * 	handler init
 */
bool nss_nlipv6_init(void)
{
	int error;

	nss_nl_info_always("Init NSS netlink IPV6 handler\n");

	/*
	 * register NETLINK ops with the family
	 */
	error = genl_register_family_with_ops_groups(&nss_nlipv6_family, nss_nlipv6_ops, nss_nlipv6_mcgrp);
	if (error != 0) {
		nss_nl_info_always("Error: unable to register IPV6 family\n");
		return false;
	}

	/*
	 * register device call back handler for ipv6 from NSS
	 */
	gbl_ctx.nss = nss_ipv6_notify_register(nss_nlipv6_process_notify, &gbl_ctx);
	if (!gbl_ctx.nss) {
		nss_nl_info_always("Error: retreiving the NSS Context \n");
		goto unreg_family;
	}

	return true;

	/*
	 * undo all registeration
	 */
unreg_family:
	genl_unregister_family(&nss_nlipv6_family);

	return false;
}

/*
 * nss_nlipv6_exit()
 *	handler exit
 */
bool nss_nlipv6_exit(void)
{
	int error;

	nss_nl_info_always("Exit NSS netlink IPV6 handler\n");

	/*
	 * unregister the ops family
	 */
	error = genl_unregister_family(&nss_nlipv6_family);
	if (error != 0) {
		nss_nl_info_always("unable to unregister IPV6 NETLINK family\n");
		return false;
	}

	/*
	 * Unregister the device callback handler for ipv6
	 */
	nss_ipv6_notify_unregister();

	gbl_ctx.nss = NULL;

	return true;
}
