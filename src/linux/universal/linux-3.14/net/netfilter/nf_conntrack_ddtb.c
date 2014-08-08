#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ddtb.h>

#include <net/ip.h>
#include <net/route.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_nat.h>

static void ddtb_tuple_set_dev(struct ddtb_tuple *t, struct net_device *dev)
{
	if (dev->priv_flags & IFF_802_1Q_VLAN) {
		t->dev = vlan_dev_real_dev(dev);
		t->vid = vlan_dev_vlan_id(dev);
	} else {
		t->dev = dev;
		t->vid = -1;
	}
}

static void ddtb_fill_nf_tuple(struct ddtb_dir *dir, struct nf_conntrack_tuple *t)
{
	memcpy(dir->tuple.dip, &t->dst.u3, sizeof(dir->tuple.dip));
	dir->tuple.dp = t->dst.u.tcp.port;
	memcpy(dir->tuple.sip, &t->src.u3, sizeof(dir->tuple.sip));
	dir->tuple.sp = t->src.u.tcp.port;
}

#if 0
static char *get_ipaddr(__be32 val)
{
	static char bufs[20 * 4];
	static int idx;
	char *buf = &bufs[20 * idx];
	u32 ip = be32_to_cpu(val);

	if (++idx > 3)
		idx = 0;

	sprintf(buf, "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip & 0xff);
	return buf;
}
#endif

static void ddtb_add_ipv4(struct sk_buff *skb, struct nf_conn *ct,
			  u8 protocol, enum ip_conntrack_info ctinfo)
{
	struct net_device *dst_dev;
	struct ddtb_dir *src, *dest;
	struct ddtb_conn c;
	struct rtable *rt;
	bool dst_fastpath;

	if (!test_bit(IPS_ASSURED_BIT, &ct->status))
		return;

	if (ct->ddtb || !skb->orig_dev || !skb_dst(skb) || !skb_dst(skb)->dev)
		return;

	/* make sure its a forwarding conn */
	rt = skb_rtable(skb);
	if (!rt || rt->rt_type != RTN_UNICAST || !rt_is_input_route(rt))
		return;

	memset(&c, 0, sizeof(struct ddtb_conn));
	ddtb_fill_nf_tuple(&c.request, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	ddtb_fill_nf_tuple(&c.reply, &ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	if (CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL) {
		src = &c.request;
		dest = &c.reply;
	} else {
		src = &c.reply;
		dest = &c.request;
	}

#if 0
	printk("packet: %s -> %s\n", get_ipaddr(ip_hdr(skb)->saddr), get_ipaddr(ip_hdr(skb)->daddr));
	printk("dir: %s\n", CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL ? "orig" : "reply");
	printk("conn orig: %s -> %s\n",
		get_ipaddr(src->tuple.sip[0]),
		get_ipaddr(src->tuple.dip[0]));
	printk("conn reply: %s -> %s\n",
		get_ipaddr(dest->tuple.sip[0]),
		get_ipaddr(dest->tuple.dip[0]));
#endif

	/*
	 * When routing from bridge to bridge, we cannot get both the input
	 * and the output device from one packet. Simply allow routing with
	 * fast path via the bridge in that case
	 */
	dst_dev = skb_dst(skb)->dev;
	dst_fastpath = dst_dev->priv_flags & IFF_FAST_PATH;
	if ((dst_dev->priv_flags & skb->orig_dev->priv_flags) & IFF_EBRIDGE)
	    dst_fastpath = true;

	if (dst_fastpath) {
		struct neighbour *neigh;
		bool valid = false;

		/* verify that the target is a unicast addr */
		neigh = dst_neigh_lookup(&rt->dst, &ip_hdr(skb)->daddr);
		if (!neigh)
			return;

		valid = (neigh->nud_state &
			 (NUD_PERMANENT | NUD_REACHABLE | NUD_STALE |
			  NUD_DELAY | NUD_PROBE));
		memcpy(dest->tuple.mac, neigh->ha, ETH_ALEN);
		neigh_release(neigh);
		if (!valid)
			return;

	} else if (!(dst_dev->flags & IFF_POINTOPOINT)) {
		return;
	}

	if (skb->orig_dev->priv_flags & IFF_FAST_PATH)
		memcpy(src->tuple.mac, eth_hdr(skb)->h_source, ETH_ALEN);
	else if (!(skb->orig_dev->flags & IFF_POINTOPOINT))
		return;

	src->tuple.proto = dest->tuple.proto = protocol;
	ddtb_tuple_set_dev(&dest->tuple, dst_dev);
	ddtb_tuple_set_dev(&src->tuple, skb->orig_dev);

	if (ct->status & IPS_DST_NAT)
		c.request.flags |= DDTB_F_DNAT;
	if (ct->status & IPS_SRC_NAT)
		c.request.flags |= DDTB_F_SNAT;

#ifdef CONFIG_NF_CONNTRACK_MARK
	c.mark = ct->mark;
#endif

	rcu_assign_pointer(ct->ddtb, ddtb_ip_add(&c, src->tuple.dev, false));
}

static int ddtb_help_tcp(struct sk_buff *skb, unsigned int protoff,
			 struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	if (ctinfo != IP_CT_ESTABLISHED && ctinfo != IP_CT_ESTABLISHED_REPLY)
		return NF_ACCEPT;

	if (ct->proto.tcp.state >= TCP_CONNTRACK_FIN_WAIT &&
	    ct->proto.tcp.state <= TCP_CONNTRACK_TIME_WAIT)
		return NF_ACCEPT;

	ddtb_add_ipv4(skb, ct, IPPROTO_TCP, ctinfo);
	return NF_ACCEPT;
}

static int ddtb_help_udp(struct sk_buff *skb, unsigned int protoff,
			 struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
	ddtb_add_ipv4(skb, ct, IPPROTO_UDP, ctinfo);
	return NF_ACCEPT;
}

static const struct nf_conntrack_expect_policy ddtb_exp_policy = {
	.max_expected = 1,
};

static struct nf_conntrack_helper ddtb[] __read_mostly = {
	{
		.name = "ddtb",
		.tuple.src.l3num = AF_INET,
		.tuple.dst.protonum = IPPROTO_TCP,
		.help = ddtb_help_tcp,
		.expect_policy = &ddtb_exp_policy,
		.flags = NF_CT_HELPER_F_OVERRIDE,
	},
	{
		.name = "ddtb",
		.tuple.src.l3num = AF_INET,
		.tuple.dst.protonum = IPPROTO_UDP,
		.help = ddtb_help_udp,
		.expect_policy = &ddtb_exp_policy,
		.flags = NF_CT_HELPER_F_OVERRIDE,
	},
};

void nf_conntrack_ddtb_init(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ddtb); i++)
		nf_conntrack_helper_register(&ddtb[i]);
}

void nf_conntrack_ddtb_exit(void)
{
	int i;

	for (i = ARRAY_SIZE(ddtb); i > 0; i--)
		nf_conntrack_helper_register(&ddtb[i - 1]);
}
