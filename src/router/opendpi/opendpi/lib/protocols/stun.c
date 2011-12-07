/*
 * stun.c
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
#ifdef IPOQUE_PROTOCOL_STUN


static void ipoque_int_stun_add_connection(struct ipoque_detection_module_struct *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_STUN, IPOQUE_REAL_PROTOCOL);
}



typedef enum {
	IPOQUE_IS_STUN,
	IPOQUE_IS_NOT_STUN
} ipoque_int_stun_result_t;

static ipoque_int_stun_result_t ipoque_int_check_stun(struct ipoque_detection_module_struct *ipoque_struct,
													  const u8 * payload, const u16 payload_length)
{
	u16 a;

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
	 *
	 * 0x8003, 0x8004 used by facetime
	 */

	if (payload_length >= 20 && ntohs(get_u16(payload, 2)) + 20 == payload_length &&
		((payload[0] == 0x00 && (payload[1] >= 0x01 && payload[1] <= 0x04)) ||
		 (payload[0] == 0x01 &&
		  ((payload[1] >= 0x01 && payload[1] <= 0x04) || (payload[1] >= 0x11 && payload[1] <= 0x15))))) {
		u8 mod;
		u8 old = 1;
		u8 padding = 0;
		IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "len and type match.\n");

		if (payload_length == 20) {
			IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found stun.\n");
			return IPOQUE_IS_STUN;
		}

		a = 20;

		while (a < payload_length) {

			if (old && payload_length >= a + 4
				&&
				((payload[a] == 0x00
				  && ((payload[a + 1] >= 0x01 && payload[a + 1] <= 0x16) || payload[a + 1] == 0x19
					  || payload[a + 1] == 0x20 || payload[a + 1] == 0x22 || payload[a + 1] == 0x24
					  || payload[a + 1] == 0x25))
				 || (payload[a] == 0x80
					 && (payload[a + 1] == 0x01 || payload[a + 1] == 0x03 || payload[a + 1] == 0x04
						 || payload[a + 1] == 0x06 || payload[a + 1] == 0x08 || payload[a + 1] == 0x15
						 || payload[a + 1] == 0x20 || payload[a + 1] == 0x22 || payload[a + 1] == 0x28
						 || payload[a + 1] == 0x2a || payload[a + 1] == 0x29 || payload[a + 1] == 0x50
						 || payload[a + 1] == 0x54 || payload[a + 1] == 0x55)))) {

				IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "attribute match.\n");

				a += ((payload[a + 2] << 8) + payload[a + 3] + 4);
				mod = a % 4;
				if (mod) {
					padding = 4 - mod;
				}
				if (a == payload_length || (padding && (a + padding) == payload_length)) {
					IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found stun.\n");
					return IPOQUE_IS_STUN;
				}

			} else if (payload_length >= a + padding + 4
					   &&
					   ((payload[a + padding] == 0x00
						 && ((payload[a + 1 + padding] >= 0x01 && payload[a + 1 + padding] <= 0x16)
							 || payload[a + 1 + padding] == 0x19 || payload[a + 1 + padding] == 0x20
							 || payload[a + 1 + padding] == 0x22 || payload[a + 1 + padding] == 0x24
							 || payload[a + 1 + padding] == 0x25))
						|| (payload[a + padding] == 0x80
							&& (payload[a + 1 + padding] == 0x01 || payload[a + 1 + padding] == 0x03
								|| payload[a + 1 + padding] == 0x04 || payload[a + 1 + padding] == 0x06
								|| payload[a + 1 + padding] == 0x08 || payload[a + 1 + padding] == 0x15
								|| payload[a + 1 + padding] == 0x20 || payload[a + 1 + padding] == 0x22
								|| payload[a + 1 + padding] == 0x28 || payload[a + 1 + padding] == 0x2a
								|| payload[a + 1 + padding] == 0x29 || payload[a + 1 + padding] == 0x50
								|| payload[a + 1 + padding] == 0x54 || payload[a + 1 + padding] == 0x55)))) {

				IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "New STUN - attribute match.\n");

				old = 0;
				a += ((payload[a + 2 + padding] << 8) + payload[a + 3 + padding] + 4);
				padding = 0;
				mod = a % 4;
				if (mod) {
					a += 4 - mod;
				}
				if (a == payload_length) {
					IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found stun.\n");
					return IPOQUE_IS_STUN;
				}
			} else {
				break;
			}
		}
	}

	return IPOQUE_IS_NOT_STUN;
}

void ipoque_search_stun(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;

	IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "search stun.\n");


	if (packet->tcp) {

		/* STUN may be encapsulated in TCP packets */

		if (packet->payload_packet_len >= 2 + 20 &&
			ntohs(get_u16(packet->payload, 0)) + 2 == packet->payload_packet_len) {

			/* TODO there could be several STUN packets in a single TCP packet so maybe the detection could be
			 * improved by checking only the STUN packet of given length */

			if (ipoque_int_check_stun(ipoque_struct, packet->payload + 2, packet->payload_packet_len - 2) ==
				IPOQUE_IS_STUN) {
				IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found TCP stun.\n");
				ipoque_int_stun_add_connection(ipoque_struct);
				return;
			}
		}
	}
	if (ipoque_int_check_stun(ipoque_struct, packet->payload, packet->payload_packet_len) == IPOQUE_IS_STUN) {
		IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "found UDP stun.\n");
		ipoque_int_stun_add_connection(ipoque_struct);
		return;
	}

	IPQ_LOG(IPOQUE_PROTOCOL_STUN, ipoque_struct, IPQ_LOG_DEBUG, "exclude stun.\n");
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_STUN);
}

#endif
