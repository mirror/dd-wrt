/*
 * snmp_agent.c
 *
 * Simple Network Management Protocol (RFC 1067).
 */
/***********************************************************
	Copyright 1988, 1989 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/

#include <net-snmp/net-snmp-config.h>

#include <sys/types.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
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
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <errno.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#define SNMP_NEED_REQUEST_LIST
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/library/snmp_assert.h>

#ifdef USE_LIBWRAP
#include <syslog.h>
#include <tcpd.h>
#include <syslog.h>
int             allow_severity = LOG_INFO;
int             deny_severity = LOG_WARNING;
#endif

#include "snmpd.h"
#include "mibgroup/struct.h"
#include "mibgroup/util_funcs.h"
#include <net-snmp/agent/mib_module_config.h>

#ifdef USING_AGENTX_PROTOCOL_MODULE
#include "agentx/protocol.h"
#endif

#ifdef USING_AGENTX_MASTER_MODULE
#include "agentx/master.h"
#endif

#define SNMP_ADDRCACHE_SIZE 10

struct addrCache {
    char           *addr;
    enum { SNMP_ADDRCACHE_UNUSED = 0,
        SNMP_ADDRCACHE_USED = 1,
        SNMP_ADDRCACHE_OLD = 2
    } status;
};

static struct addrCache addrCache[SNMP_ADDRCACHE_SIZE];
int             lastAddrAge = 0;
int             log_addresses = 0;



typedef struct _agent_nsap {
    int             handle;
    netsnmp_transport *t;
    void           *s;          /*  Opaque internal session pointer.  */
    struct _agent_nsap *next;
} agent_nsap;

static agent_nsap *agent_nsap_list = NULL;
static netsnmp_agent_session *agent_session_list = NULL;
static netsnmp_agent_session *netsnmp_processing_set = NULL;
netsnmp_agent_session *agent_delegated_list = NULL;
netsnmp_agent_session *netsnmp_agent_queued_list = NULL;


int             netsnmp_agent_check_packet(netsnmp_session *,
                                           struct netsnmp_transport_s *,
                                           void *, int);
int             netsnmp_agent_check_parse(netsnmp_session *, netsnmp_pdu *,
                                          int);
void            delete_subnetsnmp_tree_cache(netsnmp_agent_session *asp);
int             handle_pdu(netsnmp_agent_session *asp);
int             netsnmp_handle_request(netsnmp_agent_session *asp,
                                       int status);
int             netsnmp_wrap_up_request(netsnmp_agent_session *asp,
                                        int status);
int             check_delayed_request(netsnmp_agent_session *asp);
int             handle_getnext_loop(netsnmp_agent_session *asp);
int             handle_set_loop(netsnmp_agent_session *asp);

int             netsnmp_check_queued_chain_for(netsnmp_agent_session *asp);
int             netsnmp_add_queued(netsnmp_agent_session *asp);
int             netsnmp_remove_from_delegated(netsnmp_agent_session *asp);


static int      current_globalid = 0;

int
netsnmp_allocate_globalcacheid(void)
{
    return ++current_globalid;
}

int
netsnmp_get_local_cachid(netsnmp_cachemap *cache_store, int globalid)
{
    while (cache_store != NULL) {
        if (cache_store->globalid == globalid)
            return cache_store->cacheid;
        cache_store = cache_store->next;
    }
    return -1;
}

netsnmp_cachemap *
netsnmp_get_or_add_local_cachid(netsnmp_cachemap **cache_store,
                                int globalid, int localid)
{
    netsnmp_cachemap *tmpp;

    tmpp = SNMP_MALLOC_TYPEDEF(netsnmp_cachemap);
    if (*cache_store) {
        tmpp->next = *cache_store;
        *cache_store = tmpp;
    } else {
        *cache_store = tmpp;
    }

    tmpp->globalid = globalid;
    tmpp->cacheid = localid;
    return tmpp;
}

void
netsnmp_free_cachemap(netsnmp_cachemap *cache_store)
{
    netsnmp_cachemap *tmpp;
    while (cache_store) {
        tmpp = cache_store;
        cache_store = cache_store->next;
        free(tmpp);
    }
}


typedef struct agent_set_cache_s {
    /*
     * match on these 2 
     */
    int             transID;
    netsnmp_session *sess;

    /*
     * store this info 
     */
    netsnmp_tree_cache *treecache;
    int             treecache_len;
    int             treecache_num;

    int             vbcount;
    netsnmp_request_info *requests;
    netsnmp_data_list *agent_data;

    /*
     * list 
     */
    struct agent_set_cache_s *next;
} agent_set_cache;

static agent_set_cache *Sets = NULL;

agent_set_cache *
save_set_cache(netsnmp_agent_session *asp)
{
    agent_set_cache *ptr;

    if (!asp->reqinfo || !asp->pdu)
        return NULL;

    ptr = SNMP_MALLOC_TYPEDEF(agent_set_cache);
    if (ptr == NULL)
        return NULL;

    /*
     * Save the important information 
     */
    ptr->transID = asp->pdu->transid;
    ptr->sess = asp->session;
    ptr->treecache = asp->treecache;
    ptr->treecache_len = asp->treecache_len;
    ptr->treecache_num = asp->treecache_num;
    ptr->agent_data = asp->reqinfo->agent_data;
    ptr->requests = asp->requests;
    ptr->vbcount = asp->vbcount;

    /*
     * make the agent forget about what we've saved 
     */
    asp->treecache = NULL;
    asp->reqinfo->agent_data = NULL;
    asp->pdu->variables = NULL;
    asp->requests = NULL;

    ptr->next = Sets;
    Sets = ptr;

    return ptr;
}

int
get_set_cache(netsnmp_agent_session *asp)
{
    agent_set_cache *ptr, *prev = NULL;

    for (ptr = Sets; ptr != NULL; ptr = ptr->next) {
        if (ptr->sess == asp->session && ptr->transID == asp->pdu->transid) {
            if (prev)
                prev->next = ptr->next;
            else
                Sets = ptr->next;

            /*
             * found it.  Get the needed data 
             */
            asp->treecache = ptr->treecache;
            asp->treecache_len = ptr->treecache_len;
            asp->treecache_num = ptr->treecache_num;
            asp->requests = ptr->requests;
            asp->vbcount = ptr->vbcount;
            if (!asp->reqinfo) {
                asp->reqinfo =
                    SNMP_MALLOC_TYPEDEF(netsnmp_agent_request_info);
                if (asp->reqinfo) {
                    asp->reqinfo->asp = asp;
                    asp->reqinfo->agent_data = ptr->agent_data;
                }
            }
            free(ptr);
            return SNMP_ERR_NOERROR;
        }
        prev = ptr;
    }
    return SNMP_ERR_GENERR;
}

int
getNextSessID()
{
    static int      SessionID = 0;

    return ++SessionID;
}

int
agent_check_and_process(int block)
{
    int             numfds;
    fd_set          fdset;
    struct timeval  timeout = { LONG_MAX, 0 }, *tvp = &timeout;
    int             count;
    int             fakeblock = 0;

    numfds = 0;
    FD_ZERO(&fdset);
    snmp_select_info(&numfds, &fdset, tvp, &fakeblock);
    if (block != 0 && fakeblock != 0) {
        /*
         * There are no alarms registered, and the caller asked for blocking, so
         * let select() block forever.  
         */

        tvp = NULL;
    } else if (block != 0 && fakeblock == 0) {
        /*
         * The caller asked for blocking, but there is an alarm due sooner than
         * LONG_MAX seconds from now, so use the modified timeout returned by
         * snmp_select_info as the timeout for select().  
         */

    } else if (block == 0) {
        /*
         * The caller does not want us to block at all.  
         */

        tvp->tv_sec = 0;
        tvp->tv_usec = 0;
    }

    count = select(numfds, &fdset, 0, 0, tvp);

    if (count > 0) {
        /*
         * packets found, process them 
         */
        snmp_read(&fdset);
    } else
        switch (count) {
        case 0:
            snmp_timeout();
            break;
        case -1:
            if (errno != EINTR) {
                snmp_log_perror("select");
            }
            return -1;
        default:
            snmp_log(LOG_ERR, "select returned %d\n", count);
            return -1;
        }                       /* endif -- count>0 */

    /*
     * Run requested alarms.  
     */
    run_alarms();

    return count;
}



/*
 * Set up the address cache.  
 */
void
netsnmp_addrcache_initialise(void)
{
    int             i = 0;

    for (i = 0; i < SNMP_ADDRCACHE_SIZE; i++) {
        addrCache[i].addr = NULL;
        addrCache[i].status = SNMP_ADDRCACHE_UNUSED;
    }
}



/*
 * Age the entries in the address cache.  
 */

void
netsnmp_addrcache_age(void)
{
    int             i = 0;

    lastAddrAge = 0;
    for (i = 0; i < SNMP_ADDRCACHE_SIZE; i++) {
        if (addrCache[i].status == SNMP_ADDRCACHE_OLD) {
            addrCache[i].status = SNMP_ADDRCACHE_UNUSED;
            if (addrCache[i].addr != NULL) {
                free(addrCache[i].addr);
                addrCache[i].addr = NULL;
            }
        }
        if (addrCache[i].status == SNMP_ADDRCACHE_USED) {
            addrCache[i].status = SNMP_ADDRCACHE_OLD;
        }
    }
}

/*******************************************************************-o-******
 * netsnmp_agent_check_packet
 *
 * Parameters:
 *	session, transport, transport_data, transport_data_length
 *      
 * Returns:
 *	1	On success.
 *	0	On error.
 *
 * Handler for all incoming messages (a.k.a. packets) for the agent.  If using
 * the libwrap utility, log the connection and deny/allow the access. Print
 * output when appropriate, and increment the incoming counter.
 *
 */

int
netsnmp_agent_check_packet(netsnmp_session * session,
                           netsnmp_transport *transport,
                           void *transport_data, int transport_data_length)
{
    char           *addr_string = NULL;
#ifdef USE_LIBWRAP
    char           *fcolon_ptr = NULL, *lcolon_ptr = NULL, *wrap_addr = NULL;
#endif
    int             i = 0;

    /*
     * Log the message and/or dump the message.
     * Optionally cache the network address of the sender.
     */

    if (transport != NULL && transport->f_fmtaddr != NULL) {
        /*
         * Okay I do know how to format this address for logging.  
         */
        addr_string = transport->f_fmtaddr(transport, transport_data,
                                           transport_data_length);
        /*
         * Don't forget to free() it.  
         */
    }
#ifdef  USE_LIBWRAP
    if (addr_string != NULL) {
        if ((wrap_addr = strdup(addr_string)) != NULL) {
	  fcolon_ptr = strchr(wrap_addr, ':');
	  lcolon_ptr = strrchr(wrap_addr, ':');
	  if (lcolon_ptr != NULL)
	     *lcolon_ptr = NULL;
	  if (fcolon_ptr != NULL) {
	    fcolon_ptr++;
	    if (hosts_ctl("snmpd", STRING_UNKNOWN, fcolon_ptr, STRING_UNKNOWN)) {
		snmp_log(allow_severity, "Connection from %s\n", addr_string);
	    } else {
		snmp_log(deny_severity, "Connection from %s REFUSED\n",
			 addr_string);
		free(wrap_addr);
		free(addr_string);
		return 0;
	    }
	  }
	  free(wrap_addr);
	  wrap_addr = NULL;
	}
    } else {
        if (hosts_ctl("snmpd", STRING_UNKNOWN, STRING_UNKNOWN, STRING_UNKNOWN)){
            snmp_log(allow_severity, "Connection from <UNKNOWN>\n");
            addr_string = strdup("<UNKNOWN>");
        } else {
            snmp_log(deny_severity, "Connection from <UNKNOWN> REFUSED\n");
            return 0;
        }
    }
#endif                          /*USE_LIBWRAP */

    snmp_increment_statistic(STAT_SNMPINPKTS);

    if (log_addresses || netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
						NETSNMP_DS_AGENT_VERBOSE)) {
        for (i = 0; i < SNMP_ADDRCACHE_SIZE; i++) {
            if ((addrCache[i].status != SNMP_ADDRCACHE_UNUSED) &&
                (strcmp(addrCache[i].addr, addr_string) == 0)) {
                break;
            }
        }

        if (i >= SNMP_ADDRCACHE_SIZE ||
	    netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				   NETSNMP_DS_AGENT_VERBOSE)) {
            /*
             * Address wasn't in the cache, so log the packet...  
             */
            snmp_log(LOG_INFO, "Received SNMP packet(s) from %s\n",
                     addr_string);
            /*
             * ...and try to cache the address.  
             */
            for (i = 0; i < SNMP_ADDRCACHE_SIZE; i++) {
                if (addrCache[i].status == SNMP_ADDRCACHE_UNUSED) {
                    if (addrCache[i].addr != NULL) {
                        free(addrCache[i].addr);
                    }
                    addrCache[i].addr = addr_string;
                    addrCache[i].status = SNMP_ADDRCACHE_USED;
                    addr_string = NULL; /* Don't free this 'temporary' string
                                         * since it's now part of the cache */
                    break;
                }
            }
            if (i >= SNMP_ADDRCACHE_SIZE) {
                /*
                 * We didn't find a free slot to cache the address.  Perhaps
                 * we should be using an LRU replacement policy here or
                 * something.  Oh well.
                 */
                DEBUGMSGTL(("netsnmp_agent_check_packet",
                            "cache overrun"));
            }
        } else {
            addrCache[i].status = SNMP_ADDRCACHE_USED;
        }
    }

    if (addr_string != NULL) {
        free(addr_string);
        addr_string = NULL;
    }
    return 1;
}


int
netsnmp_agent_check_parse(netsnmp_session * session, netsnmp_pdu *pdu,
                          int result)
{
    if (result == 0) {
        if (snmp_get_do_logging() &&
	    netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
				   NETSNMP_DS_AGENT_VERBOSE)) {
            netsnmp_variable_list *var_ptr;

            switch (pdu->command) {
            case SNMP_MSG_GET:
                snmp_log(LOG_DEBUG, "  GET message\n");
                break;
            case SNMP_MSG_GETNEXT:
                snmp_log(LOG_DEBUG, "  GETNEXT message\n");
                break;
            case SNMP_MSG_RESPONSE:
                snmp_log(LOG_DEBUG, "  RESPONSE message\n");
                break;
            case SNMP_MSG_SET:
                snmp_log(LOG_DEBUG, "  SET message\n");
                break;
            case SNMP_MSG_TRAP:
                snmp_log(LOG_DEBUG, "  TRAP message\n");
                break;
            case SNMP_MSG_GETBULK:
                snmp_log(LOG_DEBUG, "  GETBULK message, non-rep=%d, max_rep=%d\n",
                         pdu->errstat, pdu->errindex);
                break;
            case SNMP_MSG_INFORM:
                snmp_log(LOG_DEBUG, "  INFORM message\n");
                break;
            case SNMP_MSG_TRAP2:
                snmp_log(LOG_DEBUG, "  TRAP2 message\n");
                break;
            case SNMP_MSG_REPORT:
                snmp_log(LOG_DEBUG, "  REPORT message\n");
                break;

            case SNMP_MSG_INTERNAL_SET_RESERVE1:
                snmp_log(LOG_DEBUG, "  INTERNAL RESERVE1 message\n");
                break;

            case SNMP_MSG_INTERNAL_SET_RESERVE2:
                snmp_log(LOG_DEBUG, "  INTERNAL RESERVE2 message\n");
                break;

            case SNMP_MSG_INTERNAL_SET_ACTION:
                snmp_log(LOG_DEBUG, "  INTERNAL ACTION message\n");
                break;

            case SNMP_MSG_INTERNAL_SET_COMMIT:
                snmp_log(LOG_DEBUG, "  INTERNAL COMMIT message\n");
                break;

            case SNMP_MSG_INTERNAL_SET_FREE:
                snmp_log(LOG_DEBUG, "  INTERNAL FREE message\n");
                break;

            case SNMP_MSG_INTERNAL_SET_UNDO:
                snmp_log(LOG_DEBUG, "  INTERNAL UNDO message\n");
                break;

            default:
                snmp_log(LOG_DEBUG, "  UNKNOWN message, type=%02X\n",
                         pdu->command);
                snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
                return 0;
            }

            for (var_ptr = pdu->variables; var_ptr != NULL;
                 var_ptr = var_ptr->next_variable) {
                size_t          c_oidlen = 256, c_outlen = 0;
                u_char         *c_oid = (u_char *) malloc(c_oidlen);

                if (c_oid) {
                    if (!sprint_realloc_objid
                        (&c_oid, &c_oidlen, &c_outlen, 1, var_ptr->name,
                         var_ptr->name_length)) {
                        snmp_log(LOG_DEBUG, "    -- %s [TRUNCATED]\n",
                                 c_oid);
                    } else {
                        snmp_log(LOG_DEBUG, "    -- %s\n", c_oid);
                    }
                    free(c_oid);
                }
            }
        }
        return 1;
    }
    return 0;                   /* XXX: does it matter what the return value
                                 * is?  Yes: if we return 0, then the PDU is
                                 * dumped.  */
}


/*
 * Global access to the primary session structure for this agent.
 * for Index Allocation use initially. 
 */

/*
 * I don't understand what this is for at the moment.  AFAICS as long as it
 * gets set and points at a session, that's fine.  ???  
 */

netsnmp_session *main_session = NULL;



/*
 * Set up an agent session on the given transport.  Return a handle
 * which may later be used to de-register this transport.  A return
 * value of -1 indicates an error.  
 */

int
netsnmp_register_agent_nsap(netsnmp_transport *t)
{
    netsnmp_session *s, *sp = NULL;
    agent_nsap     *a = NULL, *n = NULL, **prevNext = &agent_nsap_list;
    int             handle = 0;
    void           *isp = NULL;

    if (t == NULL) {
        return -1;
    }

    DEBUGMSGTL(("netsnmp_register_agent_nsap", "fd %d\n", t->sock));

    n = (agent_nsap *) malloc(sizeof(agent_nsap));
    if (n == NULL) {
        return -1;
    }
    s = (netsnmp_session *) malloc(sizeof(netsnmp_session));
    if (s == NULL) {
        free(n);
        return -1;
    }
    memset(s, 0, sizeof(netsnmp_session));
    snmp_sess_init(s);

    /*
     * Set up the session appropriately for an agent.  
     */

    s->version = SNMP_DEFAULT_VERSION;
    s->callback = handle_snmp_packet;
    s->authenticator = NULL;
    s->flags = netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, 
				  NETSNMP_DS_AGENT_FLAGS);
    s->isAuthoritative = SNMP_SESS_AUTHORITATIVE;

    sp = snmp_add(s, t, netsnmp_agent_check_packet,
                  netsnmp_agent_check_parse);
    if (sp == NULL) {
        free(s);
        free(n);
        return -1;
    }

    isp = snmp_sess_pointer(sp);
    if (isp == NULL) {          /*  over-cautious  */
        free(s);
        free(n);
        return -1;
    }

    n->s = isp;
    n->t = t;

    if (main_session == NULL) {
        main_session = snmp_sess_session(isp);
    }

    for (a = agent_nsap_list; a != NULL && handle + 1 >= a->handle;
         a = a->next) {
        handle = a->handle;
        prevNext = &(a->next);
    }

    if (handle < INT_MAX) {
        n->handle = handle + 1;
        n->next = a;
        *prevNext = n;
        free(s);
        return n->handle;
    } else {
        free(s);
        free(n);
        return -1;
    }
}

void
netsnmp_deregister_agent_nsap(int handle)
{
    agent_nsap     *a = NULL, **prevNext = &agent_nsap_list;
    int             main_session_deregistered = 0;

    DEBUGMSGTL(("netsnmp_deregister_agent_nsap", "handle %d\n", handle));

    for (a = agent_nsap_list; a != NULL && a->handle < handle; a = a->next) {
        prevNext = &(a->next);
    }

    if (a != NULL && a->handle == handle) {
        *prevNext = a->next;
        if (main_session == snmp_sess_session(a->s)) {
            main_session_deregistered = 1;
        }
        snmp_close(snmp_sess_session(a->s));
        /*
         * The above free()s the transport and session pointers.  
         */
        free(a);
    }

    /*
     * If we've deregistered the session that main_session used to point to,
     * then make it point to another one, or in the last resort, make it equal
     * to NULL.  Basically this shouldn't ever happen in normal operation
     * because main_session starts off pointing at the first session added by
     * init_master_agent(), which then discards the handle.  
     */

    if (main_session_deregistered) {
        if (agent_nsap_list != NULL) {
            DEBUGMSGTL(("snmp_agent",
			"WARNING: main_session ptr changed from %p to %p\n",
                        main_session, snmp_sess_session(agent_nsap_list->s)));
            main_session = snmp_sess_session(agent_nsap_list->s);
        } else {
            DEBUGMSGTL(("snmp_agent",
			"WARNING: main_session ptr changed from %p to NULL\n",
                        main_session));
            main_session = NULL;
        }
    }
}



/*
 * 
 * This function has been modified to use the experimental
 * netsnmp_register_agent_nsap interface.  The major responsibility of this
 * function now is to interpret a string specified to the agent (via -p on the
 * command line, or from a configuration file) as a list of agent NSAPs on
 * which to listen for SNMP packets.  Typically, when you add a new transport
 * domain "foo", you add code here such that if the "foo" code is compiled
 * into the agent (SNMP_TRANSPORT_FOO_DOMAIN is defined), then a token of the
 * form "foo:bletch-3a0054ef%wob&wob" gets turned into the appropriate
 * transport descriptor.  netsnmp_register_agent_nsap is then called with that
 * transport descriptor and sets up a listening agent session on it.
 * 
 * Everything then works much as normal: the agent runs in an infinite loop
 * (in the snmpd.c/receive()routine), which calls snmp_read() when a request
 * is readable on any of the given transports.  This routine then traverses
 * the library 'Sessions' list to identify the relevant session and eventually
 * invokes '_sess_read'.  This then processes the incoming packet, calling the
 * pre_parse, parse, post_parse and callback routines in turn.
 * 
 * JBPN 20001117
 */

int
init_master_agent(void)
{
    netsnmp_transport *transport;
    char           *cptr;
    char            buf[SPRINT_MAX_LEN];

    /* default to turning off lookup caching */
    netsnmp_set_lookup_cache_size(0);

    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_ROLE) != MASTER_AGENT) {
        DEBUGMSGTL(("snmp_agent",
                    "init_master_agent; not master agent\n"));
        return 0;               /*  No error if ! MASTER_AGENT  */
    }
#ifdef USING_AGENTX_MASTER_MODULE
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_AGENTX_MASTER) == 1)
        real_init_master();
#endif

    /*
     * Have specific agent ports been specified?  
     */
    cptr = netsnmp_ds_get_string(NETSNMP_DS_APPLICATION_ID, 
				 NETSNMP_DS_AGENT_PORTS);

    if (cptr) {
        snprintf(buf, sizeof(buf), "%s", cptr);
        buf[ sizeof(buf)-1 ] = 0;
    } else {
        /*
         * No, so just specify the default port.  
         */
        if (netsnmp_ds_get_int(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_FLAGS) & SNMP_FLAGS_STREAM_SOCKET) {
            sprintf(buf, "tcp:%d", SNMP_PORT);
        } else {
            sprintf(buf, "udp:%d", SNMP_PORT);
        }
    }

    DEBUGMSGTL(("snmp_agent", "final port spec: %s\n", buf));
    cptr = strtok(buf, ",");
    while (cptr) {
        /*
         * Specification format: 
         * 
         * NONE:                      (a pseudo-transport)
         * UDP:[address:]port        (also default if no transport is specified)
         * TCP:[address:]port         (if supported)
         * Unix:pathname              (if supported)
         * AAL5PVC:itf.vpi.vci        (if supported)
         * IPX:[network]:node[/port] (if supported)
         * 
         */

        DEBUGMSGTL(("snmp_agent", "installing master agent on port %s\n",
                    cptr));

        if (!cptr || !(*cptr)) {
            snmp_log(LOG_ERR, "improper port specification\n");
            return 1;
        }

        if (strncasecmp(cptr, "none", 4) == 0) {
            DEBUGMSGTL(("snmp_agent",
                        "init_master_agent; pseudo-transport \"none\" requested\n"));
            return 0;
        }
        transport = netsnmp_tdomain_transport(cptr, 1, "udp");

        if (transport == NULL) {
            snmp_log(LOG_ERR, "Error opening specified endpoint \"%s\"\n",
                     cptr);
            return 1;
        }

        if (netsnmp_register_agent_nsap(transport) == 0) {
            snmp_log(LOG_ERR,
                     "Error registering specified transport \"%s\" as an agent NSAP\n",
                     cptr);
            return 1;
        } else {
            DEBUGMSGTL(("snmp_agent",
                        "init_master_agent; \"%s\" registered as an agent NSAP\n",
                        cptr));
        }

        /*
         * Next transport please...  
         */
        cptr = strtok(NULL, ",");
    }

    return 0;
}



netsnmp_agent_session *
init_agent_snmp_session(netsnmp_session * session, netsnmp_pdu *pdu)
{
    netsnmp_agent_session *asp = (netsnmp_agent_session *)
        calloc(1, sizeof(netsnmp_agent_session));

    if (asp == NULL) {
        return NULL;
    }

    DEBUGMSGTL(("snmp_agent","agent_sesion %08p created\n", asp));
    asp->session = session;
    asp->pdu = snmp_clone_pdu(pdu);
    asp->orig_pdu = snmp_clone_pdu(pdu);
    asp->rw = READ;
    asp->exact = TRUE;
    asp->next = NULL;
    asp->mode = RESERVE1;
    asp->status = SNMP_ERR_NOERROR;
    asp->index = 0;
    asp->oldmode = 0;
    asp->treecache_num = -1;
    asp->treecache_len = 0;

    return asp;
}

void
free_agent_snmp_session(netsnmp_agent_session *asp)
{
    if (!asp)
        return;

    DEBUGMSGTL(("snmp_agent","agent_sesion %08p released\n", asp));

    netsnmp_remove_from_delegated(asp);
    
    if (asp->orig_pdu)
        snmp_free_pdu(asp->orig_pdu);
    if (asp->pdu)
        snmp_free_pdu(asp->pdu);
    if (asp->reqinfo)
        netsnmp_free_agent_request_info(asp->reqinfo);
    if (asp->treecache) {
        free(asp->treecache);
    }
    if (asp->bulkcache) {
        free(asp->bulkcache);
    }
    if (asp->requests) {
        int             i;
        for (i = 0; i < asp->vbcount; i++) {
            netsnmp_free_request_data_sets(&asp->requests[i]);
        }
    }
    if (asp->requests) {
        free(asp->requests);
    }
    if (asp->cache_store) {
        netsnmp_free_cachemap(asp->cache_store);
        asp->cache_store = NULL;
    }
    free(asp);
}

int
netsnmp_check_for_delegated(netsnmp_agent_session *asp)
{
    int             i;
    netsnmp_request_info *request;

    if (NULL == asp->treecache)
        return 0;
    
    for (i = 0; i <= asp->treecache_num; i++) {
        for (request = asp->treecache[i].requests_begin; request;
             request = request->next) {
            if (request->delegated)
                return 1;
        }
    }
    return 0;
}

int
netsnmp_check_delegated_chain_for(netsnmp_agent_session *asp)
{
    netsnmp_agent_session *asptmp;
    for (asptmp = agent_delegated_list; asptmp; asptmp = asptmp->next) {
        if (asptmp == asp)
            return 1;
    }
    return 0;
}

int
netsnmp_check_for_delegated_and_add(netsnmp_agent_session *asp)
{
    if (netsnmp_check_for_delegated(asp)) {
        if (!netsnmp_check_delegated_chain_for(asp)) {
            /*
             * add to delegated request chain 
             */
            asp->next = agent_delegated_list;
            agent_delegated_list = asp;
            DEBUGMSGTL(("snmp_agent", "delegate session == %08p\n", asp));
        }
        return 1;
    }
    return 0;
}

int
netsnmp_remove_from_delegated(netsnmp_agent_session *asp)
{
    netsnmp_agent_session *curr, *prev = NULL;
    
    for (curr = agent_delegated_list; curr; prev = curr, curr = curr->next) {
        /*
         * is this us?
         */
        if (curr != asp)
            continue;
        
        /*
         * remove from queue 
         */
        if (prev != NULL)
            prev->next = asp->next;
        else
            agent_delegated_list = asp->next;

        DEBUGMSGTL(("snmp_agent", "remove delegated session == %08p\n", asp));

        return 1;
    }

    return 0;
}

/*
 * netsnmp_remove_delegated_requests_for_session
 *
 * called when a session is being closed. Check all delegated requests to
 * see if the are waiting on this session, and if set, set the status for
 * that request to GENERR.
 */
int
netsnmp_remove_delegated_requests_for_session(netsnmp_session *sess)
{
    netsnmp_agent_session *asp;
    int count = 0;
    
    for (asp = agent_delegated_list; asp; asp = asp->next) {
        /*
         * check each request
         */
        netsnmp_request_info *request;
        for(request = asp->requests; request; request = request->next) {
            /*
             * check session
             */
            netsnmp_assert(NULL!=request->subtree);
            if(request->subtree->session != sess)
                continue;

            /*
             * matched! mark request as done
             */
            netsnmp_set_mode_request_error(MODE_SET_BEGIN, request,
                                           SNMP_ERR_GENERR);
            ++count;
        }
    }

    /*
     * if we found any, that request may be finished now
     */
    if(count) {
        DEBUGMSGTL(("snmp_agent", "removed %d delegated request(s) for session "
                    "%08p\n", count, sess));
        netsnmp_check_outstanding_agent_requests();
    }
    
    return count;
}

int
netsnmp_check_queued_chain_for(netsnmp_agent_session *asp)
{
    netsnmp_agent_session *asptmp;
    for (asptmp = netsnmp_agent_queued_list; asptmp; asptmp = asptmp->next) {
        if (asptmp == asp)
            return 1;
    }
    return 0;
}

int
netsnmp_add_queued(netsnmp_agent_session *asp)
{
    netsnmp_agent_session *asp_tmp;

    /*
     * first item?
     */
    if (NULL == netsnmp_agent_queued_list) {
        netsnmp_agent_queued_list = asp;
        return 1;
    }


    /*
     * add to end of queued request chain 
     */
    asp_tmp = netsnmp_agent_queued_list;
    for (; asp_tmp; asp_tmp = asp_tmp->next) {
        /*
         * already in queue?
         */
        if (asp_tmp == asp)
            break;

        /*
         * end of queue?
         */
        if (NULL == asp_tmp->next)
            asp_tmp->next = asp;
    }
    return 1;
}


int
netsnmp_wrap_up_request(netsnmp_agent_session *asp, int status)
{
    netsnmp_variable_list *var_ptr;
    int             i, n = 0, r = 0;

    /*
     * Update asp->status if necessary. Fixes ro/rw problem.
     */
    if ( status != 0  && asp->status == 0 )
          asp->status = status;

    /*
     * if this request was a set, clear the global now that we are
     * done.
     */
    if (asp == netsnmp_processing_set) {
        DEBUGMSGTL(("snmp_agent", "SET request complete, asp = %08p\n",
                    asp));
        netsnmp_processing_set = NULL;
    }

    /*
     * some stuff needs to be saved in special subagent cases 
     */
    if (asp->pdu) {

        switch (asp->pdu->command) {
            case SNMP_MSG_INTERNAL_SET_BEGIN:
            case SNMP_MSG_INTERNAL_SET_RESERVE1:
            case SNMP_MSG_INTERNAL_SET_RESERVE2:
            case SNMP_MSG_INTERNAL_SET_ACTION:
                save_set_cache(asp);
                break;
        }

        /*
         * if this is a GETBULK response we need to rearrange the varbinds 
         */
        if (asp->pdu->command == SNMP_MSG_GETBULK) {
            int             repeats = asp->pdu->errindex;
            int             j;
            
            if (asp->pdu->errstat < asp->vbcount) {
                n = asp->pdu->errstat;
            } else {
                n = asp->vbcount;
            }
            if ((r = asp->vbcount - n) < 0) {
                r = 0;
            }
            
            for (i = 0; i < r - 1; i++) {
                for (j = 0; j < repeats; j++) {
                    asp->bulkcache[i * repeats + j]->next_variable =
                        asp->bulkcache[(i + 1) * repeats + j];
                }
            }
            if (r > 0) {
                for (j = 0; j < repeats - 1; j++) {
                    asp->bulkcache[(r - 1) * repeats + j]->next_variable =
                        asp->bulkcache[j + 1];
                }
            }
        }

        /*
         * May need to "dumb down" a SET error status for a
         * v1 query.  See RFC2576 - section 4.3
         */
        if ((asp->pdu->command == SNMP_MSG_SET) &&
            (asp->pdu->version == SNMP_VERSION_1)) {
            switch (status) {
                case SNMP_ERR_WRONGVALUE:
                case SNMP_ERR_WRONGENCODING:
                case SNMP_ERR_WRONGTYPE:
                case SNMP_ERR_WRONGLENGTH:
                case SNMP_ERR_INCONSISTENTVALUE:
                    status = SNMP_ERR_BADVALUE;
                    asp->status = SNMP_ERR_BADVALUE;
                    break;
                case SNMP_ERR_NOACCESS:
                case SNMP_ERR_NOTWRITABLE:
                case SNMP_ERR_NOCREATION:
                case SNMP_ERR_INCONSISTENTNAME:
                case SNMP_ERR_AUTHORIZATIONERROR:
                    status = SNMP_ERR_NOSUCHNAME;
                    asp->status = SNMP_ERR_NOSUCHNAME;
                    break;
                case SNMP_ERR_RESOURCEUNAVAILABLE:
                case SNMP_ERR_COMMITFAILED:
                case SNMP_ERR_UNDOFAILED:
                    status = SNMP_ERR_GENERR;
                    asp->status = SNMP_ERR_GENERR;
                    break;
            }
        }
        /*
         * Similarly we may need to "dumb down" v2 exception
         *  types to throw an error for a v1 query.
         *  See RFC2576 - section 4.1.2.3
         */
        if ((asp->pdu->command != SNMP_MSG_SET) &&
            (asp->pdu->version == SNMP_VERSION_1)) {
            for (var_ptr = asp->pdu->variables, i = 1;
                 var_ptr != NULL; var_ptr = var_ptr->next_variable, i++) {
                switch (var_ptr->type) {
                    case SNMP_NOSUCHOBJECT:
                    case SNMP_NOSUCHINSTANCE:
                    case SNMP_ENDOFMIBVIEW:
                    case ASN_COUNTER64:
                        status = SNMP_ERR_NOSUCHNAME;
                        asp->status = SNMP_ERR_NOSUCHNAME;
                        asp->index = i;
                        break;
                }
            }
        }
    } /** if asp->pdu */

    /*
     * Update the snmp error-count statistics
     *   XXX - should we include the V2 errors in this or not?
     */
#define INCLUDE_V2ERRORS_IN_V1STATS

    switch (status) {
#ifdef INCLUDE_V2ERRORS_IN_V1STATS
    case SNMP_ERR_WRONGVALUE:
    case SNMP_ERR_WRONGENCODING:
    case SNMP_ERR_WRONGTYPE:
    case SNMP_ERR_WRONGLENGTH:
    case SNMP_ERR_INCONSISTENTVALUE:
#endif
    case SNMP_ERR_BADVALUE:
        snmp_increment_statistic(STAT_SNMPOUTBADVALUES);
        break;
#ifdef INCLUDE_V2ERRORS_IN_V1STATS
    case SNMP_ERR_NOACCESS:
    case SNMP_ERR_NOTWRITABLE:
    case SNMP_ERR_NOCREATION:
    case SNMP_ERR_INCONSISTENTNAME:
    case SNMP_ERR_AUTHORIZATIONERROR:
#endif
    case SNMP_ERR_NOSUCHNAME:
        snmp_increment_statistic(STAT_SNMPOUTNOSUCHNAMES);
        break;
#ifdef INCLUDE_V2ERRORS_IN_V1STATS
    case SNMP_ERR_RESOURCEUNAVAILABLE:
    case SNMP_ERR_COMMITFAILED:
    case SNMP_ERR_UNDOFAILED:
#endif
    case SNMP_ERR_GENERR:
        snmp_increment_statistic(STAT_SNMPOUTGENERRS);
        break;

    case SNMP_ERR_TOOBIG:
        snmp_increment_statistic(STAT_SNMPOUTTOOBIGS);
        break;
    }

    if ((status == SNMP_ERR_NOERROR) && (asp->pdu)) {
        snmp_increment_statistic_by((asp->pdu->command == SNMP_MSG_SET ?
                                     STAT_SNMPINTOTALSETVARS :
                                     STAT_SNMPINTOTALREQVARS),
                                    count_varbinds(asp->pdu->variables));
    } else {
        /*
         * Use a copy of the original request
         *   to report failures.
         */
        snmp_free_pdu(asp->pdu);
        asp->pdu = asp->orig_pdu;
        asp->orig_pdu = NULL;
    }
    if (asp->pdu) {
        asp->pdu->command = SNMP_MSG_RESPONSE;
        asp->pdu->errstat = asp->status;
        asp->pdu->errindex = asp->index;
        if (!snmp_send(asp->session, asp->pdu)) {
            snmp_free_pdu(asp->pdu);
            asp->pdu = NULL;
        }
        snmp_increment_statistic(STAT_SNMPOUTPKTS);
        snmp_increment_statistic(STAT_SNMPOUTGETRESPONSES);
        asp->pdu = NULL;
        netsnmp_remove_and_free_agent_snmp_session(asp);
    }
    return 1;
}

void
dump_sess_list(void)
{
    netsnmp_agent_session *a;

    DEBUGMSGTL(("snmp_agent", "DUMP agent_sess_list -> "));
    for (a = agent_session_list; a != NULL; a = a->next) {
        DEBUGMSG(("snmp_agent", "%08p[session %08p] -> ", a, a->session));
    }
    DEBUGMSG(("snmp_agent", "[NIL]\n"));
}

void
netsnmp_remove_and_free_agent_snmp_session(netsnmp_agent_session *asp)
{
    netsnmp_agent_session *a, **prevNext = &agent_session_list;

    DEBUGMSGTL(("snmp_agent", "REMOVE session == %08p\n", asp));

    for (a = agent_session_list; a != NULL; a = *prevNext) {
        if (a == asp) {
            *prevNext = a->next;
            a->next = NULL;
            free_agent_snmp_session(a);
            asp = NULL;
            break;
        } else {
            prevNext = &(a->next);
        }
    }

    if (a == NULL && asp != NULL) {
        /*
         * We coulnd't find it on the list, so free it anyway.  
         */
        free_agent_snmp_session(asp);
    }
}

void
netsnmp_free_agent_snmp_session_by_session(netsnmp_session * sess,
                                           void (*free_request)
                                           (netsnmp_request_list *))
{
    netsnmp_agent_session *a, *next, **prevNext = &agent_session_list;

    DEBUGMSGTL(("snmp_agent", "REMOVE session == %08p\n", sess));

    for (a = agent_session_list; a != NULL; a = next) {
        if (a->session == sess) {
            *prevNext = a->next;
            next = a->next;
            free_agent_snmp_session(a);
        } else {
            prevNext = &(a->next);
            next = a->next;
        }
    }
}

/** handles an incoming SNMP packet into the agent */
int
handle_snmp_packet(int op, netsnmp_session * session, int reqid,
                   netsnmp_pdu *pdu, void *magic)
{
    netsnmp_agent_session *asp;
    int             status, access_ret, rc;

    /*
     * We only support receiving here.  
     */
    if (op != NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
        return 1;
    }

    /*
     * RESPONSE messages won't get this far, but TRAP-like messages
     * might.  
     */
    if (pdu->command == SNMP_MSG_TRAP || pdu->command == SNMP_MSG_INFORM ||
        pdu->command == SNMP_MSG_TRAP2) {
        DEBUGMSGTL(("snmp_agent", "received trap-like PDU (%02x)\n",
                    pdu->command));
        pdu->command = SNMP_MSG_TRAP2;
        snmp_increment_statistic(STAT_SNMPUNKNOWNPDUHANDLERS);
        return 1;
    }

    if (magic == NULL) {
        asp = init_agent_snmp_session(session, pdu);
        status = SNMP_ERR_NOERROR;
    } else {
        asp = (netsnmp_agent_session *) magic;
        status = asp->status;
    }

    if ((access_ret = check_access(pdu)) != 0) {
        if (access_ret == VACM_NOSUCHCONTEXT) {
            /*
             * rfc2573 section 3.2, step 5 says that we increment the
             * counter but don't return a response of any kind 
             */

            /*
             * we currently don't support unavailable contexts, as
             * there is no reason to that I currently know of 
             */
            snmp_increment_statistic(STAT_SNMPUNKNOWNCONTEXTS);

            /*
             * drop the request 
             */
            netsnmp_remove_and_free_agent_snmp_session(asp);
            return 0;
        } else {
            /*
             * access control setup is incorrect 
             */
            send_easy_trap(SNMP_TRAP_AUTHFAIL, 0);
            if (asp->pdu->version != SNMP_VERSION_1
                && asp->pdu->version != SNMP_VERSION_2c) {
                asp->pdu->errstat = SNMP_ERR_AUTHORIZATIONERROR;
                asp->pdu->command = SNMP_MSG_RESPONSE;
                snmp_increment_statistic(STAT_SNMPOUTPKTS);
                if (!snmp_send(asp->session, asp->pdu))
                    snmp_free_pdu(asp->pdu);
                asp->pdu = NULL;
                netsnmp_remove_and_free_agent_snmp_session(asp);
                return 1;
            } else {
                /*
                 * drop the request 
                 */
                netsnmp_remove_and_free_agent_snmp_session(asp);
                return 0;
            }
        }
    }

    rc = netsnmp_handle_request(asp, status);

    /*
     * done 
     */
    DEBUGMSGTL(("snmp_agent", "end of handle_snmp_packet, asp = %08p\n",
                asp));
    return rc;
}

netsnmp_request_info *
netsnmp_add_varbind_to_cache(netsnmp_agent_session *asp, int vbcount,
                             netsnmp_variable_list * varbind_ptr,
                             netsnmp_subtree *tp)
{
    netsnmp_request_info *request = NULL;
    int             cacheid;

    DEBUGMSGTL(("snmp_agent", "add_vb_to_cache(%8p, %d, ", asp, vbcount));
    DEBUGMSGOID(("snmp_agent", varbind_ptr->name,
                 varbind_ptr->name_length));
    DEBUGMSG(("snmp_agent", ", %8p)\n", tp));

    if (tp &&
        (asp->pdu->command == SNMP_MSG_GETNEXT ||
         asp->pdu->command == SNMP_MSG_GETBULK)) {
        int result;
        int prefix_len;

        prefix_len = netsnmp_oid_find_prefix(tp->start_a,
                                             tp->start_len,
                                             tp->end_a, tp->end_len);
        result =
            netsnmp_acm_check_subtree(asp->pdu, tp->start_a, prefix_len);

        while (result == VACM_NOTINVIEW) {
            /* the entire subtree is not in view. Skip it. */
            /** @todo make this be more intelligent about ranges.
                Right now we merely take the highest level
                commonality of a registration range and use that.
                At times we might be able to be smarter about
                checking the range itself as opposed to the node
                above where the range exists, but I doubt this will
                come up all that frequently. */
            tp = tp->next;
            if (tp) {
                prefix_len = netsnmp_oid_find_prefix(tp->start_a,
                                                     tp->start_len,
                                                     tp->end_a,
                                                     tp->end_len);
                result =
                    netsnmp_acm_check_subtree(asp->pdu,
                                              tp->start_a, prefix_len);
            }
            else
                break;
        }
    }
    if (tp == NULL) {
        /*
         * no appropriate registration found 
         */
        /*
         * make up the response ourselves 
         */
        switch (asp->pdu->command) {
        case SNMP_MSG_GETNEXT:
        case SNMP_MSG_GETBULK:
            varbind_ptr->type = SNMP_ENDOFMIBVIEW;
            break;

        case SNMP_MSG_SET:
            varbind_ptr->type = SNMP_NOSUCHOBJECT;
            break;

        case SNMP_MSG_GET:
            varbind_ptr->type = SNMP_NOSUCHOBJECT;
            break;

        default:
            return NULL;        /* shouldn't get here */
        }
    } else {
        DEBUGMSGTL(("snmp_agent", "tp->start "));
        DEBUGMSGOID(("snmp_agent", tp->start_a, tp->start_len));
        DEBUGMSG(("snmp_agent", ", tp->end "));
        DEBUGMSGOID(("snmp_agent", tp->end_a, tp->end_len));
        DEBUGMSG(("snmp_agent", ", \n"));

        /*
         * malloc the request structure 
         */
        request = &(asp->requests[vbcount - 1]);
        request->index = vbcount;
        request->delegated = 0;
        request->processed = 0;
        request->status = 0;
        request->subtree = tp;
        if (request->parent_data) {
            netsnmp_free_request_data_sets(request);
        }

        /*
         * for non-SET modes, set the type to NULL 
         */
        if (!MODE_IS_SET(asp->pdu->command)) {
            if (varbind_ptr->type == ASN_PRIV_INCL_RANGE) {
                DEBUGMSGTL(("snmp_agent", "varbind %d is inclusive\n",
                            request->index));
                request->inclusive = 1;
            }
            varbind_ptr->type = ASN_NULL;
        }

        /*
         * place them in a cache 
         */
        if (tp->global_cacheid) {
            /*
             * we need to merge all marked subtrees together 
             */
            if (asp->cache_store && -1 !=
                (cacheid = netsnmp_get_local_cachid(asp->cache_store,
                                                    tp->global_cacheid))) {
            } else {
                cacheid = ++(asp->treecache_num);
                netsnmp_get_or_add_local_cachid(&asp->cache_store,
                                                tp->global_cacheid,
                                                cacheid);
                goto mallocslot;        /* XXX: ick */
            }
        } else if (tp->cacheid > -1 && tp->cacheid <= asp->treecache_num &&
                   asp->treecache[tp->cacheid].subtree == tp) {
            /*
             * we have already added a request to this tree
             * pointer before 
             */
            cacheid = tp->cacheid;
        } else {
            cacheid = ++(asp->treecache_num);
          mallocslot:
            /*
             * new slot needed 
             */
            if (asp->treecache_num >= asp->treecache_len) {
                /*
                 * exapand cache array 
                 */
                /*
                 * WWW: non-linear expansion needed (with cap) 
                 */
#define CACHE_GROW_SIZE 16
                asp->treecache_len =
                    (asp->treecache_len + CACHE_GROW_SIZE);
                asp->treecache =
                    realloc(asp->treecache,
                            sizeof(netsnmp_tree_cache) *
                            asp->treecache_len);
                if (asp->treecache == NULL)
                    return NULL;
                memset(&(asp->treecache[cacheid]), 0x00,
                       sizeof(netsnmp_tree_cache) * (CACHE_GROW_SIZE));
            }
            asp->treecache[cacheid].subtree = tp;
            asp->treecache[cacheid].requests_begin = request;
            tp->cacheid = cacheid;
        }

        /*
         * if this is a search type, get the ending range oid as well 
         */
        if (asp->pdu->command == SNMP_MSG_GETNEXT ||
            asp->pdu->command == SNMP_MSG_GETBULK) {
            request->range_end = tp->end_a;
            request->range_end_len = tp->end_len;
        } else {
            request->range_end = NULL;
            request->range_end_len = 0;
        }

        /*
         * link into chain 
         */
        if (asp->treecache[cacheid].requests_end)
            asp->treecache[cacheid].requests_end->next = request;
        request->next = NULL;
        request->prev = asp->treecache[cacheid].requests_end;
        asp->treecache[cacheid].requests_end = request;

        /*
         * add the given request to the list of requests they need
         * to handle results for 
         */
        request->requestvb = request->requestvb_start = varbind_ptr;
    }
    return request;
}

/*
 * check the ACM(s) for the results on each of the varbinds.
 * If ACM disallows it, replace the value with type
 * 
 * Returns number of varbinds with ACM errors
 */
int
check_acm(netsnmp_agent_session *asp, u_char type)
{
    int             view;
    int             i, j, k;
    netsnmp_request_info *request;
    int             ret = 0;
    netsnmp_variable_list *vb, *vb2, *vbc;
    int             earliest = 0;

    for (i = 0; i <= asp->treecache_num; i++) {
        for (request = asp->treecache[i].requests_begin;
             request; request = request->next) {
            /*
             * for each request, run it through in_a_view() 
             */
            earliest = 0;
            for(j = request->repeat, vb = request->requestvb_start;
                vb && j > -1;
                j--, vb = vb->next_variable) {
                if (vb->type != ASN_NULL &&
                    vb->type != ASN_PRIV_RETRY) { /* not yet processed */
                    view =
                        in_a_view(vb->name, &vb->name_length,
                                  asp->pdu, vb->type);

                    /*
                     * if a ACM error occurs, mark it as type passed in 
                     */
                    if (view != VACM_SUCCESS) {
                        ret++;
                        if (request->repeat < request->orig_repeat) {
                            /* basically this means a GETBULK */
                            request->repeat++;
                            if (!earliest) {
                                request->requestvb = vb;
                                earliest = 1;
                            }

                            /* ugh.  if a whole now exists, we need to
                               move the contents up the chain and fill
                               in at the end else we won't end up
                               lexographically sorted properly */
                            if (j > -1 && vb->next_variable &&
                                vb->next_variable->type != ASN_NULL &&
                                vb->next_variable->type != ASN_PRIV_RETRY) {
                                for(k = j, vbc = vb, vb2 = vb->next_variable;
                                    k > -2 && vbc && vb2;
                                    k--, vbc = vb2, vb2 = vb2->next_variable) {
                                    /* clone next into the current */
                                    snmp_clone_var(vb2, vbc);
                                    vbc->next_variable = vb2;
                                }
                            }
                        }
                        snmp_set_var_typed_value(vb, type, NULL, 0);
                    }
                }
            }
        }
    }
    return ret;
}


int
netsnmp_create_subtree_cache(netsnmp_agent_session *asp)
{
    netsnmp_subtree *tp;
    netsnmp_variable_list *varbind_ptr, *vbsave, *vbptr, **prevNext;
    int             view;
    int             vbcount = 0;
    int             bulkcount = 0, bulkrep = 0;
    int             i = 0, n = 0, r = 0;
    netsnmp_request_info *request;

    if (asp->treecache == NULL && asp->treecache_len == 0) {
        asp->treecache_len = SNMP_MAX(1 + asp->vbcount / 4, 16);
        asp->treecache =
            calloc(asp->treecache_len, sizeof(netsnmp_tree_cache));
        if (asp->treecache == NULL)
            return SNMP_ERR_GENERR;
    }
    asp->treecache_num = -1;

    if (asp->pdu->command == SNMP_MSG_GETBULK) {
        /*
         * getbulk prep 
         */
        int             count = count_varbinds(asp->pdu->variables);

        if (asp->pdu->errstat < 0) {
            asp->pdu->errstat = 0;
        }
        if (asp->pdu->errindex < 0) {
            asp->pdu->errindex = 0;
        }

        if (asp->pdu->errstat < count) {
            n = asp->pdu->errstat;
        } else {
            n = count;
        }
        if ((r = count - n) < 0) {
            r = 0;
            asp->bulkcache = NULL;
        } else {
            asp->bulkcache =
                (netsnmp_variable_list **) malloc(asp->pdu->errindex * r *
                                                  sizeof(struct
                                                         varbind_list *));
        }
        DEBUGMSGTL(("snmp_agent", "GETBULK N = %d, M = %d, R = %d\n",
                    n, asp->pdu->errindex, r));
    }

    /*
     * collect varbinds into their registered trees 
     */
    prevNext = &(asp->pdu->variables);
    for (varbind_ptr = asp->pdu->variables; varbind_ptr;
         varbind_ptr = vbsave) {

        /*
         * getbulk mess with this pointer, so save it 
         */
        vbsave = varbind_ptr->next_variable;

        if (asp->pdu->command == SNMP_MSG_GETBULK) {
            if (n > 0) {
                n--;
            } else {
                /*
                 * repeate request varbinds on GETBULK.  These will
                 * have to be properly rearranged later though as
                 * responses are supposed to actually be interlaced
                 * with each other.  This is done with the asp->bulkcache. 
                 */
                bulkrep = asp->pdu->errindex - 1;
                if (asp->pdu->errindex > 0) {
                    vbptr = varbind_ptr;
                    asp->bulkcache[bulkcount++] = vbptr;

                    for (i = 1; i < asp->pdu->errindex; i++) {
                        vbptr->next_variable =
                            SNMP_MALLOC_STRUCT(variable_list);
                        /*
                         * don't clone the oid as it's got to be
                         * overwwritten anyway 
                         */
                        if (!vbptr->next_variable) {
                            /*
                             * XXXWWW: ack!!! 
                             */
                        } else {
                            vbptr = vbptr->next_variable;
                            vbptr->name_length = 0;
                            vbptr->type = ASN_NULL;
                            asp->bulkcache[bulkcount++] = vbptr;
                        }
                    }
                    vbptr->next_variable = vbsave;
                } else {
                    /*
                     * 0 repeats requested for this varbind, so take it off
                     * the list.  
                     */
                    vbptr = varbind_ptr;
                    *prevNext = vbptr->next_variable;
                    vbptr->next_variable = NULL;
                    snmp_free_varbind(vbptr);
                    asp->vbcount--;
                    continue;
                }
            }
        }

        /*
         * count the varbinds 
         */
        ++vbcount;

        /*
         * find the owning tree 
         */
        tp = netsnmp_subtree_find(varbind_ptr->name, varbind_ptr->name_length,
				  NULL, asp->pdu->contextName);

        /*
         * check access control 
         */
        switch (asp->pdu->command) {
        case SNMP_MSG_GET:
            view = in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
                             asp->pdu, varbind_ptr->type);
            if (view != VACM_SUCCESS)
                snmp_set_var_typed_value(varbind_ptr, SNMP_NOSUCHOBJECT,
                                         NULL, 0);
            break;

        case SNMP_MSG_SET:
            view = in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
                             asp->pdu, varbind_ptr->type);
            if (view != VACM_SUCCESS)
                return SNMP_ERR_NOTWRITABLE;
            break;

        case SNMP_MSG_GETNEXT:
        case SNMP_MSG_GETBULK:
        default:
            view = VACM_SUCCESS;
            /*
             * XXXWWW: check VACM here to see if "tp" is even worthwhile 
             */
        }
        if (view == VACM_SUCCESS) {
            request = netsnmp_add_varbind_to_cache(asp, vbcount, varbind_ptr,
						   tp);
            if (request && asp->pdu->command == SNMP_MSG_GETBULK) {
                request->repeat = request->orig_repeat = bulkrep;
            }
        }

        prevNext = &(varbind_ptr->next_variable);
    }

    return SNMPERR_SUCCESS;
}

/*
 * this function is only applicable in getnext like contexts 
 */
int
netsnmp_reassign_requests(netsnmp_agent_session *asp)
{
    /*
     * assume all the requests have been filled or rejected by the
     * subtrees, so reassign the rejected ones to the next subtree in
     * the chain 
     */

    int             i;

    /*
     * get old info 
     */
    netsnmp_tree_cache *old_treecache = asp->treecache;

    /*
     * malloc new space 
     */
    asp->treecache =
        (netsnmp_tree_cache *) calloc(asp->treecache_len,
                                      sizeof(netsnmp_tree_cache));
    asp->treecache_num = -1;
    if (asp->cache_store) {
        netsnmp_free_cachemap(asp->cache_store);
        asp->cache_store = NULL;
    }

    for (i = 0; i < asp->vbcount; i++) {
        if (asp->requests[i].requestvb->type == ASN_NULL) {
            if (!netsnmp_add_varbind_to_cache(asp, asp->requests[i].index,
                                              asp->requests[i].requestvb,
                                              asp->requests[i].subtree->next)) {
                if (old_treecache != NULL) {
                    free(old_treecache);
                    old_treecache = NULL;
                }
            }
        } else if (asp->requests[i].requestvb->type == ASN_PRIV_RETRY) {
            /*
             * re-add the same subtree 
             */
            asp->requests[i].requestvb->type = ASN_NULL;
            if (!netsnmp_add_varbind_to_cache(asp, asp->requests[i].index,
                                              asp->requests[i].requestvb,
                                              asp->requests[i].subtree)) {
                if (old_treecache != NULL) {
                    free(old_treecache);
                    old_treecache = NULL;
                }
            }
        }
    }

    if (old_treecache != NULL) {
        free(old_treecache);
    }
    return SNMP_ERR_NOERROR;
}

void
netsnmp_delete_request_infos(netsnmp_request_info *reqlist)
{
    while (reqlist) {
        netsnmp_free_request_data_sets(reqlist);
        reqlist = reqlist->next;
    }
}

void
netsnmp_delete_subtree_cache(netsnmp_agent_session *asp)
{
    while (asp->treecache_num >= 0) {
        /*
         * don't delete subtrees 
         */
        netsnmp_delete_request_infos(asp->treecache[asp->treecache_num].
                                     requests_begin);
        asp->treecache_num--;
    }
}

int
netsnmp_check_requests_status(netsnmp_agent_session *asp,
                              netsnmp_request_info *requests,
                              int look_for_specific)
{
    /*
     * find any errors marked in the requests 
     */
    while (requests) {
        if (requests->status != SNMP_ERR_NOERROR &&
            (!look_for_specific || requests->status == look_for_specific)
            && (look_for_specific || asp->index == 0
                || requests->index < asp->index)) {
            asp->index = requests->index;
            asp->status = requests->status;
        }
        requests = requests->next;
    }
    return asp->status;
}

int
netsnmp_check_all_requests_status(netsnmp_agent_session *asp,
                                  int look_for_specific)
{
    int             i;
    for (i = 0; i <= asp->treecache_num; i++) {
        netsnmp_check_requests_status(asp,
                                      asp->treecache[i].requests_begin,
                                      look_for_specific);
    }
    return asp->status;
}

int
handle_var_requests(netsnmp_agent_session *asp)
{
    int             i, retstatus = SNMP_ERR_NOERROR,
        status = SNMP_ERR_NOERROR, final_status = SNMP_ERR_NOERROR;
    netsnmp_handler_registration *reginfo;

    /*
     * create the netsnmp_agent_request_info data 
     */
    if (!asp->reqinfo) {
        asp->reqinfo = SNMP_MALLOC_TYPEDEF(netsnmp_agent_request_info);
        if (!asp->reqinfo)
            return SNMP_ERR_GENERR;
    }

    asp->reqinfo->asp = asp;
    asp->reqinfo->mode = asp->mode;

    /*
     * now, have the subtrees in the cache go search for their results 
     */
    for (i = 0; i <= asp->treecache_num; i++) {
        reginfo = asp->treecache[i].subtree->reginfo;
        status = netsnmp_call_handlers(reginfo, asp->reqinfo,
                                       asp->treecache[i].requests_begin);

        /*
         * find any errors marked in the requests.  For later parts of
         * SET processing, only check for new errors specific to that
         * set processing directive (which must superceed the previous
         * errors).
         */
        switch (asp->mode) {
        case MODE_SET_COMMIT:
            retstatus = netsnmp_check_requests_status(asp,
						      asp->treecache[i].
						      requests_begin,
						      SNMP_ERR_COMMITFAILED);
            break;

        case MODE_SET_UNDO:
            retstatus = netsnmp_check_requests_status(asp,
						      asp->treecache[i].
						      requests_begin,
						      SNMP_ERR_UNDOFAILED);
            break;

        default:
            retstatus = netsnmp_check_requests_status(asp,
						      asp->treecache[i].
						      requests_begin, 0);
            break;
        }

        /*
         * always take lowest varbind if possible 
         */
        if (retstatus != SNMP_ERR_NOERROR) {
            status = retstatus;
	}

        /*
         * other things we know less about (no index) 
         */
        /*
         * WWW: drop support for this? 
         */
        if (final_status == SNMP_ERR_NOERROR && status != SNMP_ERR_NOERROR) {
            /*
             * we can't break here, since some processing needs to be
             * done for all requests anyway (IE, SET handling for UNDO
             * needs to be called regardless of previous status
             * results.
             * WWW:  This should be predictable though and
             * breaking should be possible in some cases (eg GET,
             * GETNEXT, ...) 
             */
            final_status = status;
        }
    }

    return final_status;
}

/*
 * loop through our sessions known delegated sessions and check to see
 * if they've completed yet. If there are no more delegated sessions,
 * check for and process any queued requests
 */
void
netsnmp_check_outstanding_agent_requests(void)
{
    netsnmp_agent_session *asp, *prev_asp = NULL, *next_asp = NULL;

    /*
     * deal with delegated requests
     */
    for (asp = agent_delegated_list; asp; prev_asp = asp, asp = next_asp) {
        next_asp = asp->next;   /* save in case we clean up asp */
        if (!netsnmp_check_for_delegated(asp)) {

            /*
             * we're done with this one, remove from queue 
             */
            if (prev_asp != NULL)
                prev_asp->next = asp->next;
            else
                agent_delegated_list = asp->next;

            /*
             * check request status
             */
            netsnmp_check_all_requests_status(asp, 0);
            
            /*
             * continue processing or finish up 
             */
            check_delayed_request(asp);
        }
    }

    /*
     * if we are processing a set and there are more delegated
     * requests, keep waiting before getting to queued requests.
     */
    if (netsnmp_processing_set && (NULL != agent_delegated_list))
        return;

    while (netsnmp_agent_queued_list) {

        /*
         * if we are processing a set, the first item better be
         * the set being (or waiting to be) processed.
         */
        netsnmp_assert((!netsnmp_processing_set) ||
                       (netsnmp_processing_set == netsnmp_agent_queued_list));

        /*
         * if the top request is a set, don't pop it
         * off if there are delegated requests
         */
        if ((netsnmp_agent_queued_list->pdu->command == SNMP_MSG_SET) &&
            (agent_delegated_list)) {

            netsnmp_assert(netsnmp_processing_set == NULL);

            netsnmp_processing_set = netsnmp_agent_queued_list;
            DEBUGMSGTL(("snmp_agent", "SET request remains queued while "
                        "delegated requests finish, asp = %08p\n", asp));
            break;
        }

        /*
         * pop the first request and process it
         */
        asp = netsnmp_agent_queued_list;
        netsnmp_agent_queued_list = asp->next;
        DEBUGMSGTL(("snmp_agent",
                    "processing queued request, asp = %08p\n", asp));

        netsnmp_handle_request(asp, asp->status);

        /*
         * if we hit a set, stop
         */
        if (NULL != netsnmp_processing_set)
            break;
    }
}

/** Decide if the requested transaction_id is still being processed
   within the agent.  This is used to validate whether a delayed cache
   (containing possibly freed pointers) is still usable.

   returns SNMPERR_SUCCESS if it's still valid, or SNMPERR_GENERR if not. */
int
netsnmp_check_transaction_id(int transaction_id)
{
    netsnmp_agent_session *asp, *prev_asp = NULL;

    for (asp = agent_delegated_list; asp; prev_asp = asp, asp = asp->next) {
        if (asp->pdu->transid == transaction_id)
            return SNMPERR_SUCCESS;
    }
    return SNMPERR_GENERR;
}


/*
 * check_delayed_request(asp)
 *
 * Called to rexamine a set of requests and continue processing them
 * once all the previous (delayed) requests have been handled one way
 * or another.
 */

int
check_delayed_request(netsnmp_agent_session *asp)
{
    int             status = SNMP_ERR_NOERROR;

    DEBUGMSGTL(("snmp_agent", "processing delegated request, asp = %08p\n",
                asp));

    switch (asp->mode) {
    case SNMP_MSG_GETBULK:
    case SNMP_MSG_GETNEXT:
        netsnmp_check_all_requests_status(asp, 0);
        handle_getnext_loop(asp);
        if (netsnmp_check_for_delegated(asp) &&
            netsnmp_check_transaction_id(asp->pdu->transid) !=
            SNMPERR_SUCCESS) {
            /*
             * add to delegated request chain 
             */
            if (!netsnmp_check_delegated_chain_for(asp)) {
                asp->next = agent_delegated_list;
                agent_delegated_list = asp;
            }
        }
        break;

    case MODE_SET_COMMIT:
        netsnmp_check_all_requests_status(asp, SNMP_ERR_COMMITFAILED);
        goto settop;

    case MODE_SET_UNDO:
        netsnmp_check_all_requests_status(asp, SNMP_ERR_UNDOFAILED);
        goto settop;

    case MODE_SET_BEGIN:
    case MODE_SET_RESERVE1:
    case MODE_SET_RESERVE2:
    case MODE_SET_ACTION:
    case MODE_SET_FREE:
      settop:
        handle_set_loop(asp);
        if (asp->mode != FINISHED_SUCCESS && asp->mode != FINISHED_FAILURE) {

            if (netsnmp_check_for_delegated_and_add(asp)) {
                /*
                 * add to delegated request chain 
                 */
                if (!asp->status)
                    asp->status = status;
            }

            return SNMP_ERR_NOERROR;
        }
        break;

    default:
        break;
    }

    /*
     * if we don't have anything outstanding (delegated), wrap up 
     */
    if (!netsnmp_check_for_delegated(asp))
        return netsnmp_wrap_up_request(asp, status);

    return 1;
}

/** returns 1 if there are valid GETNEXT requests left.  Returns 0 if not. */
int
check_getnext_results(netsnmp_agent_session *asp)
{
    /*
     * get old info 
     */
    netsnmp_tree_cache *old_treecache = asp->treecache;
    int             old_treecache_num = asp->treecache_num;
    int             count = 0;
    int             i, special = 0;
    netsnmp_request_info *request;

    if (asp->mode == SNMP_MSG_GET) {
        /*
         * Special case for doing INCLUSIVE getNext operations in
         * AgentX subagents.  
         */
        DEBUGMSGTL(("snmp_agent",
                    "asp->mode == SNMP_MSG_GET in ch_getnext\n"));
        asp->mode = asp->oldmode;
        special = 1;
    }

    for (i = 0; i <= old_treecache_num; i++) {
        for (request = old_treecache[i].requests_begin; request;
             request = request->next) {

            /*
             * If we have just done the special case AgentX GET, then any
             * requests which were not INCLUSIVE will now have a wrong
             * response, so junk them and retry from the same place (except
             * that this time the handler will be called in "inexact"
             * mode).  
             */

            if (special) {
                if (!request->inclusive) {
                    DEBUGMSGTL(("snmp_agent",
                                "request %d wasn't inclusive\n",
                                request->index));
                    snmp_set_var_typed_value(request->requestvb,
                                             ASN_PRIV_RETRY, NULL, 0);
                } else if (request->requestvb->type == ASN_NULL ||
                           request->requestvb->type == SNMP_NOSUCHINSTANCE ||
                           request->requestvb->type == SNMP_NOSUCHOBJECT) {
                    /*
                     * it was inclusive, but no results.  Still retry this
                     * search. 
                     */
                    snmp_set_var_typed_value(request->requestvb,
                                             ASN_PRIV_RETRY, NULL, 0);
                }
            }

            /*
             * out of range? 
             */
            if (snmp_oid_compare(request->requestvb->name,
                                 request->requestvb->name_length,
                                 request->range_end,
                                 request->range_end_len) >= 0) {
                /*
                 * ack, it's beyond the accepted end of range. 
                 */
                /*
                 * fix it by setting the oid to the end of range oid instead 
                 */
                DEBUGMSGTL(("check_getnext_results",
                            "request response %d out of range\n",
                            request->index));
                request->inclusive = 1;
                /*
                 * XXX: should set this to the original OID? 
                 */
                snmp_set_var_objid(request->requestvb,
                                   request->range_end,
                                   request->range_end_len);
                snmp_set_var_typed_value(request->requestvb, ASN_NULL,
                                         NULL, 0);
            }

            /*
             * mark any existent requests with illegal results as NULL 
             */
            if (request->requestvb->type == SNMP_ENDOFMIBVIEW) {
                /*
                 * illegal response from a subagent.  Change it back to NULL 
                 */
                request->requestvb->type = ASN_NULL;
                request->inclusive = 1;
            }

            if (request->requestvb->type == ASN_NULL ||
                request->requestvb->type == ASN_PRIV_RETRY ||
                (asp->reqinfo->mode == MODE_GETBULK
                 && request->repeat > 0))
                count++;
        }
    }
    return count;
}

/** repeatedly calls getnext handlers looking for an answer till all
   requests are satisified.  It's expected that one pass has been made
   before entering this function */
int
handle_getnext_loop(netsnmp_agent_session *asp)
{
    int             status;
    netsnmp_variable_list *var_ptr;

    /*
     * loop 
     */
    while (1) {

        /*
         * bail for now if anything is delegated. 
         */
        if (netsnmp_check_for_delegated(asp)) {
            return SNMP_ERR_NOERROR;
        }

        /*
         * check vacm against results 
         */
        check_acm(asp, ASN_PRIV_RETRY);

        /*
         * need to keep going we're not done yet. 
         */
        if (!check_getnext_results(asp))
            /*
             * nothing left, quit now 
             */
            break;

        /*
         * never had a request (empty pdu), quit now 
         */
        /*
         * XXXWWW: huh?  this would be too late, no?  shouldn't we
         * catch this earlier? 
         */
        /*
         * if (count == 0)
         * break; 
         */

        DEBUGIF("results") {
            DEBUGMSGTL(("results",
                        "getnext results, before next pass:\n"));
            for (var_ptr = asp->pdu->variables; var_ptr;
                 var_ptr = var_ptr->next_variable) {
                DEBUGMSGTL(("results", "\t"));
                DEBUGMSGVAR(("results", var_ptr));
                DEBUGMSG(("results", "\n"));
            }
        }

        netsnmp_reassign_requests(asp);
        status = handle_var_requests(asp);
        if (status != SNMP_ERR_NOERROR) {
            return status;      /* should never really happen */
        }
    }
    return SNMP_ERR_NOERROR;
}

int
handle_set(netsnmp_agent_session *asp)
{
    int             status;
    /*
     * SETS require 3-4 passes through the var_op_list.
     * The first two
     * passes verify that all types, lengths, and values are valid
     * and may reserve resources and the third does the set and a
     * fourth executes any actions.  Then the identical GET RESPONSE
     * packet is returned.
     * If either of the first two passes returns an error, another
     * pass is made so that any reserved resources can be freed.
     * If the third pass returns an error, another pass is
     * made so that
     * any changes can be reversed.
     * If the fourth pass (or any of the error handling passes)
     * return an error, we'd rather not know about it!
     */
    if (!(asp->pdu->flags & UCD_MSG_FLAG_ONE_PASS_ONLY)) {
        switch (asp->mode) {
        case MODE_SET_BEGIN:
            snmp_increment_statistic(STAT_SNMPINSETREQUESTS);
            asp->rw = WRITE;    /* WWW: still needed? */
            asp->mode = MODE_SET_RESERVE1;
            asp->status = SNMP_ERR_NOERROR;
            break;

        case MODE_SET_RESERVE1:

            if (asp->status != SNMP_ERR_NOERROR)
                asp->mode = MODE_SET_FREE;
            else
                asp->mode = MODE_SET_RESERVE2;
            break;

        case MODE_SET_RESERVE2:
            if (asp->status != SNMP_ERR_NOERROR)
                asp->mode = MODE_SET_FREE;
            else
                asp->mode = MODE_SET_ACTION;
            break;

        case MODE_SET_ACTION:
            if (asp->status != SNMP_ERR_NOERROR)
                asp->mode = MODE_SET_UNDO;
            else
                asp->mode = MODE_SET_COMMIT;
            break;

        case MODE_SET_COMMIT:
            if (asp->status != SNMP_ERR_NOERROR) {
                asp->mode = FINISHED_FAILURE;
            } else {
                asp->mode = FINISHED_SUCCESS;
            }
            break;

        case MODE_SET_UNDO:
            asp->mode = FINISHED_FAILURE;
            break;

        case MODE_SET_FREE:
            asp->mode = FINISHED_FAILURE;
            break;
        }
    }

    if (asp->mode != FINISHED_SUCCESS && asp->mode != FINISHED_FAILURE) {
        DEBUGMSGTL(("agent_set", "doing set mode = %d (%s)\n", asp->mode,
                    se_find_label_in_slist("agent_mode", asp->mode)));
        status = handle_var_requests(asp);
        DEBUGMSGTL(("agent_set", "did set mode = %d, status = %d\n",
                    asp->mode, status));
        if ((status != SNMP_ERR_NOERROR && asp->status == SNMP_ERR_NOERROR) ||
	    status == SNMP_ERR_COMMITFAILED || 
	    status == SNMP_ERR_UNDOFAILED) {
            asp->status = status;
        }
    }
    return asp->status;
}

int
handle_set_loop(netsnmp_agent_session *asp)
{
    while (asp->mode != FINISHED_FAILURE && asp->mode != FINISHED_SUCCESS) {
        handle_set(asp);
        if (netsnmp_check_for_delegated(asp)) {
            return SNMP_ERR_NOERROR;
	}
        if (asp->pdu->flags & UCD_MSG_FLAG_ONE_PASS_ONLY) {
            return asp->status;
	}
    }
    return asp->status;
}

int
netsnmp_handle_request(netsnmp_agent_session *asp, int status)
{
    /*
     * if this isn't a delegated request trying to finish,
     * processing of a set request should not start until all
     * delegated requests have completed, and no other new requests
     * should be processed until the set request completes.
     */
    if ((0 == netsnmp_check_delegated_chain_for(asp)) &&
        (asp != netsnmp_processing_set)) {
        /*
         * if we are processing a set and this is not a delegated
         * request, queue the request
         */
        if (netsnmp_processing_set) {
            netsnmp_add_queued(asp);
            DEBUGMSGTL(("snmp_agent",
                        "request queued while processing set, "
                        "asp = %08p\n", asp));
            return 1;
        }

        /*
         * check for set request
         */
        if (asp->pdu->command == SNMP_MSG_SET) {
            netsnmp_processing_set = asp;

            /*
             * if there are delegated requests, we must wait for them
             * to finishd.
             */
            if (agent_delegated_list) {
                DEBUGMSGTL(("snmp_agent", "SET request queued while "
                            "delegated requests finish, asp = %08p\n",
                            asp));
                netsnmp_add_queued(asp);
                return 1;
            }
        }
    }

    /*
     * process the request 
     */
    status = handle_pdu(asp);

    /*
     * print the results in appropriate debugging mode 
     */
    DEBUGIF("results") {
        netsnmp_variable_list *var_ptr;
        DEBUGMSGTL(("results", "request results (status = %d):\n",
                    status));
        for (var_ptr = asp->pdu->variables; var_ptr;
             var_ptr = var_ptr->next_variable) {
            DEBUGMSGTL(("results", "\t"));
            DEBUGMSGVAR(("results", var_ptr));
            DEBUGMSG(("results", "\n"));
        }
    }

    /*
     * check for uncompleted requests 
     */
    if (netsnmp_check_for_delegated_and_add(asp)) {
        /*
         * add to delegated request chain 
         */
        asp->status = status;
    } else {
        /*
         * if we don't have anything outstanding (delegated), wrap up
         */
        return netsnmp_wrap_up_request(asp, status);
    }

    return 1;
}

int
handle_pdu(netsnmp_agent_session *asp)
{
    int             status, inclusives = 0;
    netsnmp_variable_list *v = NULL;

    /*
     * for illegal requests, mark all nodes as ASN_NULL 
     */
    switch (asp->pdu->command) {

    case SNMP_MSG_INTERNAL_SET_RESERVE2:
    case SNMP_MSG_INTERNAL_SET_ACTION:
    case SNMP_MSG_INTERNAL_SET_COMMIT:
    case SNMP_MSG_INTERNAL_SET_FREE:
    case SNMP_MSG_INTERNAL_SET_UNDO:
        status = get_set_cache(asp);
        if (status != SNMP_ERR_NOERROR)
            return status;
        break;

    case SNMP_MSG_GET:
    case SNMP_MSG_GETNEXT:
    case SNMP_MSG_GETBULK:
        for (v = asp->pdu->variables; v != NULL; v = v->next_variable) {
            if (v->type == ASN_PRIV_INCL_RANGE) {
                /*
                 * Leave the type for now (it gets set to
                 * ASN_NULL in netsnmp_add_varbind_to_cache,
                 * called by create_subnetsnmp_tree_cache below).
                 * If we set it to ASN_NULL now, we wouldn't be
                 * able to distinguish INCLUSIVE search
                 * ranges.  
                 */
                inclusives++;
            } else {
                snmp_set_var_typed_value(v, ASN_NULL, NULL, 0);
            }
        }
        /*
         * fall through 
         */

    case SNMP_MSG_INTERNAL_SET_BEGIN:
    case SNMP_MSG_INTERNAL_SET_RESERVE1:
    default:
        asp->vbcount = count_varbinds(asp->pdu->variables);
        if (asp->vbcount) /* efence doesn't like 0 size allocs */
            asp->requests = (netsnmp_request_info *)
                calloc(asp->vbcount, sizeof(netsnmp_request_info));
        /*
         * collect varbinds 
         */
        status = netsnmp_create_subtree_cache(asp);
        if (status != SNMP_ERR_NOERROR)
            return status;
    }

    asp->mode = asp->pdu->command;
    switch (asp->mode) {
    case SNMP_MSG_GET:
        /*
         * increment the message type counter 
         */
        snmp_increment_statistic(STAT_SNMPINGETREQUESTS);

        /*
         * check vacm ahead of time 
         */
        check_acm(asp, SNMP_NOSUCHOBJECT);

        /*
         * get the results 
         */
        status = handle_var_requests(asp);

        /*
         * Deal with unhandled results -> noSuchInstance (rather
         * than noSuchObject -- in that case, the type will
         * already have been set to noSuchObject when we realised
         * we couldn't find an appropriate tree).  
         */
        if (status == SNMP_ERR_NOERROR)
            snmp_replace_var_types(asp->pdu->variables, ASN_NULL,
                                   SNMP_NOSUCHINSTANCE);
        break;

    case SNMP_MSG_GETNEXT:
        snmp_increment_statistic(STAT_SNMPINGETNEXTS);
        /*
         * fall through 
         */

    case SNMP_MSG_GETBULK:     /* note: there is no getbulk stat */
        /*
         * loop through our mib tree till we find an
         * appropriate response to return to the caller. 
         */

        if (inclusives) {
            /*
             * This is a special case for AgentX INCLUSIVE getNext
             * requests where a result lexi-equal to the request is okay
             * but if such a result does not exist, we still want the
             * lexi-next one.  So basically we do a GET first, and if any
             * of the INCLUSIVE requests are satisfied, we use that
             * value.  Then, unsatisfied INCLUSIVE requests, and
             * non-INCLUSIVE requests get done as normal.  
             */

            DEBUGMSGTL(("snmp_agent", "inclusive range(s) in getNext\n"));
            asp->oldmode = asp->mode;
            asp->mode = SNMP_MSG_GET;
        }

        /*
         * first pass 
         */
        status = handle_var_requests(asp);
        if (status != SNMP_ERR_NOERROR) {
            if (!inclusives)
                return status;  /* should never really happen */
            else
                asp->status = SNMP_ERR_NOERROR;
        }

        /*
         * loop through our mib tree till we find an
         * appropriate response to return to the caller. 
         */

        status = handle_getnext_loop(asp);
        break;

    case SNMP_MSG_SET:
        /*
         * check access permissions first 
         */
        if (check_acm(asp, SNMP_NOSUCHOBJECT))
            return SNMP_ERR_NOTWRITABLE;

        asp->mode = MODE_SET_BEGIN;
        status = handle_set_loop(asp);

        break;

    case SNMP_MSG_INTERNAL_SET_BEGIN:
    case SNMP_MSG_INTERNAL_SET_RESERVE1:
    case SNMP_MSG_INTERNAL_SET_RESERVE2:
    case SNMP_MSG_INTERNAL_SET_ACTION:
    case SNMP_MSG_INTERNAL_SET_COMMIT:
    case SNMP_MSG_INTERNAL_SET_FREE:
    case SNMP_MSG_INTERNAL_SET_UNDO:
        asp->pdu->flags |= UCD_MSG_FLAG_ONE_PASS_ONLY;
        status = handle_set_loop(asp);
        /*
         * asp related cache is saved in cleanup 
         */
        break;

    case SNMP_MSG_RESPONSE:
        snmp_increment_statistic(STAT_SNMPINGETRESPONSES);
        return SNMP_ERR_NOERROR;

    case SNMP_MSG_TRAP:
    case SNMP_MSG_TRAP2:
        snmp_increment_statistic(STAT_SNMPINTRAPS);
        return SNMP_ERR_NOERROR;

    default:
        /*
         * WWW: are reports counted somewhere ? 
         */
        snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
        return SNMPERR_GENERR;  /* shouldn't get here */
        /*
         * WWW 
         */
    }
    return status;
}

int
netsnmp_set_request_error(netsnmp_agent_request_info *reqinfo,
                          netsnmp_request_info *request, int error_value)
{
    if (!request || !reqinfo)
        return error_value;

    return netsnmp_set_mode_request_error(reqinfo->mode, request,
                                          error_value);
}

int
netsnmp_set_mode_request_error(int mode, netsnmp_request_info *request,
                               int error_value)
{
    if (!request)
        return error_value;

    request->processed = 1;
    request->delegated = REQUEST_IS_NOT_DELEGATED;

    switch (error_value) {
    case SNMP_NOSUCHOBJECT:
    case SNMP_NOSUCHINSTANCE:
    case SNMP_ENDOFMIBVIEW:
        /*
         * these are exceptions that should be put in the varbind
         * in the case of a GET but should be translated for a SET
         * into a real error status code and put in the request 
         */
        switch (mode) {
        case MODE_GET:
            request->requestvb->type = error_value;
            return error_value;

        case MODE_GETNEXT:
        case MODE_GETBULK:
            /*
             * ignore these.  They're illegal to set by the
             * client APIs for these modes 
             */
            snmp_log(LOG_ERR, "Illegal error_value %d for mode %d ignored\n",
                     error_value, mode);
            return error_value;

        default:
            request->status = SNMP_ERR_NOSUCHNAME;      /* WWW: correct? */
            return error_value;
        }
        break;                  /* never get here */

    default:
        if (request->status < 0) {
            /*
             * illegal local error code.  translate to generr 
             */
            /*
             * WWW: full translation map? 
             */
            snmp_log(LOG_ERR, "Illegal error_value %d translated to %d\n",
                     error_value, SNMP_ERR_GENERR);
            request->status = SNMP_ERR_GENERR;
        } else {
            /*
             * WWW: translations and mode checking? 
             */
            request->status = error_value;
        }
        return error_value;
    }
    return error_value;
}

int
netsnmp_set_all_requests_error(netsnmp_agent_request_info *reqinfo,
                               netsnmp_request_info *requests,
                               int error_value)
{
    while (requests) {
        netsnmp_set_request_error(reqinfo, requests, error_value);
        requests = requests->next;
    }
    return error_value;
}

extern struct timeval starttime;

                /*
                 * Return the value of 'sysUpTime' at the given marker 
                 */
u_long
netsnmp_marker_uptime(marker_t pm)
{
    u_long          res;
    marker_t        start = (marker_t) & starttime;

    res = uatime_hdiff(start, pm);
    return res;                 /* atime_diff works in msec, not csec */
}

                        /*
                         * struct timeval equivalents of these 
                         */
u_long
netsnmp_timeval_uptime(struct timeval * tv)
{
    return netsnmp_marker_uptime((marker_t) tv);
}

                /*
                 * Return the current value of 'sysUpTime' 
                 */
u_long
netsnmp_get_agent_uptime(void)
{
    struct timeval  now;
    gettimeofday(&now, NULL);

    return netsnmp_timeval_uptime(&now);
}



NETSNMP_INLINE void
netsnmp_agent_add_list_data(netsnmp_agent_request_info *ari,
                            netsnmp_data_list *node)
{
    if (ari) {
	if (ari->agent_data) {
            netsnmp_add_list_data(&ari->agent_data, node);
        } else {
            ari->agent_data = node;
	}
    }
}

NETSNMP_INLINE void    *
netsnmp_agent_get_list_data(netsnmp_agent_request_info *ari,
                            const char *name)
{
    if (ari) {
        return netsnmp_get_list_data(ari->agent_data, name);
    }
    return NULL;
}

NETSNMP_INLINE void
netsnmp_free_agent_data_set(netsnmp_agent_request_info *ari)
{
    if (ari) {
        netsnmp_free_list_data(ari->agent_data);
    }
}

NETSNMP_INLINE void
netsnmp_free_agent_data_sets(netsnmp_agent_request_info *ari)
{
    if (ari) {
        netsnmp_free_all_list_data(ari->agent_data);
    }
}

NETSNMP_INLINE void
netsnmp_free_agent_request_info(netsnmp_agent_request_info *ari)
{
    if (ari) {
        if (ari->agent_data) {
            netsnmp_free_all_list_data(ari->agent_data);
	}
        free(ari);
    }
}
