#ifndef CYGONCE_NET_TCPIP_DHCP_H
#define CYGONCE_NET_TCPIP_DHCP_H

//==========================================================================
//
//      include/dhcp.h
//
//      DHCP protocol support
//
//==========================================================================
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
// Author(s):    hmt
// Contributors: gthomas
// Date:         2000-07-01
// Purpose:      Support DHCP initialization in eCos TCPIP stack
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// 
// DHCP.  RFC2131, RFC1533, RFC1534
// See also bootp.h
// 

#include <pkgconf/system.h>
#include <pkgconf/net.h>

#ifdef CYGPKG_NET_DHCP

#include <machine/types.h>

#include <cyg/kernel/kapi.h>

#include <bootp.h>

// DHCP messages; these are sent in the tag TAG_DHCP_MESS_TYPE already
// defined in bootp.h

#define DHCPDISCOVER	1
#define DHCPOFFER	2
#define DHCPREQUEST	3
#define DHCPDECLINE	4
#define DHCPACK		5
#define DHCPNAK		6
#define DHCPRELEASE	7

// DHCP interface state machine states; these are published so that app
// code can know what to do... (see page 35 of RFC2131)

// These we will use in the normal course of events
#define DHCPSTATE_INIT		   1
#define DHCPSTATE_SELECTING	   2 // wait for replies to b/c DISCOVER
#define DHCPSTATE_REQUESTING	   3
#define DHCPSTATE_REQUEST_RECV	   4 // wait for replies to b/c REQUEST
#define DHCPSTATE_BOUND		   5
#define DHCPSTATE_RENEWING	   6 // wait for replies to u/c REQUEST
#define DHCPSTATE_RENEW_RECV	   7
#define DHCPSTATE_REBINDING	   8 // wait for replies to b/c REQUEST
#define DHCPSTATE_REBIND_RECV      9
#define DHCPSTATE_BOOTP_FALLBACK  10 // fall back to plain bootp
#define DHCPSTATE_NOTBOUND        11 // To let app tidy up
#define DHCPSTATE_FAILED          12 // Net is down
#define DHCPSTATE_DO_RELEASE      13 // Force release of the current lease
// These we don't use
//#define DHCPSTATE_INITREBOOT
//#define DHCPSTATE_REBOOTING

// These are to let the app inspect the state of the interfaces when
// managing them itself, by analogy with eth0_up &c; eth0_bootp_data and so
// on will still be used with DHCP.
#ifdef CYGHWR_NET_DRIVER_ETH0
extern cyg_uint8   eth0_dhcpstate;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
extern cyg_uint8   eth1_dhcpstate;
#endif

// This is public so the app can wait on it or poll it when managing DHCP
// itself.  It will be zero while the app should wait, and posted when a
// call to do_dhcp() is needed.  If, instead, the app wishes to manage DHCP
// with a thread per interface somehow, then separate semaphores may be used.
// See the dhcp_lease structure definition below.
extern cyg_sem_t dhcp_needs_attention;

// This routine is used at startup time, and after relinquishing leases or
// after a lease timeout: it does DHCP or bootp or static setup according
// to configuration.
extern void init_all_network_interfaces(void);

// This routine does the work of renewing leases &c.
// return value: 1 => everything OK, no change.
// 0 => close your connections, then call do_dhcp_halt() to halt the
// interface(s) in question (it knows because the state will be NOTBOUND).
// After that you can return to the start and use
// init_all_network_interfaces() as usual.
extern int dhcp_bind( void );

// Shutdown any interface which is not already shut down - whether
// initialized by DHCP or not.  Reason: because of the broadcast-while-not-
// -fully-initialized nature of the DHCP conversation, all other interfaces
// must be shut down during that.  So one down, all down, is required.
extern int dhcp_halt( void );

// Release (and set state to DHCPSTATE_NOTBOUND) all interfaces - we are
// closing down.  (unlikely but useful for testing)
// Interfaces are left up; use dhcp_halt() to bring them right down, then
// call init_all_network_interfaces() as usual to restart all.
extern int dhcp_release( void );

// The intent with this API is that a simple DHCP client thread, which
// maintains the state of the interfaces, can go as follows:
// (after init_all_networks is called from elsewhere)
//
//    while ( 1 ) {
//        while ( 1 ) {
//            cyg_semaphore_wait( &dhcp_needs_attention );
//            if ( ! dhcp_bind() ) // a lease expired
//                break; // If we need to re-bind
//        }
//        dhcp_halt(); // tear everything down
//        init_all_network_interfaces(); // re-initialize
//    }
//
// and if the application does not want to suffer the overhead of a
// separate thread and its stack for this, this functionality can be placed
// in the app's server loop in an obvious fashion.  That is the goal of
// breaking out these internal elements.  For example, some server might
// be arranged to poll DHCP from time to time like this:
//
//    while ( 1 ) {
//        init_all_network_interfaces();
//        open-my-listen-sockets();
//        while ( 1 ) {
//            serve-one-request();
//            // sleeps if no connections, but not forever; so this loop is
//            // polled a few times a minute...
//            if ( cyg_semaphore_trywait( &dhcp_needs_attention )) {
//                if ( ! dhcp_bind() ) {
//                    close-my-listen-sockets();
//                    dhcp_halt();
//                    break;
//                }
//            }
//        }
//    }
//
// ------------------------------------------------------------------------

// Set hostname to be used with the DHCP TAG_HOST_NAME option.
// Call this before calling init_all_network_interfaces() to
// set the hostname value.
#ifdef CYGOPT_NET_DHCP_OPTION_HOST_NAME
extern void dhcp_set_hostname(char *hostname);
#else
#define dhcp_set_hostname(hostname)        CYG_EMPTY_STATEMENT
#endif

#ifdef CYGOPT_NET_DHCP_DHCP_THREAD
// Then we provide such a thread...

// Provide a separate thread to renew DHCP leases; otherwise the
// application MUST periodically examine the semaphore dhcp_needs_attention
// and call dhcp_bind() if it is signalled.  If enabled, this thread does
// all that for you.  Independent of this option, initialization of the
// interfaces still occurs in init_all_network_interfaces() and your
// startup code must call that.  It will start the DHCP management thread
// if necessary.  If a lease fails to be renewed, the management thread
// will shut down all interfaces and attempt to initialize all the
// interfaces again from scratch.  This may cause chaos in the app, which
// is why managing the DHCP state in an application aware thread is
// actually better, just far less convenient for testing.

extern cyg_handle_t dhcp_mgt_thread_h; // To allow its external manipulation.
extern cyg_thread   dhcp_mgt_thread;   // The object itself

extern void dhcp_start_dhcp_mgt_thread( void );

#endif

// The function is provided unconditionally so that the app can put it in a
// thread of its own.  If the parameter is true, it loops forever; if
// false, the call returns if a lease expires, and the caller must tidy up
// or reboot the whole machine.
extern cyg_thread_entry_t dhcp_mgt_entry;
extern void dhcp_mgt_entry( cyg_addrword_t loop_on_failure ); // the function

// ---------------------------------------------------------------------------
// These are rather more internal routines, internal to the protocol engine
// in dhcp_prot.c - those above are in dhcp_support.c

#define DHCP_LEASE_T1 1
#define DHCP_LEASE_T2 2
#define DHCP_LEASE_EX 4

struct dhcp_lease {
    // Client settable: Semaphore to signal when attention is needed:
    cyg_sem_t          *needs_attention;
    // Initialize all the rest to zero:
    cyg_tick_count_t    t1, t2, expiry;
    volatile cyg_uint8  next;
    volatile cyg_uint8  which;
    cyg_handle_t        alarm;
    // except this one, which is just some space:
    cyg_alarm           alarm_obj;
};

// These dhcp_lease objects are initialized to use
//         extern cyg_sem_t dhcp_needs_attention;
// for the semaphore.
#ifdef CYGHWR_NET_DRIVER_ETH0
extern struct dhcp_lease eth0_lease;
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
extern struct dhcp_lease eth1_lease;
#endif

extern int
do_dhcp(const char *interface, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease);
// NB *res and *pstate and *lease are all INOUT; *res must point to a valid
// record from "last time".

extern int
do_dhcp_down_net(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease);

extern int
do_dhcp_release(const char *intf, struct bootp *res,
        cyg_uint8 *pstate, struct dhcp_lease *lease);

#endif // CYGPKG_NET_DHCP

#endif // CYGONCE_NET_TCPIP_DHCP_H
// EOF dhcp.h
