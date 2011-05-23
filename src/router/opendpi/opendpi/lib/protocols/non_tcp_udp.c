/*
 * non_tcp_udp.c
 * Copyright (C) 2009-2010 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ipq_protocols.h"

#if defined(IPOQUE_PROTOCOL_IPSEC) || defined(IPOQUE_PROTOCOL_GRE) || defined(IPOQUE_PROTOCOL_ICMP)  || defined(IPOQUE_PROTOCOL_IGMP) || defined(IPOQUE_PROTOCOL_EGP) || defined(IPOQUE_PROTOCOL_SCTP) || defined(IPOQUE_PROTOCOL_OSPF) || defined(IPOQUE_PROTOCOL_IP_IN_IP)


#define IPQ_IPSEC_PROTOCOL_ESP	50
#define IPQ_IPSEC_PROTOCOL_AH	51

#define IPQ_GRE_PROTOCOL_TYPE	0x2F

#define IPQ_ICMP_PROTOCOL_TYPE	0x01
#define IPQ_IGMP_PROTOCOL_TYPE	0x02

#define IPQ_EGP_PROTOCOL_TYPE	0x08

#define IPQ_OSPF_PROTOCOL_TYPE	0x59

#define IPQ_SCTP_PROTOCOL_TYPE	132

#define IPQ_IPIP_PROTOCOL_TYPE  0x04

#define set_protocol_and_bmask(nprot)	\
do {													\
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(ipoque_struct->sd->detection_bitmask,nprot) != 0)		\
		ipq_connection_detected(ipoque_struct, nprot);						\
} while(0)


static void ipoque_search_in_non_tcp_udp(struct ipoque_detection_module_struct
								  *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	const struct iphdr *decaps_iph = ipoque_struct->packet.iph;

	if (packet->iph == NULL) {
		return;
	}
	switch (decaps_iph->protocol) {
#ifdef IPOQUE_PROTOCOL_IPSEC
	case IPQ_IPSEC_PROTOCOL_ESP:
	case IPQ_IPSEC_PROTOCOL_AH:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_IPSEC);
		break;
#endif							/* IPOQUE_PROTOCOL_IPSEC */
#ifdef IPOQUE_PROTOCOL_GRE
	case IPQ_GRE_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_GRE);
		break;
#endif							/* IPOQUE_PROTOCOL_GRE */
#ifdef IPOQUE_PROTOCOL_ICMP
	case IPQ_ICMP_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_ICMP);
		break;
#endif							/* IPOQUE_PROTOCOL_ICMP */
#ifdef IPOQUE_PROTOCOL_IGMP
	case IPQ_IGMP_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_IGMP);
		break;
#endif							/* IPOQUE_PROTOCOL_IGMP */
#ifdef IPOQUE_PROTOCOL_EGP
	case IPQ_EGP_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_EGP);
		break;
#endif							/* IPOQUE_PROTOCOL_EGP */
#ifdef IPOQUE_PROTOCOL_SCTP
	case IPQ_SCTP_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_SCTP);
		break;
#endif							/* IPOQUE_PROTOCOL_SCTP */
#ifdef IPOQUE_PROTOCOL_OSPF
	case IPQ_OSPF_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_OSPF);
		break;
#endif							/* IPOQUE_PROTOCOL_OSPF */
#ifdef IPOQUE_PROTOCOL_IP_IN_IP
	case IPQ_IPIP_PROTOCOL_TYPE:
		set_protocol_and_bmask(IPOQUE_PROTOCOL_IP_IN_IP);
		break;
#endif							/* IPOQUE_PROTOCOL_IP_IN_IP */
	}
}

#endif
