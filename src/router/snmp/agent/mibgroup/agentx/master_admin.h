#ifndef _AGENTX_MASTER_ADMIN_H
#define _AGENTX_MASTER_ADMIN_H

int             handle_master_agentx_packet(int, netsnmp_session *,
                                            int, netsnmp_pdu *, void *);

int             close_agentx_session(netsnmp_session * session,
                                     int sessid);

#endif                          /* _AGENTX_MASTER_ADMIN_H */
