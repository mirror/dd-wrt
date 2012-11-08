/*
 * teamviewer.c
 * Copyright (C) 2012 by Gianluca Costa xplico.org
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



#include "ndpi_protocols.h"

#ifdef NDPI_PROTOCOL_TEAMVIEWER

static void ndpi_int_teamview_add_connection(struct ndpi_detection_module_struct
                                             *ndpi_struct, struct ndpi_flow_struct *flow)
{
    ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_TEAMVIEWER, NDPI_REAL_PROTOCOL);
}


static void ndpi_search_teamview(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
    struct ndpi_packet_struct *packet = &flow->packet;
    
    
    if (packet->udp != NULL) {
         if (packet->payload_packet_len > 13) {
             if (packet->payload[0] == 0x00 && packet->payload[11] == 0x17 && packet->payload[12] == 0x24) { /* byte 0 is a counter/seq number, and at the start is 0 */
                flow->l4.udp.teamviewer_stage++;
                if (flow->l4.udp.teamviewer_stage == 4 || 
                    packet->udp->dest == ntohs(5938) || packet->udp->source == ntohs(5938)) {
                    ndpi_int_teamview_add_connection(ndpi_struct, flow);
                }
                return;
            }
        }
    }
    else if(packet->tcp != NULL) {
        if (packet->payload_packet_len > 2) {
            if (packet->payload[0] == 0x17 && packet->payload[1] == 0x24) {
                flow->l4.udp.teamviewer_stage++;
                if (flow->l4.udp.teamviewer_stage == 4 || 
                    packet->tcp->dest == ntohs(5938) || packet->tcp->source == ntohs(5938)) {
                    ndpi_int_teamview_add_connection(ndpi_struct, flow);
                }
                return;
            }
            else if (flow->l4.udp.teamviewer_stage) {
                if (packet->payload[0] == 0x11 && packet->payload[1] == 0x30) {
                    flow->l4.udp.teamviewer_stage++;
                    if (flow->l4.udp.teamviewer_stage == 4)
                        ndpi_int_teamview_add_connection(ndpi_struct, flow);
                }
                return;
            }
        }
    }
    
    NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_TEAMVIEWER);
}
#endif
