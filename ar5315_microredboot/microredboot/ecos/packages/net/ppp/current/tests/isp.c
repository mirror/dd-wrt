//==========================================================================
//
//      tests/isp.c
//
//      Test of PPP and CHAT connection to an ISP
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
// Description:  This test uses CHAT to set up a modem, dial, talk to an ISP
//               and then ping some well known addresses. It then brings the
//               link down.
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// PPP test code

#include <network.h>

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/ppp.h>

#include <cyg/ppp/ppp.h>

#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include <cyg/infra/testcase.h>

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

static void do_ping(char *addr)
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
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    inet_aton(addr, &host.sin_addr);
    host.sin_port = 0;

    ping_host(s, &host);
    
}

//==========================================================================


#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

//==========================================================================
/*

Captured session:


AT S7=45 S0=0 L1 V1 X4 &c1 E1 Q0                     
OK                                                   
ATD08440416662                                       
CONNECT 42666/ARQ/V90/LAPM/V42BIS                    
anchor-du-10.access.demon.net


login: xxxxxx
Password: yyyyyy
Protocol: ppp
xxxxxx: IP Address: XXX.XXX.XXX.XXX  Running PPP on 1223
No operational problems reported.
Finger status@gate.demon.co.uk for more info - Last change 16:30 Jun 08
HELLO

                                                                                
                                                                                
OK                                                                              
ATH                                                                             
OK                                                                              

*/

static char *script[] =
{
    "ABORT"             ,       "BUSY"                                  ,
    "ABORT"             ,       "NO CARRIER"                            ,
    "ABORT"             ,       "ERROR"                                 ,
    ""                  ,       "ATZ"                                   ,
    "OK"                ,       "AT S7=45 S0=0 L1 V1 X4 &C1 E1 Q0"      ,
    "OK"                ,       "ATD" CYGPKG_PPP_DEFAULT_DIALUP_NUMBER  ,
    "ogin:--ogin:"      ,       CYGPKG_PPP_AUTH_DEFAULT_USER            ,
    "assword:"          ,       CYGPKG_PPP_AUTH_DEFAULT_PASSWD          ,
    "otocol:"           ,       "ppp"                                   ,
    "HELLO"             ,       "\\c"                                   ,
    0
};

//==========================================================================

void
isp_test(cyg_addrword_t p)
{
    CYG_TEST_INIT();

    diag_printf("Start ISP test\n");

    init_all_network_interfaces();

    {
        cyg_ppp_options_t options;
        cyg_ppp_handle_t ppp_handle;

        cyg_ppp_options_init( &options );

//        options.debug = 1;
//        options.kdebugflag = 1;

        options.modem = 1;
        options.script = &script[0];
        
        ppp_handle = cyg_ppp_up( CYGPKG_PPP_TEST_DEVICE, &options );

        CYG_TEST_INFO( "Waiting for PPP to come up");
    
        cyg_ppp_wait_up( ppp_handle );

        CYG_TEST_INFO( "Delaying for a while...");
        
        cyg_thread_delay(10*100);
        
        CYG_TEST_INFO( "Pinging");
        
        do_ping("195.173.57.10");       // anchor-du-10.access.demon.net
        do_ping("194.154.160.254");     // fluffy.ecoscentric.com
        do_ping("66.187.233.205");      // sources.redhat.com
        
        CYG_TEST_INFO( "Bringing PPP down");

        cyg_ppp_down( ppp_handle );
    
        CYG_TEST_INFO( "Waiting for PPP to go down");

        cyg_ppp_wait_down( ppp_handle );
    }
    
    CYG_TEST_PASS_FINISH("ISP test OK");
}

//==========================================================================

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      isp_test,          // entry
                      0,                 // entry parameter
                      "ISP test",        // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}

//==========================================================================
