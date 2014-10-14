/*-
 * Copyright (c) 2001, 2003, 2004 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: process.c,v 1.18 2004/04/17 22:31:37 vlm Exp $
 */
#define _BSD_SOURCE	/* for th_flags in tcp.h */
#include "rflow.h"
#include "rw.h"
#include "opt.h"
#include "storage.h"
#include "cfgvar.h"

#include <pcap-bpf.h>

/*
 * Get a pointer to the IPv4 header given the type of the input source,
 * start of the link-level packet and captured size.
 * IP header is guaranteed to be fully contained within the captured data,
 * except for options, which may be missing.
 */
static struct ip *
find_ipv4_level(packet_source_t *ps, char *packet, int caplen) {
	register struct ip *ipv4h = NULL;

	switch(ps->dlt) {
	case DLT_EN10MB:

		ipv4h = (struct ip *)(packet + ETHER_HDR_LEN);

		break;
	case DLT_PPP:
#ifdef	DLT_C_HDLC
	case DLT_C_HDLC:
#endif
		ipv4h = (struct ip *)(packet + 4);
		break;
	case DLT_RAW:	/* Some PPP implementations */
		/*
		 * Packets are always IP.
		 */
		ipv4h = (struct ip *)packet;
		break;
#ifdef	DLT_LOOP
	case DLT_LOOP:
#endif	/* DLT_LOOP */
	case DLT_NULL:
		ipv4h = (struct ip *)(packet + 4);
		break;
#ifdef	DLT_LINUX_SLL
	case DLT_LINUX_SLL:	/* Linux cooked socket */
		ipv4h = (struct ip *)(packet + 16);
		break;
#endif
	case DLT_IEEE802:	/* Token Ring */
		ipv4h = (struct ip *)(packet + 22);
		break;
	default:
		return NULL;
	}

	if(caplen - ((char *)ipv4h - (char *)packet)
		< (int)sizeof(struct ip)) {
		/* It is not complete IP header */
		return NULL;
	}

	if(ipv4h->ip_v != IPVERSION)
		return NULL;

	return ipv4h;
}

/*
 * Fill-in the ports of the flow structure, if it is a UDP or TCP packet.
 * Also find ICMP type and code.
 */
static void
find_ipv4_ports(struct ip *ipv4h, int caplen, flow_t *flow) {
	unsigned l4_offset;

	switch((flow->ip_p = ipv4h->ip_p)) {
	case IPPROTO_ICMP:
		l4_offset = ipv4h->ip_hl << 2;
		if(l4_offset >= sizeof(struct ip)
		&& (int)(l4_offset + 2) <= caplen) {
			u_int8_t *typecode_ptr;	/* two 1-byte values */
			typecode_ptr = (void *)((void *)ipv4h + l4_offset);
			flow->src_port = typecode_ptr[0];
			flow->dst_port = typecode_ptr[1];
			return;
		}
		break;
	case IPPROTO_UDP:
	case IPPROTO_TCP:
	case IPPROTO_SCTP:
		l4_offset = ipv4h->ip_hl << 2;
		if(l4_offset >= sizeof(struct ip)
		&& (int)(l4_offset + 4) <= caplen) {
			u_int8_t *ports_ptr;	/* two 2-byte values */
			ports_ptr = (void *)((void *)ipv4h + l4_offset);
			flow->src_port = (ports_ptr[0] << 8) | ports_ptr[1];
			flow->dst_port = (ports_ptr[2] << 8) | ports_ptr[3];
			return;
		}
		break;
	}

	flow->src_port = -1;
	flow->dst_port = -1;
}

/*
 * Fill-in the ports of the flow structure for NetFlow.
 */
static void
find_netflow_stuff(struct ip *ipv4h, int caplen, flow_t *flow) {
	unsigned l4_offset;

	/*
	 * The ToS is required to be an identifier of a flow uniqueness,
	 * according to Cisco. However, many have complained that this
	 * creates unanticipated duplicate flows. Although this behavior
	 * is strictly expected, you may comment out this line to disable
	 * differentiating between ToS-marked flows, if it bothers you.
	 */
	flow->ip_tos = ipv4h->ip_tos;

	switch((flow->ip_p = ipv4h->ip_p)) {
	case IPPROTO_ICMP:
		/* Do not capture ICMP T/C as ports for NetFlow */
		break;
	case IPPROTO_UDP:
	case IPPROTO_TCP:
	case IPPROTO_SCTP:
		l4_offset = ipv4h->ip_hl << 2;
		if(l4_offset >= sizeof(struct ip)
		&& (int)(l4_offset + 4) <= caplen) {
			u_int8_t *ports_ptr;	/* two 2-byte values */
			ports_ptr = (void *)((void *)ipv4h + l4_offset);
			flow->src_port = (ports_ptr[0] << 8) | ports_ptr[1];
			flow->dst_port = (ports_ptr[2] << 8) | ports_ptr[3];

			if(flow->ip_p == IPPROTO_TCP
			&& (int)(l4_offset + offsetof(struct tcphdr, th_flags))
				< caplen) {
				struct tcphdr *th = (void *)ipv4h + l4_offset;
				flow->tcp_flags = th->th_flags;
			}
			return;
		}
		break;
	}

	flow->src_port = -1;
	flow->dst_port = -1;
}

void
process_packet_data(packet_source_t *ps,
		const unsigned char *packet, int caplen) {
	flow_t flow;
	struct ip *ipv4h;
	int iplen;
	agg_e nf_agg = AGG_NONE;	/* What to aggregate in NetFlow */

	ipv4h = find_ipv4_level(ps, (char *)packet, caplen);
	if(!ipv4h)
		return;

	/*
	 * Fetch some IP-level properties from the found header.
	 */
	iplen = ntohs(ipv4h->ip_len);

	flow.src = ipv4h->ip_src;
	flow.dst = ipv4h->ip_dst;
	flow.packets = 1;	/* Single packet being added */
	flow.bytes = iplen;	/* Bytes in packet */
	flow.ip_tos = 0;	/* IP type of service */
	flow.tcp_flags = 0;	/* TCP flags */
	flow.ifSource = 0;	/* Interface */
	flow.ifName[0] = '\0';

	/*
	 * Find ports, if instructed to account for them.
	 */
	if((ps->iflags & IFLAG_RSH_EXTRA)) {
		flow.ifSource = ps;
		find_ipv4_ports(ipv4h,
			caplen - ((void *)ipv4h - (void *)packet),
			&flow);
	} else {
		flow.ip_p = 0;		/* IP protocol */
		flow.src_port = -1;
		flow.dst_port = -1;
		nf_agg = AGG_PORTS;
	}

	/*
	 * Enter critical section to update flow.
	 */

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

#if AFLOW
	/*
	 * This update goes into the main storage for RSH export.
	 */
	flow_update(&active_storage, &flow, AGG_ALL);
#endif
	/*
	 * This update goes into the NetFlow cache.
	 */
	if(!(ps->iflags & IFLAG_NF_DISABLE)) {
		if((ps->iflags & IFLAG_NF_SAMPLED)
		&& ((ps->sample_count++) % conf->netflow_packet_interval)) {
			/*
			 * NetFlow sampling-mode packet-interval feature.
			 */
		} else {
			find_netflow_stuff(ipv4h,
				caplen - ((void *)ipv4h - (void *)packet),
				&flow);
			flow.ifSource = ps;
			flow_update(&netflow_storage, &flow, nf_agg);
		}
	}

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	/*
	 * Exit critical section: flow updated or created.
	 */

	/* Perform statistics counting */
	ps->bytes_cur += iplen;
	ps->packets_cur += 1;
}

