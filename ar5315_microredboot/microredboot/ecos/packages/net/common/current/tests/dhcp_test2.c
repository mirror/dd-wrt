//==========================================================================
//
//      tests/dhcp_test2.c
//
//      Test of repeatedly releasing and reacquiring DHCP leases
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
// Author(s):    hmt, tomislav.sostaric@ascom.ch
// Contributors: 
// Date:         2002-03-11
// Purpose:      Test repeated up-down cycles of interfaces with DHCP.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

/* 
 * A test program for bringing out the DHCP memory leak bug
 * This file is in the public domain and may be used for any purpose
 */

/* INCLUDES */

#include <network.h>

#include <pkgconf/system.h>
#include <pkgconf/net.h>

#include <cyg/infra/testcase.h>

#ifdef CYGPKG_NET_DHCP

#ifdef CYGBLD_DEVS_ETH_DEVICE_H    // Get the device config if it exists
#include CYGBLD_DEVS_ETH_DEVICE_H  // May provide CYGTST_DEVS_ETH_TEST_NET_REALTIME
#endif


#include <stdio.h>                      /* printf */
#include <string.h>                     /* strlen */
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
#include <cyg/io/io.h>                  /* I/O functions */
#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */
#include <network.h>
#include <dhcp.h>

// ------------------------------------------------------------------------

#define FALSE 0
#define TRUE 1
#define TICKS_PER_SEC 100

#define nLOOPS 1000000
#define LOOPS 10

// ------------------------------------------------------------------------

#define NINTERFACES (2)

struct bootp bootp_data[ NINTERFACES ];
cyg_uint8 state[ NINTERFACES ];
struct dhcp_lease lease[ NINTERFACES ];
int up[ NINTERFACES ], oldup[ NINTERFACES ];

char *eth[ NINTERFACES ] = { "eth0", "eth1" };

int counter = 0;

void
tdhcp_init( int which )
{
    printf("%s: Initializing device to use DHCP.\n", eth[which] );
    bzero( &bootp_data[which], sizeof( *bootp_data ) );
    state[which] = 0;
    bzero( &lease[which], sizeof( *lease ) );
    up[which] = oldup[which] = FALSE;
}


void
tdhcp_do_one( int which )
{
    oldup[which] = up[which];
    up[which] = do_dhcp( eth[which], &bootp_data[which], &state[which], &lease[which] );
}

void
tdhcp_updown_one( int which )
{
    /* DHCP wants interface to go up or down, do it. */
    if (up[which] != oldup[which]) {
	if (up[which]) {
	    char tbuf[256];
	    int result;

	    result = init_net( eth[which], &bootp_data[which]);
	    if (!result) {
		printf("%s: Initialization (DHCP) failed.\n", eth[which] );
		return;
	    }
	    
	    if (lease[which].expiry == (cyg_tick_count_t)-1) {
		strcpy(tbuf, "infinite");
	    } else {
		cyg_tick_count_t now;
		unsigned int exp;
		
		now = cyg_current_time();
		exp = ((lease[which].expiry > now) ? lease[which].expiry - now : 0) / TICKS_PER_SEC;
		sprintf(tbuf, "%ud %uh %um %us", exp / 86400, 
			(exp / 3600) % 24, (exp / 60) % 60, exp % 60);
	    }
	    printf("%s: Configured by DHCP, IP address %s, "
		   "lease expiry: %s.\n", eth[which], 
		   inet_ntoa(bootp_data[which].bp_yiaddr), tbuf);
	    printf("%s: Interface ready\n", eth[which] );
	} else {
	    printf("%s: Deconfigured by DHCP.\n", eth[which] );
	    cyg_thread_delay(10);
	    do_dhcp_down_net( eth[which], &bootp_data[which], &state[which], &lease[which]);
	    state[which] = DHCPSTATE_INIT;
	}
    }
}

void
tdhcp_release_one( int which )
{
    /* If DHCP failed (most probably because there was no DHCP server around),
     * sleep a bit, then try again. Otherwise, just wait until DHCP needs our
     * attention.
     */
    if (state[which] == DHCPSTATE_FAILED) {
	printf("%s: DHCP failed, will retry later.\n", eth[which] );
	cyg_thread_delay(10);
	printf("%s: Retrying DHCP.\n", eth[which] );
    }
    cyg_thread_delay(10);
    printf("%s: Releasing DHCP lease.\n", eth[which] );
    do_dhcp_release( eth[which], &bootp_data[which], &state[which], &lease[which]);
    cyg_thread_delay(10);
    up[which] = FALSE;
    state[which] = DHCPSTATE_INIT;
}


static void
dhcp_if_fn(cyg_addrword_t data)
{
    CYG_TEST_INIT();

#ifdef CYGHWR_NET_DRIVER_ETH0
    tdhcp_init( 0 );
#endif // CYGHWR_NET_DRIVER_ETH0
#ifdef CYGHWR_NET_DRIVER_ETH1
    tdhcp_init( 1 );
#endif // CYGHWR_NET_DRIVER_ETH1
    
    while ( ++counter < LOOPS ) {
	diag_printf( "--------- counter %d ---------\n", counter );
#ifdef CYGHWR_NET_DRIVER_ETH0
	tdhcp_do_one( 0 );
#endif // CYGHWR_NET_DRIVER_ETH0
#ifdef CYGHWR_NET_DRIVER_ETH1
	tdhcp_do_one( 1 );
#endif // CYGHWR_NET_DRIVER_ETH1

#ifdef CYGHWR_NET_DRIVER_ETH0
	tdhcp_updown_one( 0 );
#endif // CYGHWR_NET_DRIVER_ETH0
#ifdef CYGHWR_NET_DRIVER_ETH1
	tdhcp_updown_one( 1 );
#endif // CYGHWR_NET_DRIVER_ETH1

#ifdef CYGHWR_NET_DRIVER_ETH0
	tdhcp_release_one( 0 );
#endif // CYGHWR_NET_DRIVER_ETH0
#ifdef CYGHWR_NET_DRIVER_ETH1
	tdhcp_release_one( 1 );
#endif // CYGHWR_NET_DRIVER_ETH1
    }
    CYG_TEST_PASS_EXIT( "All done" );
}


// ------------------------------------------------------------------------

#define NTHREADS 1
#define STACKSIZE ( CYGNUM_HAL_STACK_SIZE_TYPICAL + 4096 )

static cyg_handle_t thread[NTHREADS];
static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

void cyg_user_start(void)
{
    // Use the DHCP thread prio as provided even though we're not using the thread itself;
    // This priroty should be right (lower than net thread prio) by default.
    cyg_thread_create(CYGPKG_NET_DHCP_THREAD_PRIORITY, dhcp_if_fn, (cyg_addrword_t) 0, "dhcp",
                      (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);
}

// ------------------------------------------------------------------------

#else // CYGPKG_NET_DHCP

void cyg_user_start(void)
{
    CYG_TEST_INIT();
    CYG_TEST_NA( "DHCP is not enabled" );
    CYG_TEST_EXIT( "DHCP is not enabled" );
}

#endif // CYGPKG_NET_DHCP

// ------------------------------------------------------------------------

// EOF dhcp_test2.c
