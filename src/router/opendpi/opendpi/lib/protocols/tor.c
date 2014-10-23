/*
 * tor.c
 *
 * Copyright (C) 2013 Remy Mudingay <mudingay@ill.fr>
 *
 */


#include "ndpi_api.h"


#ifdef NDPI_PROTOCOL_TOR
static void ndpi_int_tor_add_connection(struct ndpi_detection_module_struct
					*ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_TOR, NDPI_CORRELATED_PROTOCOL);
}

static void ndpi_search_tor(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int16_t dport = 0, sport = 0;

  NDPI_LOG(NDPI_PROTOCOL_TOR, ndpi_struct, NDPI_LOG_DEBUG, "search for TOR.\n");

  if(packet->tcp != NULL) {
    sport = ntohs(packet->tcp->source), dport = ntohs(packet->tcp->dest);
    NDPI_LOG(NDPI_PROTOCOL_TOR, ndpi_struct, NDPI_LOG_DEBUG, "calculating TOR over tcp.\n");

    if ((((dport == 9001) || (sport == 9001)) || ((dport == 9030) || (sport == 9030)))
	&& ((packet->payload[0] == 0x17) || (packet->payload[0] == 0x16)) 
	&& (packet->payload[1] == 0x03) 
	&& (packet->payload[2] == 0x01) 
	&& (packet->payload[3] == 0x00)) {
      NDPI_LOG(NDPI_PROTOCOL_TOR, ndpi_struct, NDPI_LOG_DEBUG, "found tor.\n");
      ndpi_int_tor_add_connection(ndpi_struct, flow);
    }
  } else {
    NDPI_LOG(NDPI_PROTOCOL_TOR, ndpi_struct, NDPI_LOG_DEBUG, "exclude TOR.\n");
    NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_TOR);
  }
}
#endif
