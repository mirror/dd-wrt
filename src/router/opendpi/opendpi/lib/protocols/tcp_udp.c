/*
 * tcp_or_udp.c
 *
 * Copyright (C) 2011-13 - ntop.org
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


#include "ndpi_api.h"


u_int ndpi_search_tcp_or_udp_raw(struct ndpi_detection_module_struct *ndpi_struct, 
				 u_int8_t protocol,
				 u_int32_t saddr, u_int32_t daddr, /* host endianess */
				 u_int16_t sport, u_int16_t dport) /* host endianess */
{
  if(protocol == IPPROTO_UDP) {
    if((sport == dport) && (sport == 17500)) {
      return(NDPI_PROTOCOL_DROPBOX);
    }
  }

    /*
      Citrix GotoMeeting (AS16815, AS21866)
      216.115.208.0/20
      216.219.112.0/20
    */
    /* printf("[SSL] %08X / %08X\n", saddr , daddr); */

    if(((saddr & 0xFFFFF000 /* 255.255.240.0 */) == 0xD873D000 /* 216.115.208.0 */)
       || ((daddr & 0xFFFFF000 /* 255.255.240.0 */) == 0xD873D000 /* 216.115.208.0 */)

       || ((saddr & 0xFFFFF000 /* 255.255.240.0 */) == 0xD8DB7000 /* 216.219.112.0 */)
       || ((daddr & 0xFFFFF000 /* 255.255.240.0 */) == 0xD8DB7000 /* 216.219.112.0 */)
       ) {
      return(NDPI_PROTOCOL_CITRIX_ONLINE);
    }

    /*
      Webex
      66.114.160.0/20
    */
    if(((saddr & 0xFFFFF000 /* 255.255.240.0 */) == 0x4272A000 /* 66.114.160.0 */)
       || ((daddr & 0xFFFFF000 /* 255.255.240.0 */) ==0x4272A000 /* 66.114.160.0 */)) {
      return(NDPI_PROTOCOL_WEBEX);
    }

    /*
      Apple (FaceTime, iMessage,...)
      17.0.0.0/8
    */
    if(((saddr & 0xFF000000 /* 255.0.0.0 */) == 0x11000000 /* 17.0.0.0 */)
       || ((daddr & 0xFF000000 /* 255.0.0.0 */) == 0x11000000 /* 17.0.0.0 */)) {
      return(NDPI_SERVICE_APPLE);
    }

    /*
      Dropbox
      108.160.160.0/20
      199.47.216.0/22
    */
    if(((saddr & 0xFFFFF000 /* 255.255.240.0 */) == 0x6CA0A000 /* 108.160.160.0 */) || ((daddr & 0xFFFFF000 /* 255.255.240.0 */) == 0x6CA0A000 /* 108.160.160.0 */)
       || ((saddr & 0xFFFFFC00 /* 255.255.240.0 */) == 0xC72FD800 /* 199.47.216.0 */) || ((daddr & 0xFFFFFC00 /* 255.255.240.0 */) == 0xC72FD800 /* 199.47.216.0 */)
       ) {
      return(NDPI_PROTOCOL_DROPBOX);
    }

    if(((saddr & 0xFFFFF000 /* 255.255.240.0.0 */) == 0x6CA0A000 /* 108.160.160.0 */)
       || ((daddr & 0xFFFFF000 /* 255.255.240.0 */) == 0x6CA0A000 /* 108.160.160.0 */)) {
      return(NDPI_PROTOCOL_DROPBOX);
    }

    /* 
       Skype
       157.56.0.0/14, 157.60.0.0/16, 157.54.0.0/15
    */
    if(
       (((saddr & 0xFF3F0000 /* 255.63.0.0 */) == 0x9D380000 /* 157.56.0.0/ */) || ((daddr & 0xFF3F0000 /* 255.63.0.0 */) == 0x9D380000))
       || (((saddr & 0xFFFF0000 /* 255.255.0.0 */) == 0x9D3C0000 /* 157.60.0.0/ */) || ((daddr & 0xFFFF0000 /* 255.255.0.0 */) == 0x9D3D0000))
       || (((saddr & 0xFF7F0000 /* 255.255.0.0 */) == 0x9D360000 /* 157.54.0.0/ */) || ((daddr & 0xFF7F0000 /* 255.127.0.0 */) == 0x9D360000))
       || (((saddr & 0xFFFE0000 /* 255.254.0.0 */) == 0x9D360000 /* 157.54.0.0/ */) || ((daddr & 0xFFFE0000 /* 255.254.0.0 */) == 0x9D360000))
       ) {
      return(NDPI_PROTOCOL_SKYPE);
    }
  
    /*
      Google
      173.194.0.0/16
    */
    if(((saddr & 0xFFFF0000 /* 255.255.0.0 */) == 0xADC20000  /* 173.194.0.0 */)
       || ((daddr & 0xFFFF0000 /* 255.255.0.0 */) ==0xADC20000 /* 173.194.0.0 */)) {      
      return(NDPI_SERVICE_GOOGLE);
    }

    /*
      Ubuntu One
      91.189.89.0/21 (255.255.248.0)
    */
    if(((saddr & 0xFFFFF800 /* 255.255.248.0 */) == 0x5BBD5900 /* 91.189.89.0 */)
       || ((daddr & 0xFFFFF800 /* 255.255.248.0 */) == 0x5BBD5900 /* 91.189.89.0 */)) {
      return(NDPI_PROTOCOL_UBUNTUONE);
    }    

  return(NDPI_PROTOCOL_UNKNOWN);
}

void ndpi_search_tcp_or_udp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  u_int16_t sport, dport;
  u_int proto;
  struct ndpi_packet_struct *packet = &flow->packet;

  if(packet->udp) sport = ntohs(packet->udp->source), dport = ntohs(packet->udp->dest);
  else if(packet->tcp) sport = ntohs(packet->tcp->source), dport = ntohs(packet->tcp->dest);
  else sport = dport = 0;
  
  if(packet->iph /* IPv4 Only: we need to support packet->iphv6 at some point */) {
    proto = ndpi_search_tcp_or_udp_raw(ndpi_struct,
				       flow->packet.iph ? flow->packet.iph->protocol : flow->packet.iphv6->nexthdr,
				       ntohl(packet->iph->saddr), 
				       ntohl(packet->iph->daddr),
				       sport, dport);
    
    if(proto != NDPI_PROTOCOL_UNKNOWN)
      ndpi_int_add_connection(ndpi_struct, flow, proto, NDPI_REAL_PROTOCOL);
  }
}



