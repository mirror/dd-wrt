//==========================================================================
//
//      tests/ppp_updown.c
//
//      Simple test of PPP and networking support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Portions created by Nick Garnett are
// Copyright (C) 2003 eCosCentric Ltd.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: gthomas (ping code), nickg
// Date:         2003-06-01
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PPP test code

#include <network.h>

#include <pkgconf/system.h>
#include <pkgconf/net.h>

#include "ppp_test_support.inl"

//==========================================================================

typedef void pr_fun(char *fmt, ...);

externC void show_network_tables(pr_fun *pr);

//==========================================================================

#include <arpa/inet.h>

// Fill in the blanks if necessary
#ifndef TNR_OFF
# define TNR_OFF()
#endif
#ifndef TNR_ON
# define TNR_ON()
#endif
#ifndef TNR_INIT
# define TNR_INIT()
#endif
#ifndef TNR_PRINT_ACTIVITY
# define TNR_PRINT_ACTIVITY()
#endif

#define NUM_PINGS 16
#define MAX_PACKET 4096
#define MIN_PACKET   64
#define MAX_SEND   4000

#define PACKET_ADD  ((MAX_SEND - MIN_PACKET)/NUM_PINGS)
#define nPACKET_ADD  1 

static unsigned char pkt1[MAX_PACKET], pkt2[MAX_PACKET];

#define UNIQUEID 0x1234

void
pexit(char *s)
{
    CYG_TEST_FAIL_FINISH(s);
}

// Compute INET checksum
int
inet_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register u_short answer;
    register u_int sum = 0;
    u_short odd_byte = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if( nleft == 1 ) {
        *(u_char *)(&odd_byte) = *(u_char *)w;
        sum += odd_byte;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0x0000ffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */
    answer = ~sum;                          /* truncate to 16 bits */
    return (answer);
}

static int
show_icmp(unsigned char *pkt, int len, 
          struct sockaddr_in *from, struct sockaddr_in *to)
{
#if 1
    cyg_tick_count_t *tp, tv;
    struct ip *ip;
    struct icmp *icmp;
    tv = cyg_current_time();
    ip = (struct ip *)pkt;
    if ((len < sizeof(*ip)) || ip->ip_v != IPVERSION) {
        diag_printf("%s: Short packet or not IP! - Len: %d, Version: %d\n", 
                    inet_ntoa(from->sin_addr), len, ip->ip_v);
        return 0;
    }
    icmp = (struct icmp *)(pkt + sizeof(*ip));
    len -= (sizeof(*ip) + 8);
    tp = (cyg_tick_count_t *)&icmp->icmp_data;
    if (icmp->icmp_type != ICMP_ECHOREPLY) {
        diag_printf("%s: Invalid ICMP - type: %d\n", 
                    inet_ntoa(from->sin_addr), icmp->icmp_type);
        return 0;
    }
    if (icmp->icmp_id != UNIQUEID) {
        diag_printf("%s: ICMP received for wrong id - sent: %x, recvd: %x\n", 
                    inet_ntoa(from->sin_addr), UNIQUEID, icmp->icmp_id);
    }
    diag_printf("%d bytes from %s: ", len, inet_ntoa(from->sin_addr));
    diag_printf("icmp_seq=%d", icmp->icmp_seq);
    diag_printf(", time=%dms\n", (int)(tv - *tp)*10);
    return (from->sin_addr.s_addr == to->sin_addr.s_addr);
#else
    return 1;
#endif
}

static void
ping_host(int s, struct sockaddr_in *host)
{
    struct icmp *icmp = (struct icmp *)pkt1;
    int icmp_len = MIN_PACKET;
    int seq, ok_recv, bogus_recv;
    cyg_tick_count_t *tp;
    long *dp;
    struct sockaddr_in from;
    int i, len, fromlen;

    ok_recv = 0;
    bogus_recv = 0;
    diag_printf("PING server %s\n", inet_ntoa(host->sin_addr));
    for (seq = 0;  seq < NUM_PINGS;  seq++, icmp_len += PACKET_ADD ) {
        TNR_ON();
        // Build ICMP packet
        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_cksum = 0;
        icmp->icmp_seq = seq;
        icmp->icmp_id = 0x1234;
        // Set up ping data
        tp = (cyg_tick_count_t *)&icmp->icmp_data;
        *tp++ = cyg_current_time();
        dp = (long *)tp;
        for (i = sizeof(*tp);  i < icmp_len;  i += sizeof(*dp)) {
            *dp++ = i;
        }
        // Add checksum
        icmp->icmp_cksum = inet_cksum( (u_short *)icmp, icmp_len+8);
        // Send it off
        if (sendto(s, icmp, icmp_len+8, 0, (struct sockaddr *)host, sizeof(*host)) < 0) {
            TNR_OFF();
            perror("sendto");
            continue;
        }
        // Wait for a response
        fromlen = sizeof(from);
        len = recvfrom(s, pkt2, sizeof(pkt2), 0, (struct sockaddr *)&from, &fromlen);
        TNR_OFF();
        if (len < 0) {
            perror("recvfrom");
            icmp_len = MIN_PACKET - PACKET_ADD; // just in case - long routes
        } else {
            if (show_icmp(pkt2, len, &from, host)) {
                ok_recv++;
            } else {
                bogus_recv++;
            }
        }
    }
    TNR_OFF();
    diag_printf("Sent %d packets, received %d OK, %d bad\n", NUM_PINGS, ok_recv, bogus_recv);
}

static void do_ping(void)
{
    struct protoent *p;
    struct timeval tv;
    struct sockaddr_in host;
    int s;

    if ((p = getprotobyname("icmp")) == (struct protoent *)0) {
        pexit("getprotobyname");
        return;
    }
    s = socket(AF_INET, SOCK_RAW, p->p_proto);
    if (s < 0) {
        pexit("socket");
        return;
    }
    tv.tv_sec = 7;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    host.sin_port = 0;

    // Set a default remote end address
    inet_aton("10.0.0.100", &host.sin_addr);

    // Now quiz the ppp0 interface for its destination address and use
    // that as our ping-buddy.
    {
        struct ifreq ifr;
        int sock = socket(AF_INET, SOCK_DGRAM, 0);

        if( sock != -1 )
        {
            strncpy( ifr.ifr_name, "ppp0", sizeof(ifr.ifr_name));
            if( ioctl( sock, SIOCGIFDSTADDR, &ifr ) == 0 )
                host.sin_addr = ((struct sockaddr_in *)&ifr.ifr_dstaddr)->sin_addr;
        
            close(sock);
        }
    }
    
    ping_host(s, &host);
    
}

//==========================================================================
    
#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;


//==========================================================================

static void do_test( cyg_serial_baud_rate_t baud )
{
    cyg_ppp_options_t options;
    cyg_ppp_handle_t ppp_handle;

    ppp_test_announce( "PPP_UPDOWN" );
    
    cyg_ppp_options_init( &options );

//    options.debug = 1;
//    options.kdebugflag = 1;

    options.baud = baud;
    
    ppp_handle = cyg_ppp_up( CYGPKG_PPP_TEST_DEVICE, &options );

    CYG_TEST_INFO( "Waiting for PPP to come up");
    
    cyg_ppp_wait_up( ppp_handle );

//    show_network_tables( diag_printf );

    CYG_TEST_INFO( "Delaying...");

    cyg_thread_delay(10*100);
    
    CYG_TEST_INFO( "Pinging remote");

    do_ping();

//    show_network_tables( diag_printf );
    
    CYG_TEST_INFO( "Bringing PPP down");

    cyg_ppp_down( ppp_handle );
    
    CYG_TEST_INFO( "Waiting for PPP to go down");

    cyg_ppp_wait_down( ppp_handle );
}

//==========================================================================

void
ppp_test(cyg_addrword_t p)
{
    cyg_serial_baud_rate_t old;
    
    CYG_TEST_INIT();
    diag_printf("Start PPP test\n");

    init_all_network_interfaces();

    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_115200 );
    do_test( CYGNUM_SERIAL_BAUD_115200 );

#ifdef CYGPKG_PPP_TESTS_AUTOMATE

    CYG_TEST_INFO( "Delaying...");
    cyg_thread_delay(10*100);
    
    {
        static cyg_serial_baud_rate_t test_rates[] =
            { CYGDAT_PPP_TEST_BAUD_RATES, 0 };

        int i;

        for( i = 0; test_rates[i] != 0; i++ )
        {
            ppp_test_set_baud( test_rates[i] );
            do_test( test_rates[i] );

            CYG_TEST_INFO( "Delaying...");
            cyg_thread_delay(10*100);
            
        }

        ppp_test_set_baud( old );
    
        ppp_test_finish();
        
    }
           
#endif
    
    CYG_TEST_PASS_FINISH("PPP test OK");
}

//==========================================================================

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      ppp_test,          // entry
                      0,                 // entry parameter
                      "PPP test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}

//==========================================================================
