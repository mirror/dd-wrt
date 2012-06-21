/*
 * skype.c
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

#ifdef NTOP_PROTOCOL_RADIUS

struct radius_header {
  u_int8_t code;
  u_int8_t packet_id;
  u_int16_t len;
};

static void ntop_check_radius(struct ipoque_detection_module_struct *ipoque_struct)
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
    struct radius_header *h = (struct radius_header*)packet->payload;

    h->len = ntohs(h->len);

    if((payload_len > sizeof(struct radius_header))
       && (h->code <= 5)
       && (h->len == payload_len)) {
      IPQ_LOG(NTOP_PROTOCOL_RADIUS, ipoque_struct, IPQ_LOG_DEBUG, "Found radius.\n");
      ipoque_int_add_connection(ipoque_struct, NTOP_PROTOCOL_RADIUS, IPOQUE_REAL_PROTOCOL);	
      
      return;
    }
    
    IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NTOP_PROTOCOL_RADIUS);
    return;
  }
}

void ntop_search_radius(struct ipoque_detection_module_struct *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;

  IPQ_LOG(NTOP_PROTOCOL_RADIUS, ipoque_struct, IPQ_LOG_DEBUG, "radius detection...\n");

  /* skip marked packets */
  if(packet->detected_protocol_stack[0] != NTOP_PROTOCOL_RADIUS)
    ntop_check_radius(ipoque_struct);
}

#endif
