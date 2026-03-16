/*
 * Copyright (c) 2012, 2013, 2020
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
"$Id: math.c,v 1.16.10.2 2020/11/11 16:11:59 karls Exp $";

#include <math.h>

unsigned long
medtv(struct timeval *tvarr, size_t tvsize)
{
   struct timeval med;

   if (tvsize == 0)
      return 0;
   else if (tvsize == 1)
      med = tvarr[tvsize - 1];
   else if (tvsize % 2 == 1)
      med = tvarr[(tvsize - 1) / 2];
   else {
      timeradd(&tvarr[(tvsize - 1) / 2], &tvarr[(tvsize - 1) / 2 + 1], &med);
               return tv2us(&med) / 2;
   }

   return tv2us(&med);
}

unsigned long
avgtv(struct timeval *tvarr, size_t tvsize)
{
   struct timeval sum, nsum;
   size_t i;

   if (tvsize == 0)
      return 0;

   timerclear(&sum);
   for (i = 0; i < tvsize; ++i) {
      /* don't assume multiple arguments to timer*() can point to same mem. */
      timeradd(&sum, &tvarr[i], &nsum);
      sum = nsum;
   }

   return tv2us(&sum) / tvsize;
}

unsigned long
stddevtv(struct timeval *tvarr, size_t tvsize, unsigned long avg)
{
   unsigned long long diffsum;
   size_t i;

   if (tvsize <= 1)
      return 0;

   /* get the squared sum of differences from the mean */
   for (i = 0, diffsum = 0; i < tvsize; ++i) {
      const unsigned long long diff = tv2us(&tvarr[i]) - avg;

      diffsum += diff * diff;
   }

   return (unsigned long)lround(sqrt(((double)diffsum) / tvsize));
}
