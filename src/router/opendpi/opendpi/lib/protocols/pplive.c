/*
 * pplive.c
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
#include "ndpi_utils.h"

#ifdef NDPI_PROTOCOL_PPLIVE

static void ndpi_int_pplive_add_connection(struct ndpi_detection_module_struct *ndpi_struct, 
					   struct ndpi_flow_struct *flow, ndpi_protocol_type_t protocol_type)
{
  ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_PPLIVE, protocol_type);
}

static void ndpi_search_pplive_tcp_udp(struct ndpi_detection_module_struct
				*ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
	
  struct ndpi_id_struct *src = flow->src;
  struct ndpi_id_struct *dst = flow->dst;


  u_int16_t a;

  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "search pplive.\n");


  if (packet->udp != NULL) {

    if (src != NULL && src->pplive_vod_cli_port == packet->udp->source
	&& NDPI_COMPARE_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, NDPI_PROTOCOL_PPLIVE)) {
      if (src->pplive_last_packet_time_set == 1 && (u_int32_t)
	  (packet->tick_timestamp - src->pplive_last_packet_time) < ndpi_struct->pplive_connection_timeout) {
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	src->pplive_last_packet_time_set = 1;
	src->pplive_last_packet_time = packet->tick_timestamp;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "timestamp src.\n");
	return;
      } else {
	src->pplive_vod_cli_port = 0;
	src->pplive_last_packet_time = 0;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: VOD port timer reset.\n");
      }
    }

    if (dst != NULL && dst->pplive_vod_cli_port == packet->udp->dest
	&& NDPI_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, NDPI_PROTOCOL_PPLIVE)) {
      if (dst->pplive_last_packet_time_set == 1 && (u_int32_t)
	  (packet->tick_timestamp - dst->pplive_last_packet_time) < ndpi_struct->pplive_connection_timeout) {
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	dst->pplive_last_packet_time_set = 1;
	dst->pplive_last_packet_time = packet->tick_timestamp;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "timestamp dst.\n");
	return;
      } else {
	dst->pplive_last_packet_time_set = 0;
	dst->pplive_vod_cli_port = 0;
	dst->pplive_last_packet_time = 0;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: VOD port timer reset.\n");
      }
    }

    if ((packet->payload_packet_len >= 76) && ((packet->payload[0] == 0x01) || (packet->payload[0] == 0x18)
					       || (packet->payload[0] == 0x05))
	&& (packet->payload[1] == 0x00)
	&& get_l32(packet->payload, 12) == 0 && (packet->payload[16] == 0 || packet->payload[16] == 1)
	&& (packet->payload[17] == 0) && (packet->payload[24] == 0xac)) {
      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "found pplive.\n");
      ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
      return;
    }

    if (packet->payload_packet_len > 50 && packet->payload[0] == 0xe9
	&& packet->payload[1] == 0x03 && (packet->payload[3] == 0x00 || packet->payload[3] == 0x01)
	&& packet->payload[4] == 0x98 && packet->payload[5] == 0xab
	&& packet->payload[6] == 0x01 && packet->payload[7] == 0x02) {
      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "found pplive.\n");
      ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
      return;
    }

    if (packet->payload_packet_len == 94 && packet->payload[8] == 0x00
	&& ((get_u_int32_t(packet->payload, 9) == ntohl(0x03010000))
	    || (get_u_int32_t(packet->payload, 9) == ntohl(0x02010000))
	    || (get_u_int32_t(packet->payload, 9) == ntohl(0x01010000)))
	&& ((get_u_int32_t(packet->payload, 58) == ntohl(0xb1130000))
	    || ((packet->payload[60] == packet->payload[61])
		&& (packet->payload[78] == 0x00 || packet->payload[78] == 0x01)
		&& (packet->payload[79] == 0x00 || packet->payload[79] == 0x01))
	    || ((get_u_int16_t(packet->payload, 58) == ntohs(0xb113))
		&& (packet->payload[78] == 0x00 || packet->payload[78] == 0x01)
		&& (packet->payload[79] == 0x00 || packet->payload[79] == 0x01)))) {
      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "found pplive.\n");
      ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
      return;
    }

    if ((packet->payload_packet_len >= 90 && packet->payload_packet_len <= 110)
	&& (packet->payload[0] >= 0x0a && packet->payload[0] <= 0x0f)
	&& get_u_int16_t(packet->payload, 86) == 0) {
      int i;
      for (i = 54; i < 68; i += 2) {
	if (((get_u_int32_t(packet->payload, i) == ntohl(0x4fde7e7f))
	     || (get_u_int32_t(packet->payload, i) == ntohl(0x7aa6090d)))
	    && (get_u_int16_t(packet->payload, i + 4) == 0)) {
	  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "found pplive through "
		   "bitpatterns either 4f de 7e 7f 00 00 or 7a a6 09 0d 00 00.\n");
	  ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	  return;
	}
      }
    }
    if (flow->packet_counter < 5 && !flow->pplive_stage) {	/* With in 1st 4 packets */
      if (((packet->payload_packet_len >= 90 && packet->payload_packet_len <= 110)
	   && (!get_u_int32_t(packet->payload, packet->payload_packet_len - 16)
	       || !get_u_int32_t(packet->payload, packet->payload_packet_len - 4)))
	  ) {
	flow->pplive_stage = 2;	/* Now start looking for size(28 | 30) */
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
		 "Maybe found pplive packet. Now start looking for size(28 | 30).\n");
      }
      if (68 == packet->payload_packet_len
	  && get_l16(packet->payload, 0) == 0x21 && packet->payload[19] == packet->payload[20]
	  && packet->payload[20] == packet->payload[21]
	  && packet->payload[12] == packet->payload[13]
	  && packet->payload[14] == packet->payload[15]) {
	flow->pplive_stage = 3 + packet->packet_direction;
      }

      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "need next packet I.\n");
      return;
    }
    if (flow->pplive_stage == 3 + packet->packet_direction) {
      /* Because we are expecting packet in reverese direction.. */
      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "need next packet II.\n");
      return;
    }
    if (flow->pplive_stage == (4 - packet->packet_direction)
	&& packet->payload_packet_len > 67
	&& (get_l16(packet->payload, 0) == 0x21
	    || (get_l16(packet->payload, 0) == 0x22 && !get_l16(packet->payload, 28)))) {
      if (dst != NULL) {
	dst->pplive_vod_cli_port = packet->udp->dest;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		 NDPI_LOG_DEBUG, "PPLIVE: VOD Port marked %u.\n", ntohs(packet->udp->dest));
	dst->pplive_last_packet_time = packet->tick_timestamp;
	dst->pplive_last_packet_time_set = 1;
      }
      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "found pplive.\n");
      ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
      return;
    }

    if (flow->pplive_stage == 2) {
      if ((packet->payload_packet_len == 30 && (packet->payload[0] == 0x02 || packet->payload[0] == 0x03)
	   && get_u_int32_t(packet->payload, 21) == ntohl(0x00000001))
	  || (packet->payload_packet_len == 28 && (packet->payload[0] == 0x01 || packet->payload[0] == 0x00)
	      && (get_u_int32_t(packet->payload, 19) == ntohl(0x00000001)))) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "found pplive.\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	return;
      }
      if (flow->packet_counter < 45) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "need next packet III.\n");
	return;
      }
    }
  } else if (packet->tcp != NULL) {

    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
	     "PPLIVE: TCP found, plen = %d, stage = %d, payload[0] = %x, payload[1] = %x, payload[2] = %x, payload[3] = %x \n",
	     packet->payload_packet_len, flow->pplive_stage, packet->payload[0], packet->payload[1],
	     packet->payload[2], packet->payload[3]);

    if (src != NULL && src->pplive_vod_cli_port == packet->tcp->source
	&& NDPI_COMPARE_PROTOCOL_TO_BITMASK(src->detected_protocol_bitmask, NDPI_PROTOCOL_PPLIVE)) {
      if (src->pplive_last_packet_time_set == 1 && (u_int32_t)
	  (packet->tick_timestamp - src->pplive_last_packet_time) < ndpi_struct->pplive_connection_timeout) {
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	src->pplive_last_packet_time_set = 1;
	src->pplive_last_packet_time = packet->tick_timestamp;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "timestamp src.\n");
	return;
      } else {
	src->pplive_vod_cli_port = 0;
	src->pplive_last_packet_time = 0;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: VOD port timer reset.\n");
      }
    }

    if (dst != NULL && dst->pplive_vod_cli_port == packet->tcp->dest
	&& NDPI_COMPARE_PROTOCOL_TO_BITMASK(dst->detected_protocol_bitmask, NDPI_PROTOCOL_PPLIVE)) {
      if (dst->pplive_last_packet_time_set == 1 && (u_int32_t)
	  (packet->tick_timestamp - dst->pplive_last_packet_time) < ndpi_struct->pplive_connection_timeout) {
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	dst->pplive_last_packet_time_set = 1;
	dst->pplive_last_packet_time = packet->tick_timestamp;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "timestamp dst.\n");
	return;
      } else {
	dst->pplive_last_packet_time_set = 0;
	dst->pplive_vod_cli_port = 0;
	dst->pplive_last_packet_time = 0;
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: VOD port timer reset.\n");
      }
    }

    if (packet->payload_packet_len > 4 && memcmp(packet->payload, "GET /", 5) == 0) {
      ndpi_parse_packet_line_info(ndpi_struct, flow);
      if ((packet->parsed_lines == 8 || packet->parsed_lines == 9)
	  && packet->line[0].ptr != NULL && packet->line[0].len >= 8
	  && memcmp(&packet->payload[packet->line[0].len - 8], "HTTP/1.", 7) == 0
	  && packet->line[2].ptr != NULL && packet->line[2].len >= 16
	  && memcmp(packet->line[2].ptr, "x-flash-version:", 16) == 0
	  && packet->user_agent_line.ptr != NULL && packet->user_agent_line.len >= 11
	  && memcmp(packet->user_agent_line.ptr, "Mozilla/4.0", 11) == 0
	  && packet->line[6].ptr != NULL && packet->line[6].len >= 21
	  && memcmp(packet->line[6].ptr, "Pragma: Client=PPLive", 21) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: found HTTP request.\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	return;
      } else if ((packet->parsed_lines >= 6 && packet->parsed_lines <= 8)
		 && packet->line[0].ptr != NULL && packet->line[0].len >= 8
		 && memcmp(&packet->payload[packet->line[0].len - 8], "HTTP/1.", 7) == 0
		 && (((packet->user_agent_line.ptr != NULL && packet->user_agent_line.len >= 10)
		      && (memcmp(packet->user_agent_line.ptr, "PPLive DAC", 10) == 0))
		     || ((packet->user_agent_line.ptr != NULL && packet->user_agent_line.len >= 19)
			 && (memcmp(packet->user_agent_line.ptr, "PPLive-Media-Player", 19) == 0)))) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: found HTTP request.\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	return;
      } else if (packet->host_line.ptr != NULL && packet->host_line.len >= NDPI_STATICSTRING_LEN("player.pplive")
		 && memcmp(packet->host_line.ptr, "player.pplive", NDPI_STATICSTRING_LEN("player.pplive")) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: found via Host header.\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	return;
      } else if (packet->referer_line.ptr != NULL
		 && packet->referer_line.len >= NDPI_STATICSTRING_LEN("http://player.pplive")
		 && memcmp(packet->referer_line.ptr, "http://player.pplive",
			   NDPI_STATICSTRING_LEN("http://player.pplive")) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: found via Referer header.\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	return;
      } else if (packet->parsed_lines >= 8 &&
		 packet->line[0].ptr != NULL && packet->line[0].len >= 8 &&
		 memcmp(&packet->payload[packet->line[0].len - 8], "HTTP/1.", 7) == 0) {

	u_int8_t i, flag = 0;

	for (i = 0; i < packet->parsed_lines && i < 10 && flag < 2; i++) {
	  if (packet->line[i].ptr != NULL && packet->line[i].len >= NDPI_STATICSTRING_LEN("x-flash-version:")
	      && memcmp(packet->line[i].ptr, "x-flash-version:",
			NDPI_STATICSTRING_LEN("x-flash-version:")) == 0) {
	    flag++;
	  } else if (packet->line[i].ptr != NULL
		     && packet->line[i].len >= NDPI_STATICSTRING_LEN("Pragma: Client=PPLive")
		     && memcmp(packet->line[i].ptr, "Pragma: Client=PPLive",
			       NDPI_STATICSTRING_LEN("Pragma: Client=PPLive")) == 0) {
	    flag++;
	  }
	}
	if (flag == 2) {
	  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "PPLIVE: found HTTP request.\n");
	  ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_CORRELATED_PROTOCOL);
	  return;
	}
      }
    }
    // searches for packets > 20 byte that begin with a hex number == packet->payload_packet_len - 4
    // and with the same number at position 16, 17, 18, 19
    if (packet->payload_packet_len > 20 && ntohl(get_u_int32_t(packet->payload, 0)) == packet->payload_packet_len - 4) {
      if (packet->payload[4] == 0x21 && packet->payload[5] == 0x00) {
	if ((packet->payload[9] == packet->payload[10]) && (packet->payload[9] == packet->payload[11])) {
	  if ((packet->payload[16] == packet->payload[17]) &&
	      (packet->payload[16] == packet->payload[18]) && (packet->payload[16] == packet->payload[19])) {
	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		     NDPI_LOG_DEBUG, "PPLIVE: direct server request or response found\n");
	    ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	    return;
	  }
	}
      }
    }
    // stage > 0, packet begins with 21 00, bytes at positions 5, 6, 7 are equal, bytes at positions 12, 13, 14, 15 are equal,
    if (packet->payload_packet_len > 20 && flow->pplive_stage) {
      if (packet->payload[0] == 0x21 && packet->payload[1] == 0x00) {
	if (packet->payload[5] == packet->payload[6] && packet->payload[5] == packet->payload[7]) {
	  if (packet->payload[12] == packet->payload[13] && packet->payload[14] == packet->payload[15]
	      && packet->payload[12] == packet->payload[14]) {
	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		     NDPI_LOG_DEBUG, "PPLIVE: direct server request or response found\n");
	    ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	    return;
	  }
	}
      }
    }
    // packet (len>11) begins with a hex number == packet->payload_packet_len - 4 and matches certain bitmuster
    if (packet->payload_packet_len > 11 && ntohl(get_u_int32_t(packet->payload, 0)) == packet->payload_packet_len - 4) {
      if (packet->payload[4] == 0xe9 && packet->payload[5] == 0x03 &&
	  ((packet->payload[7] == packet->payload[10]) || (packet->payload[7] == packet->payload[11]))) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		 NDPI_LOG_DEBUG, "PPLIVE: direct server request or response found\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	return;
      }
    }
    // stage > 0, len>10, begins with e9 03, matches certain pattern
    if (packet->payload_packet_len > 10 && flow->pplive_stage) {
      if (packet->payload[0] == 0xe9 && packet->payload[1] == 0x03 &&
	  ((packet->payload[3] == packet->payload[6]) || (packet->payload[3] == packet->payload[7]))) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		 NDPI_LOG_DEBUG, "PPLIVE: direct server request or response found\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	return;
      }
    }

    /* Adware in the PPLive Client -> first TCP Packet has length of 4 Bytes -> 2nd TCP Packet has length of 96 Bytes */
    /* or */
    /* Peer-List Requests over TCP -> first Packet has length of 4 Bytes -> 2nd TCP Packet has length of 71 Bytes */
    /* there are different possibilities of the order of the packets */

    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
	     "PPLIVE: TCP found, plen = %d, stage = %d, payload[0] = %x, payload[1] = %x, payload[2] = %x, payload[3] = %x \n",
	     packet->payload_packet_len, flow->pplive_stage,
	     packet->payload[0], packet->payload[1], packet->payload[2], packet->payload[3]);

    /* generic pplive detection (independent of the stage) !!! */
    // len > 11, packet begins with a hex number == packet->payload_packet_len - 4, pattern: ?? ?? ?? ?? 21 00 ?? ?? 98 ab 01 02
    if (packet->payload_packet_len > 11 && ntohl(get_u_int32_t(packet->payload, 0)) == packet->payload_packet_len - 4) {
      if (packet->payload[4] == 0x21 && packet->payload[5] == 0x00
	  && ((packet->payload[8] == 0x98 && packet->payload[9] == 0xab
	       && packet->payload[10] == 0x01 && packet->payload[11] == 0x02)
	      )) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		 NDPI_LOG_DEBUG, "PPLIVE: direct server request or response found\n");
	ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	return;
      }
      // packet 4 to 19 have a hex representation from 0x30 to 0x39
      if (packet->payload_packet_len > 20) {
	a = 4;
	while (a < 20) {
	  if (packet->payload[a] >= '0' && packet->payload[a] <= '9') {
	    if (a == 19) {
	      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		       NDPI_LOG_DEBUG, "PPLIVE: direct new header format found\n");
	      ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	      return;
	    } else {
	      a++;
	    }
	  } else {
	    break;
	  }
	}
      }
    }

    /* 1st and 2nd (KD: ??????? )Packet of Client is 4 Byte  */
    // stage == 0, p_len == 4, pattern: 04 00 00 00 --> need next packet
    if (flow->pplive_stage == 0) {
      if (packet->payload_packet_len == 4 && packet->payload[0] > 0x04
	  && packet->payload[1] == 0x00 && packet->payload[2] == 0x00 && packet->payload[3] == 0x00) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		 NDPI_LOG_DEBUG, "PPLIVE: 4Byte TCP Packet Request found \n");

	/* go to the 2nd Client Packet */
	flow->pplive_stage = 1 + packet->packet_direction;
	flow->l4.tcp.pplive_next_packet_size[packet->packet_direction] = packet->payload[0];
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "need next packet i.\n");
	return;
      }
    } else if (flow->pplive_stage == 2 - packet->packet_direction) {
      if (packet->payload_packet_len == 4 && packet->payload[0] > 0x04
	  && packet->payload[1] == 0x00 && packet->payload[2] == 0x00 && packet->payload[3] == 0x00) {
	NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		 NDPI_LOG_DEBUG, "PPLIVE: 4Byte TCP Packet Response found \n");

	/* go to the 2nd Client Packet */
	flow->l4.tcp.pplive_next_packet_size[packet->packet_direction] = packet->payload[0];
      }
      flow->pplive_stage = 3;
      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "need next packet ii.\n");
      return;
    } else if (flow->pplive_stage == 1 + packet->packet_direction || flow->pplive_stage == 3) {
      if (packet->payload_packet_len > 7 && flow->l4.tcp.pplive_next_packet_size[packet->packet_direction] >= 4) {
	if (packet->payload_packet_len == flow->l4.tcp.pplive_next_packet_size[packet->packet_direction]) {

	  if (packet->payload[0] == 0xe9 && packet->payload[1] == 0x03
	      && ((packet->payload[4] == 0x98
		   && packet->payload[5] == 0xab && packet->payload[6] == 0x01 && packet->payload[7] == 0x02)
		  )) {
	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct,
		     NDPI_LOG_DEBUG, "PPLIVE: two packet response found\n");

	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
		     "found pplive over tcp with pattern iii.\n");
	    ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	    return;
	  }
	  // packet 4 to 19 have a hex representation from 0x30 to 0x39
	  if (packet->payload_packet_len > 16) {
	    a = 0;
	    while (a < 16) {
	      if (packet->payload[a] >= '0' && packet->payload[a] <= '9') {
		if (a == 15) {
		  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
			   "PPLIVE: new header format found\n");
		  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
			   "found pplive over tcp with pattern v.\n");
		  ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
		  return;
		} else {
		  a++;
		}
	      } else {
		break;
	      }
	    }
	  }
	  // p_len>79 and a lot of 00 in the end
	  if (packet->payload_packet_len > 79
	      && get_u_int32_t(packet->payload, packet->payload_packet_len - 9) == 0x00000000
	      && get_u_int32_t(packet->payload, packet->payload_packet_len - 5) == 0x00000000) {
	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
		     "PPLIVE: Last 8 NULL bytes found.\n");
	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
		     "found pplive over tcp with pattern vi.\n");
	    ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
	    return;
	  }
	}
	if (packet->payload_packet_len > flow->l4.tcp.pplive_next_packet_size[packet->packet_direction]) {
	  if (packet->payload[0] == 0xe9 && packet->payload[1] == 0x03
	      && packet->payload[4] == 0x98 && packet->payload[5] == 0xab
	      && packet->payload[6] == 0x01 && packet->payload[7] == 0x02) {
	    a = flow->l4.tcp.pplive_next_packet_size[packet->packet_direction];
	    NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "a=%u.\n", a);
	    if (packet->payload_packet_len > a + 4
		&& packet->payload[a + 2] == 0x00 && packet->payload[a + 3] == 0x00
		&& packet->payload[a] != 0) {
	      a += ((packet->payload[a + 1] << 8) + packet->payload[a] + 4);
	      NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "a=%u.\n", a);
	      if (packet->payload_packet_len == a) {
		NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
			 "found pplive over tcp with pattern vii.\n");
		ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
		return;
	      }
	      if (packet->payload_packet_len > a + 4
		  && packet->payload[a + 2] == 0x00 && packet->payload[a + 3] == 0x00
		  && packet->payload[a] != 0) {
		a += ((packet->payload[a + 1] << 8) + packet->payload[a] + 4);
		if (packet->payload_packet_len == a) {
		  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG,
			   "found pplive over tcp with pattern viii.\n");
		  ndpi_int_pplive_add_connection(ndpi_struct, flow, NDPI_REAL_PROTOCOL);
		  return;
		}
	      }

	    }
	  }
	}
      }
    }
  }


  NDPI_LOG(NDPI_PROTOCOL_PPLIVE, ndpi_struct, NDPI_LOG_DEBUG, "exclude pplive.\n");
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_PPLIVE);
}
#endif
