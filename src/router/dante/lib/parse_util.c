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
"$Id: parse_util.c,v 1.7 2013/10/25 12:55:01 karls Exp $";

extern unsigned char parsingconfig;
extern char          *atype;
extern int           yylineno;
extern char          *yytext;

static char *
getparsingerror(char *buf, const size_t buflen);
/*
 * Writes the text related to the current parsing error to the string
 * "buf", which of size buflen.
 */


void
yyerror(const char *fmt, ...)
{
   va_list ap;
   char buf[2048];
   size_t bufused;

   if (parsingconfig) {
      char prefix[512];

      bufused = snprintf(buf, sizeof(buf), "%s: ",
                         getparsingerror(prefix, sizeof(prefix)));
   }
   else
      bufused = 0;

   va_start(ap, fmt);
   vsnprintf(&buf[bufused], sizeof(buf) - bufused, fmt, ap);
   va_end(ap);

   if (errno == 0)
      serrx("%s.  Please see the %s manual for more information", buf, PRODUCT);
   else
      serrx("%s: %s.  Please see the %s manual for more information",
            buf, strerror(errno), PRODUCT);
}

void
yyerrorx(const char *fmt, ...)
{
   va_list ap;
   char buf[2048];
   size_t bufused;

   if (parsingconfig) {
      char prefix[512];

      bufused = snprintf(buf, sizeof(buf), "%s: ",
                         getparsingerror(prefix, sizeof(prefix)));
   }
   else
      bufused = 0;

   va_start(ap, fmt);
   vsnprintf(&buf[bufused], sizeof(buf) - bufused, fmt, ap);
   va_end(ap);

   serrx("%s.  Please see the %s manual for more information", buf, PRODUCT);
}

void
yywarn(const char *fmt, ...)
{
   va_list ap;
   char buf[2048];
   size_t bufused;

   if (parsingconfig) {
      char prefix[512];

      bufused = snprintf(buf, sizeof(buf), "%s: ",
                         getparsingerror(prefix, sizeof(prefix)));
   }
   else
      bufused = 0;

   va_start(ap, fmt);
   vsnprintf(&buf[bufused], sizeof(buf) - bufused, fmt, ap);
   va_end(ap);

   if (errno == 0)
      swarnx("%s.  Please see the %s manual for more information",
             buf, PRODUCT);
   else
      swarnx("%s.  %s.  Please see the %s manual for more information",
             buf, strerror(errno), PRODUCT);
}

void
yywarnx(const char *fmt, ...)
{
   va_list ap;
   char buf[2048];
   size_t bufused;

   if (parsingconfig) {
      char prefix[512];

      bufused = snprintf(buf, sizeof(buf), "%s: ",
                         getparsingerror(prefix, sizeof(prefix)));
   }
   else
      bufused = 0;

   va_start(ap, fmt);
   vsnprintf(&buf[bufused], sizeof(buf) - bufused, fmt, ap);
   va_end(ap);

   swarnx("%s.  Please see the %s manual for more information", buf, PRODUCT);
}

void
yylog(const int loglevel, const char *fmt, ...)
{
   va_list ap;
   char buf[2048];
   size_t bufused;

   if (parsingconfig) {
      char prefix[512];

      bufused = snprintf(buf, sizeof(buf), "%s: ",
                         getparsingerror(prefix, sizeof(prefix)));
   }
   else
      bufused = 0;

   va_start(ap, fmt);
   vsnprintf(&buf[bufused], sizeof(buf) - bufused, fmt, ap);
   va_end(ap);

   slog(loglevel, "%s.  Please see the %s manual for more information",
        buf, PRODUCT);
}


void
yyerrorx_hasnetmask(void)
{

   yyerrorx("A netmask has been specified for this %s, but no netmask "
            "should be specified in this context",
            atype2string(*atype));
}

void
yyerrorx_nonetmask(void)
{

   yyerrorx("No netmask has been specified for this %s, but a netmask "
            "must be specified in this context.  "
            "You can specify a netmask using the standard "
            "\"/ <netmask bits>\" syntax",
            atype2string(*atype));
}

void
yyerrorx_nolib(library)
   const char *library;
{

   yyerrorx("in order to set the given keyword/value, %s must be linked "
            "with an external %s library at compiletime.  When %s was "
            "compiled, no such linking was done, perhaps due to no "
            "usable %s library being installed on the system at the time.  "
            "If a %s library has since been installed on this system, "
            "please rerun ./configure in %s's source directory, recompile "
            "and reinstall %s",
            PRODUCT,
            library,
            PRODUCT,
            library,
            library,
            PRODUCT,
            PRODUCT);
}

void
yywarnx_deprecated(oldkeyword, newkeyword)
   const char *oldkeyword;
   const char *newkeyword;
{

   if (newkeyword == NULL)
      yywarnx("keyword \"%s\" is deprecated and no longer used.  "
              "Please remove the keyword from %s's config file (%s)",
              oldkeyword, PRODUCT, sockscf.option.configfile);
   else
      yywarnx("keyword \"%s\" is deprecated - assuming the new keyword \"%s\" "
              "was meant.  Please update %s's config file (%s) to use the "
              "new keyword as appropriate",
              oldkeyword,
              newkeyword,
              PRODUCT,
              sockscf.option.configfile);
}


static char *
getparsingerror(buf, buflen)
   char *buf;
   const size_t buflen;
{
#if 0 /* something wrong with the lex-file. */
   const char *strip = " \n\t";
   char visbuf1[100], visbuf2[sizeof(visbuf1)], yytextvis[100],
        *l1 = previouslexline, *l2 = currentlexline;

   SKIPLEADING(l1, strip);
   STRIPTRAILING(l1, strlen(l1), strip);

   SKIPLEADING(l2, strip);
   STRIPTRAILING(l2, strlen(l2), strip);

   snprintfn(buf, buflen,
             "%s: problem on line %d near \"%s %s\", at token \"%.20s\"",
             sockscf.option.configfile,
             yylineno,
             str2vis(l1, strlen(l1), visbuf1, sizeof(visbuf1)),
             str2vis(l2, strlen(l2), visbuf2, sizeof(visbuf2)),
             (yytext == NULL || *yytext == NUL) ?
                 "<start of line>" : str2vis(yytext,
                                             strlen(yytext),
                                             yytextvis,
                                             sizeof(yytextvis)));
#else
   char yytextvis[100];

   snprintfn(buf, buflen,
             "%s: problem on line %d near token \"%.20s\"",
             sockscf.option.configfile,
             yylineno,
             (yytext == NULL || *yytext == NUL) ?
                 "'start of line'" : str2vis(yytext,
                                             strlen(yytext),
                                             yytextvis,
                                             sizeof(yytextvis)));
#endif

   return buf;
}
