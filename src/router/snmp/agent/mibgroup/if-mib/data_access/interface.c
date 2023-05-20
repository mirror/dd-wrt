/*
 *  Interface MIB architecture support
 *
 * $Id$
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-features.h>
#include <net-snmp/net-snmp-includes.h>
#include <errno.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_enum.h>
#include <net-snmp/data_access/interface.h>

#include "mibII/mibII_common.h"
#include "if-mib/ifTable/ifTable.h"
#include "if-mib/data_access/interface.h"
#include "interface_private.h"
#if defined(HAVE_PCRE_H)
#include <pcre.h>
#elif defined(HAVE_REGEX_H)
#include <sys/types.h>
#include <regex.h>
#endif

netsnmp_feature_child_of(interface_all, libnetsnmpmibs);
netsnmp_feature_child_of(interface, interface_all);
netsnmp_feature_child_of(interface_access_entry_set_admin_status, interface_all);
netsnmp_feature_child_of(interface_legacy, interface_all);

#ifdef NETSNMP_FEATURE_REQUIRE_INTERFACE_ACCESS_ENTRY_SET_ADMIN_STATUS
netsnmp_feature_require(interface_arch_set_admin_status);
#endif /* NETSNMP_FEATURE_REQUIRE_INTERFACE_ACCESS_ENTRY_SET_ADMIN_STATUS */

/**---------------------------------------------------------------------*/
/*
 * local static vars
 */
static netsnmp_conf_if_list *conf_list = NULL;
static int need_wrap_check = -1;
static int _access_interface_init = 0;
static netsnmp_include_if_list *include_list;
static int ifmib_max_num_ifaces = 0;

/*
 * local static prototypes
 */
static int _access_interface_entry_compare_name(const void *lhs,
                                                const void *rhs);
static void _access_interface_entry_release(netsnmp_interface_entry * entry,
                                            void *unused);
static void _access_interface_entry_save_name(const char *name, oid index);
static void _parse_interface_config(const char *token, char *cptr);
static void _parse_ifmib_max_num_ifaces(const char *token, char *cptr);
static void _free_interface_config(void);
static void _parse_include_if_config(const char *token, char *cptr);
static void _free_include_if_config(void);

/*
 * This function is called after the snmpd configuration has been read
 * and loads the interface list if it has not yet been loaded.
 */
static int
_load_if_list(int majorID, int minorID, void *serverargs, void *clientarg)
{
    netsnmp_access_interface_init();
    return 0;
}

/**
 * initialization
 */
void
init_interface(void)
{
    snmpd_register_config_handler("interface", _parse_interface_config,
                                  _free_interface_config,
                                  "name type speed");

    snmpd_register_config_handler("ifmib_max_num_ifaces",
                                  _parse_ifmib_max_num_ifaces,
                                  NULL,
                                  "IF-MIB MAX Number of ifaces");

    snmpd_register_config_handler("include_ifmib_iface_prefix",
                                  _parse_include_if_config,
                                  _free_include_if_config,
                                  "IF-MIB iface names included");

    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
                           SNMP_CALLBACK_POST_READ_CONFIG,
                           _load_if_list, NULL);
}

/* May be called multiple times. */
void
netsnmp_access_interface_init(void)
{
    if (1 == _access_interface_init)
        return;

    _access_interface_init = 1;

    {
        netsnmp_container * ifcontainer;

        netsnmp_arch_interface_init();
        
        /*
         * load once to set up ifIndexes
         */
        ifcontainer = netsnmp_access_interface_container_load(NULL, 0);
        if(NULL != ifcontainer)
            netsnmp_access_interface_container_free(ifcontainer, 0);
    }
}

/**---------------------------------------------------------------------*/
/*
 * container functions
 */
/**
 * initialize interface container
 */
netsnmp_container *
netsnmp_access_interface_container_init(u_int flags)
{
    netsnmp_container *container1;

    DEBUGMSGTL(("access:interface:container", "init\n"));

    /*
     * create the containers. one indexed by ifIndex, the other
     * indexed by ifName.
     */
    container1 = netsnmp_container_find("access_interface:table_container");
    if (NULL == container1)
        return NULL;

    container1->container_name = strdup("interface container");
    if (flags & NETSNMP_ACCESS_INTERFACE_INIT_ADDL_IDX_BY_NAME) {
        netsnmp_container *container2 =
            netsnmp_container_find("access_interface_by_name:access_interface:table_container");
        if (NULL == container2)
            return NULL;

        container2->container_name = strdup("interface name container");
        container2->compare = _access_interface_entry_compare_name;
        
        netsnmp_container_add_index(container1, container2);
    }

    return container1;
}

/**
 * load interface information in specified container
 *
 * @param container empty container, or NULL to have one created for you
 * @param load_flags flags to modify behaviour. Examples:
 *                   NETSNMP_ACCESS_INTERFACE_INIT_ADDL_IDX_BY_NAME
 *
 * @retval NULL  error
 * @retval !NULL pointer to container
 */
netsnmp_container*
netsnmp_access_interface_container_load(netsnmp_container* container, u_int load_flags)
{
    int rc;

    DEBUGMSGTL(("access:interface:container", "load\n"));
    netsnmp_assert(1 == _access_interface_init);

    if (NULL == container)
        container = netsnmp_access_interface_container_init(load_flags);
    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_interface\n");
        return NULL;
    }

    rc =  netsnmp_arch_interface_container_load(container, load_flags);
    if (0 != rc) {
        netsnmp_access_interface_container_free(container,
                                                NETSNMP_ACCESS_INTERFACE_FREE_NOFLAGS);
        container = NULL;
    }

    return container;
}

void
netsnmp_access_interface_container_free(netsnmp_container *container, u_int free_flags)
{
    DEBUGMSGTL(("access:interface:container", "free\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR, "invalid container for netsnmp_access_interface_free\n");
        return;
    }

    if(! (free_flags & NETSNMP_ACCESS_INTERFACE_FREE_DONT_CLEAR)) {
        /*
         * free all items.
         */
        CONTAINER_CLEAR(container,
                        (netsnmp_container_obj_func*)_access_interface_entry_release,
                        NULL);
    }

    CONTAINER_FREE(container);
}

/**
 * @retval 0  interface not found
 */
oid
netsnmp_access_interface_index_find(const char *name)
{
    DEBUGMSGTL(("access:interface:find", "index\n"));
    netsnmp_assert(1 == _access_interface_init);

    return netsnmp_arch_interface_index_find(name);
}

/**---------------------------------------------------------------------*/
/*
 * ifentry functions
 */
/**
 */
netsnmp_interface_entry *
netsnmp_access_interface_entry_get_by_index(netsnmp_container *container, oid index)
{
    netsnmp_index   tmp;

    DEBUGMSGTL(("access:interface:entry", "by_index\n"));
    netsnmp_assert(1 == _access_interface_init);

    if (NULL == container) {
        snmp_log(LOG_ERR,
                 "invalid container for netsnmp_access_interface_entry_get_by_index\n");
        return NULL;
    }

    tmp.len = 1;
    tmp.oids = &index;

    return (netsnmp_interface_entry *) CONTAINER_FIND(container, &tmp);
}

/**
 */
netsnmp_interface_entry *
netsnmp_access_interface_entry_get_by_name(netsnmp_container *container,
                                const char *name)
{
    netsnmp_interface_entry tmp;

    DEBUGMSGTL(("access:interface:entry", "by_name\n"));
    netsnmp_assert(1 == _access_interface_init);

    if (NULL == container) {
        snmp_log(LOG_ERR,
                 "invalid container for netsnmp_access_interface_entry_get_by_name\n");
        return NULL;
    }

    if (NULL == container->next) {
        snmp_log(LOG_ERR,
                 "secondary index missing for netsnmp_access_interface_entry_get_by_name\n");
        return NULL;
    }

    tmp.name = NETSNMP_REMOVE_CONST(char *, name);
    return (netsnmp_interface_entry*)CONTAINER_FIND(container->next, &tmp);
}

/**
 * @retval NULL  index not found
 */
const char *
netsnmp_access_interface_name_find(oid index)
{
    DEBUGMSGTL(("access:interface:find", "name\n"));
    netsnmp_assert(1 == _access_interface_init);

    return se_find_label_in_slist("interfaces", index);
}

/**
 */
netsnmp_interface_entry *
netsnmp_access_interface_entry_create(const char *name, oid if_index)
{
    netsnmp_interface_entry *entry =
        SNMP_MALLOC_TYPEDEF(netsnmp_interface_entry);

    DEBUGMSGTL(("access:interface:entry", "create\n"));
    netsnmp_assert(1 == _access_interface_init);

    if(NULL == entry)
        return NULL;

    if(NULL != name)
        entry->name = strdup(name);

    /*
     * get if index, and save name for reverse lookup
     */
    if (0 == if_index)
        entry->index = netsnmp_access_interface_index_find(name);
    else
        entry->index = if_index;
    netsnmp_assert(entry->index != 0);
    _access_interface_entry_save_name(name, entry->index);

    if (name)
        entry->descr = strdup(name);

    /*
     * make some assumptions
     */
    entry->connector_present = 1;

    entry->oid_index.len = 1;
    entry->oid_index.oids = &entry->index;

    return entry;
}

/**
 */
void
netsnmp_access_interface_entry_free(netsnmp_interface_entry * entry)
{
    DEBUGMSGTL(("access:interface:entry", "free\n"));

    if (NULL == entry)
        return;

    /*
     * SNMP_FREE not needed, for any of these, 
     * since the whole entry is about to be freed
     */

    free(entry->old_stats);
    free(entry->name);
    free(entry->descr);
    free(entry->paddr);
    free(entry);
}

#ifndef NETSNMP_FEATURE_REMOVE_INTERFACE_LEGACY
/*
 * Blech - backwards compatible mibII/interfaces style interface
 * functions, so we don't have to update older modules to use
 * the new code to get correct ifIndex values.
 */
#if defined( USING_IF_MIB_IFTABLE_IFTABLE_DATA_ACCESS_MODULE ) && \
    ! defined( NETSNMP_NO_BACKWARDS_COMPATABILITY )

static netsnmp_iterator *it = NULL;
static ifTable_rowreq_ctx *row = NULL;

/**
 * Setup an iterator for scanning the interfaces using the cached entry
 * from if-mib/ifTable.
 */
void
Interface_Scan_Init(void)
{
    netsnmp_container *cont = NULL;
    netsnmp_cache *cache    = NULL; 
    
    cache = netsnmp_cache_find_by_oid(ifTable_oid, ifTable_oid_size);
    if (NULL != cache) {
        netsnmp_cache_check_and_reload(cache);
        cont = (netsnmp_container*) cache->magic;
    }
    
    if (NULL != cont) {
        if (NULL != it)
            ITERATOR_RELEASE(it);
    
        it = CONTAINER_ITERATOR(cont);
    }
   
    if (NULL != it)
        row = (ifTable_rowreq_ctx*)ITERATOR_FIRST(it);
}

int
Interface_Scan_Next(short *index, char *name, netsnmp_interface_entry **entry,
                    void *dc)
{
    int returnIndex = 0;
    int ret;
    if (index)
        returnIndex = *index;

    ret = Interface_Scan_NextInt( &returnIndex, name, entry, dc );
    if (index)
        *index = (returnIndex & 0x8fff);
    return ret;
}

int
Interface_Scan_NextInt(int *index, char *name, netsnmp_interface_entry **entry,
                    void *dc)
{
    netsnmp_interface_entry* e = NULL;

    if (NULL == row)
        return 0;
    
    e = row->data.ifentry;
    if(index)
        *index = e->index;

    if(name)
        strcpy(name, e->name);

    if (entry)
        *entry = e;

    row = (ifTable_rowreq_ctx*) ITERATOR_NEXT(it);

    return 1;
}
#endif /* NETSNMP_NO_BACKWARDS_COMPATABILITY */
#endif /* NETSNMP_FEATURE_REMOVE_INTERFACE_LEGACY */

#ifndef NETSNMP_FEATURE_REMOVE_INTERFACE_ACCESS_ENTRY_SET_ADMIN_STATUS
/**
 *
 * @retval 0   : success
 * @retval < 0 : error
 */
int
netsnmp_access_interface_entry_set_admin_status(netsnmp_interface_entry * entry,
                                                int ifAdminStatus)
{
    int rc;

    DEBUGMSGTL(("access:interface:entry", "set_admin_status\n"));

    if (NULL == entry)
        return -1;

    if ((ifAdminStatus < IFADMINSTATUS_UP) ||
         (ifAdminStatus > IFADMINSTATUS_TESTING))
        return -2;

    rc = netsnmp_arch_set_admin_status(entry, ifAdminStatus);
    if (0 == rc) /* success */
        entry->admin_status = ifAdminStatus;

    return rc;
}
#endif /* NETSNMP_FEATURE_REMOVE_INTERFACE_ACCESS_ENTRY_SET_ADMIN_STATUS */

/**---------------------------------------------------------------------*/
/*
 * Utility routines
 */

/**
 */
static int
_access_interface_entry_compare_name(const void *lhs, const void *rhs)
{
    return strcmp(((const netsnmp_interface_entry *) lhs)->name,
                  ((const netsnmp_interface_entry *) rhs)->name);
}

/**
 */
static void
_access_interface_entry_release(netsnmp_interface_entry * entry, void *context)
{
    netsnmp_access_interface_entry_free(entry);
}

/**
 */
static void
_access_interface_entry_save_name(const char *name, oid index)
{
    int tmp;

    if(NULL == name)
        return;

    tmp = se_find_value_in_slist("interfaces", name);
    if (tmp == SE_DNE) {
        se_add_pair_to_slist("interfaces", strdup(name), index);
        DEBUGMSGTL(("access:interface:ifIndex",
                    "saved ifIndex %" NETSNMP_PRIo "u for %s\n",
                    index, name));
    }
    else
        if (index != (oid)tmp) {
            NETSNMP_LOGONCE((LOG_ERR, "IfIndex of an interface changed. Such " \
                         "interfaces will appear multiple times in IF-MIB.\n"));
            DEBUGMSGTL(("access:interface:ifIndex",
                        "index %" NETSNMP_PRIo "u != tmp for %s\n",
                        index, name));
        }
}

/**
 * update stats
 *
 * @retval  0 : success
 * @retval -1 : error
 */
int
netsnmp_access_interface_entry_update_stats(netsnmp_interface_entry * prev_vals,
                                            netsnmp_interface_entry * new_vals)
{
    DEBUGMSGTL(("access:interface", "check_wrap\n"));
    
    /*
     * sanity checks
     */
    if ((NULL == prev_vals) || (NULL == new_vals) ||
        (NULL == prev_vals->name) || (NULL == new_vals->name) ||
        (0 != strncmp(prev_vals->name, new_vals->name, strlen(prev_vals->name))))
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
         * if we don't have old stats, copy previous stats
         */
        prev_vals->old_stats = SNMP_MALLOC_TYPEDEF(netsnmp_interface_stats);
        if (NULL == prev_vals->old_stats) {
            return -2;
        }
        memcpy(prev_vals->old_stats, &prev_vals->stats, sizeof(prev_vals->stats));
    }

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.ibytes,
                                       &new_vals->stats.ibytes,
                                       &prev_vals->old_stats->ibytes,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCInOctets to 64bits\n"));

        if (new_vals->ns_flags & NETSNMP_INTERFACE_FLAGS_CALCULATE_UCAST) {
            if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.iall,
                                           &new_vals->stats.iall,
                                           &prev_vals->old_stats->iall,
                                           &need_wrap_check))
                DEBUGMSGTL(("access:interface",
                        "Error expanding packet count to 64bits\n"));
        } else {
            if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.iucast,
                                           &new_vals->stats.iucast,
                                           &prev_vals->old_stats->iucast,
                                           &need_wrap_check))
                DEBUGMSGTL(("access:interface",
                        "Error expanding ifHCInUcastPkts to 64bits\n"));
        }

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.iucast,
                                       &new_vals->stats.iucast,
                                       &prev_vals->old_stats->iucast,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCInUcastPkts to 64bits\n"));

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.imcast,
                                       &new_vals->stats.imcast,
                                       &prev_vals->old_stats->imcast,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCInMulticastPkts to 64bits\n"));

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.ibcast,
                                       &new_vals->stats.ibcast,
                                       &prev_vals->old_stats->ibcast,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCInBroadcastPkts to 64bits\n"));

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.obytes,
                                       &new_vals->stats.obytes,
                                       &prev_vals->old_stats->obytes,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCOutOctets to 64bits\n"));

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.oucast,
                                       &new_vals->stats.oucast,
                                       &prev_vals->old_stats->oucast,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCOutUcastPkts to 64bits\n"));

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.omcast,
                                       &new_vals->stats.omcast,
                                       &prev_vals->old_stats->omcast,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCOutMulticastPkts to 64bits\n"));

        if (0 != netsnmp_c64_check32_and_update(&prev_vals->stats.obcast,
                                       &new_vals->stats.obcast,
                                       &prev_vals->old_stats->obcast,
                                       &need_wrap_check))
            DEBUGMSGTL(("access:interface",
                    "Error expanding ifHCOutBroadcastPkts to 64bits\n"));

    /*
     * Copy 32 bit counters
     */
    prev_vals->stats.ierrors = new_vals->stats.ierrors;
    prev_vals->stats.idiscards = new_vals->stats.idiscards;
    prev_vals->stats.iunknown_protos = new_vals->stats.iunknown_protos;
    prev_vals->stats.inucast = new_vals->stats.inucast;
    prev_vals->stats.oerrors = new_vals->stats.oerrors;
    prev_vals->stats.odiscards = new_vals->stats.odiscards;
    prev_vals->stats.oqlen = new_vals->stats.oqlen;
    prev_vals->stats.collisions = new_vals->stats.collisions;
    prev_vals->stats.onucast = new_vals->stats.onucast;

    /*
     * if we've decided we no longer need to check wraps, free old stats
     */
    if (0 == need_wrap_check) {
        SNMP_FREE(prev_vals->old_stats);
    }
    else {
        /*
         * update old stats from new stats.
         * careful - old_stats is a pointer to stats...
         */
        memcpy(prev_vals->old_stats, &new_vals->stats, sizeof(new_vals->stats));
    }
    
    return 0;
}

/**
 * Calculate stats
 *
 * @retval  0 : success
 * @retval -1 : error
 */
int
netsnmp_access_interface_entry_calculate_stats(netsnmp_interface_entry *entry)
{
    DEBUGMSGTL(("access:interface", "calculate_stats\n"));
    if (entry->ns_flags & NETSNMP_INTERFACE_FLAGS_CALCULATE_UCAST) {
        u64Subtract(&entry->stats.iall, &entry->stats.imcast,
                &entry->stats.iucast);
    }
    return 0;
}

/**
 * copy interface entry data (after checking for counter wraps)
 *
 * @retval -2 : malloc failed
 * @retval  0 : no error
 */
int
netsnmp_access_interface_entry_copy(netsnmp_interface_entry * lhs,
                                    netsnmp_interface_entry * rhs)
{
    DEBUGMSGTL(("access:interface", "copy\n"));

    /*
     * update stats
     */
    netsnmp_access_interface_entry_update_stats(lhs, rhs);
    netsnmp_access_interface_entry_calculate_stats(lhs);

    /*
     * update data
     */
    lhs->ns_flags = rhs->ns_flags;
    if (!lhs->descr || !rhs->descr || strcmp(lhs->descr, rhs->descr) != 0) {
        SNMP_FREE(lhs->descr);
        if (rhs->descr) {
            lhs->descr = strdup(rhs->descr);
            if (!lhs->descr)
                return -2;
        }
    }
    lhs->type = rhs->type;
    lhs->speed = rhs->speed;
    lhs->speed_high = rhs->speed_high;
    lhs->retransmit_v6 = rhs->retransmit_v6;
    lhs->retransmit_v4 = rhs->retransmit_v4;
    lhs->reachable_time = rhs->reachable_time;
    lhs->mtu = rhs->mtu;
    lhs->lastchange = rhs->lastchange;
    lhs->discontinuity = rhs->discontinuity;
    lhs->reasm_max_v4 = rhs->reasm_max_v4;
    lhs->reasm_max_v6 = rhs->reasm_max_v6;
    lhs->admin_status = rhs->admin_status;
    lhs->oper_status = rhs->oper_status;
    lhs->promiscuous = rhs->promiscuous;
    lhs->connector_present = rhs->connector_present;
    lhs->forwarding_v6 = rhs->forwarding_v6;
    lhs->os_flags = rhs->os_flags;
    if(lhs->paddr_len == rhs->paddr_len) {
        if(rhs->paddr_len)
            memcpy(lhs->paddr,rhs->paddr,rhs->paddr_len);
    } else {
        SNMP_FREE(lhs->paddr);
        if (rhs->paddr) {
            lhs->paddr = (char*)malloc(rhs->paddr_len);
            if(NULL == lhs->paddr)
                return -2;
            memcpy(lhs->paddr,rhs->paddr,rhs->paddr_len);
        }
    }
    lhs->paddr_len = rhs->paddr_len;
    
    return 0;
}

void
netsnmp_access_interface_entry_guess_speed(netsnmp_interface_entry *entry)
{
    if (entry->type == IANAIFTYPE_ETHERNETCSMACD)
        entry->speed = 10000000;
    else if (entry->type == IANAIFTYPE_SOFTWARELOOPBACK)
        entry->speed = 10000000;
    else if (entry->type == IANAIFTYPE_ISO88025TOKENRING)
        entry->speed = 4000000;
    else
        entry->speed = 0;
    entry->speed_high = entry->speed / 1000000LL;
}

netsnmp_conf_if_list *
netsnmp_access_interface_entry_overrides_get(const char * name)
{
    netsnmp_conf_if_list * if_ptr;

    netsnmp_assert(1 == _access_interface_init);
    if(NULL == name)
        return NULL;

    for (if_ptr = conf_list; if_ptr; if_ptr = if_ptr->next)
        if (!strcmp(if_ptr->name, name))
            break;

    return if_ptr;
}

void
netsnmp_access_interface_entry_overrides(netsnmp_interface_entry *entry)
{
    netsnmp_conf_if_list * if_ptr;

    netsnmp_assert(1 == _access_interface_init);
    if (NULL == entry)
        return;

    /*
     * enforce mib size limit
     */
    if(entry->descr && (strlen(entry->descr) > 255))
        entry->descr[255] = 0;

    if_ptr =
        netsnmp_access_interface_entry_overrides_get(entry->name);
    if (if_ptr) {
        entry->type = if_ptr->type;
        if (if_ptr->speed > 0xffffffff) {
            entry->speed = 0xffffffff;
        } else {
            entry->speed = if_ptr->speed;
        }
        entry->speed_high = if_ptr->speed / 1000000LL;
    }
}

/*
 * ifmib_max_num_ifaces config token
 *
 * Users may configure a maximum number if interfaces for
 * the IF-MIB to include. This is useful in case there are
 * a large number of interfaces (bridges, bonds, SVIs) that
 * can slow things down.
 */
int netsnmp_access_interface_max_reached(const char *name)
{
    if (!name)
        return FALSE;

    if (ifmib_max_num_ifaces == 0)
        /* nothing was set as a max so we include it all */
        return FALSE;

    if (netsnmp_arch_interface_index_find(name) > ifmib_max_num_ifaces)
        /* We have gone over the max configured iface count */
        return TRUE;

    return FALSE;
}

/*
 * include_ifmib_iface_prefix config token
 *
 * If and only if there is an iface prefix name match, we return TRUE.
 * If there are no ifaces defined at all, return 1 so that the
 * default behavior is to include all ifaces (include everything).
 * (Note: including at least one iface prefix means we will only include
 * those iface names that match the prefix and exclude all others.)
 */
int netsnmp_access_interface_include(const char *name)
{
    netsnmp_include_if_list *if_ptr;
#ifdef HAVE_PCRE_H
    int                      found_ndx[3];
#endif

    if (!name)
        return TRUE;

    if (!include_list)
        /*
         * If include_ifmib_iface_prefix was not configured, we should include
         * all interfaces (which is the default).
         */
        return TRUE;


    for (if_ptr = include_list; if_ptr; if_ptr = if_ptr->next) {
#if defined(HAVE_PCRE_H)
        if (pcre_exec(if_ptr->regex_ptr, NULL, name, strlen(name), 0, 0,
                      found_ndx, 3) >= 0)
            return TRUE;
#elif defined(HAVE_REGEX_H)
        if (regexec(if_ptr->regex_ptr, name, 0, NULL, 0) == 0)
            return TRUE;
#else
        if (strncmp(name, if_ptr->name, strlen(if_ptr->name)) == 0)
            return TRUE;
#endif
    }

    return FALSE;
}

/**---------------------------------------------------------------------*/
/*
 * interface config token
 */
/**
 */
static void
_parse_interface_config(const char *token, char *cptr)
{
    netsnmp_conf_if_list   *if_ptr, *if_new;
    char                   *name, *type, *speed, *ecp;
    char                   *st;

    name = strtok_r(cptr, " \t", &st);
    if (!name) {
        config_perror("Missing NAME parameter");
        return;
    }
    type = strtok_r(NULL, " \t", &st);
    if (!type) {
        config_perror("Missing TYPE parameter");
        return;
    }
    speed = strtok_r(NULL, " \t", &st);
    if (!speed) {
        config_perror("Missing SPEED parameter");
        return;
    }
    if_ptr = conf_list;
    while (if_ptr)
        if (strcmp(if_ptr->name, name))
            if_ptr = if_ptr->next;
        else
            break;
    if (if_ptr)
        config_pwarn("Duplicate interface specification");
    if_new = SNMP_MALLOC_TYPEDEF(netsnmp_conf_if_list);
    if (!if_new) {
        config_perror("Out of memory");
        return;
    }
    if_new->speed = strtoull(speed, &ecp, 0);
    if (*ecp) {
        config_perror("Bad SPEED value");
        free(if_new);
        return;
    }
    if_new->type = strtol(type, &ecp, 0);
    if (*ecp || if_new->type < 0) {
        config_perror("Bad TYPE");
        free(if_new);
        return;
    }
    if_new->name = strdup(name);
    if (!if_new->name) {
        config_perror("Out of memory");
        free(if_new);
        return;
    }
    if_new->next = conf_list;
    conf_list = if_new;
}

static void
_free_interface_config(void)
{
    netsnmp_conf_if_list   *if_ptr = conf_list, *if_next;
    while (if_ptr) {
        if_next = if_ptr->next;
        free(NETSNMP_REMOVE_CONST(char *, if_ptr->name));
        free(if_ptr);
        if_ptr = if_next;
    }
    conf_list = NULL;
}

/*
 * Maximum number of interfaces to include in IF-MIB
 */
static void
_parse_ifmib_max_num_ifaces(const char *token, char *cptr)
{
    int temp_max;
    char *name, *st;

    errno = 0;
    name = strtok_r(cptr, " \t", &st);
    if (!name) {
        config_perror("Missing NUMBER parameter");
        return;
    }
    if (sscanf(cptr, "%d", &temp_max) != 1) {
        config_perror("Error converting parameter");
        return;
    }

    ifmib_max_num_ifaces = temp_max;
}


/*
 * include interface config token
 */
static void
_parse_include_if_config(const char *token, char *cptr)
{
    netsnmp_include_if_list *if_ptr, *if_new;
    char                    *name, *st;
#if defined(HAVE_PCRE_H)
    const char              *pcre_error;
    int                     pcre_error_offset;
#elif defined(HAVE_REGEX_H)
    int                     r = 0;
#endif

    name = strtok_r(cptr, " \t", &st);
    if (!name) {
        config_perror("Missing NAME parameter");
        return;
    }

    /* check for duplicate prefix configuration */
    while (name != NULL) {
        for (if_ptr = include_list; if_ptr; if_ptr = if_ptr->next) {
            if (strncmp(name, if_ptr->name, strlen(if_ptr->name)) == 0) {
                config_pwarn("Duplicate include interface prefix specification");
                return;
            }
        }
        /* now save the prefixes */
        if_new = SNMP_MALLOC_TYPEDEF(netsnmp_include_if_list);
        if (!if_new) {
            config_perror("Out of memory");
            goto err;
        }
        if_new->name = strdup(name);
        if (!if_new->name) {
            config_perror("Out of memory");
            goto err;
        }
#if defined(HAVE_PCRE_H)
        if_new->regex_ptr = pcre_compile(if_new->name, 0,  &pcre_error,
                                         &pcre_error_offset, NULL);
        if (!if_new->regex_ptr) {
            config_perror(pcre_error);
            goto err;
        }
#elif defined(HAVE_REGEX_H)
        if_new->regex_ptr = malloc(sizeof(regex_t));
        if (!if_new->regex_ptr) {
            config_perror("Out of memory");
            goto err;
        }
        r = regcomp(if_new->regex_ptr, if_new->name, REG_NOSUB);
        if (r) {
            char buf[BUFSIZ];
            size_t regerror_len = 0;

            regerror_len = regerror(r, if_new->regex_ptr, buf, BUFSIZ);
            if (regerror_len >= BUFSIZ)
                buf[BUFSIZ - 1] = '\0';
            else
                buf[regerror_len] = '\0';
            config_perror(buf);
            goto err;
        }
#endif
        if_new->next = include_list;
        include_list = if_new;
        if_new = NULL;
        name = strtok_r(NULL, " \t", &st);
    }
    return;

err:
    if (if_new) {
#if defined(HAVE_PCRE_H) || defined(HAVE_REGEX_H)
        free(if_new->regex_ptr);
#endif
        free(if_new->name);
    }
    free(if_new);
}

static void
_free_include_if_config(void)
{
    netsnmp_include_if_list   *if_ptr = include_list, *if_next;

    while (if_ptr) {
        if_next = if_ptr->next;
#if defined(HAVE_PCRE_H)
        free(if_ptr->regex_ptr);
#elif defined(HAVE_REGEX_H)
        regfree(if_ptr->regex_ptr);
        free(if_ptr->regex_ptr);
#endif
        free(if_ptr->name);
        free(if_ptr);
        if_ptr = if_next;
    }
    include_list = NULL;
}
