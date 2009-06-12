//==========================================================================
//
//      tests/host_echo.c
//
//      Simple TCP throughput test - echo component FOR *HOST*
//      * CAUTION: host, i.e. non eCos, only *
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

#undef _KERNEL
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>

#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#include <netdb.h>

#define true 1
#define false 1

#define TNR_ON()
#define TNR_OFF()

#define diag_printf printf

void
pexit(char *s)
{
    perror(s);
    exit(1);
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

int max( int a, int b ) { return (a>b)?a:b; }
int min( int a, int b ) { return (a<b)?a:b; }


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

static void
echo_test(void * p)
{
    int s_source, s_sink, e_source, e_sink;
    struct sockaddr_in e_source_addr, e_sink_addr, local;
    int one = 1;
    fd_set in_fds;
    int i, num, len;
    struct test_params params,nparams;
    struct test_status status,nstatus;

    s_source = socket(AF_INET, SOCK_STREAM, 0);
    if (s_source < 0) {
        pexit("stream socket");
    }
    if (setsockopt(s_source, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt /source/ SO_REUSEADDR");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
//    local.sin_len = sizeof(local);
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
//    local.sin_len = sizeof(local);
    local.sin_port = ntohs(SINK_PORT);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(s_sink, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /sink/ error");
    }
    if (setsockopt(s_sink, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt /sink/ SO_REUSEADDR");
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

    TNR_ON();

    // Echo the data from the source to the sink hosts
    for (i = 0;  i < params.nbufs;  i++) {
        if ((len = do_read(e_source, data_buf, params.bufsize)) != params.bufsize) {
            TNR_OFF();
            diag_printf("Can't read buf #%d: ", i+1);
            if (len < 0) {
                perror("I/O error");
            } else {
                diag_printf("short read - only %d bytes\n", len);
            }
            TNR_ON();
        }
        if ((len = do_write(e_sink, data_buf, params.bufsize)) != params.bufsize) {
            TNR_OFF();
            diag_printf("Can't write buf #%d: ", i+1);
            if (len < 0) {
                perror("I/O error");
            } else {
                diag_printf("short write - only %d bytes\n", len);
            }
            TNR_ON();
        }
    }

    TNR_OFF();

    // Wait for the data to drain and the "sink" to tell us all is OK.
    if (do_read(e_sink, &status, sizeof(status)) != sizeof(status)) {
        pexit("Can't receive ACK from 'sink' host");
    }

}


int
main(int argc, char *argv[])
{
    echo_test(argv[1]);
    return 0;
}


// EOF

