/*
 *  Interface MIB architecture support
 *
 * $Id: arp_linux.c,v 1.6 2006/09/15 00:48:40 tanders Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/arp.h>
#include <net-snmp/data_access/interface.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

int _load_v4(netsnmp_container *container, int idx_offset);

/**
 */
int
netsnmp_access_arp_container_arch_load(netsnmp_container *container)
{
    int rc = 0, idx_offset = 0;

    rc = _load_v4(container, idx_offset);
    if(rc < 0) {
        u_int flags = NETSNMP_ACCESS_ARP_FREE_KEEP_CONTAINER;
        netsnmp_access_arp_container_free(container, flags);
        return rc;
    }

#if defined (NETSNMP_ENABLE_IPV6) && 0 /* xx-rks: arp for v6? */
    idx_offset = rc;

    rc = _load_v6(container, idx_offset);
    if(rc < 0) {
        u_int flags = NETSNMP_ACCESS_ARP_FREE_KEEP_CONTAINER;
        netsnmp_access_arp_container_free(container, flags);
    }
#endif

    /*
     * return no errors (0) if we found any interfaces
     */
    if(rc > 0)
        rc = 0;

    return rc;
}

/**
 */
int
_load_v4(netsnmp_container *container, int idx_offset)
{
    FILE           *in;
    char            line[128];
    int             rc = 0;
    netsnmp_arp_entry *entry;
    
    netsnmp_assert(NULL != container);

#define PROCFILE "/proc/net/arp"
    if (!(in = fopen(PROCFILE, "r"))) {
        snmp_log(LOG_ERR,"could not open " PROCFILE "\n");
        return -2;
    }

    /*
     * Get rid of the header line 
     */
    fgets(line, sizeof(line), in);

    /*
     * IP address | HW | Flag | HW address      | Mask | Device
     * 192.168.1.4  0x1  0x2   00:40:63:CC:1C:8C  *      eth0
     */
    while (fgets(line, sizeof(line), in)) {
        
        int             za, zb, zc, zd, ze, zf, zg, zh, zi, zj;
        int             tmp_flags;
        char            ifname[21];

        rc = sscanf(line,
                    "%d.%d.%d.%d 0x%*x 0x%x %x:%x:%x:%x:%x:%x %*[^ ] %20s\n",
                    &za, &zb, &zc, &zd, &tmp_flags, &ze, &zf, &zg, &zh, &zi,
                    &zj, ifname);
        if (12 != rc) {            
            snmp_log(LOG_ERR, PROCFILE " data format error (%d!=12)\n", rc);
            snmp_log(LOG_ERR, " line ==|%s|\n", line);
            continue;
        }
        DEBUGMSGTL(("access:arp:container",
                    "ip addr %d.%d.%d.%d, flags 0x%X, hw addr "
                    "%x:%x:%x:%x:%x:%x, name %s\n",
                    za,zb,zc,zd, tmp_flags, ze,zf,zg,zh,zi,zj, ifname ));

        /*
         */
        entry = netsnmp_access_arp_entry_create();
        if(NULL == entry) {
            rc = -3;
            break;
        }

        /*
         * look up ifIndex
         */
        entry->if_index = netsnmp_access_interface_index_find(ifname);
        if(0 == entry->if_index) {
            snmp_log(LOG_ERR,"couldn't find ifIndex for '%s', skipping\n",
                     ifname);
            netsnmp_access_arp_entry_free(entry);
            continue;
        }

        /*
         * now that we've passed all the possible 'continue', assign
         * index offset.
         */
        entry->ns_arp_index = ++idx_offset;

        /*
         * parse ip addr
         */
        entry->arp_ipaddress[0] = za;
        entry->arp_ipaddress[1] = zb;
        entry->arp_ipaddress[2] = zc;
        entry->arp_ipaddress[3] = zd;
        entry->arp_ipaddress_len = 4;

        /*
         * parse hw addr
         */
        entry->arp_physaddress[0] = ze;
        entry->arp_physaddress[1] = zf;
        entry->arp_physaddress[2] = zg;
        entry->arp_physaddress[3] = zh;
        entry->arp_physaddress[4] = zi;
        entry->arp_physaddress[5] = zj;
        entry->arp_physaddress_len = 6;

        /*
         * what can we do with hw? from arp manpage:

         default  value  of  this  parameter is ether (i.e. hardware code
         0x01 for  IEEE  802.3  10Mbps  Ethernet).   Other  values  might
         include  network  technologies  such as ARCnet (arcnet) , PROnet
         (pronet) , AX.25 (ax25) and NET/ROM (netrom).
        */

        /*
         * parse mask
         */
        /* xxx-rks: what is mask? how to interpret '*'? */


        /*
         * process type
         */
        if(tmp_flags & ATF_PERM)
            entry->arp_type = INETNETTOMEDIATYPE_STATIC;
        else
            entry->arp_type = INETNETTOMEDIATYPE_DYNAMIC;

        /*
         * process status
         * if flags are 0, we can't tell the difference between
         * stale or incomplete.
         */
        if(tmp_flags & ATF_COM)
            entry->arp_state = INETNETTOMEDIASTATE_REACHABLE;
        else
            entry->arp_state = INETNETTOMEDIASTATE_UNKNOWN;

        /*
         * add entry to container
         */
        CONTAINER_INSERT(container, entry);
    }

    fclose(in);
    if( rc < 0 )
        return rc;

    return idx_offset;
}
