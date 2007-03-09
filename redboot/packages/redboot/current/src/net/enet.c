//==========================================================================
//
//      net/enet.c
//
//      Stand-alone ethernet [link-layer] support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
#include <cyg/io/eth/eth_drv.h>       // Logical driver interfaces

//#define ENET_STATS 1

#ifdef ENET_STATS
static int num_ip = 0;
static int num_arp = 0;
#ifdef NET_SUPPORT_RARP
static int num_rarp = 0;
#endif
static int num_received = 0;
static int num_transmitted = 0;
#endif

//
// Support for user handlers of additional ethernet packets (nonIP)
//

#define NUM_EXTRA_HANDLERS 4
static struct {
    int type;
    pkt_handler_t handler;
} eth_handlers[NUM_EXTRA_HANDLERS];

pkt_handler_t 
__eth_install_listener(int eth_type, pkt_handler_t handler)
{
    int i, empty;
    pkt_handler_t old;

    if (eth_type > 0x800 || handler != (pkt_handler_t)0) {
	empty = -1;
	for (i = 0;  i < NUM_EXTRA_HANDLERS;  i++) {
	    if (eth_handlers[i].type == eth_type) {
		// Replace existing handler
		old = eth_handlers[i].handler;
		eth_handlers[i].handler = handler;
		return old;
	    }
	    if (eth_handlers[i].type == 0) {
		empty = i;
	    }
	}
	if (empty >= 0) {
	    // Found a free slot
	    eth_handlers[empty].type = eth_type;
	    eth_handlers[empty].handler = handler;
	    return (pkt_handler_t)0;
	}
    }
    diag_printf("** Warning: can't install listener for ethernet type 0x%02x\n", eth_type);
    return (pkt_handler_t)0;
}

void 
__eth_remove_listener(int eth_type)
{
    int i;
    
    for (i = 0;  i < NUM_EXTRA_HANDLERS;  i++) {
        if (eth_handlers[i].type == eth_type) {
            eth_handlers[i].type = 0;
        }
    }    
}

/*
 * Non-blocking poll of ethernet link. Process packets until no more
 * are available.
 */
void
__enet_poll(void)
{
    pktbuf_t *pkt;
    eth_header_t eth_hdr;
    int i, type;
#ifdef DEBUG_PKT_EXHAUSTION
    static bool was_exhausted = false;
#endif

    while (true) {
        /*
         * Try to get a free pktbuf and return if none
         * are available.
         */
        if ((pkt = __pktbuf_alloc(ETH_MAX_PKTLEN)) == NULL) {
#ifdef DEBUG_PKT_EXHAUSTION
            if (!was_exhausted) {
                int old = start_console();  // Force output to standard port
                diag_printf("__enet_poll: no more buffers\n");
                __pktbuf_dump();
                was_exhausted = true;
                end_console(old);
            } 
#endif
            return;
        }
#ifdef DEBUG_PKT_EXHAUSTION
        was_exhausted = false;  // Report the next time we're out of buffers
#endif

        if ((pkt->pkt_bytes = eth_drv_read((char *)&eth_hdr, (char *)pkt->buf,
                                           ETH_MAX_PKTLEN)) > 0) {
#ifdef ENET_STATS
            ++num_received;
#endif
            switch (type = ntohs(eth_hdr.type)) {

            case ETH_TYPE_IP:
#ifdef ENET_STATS
                ++num_ip;
#endif
                pkt->ip_hdr = (ip_header_t *)pkt->buf;
                __ip_handler(pkt, &eth_hdr.source);
                break;

            case ETH_TYPE_ARP:
#ifdef ENET_STATS
                ++num_arp;
#endif
                pkt->arp_hdr = (arp_header_t *)pkt->buf;
                __arp_handler(pkt);
                break;

#ifdef NET_SUPPORT_RARP
            case ETH_TYPE_RARP:
#ifdef ENET_STATS
                ++num_rarp;
#endif
                pkt->arp_hdr = (arp_header_t *)pkt->buf;
                __rarp_handler(pkt);
                break;
#endif

            default:
	        if (type > 0x800) {
		    for (i = 0;  i < NUM_EXTRA_HANDLERS;  i++) {
			if (eth_handlers[i].type == type) {
			    (eth_handlers[i].handler)(pkt, &eth_hdr);
			}
		    }
		}
                __pktbuf_free(pkt);
                break;
            }
        } else {
            __pktbuf_free(pkt);
            break;
        } 
    }
}



/*
 * Send an ethernet packet.
 */
void
__enet_send(pktbuf_t *pkt, enet_addr_t *dest, int eth_type)
{
    eth_header_t eth_hdr;

    // Set up ethernet header
    memcpy(&eth_hdr.destination, dest, sizeof(enet_addr_t));
    memcpy(&eth_hdr.source, __local_enet_addr, sizeof(enet_addr_t));
    eth_hdr.type = htons(eth_type);

    eth_drv_write((char *)&eth_hdr, (char *)pkt->buf, pkt->pkt_bytes);
#ifdef ENET_STATS
    ++num_transmitted;
#endif
}

#ifdef __LITTLE_ENDIAN__

unsigned long  
ntohl(unsigned long x)
{
    return (((x & 0x000000FF) << 24) |
            ((x & 0x0000FF00) <<  8) |
            ((x & 0x00FF0000) >>  8) |
            ((x & 0xFF000000) >> 24));
}

unsigned long
ntohs(unsigned short x)
{
    return (((x & 0x00FF) << 8) |
            ((x & 0xFF00) >> 8));
}

#endif
