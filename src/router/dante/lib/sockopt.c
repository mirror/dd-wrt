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

#include "qos.h"

static const char rcsid[] =
"$Id: sockopt.c,v 1.26.18.1 2024/11/21 10:22:42 michaels Exp $";

struct option {
   int        level;
   int        optname;
   const char *optstr;
};
static const struct option option[];
static const sockopt_t sockopts[];
static const sockoptvalsym_t sockoptvalsyms[];


int
socketoptdup(s, new_s)
   int s;
   int new_s;
{
   const char *function = "socketoptdup()";
   unsigned int i;
   int flags, errno_s;
   socklen_t len;
   socketoptvalue_t val;

   errno_s = errno;

   slog(LOG_DEBUG, "%s: fd %d, fd %d", function, s, new_s);

   if (new_s == -1) {
      struct sockaddr_storage addr;
      int domain;

      len = sizeof(addr);
      if (getsockname(s, TOSA(&addr), &len) == -1) {
         swarn("%s: getsockname(2) failed" , function);
         return -1;
      }

      domain = addr.ss_family;

      len = sizeof(val.int_val);
      if (getsockopt(s, SOL_SOCKET, SO_TYPE, &val, &len) == -1) {
         swarn("%s: getsockopt(SO_TYPE) failed", function);
         return -1;
      }

      if ((new_s = socket(domain, val.int_val, 0)) == -1) {
         swarn("%s: socket(%d, %d)", function, domain, val.int_val);
         return -1;
      }
   }

   for (i = 0; i < HAVE_DUPSOCKOPT_MAX; ++i) {
      len = sizeof(val);
      if (getsockopt(s, option[i].level, option[i].optname, &val, &len) == -1) {
         if (errno != ENOPROTOOPT)
            slog(LOG_DEBUG, "%s: getsockopt(%d, %d) failed: %s",
                 function, option[i].level, option[i].optname, strerror(errno));

         continue;
      }

      if (setsockopt(new_s, option[i].level, option[i].optname, &val, len)
      == -1)
         if (errno != ENOPROTOOPT)
            slog(LOG_DEBUG, "%s: setsockopt(%d, %d) failed: %s",
                 function, option[i].level, option[i].optname, strerror(errno));
   }

   if ((flags = fcntl(s, F_GETFL, 0))          == -1
   ||           fcntl(new_s, F_SETFL, flags)   == -1)
      swarn("%s: fcntl(F_GETFL/F_SETFL)", function);

   errno = errno_s;
   return new_s;
}

#if DEBUG
void
printsocketopts(s)
   const int s;
{
   const char *function = "printsocketopts()";
   unsigned int i;
   int flags, errno_s;
   socklen_t len;
   socketoptvalue_t val;

   errno_s = errno;

   len = sizeof(val);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &val, &len) == -1) {
      swarn("%s: getsockopt(SO_TYPE)", function);
      return;
   }

   for (i = 0; i < HAVE_DUPSOCKOPT_MAX; ++i) {
      len = sizeof(val);
      if (getsockopt(s, option[i].level, option[i].optname, &val, &len) == -1) {
         if (errno != ENOPROTOOPT)
            swarn("%s: getsockopt(%s) failed", function, option[i].optstr);

         continue;
      }

      slog(LOG_DEBUG, "%s: value of socket option \"%s\" is %d\n",
           function, option[i].optstr, val.int_val);
   }

   if ((flags = fcntl(s, F_GETFL, 0)) == -1)
      swarn("%s: fcntl(F_GETFL)", function);
   else
      slog(LOG_DEBUG, "%s: value of file status flags: %d\n", function, flags);

   if ((flags = fcntl(s, F_GETFD, 0)) == -1)
      swarn("fcntl(F_GETFD)");
   else
      slog(LOG_DEBUG, "%s: value of file descriptor flags: %d\n",
           function, flags);

   errno = errno_s;
}

#endif /* DEBUG */

void
sockopts_dump(void)
{
   const char *function = "sockopts_dump()";
   int i;

   slog(LOG_DEBUG, "%s: socket option name (level/value) (%d entries):",
        function, HAVE_SOCKOPTVAL_MAX);

   for (i = 0; i < HAVE_SOCKOPTVAL_MAX; i++)
      slog(LOG_DEBUG, "%s: %02d: %s (%d/%d)",
           function, i, sockopts[i].name, sockopts[i].level, sockopts[i].value);

   slog(LOG_DEBUG, "%s: socket option symbolic values (%d entries):",
        function, HAVE_SOCKOPTVALSYM_MAX);

   for (i = 0; i < HAVE_SOCKOPTVALSYM_MAX; i++) {
      const sockopt_t *opt;

      SASSERTX(sockoptvalsyms[i].optid < HAVE_SOCKOPTVAL_MAX);

      opt = &sockopts[sockoptvalsyms[i].optid];

      slog(LOG_DEBUG, "%s: %02d: %s: %s (%s)",
           function,
           i,
           opt->name,
           sockoptvalsyms[i].name,
           sockoptval2string(sockoptvalsyms[i].symval, opt->opttype, NULL, 0));
   }
}

const sockopt_t *
optname2sockopt(char *name)
{
   int i;

   for (i = 0; i < HAVE_SOCKOPTVAL_MAX; i++) {
      if (strcmp(name, sockopts[i].name) == 0)
         return &sockopts[i];
   }

   return NULL;
}

const sockopt_t *
optval2sockopt(int level, int value)
{
   int i;

   for (i = 0; i < HAVE_SOCKOPTVAL_MAX; i++) {
      if (level == sockopts[i].level && value == sockopts[i].value)
         return &sockopts[i];
   }

   return NULL;
}

const sockopt_t *
optid2sockopt(size_t optid)
{
   SASSERTX(optid < HAVE_SOCKOPTVAL_MAX);

   return &sockopts[optid];
}

const sockoptvalsym_t *
optval2valsym(size_t optid, char *name)
{
   int i;

   for (i = 0; i < HAVE_SOCKOPTVALSYM_MAX; i++) {
      if (optid == sockoptvalsyms[i].optid
      && strcmp(name, sockoptvalsyms[i].name) == 0)
         return &sockoptvalsyms[i];
   }

   return NULL;
}

/*
 * Include platform dependent socket option code.
 */
#include "sockopt_gen.c"
