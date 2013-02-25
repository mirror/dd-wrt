/*
 * non_tcp_udp.c
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-13 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "ndpi_protocols.h"

#if defined(NDPI_PROTOCOL_IPSEC) || defined(NDPI_PROTOCOL_GRE) || defined(NDPI_PROTOCOL_ICMP)  || defined(NDPI_PROTOCOL_IGMP) || defined(NDPI_PROTOCOL_EGP) || defined(NDPI_PROTOCOL_SCTP) || defined(NDPI_PROTOCOL_OSPF) || defined(NDPI_PROTOCOL_IP_IN_IP)


#define NDPI_IPSEC_PROTOCOL_ESP	50
#define NDPI_IPSEC_PROTOCOL_AH	51

#define NDPI_GRE_PROTOCOL_TYPE	0x2F

#define NDPI_ICMP_PROTOCOL_TYPE	0x01
#define NDPI_IGMP_PROTOCOL_TYPE	0x02

#define NDPI_EGP_PROTOCOL_TYPE	0x08

#define NDPI_OSPF_PROTOCOL_TYPE	0x59

#define NDPI_SCTP_PROTOCOL_TYPE	132

#define NDPI_IPIP_PROTOCOL_TYPE  0x04

#define NDPI_ICMPV6_PROTOCOL_TYPE  0x3a


#define set_protocol_and_bmask(nprot)	\
{													\
	if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask,nprot) != 0)		\
	{												\
	    ndpi_int_add_connection(ndpi_struct, flow,    \
                                  nprot,                \
							      NDPI_REAL_PROTOCOL); \
	}												\
}


static void ndpi_search_in_non_tcp_udp(struct ndpi_detection_module_struct
								  *ndpi_struct, struct ndpi_flow_struct *flow)
{
	struct ndpi_packet_struct *packet = &flow->packet;

	if (packet->iph == NULL) {
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		if (packet->iphv6 == NULL)
#endif
			return;
	}
	switch (packet->l4_protocol) {
#ifdef NDPI_PROTOCOL_IPSEC
	case NDPI_IPSEC_PROTOCOL_ESP:
	case NDPI_IPSEC_PROTOCOL_AH:
		set_protocol_and_bmask(NDPI_PROTOCOL_IPSEC);
		break;
#endif							/* NDPI_PROTOCOL_IPSEC */
#ifdef NDPI_PROTOCOL_GRE
	case NDPI_GRE_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_GRE);
		break;
#endif							/* NDPI_PROTOCOL_GRE */
#ifdef NDPI_PROTOCOL_ICMP
	case NDPI_ICMP_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_ICMP);
		break;
#endif							/* NDPI_PROTOCOL_ICMP */
#ifdef NDPI_PROTOCOL_IGMP
	case NDPI_IGMP_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_IGMP);
		break;
#endif							/* NDPI_PROTOCOL_IGMP */
#ifdef NDPI_PROTOCOL_EGP
	case NDPI_EGP_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_EGP);
		break;
#endif							/* NDPI_PROTOCOL_EGP */
#ifdef NDPI_PROTOCOL_SCTP
	case NDPI_SCTP_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_SCTP);
		break;
#endif							/* NDPI_PROTOCOL_SCTP */
#ifdef NDPI_PROTOCOL_OSPF
	case NDPI_OSPF_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_OSPF);
		break;
#endif							/* NDPI_PROTOCOL_OSPF */
#ifdef NDPI_PROTOCOL_IP_IN_IP
	case NDPI_IPIP_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_IP_IN_IP);
		break;
#endif							/* NDPI_PROTOCOL_IP_IN_IP */
#ifdef NDPI_PROTOCOL_ICMPV6
	case NDPI_ICMPV6_PROTOCOL_TYPE:
		set_protocol_and_bmask(NDPI_PROTOCOL_ICMPV6);
		break;
#endif							/* NDPI_PROTOCOL_ICMPV6 */
	}
}

#endif
