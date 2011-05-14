/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/udp.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>
#include <net/checksum.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_log.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>
#if defined (CONFIG_CNS3XXX_SPPE)
#include <linux/cns3xxx/sppe.h>
#endif

static unsigned int nf_ct_udp_timeout __read_mostly = 30*HZ;
static unsigned int nf_ct_udp_timeout_stream __read_mostly = 180*HZ;

static bool udp_pkt_to_tuple(const struct sk_buff *skb,
			     unsigned int dataoff,
			     struct nf_conntrack_tuple *tuple)
{
	const struct udphdr *hp;
	struct udphdr _hdr;

	/* Actually only need first 8 bytes. */
	hp = skb_header_pointer(skb, dataoff, sizeof(_hdr), &_hdr);
	if (hp == NULL)
		return false;

	tuple->src.u.udp.port = hp->source;
	tuple->dst.u.udp.port = hp->dest;

	return true;
}

static bool udp_invert_tuple(struct nf_conntrack_tuple *tuple,
			     const struct nf_conntrack_tuple *orig)
{
	tuple->src.u.udp.port = orig->dst.u.udp.port;
	tuple->dst.u.udp.port = orig->src.u.udp.port;
	return true;
}

/* Print out the per-protocol part of the tuple. */
static int udp_print_tuple(struct seq_file *s,
			   const struct nf_conntrack_tuple *tuple)
{
	return seq_printf(s, "sport=%hu dport=%hu ",
			  ntohs(tuple->src.u.udp.port),
			  ntohs(tuple->dst.u.udp.port));
}

#if defined (CONFIG_CNS3XXX_SPPE)
static int sppe_udp_flow_add_ipv4(struct nf_conn *ct)
{
    SPPE_PARAM param;
    struct nf_conntrack_tuple *orig, *reply;

    orig = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    reply = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;

    memset(&param, 0, sizeof(SPPE_PARAM));

    param.cmd = SPPE_CMD_FLOW_NAT_IPV4;
    param.op = SPPE_OP_SET;

    param.data.flow_nat_ipv4.fw = 0;
    param.data.flow_nat_ipv4.sip = htonl(orig->src.u3.ip);
    param.data.flow_nat_ipv4.dip = htonl(orig->dst.u3.ip);
    param.data.flow_nat_ipv4.l4_prot = SPPE_PROT_UDP;

    param.data.flow_nat_ipv4.l4.port.src = htons(orig->src.u.tcp.port);
    param.data.flow_nat_ipv4.l4.port.dst = htons(orig->dst.u.tcp.port);

    param.data.flow_nat_ipv4.nat_ip = htonl(reply->dst.u3.ip);
    param.data.flow_nat_ipv4.nat_port = htons(reply->dst.u.tcp.port);

    if (sppe_func_hook(&param)) {
        //printk("<%s> fail to add IPv4 UDP from-LAN flow!!\n", __FUNCTION__);
    }
    param.data.flow_nat_ipv4.fw = 1;
    param.data.flow_nat_ipv4.sip = htonl(reply->src.u3.ip);
    param.data.flow_nat_ipv4.dip = htonl(reply->dst.u3.ip);

    param.data.flow_nat_ipv4.l4.port.src = htons(reply->src.u.tcp.port);
    param.data.flow_nat_ipv4.l4.port.dst = htons(reply->dst.u.tcp.port);

    param.data.flow_nat_ipv4.nat_ip = htonl(orig->src.u3.ip);
    param.data.flow_nat_ipv4.nat_port = htons(orig->src.u.tcp.port);

    if (sppe_func_hook(&param)) {
        //printk("<%s> fail to add IPv4 UDP from-WAN flow!!\n", __FUNCTION__);
    }

    return 0;
}

static int sppe_udp_flow_add_ipv6(struct nf_conn *ct)
{
    SPPE_PARAM param;
    struct nf_conntrack_tuple *orig, *reply;

    orig = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    reply = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;

    memset(&param, 0, sizeof(SPPE_PARAM));

    param.cmd = SPPE_CMD_FLOW_ROUTE_IPV6;
    param.op = SPPE_OP_SET;

    /* from-LAN flow */
    param.data.flow_route_ipv6.fw = 0;
    param.data.flow_route_ipv6.sip[0] = htonl(orig->src.u3.ip6[0]);
    param.data.flow_route_ipv6.sip[1] = htonl(orig->src.u3.ip6[1]);
    param.data.flow_route_ipv6.sip[2] = htonl(orig->src.u3.ip6[2]);
    param.data.flow_route_ipv6.sip[3] = htonl(orig->src.u3.ip6[3]);
    param.data.flow_route_ipv6.dip[0] = htonl(orig->dst.u3.ip6[0]);
    param.data.flow_route_ipv6.dip[1] = htonl(orig->dst.u3.ip6[1]);
    param.data.flow_route_ipv6.dip[2] = htonl(orig->dst.u3.ip6[2]);
    param.data.flow_route_ipv6.dip[3] = htonl(orig->dst.u3.ip6[3]);
    param.data.flow_route_ipv6.l4_prot = SPPE_PROT_UDP;
    param.data.flow_route_ipv6.l4.port.src = htons(orig->src.u.udp.port);
    param.data.flow_route_ipv6.l4.port.dst = htons(orig->dst.u.udp.port);

    if (sppe_func_hook(&param)) {
        //printk("<%s> fail to add IPv6 UDP from-LAN flow!!\n", __FUNCTION__);
    }

    /* from-WAN flow */
    param.data.flow_route_ipv6.fw = 1;
    param.data.flow_route_ipv6.sip[0] = htonl(reply->src.u3.ip6[0]);
    param.data.flow_route_ipv6.sip[1] = htonl(reply->src.u3.ip6[1]);
    param.data.flow_route_ipv6.sip[2] = htonl(reply->src.u3.ip6[2]);
    param.data.flow_route_ipv6.sip[3] = htonl(reply->src.u3.ip6[3]);
    param.data.flow_route_ipv6.dip[0] = htonl(reply->dst.u3.ip6[0]);
    param.data.flow_route_ipv6.dip[1] = htonl(reply->dst.u3.ip6[1]);
    param.data.flow_route_ipv6.dip[2] = htonl(reply->dst.u3.ip6[2]);
    param.data.flow_route_ipv6.dip[3] = htonl(reply->dst.u3.ip6[3]);
    param.data.flow_route_ipv6.l4.port.src = htons(reply->src.u.udp.port);
    param.data.flow_route_ipv6.l4.port.dst = htons(reply->dst.u.udp.port);

    if (sppe_func_hook(&param)) {
        //printk("<%s> fail to add IPv6 UDP from-LAN flow!!\n", __FUNCTION__);
    }

    return 0;
}

static int sppe_udp_flow_add(struct nf_conn *ct)
{
    if (0 == sppe_hook_ready) {
        return 0;
    }

    if (AF_INET == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num) {
        sppe_udp_flow_add_ipv4(ct);
        return 0;
    } else if (AF_INET6 == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num) {
        sppe_udp_flow_add_ipv6(ct);
        return 0;
    }

    /* return fail */
    return (-1);
}
#endif


/* Returns verdict for packet, and may modify conntracktype */
static int udp_packet(struct nf_conn *ct,
		      const struct sk_buff *skb,
		      unsigned int dataoff,
		      enum ip_conntrack_info ctinfo,
		      u_int8_t pf,
		      unsigned int hooknum)
{
	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */
	if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
		nf_ct_refresh_acct(ct, ctinfo, skb, nf_ct_udp_timeout_stream);
		/* Also, more likely to be important, and not a probe */
		if (!test_and_set_bit(IPS_ASSURED_BIT, &ct->status))
#if defined (CONFIG_CNS3XXX_SPPE)
        {
#endif
			nf_conntrack_event_cache(IPCT_STATUS, ct);
#if defined (CONFIG_CNS3XXX_SPPE)
            /* Add SPPE hardware flow */
					if(sppe_hook_mode)
            sppe_udp_flow_add(ct);
        }
#endif
	} else
		nf_ct_refresh_acct(ct, ctinfo, skb, nf_ct_udp_timeout);

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static bool udp_new(struct nf_conn *ct, const struct sk_buff *skb,
		    unsigned int dataoff)
{
	return true;
}

static int udp_error(struct net *net, struct sk_buff *skb, unsigned int dataoff,
		     enum ip_conntrack_info *ctinfo,
		     u_int8_t pf,
		     unsigned int hooknum)
{
	unsigned int udplen = skb->len - dataoff;
	const struct udphdr *hdr;
	struct udphdr _hdr;

	/* Header is too small? */
	hdr = skb_header_pointer(skb, dataoff, sizeof(_hdr), &_hdr);
	if (hdr == NULL) {
		if (LOG_INVALID(net, IPPROTO_UDP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				      "nf_ct_udp: short packet ");
		return -NF_ACCEPT;
	}

	/* Truncated/malformed packets */
	if (ntohs(hdr->len) > udplen || ntohs(hdr->len) < sizeof(*hdr)) {
		if (LOG_INVALID(net, IPPROTO_UDP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				"nf_ct_udp: truncated/malformed packet ");
		return -NF_ACCEPT;
	}

	/* Packet with no checksum */
	if (!hdr->check)
		return NF_ACCEPT;

	/* Checksum invalid? Ignore.
	 * We skip checking packets on the outgoing path
	 * because the checksum is assumed to be correct.
	 * FIXME: Source route IP option packets --RR */
	if (net->ct.sysctl_checksum && hooknum == NF_INET_PRE_ROUTING &&
	    nf_checksum(skb, hooknum, dataoff, IPPROTO_UDP, pf)) {
		if (LOG_INVALID(net, IPPROTO_UDP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				"nf_ct_udp: bad UDP checksum ");
		return -NF_ACCEPT;
	}

	return NF_ACCEPT;
}

#ifdef CONFIG_SYSCTL
static unsigned int udp_sysctl_table_users;
static struct ctl_table_header *udp_sysctl_header;
static struct ctl_table udp_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_udp_timeout",
		.data		= &nf_ct_udp_timeout,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_udp_timeout_stream",
		.data		= &nf_ct_udp_timeout_stream,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name	= 0
	}
};
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table udp_compat_sysctl_table[] = {
	{
		.procname	= "ip_conntrack_udp_timeout",
		.data		= &nf_ct_udp_timeout,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_udp_timeout_stream",
		.data		= &nf_ct_udp_timeout_stream,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name	= 0
	}
};
#endif /* CONFIG_NF_CONNTRACK_PROC_COMPAT */
#endif /* CONFIG_SYSCTL */

struct nf_conntrack_l4proto nf_conntrack_l4proto_udp4 __read_mostly =
{
	.l3proto		= PF_INET,
	.l4proto		= IPPROTO_UDP,
	.name			= "udp",
	.pkt_to_tuple		= udp_pkt_to_tuple,
	.invert_tuple		= udp_invert_tuple,
	.print_tuple		= udp_print_tuple,
	.packet			= udp_packet,
	.new			= udp_new,
	.error			= udp_error,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nlattr_tuple_size	= nf_ct_port_nlattr_tuple_size,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &udp_sysctl_table_users,
	.ctl_table_header	= &udp_sysctl_header,
	.ctl_table		= udp_sysctl_table,
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	.ctl_compat_table	= udp_compat_sysctl_table,
#endif
#endif
};
EXPORT_SYMBOL_GPL(nf_conntrack_l4proto_udp4);

struct nf_conntrack_l4proto nf_conntrack_l4proto_udp6 __read_mostly =
{
	.l3proto		= PF_INET6,
	.l4proto		= IPPROTO_UDP,
	.name			= "udp",
	.pkt_to_tuple		= udp_pkt_to_tuple,
	.invert_tuple		= udp_invert_tuple,
	.print_tuple		= udp_print_tuple,
	.packet			= udp_packet,
	.new			= udp_new,
	.error			= udp_error,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nlattr_tuple_size	= nf_ct_port_nlattr_tuple_size,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &udp_sysctl_table_users,
	.ctl_table_header	= &udp_sysctl_header,
	.ctl_table		= udp_sysctl_table,
#endif
};
EXPORT_SYMBOL_GPL(nf_conntrack_l4proto_udp6);
