/*
 * arp data access header
 *
 * $Id: arp.h,v 1.3 2006/09/15 00:48:47 tanders Exp $
 */
#ifndef NETSNMP_ACCESS_ARP_H
#define NETSNMP_ACCESS_ARP_H

#ifdef __cplusplus
extern          "C" {
#endif

/**---------------------------------------------------------------------*/
#if defined( NETSNMP_ENABLE_IPV6 )
#   define NETSNMP_ACCESS_ARP_IPADDR_BUF_SIZE 16
#else
#   define NETSNMP_ACCESS_ARP_IPADDR_BUF_SIZE 4
#endif

/** MAC address is 6 - do we want to support anything larger? */
#define NETSNMP_ACCESS_ARP_PHYSADDR_BUF_SIZE 6

/*************************************************************
 * constants for enums for the MIB node
 * inetNetToMediaType (INTEGER / ASN_INTEGER)
 *
 * since a Textual Convention may be referenced more than once in a
 * MIB, protect againt redefinitions of the enum values.
 */
#ifndef inetNetToMediaType_ENUMS
#define inetNetToMediaType_ENUMS

#define INETNETTOMEDIATYPE_OTHER  1
#define INETNETTOMEDIATYPE_INVALID  2
#define INETNETTOMEDIATYPE_DYNAMIC  3
#define INETNETTOMEDIATYPE_STATIC  4
#define INETNETTOMEDIATYPE_LOCAL  5

#endif                          /* inetNetToMediaType_ENUMS */

/*************************************************************
 * constants for enums for the MIB node
 * inetNetToMediaState (INTEGER / ASN_INTEGER)
 *
 * since a Textual Convention may be referenced more than once in a
 * MIB, protect againt redifinitions of the enum values.
 */
#ifndef inetNetToMediaState_ENUMS
#define inetNetToMediaState_ENUMS

#define INETNETTOMEDIASTATE_REACHABLE  1
#define INETNETTOMEDIASTATE_STALE  2
#define INETNETTOMEDIASTATE_DELAY  3
#define INETNETTOMEDIASTATE_PROBE  4
#define INETNETTOMEDIASTATE_INVALID  5
#define INETNETTOMEDIASTATE_UNKNOWN  6
#define INETNETTOMEDIASTATE_INCOMPLETE  7

#endif                          /* inetNetToMediaState_ENUMS */

/**---------------------------------------------------------------------*/
/*
 * structure definitions
 */
/*
 * netsnmp_arp_entry
 *   - primary arp structure for both ipv4 & ipv6
 */
typedef struct netsnmp_arp_s {

   netsnmp_index oid_index;      /* MUST BE FIRST!! for container use */
   oid           ns_arp_index;  /* arbitrary index */

   int       flags; /* for net-snmp use */

   oid       if_index;

   u_char    arp_physaddress[NETSNMP_ACCESS_ARP_PHYSADDR_BUF_SIZE];
   u_char    arp_ipaddress[NETSNMP_ACCESS_ARP_IPADDR_BUF_SIZE];

   u_char    arp_physaddress_len;/* phys address len, 6 */
   u_char    arp_ipaddress_len;  /* ip address len, 4 | 16 */
   u_char    arp_type;           /* inetNetToMediaType 1-5 */
   u_char    arp_state;          /* inetNetToMediaState 1-7 */

} netsnmp_arp_entry;


/**---------------------------------------------------------------------*/
/*
 * ACCESS function prototypes
 */
/*
 * ifcontainer init
 */
netsnmp_container * netsnmp_access_arp_container_init(u_int init_flags);
#define NETSNMP_ACCESS_ARP_INIT_NOFLAGS               0x0000

/*
 * ifcontainer load and free
 */
netsnmp_container*
netsnmp_access_arp_container_load(netsnmp_container* container,
                                    u_int load_flags);
#define NETSNMP_ACCESS_ARP_LOAD_NOFLAGS               0x0000

void netsnmp_access_arp_container_free(netsnmp_container *container,
                                         u_int free_flags);
#define NETSNMP_ACCESS_ARP_FREE_NOFLAGS               0x0000
#define NETSNMP_ACCESS_ARP_FREE_DONT_CLEAR            0x0001
#define NETSNMP_ACCESS_ARP_FREE_KEEP_CONTAINER        0x0002


/*
 * create/free a arp+entry
 */
netsnmp_arp_entry *
netsnmp_access_arp_entry_create(void);

void netsnmp_access_arp_entry_free(netsnmp_arp_entry * entry);

/*
 * find entry in container
 */
/** not yet */

/**---------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* NETSNMP_ACCESS_ARP_H */
