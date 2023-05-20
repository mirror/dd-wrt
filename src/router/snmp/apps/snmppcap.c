/*
 * Copyright (c) 2015, Arista Networks, inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/large_fd_set.h>
#include <net-snmp/agent/snmp_agent.h>
#include <net-snmp/agent/snmp_vars.h>
#include <net-snmp/agent/mib_modules.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pcap/pcap.h>

#define FAKE_FD 3
/*
 * This is a funny little program, hooking together callbacks
 * to get packets from a pcap format file to go through
 * net-snmp's main loop.  Both net-snmp and libpcap have
 * callback-based processing, so:
 *
 * We create a fake snmp transport that calls snmppcap_recv()
 * to receive a packet, and use snmp_add() to add the session
 * and the transport directly.
 *
 * We create a session callback on the session that just
 * prints out the varbind list that we got.
 *
 * We use the libpcap pcap_dispatch() to loop through all
 * the packets in the pcap file, and then pretend to snmp_read2()
 * that a fake file descriptor is ready.  Luckily, there is
 * only one session active, and it happens to also claim to
 * be using that file descriptor, so snmplib calls the transport
 * receive function, snmppcap_recv().  If the packet parses,
 * we get a callback at snmppcap_callback().  If it doesn't,
 * you may need to use the -D options.
 */
typedef struct mystuff {
    int pktnum;
} mystuff_t;

/*
 * These two globals are used to communicate between handle_pcap()
 * and snmppcap_recv().  Don't try to multi-thread!  :-)
 */
const void *recv_data;
int recv_datalen;

/*
 * Are we acting as an agent, processing packets, or are we just
 * parsing them and dumping them?
 */
int acting_as_agent = 0;

int
snmppcap_recv(netsnmp_transport *t, void *buf, int bufsiz, void **opaque, int *opaque_len)
{
    if (bufsiz > recv_datalen) {
        memcpy(buf, recv_data, recv_datalen);
        return recv_datalen;
    } else {
        return -1;
    }
}

int
snmppcap_send(netsnmp_transport *t, const void *buf, int size,
	                     void **opaque, int *olength)
{
    /*
     * This seems silly - we have a buffer full of an SNMP packet
     * but all we want is the parsed PDU.
     */
    printf( "Reply:\n" );
    printf( "/* TODO: re-parse the data into a PDU? */\n" );
    return 0;
}

/*
 * snmplib calls us back with the received packet.
 */
static int
snmppcap_callback(int op, netsnmp_session *sess, int reqid, netsnmp_pdu *pdu,
                  void *magic)
{
    mystuff_t *mystuff = (mystuff_t *)magic;
    netsnmp_variable_list *vars;

    /*
     * We ignore op, since we know there is only way we can be
     * called back, since this is not a "real" transport.
     */
    printf( "Packet %d PDU contents:\n", mystuff->pktnum );
    /*
     * TODO: print PDU type and other info?
     */
    for (vars = pdu->variables; vars; vars = vars->next_variable) {
       printf( "   " );
       print_variable(vars->name, vars->name_length, vars );
    }

    if (acting_as_agent) {
	pdu->flags = UCD_MSG_FLAG_ALWAYS_IN_VIEW;	/* Bypass VACM for now */
	pdu->version = sess->version;
	return handle_snmp_packet( op, sess, reqid, pdu, NULL );
    }
    return 0;
}

void
handle_pcap(u_char *user, const struct pcap_pkthdr *h,
                                         const u_char *bytes)
{
    size_t len;
    const u_char *buf;
    int skip;
    mystuff_t *mystuff = (mystuff_t *)user;
    netsnmp_large_fd_set lfdset;

    mystuff->pktnum++;

    /*
     * If it's not a full packet, then we can't parse it.
     */
    if ( h->caplen < h->len ) {
        printf( "Skipping packet #%d; we only have %d of %d bytes\n", mystuff->pktnum, h->caplen, h->len );
        return;
    }

    /*
     * For now, no error checking and almost no parsing.
     * Assume that we have all Ethernet/IPv4/UDP/SNMP.
     */
    skip = 14 /* Ethernet */ + 20 /* IPv4 */ + 8 /* UDP */;
    buf = bytes + skip;
    len = h->len - skip;

    printf( "Packet #%d:\n", mystuff->pktnum );

    /*
     * Store the data in the globals that we use to communicate
     */
    recv_data = buf;
    recv_datalen = len;

    /*
     * We call snmp_read2() pretending that our
     * fake file descriptor is ready to read.
     * This is a funny API to fake up - we need to
     * set our fake file descriptor so that our fake
     * receive function gets called.
     */
    netsnmp_large_fd_set_init(&lfdset, FD_SETSIZE);
    netsnmp_large_fd_setfd(FAKE_FD, &lfdset);
    snmp_read2(&lfdset);
    netsnmp_large_fd_set_cleanup(&lfdset);
}

void
usage(void)
{
    fprintf(stderr, "USAGE: snmppcap [OPTIONS] FILE\n\n");
    /* can't use snmp_parse_args_usage because it assumes an agent */
    snmp_parse_args_descriptions(stderr);
    fprintf(stderr, "\nsnmppcap options:\n");
    fprintf(stderr,
	    "\t-Ca\tAct as agent and respond to request (try with -Dresponse)\n");
}

static void
optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
	    case 'a':
		acting_as_agent = 1;
		break;
	    default:
		fprintf(stderr,
			"Unknown flag passed to -C: %c\n", optarg[-1]);
		exit(1);
	    }
	}
    }
}

int main(int argc, char **argv)
{
    netsnmp_session *ss;
    netsnmp_transport *transport;
    int arg;
    char errbuf[PCAP_ERRBUF_SIZE];
    char *fname;
    pcap_t *p;
    mystuff_t mystuff;

    ss = SNMP_MALLOC_TYPEDEF(netsnmp_session);
    switch (arg = snmp_parse_args(argc, argv, ss, "C:", optProc)) {
    case NETSNMP_PARSE_ARGS_ERROR:
        exit(1);
    case NETSNMP_PARSE_ARGS_SUCCESS_EXIT:
        exit(0);
    case NETSNMP_PARSE_ARGS_ERROR_USAGE:
        usage();
        exit(1);
    default:
        break;
    }
    if (arg != argc) {
        fprintf(stderr, "Specify exactly one file name\n");
        usage();
        exit(1);
    }
    fname = argv[ arg-1 ];
    p = pcap_open_offline( fname, errbuf );
    if ( p == NULL ) {
        fprintf(stderr, "%s: %s\n", fname, errbuf );
        return 1;
    }
    if ( pcap_datalink( p ) != DLT_EN10MB) {
        fprintf(stderr, "Only Ethernet pcaps currently supported\n");
        return 2;
    }

    const char *app_name = "snmppcap";
    if (acting_as_agent) {
	if (init_agent(app_name) != 0) {
	    snmp_log(LOG_ERR, "Agent initialization failed\n");
	    return 4;
	}
	init_mib_modules();
    }
    init_snmp(app_name);

    transport = SNMP_MALLOC_TYPEDEF(netsnmp_transport);
    if ( transport == NULL ) {
        fprintf(stderr, "Could not malloc transport\n" );
        return 3;
    }
    /*
     * We set up just enough of the transport to fake the main
     * loop into calling us back.
     */
    transport->sock = FAKE_FD;        /* nobody actually uses this as a file descriptor */
    transport->f_recv = snmppcap_recv;
    transport->f_send = snmppcap_send;
    transport->msgMaxSize = SNMP_MAX_PACKET_LEN;

    ss->callback = snmppcap_callback;
    ss->callback_magic = (void *)&mystuff;

    /* todo: add the option of a filter here */
    mystuff.pktnum = 0;
    /* todo: user, etc. parsing. */
    ss->securityModel = SNMP_SEC_MODEL_USM;
    printf("flags %lx securityModel %d version %ld securityNameLen %" NETSNMP_PRIz "d securityEngineIDLen %" NETSNMP_PRIz "d\n",
          ss->flags, ss->securityModel, ss->version,
          ss->securityNameLen, ss->securityEngineIDLen);
    create_user_from_session(ss);

    /*
     * We use snmp_add() to specify the transport
     * explicitly.
     */
    snmp_add(ss, transport, NULL, NULL);

    pcap_loop(p, -1, handle_pcap, (void *)&mystuff);

    pcap_close(p);

    SNMP_FREE(ss);

    return 0;
}
