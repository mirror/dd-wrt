#ifndef _AGENTX_SUBAGENT_H
#define _AGENTX_SUBAGENT_H

config_require(agentx/protocol)
config_require(agentx/client)
config_require(agentx/agentx_config)

    void             init_subagent(void);
     int             subagent_pre_init(void);
     int             handle_agentx_packet(int, netsnmp_session *, int,
                                          netsnmp_pdu *, void *);
     SNMPCallback    agentx_register_callback;
     SNMPCallback    agentx_unregister_callback;
     SNMPAlarmCallback agentx_check_session;

     extern netsnmp_session *agentx_session;

#endif                          /* _AGENTX_SUBAGENT_H */
