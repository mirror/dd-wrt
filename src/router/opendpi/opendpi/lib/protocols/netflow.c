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


#include "ndpi_utils.h"

#ifdef NDPI_PROTOCOL_NETFLOW

static void ndpi_check_netflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  
  const u_int8_t *packet_payload = packet->payload;
  u_int32_t payload_len = packet->payload_packet_len;
  time_t now;

  if((packet->udp != NULL)
     && (payload_len >= 24)      
     && (packet_payload[0] == 0)
     && ((packet_payload[1] == 5)
	 || (packet_payload[1] == 9)
	 || (packet_payload[1] == 10 /* IPFIX */))
     && (packet_payload[3] <= 48 /* Flow count */)) {    
    u_int32_t when, *_when;

    _when = (u_int32_t*)&packet_payload[8]; /* Sysuptime */

    when = ntohl(*_when);

#ifndef OPENDPI_NETFILTER_MODULE
    now = time(NULL);
#else
    struct timeval now_tv;
    do_gettimeofday(&now_tv);
    now = now_tv.tv_sec;
#endif

    if((when >= 946684800 /* 1/1/2000 */) && (when <= now)) {
      NDPI_LOG(NDPI_PROTOCOL_NETFLOW, ndpi_struct, NDPI_LOG_DEBUG, "Found netflow.\n");
      ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_NETFLOW, NDPI_REAL_PROTOCOL);
      return;
    }
  }
}

static void ndpi_search_netflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  NDPI_LOG(NDPI_PROTOCOL_NETFLOW, ndpi_struct, NDPI_LOG_DEBUG, "netflow detection...\n");
  ndpi_check_netflow(ndpi_struct, flow);
}

#endif
