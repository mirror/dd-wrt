/*
 * skype.c
 * Copyright (C) 2011 by ntop.org
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

#ifdef NTOP_PROTOCOL_SKYPE

static u_int is_private_addr(u32 addr) {
  addr = ntohl(addr);

  if(((addr & 0xFF000000) == 0x0A000000) /* 10.0.0.0/8  */
     || ((addr & 0xFFF00000) == 0xAC100000) /* 172.16/12   */
	|| ((addr & 0xFFFF0000) == 0xC0A80000) /* 192.168/16  */
     || ((addr & 0xFF000000) == 0x7F000000) /* 127.0.0.0/8 */
     )
    return(1);
  else
    return(0);
}

static void ntop_check_skype(struct ipoque_detection_module_struct *ipoque_struct)
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

  if(ipoque_struct->packet.udp != NULL) {
    flow->l4.udp.skype_packet_id++;

    if(flow->l4.udp.skype_packet_id < 5) {
      /* skype-to-skype */
      if(((payload_len == 3) && ((packet->payload[2] & 0x0F)== 0x0d))
	 || ((payload_len >= 16) 
	     && (packet->payload[0] != 0x30) /* Avoid invalid SNMP detection */
	     && (packet->payload[2] == 0x02))) {
	IPQ_LOG(NTOP_PROTOCOL_SKYPE, ipoque_struct, IPQ_LOG_DEBUG, "Found skype.\n");
	ipoque_int_add_connection(ipoque_struct, NTOP_PROTOCOL_SKYPE, IPOQUE_REAL_PROTOCOL);	
      }
      
      return;
    }

    IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NTOP_PROTOCOL_SKYPE);
    return;
  } else if(ipoque_struct->packet.tcp != NULL) {
    flow->l4.tcp.skype_packet_id++;

    if(flow->l4.tcp.skype_packet_id < 3) {
      ; /* Too early */
    } else if((flow->l4.tcp.skype_packet_id == 3)
	      /* We have seen the 3-way handshake */
	      && flow->l4.tcp.seen_syn
	      && flow->l4.tcp.seen_syn_ack
	      && flow->l4.tcp.seen_ack) {
      if((payload_len == 8) || (payload_len == 3)) {
	IPQ_LOG(NTOP_PROTOCOL_SKYPE, ipoque_struct, IPQ_LOG_DEBUG, "Found skype.\n");
	ipoque_int_add_connection(ipoque_struct, NTOP_PROTOCOL_SKYPE, IPOQUE_REAL_PROTOCOL);
      }

      /* printf("[SKYPE] [id: %u][len: %d]\n", flow->l4.tcp.skype_packet_id, payload_len);  */
    } else
      IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NTOP_PROTOCOL_SKYPE);

    return;
  }
}

void ntop_search_skype(struct ipoque_detection_module_struct *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;

  IPQ_LOG(NTOP_PROTOCOL_SKYPE, ipoque_struct, IPQ_LOG_DEBUG, "skype detection...\n");

  /* skip marked packets */
  if(packet->detected_protocol_stack[0] != NTOP_PROTOCOL_SKYPE)
    ntop_check_skype(ipoque_struct);
}

#endif
