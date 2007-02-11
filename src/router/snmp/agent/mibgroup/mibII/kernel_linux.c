/*
 *  Linux kernel interface
 *
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "util_funcs.h"

#if HAVE_STRING_H
#include <string.h>
#endif
#include <sys/types.h>
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#include "kernel_linux.h"

struct ip_mib   cached_ip_mib;
struct icmp_mib cached_icmp_mib;
struct tcp_mib  cached_tcp_mib;
struct udp_mib  cached_udp_mib;

#define IP_STATS_LINE	"Ip: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define ICMP_STATS_LINE	"Icmp: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define TCP_STATS_LINE	"Tcp: %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu"
#define UDP_STATS_LINE	"Udp: %lu %lu %lu %lu"

#define IP_STATS_PREFIX_LEN	4
#define ICMP_STATS_PREFIX_LEN	6
#define TCP_STATS_PREFIX_LEN	5
#define UDP_STATS_PREFIX_LEN	5


int
linux_read_mibII_stats(void)
{
    FILE           *in = fopen("/proc/net/snmp", "r");
    char            line[1024];

    if (!in) {
        return -1;
    }


    while (line == fgets(line, sizeof(line), in)) {
        if (!strncmp(line, IP_STATS_LINE, IP_STATS_PREFIX_LEN)) {
            sscanf(line, IP_STATS_LINE,
                   &cached_ip_mib.ipForwarding,
                   &cached_ip_mib.ipDefaultTTL,
                   &cached_ip_mib.ipInReceives,
                   &cached_ip_mib.ipInHdrErrors,
                   &cached_ip_mib.ipInAddrErrors,
                   &cached_ip_mib.ipForwDatagrams,
                   &cached_ip_mib.ipInUnknownProtos,
                   &cached_ip_mib.ipInDiscards,
                   &cached_ip_mib.ipInDelivers,
                   &cached_ip_mib.ipOutRequests,
                   &cached_ip_mib.ipOutDiscards,
                   &cached_ip_mib.ipOutNoRoutes,
                   &cached_ip_mib.ipReasmTimeout,
                   &cached_ip_mib.ipReasmReqds,
                   &cached_ip_mib.ipReasmOKs,
                   &cached_ip_mib.ipReasmFails,
                   &cached_ip_mib.ipFragOKs,
                   &cached_ip_mib.ipFragFails,
                   &cached_ip_mib.ipFragCreates);
            cached_ip_mib.ipRoutingDiscards = 0;        /* XXX */
        } else if (!strncmp(line, ICMP_STATS_LINE, ICMP_STATS_PREFIX_LEN)) {
            sscanf(line, ICMP_STATS_LINE,
                   &cached_icmp_mib.icmpInMsgs,
                   &cached_icmp_mib.icmpInErrors,
                   &cached_icmp_mib.icmpInDestUnreachs,
                   &cached_icmp_mib.icmpInTimeExcds,
                   &cached_icmp_mib.icmpInParmProbs,
                   &cached_icmp_mib.icmpInSrcQuenchs,
                   &cached_icmp_mib.icmpInRedirects,
                   &cached_icmp_mib.icmpInEchos,
                   &cached_icmp_mib.icmpInEchoReps,
                   &cached_icmp_mib.icmpInTimestamps,
                   &cached_icmp_mib.icmpInTimestampReps,
                   &cached_icmp_mib.icmpInAddrMasks,
                   &cached_icmp_mib.icmpInAddrMaskReps,
                   &cached_icmp_mib.icmpOutMsgs,
                   &cached_icmp_mib.icmpOutErrors,
                   &cached_icmp_mib.icmpOutDestUnreachs,
                   &cached_icmp_mib.icmpOutTimeExcds,
                   &cached_icmp_mib.icmpOutParmProbs,
                   &cached_icmp_mib.icmpOutSrcQuenchs,
                   &cached_icmp_mib.icmpOutRedirects,
                   &cached_icmp_mib.icmpOutEchos,
                   &cached_icmp_mib.icmpOutEchoReps,
                   &cached_icmp_mib.icmpOutTimestamps,
                   &cached_icmp_mib.icmpOutTimestampReps,
                   &cached_icmp_mib.icmpOutAddrMasks,
                   &cached_icmp_mib.icmpOutAddrMaskReps);
        } else if (!strncmp(line, TCP_STATS_LINE, TCP_STATS_PREFIX_LEN)) {
            int             ret = sscanf(line, TCP_STATS_LINE,
                                         &cached_tcp_mib.tcpRtoAlgorithm,
                                         &cached_tcp_mib.tcpRtoMin,
                                         &cached_tcp_mib.tcpRtoMax,
                                         &cached_tcp_mib.tcpMaxConn,
                                         &cached_tcp_mib.tcpActiveOpens,
                                         &cached_tcp_mib.tcpPassiveOpens,
                                         &cached_tcp_mib.tcpAttemptFails,
                                         &cached_tcp_mib.tcpEstabResets,
                                         &cached_tcp_mib.tcpCurrEstab,
                                         &cached_tcp_mib.tcpInSegs,
                                         &cached_tcp_mib.tcpOutSegs,
                                         &cached_tcp_mib.tcpRetransSegs,
                                         &cached_tcp_mib.tcpInErrs,
                                         &cached_tcp_mib.tcpOutRsts);
            cached_tcp_mib.tcpInErrsValid = (ret > 12) ? 1 : 0;
            cached_tcp_mib.tcpOutRstsValid = (ret > 13) ? 1 : 0;
        } else if (!strncmp(line, UDP_STATS_LINE, UDP_STATS_PREFIX_LEN)) {
            sscanf(line, UDP_STATS_LINE,
                   &cached_udp_mib.udpInDatagrams,
                   &cached_udp_mib.udpNoPorts,
                   &cached_udp_mib.udpInErrors,
                   &cached_udp_mib.udpOutDatagrams);
        }
    }
    fclose(in);

    /*
     * Tweak illegal values:
     *
     * valid values for ipForwarding are 1 == yup, 2 == nope
     * a 0 is forbidden, so patch:
     */
    if (!cached_ip_mib.ipForwarding)
        cached_ip_mib.ipForwarding = 2;

    /*
     * 0 is illegal for tcpRtoAlgorithm
     * so assume `other' algorithm:
     */
    if (!cached_tcp_mib.tcpRtoAlgorithm)
        cached_tcp_mib.tcpRtoAlgorithm = 1;
    return 0;
}

int
linux_read_ip_stat(struct ip_mib *ipstat)
{
    memset((char *) ipstat, (0), sizeof(*ipstat));
    if (linux_read_mibII_stats() == -1)
        return -1;
    memcpy((char *) ipstat, (char *) &cached_ip_mib, sizeof(*ipstat));
    return 0;
}

int
linux_read_icmp_stat(struct icmp_mib *icmpstat)
{
    memset((char *) icmpstat, (0), sizeof(*icmpstat));
    if (linux_read_mibII_stats() == -1)
        return -1;
    memcpy((char *) icmpstat, (char *) &cached_icmp_mib,
           sizeof(*icmpstat));
    return 0;
}

int
linux_read_tcp_stat(struct tcp_mib *tcpstat)
{
    memset((char *) tcpstat, (0), sizeof(*tcpstat));
    if (linux_read_mibII_stats() == -1)
        return -1;
    memcpy((char *) tcpstat, (char *) &cached_tcp_mib, sizeof(*tcpstat));
    return 0;
}

int
linux_read_udp_stat(struct udp_mib *udpstat)
{
    memset((char *) udpstat, (0), sizeof(*udpstat));
    if (linux_read_mibII_stats() == -1)
        return -1;
    memcpy((char *) udpstat, (char *) &cached_udp_mib, sizeof(*udpstat));
    return 0;
}
