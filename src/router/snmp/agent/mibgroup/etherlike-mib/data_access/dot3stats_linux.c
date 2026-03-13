/*
 * standard Net-SNMP includes 
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/interface.h>

#if !(defined(HAVE_LIBNL3) && defined(HAVE_NETLINK_ROUTE_LINK_H))
 #ifndef HAVE_LIBNL3
  #error libnl3-devel (RedHat derivatives) / libnl-3-dev (Debian derivatives) required
 #endif
 #ifndef HAVE_NETLINK_ROUTE_LINK_H
  #error libnl-route-3-dev required (Debian derivatives)
 #endif
 #if !defined(HAVE_LIBNL3) && !defined(HAVE_NETLINK_ROUTE_LINK_H) && defined(__linux__)
  #error pkg-config/pkgconfig is required to detect libnl-3
 #endif
#endif

#include <netlink/cache.h>
#include <netlink/netlink.h>
#include <netlink/route/addr.h>

/*
 * include our parent header 
 */
#include "etherlike-mib/dot3StatsTable/dot3StatsTable.h"
#include "etherlike-mib/dot3StatsTable/dot3StatsTable_data_access.h"
#include "etherlike-mib/dot3StatsTable/ioctl_imp_common.h"

static void dot3StatsTable_add_interface(netsnmp_container *container,
                                         int if_index,
                                         struct rtnl_link *rtnl_link)
{
    uint64_t tx_err, tx_drop, tx_colls, tx_carrier;
    uint64_t rx_errs, rx_fifo, rx_frame;
    dot3StatsTable_rowreq_ctx *row;
    int rc;

    row = dot3StatsTable_allocate_rowreq_ctx(NULL);
    if (!row)
        return;
    dot3StatsTable_indexes_set(row, if_index);

    /*
     * See also dev_seq_printf_stats() in the Linux kernel source file
     * net/core/net-procfs.c.
     */
    rx_errs = rtnl_link_get_stat(rtnl_link, RTNL_LINK_RX_ERRORS);
    rx_fifo = rtnl_link_get_stat(rtnl_link, RTNL_LINK_RX_DROPPED) +
        rtnl_link_get_stat(rtnl_link, RTNL_LINK_RX_MISSED_ERR);
    rx_frame = rtnl_link_get_stat(rtnl_link, RTNL_LINK_MULTICAST);

    tx_err = rtnl_link_get_stat(rtnl_link, RTNL_LINK_TX_ERRORS);
    tx_drop = rtnl_link_get_stat(rtnl_link, RTNL_LINK_TX_DROPPED);
    tx_colls = rtnl_link_get_stat(rtnl_link, RTNL_LINK_TX_ABORT_ERR);
    tx_carrier = rtnl_link_get_stat(rtnl_link, RTNL_LINK_TX_CARRIER_ERR);

    row->column_exists_flags |=
        COLUMN_DOT3STATSINDEX_FLAG |
        COLUMN_DOT3STATSFCSERRORS_FLAG |
        COLUMN_DOT3STATSINTERNALMACRECEIVEERRORS_FLAG |
        COLUMN_DOT3STATSFRAMETOOLONGS_FLAG |
        COLUMN_DOT3STATSDEFERREDTRANSMISSIONS_FLAG |
        COLUMN_DOT3STATSINTERNALMACTRANSMITERRORS_FLAG |
        COLUMN_DOT3STATSSINGLECOLLISIONFRAMES_FLAG |
        COLUMN_DOT3STATSCARRIERSENSEERRORS_FLAG;
    row->data.dot3StatsFCSErrors = rx_errs;
    row->data.dot3StatsInternalMacReceiveErrors = rx_fifo;
    row->data.dot3StatsFrameTooLongs = rx_frame;
    row->data.dot3StatsDeferredTransmissions = tx_err;
    row->data.dot3StatsInternalMacTransmitErrors = tx_drop;
    row->data.dot3StatsSingleCollisionFrames = tx_colls;
    row->data.dot3StatsCarrierSenseErrors = tx_carrier;

    rc = CONTAINER_INSERT(container, row);
    netsnmp_assert(rc == 0);
}

/*
 * @retval  0 success
 * @retval -1 getifaddrs failed
 * @retval -2 memory allocation failed
 */
int dot3StatsTable_container_load_impl(netsnmp_container *container)
{
    struct nl_cache *link_cache;
    struct nl_object *nl_object;
    struct nl_sock *nl_sock;
    int ret;

    nl_sock = nl_socket_alloc();
    if (!nl_sock)
        return -1;
    ret = nl_connect(nl_sock, NETLINK_ROUTE);
    if (ret < 0)
        return -1;
    ret = rtnl_link_alloc_cache(nl_sock, AF_UNSPEC, &link_cache);
    if (ret) {
        ret = -1;
        goto free_nl_sock;
    }

    for (nl_object = nl_cache_get_first(link_cache); nl_object;
         nl_object = nl_cache_get_next(nl_object)) {
        struct rtnl_link *rtnl_link = (void *)nl_object;
        int if_index = rtnl_link_get_ifindex(rtnl_link);

        netsnmp_assert(if_index > 0);
        if (if_index <= 0)
            continue;
        dot3StatsTable_add_interface(container, if_index, rtnl_link);
    }

    nl_cache_put(link_cache);

free_nl_sock:
    nl_socket_free(nl_sock);

    return ret;
}
