#include <net-snmp/net-snmp-config.h>

#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "proxy.h"

static struct simple_proxy *proxies = NULL;

oid             testoid[] = { 1, 3, 6, 1, 4, 1, 2021, 8888, 1 };

/*
 * this must be standardized somewhere, right? 
 */
#define MAX_ARGS 128

char           *context_string;

static void
proxyOptProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'n':
                optind++;
                if (optind < argc) {
                    context_string = argv[optind - 1];
                } else {
                    config_perror("No context name passed to -Cn");
                }
                break;
            default:
                config_perror("unknown argument passed to -C");
                break;
            }
        }
        break;
    default:
        break;
        /*
         * shouldn't get here 
         */
    }
}

void
proxy_parse_config(const char *token, char *line)
{
    /*
     * proxy args [base-oid] [remap-to-remote-oid] 
     */

    netsnmp_session session, *ss;
    struct simple_proxy *newp, **listpp;
    char            args[MAX_ARGS][SPRINT_MAX_LEN], *argv[MAX_ARGS];
    int             argn, arg;
    char           *cp;
    netsnmp_handler_registration *reg;

    context_string = NULL;

    DEBUGMSGTL(("proxy_config", "entering\n"));

    /*
     * create the argv[] like array 
     */
    strcpy(argv[0] = args[0], "snmpd-proxy");   /* bogus entry for getopt() */
    for (argn = 1, cp = line; cp && argn < MAX_ARGS;
         cp = copy_nword(cp, argv[argn] = args[argn++], SPRINT_MAX_LEN)) {
    }

    for (arg = 0; arg < argn; arg++) {
        DEBUGMSGTL(("proxy_args", "final args: %d = %s\n", arg,
                    argv[arg]));
    }

    DEBUGMSGTL(("proxy_config", "parsing args: %d\n", argn));
    arg = snmp_parse_args(argn, argv, &session, "C:", proxyOptProc);
    DEBUGMSGTL(("proxy_config", "done parsing args\n"));

    if (arg >= argn) {
        config_perror("missing base oid");
        return;
    }

    SOCK_STARTUP;
    /*
     * usm_set_reportErrorOnUnknownID(0); 
     *
     * hack, stupid v3 ASIs. 
     */
    /*
     * XXX: on a side note, we don't really need to be a reference
     * platform any more so the proper thing to do would be to fix
     * snmplib/snmpusm.c to pass in the pdu type to usm_process_incoming
     * so this isn't needed. 
     */
    ss = snmp_open(&session);
    /*
     * usm_set_reportErrorOnUnknownID(1); 
     */
    if (ss == NULL) {
        /*
         * diagnose snmp_open errors with the input netsnmp_session pointer 
         */
        snmp_sess_perror("snmpget", &session);
        SOCK_CLEANUP;
        return;
    }

    newp = (struct simple_proxy *) calloc(1, sizeof(struct simple_proxy));

    newp->sess = ss;
    DEBUGMSGTL(("proxy_init", "name = %s\n", args[arg]));
    newp->name_len = MAX_OID_LEN;
    if (!snmp_parse_oid(args[arg++], newp->name, &newp->name_len)) {
        snmp_perror("proxy");
        config_perror("illegal proxy oid specified\n");
        return;
    }

    if (arg < argn) {
        DEBUGMSGTL(("proxy_init", "base = %s\n", args[arg]));
        newp->base_len = MAX_OID_LEN;
        if (!snmp_parse_oid(args[arg++], newp->base, &newp->base_len)) {
            snmp_perror("proxy");
            config_perror("illegal variable name specified (base oid)\n");
            return;
        }
    }

    DEBUGMSGTL(("proxy_init", "registering at: "));
    DEBUGMSGOID(("proxy_init", newp->name, newp->name_len));
    DEBUGMSG(("proxy_init", "\n"));

    /*
     * add to our chain 
     */
    /*
     * must be sorted! 
     */
    listpp = &proxies;
    while (*listpp &&
           snmp_oid_compare(newp->name, newp->name_len,
                            (*listpp)->name, (*listpp)->name_len) > 0) {
        listpp = &((*listpp)->next);
    }

    /*
     * listpp should be next in line from us. 
     */
    if (*listpp) {
        /*
         * make our next in the link point to the current link 
         */
        newp->next = *listpp;
    }
    /*
     * replace current link with us 
     */
    *listpp = newp;

    reg = netsnmp_create_handler_registration("proxy",
                                              proxy_handler,
                                              newp->name,
                                              newp->name_len,
                                              HANDLER_CAN_RWRITE);
    reg->handler->myvoid = newp;
    if (context_string)
        reg->contextName = strdup(context_string);

    netsnmp_register_handler(reg);
}

void
proxy_free_config(void)
{
    struct simple_proxy *rm;

    /*
     * XXX: finish me (needs unregister_mib()) 
     */
    return;

    while (proxies) {
        rm = proxies;
        proxies = rm->next;
        SNMP_FREE(rm->variables);
        snmp_close(rm->sess);
        SNMP_FREE(rm);
    }
}

void
init_proxy(void)
{
    snmpd_register_config_handler("proxy", proxy_parse_config,
                                  proxy_free_config,
                                  "[snmpcmd args] host oid [remoteoid]");
}

int
proxy_handler(netsnmp_mib_handler *handler,
              netsnmp_handler_registration *reginfo,
              netsnmp_agent_request_info *reqinfo,
              netsnmp_request_info *requests)
{

    netsnmp_pdu    *pdu;
    struct simple_proxy *sp;
    oid            *ourname;
    size_t          ourlength;
    netsnmp_request_info *request = requests;

    DEBUGMSGTL(("proxy", "proxy handler starting, mode = %d\n",
                reqinfo->mode));

    switch (reqinfo->mode) {
    case MODE_GET:
    case MODE_GETNEXT:
    case MODE_GETBULK:         /* WWWXXX */
        pdu = snmp_pdu_create(reqinfo->mode);
        break;

    case MODE_SET_COMMIT:
        pdu = snmp_pdu_create(SNMP_MSG_SET);
        break;

    default:
        snmp_log(LOG_WARNING, "unsupported mode for proxy called\n");
        return SNMP_ERR_NOERROR;
    }

    sp = (struct simple_proxy *) handler->myvoid;

    if (!pdu || !sp) {
        netsnmp_set_request_error(reqinfo, requests, SNMP_ERR_GENERR);
        return SNMP_ERR_NOERROR;
    }

    while (request) {
        ourname = request->requestvb->name;
        ourlength = request->requestvb->name_length;

        if (sp->base_len > 0) {
            if ((ourlength - sp->name_len + sp->base_len) > MAX_OID_LEN) {
                /*
                 * too large 
                 */
                snmp_log(LOG_ERR,
                         "proxy oid request length is too long\n");
                return SNMP_ERR_NOERROR;
            }
            /*
             * suffix appended? 
             */
            DEBUGMSGTL(("proxy", "length=%d, base_len=%d, name_len=%d\n",
                        ourlength, sp->base_len, sp->name_len));
            if (ourlength > (int) sp->name_len)
                memcpy(&(sp->base[sp->base_len]), &(ourname[sp->name_len]),
                       sizeof(oid) * (ourlength - sp->name_len));
            ourlength = ourlength - sp->name_len + sp->base_len;
            ourname = sp->base;
        }

        snmp_pdu_add_variable(pdu, ourname, ourlength,
                              request->requestvb->type,
                              request->requestvb->val.string,
                              request->requestvb->val_len);
        request->delegated = 1;
        request = request->next;
    }

    /*
     * send the request out 
     */
    DEBUGMSGTL(("proxy", "sending pdu\n"));
    snmp_async_send(sp->sess, pdu, proxy_got_response,
                    netsnmp_create_delegated_cache(handler, reginfo,
                                                   reqinfo, requests,
                                                   (void *) sp));
    return SNMP_ERR_NOERROR;
}

int
proxy_got_response(int operation, netsnmp_session * sess, int reqid,
                   netsnmp_pdu *pdu, void *cb_data)
{
    netsnmp_delegated_cache *cache = (netsnmp_delegated_cache *) cb_data;
    netsnmp_request_info *requests, *request;
    netsnmp_variable_list *vars, *var;

    struct simple_proxy *sp;
    oid             myname[MAX_OID_LEN];
    size_t          myname_len = MAX_OID_LEN;

    cache = netsnmp_handler_check_cache(cache);

    if (!cache) {
        DEBUGMSGTL(("proxy", "a proxy request was no longer valid.\n"));
        return SNMP_ERR_NOERROR;
    }

    requests = cache->requests;


    sp = (struct simple_proxy *) cache->localinfo;

    if (!sp) {
        DEBUGMSGTL(("proxy", "a proxy request was no longer valid.\n"));
        return SNMP_ERR_NOERROR;
    }

    switch (operation) {
    case NETSNMP_CALLBACK_OP_TIMED_OUT:
        /*
         * WWWXXX: don't leave requests delayed if operation is
         * something like TIMEOUT 
         */
        DEBUGMSGTL(("proxy", "got timed out... requests = %08p\n", requests));

        netsnmp_handler_mark_requests_as_delegated(requests,
						   REQUEST_IS_NOT_DELEGATED);
        netsnmp_set_request_error(cache->reqinfo, requests, /* XXXWWW: should be index = 0 */
				  SNMP_ERR_GENERR);
        netsnmp_free_delegated_cache(cache);
        return 0;

    case NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE:
        vars = pdu->variables;

        /*
         * update the original request varbinds with the results 
         */
        for (var = vars, request = requests;
             request && var;
             request = request->next, var = var->next_variable) {
            snmp_set_var_typed_value(request->requestvb, var->type,
                                     var->val.string, var->val_len);

            DEBUGMSGTL(("proxy", "got response... "));
            DEBUGMSGOID(("proxy", var->name, var->name_length));
            DEBUGMSG(("proxy", "\n"));
            request->delegated = 0;

            /*
             * copy the oid it belongs to 
             */
            if (sp->base_len &&
                (var->name_length < sp->base_len ||
                 snmp_oid_compare(var->name, sp->base_len, sp->base,
                                  sp->base_len) != 0)) {
                DEBUGMSGTL(("proxy", "out of registered range... "));
                DEBUGMSGOID(("proxy", var->name, sp->base_len));
                DEBUGMSG(("proxy", " (%d) != ", sp->base_len));
                DEBUGMSGOID(("proxy", sp->base, sp->base_len));
                DEBUGMSG(("proxy", "\n"));

                continue;
            } else if (!sp->base_len &&
                       (var->name_length < sp->name_len ||
                        snmp_oid_compare(var->name, sp->name_len, sp->name,
                                         sp->name_len) != 0)) {
                DEBUGMSGTL(("proxy", "out of registered base range...\n"));
                /*
                 * or not if its out of our search range 
                 */
                continue;
            } else {
                if (sp->base_len) {
                    /*
                     * XXX: oid size maxed? 
                     */
                    memcpy(myname, sp->name, sizeof(oid) * sp->name_len);
                    myname_len =
                        sp->name_len + var->name_length - sp->base_len;
                    if (myname_len > MAX_OID_LEN) {
                        snmp_log(LOG_WARNING,
                                 "proxy OID return length too long.\n");
                        netsnmp_set_request_error(cache->reqinfo, requests,
                                                  SNMP_ERR_GENERR);
                        if (pdu)
                            snmp_free_pdu(pdu);
                        netsnmp_free_delegated_cache(cache);
                        return 1;
                    }

                    if (var->name_length > sp->base_len)
                        memcpy(&myname[sp->name_len],
                               &var->name[sp->base_len],
                               sizeof(oid) * (var->name_length -
                                              sp->base_len));
                    snmp_set_var_objid(request->requestvb, myname,
                                       myname_len);
                } else {
                    snmp_set_var_objid(request->requestvb, var->name,
                                       var->name_length);
                }
            }
        }

        if (request || var) {
            /*
             * ack, this is bad.  The # of varbinds don't match and
             * there is no way to fix the problem 
             */
            if (pdu)
                snmp_free_pdu(pdu);
            snmp_log(LOG_ERR,
                     "response to proxy request illegal.  We're screwed.\n");
            netsnmp_set_request_error(cache->reqinfo, requests,
                                      SNMP_ERR_GENERR);
        }

        /* fix bulk_to_next operations */
        if (cache->reqinfo->mode == MODE_GETBULK)
            netsnmp_bulk_to_next_fix_requests(requests);
        
        /*
         * free the response 
         */
        if (pdu && 0)
            snmp_free_pdu(pdu);
	break;

    default:
        DEBUGMSGTL(("proxy", "no response received: op = %d\n",
                    operation));
	break;
    }

    netsnmp_free_delegated_cache(cache);
    return 1;
}
