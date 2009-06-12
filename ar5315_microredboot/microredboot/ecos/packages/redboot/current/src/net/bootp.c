//==========================================================================
//
//      net/bootp.c
//
//      Stand-alone minimal BOOTP support for RedBoot
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
#include <net/bootp.h>

#define SHOULD_BE_RANDOM  0x12345555

/* How many milliseconds to wait before retrying the request */
#define RETRY_TIME  2000
#define MAX_RETRIES    8

static bootp_header_t *bp_info;
  
#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
static const unsigned char dhcpCookie[] = {99,130,83,99};
static const unsigned char dhcpEnd[] = {255};
static const unsigned char dhcpDiscover[] = {53,1,1};
static const unsigned char dhcpRequest[] = {53,1,3};
static const unsigned char dhcpRequestIP[] = {50,4};
static const unsigned char dhcpParamRequestList[] = {55,3,1,3,6};
static enum {
    DHCP_NONE = 0,
    DHCP_DISCOVER,
    DHCP_OFFER,
    DHCP_REQUEST,
    DHCP_ACK
} dhcpState;
#endif

static void
bootp_handler(udp_socket_t *skt, char *buf, int len,
	      ip_route_t *src_route, word src_port)
{
    bootp_header_t *b;
#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
    unsigned char *p, expected = 0;
#endif

    b = (bootp_header_t *)buf;
    if (bp_info) {
        memset(bp_info,0,sizeof *bp_info);
        if (len > sizeof *bp_info)
            len = sizeof *bp_info;
        memcpy(bp_info, b, len);
    }

    // Only accept pure REPLY responses
    if (b->bp_op != BOOTREPLY)
      return;
    
    // Must be sent to me, as well!
    if (memcmp(b->bp_chaddr, __local_enet_addr, 6))
      return;
        
#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
    p = b->bp_vend;
    if (memcmp(p, dhcpCookie, sizeof(dhcpCookie)))
      return;
    p += 4;

    // Find the DHCP Message Type tag
    while (*p != TAG_DHCP_MESS_TYPE) {
        p += p[1] + 2;
        if (p >= (unsigned char*)b + sizeof(*bp_info))
            return;
    }

    p += 2;

    switch (dhcpState) {
    case DHCP_DISCOVER:
        // The discover message has been sent, only accept an offer reply
        if (*p == DHCP_MESS_TYPE_OFFER) {
            dhcpState = DHCP_OFFER;
            return;
        } else {
            expected = DHCP_MESS_TYPE_OFFER;
        }
        break;
    case DHCP_REQUEST:
        // The request message has been sent, only accept an ack reply
        if (*p == DHCP_MESS_TYPE_ACK) {
            dhcpState = DHCP_ACK;
            return;
        } else {
            expected = DHCP_MESS_TYPE_ACK;
        }
        break;
    case DHCP_NONE:
    case DHCP_OFFER:
    case DHCP_ACK:
        // Quitely ignore these - they indicate repeated message from server
        return;
    }
    // See if we've been NAK'd - if so, give up and try again
    if (*p == DHCP_MESS_TYPE_NAK) {
        dhcpState = DHCP_NONE;
        return;
    }
    diag_printf("DHCP reply: %d, not %d\n", (int)*p, (int)expected);
    return;
#else
    // Simple BOOTP - this is all there is!
    memcpy(__local_ip_addr, &b->bp_yiaddr, 4);
#endif
}

#define AddOption(p,d) do {memcpy(p,d,sizeof d); p += sizeof d;} while (0)

/*
 * Find our IP address and copy to __local_ip_addr.
 * Return zero if successful, -1 if not.
 */
int
__bootp_find_local_ip(bootp_header_t *info)
{
    udp_socket_t udp_skt;
    bootp_header_t b;
    ip_route_t     r;
    int            retry;
    unsigned long  start;
    ip_addr_t saved_ip_addr;
#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
    unsigned char *p;
    int oldState;
#endif
    int txSize;
    bool abort = false;
    static int xid = SHOULD_BE_RANDOM;

#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
    dhcpState = DHCP_NONE;
#endif

    // Where we want the results saved
    bp_info = info;
    // Preserve any IP address we currently have, just in case
    memcpy(saved_ip_addr, __local_ip_addr, sizeof(__local_ip_addr));

    // fill out route for a broadcast
    r.ip_addr[0] = 255;
    r.ip_addr[1] = 255;
    r.ip_addr[2] = 255;
    r.ip_addr[3] = 255;
    r.enet_addr[0] = 255;
    r.enet_addr[1] = 255;
    r.enet_addr[2] = 255;
    r.enet_addr[3] = 255;
    r.enet_addr[4] = 255;
    r.enet_addr[5] = 255;

    // setup a socket listener for bootp replies
    __udp_install_listener(&udp_skt, IPPORT_BOOTPC, bootp_handler);

    retry = MAX_RETRIES;  
    while (!abort && (retry-- > 0)) {
	start = MS_TICKS();

        // Build up the BOOTP/DHCP request
        memset(&b, 0, sizeof(b));
        b.bp_op = BOOTREQUEST;
        b.bp_htype = HTYPE_ETHERNET;
        b.bp_hlen = 6;
        b.bp_xid = xid++;
        memcpy(b.bp_chaddr, __local_enet_addr, 6);
        memset(__local_ip_addr, 0, sizeof(__local_ip_addr));
         
#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
        p = b.bp_vend;
        switch (dhcpState) {
        case DHCP_NONE:
        case DHCP_DISCOVER:
            AddOption(p,dhcpCookie);
            AddOption(p,dhcpDiscover);
            AddOption(p,dhcpParamRequestList);
            AddOption(p,dhcpEnd);
            dhcpState = DHCP_DISCOVER;
            break;
        case DHCP_OFFER:
            retry = MAX_RETRIES;
        case DHCP_REQUEST:
            b.bp_xid = bp_info->bp_xid;  // Match what server sent
            AddOption(p,dhcpCookie);
            AddOption(p,dhcpRequest);
            AddOption(p,dhcpRequestIP);
            memcpy(p, &bp_info->bp_yiaddr, 4);  p += 4;  // Ask for the address just given
            AddOption(p,dhcpParamRequestList);
            AddOption(p,dhcpEnd);
            dhcpState = DHCP_REQUEST;
            memset(&b.bp_yiaddr, 0xFF, 4);
            memset(&b.bp_siaddr, 0xFF, 4);
            memset(&b.bp_yiaddr, 0x00, 4);
            memset(&b.bp_siaddr, 0x00, 4);
            break;
        case DHCP_ACK:
            // Ignore these states (they won't happen)
            break;
        }
     
        // Some servers insist on a minimum amount of "vendor" data
        if (p < &b.bp_vend[BP_MIN_VEND_SIZE]) p = &b.bp_vend[BP_MIN_VEND_SIZE];
        txSize = p - (unsigned char*)&b;
        oldState = dhcpState;
#else
        txSize = sizeof(b);
#endif

	__udp_send((char *)&b, txSize, &r, IPPORT_BOOTPS, IPPORT_BOOTPC);

	do {
	    __enet_poll();
#ifdef CYGSEM_REDBOOT_NETWORKING_DHCP
            if (dhcpState != oldState) {
                if (dhcpState == DHCP_ACK) {
                    unsigned char *end;
                    int optlen;
                    // Address information has now arrived!
                    memcpy(__local_ip_addr, &bp_info->bp_yiaddr, 4);
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
                    memcpy(__local_ip_gate, &bp_info->bp_giaddr, 4);
#endif
                    p = bp_info->bp_vend+4;
                    end = (unsigned char *)bp_info+sizeof(*bp_info);
                    while (p < end) {
                        unsigned char tag = *p;
                        if (tag == TAG_END)
                            break;
                        if (tag == TAG_PAD)
                            optlen = 1;
                        else {
                            optlen = p[1];
                            p += 2;
                            switch (tag) {
#ifdef CYGSEM_REDBOOT_NETWORKING_USE_GATEWAY
                            case TAG_SUBNET_MASK:  // subnet mask
                                memcpy(__local_ip_mask,p,4); 
                                break;
                            case TAG_GATEWAY:  // router
                                memcpy(__local_ip_gate,p,4); 
                                break;
#endif
#ifdef CYGPKG_REDBOOT_NETWORKING_DNS
			    case TAG_DOMAIN_SERVER:
//				diag_printf(" DNS server found!\n");
				memcpy(&__bootp_dns_addr, p, 4);
				__bootp_dns_set = 1;
				break;
#endif
                            default:
                                break;
                            }
                        }
                        p += optlen;
                    }
                    __udp_remove_listener(IPPORT_BOOTPC);
                    return 0;
                } else {
                    break;  // State changed, handle it
                }
            }
#else
            // All done, if address response has arrived
	    if (__local_ip_addr[0] || __local_ip_addr[1] ||
		__local_ip_addr[2] || __local_ip_addr[3]) {
		/* success */
		__udp_remove_listener(IPPORT_BOOTPC);
		return 0;
	    }
#endif
            if (_rb_break(1)) {
                // The user typed ^C on the console
                abort = true;
                break;
            }
            MS_TICKS_DELAY();  // Count for ^C test
	} while ((MS_TICKS_DELAY() - start) < RETRY_TIME);

        // Warn the user that we're polling for BOOTP info
        if (retry == (MAX_RETRIES-1)) {
            diag_printf("... waiting for BOOTP information\n");
        }
    }

    // timed out
    __udp_remove_listener(IPPORT_BOOTPC);
    // Restore any previous IP address
    memcpy(__local_ip_addr, saved_ip_addr, sizeof(__local_ip_addr));
    return -1;
}


