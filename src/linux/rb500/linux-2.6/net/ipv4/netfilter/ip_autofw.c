/*
 * Automatic port forwarding target. When this target is entered, a
 * related connection to a port in the reply direction will be
 * expected. This connection may be mapped to a different port.
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: ip_autofw.c,v 1.1.1.6 2004/04/12 04:33:22 honor Exp $
 */

#include <linux/config.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_autofw.h>
#include <linux/netfilter_ipv4/lockhelp.h>

DECLARE_LOCK(ip_autofw_lock);

#define DEBUGP(format, args...)

static unsigned int
autofw_nat_help(struct ip_conntrack *ct,
	     struct ip_conntrack_expect *exp,
	     struct ip_nat_info *info,
	     enum ip_conntrack_info ctinfo,
	     unsigned int hooknum,
	     struct sk_buff **pskb)
{
	return NF_ACCEPT;
}

static unsigned int
autofw_nat_expected(struct sk_buff **pskb,
		 unsigned int hooknum,
		 struct ip_conntrack *ct,
		 struct ip_nat_info *info)
{
	struct ip_nat_multi_range mr;
	u_int32_t newdstip, newsrcip, newip;
	u_int16_t port;
	struct ip_conntrack *master = master_ct(ct);

	IP_NF_ASSERT(info);
	IP_NF_ASSERT(master);

	IP_NF_ASSERT(!(info->initialized & (1<<HOOK2MANIP(hooknum))));

	DEBUGP("autofw_nat_expected: got ");
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);

	LOCK_BH(&ip_autofw_lock);

	port = ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all);
	newdstip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
#ifdef NEW_PORT_TRIG
	newsrcip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
#else
	newsrcip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
#endif

	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC)
		newip = newsrcip;
	else {
		if (port < ntohs(ct->master->help.exp_autofw_info.dport[0]) ||
		    port > ntohs(ct->master->help.exp_autofw_info.dport[1])) {
			UNLOCK_BH(&ip_autofw_lock);
			return NF_DROP;
		}
		newip = newdstip;
	}

	mr.rangesize = 1;
	/* We don't want to manip the per-protocol, just the IPs... */
	mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
	mr.range[0].min_ip = mr.range[0].max_ip = newip;

	/* ... unless we're doing a MANIP_DST, in which case, make
	   sure we map to the correct port */
	port -= ntohs(ct->master->help.exp_autofw_info.dport[0]);
	port += ntohs(ct->master->help.exp_autofw_info.to[0]);
	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_DST) {
		mr.range[0].flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
		mr.range[0].min = mr.range[0].max
			= ((union ip_conntrack_manip_proto)
				{ htons(port) });
		DEBUGP("autofw_nat_expected: to %u.%u.%u.%u:%hu\n", NIPQUAD(newip), port);
	}

	UNLOCK_BH(&ip_autofw_lock);

	return ip_nat_setup_info(ct, &mr, hooknum);
}


#ifdef NEW_PORT_TRIG

static int autofw_ct_help(const struct iphdr *iph, size_t len,
             struct ip_conntrack *ct, 
             enum ip_conntrack_info ctinfo)
{
	ip_ct_refresh(ct, (AUTOFW_MASTER_TIMEOUT * HZ));
	return NF_ACCEPT;
}

static struct ip_conntrack_helper autofw_ct_helper =
    { { NULL, NULL },
          "autofw",              /* name */
          IP_CT_HELPER_F_REUSE_EXPECT,      /* flags */
          NULL,                 /* module */
          100,                    /* max_ expected */
          0,                  /* timeout */
          { { 0, { 0 } },           /* tuple */
            { 0, { 0 }, IPPROTO_TCP } },
          { { 0, { 0xFFFF } },          /* mask */
            { 0, { 0 }, 0xFFFF } },
          autofw_ct_help             /* helper */
    };

#endif /* NEW_PORT_TRIG */

static struct ip_nat_helper autofw_nat_helper = 
	{ { NULL, NULL },
	  "autofw",				/* name */
	  0,					/* flags */
          NULL,					/* module */
	  { { 0, { 0 } },			/* tuple */
	    { 0, { 0 }, 0 } },
	  { { 0, { 0 } },			/* mask */
	    { 0, { 0 }, 0 } },
	  autofw_nat_help,			/* helper */
	  autofw_nat_expected };		/* expectfn */

static int
autofw_expect(struct ip_conntrack *ct)
{
	u_int16_t port;
	struct ip_conntrack_expect expect, *exp = &expect;

	DEBUGP("autofw_expect: got ");
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);

	LOCK_BH(&ip_autofw_lock);

	port = ntohs(ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all);
	if (port >= ntohs(ct->master->help.exp_autofw_info.dport[0]) &&
	    port <= ntohs(ct->master->help.exp_autofw_info.dport[1])) {
		WRITE_LOCK(&ip_conntrack_lock);
		/* This is so perverse. Pretend like we were expecting to NAT all along. */
		master_ct(ct)->nat.info.helper = &autofw_nat_helper;
#ifdef NEW_PORT_TRIG
		master_ct(ct)->helper = &autofw_ct_helper;
#endif
		DEBUGP("autofw_expect: helper for %p added\n", ct);
		WRITE_UNLOCK(&ip_conntrack_lock);
	}

	/* This is even more perverse. Expect it again. */
	memset(exp, 0, sizeof(struct ip_conntrack_expect));
#ifdef NEW_PORT_TRIG
	exp->tuple.dst.u.udp.port =  ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all;
	exp->mask.dst.u.udp.port = 0xFFFF;
#else
	exp->tuple.src.ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
	exp->mask.src.ip = 0xFFFFFFFF;
#endif
	exp->tuple.dst.protonum = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.protonum;
	exp->mask.dst.protonum = 0xFFFF;
	exp->expectfn = autofw_expect;
	exp->help.exp_autofw_info.dport[0] = ct->master->help.exp_autofw_info.dport[0];
	exp->help.exp_autofw_info.dport[1] = ct->master->help.exp_autofw_info.dport[1];
	exp->help.exp_autofw_info.to[0] = ct->master->help.exp_autofw_info.to[0];
	exp->help.exp_autofw_info.to[1] = ct->master->help.exp_autofw_info.to[1];

	DEBUGP("autofw_expect: expecting ");
	DUMP_TUPLE(&exp->tuple);

	/* Ignore failure; should only happen with NAT */
	ip_conntrack_expect_related(master_ct(ct), exp);

	UNLOCK_BH(&ip_autofw_lock);

	return NF_ACCEPT;	/* unused */
}


static unsigned int
target(struct sk_buff **pskb,
       unsigned int hooknum,
       const struct net_device *in,
       const struct net_device *out,
       const void *targinfo,
       void *userinfo)
{
	const struct ip_autofw_info *info = targinfo;
#ifndef NEW_PORT_TRIG
	const struct iphdr *iph = (*pskb)->nh.iph;
#endif
	struct ip_conntrack *ct;
	enum ip_conntrack_info ctinfo;
	struct ip_conntrack_expect expect, *exp = &expect;
#ifdef NEW_PORT_TRIG
	u_int16_t port;
#endif

	if (!(ct = ip_conntrack_get(*pskb, &ctinfo)))
		return IPT_CONTINUE;

	LOCK_BH(&ip_autofw_lock);

#ifdef NEW_PORT_TRIG
	for (port = ntohs(info->dport[0]); port <= ntohs(info->dport[1]); port++) {
		memset(exp, 0, sizeof(struct ip_conntrack_expect));
		exp->tuple.dst.protonum = info->proto;
		exp->tuple.dst.u.udp.port = htons(port);
		exp->mask.dst.protonum = 0xFFFF;
		exp->mask.dst.u.udp.port = 0xFFFF;

		exp->expectfn = autofw_expect;
 		exp->help.exp_autofw_info.dport[0] = info->dport[0];
 		exp->help.exp_autofw_info.dport[1] = info->dport[1];
 		exp->help.exp_autofw_info.to[0] = info->to[0];
 		exp->help.exp_autofw_info.to[1] = info->to[1];

		DEBUGP("autofw_expect: expecting ");
		DUMP_TUPLE(&exp->tuple);

		/* Ignore failure; should only happen with NAT */
		ip_conntrack_expect_related(ct, exp);
	}
#else
	memset(exp, 0, sizeof(struct ip_conntrack_expect));
	exp->tuple.src.ip = iph->daddr;
	exp->tuple.dst.protonum = info->proto;
	exp->mask.src.ip = 0xFFFFFFFF;
	exp->mask.dst.protonum = 0xFFFF;
	exp->expectfn = autofw_expect;
	exp->help.exp_autofw_info.dport[0] = info->dport[0];
	exp->help.exp_autofw_info.dport[1] = info->dport[1];
	exp->help.exp_autofw_info.to[0] = info->to[0];
	exp->help.exp_autofw_info.to[1] = info->to[1];

	DEBUGP("autofw_expect: expecting ");
	DUMP_TUPLE(&exp->tuple);

	/* Ignore failure; should only happen with NAT */
	ip_conntrack_expect_related(ct, exp);
#endif

	UNLOCK_BH(&ip_autofw_lock);

	return IPT_CONTINUE;
}

/* Must specify -p tcp/udp --dport port */
static int
checkentry(const char *tablename,
	   const struct ipt_entry *e,
	   void *targinfo,
	   unsigned int targinfosize,
	   unsigned int hook_mask)
{
	const struct ip_autofw_info *info = targinfo; 

	if (strcmp(tablename, "nat") != 0) {
		DEBUGP("autofw_check: bad table `%s'.\n", tablename);
		return 0;
	}
	if (targinfosize != IPT_ALIGN(sizeof(*info))) {
		DEBUGP("autofw_check: size %u != %u.\n",
		       targinfosize, sizeof(*info));
		return 0;
	}
	if (hook_mask & ~(1 << NF_IP_PRE_ROUTING)) {
		DEBUGP("autofw_check: bad hooks %x.\n", hook_mask);
		return 0;
	}
	if (info->proto != IPPROTO_TCP && info->proto != IPPROTO_UDP) {
		DEBUGP("autofw_check: bad proto %d.\n", info->proto);
		return 0;
	}
		
	return 1;
}

static struct ipt_target autofw_target
= { { NULL, NULL }, "autofw",
    target, checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_target(&autofw_target);
}

static void __exit fini(void)
{
	ipt_unregister_target(&autofw_target);
}

module_init(init);
module_exit(fini);
