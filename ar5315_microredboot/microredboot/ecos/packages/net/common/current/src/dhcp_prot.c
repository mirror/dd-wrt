/*==========================================================================
//
//      dhcp_prot.c
//
//      DHCP protocol implementation for DHCP client
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Andrew Lunn
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
// Author(s):   hmt
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:        2000-07-01
// Purpose:     DHCP support
// Description: 
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/net.h>

#ifdef CYGPKG_NET_DHCP

#ifdef CYGPKG_NET_SNTP
#include <pkgconf/net_sntp.h>
#endif /* CYGPKG_NET_SNTP */

#if 0
#define perror( txt ) // nothing
#endif

#include <network.h>
#include <dhcp.h>
#include <errno.h>

#include <cyg/infra/cyg_ass.h>

#ifdef INET6
#include <net/if_var.h>
#include <netinet6/in6_var.h>
#endif

extern int  cyg_arc4random(void);

#ifdef CYGOPT_NET_DHCP_OPTION_HOST_NAME
static char dhcp_hostname[CYGNUM_NET_DHCP_OPTION_HOST_NAME_LEN+1];

// Set the hostname used by the DHCP TAG_HOST_NAME option.  We
// copy the callers name into a private buffer, since we don't
// know the context in which the callers hostname was allocated.
void dhcp_set_hostname(char *hostname)
{
    CYG_ASSERT( (strlen(hostname)<=CYGNUM_NET_DHCP_OPTION_HOST_NAME_LEN), "dhcp hostname too long" );
    strncpy(dhcp_hostname, hostname, CYGNUM_NET_DHCP_OPTION_HOST_NAME_LEN);
}
#endif

// ------------------------------------------------------------------------
// Returns a pointer to the end of dhcp message (or NULL if invalid)
// meaning the address of the byte *after* the TAG_END token in the vendor
// data.

static unsigned char *
scan_dhcp_size( struct bootp *ppkt )
{
    unsigned char *op;
    
    op = &ppkt->bp_vend[0];
    // First check for the cookie!
    if ( op[0] !=  99 ||
         op[1] != 130 ||
         op[2] !=  83 ||
         op[3] !=  99 ) {
        CYG_FAIL( "Bad DHCP cookie" );
        return NULL;
    }
    op += 4;

    // This will only scan the options field.
    while (*op != TAG_END) {
        if ( *op == TAG_PAD ) {
            op++;
        } else {
          op += *(op+1)+2;
        }
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in dhcp_size" );
            return NULL;
        }
    }
    // Check op has not gone wild
    CYG_ASSERT( op > (unsigned char *)(&ppkt[0]), "op pointer underflow!" );
    // Compare op with non-existent "next" struct bootp in the array.
    CYG_ASSERT( op < (unsigned char *)(&ppkt[1]), "op pointer overflow!" );
    return op + 1; // Address of first invalid byte
}

// ------------------------------------------------------------------------
// Get the actual packet size of an initialized buffer

static int
dhcp_size( struct bootp *ppkt )
{
    unsigned char *op;

    op = scan_dhcp_size( ppkt );
    if ( !op ) return 0;
    return (op - (unsigned char *)ppkt);
}


// ------------------------------------------------------------------------
// Get the actual packet size of an initialized buffer
// This will also pad the packet with 0 if length is less
// than BP_STD_TX_MINPKTSZ.

static int
dhcp_size_for_send( struct bootp *ppkt )
{
    unsigned char *op;

    op = scan_dhcp_size( ppkt );
    if ( !op ) return 0; // Better not scribble!
    // Zero extra bytes until the packet is large enough.
    for ( ; op < (((unsigned char *)ppkt) + BP_STD_TX_MINPKTSZ); op++ )
        *op = 0;
    return (op - (unsigned char *)ppkt);
}

// ------------------------------------------------------------------------
// Insert/set an option value in an initialized buffer

static int
set_fixed_tag( struct bootp *ppkt,
               unsigned char tag,
               cyg_uint32 value,
               int len)
{
    unsigned char *op;

    // Initially this will only scan the options field.
    
    op = &ppkt->bp_vend[4];
    while (*op != TAG_END) {
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_fixed_tag" );
            return false;
        }
        if (*op == tag)                 // Found it...
            break;
        op += *(op+1)+2;
    }
    
    if (*op == tag) { // Found it...
        if ( *(op+1) != len ) {
            CYG_FAIL( "Wrong size in set_fixed_tag" );
            return false;           // wrong size
        }
    }
    else { // overwrite the end tag and install a new one
        if ( op + len + 2 > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_fixed_tag append" );
            return false;
        }
        *op = tag;
        *(op+1) = len;
        *(op + len + 2) = TAG_END;
    }
    // and insert the value.  Net order is BE.
    op += len + 2 - 1;              // point to end of value
    while ( len-- > 0 ) {
        *op-- = (unsigned char)(value & 255);
        value >>= 8;
    }
    return true;
}

// Note that this does not permit changing the size of an extant tag.
static int
set_variable_tag( struct bootp *ppkt,
               unsigned char tag,
               cyg_uint8 *pvalue,
               int len)
{
    unsigned char *op;

    // Initially this will only scan the options field.
    op = &ppkt->bp_vend[4];
    while (*op != TAG_END) {
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_variable_tag" );
            return false;
        }
        if (*op == tag)                 // Found it...
            break;
        op += *(op+1)+2;
    }
    
    if (*op == tag) { // Found it...
        if ( *(op+1) != len ) {
            CYG_FAIL( "Wrong size in set_variable_tag" );
            return false;           // wrong size
        }
    }
    else { // overwrite the end tag and install a new one
        if ( op + len + 2 > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in set_variable_tag append" );
            return false;
        }
        *op = tag;
        *(op+1) = len;
        *(op + len + 2) = TAG_END;
    }
    // and insert the value.  No order is implied.
    op += 2;               // point to start of value
    while ( len-- > 0 ) {
        *op++ = *pvalue++;
    }
    return true;
}

static int
unset_tag( struct bootp *ppkt,
           unsigned char tag )
{
    unsigned char *op, *nextp = 0, *killp = 0;

    // Initially this will only scan the options field.
    
    op = &ppkt->bp_vend[4];
    while (*op != TAG_END) {
        if ( op > &ppkt->bp_vend[BP_VEND_LEN-1] ) {
            CYG_FAIL( "Oversize DHCP packet in unset_tag" );
            return false;
        }
        if (*op == tag) {               // Found it...
            killp = op;                 // item to kill
            nextp = op + *(op+1)+2;     // next item address
        }
        op += *(op+1)+2;                // scan to the end
    }

    if ( !killp )
        return false;

    // Obliterate the found op by copying down: *op is the end.
    while( nextp <= op )                // <= to copy the TAG_END too.
        *killp++ = *nextp++;
    
    return true;
}

// ------------------------------------------------------------------------
// Bring up an interface enough to broadcast, before we know who we are

static int
bring_half_up(const char *intf, struct ifreq *ifrp )
{
    int s = -1;
    int one = 1;

    struct sockaddr_in *addrp;
    struct ecos_rtentry route;
    int retcode = false;

    // Ensure clean slate
    cyg_route_reinit();  // Force any existing routes to be forgotten

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
        goto out;
    }

    addrp = (struct sockaddr_in *) &ifrp->ifr_addr;
    memset(addrp, 0, sizeof(*addrp));
    addrp->sin_family = AF_INET;
    addrp->sin_len = sizeof(*addrp);
    addrp->sin_port = 0;
    addrp->sin_addr.s_addr = INADDR_ANY;

    strcpy(ifrp->ifr_name, intf);
    if (ioctl(s, SIOCSIFADDR, ifrp)) { /* set ifnet address */
        perror("SIOCSIFADDR");
        goto out;
    }

    if (ioctl(s, SIOCSIFNETMASK, ifrp)) { /* set net addr mask */
        perror("SIOCSIFNETMASK");
        goto out;
    }

    /* the broadcast address is 255.255.255.255 */
    memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
    if (ioctl(s, SIOCSIFBRDADDR, ifrp)) { /* set broadcast addr */
        perror("SIOCSIFBRDADDR");
        goto out;
    }

    ifrp->ifr_flags = IFF_UP | IFF_BROADCAST | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, ifrp)) { /* set ifnet flags */
        perror("SIOCSIFFLAGS up");
        goto out;
    }

    if (ioctl(s, SIOCGIFHWADDR, ifrp) < 0) { /* get MAC address */
        perror("SIOCGIFHWADDR 1");
        goto out;
    }

    // Set up routing
    addrp->sin_family = AF_INET;
    addrp->sin_port = 0;
    addrp->sin_len = sizeof(*addrp);  // Size of address

    /* the broadcast address is 255.255.255.255 */
    memset(&addrp->sin_addr, 255, sizeof(addrp->sin_addr));
    memset(&route, 0, sizeof(route));
    memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

    addrp->sin_addr.s_addr = INADDR_ANY;
    memcpy(&route.rt_dst, addrp, sizeof(*addrp));
    memcpy(&route.rt_genmask, addrp, sizeof(*addrp));

    route.rt_dev = ifrp->ifr_name;
    route.rt_flags = RTF_UP|RTF_GATEWAY;
    route.rt_metric = 0;

    if (ioctl(s, SIOCADDRT, &route)) { /* add route */
        if (errno != EEXIST) {
            perror("SIOCADDRT 3");
            goto out;
        }
    }
    retcode = true;
 out:
    if (s != -1) 
      close(s);

    return retcode;
}


// ------------------------------------------------------------------------
// DHCP retransmission timeouts and number of tries
// 
// To work better with simulated failures (or real ones!) so that the rest
// of the system is tested, rather than DHCP renewal failures pulling
// everything down, we try a little more zealously than the RFC suggests.

static unsigned char timeout_random = 0;

struct timeout_state {
    unsigned int secs;
    int countdown;
};

static inline void reset_timeout( struct timeval *ptv, struct timeout_state *pstate )
{
    timeout_random++;
    pstate->countdown = 4; // initial fast retries
    pstate->secs = 3 + (timeout_random & 3);
    ptv->tv_sec = 0;
    ptv->tv_usec = 65536 * (2 + (timeout_random & 3)); // 0.1 - 0.3S, about
}

static inline int next_timeout( struct timeval *ptv, struct timeout_state *pstate )
{
    if ( 0 < pstate->countdown-- )
        return true;
    if ( 0 == ptv->tv_sec )
        ptv->tv_sec = pstate->secs;
    else {
        timeout_random++;
        pstate->secs = ptv->tv_sec * 2 - 2 + (timeout_random & 3);
        pstate->countdown = 2; // later fast retries
        ptv->tv_sec = 0;
    }
    // If longer, too many tries...
    return pstate->secs < CYGNUM_NET_DHCP_MIN_RETRY_TIME; 
}

// ------------------------------------------------------------------------
// Lease expiry and alarms to notify it

static cyg_alarm_t alarm_function;

static void alarm_function(cyg_handle_t alarm, cyg_addrword_t data)
{
    struct dhcp_lease *lease = (struct dhcp_lease *)data;
    lease->which |= lease->next;
    if ( lease->needs_attention )
        cyg_semaphore_post( lease->needs_attention );

    // Step the lease on into its next state of being alarmed ;-)
    if ( lease->next & DHCP_LEASE_EX ) {
        cyg_alarm_disable( alarm );
    }
    else if ( lease->next & DHCP_LEASE_T2 ) {
        lease->next = DHCP_LEASE_EX;
        cyg_alarm_initialize( lease->alarm, lease->expiry, 0 );
        cyg_alarm_enable( lease->alarm );
    }
    else if ( lease->next & DHCP_LEASE_T1 ) {
        lease->next = DHCP_LEASE_T2;
        cyg_alarm_initialize( lease->alarm, lease->t2, 0 );
        cyg_alarm_enable( lease->alarm );
    }
}

static inline void no_lease( struct dhcp_lease *lease )
{
    if ( lease->alarm ) {
        // Already set: delete this.
        cyg_alarm_disable( lease->alarm );
        cyg_alarm_delete( lease->alarm );
        lease->alarm = 0;
    }
}

static inline void new_lease( struct bootp *bootp, struct dhcp_lease *lease )
{
    cyg_tick_count_t now = cyg_current_time();
    cyg_tick_count_t then;
    cyg_uint32 tag = 0;
    cyg_uint32 expiry_then;
    cyg_resolution_t resolution = 
        cyg_clock_get_resolution(cyg_real_time_clock());
    cyg_handle_t h;
    unsigned int length;
    
    // Silence any jabbering from past lease on this interface
    no_lease( lease );
    lease->which = lease->next = 0;
    cyg_clock_to_counter(cyg_real_time_clock(), &h);
    cyg_alarm_create( h, alarm_function, (cyg_addrword_t)lease,
                      &lease->alarm, &lease->alarm_obj );

    // extract the lease time and scale it &c to now.
    length = sizeof(tag);
    if(!get_bootp_option( bootp, TAG_DHCP_LEASE_TIME, &tag ,&length))
        tag = 0xffffffff;

    if ( 0xffffffff == tag ) {
        lease->expiry = 0xffffffff;
        lease->t2     = 0xffffffff;
        lease->t1     = 0xffffffff;
        return; // it's an infinite lease, hurrah!
    }

    then = (cyg_uint64)(ntohl(tag));
    expiry_then = then;

    then *= 1000000000; // into nS - we know there is room in a tick_count_t
    then = (then / resolution.dividend) * resolution.divisor; // into system ticks
    lease->expiry = now + then;
    length = sizeof(tag);
    if (get_bootp_option( bootp, TAG_DHCP_REBIND_TIME, &tag, &length ))
        then = (cyg_uint64)(ntohl(tag));
    else
        then = expiry_then - expiry_then/4;
    then *= 1000000000; // into nS - we know there is room in a tick_count_t
    then = (then / resolution.dividend) * resolution.divisor; // into system ticks
    lease->t2 = now + then;

    length = sizeof(tag);
    if (get_bootp_option( bootp, TAG_DHCP_RENEWAL_TIME, &tag, &length ))
        then = (cyg_uint64)(ntohl(tag));
    else
        then = expiry_then/2;
    then *= 1000000000; // into nS - we know there is room in a tick_count_t
    then = (then / resolution.dividend) * resolution.divisor; // into system ticks
    lease->t1 = now + then;

#if 0 // for testing this mechanism
    lease->expiry = now + 5000; // 1000 here makes for failure in the DHCP test
    lease->t2     = now + 3500;
    lease->t1     = now + 2500;
#endif

#ifdef CYGDBG_NET_DHCP_CHATTER
    diag_printf("new_lease:\n");
    diag_printf("  expiry = %d\n",lease->expiry);
    diag_printf("      t1 = %d\n",lease->t1);
    diag_printf("      t2 = %d\n",lease->t2);
#endif

    lease->next = DHCP_LEASE_T1;

    cyg_alarm_initialize( lease->alarm, lease->t1, 0 );
    cyg_alarm_enable( lease->alarm );
}

// ------------------------------------------------------------------------
// Set all the tags we want to use when sending a packet.
// This has expanded to a large, explicit set to interwork better
// with a variety of DHCP servers.

static void set_default_dhcp_tags( struct bootp *xmit )
{
    // Explicitly request full set of params that are default for LINUX
    // dhcp servers, but not default for others.  This is rather arbitrary,
    // but it preserves behaviour for people using those servers.
    // Perhaps configury of this set will be needed in future?
    //
    // Here's the set:
    static cyg_uint8 req_list[]  = {
#ifdef CYGOPT_NET_DHCP_PARM_REQ_LIST_REPLACE
        CYGOPT_NET_DHCP_PARM_REQ_LIST_REPLACE ,
#else
        TAG_DHCP_SERVER_ID    ,     //     DHCP server id: 10.16.19.66
        TAG_DHCP_LEASE_TIME   ,     //     DHCP time 51: 60
        TAG_DHCP_RENEWAL_TIME ,     //     DHCP time 58: 30
        TAG_DHCP_REBIND_TIME  ,     //     DHCP time 59: 52
        TAG_SUBNET_MASK       ,     //     subnet mask: 255.255.255.0
        TAG_GATEWAY           ,     //     gateway: 10.16.19.66
        TAG_DOMAIN_SERVER     ,     //     domain server: 10.16.19.66
        TAG_DOMAIN_NAME       ,     //     domain name: hmt10.cambridge.redhat.com
        TAG_IP_BROADCAST      ,     //     IP broadcast: 10.16.19.255
#endif
#ifdef CYGNUM_NET_SNTP_UNICAST_MAXDHCP
        TAG_NTP_SERVER        ,     //     NTP Server Addresses(es)
#endif
#ifdef CYGOPT_NET_DHCP_PARM_REQ_LIST_ADDITIONAL
        CYGOPT_NET_DHCP_PARM_REQ_LIST_ADDITIONAL ,
#endif
    };

    if ( req_list[0] ) // So that one may easily turn it all off by configury
        set_variable_tag( xmit, TAG_DHCP_PARM_REQ_LIST,
                          &req_list[0], sizeof( req_list ) );

#ifdef CYGOPT_NET_DHCP_OPTION_HOST_NAME
{
    int nlen = strlen(dhcp_hostname);

    if (nlen > 0)
    	set_variable_tag( xmit, TAG_HOST_NAME, dhcp_hostname, nlen + 1);
}
#endif
#ifdef CYGOPT_NET_DHCP_OPTION_DHCP_CLIENTID_MAC
{
	cyg_uint8 id[16+1];	/* sizeof bp_chaddr[] + 1 */

	id[0] = 1;  /* 1-byte hardware type: 1=ethernet. */
    CYG_ASSERT( xmit->bp_hlen<=(sizeof(id)-1), "HW address invalid" );
    memcpy(&id[1], &xmit->bp_chaddr, xmit->bp_hlen);
    set_variable_tag( xmit, TAG_DHCP_CLIENTID, id, xmit->bp_hlen+1);
}
#endif

    // Explicitly specify our max message size.
    set_fixed_tag( xmit, TAG_DHCP_MAX_MSGSZ, BP_MINPKTSZ, 2 );
}

// ------------------------------------------------------------------------
// the DHCP state machine - this does all the work

int
do_dhcp(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease)
{
    struct ifreq ifr;
    struct sockaddr_in cli_addr, broadcast_addr, server_addr, rx_addr;
    int s = -1, addrlen;
    int one = 1;
    unsigned char mincookie[] = {99,130,83,99,255} ;
    struct timeval tv;
    struct timeout_state timeout_scratch;
    cyg_uint8 oldstate = *pstate;
    cyg_uint8 msgtype = 0, seen_bootp_reply = 0;
    unsigned int length;
    
    cyg_uint32 xid;

#define CHECK_XID() (  /* and other details */                                  \
    received->bp_xid   != xid            || /* not the same transaction */      \
    received->bp_htype != xmit->bp_htype || /* not the same ESA type    */      \
    received->bp_hlen  != xmit->bp_hlen  || /* not the same length      */      \
    bcmp( &received->bp_chaddr, &xmit->bp_chaddr, xmit->bp_hlen )               \
    )

    // IMPORTANT: xmit is the same as res throughout this; *received is a
    // scratch buffer for reception; its contents are always copied to res
    // when we are happy with them.  So we always transmit from the
    // existing state.
    struct bootp rx_local;
    struct bootp *received = &rx_local;
    struct bootp *xmit = res;
    struct bootp xmit2;
    int xlen;

    // First, get a socket on the interface in question.  But Zeroth, if
    // needs be, bring it to the half-up broadcast only state if needs be.
    
    if ( DHCPSTATE_INIT      == oldstate
         || DHCPSTATE_FAILED == oldstate
         || 0                == oldstate ) {
        // either explicit init state or the beginning of time or retry
        if ( ! bring_half_up( intf, &ifr ) )
            return false;

        *pstate = DHCPSTATE_INIT;
        lease->which = lease->next = 0;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {
        perror("setsockopt");
        goto out;
    }

    memset((char *) &cli_addr, 0, sizeof(cli_addr));
    cli_addr.sin_family = AF_INET;
    cli_addr.sin_len = sizeof(cli_addr);
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(IPPORT_BOOTPC);
    
    memset((char *) &broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_len = sizeof(broadcast_addr);
    broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    broadcast_addr.sin_port = htons(IPPORT_BOOTPS);

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_len = sizeof(server_addr);
    server_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST); // overwrite later
    server_addr.sin_port = htons(IPPORT_BOOTPS);

    if(bind(s, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
        perror("bind error");
        goto out;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        perror("setsockopt SO_REUSEADDR");
        goto out;
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        perror("setsockopt SO_REUSEPORT");
        goto out;
    }
    
    // Now, we can launch into the DHCP state machine.  I think this will
    // be the neatest way to do it; it returns from within the switch arms
    // when all is well, or utterly failed.

    reset_timeout( &tv, &timeout_scratch );

    // Choose a new XID: first get the ESA as a basis:
    strcpy(&ifr.ifr_name[0], intf);
    if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        perror("SIOCGIFHWADDR 2");
        goto out;
    }

    // Choose from scratch depending on ifr_hwaddr...[]
    xid = ifr.ifr_hwaddr.sa_data[5];
    xid |= (ifr.ifr_hwaddr.sa_data[4]) << 8;
    xid |= (ifr.ifr_hwaddr.sa_data[3]) << 16;
    xid |= (ifr.ifr_hwaddr.sa_data[2]) << 24;
    xid ^= (cyg_arc4random() & 0xffff0000);

    // Avoid adjacent ESAs colliding by increment
#define NEW_XID(_xid) CYG_MACRO_START (_xid)+= 0x10000; CYG_MACRO_END

    while ( 1 ) {

        // If we are active rather than in the process of shutting down,
        // check for any lease expiry every time round, so that alarms
        // *can* change the course of events even when already renewing,
        // for example.
        if ( DHCPSTATE_DO_RELEASE   != *pstate
             && DHCPSTATE_NOTBOUND  != *pstate
             && DHCPSTATE_FAILED    != *pstate ) {
            cyg_uint8 lease_state;

            cyg_scheduler_lock();
            lease_state = lease->which;
            lease->which = 0; // flag that we have noticed it
            cyg_scheduler_unlock();

            if ( lease_state & DHCP_LEASE_EX ) {
                // then the lease has expired completely!
                *pstate = DHCPSTATE_NOTBOUND;
            }
            else if ( lease_state & DHCP_LEASE_T2 ) {
                // Time to renew
                reset_timeout( &tv, &timeout_scratch ); // next conversation
                *pstate = DHCPSTATE_REBINDING;
            }
            else if ( lease_state & DHCP_LEASE_T1 ) {
                // Time to renew
                reset_timeout( &tv, &timeout_scratch ); // next conversation
                *pstate = DHCPSTATE_RENEWING;
            }
        }

        switch ( *pstate ) {

        case DHCPSTATE_INIT:

            // Send the DHCPDISCOVER packet

            // Fill in the BOOTP request - DHCPDISCOVER packet
            bzero(xmit, sizeof(*xmit));
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_htype = HTYPE_ETHERNET;
            xmit->bp_hlen = IFHWADDRLEN;
            xmit->bp_xid = xid;
            xmit->bp_secs = cyg_current_time() / 100;
            xmit->bp_flags = htons(0x8000); // BROADCAST FLAG
            bcopy(ifr.ifr_hwaddr.sa_data, &xmit->bp_chaddr, xmit->bp_hlen);
            bcopy(mincookie, xmit->bp_vend, sizeof(mincookie));

            // remove the next line to test ability to handle bootp packets.
            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPDISCOVER, 1 );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );

#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_INIT sending:\n" );
            show_bootp( intf, xmit );
#endif            
            if(sendto(s, xmit, dhcp_size_for_send(xmit), 0, 
                      (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                *pstate = DHCPSTATE_FAILED;
                break;
            }

            seen_bootp_reply = 0;
            *pstate = DHCPSTATE_SELECTING;
            break;

        case DHCPSTATE_SELECTING:
            // This is a separate state so that we can listen again
            // *without* retransmitting.
            
            // listen for the DHCPOFFER reply

            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            addrlen = sizeof(rx_addr);
            if (recvfrom(s, received, sizeof(struct bootp), 0,
                         (struct sockaddr *)&rx_addr, &addrlen) < 0) {
                // No packet arrived (this time)
                if ( seen_bootp_reply ) { // then already have a bootp reply
                    // Save the good packet in *xmit
                    bcopy( received, xmit, dhcp_size(received) );
                    *pstate = DHCPSTATE_BOOTP_FALLBACK;
                    NEW_XID( xid ); // Happy to advance, so new XID
                    reset_timeout( &tv, &timeout_scratch );
                    break;
                }       
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
                    *pstate = DHCPSTATE_FAILED;
                    break;
                }
                *pstate = DHCPSTATE_INIT; // to retransmit
                break;
            }
            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_SELECTING received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // XID and ESA matches?
                break;                  // listen again...

            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                length = sizeof(received->bp_siaddr.s_addr);
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // see if it was a DHCP reply or a bootp reply; it could be
            // either.
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPOFFER == msgtype ) { // all is well
                    // Save the good packet in *xmit
                    bcopy( received, xmit, dhcp_size(received) );
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    *pstate = DHCPSTATE_REQUESTING;
                    NEW_XID( xid ); // Happy to advance, so new XID
                }
            }
            else // No TAG_DHCP_MESS_TYPE entry so it's a bootp reply
                seen_bootp_reply = 1; // (keep the bootp packet in received)
                
            // If none of the above state changes occurred, we got a packet
            // that "should not happen", OR we have a bootp reply in our
            // hand; so listen again with the same timeout, without
            // retrying the send, in the hope of getting a DHCP reply.
            break;

        case DHCPSTATE_REQUESTING:
            // Just send what you got with a DHCPREQUEST in the message type.
            // then wait for an ACK in DHCPSTATE_REQUEST_RECV.

            // Fill in the BOOTP request - DHCPREQUEST packet
            xmit->bp_xid = xid;
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0x8000); // BROADCAST FLAG

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPREQUEST, 1 );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
            // And this will be a new one:
            set_fixed_tag( xmit, TAG_DHCP_REQ_IP, ntohl(xmit->bp_yiaddr.s_addr), 4 );
            
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REQUESTING sending:\n" );
            show_bootp( intf, xmit );
#endif            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send( xmit );
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0, 
                      (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                *pstate = DHCPSTATE_FAILED;
                break;
            }

            *pstate = DHCPSTATE_REQUEST_RECV;
            break;

        case DHCPSTATE_REQUEST_RECV:
            // wait for an ACK or a NACK - retry by going back to
            // DHCPSTATE_REQUESTING; NACK means go back to INIT.

            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            addrlen = sizeof(rx_addr);
            if (recvfrom(s, received, sizeof(struct bootp), 0,
                         (struct sockaddr *)&rx_addr, &addrlen) < 0) {
                // No packet arrived
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
                    *pstate = DHCPSTATE_FAILED;
                    break;
                }
                *pstate = DHCPSTATE_REQUESTING;
                break;
            }
            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REQUEST_RECV received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // not the same transaction;
                break;                  // listen again...

            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                length = sizeof(received->bp_siaddr.s_addr );
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // check it was a DHCP reply
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPACK == msgtype // Same offer & server?
                     && received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr
                     && received->bp_siaddr.s_addr == xmit->bp_siaddr.s_addr) {
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    // Record the new lease and set up timers &c
                    new_lease( received, lease );
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                if ( DHCPNAK == msgtype // Same server?
                     && received->bp_siaddr.s_addr == xmit->bp_siaddr.s_addr) {
                    // we're bounced!
                    *pstate = DHCPSTATE_INIT;  // So back the start of the rigmarole.
                    NEW_XID( xid ); // Unhappy to advance, so new XID
                    reset_timeout( &tv, &timeout_scratch );
                    break;
                }
                // otherwise it's something else, maybe another offer, or a bogus
                // NAK from someone we are not asking!
                // Just listen again, which implicitly discards it.
            }
            break;

        case DHCPSTATE_BOUND:

            // We are happy now, we have our address.

            // All done with socket
            close(s);
            s = -1;
            
            // Re-initialize the interface with the new state
            if ( DHCPSTATE_BOUND != oldstate ) {
                // Then need to go down and up
                do_dhcp_down_net( intf, res, &oldstate, lease ); // oldstate used
                if ( 0 != oldstate ) {
                    // Then not called from init_all_network_interfaces()
                    // so we must initialize the interface ourselves
                    if (!init_net(intf, res)) {
                        do_dhcp_down_net( intf, res, pstate, lease );
                        *pstate = DHCPSTATE_FAILED;
                        goto out;
                    }
                }
            }

            // Otherwise, nothing whatsoever to do...
            return true;

        case DHCPSTATE_RENEWING:
            // Just send what you got with a DHCPREQUEST in the message
            // type UNICAST straight to the server.  Then wait for an ACK.

            // Fill in the BOOTP request - DHCPREQUEST packet
            xmit->bp_xid = xid;
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0); // No BROADCAST FLAG
            // Use the *client* address here:
            xmit->bp_ciaddr.s_addr = xmit->bp_yiaddr.s_addr;

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPREQUEST, 1 );
            // These must not be set in this context
            unset_tag( xmit, TAG_DHCP_REQ_IP );
            unset_tag( xmit, TAG_DHCP_SERVER_ID );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
            
            // Set unicast address to *server*
            server_addr.sin_addr.s_addr = res->bp_siaddr.s_addr;

#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_RENEWING sending:\n" );
            diag_printf( "UNICAST to family %d, addr %08x, port %d\n",
                         server_addr.sin_family,
                         server_addr.sin_addr.s_addr,
                         server_addr.sin_port );
            show_bootp( intf, xmit );
#endif            
            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send(xmit);
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0,
                       // UNICAST address of the server:
                      (struct sockaddr *)&server_addr,
                      sizeof(server_addr)) < 0) {
                *pstate = DHCPSTATE_FAILED;
                break;
            }

            *pstate = DHCPSTATE_RENEW_RECV;
            break;

        case DHCPSTATE_RENEW_RECV:
            // wait for an ACK or a NACK - retry by going back to
            // DHCPSTATE_RENEWING; NACK means go to NOTBOUND.
            // No answer means just wait for T2, to broadcast.

            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            addrlen = sizeof(rx_addr);
            if (recvfrom(s, received, sizeof(struct bootp), 0,
                         (struct sockaddr *)&rx_addr, &addrlen) < 0) {
                // No packet arrived
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
                    // If we timed out completely, just give up until T2
                    // expires - retain the lease meanwhile.  The normal
                    // lease mechanism will invoke REBINDING as and when
                    // necessary.
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                *pstate = DHCPSTATE_RENEWING;
                break;
            }
            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_RENEW_RECV received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // not the same transaction;
                break;                  // listen again...

            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                length = sizeof(received->bp_siaddr.s_addr);
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // check it was a DHCP reply
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPACK == msgtype  // Same offer?
                     && received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr) {
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    // Record the new lease and set up timers &c
                    new_lease( received, lease );
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                if ( DHCPNAK == msgtype ) { // we're bounced!
                    *pstate = DHCPSTATE_NOTBOUND;  // So quit out.
                    break;
                }
                // otherwise it's something else, maybe another offer.
                // Just listen again, which implicitly discards it.
            }
            break;

        case DHCPSTATE_REBINDING:
            // Just send what you got with a DHCPREQUEST in the message type.
            // Then wait for an ACK.  This one is BROADCAST.

            // Fill in the BOOTP request - DHCPREQUEST packet
            xmit->bp_xid = xid;
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0); // no BROADCAST FLAG
            // Use the *client* address here:
            xmit->bp_ciaddr.s_addr = xmit->bp_yiaddr.s_addr;

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPREQUEST, 1 );
            // These must not be set in this context
            unset_tag( xmit, TAG_DHCP_REQ_IP );
            unset_tag( xmit, TAG_DHCP_SERVER_ID );
            // Set all the tags we want to use when sending a packet
            set_default_dhcp_tags( xmit );
            
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REBINDING sending:\n" );
            show_bootp( intf, xmit );
#endif            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send( xmit );
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0, 
                      (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                *pstate = DHCPSTATE_FAILED;
                break;
            }

            *pstate = DHCPSTATE_REBIND_RECV;
            break;

        case DHCPSTATE_REBIND_RECV:
            // wait for an ACK or a NACK - retry by going back to
            // DHCPSTATE_REBINDING; NACK means go to NOTBOUND.
            // No answer means just wait for expiry; we tried!

            setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            addrlen = sizeof(rx_addr);
            if (recvfrom(s, received, sizeof(struct bootp), 0,
                         (struct sockaddr *)&rx_addr, &addrlen) < 0) {
                // No packet arrived
                // go to the next larger timeout and re-send:
                if ( ! next_timeout( &tv, &timeout_scratch ) ) {
                    // If we timed out completely, just give up until EX
                    // expires - retain the lease meanwhile.  The normal
                    // lease mechanism will invoke NOTBOUND state as and
                    // when necessary.
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                *pstate = DHCPSTATE_REBINDING;
                break;
            }
            // Check for well-formed packet with correct termination (not truncated)
            length = dhcp_size( received );
#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_REBIND_RECV received:\n" );
            if ( length <= 0 )
                diag_printf( "WARNING! malformed or truncated packet\n" );
            diag_printf( "...rx_addr is family %d, addr %08x, port %d\n",
                         rx_addr.sin_family,
                         rx_addr.sin_addr.s_addr,
                         rx_addr.sin_port );
            show_bootp( intf, received );
#endif            
            if ( length <= 0 )
                break;
            if ( CHECK_XID() )          // not the same transaction;
                break;                  // listen again...

            if ( 0 == received->bp_siaddr.s_addr ) {
                // then fill in from the options...
                int length = sizeof(received->bp_siaddr.s_addr );
                get_bootp_option( received, TAG_DHCP_SERVER_ID,
                                  &received->bp_siaddr.s_addr,
                                  &length);
            }

            // check it was a DHCP reply
            length = sizeof(msgtype);
            if ( get_bootp_option( received, TAG_DHCP_MESS_TYPE, &msgtype,
                                   &length) ) {
                if ( DHCPACK == msgtype  // Same offer?
                     && received->bp_yiaddr.s_addr == xmit->bp_yiaddr.s_addr) {
                    // we like the packet, so reset the timeout for next time
                    reset_timeout( &tv, &timeout_scratch );
                    // Record the new lease and set up timers &c
                    new_lease( received, lease );
                    *pstate = DHCPSTATE_BOUND;
                    break;
                }
                else if ( DHCPNAK == msgtype ) { // we're bounced!
                    *pstate = DHCPSTATE_NOTBOUND;  // So back the start of the rigmarole.
                    break;
                }
                // otherwise it's something else, maybe another offer.
                // Just listen again, which implicitly discards it.
            }
            break;

        case DHCPSTATE_BOOTP_FALLBACK:
            // All done with socket
            close(s);
            s = -1;
            
            // And no lease should have become active, but JIC
            no_lease( lease );
            // Re-initialize the interface with the new state
            if ( DHCPSTATE_BOOTP_FALLBACK != oldstate ) {
                // Then need to go down and up
                do_dhcp_down_net( intf, res, &oldstate, lease ); // oldstate used
                if ( 0 != oldstate ) {
                    // Then not called from init_all_network_interfaces()
                    // so we must initialize the interface ourselves
                    if (!init_net(intf, res)) {
                        do_dhcp_down_net( intf, res, pstate, lease );
                        *pstate = DHCPSTATE_FAILED;
                        goto out;
                    }
                }
            }

            // Otherwise, nothing whatsoever to do...
            return true;

        case DHCPSTATE_NOTBOUND:
            // All done with socket
            close(s);
            // No lease active
            no_lease( lease );
            // Leave interface up so app can tidy.
            return true;

        case DHCPSTATE_FAILED:
            // All done with socket
            close(s);
            // No lease active
            no_lease( lease );
            // Unconditionally down the interface.
            do_dhcp_down_net( intf, res, &oldstate, lease );
            return false;

        case DHCPSTATE_DO_RELEASE:
            // We have been forced here by external means, to release the
            // lease for graceful shutdown.

            // Just send what you got with a DHCPRELEASE in the message
            // type UNICAST straight to the server.  No ACK.  Then go to
            // NOTBOUND state.
            NEW_XID( xid );
            xmit->bp_xid = xid;
            xmit->bp_op = BOOTREQUEST;
            xmit->bp_flags = htons(0); // no BROADCAST FLAG
            // Use the *client* address here:
            xmit->bp_ciaddr.s_addr = xmit->bp_yiaddr.s_addr;

            set_fixed_tag( xmit, TAG_DHCP_MESS_TYPE, DHCPRELEASE, 1 );

            // Set unicast address to *server*
            server_addr.sin_addr.s_addr = res->bp_siaddr.s_addr;

#ifdef CYGDBG_NET_DHCP_CHATTER
            diag_printf( "---------DHCPSTATE_DO_RELEASE sending:\n" );
            diag_printf( "UNICAST to family %d, addr %08x, port %d\n",
                         server_addr.sin_family,
                         server_addr.sin_addr.s_addr,
                         server_addr.sin_port );
            show_bootp( intf, xmit );
#endif            
            // Send back a [modified] copy.  Note that some fields are explicitly
            // cleared, as per the RFC.  We need the copy because these fields are
            // still useful to us (and currently stored in the 'result' structure)
            xlen = dhcp_size_for_send( xmit );
            bcopy( xmit, &xmit2, xlen );
            xmit2.bp_yiaddr.s_addr = xmit2.bp_siaddr.s_addr = xmit2.bp_giaddr.s_addr = 0;
            xmit2.bp_hops = 0;
            if(sendto(s, &xmit2, xlen, 0, 
                       // UNICAST address of the server:
                      (struct sockaddr *)&server_addr,
                      sizeof(server_addr)) < 0) {
                *pstate = DHCPSTATE_FAILED;
                break;
            }

            *pstate = DHCPSTATE_NOTBOUND;
            break;

        default:
            no_lease( lease );
            close(s);
            return false;
        }
    }
out:
    if (s != -1) 
      close (s);
    
    return false;
}

// ------------------------------------------------------------------------
// Bring an interface down, failed to initialize it or lease is expired
// Also part of normal startup, bring down for proper reinitialization

int
do_dhcp_down_net(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease)
{
    struct sockaddr_in *addrp;
    struct ifreq ifr;
    int s = -1;
    int retcode = false;

    // Ensure clean slate
    cyg_route_reinit();  // Force any existing routes to be forgotten

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        goto out;
    }

    addrp = (struct sockaddr_in *) &ifr.ifr_addr;

    // Remove any existing address
    if ( DHCPSTATE_FAILED  == *pstate
         || DHCPSTATE_INIT == *pstate
         || 0              == *pstate ) {
        // it was configured for broadcast only, "half-up"
        memset(addrp, 0, sizeof(*addrp));
        addrp->sin_family = AF_INET;
        addrp->sin_len = sizeof(*addrp);
        addrp->sin_port = 0;
        addrp->sin_addr.s_addr = INADDR_ANY;
    }
    else {
        // get the specific address that was used
        strcpy(ifr.ifr_name, intf);
        if (ioctl(s, SIOCGIFADDR, &ifr)) {
            perror("SIOCGIFADDR 1");
            goto out;
        }
    }

    strcpy(ifr.ifr_name, intf);
    if (ioctl(s, SIOCDIFADDR, &ifr)) { /* delete IF addr */
        perror("SIOCDIFADDR1");
    }

#ifdef INET6
    {
      int s6;
      struct if_laddrreq iflr;
      
      s6 = socket(AF_INET6, SOCK_DGRAM, 0);
      if (s6 < 0) {
        perror("socket AF_INET6");
        close (s);
        return false;
      }
      // Now delete the ipv6 addr
      memset(&iflr,0,sizeof(iflr));
      strcpy(iflr.iflr_name, intf);
      if (!ioctl(s6, SIOCGLIFADDR, &iflr)) {
      
	strcpy(iflr.iflr_name, intf);
	if (ioctl(s6, SIOCDLIFADDR, &ifr)) { /* delete IF addr */
	  perror("SIOCDIFADDR_IN61");
	}
      }
      close(s6);
    }
#endif /* IP6 */

    // Shut down interface so it can be reinitialized
    ifr.ifr_flags &= ~(IFF_UP | IFF_RUNNING);
    if (ioctl(s, SIOCSIFFLAGS, &ifr)) { /* set ifnet flags */
        perror("SIOCSIFFLAGS down");
        goto out;
    }
    retcode = true;
 
    if ( 0 != *pstate ) // preserve initial state
        *pstate = DHCPSTATE_INIT;

    
 out:
    if (s != -1)
      close(s);
    
    return retcode;
}

// ------------------------------------------------------------------------
// Release (relinquish) a leased address - if we have one - and bring down
// the interface.
int
do_dhcp_release(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease)
{
    if ( 0                           != *pstate
         && DHCPSTATE_INIT           != *pstate
         && DHCPSTATE_NOTBOUND       != *pstate
         && DHCPSTATE_FAILED         != *pstate
         && DHCPSTATE_BOOTP_FALLBACK != *pstate ) {
        *pstate = DHCPSTATE_DO_RELEASE;
        do_dhcp( intf, res, pstate, lease ); // to send the release packet
        cyg_thread_delay( 100 );             // to let it leave the building
    }
    return true;
}

// ------------------------------------------------------------------------

#endif // CYGPKG_NET_DHCP

// EOF dhcp_prot.c
