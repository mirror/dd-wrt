/*
 * citrix.c
 * Copyright (C) 2012 by ntop.org
 *
 * This module is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "ipq_utils.h"

#ifdef NTOP_PROTOCOL_CITRIX

/* ************************************ */

static void ntop_check_citrix(struct ipoque_detection_module_struct *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  struct ipoque_flow_struct *flow = ipoque_struct->flow;
  const u8 *packet_payload = packet->payload;
  u32 payload_len = packet->payload_packet_len;

#if 0
  printf("[len=%u][%02X %02X %02X %02X]\n", payload_len,
	 packet->payload[0] & 0xFF,
	 packet->payload[1] & 0xFF,
	 packet->payload[2] & 0xFF,
	 packet->payload[3] & 0xFF);
#endif

  if(ipoque_struct->packet.tcp != NULL) {
    flow->l4.tcp.citrix_packet_id++;
    
    if((flow->l4.tcp.citrix_packet_id == 3)
       /* We have seen the 3-way handshake */
       && flow->l4.tcp.seen_syn
       && flow->l4.tcp.seen_syn_ack
       && flow->l4.tcp.seen_ack) {
      if(payload_len == 6) {
	char citrix_header[] = { 0x07, 0x07, 0x49, 0x43, 0x41, 0x00 };
	
	if(memcmp(packet->payload, citrix_header, sizeof(citrix_header)) == 0) {
	  IPQ_LOG(NTOP_PROTOCOL_CITRIX, ipoque_struct, IPQ_LOG_DEBUG, "Found citrix.\n");
	  ipoque_int_add_connection(ipoque_struct, NTOP_PROTOCOL_CITRIX, IPOQUE_REAL_PROTOCOL);
	}

	return;
      } else if(payload_len > 4) {
	char citrix_header[] = { 0x1a, 0x43, 0x47, 0x50, 0x2f, 0x30, 0x31 };
	
	if((memcmp(packet->payload, citrix_header, sizeof(citrix_header)) == 0)
	   || (ntop_strnstr(packet->payload, "Citrix.TcpProxyService", payload_len) != NULL)) {
	  IPQ_LOG(NTOP_PROTOCOL_CITRIX, ipoque_struct, IPQ_LOG_DEBUG, "Found citrix.\n");
	  ipoque_int_add_connection(ipoque_struct, NTOP_PROTOCOL_CITRIX, IPOQUE_REAL_PROTOCOL);
	}

	return;	
      }
      
      
      IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NTOP_PROTOCOL_CITRIX);
    } else if(flow->l4.tcp.citrix_packet_id > 3)
      IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NTOP_PROTOCOL_CITRIX);
    
    return;
  }
}

void ntop_search_citrix(struct ipoque_detection_module_struct *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;

  IPQ_LOG(NTOP_PROTOCOL_CITRIX, ipoque_struct, IPQ_LOG_DEBUG, "citrix detection...\n");

  /* skip marked packets */
  if(packet->detected_protocol_stack[0] != NTOP_PROTOCOL_CITRIX)
    ntop_check_citrix(ipoque_struct);
}

#endif
