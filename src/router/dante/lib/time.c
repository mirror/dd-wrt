/*
 * Copyright (c) 2013
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
"$Id: time.c,v 1.9 2013/10/25 12:55:01 karls Exp $";

time_t
time_monotonic(tloc)
   time_t *tloc;
{
   struct timeval tnow;

   gettimeofday_monotonic(&tnow);

   if (tloc != NULL)
      *tloc = tnow.tv_sec;

   return tnow.tv_sec;
}

struct timeval *
gettimeofday_monotonic(tv)
   struct timeval *tv;
{
   const char *function = "gettimeofday_monotonic()";
   static struct timeval tv_lasttime;
   int rc;

#if HAVE_CLOCK_GETTIME_MONOTONIC
   struct timespec ts;

   rc = clock_gettime(CLOCK_MONOTONIC, &ts);
   SASSERT(rc == 0);

   SASSERTX(ts.tv_nsec <= 999999999);

   /* use timeval for now, so it's usable with timeval-macros. */
   tv->tv_sec  = ts.tv_sec;
   tv->tv_usec = ts.tv_nsec / 1000;

#else /* HAVE_CLOCK_GETTIME_MONOTONIC */

   rc = gettimeofday(tv, NULL);
   SASSERT(rc == 0);

#endif /* !HAVE_CLOCK_GETTIME_MONOTONIC */

   SASSERTX(tv->tv_usec <= 999999);

   if (timerisset(&tv_lasttime) && timercmp(tv, &tv_lasttime, <)) {
      slog(HAVE_CLOCK_GETTIME_MONOTONIC ? LOG_WARNING : LOG_DEBUG,
           "%s: looks like the clock was stepped backwards.  "
           "Was %ld.%06ld, is %ld.%06ld",
           function,
           (long)tv_lasttime.tv_sec,
           (long)tv_lasttime.tv_usec,
           (long)tv->tv_sec,
           (long)tv->tv_usec);

      *tv = tv_lasttime;

      if (tv->tv_usec < 999999)
         ++tv->tv_usec; /* no idea, but better than nothing. */
   }

   /*
    * for some reason Coverity produces a warning about
    * tv_lasttime.tv_usec being unitialized if we do a struct
    * assignment here. :-/
    */
   tv_lasttime.tv_sec  = tv->tv_sec;
   tv_lasttime.tv_usec = tv->tv_usec;

   return tv;
}

unsigned long
tv2us(tv)
   const struct timeval *tv;
{

   return (unsigned long)((tv)->tv_sec * 1000000 + (tv)->tv_usec);
}

struct timeval *
us2tv(us, tv)
   const unsigned long us;
   struct timeval *tv;
{

   if (us < 1000000) {
      tv->tv_sec  = 0;
      tv->tv_usec = us;
   }
   else {
      tv->tv_sec  = us / 1000000;
      tv->tv_usec = us % 1000000;
   }

   return tv;
}
