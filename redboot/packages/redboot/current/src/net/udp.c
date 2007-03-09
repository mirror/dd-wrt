//==========================================================================
//
//      net/udp.c
//
//      Stand-alone UDP networking support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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

#include <redboot.h>
#include <net/net.h>

#ifdef UDP_STATS
static int udp_rx_total;
static int udp_rx_handled;
static int udp_rx_cksum;
static int udp_rx_dropped;
#endif

#define MAX_UDP_DATA (ETH_MAX_PKTLEN - (ETH_HDR_SIZE + \
					sizeof(ip_header_t)  + \
					sizeof(udp_header_t)))

/*
 * A major assumption is that only a very small number of sockets will
 * active, so a simple linear search of those sockets is acceptible.
 */
static udp_socket_t *udp_list;


/*
 * Install a handler for incoming udp packets.
 * Caller provides the udp_socket_t structure.
 * Returns zero if successful, -1 if socket is already used.
 */
int
__udp_install_listener(udp_socket_t *s, word port, udp_handler_t handler)
{
    udp_socket_t *p;

    /*
     * Make sure we only have one handler per port.
     */
    for (p = udp_list; p; p = p->next)
	if (p->our_port == port)
	    return -1;
    
    s->our_port = htons(port);
    s->handler = handler;
    s->next = udp_list;
    udp_list = s;

    return 0;
}


/*
 * Remove the handler for the given socket.
 */
void
__udp_remove_listener(word port)
{
    udp_socket_t *prev, *s;

    for (prev = NULL, s = udp_list; s; prev = s, s = s->next)
	if (s->our_port == htons(port)) {
	    if (prev)
		prev->next = s->next;
	    else
		udp_list = s->next;
	}
}


/*
 * Handle incoming UDP packets.
 */
void
__udp_handler(pktbuf_t *pkt, ip_route_t *r)
{
    udp_header_t *udp = pkt->udp_hdr;
    ip_header_t  *ip = pkt->ip_hdr;
    udp_socket_t *s;

    if (udp->checksum == 0xffff)
	udp->checksum = 0;

    /* copy length for pseudo sum calculation */
    ip->length = udp->length;

    if (__sum((word *)udp, ntohs(udp->length), __pseudo_sum(ip)) == 0) {
	for (s = udp_list; s; s = s->next) {
	    if (s->our_port == udp->dest_port) {
		(*s->handler)(s, ((char *)udp) + sizeof(udp_header_t),
			      ntohs(udp->length) - sizeof(udp_header_t),
			      r, ntohs(udp->src_port));
                __pktbuf_free(pkt);
		return;
	    }
	}
    }
    __pktbuf_free(pkt);
}


/*
 * Send a UDP packet.
 */
int
__udp_send(char *buf, int len, ip_route_t *dest_ip,
	   word dest_port, word src_port)
{
    pktbuf_t *pkt;
    udp_header_t *udp;
    ip_header_t *ip;
    unsigned short cksum;
    int ret;

    /* dumb */
    if (len > MAX_UDP_DATA)
	return -1;

    /* just drop it if can't get a buffer */
    if ((pkt = __pktbuf_alloc(ETH_MAX_PKTLEN)) == NULL)
	return -1;

    udp = pkt->udp_hdr;
    ip = pkt->ip_hdr;

    pkt->pkt_bytes = len + sizeof(udp_header_t);

    udp->src_port = htons(src_port);
    udp->dest_port = htons(dest_port);
    udp->length = htons(pkt->pkt_bytes);
    udp->checksum = 0;

    memcpy(((char *)udp) + sizeof(udp_header_t), buf, len);

    /* fill in some pseudo-header fields */
    memcpy(ip->source, __local_ip_addr, sizeof(ip_addr_t));
    memcpy(ip->destination, dest_ip->ip_addr, sizeof(ip_addr_t));
    ip->protocol = IP_PROTO_UDP;
    ip->length = udp->length;

    cksum = __sum((word *)udp, pkt->pkt_bytes, __pseudo_sum(ip));
    udp->checksum = htons(cksum);

    ret = __ip_send(pkt, IP_PROTO_UDP, dest_ip);
    __pktbuf_free(pkt);
    return ret;
}

int
__udp_sendto(char *data, int len, struct sockaddr_in *server, 
             struct sockaddr_in *local)
{
    ip_route_t rt;

   if (__arp_lookup((ip_addr_t *)&server->sin_addr, &rt) < 0) {
       diag_printf("%s: Can't find address of server\n", __FUNCTION__);
       return -1;
   } else {
      __udp_send(data, len, &rt, ntohs(server->sin_port), ntohs(local->sin_port));
       return 0;
   }
}

static char               *recvfrom_buf;
static int                 recvfrom_len;
static struct sockaddr_in *recvfrom_server;

static void
__udp_recvfrom_handler(udp_socket_t *skt, char *buf, int len,
                       ip_route_t *src_route, word src_port)
{
    if (recvfrom_server == NULL || recvfrom_buf == NULL)
	return;

    if (recvfrom_server->sin_port && recvfrom_server->sin_port != htons(src_port))
	return;

    // Move data to waiting buffer
    recvfrom_len = len;
    memcpy(recvfrom_buf, buf, len);
    if (recvfrom_server) {
        recvfrom_server->sin_port = htons(src_port);
        memcpy(&recvfrom_server->sin_addr, &src_route->ip_addr, sizeof(src_route->ip_addr));
        recvfrom_buf = (char *)0;  // Tell reader we got a packet
    } else {
        diag_printf("udp_recvfrom - dropped packet of %d bytes\n", len);
    }
}

int
__udp_recvfrom(char *data, int len, struct sockaddr_in *server, 
               struct sockaddr_in *local, struct timeval *timo)
{
    int res, my_port, total_ms;
    udp_socket_t skt;
    unsigned long start;

    my_port = ntohs(local->sin_port);
    if (__udp_install_listener(&skt, my_port, __udp_recvfrom_handler) < 0) {
        return -1;
    }
    recvfrom_buf = data;
    recvfrom_len = len;
    recvfrom_server = server;
    total_ms = (timo->tv_sec * 1000) + (timo->tv_usec / 1000);
    start = MS_TICKS();
    res = -1;
    do {
        __enet_poll();  // Handle the hardware
        if (!recvfrom_buf) {
            // Data have arrived
            res = recvfrom_len;
            break;
        }
    } while ((MS_TICKS_DELAY() - start) < total_ms);
    __udp_remove_listener(my_port);
    return res;
}
