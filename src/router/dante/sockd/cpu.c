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

#include "common.h"

static const char rcsid[] =
"$Id: cpu.c,v 1.25 2013/10/27 15:24:42 karls Exp $";

int
sockd_setcpusettings(old, new)
   const cpusetting_t *old;
   const cpusetting_t *new;
{
#if HAVE_SCHED_SETSCHEDULER || HAVE_SCHED_SETAFFINITY
   const char *function = "sockd_setcpusettings()";
   int rc;

#if HAVE_SCHED_SETSCHEDULER
   SASSERTX(old->scheduling_isset);
   SASSERTX(new->scheduling_isset);

   slog(LOG_DEBUG, "%s: old cpu scheduling policy/priority: %s/%d, new: %s/%d",
        function,
        numeric2cpupolicy(old->policy),
        old->param.sched_priority,
        numeric2cpupolicy(new->policy),
        new->param.sched_priority);

   if (old->policy                != new->policy
   ||  old->param.sched_priority  != new->param.sched_priority) {
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
      rc = sched_setscheduler(0, new->policy, &new->param);
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);

      if (rc != 0) {
         swarn("%s: could not change cpu scheduling policy/priority from "
               "%s/%d to %s/%d%s",
               function,
               numeric2cpupolicy(old->policy),
               old->param.sched_priority,
               numeric2cpupolicy(new->policy),
               new->param.sched_priority,
               sockscf.state.haveprivs ? "" : " (normally special privileges "
                                              "are required for this)");

         return -1;
      }

      sockscf.state.cpu.policy = new->policy;
      sockscf.state.cpu.param  = new->param;
   }
#endif /* HAVE_SCHED_SETSCHEDULER */

#if HAVE_SCHED_SETAFFINITY
   if (new->affinity_isset && (cpu_equal(&old->mask, &new->mask) == 0)) {
      const size_t setsize = cpu_get_setsize();
      const long cpuc      = sysconf(_SC_NPROCESSORS_ONLN);
      size_t i, cpus_used, oldcpus_used, errcpus_used;
      char cpus[2048], oldcpus[sizeof(cpus)], errcpus[sizeof(cpus)];

      if (cpuc == -1)
         serr("sysconf(_SC_NPROCESSORS_ONLN) failed");

      *cpus     = *oldcpus     = *errcpus     = NUL;
      cpus_used = oldcpus_used = errcpus_used = 0;
      for (i = 0; i < setsize; ++i) {
         if (cpu_isset(i, &old->mask)) {
            oldcpus_used += snprintf(&oldcpus[oldcpus_used],
                                 sizeof(oldcpus) - oldcpus_used,
                                 "%ld ", (long)i);
         }

         if (cpu_isset(i, &new->mask)) {
            cpus_used += snprintf(&cpus[cpus_used],
                                 sizeof(cpus) - cpus_used,
                                 "%ld ", (long)i);

            if (i + 1 >= (size_t)cpuc)
               errcpus_used += snprintf(&errcpus[errcpus_used],
                                    sizeof(errcpus) - errcpus_used,
                                    "%ld ", (long)i);
         }
      }

      slog(LOG_DEBUG, "%s: old cpu affinity: %s, new: %s",
           function,
           *oldcpus == NUL ? "<none set>" : oldcpus,
           *cpus    == NUL ? "<none set>" : cpus);

      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
      rc = cpu_setaffinity(0, sizeof(new->mask), &new->mask);
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);

      if (rc != 0) {
         if (*errcpus != NUL)
            swarn("%s: failed to set new cpu affinity.  Probably because the "
                  "configured new mask contains the following cpus which do "
                  "not appear to be present on this system (which as far as we "
                  "can see, has a total of %ld cpus): %s",
                  function, cpuc, errcpus);
         else
            swarn("%s: failed to set new cpu affinity using mask %s",
                  function, cpuset2string(&new->mask, NULL, 0));

         return -1;
      }

      sockscf.state.cpu.mask = new->mask;
   }

#endif /* HAVE_SCHED_SETAFFINITY */
#endif /* HAVE_SCHED_SETSCHEDULER || HAVE_SCHED_SETAFFINITY */

#if HAVE_PROCESSOR_BIND
#endif /* HAVE_PROCESSOR_BIND */

   return 0;
}
