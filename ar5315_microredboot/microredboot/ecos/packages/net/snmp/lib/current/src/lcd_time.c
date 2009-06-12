//==========================================================================
//
//      ./lib/current/src/lcd_time.c
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
 * lcd_time.c
 *
 * XXX	Should etimelist entries with <0,0> time tuples be timed out?
 * XXX	Need a routine to free the memory?  (Perhaps at shutdown?)
 */

#include <config.h>

#ifdef CYGPKG_SNMPAGENT_V3_SUPPORT
#include <sys/types.h>
#if HAVE_WINSOCK_H
#include <winsock.h>
#endif
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
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
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "callback.h"
#include "snmpusm.h"
#include "lcd_time.h"
#include "snmp_debug.h"
#include "tools.h"
#include "scapi.h"

#include "transform_oids.h"


/*
 * Global static hashlist to contain Enginetime entries.
 *
 * New records are prepended to the appropriate list at the hash index.
 */
static Enginetime etimelist[ETIMELIST_SIZE];




/*******************************************************************-o-******
 * get_enginetime
 *
 * Parameters:
 *	*engineID
 *	 engineID_len
 *	*engineboot
 *	*engine_time
 *      
 * Returns:
 *	SNMPERR_SUCCESS		Success -- when a record for engineID is found.
 *	SNMPERR_GENERR		Otherwise.
 *
 *
 * Lookup engineID and return the recorded values for the
 * <engine_time, engineboot> tuple adjusted to reflect the estimated time
 * at the engine in question.
 *
 * Special case: if engineID is NULL or if engineID_len is 0 then
 * the time tuple is returned immediately as zero.
 *
 * XXX	What if timediff wraps?  >shrug<
 * XXX  Then: you need to increment the boots value.  Now.  Detecting
 *            this is another matter.
 */
int
get_enginetime(	u_char	*engineID,	
		u_int	 engineID_len,
		u_int	*engineboot,
		u_int	*engine_time,
		u_int   authenticated)
{
	int		rval	 = SNMPERR_SUCCESS;
	time_t		timediff = 0;
	Enginetime	e	 = NULL;



	/*
	 * Sanity check.
	 */
	if ( !engine_time || !engineboot ) {
		QUITFUN(SNMPERR_GENERR, get_enginetime_quit);
	}


	/*
	 * Compute estimated current engine_time tuple at engineID if
	 * a record is cached for it.
	 */
	*engine_time = *engineboot = 0;

	if ( !engineID || (engineID_len<=0) ) {
		QUITFUN(SNMPERR_GENERR, get_enginetime_quit);
	}

	if ( !(e = search_enginetime_list(engineID, engineID_len)) ) {
		QUITFUN(SNMPERR_GENERR, get_enginetime_quit);
	}

#ifdef LCD_TIME_SYNC_OPT
        if (!authenticated || e->authenticatedFlag) {
#endif	
	*engine_time = e->engineTime;
	*engineboot = e->engineBoot;

	timediff = time(NULL) - e->lastReceivedEngineTime;
#ifdef LCD_TIME_SYNC_OPT	
        }
#endif	

	if ( timediff > (int)(ENGINETIME_MAX - *engine_time) ) {
		*engine_time = (timediff - (ENGINETIME_MAX - *engine_time));

		/* FIX -- move this check up... should not change anything
		 * if engineboot is already locked.  ???
		 */
		if (*engineboot < ENGINEBOOT_MAX) {
			*engineboot += 1;
		}

	} else {
		*engine_time += timediff;
	}

        DEBUGMSGTL(("lcd_get_enginetime", "engineID "));
        DEBUGMSGHEX(("lcd_get_enginetime", engineID, engineID_len));
        DEBUGMSG(("lcd_get_enginetime", ": boots=%d, time=%d\n", *engineboot,
                  *engine_time));

get_enginetime_quit:
	return rval;

}  /* end get_enginetime() */

/*******************************************************************-o-******
 * get_enginetime
 *
 * Parameters:
 *	*engineID
 *	 engineID_len
 *	*engineboot
 *	*engine_time
 *      
 * Returns:
 *	SNMPERR_SUCCESS		Success -- when a record for engineID is found.
 *	SNMPERR_GENERR		Otherwise.
 *
 *
 * Lookup engineID and return the recorded values for the
 * <engine_time, engineboot> tuple adjusted to reflect the estimated time
 * at the engine in question.
 *
 * Special case: if engineID is NULL or if engineID_len is 0 then
 * the time tuple is returned immediately as zero.
 *
 * XXX	What if timediff wraps?  >shrug<
 * XXX  Then: you need to increment the boots value.  Now.  Detecting
 *            this is another matter.
 */
int
get_enginetime_ex(	u_char	*engineID,	
		u_int	 engineID_len,
		u_int	*engineboot,
		u_int	*engine_time,
		u_int	*last_engine_time,
		u_int   authenticated)
{
	int		rval	 = SNMPERR_SUCCESS;
	time_t		timediff = 0;
	Enginetime	e	 = NULL;



	/*
	 * Sanity check.
	 */
	if ( !engine_time || !engineboot || !last_engine_time) {
		QUITFUN(SNMPERR_GENERR, get_enginetime_ex_quit);
	}


	/*
	 * Compute estimated current engine_time tuple at engineID if
	 * a record is cached for it.
	 */
	*last_engine_time = *engine_time = *engineboot = 0;

	if ( !engineID || (engineID_len<=0) ) {
		QUITFUN(SNMPERR_GENERR, get_enginetime_ex_quit);
	}

	if ( !(e = search_enginetime_list(engineID, engineID_len)) ) {
		QUITFUN(SNMPERR_GENERR, get_enginetime_ex_quit);
	}

#ifdef LCD_TIME_SYNC_OPT
        if (!authenticated || e->authenticatedFlag) {
#endif	
	*last_engine_time = *engine_time = e->engineTime;
	*engineboot = e->engineBoot;

	timediff = time(NULL) - e->lastReceivedEngineTime;
#ifdef LCD_TIME_SYNC_OPT	
        }
#endif	

	if ( timediff > (int)(ENGINETIME_MAX - *engine_time) ) {
		*engine_time = (timediff - (ENGINETIME_MAX - *engine_time));

		/* FIX -- move this check up... should not change anything
		 * if engineboot is already locked.  ???
		 */
		if (*engineboot < ENGINEBOOT_MAX) {
			*engineboot += 1;
		}

	} else {
		*engine_time += timediff;
	}

        DEBUGMSGTL(("lcd_get_enginetime_ex", "engineID "));
        DEBUGMSGHEX(("lcd_get_enginetime_ex", engineID, engineID_len));
        DEBUGMSG(("lcd_get_enginetime_ex", ": boots=%d, time=%d\n", *engineboot,
                  *engine_time));

get_enginetime_ex_quit:
	return rval;

}  /* end get_enginetime_ex() */




/*******************************************************************-o-******
 * set_enginetime
 *
 * Parameters:
 *	*engineID
 *	 engineID_len
 *	 engineboot
 *	 engine_time
 *      
 * Returns:
 *	SNMPERR_SUCCESS		Success.
 *	SNMPERR_GENERR		Otherwise.
 *
 *
 * Lookup engineID and store the given <engine_time, engineboot> tuple
 * and then stamp the record with a consistent source of local time.
 * If the engineID record does not exist, create one.
 *
 * Special case: engineID is NULL or engineID_len is 0 defines an engineID
 * that is "always set."
 *
 * XXX	"Current time within the local engine" == time(NULL)...
 */
int
set_enginetime(	u_char	*engineID,
		u_int	 engineID_len,
		u_int	 engineboot,
		u_int  	 engine_time,
		u_int    authenticated)
{
	int		rval = SNMPERR_SUCCESS,
			iindex;
	Enginetime	e = NULL;



	/*
	 * Sanity check.
	 */
	if ( !engineID || (engineID_len <= 0) ) {
		return rval;
	}


	/*
	 * Store the given <engine_time, engineboot> tuple in the record
	 * for engineID.  Create a new record if necessary.
	 */
	if ( !(e = search_enginetime_list(engineID, engineID_len)) )
	{
		if ( (iindex = hash_engineID(engineID, engineID_len)) < 0 )
		{
			QUITFUN(SNMPERR_GENERR, set_enginetime_quit);
		}

		e = (Enginetime) calloc(1,sizeof(*e));

		e->next = etimelist[iindex];
		etimelist[iindex] = e;

		e->engineID = (u_char *) calloc(1,engineID_len);
		memcpy(e->engineID, engineID, engineID_len);

		e->engineID_len = engineID_len;
	}
#ifdef LCD_TIME_SYNC_OPT	
	if (authenticated || !e->authenticatedFlag) {
	  e->authenticatedFlag = authenticated;
#else
	if (authenticated) {
#endif
	  e->engineTime		  = engine_time;
	  e->engineBoot		  = engineboot;
	  e->lastReceivedEngineTime = time(NULL);
        }

	e = NULL;	/* Indicates a successful update. */

        DEBUGMSGTL(("lcd_set_enginetime", "engineID "));
        DEBUGMSGHEX(("lcd_set_enginetime", engineID, engineID_len));
        DEBUGMSG(("lcd_set_enginetime", ": boots=%d, time=%d\n", engineboot,
                  engine_time));

set_enginetime_quit:
	SNMP_FREE(e);

	return rval;

}  /* end set_enginetime() */




/*******************************************************************-o-******
 * search_enginetime_list
 *
 * Parameters:
 *	*engineID
 *	 engineID_len
 *      
 * Returns:
 *	Pointer to a etimelist record with engineID <engineID>  -OR-
 *	NULL if no record exists.
 *
 *
 * Search etimelist for an entry with engineID.
 *
 * ASSUMES that no engineID will have more than one record in the list.
 */
Enginetime
search_enginetime_list(u_char *engineID, u_int engineID_len)
{
	int		rval = SNMPERR_SUCCESS;
	Enginetime	e    = NULL;


	/*
	 * Sanity check.
	 */
	if ( !engineID || (engineID_len<=0) ) {
		QUITFUN(SNMPERR_GENERR, search_enginetime_list_quit);
	}


	/*
	 * Find the entry for engineID if there be one.
	 */
	rval = hash_engineID(engineID, engineID_len);
	if (rval < 0) {
		QUITFUN(SNMPERR_GENERR, search_enginetime_list_quit);
	}
	e = etimelist[rval];

	for ( /*EMPTY*/; e; e = e->next )
	{
		if ( (engineID_len == e->engineID_len)
			&& !memcmp(e->engineID, engineID, engineID_len) )
		{
			break;
		}
	}
	

search_enginetime_list_quit:
	return e;

}  /* end search_enginetime_list() */





/*******************************************************************-o-******
 * hash_engineID
 *
 * Parameters:
 *	*engineID
 *	 engineID_len
 *      
 * Returns:
 *	>0			etimelist index for this engineID.
 *	SNMPERR_GENERR		Error.
 *	
 * 
 * Use a cheap hash to build an index into the etimelist.  Method is 
 * to hash the engineID, then split the hash into u_int's and add them up
 * and modulo the size of the list.
 *
 */
int
hash_engineID(u_char *engineID, u_int engineID_len)
{
	int		 rval		= SNMPERR_GENERR;
	size_t		 buf_len	= SNMP_MAXBUF;
	u_int		 additive	= 0;
	u_char		*bufp,
			 buf[SNMP_MAXBUF];
	void		*context = NULL;



	/*
	 * Sanity check.
	 */
	if ( !engineID || (engineID_len <= 0) ) {
		QUITFUN(SNMPERR_GENERR, hash_engineID_quit);
	}


	/*
	 * Hash engineID into a list index.
	 */
        rval = sc_hash(usmHMACMD5AuthProtocol,
                       sizeof(usmHMACMD5AuthProtocol)/sizeof(oid),
                       engineID, engineID_len,
                       buf, &buf_len);
	QUITFUN(rval, hash_engineID_quit);
        
	for ( bufp = buf; (bufp-buf) < (int)buf_len; bufp += 4 ) {
		additive += (u_int) *bufp;
	}

hash_engineID_quit:
	SNMP_FREE(context);
	memset(buf, 0, SNMP_MAXBUF);

	return (rval < 0) ? rval : (additive % ETIMELIST_SIZE);

}  /* end hash_engineID() */




#ifdef SNMP_TESTING_CODE
/*******************************************************************-o-******
 * dump_etimelist_entry
 *
 * Parameters:
 *	e
 *	count
 */
void
dump_etimelist_entry(Enginetime e, int count)
{
	u_int	 buflen;
	char	 tabs[SNMP_MAXBUF],
		*t = tabs, 
		*s;



	count += 1;
	while (count--) {
		t += sprintf(t, "  ");
	}


	buflen = e->engineID_len;
#ifdef SNMP_TESTING_CODE
	if ( !(s = dump_snmpEngineID(e->engineID, &buflen)) ) {
#endif
		binary_to_hex(e->engineID, e->engineID_len, &s);
#ifdef SNMP_TESTING_CODE
	}
#endif

	DEBUGMSGTL(("dump_etimelist", "%s\n",tabs));
	DEBUGMSGTL(("dump_etimelist", "%s%s (len=%d) <%d,%d>\n", tabs,
                    s, e->engineID_len,
                    e->engineTime, e->engineBoot));
	DEBUGMSGTL(("dump_etimelist", "%s%ld (%ld) -- %s", tabs,
                    e->lastReceivedEngineTime,
                    time(NULL) - e->lastReceivedEngineTime,
                    ctime(&e->lastReceivedEngineTime)));

	SNMP_FREE(s);

}  /* end dump_etimelist_entry() */




/*******************************************************************-o-******
 * dump_etimelist
 */
void
dump_etimelist(void)
{
	int		iindex = -1,
			count = 0;
	Enginetime	e;



	DEBUGMSGTL(("dump_etimelist", "\n"));

	while (++iindex < ETIMELIST_SIZE) {
		DEBUGMSG(("dump_etimelist", "[%d]", iindex));

		count = 0;
		e = etimelist[iindex];

		while (e) {
			dump_etimelist_entry(e, count++);
			e = e->next;
		}

		if (count > 0) {
			DEBUGMSG(("dump_etimelist", "\n"));
		}
	}  /* endwhile */

	DEBUGMSG(("dump_etimelist", "\n"));

}  /* end dump_etimelist() */
#endif /* SNMP_TESTING_CODE */

#endif /* CYGPKG_SNMPAGENT_V3_SUPPORT */
