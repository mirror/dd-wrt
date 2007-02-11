/*
 * systemstats data access header
 *
 * $Id: systemstats.h,v 1.3 2005/11/16 19:51:15 rstory Exp $
 */
#ifndef NETSNMP_ACCESS_SYSTEMSTATS_H
#define NETSNMP_ACCESS_SYSTEMSTATS_H

# ifdef __cplusplus
extern          "C" {
#endif

/**---------------------------------------------------------------------*/
/*
 * structure definitions
 */


/*
 * netsnmp_systemstats_entry
 */
typedef struct netsnmp_systemstats_s {

   netsnmp_index oid_index;   /* MUST BE FIRST!! for container use */
   oid           ns_ip_version;

   int       flags; /* for net-snmp use */

   /*
    * mib related data (considered for
    *  netsnmp_access_systemstats_entry_update)
    */
   netsnmp_ipstats stats;

   /** old_stats is used in netsnmp_access_interface_entry_update_stats */
   netsnmp_ipstats *old_stats;

} netsnmp_systemstats_entry;


/**---------------------------------------------------------------------*/
/*
 * ACCESS function prototypes
 */
/*
 * init
 */
netsnmp_container * netsnmp_access_systemstats_container_init(u_int init_flags);
#define NETSNMP_ACCESS_SYSTEMSTATS_INIT_NOFLAGS               0x0000
#define NETSNMP_ACCESS_SYSTEMSTATS_INIT_ADDL_IDX_BY_ADDR      0x0001

/*
 * load and free
 */
netsnmp_container*
netsnmp_access_systemstats_container_load(netsnmp_container* container,
                                    u_int load_flags);
#define NETSNMP_ACCESS_SYSTEMSTATS_LOAD_NOFLAGS               0x0000

void netsnmp_access_systemstats_container_free(netsnmp_container *container,
                                         u_int free_flags);
#define NETSNMP_ACCESS_SYSTEMSTATS_FREE_NOFLAGS               0x0000
#define NETSNMP_ACCESS_SYSTEMSTATS_FREE_DONT_CLEAR            0x0001
#define NETSNMP_ACCESS_SYSTEMSTATS_FREE_KEEP_CONTAINER        0x0002


/*
 * create/free an entry
 */
netsnmp_systemstats_entry *
netsnmp_access_systemstats_entry_create(int version);

void netsnmp_access_systemstats_entry_free(netsnmp_systemstats_entry * entry);

/*
 * update/compare
 */
int
netsnmp_access_systemstats_entry_update(netsnmp_systemstats_entry *old, 
                                        netsnmp_systemstats_entry *new_val);


/**---------------------------------------------------------------------*/

# ifdef __cplusplus
}
#endif

#endif /* NETSNMP_ACCESS_SYSTEMSTATS_H */
