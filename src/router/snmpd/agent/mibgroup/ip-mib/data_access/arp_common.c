/*
 *  Arp MIB architecture support
 *
 * $Id: arp_common.c,v 1.2 2005/12/11 17:55:01 rstory Exp $
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/data_access/arp.h>

/**---------------------------------------------------------------------*/
/*
 * local static prototypes
 */
static void _access_arp_entry_release(netsnmp_arp_entry * entry,
                                      void *unused);

/**---------------------------------------------------------------------*/
/*
 * external per-architecture functions prototypes
 *
 * These shouldn't be called by the general public, so they aren't in
 * the header file.
 */
extern int
netsnmp_access_arp_container_arch_load(netsnmp_container* container,
                                       u_int load_flags);


/**---------------------------------------------------------------------*/
/*
 * container functions
 */
/**
 */
netsnmp_container *
netsnmp_access_arp_container_init(u_int flags)
{
    netsnmp_container *container1;

    DEBUGMSGTL(("access:arp:container", "init\n"));

    /*
     * create the containers. one indexed by ifIndex, the other
     * indexed by ifName.
     */
    container1 = netsnmp_container_find("access_arp:table_container");
    if (NULL == container1)
        return NULL;
    return container1;
}

/**
 * @retval NULL  error
 * @retval !NULL pointer to container
 */
netsnmp_container*
netsnmp_access_arp_container_load(netsnmp_container* container, u_int load_flags)
{
    int rc;

    DEBUGMSGTL(("access:arp:container", "load\n"));

    if (NULL == container)
        container = netsnmp_container_find("access:arp:table_container");
    if (NULL == container) {
        snmp_log(LOG_ERR, "no container specified/found for access_arp\n");
        return NULL;
    }

    rc =  netsnmp_access_arp_container_arch_load(container, load_flags);
    if (0 != rc) {
        netsnmp_access_arp_container_free(container,
                                          NETSNMP_ACCESS_ARP_FREE_NOFLAGS);
        container = NULL;
    }

    return container;
}

void
netsnmp_access_arp_container_free(netsnmp_container *container, u_int free_flags)
{
    DEBUGMSGTL(("access:arp:container", "free\n"));

    if (NULL == container) {
        snmp_log(LOG_ERR, "invalid container for netsnmp_access_arp_free\n");
        return;
    }

    if(! (free_flags & NETSNMP_ACCESS_ARP_FREE_DONT_CLEAR)) {
        /*
         * free all items.
         */
        CONTAINER_CLEAR(container,
                        (netsnmp_container_obj_func*)_access_arp_entry_release,
                        NULL);
    }

    if(! (free_flags & NETSNMP_ACCESS_ARP_FREE_KEEP_CONTAINER))
        CONTAINER_FREE(container);
}

/**---------------------------------------------------------------------*/
/*
 * arp_entry functions
 */
/**
 */
netsnmp_arp_entry *
netsnmp_access_arp_entry_create(void)
{
    netsnmp_arp_entry *entry =
        SNMP_MALLOC_TYPEDEF(netsnmp_arp_entry);

    entry->oid_index.len = 1;
    entry->oid_index.oids = &entry->ns_arp_index;

    return entry;
}

/**
 */
void
netsnmp_access_arp_entry_free(netsnmp_arp_entry * entry)
{
    free(entry);
}

/**---------------------------------------------------------------------*/
/*
 * Utility routines
 */

/**
 */
void
_access_arp_entry_release(netsnmp_arp_entry * entry, void *context)
{
    netsnmp_access_arp_entry_free(entry);
}
