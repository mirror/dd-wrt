//==========================================================================
//
//      tests/addr_test.c
//
//      Test network "address" functions
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
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:         2002-03-19
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// Networking library test code

#include <network.h>
#include <cyg/infra/testcase.h>

#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

void
pexit(char *s)
{
    char msg[256];

    diag_sprintf(msg, "%s: %s", s, strerror(errno));
    CYG_TEST_FAIL_FINISH(msg);
}

static void
walk_addrs(struct addrinfo *ai, char *title)
{
    char addr_buf[256];

    diag_printf("INFO: %s\n", title);
    while (ai) {
        _inet_ntop(ai->ai_addr, addr_buf, sizeof(addr_buf));
        diag_printf("INFO: Family: %2d, Socket: %d, Addr: %s, Port: %d\n", 
                    ai->ai_family, ai->ai_socktype, addr_buf, _inet_port(ai->ai_addr));
        switch (ai->ai_family) {
        case AF_INET:
          if (ai->ai_addrlen != sizeof(struct sockaddr_in)) {
            CYG_TEST_FAIL("ai_addrlen wrong for an IPv4 address");
          }
          break;
#ifdef CYGPKG_NET_INET6
        case AF_INET6:
          if (ai->ai_addrlen != sizeof(struct sockaddr_in6)) {
            CYG_TEST_FAIL("ai_addrlen wrong for an IPv6 address");
          }
          break;
#endif
        default:
          CYG_TEST_FAIL("Unknown address family");
        }
        ai = ai->ai_next;
    }
}

void 
net_test(CYG_ADDRWORD data)
{
    int err;
    struct addrinfo *addrs, hints;
    size_t hostlen = 128;
    size_t servlen = 64;
    char host[hostlen];
    char serv[servlen];

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all passive");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      NULL, 0, serv,servlen, 0);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: service: %s\n",serv);
    freeaddrinfo(addrs);

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all active");
    freeaddrinfo(addrs);

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "IPv4 passive");
    freeaddrinfo(addrs);

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo("192.168.1.2", "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "IPv4 passive 192.168.1.2");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      host, hostlen, serv, servlen, NI_NUMERICHOST);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: host: %s service: %s\n",host, serv);
    freeaddrinfo(addrs);

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo("192.168.1.2", "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all passive 192.168.1.2");
    freeaddrinfo(addrs);

#ifdef CYGPKG_NET_INET6
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "IPv6 passive");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      NULL, 0, serv,servlen, 0);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: service: %s\n",serv);
    freeaddrinfo(addrs);
    
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "7734", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all passive");
    freeaddrinfo(addrs);
    
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICHOST;
    if ((err = getaddrinfo("fe80::260:97ff:feb0:866e", "7734", &hints, &addrs)) 
	!= EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all passive fe80::260:97ff:feb0:866e");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      host, hostlen, serv, servlen, NI_NUMERICHOST);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: host: %s service: %s\n",host, serv);
    freeaddrinfo(addrs);
#endif

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "ftp", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all snmp/udp");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      NULL, 0, serv,servlen, 0);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: service: %s\n",serv);
    freeaddrinfo(addrs);

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE|AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "snmp", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all snmp/udp");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      NULL, 0, serv,servlen, 0);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: service: %s\n",serv);
    freeaddrinfo(addrs);
    
    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_flags = AI_NUMERICHOST;
    if ((err = getaddrinfo(NULL, "snmp", &hints, &addrs)) != EAI_NONE) {
        diag_printf("<ERROR> can't getaddrinfo(): %s\n", gai_strerror(err));
        pexit("getaddrinfo");
    }
    walk_addrs(addrs, "all snmp/*");

    err = getnameinfo(addrs->ai_addr, addrs->ai_addrlen, 
                      NULL, 0, serv,servlen, NI_NUMERICSERV);
    if (err != EAI_NONE) {
        diag_printf("<ERROR> can't getnameinfo(): %s\n", gai_strerror(err));
        pexit("getnameinfo");
    }
    diag_printf("INFO: service: %s\n",serv);
    freeaddrinfo(addrs);

    CYG_TEST_PASS_FINISH("Address [library] test OK");

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
