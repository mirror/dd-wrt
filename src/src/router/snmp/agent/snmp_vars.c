/*
 * snmp_vars.c - return a pointer to the named variable.
 *
 *
 */
/***********************************************************
	Copyright 1988, 1989, 1990 by Carnegie Mellon University
	Copyright 1989	TGV, Incorporated

		      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and TGV not be used
in advertising or publicity pertaining to distribution of the software
without specific, written prior permission.

CMU AND TGV DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
EVENT SHALL CMU OR TGV BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
******************************************************************/
/*
 * additions, fixes and enhancements for Linux by Erik Schoenfelder
 * (schoenfr@ibr.cs.tu-bs.de) 1994/1995.
 * Linux additions taken from CMU to UCD stack by Jennifer Bray of Origin
 * (jbray@origin-at.co.uk) 1997
 */

/*
 * XXXWWW merge todo: incl/excl range changes in differences between
 * 1.194 and 1.199 
 */

#include <net-snmp/net-snmp-config.h>
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>

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
# include <winsock.h>
#endif
#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#if HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#if HAVE_SYS_SOCKETVAR_H
# include <sys/socketvar.h>
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

#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "kernel.h"

#include "mibgroup/struct.h"
#include "snmpd.h"
#include "net-snmp/agent/all_helpers.h"
#include "mib_module_includes.h"
#include "net-snmp/library/container.h"

#ifndef  MIN
#define  MIN(a,b)                     (((a) < (b)) ? (a) : (b))
#endif

/*
 * mib clients are passed a pointer to a oid buffer.  Some mib clients
 * * (namely, those first noticed in mibII/vacm.c) modify this oid buffer
 * * before they determine if they really need to send results back out
 * * using it.  If the master agent determined that the client was not the
 * * right one to talk with, it will use the same oid buffer to pass to the
 * * rest of the clients, which may not longer be valid.  This should be
 * * fixed in all clients rather than the master.  However, its not a
 * * particularily easy bug to track down so this saves debugging time at
 * * the expense of a few memcpy's.
 */
#define MIB_CLIENTS_ARE_EVIL 1

extern netsnmp_subtree *subtrees;

/*
 *      Each variable name is placed in the variable table, without the
 * terminating substring that determines the instance of the variable.  When
 * a string is found that is lexicographicly preceded by the input string,
 * the function for that entry is called to find the method of access of the
 * instance of the named variable.  If that variable is not found, NULL is
 * returned, and the search through the table continues (it will probably
 * stop at the next entry).  If it is found, the function returns a character
 * pointer and a length or a function pointer.  The former is the address
 * of the operand, the latter is a write routine for the variable.
 *
 * u_char *
 * findVar(name, length, exact, var_len, write_method)
 * oid      *name;          IN/OUT - input name requested, output name found
 * int      length;         IN/OUT - number of sub-ids in the in and out oid's
 * int      exact;          IN - TRUE if an exact match was requested.
 * int      len;            OUT - length of variable or 0 if function returned.
 * int      write_method;   OUT - pointer to function to set variable,
 *                                otherwise 0
 *
 *     The writeVar function is returned to handle row addition or complex
 * writes that require boundary checking or executing an action.
 * This routine will be called three times for each varbind in the packet.
 * The first time for each varbind, action is set to RESERVE1.  The type
 * and value should be checked during this pass.  If any other variables
 * in the MIB depend on this variable, this variable will be stored away
 * (but *not* committed!) in a place where it can be found by a call to
 * writeVar for a dependent variable, even in the same PDU.  During
 * the second pass, action is set to RESERVE2.  If this variable is dependent
 * on any other variables, it will check them now.  It must check to see
 * if any non-committed values have been stored for variables in the same
 * PDU that it depends on.  Sometimes resources will need to be reserved
 * in the first two passes to guarantee that the operation can proceed
 * during the third pass.  During the third pass, if there were no errors
 * in the first two passes, writeVar is called for every varbind with action
 * set to COMMIT.  It is now that the values should be written.  If there
 * were errors during the first two passes, writeVar is called in the third
 * pass once for each varbind, with the action set to FREE.  An opportunity
 * is thus provided to free those resources reserved in the first two passes.
 * 
 * writeVar(action, var_val, var_val_type, var_val_len, statP, name, name_len)
 * int      action;         IN - RESERVE1, RESERVE2, COMMIT, or FREE
 * u_char   *var_val;       IN - input or output buffer space
 * u_char   var_val_type;   IN - type of input buffer
 * int      var_val_len;    IN - input and output buffer len
 * u_char   *statP;         IN - pointer to local statistic
 * oid      *name           IN - pointer to name requested
 * int      name_len        IN - number of sub-ids in the name
 */

long            long_return;
#ifndef ibm032
u_char          return_buf[258];
#else
u_char          return_buf[256];        /* nee 64 */
#endif

struct timeval  starttime;
netsnmp_session *callback_master_sess;
int             callback_master_num;

/*
 * init_agent() returns non-zero on error 
 */
int
init_agent(const char *app)
{
    int             r = 0;

    /*
     * get current time (ie, the time the agent started) 
     */
    gettimeofday(&starttime, NULL);
    starttime.tv_sec--;
    starttime.tv_usec += 1000000L;

    /*
     * we handle alarm signals ourselves in the select loop 
     */
    netsnmp_ds_set_boolean(NETSNMP_DS_LIBRARY_ID, 
			   NETSNMP_DS_LIB_ALARM_DONT_USE_SIG, 1);

#ifdef CAN_USE_NLIST
    init_kmem("/dev/kmem");
#endif

    setup_tree();

    init_agent_read_config(app);

#ifdef TESTING
    auto_nlist_print_tree(-2, 0);
#endif

#ifndef WIN32
	/*
	 * the pipe call creates fds that select chokes on, so
	 * disable callbacks on WIN32 until a fix can be found
	 */
    /*
     * always register a callback transport for internal use 
     */
    callback_master_sess = netsnmp_callback_open(0, handle_snmp_packet,
                                                 netsnmp_agent_check_packet,
                                                 netsnmp_agent_check_parse);
    if (callback_master_sess)
        callback_master_num = callback_master_sess->local_port;
    else
#endif
        callback_master_num = -1;

    netsnmp_init_helpers();
    init_traps();
    netsnmp_container_init_list();

    /*
     * initialize agentx subagent if necessary. 
     */
#ifdef USING_AGENTX_SUBAGENT_MODULE
    if (netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID, 
			       NETSNMP_DS_AGENT_ROLE) == SUB_AGENT) {
        r = subagent_pre_init();
        init_subagent();
    }
#endif

    /*
     * Register configuration tokens from transport modules.  
     */
#ifdef SNMP_TRANSPORT_UDP_DOMAIN
    netsnmp_udp_agent_config_tokens_register();
#endif
#ifdef SNMP_TRANSPORT_UDPIPV6_DOMAIN
    netsnmp_udp6_agent_config_tokens_register();
#endif

#ifdef NETSNMP_EMBEDDED_PERL
    init_perl();
#endif

    return r;
}                               /* end init_agent() */

oid             nullOid[] = { 0, 0 };
int             nullOidLen = sizeof(nullOid);
