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

#include "common.h"

#if HAVE_BSDAUTH

static const char rcsid[] =
"$Id: auth_bsd.c,v 1.25 2013/10/27 15:24:42 karls Exp $";

#include <login_cap.h>
#include <bsd_auth.h>

int
bsdauth_passwordcheck(s, src, dst, auth, emsg, emsgsize)
   int s;
   const struct sockaddr_storage *src, *dst;
   authmethod_bsd_t *auth;
   char *emsg;
   size_t emsgsize;
{
   const char *function = "bsdauth_passwordcheck()";
   char password[MAXPWLEN], *style;
   char visname[MAXNAMELEN * 4];

   int rc;

   if (*auth->style == NUL)
      style = NULL;
   else
      style = auth->style;

   /* auth_userokay clears password parameter, pass a copy */
   strncpy(password, (char *)auth->password, sizeof(password) - 1);
   password[sizeof(password) - 1] = NUL;

   str2vis((char *)auth->name,
           strlen((char *)auth->name),
           visname, sizeof(visname));

   slog(LOG_DEBUG, "%s: bsdauth style to use for user \"%s\": %s",
        function, visname, style == NULL ? "default" : style);

   /*
    * note: NULL password would lead to libc requesting it interactively.
    * if NULL, user can specify in username, e.g., uname:radius
    */
   sockd_priv(SOCKD_PRIV_BSDAUTH, PRIV_ON);
   rc = auth_userokay((char *)auth->name, style, "auth-sockd", password);
   sockd_priv(SOCKD_PRIV_BSDAUTH, PRIV_OFF);

   if (rc == 0) {
      slog(LOG_DEBUG, "%s: bsdauth method failed for user \"%s\": (%s)",
           function, visname, style == NULL ? "default" : style);

      snprintf(emsg, emsgsize, "%s: auth_userokay failed: %s",
               function, strerror(errno));

      return -1;
   }

   return 0;
}

#endif /* HAVE_BSDAUTH */
