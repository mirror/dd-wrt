//==========================================================================
//
//      tests/tftp_client_test.c
//
//      Simple TFTP client test
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2000-04-07
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
// TFTP test code

#include <network.h>
#include <tftp_support.h>

// Note: the TFTP client calls need at least (SEGSIZE==512)+4
// additional bytes of workspace, thus the padding.
#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL+0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

#define min(x,y) (x<y ? x : y)

extern void
cyg_test_exit(void);

void
pexit(char *s)
{
    perror(s);
    cyg_test_exit();
}

static char buf[32*1024];

#define GETFILE "/tftpboot/tftp_get"
#define PUTFILE "/tftpboot/tftp_put"

static void
tftp_test(struct bootp *bp)
{
    int res, err, len;
    struct sockaddr_in host;
#ifdef CYGPKG_NET_INET6
    struct sockaddr_in6 ipv6router;
    char server[64];
#endif

    memset((char *)&host, 0, sizeof(host));
    host.sin_len = sizeof(host);
    host.sin_family = AF_INET;
    host.sin_addr = bp->bp_siaddr;
    host.sin_port = 0;
    diag_printf("Trying tftp_get %s %16s...\n", GETFILE, inet_ntoa(host.sin_addr));
    res = tftp_get( GETFILE, &host, buf, sizeof(buf), TFTP_OCTET, &err);
    diag_printf("res = %d, err = %d\n", res, err);
    if (res > 0) {
        diag_dump_buf(buf, min(res,1024));
    }
    len = res;
    diag_printf("Trying tftp_put %s %16s, length %d\n",
                PUTFILE, inet_ntoa(host.sin_addr), len);
    res = tftp_put( PUTFILE, &host, buf, len, TFTP_OCTET, &err);
    diag_printf("put - res: %d\n", res);

#ifdef CYGPKG_NET_INET6
    // Wait for router solicit process to happen.
    if (!cyg_net_get_ipv6_advrouter(&ipv6router)) {
      diag_printf("No router advertisement recieved\n");
      cyg_test_exit();
    }

    getnameinfo((struct sockaddr *)&ipv6router,sizeof(ipv6router),
		server, sizeof(server), 0 ,0 ,NI_NUMERICHOST);

    diag_printf("Trying tftp_get %s using IPv6 from %16s...\n", GETFILE, server);

    res = tftp_client_get( GETFILE, server, 0, buf, sizeof(buf), 
			   TFTP_OCTET, &err);
    diag_printf("IPv6 res = %d, err = %d\n", res, err);
    if (res > 0) {
        diag_dump_buf(buf, min(res,1024));
    }
    len = res;
    diag_printf("Trying tftp_put %s using IPv6 to %16s, length %d\n",
                PUTFILE, server, len);
    res = tftp_client_put( PUTFILE, server, 0, buf, len, TFTP_OCTET, &err);
    diag_printf("put - res: %d\n", res);
#endif
}

void
net_test(cyg_addrword_t param)
{
    diag_printf("Start TFTP test\n");
    init_all_network_interfaces();
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up) {
        tftp_test(&eth0_bootp_data);
    }
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if (eth1_up) {
        tftp_test(&eth1_bootp_data);
    }
#endif
    cyg_test_exit();
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


