//==========================================================================
//
//      ./lib/current/src/snmp_alarm.c
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
/* snmp_alarm.c: generic library based alarm timers for various parts
   of an application */

#include <config.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <signal.h>
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
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

#if HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "snmp_debug.h"
#include "tools.h"
#include "default_store.h"
#include "callback.h"
#include "snmp_alarm.h"

static struct snmp_alarm *thealarms;
static int start_alarms = 0;
static unsigned int regnum = 1;

int
init_alarm_post_config(int majorid, int minorid, void *serverarg,
                     void *clientarg) {
  start_alarms = 1;
  set_an_alarm();
  return SNMPERR_SUCCESS;
}

void
init_snmp_alarm(void) {
  start_alarms = 0;
  snmp_register_callback(SNMP_CALLBACK_LIBRARY, SNMP_CALLBACK_POST_READ_CONFIG,
                         init_alarm_post_config, NULL);
}

void
sa_update_entry(struct snmp_alarm *alrm) {
  if (alrm->seconds == 0) {
    DEBUGMSGTL(("snmp_alarm_update_entry","illegal 0 length alarm timer specified\n"));
    return; /* illegal */
  }
  if (alrm->lastcall == 0) {
    /* never been called yet, call seconds from now. */
    alrm->lastcall = time(NULL);
    alrm->nextcall = alrm->lastcall + alrm->seconds;
  } else if (alrm->nextcall == 0) {
    /* We've been called but not reset for the next? call */
    if ((alrm->flags & SA_REPEAT) == SA_REPEAT) {
      alrm->nextcall = alrm->lastcall + alrm->seconds;
    } else {
      /* single time call, remove it */
      snmp_alarm_unregister(alrm->clientreg);
    }
  }
}

void
snmp_alarm_unregister(unsigned int clientreg) {
  struct snmp_alarm *sa_ptr, *alrm=NULL;

  if (thealarms == NULL)
    return;

  if (clientreg == thealarms->clientreg) {
    alrm = thealarms;
    thealarms = alrm->next;
  }
  else {
    for(sa_ptr = thealarms;
        sa_ptr != NULL && sa_ptr->next->clientreg != clientreg;
        sa_ptr = sa_ptr->next);
    if (sa_ptr) {
      if (sa_ptr->next) {
        alrm = sa_ptr->next;
        sa_ptr->next = sa_ptr->next->next;
      }
    }
  }

  /* Note:  do not free the clientarg, its the clients responsibility */
  if (alrm)
    free(alrm);
}
  

struct snmp_alarm *
sa_find_next(void) {
  struct snmp_alarm *sa_ptr, *sa_ret = NULL;
  for(sa_ptr = thealarms; sa_ptr != NULL; sa_ptr = sa_ptr->next) {
    if (sa_ret == NULL || sa_ptr->nextcall < sa_ret->nextcall)
      sa_ret = sa_ptr;
  }
  return sa_ret;
}

void
run_alarms(void) {
  int done=0;
  struct snmp_alarm *sa_ptr;

  /* loop through everything we have repeatedly looking for the next
     thing to call until all events are finally in the future again */
  DEBUGMSGTL(("snmp_alarm_run_alarms","looking for alarms to run...\n"));
  while(done == 0) {
    sa_ptr = sa_find_next();
    if (sa_ptr == NULL)
      return;
    if (sa_ptr->nextcall <= time(NULL)) {
      DEBUGMSGTL(("snmp_alarm_run_alarms","  running alarm %d\n",
                  sa_ptr->clientreg));
      (*(sa_ptr->thecallback))(sa_ptr->clientreg, sa_ptr->clientarg);
      DEBUGMSGTL(("snmp_alarm_run_alarms","     ... done\n"));
      sa_ptr->lastcall = time(NULL);
      sa_ptr->nextcall = 0;
      sa_update_entry(sa_ptr);
    } else {
      done = 1;
    }
  }
  DEBUGMSGTL(("snmp_alarm_run_alarms","Done.\n"));
}


RETSIGTYPE
alarm_handler(int a) {
  run_alarms();
  set_an_alarm();
}

int
get_next_alarm_delay_time(void) {
  struct snmp_alarm *sa_ptr;
  int nexttime = 0;

  sa_ptr = sa_find_next();
  if (sa_ptr) {
    nexttime = sa_ptr->nextcall - time(NULL);
    if (nexttime <= 0)
      nexttime = 1; /* occurred already, return 1 second */
  }
  return nexttime;
}


void
set_an_alarm(void) {
  int nexttime = get_next_alarm_delay_time();
  
  /* we don't use signals if they asked us nicely not to.  It's
     expected they'll check the next alarm time and do their own
     calling of run_alarms(). */
  if (!ds_get_boolean(DS_LIBRARY_ID, DS_LIB_ALARM_DONT_USE_SIG) && nexttime) {
#ifndef WIN32
#ifdef SIGALRM
//FIXMEHMTHMT    alarm(nexttime);
    DEBUGMSGTL(("snmp_alarm_set_an_alarm","setting an alarm for %d seconds from now\n",nexttime));
    signal(SIGALRM, alarm_handler);
#endif /* SIGALRM */
#endif

  } else {
    DEBUGMSGTL(("snmp_alarm_set_an_alarm","no alarms found to handle\n"));
  }
}

unsigned int
snmp_alarm_register(unsigned int when, unsigned int flags,
                    SNMPAlarmCallback *thecallback, void *clientarg) {
  struct snmp_alarm **sa_pptr;
  if (thealarms != NULL) {
    for(sa_pptr = &thealarms; (*sa_pptr) != NULL;
        sa_pptr = &((*sa_pptr)->next));
  } else {
    sa_pptr = &thealarms;
  }

  *sa_pptr = SNMP_MALLOC_STRUCT(snmp_alarm);
  if (*sa_pptr == NULL)
    return 0;

  (*sa_pptr)->seconds = when;
  (*sa_pptr)->flags = flags;
  (*sa_pptr)->clientarg = clientarg;
  (*sa_pptr)->thecallback = thecallback;
  (*sa_pptr)->clientreg = regnum++;
  sa_update_entry(*sa_pptr);

  DEBUGMSGTL(("snmp_alarm_register","registered alarm %d, secends=%d, flags=%d\n",
              (*sa_pptr)->clientreg, (*sa_pptr)->seconds, (*sa_pptr)->flags));

  if (start_alarms)
    set_an_alarm();
  return (*sa_pptr)->clientreg;
} 
