/*
 * sflow.c
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

#ifdef NDPI_PROTOCOL_SFLOW

static void ndpi_check_sflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  
  const u_int8_t *packet_payload = packet->payload;
  u_int32_t payload_len = packet->payload_packet_len;

  if((packet->udp != NULL)
     && (payload_len >= 24)
     /* Version */
     && (packet_payload[0] == 0) && (packet_payload[1] == 0) && (packet_payload[2] == 0)
     && ((packet_payload[3] == 2) || (packet_payload[3] == 5))) {
    NDPI_LOG(NDPI_PROTOCOL_SFLOW, ndpi_struct, NDPI_LOG_DEBUG, "Found sflow.\n");
    ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_SFLOW, NDPI_REAL_PROTOCOL);
    return;
  }
}

static void ndpi_search_sflow(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  NDPI_LOG(NDPI_PROTOCOL_SFLOW, ndpi_struct, NDPI_LOG_DEBUG, "sflow detection...\n");
  ndpi_check_sflow(ndpi_struct, flow);
}

#endif
