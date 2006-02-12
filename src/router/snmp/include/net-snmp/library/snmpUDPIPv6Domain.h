#ifndef _SNMPUDPIPV6DOMAIN_H
#define _SNMPUDPIPV6DOMAIN_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>
#include <net-snmp/library/asn1.h>

extern oid      netsnmp_UDPIPv6Domain[]; /* = { ENTERPRISE_MIB, 3, 3, 4 }; */

netsnmp_transport *snmp_udp6_transport(struct sockaddr_in6 *addr,
                                       int local);


/*
 * Convert a "traditional" peername into a sockaddr_in6 structure which is
 * written to *addr.  Returns 1 if the conversion was successful, or 0 if it
 * failed.  
 */

int             netsnmp_sockaddr_in6(struct sockaddr_in6 *addr,
                                     const char *peername,
                                     int remote_port);

void            netsnmp_udp6_agent_config_tokens_register(void);
void            netsnmp_udp6_parse_security(const char *token,
                                            char *param);

int             netsnmp_udp6_getSecName(void *opaque, int olength,
                                        const char *community,
                                        int community_len, char **secname);

/*
 * "Constructor" for transport domain object.  
 */

void            netsnmp_udp6_ctor(void);

#ifdef __cplusplus
}
#endif
#endif/*_SNMPUDPIPV6DOMAIN_H*/
