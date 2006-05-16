/*
 * agent_read_config.c
 */

#include <net-snmp/net-snmp-config.h>

#include <sys/types.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif
#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif
#ifdef INET6
#if HAVE_NETINET_IP6_H
#include <netinet/ip6.h>
#endif
#endif
#if HAVE_SYS_QUEUE_H
#include <sys/queue.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#if HAVE_SYS_SOCKETVAR_H
#ifndef dynix
#include <sys/socketvar.h>
#else
#include <sys/param.h>
#endif
#endif
#elif HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif
#if HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif
#ifdef INET6
#if HAVE_NETINET6_IP6_VAR_H
#include <netinet6/ip6_var.h>
#endif
#endif
#if HAVE_NETINET_IN_PCB_H
#include <netinet/in_pcb.h>
#endif
#if HAVE_INET_MIB2_H
#include <inet/mib2.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#ifdef HAVE_GRP_H
#include <grp.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "mibgroup/struct.h"
#include <net-snmp/agent/agent_trap.h>
#include "snmpd.h"
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/table.h>
#include <net-snmp/agent/table_iterator.h>
#include <net-snmp/agent/table_data.h>
#include <net-snmp/agent/table_dataset.h>
#include "mib_module_includes.h"

char            dontReadConfigFiles;
char           *optconfigfile;

#ifdef HAVE_UNISTD_H
void
snmpd_set_agent_user(const char *token, char *cptr)
{
#if defined(HAVE_GETPWNAM) && defined(HAVE_PWD_H)
    struct passwd  *info;
#endif

    if (cptr[0] == '#') {
        char           *ecp;
        int             uid;
        uid = strtoul(cptr + 1, &ecp, 10);
        if (*ecp != 0) {
            config_perror("Bad number");
	} else {
	    netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_USERID, uid);
	}
    }
#if defined(HAVE_GETPWNAM) && defined(HAVE_PWD_H)
    else if ((info = getpwnam(cptr)) != NULL) {
        netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID, 
			   NETSNMP_DS_AGENT_USERID, info->pw_uid);
    } else {
        config_perror("User not found in passwd database");
    }
#endif
}

void
snmpd_set_agent_group(const char *token, char *cptr)
{
#if defined(HAVE_GETGRNAM) && defined(HAVE_GRP_H)
    struct group   *info;
#endif

    if (cptr[0] == '#') {
        char           *ecp;
        int             gid = strtoul(cptr + 1, &ecp, 10);
        if (*ecp != 0) {
            config_perror("Bad number");
	} else {
            netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_GROUPID, gid);
	}
    }
#if defined(HAVE_GETGRNAM) && defined(HAVE_GRP_H)
    else if ((info = getgrnam(cptr)) != NULL) {
        netsnmp_ds_set_int(NETSNMP_DS_APPLICATION_ID, 
			   NETSNMP_DS_AGENT_GROUPID, info->gr_gid);
    } else {
        config_perror("Group not found in group database");
    }
#endif
}
#endif

void
snmpd_set_agent_address(const char *token, char *cptr)
{
    char            buf[SPRINT_MAX_LEN];
    char           *ptr;

    /*
     * has something been specified before? 
     */
    ptr = netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID, 
				NETSNMP_DS_AGENT_PORTS);

    if (ptr) {
        /*
         * append to the older specification string 
         */
        sprintf(buf, "%s,%s", ptr, cptr);
    } else {
        strcpy(buf, cptr);
    }

    DEBUGMSGTL(("snmpd_ports", "port spec: %s\n", buf));
    netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID, 
			  NETSNMP_DS_AGENT_PORTS, buf);
}

void
init_agent_read_config(const char *app)
{
    if (app != NULL) {
        netsnmp_ds_set_string(NETSNMP_DS_LIBRARY_ID, 
			      NETSNMP_DS_LIB_APPTYPE, app);
    } else {
        app = netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, 
				    NETSNMP_DS_LIB_APPTYPE);
    }

    register_app_config_handler("authtrapenable",
                                snmpd_parse_config_authtrap, NULL,
                                "1 | 2\t\t(1 = enable, 2 = disable)");
    register_app_config_handler("pauthtrapenable",
                                snmpd_parse_config_authtrap, NULL, NULL);


    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_ROLE) == MASTER_AGENT) {
        register_app_config_handler("trapsink",
                                    snmpd_parse_config_trapsink,
                                    snmpd_free_trapsinks,
                                    "host [community] [port]");
        register_app_config_handler("trap2sink",
                                    snmpd_parse_config_trap2sink, NULL,
                                    "host [community] [port]");
        register_app_config_handler("informsink",
                                    snmpd_parse_config_informsink, NULL,
                                    "host [community] [port]");
        register_app_config_handler("trapsess",
                                    snmpd_parse_config_trapsess, NULL,
                                    "[snmpcmdargs] host");
    }
    register_app_config_handler("trapcommunity",
                                snmpd_parse_config_trapcommunity,
                                snmpd_free_trapcommunity,
                                "community-string");
#ifdef HAVE_UNISTD_H
    register_app_config_handler("agentuser",
                                snmpd_set_agent_user, NULL, "userid");
    register_app_config_handler("agentgroup",
                                snmpd_set_agent_group, NULL, "groupid");
#endif
    register_app_config_handler("agentaddress",
                                snmpd_set_agent_address, NULL,
                                "SNMP bind address");
    register_app_config_handler("table",
                                netsnmp_config_parse_table_set, NULL,
                                "tableoid");
    register_app_config_handler("add_row", netsnmp_config_parse_add_row,
                                NULL, "indexes... values...");
    netsnmp_ds_register_config(ASN_BOOLEAN, app, "quit", 
			       NETSNMP_DS_APPLICATION_ID,
			       NETSNMP_DS_AGENT_QUIT_IMMEDIATELY);
    netsnmp_ds_register_config(ASN_BOOLEAN, app, "leave_pidfile", 
			       NETSNMP_DS_APPLICATION_ID,
			       NETSNMP_DS_AGENT_LEAVE_PIDFILE);
    netsnmp_init_handler_conf();

#include "mib_module_dot_conf.h"
#ifdef TESTING
    print_config_handlers();
#endif
}

void
update_config(void)
{
    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_PRE_UPDATE_CONFIG, NULL);
    free_config();
    read_configs();
}


void
snmpd_register_config_handler(const char *token,
                              void (*parser) (const char *, char *),
                              void (*releaser) (void), const char *help)
{
    DEBUGMSGTL(("snmpd_register_app_config_handler",
                "registering .conf token for \"%s\"\n", token));
    register_app_config_handler(token, parser, releaser, help);
}

void
snmpd_unregister_config_handler(const char *token)
{
    unregister_app_config_handler(token);
}

/*
 * this function is intended for use by mib-modules to store permenant
 * configuration information generated by sets or persistent counters 
 */
void
snmpd_store_config(const char *line)
{
    read_app_config_store(line);
}
