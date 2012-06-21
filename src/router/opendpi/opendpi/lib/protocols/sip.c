/*
 * sip.c
 * Copyright (C) 2009-2011 by ipoque GmbH
 * 
 * This file is part of OpenDPI, an open source deep packet inspection
 * library based on the PACE technology by ipoque GmbH
 * 
 * OpenDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OpenDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with OpenDPI.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */


#include "ipq_utils.h"

#ifdef IPOQUE_PROTOCOL_SIP
static void ipoque_int_sip_add_connection(struct ipoque_detection_module_struct
					  *ipoque_struct, u8 due_to_correlation)
{

  ipoque_int_add_connection(ipoque_struct,
			    IPOQUE_PROTOCOL_SIP,
			    due_to_correlation ? IPOQUE_CORRELATED_PROTOCOL : IPOQUE_REAL_PROTOCOL);
}


	
#if !(defined(HAVE_NTOP) && defined(WIN32))
static inline
#else
__forceinline static
#endif
void ipoque_search_sip_handshake(struct ipoque_detection_module_struct
				 *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  struct ipoque_flow_struct *flow = ipoque_struct->flow;
  //      struct ipoque_id_struct         *src=ipoque_struct->src;
  //      struct ipoque_id_struct         *dst=ipoque_struct->dst;
  const u8 *packet_payload = packet->payload;
  u32 payload_len = packet->payload_packet_len;


  if (payload_len > 4) {
    /* search for STUN Turn ChannelData Prefix */
    u16 message_len = ntohs(get_u16(packet->payload, 2));
    if (payload_len - 4 == message_len) {
      IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found STUN TURN ChannelData prefix.\n");
      payload_len -= 4;
      packet_payload += 4;
    }
  }
#ifndef IPOQUE_PROTOCOL_YAHOO
  if (payload_len >= 14 && packet_payload[payload_len - 2] == 0x0d && packet_payload[payload_len - 1] == 0x0a)
#endif
#ifdef IPOQUE_PROTOCOL_YAHOO
    if (payload_len >= 14)
#endif
      {

#if defined(HAVE_NTOP)
	if ((memcmp(packet_payload, "NOTIFY ", 7) == 0 || memcmp(packet_payload, "notify ", 7) == 0)
	    && (memcmp(&packet_payload[7], "SIP:", 4) == 0 || memcmp(&packet_payload[7], "sip:", 4) == 0)) {

	  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip NOTIFY.\n");
	  ipoque_int_sip_add_connection(ipoque_struct, 0);
	  return;
	}
#endif

	if ((memcmp(packet_payload, "REGISTER ", 9) == 0 || memcmp(packet_payload, "register ", 9) == 0)
	    && (memcmp(&packet_payload[9], "SIP:", 4) == 0 || memcmp(&packet_payload[9], "sip:", 4) == 0)) {

	  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip REGISTER.\n");
	  ipoque_int_sip_add_connection(ipoque_struct, 0);
	  return;
	}
	if ((memcmp(packet_payload, "INVITE ", 7) == 0 || memcmp(packet_payload, "invite ", 7) == 0)
	    && (memcmp(&packet_payload[7], "SIP:", 4) == 0 || memcmp(&packet_payload[7], "sip:", 4) == 0)) {
	  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip INVITE.\n");
	  ipoque_int_sip_add_connection(ipoque_struct, 0);
	  return;
	}
	/* seen this in second direction on the third position,
	 * maybe it could be deleted, if somebody sees it in the first direction,
	 * please delete this comment.
	 */
	if (memcmp(packet_payload, "SIP/2.0 200 OK", 14) == 0 || memcmp(packet_payload, "sip/2.0 200 OK", 14) == 0) {
	  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip SIP/2.0 0K.\n");
	  ipoque_int_sip_add_connection(ipoque_struct, 0);
	  return;
	}

#if defined(HAVE_NTOP) 
	/* Courtesy of Miguel Quesada <mquesadab@gmail.com> */
	if ((memcmp(packet_payload, "OPTIONS ", 8) == 0
	     || memcmp(packet_payload, "options ", 8) == 0)
	    && (memcmp(&packet_payload[8], "SIP:", 4) == 0
		|| memcmp(&packet_payload[8], "sip:", 4) == 0)) {
	  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "found sip OPTIONS.\n");
	  ipoque_int_sip_add_connection(ipoque_struct, 0);
	  return;
	}
#endif
      }

  /* add bitmask for tcp only, some stupid udp programs
   * send a very few (< 10 ) packets before invite (mostly a 0x0a0x0d, but just search the first 3 payload_packets here */
  if (packet->udp != NULL && flow->packet_counter < 20) {
    IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "need next packet.\n");
    return;
  }
#ifdef IPOQUE_PROTOCOL_STUN
  /* for STUN flows we need some more packets */
  if (packet->udp != NULL && flow->detected_protocol_stack[0] == IPOQUE_PROTOCOL_STUN && flow->packet_counter < 40) {
    IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "need next STUN packet.\n");
    return;
  }
#endif

  if (payload_len == 4 && get_u32(packet_payload, 0) == 0) {
    IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "maybe sip. need next packet.\n");
    return;
  }
#ifdef IPOQUE_PROTOCOL_YAHOO
  if (payload_len > 30 && packet_payload[0] == 0x90
      && packet_payload[3] == payload_len - 20 && get_u32(packet_payload, 4) == 0
      && get_u32(packet_payload, 8) == 0) {
    flow->sip_yahoo_voice = 1;
    IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "maybe sip yahoo. need next packet.\n");
  }
  if (flow->sip_yahoo_voice && flow->packet_counter < 10) {
    return;
  }
#endif
  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "exclude sip.\n");
  IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_SIP);
  return;


}

void ipoque_search_sip(struct ipoque_detection_module_struct *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  //  struct ipoque_flow_struct   *flow = ipoque_struct->flow;
  //      struct ipoque_id_struct         *src=ipoque_struct->src;
  //      struct ipoque_id_struct         *dst=ipoque_struct->dst;

  IPQ_LOG(IPOQUE_PROTOCOL_SIP, ipoque_struct, IPQ_LOG_DEBUG, "sip detection...\n");

  /* skip marked packets */
  if (packet->detected_protocol_stack[0] != IPOQUE_PROTOCOL_SIP) {
    if (packet->tcp_retransmission == 0) {
      ipoque_search_sip_handshake(ipoque_struct);
    }
  }
}

#endif
