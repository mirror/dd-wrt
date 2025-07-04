/*
 * dcerpc.c
 *
 * Copyright (C) 2011-18 by ntop.org
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

#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_DCERPC

#include "ndpi_api.h"
#include "ndpi_private.h"
//#include <stdbool.h>

static void ndpi_int_dcerpc_add_connection(struct ndpi_detection_module_struct
					     *ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_DCERPC, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
}

static bool is_connection_oriented_dcerpc(struct ndpi_packet_struct *packet)
{
  if((packet->tcp != NULL)
     && (packet->payload_packet_len >= 64)
     && (packet->payload[0] == 0x05) /* version 5 */
     && (packet->payload[2] < 16) /* Packet type */
		 && (((packet->payload[9]<<8) | packet->payload[8]) == packet->payload_packet_len) /* Packet Length */
     ) {
    return true;
  }
  return false;
}

static bool is_connectionless_dcerpc(struct ndpi_packet_struct *packet)
{
  u_int16_t fragment_len;
  
  if (packet->udp == NULL)
    return false;
  if (packet->payload_packet_len < 80)
    return false;
  if (packet->payload[0] != 0x04) /* type must be equal to 4 */
    return false;
  if (packet->payload[1] > 10) /* must be <= CANCEL ACK or it's not connectionless DCE/RPC */
    return false;
  if (packet->payload[3] & 0xFC) /* flags2: bit 3:8 are reserved for future use and must be set to 0 */
    return false;
  if (packet->payload[4] & 0xEE) /* neither big endian nor little endian */
    return false;
  if (packet->payload[5] > 3) /* invalid floating point type */
    return false;

  if(packet->payload[4] == 0x10)
    fragment_len = (packet->payload[75] << 8) + packet->payload[74]; /* Big endian */
  else
    fragment_len = (packet->payload[74] << 8) + packet->payload[75]; /* Little endian */

  if(packet->payload_packet_len != (fragment_len+76 /* offset */ + 4 /* rest of the packet */))
    return false; /* Too short or too long, bot RPC */
  
  return true;
}

static void ndpi_search_dcerpc(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);

  NDPI_LOG_DBG(ndpi_struct, "search DCERPC\n");
  if (is_connection_oriented_dcerpc(packet) || is_connectionless_dcerpc(packet)) {
    NDPI_LOG_INFO(ndpi_struct, "found DCERPC\n");
    ndpi_int_dcerpc_add_connection(ndpi_struct, flow);
    return;
  }

  if(packet->payload_packet_len>1)
    NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
}


void init_dcerpc_dissector(struct ndpi_detection_module_struct *ndpi_struct)
{
  register_dissector("DCERPC", ndpi_struct,
                     ndpi_search_dcerpc,
                     NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
                     1, NDPI_PROTOCOL_DCERPC);
}
