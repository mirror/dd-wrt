/*
 * gnutella.c
 *
 * Copyright (C) 2009-11 - ipoque GmbH
 * Copyright (C) 2011-25 - ntop.org
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

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_GNUTELLA

#include "ndpi_api.h"
#include "ndpi_private.h"


static void ndpi_int_gnutella_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					     struct ndpi_flow_struct *flow,
					     ndpi_confidence_t confidence)
{
  NDPI_LOG_INFO(ndpi_struct, "found GNUTELLA\n");
  ndpi_set_detected_protocol_keeping_master(ndpi_struct, flow, NDPI_PROTOCOL_GNUTELLA,
					    confidence);
}

static void ndpi_search_gnutella(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);
	
  u_int16_t c;

  NDPI_LOG_DBG(ndpi_struct, "search GNUTELLA\n");

  /* skip packets without payload */
  if (packet->payload_packet_len < 2) {
    return;
  }
  if (packet->tcp != NULL) {
    /* this case works asymmetrically */
    if (packet->payload_packet_len > 10 && memcmp(packet->payload, "GNUTELLA/", 9) == 0) {
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    /* this case works asymmetrically */
    if (packet->payload_packet_len > 17 && memcmp(packet->payload, "GNUTELLA CONNECT/", 17) == 0) {
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      /* Extract some metadata HTTP-like */
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      if(packet->user_agent_line.ptr != NULL)
        ndpi_user_agent_set(flow, packet->user_agent_line.ptr, packet->user_agent_line.len);
      return;
    }

    if (packet->payload_packet_len > 50 && ((memcmp(packet->payload, "GET /get/", 9) == 0)
					    || (memcmp(packet->payload, "GET /uri-res/", 13) == 0)
					    )) {
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      for (c = 0; c < packet->parsed_lines; c++) {
	if ((packet->line[c].len > 19 && memcmp(packet->line[c].ptr, "User-Agent: Gnutella", 20) == 0)
	    || (packet->line[c].len > 10 && memcmp(packet->line[c].ptr, "X-Gnutella-", 11) == 0)
	    || (packet->line[c].len > 7 && memcmp(packet->line[c].ptr, "X-Queue:", 8) == 0)
	    || (packet->line[c].len > 36 && memcmp(packet->line[c].ptr,
						   "Content-Type: application/x-gnutella-", 37) == 0)) {
	  ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	  /* Extract some metadata HTTP-like */
	  if(packet->user_agent_line.ptr != NULL)
	    ndpi_user_agent_set(flow, packet->user_agent_line.ptr, packet->user_agent_line.len);
	  return;
	}
      }
    }
    if (packet->payload_packet_len > 50 && ((memcmp(packet->payload, "GET / HTTP", 10) == 0))) {
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      if ((packet->user_agent_line.ptr != NULL && packet->user_agent_line.len > 15
	   && memcmp(packet->user_agent_line.ptr, "BearShare Lite ", 15) == 0)
	  || (packet->accept_line.ptr != NULL && packet->accept_line.len > 24
	      && memcmp(packet->accept_line.ptr, "application n/x-gnutella", 24) == 0)) {
	ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	/* Extract some metadata HTTP-like */
	if(packet->user_agent_line.ptr != NULL)
	  ndpi_user_agent_set(flow, packet->user_agent_line.ptr, packet->user_agent_line.len);
      }

    }
    /* haven't found this pattern in any trace. */
    if (packet->payload_packet_len > 50 && ((memcmp(packet->payload, "GET /get/", 9) == 0)
					    || (memcmp(packet->payload, "GET /uri-res/", 13) == 0))) {
      c = 8;
      while (c < (packet->payload_packet_len - 9)) {
	if (packet->payload[c] == '?') {
	  c++;
	  break;
	}
	c++;
      }

      if (c < (packet->payload_packet_len - 9) && memcmp(&packet->payload[c], "urn:sha1:", 9) == 0) {
	NDPI_LOG_DBG2(ndpi_struct, "detected GET /get/ or GET /uri-res/\n");
	ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      }

    }

    /* answer to this packet is HTTP/1.1 ..... Content-Type: application/x-gnutella-packets,
     * it is searched in the upper paragraph. */
    if (packet->payload_packet_len > 30 && memcmp(packet->payload, "HEAD /gnutella/push-proxy?", 26) == 0) {
      NDPI_LOG_DBG2(ndpi_struct, "detected HEAD /gnutella/push-proxy?\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    /* haven't found any trace with this pattern */
    if (packet->payload_packet_len == 46
	&& memcmp(packet->payload, "\x50\x55\x53\x48\x20\x67\x75\x69\x64\x3a", 10) == 0) {
      NDPI_LOG_DBG2(ndpi_struct,
	       "detected \x50\x55\x53\x48\x20\x67\x75\x69\x64\x3a\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    /* haven't found any trace with this pattern */
    if (packet->payload_packet_len > 250 && memcmp(packet->payload, "GET /gnutella/", 14) == 0)
      //PATTERN IS :: GET /gnutella/tigertree/v3?urn:tree:tiger/:
      {
	const u_int16_t end = packet->payload_packet_len - 3;

	c = 13;
	while (c < end) {
	  if ((memcmp(&packet->payload[14], "tigertree/", 10) == 0)
	      || (end - c > 18 && memcmp(&packet->payload[c], "\r\nUser-Agent: Foxy", 18) == 0)
	      || (end - c > 44
		  && memcmp(&packet->payload[c],
			    "\r\nAccept: application/tigertree-breadthfirst",
			    44) == 0) || (end - c > 10 && memcmp(&packet->payload[c], "\r\nX-Queue:", 10) == 0)
	      || (end - c > 13 && memcmp(&packet->payload[c], "\r\nX-Features:", 13) == 0)) {

	    NDPI_LOG_DBG2(ndpi_struct, "FOXY :: GNUTELLA GET 2 DETECTED\n");
	    ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	    return;
	  }

	  c++;
	}
      }
    /* haven't found any trace with this pattern */
    if (packet->payload_packet_len > 1 && packet->payload[packet->payload_packet_len - 1] == 0x0a
	&& packet->payload[packet->payload_packet_len - 2] == 0x0a) {
      if (packet->payload_packet_len > 3 && memcmp(packet->payload, "GIV", 3) == 0) {
	NDPI_LOG_DBG2(ndpi_struct, "MORPHEUS GIV DETECTED\n");
	/* Not Excluding the flow now.. We shall Check the next Packet too for Gnutella Patterns */
	return;
      }
    }
    /* might be super tricky new ssl gnutella transmission, but the certificate is strange... */
    if (packet->payload_packet_len == 46 && get_u_int32_t(packet->payload, 0) == htonl(0x802c0103) &&
	get_u_int32_t(packet->payload, 4) == htonl(0x01000300) && get_u_int32_t(packet->payload, 8) == htonl(0x00002000) &&
	get_u_int16_t(packet->payload, 12) == htons(0x0034)) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella len == 46\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    if (packet->payload_packet_len == 49 &&
	memcmp(packet->payload, "\x80\x2f\x01\x03\x01\x00\x06\x00\x00\x00\x20\x00\x00\x34\x00\x00\xff\x4d\x6c",
	       19) == 0) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella len == 49\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    if (packet->payload_packet_len == 89 && memcmp(&packet->payload[43], "\x20\x4d\x6c", 3) == 0 &&
	memcmp(packet->payload, "\x16\x03\x01\x00\x54\x01\x00\x00\x50\x03\x01\x4d\x6c", 13) == 0 &&
	memcmp(&packet->payload[76], "\x00\x02\x00\x34\x01\x00\x00\x05", 8) == 0) {
      NDPI_LOG_DBG2(ndpi_struct,
	       "detected gnutella asymmetrically len == 388.\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    } else if (packet->payload_packet_len == 82) {
      if (get_u_int32_t(packet->payload, 0) == htonl(0x16030100)
	  && get_u_int32_t(packet->payload, 4) == htonl(0x4d010000)
	  && get_u_int16_t(packet->payload, 8) == htons(0x4903)
	  && get_u_int16_t(packet->payload, 76) == htons(0x0002)
	  && get_u_int32_t(packet->payload, 78) == htonl(0x00340100)) {
	NDPI_LOG_DBG2(ndpi_struct, "detected len == 82\n");
	ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	return;
      }
    }
  } else if (packet->udp != NULL) {
    /* Check for Mojito-DHT encapsulated gnutella (gtk-gnutella). */
    if (packet->payload_packet_len >= 28 &&
        ntohl(get_u_int32_t(packet->payload, 24)) == 0x47544b47 /* GTKG */)
    {
        u_int32_t gnutella_payload_len = le32toh(get_u_int32_t(packet->payload, 19));

        if (gnutella_payload_len == (u_int32_t)packet->payload_packet_len - 23)
        {
            NDPI_LOG_DBG2(ndpi_struct, "detected mojito-dht/gnutella udp\n");
            ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
            return;
        }
    }

    /* observations:
     * all the following patterns send out many packets which are the only ones of their flows,
     * often on the very beginning of the traces, or flows with many packets in one direction only.
     * but then suddenly, one gets an answer as you can see in netpeker-gnutella-rpc.pcap packet 11483.
     * Maybe gnutella tries to send out keys?
     */
    if (packet->payload_packet_len == 23 && packet->payload[15] == 0x00
	&& packet->payload[16] == 0x41 && packet->payload[17] == 0x01
	&& packet->payload[18] == 0x00 && packet->payload[19] == 0x00
	&& packet->payload[20] == 0x00 && packet->payload[21] == 0x00 && packet->payload[22] == 0x00) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 23\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);

      return;
    }
    if (packet->payload_packet_len == 35 && packet->payload[25] == 0x49
	&& packet->payload[26] == 0x50 && packet->payload[27] == 0x40
	&& packet->payload[28] == 0x83 && packet->payload[29] == 0x53
	&& packet->payload[30] == 0x43 && packet->payload[31] == 0x50 && packet->payload[32] == 0x41) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 35\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    if (packet->payload_packet_len == 32
	&& (memcmp(&packet->payload[16], "\x31\x01\x00\x09\x00\x00\x00\x4c\x49\x4d\x45", 11) == 0)) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 32\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    if (packet->payload_packet_len == 34 && (memcmp(&packet->payload[25], "SCP@", 4) == 0)
	&& (memcmp(&packet->payload[30], "DNA@", 4) == 0)) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 34\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    if ((packet->payload_packet_len == 73 || packet->payload_packet_len == 96)
	&& memcmp(&packet->payload[32], "urn:sha1:", 9) == 0) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 73,96\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }

    if (packet->payload_packet_len >= 3 && memcmp(packet->payload, "GND", 3) == 0) {
      if ((packet->payload_packet_len == 8 && (memcmp(&packet->payload[6], "\x01\x00", 2) == 0))
	  || (packet->payload_packet_len == 11 && (memcmp(&packet->payload[6], "\x01\x01\x08\x50\x49", 5)
						   == 0)) || (packet->payload_packet_len == 17
							      &&
							      (memcmp
							       (&packet->payload[6], "\x01\x01\x4c\x05\x50",
								5) == 0))
	  || (packet->payload_packet_len == 28
	      && (memcmp(&packet->payload[6], "\x01\x01\x54\x0f\x51\x4b\x52\x50\x06\x52", 10) == 0))
	  || (packet->payload_packet_len == 41
	      && (memcmp(&packet->payload[6], "\x01\x01\x5c\x1b\x50\x55\x53\x48\x48\x10", 10) == 0))
	  || (packet->payload_packet_len > 200 && packet->payload_packet_len < 300 && packet->payload[3] == 0x03)
	  || (packet->payload_packet_len > 300 && (packet->payload[3] == 0x01 || packet->payload[3] == 0x03))) {
	NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, GND\n");
	ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	return;
      }
    }
    if (packet->payload_packet_len >= 4 && memcmp(packet->payload, "GND\x10", 4) == 0) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, GND (2)\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }

    if ((packet->payload_packet_len == 32)
	&& memcmp(&packet->payload[16], "\x31\x01\x00\x09\x00\x00\x00", 7) == 0) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 32 ii\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
    if ((packet->payload_packet_len == 23)
	&& memcmp(&packet->payload[16], "\x00\x01\x00\x00\x00\x00\x00", 7) == 0) {
      NDPI_LOG_DBG2(ndpi_struct, "detected gnutella udp, len = 23 ii\n");
      ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
      return;
    }
  }
  //neonet detection follows

  /* haven't found any trace with this pattern */
  if (packet->tcp != NULL && ntohs(packet->tcp->source) >= 1024 && ntohs(packet->tcp->dest) >= 1024) {
    if (flow->l4.tcp.gnutella_stage == 0) {
      if (flow->packet_counter == 1
	  && (packet->payload_packet_len == 11
	      || packet->payload_packet_len == 33 || packet->payload_packet_len == 37)) {
	flow->l4.tcp.gnutella_msg_id[0] = packet->payload[4];
	flow->l4.tcp.gnutella_msg_id[1] = packet->payload[6];
	flow->l4.tcp.gnutella_msg_id[2] = packet->payload[8];
	flow->l4.tcp.gnutella_stage = 1 + packet->packet_direction;
	return;
      }
    } else if (flow->l4.tcp.gnutella_stage == 1 + packet->packet_direction) {
      if (flow->packet_counter == 2 && (packet->payload_packet_len == 33 || packet->payload_packet_len == 22)
	  && flow->l4.tcp.gnutella_msg_id[0] == packet->payload[0]
	  && flow->l4.tcp.gnutella_msg_id[1] == packet->payload[2]
	  && flow->l4.tcp.gnutella_msg_id[2] == packet->payload[4]) {
	NDPI_LOG_DBG2(ndpi_struct, "GNUTELLA DETECTED due to message ID match (NEONet protocol)\n");
	ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	return;
      }
    } else if (flow->l4.tcp.gnutella_stage == 2 - packet->packet_direction) {
      if (flow->packet_counter == 2 && (packet->payload_packet_len == 10 || packet->payload_packet_len == 75)
	  && flow->l4.tcp.gnutella_msg_id[0] == packet->payload[0]
	  && flow->l4.tcp.gnutella_msg_id[1] == packet->payload[2]
	  && flow->l4.tcp.gnutella_msg_id[2] == packet->payload[4]) {
	NDPI_LOG_DBG2(ndpi_struct, "GNUTELLA DETECTED due to message ID match (NEONet protocol)\n");
	ndpi_int_gnutella_add_connection(ndpi_struct, flow, NDPI_CONFIDENCE_DPI);
	return;
      }
    }
  }

  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
}


void init_gnutella_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id)
{
  ndpi_set_bitmask_protocol_detection("Gnutella", ndpi_struct, *id,
				      NDPI_PROTOCOL_GNUTELLA,
				      ndpi_search_gnutella,
				      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);

  *id += 1;
}

