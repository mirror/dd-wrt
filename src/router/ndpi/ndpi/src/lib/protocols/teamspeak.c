/*
 * teamspeak.c 
 *
 * Copyright (C) 2013 Remy Mudingay <mudingay@ill.fr>
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
 */

#include "ndpi_api.h"


#ifdef NDPI_PROTOCOL_TEAMSPEAK

static void ndpi_int_teamspeak_add_connection(struct ndpi_detection_module_struct
                                             *ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_TEAMSPEAK, NDPI_PROTOCOL_UNKNOWN);
}
static  u_int16_t b_tdport = 0, b_tsport = 0;
static  u_int16_t b_udport = 0, b_usport = 0;


static void ndpi_search_teamspeak(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
    struct ndpi_packet_struct *packet = &flow->packet;

if (packet->udp != NULL) {
  b_usport = ntohs(packet->udp->source), b_udport = ntohs(packet->udp->dest);
  /* http://www.imfirewall.com/en/protocols/teamSpeak.htm  */
  if (((b_usport == 9987 || b_udport == 9987) || (b_usport == 8767 || b_udport == 8767)) && packet->payload_packet_len >= 20) {
     NDPI_LOG(NDPI_PROTOCOL_TEAMSPEAK, ndpi_struct, NDPI_LOG_DEBUG, "found TEAMSPEAK udp.\n");
     ndpi_int_teamspeak_add_connection(ndpi_struct, flow);
  }
}
else if (packet->tcp != NULL) {
  b_tsport = ntohs(packet->tcp->source), b_tdport = ntohs(packet->tcp->dest);
  /* https://github.com/Youx/soliloque-server/wiki/Connection-packet */
  if(packet->payload_packet_len >= 20) {
    if (((memcmp(packet->payload, "\xf4\xbe\x03\x00", 4) == 0)) ||
          ((memcmp(packet->payload, "\xf4\xbe\x02\x00", 4) == 0)) ||
            ((memcmp(packet->payload, "\xf4\xbe\x01\x00", 4) == 0))) {
     NDPI_LOG(NDPI_PROTOCOL_TEAMSPEAK, ndpi_struct, NDPI_LOG_DEBUG, "found TEAMSPEAK tcp.\n");
     ndpi_int_teamspeak_add_connection(ndpi_struct, flow);
    }  /* http://www.imfirewall.com/en/protocols/teamSpeak.htm  */
  } else if ((b_tsport == 14534 || b_tdport == 14534) || (b_tsport == 51234 || b_tdport == 51234)) {
     NDPI_LOG(NDPI_PROTOCOL_TEAMSPEAK, ndpi_struct, NDPI_LOG_DEBUG, "found TEAMSPEAK.\n");
     ndpi_int_teamspeak_add_connection(ndpi_struct, flow);
   }
  }
  NDPI_LOG(NDPI_PROTOCOL_TEAMSPEAK, ndpi_struct, NDPI_LOG_DEBUG, "TEAMSPEAK excluded.\n");
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_TEAMSPEAK);
  return;
}

static void init_teamspeak_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id, NDPI_PROTOCOL_BITMASK *detection_bitmask)
{
  ndpi_set_bitmask_protocol_detection("TeamSpeak", ndpi_struct, detection_bitmask, *id,
				      NDPI_PROTOCOL_TEAMSPEAK,
				      ndpi_search_teamspeak,
				      NDPI_SELECTION_BITMASK_PROTOCOL_TCP_OR_UDP_WITH_PAYLOAD,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);

  *id += 1;
}

#endif
