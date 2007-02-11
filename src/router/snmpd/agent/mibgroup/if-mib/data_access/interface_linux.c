/*
 *  Interface MIB architecture support
 *
 * $Id: interface_linux.c,v 1.38 2006/09/21 13:09:18 tanders Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "mibII/mibII_common.h"
#include "if-mib/ifTable/ifTable_constants.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#else
#error "linux should have sys/ioctl header"
#endif

#include <net-snmp/data_access/interface.h>
#include <net-snmp/data_access/ipaddress.h>
#include "if-mib/data_access/interface.h"
#include "interface_ioctl.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_LINUX_ETHTOOL_H
typedef unsigned long long u64;         /* hack, so we may include kernel's ethtool.h */
typedef __uint32_t u32;         /* ditto */
typedef __uint16_t u16;         /* ditto */
typedef __uint8_t u8;           /* ditto */
#include <linux/ethtool.h>
#endif /* HAVE_LINUX_ETHTOOL_H */
#include <linux/sockios.h>

unsigned int
netsnmp_linux_interface_get_if_speed(int fd, const char *name);
#ifdef HAVE_LINUX_ETHTOOL_H
unsigned int
netsnmp_linux_interface_get_if_speed_mii(int fd, const char *name);
#endif

#define PROC_SYS_NET_IPVx_NEIGH_RETRANS_TIME_MS "/proc/sys/net/ipv%d/neigh/%s/retrans_time_ms"
#define PROC_SYS_NET_IPVx_NEIGH_RETRANS_TIME    "/proc/sys/net/ipv%d/neigh/%s/retrans_time"
static const char *proc_sys_retrans_time;
static unsigned short retrans_time_factor = 1;


#define PROC_SYS_NET_IPVx_BASE_REACHABLE_TIME_MS "/proc/sys/net/ipv%d/neigh/%s/base_reachable_time_ms"
#define PROC_SYS_NET_IPVx_BASE_REACHABLE_TIME "/proc/sys/net/ipv%d/neigh/%s/base_reachable_time"
static const char *proc_sys_basereachable_time;
static unsigned short basereachable_time_ms = 0;
void
netsnmp_arch_interface_init(void)
{
    /*
     * Check which retransmit time interface is available
     */
    char proc_path[ 64+IF_NAMESIZE];
    char proc_path2[64+IF_NAMESIZE];
    struct stat st;

    snprintf(proc_path,  sizeof(proc_path),
             PROC_SYS_NET_IPVx_NEIGH_RETRANS_TIME_MS, 6, "default");
    snprintf(proc_path2, sizeof(proc_path2),
             PROC_SYS_NET_IPVx_NEIGH_RETRANS_TIME_MS, 4, "default");

    if ((stat(proc_path, &st) == 0) || (stat(proc_path2, &st) == 0)) {
        proc_sys_retrans_time = PROC_SYS_NET_IPVx_NEIGH_RETRANS_TIME_MS;
    } else {
        proc_sys_retrans_time = PROC_SYS_NET_IPVx_NEIGH_RETRANS_TIME;
        retrans_time_factor = 10;
    }

    snprintf(proc_path,  sizeof(proc_path),  PROC_SYS_NET_IPVx_BASE_REACHABLE_TIME_MS, 6, "default");
    snprintf(proc_path2,  sizeof(proc_path),  PROC_SYS_NET_IPVx_BASE_REACHABLE_TIME, 4, "default");

    if ((stat(proc_path, &st) == 0) || (stat(proc_path2, &st) == 0)) {
        proc_sys_basereachable_time = PROC_SYS_NET_IPVx_BASE_REACHABLE_TIME_MS;
        basereachable_time_ms = 1;
    }
    else {
        proc_sys_basereachable_time = PROC_SYS_NET_IPVx_BASE_REACHABLE_TIME;
    }
}

/*
 * find the ifIndex for an interface name
 * NOTE: The Linux version is not efficient for large numbers of calls.
 *   consider using netsnmp_access_interface_ioctl_ifindex_get()
 *   for loops which need to look up a lot of indexes.
 *
 * @retval 0 : no index found
 * @retval >0: ifIndex for interface
 */
oid
netsnmp_arch_interface_index_find(const char *name)
{
    return netsnmp_access_interface_ioctl_ifindex_get(-1, name);
}


/*
 * check for ipv6 addresses
 */
void
_arch_interface_has_ipv6(oid if_index, u_int *flags,
                         netsnmp_container *addr_container)
{
#ifdef NETSNMP_ENABLE_IPV6
    netsnmp_ipaddress_entry *addr_entry = NULL;
    netsnmp_iterator        *addr_it = NULL;
    u_int                    addr_container_flags = 0; /* must init to 0 */
#endif

    if (NULL == flags)
        return;

    *flags &= ~NETSNMP_INTERFACE_FLAGS_HAS_IPV6;

#ifdef NETSNMP_ENABLE_IPV6
    /*
     * get ipv6 addresses
     */
    if (NULL == addr_container) {
        /*
         * we only care about ipv6, if we need to allocate our own
         * temporary container. set the flags (which we also use later
         * to determine if we need to free the container).
         */
        addr_container_flags = NETSNMP_ACCESS_IPADDRESS_LOAD_IPV6_ONLY;
        addr_container =
            netsnmp_access_ipaddress_container_load(NULL,
                                                    addr_container_flags);
        if (NULL == addr_container) {
            DEBUGMSGTL(("access:ifcontainer",
                        "couldn't get ip addresses container\n"));
            return;
        }
    }
    else {
        /*
         * addr_container flags must be 0, so we don't release the
         * user's container.
         */
        netsnmp_assert(0 == addr_container_flags);
    }


    /*
     * get an ipaddress container iterator, and look for ipv6 addrs
     */   
    addr_it = CONTAINER_ITERATOR(addr_container);
    if (NULL == addr_it) {
        DEBUGMSGTL(("access:ifcontainer",
                    "couldn't get ip addresses iterator\n"));
        if (0!=addr_container_flags)
            netsnmp_access_ipaddress_container_free(addr_container, 0);
        return;
    }

    addr_entry = ITERATOR_FIRST(addr_it);
    for( ; addr_entry ; addr_entry = ITERATOR_NEXT(addr_it) ) {
        /*
         * skip non matching indexes and ipv4 addresses
         */
        if ((if_index != addr_entry->if_index) ||
            (4 == addr_entry->ia_address_len))
            continue;

        /*
         * found one! no need to keep looking, set the flag and bail
         */
        *flags |= NETSNMP_INTERFACE_FLAGS_HAS_IPV6;
        break;
    }

    /*
     * make mama proud and clean up after ourselves
     */
    ITERATOR_RELEASE(addr_it);
    if (0!=addr_container_flags)
        netsnmp_access_ipaddress_container_free(addr_container, 0);    
#endif
}

/**
 * @internal
 */
static void
_arch_interface_flags_v4_get(netsnmp_interface_entry *entry)
{
    FILE           *fin;
    char            line[256];

    /*
     * get the retransmit time
     */
    snprintf(line,sizeof(line), proc_sys_retrans_time, 4,
             entry->name);
    if (!(fin = fopen(line, "r"))) {
        DEBUGMSGTL(("access:interface",
                    "Failed to open %s\n", line));
    }
    else {
        if (fgets(line, sizeof(line), fin)) {
            entry->retransmit_v4 = atoi(line) * retrans_time_factor;
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_V4_RETRANSMIT;
        }
        fclose(fin);
    }
}


#ifdef NETSNMP_ENABLE_IPV6
/**
 * @internal
 */
static void
_arch_interface_flags_v6_get(netsnmp_interface_entry *entry)
{
    FILE           *fin;
    char            line[256];

    /*
     * get the retransmit time
     */
    snprintf(line,sizeof(line), proc_sys_retrans_time, 6,
             entry->name);
    if (!(fin = fopen(line, "r"))) {
        DEBUGMSGTL(("access:interface",
                    "Failed to open %s\n", line));
    }
    else {
        if (fgets(line, sizeof(line), fin)) {
            entry->retransmit_v6 = atoi(line) * retrans_time_factor;
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_V6_RETRANSMIT;
        }
        fclose(fin);
    }

    /*
     * get the forwarding status
     */
    snprintf(line, sizeof(line), "/proc/sys/net/ipv6/conf/%s/forwarding",
             entry->name);
    if (!(fin = fopen(line, "r"))) {
        DEBUGMSGTL(("access:interface",
                    "Failed to open %s\n", line));
    }
    else {
        if (fgets(line, sizeof(line), fin)) {
            entry->forwarding_v6 = atoi(line);
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_V6_FORWARDING;
        }
        fclose(fin);
    }

    /*
     * get the reachable time
     */
    snprintf(line, sizeof(line), proc_sys_basereachable_time, 6, entry->name);
    if (!(fin = fopen(line, "r"))) {
        DEBUGMSGTL(("access:interface",
                    "Failed to open %s\n", line));
    }
    else {
        if (fgets(line, sizeof(line), fin)) {
            if (basereachable_time_ms) {
                entry->reachable_time = atoi(line); /* millisec */
            } else {
                entry->reachable_time = atoi(line)*1000; /* sec to  millisec */
            }

            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_V6_REACHABLE;
        }
        fclose(fin);
    }
}
#endif /* NETSNMP_ENABLE_IPV6 */

/**
 * @internal
 */
static int
_parse_stats(netsnmp_interface_entry *entry, char *stats, int expected)
{
    /*
     * scanline_2_2:
     *  [               IN                        ]
     *   byte pkts errs drop fifo frame cmprs mcst |
     *  [               OUT                               ]
     *   byte pkts errs drop fifo colls carrier compressed
     */
#ifdef SCNuMAX
    uintmax_t       rec_pkt, rec_oct, rec_err, rec_drop, rec_mcast;
    uintmax_t       snd_pkt, snd_oct, snd_err, snd_drop, coll;
    const char     *scan_line_2_2 =
        "%"   SCNuMAX " %"  SCNuMAX " %"  SCNuMAX " %"  SCNuMAX
        " %*" SCNuMAX " %*" SCNuMAX " %*" SCNuMAX " %"  SCNuMAX
        " %"  SCNuMAX " %"  SCNuMAX " %"  SCNuMAX " %"  SCNuMAX
        " %*" SCNuMAX " %"  SCNuMAX;
    const char     *scan_line_2_0 =
        "%"   SCNuMAX " %"  SCNuMAX " %*" SCNuMAX " %*" SCNuMAX
        " %*" SCNuMAX " %"  SCNuMAX " %"  SCNuMAX " %*" SCNuMAX
        " %*" SCNuMAX " %"  SCNuMAX;
#else
    unsigned long   rec_pkt, rec_oct, rec_err, rec_drop, rec_mcast;
    unsigned long   snd_pkt, snd_oct, snd_err, snd_drop, coll;
    const char     *scan_line_2_2 =
        "%lu %lu %lu %lu %*lu %*lu %*lu %lu %lu %lu %lu %lu %*lu %lu";
    const char     *scan_line_2_0 =
        "%lu %lu %*lu %*lu %*lu %lu %lu %*lu %*lu %lu";
#endif
    static const char     *scan_line_to_use = NULL;
    int             scan_count;

    if (10 == expected)
        scan_line_to_use = scan_line_2_2;
    else {
        netsnmp_assert(5 == expected);
        scan_line_to_use = scan_line_2_0;
    }

    while (*stats == ' ')
        stats++;

    /*
     * Now parse the rest of the line (i.e. starting from 'stats')
     *      to extract the relevant statistics, and populate
     *      data structure accordingly.
     * Use the entry flags field to indicate which counters are valid
     */
    rec_pkt = rec_oct = rec_err = rec_drop = rec_mcast = 0;
    snd_pkt = snd_oct = snd_err = snd_drop = coll = 0;
    if (scan_line_to_use == scan_line_2_2) {
        scan_count = sscanf(stats, scan_line_to_use,
                            &rec_oct, &rec_pkt, &rec_err, &rec_drop, &rec_mcast,
                            &snd_oct, &snd_pkt, &snd_err, &snd_drop,
                            &coll);
        if (scan_count == expected) {
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_BYTES;
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_DROPS;
            /*
             *  2.4 kernel includes a single multicast (input) counter?
             */
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_MCAST_PKTS;
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_HIGH_SPEED;
#ifdef SCNuMAX   /* XXX - should be flag for 64-bit variables */
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_HIGH_BYTES;
            entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_HIGH_PACKETS;
#endif
        }
    } else {
        scan_count = sscanf(stats, scan_line_to_use,
                            &rec_pkt, &rec_err,
                            &snd_pkt, &snd_err, &coll);
        if (scan_count == expected) {
            entry->ns_flags &= ~NETSNMP_INTERFACE_FLAGS_HAS_MCAST_PKTS;
            rec_oct = rec_drop = 0;
            snd_oct = snd_drop = 0;
        }
    }
    if(scan_count != expected) {
        snmp_log(LOG_ERR,
                 "error scanning interface data (expected %d, got %d)\n",
                 expected, scan_count);
        netsnmp_access_interface_entry_free(entry);
        return scan_count;
    }
    entry->ns_flags |= NETSNMP_INTERFACE_FLAGS_ACTIVE;
    
    /*
     * linux previous to 1.3.~13 may miss transmitted loopback pkts: 
     */
    if (!strcmp(entry->name, "lo") && rec_pkt > 0 && !snd_pkt)
        snd_pkt = rec_pkt;
    
    /*
     * subtract out multicast packets from rec_pkt before
     * we store it as unicast counter.
     */
    rec_pkt -= rec_mcast;

    entry->stats.ibytes.low = rec_oct & 0xffffffff;
    entry->stats.iucast.low = rec_pkt & 0xffffffff;
    entry->stats.imcast.low = rec_mcast & 0xffffffff;
    entry->stats.obytes.low = snd_oct & 0xffffffff;
    entry->stats.oucast.low = snd_pkt & 0xffffffff;
#ifdef SCNuMAX   /* XXX - should be flag for 64-bit variables */
    entry->stats.ibytes.high = rec_oct >> 32;
    entry->stats.iucast.high = rec_pkt >> 32;
    entry->stats.imcast.high = rec_mcast >> 32;
    entry->stats.obytes.high = snd_oct >> 32;
    entry->stats.oucast.high = snd_pkt >> 32;
#endif
    entry->stats.ierrors   = rec_err;
    entry->stats.idiscards = rec_drop;
    entry->stats.oerrors   = snd_err;
    entry->stats.odiscards = snd_drop;
    entry->stats.collisions = coll;
    
    /*
     * calculated stats.
     *
     *  we have imcast, but not ibcast.
     */
    entry->stats.inucast = entry->stats.imcast.low +
        entry->stats.ibcast.low;
    entry->stats.onucast = entry->stats.omcast.low +
        entry->stats.obcast.low;
    
    return 0;
}

/*
 *
 * @retval  0 success
 * @retval -1 no container specified
 * @retval -2 could not open /proc/net/dev
 * @retval -3 could not create entry (probably malloc)
 */
int
netsnmp_arch_interface_container_load(netsnmp_container* container,
                                      u_int load_flags)
{
    FILE           *devin;
    char            line[256];
    netsnmp_interface_entry *entry = NULL;
    static char     scan_expected = 0;
    int             fd;
#ifdef NETSNMP_ENABLE_IPV6
    netsnmp_container *addr_container;
#endif

    DEBUGMSGTL(("access:interface:container:arch", "load (flags %p)\n",
                load_flags));

    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for interface\n");
        return -1;
    }

    if (!(devin = fopen("/proc/net/dev", "r"))) {
        DEBUGMSGTL(("access:interface",
                    "Failed to load Interface Table (linux1)\n"));
        snmp_log(LOG_ERR, "cannot open /proc/net/dev ...\n");
        return -2;
    }

    /*
     * create socket for ioctls
     */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {
        snmp_log(LOG_ERR, "could not create socket\n");
        return -2;
    }

#ifdef NETSNMP_ENABLE_IPV6
    /*
     * get ipv6 addresses
     */
    addr_container = netsnmp_access_ipaddress_container_load(NULL, 0);
#endif

    /*
     * Read the first two lines of the file, containing the header
     * This indicates which version of the kernel we're working with,
     * and hence which statistics are actually available.
     *
     * Wes originally suggested parsing the field names in this header
     * to detect the position of individual fields directly,
     * but I suspect this is probably more trouble than it's worth.
     */
    fgets(line, sizeof(line), devin);
    fgets(line, sizeof(line), devin);

    if( 0 == scan_expected ) {
        if (strstr(line, "compressed")) {
            scan_expected = 10;
            DEBUGMSGTL(("access:interface",
                        "using linux 2.2 kernel /proc/net/dev\n"));
        } else {
            scan_expected = 5;
            DEBUGMSGTL(("access:interface",
                        "using linux 2.0 kernel /proc/net/dev\n"));
        }
    }

    /*
     * The rest of the file provides the statistics for each interface.
     * Read in each line in turn, isolate the interface name
     *   and retrieve (or create) the corresponding data structure.
     */
    while (fgets(line, sizeof(line), devin)) {
        char           *stats, *ifstart = line;
        u_int           flags;
        oid             if_index;

        flags = 0;
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        while (*ifstart && *ifstart == ' ')
            ifstart++;

        if ((!*ifstart) || ((stats = strrchr(ifstart, ':')) == NULL)) {
            snmp_log(LOG_ERR,
                     "interface data format error 1, line ==|%s|\n", line);
            continue;
        }
        if ((scan_expected == 10) && ((stats - line) < 6)) {
            snmp_log(LOG_ERR,
                     "interface data format error 2 (%d < 6), line ==|%s|\n",
                     stats - line, line);
        }

        DEBUGMSGTL(("9:access:ifcontainer", "processing '%s'\n", ifstart));

        /*
         * get index via ioctl.
         * If we've met this interface before, use the same index.
         * Otherwise find an unused index value and use that.
         */
        *stats++ = 0; /* null terminate name */

        if_index = netsnmp_arch_interface_index_find(ifstart);

        /*
         * set address type flags.
         * the only way I know of to check an interface for
         * ip version is to look for ip addresses. If anyone
         * knows a better way, put it here!
         */
#ifdef NETSNMP_ENABLE_IPV6
        _arch_interface_has_ipv6(if_index, &flags, addr_container);
#endif
        netsnmp_access_interface_ioctl_has_ipv4(fd, ifstart, 0, &flags);

        /*
         * do we only want one address type?
         */
        if (((load_flags & NETSNMP_ACCESS_INTERFACE_LOAD_IP4_ONLY) &&
             ((flags & NETSNMP_INTERFACE_FLAGS_HAS_IPV4) == 0)) ||
            ((load_flags & NETSNMP_ACCESS_INTERFACE_LOAD_IP6_ONLY) &&
             ((flags & NETSNMP_INTERFACE_FLAGS_HAS_IPV6) == 0))) {
            DEBUGMSGTL(("9:access:ifcontainer",
                        "interface '%s' excluded by ip version\n",
                        ifstart));
            continue;
        }

        entry = netsnmp_access_interface_entry_create(ifstart, 0);
        if(NULL == entry) {
#ifdef NETSNMP_ENABLE_IPV6
            netsnmp_access_ipaddress_container_free(addr_container, 0);
#endif
            netsnmp_access_interface_container_free(container,
                                                    NETSNMP_ACCESS_INTERFACE_FREE_NOFLAGS);
            fclose(devin);
            close(fd);
            return -3;
        }
        entry->ns_flags = flags; /* initial flags; we'll set more later */

        /*
         * xxx-rks: get descr by linking mem from /proc/pci and /proc/iomem
         */


        /*
         * use ioctls for some stuff
         *  (ignore rc, so we get as much info as possible)
         */
        netsnmp_access_interface_ioctl_physaddr_get(fd, entry);

        /*
         * physaddr should have set type. make some guesses (based
         * on name) if not.
         */
        if(0 == entry->type) {
            typedef struct _match_if {
               int             mi_type;
               const char     *mi_name;
            }              *pmatch_if, match_if;
            
            static match_if lmatch_if[] = {
                {IANAIFTYPE_SOFTWARELOOPBACK, "lo"},
                {IANAIFTYPE_ETHERNETCSMACD, "eth"},
                {IANAIFTYPE_ETHERNETCSMACD, "vmnet"},
                {IANAIFTYPE_ISO88025TOKENRING, "tr"},
                {IANAIFTYPE_FASTETHER, "feth"},
                {IANAIFTYPE_GIGABITETHERNET,"gig"},
                {IANAIFTYPE_PPP, "ppp"},
                {IANAIFTYPE_SLIP, "sl"},
                {IANAIFTYPE_TUNNEL, "sit"},
                {IANAIFTYPE_BASICISDN, "ippp"},
                {IANAIFTYPE_PROPVIRTUAL, "bond"}, /* Bonding driver find fastest slave */
                {IANAIFTYPE_PROPVIRTUAL, "vad"},  /* ANS driver - ?speed? */
                {0, 0}                  /* end of list */
            };

            int             ii, len;
            register pmatch_if pm;
            
            for (ii = 0, pm = lmatch_if; pm->mi_name; pm++) {
                len = strlen(pm->mi_name);
                if (0 == strncmp(entry->name, pm->mi_name, len)) {
                    entry->type = pm->mi_type;
                    break;
                }
            }
            if(NULL == pm->mi_name)
                entry->type = IANAIFTYPE_OTHER;
        }

        if (IANAIFTYPE_ETHERNETCSMACD == entry->type)
            entry->speed =
                netsnmp_linux_interface_get_if_speed(fd, entry->name);
#ifdef APPLIED_PATCH_836390   /* xxx-rks ifspeed fixes */
        else if (IANAIFTYPE_PROPVIRTUAL == entry->type)
            entry->speed = _get_bonded_if_speed(entry);
#endif
        else
            netsnmp_access_interface_entry_guess_speed(entry);
        
        netsnmp_access_interface_ioctl_flags_get(fd, entry);

        netsnmp_access_interface_ioctl_mtu_get(fd, entry);

        /*
         * Zero speed means link problem.
         * - i'm not sure this is always true...
         */
        if((entry->speed == 0) && (entry->os_flags & IFF_UP)) {
            entry->os_flags &= ~IFF_RUNNING;
        }

        /*
         * check for promiscuous mode.
         *  NOTE: there are 2 ways to set promiscuous mode in Linux
         *  (kernels later than 2.2.something) - using ioctls and
         *  using setsockopt. The ioctl method tested here does not
         *  detect if an interface was set using setsockopt. google
         *  on IFF_PROMISC and linux to see lots of arguments about it.
         */
        if(entry->os_flags & IFF_PROMISC) {
            entry->promiscuous = 1; /* boolean */
        }

        /*
         * hardcoded max packet size
         * (see ip_frag_reasm: if(len > 65535) goto out_oversize;)
         */
        entry->reasm_max_v4 = entry->reasm_max_v6 = 65535;

        netsnmp_access_interface_entry_overrides(entry);

        entry->speed_high = entry->speed / 1000000;

        if (! (load_flags & NETSNMP_ACCESS_INTERFACE_LOAD_NO_STATS))
            _parse_stats(entry, stats, scan_expected);

        if (flags & NETSNMP_INTERFACE_FLAGS_HAS_IPV4)
            _arch_interface_flags_v4_get(entry);

#ifdef NETSNMP_ENABLE_IPV6
        if (flags & NETSNMP_INTERFACE_FLAGS_HAS_IPV6)
            _arch_interface_flags_v6_get(entry);
#endif /* NETSNMP_ENABLE_IPV6 */

        /*
         * add to container
         */
        CONTAINER_INSERT(container, entry);
    }
#ifdef NETSNMP_ENABLE_IPV6
    netsnmp_access_ipaddress_container_free(addr_container, 0);
#endif
    fclose(devin);
    close(fd);
    return 0;
}

int
netsnmp_arch_set_admin_status(netsnmp_interface_entry * entry,
                              int ifAdminStatus_val)
{
    int and_complement;
    
    DEBUGMSGTL(("access:interface:arch", "set_admin_status\n"));

    if(IFADMINSTATUS_UP == ifAdminStatus_val)
        and_complement = 0; /* |= */
    else
        and_complement = 1; /* &= ~ */

    return netsnmp_access_interface_ioctl_flags_set(-1, entry,
                                                    IFF_UP, and_complement);
}

#ifdef HAVE_LINUX_ETHTOOL_H
/**
 * Determines network interface speed from ETHTOOL_GSET
 */
unsigned int
netsnmp_linux_interface_get_if_speed(int fd, const char *name)
{
    struct ifreq ifr;
    struct ethtool_cmd edata;

    memset(&ifr, 0, sizeof(ifr));
    edata.cmd = ETHTOOL_GSET;
    
    strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name)-1);
    ifr.ifr_data = (char *) &edata;
    
    if (ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
        DEBUGMSGTL(("mibII/interfaces", "ETHTOOL_GSET on %s failed\n",
                    ifr.ifr_name));
        return netsnmp_linux_interface_get_if_speed_mii(fd,name);
    }
    
    if (edata.speed != SPEED_10 && edata.speed != SPEED_100 &&
        edata.speed != SPEED_1000) {
        DEBUGMSGTL(("mibII/interfaces", "fallback to mii for %s\n",
                    ifr.ifr_name));
        /* try MII */
        return netsnmp_linux_interface_get_if_speed_mii(fd,name);
    }

    /* return in bps */
    DEBUGMSGTL(("mibII/interfaces", "ETHTOOL_GSET on %s speed = %d\n",
                ifr.ifr_name, edata.speed));
    return edata.speed*1000*1000;
}
#endif
 
/**
 * Determines network interface speed from MII
 */
unsigned int
#ifdef HAVE_LINUX_ETHTOOL_H
netsnmp_linux_interface_get_if_speed_mii(int fd, const char *name)
#else
netsnmp_linux_interface_get_if_speed(int fd, const char *name)
#endif
{
    unsigned int retspeed = 10000000;
    struct ifreq ifr;

    /* the code is based on mii-diag utility by Donald Becker
     * see ftp://ftp.scyld.com/pub/diag/mii-diag.c
     */
    ushort *data = (ushort *)(&ifr.ifr_data);
    unsigned phy_id;
    unsigned char new_ioctl_nums = 0;
    int mii_reg, i;
    ushort mii_val[32];
    ushort bmcr, bmsr, nway_advert, lkpar;
    const unsigned int media_speeds[] = {10000000, 10000000, 100000000, 100000000, 10000000, 0};	
    /* It corresponds to "10baseT", "10baseT-FD", "100baseTx", "100baseTx-FD", "100baseT4", "Flow-control", 0, */

    strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
    ifr.ifr_name[ sizeof(ifr.ifr_name)-1 ] = 0;
    data[0] = 0;
    
    if (ioctl(fd, 0x8947, &ifr) >= 0) {
        new_ioctl_nums = 1;
    } else if (ioctl(fd, SIOCDEVPRIVATE, &ifr) >= 0) {
        new_ioctl_nums = 0;
    } else {
        DEBUGMSGTL(("mibII/interfaces", "SIOCGMIIPHY on %s failed\n",
                    ifr.ifr_name));
        return retspeed;
    }

    /* Begin getting mii register values */
    phy_id = data[0];
    for (mii_reg = 0; mii_reg < 8; mii_reg++){
        data[0] = phy_id;
        data[1] = mii_reg;
        if(ioctl(fd, new_ioctl_nums ? 0x8948 : SIOCDEVPRIVATE+1, &ifr) <0){
            DEBUGMSGTL(("mibII/interfaces", "SIOCGMIIREG on %s failed\n", ifr.ifr_name));
        }
        mii_val[mii_reg] = data[3];		
    }
    /*Parsing of mii values*/
    /*Invalid basic mode control register*/
    if (mii_val[0] == 0xffff  ||  mii_val[1] == 0x0000) {
        DEBUGMSGTL(("mibII/interfaces", "No MII transceiver present!.\n"));
        return retspeed;
    }
    /* Descriptive rename. */
    bmcr = mii_val[0]; 	  /*basic mode control register*/
    bmsr = mii_val[1]; 	  /* basic mode status register*/
    nway_advert = mii_val[4]; /* autonegotiation advertisement*/
    lkpar = mii_val[5]; 	  /*link partner ability*/
    
    /*Check for link existence, returns 0 if link is absent*/
    if ((bmsr & 0x0016) != 0x0004){
        DEBUGMSGTL(("mibII/interfaces", "No link...\n"));
        retspeed = 0;
        return retspeed;
    }
    
    if(!(bmcr & 0x1000) ){
        DEBUGMSGTL(("mibII/interfaces", "Auto-negotiation disabled.\n"));
        retspeed = bmcr & 0x2000 ? 100000000 : 10000000;
        return retspeed;
    }
    /* Link partner got our advertised abilities */	
    if (lkpar & 0x4000) {
        int negotiated = nway_advert & lkpar & 0x3e0;
        int max_capability = 0;
        /* Scan for the highest negotiated capability, highest priority
           (100baseTx-FDX) to lowest (10baseT-HDX). */
        int media_priority[] = {8, 9, 7, 6, 5}; 	/* media_names[i-5] */
        for (i = 0; media_priority[i]; i++){
            if (negotiated & (1 << media_priority[i])) {
                max_capability = media_priority[i];
                break;
            }
        }
        if (max_capability)
            retspeed = media_speeds[max_capability - 5];
        else
            DEBUGMSGTL(("mibII/interfaces", "No common media type was autonegotiated!\n"));
    }else if(lkpar & 0x00A0){
        retspeed = (lkpar & 0x0080) ? 100000000 : 10000000;
    }
    return retspeed;
}
