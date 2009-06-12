//==========================================================================
//
//      ./agent/current/src/snmp_agent.c
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

#include <config.h>

#include <sys/types.h>
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

#include "asn1.h"
#define SNMP_NEED_REQUEST_LIST
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp.h"
#include "mib.h"
#include "snmp_client.h"

#include "snmp_vars.h"
#include "snmpd.h"
#include "mibgroup/struct.h"
#include "mibgroup/util_funcs.h"
#include "var_struct.h"
#include "read_config.h"
#include "snmp_logging.h"
#include "snmp_debug.h"
#include "mib_module_config.h"

#include "default_store.h"
#include "ds_agent.h"
#include "snmp_agent.h"
#include "agent_trap.h"

static int snmp_vars_inc;

static struct agent_snmp_session *agent_session_list = NULL;


static void dump_var(oid *, size_t, int, void *, size_t);
static int goodValue(u_char, size_t, u_char, size_t);
static void setVariable(u_char *, u_char, size_t, u_char *, size_t);

static void dump_var (
    oid *var_name,
    size_t var_name_len,
    int statType,
    void *statP,
    size_t statLen)
{
    char buf [SPRINT_MAX_LEN];
    struct variable_list temp_var;

    temp_var.type = statType;
    temp_var.val.string = (u_char *)statP;
    temp_var.val_len = statLen;
    sprint_variable (buf, var_name, var_name_len, &temp_var);
    snmp_log(LOG_INFO, "    >> %s\n", buf);
}


int getNextSessID()
{
    static int SessionID = 0;

    return ++SessionID;
}

int
agent_check_and_process(int block) {
  int numfds;
  fd_set fdset;
  struct timeval	timeout, *tvp = &timeout;
  int count;
  int fakeblock=0;
  
  tvp =  &timeout;
  tvp->tv_sec  = 0;
  tvp->tv_usec = 0;

  numfds = 0;
  FD_ZERO(&fdset);
  snmp_select_info(&numfds, &fdset, tvp, &fakeblock);
  if (block == 1 && fakeblock == 1)
    tvp = NULL; /* block without timeout */
  else if (block == 0) {
      tvp->tv_sec = 0;
      tvp->tv_usec = 0;
  }

  count = select(numfds, &fdset, 0, 0, tvp);

  if (count > 0){
    /* packets found, process them */
    snmp_read(&fdset);
  } else switch(count){
    case 0:
      snmp_timeout();
      break;
    case -1:
      if (errno == EINTR){
        return -1;
      } else {
        snmp_log_perror("select");
      }
      return -1;
    default:
      snmp_log(LOG_ERR, "select returned %d\n", count);
      return -1;
  }  /* endif -- count>0 */
  return count;
}


/*
 * The session is created using the "traditional API" routine snmp_open()
 * so is linked into the global library Sessions list.  It also opens a
 * socket that listens for incoming requests.
 * 
 *   The agent runs in an infinite loop (in the 'receive()' routine),
 * which calls snmp_read() when such a request is received on this socket.
 * This routine then traverses the library 'Sessions' list to identify the
 * relevant session and eventually invokes '_sess_read'.
 *   This then processes the incoming packet, calling the pre_parse, parse,
 * post_parse and callback routines in turn.
 */

	/* Global access to the primary session structure for this agent.
		for Index Allocation use initially. */
struct snmp_session *main_session;

int
init_master_agent(int dest_port, 
                  int (*pre_parse) (struct snmp_session *, snmp_ipaddr),
                  int (*post_parse) (struct snmp_session *, struct snmp_pdu *,int))
{
    struct snmp_session sess, *session;

    if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE) != MASTER_AGENT )
	return 0; /* no error if ! MASTER_AGENT */

    DEBUGMSGTL(("snmpd","installing master agent on port %d\n", dest_port));

    snmp_sess_init( &sess );
    
    sess.version = SNMP_DEFAULT_VERSION;
    sess.peername = SNMP_DEFAULT_PEERNAME;
    sess.community_len = SNMP_DEFAULT_COMMUNITY_LEN;
     
    sess.local_port = dest_port;
    sess.callback = handle_snmp_packet;
    sess.authenticator = NULL;
    sess.flags = ds_get_int(DS_APPLICATION_ID, DS_AGENT_FLAGS);
    session = snmp_open_ex( &sess, pre_parse, 0, post_parse, 0, 0 );

    if ( session == NULL ) {
      /* diagnose snmp_open errors with the input struct snmp_session pointer */
	snmp_sess_perror("init_master_agent", &sess);
		return 1;
    }
    main_session = session;
	return 0;
}

struct agent_snmp_session  *
init_agent_snmp_session( struct snmp_session *session, struct snmp_pdu *pdu )
{
    struct agent_snmp_session  *asp;

    asp = malloc( sizeof( struct agent_snmp_session ));
    if ( asp == NULL )
	return NULL;
    asp->start = pdu->variables;
    asp->end   = pdu->variables;
    if ( asp->end != NULL )
	while ( asp->end->next_variable != NULL )
	    asp->end = asp->end->next_variable;
    asp->session = session;
    asp->pdu     = pdu;
    asp->rw      = READ;
    asp->exact   = TRUE;
    asp->outstanding_requests = NULL;
    asp->next    = NULL;
    asp->mode    = RESERVE1;
    asp->status  = SNMP_ERR_NOERROR;

    return asp;
}

int
count_varbinds( struct snmp_pdu *pdu )
{
  int count = 0;
  struct variable_list *var_ptr;
  
  for ( var_ptr = pdu->variables ; var_ptr != NULL ;
		  	var_ptr = var_ptr->next_variable )
	count++;

  return count;
}

int
handle_snmp_packet(int operation, struct snmp_session *session, int reqid,
                   struct snmp_pdu *pdu, void *magic)
{
    struct agent_snmp_session  *asp;
    int status, allDone, i;
    struct variable_list *var_ptr, *var_ptr2;

    if ( magic == NULL ) {
	asp = init_agent_snmp_session( session, snmp_clone_pdu(pdu) );
	status = SNMP_ERR_NOERROR;
    }
    else {
	asp = (struct agent_snmp_session *)magic;
        status =   asp->status;
    }

    if (asp->outstanding_requests != NULL)
	return 1;

    if ( check_access(pdu) != 0) {
        /* access control setup is incorrect */
	send_easy_trap(SNMP_TRAP_AUTHFAIL, 0);
        if (asp->pdu->version != SNMP_VERSION_1 && asp->pdu->version != SNMP_VERSION_2c) {
            asp->pdu->errstat = SNMP_ERR_AUTHORIZATIONERROR;
            asp->pdu->command = SNMP_MSG_RESPONSE;
            snmp_increment_statistic(STAT_SNMPOUTPKTS);
            snmp_send( asp->session, asp->pdu );
            free( asp );
            return 1;
        } else {
            /* drop the request */
            free( asp );
            return 0;
        }
    }

    switch (pdu->command) {
    case SNMP_MSG_GET:
	if ( asp->mode != RESERVE1 )
	    break;			/* Single pass */
        snmp_increment_statistic(STAT_SNMPINGETREQUESTS);
	status = handle_next_pass( asp );
	asp->mode = RESERVE2;
	break;

    case SNMP_MSG_GETNEXT:
	if ( asp->mode != RESERVE1 )
	    break;			/* Single pass */
        snmp_increment_statistic(STAT_SNMPINGETNEXTS);
	asp->exact   = FALSE;
	status = handle_next_pass( asp );
	asp->mode = RESERVE2;
	break;

    case SNMP_MSG_GETBULK:
	    /*
	     * GETBULKS require multiple passes. The first pass handles the
	     * explicitly requested varbinds, and subsequent passes append
	     * to the existing var_op_list.  Each pass (after the first)
	     * uses the results of the preceeding pass as the input list
	     * (delimited by the start & end pointers.
	     * Processing is terminated if all entries in a pass are
	     * EndOfMib, or the maximum number of repetitions are made.
	     */
	if ( asp->mode == RESERVE1 ) {
            snmp_increment_statistic(STAT_SNMPINGETREQUESTS);
	    asp->exact   = FALSE;
		    /*
		     * Limit max repetitions to something reasonable
		     *	XXX: We should figure out what will fit somehow...
		     */
	    if ( asp->pdu->errindex > 100 )
	        asp->pdu->errindex = 100;
    
	    status = handle_next_pass( asp );	/* First pass */
	    asp->mode = RESERVE2;
	    if ( status != SNMP_ERR_NOERROR )
	        break;
    
	    while ( asp->pdu->errstat-- > 0 )	/* Skip non-repeaters */
	        asp->start = asp->start->next_variable;
	    asp->pdu->errindex--;           /* Handled first repetition */

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	while ( asp->pdu->errindex-- > 0 ) {	/* Process repeaters */
		/*
		 * Add new variable structures for the
		 * repeating elements, ready for the next pass.
		 * Also check that these are not all EndOfMib
		 */
	    allDone = TRUE;		/* Check for some content */
	    for ( var_ptr = asp->start;
		  var_ptr != asp->end->next_variable;
		  var_ptr = var_ptr->next_variable ) {
				/* XXX: we don't know the size of the next
					OID, so assume the maximum length */
		if ( var_ptr->type != SNMP_ENDOFMIBVIEW )
		{
		var_ptr2 = snmp_add_null_var(asp->pdu, var_ptr->name, MAX_OID_LEN);
		for ( i=var_ptr->name_length ; i<MAX_OID_LEN ; i++)
		    var_ptr2->name[i] = 0;
		var_ptr2->name_length = var_ptr->name_length;

		    allDone = FALSE;
		}
	    }
	    if ( allDone )
		break;

	    asp->start = asp->end->next_variable;
	    while ( asp->end->next_variable != NULL )
		asp->end = asp->end->next_variable;
	    
	    status = handle_next_pass( asp );
	    if ( status != SNMP_ERR_NOERROR )
		break;
	    if ( asp->outstanding_requests != NULL )
		return 1;
	}
	break;

    case SNMP_MSG_SET:
    	    /*
	     * SETS require 3-4 passes through the var_op_list.  The first two
	     * passes verify that all types, lengths, and values are valid
	     * and may reserve resources and the third does the set and a
	     * fourth executes any actions.  Then the identical GET RESPONSE
	     * packet is returned.
	     * If either of the first two passes returns an error, another
	     * pass is made so that any reserved resources can be freed.
	     * If the third pass returns an error, another pass is made so that
	     * any changes can be reversed.
	     * If the fourth pass (or any of the error handling passes)
	     * return an error, we'd rather not know about it!
	     */
	if ( asp->mode == RESERVE1 ) {
            snmp_increment_statistic(STAT_SNMPINSETREQUESTS);
	    asp->rw      = WRITE;

	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR )
	        asp->mode = FREE;
	    else
	        asp->mode = RESERVE2;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == RESERVE2 ) {
	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR )
	        asp->mode = FREE;
	    else
	        asp->mode = ACTION;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == ACTION ) {
	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR )
	        asp->mode = UNDO;
	    else
	        asp->mode = COMMIT;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == COMMIT ) {
	    status = handle_next_pass( asp );

	    if ( status != SNMP_ERR_NOERROR ) {
		status    = SNMP_ERR_COMMITFAILED;
	        asp->mode = FINISHED_FAILURE;
	    }
	    else
	        asp->mode = FINISHED_SUCCESS;

	    if ( asp->outstanding_requests != NULL )
		return 1;
	}

	if ( asp->mode == UNDO ) {
	    if (handle_next_pass( asp ) != SNMP_ERR_NOERROR )
		status = SNMP_ERR_UNDOFAILED;

	    asp->mode = FINISHED_FAILURE;
	    break;
	}

	if ( asp->mode == FREE ) {
	    (void) handle_next_pass( asp );
	    break;
	}

	break;

    case SNMP_MSG_RESPONSE:
        snmp_increment_statistic(STAT_SNMPINGETRESPONSES);
	free( asp );
	return 0;
    case SNMP_MSG_TRAP:
    case SNMP_MSG_TRAP2:
        snmp_increment_statistic(STAT_SNMPINTRAPS);
	free( asp );
	return 0;
    default:
        snmp_increment_statistic(STAT_SNMPINASNPARSEERRS);
	free( asp );
	return 0;
    }

    if ( asp->outstanding_requests != NULL ) {
	asp->status = status;
	asp->next = agent_session_list;
	agent_session_list = asp;
    }
    else {
		/*
		 * May need to "dumb down" a SET error status for a
		 *  v1 query.  See RFC2576 - section 4.3
		 */
	if (( asp->pdu->command == SNMP_MSG_SET ) &&
	    ( asp->pdu->version == SNMP_VERSION_1 )) {
	    switch ( status ) {
		case SNMP_ERR_WRONGVALUE:
		case SNMP_ERR_WRONGENCODING:
		case SNMP_ERR_WRONGTYPE:
		case SNMP_ERR_WRONGLENGTH:
		case SNMP_ERR_INCONSISTENTVALUE:
			status = SNMP_ERR_BADVALUE;
			break;
		case SNMP_ERR_NOACCESS:
		case SNMP_ERR_NOTWRITABLE:
		case SNMP_ERR_NOCREATION:
		case SNMP_ERR_INCONSISTENTNAME:
		case SNMP_ERR_AUTHORIZATIONERROR:
			status = SNMP_ERR_NOSUCHNAME;
			break;
		case SNMP_ERR_RESOURCEUNAVAILABLE:
		case SNMP_ERR_COMMITFAILED:
		case SNMP_ERR_UNDOFAILED:
			status = SNMP_ERR_GENERR;
			break;
	    }
	}
		/*
		 * Similarly we may need to "dumb down" v2 exception
		 *  types to throw an error for a v1 query.
		 *  See RFC2576 - section 4.1.2.3
		 */
	if (( asp->pdu->command != SNMP_MSG_SET ) &&
	    ( asp->pdu->version == SNMP_VERSION_1 )) {
		for ( var_ptr = asp->pdu->variables, i=0 ;
			var_ptr != NULL ;
			var_ptr = var_ptr->next_variable, i++ ) {
		    switch ( var_ptr->type ) {
			case SNMP_NOSUCHOBJECT:
			case SNMP_NOSUCHINSTANCE:
			case SNMP_ENDOFMIBVIEW:
			case ASN_COUNTER64:
				status = SNMP_ERR_NOSUCHNAME;
				asp->pdu->errindex=i;
				break;
		    }
		}
	}
	if ( status == SNMP_ERR_NOERROR ) {
	    snmp_increment_statistic_by(
		(asp->pdu->command == SNMP_MSG_SET ?
			STAT_SNMPINTOTALSETVARS : STAT_SNMPINTOTALREQVARS ),
	    	count_varbinds( asp->pdu ));
	}
	else {
		/*
		 * Use a copy of the original request
		 *   to report failures.
		 */
	    i = asp->pdu->errindex;
	    snmp_free_pdu( asp->pdu );
	    asp->pdu = snmp_clone_pdu( pdu );
	    asp->pdu->errindex = i;
	}
	asp->pdu->command = SNMP_MSG_RESPONSE;
	asp->pdu->errstat = status;
	snmp_send( asp->session, asp->pdu );
	snmp_increment_statistic(STAT_SNMPOUTPKTS);
	snmp_increment_statistic(STAT_SNMPOUTGETRESPONSES);
	free( asp );
    }

    return 1;
}


int
handle_next_pass(struct agent_snmp_session  *asp)
{
    int status;
    struct snmp_pdu *pdu = asp->pdu;
    struct request_list *req_p, *next_req;


        if ( asp->outstanding_requests != NULL )
	    return SNMP_ERR_NOERROR;
	status = handle_var_list( asp );
        if ( asp->outstanding_requests != NULL ) {
	    if ( status == SNMP_ERR_NOERROR ) {
		/* Send out any subagent requests */
		for ( req_p = asp->outstanding_requests ;
			req_p != NULL ; req_p = req_p->next_request ) {

		    snmp_async_send( req_p->session,  req_p->pdu,
				      req_p->callback, req_p->cb_data );
		}
		asp->pdu = snmp_clone_pdu( pdu );
		asp->pdu->variables = pdu->variables;
		pdu->variables = NULL;
	    }
	    else {
	    	/* discard outstanding requests */
		for ( req_p = asp->outstanding_requests ;
			req_p != NULL ; req_p = next_req ) {
			
			next_req = req_p->next_request;
			free( req_p );
		}
		asp->outstanding_requests = NULL;
	    }
	}
	return status;
}


int
handle_var_list(struct agent_snmp_session  *asp)
{
    struct variable_list *varbind_ptr;
    u_char  statType;
    u_char *statP;
    size_t  statLen;
    u_short acl;
    WriteMethod *write_method;
    AddVarMethod *add_method;
    int	    noSuchObject = TRUE;
    int     count, view;
    
    count = 0;
    varbind_ptr = asp->start;
    if ( !varbind_ptr ) {
	return SNMP_ERR_NOERROR;
    }

    while (1) {
    
	count++;
statp_loop:
	statP = getStatPtr(  varbind_ptr->name,
			   &varbind_ptr->name_length,
			   &statType, &statLen, &acl,
			   asp->exact, &write_method, asp->pdu, &noSuchObject);
			   
	if (statP == NULL && (asp->rw != WRITE || write_method == NULL)) {
	        /*  Careful -- if the varbind was lengthy, it will have
		    allocated some memory.  */
	        snmp_set_var_value(varbind_ptr, NULL, 0);
	    	varbind_ptr->val.integer   = NULL;
	    	varbind_ptr->val_len = 0;
		if ( asp->exact ) {
	            if ( noSuchObject == TRUE ){
		        statType = SNMP_NOSUCHOBJECT;
		    } else {
		        statType = SNMP_NOSUCHINSTANCE;
		    }
		} else {
	            statType = SNMP_ENDOFMIBVIEW;
		}
		if (asp->pdu->version == SNMP_VERSION_1) {
		    asp->pdu->errstat = SNMP_ERR_NOSUCHNAME;
		    asp->pdu->errindex = count;
		    return SNMP_ERR_NOSUCHNAME;
		}
		else if (asp->rw == WRITE) {
		    asp->pdu->errstat =
			( noSuchObject	? SNMP_ERR_NOTWRITABLE
					: SNMP_ERR_NOCREATION );
		    asp->pdu->errindex = count;
		    return asp->pdu->errstat;
		}
		else
		    varbind_ptr->type = statType;
	}
                /* Delegated variables should be added to the
                   relevant outgoing request */
        else if ( IS_DELEGATED(statType)) {
                add_method = (AddVarMethod*)statP;
                statType = (*add_method)( asp, varbind_ptr );
        }
		/* GETNEXT/GETBULK should just skip inaccessible entries */
	else if ((view = in_a_view(varbind_ptr->name, &varbind_ptr->name_length,
				   asp->pdu, varbind_ptr->type))
			 && !asp->exact) {
		if (view != 5) send_easy_trap(SNMP_TRAP_AUTHFAIL, 0);
		goto statp_loop;
	}
		/* Other access problems are permanent */
	else if (( asp->rw == WRITE && !(acl & 2)) || view) {
	    if (asp->pdu->version == SNMP_VERSION_1 || asp->rw != WRITE) {
		if (ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE))
                  DEBUGMSGTL(("snmp_agent", "    >> noSuchName (read-only)\n"));
		ERROR_MSG("read-only");
		statType = SNMP_ERR_NOSUCHNAME;
	    }
	    else {
		if (ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE))
                  DEBUGMSGTL(("snmp_agent", "    >> notWritable\n"));
		ERROR_MSG("Not Writable");
		statType = SNMP_ERR_NOTWRITABLE;
	    }
	    asp->pdu->errstat = statType;
	    asp->pdu->errindex = count;
	    send_easy_trap(SNMP_TRAP_AUTHFAIL, 0);
	    return statType;
        }
	else {
            /* dump verbose info */
	    if (ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_VERBOSE) && statP)
	        dump_var(varbind_ptr->name, varbind_ptr->name_length,
				statType, statP, statLen);

		/*  FINALLY we can act on SET requests ....*/
	    if ( asp->rw == WRITE ) {
	        if ( write_method != NULL ) {
		    statType = (*write_method)(asp->mode,
                                               varbind_ptr->val.string,
                                               varbind_ptr->type,
                                               varbind_ptr->val_len, statP,
                                               varbind_ptr->name,
                                               varbind_ptr->name_length);
                    if (statType != SNMP_ERR_NOERROR) {
                      asp->pdu->errstat = statType;
                      asp->pdu->errindex = count;
                      return statType;
                    }
		}
		else {
                    if (!goodValue(varbind_ptr->type, varbind_ptr->val_len,
                                    statType, statLen)){
                        if (asp->pdu->version == SNMP_VERSION_1)
                            statType = SNMP_ERR_BADVALUE;
                        else
                            statType = SNMP_ERR_WRONGTYPE; /* poor approximation */
			asp->pdu->errstat = statType;
			asp->pdu->errindex = count;
			return statType;
                    }
                    /* actually do the set if necessary */
                    if (asp->mode == COMMIT)
                        setVariable(varbind_ptr->val.string, varbind_ptr->type,
                                    varbind_ptr->val_len, statP, statLen);
                }
	    }
		/* ... or save the results from assorted GETs */
	    else {
		     snmp_set_var_value(varbind_ptr, statP, statLen);
		     varbind_ptr->type = statType;
	    }
	}
	
	if ( varbind_ptr == asp->end )
	     return SNMP_ERR_NOERROR;
	varbind_ptr = varbind_ptr->next_variable;
	if ( asp->mode == RESERVE1 )
	    snmp_vars_inc++;
    }
}



static int
goodValue(u_char inType, 
	  size_t inLen,
	  u_char actualType,
	  size_t actualLen)
{
    if (inLen > actualLen)
	return FALSE;
    return (inType == actualType);
}

static void
setVariable(u_char *var_val,
	    u_char var_val_type,
	    size_t var_val_len,
	    u_char *statP,
	    size_t statLen)
{
    size_t buffersize = 1000;

    switch(var_val_type){
	case ASN_INTEGER:
	    asn_parse_int(var_val, &buffersize, &var_val_type, (long *)statP, statLen);
	    break;
	case ASN_COUNTER:
	case ASN_GAUGE:
	case ASN_TIMETICKS:
	    asn_parse_unsigned_int(var_val, &buffersize, &var_val_type, (u_long *)statP, statLen);
	    break;
	case ASN_COUNTER64:
	    asn_parse_unsigned_int64(var_val, &buffersize, &var_val_type,
				     (struct counter64 *)statP, statLen);
	    break;
	case ASN_OCTET_STR:
	case ASN_IPADDRESS:
	case ASN_OPAQUE:
	case ASN_NSAP:
	    asn_parse_string(var_val, &buffersize, &var_val_type, statP, &statLen);
	    break;
	case ASN_OBJECT_ID:
	    asn_parse_objid(var_val, &buffersize, &var_val_type, (oid *)statP, &statLen);
	    break;
	case ASN_BIT_STR:
	    asn_parse_bitstring(var_val, &buffersize, &var_val_type, statP, &statLen);
	    break;
    }
}
