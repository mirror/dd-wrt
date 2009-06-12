//==========================================================================
//
//      tests/ftp_test.c
//
//      Simple FTP test
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
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
// FTP test code

#include <network.h>

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

extern void
cyg_test_exit(void);

void
pexit(char *s)
{
    perror(s);
    cyg_test_exit();
}

static void
ftp_test(struct bootp *bp)
{
    int s, len, slen;
    struct sockaddr_in host, local;
    struct servent *sent;
    char buf[256];
    char _USER[] = "USER anonymous\r\n";
    char _PASS[] = "PASS none@abc.com\r\n";
    char _QUIT[] = "QUIT\r\n";

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        pexit("stream socket");
    }
    sent = getservbyname("ftp", "tcp");
    if (sent == (struct servent *)0) {
        pexit("getservbyname");
    }
    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    host.sin_addr = bp->bp_siaddr;
    host.sin_port = sent->s_port; // Network order already
    if (connect(s, (struct sockaddr *)&host, sizeof(host)) < 0) {
        pexit("connect");
    }
    len = sizeof(local);
    if (getsockname(s, (struct sockaddr *)&local, &len) < 0) {
        pexit("getsockname");
    }
    diag_printf("connected to %s.%d", inet_ntoa(host.sin_addr), ntohs(host.sin_port));
    diag_printf(" as %s.%d\n", inet_ntoa(local.sin_addr), ntohs(local.sin_port));
    if ((len = read(s, buf, sizeof(buf))) < 0) {
        pexit("read 1");
    }
    buf[len] = '\0';
    diag_printf(">> %s", buf);
    // Try and log in as 'anonymous'
    slen = strlen(_USER);
    if ((len = write(s, _USER, slen)) != slen) {
        if (len < 0) {
            pexit("write 1");
        } else {
            diag_printf("wrote only %d bytes\n", len);
        }
    }
    if ((len = read(s, buf, sizeof(buf))) < 0) {
        pexit("read 1");
    }
    buf[len] = '\0';
    diag_printf(">> %s", buf);
    slen = strlen(_PASS);
    if ((len = write(s, _PASS, slen)) != slen) {
        if (len < 0) {
            pexit("write 1");
        } else {
            diag_printf("wrote only %d bytes\n", len);
        }
    }
    if ((len = read(s, buf, sizeof(buf))) < 0) {
        pexit("read 2");
    }
    buf[len] = '\0';
    diag_printf(">> %s", buf);
    slen = strlen(_QUIT);
    if ((len = write(s, _QUIT, slen)) != slen) {
        if (len < 0) {
            pexit("write 1");
        } else {
            diag_printf("wrote only %d bytes\n", len);
        }
    }
    while ((len = read(s, buf, sizeof(buf))) > 0) {
        buf[len] = '\0';
        diag_printf(">> %s", buf);
    }
    if (len < 0) {
        perror("read 3");
    }
    close(s);
}

void
net_test(cyg_addrword_t param)
{
    diag_printf("Start FTP test\n");
    init_all_network_interfaces();
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up) {
        ftp_test(&eth0_bootp_data);
    }
#endif
#ifdef CYGHWR_NET_DRIVER_ETH1
    if (eth1_up) {
        ftp_test(&eth1_bootp_data);
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
