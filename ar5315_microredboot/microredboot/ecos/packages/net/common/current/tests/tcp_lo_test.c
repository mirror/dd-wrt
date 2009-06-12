//==========================================================================
//
//      tests/tcp_lo_test.c
// 
//      Simple TCP throughput test 
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
static unsigned char data_buf_write[MAX_BUF]="Client is alive. You may continue ....";

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
    int s_source, e_source;
    struct sockaddr_in e_source_addr, local;
    int one = 1;
    fd_set in_fds;
    int len;
    
    char *hello_string=" Hello eCos network \n";
    diag_printf("TCP SERVER:");
    diag_printf(hello_string);

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
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(s_source, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /source/ error");
    }
    listen(s_source, SOMAXCONN);
   
    e_source = 0; 
    while (true) {
	FD_ZERO(&in_fds);
	FD_SET(s_source, &in_fds);
        len = sizeof(e_source_addr);
        if ((e_source = accept(s_source,(struct sockaddr *)&e_source_addr,&len))<0) {
            pexit("accept /source/");
        }
	diag_printf("TCP SERVER connection from %s: %d\n",
	       inet_ntoa(e_source_addr.sin_addr),ntohs(e_source_addr.sin_port));

        if (e_source != 0) {
            break;
        }
    }   /* while (true) */ 
    
    if ((len = read(e_source, data_buf, MAX_BUF)) < 0  ) {
        CYG_TEST_FAIL_FINISH( "I/O error" );
    }
    diag_printf("SERVER : %s\n",data_buf);
    
}

void client(void)
{
    int s_source;
    struct sockaddr_in local;
    int len;

    diag_printf("client:started\n");
    
    s_source = socket(AF_INET, SOCK_STREAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(SOURCE_PORT);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    if (connect(s_source, (struct sockaddr *)&local, sizeof(local)) < 0) {
        pexit("Can't connect to target");
    }
    
    if ((len = write(s_source,data_buf_write,40)) < 0){
        CYG_TEST_FAIL_FINISH( "Error writing buffer");
    } 
}

void
tcp_server(cyg_addrword_t param)
{
    init_all_network_interfaces();
    diag_printf("Start TCP server - test\n");
    cyg_thread_resume(client_thread_handle);    // Start it
#if NLOOP > 0
    server();
    CYG_TEST_PASS_FINISH( "server returned OK" );
#endif
    CYG_TEST_NA( "No loopback devs" );
}

void
tcp_client(cyg_addrword_t param)
{
    diag_printf("Start TCP client - test\n");
#if NLOOP > 0
    client(); 
#endif
}



void
cyg_start(void)
{
    CYG_TEST_INIT();

    cyg_thread_create(MAIN_THREAD_PRIORITY,     // Priority
                      tcp_server,               // entry
                      0,                        // entry parameter
                      "TCP loopback server",    // Name
                      &stack_server[0],         // Stack
                      STACK_SIZE,               // Size
                      &server_thread_handle,    // Handle
                      &server_thread_data       // Thread data structure
            );
    cyg_thread_resume(server_thread_handle);    // Start it

    cyg_thread_create(MAIN_THREAD_PRIORITY,     // Priority
                      tcp_client,               // entry
                      0,                        // entry parameter
                      "TCP loopback client",    // Name
                      &stack_client[0],         // Stack
                      STACK_SIZE,               // Size
                      &client_thread_handle,    // Handle
                      &client_thread_data       // Thread data structure
            );
    cyg_scheduler_start();
}



