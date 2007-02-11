/*
 *  Systemstats MIB architecture support
 *
 * $Id: systemstats_common.c,v 1.3 2005/02/08 21:56:40 nba Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "ip-mib/ipSystemStatsTable/ipSystemStatsTable_constants.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/ipstats.h>
#include <net-snmp/data_access/systemstats.h>

/**---------------------------------------------------------------------*/
/*
 * local static vars
 */
static int need_wrap_check = -1;

/*
 * local static prototypes
 */
static void _entry_release(netsnmp_systemstats_entry * entry, void *unused);

/**---------------------------------------------------------------------*/
/*
 * external per-architecture functions prototypes
 *
 * These shouldn't be called by the general public, so they aren't in
 * the header file.
 */
extern int
netsnmp_access_systemstats_container_arch_load(netsnmp_container* container,
                                             u_int load_flags);
extern void
netsnmp_access_systemstats_arch_init(void);

/**---------------------------------------------------------------------*/
/*
 * initialization
 */
void
netsnmp_access_systemstats_init(void)
{
    netsnmp_container * ifcontainer;

    netsnmp_access_systemstats_arch_init();

    /*
     * load once to set up ifIndexes
     */
    ifcontainer = netsnmp_access_systemstats_container_load(NULL, 0);
    if(NULL != ifcontainer)
        netsnmp_access_systemstats_container_free(ifcontainer, 0);

}

/**---------------------------------------------------------------------*/
/*
 * container functions
 */
/**
 * initialize systemstats container
 */
netsnmp_container *
netsnmp_access_systemstats_container_init(u_int flags)
{
    netsnmp_container *container;

    DEBUGMSGTL(("access:systemstats:container", "init\n"));

    /*
     * create the containers. one indexed by ifIndex, the other
     * indexed by ifName.
     */
    container = netsnmp_container_find("access_systemstats:table_container");
    if (NULL == container)
        return NULL;

    return container;
}

/**
 * load systemstats information in specified container
 *
 * @param container empty container, or NULL to have one created for you
 * @param load_flags flags to modify behaviour.
 *
 * @retval NULL  error
 * @retval !NULL pointer to container
 */
netsnmp_container*
netsnmp_access_systemstats_container_load(netsnmp_container* container, u_int load_flags)
{
    int rc;

    DEBUGMSGTL(("access:systemstats:container", "load\n"));

    if (NULL == container)
        container = netsnmp_access_systemstats_container_init(load_flags);
    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_systemstats\n");
        return NULL;
    }

    rc =  netsnmp_access_systemstats_container_arch_load(container, load_flags);
    if (0 != rc) {
        netsnmp_access_systemstats_container_free(container,
                                                NETSNMP_ACCESS_SYSTEMSTATS_FREE_NOFLAGS);
        container = NULL;
    }

    return container;
}

void
netsnmp_access_systemstats_container_free(netsnmp_container *container, u_int free_flags)
{
    DEBUGMSGTL(("access:systemstats:container", "free\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR, "invalid container for netsnmp_access_systemstats_free\n");
        return;
    }

    if(! (free_flags & NETSNMP_ACCESS_SYSTEMSTATS_FREE_DONT_CLEAR)) {
        /*
         * free all items.
         */
        CONTAINER_CLEAR(container,
                        (netsnmp_container_obj_func*)_entry_release,
                        NULL);
    }

    CONTAINER_FREE(container);
}

/**---------------------------------------------------------------------*/
/*
 * entry functions
 */
/**
 */
netsnmp_systemstats_entry *
netsnmp_access_systemstats_entry_get_by_index(netsnmp_container *container, oid index)
{
    netsnmp_index   tmp;

    DEBUGMSGTL(("access:systemstats:entry", "by_index\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR,
                 "invalid container for netsnmp_access_systemstats_entry_get_by_index\n");
        return NULL;
    }

    tmp.len = 1;
    tmp.oids = &index;

    return (netsnmp_systemstats_entry *) CONTAINER_FIND(container, &tmp);
}

/**
 */
netsnmp_systemstats_entry *
netsnmp_access_systemstats_entry_create(int version)
{
    netsnmp_systemstats_entry *entry =
        SNMP_MALLOC_TYPEDEF(netsnmp_systemstats_entry);

    DEBUGMSGTL(("access:systemstats:entry", "create\n"));

    if(NULL == entry)
        return NULL;

    entry->ns_ip_version = version;

    entry->oid_index.len = 1;
    entry->oid_index.oids = (oid *) & entry->ns_ip_version;

    return entry;
}

/**
 */
void
netsnmp_access_systemstats_entry_free(netsnmp_systemstats_entry * entry)
{
    DEBUGMSGTL(("access:systemstats:entry", "free\n"));

    if (NULL == entry)
        return;

    /*
     * SNMP_FREE not needed, for any of these, 
     * since the whole entry is about to be freed
     */

    if (NULL != entry->old_stats)
        free(entry->old_stats);

    free(entry);
}


/**---------------------------------------------------------------------*/
/*
 * Utility routines
 */

/**
 * \internal
 */
static void
_entry_release(netsnmp_systemstats_entry * entry, void *context)
{
    netsnmp_access_systemstats_entry_free(entry);
}

/**
 * update entry stats (checking for counter wrap)
 *
 * @retval  0 : success
 * @retval <0 : error
 */
int
netsnmp_access_systemstats_entry_update_stats(netsnmp_systemstats_entry * prev_vals,
                                              netsnmp_systemstats_entry * new_vals)
{
    DEBUGMSGTL(("access:systemstats", "check_wrap\n"));
    
    /*
     * sanity checks
     */
    if ((NULL == prev_vals) || (NULL == new_vals) ||
        (prev_vals->ns_ip_version != new_vals->ns_ip_version))
        return -1;

    /*
     * if we've determined that we have 64 bit counters, just copy them.
     */
    if (0 == need_wrap_check) {
        memcpy(&prev_vals->stats, &new_vals->stats, sizeof(new_vals->stats));
        return 0;
    }

    if (NULL == prev_vals->old_stats) {
        /*
         * if we don't have old stats, they can't have wrapped, so just copy
         */
        prev_vals->old_stats = SNMP_MALLOC_TYPEDEF(netsnmp_ipstats);
        if (NULL == prev_vals->old_stats) {
            return -2;
        }
    }
    else {
        /*
         * update straight 32 bit counters
         */
        prev_vals->stats.InHdrErrors = new_vals->stats.InHdrErrors;
        prev_vals->stats.InNoRoutes = new_vals->stats.InNoRoutes;
        prev_vals->stats.InAddrErrors = new_vals->stats.InAddrErrors;
        prev_vals->stats.InUnknownProtos = new_vals->stats.InUnknownProtos;
        prev_vals->stats.InTruncatedPkts = new_vals->stats.InTruncatedPkts;
        prev_vals->stats.ReasmReqds = new_vals->stats.InTruncatedPkts;
        prev_vals->stats.ReasmOKs = new_vals->stats.ReasmOKs;
        prev_vals->stats.ReasmFails = new_vals->stats.ReasmFails;
        prev_vals->stats.InDiscards = new_vals->stats.InDiscards;
        prev_vals->stats.OutNoRoutes = new_vals->stats.OutNoRoutes;
        prev_vals->stats.OutDiscards = new_vals->stats.OutDiscards;
        prev_vals->stats.OutFragReqds = new_vals->stats.OutFragReqds;
        prev_vals->stats.OutFragOKs = new_vals->stats.OutFragOKs;
        prev_vals->stats.OutFragFails = new_vals->stats.OutFragFails;
        prev_vals->stats.OutFragCreates = new_vals->stats.OutFragCreates;

        /*
         * update 64bit counters
         */
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInReceives,
                                       &new_vals->stats.HCInReceives,
                                       &prev_vals->old_stats->HCInReceives,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInOctets,
                                       &new_vals->stats.HCInOctets,
                                       &prev_vals->old_stats->HCInOctets,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInForwDatagrams,
                                       &new_vals->stats.HCInForwDatagrams,
                                       &prev_vals->old_stats->HCInForwDatagrams,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInDelivers,
                                       &new_vals->stats.HCInDelivers,
                                       &prev_vals->old_stats->HCInDelivers,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutRequests,
                                       &new_vals->stats.HCOutRequests,
                                       &prev_vals->old_stats->HCOutRequests,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutForwDatagrams,
                                       &new_vals->stats.HCOutForwDatagrams,
                                       &prev_vals->old_stats->HCOutForwDatagrams,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutTransmits,
                                       &new_vals->stats.HCOutTransmits,
                                       &prev_vals->old_stats->HCOutTransmits,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutOctets,
                                       &new_vals->stats.HCOutOctets,
                                       &prev_vals->old_stats->HCOutOctets,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInMcastPkts,
                                       &new_vals->stats.HCInMcastPkts,
                                       &prev_vals->old_stats->HCInMcastPkts,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInMcastOctets,
                                       &new_vals->stats.HCInMcastOctets,
                                       &prev_vals->old_stats->HCInMcastOctets,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutMcastPkts,
                                       &new_vals->stats.HCOutMcastPkts,
                                       &prev_vals->old_stats->HCOutMcastPkts,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutMcastOctets,
                                       &new_vals->stats.HCOutMcastOctets,
                                       &prev_vals->old_stats->HCOutMcastOctets,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCInBcastPkts,
                                       &new_vals->stats.HCInBcastPkts,
                                       &prev_vals->old_stats->HCInBcastPkts,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.HCOutBcastPkts,
                                       &new_vals->stats.HCOutBcastPkts,
                                       &prev_vals->old_stats->HCOutBcastPkts,
                                       &need_wrap_check);
    }
    
    /*
     * if we've decided we no longer need to check wraps, free old stats
     */
    if (0 == need_wrap_check) {
        SNMP_FREE(prev_vals->old_stats);
    }
    
    /*
     * update old stats from new stats.
     * careful - old_stats is a pointer to stats...
     */
    memcpy(prev_vals->old_stats, &new_vals->stats, sizeof(new_vals->stats));
    
    return 0;
}

/**
 * update systemstats entry data (checking for counter wraps)
 *
 * Given an existing entry, update it with the new values from another
 * entry.
 *
 * @retval -2 : malloc failed
 * @retval -1 : systemstatss not the same
 * @retval  0 : no error
 */
int
netsnmp_access_systemstats_entry_update(netsnmp_systemstats_entry * lhs,
                                        netsnmp_systemstats_entry * rhs)
{
    DEBUGMSGTL(("access:systemstats", "copy\n"));
    
    if ((NULL == lhs) || (NULL == rhs) ||
        (lhs->ns_ip_version != rhs->ns_ip_version))
        return -1;

    /*
     * update stats
     */
    netsnmp_access_systemstats_entry_update_stats(lhs, rhs);

    /*
     * update other data
     */
    lhs->flags = rhs->flags;
    
    return 0;
}

/**---------------------------------------------------------------------*/
/*
 *
 */
