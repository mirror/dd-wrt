#ifndef _SNMPTCPIPV6DOMAIN_H
#define _SNMPTCPIPV6DOMAIN_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

extern oid      netsnmp_TCPIPv6Domain[]; /* = { ENTERPRISE_MIB, 3, 3, 5 }; */

netsnmp_transport *netsnmp_tcp6_transport(struct sockaddr_in6 *addr, 
					  int local);

/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_tcp6_ctor(void);

#ifdef __cplusplus
}
#endif
#endif/*_SNMPTCPIPV6DOMAIN_H*/
