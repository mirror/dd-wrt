//==========================================================================
//
//      ./lib/current/src/snmp_client.c
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
 * snmp_client.c - a toolkit of common functions for an SNMP client.
 *
 */
/**********************************************************************
	Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

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

#include <config.h>

#include <stdio.h>
#include <errno.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
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
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#include "asn1.h"
#include "snmp.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp_client.h"
#include "mib.h"


#ifndef BSD4_3
#define BSD4_2
#endif

#ifndef FD_SET

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	memset((p), 0, sizeof(*(p)))
#endif

#define PARTY_MIB_BASE	 ".1.3.6.1.6.3.3.1.3.127.0.0.1.1"
#define CONTEXT_MIB_BASE ".1.3.6.1.6.3.3.1.4.127.0.0.1.1"


struct snmp_pdu *
snmp_pdu_create(int command)
{
    struct snmp_pdu *pdu;
    struct sockaddr_in *pduIp;

    pdu = (struct snmp_pdu *)calloc(1,sizeof(struct snmp_pdu));
    if (pdu) {
    pduIp = (struct sockaddr_in *)&(pdu->address);
    pdu->version		 = SNMP_DEFAULT_VERSION;
    pdu->command		 = command;
    pdu->errstat		 = SNMP_DEFAULT_ERRSTAT;
    pdu->errindex		 = SNMP_DEFAULT_ERRINDEX;
    pduIp->sin_addr.s_addr       = SNMP_DEFAULT_ADDRESS;
    pdu->securityNameLen	 = 0;
    pdu->contextNameLen		 = 0;
    pdu->reqid                   = snmp_get_next_reqid();
    pdu->msgid                   = snmp_get_next_msgid();
    }
    return pdu;

}


/*
 * Add a null variable with the requested name to the end of the list of
 * variables for this pdu.
 */
struct variable_list* snmp_add_null_var(struct snmp_pdu * pdu, 
					oid *name, 
					size_t name_length)
{
    return snmp_pdu_add_variable(pdu, name, name_length, ASN_NULL, NULL, 0);
}



int
snmp_synch_input(int op,
		 struct snmp_session *session,
		 int reqid,
		 struct snmp_pdu *pdu,
		 void *magic)
{
    struct synch_state *state = (struct synch_state *)magic;
    int rpt_type;

    if (reqid != state->reqid && pdu->command != SNMP_MSG_REPORT)
	return 0;

    state->waiting = 0;
    if (op == RECEIVED_MESSAGE) {
      if (pdu->command == SNMP_MSG_REPORT) {
#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
	rpt_type = snmpv3_get_report_type(pdu);
	if (SNMPV3_IGNORE_UNAUTH_REPORTS || 
	    rpt_type == SNMPERR_NOT_IN_TIME_WINDOW) 
	  state->waiting = 1;
	session->s_snmp_errno = rpt_type;
#else
	session->s_snmp_errno = SNMPERR_UNSUPPORTED_SEC_LEVEL;
#endif
	state->pdu = NULL;
	state->status = STAT_ERROR;
        SET_SNMP_ERROR(rpt_type);
      } else if (pdu->command == SNMP_MSG_RESPONSE) {
	/* clone the pdu to return to snmp_synch_response */
	state->pdu = snmp_clone_pdu(pdu);
	state->status = STAT_SUCCESS;
	session->s_snmp_errno = SNMPERR_SUCCESS;
      }
    } else if (op == TIMED_OUT){
	state->pdu		 = NULL;
	state->status		 = STAT_TIMEOUT;
	session->s_snmp_errno	 = SNMPERR_TIMEOUT;
        SET_SNMP_ERROR(SNMPERR_TIMEOUT);
    }

    return 1;
}


/*
 * Clone an SNMP variable data structure.
 * Sets pointers to structure private storage, or
 * allocates larger object identifiers and values as needed.
 *
 * Caller must make list association for cloned variable.
 *
 * Returns 0 if successful.
 */
int
snmp_clone_var(struct variable_list *var, struct variable_list *newvar)
{
    if (!newvar || !var) return 1;

    memmove(newvar, var, sizeof(struct variable_list));
    newvar->next_variable = 0; newvar->name = 0; newvar->val.string = 0;

    /*
     * Clone the object identifier and the value.
     * Allocate memory iff original will not fit into local storage.
     */
    if (snmp_set_var_objid(newvar, var->name, var->name_length))
        return 1;

    /* need a pointer and a length to copy a string value. */
    if (var->val.string && var->val_len) {
      if (var->val.string != &var->buf[0]){
        if (var->val_len <= sizeof(var->buf))
            newvar->val.string = newvar->buf;
        else {
            newvar->val.string = (u_char *)malloc(var->val_len);
            if (!newvar->val.string) return 1;
        }
        memmove(newvar->val.string, var->val.string, var->val_len);
      }
      else { /* fix the pointer to new local store */
        newvar->val.string = newvar->buf;
      }
    }
    else {
        newvar->val.string = 0; newvar->val_len = 0;
    }

    return 0;
}


/*
 * Possibly make a copy of source memory buffer.
 * Will reset destination pointer if source pointer is NULL.
 * Returns 0 if successful, 1 if memory allocation fails.
 */
int
snmp_clone_mem(void ** dstPtr, void * srcPtr, unsigned len)
{
    *dstPtr = 0;
    if (srcPtr){
        *dstPtr = malloc(len + 1);
        if (! *dstPtr){
            return 1;
        }
        memmove(*dstPtr, srcPtr, len);
	/* this is for those routines that expect 0-terminated strings!!!
	   someone should rather have called strdup
	*/
	((char *)*dstPtr)[len] = 0;
    }
    return 0;
}


/*
 * Creates and allocates a clone of the input PDU,
 * but does NOT copy the variables.
 * This function should be used with another function,
 * such as _copy_pdu_vars.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
static
struct snmp_pdu *
_clone_pdu_header(struct snmp_pdu *pdu)
{
    struct snmp_pdu *newpdu;

    newpdu = (struct snmp_pdu *)malloc(sizeof(struct snmp_pdu));
    if (!newpdu) return 0;
    memmove(newpdu, pdu, sizeof(struct snmp_pdu));

    /* reset copied pointers if copy fails */
    newpdu->variables = 0; newpdu->enterprise = 0; newpdu->community = 0;
    newpdu->securityEngineID = 0; newpdu->securityName = 0;
    newpdu->contextEngineID  = 0; newpdu->contextName  = 0;

    /* copy buffers individually. If any copy fails, all are freed. */
    if ( snmp_clone_mem((void **)&newpdu->enterprise, pdu->enterprise,
                                    sizeof(oid)*pdu->enterprise_length)
     ||  snmp_clone_mem((void **)&newpdu->community, pdu->community,
                                    pdu->community_len)
#ifdef SNMPERR_UNSUPPORTED_SEC_LEVEL
     ||  snmp_clone_mem((void **)&newpdu->contextEngineID, pdu->contextEngineID,
                                    pdu->contextEngineIDLen)
     ||  snmp_clone_mem((void **)&newpdu->securityEngineID, pdu->securityEngineID,
                                    pdu->securityEngineIDLen)
     ||  snmp_clone_mem((void **)&newpdu->contextName, pdu->contextName,
                                    pdu->contextNameLen)
     ||  snmp_clone_mem((void **)&newpdu->securityName, pdu->securityName,
                                    pdu->securityNameLen)
#endif
       )
    {
        snmp_free_pdu(newpdu); return 0;
    }
    return newpdu;
}


/*
 * Copy some or all variables from source PDU to target PDU.
 * This function consolidates many of the needs of PDU variables:
 * Clone PDU : copy all the variables.
 * Split PDU : skip over some variables to copy other variables.
 * Fix PDU   : remove variable associated with error index.
 *
 * Designed to work with _clone_pdu_header.
 *
 * If drop_err is set, drop any variable associated with errindex.
 * If skip_count is set, skip the number of variable in pdu's list.
 * While copy_count is greater than zero, copy pdu variables to newpdu.
 *
 * If an error occurs, newpdu is freed and pointer is set to 0.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
static
struct snmp_pdu *
_copy_pdu_vars(struct snmp_pdu *pdu,  /* source PDU */
        struct snmp_pdu *newpdu,      /* target PDU */
        int drop_err,                 /* !=0 drop errored variable */
        int skip_count,               /* !=0 number of variables to skip */
        int copy_count)               /* !=0 number of variables to copy */
{
    struct variable_list *var, *newvar, *oldvar;
    int ii, copied;

    if (!newpdu) return 0;            /* where is PDU to copy to ? */

    var = pdu->variables;
    while (var && (skip_count-- > 0)) /* skip over pdu variables */
        var = var->next_variable;

    oldvar = 0; ii = 0; copied = 0;
    if (pdu->flags & UCD_MSG_FLAG_FORCE_PDU_COPY)
	copied = 1;	/* We're interested in 'empty' responses too */
    while (var && (copy_count-- > 0))
    {
        /* errindex starts from 1. If drop_err, skip the errored variable */
        if (drop_err && (++ii == pdu->errindex)) {
            var = var->next_variable; continue;
        }

        /* clone the next variable. Cleanup if alloc fails */
        newvar = (struct variable_list *)malloc(sizeof(struct variable_list));
        if (snmp_clone_var(var, newvar)){
            if (newvar) free((char *)newvar);
            snmp_free_pdu(newpdu); return 0;
        }
        copied++;

        /* add cloned variable to new PDU */
        if (0 == newpdu->variables) newpdu->variables = newvar;
        if (oldvar) oldvar->next_variable = newvar;
        oldvar = newvar;

        var = var->next_variable;
    }
    /* Error if bad errindex or if target PDU has no variables copied */
    if ((drop_err && (ii < pdu->errindex))
#if TEMPORARILY_DISABLED
		/* SNMPv3 engineID probes are allowed to be empty.
		   See the comment in snmp_api.c for further details */
        || copied == 0
#endif
      ) {
        snmp_free_pdu(newpdu); return 0;
    }
    return newpdu;
}


/*
 * Creates (allocates and copies) a clone of the input PDU.
 * If drop_err is set, don't copy any variable associated with errindex.
 * This function is called by snmp_clone_pdu and snmp_fix_pdu.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
static
struct snmp_pdu *
_clone_pdu(struct snmp_pdu *pdu, int drop_err)
{
    struct snmp_pdu *newpdu;
    newpdu = _clone_pdu_header(pdu);
    newpdu = _copy_pdu_vars(pdu, newpdu,
	    drop_err,
	    0, 10000);  /* skip none, copy all */

    return newpdu;
}


/*
 * This function will clone a PDU including all of its variables.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure
 */
struct snmp_pdu *
snmp_clone_pdu(struct snmp_pdu *pdu)
{
    return _clone_pdu(pdu, 0); /* copies all variables */
}


/*
 * This function will clone a PDU including some of its variables.
 *
 * If skip_count is not zero, it defines the number of variables to skip.
 * If copy_count is not zero, it defines the number of variables to copy.
 *
 * Returns a pointer to the cloned PDU if successful.
 * Returns 0 if failure.
 */
struct snmp_pdu *
snmp_split_pdu(struct snmp_pdu *pdu, int skip_count, int copy_count)
{
    struct snmp_pdu *newpdu;
    newpdu = _clone_pdu_header(pdu);
    newpdu = _copy_pdu_vars(pdu, newpdu,
	    0,         /* don't drop any variables */
	    skip_count,
            copy_count);

    return newpdu;
}


/*
 * If there was an error in the input pdu, creates a clone of the pdu
 * that includes all the variables except the one marked by the errindex.
 * The command is set to the input command and the reqid, errstat, and
 * errindex are set to default values.
 * If the error status didn't indicate an error, the error index didn't
 * indicate a variable, the pdu wasn't a get response message, or there
 * would be no remaining variables, this function will return 0.
 * If everything was successful, a pointer to the fixed cloned pdu will
 * be returned.
 */
struct snmp_pdu *
snmp_fix_pdu(struct snmp_pdu *pdu, int command)
{
    struct snmp_pdu *newpdu;

    if ((pdu->command != SNMP_MSG_RESPONSE)
     || (pdu->errstat == SNMP_ERR_NOERROR)
     || (0 == pdu->variables)
     || (pdu->errindex <= 0))
    {
            return 0; /* pre-condition tests fail */
    }

    newpdu = _clone_pdu(pdu, 1); /* copies all except errored variable */
    if (!newpdu)
        return 0;
    if (!newpdu->variables) {
        snmp_free_pdu(newpdu);
        return 0; /* no variables. "should not happen" */
    }
    newpdu->command = command;
    newpdu->reqid = snmp_get_next_reqid();
    newpdu->msgid = snmp_get_next_msgid();
    newpdu->errstat = SNMP_DEFAULT_ERRSTAT;
    newpdu->errindex = SNMP_DEFAULT_ERRINDEX;

    return newpdu;
}


/*
 * Returns the number of variables bound to a PDU structure
 */
unsigned long
snmp_varbind_len(struct snmp_pdu * pdu)
{
    register struct variable_list *vars;
    unsigned long retVal = 0;
    if (pdu)
      for (vars = pdu->variables; vars; vars = vars->next_variable)
      {    
        retVal++;
      }
    
    return retVal;
}

/*
 * Add object identifier name to SNMP variable.
 * If the name is large, additional memory is allocated.
 * Returns 0 if successful.
 */

int
snmp_set_var_objid (struct variable_list *vp,
                    const oid *objid, size_t name_length)
{
    size_t len = sizeof(oid) * name_length;

    /* use built-in storage for smaller values */
    if (len <= sizeof(vp->name_loc)) {
        vp->name = vp->name_loc;
    }
    else {
        vp->name = (oid *)malloc(len);
        if (!vp->name) return 1;
    }
    memmove(vp->name, objid, len);
    vp->name_length = name_length;
    return 0;
}

/*
 * Add some value to SNMP variable.
 * If the value is large, additional memory is allocated.
 * Returns 0 if successful.
 */

int
snmp_set_var_value(struct variable_list *newvar,
                    u_char *val_str, size_t val_len)
{
    if (newvar->val.string &&
        newvar->val.string != newvar->buf)
    {
        free(newvar->val.string);
    }

    newvar->val.string = 0; newvar->val_len = 0;

    /* need a pointer and a length to copy a string value. */
    if (val_str && val_len)
    {
        if (val_len <= sizeof(newvar->buf))
            newvar->val.string = newvar->buf;
        else {
            newvar->val.string = (u_char *)malloc(val_len);
            if (!newvar->val.string) return 1;
        }
        memmove(newvar->val.string, val_str, val_len);
        newvar->val_len = val_len;
    }

    return 0;
}


int
snmp_synch_response_cb(struct snmp_session *ss,
		    struct snmp_pdu *pdu,
		    struct snmp_pdu **response,
		    snmp_callback pcb)
{
    struct synch_state lstate, *state;
    snmp_callback cbsav;
    void * cbmagsav;
    int numfds, count;
    fd_set fdset;
    struct timeval timeout, *tvp;
    int block;

    memset((void *)&lstate, 0, sizeof(lstate));
    state = &lstate;
    cbsav = ss->callback;
    cbmagsav = ss->callback_magic;
    ss->callback = pcb;
    ss->callback_magic = (void *)state;

    if ((state->reqid = snmp_send(ss, pdu)) == 0){
	snmp_free_pdu(pdu);
	state->status = STAT_ERROR;
    }
    else
	state->waiting = 1;

    while(state->waiting){
	numfds = 0;
	FD_ZERO(&fdset);
	block = SNMPBLOCK;
	tvp = &timeout;
	timerclear(tvp);
	snmp_select_info(&numfds, &fdset, tvp, &block);
	if (block == 1)
	    tvp = NULL;	/* block without timeout */
	count = select(numfds, &fdset, 0, 0, tvp);
	if (count > 0){
	    snmp_read(&fdset);
	} else switch(count){
	    case 0:
		snmp_timeout();
		break;
	    case -1:
		if (errno == EINTR){
		    continue;
		} else {
		    snmp_errno = SNMPERR_GENERR;
		/* CAUTION! if another thread closed the socket(s)
		   waited on here, the session structure was freed.
		   It would be nice, but we can't rely on the pointer.
		    ss->s_snmp_errno = SNMPERR_GENERR;
		    ss->s_errno = errno;
		 */
		    snmp_set_detail(strerror(errno));
		}
	    /* FALLTHRU */
	    default:
		state->status = STAT_ERROR;
		state->waiting = 0;
	}
    }
    *response = state->pdu;
    ss->callback = cbsav;
    ss->callback_magic = cbmagsav;
    return state->status;
}

int
snmp_synch_response(struct snmp_session *ss,
		    struct snmp_pdu *pdu,
		    struct snmp_pdu **response)
{
    return snmp_synch_response_cb(ss,pdu,response,snmp_synch_input);
}

int
snmp_sess_synch_response(void *sessp,
			 struct snmp_pdu *pdu,
			 struct snmp_pdu **response)
{
    struct snmp_session *ss;
    struct synch_state lstate, *state;
    snmp_callback cbsav;
    void * cbmagsav;
    int numfds, count;
    fd_set fdset;
    struct timeval timeout, *tvp;
    int block;

    ss = snmp_sess_session(sessp);
    memset((void *)&lstate, 0, sizeof(lstate));
    state = &lstate;
    cbsav = ss->callback;
    cbmagsav = ss->callback_magic;
    ss->callback = snmp_synch_input;
    ss->callback_magic = (void *)state;

    if ((state->reqid = snmp_sess_send(sessp, pdu)) == 0){
	snmp_free_pdu(pdu);
	state->status = STAT_ERROR;
    }
    else
	state->waiting = 1;

    while(state->waiting){
	numfds = 0;
	FD_ZERO(&fdset);
	block = SNMPBLOCK;
	tvp = &timeout;
	timerclear(tvp);
	snmp_sess_select_info(sessp, &numfds, &fdset, tvp, &block);
	if (block == 1)
	    tvp = NULL;	/* block without timeout */
	count = select(numfds, &fdset, 0, 0, tvp);
	if (count > 0){
	    snmp_sess_read(sessp, &fdset);
	} else switch(count){
	    case 0:
		snmp_sess_timeout(sessp);
		break;
	    case -1:
		if (errno == EINTR){
		    continue;
		} else {
		    snmp_errno = SNMPERR_GENERR;
		/* CAUTION! if another thread closed the socket(s)
		   waited on here, the session structure was freed.
		   It would be nice, but we can't rely on the pointer.
		    ss->s_snmp_errno = SNMPERR_GENERR;
		    ss->s_errno = errno;
		 */
		    snmp_set_detail(strerror(errno));
		}
	    /* FALLTHRU */
	    default:
		state->status = STAT_ERROR;
		state->waiting = 0;
	}
    }
    *response = state->pdu;
    ss->callback = cbsav;
    ss->callback_magic = cbmagsav;
    return state->status;
}


const char *error_string[19] = {
    "(noError) No Error",
    "(tooBig) Response message would have been too large.",
    "(noSuchName) There is no such variable name in this MIB.",
    "(badValue) The value given has the wrong type or length.",
    "(readOnly) The two parties used do not have access to use the specified SNMP PDU.",
    "(genError) A general failure occured",
    "noAccess",
    "wrongType",
    "wrongLength",
    "wrongEncoding",
    "wrongValue",
    "noCreation",
    "inconsistentValue",
    "resourceUnavailable",
    "commitFailed",
    "undoFailed",
    "authorizationError",
    "notWritable",
    "inconsistentName"
};

const char *
snmp_errstring(int errstat)
{
    if (errstat <= MAX_SNMP_ERR && errstat >= SNMP_ERR_NOERROR){
	return error_string[errstat];
    } else {
	return "Unknown Error";
    }
}
