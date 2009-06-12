//==========================================================================
//
//      ./agent/current/src/snmp_vars.c
//
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//####UCDSNMPCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from the UCD-SNMP
// project,  <http://ucd-snmp.ucdavis.edu/>  from the University of
// California at Davis, which was originally based on the Carnegie Mellon
// University SNMP implementation.  Portions of this software are therefore
// covered by the appropriate copyright disclaimers included herein.
//
// The release used was version 4.1.2 of May 2000.  "ucd-snmp-4.1.2"
// -------------------------------------------
//
//####UCDSNMPCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    hmt
// Contributors: hmt
// Date:         2000-05-30
// Purpose:      Port of UCD-SNMP distribution to eCos.
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/********************************************************************
       Copyright 1989, 1991, 1992 by Carnegie Mellon University

			  Derivative Work -
Copyright 1996, 1998, 1999, 2000 The Regents of the University of California

			 All Rights Reserved

Permission to use, copy, modify and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both that copyright notice and this permission notice appear in
supporting documentation, and that the name of CMU and The Regents of
the University of California not be used in advertising or publicity
pertaining to distribution of the software without specific written
permission.

CMU AND THE REGENTS OF THE UNIVERSITY OF CALIFORNIA DISCLAIM ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL CMU OR
THE REGENTS OF THE UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
FROM THE LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*********************************************************************/
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


#include <config.h>
#if HAVE_STRING_H
#include <string.h>
#endif
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#if HAVE_FCNTL_H
#include <fcntl.h>
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
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#elif HAVE_WINSOCK_H
#include <winsock.h>
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

#include "mibincl.h"
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include "snmpv3.h"
#include "snmpusm.h"
#endif
#include "system.h"
#include "kernel.h"
#include "snmp_vars.h"
#include "default_store.h"
#include "ds_agent.h"

#include "mibgroup/struct.h"
#include "read_config.h"
#include "snmp_vars.h"
#include "agent_read_config.h"
#include "agent_registry.h"
#include "transform_oids.h"
#include "callback.h"
#include "snmp_alarm.h"
#include "snmpd.h"
#include "mib_module_includes.h"

#ifndef  MIN
#define  MIN(a,b)                     (((a) < (b)) ? (a) : (b)) 
#endif

/* mib clients are passed a pointer to a oid buffer.  Some mib clients
 * (namely, those first noticed in mibII/vacm.c) modify this oid buffer
 * before they determine if they really need to send results back out
 * using it.  If the master agent determined that the client was not the
 * right one to talk with, it will use the same oid buffer to pass to the
 * rest of the clients, which may not longer be valid.  This should be
 * fixed in all clients rather than the master.  However, its not a
 * particularily easy bug to track down so this saves debugging time at
 * the expense of a few memcpy's.
 */
#define MIB_CLIENTS_ARE_EVIL 1
 
extern struct subtree *subtrees;
int subtree_size;
int subtree_malloc_size;

/*
 *	Each variable name is placed in the variable table, without the
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
 * oid	    *name;	    IN/OUT - input name requested, output name found
 * int	    length;	    IN/OUT - number of sub-ids in the in and out oid's
 * int	    exact;	    IN - TRUE if an exact match was requested.
 * int	    len;	    OUT - length of variable or 0 if function returned.
 * int	    write_method;   OUT - pointer to function to set variable,
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
 * int	    action;	    IN - RESERVE1, RESERVE2, COMMIT, or FREE
 * u_char   *var_val;	    IN - input or output buffer space
 * u_char   var_val_type;   IN - type of input buffer
 * int	    var_val_len;    IN - input and output buffer len
 * u_char   *statP;	    IN - pointer to local statistic
 * oid      *name           IN - pointer to name requested
 * int      name_len        IN - number of sub-ids in the name
 */

long		long_return;
#ifndef ibm032
u_char		return_buf[258];  
#else
u_char		return_buf[256]; /* nee 64 */
#endif

struct timeval	starttime;

void
init_agent (const char *app)
{
  /* get current time (ie, the time the agent started) */
  gettimeofday(&starttime, NULL);
  starttime.tv_sec--;
  starttime.tv_usec += 1000000L;

  /* we handle alarm signals ourselves in the select loop */
  ds_set_boolean(DS_LIBRARY_ID, DS_LIB_ALARM_DONT_USE_SIG, 1);

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
  usm_set_reportErrorOnUnknownID(1);
#endif

#ifdef CAN_USE_NLIST
  init_kmem("/dev/kmem");
#endif

  setup_tree();

  init_agent_read_config(app);

#ifdef TESTING
  auto_nlist_print_tree(-2, 0);
#endif

  /* initialize agentx subagent if necessary. */
#ifdef USING_AGENTX_SUBAGENT_MODULE
  if(ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE) == SUB_AGENT)
      subagent_pre_init();
#endif

}  /* end init_agent() */



oid nullOid[] = {0,0};
int nullOidLen = sizeof(nullOid)/sizeof(oid);

/*
 * getStatPtr - return a pointer to the named variable, as well as it's
 * type, length, and access control list.
 * Now uses 'search_subtree' (recursively) and 'search_subtree_vars'
 * to do most of the work
 *
 * If an exact match for the variable name exists, it is returned.  If not,
 * and exact is false, the next variable lexicographically after the
 * requested one is returned.
 *
 * If no appropriate variable can be found, NULL is returned.
 */
static  int 		found;

static u_char *
search_subtree_vars(struct subtree *tp,
		    oid *name,    /* IN - name of var, OUT - name matched */
		    size_t *namelen, /* IN -number of sub-ids in name,
                                     OUT - subid-is in matched name */
		    u_char *type, /* OUT - type of matched variable */
		    size_t *len,  /* OUT - length of matched variable */
		    u_short *acl, /* OUT - access control list */
		    int exact,    /* IN - TRUE if exact match wanted */
		    WriteMethod **write_method,
		    struct snmp_pdu *pdu, /* IN - relevant auth info re PDU */
		    int *noSuchObject)
{
    register struct variable *vp;
    struct variable	compat_var, *cvp = &compat_var;
    register int	x;
    u_char		*access = NULL;
    int			result;
    oid 		*suffix;
    size_t		suffixlen;
#if MIB_CLIENTS_ARE_EVIL
    oid			save[MAX_OID_LEN];
    size_t		savelen = 0;
#endif

	    if ( tp->variables == NULL )
		return NULL;

	    result = compare_tree(name, *namelen, tp->name, tp->namelen);
	    suffixlen = *namelen - tp->namelen;
	    suffix = name + tp->namelen;
	    /* the following is part of the setup for the compatability
	       structure below that has been moved out of the main loop.
	     */
	    memcpy(cvp->name, tp->name, tp->namelen * sizeof(oid));

            *noSuchObject = TRUE;	/* In case of null variables_len */
	    for(x = 0, vp = tp->variables; x < tp->variables_len;
		vp =(struct variable *)((char *)vp +tp->variables_width), x++){
		/* if exact and ALWAYS
		   if next  and result >= 0 */
                /* and if vp->namelen != 0   -- Wes */
		if (vp->namelen && (exact || result >= 0)){
		    result = compare_tree(suffix, suffixlen, vp->name,
				     vp->namelen);
		}
		/* if exact and result == 0
		   if next  and result <= 0 */
                /* or if vp->namelen == 0    -- Wes */
		if ((!exact && (result <= 0)) || (exact && (result == 0)) ||
                  vp->namelen == 0) {
		    /* builds an old (long) style variable structure to retain
		       compatability with var_* functions written previously.
		     */
                  if (vp->namelen)
                    memcpy((cvp->name + tp->namelen),
			  vp->name, vp->namelen * sizeof(oid));
		    cvp->namelen = tp->namelen + vp->namelen;
		    cvp->type = vp->type;
		    cvp->magic = vp->magic;
		    cvp->acl = vp->acl;
		    cvp->findVar = vp->findVar;
                    *write_method = NULL;
#if MIB_CLIENTS_ARE_EVIL
                    memcpy(save, name, *namelen*sizeof(oid));
                    savelen = *namelen;
#endif
		    DEBUGMSGTL(("snmp_vars", "Trying variable: "));
		    DEBUGMSGOID(("snmp_vars", cvp->name, cvp->namelen));
		    DEBUGMSG(("snmp_vars"," ...\n"));

		gaga:
		    access = (*(vp->findVar))(cvp, name, namelen, exact,
						  len, write_method);
	    	    DEBUGMSGTL(("snmp_vars", "Returned %s\n",
				(access==NULL) ? "(null)" : "something" ));

			/*
			 * Check that the answer is acceptable.
			 *  i.e. lies within the current subtree chunk
			 *
			 * It might be worth saving this answer just in
			 *  case it turns out to be valid, but for now
			 *  we'll simply discard it.
			 */
		    if ( access && snmp_oid_compare(name, *namelen,
						    tp->end, tp->end_len) > 0) {
			memcpy(name, tp->end, tp->end_len);
			access = 0;
		    }
#if MIB_CLIENTS_ARE_EVIL
                    if (access == NULL) {
		      if (snmp_oid_compare(name, *namelen, save, savelen) != 0) {
			DEBUGMSGTL(("snmp_vars", "evil_client: "));
			DEBUGMSGOID(("snmp_vars", save, savelen));
			DEBUGMSG(("snmp_vars"," =>"));
			DEBUGMSGOID(("snmp_vars", name, *namelen));
			DEBUGMSG(("snmp_vars","\n"));
                        memcpy(name, save, savelen*sizeof(oid));
                        *namelen = savelen;
                      }
                    }
#endif
		    if (*write_method)
			*acl = cvp->acl;
                    /* check for permission to view this part of the OID tree */
		    if ((access != NULL || (*write_method != NULL && exact)) &&
                        in_a_view(name, namelen, pdu, cvp->type)) {
			if ( access && !exact ) {
				/*
				 * We've got an answer, but shouldn't use it.
				 * But we *might* be able to use a later
				 *  instance of the same object, so we can't
				 *  legitimately move on to the next variable
				 *  in the variable structure just yet.
				 * Let's try re-calling the findVar routine
				 *  with the returned name, and see whether
				 *  the next answer is acceptable
				 */
			   *write_method = NULL;
			   goto gaga;
			}
                        access = NULL;
			*write_method = NULL;
		    } else if (exact){
			found = TRUE;
		    }
		    if (access != NULL || (*write_method != NULL && exact))
			break;
		}
		/* if exact and result <= 0 */
		if (exact && (result  <= 0)){
	            *type = cvp->type;
		    *acl = cvp->acl;
		    if (found)
                      *noSuchObject = FALSE;
		    else
                      *noSuchObject = TRUE;
		    return NULL;
		}
	    }
	    if (access != NULL || (exact && *write_method != NULL)) {
	        *type = cvp->type;
		*acl = cvp->acl;
		return access;
	    }
	    return NULL;
}

u_char *
getStatPtr(
    oid		*name,	    /* IN - name of var, OUT - name matched */
    size_t	*namelen,   /* IN -number of sub-ids in name,
                               OUT - subid-is in matched name */
    u_char	*type,	    /* OUT - type of matched variable */
    size_t	*len,	    /* OUT - length of matched variable */
    u_short	*acl,	    /* OUT - access control list */
    int		exact,	    /* IN - TRUE if exact match wanted */
    WriteMethod **write_method,
    struct snmp_pdu *pdu,   /* IN - relevant auth info re PDU */
    int		*noSuchObject)
{
    struct subtree	*tp;
    oid			save[MAX_OID_LEN];
    size_t		savelen = 0;
    u_char              result_type;
    u_short             result_acl;
    u_char        	*search_return=NULL;

    found = FALSE;

    if (!exact){
	memcpy(save, name, *namelen * sizeof(oid));
	savelen = *namelen;
    }
    *write_method = NULL;

    DEBUGMSGTL(("snmp_vars", "Looking for: "));
    DEBUGMSGOID(("snmp_vars", name, *namelen));
    DEBUGMSG(("snmp_vars"," ...\n"));

    tp = find_subtree(name, *namelen, NULL);
    
    while ( search_return == NULL && tp != NULL ) {
	DEBUGMSGTL(("snmp_vars", "Trying tree: "));
	DEBUGMSGOID(("snmp_vars", tp->name, tp->namelen));
	DEBUGMSG(("snmp_vars"," ...\n"));
	search_return = search_subtree_vars( tp, name, namelen, &result_type,
                                        len, &result_acl, exact, write_method,
                                        pdu, noSuchObject);
	if ( search_return != NULL || exact )
	    break;
	tp = tp->next;
    }
    if ( tp == NULL ) {
	if (!search_return && !exact){
	    memcpy(name, save, savelen * sizeof(oid));
	    *namelen = savelen;
	}
	if (found)
	    *noSuchObject = FALSE;
	else
	    *noSuchObject = TRUE;
        return NULL;
    }
    *type = result_type;
    *acl =  result_acl;
    return search_return;
}

