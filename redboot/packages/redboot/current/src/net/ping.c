//==========================================================================
//
//      ping.c
//
//      Network utility - ping
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Date:         2001-01-22
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

#ifndef CYGPKG_REDBOOT_NETWORKING
#error CYGPKG_REDBOOT_NETWORKING required!
#else

static void do_ping(int argc, char *argv[]);
RedBoot_cmd("ping", 
            "Network connectivity test",
            "[-v] [-n <count>] [-l <length>] [-t <timeout>] [-r <rate>]\n"
            "        [-i <IP_addr>] -h <IP_addr>",
            do_ping
    );

static bool icmp_received;
static icmp_header_t hold_hdr;

static void
handle_icmp(pktbuf_t *pkt, ip_route_t *src_route)
{
    icmp_header_t *icmp;
    unsigned short cksum;

    icmp = pkt->icmp_hdr;
    if (icmp->type == ICMP_TYPE_ECHOREQUEST
	&& icmp->code == 0
	&& __sum((word *)icmp, pkt->pkt_bytes, 0) == 0) {

	icmp->type = ICMP_TYPE_ECHOREPLY;
	icmp->checksum = 0;
        cksum = __sum((word *)icmp, pkt->pkt_bytes, 0);
	icmp->checksum = htons(cksum);
        __ip_send(pkt, IP_PROTO_ICMP, src_route);
    } else if (icmp->type == ICMP_TYPE_ECHOREPLY) {
        memcpy(&hold_hdr, icmp, sizeof(*icmp));
        icmp_received = true;
    }
}

static void
do_ping(int argc, char *argv[])
{
    struct option_info opts[7];
    long count, timeout, length, rate, start_time, end_time, timer, received, tries;
    char *local_ip_addr, *host_ip_addr;
    bool local_ip_addr_set, host_ip_addr_set, count_set, 
        timeout_set, length_set, rate_set, verbose;
    struct sockaddr_in local_addr, host_addr;
    ip_addr_t hold_addr;
    icmp_header_t *icmp;
    pktbuf_t *pkt;
    ip_header_t *ip;
    unsigned short cksum;
    ip_route_t dest_ip;

    init_opts(&opts[0], 'n', true, OPTION_ARG_TYPE_NUM, 
              (void *)&count, (bool *)&count_set, "<count> - number of packets to test");
    init_opts(&opts[1], 't', true, OPTION_ARG_TYPE_NUM, 
              (void *)&timeout, (bool *)&timeout_set, "<timeout> - max #ms per packet [rount trip]");
    init_opts(&opts[2], 'i', true, OPTION_ARG_TYPE_STR, 
              (void *)&local_ip_addr, (bool *)&local_ip_addr_set, "local IP address");
    init_opts(&opts[3], 'h', true, OPTION_ARG_TYPE_STR, 
              (void *)&host_ip_addr, (bool *)&host_ip_addr_set, "host name or IP address");
    init_opts(&opts[4], 'l', true, OPTION_ARG_TYPE_NUM, 
              (void *)&length, (bool *)&length_set, "<length> - size of payload");
    init_opts(&opts[5], 'v', false, OPTION_ARG_TYPE_FLG, 
              (void *)&verbose, (bool *)0, "verbose operation");
    init_opts(&opts[6], 'r', true, OPTION_ARG_TYPE_NUM, 
              (void *)&rate, (bool *)&rate_set, "<rate> - time between packets");
    if (!scan_opts(argc, argv, 1, opts, 7, (void **)0, 0, "")) {
        diag_printf("PING - Invalid option specified\n");
        return;
    }   
    // Set defaults; this has to be done _after_ the scan, since it will
    // have destroyed all values not explicitly set.
    if (local_ip_addr_set) {
        if (!_gethostbyname(local_ip_addr, (in_addr_t *)&local_addr)) {
            diag_printf("PING - Invalid local name: %s\n", local_ip_addr);
            return;
        }
    } else {
        memcpy((in_addr_t *)&local_addr, __local_ip_addr, sizeof(__local_ip_addr));
    }
    if (host_ip_addr_set) {
        if (!_gethostbyname(host_ip_addr, (in_addr_t *)&host_addr)) {
            diag_printf("PING - Invalid host name: %s\n", host_ip_addr);
            return;
        }
        if (__arp_lookup((ip_addr_t *)&host_addr.sin_addr, &dest_ip) < 0) {
            diag_printf("PING: Cannot reach server '%s' (%s)\n", 
                        host_ip_addr, inet_ntoa((in_addr_t *)&host_addr));
            return;
        }
    } else {
        diag_printf("PING - host name or IP address required\n");
        return;
    }
#define DEFAULT_LENGTH   64
#define DEFAULT_COUNT    10
#define DEFAULT_TIMEOUT  1000
#define DEFAULT_RATE     1000
    if (!rate_set) {
        rate = DEFAULT_RATE;
    }
    if (!length_set) {
        length = DEFAULT_LENGTH;
    }
    if ((length < 64) || (length > 1400)) {
        diag_printf("Invalid length specified: %ld\n", length);
        return;
    }
    if (!count_set) {
        count = DEFAULT_COUNT;
    }
    if (!timeout_set) {
        timeout = DEFAULT_TIMEOUT;
    }
    // Note: two prints here because 'inet_ntoa' returns a static pointer
    diag_printf("Network PING - from %s",
                inet_ntoa((in_addr_t *)&local_addr));
    diag_printf(" to %s\n",
                inet_ntoa((in_addr_t *)&host_addr));
    received = 0;    
    __icmp_install_listener(handle_icmp);
    // Save default "local" address
    memcpy(hold_addr, __local_ip_addr, sizeof(hold_addr));
    for (tries = 0;  tries < count;  tries++) {
        // The network stack uses the global variable '__local_ip_addr'
        memcpy(__local_ip_addr, &local_addr, sizeof(__local_ip_addr));
        // Build 'ping' request
        if ((pkt = __pktbuf_alloc(ETH_MAX_PKTLEN)) == NULL) {
            // Give up if no packets - something is wrong
            break;
        }

        icmp = pkt->icmp_hdr;
        ip = pkt->ip_hdr;
        pkt->pkt_bytes = length + sizeof(icmp_header_t);

	icmp->type = ICMP_TYPE_ECHOREQUEST;
        icmp->code = 0;
	icmp->checksum = 0;
        icmp->seqnum = htons(tries+1);
        cksum = __sum((word *)icmp, pkt->pkt_bytes, 0);
	icmp->checksum = htons(cksum);
	
        memcpy(ip->source, (in_addr_t *)&local_addr, sizeof(ip_addr_t));
        memcpy(ip->destination, (in_addr_t *)&host_addr, sizeof(ip_addr_t));
        ip->protocol = IP_PROTO_ICMP;
        ip->length = htons(pkt->pkt_bytes);

        __ip_send(pkt, IP_PROTO_ICMP, &dest_ip);
        __pktbuf_free(pkt);

        start_time = MS_TICKS();
        timer = start_time + timeout;
        icmp_received = false;
        while (!icmp_received && (MS_TICKS_DELAY() < timer)) {            
            if (_rb_break(1)) {
                goto abort;
            }
            MS_TICKS_DELAY();
            __enet_poll();
        } 
        end_time = MS_TICKS();

        timer = MS_TICKS() + rate;
        while (MS_TICKS_DELAY() < timer) {
            if (_rb_break(1)) {
                goto abort;
            }
            MS_TICKS_DELAY();
            __enet_poll();
        } 

        if (icmp_received) {
            received++;
            if (verbose) {
                diag_printf(" seq: %ld, time: %ld (ticks)\n",
                            ntohs(hold_hdr.seqnum), end_time-start_time);
            }
        }
    }
 abort:
    __icmp_remove_listener();
    // Clean up
    memcpy(__local_ip_addr, &hold_addr, sizeof(__local_ip_addr));
    // Report
    diag_printf("PING - received %ld of %ld expected\n", received, count);
}

#endif //CYGPKG_REDBOOT_NETWORKING
