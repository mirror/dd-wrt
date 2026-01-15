/*
 *	LSCAN match for Xtables
 *	Copyright © Jan Engelhardt, 2006 - 2009
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License; either version
 *	2 or 3 as published by the Free Software Foundation.
 */
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/skbuff.h>
#include <linux/stat.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_tcpudp.h>
#include "xt_lscan.h"
#include "compat_xtables.h"
#define PFX KBUILD_MODNAME ": "

enum {
	TCP_FLAGS_ALL3 = TCP_FLAG_FIN | TCP_FLAG_RST | TCP_FLAG_SYN,
	TCP_FLAGS_ALL4 = TCP_FLAGS_ALL3 | TCP_FLAG_ACK,
	TCP_FLAGS_ALL6 = TCP_FLAGS_ALL4 | TCP_FLAG_PSH | TCP_FLAG_URG,
};

/* Module parameters */
static unsigned int
	connmark_mask = ~0,
	packet_mask   = ~0,
	mark_seen     = 0x9,
	mark_synrcv   = 0x1,
	mark_closed   = 0x2,
	mark_synscan  = 0x3,
	mark_estab1   = 0x4,
	mark_estab2   = 0x5,
	mark_cnscan   = 0x6,
	mark_grscan   = 0x7,
	mark_valid    = 0x8;

module_param(connmark_mask, uint, S_IRUGO | S_IWUSR);
module_param(packet_mask,   uint, S_IRUGO | S_IWUSR);
module_param(mark_seen,     uint, S_IRUGO | S_IWUSR);
module_param(mark_synrcv,   uint, S_IRUGO | S_IWUSR);
module_param(mark_closed,   uint, S_IRUGO | S_IWUSR);
module_param(mark_synscan,  uint, S_IRUGO | S_IWUSR);
module_param(mark_estab1,   uint, S_IRUGO | S_IWUSR);
module_param(mark_estab2,   uint, S_IRUGO | S_IWUSR);
module_param(mark_cnscan,   uint, S_IRUGO | S_IWUSR);
module_param(mark_grscan,   uint, S_IRUGO | S_IWUSR);
module_param(mark_valid,    uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(connmark_mask, "only set specified bits in connection mark");
MODULE_PARM_DESC(packet_mask,   "only set specified bits in packet mark");
MODULE_PARM_DESC(mark_seen,     "nfmark value for packet-seen state");
MODULE_PARM_DESC(mark_synrcv,   "connmark value for SYN Received state");
MODULE_PARM_DESC(mark_closed,   "connmark value for closed state");
MODULE_PARM_DESC(mark_synscan,  "connmark value for SYN Scan state");
MODULE_PARM_DESC(mark_estab1,   "connmark value for Established-1 state");
MODULE_PARM_DESC(mark_estab2,   "connmark value for Established-2 state");
MODULE_PARM_DESC(mark_cnscan,   "connmark value for Connect Scan state");
MODULE_PARM_DESC(mark_grscan,   "connmark value for Grab Scan state");
MODULE_PARM_DESC(mark_valid,    "connmark value for Valid state");

/* TCP flag functions */
static inline bool tflg_ack4(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL4) == TCP_FLAG_ACK;
}

static inline bool tflg_ack6(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL6) == TCP_FLAG_ACK;
}

static inline bool tflg_fin(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL3) == TCP_FLAG_FIN;
}

static inline bool tflg_rst(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL3) == TCP_FLAG_RST;
}

static inline bool tflg_rstack(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL4) ==
	       (TCP_FLAG_ACK | TCP_FLAG_RST);
}

static inline bool tflg_syn(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL4) == TCP_FLAG_SYN;
}

static inline bool tflg_synack(const struct tcphdr *th)
{
	return (tcp_flag_word(th) & TCP_FLAGS_ALL4) ==
	       (TCP_FLAG_SYN | TCP_FLAG_ACK);
}

/* lscan functions */
static inline bool lscan_mt_stealth(const struct tcphdr *th)
{
	/*
	 * "Connection refused" replies to our own probes must not be matched.
	 */
	if (tflg_rstack(th))
		return false;

	if (tflg_rst(th) && printk_ratelimit()) {
		printk(KERN_WARNING PFX "Warning: Pure RST received\n");
		return false;
	}

	/*
	 * -p tcp ! --syn -m conntrack --ctstate INVALID: Looking for non-start
	 * packets that are not associated with any connection -- this will
	 * match most scan types (NULL, XMAS, FIN) and ridiculous flag
	 * combinations (SYN-RST, SYN-FIN, SYN-FIN-RST, FIN-RST, etc.).
	 */
	return !tflg_syn(th);
}

static inline unsigned int lscan_mt_full(int mark,
    enum ip_conntrack_info ctstate, bool loopback, const struct tcphdr *tcph,
    unsigned int payload_len)
{
	if (mark == mark_estab2) {
		/*
		 * -m connmark --mark $ESTAB2
		 */
		if (tflg_ack4(tcph) && payload_len == 0)
			return mark; /* keep mark */
		else if (tflg_rst(tcph) || tflg_fin(tcph))
			return mark_grscan;
		else
			return mark_valid;
	} else if (mark == mark_estab1) {
		/*
		 * -m connmark --mark $ESTAB1
		 */
		if (tflg_rst(tcph) || tflg_fin(tcph))
			return mark_cnscan;
		else if (!loopback && tflg_ack4(tcph) && payload_len == 0)
			return mark_estab2;
		else
			return mark_valid;
	} else if (mark == mark_synrcv) {
		/*
		 * -m connmark --mark $SYN
		 */
		if (loopback && tflg_synack(tcph))
			return mark; /* keep mark */
		else if (loopback && tflg_rstack(tcph))
			return mark_closed;
		else if (tflg_ack6(tcph))
			return mark_estab1;
		else
			return mark_synscan;
	} else if (ctstate == IP_CT_NEW && tflg_syn(tcph)) {
		/*
		 * -p tcp --syn --ctstate NEW
		 */
		return mark_synrcv;
	}
	return mark;
}

static bool
lscan_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_lscan_mtinfo *info = par->matchinfo;
	enum ip_conntrack_info ctstate;
	const struct iphdr *iph = ip_hdr(skb);
	const struct tcphdr *tcph;
	struct nf_conn *ctdata;
	struct tcphdr tcph_buf;

	tcph = skb_header_pointer(skb, par->thoff, sizeof(tcph_buf), &tcph_buf);
	if (tcph == NULL)
		return false;
	if (info->match_fl1 & LSCAN_FL1_MIRAI && iph != NULL &&
	    iph->version == 4 && iph->daddr == tcph->seq)
		return true;

	/* Check for invalid packets: -m conntrack --ctstate INVALID */
	ctdata = nf_ct_get(skb, &ctstate);
	if (ctdata == NULL) {
		if (info->match_fl1 & LSCAN_FL1_STEALTH)
			return lscan_mt_stealth(tcph);
		/*
		 * If @ctdata is NULL, we cannot match the other scan
		 * types, return.
		 */
		return false;
	}

	/*
	 * If -m lscan was previously applied to this packet, the rules we
	 * simulate must not be run through again. And for speedup, do not call
	 * it either when the connection is already VALID.
	 */
	if ((ctdata->mark & connmark_mask) == mark_valid ||
	     (skb_nfmark(skb) & packet_mask) != mark_seen) {
		unsigned int n;

		n = lscan_mt_full(ctdata->mark & connmark_mask, ctstate,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
		    par->state->in == init_net.loopback_dev, tcph,
#else
		    par->in == init_net.loopback_dev, tcph,
#endif
		    skb->len - par->thoff - 4 * tcph->doff);

		ctdata->mark = (ctdata->mark & ~connmark_mask) | n;
		skb_nfmark(skb) = (skb_nfmark(skb) & ~packet_mask) ^ mark_seen;
	}

	return (info->match_fl1 & LSCAN_FL1_STEALTH && ctdata->mark == mark_synscan) ||
	       (info->match_fl3 & LSCAN_FL3_CN && ctdata->mark == mark_cnscan) ||
	       (info->match_fl4 & LSCAN_FL4_GR && ctdata->mark == mark_grscan);
}

static int lscan_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_lscan_mtinfo *info = par->matchinfo;

	if ((info->match_fl1 & ~(LSCAN_FL1_STEALTH | LSCAN_FL1_MIRAI)) ||
	    (info->match_fl2 & ~LSCAN_FL2_SYN) ||
	    (info->match_fl3 & ~LSCAN_FL3_CN) ||
	    (info->match_fl4 & ~LSCAN_FL4_GR)) {
		printk(KERN_WARNING PFX "Invalid flags\n");
		return -EINVAL;
	}
	return 0;
}

static struct xt_match lscan_mt_reg[] __read_mostly = {
	{
		.name       = "lscan",
		.revision   = 0,
		.family     = NFPROTO_IPV4,
		.match      = lscan_mt,
		.checkentry = lscan_mt_check,
		.matchsize  = sizeof(struct xt_lscan_mtinfo),
		.proto      = IPPROTO_TCP,
		.me         = THIS_MODULE,
	},
#ifdef WITH_IPV6
	{
		.name       = "lscan",
		.revision   = 0,
		.family     = NFPROTO_IPV6,
		.match      = lscan_mt,
		.checkentry = lscan_mt_check,
		.matchsize  = sizeof(struct xt_lscan_mtinfo),
		.proto      = IPPROTO_TCP,
		.me         = THIS_MODULE,
	},
#endif
};

static int __init lscan_mt_init(void)
{
	return xt_register_matches(lscan_mt_reg,
	       ARRAY_SIZE(lscan_mt_reg));
}

static void __exit lscan_mt_exit(void)
{
	xt_unregister_matches(lscan_mt_reg, ARRAY_SIZE(lscan_mt_reg));
}

module_init(lscan_mt_init);
module_exit(lscan_mt_exit);
MODULE_AUTHOR("Jan Engelhardt ");
MODULE_DESCRIPTION("Xtables: Low-level scan (e.g. nmap) match");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_lscan");
#ifdef WITH_IPV6
MODULE_ALIAS("ip6t_lscan");
#endif