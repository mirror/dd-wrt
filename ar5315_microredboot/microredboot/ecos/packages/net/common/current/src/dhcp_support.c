/*==========================================================================
//
//      dhcp_support.c
//
//      Support code == friendly API for DHCP client
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
// Author(s):   hmt
// Contributors: gthomas
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

#include <network.h>
#include <dhcp.h>

// ---------------------------------------------------------------------------
#ifdef CYGHWR_NET_DRIVER_ETH0
cyg_uint8   eth0_dhcpstate = 0;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
cyg_uint8   eth1_dhcpstate = 0;
#endif

cyg_sem_t dhcp_needs_attention;

#ifdef CYGHWR_NET_DRIVER_ETH0
struct dhcp_lease eth0_lease = { &dhcp_needs_attention, 0,0,0,0,0,0 };
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
struct dhcp_lease eth1_lease = { &dhcp_needs_attention, 0,0,0,0,0,0 };
#endif

// ---------------------------------------------------------------------------
//
// The point of this module is to deal with all the horrid written out in
// full stuff of having two interfaces; it's ugly but it's also most
// flexible.  The dhcp_prot.c module should do all the work... 
//
// ---------------------------------------------------------------------------

// return value: 1 => everything OK, no change.
// 0 => close your connections, then call do_dhcp_halt() to halt the
// interface(s) in question (it knows because the state will be NOTBOUND).
// After that you can return to the start and use
// init_all_network_interfaces(); as usual, or call do_dhcp_bind() by hand,
// or whatever...
int dhcp_bind( void )
{
#ifdef CYGHWR_NET_DRIVER_ETH0
    cyg_uint8 old_eth0_dhcpstate = eth0_dhcpstate;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    cyg_uint8 old_eth1_dhcpstate = eth1_dhcpstate;
#endif

    // If there are no interfaces at all, init it every time, doesn't
    // matter.  In case we are called from elsewhere...
    if ( 1
#ifdef CYGHWR_NET_DRIVER_ETH0
         && eth0_dhcpstate == 0
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
         && eth1_dhcpstate == 0
#endif
        )
        cyg_semaphore_init( &dhcp_needs_attention, 0 );

    // Run the state machine...
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up
        && DHCPSTATE_FAILED != eth0_dhcpstate )
            eth0_up = do_dhcp(eth0_name, &eth0_bootp_data, &eth0_dhcpstate, &eth0_lease);
#endif            
#ifdef CYGHWR_NET_DRIVER_ETH1
    if (eth1_up
        && DHCPSTATE_FAILED != eth1_dhcpstate )
            eth1_up = do_dhcp(eth1_name, &eth1_bootp_data, &eth1_dhcpstate, &eth1_lease);
#endif            

    // If the interface newly came up, initialize it:
    // (this duplicates the code in init_all_network_interfaces() really).
#ifdef CYGHWR_NET_DRIVER_ETH0
    if ( eth0_up
         && eth0_dhcpstate == DHCPSTATE_BOUND
         && old_eth0_dhcpstate != eth0_dhcpstate ) {
        if (!init_net(eth0_name, &eth0_bootp_data)) {
            eth0_up = false;
        }
    }
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if ( eth1_up
         && eth1_dhcpstate == DHCPSTATE_BOUND
         && old_eth1_dhcpstate != eth1_dhcpstate ) {
        if (!init_net(eth1_name, &eth1_bootp_data)) {
            eth1_up = false;
        }
    }
#endif

#ifdef CYGHWR_NET_DRIVER_ETH0
    if ( old_eth0_dhcpstate == DHCPSTATE_BOUND &&
         eth0_dhcpstate == DHCPSTATE_NOTBOUND )
        return 0; // a lease timed out; we became unbound
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if ( old_eth1_dhcpstate == DHCPSTATE_BOUND &&
         eth1_dhcpstate == DHCPSTATE_NOTBOUND )
        return 0; // a lease timed out; we became unbound
#endif
    return 1; // all is well
}


// Shutdown any interface whose state is DHCPSTATE_NOTBOUND.
int dhcp_halt( void )
{
#ifdef CYGHWR_NET_DRIVER_ETH0
    if ( eth0_up
         && eth0_dhcpstate != DHCPSTATE_FAILED ) {
        do_dhcp_down_net(eth0_name, &eth0_bootp_data, &eth0_dhcpstate, &eth0_lease);
    }
    eth0_up = false;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if ( eth1_up
         && eth1_dhcpstate != DHCPSTATE_FAILED ) {
        do_dhcp_down_net(eth1_name, &eth1_bootp_data, &eth1_dhcpstate, &eth1_lease);
    }
    eth1_up = false;
#endif
    return 1;
}


// Release (and set state to DHCPSTATE_NOTBOUND) all interfaces - we are
// closing down.  (unlikely but maybe useful for testing)
int dhcp_release( void )
{
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up)
        do_dhcp_release(eth0_name, &eth0_bootp_data, &eth0_dhcpstate, &eth0_lease);
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if (eth1_up)
        do_dhcp_release(eth1_name, &eth1_bootp_data, &eth1_dhcpstate, &eth1_lease);
#endif
    return 1;
}


// ------------------------------------------------------------------------
// The management thread function
void dhcp_mgt_entry( cyg_addrword_t loop_on_failure )
{
    int j;
    while ( 1 ) {
        while ( 1 ) {
            cyg_semaphore_wait( &dhcp_needs_attention );
            if ( ! dhcp_bind() ) // a lease expired
                break; // If we need to re-bind
        }
        dhcp_halt(); // tear everything down
        if ( !loop_on_failure )
            return; // exit the thread/return
        init_all_network_interfaces(); // re-initialize
        for ( j = 0; j < CYGPKG_NET_NLOOP; j++ )
            init_loopback_interface( j );
#ifdef CYGPKG_SNMPAGENT
        SnmpdShutDown(0); // Cycle the snmpd state
#endif
    }
}

#ifdef CYGOPT_NET_DHCP_DHCP_THREAD
// Then we provide such a thread...
cyg_handle_t dhcp_mgt_thread_h = 0;
cyg_thread   dhcp_mgt_thread;

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + sizeof(struct bootp))
static cyg_uint8 dhcp_mgt_stack[ STACK_SIZE ];

void dhcp_start_dhcp_mgt_thread( void )
{
    if ( ! dhcp_mgt_thread_h ) {
        cyg_semaphore_init( &dhcp_needs_attention, 0 );
        cyg_thread_create(
            CYGPKG_NET_DHCP_THREAD_PRIORITY, /* scheduling info (eg pri) */
            dhcp_mgt_entry,             /* entry point function */
            CYGOPT_NET_DHCP_DHCP_THREAD_PARAM, /* entry data */
            "DHCP lease mgt",           /* optional thread name */
            dhcp_mgt_stack,             /* stack base, NULL = alloc */
            STACK_SIZE,                 /* stack size, 0 = default */
            &dhcp_mgt_thread_h,         /* returned thread handle */
            &dhcp_mgt_thread           /* put thread here */
            );

        cyg_thread_resume(dhcp_mgt_thread_h);
    }
}


#endif // CYGOPT_NET_DHCP_DHCP_THREAD

#endif // CYGPKG_NET_DHCP

// EOF dhcp_support.c
