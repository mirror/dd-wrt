/*
 * pando.c
 *
 * Copyright (C) 2009-2011 by ipoque GmbH
 * Copyright (C) 2011-13 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ndpi_protocols.h"

#ifdef NDPI_PROTOCOL_PANDO

static void ndpi_int_pando_add_connection(struct ndpi_detection_module_struct
					  *ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_PANDO, NDPI_REAL_PROTOCOL);
}

	
#if !defined(WIN32)
static inline
#else
__forceinline static
#endif
u_int8_t search_pando(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  //      struct ndpi_flow_struct       *flow=ndpi_struct->flow;
  //      struct ndpi_id_struct         *src=ndpi_struct->src;
  //      struct ndpi_id_struct         *dst=ndpi_struct->dst;

  if (packet->tcp != NULL) {

    if (packet->payload_packet_len == 63 && memcmp(&packet->payload[1], "Pando protocol", 14) == 0) {
      NDPI_LOG(NDPI_PROTOCOL_PANDO, ndpi_struct, NDPI_LOG_DEBUG, "Pando download detected\n");
      goto end_pando_found;
    }

  } else if (packet->udp != NULL) {
    if (packet->payload_packet_len > 20
	&& packet->payload_packet_len < 100
	&& packet->payload[0] == 0x00
	&& packet->payload[1] == 0x00
	&& packet->payload[2] == 0x00
	&& packet->payload[3] == 0x09 && packet->payload[4] == 0x00 && packet->payload[5] == 0x00) {
      // bypass the detection because one packet has at a specific place the word Pando in it
      if (packet->payload_packet_len == 87 && memcmp(&packet->payload[25], "Pando protocol", 14) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_PANDO, ndpi_struct, NDPI_LOG_DEBUG,
		 "Pando UDP packet detected --> Pando in payload\n");
	goto end_pando_found;
      } else if (packet->payload_packet_len == 92 && memcmp(&packet->payload[72], "Pando", 5) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_PANDO, ndpi_struct, NDPI_LOG_DEBUG,
		 "Pando UDP packet detected --> Pando in payload\n");
	goto end_pando_found;
      }
      goto end_pando_maybe_found;
    }
  }

  goto end_pando_nothing_found;

 end_pando_found:
  ndpi_int_pando_add_connection(ndpi_struct, flow);
  return 1;

 end_pando_maybe_found:
  return 2;

 end_pando_nothing_found:
  return 0;
}

static void ndpi_search_pando_tcp_udp(struct ndpi_detection_module_struct
			       *ndpi_struct, struct ndpi_flow_struct *flow)
{
  //      struct ndpi_packet_struct     *packet=&flow->packet;	
  //      struct ndpi_id_struct         *src=ndpi_struct->src;
  //      struct ndpi_id_struct         *dst=ndpi_struct->dst;

  if (search_pando(ndpi_struct, flow) != 0)
    return;
  
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_PANDO);
}
#endif
