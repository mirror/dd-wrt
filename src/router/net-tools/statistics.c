/*
 * Copyright 1997,1999,2000 Andi Kleen. Subject to the GPL.
 * $Id: statistics.c,v 1.23 2010-10-29 19:24:36 ecki Exp $
 * 19980630 - i18n - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 * 19981113 - i18n fixes - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 * 19990101 - added net/netstat, -t, -u, -w supprt - Bernd Eckenfels
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "intl.h"
#include "proc.h"

static int print_static,f_raw,f_tcp,f_udp,f_sctp,f_unknown = 1;

enum State {
    number = 0, opt_number, i_forward, i_inp_icmp, i_outp_icmp, i_rto_alg,
    MaxState
};

#define normal number

struct entry {
    const char *title;
    char *out;
    enum State type;
};

struct statedesc {
    int indent;
    const char *title;
};

static const struct statedesc states[] = {
    [number] = { 4, NULL },
    [opt_number] = { 4, NULL },
    [i_forward] = { 4, NULL },
    [i_inp_icmp] = { 8, N_("ICMP input histogram:") },
    [i_outp_icmp] = { 8, N_("ICMP output histogram:") },
    [MaxState] = {0},
};

static enum State state;

#define I_STATIC (1<<16)	/* static configuration option. */
#define I_TITLE  (1<<17)

/*
 * XXX check against the snmp mib rfc.
 *
 * Don't mark the first field as translatable! It's a snmp MIB standard.
 * - acme
 */
static const struct entry Iptab[] =
{   /* Keep the entries sorted! */
    {"DefaultTTL", N_("Default TTL is %llu"), number | I_STATIC},
    {"Forwarding", N_("Forwarding is %s"), i_forward | I_STATIC},
    {"ForwDatagrams", N_("%llu forwarded"), number},
    {"FragCreates", N_("%llu fragments created"), opt_number},
    {"FragFails", N_("%llu outgoing packets failed fragmentation"), opt_number},
    {"FragOKs", N_("%llu outgoing packets fragmented ok"), opt_number},
    {"InAddrErrors", N_("%llu with invalid addresses"), opt_number},
    {"InDelivers", N_("%llu incoming packets delivered"), number},
    {"InDiscards", N_("%llu incoming packets discarded"), number},
    {"InHdrErrors", N_("%llu with invalid headers"), opt_number},
    {"InReceives", N_("%llu total packets received"), number},
    {"InUnknownProtos", N_("%llu with unknown protocol"), opt_number},
    {"OutDiscards", N_("%llu outgoing packets dropped"), opt_number},
    {"OutNoRoutes", N_("%llu dropped because of missing route"), opt_number},
    {"OutRequests", N_("%llu requests sent out"), number},	/*? */
    {"ReasmFails", N_("%llu packet reassemblies failed"), opt_number},
    {"ReasmOKs", N_("%llu packets reassembled ok"), opt_number},
    {"ReasmReqds", N_("%llu reassemblies required"), opt_number},	/* ? */
    {"ReasmTimeout", N_("%llu fragments dropped after timeout"), opt_number},
};

static const struct entry Ip6tab[] =
{   /* Keep the entries sorted! */
    {"Ip6FragCreates", N_("%llu fragments created"), opt_number},
    {"Ip6FragFails", N_("%llu outgoing packets failed fragmentation"), opt_number},
    {"Ip6FragOKs", N_("%llu outgoing packets fragmented ok"), opt_number},
    {"Ip6InAddrErrors", N_("%llu with invalid addresses"), opt_number},
    {"Ip6InDelivers", N_("%llu incoming packets delivered"), number},
    {"Ip6InDiscards", N_("%llu incoming packets discarded"), number},
    {"Ip6InHdrErrors", N_("%llu with invalid headers"), opt_number},
    {"Ip6InMcastPkts", N_("%llu incoming multicast packets"), opt_number},
    {"Ip6InNoRoutes", N_("%llu incoming packets with no route"), opt_number},
    {"Ip6InReceives", N_("%llu total packets received"), number},
    {"Ip6InTooBigErrors", N_("%llu with packets too big"), opt_number},
    {"Ip6InTruncatedPkts", N_("%llu with truncated packets"), opt_number},
    {"Ip6InUnknownProtos", N_("%llu with unknown protocol"), opt_number},
    {"Ip6OutDiscards", N_("%llu outgoing packets dropped"), opt_number},
    {"Ip6OutForwDatagrams", N_("%llu forwarded"), number},
    {"Ip6OutMcastPkts", N_("%llu outgoing multicast packets"), opt_number},
    {"Ip6OutNoRoutes", N_("%llu dropped because of missing route"), opt_number},
    {"Ip6OutRequests", N_("%llu requests sent out"), number},     /*? */
    {"Ip6ReasmFails", N_("%llu packet reassemblies failed"), opt_number},
    {"Ip6ReasmOKs", N_("%llu packets reassembled ok"), opt_number},
    {"Ip6ReasmReqds", N_("%llu reassemblies required"), opt_number}, /* ? */
    {"Ip6ReasmTimeout", N_("%llu fragments dropped after timeout"), opt_number},
};

static const struct entry Icmptab[] =
{   /* Keep the entries sorted! */
    {"InAddrMaskReps", N_("address mask replies: %llu"), i_inp_icmp | I_TITLE},	/*? */
    {"InAddrMasks", N_("address mask request: %llu"), i_inp_icmp | I_TITLE},	/*? */
    {"InDestUnreachs", N_("destination unreachable: %llu"), i_inp_icmp | I_TITLE},
    {"InEchoReps", N_("echo replies: %llu"), i_inp_icmp | I_TITLE},
    {"InEchos", N_("echo requests: %llu"), i_inp_icmp | I_TITLE},
    {"InErrors", N_("%llu input ICMP message failed"), number},
    {"InMsgs", N_("%llu ICMP messages received"), number},
    {"InParmProbs", N_("wrong parameters: %llu"), i_inp_icmp | I_TITLE},	/*? */
    {"InRedirects", N_("redirects: %llu"), i_inp_icmp | I_TITLE},
    {"InSrcQuenchs", N_("source quenches: %llu"), i_inp_icmp | I_TITLE},
    {"InTimeExcds", N_("timeout in transit: %llu"), i_inp_icmp | I_TITLE},
    {"InTimestampReps", N_("timestamp reply: %llu"), i_inp_icmp | I_TITLE},
    {"InTimestamps", N_("timestamp request: %llu"), i_inp_icmp | I_TITLE},
    {"OutAddrMaskReps", N_("address mask replies: %llu"), i_outp_icmp | I_TITLE},
    {"OutAddrMasks", N_("address mask requests: %llu"), i_outp_icmp | I_TITLE},
    {"OutDestUnreachs", N_("destination unreachable: %llu"), i_outp_icmp | I_TITLE},
    {"OutEchoReps", N_("echo replies: %llu"), i_outp_icmp | I_TITLE},
    {"OutEchos", N_("echo requests: %llu"), i_outp_icmp | I_TITLE},
    {"OutErrors", N_("%llu ICMP messages failed"), number},
    {"OutMsgs", N_("%llu ICMP messages sent"), number},
    {"OutParmProbs", N_("wrong parameters: %llu"), i_outp_icmp | I_TITLE},	/*? */
    {"OutRedirects", N_("redirect: %llu"), i_outp_icmp | I_TITLE},
    {"OutSrcQuenchs", N_("source quench: %llu"), i_outp_icmp | I_TITLE},
    {"OutTimeExcds", N_("time exceeded: %llu"), i_outp_icmp | I_TITLE},
    {"OutTimestampReps", N_("timestamp replies: %llu"), i_outp_icmp | I_TITLE},
    {"OutTimestamps", N_("timestamp requests: %llu"), i_outp_icmp | I_TITLE},
};

static const struct entry Icmp6tab[] =
{   /* Keep the entries sorted! */
    {"Icmp6InDestUnreachs", N_("destination unreachable: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InEchoReplies", N_("echo replies: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InEchos", N_("echo requests: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InErrors", N_("%llu input ICMP message failed"), number},
    {"Icmp6InGroupMembQueries", N_("group member queries: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InGroupMembReductions", N_("group member reductions: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InGroupMembResponses", N_("group member responses: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InMsgs", N_("%llu ICMP messages received"), number},
    {"Icmp6InNeighborAdvertisements", N_("neighbour advertisement: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InNeighborSolicits", N_("neighbour solicits: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InParmProblems", N_("parameter problem: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InPktTooBigs", N_("packets too big: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InRedirects", N_("redirects: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InRouterAdvertisements", N_("router advertisement: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InRouterSolicits", N_("router solicits: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6InTimeExcds", N_("received ICMPv6 time exceeded: %llu"), i_inp_icmp | I_TITLE},
    {"Icmp6OutDestUnreachs", N_("destination unreachable: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutEchoReplies", N_("echo replies: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutEchos", N_("echo requests: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutGroupMembQueries", N_("group member queries: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutGroupMembReductions", N_("group member reductions: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutGroupMembResponses", N_("group member responses: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutMsgs", N_("%llu ICMP messages sent"), number},
    {"Icmp6OutNeighborAdvertisements", N_("neighbor advertisements: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutNeighborSolicits", N_("neighbor solicits: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutParmProblems", N_("parameter problem: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutPktTooBigs", N_("packets too big: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutRedirects", N_("redirects: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutRouterAdvertisements ", N_("router advertisement: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutRouterSolicits", N_("router solicits: %llu"), i_outp_icmp | I_TITLE},
    {"Icmp6OutTimeExcds", N_("sent ICMPv6 time exceeded: %llu"), i_outp_icmp | I_TITLE},
};

static const struct entry Tcptab[] =
{   /* Keep the entries sorted! */
    {"ActiveOpens", N_("%llu active connection openings"), number},
    {"AttemptFails", N_("%llu failed connection attempts"), number},
    {"CurrEstab", N_("%llu connections established"), number},
    {"EstabResets", N_("%llu connection resets received"), number},
    {"InErrs", N_("%llu bad segments received"), number},
    {"InSegs", N_("%llu segments received"), number},
    {"MaxConn", "", number},
    {"OutRsts", N_("%llu resets sent"), number},
    {"OutSegs", N_("%llu segments sent out"), number},
    {"PassiveOpens", N_("%llu passive connection openings"), number},
    {"RetransSegs", N_("%llu segments retransmitted"), number},
    {"RtoAlgorithm", N_("RTO algorithm is %s"), i_rto_alg | I_STATIC},
    {"RtoMax", "", number},
    {"RtoMin", "", number},
};

static const struct entry Udptab[] =
{   /* Keep the entries sorted! */
    {"InDatagrams", N_("%llu packets received"), number},
    {"InErrors", N_("%llu packet receive errors"), number},
    {"NoPorts", N_("%llu packets to unknown port received"), number},
    {"OutDatagrams", N_("%llu packets sent"), number},
    {"RcvbufErrors", N_("%llu receive buffer errors"), number},
    {"SndbufErrors", N_("%llu send buffer errors"), number},
 };

static const struct entry Udp6tab[] =
{   /* Keep the entries sorted! */
    {"Udp6InDatagrams", N_("%llu packets received"), number},
    {"Udp6InErrors", N_("%llu packet receive errors"), number},
    {"Udp6NoPorts", N_("%llu packets to unknown port received"), number},
    {"Udp6OutDatagrams", N_("%llu packets sent"), number},
};

static const struct entry Tcpexttab[] =
{   /* Keep the entries sorted! */
    {"DelayedACKLocked", N_("%llu delayed acks further delayed because of locked socket"), opt_number},
    {"DelayedACKLost", N_("Quick ack mode was activated %llu times"), opt_number},
    {"DelayedACKs", N_("%llu delayed acks sent"), opt_number},
    {"EmbryonicRsts", N_("%llu resets received for embryonic SYN_RECV sockets"), opt_number},
    {"ListenDrops", N_("%llu SYNs to LISTEN sockets dropped"), opt_number},
    {"ListenOverflows", N_("%llu times the listen queue of a socket overflowed"), opt_number},
    {"LockDroppedIcmps", N_("%llu ICMP packets dropped because socket was locked"), opt_number},
    {"OfoPruned", N_("%llu packets dropped from out-of-order queue because of socket buffer overrun"), opt_number},
    {"OutOfWindowIcmps", N_("%llu ICMP packets dropped because they were out-of-window"), opt_number},
    {"PAWSActive", N_("%llu active connections rejected because of time stamp"), opt_number},
    {"PAWSEstab", N_("%llu packets rejected in established connections because of timestamp"), opt_number},
    {"PAWSPassive", N_("%llu passive connections rejected because of time stamp"), opt_number},
    {"PruneCalled", N_("%llu packets pruned from receive queue because of socket buffer overrun"), opt_number},
    {"RcvPruned", N_("%llu packets pruned from receive queue"), opt_number},
    {"SockMallocOOM", N_("Ran %llu times out of system memory during packet sending"), opt_number},
    {"SyncookiesFailed", N_("%llu invalid SYN cookies received"), opt_number},
    {"SyncookiesRecv", N_("%llu SYN cookies received"), opt_number},
    {"SyncookiesSent", N_("%llu SYN cookies sent"), opt_number},
    {"TCPAbortFailed", N_("%llu times unable to send RST due to no memory"), opt_number},
    {"TCPAbortOnClose", N_("%llu connections reset due to early user close"), opt_number},
    {"TCPAbortOnData", N_("%llu connections reset due to unexpected data"), opt_number},
    {"TCPAbortOnLinger", N_("%llu connections aborted after user close in linger timeout"), opt_number},
    {"TCPAbortOnMemory", N_("%llu connections aborted due to memory pressure"), opt_number},
    {"TCPAbortOnSyn", N_("%llu connections reset due to unexpected SYN"), opt_number},
    {"TCPAbortOnTimeout", N_("%llu connections aborted due to timeout"), opt_number},
    {"TCPDirectCopyFromBacklog", N_("%llu bytes directly in process context from backlog"), opt_number},
    {"TCPDirectCopyFromPrequeue", N_("%llu bytes directly received in process context from prequeue"), opt_number},
    {"TCPDSACKOfoRecv", N_("%llu DSACKs for out of order packets received"), opt_number},
    {"TCPDSACKOfoSent", N_("%llu DSACKs sent for out of order packets"), opt_number},
    {"TCPDSACKOldSent", N_("%llu DSACKs sent for old packets"), opt_number},
    {"TCPDSACKRecv", N_("%llu DSACKs received"), opt_number},
    {"TCPDSackUndo", N_("%llu congestion window recovered without slow start using DSACK"), opt_number},
    {"TCPDSACKUndo", N_("%llu congestion windows recovered without slow start by DSACK"), opt_number},
    {"TCPFACKReorder", N_("Detected reordering %llu times using FACK"), opt_number},
    {"TCPFastRetrans", N_("%llu fast retransmits"), opt_number},
    {"TCPForwardRetrans", N_("%llu forward retransmits"), opt_number},
    {"TCPFullUndo", N_("%llu congestion windows fully recovered without slow start"), opt_number},
    {"TCPHPAcks", N_("%llu predicted acknowledgments"), opt_number},
    {"TCPHPHits", N_("%llu packet headers predicted"), number},
    {"TCPHPHitsToUser", N_("%llu packet headers predicted and directly queued to user"), opt_number},
    {"TCPLossFailures",  N_("%llu timeouts in loss state"), opt_number},
    {"TCPLoss", N_("%llu TCP data loss events"), opt_number},
    {"TCPLossUndo", N_("%llu congestion windows recovered without slow start after partial ack"), opt_number},
    {"TCPLostRetransmits", N_("%llu retransmits lost"), opt_number},
    {"TCPMemoryPressures", N_("TCP ran low on memory %llu times"), opt_number},
    {"TCPPartialUndo", N_("%llu congestion windows partially recovered using Hoe heuristic"), opt_number},
    {"TCPPrequeued", N_("%llu packets directly queued to recvmsg prequeue"), opt_number},
    {"TCPPrequeueDropped", N_("%llu packets dropped from prequeue"), opt_number},
    {"TCPPureAcks", N_("%llu acknowledgments not containing data payload received"), opt_number},
    {"TCPRcvCollapsed", N_("%llu packets collapsed in receive queue due to low socket buffer"), opt_number},
    {"TCPRenoFailures",  N_("%llu timeouts after reno fast retransmit"), opt_number},
    {"TCPRenoRecoveryFailed", N_("%llu reno fast retransmits failed"), opt_number},
    {"TCPRenoRecoveryFail", N_("%llu classic Reno fast retransmits failed"), opt_number},
    {"TCPRenoRecovery", N_("%llu times recovered from packet loss due to fast retransmit"), opt_number},
    {"TCPRenoReorder", N_("Detected reordering %llu times using reno fast retransmit"), opt_number},
    {"TCPSackFailures",  N_("%llu timeouts after SACK recovery"), opt_number},
    {"TCPSackRecoveryFail", N_("%llu SACK retransmits failed"), opt_number},
    {"TCPSackRecovery", N_("%llu times recovered from packet loss by selective acknowledgements"), opt_number},
    {"TCPSACKReneging", N_("%llu bad SACK blocks received"), opt_number},
    {"TCPSACKReorder", N_("Detected reordering %llu times using SACK"), opt_number},
    {"TCPSchedulerFailed", N_("%llu times receiver scheduled too late for direct processing"), opt_number},
    {"TCPSlowStartRetrans", N_("%llu retransmits in slow start"), opt_number},
    {"TCPTimeouts", N_("%llu other TCP timeouts"), opt_number},
    {"TCPTSReorder", N_("Detected reordering %llu times using time stamp"), opt_number},
    {"TWKilled", N_("%llu TCP sockets finished time wait in slow timer"), opt_number},
    {"TW", N_("%llu TCP sockets finished time wait in fast timer"), opt_number},
    {"TWRecycled", N_("%llu time wait sockets recycled by time stamp"), opt_number},
};

static const struct entry Sctptab[] =
{   /* Keep the entries sorted! */
    {"SctpAborteds", N_("%llu Number of Aborteds "), number},
    {"SctpActiveEstabs", N_("%llu Active Associations"), number},
    {"SctpChecksumErrors", N_("%llu Number of Packets with invalid Checksum"), number},
    {"SctpCurrEstab", N_("%llu Current Associations"), number},
    {"SctpFragUsrMsgs", N_("%llu Number of messages fragmented"), number},
    {"SctpInCtrlChunks", N_("%llu Number of control chunks received"), number},
    {"SctpInOrderChunks", N_("%llu Number of ordered chunks received"), number},
    {"SctpInSCTPPacks", N_("%llu Number of SCTP packets received"), number},
    {"SctpInUnorderChunks", N_("%llu Number of Unordered chunks received"), number},
    {"SctpOutCtrlChunks", N_("%llu Number of control chunks sent"), number},
    {"SctpOutOfBlues", N_("%llu Number of Out of Blue packets"), number},
    {"SctpOutOrderChunks", N_("%llu Number of ordered chunks sent"), number},
    {"SctpOutSCTPPacks", N_("%llu Number of SCTP packets sent"), number},
    {"SctpOutUnorderChunks", N_("%llu Number of Unordered chunks sent"), number},
    {"SctpPassiveEstabs", N_("%llu Passive Associations"), number},
    {"SctpReasmUsrMsgs", N_("%llu Number of messages reassembled "), number},
    {"SctpShutdowns", N_("%llu Number of Graceful Terminations"), number},
};

struct tabtab {
    const char *title;
    const struct entry *tab;
    size_t size;
    int *flag;
};

static const struct tabtab snmptabs[] =
{
    {"Ip", Iptab, sizeof(Iptab), &f_raw},
    {"Icmp", Icmptab, sizeof(Icmptab), &f_raw},
    {"Tcp", Tcptab, sizeof(Tcptab), &f_tcp},
    {"Udp", Udptab, sizeof(Udptab), &f_udp},
    {"Sctp", Sctptab, sizeof(Sctptab), &f_sctp},
    {"TcpExt", Tcpexttab, sizeof(Tcpexttab), &f_tcp},
    {NULL}
};

static const struct tabtab snmp6tabs[] =
{
    {"Ip6", Ip6tab, sizeof(Ip6tab), &f_raw},
    {"Icmp6", Icmp6tab, sizeof(Icmp6tab), &f_raw},
    {"Udp6", Udp6tab, sizeof(Udp6tab), &f_udp},
    {"Tcp6", Tcptab, sizeof(Tcptab), &f_tcp},
    {NULL}
};

/* XXX IGMP */

static int cmpentries(const void *a, const void *b)
{
    return strcmp(((struct entry *) a)->title, ((struct entry *) b)->title);
}

static void printval(const struct tabtab *tab, const char *title, unsigned long long val)
{
    struct entry *ent = NULL, key;
    int type;
    char buf[512];

    key.title = title;
	if (tab->tab)
	    ent = bsearch(&key, tab->tab, tab->size / sizeof(struct entry),
			  sizeof(struct entry), cmpentries);
    if (!ent) {			/* try our best */
	if (val)
		printf("%*s%s: %llu\n", states[state].indent, "", title, val);
	return;
    }
    type = ent->type;
    if (type & I_STATIC) {
	type &= ~I_STATIC;
	if (!print_static)
	    return;
    }
    if (*ent->out == '\0')
	return;

    if (type & I_TITLE) {
	type &= ~I_TITLE;
	if (state != type)
	    printf("%*s%s\n", states[state].indent, "", _(states[type].title));
    }
    buf[0] = '\0';
    switch (type) {
    case opt_number:
	if (val == 0)
	    break;
	/*FALL THOUGH*/
    case number:
	snprintf(buf, sizeof(buf), _(ent->out), val);
	break;
    case i_forward:
	type = normal;
	snprintf(buf, sizeof(buf), _(ent->out), val == 2 ? _("enabled") : _("disabled"));
	break;
    case i_outp_icmp:
    case i_inp_icmp:
	if (val > 0)
	    snprintf(buf, sizeof(buf), _(ent->out), val);
	break;
    case i_rto_alg:		/* XXXX */
	break;
    default:
	abort();
    }
    if (buf[0])
	printf("%*s%s\n", states[type].indent, "", buf);

    state = type;
}

static const struct tabtab *newtable(const struct tabtab *tabs, const char *title)
{
    const struct tabtab *t;
	static struct tabtab dummytab;

    for (t = tabs; t->title; t++) {
		if (!strcmp(title, t->title)) {
	    	if (*(t->flag))
				printf("%s:\n", _(title));
		    state = normal;
	   		return t;
		}
	}
	if (!f_unknown)
		return NULL;
	printf("%s:\n", _(title));
	dummytab.title = title;
	dummytab.flag = &f_unknown;
	return &dummytab;
}

static int process_fd(FILE *f, int all, const char *filter)
{
    size_t filter_len = filter ? strlen(filter) : 0;
    static char *buf1, *buf2;
    static size_t buf1_len, buf2_len;
    char *sp, *np, *p;

    while (getline(&buf1, &buf1_len, f) > 0) {
	int endflag;
	const struct tabtab *tab;

        if (buf1[0] == '\n') // skip empty first line in 2.6 kernels
            continue;

	if (getline(&buf2, &buf2_len, f) <= 0)
	    break;
	sp = strchr(buf1, ':');
	np = strchr(buf2, ':');
	if (!np || !sp)
	    goto formaterr;
	*sp = '\0';

        if (!all)
           if (strncmp(buf1, filter, filter_len))
               continue;

	tab = newtable(snmptabs, buf1);
	if (tab == NULL) {
		printf("unknown %s\n", buf1);
		continue;
	}
	np++;
	sp++;

	endflag = 0;
	while (!endflag) {
	    sp += strspn(sp, " \t\n");
	    np += strspn(np, " \t\n");
	    /*if (*np == '\0') goto formaterr; */

	    p = sp+strcspn(sp, " \t\n");
	    if (*p == '\0')
		endflag = 1;
	    *p = '\0';

	    if (*sp != '\0' && *(tab->flag))
		printval(tab, sp, strtoull(np, &np, 10));

	    sp = p + 1;
	}
    }
  return 0;

formaterr:
  return -1;
}

static void cpytitle(const char *original, char *new)
{
     const char *ptr = original;
     while(*ptr != '6' && *ptr != '\0') {
           *new = *ptr;
            new++;
            ptr++;
     }
    *new = *ptr;
    new++;
    *new = '\0';
}

static void process6_fd(FILE *f)
{
   char buf1[1024],buf2[50],buf3[1024];
   unsigned long long val;
   const struct tabtab *tab = NULL;
   int cpflg = 0;

   while (fgets(buf1, sizeof buf1, f)) {
          sscanf(buf1, "%s %llu", buf2, &val);
          if(!cpflg) {
             cpytitle(buf2, buf3);
             tab = newtable(snmp6tabs, buf3);
             cpflg = 1;
          }
          if(!strstr(buf2, buf3)) {
             cpytitle(buf2, buf3);
             tab = newtable(snmp6tabs, buf3);
          }
          if (*(tab->flag))
             printval(tab, buf2, val);
   }

}

/* Process a file with name-value lines (like /proc/net/sctp/snmp) */
static void process_fd2(FILE *f, const char *filename)
{
    char buf1[1024];
    char *sp;
    const struct tabtab *tab;

    tab = newtable(snmptabs, "Sctp");

    while (fgets(buf1, sizeof buf1, f)) {
	sp = buf1 + strcspn(buf1, " \t\n");
	if (!sp) {
	    fprintf(stderr, _("error parsing %s\n"), filename);
	    return;
	}
	*sp = '\0';
	sp++;

	sp += strspn(sp, " \t\n");

	if (*sp != '\0' && *(tab->flag))
	    printval(tab, buf1, strtoul(sp, 0, 10));
    }
}

void parsesnmp(int flag_raw, int flag_tcp, int flag_udp, int flag_sctp)
{
    FILE *f;

    f_raw = flag_raw; f_tcp = flag_tcp; f_udp = flag_udp; f_sctp = flag_sctp;

    f = proc_fopen("/proc/net/snmp");
    if (!f) {
	perror(_("cannot open /proc/net/snmp"));
	return;
    }

    if (process_fd(f, 1, NULL) < 0)
      fprintf(stderr, _("Problem while parsing /proc/net/snmp\n"));

    if (ferror(f))
	perror("/proc/net/snmp");

    fclose(f);

    f = proc_fopen("/proc/net/netstat");

    if (f) {
    	if (process_fd(f, 1, NULL) <0)
          fprintf(stderr, _("Problem while parsing /proc/net/netstat\n"));

        if (ferror(f))
	    perror("/proc/net/netstat");

        fclose(f);
    }

    f = proc_fopen("/proc/net/sctp/snmp");
    if (f) {
	process_fd2(f,"/proc/net/sctp/snmp");
	if (ferror(f)) {
	    perror("/proc/net/sctp/snmp");
	    fclose(f);
	}
    }
}

void parsesnmp6(int flag_raw, int flag_tcp, int flag_udp)
{
    FILE *f;

    f_raw = flag_raw; f_tcp = flag_tcp; f_udp = flag_udp;

    f = fopen("/proc/net/snmp6", "r");
    if (!f) {
        perror(_("cannot open /proc/net/snmp6"));
        return;
    }
    process6_fd(f);
    if (ferror(f))
        perror("/proc/net/snmp6");

    fclose(f);
    f = fopen("/proc/net/snmp", "r");
    if (!f) {
        perror(_("cannot open /proc/net/snmp"));
        return;
    }
    process_fd(f, 0, "Tcp");
    if (ferror(f))
        perror("/proc/net/snmp");

    fclose(f);
}
