/*
 * ssl.c
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


#include "ndpi_utils.h"

#ifdef NDPI_PROTOCOL_SSL

#define NDPI_MAX_SSL_REQUEST_SIZE 10000

static void ndpi_int_ssl_add_connection(struct ndpi_detection_module_struct *ndpi_struct,
					struct ndpi_flow_struct *flow, u_int32_t protocol)
{
  if (protocol != NDPI_PROTOCOL_SSL) {
    ndpi_int_add_connection(ndpi_struct, flow, protocol, NDPI_CORRELATED_PROTOCOL);
  } else {
    ndpi_int_add_connection(ndpi_struct, flow, protocol, NDPI_REAL_PROTOCOL);
  }
}

/* Can't call libc functions from kernel space, define some stub instead */

#define ndpi_isalpha(ch) (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))
#define ndpi_isdigit(ch) ((ch) >= '0' && (ch) <= '9')
#define ndpi_isspace(ch) (((ch) >= '\t' && (ch) <= '\r') || ((ch) == ' '))
#define ndpi_isprint(ch) ((ch) >= 0x20 && (ch) <= 0x7e)
#define ndpi_ispunct(ch) (((ch) >= '!' && (ch) <= '/') || \
                     ((ch) >= ':' && (ch) <= '@') || \
                     ((ch) >= '[' && (ch) <= '`') || \
                     ((ch) >= '{' && (ch) <= '~')) 

static void stripCertificateTrailer(char *buffer, int buffer_len) {
  int i;

  for(i=0; i<buffer_len; i++) {
    if((buffer[i] != '.')
       && (buffer[i] != '-')
       && (!ndpi_isalpha(buffer[i]))
       && (!ndpi_isdigit(buffer[i])))
      buffer[i] = '\0';
    break;
  }
}

int getSSLcertificate(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow,
		      char *buffer, int buffer_len) {
  struct ndpi_packet_struct *packet = &flow->packet;

  /* Nothing matched so far: let's decode the certificate with some heuristics */
  if(packet->payload[0] == 0x16 /* Handshake */) {
    u_int16_t total_len  = packet->payload[4] + 5 /* SSL Header */;
    u_int8_t handshake_protocol = packet->payload[5];

    memset(buffer, 0, buffer_len);

    if(handshake_protocol == 0x02 /* Server Hello */) {
      int i;

      for(i=total_len; i < packet->payload_packet_len-3; i++) {
	if((packet->payload[i] == 0x04)
	   && (packet->payload[i+1] == 0x03)
	   && (packet->payload[i+2] == 0x0c)) {
	  u_int8_t server_len = packet->payload[i+3];

	  if(server_len+i+3 < packet->payload_packet_len) {
	    char *server_name = (char*)&packet->payload[i+4];
	    u_int8_t begin = 0, len, j, num_dots;

	    while(begin < server_len) {
	      if(!ndpi_isprint(server_name[begin]))
		begin++;
	      else
		break;
	    }

	    len = ndpi_min(server_len-begin, buffer_len-1);
	    strncpy(buffer, &server_name[begin], len);
	    buffer[len] = '\0';

	    /* We now have to check if this looks like an IP address or host name */
	    for(j=0, num_dots = 0; j<len; j++) {
	      if(!ndpi_isprint((buffer[j]))) {
		num_dots = 0; /* This is not what we look for */
		break;
	      } else if(buffer[j] == '.') {
		num_dots++;
		if(num_dots >=2) break;
	      }
	    }

	    if(num_dots >= 2) {
	      stripCertificateTrailer(buffer, buffer_len);

	      return(1 /* Server Certificate */);
	    }
	  }
	}
      }
    } else if(handshake_protocol == 0x01 /* Client Hello */) {
      u_int offset, base_offset = 43;
      u_int16_t session_id_len = packet->payload[base_offset];
      if((session_id_len+base_offset+2) >= total_len) { 
	u_int16_t cypher_len =  packet->payload[session_id_len+base_offset+2];

	offset = base_offset + session_id_len + cypher_len + 2;

	if(offset < total_len) {
	  u_int16_t compression_len;
	  u_int16_t extensions_len;

	  compression_len = packet->payload[offset+1];
	  offset += compression_len + 3;
	  extensions_len = packet->payload[offset];

	  if((extensions_len+offset) < total_len) {
	    u_int16_t extension_offset = 1; /* Move to the first extension */

	    while(extension_offset < extensions_len) {
	      u_int16_t extension_id, extension_len;

	      memcpy(&extension_id, &packet->payload[offset+extension_offset], 2);
	      extension_offset += 2;

	      memcpy(&extension_len, &packet->payload[offset+extension_offset], 2);
	      extension_offset += 2;

	      extension_id = ntohs(extension_id), extension_len = ntohs(extension_len);

	      if(extension_id == 0) {
		u_int begin = 0,len;
		char *server_name = (char*)&packet->payload[offset+extension_offset];

		while(begin < extension_len) {
		  if((!ndpi_isprint(server_name[begin]))
		     || ndpi_ispunct(server_name[begin])
		     || ndpi_isspace(server_name[begin]))
		    begin++;
		  else
		    break;
		}

		len = ndpi_min(extension_len-begin, buffer_len-1);
		strncpy(buffer, &server_name[begin], len);
		buffer[len] = '\0';
		stripCertificateTrailer(buffer, buffer_len);

		/* We're happy now */
		return(2 /* Client Certificate */);
	      }

	      extension_offset += extension_len;
	    }
	  }
	}
      }
    }
  }

  return(0); /* Not found */
}

int sslDetectProtocolFromCertificate(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow) {
  struct ndpi_packet_struct *packet = &flow->packet;

  if(!packet->iph /* IPv4 */) return(-1);

  if((packet->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN)
     || (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_SSL)) {
    char certificate[64];
    int rc = getSSLcertificate(ndpi_struct, flow, certificate, sizeof(certificate));
    
    if(rc > 0) {
      /* printf("***** [SSL] %s\n", certificate); */
      if(matchStringProtocol(ndpi_struct, flow, certificate, strlen(certificate)) != -1)
	return(rc); /* Fix courtesy of Gianluca Costa <g.costa@xplico.org> */
    }
  }

  return(0);
}

static void ssl_mark_and_payload_search_for_other_protocols(struct
							    ndpi_detection_module_struct
							    *ndpi_struct, struct ndpi_flow_struct *flow)
{
#if defined(NDPI_PROTOCOL_SOFTETHER) || defined(NDPI_PROTOCOL_MEEBO)|| defined(NDPI_PROTOCOL_TOR) || defined(NDPI_PROTOCOL_VPN_X) || defined(NDPI_PROTOCOL_UNENCRYPED_JABBER) || defined (NDPI_PROTOCOL_OOVOO) || defined (NDPI_PROTOCOL_ISKOOT) || defined (NDPI_PROTOCOL_OSCAR) || defined (NDPI_PROTOCOL_ITUNES) || defined (NDPI_PROTOCOL_GMAIL)

  struct ndpi_packet_struct *packet = &flow->packet;
#ifdef NDPI_PROTOCOL_ISKOOT
  
#endif
  //      struct ndpi_id_struct         *src=flow->src;
  //      struct ndpi_id_struct         *dst=flow->dst;
  u_int32_t a;
  u_int32_t end;
#if defined(NDPI_PROTOCOL_UNENCRYPED_JABBER)
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_UNENCRYPED_JABBER) != 0)
    goto check_for_ssl_payload;
#endif
#if defined(NDPI_PROTOCOL_OSCAR)
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_OSCAR) != 0)
    goto check_for_ssl_payload;
#endif
#if defined(NDPI_PROTOCOL_GADUGADU)
  if (NDPI_COMPARE_PROTOCOL_TO_BITMASK(ndpi_struct->detection_bitmask, NDPI_PROTOCOL_GADUGADU) != 0)
    goto check_for_ssl_payload;
#endif
  goto no_check_for_ssl_payload;

 check_for_ssl_payload:
  end = packet->payload_packet_len - 20;
  for (a = 5; a < end; a++) {
#ifdef NDPI_PROTOCOL_UNENCRYPED_JABBER
    if (packet->payload[a] == 't') {
      if (memcmp(&packet->payload[a], "talk.google.com", 15) == 0) {
	NDPI_LOG(NDPI_PROTOCOL_UNENCRYPED_JABBER, ndpi_struct, NDPI_LOG_DEBUG, "ssl jabber packet match\n");
	if (NDPI_COMPARE_PROTOCOL_TO_BITMASK
	    (ndpi_struct->detection_bitmask, NDPI_PROTOCOL_UNENCRYPED_JABBER) != 0) {
	  ndpi_int_ssl_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_UNENCRYPED_JABBER);
	  return;
	}
      }
    }
#endif
#ifdef NDPI_PROTOCOL_OSCAR
    if (packet->payload[a] == 'A' || packet->payload[a] == 'k' || packet->payload[a] == 'c'
	|| packet->payload[a] == 'h') {
      if (((a + 19) < packet->payload_packet_len && memcmp(&packet->payload[a], "America Online Inc.", 19) == 0)
	  //                        || (end - c > 3 memcmp (&packet->payload[c],"AOL", 3) == 0 )
	  //                        || (end - c > 7 && memcmp (&packet->payload[c], "AOL LLC", 7) == 0)
	  || ((a + 15) < packet->payload_packet_len && memcmp(&packet->payload[a], "kdc.uas.aol.com", 15) == 0)
	  || ((a + 14) < packet->payload_packet_len && memcmp(&packet->payload[a], "corehc@aol.net", 14) == 0)
	  || ((a + 41) < packet->payload_packet_len
	      && memcmp(&packet->payload[a], "http://crl.aol.com/AOLMSPKI/aolServerCert", 41) == 0)
	  || ((a + 28) < packet->payload_packet_len
	      && memcmp(&packet->payload[a], "http://ocsp.web.aol.com/ocsp", 28) == 0)
	  || ((a + 32) < packet->payload_packet_len
	      && memcmp(&packet->payload[a], "http://pki-info.aol.com/AOLMSPKI", 32) == 0)) {
	NDPI_LOG(NDPI_PROTOCOL_OSCAR, ndpi_struct, NDPI_LOG_DEBUG, "OSCAR SERVER SSL DETECTED\n");

	if (flow->dst != NULL && packet->payload_packet_len > 75) {
	  memcpy(flow->dst->oscar_ssl_session_id, &packet->payload[44], 32);
	  flow->dst->oscar_ssl_session_id[32] = '\0';
	  flow->dst->oscar_last_safe_access_time = packet->tick_timestamp;
	}

	ndpi_int_ssl_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_OSCAR);
	return;
      }
    }

    if (packet->payload[a] == 'm' || packet->payload[a] == 's') {
      if ((a + 21) < packet->payload_packet_len &&
	  (memcmp(&packet->payload[a], "my.screenname.aol.com", 21) == 0
	   || memcmp(&packet->payload[a], "sns-static.aolcdn.com", 21) == 0)) {
	NDPI_LOG(NDPI_PROTOCOL_OSCAR, ndpi_struct, NDPI_LOG_DEBUG, "OSCAR SERVER SSL DETECTED\n");
	ndpi_int_ssl_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_OSCAR);
	return;
      }
    }
#endif



  }
 no_check_for_ssl_payload:
#endif
  NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "found ssl connection.\n");

  sslDetectProtocolFromCertificate(ndpi_struct, flow);

  if(packet->iph /* IPv4 Only: we need to support packet->iphv6 at some point */) {
    if((packet->detected_protocol_stack[0] == NDPI_PROTOCOL_UNKNOWN)
       || (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_SSL)) {
      /*
	Citrix GotoMeeting (AS16815, AS21866)
	216.115.208.0/20
	216.219.112.0/20
      */

      /* printf("[SSL] %08X / %08X\n", ntohl(packet->iph->saddr) , ntohl(packet->iph->daddr)); */

      if(((ntohl(packet->iph->saddr) & 0xFFFFF000 /* 255.255.240.0 */) == 0xD873D000 /* 216.115.208.0 */)
	 || ((ntohl(packet->iph->daddr) & 0xFFFFF000 /* 255.255.240.0 */) == 0xD873D000 /* 216.115.208.0 */)

	 || ((ntohl(packet->iph->saddr) & 0xFFFFF000 /* 255.255.240.0 */) == 0xD8DB7000 /* 216.219.112.0 */)
	 || ((ntohl(packet->iph->daddr) & 0xFFFFF000 /* 255.255.240.0 */) == 0xD8DB7000 /* 216.219.112.0 */)
	 ) {
	ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_CITRIX_ONLINE, NDPI_REAL_PROTOCOL);
	return;
      }

      /*
	Apple (FaceTime, iMessage,...)
	17.0.0.0/8
      */
      if(((ntohl(packet->iph->saddr) & 0xFF000000 /* 255.0.0.0 */) == 0x11000000 /* 17.0.0.0 */)
	 || ((ntohl(packet->iph->daddr) & 0xFF000000 /* 255.0.0.0 */) == 0x11000000 /* 17.0.0.0 */)) {
	ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_APPLE, NDPI_REAL_PROTOCOL);
	return;
      }

      /*
	Webex
	66.114.160.0/20
      */
      if(((ntohl(packet->iph->saddr) & 0xFFFFF000 /* 255.255.240.0 */) == 0x4272A000 /* 66.114.160.0 */)
	 || ((ntohl(packet->iph->daddr) & 0xFFFFF000 /* 255.255.240.0 */) ==0x4272A000 /* 66.114.160.0 */)) {
	ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WEBEX, NDPI_REAL_PROTOCOL);
	return;
      }

      /*
	Google
	173.194.0.0/16
      */
      if(((ntohl(packet->iph->saddr) & 0xFFFF0000 /* 255.255.0.0 */) == 0xADC20000 /* 66.114.160.0 */)
	 || ((ntohl(packet->iph->daddr) & 0xFFFF0000 /* 255.255.0.0 */) ==0xDC20000 /* 66.114.160.0 */)) {
	ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_GOOGLE, NDPI_REAL_PROTOCOL);
	return;
      }
    }
  }

  ndpi_int_ssl_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_SSL);
}


static u_int8_t ndpi_search_sslv3_direction1(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{

  struct ndpi_packet_struct *packet = &flow->packet;
  //  
  //      struct ndpi_id_struct         *src=flow->src;
  //      struct ndpi_id_struct         *dst=flow->dst;


  if (packet->payload_packet_len >= 5 && packet->payload[0] == 0x16 && packet->payload[1] == 0x03
      && (packet->payload[2] == 0x00 || packet->payload[2] == 0x01 || packet->payload[2] == 0x02)) {
    u_int32_t temp;
    NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "search sslv3\n");
    // SSLv3 Record
    if (packet->payload_packet_len >= 1300) {
      return 1;
    }
    temp = ntohs(get_u_int16_t(packet->payload, 3)) + 5;
    NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "temp = %u.\n", temp);
    if (packet->payload_packet_len == temp
	|| (temp < packet->payload_packet_len && packet->payload_packet_len > 500)) {
      return 1;
    }

    if (packet->payload_packet_len < temp && temp < 5000 && packet->payload_packet_len > 9) {
      /* the server hello may be split into small packets */
      u_int32_t cert_start;

      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG,
	       "maybe SSLv3 server hello split into smaller packets\n");

      /* lets hope at least the server hello and the start of the certificate block are in the first packet */
      cert_start = ntohs(get_u_int16_t(packet->payload, 7)) + 5 + 4;
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "suspected start of certificate: %u\n",
	       cert_start);

      if (cert_start < packet->payload_packet_len && packet->payload[cert_start] == 0x0b) {
	NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG,
		 "found 0x0b at suspected start of certificate block\n");
	return 2;
      }
    }

    if ((packet->payload_packet_len > temp && packet->payload_packet_len > 100) && packet->payload_packet_len > 9) {
      /* the server hello may be split into small packets and the certificate has its own SSL Record
       * so temp contains only the length for the first ServerHello block */
      u_int32_t cert_start;

      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG,
	       "maybe SSLv3 server hello split into smaller packets but with seperate record for the certificate\n");

      /* lets hope at least the server hello record and the start of the certificate record are in the first packet */
      cert_start = ntohs(get_u_int16_t(packet->payload, 7)) + 5 + 5 + 4;
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "suspected start of certificate: %u\n",
	       cert_start);

      if (cert_start < packet->payload_packet_len && packet->payload[cert_start] == 0x0b) {
	NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG,
		 "found 0x0b at suspected start of certificate block\n");
	return 2;
      }
    }


    if (packet->payload_packet_len >= temp + 5 && (packet->payload[temp] == 0x14 || packet->payload[temp] == 0x16)
	&& packet->payload[temp + 1] == 0x03) {
      u_int32_t temp2 = ntohs(get_u_int16_t(packet->payload, temp + 3)) + 5;
      if (temp + temp2 > NDPI_MAX_SSL_REQUEST_SIZE) {
	return 1;
      }
      temp += temp2;
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "temp = %u.\n", temp);
      if (packet->payload_packet_len == temp) {
	return 1;
      }
      if (packet->payload_packet_len >= temp + 5 &&
	  packet->payload[temp] == 0x16 && packet->payload[temp + 1] == 0x03) {
	temp2 = ntohs(get_u_int16_t(packet->payload, temp + 3)) + 5;
	if (temp + temp2 > NDPI_MAX_SSL_REQUEST_SIZE) {
	  return 1;
	}
	temp += temp2;
	NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "temp = %u.\n", temp);
	if (packet->payload_packet_len == temp) {
	  return 1;
	}
	if (packet->payload_packet_len >= temp + 5 &&
	    packet->payload[temp] == 0x16 && packet->payload[temp + 1] == 0x03) {
	  temp2 = ntohs(get_u_int16_t(packet->payload, temp + 3)) + 5;
	  if (temp + temp2 > NDPI_MAX_SSL_REQUEST_SIZE) {
	    return 1;
	  }
	  temp += temp2;
	  NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "temp = %u.\n", temp);
	  if (temp == packet->payload_packet_len) {
	    return 1;
	  }
	}

      }


    }

  }
  return 0;

}

static void ndpi_search_ssl_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;
  
  //      struct ndpi_id_struct         *src=flow->src;
  //      struct ndpi_id_struct         *dst=flow->dst;

  u_int8_t ret;

  if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_SSL) {
    if (flow->l4.tcp.ssl_stage == 3 && packet->payload_packet_len > 20 && flow->packet_counter < 5) {
      /* this should only happen, when we detected SSL with a packet that had parts of the certificate in subsequent packets
       * so go on checking for certificate patterns for a couple more packets
       */
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG,
	       "ssl flow but check another packet for patterns\n");
      ssl_mark_and_payload_search_for_other_protocols(ndpi_struct, flow);
      if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_SSL) {
	/* still ssl so check another packet */
	return;
      } else {
	/* protocol has changed so we are done */
	return;
      }
    }
    return;
  }

  NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "search ssl\n");

  {
    /* Check if this is whatsapp first (this proto runs over port 443) */
    char whatsapp_pattern[] = { 0x57, 0x41, 0x01, 0x01, 0x00 };

    if((packet->payload_packet_len > 5)
       && (memcmp(packet->payload, whatsapp_pattern, sizeof(whatsapp_pattern)) == 0)) {
      ndpi_int_add_connection(ndpi_struct, flow, NDPI_PROTOCOL_WHATSAPP, NDPI_REAL_PROTOCOL);
      return;
    } else {
      /* No whatsapp, let's try SSL */
      if(sslDetectProtocolFromCertificate(ndpi_struct, flow) > 0)
	return;
    }
  }

  if (packet->payload_packet_len > 40 && flow->l4.tcp.ssl_stage == 0) {
    NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "first ssl packet\n");
    // SSLv2 Record
    if (packet->payload[2] == 0x01 && packet->payload[3] == 0x03
	&& (packet->payload[4] == 0x00 || packet->payload[4] == 0x01 || packet->payload[4] == 0x02)
	&& (packet->payload_packet_len - packet->payload[1] == 2)) {
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "sslv2 len match\n");
      flow->l4.tcp.ssl_stage = 1 + packet->packet_direction;
      return;
    }

    if (packet->payload[0] == 0x16 && packet->payload[1] == 0x03
	&& (packet->payload[2] == 0x00 || packet->payload[2] == 0x01 || packet->payload[2] == 0x02)
	&& (packet->payload_packet_len - ntohs(get_u_int16_t(packet->payload, 3)) == 5)) {
      // SSLv3 Record
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "sslv3 len match\n");
      flow->l4.tcp.ssl_stage = 1 + packet->packet_direction;
      return;
    }
  }

  if (packet->payload_packet_len > 40 &&
      flow->l4.tcp.ssl_stage == 1 + packet->packet_direction
      && flow->packet_direction_counter[packet->packet_direction] < 5) {
    return;
  }

  if (packet->payload_packet_len > 40 && flow->l4.tcp.ssl_stage == 2 - packet->packet_direction) {
    NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "second ssl packet\n");
    // SSLv2 Record
    if (packet->payload[2] == 0x01 && packet->payload[3] == 0x03
	&& (packet->payload[4] == 0x00 || packet->payload[4] == 0x01 || packet->payload[4] == 0x02)
	&& (packet->payload_packet_len - 2) >= packet->payload[1]) {
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "sslv2 server len match\n");
      ssl_mark_and_payload_search_for_other_protocols(ndpi_struct, flow);
      return;
    }

    ret = ndpi_search_sslv3_direction1(ndpi_struct, flow);
    if (ret == 1) {
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "sslv3 server len match\n");
      ssl_mark_and_payload_search_for_other_protocols(ndpi_struct, flow);
      return;
    } else if (ret == 2) {
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG,
	       "sslv3 server len match with split packet -> check some more packets for SSL patterns\n");
      ssl_mark_and_payload_search_for_other_protocols(ndpi_struct, flow);
      if (packet->detected_protocol_stack[0] == NDPI_PROTOCOL_SSL) {
	flow->l4.tcp.ssl_stage = 3;
      }
      return;
    }

    if (packet->payload_packet_len > 40 && flow->packet_direction_counter[packet->packet_direction] < 5) {
      NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "need next packet\n");
      return;
    }
  }

  NDPI_LOG(NDPI_PROTOCOL_SSL, ndpi_struct, NDPI_LOG_DEBUG, "exclude ssl\n");
  NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_SSL);
  return;
}
#endif
