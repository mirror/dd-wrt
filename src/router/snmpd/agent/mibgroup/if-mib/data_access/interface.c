/*
 *  Interface MIB architecture support
 *
 * $Id: interface.c,v 1.7 2006/09/14 17:15:32 rstory Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "mibII/mibII_common.h"
#include "if-mib/ifTable/ifTable_constants.h"
#include "if-mib/data_access/interface.h"

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_enum.h>
#include <net-snmp/data_access/interface.h>


/**---------------------------------------------------------------------*/
/*
 * local static vars
 */
static netsnmp_conf_if_list *conf_list = NULL;
static int need_wrap_check = -1;
static int _access_interface_init = 0;

/*
 * local static prototypes
 */
static int _access_interface_entry_compare_name(const void *lhs,
                                                const void *rhs);
#ifndef NETSNMP_ACCESS_INTERFACE_NOARCH
static void _access_interface_entry_release(netsnmp_interface_entry * entry,
                                            void *unused);
#endif
static void _access_interface_entry_save_name(const char *name, oid index);
static void _parse_interface_config(const char *token, char *cptr);
static void _free_interface_config(void);

/**---------------------------------------------------------------------*/
/*
 * external per-architecture functions prototypes
 *
 * These shouldn't be called by the general public, so they aren't in
 * the header file.
 */
#ifndef NETSNMP_ACCESS_INTERFACE_NOARCH
extern void netsnmp_arch_interface_init(void);
extern int
netsnmp_arch_interface_container_load(netsnmp_container* container,
                                      u_int load_flags);
extern int
netsnmp_arch_set_admin_status(netsnmp_interface_entry * entry,
                              int ifAdminStatus);
extern int netsnmp_arch_interface_index_find(const char*name);
#endif


/**
 * initialization
 */
void
init_interface(void)
{
    snmpd_register_config_handler("interface", _parse_interface_config,
                                  _free_interface_config,
                                  "name type speed");
}


void
netsnmp_access_interface_init(void)
{
    netsnmp_assert(0 == _access_interface_init); /* who is calling twice? */

    if (1 == _access_interface_init)
        return;

    _access_interface_init = 1;

#ifndef NETSNMP_ACCESS_INTERFACE_NOARCH
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
#endif
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

        container1->container_name = strdup("interface name container");
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
#ifndef NETSNMP_ACCESS_INTERFACE_NOARCH
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
#endif

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

    tmp.name = (char *)name;
    return CONTAINER_FIND(container->next, &tmp);
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
#ifndef NETSNMP_ACCESS_INTERFACE_NOARCH
    if (0 == if_index)
        entry->index = netsnmp_access_interface_index_find(name);
    else
#endif
        entry->index = if_index;
    _access_interface_entry_save_name(name, entry->index);

    entry->descr = strdup(name);

    /*
     * make some assumptions
     */
    entry->connector_present = 1;

    entry->oid_index.len = 1;
    entry->oid_index.oids = (oid *) & entry->index;

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

    if (NULL != entry->old_stats)
        free(entry->old_stats);

    if (NULL != entry->name)
        free(entry->name);

    if (NULL != entry->descr)
        free(entry->descr);

    if (NULL != entry->paddr)
        free(entry->paddr);

    free(entry);
}

/*
 * Blech - backwards compatible mibII/interfaces style interface
 * functions, so we don't have to update older modules to use
 * the new code to get correct ifIndex values.
 */
#if defined( USING_IF_MIB_IFTABLE_IFTABLE_DATA_ACCESS_MODULE ) && \
    ! defined( NETSNMP_NO_BACKWARDS_COMPATABILITY )

static netsnmp_iterator *it = NULL;
static netsnmp_container *c = NULL;
static netsnmp_interface_entry *e = NULL;

/**
 * 
 */
void
Interface_Scan_Init(void)
{
    /*
     * ifTable container shouldn't change, so we shouldn' have to
     * re-fetch it every time.
     */
    if (NULL != c)
        netsnmp_access_interface_container_free(c, 0);

    c = netsnmp_access_interface_container_load(NULL, 0);
    
    if (NULL != c) {
        if (NULL != it)
            ITERATOR_RELEASE(it);
    
        it = CONTAINER_ITERATOR(c);
    }
   
    if (NULL != it)
        e = ITERATOR_FIRST(it);
}

int
Interface_Scan_Next(short *index, char *name, netsnmp_interface_entry **entry,
                    void *dc)
{
    if (NULL == e)
        return 0;

    if(index)
        *index = e->index;

    if(name)
        strcpy(name, e->name);

    if (entry)
        *entry = e;

    e = ITERATOR_NEXT(it);

    return 1;
}
#endif /* NETSNMP_NO_BACKWARDS_COMPATABILITY */


/**
 *
 * @retval 0   : success
 * @retval < 0 : error
 */
#ifndef NETSNMP_ACCESS_INTERFACE_NOARCH
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
#endif

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
    oid tmp;

    if(NULL == name)
        return;

    tmp = se_find_value_in_slist("interfaces", name);
    if (tmp == SE_DNE) {
        se_add_pair_to_slist("interfaces", strdup(name), index);
        DEBUGMSGTL(("access:interface:ifIndex", "saved ifIndex %d for %s\n",
                    index, name));
    }
    else
        netsnmp_assert(index == tmp);
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
        memcpy(prev_vals->old_stats, &prev_vals->stats, sizeof(new_vals->stats));
    }

        netsnmp_c64_check32_and_update(&prev_vals->stats.ibytes,
                                       &new_vals->stats.ibytes,
                                       &prev_vals->old_stats->ibytes,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.iucast,
                                       &new_vals->stats.iucast,
                                       &prev_vals->old_stats->iucast,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.imcast,
                                       &new_vals->stats.imcast,
                                       &prev_vals->old_stats->imcast,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.ibcast,
                                       &new_vals->stats.ibcast,
                                       &prev_vals->old_stats->ibcast,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.obytes,
                                       &new_vals->stats.obytes,
                                       &prev_vals->old_stats->obytes,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.oucast,
                                       &new_vals->stats.oucast,
                                       &prev_vals->old_stats->oucast,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.omcast,
                                       &new_vals->stats.omcast,
                                       &prev_vals->old_stats->omcast,
                                       &need_wrap_check);
        netsnmp_c64_check32_and_update(&prev_vals->stats.obcast,
                                       &new_vals->stats.obcast,
                                       &prev_vals->old_stats->obcast,
                                       &need_wrap_check);
    
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
 * copy interface entry data (after checking for counter wraps)
 *
 * @retval -2 : malloc failed
 * @retval -1 : interfaces not the same
 * @retval  0 : no error
 */
int
netsnmp_access_interface_entry_copy(netsnmp_interface_entry * lhs,
                                    netsnmp_interface_entry * rhs)
{
    DEBUGMSGTL(("access:interface", "copy\n"));
    
    if ((NULL == lhs) || (NULL == rhs) ||
        (NULL == lhs->name) || (NULL == rhs->name) ||
        (0 != strncmp(lhs->name, rhs->name, strlen(rhs->name))))
        return -1;

    /*
     * update stats
     */
    netsnmp_access_interface_entry_update_stats(lhs, rhs);

    /*
     * update data
     */
    lhs->ns_flags = rhs->ns_flags;
    if((NULL != lhs->descr) && (NULL != rhs->descr) &&
       (0 == strcmp(lhs->descr, rhs->descr)))
        ;
    else {
        if (NULL != lhs->descr)
            SNMP_FREE(lhs->descr);
        if (rhs->descr) {
            lhs->descr = strdup(rhs->descr);
            if(NULL == lhs->descr)
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
        if (NULL != lhs->paddr)
            SNMP_FREE(lhs->paddr);
        if (rhs->paddr) {
            lhs->paddr = malloc(rhs->paddr_len);
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
        entry->speed = if_ptr->speed;
    }
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
    if_new->speed = strtoul(speed, &ecp, 0);
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
        free(if_ptr->name);
        free(if_ptr);
        if_ptr = if_next;
    }
    conf_list = NULL;
}
