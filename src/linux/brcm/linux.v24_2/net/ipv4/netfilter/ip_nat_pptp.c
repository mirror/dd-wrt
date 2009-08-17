/*
 * ip_nat_pptp.c	- Version 1.5
 *
 * NAT support for PPTP (Point to Point Tunneling Protocol).
 * PPTP is a a protocol for creating virtual private networks.
 * It is a specification defined by Microsoft and some vendors
 * working with Microsoft.  PPTP is built on top of a modified
 * version of the Internet Generic Routing Encapsulation Protocol.
 * GRE is defined in RFC 1701 and RFC 1702.  Documentation of
 * PPTP can be found in RFC 2637
 *
 * (C) 2000-2003 by Harald Welte <laforge@gnumonks.org>
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 *
 * TODO: - Support for multiple calls within one session
 * 	   (needs netfilter newnat code)
 * 	 - NAT to a unique tuple, not to TCP source port
 * 	   (needs netfilter tuple reservation)
 *
 * Changes:
 *     2002-02-10 - Version 1.3
 *       - Use ip_nat_mangle_tcp_packet() because of cloned skb's
 *	   in local connections (Philip Craig <philipc@snapgear.com>)
 *       - add checks for magicCookie and pptp version
 *       - make argument list of pptp_{out,in}bound_packet() shorter
 *       - move to C99 style initializers
 *       - print version number at module loadtime
 *     2003-09-22 - Version 1.5
 *       - use SNATed tcp sourceport as callid, since we get called before
 *         TCP header is mangled (Philip Craig <philipc@snapgear.com>)
 * 
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_pptp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_proto_gre.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>

#define IP_NAT_PPTP_VERSION "1.5"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("Netfilter NAT helper module for PPTP");


#if 0
#include "ip_conntrack_pptp_priv.h"
#define DEBUGP(format, args...) printk(KERN_DEBUG __FILE__ ":" __FUNCTION__ \
				       ": " format, ## args)
#else
#define DEBUGP(format, args...)
#endif

static unsigned int
pptp_nat_expected(struct sk_buff **pskb,
		  unsigned int hooknum,
		  struct ip_conntrack *ct,
		  struct ip_nat_info *info)
{
	struct ip_conntrack *master = master_ct(ct);
	struct ip_nat_multi_range mr;
	struct ip_ct_pptp_master *ct_pptp_info;
	struct ip_nat_pptp *nat_pptp_info;
	u_int32_t newip, newcid;
	int ret;

	IP_NF_ASSERT(info);
	IP_NF_ASSERT(master);
	IP_NF_ASSERT(!(info->initialized & (1 << HOOK2MANIP(hooknum))));

	DEBUGP("we have a connection!\n");

	LOCK_BH(&ip_pptp_lock);
	ct_pptp_info = &master->help.ct_pptp_info;
	nat_pptp_info = &master->nat.help.nat_pptp_info;

	/* need to alter GRE tuple because conntrack expectfn() used 'wrong'
	 * (unmanipulated) values */
	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_DST) {
		DEBUGP("completing tuples with NAT info \n");
		/* we can do this, since we're unconfirmed */
		if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.gre.key ==
			htonl(ct_pptp_info->pac_call_id)) {	
			/* assume PNS->PAC */
			ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.gre.key =
				htonl(nat_pptp_info->pns_call_id);
			ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.gre.key =
				htonl(nat_pptp_info->pns_call_id);
			newip = master->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
			newcid = htonl(nat_pptp_info->pac_call_id);
		} else {
			/* assume PAC->PNS */
			ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.gre.key =
				htonl(nat_pptp_info->pac_call_id);
			ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.gre.key =
				htonl(nat_pptp_info->pac_call_id);
			newip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
			newcid = htonl(nat_pptp_info->pns_call_id);
		}
	} else {
		if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.gre.key ==
			htonl(ct_pptp_info->pac_call_id)) {	
			/* assume PNS->PAC */
			newip = master->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
			newcid = htonl(ct_pptp_info->pns_call_id);
		}
		else {
			/* assume PAC->PNS */
			newip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
			newcid = htonl(ct_pptp_info->pac_call_id);
		}
	}

	mr.rangesize = 1;
	mr.range[0].flags = IP_NAT_RANGE_MAP_IPS | IP_NAT_RANGE_PROTO_SPECIFIED;
	mr.range[0].min_ip = mr.range[0].max_ip = newip;
	mr.range[0].min = mr.range[0].max = 
		((union ip_conntrack_manip_proto ) { newcid }); 
	DEBUGP("change ip to %u.%u.%u.%u\n", 
		NIPQUAD(newip));
	DEBUGP("change key to 0x%x\n", ntohl(newcid));
	ret = ip_nat_setup_info(ct, &mr, hooknum);

	UNLOCK_BH(&ip_pptp_lock);

	return ret;

}

/* outbound packets == from PNS to PAC */
static inline unsigned int
pptp_outbound_pkt(struct sk_buff **pskb,
		  struct ip_conntrack *ct,
		  enum ip_conntrack_info ctinfo,
		  struct ip_conntrack_expect *exp)

{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *) iph + iph->ihl*4;
	struct pptp_pkt_hdr *pptph = (struct pptp_pkt_hdr *) 
					((void *)tcph + tcph->doff*4);

	struct PptpControlHeader *ctlh;
	union pptp_ctrl_union pptpReq;
	struct ip_ct_pptp_master *ct_pptp_info = &ct->help.ct_pptp_info;
	struct ip_nat_pptp *nat_pptp_info = &ct->nat.help.nat_pptp_info;

	u_int16_t msg, *cid = NULL, new_callid;

	/* FIXME: size checks !!! */
	ctlh = (struct PptpControlHeader *) ((void *) pptph + sizeof(*pptph));
	pptpReq.rawreq = (void *) ((void *) ctlh + sizeof(*ctlh));

	new_callid = htons(ct_pptp_info->pns_call_id);
	
	switch (msg = ntohs(ctlh->messageType)) {
		case PPTP_OUT_CALL_REQUEST:
			cid = &pptpReq.ocreq->callID;
			/* FIXME: ideally we would want to reserve a call ID
			 * here.  current netfilter NAT core is not able to do
			 * this :( For now we use TCP source port. This breaks
			 * multiple calls within one control session */

			/* save original call ID in nat_info */
			nat_pptp_info->pns_call_id = ct_pptp_info->pns_call_id;

			/* don't use tcph->source since we are at a DSTmanip
			 * hook (e.g. PREROUTING) and pkt is not mangled yet */
			new_callid = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.tcp.port;

			/* save new call ID in ct info */
			ct_pptp_info->pns_call_id = ntohs(new_callid);
			break;
		case PPTP_IN_CALL_REPLY:
			cid = &pptpReq.icreq->callID;
			break;
		case PPTP_CALL_CLEAR_REQUEST:
			cid = &pptpReq.clrreq->callID;
			break;
		default:
			DEBUGP("unknown outbound packet 0x%04x:%s\n", msg,
			      (msg <= PPTP_MSG_MAX)? strMName[msg]:strMName[0]);
			/* fall through */

		case PPTP_SET_LINK_INFO:
			/* only need to NAT in case PAC is behind NAT box */
		case PPTP_START_SESSION_REQUEST:
		case PPTP_START_SESSION_REPLY:
		case PPTP_STOP_SESSION_REQUEST:
		case PPTP_STOP_SESSION_REPLY:
		case PPTP_ECHO_REQUEST:
		case PPTP_ECHO_REPLY:
			/* no need to alter packet */
			return NF_ACCEPT;
	}

	IP_NF_ASSERT(cid);

	DEBUGP("altering call id from 0x%04x to 0x%04x\n",
		ntohs(*cid), ntohs(new_callid));

	/* mangle packet */
	ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, (void *)cid - (void *)pptph,
				 sizeof(new_callid), (char *)&new_callid,
				 sizeof(new_callid));

	return NF_ACCEPT;
}

/* inbound packets == from PAC to PNS */
static inline unsigned int
pptp_inbound_pkt(struct sk_buff **pskb,
		 struct ip_conntrack *ct,
		 enum ip_conntrack_info ctinfo,
		 struct ip_conntrack_expect *oldexp)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *) iph + iph->ihl*4;
	struct pptp_pkt_hdr *pptph = (struct pptp_pkt_hdr *) 
					((void *)tcph + tcph->doff*4);

	struct PptpControlHeader *ctlh;
	union pptp_ctrl_union pptpReq;
	struct ip_ct_pptp_master *ct_pptp_info = &ct->help.ct_pptp_info;
	struct ip_nat_pptp *nat_pptp_info = &ct->nat.help.nat_pptp_info;

	u_int16_t msg, new_cid = 0, new_pcid, *pcid = NULL, *cid = NULL;
	u_int32_t old_dst_ip;

	struct ip_conntrack_tuple t, inv_t;
	struct ip_conntrack_tuple *orig_t, *reply_t;

	/* FIXME: size checks !!! */
	ctlh = (struct PptpControlHeader *) ((void *) pptph + sizeof(*pptph));
	pptpReq.rawreq = (void *) ((void *) ctlh + sizeof(*ctlh));

	new_pcid = htons(nat_pptp_info->pns_call_id);

	switch (msg = ntohs(ctlh->messageType)) {
	case PPTP_OUT_CALL_REPLY:
		pcid = &pptpReq.ocack->peersCallID;	
		cid = &pptpReq.ocack->callID;
		if (!oldexp) {
			DEBUGP("outcall but no expectation\n");
			break;
		}
		old_dst_ip = oldexp->tuple.dst.ip;
		t = oldexp->tuple;
		invert_tuplepr(&inv_t, &t);

		/* save original PAC call ID in nat_info */
		nat_pptp_info->pac_call_id = ct_pptp_info->pac_call_id;

		/* alter expectation */
		orig_t = &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
		reply_t = &ct->tuplehash[IP_CT_DIR_REPLY].tuple;
		if (t.src.ip == orig_t->src.ip && t.dst.ip == orig_t->dst.ip) {
			/* expectation for PNS->PAC direction */
			t.src.u.gre.key = htonl(nat_pptp_info->pns_call_id);
			t.dst.u.gre.key = htonl(ct_pptp_info->pac_call_id);
			inv_t.src.ip = reply_t->src.ip;
			inv_t.dst.ip = reply_t->dst.ip;
			inv_t.src.u.gre.key = htonl(nat_pptp_info->pac_call_id);
			inv_t.dst.u.gre.key = htonl(ct_pptp_info->pns_call_id);
		} else {
			/* expectation for PAC->PNS direction */
			t.src.u.gre.key = htonl(nat_pptp_info->pac_call_id);
			t.dst.u.gre.key = htonl(ct_pptp_info->pns_call_id);
			inv_t.src.ip = orig_t->src.ip;
			inv_t.dst.ip = orig_t->dst.ip;
			inv_t.src.u.gre.key = htonl(nat_pptp_info->pns_call_id);
			inv_t.dst.u.gre.key = htonl(ct_pptp_info->pac_call_id);
		}

		if (!ip_conntrack_change_expect(oldexp, &t)) {
			DEBUGP("successfully changed expect\n");
		} else {
			DEBUGP("can't change expect\n");
		}
		if (oldexp->proto.gre.keymap_orig)
			ip_ct_gre_keymap_change(oldexp->proto.gre.keymap_orig, 
						&t);
		if (oldexp->proto.gre.keymap_reply)
			ip_ct_gre_keymap_change(oldexp->proto.gre.keymap_reply, 
						&inv_t);
		break;
	case PPTP_IN_CALL_CONNECT:
		pcid = &pptpReq.iccon->peersCallID;
		if (!oldexp)
			break;
		old_dst_ip = oldexp->tuple.dst.ip;
		t = oldexp->tuple;

		/* alter expectation, no need for callID */
		if (t.dst.ip == ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip) {
			/* expectation for PNS->PAC direction */
			t.src.ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
		} else {
			/* expectation for PAC->PNS direction */
			t.dst.ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
		}

		if (!ip_conntrack_change_expect(oldexp, &t)) {
			DEBUGP("successfully changed expect\n");
		} else {
			DEBUGP("can't change expect\n");
		}
		break;
	case PPTP_IN_CALL_REQUEST:
		/* only need to nat in case PAC is behind NAT box */
		break;
	case PPTP_WAN_ERROR_NOTIFY:
		pcid = &pptpReq.wanerr->peersCallID;
		break;
	case PPTP_CALL_DISCONNECT_NOTIFY:
		pcid = &pptpReq.disc->callID;
		break;
	case PPTP_SET_LINK_INFO:
		pcid = &pptpReq.setlink->peersCallID;
		break;

	default:
		DEBUGP("unknown inbound packet %s\n",
			(msg <= PPTP_MSG_MAX)? strMName[msg]:strMName[0]);
		/* fall through */

	case PPTP_START_SESSION_REQUEST:
	case PPTP_START_SESSION_REPLY:
	case PPTP_STOP_SESSION_REQUEST:
	case PPTP_STOP_SESSION_REPLY:
	case PPTP_ECHO_REQUEST:
	case PPTP_ECHO_REPLY:
		/* no need to alter packet */
		return NF_ACCEPT;
	}

	/* mangle packet */
	IP_NF_ASSERT(pcid);
	DEBUGP("altering peer call id from 0x%04x to 0x%04x\n",
		ntohs(*pcid), ntohs(new_pcid));
	ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, (void *)pcid - (void *)pptph,
				 sizeof(new_pcid), (char *)&new_pcid, 
				 sizeof(new_pcid));

	if (new_cid) {
		IP_NF_ASSERT(cid);
		DEBUGP("altering call id from 0x%04x to 0x%04x\n",
			ntohs(*cid), ntohs(new_cid));
		ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, 
					 (void *)cid - (void *)pptph, 
					 sizeof(new_cid), (char *)&new_cid, 
					 sizeof(new_cid));
	}

	/* great, at least we don't need to resize packets */
	return NF_ACCEPT;
}


static unsigned int tcp_help(struct ip_conntrack *ct,
			     struct ip_conntrack_expect *exp,
			     struct ip_nat_info *info,
			     enum ip_conntrack_info ctinfo,
			     unsigned int hooknum, struct sk_buff **pskb)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *) iph + iph->ihl*4;
	unsigned int datalen = (*pskb)->len - iph->ihl*4 - tcph->doff*4;
	struct pptp_pkt_hdr *pptph;

	int dir;

	DEBUGP("entering\n");

	/* Only mangle things once: DST for original direction
	   and SRC for reply direction. */
	dir = CTINFO2DIR(ctinfo);
	if (!((HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC
	     && dir == IP_CT_DIR_ORIGINAL)
	      || (HOOK2MANIP(hooknum) == IP_NAT_MANIP_DST
		  && dir == IP_CT_DIR_REPLY))) {
		DEBUGP("Not touching dir %s at hook %s\n",
		       dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
		       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
		       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
		       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT"
		       : hooknum == NF_IP_LOCAL_IN ? "INPUT" : "???");
		return NF_ACCEPT;
	}

	/* if packet is too small, just skip it */
	if (datalen < sizeof(struct pptp_pkt_hdr)+
		      sizeof(struct PptpControlHeader)) {
		DEBUGP("pptp packet too short\n");
		return NF_ACCEPT;	
	}

	pptph = (struct pptp_pkt_hdr *) ((void *)tcph + tcph->doff*4);

	/* if it's not a control message, we can't handle it */
	if (ntohs(pptph->packetType) != PPTP_PACKET_CONTROL ||
	    ntohl(pptph->magicCookie) != PPTP_MAGIC_COOKIE) {
		DEBUGP("not a pptp control packet\n");
		return NF_ACCEPT;
	}

	LOCK_BH(&ip_pptp_lock);

	if (dir == IP_CT_DIR_ORIGINAL) {
		/* reuqests sent by client to server (PNS->PAC) */
		pptp_outbound_pkt(pskb, ct, ctinfo, exp);
	} else {
		/* response from the server to the client (PAC->PNS) */
		pptp_inbound_pkt(pskb, ct, ctinfo, exp);
	}

	UNLOCK_BH(&ip_pptp_lock);

	return NF_ACCEPT;
}

/* nat helper struct for control connection */
static struct ip_nat_helper pptp_tcp_helper = { 
	.list = { NULL, NULL },
	.name = "pptp", 
	.flags = IP_NAT_HELPER_F_ALWAYS, 
	.me = THIS_MODULE,
	.tuple = { .src = { .ip = 0, 
			    .u = { .tcp = { .port = 
				    	__constant_htons(PPTP_CONTROL_PORT) } 
				 } 
			  },
	  	   .dst = { .ip = 0, 
			    .u = { .all = 0 }, 
			    .protonum = IPPROTO_TCP 
		   	  } 
		 },

	.mask = { .src = { .ip = 0, 
			   .u = { .tcp = { .port = 0xFFFF } } 
			 },
		  .dst = { .ip = 0, 
			   .u = { .all = 0 }, 
			   .protonum = 0xFFFF 
		  	 } 
		},
	.help = tcp_help, 
	.expect = pptp_nat_expected 
};

			  
static int __init init(void)
{
	DEBUGP("%s: registering NAT helper\n", __FILE__);
	if (ip_nat_helper_register(&pptp_tcp_helper)) {
		printk(KERN_ERR "Unable to register NAT application helper "
				"for pptp\n");
		return -EIO;
	}

	printk("ip_nat_pptp version %s loaded\n", IP_NAT_PPTP_VERSION);
	return 0;
}

static void __exit fini(void)
{
	DEBUGP("cleanup_module\n" );
	ip_nat_helper_unregister(&pptp_tcp_helper);
	printk("ip_nat_pptp version %s unloaded\n", IP_NAT_PPTP_VERSION);
}

module_init(init);
module_exit(fini);
