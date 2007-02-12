/*
 *  AgentX Configuration
 */
#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "snmpd.h"
#include "agentx/agentx_config.h"

#ifdef USING_AGENTX_MASTER_MODULE
void
agentx_parse_master(const char *token, char *cptr)
{
    int             i = -1;
    char            buf[BUFSIZ];

    if (!strcmp(cptr, "agentx") ||
        !strcmp(cptr, "all") ||
        !strcmp(cptr, "yes") || !strcmp(cptr, "on")) {
        i = 1;
        snmp_log(LOG_INFO, "Turning on AgentX master support.\n");
    } else if (!strcmp(cptr, "no") || !strcmp(cptr, "off"))
        i = 0;
    else
        i = atoi(cptr);

    if (i < 0 || i > 1) {
        sprintf(buf, "master '%s' unrecognised", cptr);
        config_perror(buf);
    } else
        netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_AGENTX_MASTER, i);
}
#endif                          /* USING_AGENTX_MASTER_MODULE */

void
agentx_parse_agentx_socket(const char *token, char *cptr)
{
    DEBUGMSGTL(("agentx/config", "port spec: %s\n", cptr));
    netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET, cptr);
}

void
agentx_parse_agentx_timeout(const char *token, char *cptr)
{
    int x = atoi(cptr);
    DEBUGMSGTL(("agentx/config/timeout", "%s\n", cptr));
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                       NETSNMP_DS_AGENT_AGENTX_TIMEOUT, x * ONE_SEC);
}

void
agentx_parse_agentx_retries(const char *token, char *cptr)
{
    int x = atoi(cptr);
    DEBUGMSGTL(("agentx/config/retries", "%s\n", cptr));
    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID,
                       NETSNMP_DS_AGENT_AGENTX_RETRIES, x);
}

void
init_agentx_config(void)
{
    /*
     * Don't set this up as part of the per-module initialisation.
     * Delay this until the 'init_master_agent()' routine is called,
     *   so that the config settings have been processed.
     * This means that we can use a config directive to determine
     *   whether or not to run as an AgentX master.
     */
#ifdef USING_AGENTX_MASTER_MODULE
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE) == MASTER_AGENT)
        snmpd_register_config_handler("master",
                                      agentx_parse_master, NULL,
                                      "specify 'agentx' for AgentX support");
#endif                          /* USING_AGENTX_MASTER_MODULE */
    snmpd_register_config_handler("agentxsocket",
                                  agentx_parse_agentx_socket, NULL,
                                  "AgentX bind address");
    snmpd_register_config_handler("agentxRetries",
                                  agentx_parse_agentx_retries, NULL,
                                  "AgentX Retries");
    snmpd_register_config_handler("agentxTimeout",
                                  agentx_parse_agentx_timeout, NULL,
                                  "AgentX Timeout (seconds)");
}
