/*
 * SIP extension for IP connection tracking. 
 *
 * Copyright (C) 2004, CyberTAN Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. CYBERTAN
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/ctype.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <linux/netfilter_ipv4/lockhelp.h>
#include <linux/netfilter_ipv4/ip_conntrack_helper.h>
#include <linux/netfilter_ipv4/ip_conntrack_sip.h>
#include <linux/netfilter_ipv4/ip_conntrack_core.h>

DECLARE_LOCK(ip_sip_lock);
 
struct module *ip_conntrack_sip = THIS_MODULE;

#define MAX_PORTS 8
static int ports[MAX_PORTS];
static int ports_c;
#ifdef MODULE_PARM
MODULE_PARM(ports, "1-" __MODULE_STRING(MAX_PORTS) "i");
#endif

#if 0
  #define DEBUGP printk
#else
  #define DEBUGP(format, args...)
#endif

#define RTCP_SUPPORT

static int set_expected_rtp(struct ip_conntrack *ct, u_int32_t dstip,
		u_int16_t dstport, unsigned int conntype)
{
	struct ip_conntrack_expect expect, *exp = &expect;
	struct ip_ct_sip_expect *exp_sip_info = &exp->help.exp_sip_info;
	int ret = 0;

	memset(&expect, 0, sizeof(expect));
	LOCK_BH(&ip_sip_lock);

	DEBUGP("conntrack_sip: %s: [%s]: DST=%u.%u.%u.%u:%u\n", __FUNCTION__, 
			conntype == CONN_RTP ? "RTP" : "RTCP", NIPQUAD(dstip), dstport);

	/* exp_sip_info */
	exp_sip_info->port = dstport;
	exp_sip_info->type = conntype;
	exp_sip_info->nated = 0;

	/* new expectation */
	exp->tuple = ((struct ip_conntrack_tuple)
		{ { 0, { 0 } },
		  { dstip, { htons(dstport) }, IPPROTO_UDP }});
	exp->mask = ((struct ip_conntrack_tuple)
		{ { 0, { 0 } },
		  { 0xFFFFFFFF, { 0xFFFF }, 0xFFFF }});
	exp->expectfn = NULL;

	if ((ret=ip_conntrack_expect_related(ct, exp)) != 0) {
		DEBUGP("Can't add new expectation. \n");
	}

	UNLOCK_BH(&ip_sip_lock);

	return ret;
}

/*
static int unset_expected_rtp(struct ip_conntrack *ct, u_int32_t dstip, u_int16_t dstport)
{
	struct ip_conntrack_expect *exp;
	const struct ip_conntrack_tuple tuple = { { 0, { 0 } },
			{ dstip, { htons(dstport) }, IPPROTO_UDP }};

	LOCK_BH(&ip_sip_lock);

	exp = ip_conntrack_expect_find_get(&tuple);
	if (exp) {
		DEBUGP("Find the expectation %p, then delete it.\n", exp);
		ip_conntrack_unexpect_related(exp);
	}

	UNLOCK_BH(&ip_sip_lock);

	return 0;
}*/

/* return:
 *       0 : Not found
 *       1 : Found domain name
 *       2 : Found dotted quads
 */
int find_sdp_rtp_addr(const char *data, size_t dlen,
			unsigned int *numoff, unsigned int *numlen, u_int32_t *addr)
{
	char *st, *p = (char *)data;
	const char *limit = data + dlen;
	unsigned char p1, p2, p3, p4;

	while (p < limit) {
		/* find 'c=' line */
		if (strncmp(p, "\nc=",3) && strncmp(p, "\rc=",3)) {
			p++;
			continue;
		}
		p += 3;

		if (strncmp(p, "IN IP4 ",7))
			continue;
		p += 7;

		/* IP address */
		st = p;

		/* FQDNs or dotted quads */
		while (isalpha(*p) || isdigit(*p) || (*p=='.') || (*p=='-')) {
			p++;
			if (p == limit)
				return 0;
		}

		*numoff = st - data;
		*numlen = p - st;

		/* Convert the IP address */
		p1 = simple_strtoul(st, &st ,10);
		if (*st != '.')
			return 1;
		p2 = simple_strtoul(st+1, &st, 10);
		if (*st != '.')
			return 1;
		p3 = simple_strtoul(st+1, &st, 10);
		if (*st != '.')
			return 1;
		p4 = simple_strtoul(st+1, &st, 10);

		*addr = (p1<<24) | (p2<<16) | (p3<<8) | p4;

		return 2;
	}

	return 0;
}

u_int16_t find_sdp_audio_port(const char *data, size_t dlen,
			unsigned int *numoff, unsigned int *numlen)
{
	char *st, *p = (char *)data;
	const char *limit = data + dlen;
	u_int16_t port = 0;

	while (p < limit) {
		/* find 'm=' */
		if (strncmp(p, "\nm=", 3) && strncmp(p, "\rm=", 3)) {
			p++;
			continue;
		}
		p += 3;

		/* audio stream */
		if (strncmp(p ,"audio ",6))
			continue;
		p += 6;

		st = p;
		port = simple_strtoul(p, &p, 10);

		*numoff = st - data;
		*numlen = p - st;

		return port;
	}

	return 0;
}

static int help(const struct iphdr *iph, size_t len,
		struct ip_conntrack *ct,
		enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	unsigned int matchlen, matchoff;

	u_int32_t ipaddr=0;
	u_int16_t port = 0;

	int found = 0;
	struct udphdr *udph = (void *)iph + iph->ihl * 4;
	const char *data = (const char *)udph + 8;
	unsigned int udplen = len - iph->ihl * 4;
	unsigned int datalen = udplen - 8;
	struct ip_ct_sip_master *ct_sip_info = &ct->help.ct_sip_info;

	DEBUGP("\nconntrack_sip: help(): DIR=%d, conntrackinfo=%u\n", dir, ctinfo);
	DEBUGP("conntrack_sip: %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u\n", 
		        NIPQUAD(ct->tuplehash[dir].tuple.src.ip),
		        ntohs(ct->tuplehash[dir].tuple.src.u.udp.port),
		        NIPQUAD(ct->tuplehash[dir].tuple.dst.ip), 
		        ntohs(ct->tuplehash[dir].tuple.dst.u.udp.port) );

	/* Reset for a new incoming packet */
	ct_sip_info->mangled = 0;	

	/* keep the connection alive */
	ip_ct_refresh(ct, (SIP_EXPIRES * HZ));

	/* Don't need to set the expectation for upstream direction */
	if (dir == IP_CT_DIR_REPLY)
		return NF_ACCEPT;

	/* Need to set the expected connection for further incoming RTP stream */
	if (strncmp(data, "INVITE", 6) != 0 && strncmp(data, "SIP/2.0 200", 11) != 0) {
		DEBUGP("conntrack_sip: Not interesting packet.\n");
		return NF_ACCEPT;
	}

	/* Find RTP address */
	found = find_sdp_rtp_addr(data, datalen, &matchoff, &matchlen, &ipaddr);
	if (!found)
		return NF_ACCEPT;

	DEBUGP("conntrack_sip: 'IN IP4' is %s.\n", (found == 1) ? "FQDNs" : "dotted quads");

	/* If it's a null address, then the call is on hold */
	if (found == 2 && ipaddr == 0) {
		DEBUGP("conntrack_sip: Null address is found.\n");
		return NF_ACCEPT;
	}

	/* Find audio port, and we don't like the well-known ports,
	 * which is less than 1024 */
	port = find_sdp_audio_port(data, datalen, &matchoff, &matchlen);
	if (port < 1024)
		return NF_ACCEPT;

	DEBUGP("conntrack_sip: audio port=%d.\n", port);

	ipaddr = ct->tuplehash[dir].tuple.src.ip; 
	ct_sip_info->rtpport = port;

	/* RFC1889 - RTP uses an even port number and the corresponding RTCP
	 * stream uses the next higher (odd) port number. */
	if (set_expected_rtp(ct, ipaddr, port, CONN_RTP) == 0) {
#ifdef RTCP_SUPPORT
		set_expected_rtp(ct, ipaddr, port + 1, CONN_RTCP);
#endif
	}

	return NF_ACCEPT;
}

static struct ip_conntrack_helper sip[MAX_PORTS];


/* Not __exit: called from init() */
static void fini(void)
{
	int i;
	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		DEBUGP("ip_ct_sip: unregistering helper for port %d\n",
				ports[i]);
		ip_conntrack_helper_unregister(&sip[i]);
	}
}

static int __init init(void)
{
	int i, ret;

	if (ports[0] == 0)
		ports[0] = SIP_PORT;

	for (i = 0; (i < MAX_PORTS) && ports[i]; i++) {
		memset(&sip[i], 0, sizeof(struct ip_conntrack_helper));
		sip[i].tuple.dst.u.udp.port = htons(ports[i]);
		sip[i].tuple.dst.protonum = IPPROTO_UDP;
		sip[i].mask.dst.u.udp.port = 0xF0FF;
		sip[i].mask.dst.protonum = 0xFFFF;
		sip[i].help = help;
		sip[i].timeout = RTP_TIMEOUT;
		DEBUGP("ip_ct_sip: registering helper for port %d\n", 
				ports[i]);
		ret = ip_conntrack_helper_register(&sip[i]);

		if (ret) {
			fini();
			return ret;
		}
		ports_c++;
	}
	return 0;
}


EXPORT_SYMBOL(ip_sip_lock);
EXPORT_SYMBOL(ip_conntrack_sip);

module_init(init);
module_exit(fini);
