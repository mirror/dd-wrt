/*
 * stun.c
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
#ifdef IPOQUE_PROTOCOL_STUN


static void ipoque_int_stun_add_connection(struct ipoque_detection_module_struct *ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_STUN;
	packet->detected_protocol = IPOQUE_PROTOCOL_STUN;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_STUN);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_STUN);
	}
}



void ipoque_search_stun_udp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//  struct ipoque_id_struct *src = ipoque_struct->src;
//  struct ipoque_id_struct *dst = ipoque_struct->dst;


	u16 a;

	IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "search stun.\n");

	/*
	 * token list of message types and attribute types from
	 * http://wwwbs1.informatik.htw-dresden.de/svortrag/i02/Schoene/stun/stun.html
	 * the same list you can find in
	 * https://summersoft.fay.ar.us/repos/ethereal/branches/redhat-9/ethereal-0.10.3-1/ethereal-0.10.3/packet-stun.c
	 * token further message types and attributes from
	 * http://www.freeswitch.org/docs/group__stun1.html
	 * added further attributes observed
	 * message types: 0x0001, 0x0101, 0x0111, 0x0002, 0x0102, 0x0112, 0x0003, 0x0103, 0x0004, 0x0104, 0x0114, 0x0115
	 * attribute types: 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009,
	 * 0x000a, 0x000b, 0c000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0020,
	 * 0x0022, 0x0024, 0x8001, 0x8006, 0x8008, 0x8015, 0x8020, 0x8028, 0x802a, 0x8029, 0x8050, 0x8054, 0x8055
	 */

	if (packet->payload_packet_len >= 20
		&& ntohs(get_u16(packet->payload, 2)) + 20 ==
		packet->payload_packet_len
		&& ((packet->payload[0] == 0x00 && (packet->payload[1] >= 0x01 && packet->payload[1] <= 0x04))
			|| (packet->payload[0] == 0x01 && ((packet->payload[1] >= 0x01 && packet->payload[1] <= 0x04)
											   || (packet->payload[1] >= 0x11 && packet->payload[1] <= 0x15))))) {

		IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "len and type match.\n");

		if (packet->payload_packet_len == 20) {
			IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found stun.\n");
			ipoque_int_stun_add_connection(ipoque_struct);
			return;
		}

		a = 20;

		while (a < packet->payload_packet_len) {

			if (packet->payload_packet_len >= a + 4
				&& ((packet->payload[a] == 0x00 && ((packet->payload[a + 1] >= 0x01 && packet->payload[a + 1] <= 0x15)
													|| packet->payload[a + 1] == 0x20
													|| packet->payload[a + 1] == 0x22
													|| packet->payload[a + 1] == 0x24))
					|| (packet->payload[a] == 0x80
						&& (packet->payload[a + 1] == 0x01
							|| packet->payload[a + 1] == 0x06
							|| packet->payload[a + 1] == 0x08
							|| packet->payload[a + 1] == 0x15
							|| packet->payload[a + 1] == 0x20
							|| packet->payload[a + 1] == 0x22
							|| packet->payload[a + 1] == 0x28
							|| packet->payload[a + 1] == 0x2a
							|| packet->payload[a + 1] == 0x29
							|| packet->payload[a + 1] == 0x50
							|| packet->payload[a + 1] == 0x54 || packet->payload[a + 1] == 0x55)))) {

				IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "attribute match.\n");

				a += ((packet->payload[a + 2] << 8) + packet->payload[a + 3] + 4);

				if (a == packet->payload_packet_len) {
					IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found stun.\n");
					ipoque_int_stun_add_connection(ipoque_struct);
					return;
				}

			} else {
				break;
			}
		}

	}

	IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "exclude stun.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_STUN);
}

#endif
