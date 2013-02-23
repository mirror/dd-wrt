/*
 * gtp.c
 *
 * Copyright (C) 2011-13 - ntop.org
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


#include "ndpi_utils.h"

#ifdef NDPI_PROTOCOL_GTP

struct gtp_header_generic {
  u_int8_t flags, message_type;
  u_int16_t message_len;
  u_int32_t teid;
};

static void ndpi_check_gtp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  // const u_int8_t *packet_payload = packet->payload;
  u_int32_t payload_len = packet->payload_packet_len;

#if 0
  printf("[len=%u][%02X %02X %02X %02X]\n", payload_len,
	 packet->payload[0] & 0xFF,
	 packet->payload[1] & 0xFF,
	 packet->payload[2] & 0xFF,
	 packet->payload[3] & 0xFF);
#endif

  if((packet->udp != NULL) && (payload_len > sizeof(struct gtp_header_generic))) {
    u_int32_t gtp_u = ntohs(2152);
    u_int32_t gtp_c = ntohs(2123);

    if((packet->udp->source == gtp_u) || (packet->udp->dest == gtp_u)
       || (packet->udp->source == gtp_c) || (packet->udp->dest == gtp_c)) {
      struct gtp_header_generic *gtp = (struct gtp_header_generic*)packet->payload;
      u_int8_t gtp_version = gtp->flags & 0xE0;

      if((gtp_version == 1) || (gtp_version == 2)) {
	if(ntohs(gtp->message_len) <= (payload_len+sizeof(struct gtp_header_generic))) {
	  NDPI_LOG(NDPI_PROTOCOL_GTP, ndpi_struct, NDPI_LOG_DEBUG, "Found gtp.\n");
	  ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_GTP, NDPI_REAL_PROTOCOL);
	  return;
	}
      }
    }
  }

  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_GTP);
  return;
}

static void ndpi_search_gtp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;

  NDPI_LOG(NDPI_PROTOCOL_GTP, ndpi_struct, NDPI_LOG_DEBUG, "gtp detection...\n");

  /* skip marked packets */
  if(packet->detected_protocol_stack[0] != NDPI_PROTOCOL_GTP)
    ndpi_check_gtp(ndpi_struct, flow);
}

#endif
