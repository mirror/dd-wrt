/*
 * bittorrent.c
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


#include "ipq_protocols.h"
#ifdef IPOQUE_PROTOCOL_BITTORRENT
#define IPOQUE_PROTOCOL_UNSAFE_DETECTION 	0
#define IPOQUE_PROTOCOL_SAFE_DETECTION 		1

#define IPOQUE_PROTOCOL_PLAIN_DETECTION 	0
#define IPOQUE_PROTOCOL_WEBSEED_DETECTION 	2
static void ipoque_add_connection_as_bittorrent(struct ipoque_detection_module_struct
						*ipoque_struct, const u8 save_detection, const u8 encrypted_connection,
						ipoque_protocol_type_t protocol_type)
{
  ipoque_int_change_protocol(ipoque_struct, IPOQUE_PROTOCOL_BITTORRENT, protocol_type);
}

static u8 ipoque_int_search_bittorrent_tcp_zero(struct ipoque_detection_module_struct
						*ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  struct ipoque_flow_struct *flow = ipoque_struct->flow;
  //  struct ipoque_id_struct *src = ipoque_struct->src;
  //  struct ipoque_id_struct *dst = ipoque_struct->dst;

  u16 a = 0;

  if (packet->payload_packet_len == 1 && packet->payload[0] == 0x13) {
    /* reset stage back to 0 so we will see the next packet here too */
    flow->bittorrent_stage = 0;
    return 0;
  }
  if (flow->packet_counter == 2 && packet->payload_packet_len > 20) {

    if (memcmp(&packet->payload[0], "BitTorrent protocol", 19) == 0) {
      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT,
			 ipoque_struct, IPQ_LOG_TRACE, "BT: plain BitTorrent protocol detected\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					  IPOQUE_REAL_PROTOCOL);
      return 1;
    }
  }


  if (packet->payload_packet_len > 20) {
    /* test for match 0x13+"BitTorrent protocol" */
    if (packet->payload[0] == 0x13) {
      if (memcmp(&packet->payload[1], "BitTorrent protocol", 19) == 0) {
	IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT,
			   ipoque_struct, IPQ_LOG_TRACE, "BT: plain BitTorrent protocol detected\n");
	ipoque_add_connection_as_bittorrent(ipoque_struct,
					    IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					    IPOQUE_REAL_PROTOCOL);
	return 1;
      }
    }
  }

  if (packet->payload_packet_len > 23 && memcmp(packet->payload, "GET /webseed?info_hash=", 23) == 0) {
    IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
		       IPQ_LOG_TRACE, "BT: plain webseed BitTorrent protocol detected\n");
    ipoque_add_connection_as_bittorrent(ipoque_struct,
					IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_WEBSEED_DETECTION,
					IPOQUE_CORRELATED_PROTOCOL);
    return 1;
  }
  /* seen Azureus as server for webseed, possibly other servers existing, to implement */
  /* is Server: hypertracker Bittorrent? */
  /* no asymmetric detection possible for answer of pattern "GET /data?fid=". */
  if (packet->payload_packet_len > 60
      && memcmp(packet->payload, "GET /data?fid=", 14) == 0 && memcmp(&packet->payload[54], "&size=", 6) == 0) {
    IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
		       IPQ_LOG_TRACE, "BT: plain Bitcomet persistent seed protocol detected\n");
    ipoque_add_connection_as_bittorrent(ipoque_struct,
					IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_WEBSEED_DETECTION,
					IPOQUE_CORRELATED_PROTOCOL);
    return 1;
  }


  if (packet->payload_packet_len > 90 && (memcmp(packet->payload, "GET ", 4) == 0
					  || memcmp(packet->payload, "POST ", 5) == 0)) {
    const u8 *ptr = &packet->payload[4];
    u16 len = packet->payload_packet_len - 4;
    a = 0;


    /* parse complete get packet here into line structure elements */
    ipq_parse_packet_line_info(ipoque_struct);
    /* answer to this pattern is HTTP....Server: hypertracker */
    if (packet->user_agent_line.ptr != NULL
	&& ((packet->user_agent_line.len > 8 && memcmp(packet->user_agent_line.ptr, "Azureus ", 8) == 0)
	    || (packet->user_agent_line.len >= 10 && memcmp(packet->user_agent_line.ptr, "BitTorrent", 10) == 0)
	    || (packet->user_agent_line.len >= 11 && memcmp(packet->user_agent_line.ptr, "BTWebClient", 11) == 0))) {
      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
			 IPQ_LOG_TRACE, "Azureus /Bittorrent user agent line detected\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_WEBSEED_DETECTION,
					  IPOQUE_CORRELATED_PROTOCOL);
      return 1;
    }

    if (packet->user_agent_line.ptr != NULL
	&& (packet->user_agent_line.len >= 9 && memcmp(packet->user_agent_line.ptr, "Shareaza ", 9) == 0)
	&& (packet->parsed_lines > 8 && packet->line[8].ptr != 0
	    && packet->line[8].len >= 9 && memcmp(packet->line[8].ptr, "X-Queue: ", 9) == 0)) {
      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
			 IPQ_LOG_TRACE, "Bittorrent Shareaza detected.\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_WEBSEED_DETECTION,
					  IPOQUE_CORRELATED_PROTOCOL);
      return 1;
    }

    /* this is a self built client, not possible to catch asymmetrically */
    if ((packet->parsed_lines == 10 || (packet->parsed_lines == 11 && packet->line[11].len == 0))
	&& packet->user_agent_line.ptr != NULL
	&& packet->user_agent_line.len > 12
	&& ipq_mem_cmp(packet->user_agent_line.ptr, "Mozilla/4.0 ",
		       12) == 0
	&& packet->host_line.ptr != NULL
	&& packet->host_line.len >= 7
	&& packet->line[2].ptr != NULL
	&& packet->line[2].len > 14
	&& ipq_mem_cmp(packet->line[2].ptr, "Keep-Alive: 300", 15) == 0
	&& packet->line[3].ptr != NULL
	&& packet->line[3].len > 21
	&& ipq_mem_cmp(packet->line[3].ptr, "Connection: Keep-alive", 22) == 0
	&& packet->line[4].ptr != NULL
	&& packet->line[4].len > 10
	&& (ipq_mem_cmp(packet->line[4].ptr, "Accpet: */*", 11) == 0
	    || ipq_mem_cmp(packet->line[4].ptr, "Accept: */*", 11) == 0)

	&& packet->line[5].ptr != NULL
	&& packet->line[5].len > 12
	&& ipq_mem_cmp(packet->line[5].ptr, "Range: bytes=", 13) == 0
	&& packet->line[7].ptr != NULL
	&& packet->line[7].len > 15
	&& ipq_mem_cmp(packet->line[7].ptr, "Pragma: no-cache", 16) == 0
	&& packet->line[8].ptr != NULL
	&& packet->line[8].len > 22 && ipq_mem_cmp(packet->line[8].ptr, "Cache-Control: no-cache", 23) == 0) {

      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_TRACE, "Bitcomet LTS detected\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_UNSAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					  IPOQUE_CORRELATED_PROTOCOL);
      return 1;

    }

    /* FlashGet pattern */
    if (packet->parsed_lines == 8
	&& packet->user_agent_line.ptr != NULL
	&& packet->user_agent_line.len > (sizeof("Mozilla/4.0 (compatible; MSIE 6.0;") - 1)
	&& memcmp(packet->user_agent_line.ptr, "Mozilla/4.0 (compatible; MSIE 6.0;",
		  sizeof("Mozilla/4.0 (compatible; MSIE 6.0;") - 1) == 0
	&& packet->host_line.ptr != NULL
	&& packet->host_line.len >= 7
	&& packet->line[2].ptr != NULL
	&& packet->line[2].len == 11
	&& memcmp(packet->line[2].ptr, "Accept: */*", 11) == 0
	&& packet->line[3].ptr != NULL && packet->line[3].len >= (sizeof("Referer: ") - 1)
	&& ipq_mem_cmp(packet->line[3].ptr, "Referer: ", sizeof("Referer: ") - 1) == 0
	&& packet->line[5].ptr != NULL
	&& packet->line[5].len > 13
	&& ipq_mem_cmp(packet->line[5].ptr, "Range: bytes=", 13) == 0
	&& packet->line[6].ptr != NULL
	&& packet->line[6].len > 21 && ipq_mem_cmp(packet->line[6].ptr, "Connection: Keep-Alive", 22) == 0) {

      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_TRACE, "FlashGet detected\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_UNSAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					  IPOQUE_CORRELATED_PROTOCOL);
      return 1;

    }
    if (packet->parsed_lines == 7
	&& packet->user_agent_line.ptr != NULL
	&& packet->user_agent_line.len > (sizeof("Mozilla/4.0 (compatible; MSIE 6.0;") - 1)
	&& memcmp(packet->user_agent_line.ptr, "Mozilla/4.0 (compatible; MSIE 6.0;",
		  sizeof("Mozilla/4.0 (compatible; MSIE 6.0;") - 1) == 0
	&& packet->host_line.ptr != NULL
	&& packet->host_line.len >= 7
	&& packet->line[2].ptr != NULL
	&& packet->line[2].len == 11
	&& memcmp(packet->line[2].ptr, "Accept: */*", 11) == 0
	&& packet->line[3].ptr != NULL && packet->line[3].len >= (sizeof("Referer: ") - 1)
	&& ipq_mem_cmp(packet->line[3].ptr, "Referer: ", sizeof("Referer: ") - 1) == 0
	&& packet->line[5].ptr != NULL
	&& packet->line[5].len > 21 && ipq_mem_cmp(packet->line[5].ptr, "Connection: Keep-Alive", 22) == 0) {

      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_TRACE, "FlashGet detected\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_UNSAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					  IPOQUE_CORRELATED_PROTOCOL);
      return 1;

    }

    /* answer to this pattern is not possible to implement asymmetrically */
    while (1) {
      if (len < 50 || ptr[0] == 0x0d) {
	goto ipq_end_bt_tracker_check;
      }
      if (memcmp(ptr, "info_hash=", 10) == 0) {
	break;
      }
      len--;
      ptr++;
    }

    IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
		       IPQ_LOG_TRACE, " BT stat: tracker info hash found\n");

    /* len is > 50, so save operation here */
    len -= 10;
    ptr += 10;

    /* parse bt hash */
    for (a = 0; a < 20; a++) {
      if (len < 3) {
	goto ipq_end_bt_tracker_check;
      }
      if (*ptr == '%') {
	u8 x1 = 0xFF;
	u8 x2 = 0xFF;


	if (ptr[1] >= '0' && ptr[1] <= '9') {
	  x1 = ptr[1] - '0';
	}
	if (ptr[1] >= 'a' && ptr[1] <= 'f') {
	  x1 = 10 + ptr[1] - 'a';
	}
	if (ptr[1] >= 'A' && ptr[1] <= 'F') {
	  x1 = 10 + ptr[1] - 'A';
	}

	if (ptr[2] >= '0' && ptr[2] <= '9') {
	  x2 = ptr[2] - '0';
	}
	if (ptr[2] >= 'a' && ptr[2] <= 'f') {
	  x2 = 10 + ptr[2] - 'a';
	}
	if (ptr[2] >= 'A' && ptr[2] <= 'F') {
	  x2 = 10 + ptr[2] - 'A';
	}

	if (x1 == 0xFF || x2 == 0xFF) {
	  goto ipq_end_bt_tracker_check;
	}
	ptr += 3;
	len -= 3;
      } else if (*ptr >= 32 && *ptr < 127) {
	ptr++;
	len--;
      } else {
	goto ipq_end_bt_tracker_check;
      }
    }

    IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
		       IPQ_LOG_TRACE, " BT stat: tracker info hash parsed\n");
    ipoque_add_connection_as_bittorrent(ipoque_struct,
					IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					IPOQUE_CORRELATED_PROTOCOL);
    return 1;
  }

 ipq_end_bt_tracker_check:

  if (packet->payload_packet_len == 80) {
    /* Warez 80 Bytes Packet
     * +----------------+---------------+-----------------+-----------------+
     * |20 BytesPattern | 32 Bytes Value| 12 BytesPattern | 16 Bytes Data   |
     * +----------------+---------------+-----------------+-----------------+
     * 20 BytesPattern : 4c 00 00 00 ff ff ff ff 57 00 00 00 00 00 00 00 20 00 00 00
     * 12 BytesPattern : 28 23 00 00 01 00 00 00 10 00 00 00
     * */
    static const char pattern_20_bytes[20] = { 0x4c, 0x00, 0x00, 0x00, 0xff,
					       0xff, 0xff, 0xff, 0x57, 0x00,
					       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00
    };
    static const char pattern_12_bytes[12] = { 0x28, 0x23, 0x00, 0x00, 0x01,
					       0x00, 0x00, 0x00, 0x10, 0x00,
					       0x00, 0x00
    };

    /* did not see this pattern anywhere */
    if ((memcmp(&packet->payload[0], pattern_20_bytes, 20) == 0)
	&& (memcmp(&packet->payload[52], pattern_12_bytes, 12) == 0)) {
      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct,
			 IPQ_LOG_TRACE, "BT: Warez - Plain BitTorrent protocol detected\n");
      ipoque_add_connection_as_bittorrent(ipoque_struct,
					  IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
					  IPOQUE_REAL_PROTOCOL);
      return 1;
    }
  }

  else if (packet->payload_packet_len > 50) {
    if (memcmp(packet->payload, "GET", 3) == 0) {

      ipq_parse_packet_line_info(ipoque_struct);
      /* haven't fount this pattern anywhere */
      if (packet->host_line.ptr != NULL
	  && packet->host_line.len >= 9 && memcmp(packet->host_line.ptr, "ip2p.com:", 9) == 0) {
	IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT,
			   ipoque_struct, IPQ_LOG_TRACE,
			   "BT: Warez - Plain BitTorrent protocol detected due to Host: ip2p.com: pattern\n");
	ipoque_add_connection_as_bittorrent(ipoque_struct,
					    IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_WEBSEED_DETECTION,
					    IPOQUE_CORRELATED_PROTOCOL);
	return 1;
      }
    }
  }
  return 0;
}


/*Search for BitTorrent commands*/
static void ipoque_int_search_bittorrent_tcp(struct ipoque_detection_module_struct
					     *ipoque_struct)
{

  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  struct ipoque_flow_struct *flow = ipoque_struct->flow;
  if (packet->payload_packet_len == 0) {
    return;
  }

  if (flow->bittorrent_stage == 0 && packet->payload_packet_len != 0) {
    /* exclude stage 0 detection from next run */
    flow->bittorrent_stage = 1;
    if (ipoque_int_search_bittorrent_tcp_zero(ipoque_struct) != 0) {
      IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_DEBUG,
			 "stage 0 has detected something, returning\n");
      return;
    }

    IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT, ipoque_struct, IPQ_LOG_DEBUG,
		       "stage 0 has no direct detection, fall through\n");
  }
  return;
}

void ipoque_search_bittorrent(struct ipoque_detection_module_struct
			      *ipoque_struct)
{
  struct ipoque_packet_struct *packet = &ipoque_struct->packet;
  if (packet->detected_protocol_stack[0] != IPOQUE_PROTOCOL_BITTORRENT) {
    /* check for tcp retransmission here */

    if ((packet->tcp != NULL)
	&& (packet->tcp_retransmission == 0 || packet->num_retried_bytes)) {
      ipoque_int_search_bittorrent_tcp(ipoque_struct);
    }
#ifdef HAVE_NTOP
    else if(packet->udp != NULL) {
      struct ipoque_flow_struct *flow = ipoque_struct->flow;

      flow->bittorrent_stage++;

      if(flow->bittorrent_stage < 10) {
	if(packet->payload_packet_len > 19 /* min size */) {
	  char *begin;
	
	  if(ntop_strnstr(packet->payload, ":target20:", packet->payload_packet_len)
	     || ntop_strnstr(packet->payload, ":find_node1:", packet->payload_packet_len)
	     || ntop_strnstr(packet->payload, "d1:ad2:id20:", packet->payload_packet_len)) {
	  bittorrent_found:
	    IPQ_LOG_BITTORRENT(IPOQUE_PROTOCOL_BITTORRENT,
			       ipoque_struct, IPQ_LOG_TRACE, "BT: plain BitTorrent protocol detected\n");
	    ipoque_add_connection_as_bittorrent(ipoque_struct,
						IPOQUE_PROTOCOL_SAFE_DETECTION, IPOQUE_PROTOCOL_PLAIN_DETECTION,
						IPOQUE_REAL_PROTOCOL);
	    return;
	  } else if((begin = memchr(packet->payload, 'B',  packet->payload_packet_len-19)) != NULL) {
	    u_long offset = (u_long)begin - (u_long)packet->payload;
	    
	    if((packet->payload_packet_len-19) > offset) {
	      if(memcmp(begin, "BitTorrent protocol", 19) == 0) {
		goto bittorrent_found;
	      }
	    }
	  }
	}

	return;
      }

      IPOQUE_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, IPOQUE_PROTOCOL_BITTORRENT);
    }
#endif
    

  }
}
#endif
