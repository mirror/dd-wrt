/*
 * list.c
 *
 * Copyright (C) 2017-22 - ntop.org
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

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_LISP

#include "ndpi_api.h"
#include "ndpi_private.h"

#define LISP_PORT  4341 /* Only UDP */
#define LISP_PORT1 4342 /* TCP and UDP */

static void ndpi_int_lisp_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					    struct ndpi_flow_struct *flow)
{

  ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_LISP, NDPI_PROTOCOL_UNKNOWN, NDPI_CONFIDENCE_DPI);
}

static void ndpi_check_lisp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{

  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
  u_int16_t lisp_port1 = htons(LISP_PORT1);
  u_int16_t lisp_port = htons(LISP_PORT);

  if(packet->udp != NULL) {
    if((packet->udp->source == lisp_port && packet->udp->dest == lisp_port) ||
       (packet->udp->source == lisp_port1 && packet->udp->dest == lisp_port1)) {
      NDPI_LOG_INFO(ndpi_struct, "found lisp\n");
      ndpi_int_lisp_add_connection(ndpi_struct, flow);
      return;
    }
  } else {
    /* See draft-kouvelas-lisp-map-server-reliable-transport-07 */
    if(packet->tcp->source == lisp_port1 ||
       packet->tcp->dest == lisp_port1) {
      if(packet->payload_packet_len >= 8) {
        u_int16_t msg_len = ntohs(*(u_int16_t *)&packet->payload[2]);
	if(msg_len >= packet->payload_packet_len &&
	   /* End marker: we don't handle fragmented messages */
	   packet->payload[packet->payload_packet_len - 1] == 0xE9 &&
	   packet->payload[packet->payload_packet_len - 2] == 0xAD &&
	   packet->payload[packet->payload_packet_len - 3] == 0xAC &&
	   packet->payload[packet->payload_packet_len - 4] == 0x9F) {
	  NDPI_LOG_INFO(ndpi_struct, "found lisp\n");
	  ndpi_int_lisp_add_connection(ndpi_struct, flow);
          return;
	}
      }
    }
  }

  NDPI_EXCLUDE_DISSECTOR(ndpi_struct, flow);
}

static void ndpi_search_lisp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  NDPI_LOG_DBG(ndpi_struct, "search lisp\n");

  ndpi_check_lisp(ndpi_struct, flow);
}


void init_lisp_dissector(struct ndpi_detection_module_struct *ndpi_struct)
{
  register_dissector("LISP", ndpi_struct,
                     ndpi_search_lisp,
                     NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
                      1, NDPI_PROTOCOL_LISP);
}

