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
#include "errorsymbols.h"

static const char rcsid[] =
"$Id: errorsymbols.c,v 1.3 2013/10/25 12:55:00 karls Exp $";

static void
doscan(const char *symbol, size_t *valuec, int valuev[], const size_t tablec,
       const errorsymboltable_t tablev[]);
/*
 * Scans through "tablev", containing "tablec" elements, looking for
 * symbols matching "symbol".  For each match found, the corresponding
 * numeric value is added to "valuev", which must be large enough to
 * hold "valuec" elements.  Upon return, "valuec" is updated to contain
 * the number of values actually stored in "valuev", zero-terminated.
 */

const int *
errnovalue(symbol)
   const char *symbol;
{
   static int valuev[MAX_ERRNO_VALUES_FOR_SYMBOL + 1];
   size_t valuec;

   valuec = ELEMENTS(valuev);
   doscan(symbol, &valuec, valuev, ELEMENTS(errnosymbolv), errnosymbolv);

   if (valuec == 0)
      return NULL;
   else
      return valuev;
}

const int *
gaivalue(symbol)
   const char *symbol;
{
   static int valuev[MAX_GAIERR_VALUES_FOR_SYMBOL + 1];
   size_t valuec;

   valuec = ELEMENTS(valuev);
   doscan(symbol, &valuec, valuev, ELEMENTS(gaierrsymbolv), gaierrsymbolv);

   if (valuec == 0)
      return NULL;
   else
      return valuev;
}


static void
doscan(symbol, valuec, valuev, tablec, tablev)
   const char *symbol;
   size_t *valuec;
   int valuev[];
   const size_t tablec;
   const errorsymboltable_t tablev[];
{
   const size_t maxvaluec = *valuec;
   size_t i;

   *valuec = 0;
   for (i = 0; i < tablec; ++i) {
      if (strcasecmp(tablev[i].name, symbol) == 0) {
         valuev[(*valuec)++] = tablev[i].value;

         /*
          * don't break out of loop; same symbol can expand to multiple values.
          */
      }

      SASSERTX(*valuec < maxvaluec);
   }

   if (*valuec > 0) {
      SASSERTX(*valuec < maxvaluec);
      valuev[(*valuec)++] = 0; /* zero-terminated. */
   }

   SASSERTX(*valuec <= maxvaluec);
}
