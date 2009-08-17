/*
 * ip_conntrack_pptp.c	- Version 1.9
 *
 * Connection tracking support for PPTP (Point to Point Tunneling Protocol).
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
 * Limitations:
 * 	 - We blindly assume that control connections are always
 * 	   established in PNS->PAC direction.  This is a violation
 * 	   of RFFC2673
 *
 * TODO: - finish support for multiple calls within one session
 * 	   (needs expect reservations in newnat)
 *	 - testing of incoming PPTP calls 
 *
 * Changes: 
 * 	2002-02-05 - Version 1.3
 * 	  - Call ip_conntrack_unexpect_related() from 
 * 	    pptp_timeout_related() to destroy expectations in case
 * 	    CALL_DISCONNECT_NOTIFY or tcp fin packet was seen
 * 	    (Philip Craig <philipc@snapgear.com>)
 * 	  - Add Version information at module loadtime
 * 	2002-02-10 - Version 1.6
 * 	  - move to C99 style initializers
 * 	  - remove second expectation if first arrives
 *
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_proto_gre.h>
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>

#define IP_CT_PPTP_VERSION "1.9"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("Netfilter connection tracking helper module for PPTP");

DECLARE_LOCK(ip_pptp_lock);

#if 0
#include "ip_conntrack_pptp_priv.h"
#define DEBUGP(format, args...)	printk(KERN_DEBUG __FILE__ ":" __FUNCTION__ \
					": " format, ## args)
#else
#define DEBUGP(format, args...)
#endif

#define SECS *HZ
#define MINS * 60 SECS
#define HOURS * 60 MINS
#define DAYS * 24 HOURS

#define PPTP_GRE_TIMEOUT 		(10 MINS)
#define PPTP_GRE_STREAM_TIMEOUT 	(5 DAYS)

static int pptp_expectfn(struct ip_conntrack *ct)
{
	struct ip_conntrack *master;
	struct ip_conntrack_expect *exp;

	DEBUGP("increasing timeouts\n");
	/* increase timeout of GRE data channel conntrack entry */
	ct->proto.gre.timeout = PPTP_GRE_TIMEOUT;
	ct->proto.gre.stream_timeout = PPTP_GRE_STREAM_TIMEOUT;

	master = master_ct(ct);
	if (!master) {
		DEBUGP(" no master!!!\n");
		return 0;
	}

	exp = ct->master;
	if (!exp) {
		DEBUGP("no expectation!!\n");
		return 0;
	}

	DEBUGP("completing tuples with ct info\n");
	/* we can do this, since we're unconfirmed */
	if (ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.gre.key == 
		htonl(master->help.ct_pptp_info.pac_call_id)) {	
		/* assume PNS->PAC */
		ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.gre.key = 
			htonl(master->help.ct_pptp_info.pns_call_id);
		ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.gre.key =
			htonl(master->help.ct_pptp_info.pns_call_id);
	} else {
		/* assume PAC->PNS */
		ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.gre.key =
			htonl(master->help.ct_pptp_info.pac_call_id);
		ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.gre.key =
			htonl(master->help.ct_pptp_info.pac_call_id);
	}
	
	/* delete other expectation */
	if (exp->expected_list.next != &exp->expected_list) {
		struct ip_conntrack_expect *other_exp;
		struct list_head *cur_item, *next;

		for (cur_item = master->sibling_list.next;
		     cur_item != &master->sibling_list; cur_item = next) {
			next = cur_item->next;
			other_exp = list_entry(cur_item,
					       struct ip_conntrack_expect,
					       expected_list);
			/* remove only if occurred at same sequence number */
			if (other_exp != exp && other_exp->seq == exp->seq) {
				DEBUGP("unexpecting other direction\n");
				ip_ct_gre_keymap_destroy(other_exp);
				ip_conntrack_unexpect_related(other_exp);
			}
		}
	}

	return 0;
}

/* timeout GRE data connections */
static int pptp_timeout_related(struct ip_conntrack *ct, const struct iphdr *iph)
{
	struct list_head *cur_item, *next;
	struct ip_conntrack_expect *exp;
	enum ip_conntrack_info ctinfo;

	/* FIXME: do we have to lock something ? */
	for (cur_item = ct->sibling_list.next;
	    cur_item != &ct->sibling_list; cur_item = next) {
		next = cur_item->next;
		exp = list_entry(cur_item, struct ip_conntrack_expect,
				 expected_list);

		ip_ct_gre_keymap_destroy(exp);
		if (!exp->sibling) {
			ip_conntrack_unexpect_related(exp);
			continue;
		}

		DEBUGP("setting timeout of conntrack %p to 0\n",
			exp->sibling);
		exp->sibling->proto.gre.timeout = 0;
		exp->sibling->proto.gre.stream_timeout = 0;
		ip_ct_refresh_acct(exp->sibling, ctinfo, iph, 0);
	}

	return 0;
}

/* expect GRE connections (PNS->PAC and PAC->PNS direction) */
static inline int
exp_gre(struct ip_conntrack *master,
	u_int32_t seq,
	u_int16_t callid,
	u_int16_t peer_callid)
{
	struct ip_conntrack_expect exp;
	struct ip_conntrack_tuple inv_tuple;

	memset(&exp, 0, sizeof(exp));
	/* tuple in original direction, PNS->PAC */
	exp.tuple.src.ip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
	exp.tuple.src.u.gre.key = htonl(ntohs(peer_callid));
	exp.tuple.dst.ip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
	exp.tuple.dst.u.gre.key = htonl(ntohs(callid));
	exp.tuple.dst.protonum = IPPROTO_GRE;

	exp.mask.src.ip = 0xffffffff;
	exp.mask.src.u.all = 0;
	exp.mask.dst.u.all = 0;
	exp.mask.dst.u.gre.key = 0xffffffff;
	exp.mask.dst.ip = 0xffffffff;
	exp.mask.dst.protonum = 0xffff;
			
	exp.seq = seq;
	exp.expectfn = pptp_expectfn;

	exp.help.exp_pptp_info.pac_call_id = ntohs(callid);
	exp.help.exp_pptp_info.pns_call_id = ntohs(peer_callid);

	DEBUGP("calling expect_related ");
	DUMP_TUPLE_RAW(&exp.tuple);
	
	/* Add GRE keymap entries */
	if (ip_ct_gre_keymap_add(&exp, &exp.tuple, 0) != 0)
		return 1;

	invert_tuplepr(&inv_tuple, &exp.tuple);
	if (ip_ct_gre_keymap_add(&exp, &inv_tuple, 1) != 0) {
		ip_ct_gre_keymap_destroy(&exp);
		return 1;
	}
	
	if (ip_conntrack_expect_related(master, &exp) != 0) {
		ip_ct_gre_keymap_destroy(&exp);
		DEBUGP("cannot expect_related()\n");
		return 1;
	}

	/* tuple in reply direction, PAC->PNS */
	exp.tuple.src.ip = master->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
	exp.tuple.src.u.gre.key = htonl(ntohs(callid));
	exp.tuple.dst.ip = master->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
	exp.tuple.dst.u.gre.key = htonl(ntohs(peer_callid));

	DEBUGP("calling expect_related ");
	DUMP_TUPLE_RAW(&exp.tuple);
	
	/* Add GRE keymap entries */
	ip_ct_gre_keymap_add(&exp, &exp.tuple, 0);
	invert_tuplepr(&inv_tuple, &exp.tuple);
	ip_ct_gre_keymap_add(&exp, &inv_tuple, 1);
	/* FIXME: cannot handle error correctly, since we need to free
	 * the above keymap :( */
	
	if (ip_conntrack_expect_related(master, &exp) != 0) {
		/* free the second pair of keypmaps */
		ip_ct_gre_keymap_destroy(&exp);
		DEBUGP("cannot expect_related():\n");
		return 1;
	}

	return 0;
}

static inline int 
pptp_inbound_pkt(struct tcphdr *tcph,
		 struct pptp_pkt_hdr *pptph, 
		 size_t datalen,
		 struct ip_conntrack *ct,
		 enum ip_conntrack_info ctinfo,
		 const struct iphdr *iph)
{
	struct PptpControlHeader *ctlh;
        union pptp_ctrl_union pptpReq;
	
	struct ip_ct_pptp_master *info = &ct->help.ct_pptp_info;
	u_int16_t msg, *cid, *pcid;
	u_int32_t seq;	

	ctlh = (struct PptpControlHeader *) 
		((char *) pptph + sizeof(struct pptp_pkt_hdr));
	pptpReq.rawreq = (void *) 
		((char *) ctlh + sizeof(struct PptpControlHeader));

	msg = ntohs(ctlh->messageType);
	DEBUGP("inbound control message %s\n", strMName[msg]);

	switch (msg) {
	case PPTP_START_SESSION_REPLY:
		/* server confirms new control session */
		if (info->sstate < PPTP_SESSION_REQUESTED) {
			DEBUGP("%s without START_SESS_REQUEST\n",
				strMName[msg]);
			break;
		}
		if (pptpReq.srep->resultCode == PPTP_START_OK)
			info->sstate = PPTP_SESSION_CONFIRMED;
		else 
			info->sstate = PPTP_SESSION_ERROR;
		break;

	case PPTP_STOP_SESSION_REPLY:
		/* server confirms end of control session */
		if (info->sstate > PPTP_SESSION_STOPREQ) {
			DEBUGP("%s without STOP_SESS_REQUEST\n",
				strMName[msg]);
			break;
		}
		if (pptpReq.strep->resultCode == PPTP_STOP_OK)
			info->sstate = PPTP_SESSION_NONE;
		else
			info->sstate = PPTP_SESSION_ERROR;
		break;

	case PPTP_OUT_CALL_REPLY:
		/* server accepted call, we now expect GRE frames */
		if (info->sstate != PPTP_SESSION_CONFIRMED) {
			DEBUGP("%s but no session\n", strMName[msg]);
			break;
		}
		if (info->cstate != PPTP_CALL_OUT_REQ &&
		    info->cstate != PPTP_CALL_OUT_CONF) {
			DEBUGP("%s without OUTCALL_REQ\n", strMName[msg]);
			break;
		}
		if (pptpReq.ocack->resultCode != PPTP_OUTCALL_CONNECT) {
			info->cstate = PPTP_CALL_NONE;
			break;
		}

		cid = &pptpReq.ocack->callID;
		pcid = &pptpReq.ocack->peersCallID;

		info->pac_call_id = ntohs(*cid);
		
		if (htons(info->pns_call_id) != *pcid) {
			DEBUGP("%s for unknown callid %u\n",
				strMName[msg], ntohs(*pcid));
			break;
		}

		DEBUGP("%s, CID=%X, PCID=%X\n", strMName[msg], 
			ntohs(*cid), ntohs(*pcid));
		
		info->cstate = PPTP_CALL_OUT_CONF;

		seq = ntohl(tcph->seq) + ((void *)pcid - (void *)pptph);
		if (exp_gre(ct, seq, *cid, *pcid) != 0)
			printk("ip_conntrack_pptp: error during exp_gre\n");
		break;

	case PPTP_IN_CALL_REQUEST:
		/* server tells us about incoming call request */
		if (info->sstate != PPTP_SESSION_CONFIRMED) {
			DEBUGP("%s but no session\n", strMName[msg]);
			break;
		}
		pcid = &pptpReq.icack->peersCallID;
		DEBUGP("%s, PCID=%X\n", strMName[msg], ntohs(*pcid));
		info->cstate = PPTP_CALL_IN_REQ;
		info->pac_call_id= ntohs(*pcid);
		break;

	case PPTP_IN_CALL_CONNECT:
		/* server tells us about incoming call established */
		if (info->sstate != PPTP_SESSION_CONFIRMED) {
			DEBUGP("%s but no session\n", strMName[msg]);
			break;
		}
		if (info->sstate != PPTP_CALL_IN_REP
		    && info->sstate != PPTP_CALL_IN_CONF) {
			DEBUGP("%s but never sent IN_CALL_REPLY\n",
				strMName[msg]);
			break;
		}

		pcid = &pptpReq.iccon->peersCallID;
		cid = &info->pac_call_id;

		if (info->pns_call_id != ntohs(*pcid)) {
			DEBUGP("%s for unknown CallID %u\n", 
				strMName[msg], ntohs(*cid));
			break;
		}

		DEBUGP("%s, PCID=%X\n", strMName[msg], ntohs(*pcid));
		info->cstate = PPTP_CALL_IN_CONF;

		/* we expect a GRE connection from PAC to PNS */
		seq = ntohl(tcph->seq) + ((void *)pcid - (void *)pptph);
		if (exp_gre(ct, seq, *cid, *pcid) != 0)
			printk("ip_conntrack_pptp: error during exp_gre\n");

		break;

	case PPTP_CALL_DISCONNECT_NOTIFY:
		/* server confirms disconnect */
		cid = &pptpReq.disc->callID;
		DEBUGP("%s, CID=%X\n", strMName[msg], ntohs(*cid));
		info->cstate = PPTP_CALL_NONE;

		/* untrack this call id, unexpect GRE packets */
		pptp_timeout_related(ct, iph);
		break;

	case PPTP_WAN_ERROR_NOTIFY:
		break;

	case PPTP_ECHO_REQUEST:
	case PPTP_ECHO_REPLY:
		/* I don't have to explain these ;) */
		break;
	default:
		DEBUGP("invalid %s (TY=%d)\n", (msg <= PPTP_MSG_MAX)
			? strMName[msg]:strMName[0], msg);
		break;
	}

	return NF_ACCEPT;

}

static inline int
pptp_outbound_pkt(struct tcphdr *tcph,
		  struct pptp_pkt_hdr *pptph,
		  size_t datalen,
		  struct ip_conntrack *ct,
		  enum ip_conntrack_info ctinfo)
{
	struct PptpControlHeader *ctlh;
        union pptp_ctrl_union pptpReq;
	struct ip_ct_pptp_master *info = &ct->help.ct_pptp_info;
	u_int16_t msg, *cid, *pcid;

	ctlh = (struct PptpControlHeader *) ((void *) pptph + sizeof(*pptph));
	pptpReq.rawreq = (void *) ((void *) ctlh + sizeof(*ctlh));

	msg = ntohs(ctlh->messageType);
	DEBUGP("outbound control message %s\n", strMName[msg]);

	switch (msg) {
	case PPTP_START_SESSION_REQUEST:
		/* client requests for new control session */
		if (info->sstate != PPTP_SESSION_NONE) {
			DEBUGP("%s but we already have one",
				strMName[msg]);
		}
		info->sstate = PPTP_SESSION_REQUESTED;
		break;
	case PPTP_STOP_SESSION_REQUEST:
		/* client requests end of control session */
		info->sstate = PPTP_SESSION_STOPREQ;
		break;

	case PPTP_OUT_CALL_REQUEST:
		/* client initiating connection to server */
		if (info->sstate != PPTP_SESSION_CONFIRMED) {
			DEBUGP("%s but no session\n",
				strMName[msg]);
			break;
		}
		info->cstate = PPTP_CALL_OUT_REQ;
		/* track PNS call id */
		cid = &pptpReq.ocreq->callID;
		DEBUGP("%s, CID=%X\n", strMName[msg], ntohs(*cid));
		info->pns_call_id = ntohs(*cid);
		break;
	case PPTP_IN_CALL_REPLY:
		/* client answers incoming call */
		if (info->cstate != PPTP_CALL_IN_REQ
		    && info->cstate != PPTP_CALL_IN_REP) {
			DEBUGP("%s without incall_req\n", 
				strMName[msg]);
			break;
		}
		if (pptpReq.icack->resultCode != PPTP_INCALL_ACCEPT) {
			info->cstate = PPTP_CALL_NONE;
			break;
		}
		pcid = &pptpReq.icack->peersCallID;
		if (info->pac_call_id != ntohs(*pcid)) {
			DEBUGP("%s for unknown call %u\n", 
				strMName[msg], ntohs(*pcid));
			break;
		}
		DEBUGP("%s, CID=%X\n", strMName[msg], ntohs(*pcid));
		/* part two of the three-way handshake */
		info->cstate = PPTP_CALL_IN_REP;
		info->pns_call_id = ntohs(pptpReq.icack->callID);
		break;

	case PPTP_CALL_CLEAR_REQUEST:
		/* client requests hangup of call */
		if (info->sstate != PPTP_SESSION_CONFIRMED) {
			DEBUGP("CLEAR_CALL but no session\n");
			break;
		}
		/* FUTURE: iterate over all calls and check if
		 * call ID is valid.  We don't do this without newnat,
		 * because we only know about last call */
		info->cstate = PPTP_CALL_CLEAR_REQ;
		break;
	case PPTP_SET_LINK_INFO:
		break;
	case PPTP_ECHO_REQUEST:
	case PPTP_ECHO_REPLY:
		/* I don't have to explain these ;) */
		break;
	default:
		DEBUGP("invalid %s (TY=%d)\n", (msg <= PPTP_MSG_MAX)? 
			strMName[msg]:strMName[0], msg);
		/* unknown: no need to create GRE masq table entry */
		break;
	}

	return NF_ACCEPT;
}


/* track caller id inside control connection, call expect_related */
static int 
conntrack_pptp_help(const struct iphdr *iph, size_t len,
		    struct ip_conntrack *ct, enum ip_conntrack_info ctinfo)

{
	struct pptp_pkt_hdr *pptph;
	
	struct tcphdr *tcph = (void *) iph + iph->ihl * 4;
	u_int32_t tcplen = len - iph->ihl * 4;
	u_int32_t datalen = tcplen - tcph->doff * 4;
	void *datalimit;
	int dir = CTINFO2DIR(ctinfo);
	struct ip_ct_pptp_master *info = &ct->help.ct_pptp_info;

	int oldsstate, oldcstate;
	int ret;

	/* don't do any tracking before tcp handshake complete */
	if (ctinfo != IP_CT_ESTABLISHED 
	    && ctinfo != IP_CT_ESTABLISHED+IP_CT_IS_REPLY) {
		DEBUGP("ctinfo = %u, skipping\n", ctinfo);
		return NF_ACCEPT;
	}
	
	/* not a complete TCP header? */
	if (tcplen < sizeof(struct tcphdr) || tcplen < tcph->doff * 4) {
		DEBUGP("tcplen = %u\n", tcplen);
		return NF_ACCEPT;
	}

	/* checksum invalid? */
	if (tcp_v4_check(tcph, tcplen, iph->saddr, iph->daddr,
			csum_partial((char *) tcph, tcplen, 0))) {
		printk(KERN_NOTICE __FILE__ ": bad csum\n");
		/* W2K PPTP server sends TCP packets with wrong checksum :(( */
		//return NF_ACCEPT;
	}

	if (tcph->fin || tcph->rst) {
		DEBUGP("RST/FIN received, timeouting GRE\n");
		/* can't do this after real newnat */
		info->cstate = PPTP_CALL_NONE;

		/* untrack this call id, unexpect GRE packets */
		pptp_timeout_related(ct, iph);
	}


	pptph = (struct pptp_pkt_hdr *) ((void *) tcph + tcph->doff * 4);
	datalimit = (void *) pptph + datalen;

	/* not a full pptp packet header? */
	if ((void *) pptph+sizeof(*pptph) >= datalimit) {
		DEBUGP("no full PPTP header, can't track\n");
		return NF_ACCEPT;
	}
	
	/* if it's not a control message we can't do anything with it */
        if (ntohs(pptph->packetType) != PPTP_PACKET_CONTROL ||
	    ntohl(pptph->magicCookie) != PPTP_MAGIC_COOKIE) {
		DEBUGP("not a control packet\n");
		return NF_ACCEPT;
	}

	oldsstate = info->sstate;
	oldcstate = info->cstate;

	LOCK_BH(&ip_pptp_lock);

	/* FIXME: We just blindly assume that the control connection is always
	 * established from PNS->PAC.  However, RFC makes no guarantee */
	if (dir == IP_CT_DIR_ORIGINAL)
		/* client -> server (PNS -> PAC) */
		ret = pptp_outbound_pkt(tcph, pptph, datalen, ct, ctinfo);
	else
		/* server -> client (PAC -> PNS) */
		ret = pptp_inbound_pkt(tcph, pptph, datalen, ct, ctinfo, iph);
	DEBUGP("sstate: %d->%d, cstate: %d->%d\n",
		oldsstate, info->sstate, oldcstate, info->cstate);
	UNLOCK_BH(&ip_pptp_lock);

	return ret;
}

/* control protocol helper */
static struct ip_conntrack_helper pptp = { 
	.list = { NULL, NULL },
	.name = "pptp", 
	.flags = IP_CT_HELPER_F_REUSE_EXPECT,
	.me = THIS_MODULE,
	.max_expected = 2,
	.timeout = 0,
	.tuple = { .src = { .ip = 0, 
		 	    .u = { .tcp = { .port =  
				    __constant_htons(PPTP_CONTROL_PORT) } } 
			  }, 
		   .dst = { .ip = 0, 
			    .u = { .all = 0 },
			    .protonum = IPPROTO_TCP
			  } 
		 },
	.mask = { .src = { .ip = 0, 
			   .u = { .tcp = { .port = 0xffff } } 
			 }, 
		  .dst = { .ip = 0, 
			   .u = { .all = 0 },
			   .protonum = 0xffff 
		 	 } 
		},
	.help = conntrack_pptp_help
};

/* ip_conntrack_pptp initialization */
static int __init init(void)
{
	int retcode;

	DEBUGP(__FILE__ ": registering helper\n");
	if ((retcode = ip_conntrack_helper_register(&pptp))) {
                printk(KERN_ERR "Unable to register conntrack application "
				"helper for pptp: %d\n", retcode);
		return -EIO;
	}

	printk("ip_conntrack_pptp version %s loaded\n", IP_CT_PPTP_VERSION);
	return 0;
}

static void __exit fini(void)
{
	ip_conntrack_helper_unregister(&pptp);
	printk("ip_conntrack_pptp version %s unloaded\n", IP_CT_PPTP_VERSION);
}

module_init(init);
module_exit(fini);

EXPORT_SYMBOL(ip_pptp_lock);
