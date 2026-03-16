/*
 * Copyright (c) 2013, 2014
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
 */

#include "common.h"

static const char rcsid[] =
"$Id: fmt_customer.c,v 1.13.4.1 2014/08/15 18:16:40 karls Exp $";

/*
 * Nothing here should be changed without regression-testing against
 * the appropriate customer-test.
 */

void
log_connectfailed(side, dststr)
   const interfaceside_t side;
   const char *dststr;
{
   const char *function = "log_connectfailed()";
   const int ll = loglevel_errno(errno, side);

   if (ERRNOISNOROUTE(errno))
      slog(ll, "no route to %s: %s", dststr, strerror(errno));
   else if (errno == EINPROGRESS)
      slog(ll, "connect to host %s is now in progress", dststr);
   else
      slog(ll, "connect to host %s failed: %s", dststr, strerror(errno));
}

void
log_writefailed(side, s, dst)
   const interfaceside_t side;
   const int s;
   const struct sockaddr_storage *dst;
{
   const int ll = loglevel_errno(errno, side);
   const int errno_s = errno;
   char dststr[MAXSOCKADDRSTRING];

   if (dst != NULL)
      sockaddr2string(dst, dststr, sizeof(dststr));
   else {
      struct sockaddr_storage p;
      socklen_t len;

      len = sizeof(p);
      if (getpeername(s, TOSA(&p), &len) == -1)
         snprintf(dststr, sizeof(dststr), "N/A");
      else
         sockaddr2string(&p, dststr, sizeof(dststr));
   }

   errno = errno_s;

   if (ERRNOISNOROUTE(errno))
      slog(ll, "no route to %s: %s", dststr, strerror(errno));
   else
      slog(ll, "send to host %s failed: %s", dststr, strerror(errno));
}

void
log_resolvefailed(hostname, side, gaierr)
   const char *hostname;
   const interfaceside_t side;
   const int gaierr;
{
   const int ll = loglevel_gaierr(gaierr, side);
   char visbuf[MAXHOSTNAMELEN * 4];

   slog(ll, "could not DNS-resolve \"%s\": %s",
        str2vis(hostname, strlen(hostname), visbuf, sizeof(visbuf)),
        gaierr == EAI_SYSTEM ? strerror(errno) : gai_strerror(gaierr));
}

void
log_reversemapfailed(addr, side, gaierr)
   const struct sockaddr_storage *addr;
   const interfaceside_t side;
   const int gaierr;
{
   const int ll = loglevel_gaierr(gaierr, side);
   char addrstring[256];

   switch (socks_inet_pton(addr->ss_family,
                           GET_SOCKADDRADDR(addr),
                           addrstring,
                           NULL)) {
      case 1:
         break;

      case 0:
         STRCPY_ASSERTSIZE(addrstring, "<nonsense address>");
         break;

      case -1:
      default:
         strncpy(addrstring, strerror(errno), sizeof(addrstring) - 1);
         addrstring[sizeof(addrstring) - 1] = NUL;
         break;
   }

   slog(ll, "could not DNS reversemap address %s: %s",
        addrstring,
        gaierr == EAI_SYSTEM ? strerror(errno) : gai_strerror(gaierr));
}

#if !SOCKS_CLIENT

void
log_clientdropped(client)
   const struct sockaddr_storage *client;
{

   slog(LOG_WARNING, "new client from %s dropped: no resources",
        sockaddr2string(client, NULL, 0));
}

void
log_addchild_failed(void)
{

   slog(LOG_WARNING, "failed to add a new child to handle new clients");
}

#endif /* !SOCKS_CLIENT */
