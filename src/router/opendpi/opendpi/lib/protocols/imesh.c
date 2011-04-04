/*
 * imesh.c
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

#ifdef IPOQUE_PROTOCOL_IMESH


static void ipoque_int_imesh_add_connection(struct ipoque_detection_module_struct
											*ipoque_struct)
{

	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;

	flow->detected_protocol = IPOQUE_PROTOCOL_IMESH;
	packet->detected_protocol = IPOQUE_PROTOCOL_IMESH;

	if (src != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, IPOQUE_PROTOCOL_IMESH);
	}
	if (dst != NULL) {
		IPOQUE_ADD_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, IPOQUE_PROTOCOL_IMESH);
	}
}


void ipoque_search_imesh_tcp_udp(struct ipoque_detection_module_struct
								 *ipoque_struct)
{
	struct ipoque_packet_struct *packet = &ipoque_struct->packet;
	struct ipoque_flow_struct *flow = ipoque_struct->flow;
	struct ipoque_id_struct *src = ipoque_struct->src;
	struct ipoque_id_struct *dst = ipoque_struct->dst;


	if (packet->detected_protocol == IPOQUE_PROTOCOL_IMESH) {
		if (src != NULL) {
			src->imesh_timer = packet->tick_timestamp;
		}
		if (dst != NULL) {
			dst->imesh_timer = packet->tick_timestamp;
		}
		return;
	}

	/* skip marked packets */
	if (packet->detected_protocol != IPOQUE_PROTOCOL_UNKNOWN)
		goto imesh_not_found_end;

	if (packet->udp != NULL) {

		IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG, "UDP FOUND\n");

		// this is the login packet
		if (					//&& ((IPOQUE_TIMESTAMP_COUNTER_SIZE)(packet->tick_timestamp - src->imesh_timer)) < ipoque_struct->imesh_connection_timeout
			   packet->payload_packet_len == 28 && (get_l32(packet->payload, 0)) == 0x00000002	// PATTERN : 02 00 00 00
			   && (get_l32(packet->payload, 24)) == 0x00000000	// PATTERN : 00 00 00 00
			   && (packet->udp->dest == htons(1864) || packet->udp->source == htons(1864))) {
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG, "iMesh Login detected\n");
			if (src != NULL) {
				src->imesh_timer = packet->tick_timestamp;
			}
			if (dst != NULL) {
				dst->imesh_timer = packet->tick_timestamp;
			}
			ipoque_int_imesh_add_connection(ipoque_struct);
			return;
		} else if (				//&& ((IPOQUE_TIMESTAMP_COUNTER_SIZE)(packet->tick_timestamp - src->imesh_timer)) < ipoque_struct->imesh_connection_timeout
					  packet->payload_packet_len == 36 && (get_l32(packet->payload, 0)) == 0x00000002	// PATTERN : 02 00 00 00
					  //&& packet->payload[35]==0x0f
			) {
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG, "iMesh detected, %u\n",
					ipoque_struct->imesh_connection_timeout);
			if (src != NULL) {
				IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG, "iMesh: src < %u, %u, %u\n",
						(packet->tick_timestamp - src->imesh_timer), packet->tick_timestamp, src->imesh_timer);
				if (((IPOQUE_TIMESTAMP_COUNTER_SIZE)
					 (packet->tick_timestamp - src->imesh_timer)) < ipoque_struct->imesh_connection_timeout) {
					src->imesh_timer = packet->tick_timestamp;
					ipoque_int_imesh_add_connection(ipoque_struct);
				}
			}
			if (dst != NULL) {
				IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG, "iMesh: dst < %u, %u, %u\n",
						(packet->tick_timestamp - dst->imesh_timer), packet->tick_timestamp, dst->imesh_timer);
				if (((IPOQUE_TIMESTAMP_COUNTER_SIZE)
					 (packet->tick_timestamp - dst->imesh_timer)) < ipoque_struct->imesh_connection_timeout) {
					dst->imesh_timer = packet->tick_timestamp;
					ipoque_int_imesh_add_connection(ipoque_struct);
				}
			}
			return;
		}

		IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
				"iMesh UDP packetlen: %d\n", packet->payload_packet_len);

	} else if (packet->tcp != NULL) {

		IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
				"TCP FOUND :: Payload %u\n", packet->payload_packet_len);

		if (packet->actual_payload_len == 0) {
			return;

		} else if ((packet->actual_payload_len == 8 || packet->payload_packet_len == 10)	/* PATTERN:: 04 00 00 00 00 00 00 00 [00 00] */
				   &&packet->payload[0] == 0x04
				   && packet->payload[1] == 0x00
				   && packet->payload[2] == 0x00
				   && packet->payload[3] == 0x00
				   && packet->payload[4] == 0x00
				   && packet->payload[5] == 0x00 && packet->payload[6] == 0x00 && packet->payload[7] == 0x00) {
			flow->imesh_stage += 2;

			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		} else if (packet->actual_payload_len == 10	/* PATTERN:: ?? ?? 04|00 00 64|00 00 */
				   && (packet->payload[2] == 0x04 || packet->payload[2] == 0x00)
				   && packet->payload[3] == 0x00 && (packet->payload[4] == 0x00 || packet->payload[4] == 0x64)
				   && packet->payload[5] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		} else if (packet->actual_payload_len == 2 && packet->payload[0] == 0x06 && packet->payload[1] == 0x00) {
			flow->imesh_stage++;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		} else if (packet->actual_payload_len == 10	/* PATTERN:: 06 00 04|00 00 01|00 00 01|00 00 ?? 00 */
				   && packet->payload[0] == 0x06
				   && packet->payload[1] == 0x00 && (packet->payload[2] == 0x04 || packet->payload[2] == 0x00)
				   && packet->payload[3] == 0x00 && (packet->payload[4] == 0x00 || packet->payload[4] == 0x01)
				   && packet->payload[5] == 0x00 && (packet->payload[6] == 0x01 || packet->payload[6] == 0x00)
				   && packet->payload[7] == 0x00 && packet->payload[9] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);
		}

		else if (packet->actual_payload_len == 24 && packet->payload[0] == 0x06	// PATTERN :: 06 00 12 00 00 00 34 00 00
				 && packet->payload[1] == 0x00
				 && packet->payload[2] == 0x12
				 && packet->payload[3] == 0x00
				 && packet->payload[4] == 0x00
				 && packet->payload[5] == 0x00
				 && packet->payload[6] == 0x34 && packet->payload[7] == 0x00 && packet->payload[8] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);
		}

		else if (packet->actual_payload_len == 8	/* PATTERN:: 06|00 00 02 00 00 00 33 00 */
				 && (packet->payload[0] == 0x06 || packet->payload[0] == 0x00)
				 && packet->payload[1] == 0x00
				 && packet->payload[2] == 0x02
				 && packet->payload[3] == 0x00
				 && packet->payload[4] == 0x00
				 && packet->payload[5] == 0x00 && packet->payload[6] == 0x33 && packet->payload[7] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);
		}

		else if (packet->payload_packet_len == 6	/* PATTERN:: 02 00 00 00 33 00 */
				 && packet->payload[0] == 0x02
				 && packet->payload[1] == 0x00
				 && packet->payload[2] == 0x00
				 && packet->payload[3] == 0x00 && packet->payload[4] == 0x33 && packet->payload[5] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);
		}

		else if (packet->actual_payload_len == 12 && packet->payload[0] == 0x06	// PATTERN : 06 00 06 00 00 00 64 00
				 && packet->payload[1] == 0x00
				 && packet->payload[2] == 0x06
				 && packet->payload[3] == 0x00
				 && packet->payload[4] == 0x00
				 && packet->payload[5] == 0x00 && packet->payload[6] == 0x64 && packet->payload[7] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);
		}

		else if (packet->actual_payload_len == 10	/* PATTERN:: 06 00 04|01 00 00 00 01|00 00 ?? 00 */
				 && packet->payload[0] == 0x06
				 && packet->payload[1] == 0x00 && (packet->payload[2] == 0x04 || packet->payload[2] == 0x01)
				 && packet->payload[3] == 0x00
				 && packet->payload[4] == 0x00
				 && packet->payload[5] == 0x00 && (packet->payload[6] == 0x01 || packet->payload[6] == 0x00)
				 && packet->payload[7] == 0x00
				 /* && packet->payload[8]==0x00 */
				 && packet->payload[9] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		} else if ((packet->actual_payload_len == 64 || packet->actual_payload_len == 52	/* PATTERN:: [len] 00 00 00 00 */
					|| packet->actual_payload_len == 95)
				   && get_u16(packet->payload, 0) == (packet->actual_payload_len)
				   && packet->payload[1] == 0x00 && packet->payload[2] == 0x00
				   && packet->payload[3] == 0x00 && packet->payload[4] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		} else if (packet->actual_payload_len == 6 && packet->payload[0] == 0x06	// PATTERN : 06 00 04|6c 00|01 00 00
				   && packet->payload[1] == 0x00 && (packet->payload[2] == 0x04 || packet->payload[2] == 0x6c)
				   && (packet->payload[3] == 0x00 || packet->payload[3] == 0x01)
				   && packet->payload[4] == 0x00 && packet->payload[5] == 0x00) {

			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		} else if (packet->actual_payload_len == 6	/* PATTERN:: [len] ?? ee 00 00 00 */
				   && get_u16(packet->payload, 0) == (packet->actual_payload_len)
				   && packet->payload[2] == 0xee
				   && packet->payload[3] == 0x00 && packet->payload[4] == 0x00 && packet->payload[5] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);
		}

		else if (packet->actual_payload_len == 10	/* PATTERN:: 06 00 00 00 00 00 00 00 */
				 && packet->payload[0] == 0x06
				 && packet->payload[1] == 0x00
				 && packet->payload[2] == 0x00
				 && packet->payload[3] == 0x00
				 && packet->payload[4] == 0x00
				 && packet->payload[5] == 0x00 && packet->payload[6] == 0x00 && packet->payload[7] == 0x00) {
			flow->imesh_stage += 2;
			IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG,
					"IMESH FOUND :: Payload %u\n", packet->actual_payload_len);

		}
	}

	/*give one packet tolerance for detection */
	if (flow->imesh_stage >= 4)

		ipoque_int_imesh_add_connection(ipoque_struct);


	else if ((flow->packet_counter < 5) || packet->actual_payload_len == 0) {
		return;
	} else {
		goto imesh_not_found_end;
	}

  imesh_not_found_end:
	IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_IMESH);
	IPQ_LOG(IPOQUE_PROTOCOL_IMESH, ipoque_struct, IPQ_LOG_DEBUG, "iMesh excluded at stage %d\n", flow->imesh_stage);

}
#endif
