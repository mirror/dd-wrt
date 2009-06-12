//==========================================================================
//
//      tests/nc_test_slave.c
//
//      Network characterizations test (slave portion)
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Portions created by Nick Garnett are
// Copyright (C) 2003 eCosCentric Ltd.
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD, 
// FreeBSD or other sources, and are covered by the appropriate
// copyright disclaimers included herein.
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

// Network characterization test code - slave portion

#include <cyg/ppp/ppp.h>

#include <cyg/infra/testcase.h>

#include "nc_test_framework.h"
#include <math.h>

#ifdef __ECOS

#include "ppp_test_support.inl"

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif
#define STACK_SIZE               (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
#define MAX_LOAD_THREAD_LEVEL    20
#define MIN_LOAD_THREAD_LEVEL    0
#define NUM_LOAD_THREADS         10
#define IDLE_THREAD_PRIORITY     CYGNUM_PPP_PPPD_THREAD_PRIORITY+3
#define LOAD_THREAD_PRIORITY     CYGPKG_NET_THREAD_PRIORITY-1
#define MAIN_THREAD_PRIORITY     CYGPKG_NET_THREAD_PRIORITY-2
#define DESIRED_BACKGROUND_LOAD  20
static char         main_thread_stack[CYGHWR_NET_DRIVERS][STACK_SIZE];
static cyg_thread   main_thread_data[CYGHWR_NET_DRIVERS];
static cyg_handle_t main_thread_handle[CYGHWR_NET_DRIVERS];
static char         idle_thread_stack[STACK_SIZE];
static cyg_thread   idle_thread_data;
static cyg_handle_t idle_thread_handle;
static cyg_sem_t    idle_thread_sem;
volatile static long long    idle_thread_count;
static cyg_tick_count_t idle_thread_start_time;
static cyg_tick_count_t idle_thread_stop_time;
static char         load_thread_stack[NUM_LOAD_THREADS][STACK_SIZE];
static cyg_thread   load_thread_data[NUM_LOAD_THREADS];
static cyg_handle_t load_thread_handle[NUM_LOAD_THREADS];
static cyg_sem_t    load_thread_sem[NUM_LOAD_THREADS];
static long         load_thread_level;
static void calibrate_load(int load);
static void start_load(int load);
static void do_some_random_computation(int p);
#define abs(n) ((n) < 0 ? -(n) : (n))
#endif

#ifdef __ECOS
#define test_param_t cyg_addrword_t
#ifdef CYGDBG_NET_TIMING_STATS
extern void show_net_times(void);
#endif
#else
#define test_param_t int
#endif

#define MAX_BUF 8192
static unsigned char in_buf[MAX_BUF], out_buf[MAX_BUF];

#ifdef __ECOS
extern void
cyg_test_exit(void);
#else
void
cyg_test_exit(void)
{
    test_printf("... Done\n");
    exit(1);
}

static void
show_net_times(void)
{
}
#endif

#ifdef __ECOS
static void
test_delay(int ticks)
{
    cyg_thread_delay(ticks);
}

#else

static void
test_delay(int ticks)
{
    usleep(ticks * 10000);
}
#endif

void
pexit(char *s)
{
    perror(s);
#ifdef CYGDBG_NET_TIMING_STATS
    show_net_times();
#endif
    cyg_test_exit();
}

//
// Generic UDP test
//

static void
do_udp_test(int s1, struct nc_request *req, struct sockaddr_in *master)
{
    int i, s, td_len, seq, seq_errors, lost;
    struct sockaddr_in test_chan_slave, test_chan_master;
    fd_set fds;
    struct timeval timeout;
    struct nc_test_results results;
    struct nc_test_data *tdp;
    int nsent, nrecvd;
    int need_recv, need_send;

    need_recv = true;  need_send = true;
    switch (ntohl(req->type)) {
    case NC_REQUEST_UDP_SEND:
        need_recv = false;
        need_send = true;
        break;
    case NC_REQUEST_UDP_RECV:
        need_recv = true;
        need_send = false;
        break;
    case NC_REQUEST_UDP_ECHO:
        break;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        pexit("datagram socket");
    }

    memset((char *) &test_chan_slave, 0, sizeof(test_chan_slave));
    test_chan_slave.sin_family = AF_INET;
#ifdef __ECOS
    test_chan_slave.sin_len = sizeof(test_chan_slave);
#endif
    test_chan_slave.sin_addr.s_addr = htonl(INADDR_ANY);
    test_chan_slave.sin_port = htons(ntohl(req->slave_port));
    
    if (bind(s, (struct sockaddr *) &test_chan_slave, sizeof(test_chan_slave)) < 0) {
        perror("bind");
        close(s);
    }

    memcpy(&test_chan_master, master, sizeof(*master));
    test_chan_master.sin_port = htons(ntohl(req->master_port));
    nsent = 0;  nrecvd = 0;  seq = 0;  seq_errors = 0;  lost = 0;
    for (i = 0;  i < ntohl(req->nbufs);  i++) {
        if (need_recv) {
            FD_ZERO(&fds);
            FD_SET(s, &fds);
            timeout.tv_sec = NC_TEST_TIMEOUT;
            timeout.tv_usec = 0;
            if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
                test_printf("recvfrom timeout, expecting seq #%d\n", seq);            
                if (++lost > MAX_ERRORS) {
                    test_printf("... giving up\n");
                    break;
                }
            } else {
                nrecvd++;
                tdp = (struct nc_test_data *)in_buf;
                td_len = ntohl(req->buflen) + sizeof(struct nc_test_data);
                if (recvfrom(s, tdp, td_len, 0, 0, 0) < 0) {
                    perror("recvfrom");
                    close(s);
                    return;
                }
                if ((ntohl(tdp->key1) == NC_TEST_DATA_KEY1) &&
                    (ntohl(tdp->key2) == NC_TEST_DATA_KEY2)) {
                    if (ntohl(tdp->seq) != seq) {
                        test_printf("Packets out of sequence - recvd: %d, expected: %d\n",
                                    ntohl(tdp->seq), seq);
                        seq = ntohl(tdp->seq);
                        seq_errors++;
                    }
                } else {
                    test_printf("Bad data packet - key: %x/%x, seq: %d\n",
                                ntohl(tdp->key1), ntohl(tdp->key2),
                                ntohl(tdp->seq));
                }
            }
        }
        if (need_send) {
            int retries = 10;
            int sent = false;
            int res;

            tdp = (struct nc_test_data *)out_buf;
            tdp->key1 = htonl(NC_TEST_DATA_KEY1);
            tdp->key2 = htonl(NC_TEST_DATA_KEY2);
            tdp->seq = htonl(seq);
            td_len = ntohl(req->buflen) + sizeof(struct nc_test_data);
            tdp->len = htonl(td_len);
            while (!sent && (--retries >= 0)) {
                res = sendto(s, tdp, td_len, 0, 
                             (struct sockaddr *)&test_chan_master, sizeof(test_chan_master));
                if (res > 0) {
                    sent = true;
                    break;
                }
                if (errno == ENOBUFS) {
                    // Saturated the system
                    test_delay(1);   // Time for 200 500 byte 10-baseT packets 
                } else {
                    // What else to do?
                    close(s);
                    return;
                }
            }
            if (sent) {
                nsent++;
            } else {
                perror("sendto");
            }
        }
        seq++;
    }
    results.key1 = htonl(NC_TEST_RESULT_KEY1);
    results.key2 = htonl(NC_TEST_RESULT_KEY2);
    results.seq = req->seq;
    results.nsent = htonl(nsent);
    results.nrecvd = htonl(nrecvd);
    if (sendto(s, &results, sizeof(results), 0, 
               (struct sockaddr *)&test_chan_master, sizeof(test_chan_master)) < 0) {
        perror("sendto results");
    }
    close(s);
}

//
// Read data from a stream, accounting for the fact that packet 'boundaries'
// are not preserved.  This can also timeout (which would probably wreck the
// data boundaries).
//

int
do_read(int fd, void *buf, int buflen)
{
    char *p = (char *)buf;
    int len = buflen;
    int res;
    while (len) {
        res = read(fd, p, len);
        if (res < 0) {
            perror("read");
        } else {
            len -= res;
            p += res;
            if (res == 0) {
                break;
            }
        }
    }
    return (buflen - len);
}

//
// Generic TCP test
//

static void
do_tcp_test(int s1, struct nc_request *req, struct sockaddr_in *master)
{
    int i, s, len, td_len, seq, seq_errors, lost, test_chan, res;
    struct sockaddr_in test_chan_slave, test_chan_master;
    struct nc_test_results results;
    struct nc_test_data *tdp;
    int nsent, nrecvd;
    int need_recv, need_send;
    int one = 1;
    static int slave_tcp_port = -1;

    need_recv = true;  need_send = true;
    switch (ntohl(req->type)) {
    case NC_REQUEST_TCP_SEND:
        need_recv = false;
        need_send = true;
        break;
    case NC_REQUEST_TCP_RECV:
        need_recv = true;
        need_send = false;
        break;
    case NC_REQUEST_TCP_ECHO:
        break;
    }

    if (slave_tcp_port < 0) {
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            pexit("datagram socket");
        }

        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
            perror("setsockopt SO_REUSEADDR");
            return;
        }
#ifdef SO_REUSEPORT
        if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
            perror("setsockopt SO_REUSEPORT");
            return;
        }
#endif
        memset((char *) &test_chan_slave, 0, sizeof(test_chan_slave));
        test_chan_slave.sin_family = AF_INET;
#ifdef __ECOS
        test_chan_slave.sin_len = sizeof(test_chan_slave);
#endif
        test_chan_slave.sin_addr.s_addr = htonl(INADDR_ANY);
        test_chan_slave.sin_port = htons(ntohl(req->slave_port));
    
        if (bind(s, (struct sockaddr *) &test_chan_slave, sizeof(test_chan_slave)) < 0) {
            perror("bind");
            close(s);
        }
        listen(s, SOMAXCONN);
        slave_tcp_port = s;
    }

    s = slave_tcp_port;
    len = sizeof(test_chan_master);
    if ((test_chan = accept(s, (struct sockaddr *)&test_chan_master, &len)) < 0) {
        pexit("accept");
    }
    len = sizeof(test_chan_master);
    getpeername(test_chan, (struct sockaddr *)&test_chan_master, &len);
    test_printf("connection from %s.%d\n", inet_ntoa(test_chan_master.sin_addr), 
                ntohs(test_chan_master.sin_port));

    nsent = 0;  nrecvd = 0;  seq = 0;  seq_errors = 0;  lost = 0;
    for (i = 0;  i < ntohl(req->nbufs);  i++) {
        if (need_recv) {
            tdp = (struct nc_test_data *)in_buf;
            td_len = ntohl(req->buflen) + sizeof(struct nc_test_data);
            res = do_read(test_chan, tdp, td_len);
            if (res != td_len) {
                test_printf("recvfrom timeout, expecting seq #%d\n", seq);            
                if (++lost > MAX_ERRORS) {
                    test_printf("... giving up\n");
                    break;
                }
            } else {
                nrecvd++;
                if ((ntohl(tdp->key1) == NC_TEST_DATA_KEY1) &&
                    (ntohl(tdp->key2) == NC_TEST_DATA_KEY2)) {
                    if (ntohl(tdp->seq) != seq) {
                        test_printf("Packets out of sequence - recvd: %d, expected: %d\n",
                                    ntohl(tdp->seq), seq);
                        seq = ntohl(tdp->seq);
                        seq_errors++;
                    }
                } else {
                    test_printf("Bad data packet - key: %x/%x, seq: %d\n",
                                ntohl(tdp->key1), ntohl(tdp->key2),
                                ntohl(tdp->seq));
                }
            }
        }
        if (need_send) {
            tdp = (struct nc_test_data *)out_buf;
            tdp->key1 = htonl(NC_TEST_DATA_KEY1);
            tdp->key2 = htonl(NC_TEST_DATA_KEY2);
            tdp->seq = htonl(seq);
            td_len = ntohl(req->buflen) + sizeof(struct nc_test_data);
            tdp->len = htonl(td_len);
            if (write(test_chan, tdp, td_len) != td_len) {
                perror("write");
                if (errno == ENOBUFS) {
                    // Saturated the system
                    test_delay(25);
                } else {
                    // What else to do?
                    close(test_chan);
                    return;
                }
            } else {
                nsent++;
            }
        }
        seq++;
    }
    results.key1 = htonl(NC_TEST_RESULT_KEY1);
    results.key2 = htonl(NC_TEST_RESULT_KEY2);
    results.seq = req->seq;
    results.nsent = htonl(nsent);
    results.nrecvd = htonl(nrecvd);
    if (write(test_chan, &results, sizeof(results)) != sizeof(results)) {
        perror("write");
    }
    close(test_chan);
}

//
// Protocol driver for testing slave.
//
// This function is the main routine running here, handling requests sent from
// the master and providing various responses.
//
static void
nc_slave(test_param_t param)
{
    int s, masterlen;
    struct sockaddr_in my_addr, master;
    struct nc_request req;
    struct nc_reply reply;
    int done = false;

    test_printf("Start test for eth%d\n", param);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        pexit("datagram socket");
    }

    memset((char *) &my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
#ifdef __ECOS
    my_addr.sin_len = sizeof(my_addr);
#endif
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(NC_SLAVE_PORT);
    
    if (bind(s, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
        pexit("bind");
    }

    while (!done) {
        masterlen = sizeof(master);
        if (recvfrom(s, &req, sizeof(req), 0, (struct sockaddr *)&master, &masterlen) < 0) {
            pexit("recvfrom");
        }
#if 0
        test_printf("Request %d from %s:%d\n", ntohl(req.type), 
                    inet_ntoa(master.sin_addr), ntohs(master.sin_port));
#endif
        reply.response = htonl(NC_REPLY_ACK);
        reply.seq = req.seq;
        switch (ntohl(req.type)) {
        case NC_REQUEST_DISCONNECT:
            done = true;
            break;
        case NC_REQUEST_UDP_SEND:
            test_printf("UDP send - %d buffers, %d bytes\n", ntohl(req.nbufs), ntohl(req.buflen));
            break;
        case NC_REQUEST_UDP_RECV:
            test_printf("UDP recv - %d buffers, %d bytes\n", ntohl(req.nbufs), ntohl(req.buflen));
            break;
        case NC_REQUEST_UDP_ECHO:
            test_printf("UDP echo - %d buffers, %d bytes\n", ntohl(req.nbufs), ntohl(req.buflen));
            break;
        case NC_REQUEST_TCP_SEND:
            test_printf("TCP send - %d buffers, %d bytes\n", ntohl(req.nbufs), ntohl(req.buflen));
            break;
        case NC_REQUEST_TCP_RECV:
            test_printf("TCP recv - %d buffers, %d bytes\n", ntohl(req.nbufs), ntohl(req.buflen));
            break;
        case NC_REQUEST_TCP_ECHO:
            test_printf("TCP echo - %d buffers, %d bytes\n", ntohl(req.nbufs), ntohl(req.buflen));
            break;
#ifdef __ECOS
        case NC_REQUEST_SET_LOAD:
            start_load(ntohl(req.nbufs));
            break;
        case NC_REQUEST_START_IDLE:
            test_printf("Start IDLE thread\n");
            idle_thread_count = 0;
            idle_thread_start_time = cyg_current_time();
            cyg_semaphore_post(&idle_thread_sem);
            break;
        case NC_REQUEST_STOP_IDLE:
            cyg_semaphore_wait(&idle_thread_sem);
            idle_thread_stop_time = cyg_current_time();
            test_printf("Stop IDLE thread\n");
            reply.misc.idle_results.elapsed_time = htonl(idle_thread_stop_time - idle_thread_start_time);
            reply.misc.idle_results.count[0] = htonl(idle_thread_count >> 32);
            reply.misc.idle_results.count[1] = htonl((long)idle_thread_count);
            break;
#endif
        default:
            test_printf("Unrecognized request: %d\n", ntohl(req.type));
            reply.response = htonl(NC_REPLY_NAK);
            reply.reason = htonl(NC_REPLY_NAK_UNKNOWN_REQUEST);
            break;
        }
        if (sendto(s, &reply, sizeof(reply), 0, (struct sockaddr *)&master, masterlen) < 0) {
            pexit("sendto");
        }
        if (reply.response == ntohl(NC_REPLY_NAK)) {
            continue;
        }
        switch (ntohl(req.type)) {
        case NC_REQUEST_UDP_SEND:
        case NC_REQUEST_UDP_RECV:
        case NC_REQUEST_UDP_ECHO:
            do_udp_test(s, &req, &master);
            break;
        case NC_REQUEST_TCP_SEND:
        case NC_REQUEST_TCP_RECV:
        case NC_REQUEST_TCP_ECHO:
            do_tcp_test(s, &req, &master);
            break;
        case NC_REQUEST_START_IDLE:
        case NC_REQUEST_STOP_IDLE:
        case NC_REQUEST_SET_LOAD:
        default:
            break;
        }
    }
    close(s);
}

void
net_test(test_param_t param)
{
#ifdef __ECOS
    cyg_serial_baud_rate_t old;    
    cyg_ppp_options_t options;
    cyg_ppp_handle_t ppp_handle;

    CYG_TEST_INIT();
#endif

//    int i;
    if (param == 0) {
        test_printf("Start Network Characterization - SLAVE\n");
#ifdef __ECOS
        init_all_network_interfaces();
        calibrate_load(DESIRED_BACKGROUND_LOAD);
#if 0
// I can see what this is trying to do, but I get "bind: Address already in
// use" errors from the 2nd interface - and the parameter is not used
// anyway, so one thread does quite well enough (but only tests one i/f at
// once).

// Comment in the 'int i' above too.
        for (i = 1;  i < CYGHWR_NET_DRIVERS;  i++) {
            cyg_thread_resume(main_thread_handle[i]);   // Start other threads
        }
#endif
#endif
    }

#ifdef __ECOS

    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_115200 );

    ppp_test_announce( "NC_TEST_SLAVE" );
    
    cyg_ppp_options_init( &options );

//    options.debug = 1;
//    options.kdebugflag = 1;

//    options.script = script;
//    options.flowctl = CYG_PPP_FLOWCTL_SOFTWARE;

    ppp_handle = cyg_ppp_up( CYGPKG_PPP_TEST_DEVICE, &options );

    CYG_TEST_INFO( "Waiting for PPP to come up");
    
    cyg_ppp_wait_up( ppp_handle );
#endif
    
    nc_slave(param);
#ifdef CYGDBG_NET_TIMING_STATS
    show_net_times();
#endif

#ifdef __ECOS
    CYG_TEST_INFO( "Bringing PPP down");

    cyg_ppp_down( ppp_handle );
    
    CYG_TEST_INFO( "Waiting for PPP to go down");

    cyg_ppp_wait_down( ppp_handle );

    cyg_thread_delay( 200 );

    ppp_test_set_baud( old );

    ppp_test_finish();

    CYG_TEST_PASS_FINISH( "Network Characterization - SLAVE" );
    
#endif
    
    cyg_test_exit();
}

#ifdef __ECOS

//
// This function is called to calibrate the "background load" which can be
// applied during testing.  It will be called before any commands from the
// host are managed.
//
static void
calibrate_load(int desired_load)
{
    long long no_load_idle, load_idle;
    int percent_load;
    int high, low;

    // Set limits
    high = MAX_LOAD_THREAD_LEVEL;
    low = MIN_LOAD_THREAD_LEVEL;

    // Compute the "no load" idle value
    idle_thread_count = 0;
    cyg_semaphore_post(&idle_thread_sem);  // Start idle thread
    cyg_thread_delay(1*100);               // Pause for one second
    cyg_semaphore_wait(&idle_thread_sem);  // Stop idle thread
    no_load_idle = idle_thread_count;
    diag_printf("No load = %d\n", (int)idle_thread_count);

    // First ensure that the HIGH level is indeed higher
    while (true) {
        load_thread_level = high;
        start_load(desired_load);              // Start up a given load
        idle_thread_count = 0;
        cyg_semaphore_post(&idle_thread_sem);  // Start idle thread
        cyg_thread_delay(1*100);               // Pause for one second
        cyg_semaphore_wait(&idle_thread_sem);  // Stop idle thread
        load_idle = idle_thread_count;
        start_load(0);                         // Shut down background load
        percent_load = 100 - ((load_idle * 100) / no_load_idle);
        diag_printf("High Load[%d] = %d => %d%%\n", load_thread_level, 
                    (int)idle_thread_count, percent_load);
        if ( percent_load > desired_load )
            break; // HIGH level is indeed higher
        low = load_thread_level; // known to be lower
        high *= 2; // else double it and try again
    }

    // Now chop down to the level required
    while (true) {
        load_thread_level = (high + low) / 2;
        start_load(desired_load);              // Start up a given load
        idle_thread_count = 0;
        cyg_semaphore_post(&idle_thread_sem);  // Start idle thread
        cyg_thread_delay(1*100);               // Pause for one second
        cyg_semaphore_wait(&idle_thread_sem);  // Stop idle thread
        load_idle = idle_thread_count;
        start_load(0);                         // Shut down background load
        percent_load = 100 - ((load_idle * 100) / no_load_idle);
        diag_printf("Load[%d] = %d => %d%%\n", load_thread_level, 
                    (int)idle_thread_count, percent_load);
        if (((high-low) <= 1) || (abs(desired_load-percent_load) <= 2)) break;
        if (percent_load < desired_load) {
            low = load_thread_level;
        } else {            
            high = load_thread_level;
        }
    }

    // Now we are within a few percent of the target; scale the load
    // factor to get a better fit, and test it, print the answer.
    load_thread_level *= desired_load;
    load_thread_level /= percent_load;
    start_load(desired_load);              // Start up a given load
    idle_thread_count = 0;
    cyg_semaphore_post(&idle_thread_sem);  // Start idle thread
    cyg_thread_delay(1*100);               // Pause for one second
    cyg_semaphore_wait(&idle_thread_sem);  // Stop idle thread
    load_idle = idle_thread_count;
    start_load(0);                         // Shut down background load
    percent_load = 100 - ((load_idle * 100) / no_load_idle);
    diag_printf("Final load[%d] = %d => %d%%\n", load_thread_level, 
                (int)idle_thread_count, percent_load);
//    no_load_idle_count_1_second = no_load_idle;
}

//
// This function is called to set up a load level of 'load' percent (given
// as a whole number, e.g. start_load(20) would mean initiate a background
// load of 20%, leaving the cpu 80% idle).
//
static void
start_load(int load)
{
    static int prev_load = 0;
    int i;
    test_printf("Set background load = %d%%\n", load);
    if (load == 0) {
        if (prev_load == 0) return;  // Nothing out there to stop
        for (i = 0;  i < prev_load/10;  i++) {
            cyg_semaphore_wait(&load_thread_sem[i]);
        }
        prev_load = 0;
    } else {
        for (i = 0;  i < load/10;  i++) {
            cyg_semaphore_post(&load_thread_sem[i]);
        }
        prev_load = load;
    }
}

//
// These thread(s) do some amount of "background" computing.  This is used
// to simulate a given load level.  They need to be run at a higher priority 
// than the network code itself.
//
// Like the "idle" thread, they run as long as their "switch" (aka semaphore)
// is enabled.
//
void
net_load(cyg_addrword_t who)
{
    int i;
    while (true) {
        cyg_semaphore_wait(&load_thread_sem[who]);
        for (i = 0;  i < load_thread_level;  i++) {
            do_some_random_computation(i);
        }
        cyg_thread_delay(1);  // Wait until the next 'tick'
        cyg_semaphore_post(&load_thread_sem[who]);
    }
}

//
// Some arbitrary computation, designed to use up the CPU and cause associated
// cache "thrash" behaviour - part of background load modelling.
//
static void
do_some_random_computation(int p)
{
    // Just something that might be "hard"
    volatile double x;
    x = ((p * 10) * 3.14159) / 180.0;  // radians
}

//
// This thread does nothing but count.  It will be allowed to count
// as long as the semaphore is "free".  
//
void
net_idle(cyg_addrword_t param)
{
    while (true) {
        cyg_semaphore_wait(&idle_thread_sem);
        idle_thread_count++;
        cyg_semaphore_post(&idle_thread_sem);
    }
}

void
cyg_start(void)
{
    int i;
    // Create processing threads
    for (i = 0;  i < CYGHWR_NET_DRIVERS;  i++) {
        cyg_thread_create(MAIN_THREAD_PRIORITY,     // Priority
                          net_test,                 // entry
                          i,                        // entry parameter
                          "Network test",           // Name
                          &main_thread_stack[i][0], // Stack
                          STACK_SIZE,               // Size
                          &main_thread_handle[i],   // Handle
                          &main_thread_data[i]      // Thread data structure
            );
    }
    cyg_thread_resume(main_thread_handle[0]);   // Start first one
    // Create the idle thread environment
    cyg_semaphore_init(&idle_thread_sem, 0);
    cyg_thread_create(IDLE_THREAD_PRIORITY,     // Priority
                      net_idle,                 // entry
                      0,                        // entry parameter
                      "Network idle",           // Name
                      &idle_thread_stack[0],    // Stack
                      STACK_SIZE,               // Size
                      &idle_thread_handle,      // Handle
                      &idle_thread_data         // Thread data structure
            );
    cyg_thread_resume(idle_thread_handle);      // Start it
    // Create the load threads and their environment(s)
    for (i = 0;  i < NUM_LOAD_THREADS;  i++) {
        cyg_semaphore_init(&load_thread_sem[i], 0);
        cyg_thread_create(LOAD_THREAD_PRIORITY,     // Priority
                          net_load,                 // entry
                          i,                        // entry parameter
                          "Background load",        // Name
                          &load_thread_stack[i][0], // Stack
                          STACK_SIZE,               // Size
                          &load_thread_handle[i],   // Handle
                          &load_thread_data[i]      // Thread data structure
            );
        cyg_thread_resume(load_thread_handle[i]);   // Start it
    }
    cyg_scheduler_start();
}

#else

int 
main(int argc, char *argv[])
{
    net_test(0);
}
#endif
