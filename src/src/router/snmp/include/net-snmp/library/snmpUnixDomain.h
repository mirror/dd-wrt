#ifndef _SNMPUNIXDOMAIN_H
#define _SNMPUNIXDOMAIN_H

#ifdef SNMP_TRANSPORT_UNIX_DOMAIN

#ifdef __cplusplus
extern          "C" {
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_SYS_UN_H
#include <sys/un.h>
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

extern oid netsnmp_UnixDomain[10];   /*  = { ENTERPRISE_MIB, 3, 3, 2 };  */

netsnmp_transport *netsnmp_unix_transport(struct sockaddr_un *addr,
                                          int local);
/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_unix_ctor(void);

#ifdef __cplusplus
}
#endif
#endif                          /*SNMP_TRANSPORT_UNIX_DOMAIN */

#endif/*_SNMPUNIXDOMAIN_H*/
