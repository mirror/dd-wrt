/*
 * Copyright (c) 2011, 2012, 2013
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
"$Id: statistics.c,v 1.33 2013/10/27 15:24:43 karls Exp $";


int
sockd_check_ipclatency(description, tsent, treceived, tnow)
   const char *description;
   const struct timeval *tsent;
   const struct timeval *treceived;
   const struct timeval *tnow;
{
#if DIAGNOSTIC
   const char *function = "sockd_check_ipclatency()";
   const size_t samplesneeded             = 1000,
                minoccurences             = 10;
   const time_t tseconds_between_warnings = 60;
   static struct timeval tmaxdelay, tmaxdelay_so_far;
   static size_t samplec;
   struct timeval tdiff;

   timersub(treceived, tsent, &tdiff);

   if (tdiff.tv_sec < 0) {
      swarnx("%s: strange ... received ts (%ld.%06ld) is earlier than "
             "sent ts (%ld.%06ld) .  Did the clock step backwards?",
             function,
             (long)treceived->tv_sec,
             (long)treceived->tv_usec,
             (long)tsent->tv_sec,
             (long)tsent->tv_usec);

      return 0;
   }
   else
      slog(LOG_DEBUG, "%s: %s: used %luus to receive object",
           function, description, tv2us(&tdiff));

   if (timerisset(&tmaxdelay)) {
      if (timercmp(&tdiff, &tmaxdelay, >)) {
         static time_t tlastwarn, tlongest;
         static size_t overloadc;

         ++overloadc;

         if ((time_t)tv2us(&tdiff) > tlongest)
            tlongest = (time_t)tv2us(&tdiff);

         if (socks_difftime(tnow->tv_sec, tlastwarn)
         >= tseconds_between_warnings) {
            if (overloadc >= minoccurences)
               slog(LOG_NOTICE,
                    "server overload condition detected %lu time%s regarding "
                    "%s.  Used up to %ldus to receive new client objects "
                    "during the last %lds, but expected maximum was "
                    "calibrated to %luus",
                    (unsigned long)overloadc,
                    (unsigned long)overloadc == 1 ? "" : "s",
                    description,
                    (long)tlongest,
                    (long)socks_difftime(tnow->tv_sec, tlastwarn),
                    tv2us(&tmaxdelay));

            tlastwarn = tnow->tv_sec;
            overloadc = 0;
            tlongest  = 0;
         }

         return 1;
      }

      return 0;
   }

   if (timercmp(&tdiff, &tmaxdelay_so_far, >))
      tmaxdelay_so_far = tdiff;

   if (samplec == samplesneeded) {
      tmaxdelay = tmaxdelay_so_far;

      slog(DEBUG ? LOG_INFO : LOG_DEBUG,
           "%s: max IPC delay for this %s process calibrated to be %ld.%06lds",
           function,
           childtype2string(sockscf.state.type),
           (long)tmaxdelay.tv_sec,
           (long)tmaxdelay.tv_usec);
   }
   else
      ++samplec;
#endif /* DIAGNOSTIC */

   return 0;
}
