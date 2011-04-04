/*
 * flash.c
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
#ifdef IPOQUE_PROTOCOL_FLASH


static void ipoque_int_flash_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_FLASH;
	packet->detected_protocol = IPOQUE_PROTOCOL_FLASH;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_FLASH);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_FLASH);
	}
}

void ipoque_search_flash(struct ipoque_detection_module_struct *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
//      struct ipoque_id_struct         *src=ipoque_struct->src;
//      struct ipoque_id_struct         *dst=ipoque_struct->dst;

	if (1 /*(packet->tcp->dest == htons(1935) || packet->tcp->dest == htons(80) || packet->tcp->dest == htons(443)) */ ) {
		if (flow->flash_stage == 0 && packet->payload_packet_len > 0
			&& (packet->payload[0] == 0x03 || packet->payload[0] == 0x06)) {
			flow->flash_bytes = packet->payload_packet_len;
			if (packet->tcp->psh == 0) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH pass 1: \n");
				flow->flash_stage = packet->packet_direction + 1;

				IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
						"FLASH pass 1: flash_stage: %u, flash_bytes: %u\n", flow->flash_stage, flow->flash_bytes);
				return;
			} else if (packet->tcp->psh != 0 && flow->flash_bytes == 1537) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
						"FLASH hit: flash_stage: %u, flash_bytes: %u\n", flow->flash_stage, flow->flash_bytes);
				ipoque_int_flash_add_connection(ipoque_struct);
				return;
			}
		} else if (flow->flash_stage == 1 + packet->packet_direction) {
			flow->flash_bytes += packet->payload_packet_len;
			if (packet->tcp->psh != 0 && flow->flash_bytes == 1537) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
						"FLASH hit: flash_stage: %u, flash_bytes: %u\n", flow->flash_stage, flow->flash_bytes);
				ipoque_int_flash_add_connection(ipoque_struct);
				return;
			} else if (packet->tcp->psh == 0 && flow->flash_bytes < 1537) {
				IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
						"FLASH pass 2: flash_stage: %u, flash_bytes: %u\n", flow->flash_stage, flow->flash_bytes);
				return;
			}
		}
	}

	if (packet->tcp->dest == htons(843)) {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "found port 843. \n");
		if (flow->flash_stage == 0 && packet->payload_packet_len >= 22
			&& ipq_mem_cmp(packet->payload, "<policy-file-request/>", 22) == 0) {
			IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "detected flash. \n");
			ipoque_int_flash_add_connection(ipoque_struct);
			return;
		}
	}

	IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG,
			"FLASH might be excluded: flash_stage: %u, flash_bytes: %u, packet_direction: %u\n",
			flow->flash_stage, flow->flash_bytes, packet->packet_direction);

#ifdef IPOQUE_PROTOCOL_HTTP
	if (IPOQUE_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_HTTP) != 0) {
#endif							/* IPOQUE_PROTOCOL_HTTP */
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH: exclude\n");
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_FLASH);
#ifdef IPOQUE_PROTOCOL_HTTP
	} else {
		IPQ_LOG(IPOQUE_PROTOCOL_FLASH, ipoque_struct, IPQ_LOG_DEBUG, "FLASH avoid early exclude from http\n");
	}
#endif							/* IPOQUE_PROTOCOL_HTTP */

}
#endif
