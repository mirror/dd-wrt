//==========================================================================
//
//      tests/udp_lo_test.c
// 
//      Simple UDP throughput test
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
// Author(s):    sorin@netappi.com 
// Contributors: gthomas,sorin@netappi.com, hmt
// Date:         2000-05-24


// Network throughput test code

#include <network.h>

#include <cyg/infra/testcase.h>

#define SOURCE_PORT 9990
#define SINK_PORT   9991

#define NUM_BUF 1024
#define MAX_BUF 8192
static unsigned char data_buf[MAX_BUF];
static unsigned char data_buf_write[MAX_BUF]="Client UDP is alive. You may continue ....";

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x10000)
static char stack_server[STACK_SIZE];
static cyg_thread server_thread_data;
static cyg_handle_t server_thread_handle;

static char stack_client[STACK_SIZE];
static cyg_thread client_thread_data; 
static cyg_handle_t client_thread_handle;  


#define MAIN_THREAD_PRIORITY     CYGPKG_NET_THREAD_PRIORITY-4

void
pexit(char *s)
{
    CYG_TEST_FAIL_FINISH( s );
}


void server(void)
{
    int s_source;
    struct sockaddr_in local,c_addr;
    int c_len;
    int len;
    
    char *hello_string=" Hello eCos network \n";
    diag_printf("UDP SERVER:");
    diag_printf(hello_string);

    s_source = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }   
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SOURCE_PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);  //accepts everyone...
    if(bind(s_source, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /source/ error");
    }
    c_len = sizeof(c_addr);

    if ((len = recvfrom(s_source, data_buf, sizeof(data_buf),0,
                        (struct sockaddr *)&c_addr,&c_len)) < 0  ) {
        CYG_TEST_FAIL_FINISH("I/O error");
    }
    diag_printf("SERVER : message arrived from %s\n",inet_ntoa(c_addr.sin_addr));
    diag_printf("SERVER : Message : %s\n",data_buf);
    close(s_source); 
}

void client(void)
{
    int s_source;
    struct sockaddr_in local;
    int len;

    diag_printf("client:started\n");
    
    s_source = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(SOURCE_PORT);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if ( (len= sendto(s_source,data_buf_write,sizeof(data_buf_write),
                      0,(struct sockaddr *)&local,sizeof(local))) < 0 ) {
        CYG_TEST_FAIL_FINISH("Error writing buffer");
    }
    close(s_source); 
}

void
udp_server(cyg_addrword_t param)
{
    init_all_network_interfaces();
    diag_printf("Start UDP server - test\n");
    cyg_thread_resume(client_thread_handle);    // Start the other one
#if NLOOP > 0
    server();
    CYG_TEST_PASS_FINISH("Server returned OK");
#endif
    CYG_TEST_NA( "No loopback devs" );
}

void
udp_client(cyg_addrword_t param)
{
    diag_printf("Start UDP client - test\n");
#if NLOOP > 0
    client(); 
#endif
}



void
cyg_start(void)
{
    CYG_TEST_INIT();

    cyg_thread_create(MAIN_THREAD_PRIORITY,     // Priority
                      udp_server,               // entry
                      0,                        // entry parameter
                      "UDP loopback server",    // Name
                      &stack_server[0],         // Stack
                      STACK_SIZE,               // Size
                      &server_thread_handle,    // Handle
                      &server_thread_data       // Thread data structure
            );
    cyg_thread_resume(server_thread_handle);    // Start it

    cyg_thread_create(MAIN_THREAD_PRIORITY,     // Priority
                      udp_client,               // entry
                      0,                        // entry parameter
                      "UDP loopback client",    // Name
                      &stack_client[0],         // Stack
                      STACK_SIZE,               // Size
                      &client_thread_handle,    // Handle
                      &client_thread_data       // Thread data structure
            );
    cyg_scheduler_start();
}

