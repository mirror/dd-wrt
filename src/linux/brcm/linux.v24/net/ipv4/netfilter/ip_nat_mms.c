/* MMS extension for TCP NAT alteration.
 * (C) 2002 by Filip Sneppe <filip.sneppe@cronos.be>
 * based on ip_nat_ftp.c and ip_nat_irc.c
 *
 * ip_nat_mms.c v0.3 2002-09-22
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *      Module load syntax:
 *      insmod ip_nat_mms.o ports=port1,port2,...port<MAX_PORTS>
 *
 *      Please give the ports of all MMS servers You wish to connect to.
 *      If you don't specify ports, the default will be TCP port 1755.
 *
 *      More info on MMS protocol, firewalls and NAT:
 *      http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnwmt/html/MMSFirewall.asp
 *      http://www.microsoft.com/windows/windowsmedia/serve/firewall.asp
 *
 *      The SDP project people are reverse-engineering MMS:
 *      http://get.to/sdp
 */


#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_nat.h>
#include <linux/netfilter_ipv4/ip_nat_helper.h>
#include <linux/netfilter_ipv4/ip_nat_rule.h>
#include <linux/netfilter_ipv4/ip_conntrack_mms.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>

#define DEBUGP(format, args...)
#define DUMP_BYTES(address, counter)

#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int ports_c = 0;

#ifdef MODULE_PARM
MODULE_PARM(ports, "1-" __MODULE_STRING(MAX_PORTS) "i");
#endif

MODULE_AUTHOR("Filip Sneppe <filip.sneppe@cronos.be>");
MODULE_DESCRIPTION("Microsoft Windows Media Services (MMS) NAT module");
MODULE_LICENSE("GPL");

DECLARE_LOCK_EXTERN(ip_mms_lock);


static int mms_data_fixup(const struct ip_ct_mms_expect *ct_mms_info,
                          struct ip_conntrack *ct,
                          struct sk_buff **pskb,
                          enum ip_conntrack_info ctinfo,
                          struct ip_conntrack_expect *expect)
{
	u_int32_t newip;
	struct ip_conntrack_tuple t;
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *) iph + iph->ihl * 4;
	char *data = (char *)tcph + tcph->doff * 4;
	int i, j, k, port;
	u_int16_t mms_proto;

	u_int32_t *mms_chunkLenLV    = (u_int32_t *)(data + MMS_SRV_CHUNKLENLV_OFFSET);
	u_int32_t *mms_chunkLenLM    = (u_int32_t *)(data + MMS_SRV_CHUNKLENLM_OFFSET);
	u_int32_t *mms_messageLength = (u_int32_t *)(data + MMS_SRV_MESSAGELENGTH_OFFSET);

	int zero_padding;

	char buffer[28];         /* "\\255.255.255.255\UDP\65635" * 2 (for unicode) */
	char unicode_buffer[75]; /* 27*2 (unicode) + 20 + 1 */
	char proto_string[6];
	
	MUST_BE_LOCKED(&ip_mms_lock);

	/* what was the protocol again ? */
	mms_proto = expect->tuple.dst.protonum;
	sprintf(proto_string, "%u", mms_proto);
	
	DEBUGP("ip_nat_mms: mms_data_fixup: info (seq %u + %u) in %u, proto %s\n",
	       expect->seq, ct_mms_info->len, ntohl(tcph->seq),
	       mms_proto == IPPROTO_UDP ? "UDP"
	       : mms_proto == IPPROTO_TCP ? "TCP":proto_string);
	
	newip = ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.ip;

	/* Alter conntrack's expectations. */
	t = expect->tuple;
	t.dst.ip = newip;
	for (port = ct_mms_info->port; port != 0; port++) {
		t.dst.u.tcp.port = htons(port);
		if (ip_conntrack_change_expect(expect, &t) == 0) {
			DEBUGP("ip_nat_mms: mms_data_fixup: using port %d\n", port);
			break;
		}
	}
	
	if(port == 0)
		return 0;

	sprintf(buffer, "\\\\%u.%u.%u.%u\\%s\\%u",
	        NIPQUAD(newip),
		expect->tuple.dst.protonum == IPPROTO_UDP ? "UDP"
		: expect->tuple.dst.protonum == IPPROTO_TCP ? "TCP":proto_string,
		port);
	DEBUGP("ip_nat_mms: new unicode string=%s\n", buffer);
	
	memset(unicode_buffer, 0, sizeof(char)*75);

	for (i=0; i<strlen(buffer); ++i)
		*(unicode_buffer+i*2)=*(buffer+i);
	
	DEBUGP("ip_nat_mms: mms_data_fixup: padding: %u len: %u\n", ct_mms_info->padding, ct_mms_info->len);
	DEBUGP("ip_nat_mms: mms_data_fixup: offset: %u\n", MMS_SRV_UNICODE_STRING_OFFSET+ct_mms_info->len);
	DUMP_BYTES(data+MMS_SRV_UNICODE_STRING_OFFSET, 60);
	
	/* add end of packet to it */
	for (j=0; j<ct_mms_info->padding; ++j) {
		DEBUGP("ip_nat_mms: mms_data_fixup: i=%u j=%u byte=%u\n", 
		       i, j, (u8)*(data+MMS_SRV_UNICODE_STRING_OFFSET+ct_mms_info->len+j));
		*(unicode_buffer+i*2+j) = *(data+MMS_SRV_UNICODE_STRING_OFFSET+ct_mms_info->len+j);
	}

	/* pad with zeroes at the end ? see explanation of weird math below */
	zero_padding = (8-(strlen(buffer)*2 + ct_mms_info->padding + 4)%8)%8;
	for (k=0; k<zero_padding; ++k)
		*(unicode_buffer+i*2+j+k)= (char)0;
	
	DEBUGP("ip_nat_mms: mms_data_fixup: zero_padding = %u\n", zero_padding);
	DEBUGP("ip_nat_mms: original=> chunkLenLV=%u chunkLenLM=%u messageLength=%u\n",
	       *mms_chunkLenLV, *mms_chunkLenLM, *mms_messageLength);
	
	/* explanation, before I forget what I did:
	   strlen(buffer)*2 + ct_mms_info->padding + 4 must be divisable by 8;
	   divide by 8 and add 3 to compute the mms_chunkLenLM field,
	   but note that things may have to be padded with zeroes to align by 8 
	   bytes, hence we add 7 and divide by 8 to get the correct length */ 
	*mms_chunkLenLM    = (u_int32_t) (3+(strlen(buffer)*2+ct_mms_info->padding+11)/8);
	*mms_chunkLenLV    = *mms_chunkLenLM+2;
	*mms_messageLength = *mms_chunkLenLV*8;
	
	DEBUGP("ip_nat_mms: modified=> chunkLenLV=%u chunkLenLM=%u messageLength=%u\n",
	       *mms_chunkLenLV, *mms_chunkLenLM, *mms_messageLength);
	
	ip_nat_mangle_tcp_packet(pskb, ct, ctinfo, 
	                         expect->seq - ntohl(tcph->seq),
	                         ct_mms_info->len + ct_mms_info->padding, unicode_buffer,
	                         strlen(buffer)*2 + ct_mms_info->padding + zero_padding);
	DUMP_BYTES(unicode_buffer, 60);
	
	return 1;
}

static unsigned int
mms_nat_expected(struct sk_buff **pskb,
                 unsigned int hooknum,
                 struct ip_conntrack *ct,
                 struct ip_nat_info *info)
{
	struct ip_nat_multi_range mr;
	u_int32_t newdstip, newsrcip, newip;

	struct ip_conntrack *master = master_ct(ct);

	IP_NF_ASSERT(info);
	IP_NF_ASSERT(master);

	IP_NF_ASSERT(!(info->initialized & (1 << HOOK2MANIP(hooknum))));

	DEBUGP("ip_nat_mms: mms_nat_expected: We have a connection!\n");

	newdstip = master->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
	newsrcip = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.ip;
	DEBUGP("ip_nat_mms: mms_nat_expected: hook %s: newsrc->newdst %u.%u.%u.%u->%u.%u.%u.%u\n",
	       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
	       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
	       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???",
	       NIPQUAD(newsrcip), NIPQUAD(newdstip));

	if (HOOK2MANIP(hooknum) == IP_NAT_MANIP_SRC)
		newip = newsrcip;
	else
		newip = newdstip;

	DEBUGP("ip_nat_mms: mms_nat_expected: IP to %u.%u.%u.%u\n", NIPQUAD(newip));

	mr.rangesize = 1;
	/* We don't want to manip the per-protocol, just the IPs. */
	mr.range[0].flags = IP_NAT_RANGE_MAP_IPS;
	mr.range[0].min_ip = mr.range[0].max_ip = newip;

	return ip_nat_setup_info(ct, &mr, hooknum);
}


static unsigned int mms_nat_help(struct ip_conntrack *ct,
			 struct ip_conntrack_expect *exp,
			 struct ip_nat_info *info,
			 enum ip_conntrack_info ctinfo,
			 unsigned int hooknum,
			 struct sk_buff **pskb)
{
	struct iphdr *iph = (*pskb)->nh.iph;
	struct tcphdr *tcph = (void *) iph + iph->ihl * 4;
	unsigned int datalen;
	int dir;
	struct ip_ct_mms_expect *ct_mms_info;

	if (!exp)
		DEBUGP("ip_nat_mms: no exp!!");

	ct_mms_info = &exp->help.exp_mms_info;
	
	/* Only mangle things once: original direction in POST_ROUTING
	   and reply direction on PRE_ROUTING. */
	dir = CTINFO2DIR(ctinfo);
	if (!((hooknum == NF_IP_POST_ROUTING && dir == IP_CT_DIR_ORIGINAL)
	    ||(hooknum == NF_IP_PRE_ROUTING && dir == IP_CT_DIR_REPLY))) {
		DEBUGP("ip_nat_mms: mms_nat_help: not touching dir %s at hook %s\n",
		       dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
		       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
		       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
		       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???");
		return NF_ACCEPT;
	}
	DEBUGP("ip_nat_mms: mms_nat_help: beyond not touching (dir %s at hook %s)\n",
	       dir == IP_CT_DIR_ORIGINAL ? "ORIG" : "REPLY",
	       hooknum == NF_IP_POST_ROUTING ? "POSTROUTING"
	       : hooknum == NF_IP_PRE_ROUTING ? "PREROUTING"
	       : hooknum == NF_IP_LOCAL_OUT ? "OUTPUT" : "???");
	
	datalen = (*pskb)->len - iph->ihl * 4 - tcph->doff * 4;
	
	DEBUGP("ip_nat_mms: mms_nat_help: %u+%u=%u %u %u\n", exp->seq, ct_mms_info->len,
	       exp->seq + ct_mms_info->len,
	       ntohl(tcph->seq),
	       ntohl(tcph->seq) + datalen);
	
	LOCK_BH(&ip_mms_lock);
	/* Check wether the whole IP/proto/port pattern is carried in the payload */
	if (between(exp->seq + ct_mms_info->len,
	    ntohl(tcph->seq),
	    ntohl(tcph->seq) + datalen)) {
		if (!mms_data_fixup(ct_mms_info, ct, pskb, ctinfo, exp)) {
			UNLOCK_BH(&ip_mms_lock);
			return NF_DROP;
		}
	} else {
		/* Half a match?  This means a partial retransmisison.
		   It's a cracker being funky. */
		if (net_ratelimit()) {
			printk("ip_nat_mms: partial packet %u/%u in %u/%u\n",
			       exp->seq, ct_mms_info->len,
			       ntohl(tcph->seq),
			       ntohl(tcph->seq) + datalen);
		}
		UNLOCK_BH(&ip_mms_lock);
		return NF_DROP;
	}
	UNLOCK_BH(&ip_mms_lock);
	
	return NF_ACCEPT;
}

static struct ip_nat_helper mms[MAX_PORTS];
static char mms_names[MAX_PORTS][10];

/* Not __exit: called from init() */
static void fini(void)
{
	int i;

	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		DEBUGP("ip_nat_mms: unregistering helper for port %d\n", ports[i]);
		ip_nat_helper_unregister(&mms[i]);
	}
}

static int __init init(void)
{
	int i, ret = 0;
	char *tmpname;

	if (ports[0] == 0)
		ports[0] = MMS_PORT;

	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {

		memset(&mms[i], 0, sizeof(struct ip_nat_helper));

		mms[i].tuple.dst.protonum = IPPROTO_TCP;
		mms[i].tuple.src.u.tcp.port = htons(ports[i]);
		mms[i].mask.dst.protonum = 0xFFFF;
		mms[i].mask.src.u.tcp.port = 0xFFFF;
		mms[i].help = mms_nat_help;
		mms[i].me = THIS_MODULE;
		mms[i].flags = 0;
		mms[i].expect = mms_nat_expected;

		tmpname = &mms_names[i][0];
		if (ports[i] == MMS_PORT)
			sprintf(tmpname, "mms");
		else
			sprintf(tmpname, "mms-%d", i);
		mms[i].name = tmpname;

		DEBUGP("ip_nat_mms: register helper for port %d\n",
				ports[i]);
		ret = ip_nat_helper_register(&mms[i]);

		if (ret) {
			printk("ip_nat_mms: error registering "
			       "helper for port %d\n", ports[i]);
			fini();
			return ret;
		}
		ports_c++;
	}

	return ret;
}

module_init(init);
module_exit(fini);
