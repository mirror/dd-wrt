/*
 * edonkey.c
 * Copyright (C) 2009-2011 by ipoque GmbH
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
#ifdef IPOQUE_PROTOCOL_EDONKEY
/* debug defines */
#define IPOQUE_PROTOCOL_SAFE_DETECTION 		1

#define IPOQUE_PROTOCOL_PLAIN_DETECTION 	0
static void ipoque_add_connection_as_edonkey(struct ipoque_detection_module_struct
											 *ipoque_struct, const u8 save_detection, const u8 encrypted_connection)
{
	ipoque_int_change_protocol(ipoque_struct, IPOQUE_PROTOCOL_EDONKEY, IPOQUE_REAL_PROTOCOL);
}

#if !(defined(HAVE_NTOP) && defined(WIN32))
 static inline
#else
__forceinline static
#endif
	 u8 check_edk_len(const u8 * payload, u16 payload_packet_len)
{
	u32 edk_len_parsed = 0;
	// we use a do / while loop here, because we have checked the byte 0 for 0xe3 or 0xc5 already before this call
	do {
		u32 edk_len;
		edk_len = get_l32(payload, 1 + edk_len_parsed);

		/* if bigger, return here directly with an error... */
		if (edk_len > payload_packet_len)
			return 0;
		/* this is critical here:
		 * if (edk_len + 5) provokes an overflow to zero, we will have an infinite loop...
		 * the check above does prevent this, bcause the edk_len must be ((u32)-5), which is always bigger than the packet size
		 */
		edk_len_parsed += 5 + edk_len;

		if (edk_len_parsed == payload_packet_len)
			return 1;
		if (edk_len_parsed > payload_packet_len)
			return 0;
	}
	while (payload[edk_len_parsed] == 0xe3 || payload[edk_len_parsed] == 0xc5 || payload[edk_len_parsed] == 0xd4);
	return 0;
}

static void ipoque_int_edonkey_tcp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	int edk_stage2_len;

	/*len range increase if safe mode and also only once */
	if (ipoque_struct->edonkey_safe_mode == 0)
		edk_stage2_len = 140;
	else if (!flow->l4.tcp.edk_ext || packet->payload_packet_len == 212) {
		edk_stage2_len = 300;

	} else
		edk_stage2_len = 140;


	/* skip excluded connections */
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_EDONKEY) != 0)
		return;

	/* source and dst port must be 80 443 or > 1024 */
	if (ipoque_struct->edonkey_upper_ports_only != 0) {
		u16 port;
		port = ntohs(packet->tcp->source);
		/* source and dst port must be 80 443 or > 1024 */
		if (port < 1024 && port != 80 && port != 443)
			goto exclude_edk_tcp;

		port = ntohs(packet->tcp->dest);
		if (port < 1024 && port != 80 && port != 443)
			goto exclude_edk_tcp;
	}

	/* return here for empty packets, we needed them only for bt port detection */
	if (packet->payload_packet_len == 0)
		return;

	/* skip marked packets */
	if (flow->edk_stage == 0 && packet->detected_protocol_stack[0] != IPOQUE_PROTOCOL_UNKNOWN)
		return;

	/* first: check for unencrypted traffic */
	if (flow->edk_stage == 0) {
		/* check for client hello */
		if (packet->payload_packet_len >= 32 && get_l32(packet->payload, 1) <= (packet->payload_packet_len - 5)
			&& (packet->payload[0] == 0xe3 || packet->payload[0] == 0xc5)) {

			if (packet->payload[5] == 0x01 && ((packet->payload[6] == 0x10 && get_l32(packet->payload, 29) < 0x0F)
											   || (get_l32(packet->payload, 28) > 0x00
												   && get_l32(packet->payload, 28) < 0x0F))) {
				IPQ_LOG_EDONKEY(IPOQUE_PROTOCOL_EDONKEY, ipoque_struct, IPQ_LOG_DEBUG,
								"edk hello meta tag recognized\n");
				flow->edk_stage = 16 + packet->packet_direction;
				return;
			}
		}
	}
	if ((17 - packet->packet_direction) == flow->edk_stage) {
		if ((packet->payload_packet_len >= 32 && get_l32(packet->payload, 1) == 9 && (packet->payload[0] == 0xe3)
			 && packet->payload[5] == 0x40)
			|| (packet->payload_packet_len >= 32 && (packet->payload[0] == 0xe3)
				&& packet->payload[5] == 0x40 && check_edk_len(packet->payload, packet->payload_packet_len))
			|| (packet->payload_packet_len >= 32 && packet->payload[0] == 0xe3
				&& packet->payload[5] == 0x4c && (get_l32(packet->payload, 1) == (packet->payload_packet_len - 5)
												  || check_edk_len(packet->payload, packet->payload_packet_len)))
			|| (packet->payload_packet_len >= 32 && get_l32(packet->payload, 1) == (packet->payload_packet_len - 5)
				&& packet->payload[0] == 0xe3 && packet->payload[5] == 0x38)
			|| (packet->payload_packet_len >= 20 && get_l32(packet->payload, 1) == (packet->payload_packet_len - 5)
				&& packet->payload[0] == 0xc5 && packet->payload[5] == 0x92)
			|| (packet->payload_packet_len >= 20 && get_l32(packet->payload, 1) <= (packet->payload_packet_len - 5)
				&& packet->payload[0] == 0xe3 && packet->payload[5] == 0x58)
			|| (packet->payload_packet_len >= 20 && get_l32(packet->payload, 1) <= (packet->payload_packet_len - 5)
				&& (packet->payload[0] == 0xe3 || packet->payload[0] == 0xc5)
				&& packet->payload[5] == 0x01)) {
			IPQ_LOG_EDONKEY(IPOQUE_PROTOCOL_EDONKEY, ipoque_struct,
							IPQ_LOG_DEBUG, "edk 17: detected plain detection\n");
			ipoque_add_connection_as_edonkey(ipoque_struct,
											 IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION);
			return;
		}

		IPQ_LOG_EDONKEY(IPOQUE_PROTOCOL_EDONKEY, ipoque_struct, IPQ_LOG_DEBUG,
						"edk 17: id: %u, %u, %u not detected\n",
						packet->payload[0], get_l32(packet->payload, 1), packet->payload[5]);
	}
  exclude_edk_tcp:

	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_EDONKEY);

	return;
}


void ipoque_search_edonkey(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	if (packet->detected_protocol_stack[0] != IPOQUE_PROTOCOL_EDONKEY) {
		/* check for retransmission here */
		if (packet->tcp != NULL && packet->tcp_retransmission == 0)
			ipoque_int_edonkey_tcp(ipoque_struct);
	}
}
#endif
