#ifndef _SNMPTCPDOMAIN_H
#define _SNMPTCPDOMAIN_H

#ifdef SNMP_TRANSPORT_TCP_DOMAIN

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

extern oid netsnmp_snmpTCPDomain[8];	/*  = { 1, 3, 6, 1, 3, 91, 1, 1 };  */

netsnmp_transport *netsnmp_tcp_transport(struct sockaddr_in *addr, int local);

/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_tcp_ctor(void);

#ifdef __cplusplus
}
#endif
#endif                          /*SNMP_TRANSPORT_TCP_DOMAIN */

#endif/*_SNMPTCPDOMAIN_H*/
