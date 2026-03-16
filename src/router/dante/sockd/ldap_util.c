/*
 * Copyright (c) 2010, 2011, 2012, 2013
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

 /*
  * This code was contributed by
  * Markus Moeller (markus_moeller at compuserve.com).
  */

#include "common.h"

static const char rcsid[] =
"$Id: ldap_util.c,v 1.14 2013/10/27 15:24:42 karls Exp $";

#if HAVE_LDAP
char
*asciitoutf8(input)
   char *input;
{
   const char *function = "asciitoutf8()";
   size_t n, a;
   unsigned char *p, *utf8;
   char *at;
   int c, s;

   SASSERTX(input != NULL);

   for (n = 0, c = 0; n < strlen(input); ++n)
      if ((unsigned char)input[n] > 127)
         c++;

   at = strrchr(input, '@');
   if (at)
      a = at - input;
   else
      a = strlen(input) - 1;

   if (c != 0) {
      if ((p = malloc(strlen(input) + 1 + c)) == NULL)
         serrx("%s: %s", function, NOMEM);

      utf8 = p;
      for (n = 0; n < strlen(input); ++n) {
         if (n == a) {
            /* Do not change domain name */
            break;
         }
         s = (unsigned char)input[n];
         /* Change ASCII > 127 to UTF-8
            0xC2 0x80-0xBF
            0xC3 0x80-0xBF
         */
         if (s > 127 && s < 192) {
            *p = 194;
            p++;
            *p = s;
         } else if (s > 191 && s < 256) {
            *p = 195;
            p++;
            *p = s - 64;
         } else
            *p = s;
         p++;
      }

      *p = NUL;

      slog(LOG_DEBUG, "%s: Group %s as UTF-8: %s", function, input, utf8);

      return (char *)utf8;
   }
   else
      return input;
}

char
*hextoutf8(input, flag)
   const char *input;
   int flag;
{
/*
   UTF8    = UTF1 / UTFMB
   UTFMB   = UTF2 / UTF3 / UTF4

   UTF0    = %x80-BF
   UTF1    = %x00-7F
   UTF2    = %xC2-DF UTF0
   UTF3    = %xE0 %xA0-BF UTF0 / %xE1-EC 2(UTF0) /
             %xED %x80-9F UTF0 / %xEE-EF 2(UTF0)
   UTF4    = %xF0 %x90-BF 2(UTF0) / %xF1-F3 3(UTF0) /
             %xF4 %x80-8F 2(UTF0)

   http://www.utf8-chartable.de/unicode-utf8-table.pl
*/
   const char *function = "hextoutf8()";
   size_t i, n, a;
   unsigned char *utf8;
   char *p, *at;
   int ival, ichar, iUTF2, iUTF3, iUTF4;

   SASSERTX(input != NULL);

   at = strrchr(input, '@');
   if (at)
      a = at - input;
   else
      a = strlen(input) ;

   if ((utf8 = malloc(strlen(input) + 1)) == NULL)
      serrx("%s: %s", function, NOMEM);

   i     = 0;
   iUTF2 = 0;
   iUTF3 = 0;
   iUTF4 = 0;

   n = 0;
   while (n < strlen(input)) {
      if (!flag && n == a) {
         /* Do not change domain name */
         break;
      }
      if (flag < 2 && input[n] == '@') {
         /* Do not change @ separator */
         utf8[i] = '@';
         i++;
         n++;
      }

      ival = input[n];
      if (ival > 64 && ival < 71)
         ichar = (ival - 55) * 16;
      else if (ival > 96 && ival < 103)
         ichar = (ival - 87)*16;
      else if (ival > 47 && ival < 58)
         ichar = (ival - 48)*16;
      else {
         slog(LOG_DEBUG, "%s: invalid Hex value 0x%x", function, ival);

         SASSERTX((ival > 64 && ival < 71) ||
                  (ival > 96 && ival < 103) ||
                  (ival > 47 && ival < 58));
      }

      if (n == a - 1) {
        slog(LOG_DEBUG, "%s: invalid Hex UTF-8 string \"%s\"", function, input);
        SASSERTX(n < a - 1);
      }

      n++;
      ival = input[n];
      if (ival > 64 && ival < 71) ichar = ichar + ival - 55;
      else if (ival > 96 && ival < 103) ichar = ichar + ival - 87;
      else if (ival > 47 && ival < 58) ichar = ichar + ival - 48;
      else {
         slog(LOG_DEBUG, "%s: invalid Hex value 0x%x", function, ival);
         SASSERTX((ival > 64 && ival < 71) ||
                  (ival > 96 && ival < 103) ||
                  (ival > 47 && ival < 58));
      }

      if (iUTF2) {
         if (iUTF2 == 0xC2 && ichar > 0x7F && ichar < 0xC0) {
            iUTF2 = 0;
            utf8[i-1] = ichar;
         }
         else if (iUTF2 == 0xC3 && ichar > 0x7F && ichar < 0xC0) {
            iUTF2 = 0;
            utf8[i-1] = ichar + 64;
         }
         else if (iUTF2 > 0xC3 && iUTF2 < 0xE0 && ichar > 0x7F
         && ichar < 0xC0) {
            iUTF2 = 0;
            utf8[i] = ichar;
            i++;
         }
         else {
            iUTF2 = 0;
            utf8[i] = ichar;
            utf8[i + 1] = NUL;

            slog(LOG_DEBUG, "%s: invalid UTF-8 sequence for Unicode \"%s\"",
            function, utf8);

            SASSERTX((iUTF2 == 0xC2 && ichar > 0x7F && ichar < 0xC0)
            ||       (iUTF2 == 0xC3 && ichar > 0x7F && ichar < 0xC0)
            ||       (iUTF2 > 0xC3 && iUTF2 < 0xE0 && ichar > 0x7F
                   && ichar < 0xC0));
         }
      }
      else if (iUTF3) {
         if (iUTF3 == 0xE0 && ichar > 0x9F && ichar < 0xC0) {
            iUTF3 = 1;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF3 > 0xE0 && iUTF3 < 0xED && ichar > 0x7F
         && ichar < 0xC0) {
            iUTF3 = 2;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF3 == 0xED && ichar > 0x7F && ichar < 0xA0) {
            iUTF3 = 3;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF3 > 0xED && iUTF3 < 0xF0 && ichar > 0x7F && ichar < 0xC0){
            iUTF3 = 4;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF3 > 0 && iUTF3 < 5 && ichar > 0x7F && ichar < 0xC0) {
            iUTF3 = 0;
            utf8[i] = ichar;
            i++;
         }
         else {
            iUTF3 = 0;
            utf8[i] = ichar;
            utf8[i + 1] = NUL;

            slog(LOG_DEBUG, "%s: invalid UTF-8 sequence for unicode \"%s\"",
            function, utf8);

            SASSERTX((iUTF3 == 0xE0 && ichar > 0x9F && ichar < 0xC0)
            ||       (iUTF3 > 0xE0 && iUTF3 < 0xED && ichar > 0x7F
                   && ichar < 0xC0)
            ||       (iUTF3 == 0xED && ichar > 0x7F && ichar < 0xA0)
            ||       (iUTF3 > 0xED && iUTF3 < 0xF0 && ichar > 0x7F
                   && ichar < 0xC0)
            || (iUTF3 > 0 && iUTF3 < 5 && ichar > 0x7F && ichar < 0xC0));
         }
      }
      else if (iUTF4) {
         if (iUTF4 == 0xF0 && ichar > 0x8F && ichar < 0xC0) {
            iUTF4 = 1;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF4 > 0xF0 && iUTF3 < 0xF4 && ichar > 0x7F && ichar < 0xC0){
            iUTF4 = 2;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF4 == 0xF4 && ichar > 0x7F && ichar < 0x90) {
            iUTF4 = 3;
            utf8[i] = ichar;
            i++;
         }
         else if (iUTF4 > 0 && iUTF4 < 5 && ichar > 0x7F && ichar < 0xC0) {
            if (iUTF4 == 4)
               iUTF4 = 0;
            else
               iUTF4 = 4;

            utf8[i] = ichar;
            i++;
         }
         else {
            iUTF4 = 0;
            utf8[i]     = ichar;
            utf8[i + 1] = NUL;

            slog(LOG_DEBUG, "%s: invalid UTF-8 sequence for Unicode \"%s\"",
            function, utf8);

            SASSERTX((iUTF4 == 0xF0 && ichar > 0x8F && ichar < 0xC0)
            ||       (iUTF4 > 0xF0 && iUTF3 < 0xF4 && ichar > 0x7F
                   && ichar < 0xC0)
            ||       (iUTF4 == 0xF4 && ichar > 0x7F && ichar < 0x90)
            ||       (iUTF4 > 0 && iUTF4 < 5 && ichar > 0x7F && ichar < 0xC0));
         }
      }
      else if (ichar < 0x80) {
          /* UTF1 */
          utf8[i] = ichar;
          i++;
      }
      else if (ichar > 0xC1 && ichar < 0xE0) {
          /* UTF2 (Latin) */
          iUTF2 = ichar;
          utf8[i] = ichar;
          i++;
      }
      else if (ichar > 0xDF && ichar < 0xF0) {
          /* UTF3 */
          iUTF3 = ichar;
          utf8[i] = ichar;
          i++;
      }
      else if (ichar > 0xEF && ichar < 0xF5) {
          /* UTF4 */
          iUTF4 = ichar;
          utf8[i] = ichar;
          i++;
      } else {
          utf8[i]   = ichar;
          utf8[i+1] = NUL;

          slog(LOG_DEBUG, "%s: invalid UTF-8 sequence for Unicode \"%s\"",
          function, utf8);

          SASSERTX(iUTF2 || iUTF3 || iUTF4 || (ichar < 0x80)
          ||      (ichar > 0xC1 && ichar < 0xE0)
          ||      (ichar > 0xDF && ichar < 0xF0)
          ||      (ichar > 0xEF && ichar < 0xF5));
      }

      n++;
   }

   utf8[i] = NUL;
   if (iUTF2 || iUTF3 || iUTF4) {
      slog(LOG_DEBUG, "%s: invalid UTF-8 sequence for Unicode \"%s\"",
           function, utf8);

      SASSERTX(!iUTF2 && !iUTF3 && !iUTF4);
   }

   p = (char *)utf8;
   if (!flag && at)
      strcat(p, at);

   return p;
}

#endif /* HAVE_LDAP */
