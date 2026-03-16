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
 *
 */

#include "common.h"

static const char rcsid[] =
"$Id: ipv6.c,v 1.6.4.5 2014/08/15 18:16:41 karls Exp $";

int
socks_inet_pton(af, src, dst, dstscope)
   const int af;
   const void *src;
   void *dst;
   uint32_t *dstscope;
{
   const char *function = "socks_inet_pton()";
   dnsinfo_t resmem;
   struct addrinfo *res, hints;
   char visbuf[1024];
   int rc;

   if (strchr(src, '%') == NULL) /* no scope ids, can use inet_pton(). */
      return inet_pton(af, src, dst);

   bzero(&hints, sizeof(hints));
   hints.ai_flags  = AI_NUMERICHOST;
   hints.ai_family = af;

   if ((rc = cgetaddrinfo(src, NULL, &hints, &res, &resmem)) == 0) {
      SASSERTX(res->ai_addr != NULL);

      memcpy(dst, GET_SOCKADDRADDR(TOSS(res->ai_addr)), res->ai_addrlen);

      switch (af) {
         case AF_INET:
            break;

         case AF_INET6:
            if (dstscope != NULL)
               *dstscope = TOIN6(res->ai_addr)->sin6_scope_id;

            break;

         default:
            SERRX(af);
      }

      return 1;
   }

   slog(LOG_DEBUG, "%s: getaddrinfo(3) on %s failed: %s",
        function,
        str2vis(src, strlen(src), visbuf, sizeof(visbuf)),
        gai_strerror(rc));

   if (rc == EAI_FAMILY) {
      errno = EAFNOSUPPORT;
      return -1;
   }

   return 0;
}

void
set_hints_ai_family(ai_family)
   int *ai_family;
{
#if !SOCKS_CLIENT
   const char *function = "set_hints_ai_family";

   if (external_has_only_safamily(AF_INET))
      *ai_family = AF_INET;
   else if (external_has_only_safamily(AF_INET6)
   &&       external_has_global_safamily(AF_INET6))
      *ai_family = AF_INET6;
   else
      /*
       * else; have both ipv4 and ipv6 available (or none), so don't care
       * what we are returned.
       */
      *ai_family  = 0;

   slog(LOG_DEBUG, "%s: ai_family = %d", function, *ai_family);
#endif /* !SOCKS_CLIENT */
}

#if !SOCKS_CLIENT
struct in_addr *
ipv4_mapped_to_regular(ipv4_mapped, ipv4_regular)
   const struct in6_addr *ipv4_mapped;
   struct in_addr  *ipv4_regular;
{
   const char *function = "ipv4_mapped_to_regular";

   memcpy(ipv4_regular,
          &ipv4_mapped->s6_addr[sizeof(*ipv4_mapped) - sizeof(*ipv4_regular)],
          sizeof(*ipv4_regular));

   if (sockscf.option.debug) {
      char mapped[MAX(INET6_ADDRSTRLEN, 32)], regular[MAX(INET_ADDRSTRLEN, 32)];

      if (inet_ntop(AF_INET6, ipv4_mapped, mapped, sizeof(mapped)) == NULL)
         STRCPY_ASSERTSIZE(mapped, "<nonsense address>");

      if (inet_ntop(AF_INET, ipv4_regular, regular, sizeof(regular)) == NULL)
         STRCPY_ASSERTSIZE(mapped, "<nonsense address>");

      slog(LOG_DEBUG, "converted from %s to %s", mapped, regular);
   }

   return ipv4_regular;
}

#endif /* !SOCKS_CLIENT */


#undef gai_strerror
const char *
socks_gai_strerror(errcode)
   int errcode;
{
   static char emsg[1024];

#if !SOCKS_CLIENT
   int isaflimit = 0, hasaf = 0;

   switch (errcode) {
      case EAI_FAMILY:
      case EAI_FAIL:
      case EAI_NONAME:
#ifdef EAI_NODATA
      case EAI_NODATA:
#endif
         if (external_has_only_safamily(AF_INET))  {
            isaflimit = 1;
            hasaf     = AF_INET;
         }
         else if (external_has_only_safamily(AF_INET6))  {
            isaflimit = 1;
            hasaf     = AF_INET6;
         }
         break;

      default:
         break;
   }

   snprintf(emsg, sizeof(emsg), "%s%s%s",
            gai_strerror(errcode),
            isaflimit ? " for " : "",
            isaflimit ? safamily2string(hasaf) : "");

#else /* SOCKS_CLIENT */

   snprintf(emsg, sizeof(emsg), "%s", gai_strerror(errcode));

#endif  /* SOCKS_CLIENT */


   return emsg;
}
