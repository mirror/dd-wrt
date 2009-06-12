//==========================================================================
//
//      tests/socket_test.c
//
//      Test network socket functions
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
// Date:         2000-01-10
// Purpose:      HLD
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// socket test code

#include <network.h>

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

extern void
cyg_test_exit(void);

void
net_test(cyg_addrword_t param)
{
    int s;
    int one = 1;

    diag_printf("Start socket test\n");

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    diag_printf("socket() = %d\n", s);
    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    diag_printf("socket() = %d\n", s);

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one))) {        
        perror("setsockopt");
    }

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
