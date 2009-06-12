//==========================================================================
//
//      ./agent/current/src/mibgroup/mibII/sysORTable.c
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
 *  Template MIB group implementation - sysORTable.c
 *
 */
#include <config.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
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
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#if HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "mibincl.h"
#include "system.h"
#include "mibgroup/struct.h"
#include "mibgroup/util_funcs.h"
#include "mibgroup/mibII/sysORTable.h"
#include "snmpd.h"
#include "default_store.h"
#include "ds_agent.h"
#include "callback.h"
#include "agent_callbacks.h"

#ifdef USING_AGENTX_SUBAGENT_MODULE
#include "agentx/subagent.h"
#include "agentx/client.h"
#endif

extern struct timeval starttime;

struct timeval sysOR_lastchange;
static struct sysORTable *table=NULL;
static int numEntries=0;

/* define the structure we're going to ask the agent to register our
   information at */
struct variable2 sysORTable_variables[] = {
    { SYSORTABLEID,      ASN_OBJECT_ID,     RONLY, var_sysORTable, 1, {2}},
    { SYSORTABLEDESCR,   ASN_OCTET_STR,     RONLY, var_sysORTable, 1, {3}},
    { SYSORTABLEUPTIME,  ASN_TIMETICKS,     RONLY, var_sysORTable, 1, {4}}
};

/* Define the OID pointer to the top of the mib tree that we're
   registering underneath */
oid sysORTable_variables_oid[] = { SNMP_OID_MIB2,1,9,1 };
#ifdef USING_MIBII_SYSTEM_MIB_MODULE
extern oid system_module_oid[];
extern int system_module_oid_len;
extern int system_module_count;
#endif

void
init_sysORTable(void) {
  /* register ourselves with the agent to handle our mib tree */

#ifdef USING_AGENTX_SUBAGENT_MODULE
  if ( ds_get_boolean(DS_APPLICATION_ID, DS_AGENT_ROLE) == MASTER_AGENT )
	(void)register_mib_priority("mibII/sysORTable",
		(struct variable *) sysORTable_variables,
		sizeof(struct variable2),
		sizeof(sysORTable_variables)/sizeof(struct variable2),
		sysORTable_variables_oid,
		sizeof(sysORTable_variables_oid)/sizeof(oid), 1);
  else
#endif
    REGISTER_MIB("mibII/sysORTable", sysORTable_variables, variable2, sysORTable_variables_oid);

#ifdef USING_MIBII_SYSTEM_MIB_MODULE
  if ( ++system_module_count == 3 )
	REGISTER_SYSOR_TABLE( system_module_oid, system_module_oid_len,
		"The MIB module for SNMPv2 entities");
#endif

  gettimeofday(&sysOR_lastchange, NULL);
}

	/*********************
	 *
	 *  System specific implementation functions
	 *
	 *********************/

u_char *
var_sysORTable(struct variable *vp,
		oid *name,
		size_t *length,
		int exact,
		size_t *var_len,
		WriteMethod **write_method)
{
  struct timeval diff;
  int i;
  struct sysORTable *ptr;

  if (header_simple_table(vp, name, length, exact, var_len, write_method, numEntries))
    return NULL;

  DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- "));
  for(i = 1, ptr=table; ptr != NULL && i < (int)name[*length-1];
      ptr = ptr->next, i++) {
    DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- %d != %d\n",i,name[*length-1]));
  }
  if (ptr == NULL) {
    DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- no match: %d\n",i));
    return NULL;
  }
  DEBUGMSGTL(("mibII/sysORTable", "sysORTable -- match: %d\n",i));
  
  switch (vp->magic){
    case SYSORTABLEID:
      *var_len = ptr->OR_oidlen*sizeof(ptr->OR_oid[0]);
      return (u_char *) ptr->OR_oid;

    case SYSORTABLEDESCR:
      *var_len = strlen(ptr->OR_descr);
      return (u_char *) ptr->OR_descr;

    case SYSORTABLEUPTIME:
      ptr->OR_uptime.tv_sec--;
      ptr->OR_uptime.tv_usec += 1000000L;
      diff.tv_sec = ptr->OR_uptime.tv_sec - 1 - starttime.tv_sec;
      diff.tv_usec = ptr->OR_uptime.tv_usec + 1000000L - starttime.tv_usec;
      if (diff.tv_usec > 1000000L){
        diff.tv_usec -= 1000000L;
        diff.tv_sec++;
      }
      if ((diff.tv_sec * 100) + (diff.tv_usec / 10000) < 0)
        long_return = 0;
      else
        long_return = ((diff.tv_sec * 100) + (diff.tv_usec / 10000));
      return ((u_char *) &long_return);

    default:
      DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_sysORTable\n", vp->magic));
  }
  return NULL;
}


int register_sysORTable_sess(oid *oidin,
			 size_t oidlen,
			 const char *descr,
			 struct snmp_session *ss)
{
  struct sysORTable **ptr=&table;
  struct register_sysOR_parameters reg_sysOR_parms;

    DEBUGMSGTL(("mibII/sysORTable", "sysORTable registering: "));
    DEBUGMSGOID(("mibII/sysORTable", oidin, oidlen));
    DEBUGMSG(("mibII/sysORTable","\n"));

  while(*ptr != NULL)
    ptr = &((*ptr)->next);
  *ptr = (struct sysORTable *) malloc(sizeof(struct sysORTable));
  if ( *ptr == NULL ) {
	return SYS_ORTABLE_REGISTRATION_FAILED;
  }
  (*ptr)->OR_descr = (char *) malloc(strlen(descr)+1);
  if ( (*ptr)->OR_descr == NULL ) {
	free( *ptr );
	return SYS_ORTABLE_REGISTRATION_FAILED;
  }
  strcpy((*ptr)->OR_descr, descr);
  (*ptr)->OR_oidlen = oidlen;
  (*ptr)->OR_oid = (oid *) malloc(sizeof(oid)*oidlen);
  if ( (*ptr)->OR_oid == NULL ) {
	free( *ptr );
	free( (*ptr)->OR_descr );
	return SYS_ORTABLE_REGISTRATION_FAILED;
  }
  memcpy((*ptr)->OR_oid, oidin, sizeof(oid)*oidlen);
  gettimeofday(&((*ptr)->OR_uptime), NULL);
  (*ptr)->OR_sess = ss;
  (*ptr)->next = NULL;
  numEntries++;

  reg_sysOR_parms.name    = oidin;
  reg_sysOR_parms.namelen = oidlen;
  reg_sysOR_parms.descr   = descr;
  snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_REG_SYSOR,
                                       &reg_sysOR_parms);

  return SYS_ORTABLE_REGISTERED_OK;
}

int register_sysORTable(oid *oidin,
			 size_t oidlen,
			 const char *descr)
{
    return register_sysORTable_sess( oidin, oidlen, descr, NULL );
}



int unregister_sysORTable_sess(oid *oidin,
			 size_t oidlen,
			 struct snmp_session *ss)
{
  struct sysORTable **ptr=&table, *prev=NULL;
  int found = SYS_ORTABLE_NO_SUCH_REGISTRATION;
  struct register_sysOR_parameters reg_sysOR_parms;

    DEBUGMSGTL(("mibII/sysORTable", "sysORTable unregistering: "));
    DEBUGMSGOID(("mibII/sysORTable", oidin, oidlen));
    DEBUGMSG(("mibII/sysORTable","\n"));

  while(*ptr != NULL) {
    if ( snmp_oid_compare( oidin, oidlen, (*ptr)->OR_oid, (*ptr)->OR_oidlen) == 0 ) {
      if ( (*ptr)->OR_sess != ss )
	continue;	/* different session */
      if ( prev == NULL )
        table      = (*ptr)->next;
      else 
        prev->next = (*ptr)->next;

      free( (*ptr)->OR_descr );
      free( (*ptr)->OR_oid );
      free( (*ptr) );
      numEntries--;
      found = SYS_ORTABLE_UNREGISTERED_OK;
      break;
    }
    prev = *ptr;
    ptr = &((*ptr)->next);
  }

  reg_sysOR_parms.name    = oidin;
  reg_sysOR_parms.namelen = oidlen;
  snmp_call_callbacks(SNMP_CALLBACK_APPLICATION, SNMPD_CALLBACK_UNREG_SYSOR,
                                       &reg_sysOR_parms);

  return found;
}


int unregister_sysORTable(oid *oidin,
                       size_t oidlen)
{
    return unregister_sysORTable_sess( oidin, oidlen, NULL );
}

void unregister_sysORTable_by_session(struct snmp_session *ss)
{
  struct sysORTable *ptr=table, *prev=NULL, *next;

  while ( ptr != NULL  ) {
    next = ptr->next;
    if (( (ss->flags & SNMP_FLAGS_SUBSESSION) && ptr->OR_sess == ss ) ||
        (!(ss->flags & SNMP_FLAGS_SUBSESSION) &&
                              ptr->OR_sess->subsession == ss )) {
      if ( prev == NULL )
          table = next;
      else
          prev->next = next;
        free( ptr->OR_descr );
        free( ptr->OR_oid );
        free( ptr );
        numEntries--;

    } 
    else
      prev = ptr;
    ptr = next;
  }
}

