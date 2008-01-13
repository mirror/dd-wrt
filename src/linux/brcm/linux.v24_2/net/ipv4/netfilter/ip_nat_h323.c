/* 
 * H.323 'brute force' extension for NAT alteration. 
 * Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * Based on ip_masq_h323.c for 2.2 kernels from CoRiTel, Sofia project.
 * (http://www.coritel.it/projects/sofia/nat.html)
 * Uses Sampsa Ranta's excellent idea on using expectfn to 'bind'
 * the unregistered helpers to the conntrack entries.
 */


#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_tuple.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_h323.h>

MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("H.323 'brute force' connection tracking module");
MODULE_LICENSE("GPL");

DECLARE_LOCK_EXTERN(ip_h323_lock);
struct module *ip_nat_h323 = THIS_MODULE;

#define DEBUGP(format, args...)


static unsigned int 
h225_nat_expected(struct sk_buff **pskb,
		  unsigned int hooknum,
		  struct ip_conntrack *ct,
		  struct ip_nat_info *info);

static unsigned int h225_nat_help(struct ip_conntrack *ct,
				  struct ip_conntrack_expect *exp,
			 	  struct ip_nat_info *info,
			 	  enum ip_conntrack_info ctinfo,
			 	  unsigned int hooknum,
			 	  struct sk_buff **pskb);
		  
static struct ip_nat_helper h245 = 
	{ { NULL, NULL },
          "H.245",				/* name */
	  0,					/* flags */
	  NULL,					/* module */
	  { { 0, { 0 } },			/* tuple */
	    { 0, { 0 }, IPPROTO_TCP } },
	  { { 0, { 0xFFFF } },			/* mask */
	    { 0, { 0 }, 0xFFFF } },
	  h225_nat_help,			/* helper */
	  h225_nat_expected			/* expectfn */
	};

static unsigned int
h225_nat_expected(struct sk_buff **pskb,
		  unsigned int hooknum,
		  struct ip_conntrack *ct,
		  struct ip_nat_info *info)
{
	struct ip_nat_multi_range mr;
	u_int32_t newdstip, newsrcip, newip;
	u_int16_t port;
	struct ip_ct_h225_expect *exp_info;
	struct ip_ct_h225_master *master_info;
	struct ip_conntrack *master = master_ct(ct);
	unsigned int is_h225, ret;
	
	IP_NF_ASSERT(info);
	IP_NF_ASSERT(master);

	IP_NF_ASSERT(!(info->initialized & (1<<HOOK2MANIP(hooknum))));

	DEBUGP("h225_nat_expected: We have a connection!\n");
	master_info = &ct->master->expectant->help.ct_h225_info;
	exp_info = &ct->master->help.exp_h225_info;

	LOCK_BH(&ip_h323_lock);

	DEBUGP("master: ");
	DUMP_TUPLE(&master->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	DUMP_TUPLE(&master->tuplehash[IP_CT_DIR_REPLY].tuple);
	DEBUGP("conntrack: ");
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	if (exp_info->dir == IP_CT_DIR_ORIGINAL) {
		/* Make connection go to the client. */
		newdstip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
		newsrcip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
		DEBUGP("h225_nat_expected: %u.%u.%u.%u->%u.%u.%u.%u (to client)\n",
		       NIPQUAD(newsrcip), NIPQUAD(newdstip));
	} else {
		/* Make the connection go to the server */
		newdstip = master->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
		newsrcip = master->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
		DEBUGP("h225_nat_expected: %u.%u.%u.%u->%u.%u.%u.%u (to server)\n",
		       NIPQUAD(newsrcip), NIPQUAD(newdstip));
	}
	port = exp_info->port;
	is_h225 = master_info->is_h225 == H225_PORT;
	UNLOCK_BH(&ip_h323_lock);
	
	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC)
		newip = newsrcip;
	else
		newip = newdstip;

	DEBUGP("h225_nat_expected: IP to %u.%u.%u.%u\n", NIPQUAD(newip));

	mr.rangesize = 1;
	/* We don't want to manip the per-protocol, just the IPs... */
	mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
	mr.range[0].min_ip = mr.range[0].max_ip = newip;

	/* ... unless we're doing a MANIP_DST, in which case, make
	   sure we map to the correct port */
	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_DST) {
		mr.range[0].flags |= IP_NAT_RANGE_PROTO_SPECIFIED;
		mr.range[0].min = mr.range[0].max
			= ((union ip_conntrack_manip_proto)
				{ port });
	}

	ret = ip_nat_setup_info(ct, &mr, hooknum);
	
	if (is_h225) {
		DEBUGP("h225_nat_expected: H.225, setting NAT helper for %p\n", ct);
		/* NAT expectfn called with ip_nat_lock write-locked */
		info->helper = &h245;
	}
	return ret;
}

static int h323_signal_address_fixup(struct ip_conntrack *ct,
			      	     struct sk_buff **pskb,
			      	     enum ip_conntrack_info ctinfo)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	unsigned char *data;
	u_int32_t tcplen = (*pskb)->len - iph->ihl*4;
	u_int32_t datalen = tcplen - tcph->doff*4;
	struct ip_ct_h225_master *info = &ct->help.ct_h225_info; 
	u_int32_t newip;
	u_int16_t port;
	u_int8_t buffer[6];
	int i;

	MUST_BE_LOCKED(&ip_h323_lock);

	DEBUGP("h323_signal_address_fixup: %s %s\n",
		between(info->seq[IP_CT_DIR_ORIGINAL], ntohl(tcph->seq), ntohl(tcph->seq) + datalen)
			? "yes" : "no",
		between(info->seq[IP_CT_DIR_REPLY], ntohl(tcph->seq), ntohl(tcph->seq) + datalen)
			? "yes" : "no");
	if (!(between(info->seq[IP_CT_DIR_ORIGINAL], ntohl(tcph->seq), ntohl(tcph->seq) + datalen)
	      || between(info->seq[IP_CT_DIR_REPLY], ntohl(tcph->seq), ntohl(tcph->seq) + datalen)))
		return 1;

	DEBUGP("h323_signal_address_fixup: offsets %u + 6  and %u + 6 in %u\n", 
		info->offset[IP_CT_DIR_ORIGINAL], 
		info->offset[IP_CT_DIR_REPLY],
		tcplen);
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	for (i = 0; i < IP_CT_DIR_MAX; i++) {
		DEBUGP("h323_signal_address_fixup: %s %s\n",
			info->dir == IP_CT_DIR_ORIGINAL ? "original" : "reply",
			i == IP_CT_DIR_ORIGINAL ? "caller" : "callee");
		if (!between(info->seq[i], ntohl(tcph->seq), 
			     ntohl(tcph->seq) + datalen))
			continue;
		if (!between(info->seq[i] + 6, ntohl(tcph->seq),
			     ntohl(tcph->seq) + datalen)) {
			/* Partial retransmisison. It's a cracker being funky. */
			if (net_ratelimit()) {
				printk("H.323_NAT: partial packet %u/6 in %u/%u\n",
				     info->seq[i],
				     ntohl(tcph->seq),
				     ntohl(tcph->seq) + datalen);
			}
			return 0;
		}

		/* Change address inside packet to match way we're mapping
		   this connection. */
		if (i == IP_CT_DIR_ORIGINAL) {
			newip = ct->tuplehash[!info->dir].tuple.dst.ip;
			port = ct->tuplehash[!info->dir].tuple.dst.u.tcp.port;
		} else {
			newip = ct->tuplehash[!info->dir].tuple.src.ip;
			port = ct->tuplehash[!info->dir].tuple.src.u.tcp.port;
		}

		data = (char *) tcph + tcph->doff * 4 + info->offset[i];

		DEBUGP("h323_signal_address_fixup: orig %s IP:port %u.%u.%u.%u:%u\n", 
			i == IP_CT_DIR_ORIGINAL ? "source" : "dest  ", 
		        data[0], data[1], data[2], data[3],
		        (data[4] << 8 | data[5]));

		/* Modify the packet */
		memcpy(buffer, &newip, 4);
		memcpy(buffer + 4, &port, 2);
		if (!ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, info->offset[i],
					      6, buffer, 6))
			return 0;

		DEBUGP("h323_signal_address_fixup:  new %s IP:port %u.%u.%u.%u:%u\n", 
			i == IP_CT_DIR_ORIGINAL ? "source" : "dest  ", 
		        data[0], data[1], data[2], data[3],
		        (data[4] << 8 | data[5]));
	}

	return 1;
}

static int h323_data_fixup(struct ip_ct_h225_expect *info,
			   struct ip_conntrack *ct,
			   struct sk_buff **pskb,
			   enum ip_conntrack_info ctinfo,
			   struct ip_conntrack_expect *expect)
{
	u_int32_t newip;
	u_int16_t port;
	u_int8_t buffer[6];
	struct ip_conntrack_tuple newtuple;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *)iph + iph->ihl*4;
	unsigned char *data;
	u_int32_t tcplen = (*pskb)->len - iph->ihl*4;
	struct ip_ct_h225_master *master_info = &ct->help.ct_h225_info;
	int is_h225;

	MUST_BE_LOCKED(&ip_h323_lock);
	DEBUGP("h323_data_fixup: offset %u + 6 in %u\n", info->offset, tcplen);
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple);
	DUMP_TUPLE(&ct->tuplehash[IP_CT_DIR_REPLY].tuple);

	if (!between(expect->seq + 6, ntohl(tcph->seq),
		    ntohl(tcph->seq) + tcplen - tcph->doff * 4)) {
		/* Partial retransmisison. It's a cracker being funky. */
		if (net_ratelimit()) {
			printk("H.323_NAT: partial packet %u/6 in %u/%u\n",
			     expect->seq,
			     ntohl(tcph->seq),
			     ntohl(tcph->seq) + tcplen - tcph->doff * 4);
		}
		return 0;
	}

	/* Change address inside packet to match way we're mapping
	   this connection. */
	if (info->dir == IP_CT_DIR_REPLY) {
		/* Must be where client thinks server is */
		newip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
		/* Expect something from client->server */
		newtuple.src.ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
		newtuple.dst.ip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.ip;
	} else {
		/* Must be where server thinks client is */
		newip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
		/* Expect something from server->client */
		newtuple.src.ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.ip;
		newtuple.dst.ip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;
	}

	is_h225 = (master_info->is_h225 == H225_PORT);

	if (is_h225) {
		newtuple.dst.protonum = IPPROTO_TCP;
		newtuple.src.u.tcp.port = expect->tuple.src.u.tcp.port;
	} else {
		newtuple.dst.protonum = IPPROTO_UDP;
		newtuple.src.u.udp.port = expect->tuple.src.u.udp.port;
	}
	
	/* Try to get same port: if not, try to change it. */
	for (port = ntohs(info->port); port != 0; port++) {
		if (is_h225)
			newtuple.dst.u.tcp.port = htons(port);
		else
			newtuple.dst.u.udp.port = htons(port);

		if (ip_conntrack_change_expect(expect, &newtuple) == 0)
			break;
	}
	if (port == 0) {
		DEBUGP("h323_data_fixup: no free port found!\n");
		return 0;
	}

	port = htons(port);

	data = (char *) tcph + tcph->doff * 4 + info->offset;

	DEBUGP("h323_data_fixup: orig IP:port %u.%u.%u.%u:%u\n", 
	        data[0], data[1], data[2], data[3],
	        (data[4] << 8 | data[5]));

	/* Modify the packet */
	memcpy(buffer, &newip, 4);
	memcpy(buffer + 4, &port, 2);
	if (!ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, info->offset,
				      6, buffer, 6))
		return 0;
	
	DEBUGP("h323_data_fixup: new IP:port %u.%u.%u.%u:%u\n", 
	        data[0], data[1], data[2], data[3],
	        (data[4] << 8 | data[5]));

	return 1;
}

static unsigned int h225_nat_help(struct ip_conntrack *ct,
				  struct ip_conntrack_expect *exp,
			 	  struct ip_nat_info *info,
			 	  enum ip_conntrack_info ctinfo,
			 	  unsigned int hooknum,
			 	  struct sk_buff **pskb)
{
	int dir;
	struct ip_ct_h225_expect *exp_info;
	
	/* Only mangle things once: original direction in POST_ROUTING
	   and reply direction on PRE_ROUTING. */
	dir = CTINFO2DIR(ctinfo);
	DEBUGP("nat_h323: dir %s at hook %s\n",
	       dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
	       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
	       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
	       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???");
	if (!((hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
	      || (hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY))) {
		DEBUGP("nat_h323: Not touching dir %s at hook %s\n",
		       dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
		       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
		       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
		       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???");
		return NF_ACCEPT;
	}

	if (!exp) {
		LOCK_BH(&ip_h323_lock);
		if (!h323_signal_address_fixup(ct, pskb, ctinfo)) {
			UNLOCK_BH(&ip_h323_lock);
			return NF_DROP;
		}
		UNLOCK_BH(&ip_h323_lock);
		return NF_ACCEPT;
	}
		
	exp_info = &exp->help.exp_h225_info;

	LOCK_BH(&ip_h323_lock);
	if (!h323_data_fixup(exp_info, ct, pskb, ctinfo, exp)) {
		UNLOCK_BH(&ip_h323_lock);
		return NF_DROP;
	}
	UNLOCK_BH(&ip_h323_lock);

	return NF_ACCEPT;
}

static struct ip_nat_helper h225 = 
	{ { NULL, NULL },
	  "H.225",					/* name */
	  IP_NAT_HELPER_F_ALWAYS, 			/* flags */
	  THIS_MODULE,					/* module */
	  { { 0, { __constant_htons(H225_PORT) } },	/* tuple */
	    { 0, { 0 }, IPPROTO_TCP } },
	  { { 0, { 0xFFFF } },				/* mask */
	    { 0, { 0 }, 0xFFFF } },
	  h225_nat_help,				/* helper */
	  h225_nat_expected				/* expectfn */
	};

static int __init init(void)
{
	int ret;
	
	ret = ip_nat_helper_register(&h225);

	if (ret != 0)
		printk("ip_nat_h323: cannot initialize the module!\n");

	return ret;
}

static void __exit fini(void)
{
	ip_nat_helper_unregister(&h225);
}

module_init(init);
module_exit(fini);
