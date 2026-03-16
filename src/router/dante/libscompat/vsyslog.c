/* $Id: vsyslog.c,v 1.14 2013/02/24 20:01:26 karls Exp $ */

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#include "osdep.h"

#undef vsyslog /* avoid any changes for applications done by headers */

/* attempt to be clever; construct string and call syslog */

/*
 * Copyright (c) 1983, 1988, 1993
 *   The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char rcsid[] = "$OpenBSD: syslog.c,v 1.8 1998/03/19 00:30:03 millert Exp $";
#endif /* LIBC_SCCS and not lint */

void
vsyslog(pri, fmt, ap)
   int pri;
   register const char *fmt;
   va_list ap;
{
   register char ch, *t;

   int saved_errno;
#define   TBUF_LEN   2048
#define   FMT_LEN      1024
   char tbuf[TBUF_LEN], fmt_cpy[FMT_LEN];
   int fmt_left, prlen;

   saved_errno = errno;

   /* Build the message. */

   /*
    * We wouldn't need this mess if printf handled %m, or if
    * strerror() had been invented before syslog().
    */
   for (t = fmt_cpy, fmt_left = FMT_LEN; (ch = *fmt); ++fmt) {
      if (ch == '%' && fmt[1] == 'm') {
         ++fmt;
         prlen = snprintf(t, fmt_left, "%s",
             strerror(saved_errno));
         if (prlen >= fmt_left)
            prlen = fmt_left - 1;
         t += prlen;
         fmt_left -= prlen;
      } else {
         if (fmt_left > 1) {
            *t++ = ch;
            fmt_left--;
         }
      }
   }
   *t = '\0';

   prlen = vsnprintf(tbuf, TBUF_LEN, fmt_cpy, ap);

   syslog(pri, "%s", tbuf);

   return;
}
