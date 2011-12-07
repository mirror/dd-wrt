/*
 * snmp.c
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
#ifdef IPOQUE_PROTOCOL_SNMP

static void ipoque_int_snmp_add_connection(struct ipoque_detection_module_struct
										   *ipoque_struct)
{
	ipoque_int_add_connection(ipoque_struct, IPOQUE_PROTOCOL_SNMP, IPOQUE_REAL_PROTOCOL);
}

void ipoque_search_snmp(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (packet->payload_packet_len > 32 && packet->payload[0] == 0x30) {
		int offset;
		switch (packet->payload[1]) {
		case 0x81:
			offset = 3;
			break;
		case 0x82:
			offset = 4;
			break;
		default:
			if (packet->payload[1] > 0x82) {
				IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP excluded, second byte is > 0x82\n");
				goto excl;
			}
			offset = 2;
		}

		if (get_u16(packet->payload, offset) != htons(0x0201)) {
			IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP excluded, 0x0201 pattern not found\n");
			goto excl;
		}

		if (packet->payload[offset + 2] >= 0x04) {
			IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP excluded, version > 3\n");
			goto excl;
		}

		if (flow->l4.udp.snmp_stage == 0) {
			if (packet->udp->dest == htons(161) || packet->udp->dest == htons(162)) {
				IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP detected due to port.\n");
				ipoque_int_snmp_add_connection(ipoque_struct);
				return;
			}
			IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP stage 0.\n");
			if (packet->payload[offset + 2] == 3) {
				flow->l4.udp.snmp_msg_id = ntohs(get_u32(packet->payload, offset + 8));
			} else if (packet->payload[offset + 2] == 0) {
				flow->l4.udp.snmp_msg_id = get_u8(packet->payload, offset + 15);
			} else {
				flow->l4.udp.snmp_msg_id = ntohs(get_u16(packet->payload, offset + 15));
			}
			flow->l4.udp.snmp_stage = 1 + packet->packet_direction;
			return;
		} else if (flow->l4.udp.snmp_stage == 1 + packet->packet_direction) {
			if (packet->payload[offset + 2] == 0) {
				if (flow->l4.udp.snmp_msg_id != get_u8(packet->payload, offset + 15) - 1) {
					IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG,
							"SNMP v1 excluded, message ID doesn't match\n");
					goto excl;
				}
			}
		} else if (flow->l4.udp.snmp_stage == 2 - packet->packet_direction) {
			IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP stage 1-2.\n");
			if (packet->payload[offset + 2] == 3) {
				if (flow->l4.udp.snmp_msg_id != ntohs(get_u32(packet->payload, offset + 8))) {
					IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG,
							"SNMP v3 excluded, message ID doesn't match\n");
					goto excl;
				}
			} else if (packet->payload[offset + 2] == 0) {
				if (flow->l4.udp.snmp_msg_id != get_u8(packet->payload, offset + 15)) {
					IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG,
							"SNMP v1 excluded, message ID doesn't match\n");
					goto excl;
				}
			} else {
				if (flow->l4.udp.snmp_msg_id != ntohs(get_u16(packet->payload, offset + 15))) {
					IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG,
							"SNMP v2 excluded, message ID doesn't match\n");
					goto excl;
				}
			}
			IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP detected.\n");
			ipoque_int_snmp_add_connection(ipoque_struct);
			return;
		}
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_SNMP, ipoque_struct, IPQ_LOG_DEBUG, "SNMP excluded.\n");
	}
  excl:
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SNMP);

}

#endif
