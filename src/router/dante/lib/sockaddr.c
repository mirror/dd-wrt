/*
 * Copyright (c) 2012, 2013, 2014, 2020
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

#if HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif /* HAVE_NET_IF_DL_H */

static const char rcsid[] =
"$Id: sockaddr.c,v 1.33.4.3.6.2 2020/11/11 16:11:54 karls Exp $";


int
sockaddrareeq(a, b, nocompare)
   const struct sockaddr_storage *a;
   const struct sockaddr_storage *b;
   const size_t nocompare;
{
   const char *function = "sockaddrareeq()";
   char astr[MAXSOCKADDRSTRING], bstr[MAXSOCKADDRSTRING];

   if (sockscf.option.debug)
      slog(LOG_DEBUG, "%s: comparing %s and %s",
           function,
           sockaddr2string(a, astr, sizeof(astr)),
           sockaddr2string(b, bstr, sizeof(bstr)));

   if (a->ss_family != b->ss_family)
      return 0;

#if HAVE_SOCKADDR_STORAGE_SS_LEN
   if (a->ss_len != b->ss_len)
      return 0;
#endif

   if (! (nocompare & ADDRINFO_PORT))
      if (GET_SOCKADDRPORT(a) != GET_SOCKADDRPORT(b))
         return 0;

   switch (a->ss_family) {
      case AF_INET:
         return memcmp(&TOCIN(a)->sin_addr,
                       &TOCIN(b)->sin_addr,
                       sizeof(TOCIN(a)->sin_addr)) == 0;

      case AF_INET6:
         if (! (nocompare & ADDRINFO_SCOPEID))
            if (TOCIN6(a)->sin6_scope_id != TOCIN6(b)->sin6_scope_id)
               return 0;

         if (TOCIN6(a)->sin6_flowinfo != TOCIN6(b)->sin6_flowinfo)
            return 0;

         return memcmp(&TOCIN6(a)->sin6_addr,
                       &TOCIN6(b)->sin6_addr,
                       sizeof(TOCIN6(a)->sin6_addr)) == 0;

      default:
         return memcmp(a, b, salen(a->ss_family)) == 0;
   }
}

void
sockaddrcpy(dst, src, dstlen)
   struct sockaddr_storage *dst;
   const struct sockaddr_storage *src;
   const size_t dstlen;
{
   const char *function = "sockaddrcpy()";
   const socklen_t srclen  = salen(src->ss_family),
                   copylen = MIN(dstlen, srclen);

   if (srclen > copylen)
      swarnx("%s: truncating address %s (af: %lu): %lu/%lu bytes available",
             function,
             sockaddr2string(src, NULL, 0),
             (unsigned long)src->ss_family,
             (unsigned long)dstlen,
             (unsigned long)srclen);
   else {
      if (dstlen > copylen) /* zero any unused bytes left in dst. */
         bzero((char *)dst + copylen, dstlen - copylen);
   }

   memcpy(dst, src, copylen);

}

void
usrsockaddrcpy(dst, src, dstlen)
   struct sockaddr_storage *dst;
   const struct sockaddr_storage *src;
   const size_t dstlen;
{
   const char *function    = "usrsockaddrcpy()";
   const socklen_t srclen  = salen(src->ss_family),
                   copylen = MIN(dstlen, srclen);

   if (copylen < srclen)
      swarnx("%s: truncating address %s (af: %lu): %lu/%lu bytes available",
             function,
             sockaddr2string(src, NULL, 0),
             (unsigned long)src->ss_family,
             (unsigned long)dstlen,
             (unsigned long)srclen);

   if (copylen < dstlen)
      bzero((char *)dst + copylen, dstlen - copylen); /* clear unused bytes. */

   memcpy(dst, src, copylen);

#if HAVE_SOCKADDR_STORAGE_SS_LEN
   dst->ss_len = copylen; /* ensure ss_len is set */
#endif /* HAVE_SOCKADDR_STORAGE_SS_LEN */
}

sa_len_type
salen(family)
   const sa_family_t family;
{
/*   const char *function = "salen()"; */

   switch (family) {
      case AF_INET:
         return (sa_len_type)sizeof(struct sockaddr_in);

#ifdef AF_INET6
      case AF_INET6:
         return (sa_len_type)sizeof(struct sockaddr_in6);
#endif /* AF_INET6 */

#ifdef AF_LINK
      case AF_LINK:
         return (sa_len_type)sizeof(struct sockaddr_dl);
#endif /* AF_LINK */

#ifdef AF_UNSPEC
      case AF_UNSPEC:
         return (sa_len_type)sizeof(struct sockaddr); /* or? */
#endif /* AF_UNSPEC */

   }

#if !SOCKS_CLIENT
   SWARNX(family);
#endif /* !SOCKS_CLIENT */

   return (sa_len_type)sizeof(struct sockaddr);
}

sa_len_type
inaddrlen(family)
   const sa_family_t family;
{
/*   const char *function = "inaddrlen()"; */

   switch (family) {
      case AF_INET:
         return (sa_len_type)sizeof(struct in_addr);

#ifdef AF_INET6
      case AF_INET6:
         return (sa_len_type)sizeof(struct in6_addr);
#endif /* AF_INET6 */
   }

   SWARNX(family);

   return 0;
}


int
safamily_issupported(family)
   const sa_family_t family;
{
/*   const char *function = "safamily_issupported()"; */

   switch (family) {
      case AF_INET:
      case AF_INET6:
         return 1;

      default:
         return 0;
   }

   /* NOTREACHED */
}

sa_family_t
atype2safamily(atype)
   const int atype;
{

   switch (atype) {
      case SOCKS_ADDR_IPV4:
         return AF_INET;

      case SOCKS_ADDR_IPV6:
         return AF_INET6;
   }

   SERRX(atype);
}

int
safamily2atype(safamily)
   const sa_family_t safamily;
{

   switch (safamily) {
      case AF_INET:
         return SOCKS_ADDR_IPV4;

      case AF_INET6:
         return SOCKS_ADDR_IPV6;
   }

   SERRX(safamily);
}
