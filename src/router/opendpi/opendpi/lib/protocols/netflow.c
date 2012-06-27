/*
 * netflow.c
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

#ifdef NTOP_PROTOCOL_NETFLOW

static void ntop_check_netflow(struct ipoque_detection_module_struct *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  struct ipoque_flow_struct *flow = ipoque_struct->flow;
  const u8 *packet_payload = packet->payload;
  u32 payload_len = packet->payload_packet_len;
  
  if((ipoque_struct->packet.udp != NULL)
     && (payload_len >= 24)      
     && (packet->payload[0] == 0)
     && ((packet->payload[1] == 5)
	 || (packet->payload[1] == 9)
	 || (packet->payload[1] == 10 /* IPFIX */))
     && (packet->payload[3] <= 48 /* Flow count */)) {    
    u_int32_t when, *_when;

    _when = (u_int32_t*)&packet->payload[8]; /* Sysuptime */

    when = ntohl(*_when);

    if((when >= 946684800 /* 1/1/2000 */) && (when <= get_seconds())) {
      IPQ_LOG(NTOP_PROTOCOL_NETFLOW, ipoque_struct, IPQ_LOG_DEBUG, "Found netflow.\n");
      ipoque_int_add_connection(ipoque_struct, NTOP_PROTOCOL_NETFLOW, IPOQUE_REAL_PROTOCOL);
      return;
    }
  }
}

static void ntop_search_netflow(struct ipoque_detection_module_struct *ipoque_struct)
{
  IPQ_LOG(NTOP_PROTOCOL_NETFLOW, ipoque_struct, IPQ_LOG_DEBUG, "netflow detection...\n");
  ntop_check_netflow(ipoque_struct);
}

#endif
