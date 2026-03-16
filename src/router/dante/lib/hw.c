/*
 * Copyright (c) 2012
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
"$Id: hw.c,v 1.12 2012/12/31 17:09:51 karls Exp $";


static void
minmaxvalueoftype(const size_t typelen, long long *min, long long *max);

static void
uminmaxvalueoftype(const size_t typelen, unsigned long long *min,
                   unsigned long long *max);


void
runenvcheck(void)
{
   const char *function = "runenvcheck()";
   struct {
      size_t expectedsize;
      size_t actualsize;
      int    issigned;
      size_t bitlength;
   } checkv[] = {      /* expected. */   /* is. */       /* signed? */ /* len */
                     { BYTEWIDTH_8BITS,  sizeof(sbits_8),   1,            8  },
                     { BYTEWIDTH_8BITS,  sizeof(ubits_8),   0,            8  },
                     { BYTEWIDTH_16BITS, sizeof(sbits_16),  1,            16 },
                     { BYTEWIDTH_16BITS, sizeof(ubits_16),  0,            16 },
                     { BYTEWIDTH_32BITS, sizeof(sbits_32),  1,            32 },
                     { BYTEWIDTH_32BITS, sizeof(ubits_32),  0,            32 }
                };
   size_t i;

   for (i = 0; i < ELEMENTS(checkv); ++i)
      if (checkv[i].expectedsize != checkv[i].actualsize)
         serrx("%s: expected size of %s %lu bit type to be %lu (based on "
               "pre-compiletime check), but now it is %lu.  "
               "Perhaps we were ./configured on a different CPU/platform "
               "from what we were later compiled on?",
               function,
               checkv[i].issigned ? "signed" : "unsigned",
               (unsigned long)checkv[i].bitlength,
               (unsigned long)checkv[i].expectedsize,
               (unsigned long)checkv[i].actualsize);
}

long long
minvalueoftype(typelen)
   const size_t typelen;
{
   long long min, max;

   minmaxvalueoftype(typelen, &min, &max);
   return min;
}

long long
maxvalueoftype(typelen)
   const size_t typelen;
{
   long long min, max;

   minmaxvalueoftype(typelen, &min, &max);

   return max;
}

unsigned long long
uminvalueoftype(typelen)
   const size_t typelen;
{
   unsigned long long min, max;

   uminmaxvalueoftype(typelen, &min, &max);
   return min;
}

unsigned long long
umaxvalueoftype(typelen)
   const size_t typelen;
{
   unsigned long long min, max;

   uminmaxvalueoftype(typelen, &min, &max);
   return max;
}

static void
minmaxvalueoftype(typelen, min, max)
   const size_t typelen;
   long long *min;
   long long *max;
{
   const char *function = "minmaxvalueoftype()";

   switch (typelen) {
      case sizeof(int8_t):
         *min = INT8_MIN;
         *max = INT8_MAX;
         break;

      case sizeof(int16_t):
         *min = INT16_MIN;
         *max = INT16_MAX;
         break;

      case sizeof(int32_t):
         *min = INT32_MIN;
         *max = INT32_MAX;
         break;

      case sizeof(int64_t):
         *min = INT64_MIN;
         *max = INT64_MAX;
         break;

      default:
         swarnx("%s: unsupported typelength %lu",
                function, (unsigned long)typelen);

         SERRX(0);
   }
}

static void
uminmaxvalueoftype(typelen, min, max)
   const size_t typelen;
   unsigned long long *min;
   unsigned long long *max;
{
   const char *function = "minmaxvalueoftype()";

   *min = 0;

   switch (typelen) {
      case sizeof(uint8_t):
         *max = UINT8_MAX;
         break;

      case sizeof(uint16_t):
         *max = UINT16_MAX;
         break;

      case sizeof(uint32_t):
         *max = UINT32_MAX;
         break;

      case sizeof(uint64_t):
         *max = UINT64_MAX;
         break;

      default:
         swarnx("%s: unsupported typelength %lu",
                function, (unsigned long)typelen);

         SERRX(0);
   }
}
