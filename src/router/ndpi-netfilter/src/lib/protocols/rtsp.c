/*
 * rtsp.c
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

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_RTSP

#include "ndpi_api.h"
#include "ndpi_private.h"


static void ndpi_int_rtsp_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					 struct ndpi_flow_struct *flow)
{
  ndpi_set_detected_protocol_keeping_master(ndpi_struct, flow, NDPI_PROTOCOL_RTSP,
					    NDPI_CONFIDENCE_DPI);
}

/* this function searches for a rtsp-"handshake" over tcp or udp. */
static void ndpi_search_rtsp_tcp_udp(struct ndpi_detection_module_struct *ndpi_struct,
				     struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = ndpi_get_packet_struct(ndpi_struct);

  NDPI_LOG_DBG(ndpi_struct, "search RTSP\n");

  if (packet->parsed_lines == 0)
  {
    ndpi_parse_packet_line_info(ndpi_struct, flow);
  }

  if (packet->parsed_lines > 0 &&
      (LINE_ENDS(packet->line[0], "RTSP/1.0") != 0 ||
       LINE_ENDS(packet->accept_line, "application/x-rtsp-tunnelled") != 0 ||
       LINE_ENDS(packet->content_line, "application/x-rtsp-tunnelled") != 0))
  {
    ndpi_int_rtsp_add_connection(ndpi_struct, flow);

    /* Extract some metadata HTTP-like */
    if(packet->user_agent_line.ptr != NULL)
      ndpi_user_agent_set(flow, packet->user_agent_line.ptr, packet->user_agent_line.len);

    return;
  }

  if (flow->rtsprdt_stage == 0
      && !(flow->detected_protocol_stack[0] == NDPI_PROTOCOL_RTCP)
      ) {
    flow->rtsprdt_stage = 1 + packet->packet_direction;
    NDPI_LOG_DBG2(ndpi_struct, "maybe handshake 1; need next packet, return\n");
    return;
  }

  if (flow->packet_counter < 3 && flow->rtsprdt_stage == 1 + packet->packet_direction) {

    NDPI_LOG_DBG2(ndpi_struct, "maybe handshake 2; need next packet\n");
    return;
  }

  if (packet->payload_packet_len > 20 && flow->rtsprdt_stage == 2 - packet->packet_direction) {
    char buf[32] = { 0 };
    u_int len = packet->payload_packet_len;

    if(len >= (sizeof(buf)-1)) len = sizeof(buf)-1;
    strncpy(buf, (const char*)packet->payload, len);

    // RTSP Server Message
    if((memcmp(packet->payload, "RTSP/1.0 ", 9) == 0)
       || (strstr(buf, "rtsp://") != NULL)) {
      NDPI_LOG_DBG2(ndpi_struct, "found RTSP/1.0 \n");
      NDPI_LOG_INFO(ndpi_struct, "found RTSP\n");
      ndpi_int_rtsp_add_connection(ndpi_struct, flow);
      return;
    }
  }

  if (packet->udp != NULL && flow->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN
      && ((NDPI_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_RTP) == 0)
	  || (NDPI_COMPARE_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_RTCP) == 0)
	  )) {
    NDPI_LOG_DBG2(ndpi_struct,
	     "maybe RTSP RTP, RTSP RTCP, RDT; need next packet.\n");
    return;
  }


  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
  return;
}


void init_rtsp_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id)
{
  ndpi_set_bitmask_protocol_detection("RTSP", ndpi_struct, *id,
				      NDPI_PROTOCOL_RTSP,
				      ndpi_search_rtsp_tcp_udp,
				      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_OR_UDP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);
  *id += 1;
}
