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
// Contributors: gthomas,sorin@netappi.com 
// Date:         2000-05-24


// Network throughput test code

#include <network.h>

#include <cyg/infra/testcase.h>

#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif

#define SOURCE_PORT1 9990
#define SOURCE_PORT2   9991

#define NUM_BUF 1024
#define MAX_BUF 8192
static unsigned char data_buf1[MAX_BUF];
static unsigned char data_buf2[MAX_BUF];
static unsigned char data_buf_write1[MAX_BUF]="Client 1 is alive. You may continue ....";
static unsigned char data_buf_write2[MAX_BUF]="Client 2 is alive. You may continue ....";


#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x10000)

static char stack_server[STACK_SIZE];
static cyg_thread server_thread_data;
static cyg_handle_t server_thread_handle;

static char stack_client1[STACK_SIZE];
static cyg_thread client1_thread_data; 
static cyg_handle_t client1_thread_handle;  

static char stack_client2[STACK_SIZE];
static cyg_thread client2_thread_data;
static cyg_handle_t client2_thread_handle;


#define MAIN_THREAD_PRIORITY     CYGPKG_NET_THREAD_PRIORITY-4

void
pexit(char *s)
{
    CYG_TEST_FAIL_FINISH( s );
}


#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

void server(void)
{
    int s_s1, e_s1, s_s2, e_s2;
    struct sockaddr_in e_s1_addr,e_s2_addr,local;
    fd_set in_fds;
    int len;
    int num;
    
    char *hello_string=" Hello eCos network \n";
    diag_printf("TCP SERVER:");
    diag_printf(hello_string);

    s_s1 = socket(AF_INET, SOCK_STREAM, 0);
    if (s_s1 < 0) {
        pexit("stream socket");
    }   
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SOURCE_PORT1);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(s_s1, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /source_1/ error");
    }
    listen(s_s1, SOMAXCONN);

    s_s2 = socket(AF_INET, SOCK_STREAM, 0);    
    if (s_s2 < 0) {    
        pexit("stream socket");
    }  
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SOURCE_PORT2);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(s_s2, (struct sockaddr *) &local, sizeof(local)) < 0) {    
        pexit("bind /source_2/ error");
    }
    listen(s_s2, SOMAXCONN);    

   
    e_s1 = 0; e_s2 = 0; 

    
    while (true) {
	FD_ZERO(&in_fds);
	FD_SET(s_s1, &in_fds);
	FD_SET(s_s2, &in_fds);
        num = select ( max(s_s1,s_s2)+1, &in_fds,0,0,0);
	if (FD_ISSET(s_s1,&in_fds)) {
		len = sizeof(e_s1_addr);
        	if ((e_s1 = accept(s_s1,(struct sockaddr *)&e_s1_addr,&len))<0) 
			{
			pexit("accept /source_1/");
    			}
	diag_printf("TCP SERVER connection from %s: %d\n",
	       inet_ntoa(e_s1_addr.sin_addr),ntohs(e_s1_addr.sin_port));
        }
        if (FD_ISSET(s_s2,&in_fds)) {
		len = sizeof(e_s2_addr);
		if ((e_s2 = accept(s_s2,(struct sockaddr *)&e_s2_addr,&len))<0)
			{
			pexit("accept /source_2/");
			}
        diag_printf("TCP SERVER connection from %s: %d\n",
               inet_ntoa(e_s2_addr.sin_addr), ntohs(e_s2_addr.sin_port));
        }

        if ((e_s1 != 0) && ( e_s2 != 0)) {
            break;
            }
     
	}   /* while (true) */ 

     if ((len = read(e_s1, data_buf1, MAX_BUF)) < 0  )
        {
                perror("I/O error");
        }
   diag_printf("SERVER : %s\n",data_buf1);

     if ((len = read(e_s2, data_buf2, MAX_BUF)) < 0  )    
        {
                perror("I/O error");
        }
   diag_printf("SERVER : %s\n",data_buf2);
    
}

void client1(void)
{
    int s_source;
    struct sockaddr_in local;
    int len;

    diag_printf("client 1 :started\n");
    
    s_source = socket(AF_INET, SOCK_STREAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(SOURCE_PORT1);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    if (connect(s_source, (struct sockaddr *)&local, sizeof(local)) < 0) {
        pexit("Can't connect to target");
    }
    
    if ((len = write(s_source,data_buf_write1,40)) < 0)	{
	CYG_TEST_FAIL_FINISH("Error writing buffer");
    } 
}

void client2(void)
{
    int s_source;
    struct sockaddr_in local;
    int len;
    
    diag_printf("client 2 :started\n");
    
    s_source = socket(AF_INET, SOCK_STREAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_len = sizeof(local);
    local.sin_port = htons(SOURCE_PORT2);
    local.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   
    if (connect(s_source, (struct sockaddr *)&local, sizeof(local)) < 0) {
        pexit("Can't connect to target");
    }
    
    if ((len = write(s_source,data_buf_write2,40)) < 0) {
        CYG_TEST_FAIL_FINISH("Error writing buffer");
    }
}


void
tcp_server(cyg_addrword_t param)
{
    init_all_network_interfaces();
    diag_printf("Start TCP server - test\n");
    cyg_thread_resume(client1_thread_handle);   // Start it
    cyg_thread_resume(client2_thread_handle);   // Start it
#if NLOOP > 0
    server();
    CYG_TEST_PASS_FINISH("Server returned OK");
#endif
    CYG_TEST_NA( "No loopback devs" );
}

void
tcp_client_1(cyg_addrword_t param)
{
    diag_printf("Start TCP client 1 - test\n");
#if NLOOP > 0
    client1(); 
#endif
}

void
tcp_client_2(cyg_addrword_t param)
{
    diag_printf("Start TCP client 2 - test\n");
#if NLOOP > 0
    client2();
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
                      tcp_client_1,             // entry
                      0,                        // entry parameter
                      "TCP loopback client1",   // Name
                      &stack_client1[0],        // Stack
                      STACK_SIZE,               // Size
                      &client1_thread_handle,   // Handle
                      &client1_thread_data      // Thread data structure
            );

    cyg_thread_create(MAIN_THREAD_PRIORITY,     // Priority
                      tcp_client_2,             // entry
                      0,                        // entry parameter
                      "TCP llopback client2",   // Name
                      &stack_client2[0],        // Stack
                      STACK_SIZE,               // Size
                      &client2_thread_handle,   // Handle
                      &client2_thread_data      // Thread data structure
            );

    cyg_scheduler_start();
}

