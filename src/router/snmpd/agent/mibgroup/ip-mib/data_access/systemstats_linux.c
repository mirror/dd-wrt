/*
 *  Interface MIB architecture support
 *
 * $Id: systemstats_linux.c,v 1.12 2006/09/15 00:48:40 tanders Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ipstats.h>
#include <net-snmp/data_access/systemstats.h>

static int _systemstats_v4(netsnmp_container* container, u_int load_flags);
#if defined (NETSNMP_ENABLE_IPV6)
static int _systemstats_v6(netsnmp_container* container, u_int load_flags);
#endif


void
netsnmp_access_systemstats_arch_init(void)
{
    /*
     * nothing to do
     */
}

/*
  /proc/net/snmp

  Ip: Forwarding DefaultTTL InReceives InHdrErrors InAddrErrors ForwDatagrams InUnknownProtos InDiscards InDelivers OutRequests OutDiscards OutNoRoutes ReasmTimeout ReasmReqds ReasmOKs ReasmFails FragOKs FragFails FragCreates
  Ip: 2 64 7083534 0 0 0 0 0 6860233 6548963 0 0 1 286623 63322 1 259920 0 0
  
  Icmp: InMsgs InErrors InDestUnreachs InTimeExcds InParmProbs InSrcQuenchs InRedirects InEchos InEchoReps InTimestamps InTimestampReps InAddrMasks InAddrMaskReps OutMsgs OutErrors OutDestUnreachs OutTimeExcds OutParmProbs OutSrcQuenchs OutRedirects OutEchos OutEchoReps OutTimestamps OutTimestampReps OutAddrMasks OutAddrMaskReps
  Icmp: 335 36 254 72 0 0 0 0 9 0 0 0 0 257 0 257 0 0 0 0 0 0 0 0 0 0
  
  Tcp: RtoAlgorithm RtoMin RtoMax MaxConn ActiveOpens PassiveOpens AttemptFails EstabResets CurrEstab InSegs OutSegs RetransSegs InErrs OutRsts
  Tcp: 1 200 120000 -1 5985 55 27 434 10 5365077 5098096 10902 2 4413
  
  Udp: InDatagrams NoPorts InErrors OutDatagrams
  Udp: 1491094 122 0 1466178
*/


/*
 *
 * @retval  0 success
 * @retval -1 no container specified
 * @retval -2 could not open file
 * @retval -3 could not create entry (probably malloc)
 * @retval -4 file format error
 */
int
netsnmp_access_systemstats_container_arch_load(netsnmp_container* container,
                                             u_int load_flags)
{
    int rc1;
#if defined (NETSNMP_ENABLE_IPV6)
    int rc2;
#endif

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_systemstats_\n");
        return -1;
    }

    /*
     * load v4 and v6 stats. Even if one fails, try the other.
     * If they have the same rc, return it. if the differ, return
     * the smaller one. No log messages, since each individual function
     * would have logged its own message.
     */
    rc1 = _systemstats_v4(container, load_flags);
#if defined (NETSNMP_ENABLE_IPV6)
    rc2 = _systemstats_v6(container, load_flags);
    if ((rc1 == rc2) || (rc1 < rc2))
        return rc1;
        
    return rc2;
#else
    return rc1;
#endif
}

static int
_systemstats_v4(netsnmp_container* container, u_int load_flags)
{
    FILE           *devin;
    char            line[1024];
    netsnmp_systemstats_entry *entry = NULL;
    int             scan_count;
    char           *stats, *start = line;
    int             len;
    uintmax_t       scan_vals[19];

    DEBUGMSGTL(("access:systemstats:container:arch", "load v4 (flags %p)\n",
                load_flags));

    netsnmp_assert(container != NULL); /* load function shoulda checked this */

    if (!(devin = fopen("/proc/net/snmp", "r"))) {
        DEBUGMSGTL(("access:systemstats",
                    "Failed to load Systemstats Table (linux1)\n"));
        snmp_log(LOG_ERR, "cannot open /proc/net/snmp ...\n");
        return -2;
    }

    /*
     * skip header, but make sure it's the length we expect...
     */
    fgets(line, sizeof(line), devin);
    len = strlen(line);
    if (224 != len) {
        fclose(devin);
        snmp_log(LOG_ERR, "unexpected header length in /proc/net/snmp."
                 " %d != 224\n", len);
        return -4;
    }

    /*
     * This file provides the statistics for each systemstats.
     * Read in each line in turn, isolate the systemstats name
     *   and retrieve (or create) the corresponding data structure.
     */
    start = fgets(line, sizeof(line), devin);
    fclose(devin);
    if (start) {

        len = strlen(line);
        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        while (*start && *start == ' ')
            start++;

        if ((!*start) || ((stats = strrchr(start, ':')) == NULL)) {
            snmp_log(LOG_ERR,
                     "systemstats data format error 1, line ==|%s|\n", line);
            return -4;
        }

        DEBUGMSGTL(("access:systemstats", "processing '%s'\n", start));

        *stats++ = 0; /* null terminate name */
        while (*stats == ' ') /* skip spaces before stats */
            stats++;

        entry = netsnmp_access_systemstats_entry_create(1);
        if(NULL == entry) {
            netsnmp_access_systemstats_container_free(container,
                                                      NETSNMP_ACCESS_SYSTEMSTATS_FREE_NOFLAGS);
            return -3;
        }

        /*
         * OK - we've now got (or created) the data structure for
         *      this systemstats, including any "static" information.
         * Now parse the rest of the line (i.e. starting from 'stats')
         *      to extract the relevant statistics, and populate
         *      data structure accordingly.
         */

        memset(scan_vals, 0x0, sizeof(scan_vals));
        scan_count = sscanf(stats,
                            "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu"
                            "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
                            &scan_vals[0],&scan_vals[1],&scan_vals[2],
                            &scan_vals[3],&scan_vals[4],&scan_vals[5],
                            &scan_vals[6],&scan_vals[7],&scan_vals[8],
                            &scan_vals[9],&scan_vals[10],&scan_vals[11],
                            &scan_vals[12],&scan_vals[13],&scan_vals[14],
                            &scan_vals[15],&scan_vals[16],&scan_vals[17],
                            &scan_vals[18]);
        DEBUGMSGTL(("access:systemstats", "  read %d values\n", scan_count));

        if(scan_count != 19) {
            snmp_log(LOG_ERR,
                     "error scanning systemstats data (expected %d, got %d)\n",
                     19, scan_count);
            netsnmp_access_systemstats_entry_free(entry);
            return -4;
        }
        /* entry->stats. = scan_vals[0]; / * Forwarding */
        /* entry->stats. = scan_vals[1]; / * DefaultTTL */
        entry->stats.HCInReceives.low = scan_vals[2] & 0xffffffff;
        entry->stats.HCInReceives.high = scan_vals[2] >> 32;
        entry->stats.InHdrErrors = scan_vals[3];
        entry->stats.InAddrErrors = scan_vals[4];
        entry->stats.HCInForwDatagrams.low = scan_vals[5] & 0xffffffff;
        entry->stats.HCInForwDatagrams.high = scan_vals[5] >> 32;
        entry->stats.InUnknownProtos = scan_vals[6];
        entry->stats.InDiscards = scan_vals[7];
        entry->stats.HCInDelivers.low = scan_vals[8] & 0xffffffff;
        entry->stats.HCInDelivers.high = scan_vals[8] >> 32;
        entry->stats.HCOutRequests.low = scan_vals[9] & 0xffffffff;
        entry->stats.HCOutRequests.high = scan_vals[9] >> 32;
        entry->stats.OutDiscards = scan_vals[10];
        entry->stats.OutNoRoutes = scan_vals[11];
        /* entry->stats. = scan_vals[12]; / * ReasmTimeout */
        entry->stats.ReasmReqds = scan_vals[13];
        entry->stats.ReasmOKs = scan_vals[14];
        entry->stats.ReasmFails = scan_vals[15];
        entry->stats.OutFragOKs = scan_vals[16];
        entry->stats.OutFragFails = scan_vals[17];
        entry->stats.OutFragCreates = scan_vals[18];

        /*
         * add to container
         */
        CONTAINER_INSERT(container, entry);
    }
    return 0;
}
    
#if defined (NETSNMP_ENABLE_IPV6)
static int
_systemstats_v6(netsnmp_container* container, u_int load_flags)
{
    FILE           *devin;
    char            line[1024];
    netsnmp_systemstats_entry *entry = NULL;
    int             scan_count = 0;
    char           *stats, *start = line;
    int             len, rc;
    uintmax_t       scan_val;
    const char     *filename = "/proc/net/snmp6";
    static int      warned_open = 0;

    DEBUGMSGTL(("access:systemstats:container:arch", "load v6 (flags %p)\n",
                load_flags));

    netsnmp_assert(container != NULL); /* load function shoulda checked this */

    entry = netsnmp_access_systemstats_entry_create(2);
    if(NULL == entry)
        return -3;
    
    /*
     * try to open file. If we can't, that's ok - maybe the module hasn't
     * been loaded yet.
     */
    if (!(devin = fopen(filename, "r"))) {
        DEBUGMSGTL(("access:systemstats",
                    "Failed to load Systemstats Table (linux1)\n"));
        if(!warned_open) {
            ++warned_open;
            snmp_log(LOG_ERR, "cannot open %s ...\n", filename);
        }
        free(entry);
        return 0;
    }

    /*
     * This file provides the statistics for each systemstats.
     * Read in each line in turn, isolate the systemstats name
     *   and retrieve (or create) the corresponding data structure.
     */
    while (1) {
        start = fgets(line, sizeof(line), devin);
        if (NULL == start)
            break;

        len = strlen(line);
        if (line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (('I' != line[0]) || ('6' != line[2]))
            continue;

        stats = strrchr(line, ' ');
        if (NULL == stats) {
            snmp_log(LOG_ERR,
                     "systemstats data format error 1, line ==|%s|\n", line);
            continue;
        }

        DEBUGMSGTL(("access:systemstats", "processing '%s'\n", line));

        /*
         * OK - we've now got (or created) the data structure for
         *      this systemstats, including any "static" information.
         * Now parse the rest of the line (i.e. starting from 'stats')
         *      to extract the relevant statistics, and populate
         *      data structure accordingly.
         */
        scan_val = atoll(stats);
        if (0 == scan_val)
            continue;

        rc = 0;
        if ('I' == line[3]) { /* In */
            if ('A' == line[5]) {
                entry->stats.InAddrErrors = scan_val;
            } else if ('D' == line[5]) {
                if ('e' == line[6]) {
                    entry->stats.HCInDelivers.low = scan_val  & 0xffffffff;
                    entry->stats.HCInDelivers.high = scan_val >> 32;
                } else if ('i' == line[6])
                    entry->stats.InDiscards = scan_val;
                else
                    rc = 1;
            } else if ('H' == line[5]) {
                entry->stats.InHdrErrors = scan_val;
            } else if ('M' == line[5]) {
                entry->stats.HCInMcastPkts.low = scan_val  & 0xffffffff;
                entry->stats.HCInMcastPkts.high = scan_val >> 32;
            } else if ('N' == line[5]) {
                entry->stats.InNoRoutes = scan_val;
            } else if ('R' == line[5]) {
                entry->stats.HCInReceives.low = scan_val & 0xffffffff;
                entry->stats.HCInReceives.high = scan_val >> 32;
            } else if ('T' == line[5]) {
                if ('r' == line[6]) {
                    entry->stats.InTruncatedPkts = scan_val  & 0xffffffff;
                } else if ('o' == line[6])
                    ; /* TooBig isn't in the MIB, so ignore it */
                else
                    rc = 1;
            } else if ('U' == line[5]) {
                entry->stats.InUnknownProtos = scan_val;
            } else
                rc = 1;
        } else if ('O' == line[3]) { /* Out */
            if ('D' == line[6]) {
                entry->stats.OutDiscards = scan_val;
            } else if ('F' == line[6]) {
                entry->stats.HCOutForwDatagrams.low = scan_val & 0xffffffff;
                entry->stats.HCOutForwDatagrams.high = scan_val >> 32;
            } else if ('M' == line[6]) {
                entry->stats.HCOutMcastPkts.low = scan_val & 0xffffffff;
                entry->stats.HCOutMcastPkts.high = scan_val >> 32;
            } else if ('N' == line[6]) {
                entry->stats.OutNoRoutes = scan_val;
            } else if ('R' == line[6]) {
                entry->stats.HCOutRequests.low = scan_val & 0xffffffff;
                entry->stats.HCOutRequests.high = scan_val >> 32;
            } else
                rc = 1;
        } else if ('R' == line[3]) { /* Reasm */
            if ('F' == line[8]) {
                entry->stats.ReasmFails = scan_val;
            } else if ('O' == line[8]) {
                entry->stats.ReasmOKs = scan_val;
            } else if ('R' == line[8]) {
                entry->stats.ReasmReqds = scan_val;
            } else if ('T' == line[8]) {
                ; /* no mib entry for reasm timeout */
            } else
                rc = 1;
        } else if ('F' == line[3]) { /* Frag */
            if ('C' == line[7])
                entry->stats.OutFragCreates = scan_val;
            else if ('O' == line[7])
                entry->stats.OutFragOKs = scan_val;
            else if ('F' == line[7])
                entry->stats.OutFragFails = scan_val;
            else
                rc = 1;
        } else
            rc = 1;
        
        if (rc)
            DEBUGMSGTL(("access:systemstats", "unknown stat %s\n", line));
        else
            ++scan_count;
    }

    fclose(devin);

    /*
     * add to container
     */
    CONTAINER_INSERT(container, entry);

    return rc;
}
#endif /* NETSNMP_ENABLE_IPV6 */
