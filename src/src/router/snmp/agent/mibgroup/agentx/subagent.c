/*
 *  AgentX sub-agent
 */
#include <net-snmp/net-snmp-config.h>

#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
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
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_assert.h>

#include "snmpd.h"
#include "agentx/protocol.h"
#include "agentx/client.h"
#include <net-snmp/agent/agent_callbacks.h>
#include <net-snmp/agent/agent_trap.h>
#ifdef USING_MIBII_SYSORTABLE_MODULE
#include "mibII/sysORTable.h"
#endif

#include "subagent.h"

static SNMPCallback subagent_register_ping_alarm;
static SNMPAlarmCallback agentx_reopen_session;
void            agentx_register_callbacks(netsnmp_session * s);
void            agentx_unregister_callbacks(netsnmp_session * ss);
int             handle_subagent_response(int op, netsnmp_session * session,
                                         int reqid, netsnmp_pdu *pdu,
                                         void *magic);
int             handle_subagent_set_response(int op,
                                             netsnmp_session * session,
                                             int reqid, netsnmp_pdu *pdu,
                                             void *magic);

typedef struct _net_snmpsubagent_magic_s {
    int             original_command;
    netsnmp_session *session;
    netsnmp_variable_list *ovars;
} ns_subagent_magic;

struct agent_netsnmp_set_info {
    int             transID;
    int             mode;
    int             errstat;
    time_t          uptime;
    netsnmp_session *sess;
    netsnmp_variable_list *var_list;

    struct agent_netsnmp_set_info *next;
};

static struct agent_netsnmp_set_info *Sets = NULL;

netsnmp_session *agentx_callback_sess = NULL;
extern int      callback_master_num;

void
init_subagent(void)
{
    if (agentx_callback_sess == NULL) {
        agentx_callback_sess = netsnmp_callback_open(callback_master_num,
                                                     handle_subagent_response,
                                                     NULL, NULL);
        DEBUGMSGTL(("agentx/subagent", "init_subagent sess %08x\n",
                    agentx_callback_sess));
    }
}


struct agent_netsnmp_set_info *
save_set_vars(netsnmp_session * ss, netsnmp_pdu *pdu)
{
    struct agent_netsnmp_set_info *ptr;
    struct timeval  now;
    extern struct timeval starttime;

    ptr = (struct agent_netsnmp_set_info *)
        malloc(sizeof(struct agent_netsnmp_set_info));
    if (ptr == NULL)
        return NULL;

    /*
     * Save the important information
     */
    ptr->transID = pdu->transid;
    ptr->sess = ss;
    ptr->mode = SNMP_MSG_INTERNAL_SET_RESERVE1;
    gettimeofday(&now, NULL);
    ptr->uptime = calculate_time_diff(&now, &starttime);

    ptr->var_list = snmp_clone_varbind(pdu->variables);
    if (ptr->var_list == NULL) {
        free(ptr);
        return NULL;
    }

    ptr->next = Sets;
    Sets = ptr;

    return ptr;
}

struct agent_netsnmp_set_info *
restore_set_vars(netsnmp_session * sess, netsnmp_pdu *pdu)
{
    struct agent_netsnmp_set_info *ptr;

    for (ptr = Sets; ptr != NULL; ptr = ptr->next)
        if (ptr->sess == sess && ptr->transID == pdu->transid)
            break;

    if (ptr == NULL || ptr->var_list == NULL)
        return NULL;

    pdu->variables = snmp_clone_varbind(ptr->var_list);
    if (pdu->variables == NULL)
        return NULL;

    return ptr;
}


void
free_set_vars(netsnmp_session * ss, netsnmp_pdu *pdu)
{
    struct agent_netsnmp_set_info *ptr, *prev = NULL;

    for (ptr = Sets; ptr != NULL; ptr = ptr->next) {
        if (ptr->sess == ss && ptr->transID == pdu->transid) {
            if (prev)
                prev->next = ptr->next;
            else
                Sets = ptr->next;
            snmp_free_varbind(ptr->var_list);
            free(ptr);
            return;
        }
        prev = ptr;
    }
}

extern netsnmp_session *main_session;   /* from snmp_agent.c */

int
handle_agentx_packet(int operation, netsnmp_session * session, int reqid,
                     netsnmp_pdu *pdu, void *magic)
{
    struct agent_netsnmp_set_info *asi = NULL;
    snmp_callback   mycallback;
    netsnmp_pdu    *internal_pdu = NULL;
    void           *retmagic = NULL;
    ns_subagent_magic *smagic = NULL;

    if (operation == NETSNMP_CALLBACK_OP_DISCONNECT) {
        int             period =
            netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL);
        DEBUGMSGTL(("agentx/subagent",
                    "transport disconnect indication\n"));
        /*
         * Deregister the ping alarm, if any, and invalidate all other
         * references to this session.  
         */
        if (session->securityModel != SNMP_DEFAULT_SECMODEL) {
            snmp_alarm_unregister(session->securityModel);
        }
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_INDEX_STOP, (void *) session);
        agentx_unregister_callbacks(session);
        remove_trap_session(session);
        register_mib_detach();
        main_session = NULL;
        if (period != 0) {
            /*
             * Pings are enabled, so periodically attempt to re-establish contact 
             * with the master agent.  Don't worry about the handle,
             * agentx_reopen_session unregisters itself if it succeeds in talking 
             * to the master agent.  
             */
            snmp_alarm_register(period, SA_REPEAT, agentx_reopen_session,
                                NULL);
        }
        return 0;
    } else if (operation != NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
        DEBUGMSGTL(("agentx/subagent", "unexpected callback op %d\n",
                    operation));
        return 1;
    }

    /*
     * ok, we have a pdu from the net. Modify as needed 
     */

    DEBUGMSGTL(("agentx/subagent", "handling agentx request (req=0x%x,trans="
                "0x%x,sess=0x%x)\n", pdu->reqid,pdu->transid, pdu->sessid));
    pdu->version = AGENTX_VERSION_1;
    pdu->flags |= UCD_MSG_FLAG_ALWAYS_IN_VIEW;

    if (pdu->command == AGENTX_MSG_GET
        || pdu->command == AGENTX_MSG_GETNEXT
        || pdu->command == AGENTX_MSG_GETBULK) {
        smagic =
            (ns_subagent_magic *) calloc(1, sizeof(ns_subagent_magic));
        if (smagic == NULL) {
            DEBUGMSGTL(("agentx/subagent", "couldn't malloc() smagic\n"));
            return 1;
        }
        smagic->original_command = pdu->command;
        smagic->session = session;
        smagic->ovars = NULL;
        retmagic = (void *) smagic;
    }

    switch (pdu->command) {
    case AGENTX_MSG_GET:
        DEBUGMSGTL(("agentx/subagent", "  -> get\n"));
        pdu->command = SNMP_MSG_GET;
        mycallback = handle_subagent_response;
        break;

    case AGENTX_MSG_GETNEXT:
        DEBUGMSGTL(("agentx/subagent", "  -> getnext\n"));
        pdu->command = SNMP_MSG_GETNEXT;

        /*
         * We have to save a copy of the original variable list here because
         * if the master agent has requested scoping for some of the varbinds
         * that information is stored there.  
         */

        smagic->ovars = snmp_clone_varbind(pdu->variables);
        DEBUGMSGTL(("agentx/subagent", "saved variables\n"));
        mycallback = handle_subagent_response;
        break;

    case AGENTX_MSG_GETBULK:
        /*
         * WWWXXX 
         */
        DEBUGMSGTL(("agentx/subagent", "  -> getbulk\n"));
        pdu->command = SNMP_MSG_GETBULK;

        /*
         * We have to save a copy of the original variable list here because
         * if the master agent has requested scoping for some of the varbinds
         * that information is stored there.  
         */

        smagic->ovars = snmp_clone_varbind(pdu->variables);
        DEBUGMSGTL(("agentx/subagent", "saved variables at %p\n",
                    smagic->ovars));
        mycallback = handle_subagent_response;
        break;

    case AGENTX_MSG_RESPONSE:
        DEBUGMSGTL(("agentx/subagent", "  -> response\n"));
        return 1;

    case AGENTX_MSG_TESTSET:
        /*
         * XXXWWW we have to map this twice to both RESERVE1 and RESERVE2 
         */
        DEBUGMSGTL(("agentx/subagent", "  -> testset\n"));
        asi = save_set_vars(session, pdu);
        if (asi == NULL) {
            snmp_log(LOG_WARNING, "save_set_vars() failed\n");
            return 1;
        }
        asi->mode = pdu->command = SNMP_MSG_INTERNAL_SET_RESERVE1;
        mycallback = handle_subagent_set_response;
        retmagic = asi;
        break;

    case AGENTX_MSG_COMMITSET:
        DEBUGMSGTL(("agentx/subagent", "  -> commitset\n"));
        asi = restore_set_vars(session, pdu);
        if (asi == NULL) {
            snmp_log(LOG_WARNING, "restore_set_vars() failed\n");
            return 1;
        }
        if (asi->mode != SNMP_MSG_INTERNAL_SET_RESERVE2) {
            snmp_log(LOG_WARNING,
                     "dropping bad AgentX request (wrong mode %d)\n",
                     asi->mode);
            return 1;
        }
        asi->mode = pdu->command = SNMP_MSG_INTERNAL_SET_ACTION;
        mycallback = handle_subagent_set_response;
        retmagic = asi;
        break;

    case AGENTX_MSG_CLEANUPSET:
        DEBUGMSGTL(("agentx/subagent", "  -> cleanupset\n"));
        asi = restore_set_vars(session, pdu);
        if (asi == NULL) {
            snmp_log(LOG_WARNING, "restore_set_vars() failed\n");
            return 1;
        }
        if (asi->mode == SNMP_MSG_INTERNAL_SET_RESERVE1 ||
            asi->mode == SNMP_MSG_INTERNAL_SET_RESERVE2) {
            asi->mode = pdu->command = SNMP_MSG_INTERNAL_SET_FREE;
        } else if (asi->mode == SNMP_MSG_INTERNAL_SET_ACTION) {
            asi->mode = pdu->command = SNMP_MSG_INTERNAL_SET_COMMIT;
        } else {
            snmp_log(LOG_WARNING,
                     "dropping bad AgentX request (wrong mode %d)\n",
                     asi->mode);
            return 1;
        }
        mycallback = handle_subagent_set_response;
        retmagic = asi;
        break;

    case AGENTX_MSG_UNDOSET:
        DEBUGMSGTL(("agentx/subagent", "  -> undoset\n"));
        asi = restore_set_vars(session, pdu);
        if (asi == NULL) {
            snmp_log(LOG_WARNING, "restore_set_vars() failed\n");
            return 1;
        }
        asi->mode = pdu->command = SNMP_MSG_INTERNAL_SET_UNDO;
        mycallback = handle_subagent_set_response;
        retmagic = asi;
        break;

    default:
        DEBUGMSGTL(("agentx/subagent", "  -> unknown command %d (%02x)\n",
                    pdu->command, pdu->command));
        return 0;
    }

    /*
     * submit the pdu to the internal handler 
     */

    /*
     * We have to clone the PDU here, because when we return from this
     * callback, sess_process_packet will free(pdu), but this call also
     * free()s its argument PDU.  
     */

    internal_pdu = snmp_clone_pdu(pdu);
    snmp_async_send(agentx_callback_sess, internal_pdu, mycallback,
                    retmagic);
    return 1;
}

int
handle_subagent_response(int op, netsnmp_session * session, int reqid,
                         netsnmp_pdu *pdu, void *magic)
{
    ns_subagent_magic *smagic = (ns_subagent_magic *) magic;
    netsnmp_variable_list *u = NULL, *v = NULL;
    int             rc = 0;

    if (op != NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE || magic == NULL) {
        return 1;
    }

    pdu = snmp_clone_pdu(pdu);
    DEBUGMSGTL(("agentx/subagent",
                "handling AgentX response (cmd 0x%02x orig_cmd 0x%02x)\n",
                pdu->command, smagic->original_command));

    if (pdu->command == SNMP_MSG_INTERNAL_SET_FREE ||
        pdu->command == SNMP_MSG_INTERNAL_SET_UNDO ||
        pdu->command == SNMP_MSG_INTERNAL_SET_COMMIT) {
        free_set_vars(smagic->session, pdu);
    }

    if (smagic->original_command == AGENTX_MSG_GETNEXT) {
        DEBUGMSGTL(("agentx/subgaent",
                    "do getNext scope processing %p %p\n", smagic->ovars,
                    pdu->variables));
        for (u = smagic->ovars, v = pdu->variables; u != NULL && v != NULL;
             u = u->next_variable, v = v->next_variable) {
            if (snmp_oid_compare
                (u->val.objid, u->val_len / sizeof(oid), nullOid,
                 nullOidLen) != 0) {
                /*
                 * The master agent requested scoping for this variable.  
                 */
                rc = snmp_oid_compare(v->name, v->name_length,
                                      u->val.objid,
                                      u->val_len / sizeof(oid));
                DEBUGMSGTL(("agentx/subagent", "result "));
                DEBUGMSGOID(("agentx/subagent", v->name, v->name_length));
                DEBUGMSG(("agentx/subagent", " scope to "));
                DEBUGMSGOID(("agentx/subagent",
                             u->val.objid, u->val_len / sizeof(oid)));
                DEBUGMSG(("agentx/subagent", " result %d\n", rc));

                if (rc >= 0) {
                    /*
                     * The varbind is out of scope.  From RFC2741, p. 66: "If
                     * the subagent cannot locate an appropriate variable,
                     * v.name is set to the starting OID, and the VarBind is
                     * set to `endOfMibView'".  
                     */
                    snmp_set_var_objid(v, u->name, u->name_length);
                    snmp_set_var_typed_value(v, SNMP_ENDOFMIBVIEW, 0, 0);
                    DEBUGMSGTL(("agentx/subagent",
                                "scope violation -- return endOfMibView\n"));
                }
            } else {
                DEBUGMSGTL(("agentx/subagent", "unscoped var\n"));
            }
        }
    }

    /*
     * XXXJBPN: similar for GETBULK but the varbinds can get re-ordered I
     * think which makes it er more difficult.  
     */

    if (smagic->ovars != NULL) {
        snmp_free_varbind(smagic->ovars);
    }

    pdu->command = AGENTX_MSG_RESPONSE;
    pdu->version = smagic->session->version;

    if (!snmp_send(smagic->session, pdu)) {
        snmp_free_pdu(pdu);
    }
    DEBUGMSGTL(("agentx/subagent", "  FINISHED\n"));
    free(smagic);
    return 1;
}

int
handle_subagent_set_response(int op, netsnmp_session * session, int reqid,
                             netsnmp_pdu *pdu, void *magic)
{
    netsnmp_session *retsess;
    struct agent_netsnmp_set_info *asi;

    if (op != NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE || magic == NULL) {
        return 1;
    }

    DEBUGMSGTL(("agentx/subagent",
                "handling agentx subagent set response (mode=%d,req=0x%x,"
                "trans=0x%x,sess=0x%x)\n",
                pdu->command, pdu->reqid,pdu->transid, pdu->sessid));
    pdu = snmp_clone_pdu(pdu);

    asi = (struct agent_netsnmp_set_info *) magic;
    retsess = asi->sess;
    asi->errstat = pdu->errstat;

    if (asi->mode == SNMP_MSG_INTERNAL_SET_RESERVE1) {
        /*
         * reloop for RESERVE2 mode, an internal only agent mode 
         */
        /*
         * XXX: check exception statuses of reserve1 first 
         */
        if (!pdu->errstat) {
            asi->mode = pdu->command = SNMP_MSG_INTERNAL_SET_RESERVE2;
            snmp_async_send(agentx_callback_sess, pdu,
                            handle_subagent_set_response, asi);
            DEBUGMSGTL(("agentx/subagent",
                        "  going from RESERVE1 -> RESERVE2\n"));
            return 1;
        }
    } else {
        if (asi->mode == SNMP_MSG_INTERNAL_SET_FREE ||
            asi->mode == SNMP_MSG_INTERNAL_SET_UNDO ||
            asi->mode == SNMP_MSG_INTERNAL_SET_COMMIT) {
            free_set_vars(retsess, pdu);
        }
        pdu->variables = NULL;  /* the variables were added by us */
    }

    netsnmp_assert(retsess != NULL);
    pdu->command = AGENTX_MSG_RESPONSE;
    pdu->version = retsess->version;

    if (!snmp_send(retsess, pdu)) {
        snmp_free_pdu(pdu);
    }
    DEBUGMSGTL(("agentx/subagent", "  FINISHED\n"));
    return 1;
}



int
agentx_registration_callback(int majorID, int minorID, void *serverarg,
                             void *clientarg)
{
    struct register_parameters *reg_parms =
        (struct register_parameters *) serverarg;
    netsnmp_session *agentx_ss = (netsnmp_session *) clientarg;

    if (minorID == SNMPD_CALLBACK_REGISTER_OID)
        return agentx_register(agentx_ss,
                               reg_parms->name, reg_parms->namelen,
                               reg_parms->priority,
                               reg_parms->range_subid,
                               reg_parms->range_ubound, reg_parms->timeout,
                               reg_parms->flags);
    else
        return agentx_unregister(agentx_ss,
                                 reg_parms->name, reg_parms->namelen,
                                 reg_parms->priority,
                                 reg_parms->range_subid,
                                 reg_parms->range_ubound);
}


#ifdef USING_MIBII_SYSORTABLE_MODULE
int
agentx_sysOR_callback(int majorID, int minorID, void *serverarg,
                      void *clientarg)
{
    struct register_sysOR_parameters *reg_parms =
        (struct register_sysOR_parameters *) serverarg;
    netsnmp_session *agentx_ss = (netsnmp_session *) clientarg;

    if (minorID == SNMPD_CALLBACK_REG_SYSOR)
        return agentx_add_agentcaps(agentx_ss,
                                    reg_parms->name, reg_parms->namelen,
                                    reg_parms->descr);
    else
        return agentx_remove_agentcaps(agentx_ss,
                                       reg_parms->name,
                                       reg_parms->namelen);
}
#endif


static int
subagent_shutdown(int majorID, int minorID, void *serverarg, void *clientarg)
{
    netsnmp_session *thesession = (netsnmp_session *)clientarg;
    DEBUGMSGTL(("agentx/subagent", "shutting down session....\n"));
    if (thesession == NULL) {
	DEBUGMSGTL(("agentx/subagent", "Empty session to shutdown\n"));
	main_session = NULL;
	return 0;
    }
    agentx_close_session(thesession, AGENTX_CLOSE_SHUTDOWN);
    snmp_close(thesession);
    main_session = NULL;
    DEBUGMSGTL(("agentx/subagent", "shut down finished.\n"));
    return 1;
}



/*
 * Register all the "standard" AgentX callbacks for the given session.  
 */

void
agentx_register_callbacks(netsnmp_session * s)
{
    DEBUGMSGTL(("agentx/subagent",
                "registering callbacks for session %p\n", s));
    snmp_register_callback(SNMP_CALLBACK_LIBRARY,
                           SNMP_CALLBACK_POST_READ_CONFIG,
                           subagent_register_ping_alarm, s);
    snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_SHUTDOWN,
                           subagent_shutdown, s);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_REGISTER_OID,
                           agentx_registration_callback, s);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_UNREGISTER_OID,
                           agentx_registration_callback, s);
#ifdef USING_MIBII_SYSORTABLE_MODULE
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_REG_SYSOR,
                           agentx_sysOR_callback, s);
    snmp_register_callback(SNMP_CALLBACK_APPLICATION,
                           SNMPD_CALLBACK_UNREG_SYSOR,
                           agentx_sysOR_callback, s);
#endif
}

/*
 * Unregister all the callbacks associated with this session.  
 */

void
agentx_unregister_callbacks(netsnmp_session * ss)
{
    DEBUGMSGTL(("agentx/subagent",
                "unregistering callbacks for session %p\n", ss));
    snmp_unregister_callback(SNMP_CALLBACK_LIBRARY,
                             SNMP_CALLBACK_POST_READ_CONFIG,
                             subagent_register_ping_alarm, ss, 1);
    snmp_unregister_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_SHUTDOWN,
                             subagent_shutdown, ss, 1);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_REGISTER_OID,
                             agentx_registration_callback, ss, 1);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_UNREGISTER_OID,
                             agentx_registration_callback, ss, 1);
#ifdef USING_MIBII_SYSORTABLE_MODULE
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_REG_SYSOR,
                             agentx_sysOR_callback, ss, 1);
    snmp_unregister_callback(SNMP_CALLBACK_APPLICATION,
                             SNMPD_CALLBACK_UNREG_SYSOR,
                             agentx_sysOR_callback, ss, 1);
#endif

}

/*
 * Open a session to the master agent.  
 */
int
subagent_open_master_session(void)
{
    netsnmp_session sess;

    DEBUGMSGTL(("agentx/subagent", "opening session...\n"));

    if (main_session) {
        snmp_log(LOG_WARNING,
                 "AgentX session to master agent attempted to be re-opened.");
        return -1;
    }

    snmp_sess_init(&sess);
    sess.version = AGENTX_VERSION_1;
    sess.retries = SNMP_DEFAULT_RETRIES;
    sess.timeout = SNMP_DEFAULT_TIMEOUT;
    sess.flags |= SNMP_FLAGS_STREAM_SOCKET;
    if (netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET)) {
        sess.peername =
            netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET);
    } else {
        sess.peername = strdup(AGENTX_SOCKET);
    }

    sess.local_port = 0;        /* client */
    sess.remote_port = AGENTX_PORT;     /* default port */
    sess.callback = handle_agentx_packet;
    sess.authenticator = NULL;
    main_session = snmp_open_ex(&sess, NULL, agentx_parse, NULL, NULL,
                                agentx_realloc_build, agentx_check_packet);

    if (main_session == NULL) {
        /*
         * Diagnose snmp_open errors with the input
         * netsnmp_session pointer.  
         */
        if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_NO_CONNECTION_WARNINGS)) {
            if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_NO_ROOT_ACCESS)) {
                netsnmp_sess_log_error(LOG_WARNING,
                                       "Error: Failed to connect to the agentx master agent",
                                       &sess);
            } else {
                snmp_sess_perror
                    ("Error: Failed to connect to the agentx master agent",
                     &sess);
            }
        }
        return -1;
    }

    if (agentx_open_session(main_session) < 0) {
        snmp_close(main_session);
        main_session = NULL;
        return -1;
    }

    if (add_trap_session(main_session, AGENTX_MSG_NOTIFY, 1,
                         AGENTX_VERSION_1)) {
        DEBUGMSGTL(("agentx/subagent", " trap session registered OK\n"));
    } else {
        DEBUGMSGTL(("agentx/subagent",
                    "trap session registration failed\n"));
        snmp_close(main_session);
        main_session = NULL;
        return -1;
    }

    agentx_register_callbacks(main_session);

    snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                        SNMPD_CALLBACK_INDEX_START, (void *) main_session);

    DEBUGMSGTL(("agentx/subagent", "opening session...  DONE (%p)\n",
                main_session));

    return 0;
}

/*
 * returns non-zero on error 
 */
int
subagent_pre_init(void)
{
    DEBUGMSGTL(("agentx/subagent", "initializing....\n"));

    /*
     * set up callbacks to initiate master agent pings for this session 
     */
    netsnmp_ds_register_config(ASN_INTEGER,
                       netsnmp_ds_get_string(NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_APPTYPE),
                       "agentxPingInterval",
                       NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL);


    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE) != SUB_AGENT) {
        return 0;
    }

    /*
     * if a valid ping interval has been defined, call agentx_reopen_session
     * * to try to connect to master or setup a ping alarm if it couldn't
     * * succeed 
     */
    if (netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL) > 0)
        agentx_reopen_session(0, NULL);
    else                        /* if no ping interval was set up, just try to connect once */
        subagent_open_master_session();
    if (!main_session)
        return -1;

    DEBUGMSGTL(("agentx/subagent", "initializing....  DONE\n"));

    return 0;
}


/*
 * Alarm callback function to open a session to the master agent.  If a
 * transport disconnection callback occurs, indicating that the master agent
 * has died (or there has been some strange communication problem), this
 * alarm is called repeatedly to try to re-open the connection.  
 */

void
agentx_reopen_session(unsigned int clientreg, void *clientarg)
{
    DEBUGMSGTL(("agentx/subagent", "agentx_reopen_session(%d) called\n",
                clientreg));

    if (subagent_open_master_session() == 0) {
        /*
         * Successful.  Delete the alarm handle if one exists.  
         */
        if (clientreg != 0) {
            snmp_alarm_unregister(clientreg);
        }

        /*
         * Reregister all our nodes.  
         */
        register_mib_reattach();

        /*
         * Register a ping alarm (if need be).  
         */
        subagent_register_ping_alarm(0, 0, 0, main_session);
    } else {
        if (clientreg == 0) {
            /*
             * Register a reattach alarm for later 
             */
            subagent_register_ping_alarm(0, 0, 0, main_session);
        }
    }
}

/*
 * If a valid session is passed in (through clientarg), register a
 * ping handler to ping it frequently, else register an attempt to try
 * and open it again later. 
 */

static int
subagent_register_ping_alarm(int majorID, int minorID,
                             void *serverarg, void *clientarg)
{

    netsnmp_session *ss = (netsnmp_session *) clientarg;
    int             ping_interval =
        netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_AGENTX_PING_INTERVAL);

    if (!ping_interval)         /* don't do anything if not setup properly */
        return 0;

    /*
     * register a ping alarm, if desired 
     */
    if (ss) {
        if (ss->securityModel != SNMP_DEFAULT_SECMODEL) {
            DEBUGMSGTL(("agentx/subagent",
                        "unregister existing alarm %d\n",
                        ss->securityModel));
            snmp_alarm_unregister(ss->securityModel);
        }

        DEBUGMSGTL(("agentx/subagent",
                    "register ping alarm every %d seconds\n",
                    ping_interval));
        /*
         * we re-use the securityModel parameter for an alarm stash,
         * since agentx doesn't need it 
         */
        ss->securityModel = snmp_alarm_register(ping_interval, SA_REPEAT,
                                                agentx_check_session, ss);
    } else {
        /*
         * attempt to open it later instead 
         */
        DEBUGMSGTL(("agentx/subagent",
                    "subagent not properly attached, postponing registration till later....\n"));
        snmp_alarm_register(ping_interval, SA_REPEAT,
                            agentx_reopen_session, NULL);
    }
    return 0;
}

/*
 * check a session validity for connectivity to the master agent.  If
 * not functioning, close and start attempts to reopen the session 
 */
void
agentx_check_session(unsigned int clientreg, void *clientarg)
{
    netsnmp_session *ss = (netsnmp_session *) clientarg;
    if (!ss) {
        if (clientreg)
            snmp_alarm_unregister(clientreg);
        return;
    }
    DEBUGMSGTL(("agentx/subagent", "checking status of session %p\n", ss));

    if (!agentx_send_ping(ss)) {
        snmp_log(LOG_WARNING,
                 "AgentX master agent failed to respond to ping.  Attempting to re-register.\n");
        /*
         * master agent disappeared?  Try and re-register.
         * close first, just to be sure .
         */
        agentx_unregister_callbacks(ss);
        agentx_close_session(ss, AGENTX_CLOSE_TIMEOUT);
        snmp_alarm_unregister(clientreg);       /* delete ping alarm timer */
        snmp_call_callbacks(SNMP_CALLBACK_APPLICATION,
                            SNMPD_CALLBACK_INDEX_STOP, (void *) ss);
        snmp_close(main_session);
        main_session = NULL;
        agentx_reopen_session(0, NULL);
    } else {
        DEBUGMSGTL(("agentx/subagent", "session %p responded to ping\n",
                    ss));
    }
}
