//==========================================================================
//
//      tests/nc_test_framework.h
//
//      Network characterization tests framework
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

#ifndef _TESTS_NC_TEST_FRAMEWORK_H_
#define _TESTS_NC_TEST_FRAMEWORK_H_


#define test_printf diag_printf
typedef cyg_addrword_t test_param_t;

#ifndef true
#define false 0
#define true  1
#endif

#define NC_SLAVE_PORT  7777
#define NC_MASTER_PORT 7776

#define NC_TESTING_SLAVE_PORT  8770
#define NC_TESTING_MASTER_PORT 8771

#define __string(s) #s
#define _string(s) __string(s)

//
// The basic idea behind this test structure is that one end will run
// in "slave" mode and the other in "master" mode.  Typically, the slave
// will run on a target platform with the master running on a host.
//
// The slave starts up by listening for a connection on the "SLAVE_PORT".
// In order for the testing to require the minimum stack support, this
// connection (and the protocol) will use UDP.
//
// The master will connect to the slave and send it a request over this
// connection.  Once the slave accepts the request, then master and slave
// will execute the operation, typically a test.  The control connection
// will remain active until the master sends a 'disconnect' request.  The
// control connection will be broken after the reply to this request has
// been sent.
//

#define MAX_ERRORS          5   // Give up after this many errors

#define NC_REPLY_TIMEOUT    10  // The slave may be slow
#define NC_TEST_TIMEOUT     3   // More generous for tests
#define NC_RESULTS_TIMEOUT  (MAX_ERRORS+2)*NC_TEST_TIMEOUT

struct nc_request {
    int type;          // Description of request
    int seq;           // Sequence number, used to build response
    int nbufs;         // Number of "buffers" to send
    int buflen;        // Length of each buffer
    int slave_port;    // Network ports to use
    int master_port;
    int timeout;       // Max time to wait for any packet
};

#define NC_REQUEST_DISCONNECT 0x0001
#define NC_REQUEST_UDP_SEND   0x0010  // Slave to send UDP data
#define NC_REQUEST_UDP_RECV   0x0011  // Slave to receive UDP data
#define NC_REQUEST_UDP_ECHO   0x0012  // Master->slave->master
#define NC_REQUEST_TCP_SEND   0x0020  // Slave to send TCP data
#define NC_REQUEST_TCP_RECV   0x0021  // Slave to receive TCP data
#define NC_REQUEST_TCP_ECHO   0x0022  // Master->slave->master
#define NC_REQUEST_START_IDLE 0x0100  // Start some idle processing
#define NC_REQUEST_STOP_IDLE  0x0101  // Stop idle processing
#define NC_REQUEST_SET_LOAD   0x0200  // Set the background load level

struct nc_reply {
    int  response; // ACK or NAK
    int  seq;      // Must match request
    int  reason;   // If NAK, why request turned down
    union {        // Miscellaneous data, depending on request
        struct {
            long      elapsed_time;  // In 10ms "ticks"
            long      count[2];      // Result 
        } idle_results;
    } misc;
};

#define NC_REPLY_ACK  0x0001    // Request accepted
#define NC_REPLY_NAK  0x0000    // Request denied

#define NC_REPLY_NAK_UNKNOWN_REQUEST 0x0001
#define NC_REPLY_NAK_BAD_REQUEST     0x0002  // Slave can't handle
#define NC_REPLY_NAK_NO_BACKGROUND   0x0003  // Slave can't do background/idle

//
// Test data 'packets' look like this

struct nc_test_data {
    long  key1;
    int   seq;
    int   len;
    long  key2;
    char  data[0];  // Actual data
};

#define NC_TEST_DATA_KEY1 0xC0DEADC0
#define NC_TEST_DATA_KEY2 0xC0DEADC1

struct nc_test_results {
    long key1;         // Identify uniquely as a response record
    int  seq;          // Matches request
    int  nsent;
    int  nrecvd;
    long key2;         // Additional verification
};

#define NC_TEST_RESULT_KEY1 0xDEADC0DE
#define NC_TEST_RESULT_KEY2 0xDEADC1DE

#endif // _TESTS_NC_TEST_FRAMEWORK_H_



