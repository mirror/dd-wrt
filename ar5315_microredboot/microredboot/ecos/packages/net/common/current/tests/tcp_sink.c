//==========================================================================
//
//      tests/tcp_sink.c
//
//      Simple TCP throughput test - sink component
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

// Network throughput test code

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

#define SOURCE_PORT 9990
#define SINK_PORT   9991

struct test_params {
    long nbufs;
    long bufsize;
    long load;
};

struct test_status {
    long ok;
};

#define NUM_BUF 128
#define MAX_BUF 32 * 8192
static unsigned char data_buf[MAX_BUF];

void
pexit(char *s)
{
    perror(s);
    exit(1);
}

void
show_results(struct timeval *start, struct timeval *end, 
             int nbufs, int buflen)
{
    struct timeval tot_time;
    long tot_bytes = nbufs * buflen;
    double real_time, thru;
    timersub(end, start, &tot_time);
    printf("SINK complete - %d bufs of %d bytes in %ld.%02ld seconds",
           nbufs, buflen, 
           tot_time.tv_sec, tot_time.tv_usec / 10000);
    real_time = tot_time.tv_sec + ((tot_time.tv_usec / 10000) * .01);
    // Compute bytes / second (rounded up)
    thru = tot_bytes / real_time;
    // Convert to Mb / second
    printf(" - %.2f KB/S", thru / 1024.0);
    printf(" - %.4f Mbit/S (M = 10^6)", thru * 8.0 / 1000000.0);
    printf("\n");
}

int
do_read(int s, unsigned char *buf, int len)
{
    int total, slen, rlen;
    total = 0;
    rlen = len;
    while (total < len) {
        slen = read(s, buf, rlen);
        if (slen != rlen) {
            if (slen < 0) {
                printf("Error after reading %d bytes\n", total);
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
do_write(int s, unsigned char *buf, int len)
{
    int total, slen, rlen;
    total = 0;
    rlen = len;
    while (total < len) {
        slen = write(s, buf, rlen);
        if (slen != rlen) {
            if (slen < 0) {
                printf("Error after writing %d bytes\n", total);
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
sink_test(char *echo_node)
{
    int s_sink;
    struct sockaddr_in slave, local;
    int one = 1;
    int len;
    struct hostent *host;
    struct test_params params, nparams;
    struct test_status status, nstatus;
    struct timeval start_time, end_time;
    int i;

    printf("Start TCP test - SINK mode to %s\n", echo_node);

    host = gethostbyname(echo_node);
    if (host == (struct hostent *)NULL) {
        pexit("gethostbyname");
    }

    memset(&slave, 0, sizeof(slave));
    slave.sin_family = AF_INET;
#ifdef __ECOS
    slave.sin_len = sizeof(slave);
#endif
    slave.sin_port = htons(SINK_PORT);
    memcpy(&slave.sin_addr.s_addr, host->h_addr, host->h_length);

    s_sink = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sink < 0) {
        pexit("stream socket");
    }
    if (setsockopt(s_sink, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))) {
        pexit("setsockopt /sink/ SO_REUSEADDR");
    }
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
#ifdef __ECOS
    local.sin_len = sizeof(local);
#endif
    local.sin_port = INADDR_ANY;
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(s_sink, (struct sockaddr *) &local, sizeof(local)) < 0) {
        pexit("bind /sink/ error");
    }

    if (connect(s_sink, (struct sockaddr *)&slave, sizeof(slave)) < 0) { 
        pexit("Can't connect to target");
    }

    // Get testing paramters from middleman
    if (do_read(s_sink, (unsigned char *)&nparams, sizeof(nparams))
                  != sizeof(nparams)) {
        pexit("Can't read initialization parameters");
    }
   
    params.nbufs = ntohl(nparams.nbufs);
    params.bufsize = ntohl(nparams.bufsize);
    params.load = ntohl(nparams.load);
     
    printf("Using %ld buffers of %ld bytes each\n", params.nbufs, params.bufsize);

    // Actual test
    gettimeofday(&start_time, 0);
    for (i = 0;  i < params.nbufs;  i++) {
        if ((len = do_read(s_sink, data_buf, params.bufsize)) != params.bufsize) {
            printf("Error reading buffer #%d:", i+1);
            if (len < 0) {
                perror("can't read data");
            } else {
                printf(" short data, only read %d bytes\n", len);
            }
        }
    }
    gettimeofday(&end_time, 0);
    show_results(&start_time, &end_time, params.nbufs, params.bufsize);

    // Tell the middleman
   
    nstatus.ok = htonl(status.ok);
   
    if (do_write(s_sink, (unsigned char *)&nstatus, sizeof(nstatus))
                        != sizeof(nstatus)) {
        pexit("Can't send ACK to 'echo' host");
    }
}

int
main(int argc, char *argv[])
{
    sink_test(argv[1]);
    return 0;
}

