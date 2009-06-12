//==========================================================================
//
//      tests/nc_test_master.c
//
//      Network characterizations test (master portion)
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

// Network characterization test code - master portion

#include "nc_test_framework.h"

#ifdef __ECOS
#ifndef CYGPKG_LIBC_STDIO
#define perror(s) diag_printf(#s ": %s\n", strerror(errno))
#endif
#define STACK_SIZE (CYGNUM_HAL_STACK_SIZE_TYPICAL + 0x1000)
static char stack[STACK_SIZE];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;
#endif

struct test_params {
    int  argc;
    char **argv;
};

#define MAX_BUF 32*1024
static unsigned char in_buf[MAX_BUF], out_buf[MAX_BUF];

static int test_seq = 1;
static long long idle_count;
static long      idle_ticks;
#define IDLE_TEST_TIME   10

struct pause {
    int pause_ticks;
    int pause_threshold;
};
#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

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
    cyg_test_exit();
}

#ifdef __ECOS
#ifndef CYGPKG_SNMPLIB
int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
    cyg_tick_count_t cur_time;
    cur_time = cyg_current_time();
    tv->tv_sec = cur_time / 100;
    tv->tv_usec = (cur_time % 100) * 10000;
}
#else
int
gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
#endif
void
show_results(const char *msg, struct timeval *start, 
             struct timeval *end, int nbufs, int buflen,
             int lost, int seq_errors)
{
    struct timeval tot_time;
#ifndef __ECOS
    double real_time, thru;
    long tot_bytes = nbufs * buflen;
#endif
    timersub(end, start, &tot_time);
    test_printf("%s - %d bufs of %d bytes in %d.%02d seconds",
                msg, nbufs, buflen, 
                tot_time.tv_sec, tot_time.tv_usec / 10000);
#ifndef __ECOS
    real_time = tot_time.tv_sec + ((tot_time.tv_usec / 10000) * .01);
    // Compute bytes / second (rounded up)
    thru = tot_bytes / real_time;
    // Convert to Mb / second
    test_printf(" - %.2f KB/S", thru / 1024.0);
    test_printf(" - %.4f Mbit/S (M = 10^6)", thru * 8.0 / 1000000.0);
#endif
    if (lost) {
        test_printf(", %d lost", lost);
    }
    if (seq_errors) {
        test_printf(", %d out of sequence", seq_errors);
    }
    test_printf("\n");
}

void
new_test(void)
{
    test_seq++;
}

int
nc_message(int s, struct nc_request *req, 
           struct nc_reply *reply, struct sockaddr_in *slave)
{
    int len;
    fd_set fds;
    struct timeval timeout;
    len = sizeof(*slave);
    req->seq = htonl(test_seq);
    if (sendto(s, req, sizeof(*req), 0, (struct sockaddr *)slave, len) < 0) {
        perror("sendto");
        return false;
    }
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    timeout.tv_sec = NC_REPLY_TIMEOUT;
    timeout.tv_usec = 0;
    if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
        test_printf("No response to command\n");
        return false;
    }
    if (recvfrom(s, reply, sizeof(*reply), 0, 0, 0) < 0) {
        perror("recvfrom");
        return false;
    }
    if (reply->seq != req->seq) {
        test_printf("Response out of order - sent: %d, recvd: %d\n",
                    ntohl(req->seq), ntohl(reply->seq));
        return false;
    }
    return true;
}

void
show_test_results(struct nc_test_results *results)
{
    if ((ntohl(results->key1) == NC_TEST_RESULT_KEY1) &&
        (ntohl(results->key2) == NC_TEST_RESULT_KEY2) &&
        (ntohl(results->seq) == test_seq)) {
        test_printf("   slave sent %d, recvd %d\n", 
                    ntohl(results->nsent), ntohl(results->nrecvd));
    } else {
        test_printf("   ... invalid results - keys: %x/%x, seq: %d/%d\n",
                    ntohl(results->key1), ntohl(results->key2),
                    ntohl(results->seq), test_seq);
    }
}

void
do_udp_test(int s1, int type, struct sockaddr_in *slave,
            int nbufs, int buflen, int pause_time, int pause_threshold)
{
    int i, s, td_len, seq, seq_errors, total_packets;
    struct sockaddr_in test_chan_master, test_chan_slave;
    struct timeval start_time, end_time;
    struct nc_request req;
    struct nc_reply reply;
    struct nc_test_results results;
    struct nc_test_data *tdp;
    fd_set fds;
    struct timeval timeout;
    int lost_packets = 0;
    int need_send, need_recv;
    const char *type_name;
    int pkt_ctr = 0;

    need_recv = true;  need_send = true;  type_name = "UDP echo";
    switch (type) {
    case NC_REQUEST_UDP_RECV:
        need_recv = false;
        need_send = true;
        type_name = "UDP recv";
        break;
    case NC_REQUEST_UDP_SEND:
        need_recv = true;
        need_send = false;
        type_name = "UDP send";
        break;
    case NC_REQUEST_UDP_ECHO:
        break;
    }

    new_test();
    req.type = htonl(type);
    req.nbufs = htonl(nbufs);
    req.buflen = htonl(buflen);
    req.slave_port = htonl(NC_TESTING_SLAVE_PORT);
    req.master_port = htonl(NC_TESTING_MASTER_PORT);
    nc_message(s1, &req, &reply, slave);
    if (reply.response != ntohl(NC_REPLY_ACK)) {
        test_printf("Slave denied %s [%d,%d] test\n", type_name, nbufs, buflen);
        return;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        pexit("datagram socket");
    }

    memset(&test_chan_master, 0, sizeof(test_chan_master));
    test_chan_master.sin_family = AF_INET;
#ifdef __ECOS
    test_chan_master.sin_len = sizeof(test_chan_master);
#endif
    test_chan_master.sin_port = htons(ntohl(req.master_port));
    test_chan_master.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *) &test_chan_master, sizeof(test_chan_master)) < 0) {
        perror("bind");
        close(s);
        return;
    }
    test_printf("Start %s [%d,%d]", type_name, nbufs, buflen);
    if (pause_time) {
        test_printf(" - %dms delay after %d packet%s\n", pause_time*10, 
                    pause_threshold, pause_threshold > 1 ? "s" : "");
    } else {
        test_printf(" - no delays\n");
    }

    gettimeofday(&start_time, 0);
    memcpy(&test_chan_slave, slave, sizeof(*slave));
    test_chan_slave.sin_port = htons(ntohl(req.slave_port));
    seq = 0;  seq_errors = 0;  total_packets = 0;
    for (i = 0;  i < nbufs;  i++) {
        td_len = buflen + sizeof(struct nc_test_data);
        if (need_send) {
            tdp = (struct nc_test_data *)out_buf;
            tdp->key1 = htonl(NC_TEST_DATA_KEY1);
            tdp->key2 = htonl(NC_TEST_DATA_KEY2);
            tdp->seq = htonl(i);
            tdp->len = htonl(td_len);
            if (sendto(s, tdp, td_len, 0, 
                       (struct sockaddr *)&test_chan_slave, sizeof(test_chan_slave)) < 0) {
                perror("sendto");
                close(s);
                return;
            }
            total_packets++;
        }
        if (need_recv) {
            FD_ZERO(&fds);
            FD_SET(s, &fds);
            timeout.tv_sec = NC_TEST_TIMEOUT;
            timeout.tv_usec = 0;
            if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
                test_printf("Slave timed out after %d buffers\n", i);
                lost_packets++;
            } else {
                tdp = (struct nc_test_data *)in_buf;
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
                        seq_errors++;
                        if (!need_send) {
                            // Reset sequence to what the slave wants
                            seq = ntohl(tdp->seq);
                        }
                    }
                } else {
                    test_printf("Bad data packet - key: %x/%x, seq: %d\n",
                                ntohl(tdp->key1), ntohl(tdp->key2),
                                ntohl(tdp->seq));
                }
                total_packets++;
            }
            seq++;
            if (seq == nbufs) {
                break;
            }
            if (pause_time && (++pkt_ctr == pause_threshold)) {
                pkt_ctr = 0;
                test_delay(pause_time);
            }
        }
    }
    gettimeofday(&end_time, 0);
    show_results(type_name, &start_time, &end_time, total_packets, buflen, 
                 lost_packets, seq_errors);
    // Fetch results record
    FD_ZERO(&fds);
    FD_SET(s, &fds);
    timeout.tv_sec = NC_RESULTS_TIMEOUT;
    timeout.tv_usec = 0;
    if (select(s+1, &fds, 0, 0, &timeout) <= 0) {
        test_printf("No results record sent\n");
    } else {
        if (recvfrom(s, &results, sizeof(results), 0, 0, 0) < 0) {
            perror("recvfrom");
        }
        show_test_results(&results);
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

void
do_tcp_test(int s1, int type, struct sockaddr_in *slave,
            int nbufs, int buflen, int pause_time, int pause_threshold)
{
    int i, s, td_len, len, seq, seq_errors, total_packets, res;
    struct sockaddr_in test_chan_slave;
    struct timeval start_time, end_time;
    struct nc_request req;
    struct nc_reply reply;
    struct nc_test_results results;
    struct nc_test_data *tdp;
    int lost_packets = 0;
    int conn_failures = 0;
    int need_send, need_recv;
    const char *type_name;
    int pkt_ctr = 0;

    need_recv = true;  need_send = true;  type_name = "TCP echo";
    switch (type) {
    case NC_REQUEST_TCP_RECV:
        need_recv = false;
        need_send = true;
        type_name = "TCP recv";
        break;
    case NC_REQUEST_TCP_SEND:
        need_recv = true;
        need_send = false;
        type_name = "TCP send";
        break;
    case NC_REQUEST_TCP_ECHO:
        break;
    }

    new_test();
    req.type = htonl(type);
    req.nbufs = htonl(nbufs);
    req.buflen = htonl(buflen);
    req.slave_port = htonl(NC_TESTING_SLAVE_PORT);
    req.master_port = htonl(NC_TESTING_MASTER_PORT);
    nc_message(s1, &req, &reply, slave);
    if (reply.response != ntohl(NC_REPLY_ACK)) {
        test_printf("Slave denied %s [%d,%d] test\n", type_name, nbufs, buflen);
        return;
    }

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        pexit("datagram socket");
    }

    test_printf("Start %s [%d,%d]", type_name, nbufs, buflen);
    if (pause_time) {
        test_printf(" - %dms delay after %d packet%s\n", pause_time*10, 
                    pause_threshold, pause_threshold > 1 ? "s" : "");
    } else {
        test_printf(" - no delays\n");
    }

    test_delay(3*100);  
    memcpy(&test_chan_slave, slave, sizeof(*slave));
    test_chan_slave.sin_port = htons(ntohl(req.slave_port));
    while (connect(s, (struct sockaddr *)&test_chan_slave, sizeof(*slave)) < 0) { 
        perror("Can't connect to slave");
        if (++conn_failures > MAX_ERRORS) {
            test_printf("Too many connection failures - giving up\n");
            return;
        }
        if (errno == ECONNREFUSED) {
            // Give the slave a little time
            test_delay(100);  // 1 second
        } else {
            return;
        }
    }

    gettimeofday(&start_time, 0);
    seq = 0;  seq_errors = 0;  total_packets = 0;
    for (i = 0;  i < nbufs;  i++) {
        td_len = buflen + sizeof(struct nc_test_data);
        if (need_send) {
            tdp = (struct nc_test_data *)out_buf;
            tdp->key1 = htonl(NC_TEST_DATA_KEY1);
            tdp->key2 = htonl(NC_TEST_DATA_KEY2);
            tdp->seq = htonl(i);
            tdp->len = htonl(td_len);
            if ((len = write(s, tdp, td_len)) != td_len) {
                if (len < 0) {
                    perror("write");
                } else {
                    test_printf("short write - only %d or %d bytes written\n", len, td_len);
                }
                close(s);
                return;
            }
            total_packets++;
        }
        if (need_recv) {
            tdp = (struct nc_test_data *)in_buf;
            res = do_read(s, tdp, td_len);
            if (res != td_len) {
                test_printf("Slave timed out after %d buffers\n", i);
                lost_packets++;
            } else {
                if ((ntohl(tdp->key1) == NC_TEST_DATA_KEY1) &&
                    (ntohl(tdp->key2) == NC_TEST_DATA_KEY2)) {
                    if (ntohl(tdp->seq) != seq) {
                        test_printf("Packets out of sequence - recvd: %d, expected: %d\n",
                                    ntohl(tdp->seq), seq);
                        seq_errors++;
                        if (!need_send) {
                            // Reset sequence to what the slave wants
                            seq = ntohl(tdp->seq);
                        }
                    }
                } else {
                    test_printf("Bad data packet - key: %x/%x, seq: %d\n",
                                ntohl(tdp->key1), ntohl(tdp->key2),
                                ntohl(tdp->seq));
                }
                total_packets++;
            }
            seq++;
            if (seq == nbufs) {
                break;
            }
            if (pause_time && (++pkt_ctr == pause_threshold)) {
                pkt_ctr = 0;
                test_delay(pause_time);
            }
        }
    }
    gettimeofday(&end_time, 0);
    show_results(type_name, &start_time, &end_time, total_packets, buflen, 
                 lost_packets, seq_errors);
    // Fetch results record
    if (do_read(s, &results, sizeof(results)) != sizeof(results)) {
        test_printf("No results record sent\n");
    } else {
        show_test_results(&results);
    }
    close(s);
}

int
do_set_load(int s, struct sockaddr_in *slave, int load_level)
{
    struct nc_request req;
    struct nc_reply reply;
    req.type = htonl(NC_REQUEST_SET_LOAD);
    req.nbufs = htonl(load_level);
    nc_message(s, &req, &reply, slave);
    return (reply.response == ntohl(NC_REPLY_ACK));
}

int
do_start_idle(int s, struct sockaddr_in *slave)
{
    struct nc_request req;
    struct nc_reply reply;
    req.type = htonl(NC_REQUEST_START_IDLE);
    nc_message(s, &req, &reply, slave);
    return (reply.response == ntohl(NC_REPLY_ACK));
}

void
do_stop_idle(int s, struct sockaddr_in *slave, int calibrate)
{
    struct nc_request req;
    struct nc_reply reply;
    long long res_idle_count;
    long long adj_count;
    int idle, res_idle_ticks;
    req.type = htonl(NC_REQUEST_STOP_IDLE);
    nc_message(s, &req, &reply, slave);
    if (reply.response == ntohl(NC_REPLY_ACK)) {
        res_idle_ticks = ntohl(reply.misc.idle_results.elapsed_time);
        res_idle_count = ((long long)ntohl(reply.misc.idle_results.count[0]) << 32) |
            ntohl(reply.misc.idle_results.count[1]);
        test_printf("IDLE - ticks: %d, count: %ld", 
                    res_idle_ticks, res_idle_count);
        if (calibrate) {
            idle_count = res_idle_count;
            idle_ticks = res_idle_ticks;
        } else {
            adj_count = res_idle_count / res_idle_ticks;
            adj_count *= idle_ticks;
            idle = (int) ((adj_count * 100) / idle_count);
            test_printf(", %d%% idle", idle);
        }
        test_printf("\n");
    } else {
        test_printf("Slave failed on IDLE\n");
    }
}

void
do_disconnect(int s, struct sockaddr_in *slave)
{
    struct nc_request req;
    struct nc_reply reply;
    req.type = htonl(NC_REQUEST_DISCONNECT);
    nc_message(s, &req, &reply, slave);
}

static void
nc_master(struct test_params *p)
{
    int s, i;
    struct sockaddr_in slave, my_addr;
    struct hostent *host;
    struct pause pause_times[] = {
        {0,0}, {1,10}, {5,10}, {10,10}, {1,1} };

    if (p->argc != 2) {
        test_printf("Need exactly 'master <host>'\n");
        return;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        pexit("datagram socket");
    }

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
#ifdef __ECOS
    my_addr.sin_len = sizeof(my_addr);
#endif
    my_addr.sin_port = htons(NC_MASTER_PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *) &my_addr, sizeof(my_addr)) < 0) {
        pexit("bind");
    }

    host = gethostbyname(p->argv[1]);
    if (host == (struct hostent *)NULL) {
        pexit("gethostbyname");
    }

    memset(&slave, 0, sizeof(slave));
    slave.sin_family = AF_INET;
#ifdef __ECOS
    slave.sin_len = sizeof(slave);
#endif
    slave.sin_port = htons(NC_SLAVE_PORT);
    memcpy(&slave.sin_addr.s_addr, host->h_addr, host->h_length);

    test_printf("================== No load, master at 100%% ========================\n");
#if 0
    do_udp_test(s, NC_REQUEST_UDP_ECHO, &slave, 640, 1024, 0, 0);
    do_udp_test(s, NC_REQUEST_UDP_SEND, &slave, 640, 1024, 0, 0);
    do_udp_test(s, NC_REQUEST_UDP_RECV, &slave, 640, 1024, 0, 0);
    do_tcp_test(s, NC_REQUEST_TCP_ECHO, &slave, 640, 1024, 0, 0);
    do_tcp_test(s, NC_REQUEST_TCP_SEND, &slave, 640, 1024, 0, 0);
    do_tcp_test(s, NC_REQUEST_TCP_RECV, &slave, 640, 1024, 0, 0);
#endif
    do_tcp_test(s, NC_REQUEST_TCP_ECHO, &slave, 64, 10240, 0, 0);

    if (do_set_load(s, &slave, 0)) {
        test_printf("\n====================== Various slave compute loads ===================\n");
        for (i = 0;  i < 60;  i += 10) {
            test_printf(">>>>>>>>>>>> slave processing load at %d%%\n", i);
            do_set_load(s, &slave, i);
            do_udp_test(s, NC_REQUEST_UDP_ECHO, &slave, 2048, 1024, 0, 0);
            do_tcp_test(s, NC_REQUEST_TCP_ECHO, &slave, 2048, 1024, 0, 0);
        }
    }

    if (do_start_idle(s, &slave)) {
        test_printf("\n====================== Various master loads ===================\n");
        test_printf("Testing IDLE for %d seconds\n", IDLE_TEST_TIME);
        test_delay(IDLE_TEST_TIME*100);
        do_stop_idle(s, &slave, true);
        for (i = 0;  i < LENGTH(pause_times);  i++) {
            do_start_idle(s, &slave);
            do_udp_test(s, NC_REQUEST_UDP_ECHO, &slave, 2048, 1024, 
                        pause_times[i].pause_ticks, pause_times[i].pause_threshold);
            do_stop_idle(s, &slave, false);
            do_start_idle(s, &slave);
            do_tcp_test(s, NC_REQUEST_TCP_ECHO, &slave, 2048, 1024, 
                        pause_times[i].pause_ticks, pause_times[i].pause_threshold);
            do_stop_idle(s, &slave, false);
        }
    }

    do_disconnect(s, &slave);
    close(s);
}

void
net_test(test_param_t p)
{
    test_printf("Start Network Characterization - MASTER\n");
#ifdef __ECOS
    init_all_network_interfaces();
#endif
    nc_master((struct test_params *)p);
    cyg_test_exit();
}

#ifdef __ECOS
void
cyg_start(void)
{
    static struct test_params p;
    // Create a main thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(10,                // Priority - just a number
                      net_test,          // entry
                      (cyg_addrword_t)&p,// entry parameter
                      "Network test",    // Name
                      &stack[0],         // Stack
                      STACK_SIZE,        // Size
                      &thread_handle,    // Handle
                      &thread_data       // Thread data structure
            );
    cyg_thread_resume(thread_handle);  // Start it
    cyg_scheduler_start();
}

#else

int
main(int argc, char *argv[])
{
    struct test_params p;
    p.argc = argc;
    p.argv = argv;
    net_test(&p);
}
#endif
