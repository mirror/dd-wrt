/*
 * Copyright (c) 2012, 2013
 *      Inferno Nettverk A/S, Norway.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. The above copyright notice, this list of conditions and the following
 *    disclaimer must appear in all copies of the software, derivative works
 *    or modified versions, and any portions thereof, aswell as in all
 *    supporting documentation.
 * 2. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by
 *      Inferno Nettverk A/S, Norway.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Inferno Nettverk A/S requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  sdc@inet.no
 *  Inferno Nettverk A/S
 *  Oslo Research Park
 *  Gaustadalléen 21
 *  NO-0349 Oslo
 *  Norway
 *
 * any improvements or extensions that they make and grant Inferno Nettverk A/S
 * the rights to redistribute these changes.
 *
 */

#define _GNU_SOURCE /* needed for SCHED_BATCH */

#include "common.h"

#include <sched.h>

static const char rcsid[] =
"$Id: cpupolicy.c,v 1.6 2013/10/27 15:24:42 karls Exp $";

#if HAVE_SCHED_SETSCHEDULER

cpupolicy_t cpupolicies[] = {
#ifdef SCHED_OTHER
   { SCHED_OTHER, "other" },
#endif /* SCHED_OTHER */
#ifdef SCHED_FIFO
   { SCHED_FIFO, "fifo" },
#endif /* SCHED_FIFO */
#ifdef SCHED_FIFO2
   { SCHED_FIFO2, "fifo2" },
#endif /* SCHED_FIFO2 */
#ifdef SCHED_FIFO3
   { SCHED_FIFO3, "fifo3" },
#endif /* SCHED_FIFO3 */
#ifdef SCHED_FIFO4
   { SCHED_FIFO4, "fifo4" },
#endif /* SCHED_FIFO */
#ifdef SCHED_RR
   { SCHED_RR, "rr" },
#endif /* SCHED_RR */
#ifdef SCHED_IDLE
   { SCHED_IDLE, "idle" },
#endif /* SCHED_IDLE */
#ifdef SCHED_BATCH
   { SCHED_BATCH, "batch" },
#endif /* SCHED_BATCH */
   { -1, NULL },
};

int
cpupolicy2numeric(char *name)
{
   int i;

   for (i = 0; cpupolicies[i].name != NULL; i++) {
      if (strcmp(name, cpupolicies[i].name) == 0)
         return cpupolicies[i].value;
   }

   return -1;
}

char *
numeric2cpupolicy(int value)
{
   int i;

   for (i = 0; cpupolicies[i].name != NULL; i++) {
      if (value == cpupolicies[i].value)
         return cpupolicies[i].name;
   }

   return "<unknown>";
}

#endif /* HAVE_SCHED_SETSCHEDULER */
