//==========================================================================
//
//      tests/set_mac_address.c
//
//      Set_Mac_Address Utility - this carefully does NOTHING unless you
//      edit this source file to confirm that you really want to do it.
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
// Contributors: 
// Date:         2000-05-03
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#ifdef CYGBLD_DEVS_ETH_DEVICE_H    // Get the device config if it exists
#include CYGBLD_DEVS_ETH_DEVICE_H
#endif


// SET_MAC_ADDRESS test code

#include <network.h>

#include <netinet/if_ether.h>

#define NUMTHREADS 1
#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char thread_stack[NUMTHREADS][STACK_SIZE];
static cyg_thread thread_data[NUMTHREADS];
static cyg_handle_t thread_handle[NUMTHREADS];

extern void
cyg_test_exit(void);

void
pexit(char *s)
{
    perror(s);
    cyg_test_exit();
}

// ------------------------------------------------------------------------
// remove NT to make this utility be useful ;-)
#define DONT_SET_ETH0
#define DONT_SET_ETH1

// These are commented out to make sure you choose a value:
#ifdef DO_SET_ETH0
//static cyg_uint8 new_eth0_addr[6]={ 0x0,0x90,0x27,0x8c,0x57,0xdd};
//static cyg_uint8 new_eth0_addr[6]={ 0x0,0x90,0x27,0x8c,0x57,0xdb};
#endif
#ifdef DO_SET_ETH1
//static cyg_uint8 new_eth1_addr[6]={ 0x0,0x90,0x27,0x8c,0x57,0xde};
//static cyg_uint8 new_eth1_addr[6]={ 0x0,0x90,0x27,0x8c,0x57,0xdc};
#endif

// ------------------------------------------------------------------------

int
set_mac_address( const char *interface, char *mac_address )
{
    int s, i;
    struct ifreq ifr;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return false;
    }

    diag_printf( "%s socket is %d:\n", interface, s );

    strcpy(ifr.ifr_name, interface);

    for ( i = 0; i < ETHER_ADDR_LEN; i++ )
        ifr.ifr_hwaddr.sa_data[i] = mac_address[i];

    diag_printf( "Mac addr %02x:%02x:%02x:%02x:%02x:%02x\n",
                 ifr.ifr_hwaddr.sa_data[0],
                 ifr.ifr_hwaddr.sa_data[1],
                 ifr.ifr_hwaddr.sa_data[2],
                 ifr.ifr_hwaddr.sa_data[3],
                 ifr.ifr_hwaddr.sa_data[4],
                 ifr.ifr_hwaddr.sa_data[5] );

    if (ioctl(s, SIOCSIFHWADDR, &ifr)) {
        perror("SIOCSIFHWADDR");
        close( s );
        return false;
    }

    diag_printf( "%s ioctl(SIOCSIFHWADDR) succeeded\n", interface );

    close( s );

    return true;
}

// ------------------------------------------------------------------------
void
net_test(cyg_addrword_t param)
{
    int results = 0;
    diag_printf("Start set_mac_address\n");
#ifdef CYGHWR_NET_DRIVER_ETH0
#ifdef DO_SET_ETH0
    diag_printf("Setting MAC of eth0 to %02x:%02x:%02x:%02x:%02x:%02x\n",
                new_eth0_addr[0],new_eth0_addr[1],
                new_eth0_addr[2],new_eth0_addr[3],
                new_eth0_addr[4],new_eth0_addr[5] );
    results += set_mac_address( "eth0", new_eth0_addr );
#endif
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
#ifdef DO_SET_ETH1
    diag_printf("Setting MAC of eth1 to %02x:%02x:%02x:%02x:%02x:%02x\n",
                new_eth1_addr[0],new_eth1_addr[1],
                new_eth1_addr[2],new_eth1_addr[3],
                new_eth1_addr[4],new_eth1_addr[5] );
    results += set_mac_address( "eth1", new_eth1_addr );
#endif
#endif

    if ( 0 == results )
        diag_printf( "**** Did not set any MAC addresses ****\n" );

    diag_printf("Init Network Interfaces\n");
    init_all_network_interfaces();
    diag_printf("After init.\n");

    cyg_test_exit();
}

// ------------------------------------------------------------------------
void
cyg_start(void)
{
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      net_test,          // entry
                      0,                 // entry parameter
                      "Network test",    // Name
                     &thread_stack[0][0], // Stack
                      STACK_SIZE,        // Size
                      &thread_handle[0], // Handle
                      &thread_data[0]    // Thread data structure
            );
    cyg_thread_resume(thread_handle[0]);  // Start it

    cyg_scheduler_start();
}

// EOF set_mac_address.c
