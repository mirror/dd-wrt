//==========================================================================
//
//      tests/tcp_echo.c
//
//      Simple TCP throughput test - echo component
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
// Author(s):    gthomas, nickg
// Contributors: gthomas, nickg
// Date:         2000-01-10
// Purpose:      
// Description:  This is the middle part of a three part test.  The idea is
//   to test the throughput of box in a configuration like this:
//
//      +------+   port   +----+     port    +----+
//      |SOURCE|=========>|ECHO|============>|SINK|
//      +------+   9990   +----+     9991    +----+
// 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/net.h>

#include <cyg/io/io.h>
#include <cyg/io/serialio.h>

#include <cyg/ppp/ppp.h>

#include <cyg/infra/testcase.h>

#include "ppp_test_support.inl"


// Network throughput test code

#include <network.h>

static __inline__ unsigned int
max(unsigned int m, unsigned int n)
{
    return m > n ? m : n;
}

#define SOURCE_PORT 9990
#define SINK_PORT   9991

#define MAX_BUF 8192
static unsigned char data_buf[MAX_BUF];

struct test_params {
    long nbufs;
    long bufsize;
    long load;
};

struct test_status {
    long ok;
};

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

// Background load stuff
#define NUM_LOAD_THREADS         20 // Get 5% granularity
#define IDLE_THREAD_PRIORITY     CYGNUM_PPP_PPPD_THREAD_PRIORITY+3
#define LOAD_THREAD_PRIORITY     CYGPKG_NET_THREAD_PRIORITY-3
#define MAIN_THREAD_PRIORITY     CYGPKG_NET_THREAD_PRIORITY-4
#define DESIRED_BACKGROUND_LOAD  50 // should be accurate enough over range

// starting points for load calculation
#define MAX_LOAD_THREAD_LEVEL    100
#define MIN_LOAD_THREAD_LEVEL    0

static char         idle_thread_stack[STACK_SIZE];
static cyg_thread   idle_thread_data;
static cyg_handle_t idle_thread_handle;
static cyg_sem_t    idle_thread_sem;
volatile static long long    idle_thread_count;
static char         load_thread_stack[NUM_LOAD_THREADS][STACK_SIZE];
static cyg_thread   load_thread_data[NUM_LOAD_THREADS];
static cyg_handle_t load_thread_handle[NUM_LOAD_THREADS];
static cyg_sem_t    load_thread_sem[NUM_LOAD_THREADS];
static long         load_thread_level;
static void calibrate_load(int load);
static void start_load(int load);
static void do_some_random_computation(int p,int id);
#define abs(n) ((n) < 0 ? -(n) : (n))

static long long no_load_idle_count_1_second;

extern void
cyg_test_exit(void);

void
pexit(char *s)
{
    perror(s);
    cyg_test_exit();
}

int
do_read(int s, void *_buf, int len)
{
    int total, slen, rlen;
    unsigned char *buf = (unsigned char *)_buf;
    total = 0;
    rlen = len;
    while (total < len) {
        slen = read(s, buf, rlen);
        if (slen != rlen) {
            if (slen < 0) {
                diag_printf("Error after reading %d bytes\n", total);
                return -1;
            }
            rlen -= slen;
            buf += slen;
        }
        total += slen;
    }
    return total;
}

int
do_write(int s, void *_buf, int len)
{
    int total, slen, rlen;
    unsigned char *buf = (unsigned char *)_buf;
    total = 0;
    rlen = len;
    while (total < len) {
        slen = write(s, buf, rlen);
        if (slen != rlen) {
            if (slen < 0) {
                diag_printf("Error after writing %d bytes\n", total);
                return -1;
            }
            rlen -= slen;
            buf += slen;
        }
        total += slen;
    }
    return total;
}

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
    no_load_idle_count_1_second = no_load_idle;
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
    if (load == 0) {
        diag_printf("Set no background load\n");
        if (prev_load == 0) return;  // Nothing out there to stop
        for (i = 0;  i < prev_load * NUM_LOAD_THREADS/100;  i++) {
            cyg_semaphore_wait(&load_thread_sem[i]);
        }
        prev_load = 0;
    } else {
        diag_printf("Set background load = %d%% starting %d threads\n",
                    load, load * NUM_LOAD_THREADS/100 );
        for (i = 0;  i < load * NUM_LOAD_THREADS/100;  i++) {
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
            do_some_random_computation(i,who);
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
do_some_random_computation(int p,int id)
{
    // Just something that might be "hard"
#if 0
    {
        volatile double x;
        x = ((p * 10) * 3.14159) / 180.0;  // radians
    }
#endif
#if 1
    {
        static int footle[0x10001];
        static int counter = 0;
        register int i;

        i = (p << 8) + id + counter++;
        i &= 0xffff;
        footle[ i+1 ] += footle[ i ] + 1;
    }
#endif
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

static void
echo_test(cyg_addrword_t p)
{
    int s_source, s_sink, e_source, e_sink;
    struct sockaddr_in e_source_addr, e_sink_addr, local;
    int one = 1;
    fd_set in_fds;
    int i, num, len;
    struct test_params params,nparams;
    struct test_status status,nstatus;

    cyg_tick_count_t starttime, stoptime;

    s_source = socket(AF_INET, SOCK_STREAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }
    if (setsockopt(s_source, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt /source/ SO_REUSEADDR");
    }
    if (setsockopt(s_source, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        pexit("setsockopt /source/ SO_REUSEPORT");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SOURCE_PORT);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(s_source, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /source/ error");
    }
    listen(s_source, SOMAXCONN);

    s_sink = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sink < 0) {
        pexit("stream socket");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SINK_PORT);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(s_sink, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /sink/ error");
    }
    if (setsockopt(s_sink, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt /sink/ SO_REUSEADDR");
    }
    if (setsockopt(s_sink, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one))) {
        pexit("setsockopt /sink/ SO_REUSEPORT");
    }
    listen(s_sink, SOMAXCONN);

    e_source = 0;  e_sink = 0;
    while (true) {
        // Wait for a connection on either of the ports
        FD_ZERO(&in_fds);
        FD_SET(s_source, &in_fds);
        FD_SET(s_sink, &in_fds);
        num = select(max(s_sink,s_source)+1, &in_fds, 0, 0, 0);
        if (FD_ISSET(s_source, &in_fds)) {
            len = sizeof(e_source_addr);
            if ((e_source = accept(s_source, (struct sockaddr *)&e_source_addr, &len)) < 0) {
                pexit("accept /source/");
            }
            diag_printf("SOURCE connection from %s:%d\n", 
                        inet_ntoa(e_source_addr.sin_addr), ntohs(e_source_addr.sin_port));
        }
        if (FD_ISSET(s_sink, &in_fds)) {
            len = sizeof(e_sink_addr);
            if ((e_sink = accept(s_sink, (struct sockaddr *)&e_sink_addr, &len)) < 0) {
                pexit("accept /sink/");
            }
            diag_printf("SINK connection from %s:%d\n", 
                        inet_ntoa(e_sink_addr.sin_addr), ntohs(e_sink_addr.sin_port));
        }
        // Continue with test once a connection is established in both directions
        if ((e_source != 0) && (e_sink != 0)) {
            break;
        }
    }

    // Wait for "source" to tell us the testing paramters
    if (do_read(e_source, &nparams, sizeof(nparams)) != sizeof(nparams)) {
        pexit("Can't read initialization parameters");
    }
  
    params.nbufs = ntohl(nparams.nbufs);
    params.bufsize = ntohl(nparams.bufsize);
    params.load = ntohl(nparams.load);
  
    diag_printf("Using %d buffers of %d bytes each, %d%% background load\n", 
                params.nbufs, params.bufsize, params.load);

    // Tell the sink what the parameters are
    if (do_write(e_sink, &nparams, sizeof(nparams)) != sizeof(nparams)) {
        pexit("Can't write initialization parameters");
    }

    status.ok = 1;
    nstatus.ok = htonl(status.ok);
  
    // Tell the "source" to start - we're all connected and ready to go!
    if (do_write(e_source, &nstatus, sizeof(nstatus)) != sizeof(nstatus)) {
        pexit("Can't send ACK to 'source' host");
    }

    idle_thread_count = 0;
    cyg_semaphore_post(&idle_thread_sem);  // Start idle thread
    starttime = cyg_current_time();
    start_load(params.load);

    // Echo the data from the source to the sink hosts
    for (i = 0;  i < params.nbufs;  i++) {
        if ((len = do_read(e_source, data_buf, params.bufsize)) != params.bufsize) {
            diag_printf("Can't read buf #%d: ", i+1);
            if (len < 0) {
                perror("I/O error");
            } else {
                diag_printf("short read - only %d bytes\n", len);
            }
        }
//        else diag_printf("Got %d bytes\n",len);        
        if ((len = do_write(e_sink, data_buf, params.bufsize)) != params.bufsize) {
            diag_printf("Can't write buf #%d: ", i+1);
            if (len < 0) {
                perror("I/O error");
            } else {
                diag_printf("short write - only %d bytes\n", len);
            }
        }
//        else diag_printf("Sent %d bytes\n",len);        
    }

    // Wait for the data to drain and the "sink" to tell us all is OK.
    if (do_read(e_sink, &status, sizeof(status)) != sizeof(status)) {
        pexit("Can't receive ACK from 'sink' host");
    }

    start_load(0);
    cyg_semaphore_wait(&idle_thread_sem);  // Stop idle thread
    stoptime = cyg_current_time();
    stoptime -= starttime; // time taken in cS
    // expected idle loops in that time period for an idle system:
    starttime = no_load_idle_count_1_second * stoptime / 100;
    diag_printf( "%d ticks elapsed, %d kloops predicted for an idle system\n",
                 (int)stoptime, (int)(starttime/1000) );
    diag_printf( "actual kloops %d, CPU was %d%% idle during transfer\n",
                 (int)(idle_thread_count/1000),
                 (int)(idle_thread_count * 100 / starttime) );

    // Now examine how close that loading actually was:
    start_load(params.load);              // Start up a given load
    idle_thread_count = 0;
    cyg_semaphore_post(&idle_thread_sem);  // Start idle thread
    cyg_thread_delay(1*100);               // Pause for one second
    cyg_semaphore_wait(&idle_thread_sem);  // Stop idle thread
    start_load(0);                         // Shut down background load
    i = 100 - ((idle_thread_count * 100) / no_load_idle_count_1_second );
    diag_printf("Final load[%d] = %d => %d%%\n", load_thread_level, 
                (int)idle_thread_count, i);

//#ifdef CYGDBG_USE_ASSERTS
#ifdef CYGDBG_NET_TIMING_STATS 
    {
        extern void show_net_times(void);
        show_net_times();
    }
#endif
//#endif
}

//==========================================================================

void
net_test(cyg_addrword_t param)
{
    cyg_serial_baud_rate_t old;    
    cyg_ppp_options_t options;
    cyg_ppp_handle_t ppp_handle;

    CYG_TEST_INIT();
    
    diag_printf("Start TCP test - ECHO mode\n");
    init_all_network_interfaces();
    calibrate_load(DESIRED_BACKGROUND_LOAD);
#ifdef CYGPKG_SNMPAGENT
    {
        extern void cyg_net_snmp_init(void);
        cyg_net_snmp_init();
    }
#endif

    old = ppp_test_set_baud( CYGNUM_SERIAL_BAUD_115200 );

    ppp_test_announce( "TCP_ECHO" );
    
    cyg_ppp_options_init( &options );

//    options.debug = 1;
//    options.kdebugflag = 1;
//    options.flowctl = CYG_PPP_FLOWCTL_SOFTWARE;

    ppp_handle = cyg_ppp_up( CYGPKG_PPP_TEST_DEVICE, &options );

    CYG_TEST_INFO( "Waiting for PPP to come up");
    
    cyg_ppp_wait_up( ppp_handle );

    echo_test(param);

    CYG_TEST_INFO( "Bringing PPP down");

    cyg_ppp_down( ppp_handle );
    
    CYG_TEST_INFO( "Waiting for PPP to go down");

    cyg_ppp_wait_down( ppp_handle );

    cyg_thread_delay( 200 );
    
    ppp_test_set_baud( old );

    ppp_test_finish();
    
    CYG_TEST_PASS_FINISH("TCP ECHO test OK");
}

void
cyg_start(void)
{
    int i;
    // Create a main thread which actually runs the test
    cyg_thread_create(MAIN_THREAD_PRIORITY, // Priority
                      net_test,             // entry
                      0,                    // entry parameter
                      "Network test",       // Name
                      &stack[0],            // Stack
                      STACK_SIZE,           // Size
                      &thread_handle,       // Handle
                      &thread_data          // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
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

// EOF tcp_echo.c
