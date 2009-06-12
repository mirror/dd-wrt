//==========================================================================
//
//      tests/server_test.c
//
//      Simple TCP 'server' test - using getaddrinfo()
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
// Date:         2002-03-05
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
// Network server test code
#include <stdio.h>
#include <stdlib.h>
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

#define MAXSOCK 16
static void
server_test(struct bootp *bp)
{
    int sock_indx, i, s, socks[MAXSOCK], client, client_len;
    struct sockaddr client_addr;
    char buf[256], addr_buf[256];
    char host_addr_buf[256], host_port_buf[32];
    int one = 1;
    fd_set in_fds, src_fds;
    int num, len;
    struct timeval tv;
    struct addrinfo *ai, *addrs, hints;
    int err, last_sock;

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo failed");
    }
    sock_indx = 0;  last_sock = -1;
    ai = addrs;
    while (ai) {
        _inet_ntop(ai->ai_addr, addr_buf, sizeof(addr_buf));
        diag_printf("Family: %d, Socket: %d, Addr: %s\n", ai->ai_family, ai->ai_socktype, addr_buf);
        s = socket(ai->ai_family, ai->ai_socktype, 0);
        if (s < 0) {
            pexit("stream socket");
        }
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
            pexit("setsockopt SO_REUSEADDR");
        }
        if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
            pexit("setsockopt SO_REUSEPORT");
        }
        if(bind(s, ai->ai_addr, ai->ai_addr->sa_len) < 0) {
            pexit("bind error");
        }
        listen(s, SOMAXCONN);
        socks[sock_indx++] = s;
        if (sock_indx >= MAXSOCK) {
            pexit("Too many address types");
        }
        ai = ai->ai_next;
        if (s > last_sock) last_sock = s;
    }
    while (true) {
        // Wait for some activity on one of the ports
        FD_ZERO(&src_fds);
        for (s = 0;  s < sock_indx;  s++) {
            FD_SET(socks[s], &src_fds);
        }
        num = select(last_sock+1, &src_fds, 0, 0, 0);
        if (num > 0) {
            // There are 'num' sockets ready to connect
            for (i = 0;  i < sock_indx; i++) {
                s = socks[i];
                if (FD_ISSET(s, &src_fds)) {
                    client_len = sizeof(client_addr);
                    if ((client = accept(s, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                        pexit("accept");
                    }
                    client_len = sizeof(client_addr);
                    getpeername(client, &client_addr, &client_len);
                    if (getnameinfo (&client_addr, client_len, 
                                     host_addr_buf, sizeof(host_addr_buf),
                                     host_port_buf, sizeof(host_port_buf), 
                                     NI_NUMERIC) == EAI_NONE) {
                        diag_printf("connection from %s(%s)\n", host_addr_buf, host_port_buf);
                        diag_sprintf(buf, "Hello %s(%s)\n", host_addr_buf, host_port_buf);
                    } else {
                        _inet_ntop(&client_addr, addr_buf, sizeof(addr_buf));
                        diag_printf("connection from %s(%d)\n", addr_buf, _inet_port(&client_addr));
                        diag_sprintf(buf, "Hello %s(%d)\n", addr_buf, _inet_port(&client_addr));
                    }
                    write(client, buf, strlen(buf));
                    tv.tv_sec = 5;
                    tv.tv_usec = 0;
                    FD_ZERO(&in_fds);
                    FD_SET(client, &in_fds);
                    num = select(client+1, &in_fds, 0, 0, &tv);
                    if (num > 0) {
                        len = read(client, buf, sizeof(buf)-1);
                        buf[len-1] = '\0';  // Trim \n
                        diag_printf("buf = '%s'\n", buf);
                    } else if (num == 0) {
                        diag_printf("No reply - timed out\n");
                    } else {
                        perror("select");
                    }
                    close(client);
                }
            }
        } else {
            diag_printf("select returned: %d\n", num);
            pexit("bad select");
        }
    }
}

void
net_test(cyg_addrword_t param)
{
    diag_printf("Start SERVER test\n");
    init_all_network_interfaces();
#ifdef CYGHWR_NET_DRIVER_ETH0
    if (eth0_up) {
        server_test(&eth0_bootp_data);
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
