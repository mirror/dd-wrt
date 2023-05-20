/*
 *  Interface MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include "mibII/mibII_common.h"
#include "if-mib/ifTable/ifTable_constants.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/interface.h>
#include <net-snmp/data_access/ipaddress.h>
#include "if-mib/data_access/interface.h"

netsnmp_feature_child_of(interface_ioctl_flags_set, interface_all);

#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NET_IF_ARP_H
#include <net/if_arp.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include "interface_ioctl.h"
#include "ip-mib/data_access/ipaddress_ioctl.h"

/**
 * ioctl wrapper
 *
 * @param      fd : socket fd to use w/ioctl, or -1 to open/close one
 * @param  which
 * @param ifrq
 * param ifentry : ifentry to update
 * @param name
 *
 * @retval  0 : success
 * @retval -1 : invalid parameters
 * @retval -2 : couldn't create socket
 * @retval -3 : ioctl call failed
 */
static int
_ioctl_get(int fd, int which, struct ifreq *ifrq, const char* name)
{
    int ourfd = -1, rc = 0;

    DEBUGMSGTL(("verbose:access:interface:ioctl",
                "ioctl %d for %s\n", which, name));

    /*
     * sanity checks
     */
    if(NULL == name) {
        snmp_log(LOG_ERR, "invalid ifentry\n");
        return -1;
    }

    /*
     * create socket for ioctls
     */
    if(fd < 0) {
        fd = ourfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(ourfd < 0) {
            snmp_log(LOG_ERR,"couldn't create socket\n");
            return -2;
        }
    }

    strlcpy(ifrq->ifr_name, name, sizeof(ifrq->ifr_name));
    rc = ioctl(fd, which, ifrq);
    if (rc < 0)
        rc = -3;

    if(ourfd >= 0)
        close(ourfd);

    return rc;
}

int netsnmp_convert_arphrd_type(int arphrd_type)
{
    /*
     * arphrd defines vary greatly. ETHER seems to be the only common one
     */
#ifdef ARPHRD_ETHER
    switch (arphrd_type) {
    case ARPHRD_ETHER:
        return IANAIFTYPE_ETHERNETCSMACD;
#if defined(ARPHRD_TUNNEL) || defined(ARPHRD_IPGRE) || defined(ARPHRD_SIT)
#ifdef ARPHRD_TUNNEL
    case ARPHRD_TUNNEL:
    case ARPHRD_TUNNEL6:
#endif
#ifdef ARPHRD_IPGRE
    case ARPHRD_IPGRE:
#endif
#ifdef ARPHRD_SIT
    case ARPHRD_SIT:
#endif
        return IANAIFTYPE_TUNNEL;
#endif
#ifdef ARPHRD_INFINIBAND
    case ARPHRD_INFINIBAND:
        return IANAIFTYPE_INFINIBAND;
#endif
#ifdef ARPHRD_SLIP
    case ARPHRD_SLIP:
    case ARPHRD_CSLIP:
    case ARPHRD_SLIP6:
    case ARPHRD_CSLIP6:
        return IANAIFTYPE_SLIP;
#endif
#ifdef ARPHRD_PPP
    case ARPHRD_PPP:
        return IANAIFTYPE_PPP;
#endif
#ifdef ARPHRD_LOOPBACK
    case ARPHRD_LOOPBACK:
        return IANAIFTYPE_SOFTWARELOOPBACK;
#endif
#ifdef ARPHRD_FDDI
    case ARPHRD_FDDI:
        return IANAIFTYPE_FDDI;
#endif
#ifdef ARPHRD_ARCNET
    case ARPHRD_ARCNET:
        return IANAIFTYPE_ARCNET;
#endif
#ifdef ARPHRD_LOCALTLK
    case ARPHRD_LOCALTLK:
        return IANAIFTYPE_LOCALTALK;
#endif
#ifdef ARPHRD_HIPPI
    case ARPHRD_HIPPI:
        return IANAIFTYPE_HIPPI;
#endif
#ifdef ARPHRD_ATM
    case ARPHRD_ATM:
        return IANAIFTYPE_ATM;
#endif
        /*
         * XXX: more if_arp.h:ARPHRD_xxx to IANAifType mappings... 
         */
    default:
        DEBUGMSGTL(("access:interface:ioctl", "unknown entry type %d\n",
                    arphrd_type));
        return IANAIFTYPE_OTHER;
    } /* switch */
#endif /* ARPHRD_LOOPBACK */
}

#ifdef SIOCGIFHWADDR
/**
 * interface entry physaddr ioctl wrapper
 *
 * @param      fd : socket fd to use w/ioctl, or -1 to open/close one
 * @param ifentry : ifentry to update
 *
 * @retval  0 : success
 * @retval -1 : invalid parameters
 * @retval -2 : couldn't create socket
 * @retval -3 : ioctl call failed
 * @retval -4 : malloc error
 */
int
netsnmp_access_interface_ioctl_physaddr_get(int fd,
                                            netsnmp_interface_entry *ifentry)
{
    struct ifreq    ifrq;
    int rc = 0;

    DEBUGMSGTL(("access:interface:ioctl", "physaddr_get\n"));

    if((NULL != ifentry->paddr) &&
       (ifentry->paddr_len != IFHWADDRLEN)) {
        SNMP_FREE(ifentry->paddr);
    }
    if(NULL == ifentry->paddr) 
        ifentry->paddr = (char*)malloc(IFHWADDRLEN);

    if(NULL == ifentry->paddr) {
            rc = -4;
    } else {

        /*
         * NOTE: this ioctl does not guarantee 6 bytes of a physaddr.
         * In particular, a 'sit0' interface only appears to get back
         * 4 bytes of sa_data. Uncomment this memset, and suddenly
         * the sit interface will be 0:0:0:0:?:? where ? is whatever was
         * in the memory before. Not sure if this memset should be done
         * for every ioctl, as the rest seem to work ok...
         */
        memset(ifrq.ifr_hwaddr.sa_data, (0), IFHWADDRLEN);
        ifentry->paddr_len = IFHWADDRLEN;
        rc = _ioctl_get(fd, SIOCGIFHWADDR, &ifrq, ifentry->name);
        if (rc < 0) {
            memset(ifentry->paddr, (0), IFHWADDRLEN);
            rc = -3;
        } else {
            memcpy(ifentry->paddr, ifrq.ifr_hwaddr.sa_data, IFHWADDRLEN);
            ifentry->type =
                netsnmp_convert_arphrd_type(ifrq.ifr_hwaddr.sa_family);
        }
    }

    return rc;
}
#endif /* SIOCGIFHWADDR */

void
netsnmp_process_link_flags(netsnmp_interface_entry *ifentry,
                           unsigned int os_flags)
{
    ifentry->ns_flags |= NETSNMP_INTERFACE_FLAGS_HAS_IF_FLAGS;
    ifentry->os_flags = os_flags;

    /*
     * ifOperStatus description:
     *   If ifAdminStatus is down(2) then ifOperStatus should be down(2).
     */
    if (ifentry->os_flags & IFF_UP) {
        ifentry->admin_status = IFADMINSTATUS_UP;
        if (ifentry->os_flags & IFF_RUNNING)
            ifentry->oper_status = IFOPERSTATUS_UP;
        else
            ifentry->oper_status = IFOPERSTATUS_DOWN;
    } else {
        ifentry->admin_status = IFADMINSTATUS_DOWN;
        ifentry->oper_status = IFOPERSTATUS_DOWN;
    }

    /*
     * ifConnectorPresent description:
     *   This object has the value 'true(1)' if the interface sublayer has a
     *   physical connector and the value 'false(2)' otherwise."
     * So, at very least, false(2) should be returned for loopback devices.
     */
    if (ifentry->os_flags & IFF_LOOPBACK) {
        ifentry->connector_present = 0;
    } else {
        ifentry->connector_present = 1;
    }
}

#ifdef SIOCGIFFLAGS
/**
 * interface entry flags ioctl wrapper
 *
 * @param      fd : socket fd to use w/ioctl, or -1 to open/close one
 * @param ifentry : ifentry to update
 *
 * @retval  0 : success
 * @retval -1 : invalid parameters
 * @retval -2 : couldn't create socket
 * @retval -3 : ioctl call failed
 */
int
netsnmp_access_interface_ioctl_flags_get(int fd,
                                         netsnmp_interface_entry *ifentry)
{
    struct ifreq    ifrq;
    int rc = 0;

    DEBUGMSGTL(("access:interface:ioctl", "flags_get\n"));

    rc = _ioctl_get(fd, SIOCGIFFLAGS, &ifrq, ifentry->name);
    if (rc < 0) {
        ifentry->ns_flags &= ~NETSNMP_INTERFACE_FLAGS_HAS_IF_FLAGS;
        return rc;
    } else {
        netsnmp_process_link_flags(ifentry, ifrq.ifr_flags);
    }
    
    return rc;
}

#ifndef NETSNMP_FEATURE_REMOVE_INTERFACE_IOCTL_FLAGS_SET
/**
 * interface entry flags ioctl wrapper
 *
 * @param      fd : socket fd to use w/ioctl, or -1 to open/close one
 * @param ifentry : ifentry to update
 *
 * @retval  0 : success
 * @retval -1 : invalid parameters
 * @retval -2 : couldn't create socket
 * @retval -3 : ioctl get call failed
 * @retval -4 : ioctl set call failed
 */
int
netsnmp_access_interface_ioctl_flags_set(int fd,
                                         netsnmp_interface_entry *ifentry,
                                         unsigned int flags, int and_complement)
{
    struct ifreq    ifrq;
    int ourfd = -1, rc = 0;

    DEBUGMSGTL(("access:interface:ioctl", "flags_set\n"));

    /*
     * sanity checks
     */
    if((NULL == ifentry) || (NULL == ifentry->name)) {
        snmp_log(LOG_ERR, "invalid ifentry\n");
        return -1;
    }

    /*
     * create socket for ioctls
     */
    if(fd < 0) {
        fd = ourfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(ourfd < 0) {
            snmp_log(LOG_ERR,"couldn't create socket\n");
            return -2;
        }
    }

    strlcpy(ifrq.ifr_name, ifentry->name, sizeof(ifrq.ifr_name));
    rc = ioctl(fd, SIOCGIFFLAGS, &ifrq);
    if(rc < 0) {
        snmp_log(LOG_ERR,"error getting flags\n");
        close(fd);
        return -3;
    }
    if(0 == and_complement)
        ifrq.ifr_flags |= flags;
    else
        ifrq.ifr_flags &= ~flags;
    rc = ioctl(fd, SIOCSIFFLAGS, &ifrq);
    if(rc < 0) {
        close(fd);
        snmp_log(LOG_ERR,"error setting flags\n");
        ifentry->os_flags = 0;
        return -4;
    }

    if(ourfd >= 0)
        close(ourfd);

    ifentry->os_flags = ifrq.ifr_flags;

    return 0;
}
#endif /* NETSNMP_FEATURE_REMOVE_INTERFACE_IOCTL_FLAGS_SET */
#endif /* SIOCGIFFLAGS */

#ifdef SIOCGIFMTU
/**
 * interface entry mtu ioctl wrapper
 *
 * @param      fd : socket fd to use w/ioctl, or -1 to open/close one
 * @param ifentry : ifentry to update
 *
 * @retval  0 : success
 * @retval -1 : invalid parameters
 * @retval -2 : couldn't create socket
 * @retval -3 : ioctl call failed
 */
int
netsnmp_access_interface_ioctl_mtu_get(int fd,
                                       netsnmp_interface_entry *ifentry)
{
    struct ifreq    ifrq;
    int rc = 0;

    DEBUGMSGTL(("access:interface:ioctl", "mtu_get\n"));

    rc = _ioctl_get(fd, SIOCGIFMTU, &ifrq, ifentry->name);
    if (rc < 0) {
        ifentry->mtu = 0;
        return rc;
    } else {
        ifentry->mtu = ifrq.ifr_mtu;
    }

    return rc;
}
#endif /* SIOCGIFMTU */

/**
 * interface entry ifIndex ioctl wrapper
 *
 * @param      fd : socket fd to use w/ioctl, or -1 to open/close one
 * @param name   : ifentry to update
 *
 * @retval  0 : not found
 * @retval !0 : ifIndex
 */
oid
netsnmp_access_interface_ioctl_ifindex_get(int fd, const char *name)
{
#ifndef SIOCGIFINDEX
    return 0;
#else
    struct ifreq    ifrq;
    int rc = 0;

    DEBUGMSGTL(("access:interface:ioctl", "ifindex_get\n"));

    rc = _ioctl_get(fd, SIOCGIFINDEX, &ifrq, name);
    if (rc < 0) {
        DEBUGMSGTL(("access:interface:ioctl",
                   "ifindex_get error on interface '%s'\n", name));
        return 0;
    }

#if defined(freebsd3)    /* ? Should use HAVE_STRUCT_IFREQ_IFR_INDEX */
    return ifrq.ifr_index;
#else
    return ifrq.ifr_ifindex;
#endif
#endif /* SIOCGIFINDEX */
}

/**
 * check an interface for ipv4 addresses
 *
 * @param sd      : open socket descriptor
 * @param if_name : optional name. takes precedent over if_index.
 * @param if_index: optional if index. only used if no if_name specified
 * @param flags   :
 * @param pifc    : ifconf structures returned by the SIOCGIFCONF ioctl
 *
 * @retval < 0 : error
 * @retval   0 : no ip v4 addresses
 * @retval   1 : 1 or more ip v4 addresses
 */
int
netsnmp_access_interface_ioctl_has_ipv4(int sd, const char *if_name,
                                        int if_index, u_int *flags,
                                        const struct ifconf *pifc)
{
    int             i, interfaces = 0;
    struct ifreq   *ifrp;

    /*
     * one or the other
     */
    if (NULL == flags || NULL == pifc ||
        ((0 == if_index) && (NULL == if_name))) {
        return -1;
    }

    *flags &= ~NETSNMP_INTERFACE_FLAGS_HAS_IPV4;

    ifrp = pifc->ifc_req;
    interfaces = pifc->ifc_len / sizeof(struct ifreq);
    for(i=0; i < interfaces; ++i, ++ifrp) {

        DEBUGMSGTL(("access:ipaddress:container",
                    " interface %d, %s\n", i, ifrp->ifr_name));

        /*
         * search for matching if_name or if_index
         */
        if (NULL != if_name) {
            if  (strncmp(if_name, ifrp->ifr_name, sizeof(ifrp->ifr_name)) != 0)
                continue;
        }
        else {
            /*
             * I think that Linux and Solaris both use ':' in the
             * interface name for aliases.
             */
            char *ptr = strchr(ifrp->ifr_name, ':');
            if (NULL != ptr)
                *ptr = 0;
            
            if (if_index != (int)netsnmp_access_interface_ioctl_ifindex_get(sd, ifrp->ifr_name))
                continue;
        }

        /*
         * check and set v4 or v6 flag, and break if we've found both
         */
        if (AF_INET == ifrp->ifr_addr.sa_family) {
            *flags |= NETSNMP_INTERFACE_FLAGS_HAS_IPV4;
            break;
        }
    }

    return 0;
}
