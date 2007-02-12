#ifndef _SNMPCALLBACKDOMAIN_H
#define _SNMPCALLBACKDOMAIN_H

#ifdef __cplusplus
extern          "C" {
#endif

#include <net-snmp/library/snmp_transport.h>

typedef struct netsnmp_callback_pass_s {
    int             return_transport_num;
    netsnmp_pdu    *pdu;
    struct netsnmp_callback_pass_s *next;
} netsnmp_callback_pass;

typedef struct netsnmp_callback_info_s {
    int             linkedto;
    void           *parent_data;
    netsnmp_callback_pass *data;
    int             callback_num;
    int             pipefds[2];
} netsnmp_callback_info;

netsnmp_transport *netsnmp_callback_transport(int);
int             netsnmp_callback_hook_parse(netsnmp_session * sp,
                                            netsnmp_pdu *pdu,
                                            u_char * packetptr,
                                            size_t len);
int             netsnmp_callback_hook_build(netsnmp_session * sp,
                                            netsnmp_pdu *pdu,
                                            u_char * ptk, size_t * len);
int             netsnmp_callback_check_packet(u_char * pkt, size_t len);
netsnmp_pdu    *netsnmp_callback_create_pdu(netsnmp_transport *transport,
                                            void *opaque, size_t olength);
netsnmp_session *netsnmp_callback_open(int attach_to,
                                       int (*return_func) (int op,
                                                           netsnmp_session
                                                           * session,
                                                           int reqid,
                                                           netsnmp_pdu
                                                           *pdu,
                                                           void *magic),
                                       int (*fpre_parse) (netsnmp_session
                                                          *,
                                                          struct
                                                          netsnmp_transport_s
                                                          *, void *, int),
                                       int (*fpost_parse) (netsnmp_session
                                                           *,
                                                           netsnmp_pdu *,
                                                           int));

#ifdef __cplusplus
}
#endif
#endif/*_SNMPCALLBACKDOMAIN_H*/
