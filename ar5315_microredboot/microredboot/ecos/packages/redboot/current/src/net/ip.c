//==========================================================================
//
//      net/ip.c
//
//      Stand-alone IP networking support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-07-14
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <net/net.h>

#ifndef CYGDAT_REDBOOT_DEFAULT_IP_ADDR
# define CYGDAT_REDBOOT_DEFAULT_IP_ADDR 0, 0, 0, 0
#endif
#ifndef CYGDAT_REDBOOT_DEFAULT_IP_ADDR_MASK
# define CYGDAT_REDBOOT_DEFAULT_IP_ADDR_MASK 255, 255, 255, 0
#endif
#ifndef CYGDAT_REDBOOT_DEFAULT_GATEWAY_IP_ADDR
# define CYGDAT_REDBOOT_DEFAULT_GATEWAY_IP_ADDR 0, 0, 0, 0
#endif

ip_addr_t __local_ip_addr = { CYGDAT_REDBOOT_DEFAULT_IP_ADDR };
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
ip_addr_t __local_ip_mask = { CYGDAT_REDBOOT_DEFAULT_IP_ADDR_MASK };
ip_addr_t __local_ip_gate = { CYGDAT_REDBOOT_DEFAULT_GATEWAY_IP_ADDR };
#endif

static word ip_ident;

#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
/*
 * See if an address is on the local network
 */
int 
__ip_addr_local(ip_addr_t *addr)
{
  return !(
           ((__local_ip_addr[0] ^ (*addr)[0]) & __local_ip_mask[0]) |
           ((__local_ip_addr[1] ^ (*addr)[1]) & __local_ip_mask[1]) |
           ((__local_ip_addr[2] ^ (*addr)[2]) & __local_ip_mask[2]) |
           ((__local_ip_addr[3] ^ (*addr)[3]) & __local_ip_mask[3]));
}
#endif

/*
 * Match given IP address to our address.
 * Check for broadcast matches as well.
 */
static int
ip_addr_match(ip_addr_t addr)
{
    if (addr[0] == 255 && addr[1] == 255 && addr[2] == 255 && addr[3] == 255)
	return 1;

    if (!memcmp(addr, __local_ip_addr, sizeof(ip_addr_t)))
	return 1;

    /*
     * Consider it an address match if we haven't gotten our IP address yet.
     * Some DHCP servers will address IP packets to the assigned address
     * instead of a IP broadcast address.
     */
    if (__local_ip_addr[0] == 0 && __local_ip_addr[1] == 0 &&
	__local_ip_addr[2] == 0 && __local_ip_addr[3] == 0)
	return 1;

    return 0;
}


extern void __tcp_handler(pktbuf_t *, ip_route_t *);

/*
 * Handle IP packets coming from the polled ethernet interface.
 */
void
__ip_handler(pktbuf_t *pkt, enet_addr_t *src_enet_addr)
{
    ip_header_t *ip = pkt->ip_hdr;
    ip_route_t  r;
    int         hdr_bytes;

    /* first make sure its ours and has a good checksum. */
    if (!ip_addr_match(ip->destination) ||
	__sum((word *)ip, ip->hdr_len << 2, 0) != 0) {
	__pktbuf_free(pkt);
	return;
    }

    memcpy(r.ip_addr, ip->source, sizeof(ip_addr_t));
    memcpy(r.enet_addr, src_enet_addr, sizeof(enet_addr_t));

    hdr_bytes = ip->hdr_len << 2;
    pkt->pkt_bytes = ntohs(ip->length) - hdr_bytes;

    switch (ip->protocol) {

#if NET_SUPPORT_ICMP
      case IP_PROTO_ICMP:
	pkt->icmp_hdr = (icmp_header_t *)(((char *)ip) + hdr_bytes);
	__icmp_handler(pkt, &r);
	break;
#endif

#if NET_SUPPORT_TCP
      case IP_PROTO_TCP:
	pkt->tcp_hdr = (tcp_header_t *)(((char *)ip) + hdr_bytes);
	__tcp_handler(pkt, &r);
	break;
#endif

#if NET_SUPPORT_UDP
      case IP_PROTO_UDP:
	pkt->udp_hdr = (udp_header_t *)(((char *)ip) + hdr_bytes);
	__udp_handler(pkt, &r);
	break;
#endif

      default:
	__pktbuf_free(pkt);
	break;
    }
}


/*
 * Send an IP packet.
 *
 * The IP data field should contain pkt->pkt_bytes of data.
 * pkt->[udp|tcp|icmp]_hdr points to the IP data field. Any
 * IP options are assumed to be already in place in the IP
 * options field.
 */
int
__ip_send(pktbuf_t *pkt, int protocol, ip_route_t *dest)
{
    ip_header_t *ip = pkt->ip_hdr;
    int         hdr_bytes;
    unsigned short cksum;
    
    /*
     * Figure out header length. The use udp_hdr is
     * somewhat arbitrary, but works because it is
     * a union with other IP protocol headers.
     */
    hdr_bytes = (((char *)pkt->udp_hdr) - ((char *)ip));

    pkt->pkt_bytes += hdr_bytes;

    ip->version = 4;
    ip->hdr_len = hdr_bytes >> 2;
    ip->tos = 0;
    ip->length = htons(pkt->pkt_bytes);
    ip->ident = htons(ip_ident);
    ip_ident++;
    ip->fragment = 0;
    ip->ttl = 255;
    ip->ttl = 64;
    ip->protocol = protocol;
    ip->checksum = 0;
    memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(ip->destination, dest->ip_addr, sizeof(ip_addr_t));
    cksum = __sum((word *)ip, hdr_bytes, 0);
    ip->checksum = htons(cksum);

    __enet_send(pkt, &dest->enet_addr, ETH_TYPE_IP);    
    return 0;
}


