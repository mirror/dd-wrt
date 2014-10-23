/*
 * dns.c
 *
 * Copyright (C) 2012-13 - ntop.org
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

#ifdef NDPI_PROTOCOL_DNS

struct dns_packet_header {
  u_int16_t transaction_id, flags, num_queries, answer_rrs, authority_rrs, additional_rrs;
} __attribute__((packed));

static void ndpi_search_dns(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  u_int16_t dport = 0, sport = 0;
  
#define NDPI_MAX_DNS_REQUESTS			16

  NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "search DNS.\n");
  
  if (packet->udp != NULL) {
    sport = ntohs(packet->udp->source),  dport = ntohs(packet->udp->dest);
    NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "calculated dport over UDP.\n");
  } else  if(packet->tcp != NULL) {
    sport = ntohs(packet->tcp->source), dport = ntohs(packet->tcp->dest);
    NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "calculated dport over tcp.\n");
  }

  if(((dport == 53) || (sport == 53))
     && (packet->payload_packet_len > sizeof(struct dns_packet_header))) {
    struct dns_packet_header header, *dns = (struct dns_packet_header*)&packet->payload[packet->tcp ? 2 : 0];
    u_int8_t is_query, ret_code, is_dns = 0;
    
    header.flags = ntohs(dns->flags);
    header.transaction_id = ntohs(dns->transaction_id);
    header.num_queries = ntohs(dns->num_queries);
    header.answer_rrs = ntohs(dns->answer_rrs);
    header.authority_rrs = ntohs(dns->authority_rrs);
    header.additional_rrs = ntohs(dns->additional_rrs);
    is_query = (header.flags & 0x8000) ? 0 : 1;
    ret_code = is_query ? 0 : (header.flags & 0x0F);

    if(is_query) {
      /* DNS Request */
      if((header.num_queries > 0) && (header.num_queries <= NDPI_MAX_DNS_REQUESTS)
	 && (header.answer_rrs == 0)
	 && (header.authority_rrs == 0)) {
	/* This is a good query */
	is_dns = 1;
      }
    } else {
      /* DNS Reply */
      if((header.num_queries <= NDPI_MAX_DNS_REQUESTS) /* Don't assume that num_queries must be zero */
	 && (((header.answer_rrs > 0) && (header.answer_rrs <= NDPI_MAX_DNS_REQUESTS))
	     || ((header.authority_rrs > 0) && (header.authority_rrs <= NDPI_MAX_DNS_REQUESTS))
	     || ((header.additional_rrs > 0) && (header.additional_rrs <= NDPI_MAX_DNS_REQUESTS)))
	 ) {
	/* This is a good reply */
	is_dns = 1;
      }

      if((header.num_queries <= NDPI_MAX_DNS_REQUESTS)
	 && ((header.answer_rrs == 0)
	     || (header.authority_rrs == 0)
	     || (header.additional_rrs == 0))
	 && (ret_code != 0 /* 0 == OK */)
	 ) {
	/* This is a good reply */
	is_dns = 1;
      }

    }

    if(is_dns) {
      NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "found DNS.\n");      
      ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_DNS, NDPI_REAL_PROTOCOL);
    } else {
      NDPI_LOG(NDPI_PROTOCOL_DNS, ndpi_struct, NDPI_LOG_DEBUG, "exclude DNS.\n");
      NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_DNS);
    }
  }
}
#endif
