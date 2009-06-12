//==========================================================================
//
//      tests/multi_lo_test.c
// 
//      Multiple selects-at-one-time test, using lo for portability.
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    sorin@netappi.com, hmt
// Contributors: gthomas,sorin@netappi.com
// Date:         2000-05-24
// Description:
//
//      This test is to test that the internal producer operation to select
//      truly has broadcast semantics; there was a bug in there whereby it
//      doesn't, so events get lost and/or the wrong thread is awakened.
//
//      We need to create N threads selecting on different sockets
//      (different ports) (including one or two selecting on several
//      sockets) and have a further thread or threads which send data to
//      the those sockets in an order "random" with respect to the order in
//      which the N selectors entered the wait, and their thread
//      priorities.
//
//      If this all works, then we know that select always wakes the right
//      thread in the right order.  I think...
//
//      I think 10 threads 0-9 where #2,#3,#6,#7 wait for multiple threads
//      will do it.  #0-4 will be prio HI, #5-9 will be prio LO.  Sender
//      thread A at prio MID will send to sockets 1,3,5,7,9.  Sender thread
//      B at prio LOWEST will send to sockets 0,2,4,6,8.
//
//      Each sender thread will wait for a different semaphore signal
//      before doing their next send, thus confirming correct ordering.
//      Two common semaphores will also be signalled, one when a send
//      occurs, the other when a recv happens.
//
//      The master thread will start off VERYHIGHPRI, then drop after
//      starting all the others, to VERYLOW... when it next runs, those
//      common semaphores should both have value 10 == NLISTENERS.
//
//
//#####DESCRIPTIONEND#####
//
//==========================================================================

#include <network.h>

#include <cyg/infra/testcase.h>

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define SOURCE_PORT1 9900
#define SOURCE_PORT2 9800 // for those who listen to multiple ports
#define SOURCE_PORT3 9700 // for the dummy consumers of events

#define PRIO_DUMMY 4 // Really high, so they're always right back there

#define PRIO_LISTENER_HI 10
#define PRIO_LISTENER_LO 15

#define PRIO_SENDER_MID  12
#define PRIO_SENDER_LOW  17

#define PRIO_MASTERHIGH   3
#define PRIO_MASTERLOW   25

#ifndef CYGPKG_IO_FILEIO
#if CYGPKG_IO_NFILE > 30
#define NLISTENERS 10
#else
// fewer threads if not many sockets available
#define NLISTENERS (CYGPKG_IO_NFILE/3)
#endif
#else
#include <pkgconf/io_fileio.h>
#if CYGNUM_FILEIO_NFD > 30
#define NLISTENERS 10
#else
// fewer threads if not many sockets available
#define NLISTENERS (CYGNUM_FILEIO_NFD/3)
#endif
#endif

#define NDUMMIES   10

#define NSENDERS 2

#define NUM_BUF  NLISTENERS
#define MAX_BUF 100

// buffers for receiving into:
static unsigned char data_buf1[NUM_BUF][MAX_BUF];

static unsigned char data_buf_write1[]="Client is alive";

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL)
#define MASTER_STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)

static char stack_master[MASTER_STACK_SIZE];
static cyg_thread master_thread_data;
static cyg_handle_t master_thread_handle;

static char stack_dummy[NDUMMIES][STACK_SIZE];
static cyg_thread dummy_thread_data[NDUMMIES]; 
static cyg_handle_t dummy_thread_handle[NDUMMIES];  

static char stack_listener[NLISTENERS][STACK_SIZE];
static cyg_thread listener_thread_data[NLISTENERS]; 
static cyg_handle_t listener_thread_handle[NLISTENERS];  

static char stack_sender[NSENDERS][STACK_SIZE];
static cyg_thread sender_thread_data[NSENDERS];
static cyg_handle_t sender_thread_handle[NSENDERS];

static cyg_sem_t listen_sema[NLISTENERS];

static cyg_sem_t send_sema;
static cyg_sem_t recv_sema;

static cyg_thread_entry_t master;
static cyg_thread_entry_t listener;
static cyg_thread_entry_t sender;

// ------------------------------------------------------------------------

void
pexit(char *s)
{
    CYG_TEST_FAIL_FINISH( s );
}


#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

// ------------------------------------------------------------------------

void dummy( cyg_addrword_t which )
{
    // Share the same socket... we appear to run out otherwise.
    static int s_s1 = -1;
    static struct sockaddr_in local;

    // locals...
    fd_set in_fds;
    int num;

    CYG_TEST_CHECK( 0 <= which, "which under" );
    CYG_TEST_CHECK( NDUMMIES > which, "which over" );

    diag_printf( "Dummy %d alive\n", which );

    if ( s_s1 < 0 ) {
        s_s1 = socket(AF_INET, SOCK_STREAM, 0);
        if (s_s1 < 0) {
            pexit("stream socket 1");
        }
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_len = sizeof(local);
        local.sin_port = ntohs(SOURCE_PORT3 + which);
        local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if(bind(s_s1, (struct sockaddr *) &local, sizeof(local)) < 0) {
            pexit("dummy bind /source_1/ error");
        }
        listen(s_s1, SOMAXCONN);
    }

    while (true) {
        FD_ZERO(&in_fds);
        FD_SET(s_s1, &in_fds);
        num = select( s_s1+1, &in_fds,0,0,0);
        
        if (FD_ISSET(s_s1,&in_fds)) {
            CYG_TEST_FAIL( "Activity on dummy port!" );
        }
    }   /* while (true) */ 
}

// ------------------------------------------------------------------------

void listener( cyg_addrword_t which )
{
    int s_s1 = -1, e_s1 = 0, s_s2 = -1, e_s2 = 0;
    struct sockaddr_in e_s1_addr,e_s2_addr,local;
    fd_set in_fds;
    int len;
    int num;

    // do we select on multiple sources?
    int dual = (3 == (which & 3)) || (2 == (which & 3));
    // then which is 2,3,6,7 so set up a 2nd listener
    
    CYG_TEST_CHECK( 0 <= which, "which under" );
    CYG_TEST_CHECK( NLISTENERS > which, "which over" );

    diag_printf( "Listener %d alive [%s]\n", which, dual ? "dual" : "single" );

    s_s1 = socket(AF_INET, SOCK_STREAM, 0);
    if (s_s1 < 0) {
        pexit("stream socket 1");
    }   
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SOURCE_PORT1 + which);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(s_s1, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /source_1/ error");
    }
    listen(s_s1, SOMAXCONN);

    if ( dual ) {
        s_s2 = socket(AF_INET, SOCK_STREAM, 0);    
        if (s_s2 < 0) {    
            pexit("stream socket 2");
        }  
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_len = sizeof(local);
        local.sin_port = ntohs(SOURCE_PORT2 + which);
        local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if(bind(s_s2, (struct sockaddr *) &local, sizeof(local)) < 0) {    
            pexit("bind /source_2/ error");
        }
        listen(s_s2, SOMAXCONN);
    }

    while (true) {
        FD_ZERO(&in_fds);
        FD_SET(s_s1, &in_fds);
        if ( dual )
            FD_SET(s_s2, &in_fds);
        num = select ( max(s_s1,s_s2)+1, &in_fds,0,0,0);
        
        if (FD_ISSET(s_s1,&in_fds)) {
            len = sizeof(e_s1_addr);
            if ((e_s1 = accept(s_s1,(struct sockaddr *)&e_s1_addr,&len))<0) {
                pexit("accept /source_1/");
            }
            diag_printf("TCP SERVER connection from %s: %d\n",
                        inet_ntoa(e_s1_addr.sin_addr),ntohs(e_s1_addr.sin_port));
        }

        if ( dual ) {
            if (FD_ISSET(s_s2,&in_fds)) {
                len = sizeof(e_s2_addr);
                if ((e_s2 = accept(s_s2,(struct sockaddr *)&e_s2_addr,&len))<0) {
                    pexit("accept /source_2/");
                }
                diag_printf("TCP SERVER connection from %s: %d\n",
                            inet_ntoa(e_s2_addr.sin_addr), ntohs(e_s2_addr.sin_port));
            }
        }
        if ((e_s1 != 0) || ( e_s2 != 0)) {
            break;
        }
    }   /* while (true) */ 

    CYG_TEST_CHECK( 0 != e_s1, "No connection made on s1!" );
    
    if ((len = read(e_s1, data_buf1[which], MAX_BUF)) < 0  ) {
        perror("I/O error s1");
        CYG_TEST_FAIL( "Read s1 failed" );
    }
    diag_printf("Listener %d: %s\n", which, data_buf1[which]);

    close( s_s1 );
    if ( dual )
        close( s_s2 );
    if ( 0 != e_s1 )
        close ( e_s1 );
    if ( 0 != e_s2 )
        close ( e_s2 );

    cyg_semaphore_post( &listen_sema[which] ); // Verify that I was here
    cyg_semaphore_post( &recv_sema );          // Count receptions

    cyg_thread_exit(); // explicitly
}

// ------------------------------------------------------------------------
static void sender( cyg_addrword_t which ) // which means which set (odd/even) here...
{
    int s_source;
    struct sockaddr_in local;
    int len;

    diag_printf("client %d [%s] :started\n", which, (which & 1) ? "odd" : "even" );

    for ( /* which as is */; which < NLISTENERS; which += 2 ) {

        s_source = socket(AF_INET, SOCK_STREAM, 0);
        if (s_source < 0) {
            pexit("stream socket");
        }
        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_len = sizeof(local);
        local.sin_port = htons( SOURCE_PORT1 + which );
        local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
        if (connect(s_source, (struct sockaddr *)&local, sizeof(local)) < 0) {
            pexit("Can't connect to target");
        }
    
        if ((len = write(s_source,data_buf_write1,sizeof(data_buf_write1) )) < 0) {
            CYG_TEST_FAIL_FINISH("Error writing buffer");
        } 
        cyg_semaphore_wait( &listen_sema[which] ); // wait for the appropriate semaphore "reply"
        cyg_semaphore_post( &send_sema ); // count up successful sends

        close ( s_source );
    }
    cyg_thread_exit(); // explicitly
}


static void
master(cyg_addrword_t param)
{
    int i;
    cyg_handle_t self = cyg_thread_self();

    cyg_semaphore_init( &send_sema, 0 );
    cyg_semaphore_init( &recv_sema, 0 );

    for ( i = 0 ; i < NLISTENERS; i++ )
        cyg_semaphore_init( &listen_sema[i], 0 );

    init_all_network_interfaces();
    CYG_TEST_INFO("Start multiple loopback select test");
#if NLOOP > 0
    // We are currently running at high prio, so we can just go and make
    // loads of threads:

    // Some at higher prio
    for ( i = 0; i < NLISTENERS/2; i++ )
        cyg_thread_create(PRIO_LISTENER_HI,       // Priority
                          listener,               // entry
                          i,                      // entry parameter
                          "listener",             // Name
                          &stack_listener[i][0],  // Stack
                          STACK_SIZE,             // Size
                          &listener_thread_handle[i], // Handle
                          &listener_thread_data[i] // Thread data structure
            );
    // the rest at lower prio
    for (      ; i < NLISTENERS  ; i++ )
        cyg_thread_create(PRIO_LISTENER_LO,       // Priority
                          listener,               // entry
                          i,                      // entry parameter
                          "listener",             // Name
                          &stack_listener[i][0],  // Stack
                          STACK_SIZE,             // Size
                          &listener_thread_handle[i], // Handle
                          &listener_thread_data[i] // Thread data structure
            );

    // make the dummy event-grabber threads
    for ( i = 0; i < NDUMMIES; i++ )
        cyg_thread_create(PRIO_DUMMY,             // Priority
                          dummy,                  // entry
                          i,                      // entry parameter
                          "dummy",                // Name
                          &stack_dummy[i][0],     // Stack
                          STACK_SIZE,             // Size
                          &dummy_thread_handle[i], // Handle
                          &dummy_thread_data[i]   // Thread data structure
            );

    // Start those threads
    for ( i = 0; i < NLISTENERS; i++ )
        cyg_thread_resume(listener_thread_handle[i]);
    for ( i = 0; i < NDUMMIES; i++ )
        cyg_thread_resume(   dummy_thread_handle[i]);

    // and let them start up and start listening...
    cyg_thread_set_priority( self, PRIO_MASTERLOW );
    CYG_TEST_INFO("All listeners should be go now");
    cyg_thread_set_priority( self, PRIO_MASTERHIGH );

    for ( i = 0; i < NSENDERS; i++ ) {
        cyg_thread_create( (0 == i)
                           ?PRIO_SENDER_MID
                           : PRIO_SENDER_LOW,     // Priority
                           sender,                // entry
                           i,                     // entry parameter
                           "sender",              // Name
                           &stack_sender[i][0],   // Stack
                           STACK_SIZE,            // Size
                           &sender_thread_handle[i], // Handle
                           &sender_thread_data[i] // Thread data structure
            );
        cyg_thread_resume(sender_thread_handle[i]);
    }

    // Now we are still higher priority; so go low and let everyone else
    // have their head.  When we next run after this, it should all be
    // over.
    cyg_thread_set_priority( self, PRIO_MASTERLOW );

    cyg_semaphore_peek( &recv_sema, &i );
    CYG_TEST_CHECK( NLISTENERS == i, "Not enough recvs occurred!" );
    
    cyg_semaphore_peek( &send_sema, &i );
    CYG_TEST_CHECK( NLISTENERS == i, "Not enough sends occurred!" );

    CYG_TEST_PASS_FINISH("Master returned OK");
#endif
    CYG_TEST_NA( "No loopback devs" );
}

void
cyg_user_start(void)
{
    CYG_TEST_INIT();

    cyg_thread_create(PRIO_MASTERHIGH,            // Priority
                      master,                     // entry
                      0,                          // entry parameter
                      "master",                   // Name
                      &stack_master[0],           // Stack
                      MASTER_STACK_SIZE,          // Size
                      &master_thread_handle,      // Handle
                      &master_thread_data         // Thread data structure
            );
    cyg_thread_resume(master_thread_handle);      // Start it
}

// EOF multi_lo_select.c
