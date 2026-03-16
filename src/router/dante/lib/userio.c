/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2010, 2011, 2012
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
"$Id: userio.c,v 1.60.22.1 2024/11/21 10:22:43 michaels Exp $";

/* ARGSUSED */

char *
socks_getusername(
   const sockshost_t *host,
   char *buf,
   size_t buflen)
{
   const char *function = "socks_getusername()";
   const char *name;
   size_t len;
   char visname[MAXNAMELEN * 4];

   if ((name = socks_getenv(ENV_SOCKS_USERNAME, dontcare)) == NULL
   &&  (name = socks_getenv(ENV_SOCKS_USER,     dontcare)) == NULL
   &&  (name = socks_getenv(ENV_SOCKS5_USER,    dontcare)) == NULL)
      return NULL;

   slog(LOG_NEGOTIATE,
        "%s: using socks username from environment: \"%s\"",
        function, str2vis(name, -1, visname, sizeof(visname)));

   if ((len = strlen(name)) >= buflen) {
      swarnx("%s: username of length %d in environment truncated to %d",
             function, (int)len, (int)buflen - 1);

      len = buflen - 1;
   }

   memcpy(buf, name, len);
   buf[len] = NUL;

   return buf;
}
char *
socks_getpassword(
   const sockshost_t *host,
   const char *user,
   char *buf,
   size_t buflen)
{
   const char *function = "socks_getpassword()";
   const char *password;
   size_t len;

   if ((password = socks_getenv(ENV_SOCKS_PASSWORD, dontcare)) == NULL
   &&  (password = socks_getenv(ENV_SOCKS_PASSWD,   dontcare)) == NULL
   &&  (password = socks_getenv(ENV_SOCKS5_PASSWD,  dontcare)) == NULL)
      return NULL;

   if ((len = strlen(password)) >= buflen) {
      swarnx("%s: password of length %d from environment truncated to %d",
             function, (int)len, (int)buflen - 1);

      len = buflen - 1;
   }

   memcpy(buf, password, len);
   buf[len] = NUL;

   /*
    * don't bzero() environment.
    *
    * bzero(password, pwlen);
    */

   return buf;
}
const char *
socks_getenv(name, value)
   const char *name;
   value_t value;
{
   const char *p = NULL;

#if SOCKS_CLIENT
#if HAVE_CONFENV_DISABLE
   const char *safenames[] = { ENV_SOCKS_BINDLOCALONLY,
                               ENV_SOCKS_DEBUG,
                               ENV_SOCKS_DISABLE_THREADLOCK,
                               ENV_SOCKS_DIRECTROUTE_FALLBACK,
                               ENV_SOCKS_PASSWORD,
                               ENV_SOCKS_PASSWD,
                               ENV_SOCKS5_PASSWD,
                               ENV_SOCKS_USERNAME,
                               ENV_SOCKS_USER,
                               ENV_SOCKS5_USER,
   };
   size_t i;
#endif /* HAVE_CONFENV_DISABLE */

   if (strcmp(name, ENV_SOCKS_CONF)         == 0
   ||  strcmp(name, ENV_SOCKS_LOGOUTPUT)    == 0
   ||  strcmp(name, ENV_SOCKS_ERRLOGOUTPUT) == 0
   ||  strcmp(name, ENV_TMPDIR)             == 0) {
      /*
       * Even if getenv() is not disabled, we don't want to return
       * anything for this if the program may be running setuid,
       * as it could allow reading/writing of arbitrary files.
       */
      if (issetugid())
         return NULL;
      else
         return getenv(name);
   }

#if HAVE_CONFENV_DISABLE
   /*
    * If the name is safe, get it regardless of confenv being disabled.
    */
   for (i = 0; i < ELEMENTS(safenames); ++i)
      if (strcmp(name, safenames[i]) == 0) {
         p = getenv(name);
         break;
      }

#else /* !HAVE_CONFENV_DISABLE */
   p = getenv(name);
#endif /* !HAVE_CONFENV_DISABLE */

#else /* !SOCKS_CLIENT */
   p = getenv(name);
#endif /* !SOCKS_CLIENT */

   if (p == NULL || value == dontcare) {
      /*
       * Some variables have a default based on configure/define.
       */
      if (strcmp(name, ENV_SOCKS_DIRECTROUTE_FALLBACK) == 0)
         p = (SOCKS_DIRECTROUTE_FALLBACK ? "yes" : "no");
      else
         return p;
   }

   switch (value) {
      case istrue:
         if (strcasecmp(p, "yes")  == 0
         ||  strcasecmp(p, "true") == 0
         ||  strcasecmp(p, "1")    == 0)
            return p;
         return NULL;

      case isfalse:
         if (strcasecmp(p, "no")    == 0
         ||  strcasecmp(p, "false") == 0
         ||  strcasecmp(p, "0")     == 0)
            return p;
         return NULL;

      default:
         SERRX(value);
   }

   /* NOTREACHED */
}
