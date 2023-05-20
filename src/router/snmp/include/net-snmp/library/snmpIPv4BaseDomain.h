/* IPV4 base transport support functions
 */
#ifndef SNMPIPV4BASE_H
#define SNMPIPV4BASE_H

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/library/snmp_transport.h>

config_require(IPBase)

#ifdef __cplusplus
extern          "C" {
#endif

    struct netsnmp_ep;

/*
 * Prototypes
 */

    char *netsnmp_ipv4_fmtaddr(const char *prefix, netsnmp_transport *t,
                               const void *data, int len);
    void netsnmp_ipv4_get_taddr(struct netsnmp_transport_s *t, void **addr,
                                size_t *addr_len);
    int netsnmp_ipv4_ostring_to_sockaddr(struct sockaddr_in *sin,
                                         const void *o, size_t o_len);

/*
 * Convert a "traditional" peername into a sockaddr_in structure which is
 * written to *addr_  Returns 1 if the conversion was successful, or 0 if it
 * failed
 */

    int netsnmp_sockaddr_in(struct sockaddr_in *addr, const char *peername,
                            int remote_port);
    int netsnmp_sockaddr_in2(struct sockaddr_in *addr, const char *inpeername,
                             const char *default_target);
    int
    netsnmp_sockaddr_in3(struct netsnmp_ep *ep,
                         const char *inpeername, const char *default_target);

#ifdef __cplusplus
}
#endif
#endif /* SNMPIPV4BASE_H */
