//==========================================================================
//
//      ./agent/current/tests/snmpping.c
//
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
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
//==========================================================================
//
//      tests/snmpping.c
//
//      Simple test of PING (ICMP) and networking support
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
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-01-10
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


// -------------------------------------------------------------------------
// Configuration of the test... now from CDL

// Do we test the interfaces in promiscuous mode?
//#define CYGSEM_SNMPAGENT_TESTS_PROMISCUOUS

// Do we make the test run forever?
//#define CYGNUM_SNMPAGENT_TESTS_ITERATIONS 999999

// Do we initialize SNMP v3 MIBs and authentication database?
//#define CYGSEM_SNMPAGENT_TESTS_SNMPv3

// ------------------------------------------------------------------------


// PING test code

#include <network.h>
#include <stdio.h>

#include <pkgconf/system.h>
#include <pkgconf/net.h>
#include <pkgconf/snmpagent.h>

#include <unistd.h>

#ifdef  CYGSEM_SNMPAGENT_TESTS_SNMPv3
#include <ucd-snmp/config.h>
#include <ucd-snmp/asn1.h>
#include <ucd-snmp/snmp_api.h>
#include <ucd-snmp/snmp_vars.h>

#include <ucd-snmp/snmpv3.h>
#include <ucd-snmp/usmUser.h>
#include <ucd-snmp/usmStats.h>
#include <ucd-snmp/snmpEngine.h>
#endif // CYGSEM_SNMPAGENT_TESTS_SNMPv3

#include <cyg/infra/testcase.h>

#ifdef CYGBLD_DEVS_ETH_DEVICE_H    // Get the device config if it exists
#include CYGBLD_DEVS_ETH_DEVICE_H  // May provide CYGTST_DEVS_ETH_TEST_NET_REALTIME
#endif

#ifdef CYGPKG_NET_TESTS_USE_RT_TEST_HARNESS // do we use the rt test?
# ifdef CYGTST_DEVS_ETH_TEST_NET_REALTIME // Get the test ancilla if it exists
#  include CYGTST_DEVS_ETH_TEST_NET_REALTIME
# endif
#endif

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

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

#define NUM_PINGS 16
#define MAX_PACKET 4096
#define MIN_PACKET   64
#define MAX_SEND   4000

#define PACKET_ADD  ((MAX_SEND - MIN_PACKET)/NUM_PINGS)
#define nPACKET_ADD  1 

static unsigned char pkt1[MAX_PACKET], pkt2[MAX_PACKET];

#define UNIQUEID 0x1234

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
    if ( 0 >= len ) {
        diag_printf("%s: Completely bogus short packet%s\n",
                    inet_ntoa(from->sin_addr), 0 == len ? "" : " [no ICMP header]");
        return 0;
    }
    tp = (cyg_tick_count_t *)&icmp->icmp_data;
    if (icmp->icmp_type != ICMP_ECHOREPLY) {
        diag_printf("%s: Invalid ICMP - type: %d, len (databytes): %d\n", 
                    inet_ntoa(from->sin_addr), icmp->icmp_type, len);
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
    TNR_OFF();
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

static void
ping_test(struct bootp *bp)
{
    struct protoent *p;
    struct timeval tv;
    struct sockaddr_in host;
    int s;

    if ((p = getprotobyname("icmp")) == (struct protoent *)0) {
        perror("getprotobyname");
        return;
    }
    s = socket(AF_INET, SOCK_RAW, p->p_proto);
    if (s < 0) {
        perror("socket");
        return;
    }
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    host.sin_addr = bp->bp_siaddr;
    host.sin_port = 0;
    ping_host(s, &host);
    // Now try a bogus host
    // (also, map 76 <-> 191 so that if a pair runs they ping each other)
    host.sin_addr = bp->bp_yiaddr; // *my* address.
//    host.sin_addr.s_addr = htonl(ntohl(host.sin_addr.s_addr) ^ 0xf3);
    host.sin_addr.s_addr = htonl(ntohl(host.sin_addr.s_addr) ^ 2);
    ping_host(s, &host);
    close(s);
}

static void
ping_test_loopback( int lo )
{
    struct protoent *p;
    struct timeval tv;
    struct sockaddr_in host;
    int s;

    if ((p = getprotobyname("icmp")) == (struct protoent *)0) {
        perror("getprotobyname");
        return;
    }
    s = socket(AF_INET, SOCK_RAW, p->p_proto);
    if (s < 0) {
        perror("socket");
        return;
    }
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    host.sin_addr.s_addr = htonl(INADDR_LOOPBACK + (0x100 * lo));
    host.sin_port = 0;
    ping_host(s, &host);
    // Now try a bogus host
    host.sin_addr.s_addr = htonl(ntohl(host.sin_addr.s_addr) + 32);
    ping_host(s, &host);
    close(s);
}

#ifdef CYGSEM_SNMPAGENT_TESTS_PROMISCUOUS
static void 
interface_promisc(const char *intf)
{
  struct ifreq ifr;
  int s;

  s = socket(AF_INET, SOCK_DGRAM, 0);
  if (s < 0) {
    perror("socket");
    return;
  }

  strcpy(ifr.ifr_name, intf);
  ifr.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_PROMISC;
  if (ioctl(s, SIOCSIFFLAGS, &ifr)) {
    perror("SIOCSIFFLAGS");
  }
  close(s);
}
#endif // CYGSEM_SNMPAGENT_TESTS_PROMISCUOUS

void snmp_do_reinit( void )
{
    diag_printf( "SNMP re-init function\n" );
#ifdef  CYGSEM_SNMPAGENT_TESTS_SNMPv3
    // Initialisation for USM is now invoked from mib_module_inits.h
    //init_usmUser();             /* MIBs to support SNMPv3             */
    //init_usmStats();
    //init_snmpEngine();
    usm_parse_create_usmUser(NULL, "root MD5 md5passwd DES DESpasswd");
#endif //  CYGSEM_SNMPAGENT_TESTS_SNMPv3
}

int create_snmpd_conf (void) {
#ifdef CYGSEM_SNMPAGENT_TESTS_SNMPv3
#ifdef CYGPKG_SNMPLIB_FILESYSTEM_SUPPORT 
#ifdef CYGPKG_FS_RAM
  int c;
  FILE *fd;

  diag_printf ("\nStarting creation of snmpd.conf\n");

  /* Mount RAM-FS */
  if (mount ("", "/", "ramfs") != 0) {
    diag_printf ("File system mount failed; errno=%d \n", errno);
    return -1;
  }
  
  if (mkdir ("/etc", 0) != 0) {
    diag_printf ("mkdir (etc) failed;  errno=%d\n", errno);
    return -1;
  } 

  if (chdir ("/etc") != 0) {
    diag_printf ("... Change-dir (etc) failed; errno=%d\n", errno);
    return -1;
  } else { 
    diag_printf ("chdir-etc done\n");
  }

  if (mkdir ("snmp", 0) != 0) {
    diag_printf ("mkdir failed (snmp);  errno=%d\n", errno);
    return -1;
  } else {
    diag_printf ("mkdir-snmp done\n");
  }

  if (chdir ("snmp") != 0) {
    diag_printf ("... Change-dir (snmp) failed; errno=%d\n", errno);
    return -1;
  } else { 
    diag_printf ("... Change-dir (snmp) done \n");
  }

  /* Open File & Write to it  */
  if ((fd = fopen( "snmpd.conf", "w" )) == NULL) {
    diag_printf ("fopen failed\n");
    return -1;
  }

  fprintf (fd, "#        sec.name     source       community\n");
  fprintf (fd, "com2sec   public     default       crux\n");
  fprintf (fd, "com2sec   root       default       crux\n");
  fprintf (fd, "#                 sec.model   sec.name\n");
  fprintf (fd, "group     public    v1        public\n");
  fprintf (fd, "group     public    v2c       public\n");
  fprintf (fd, "group     public    usm       root\n");
  fprintf (fd, "view      all  included  .1\n");
  fprintf (fd, "access    public    \"\"   any  noauth    exact     all  none none\n");
  fprintf (fd, "\n\n");

  if (fclose (fd)) {
    diag_printf ("fclose failed\n");
    return -1;
  }

  /* Read Back */
  fd = fopen( "/etc/snmp/snmpd.conf", "r" );
  if (fd == NULL) { 
    diag_printf ("fopen failed\n");
    return -1;
  }

  while ((c=fgetc (fd)) != EOF) {
    diag_printf ("%c", c);
  }

  if (fclose (fd))  {
    diag_printf ("fclose failed\n");
    return -1;
  }
#endif
#endif
#endif
  return 0;
}

void
net_test(cyg_addrword_t p)
{
    int i = CYGNUM_SNMPAGENT_TESTS_ITERATIONS;
    int ieth0_up = 0, ieth1_up = 0;
    int j;

    diag_printf("Start PING test\n");
    TNR_INIT();
    init_all_network_interfaces();
#ifdef CYGHWR_NET_DRIVER_ETH0
    ieth0_up = eth0_up;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    ieth1_up = eth1_up;
#endif
#ifdef CYGSEM_SNMPAGENT_TESTS_PROMISCUOUS
#ifdef CYGHWR_NET_DRIVER_ETH0
        if (eth0_up)
            interface_promisc("eth0");
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
        if (eth1_up)
            interface_promisc("eth1");
#endif
#endif // CYGSEM_SNMPAGENT_TESTS_PROMISCUOUS
    {
        extern void cyg_net_snmp_init(void);
        extern void (*snmpd_reinit_function)( void );

        snmpd_reinit_function = snmp_do_reinit;

        if (create_snmpd_conf ()) {
          CYG_TEST_FAIL_EXIT("create_snmpd_conf() error\n");
        }
        cyg_net_snmp_init();
    }
    do {
        TNR_ON();
#ifdef CYGHWR_NET_DRIVER_ETH0
        if (eth0_up) {
            ping_test(&eth0_bootp_data);
            cyg_thread_delay(500);
        }
#endif
#if NLOOP > 0
        for ( j = 0; j < NLOOP; j++ ) {
            ping_test_loopback( j );
            cyg_thread_delay(500);
        }
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
        if (eth1_up) {
            ping_test(&eth1_bootp_data);
            cyg_thread_delay(500);
        }
#endif
        TNR_OFF();
        TNR_PRINT_ACTIVITY();

        // If an interface has gone down eg. due to DHCP timing out,
        // re-initialize everything:
        if (0
#ifdef CYGHWR_NET_DRIVER_ETH0
            || ieth0_up != eth0_up
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
            || ieth1_up != eth1_up 
#endif
            ) {
            diag_printf( "Re-initializing the world: eth0 %d/%d eth1 %d/%d!\n",
                         ieth0_up, 0
#ifdef CYGHWR_NET_DRIVER_ETH0
                         | eth0_up
#endif
                         , ieth1_up, 0
#ifdef CYGHWR_NET_DRIVER_ETH1
                         | eth1_up
#endif
                );
            init_all_network_interfaces();
            for ( j = 0; j < CYGPKG_NET_NLOOP; j++ )
                init_loopback_interface( j );

            diag_printf( "Re-initialized the world: eth0 %d/%d eth1 %d/%d!\n",
                         ieth0_up, 0
#ifdef CYGHWR_NET_DRIVER_ETH0
                         | eth0_up
#endif
                         , ieth1_up, 0
#ifdef CYGHWR_NET_DRIVER_ETH1
                         | eth1_up
#endif
                );
        }
    } while ( i-- > 0 );
    CYG_TEST_PASS_FINISH( "Done pinging while SNMP looks on" );
}

void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      net_test,          // entry
                      0,                 // entry parameter
                      "Network test",    // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}

// EOF snmpping.c
