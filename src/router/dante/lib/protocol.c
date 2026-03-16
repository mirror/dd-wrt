/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2010, 2011, 2012,
 *               2013, 2019
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
"$Id: protocol.c,v 1.88.10.2 2020/11/11 16:11:54 karls Exp $";

unsigned char *
sockshost2mem(host, mem, version)
   const sockshost_t *host;
   unsigned char *mem;
   int version;
{

   switch (version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V4REPLY_VERSION:
         SASSERTX(host->atype == SOCKS_ADDR_IPV4);

         /* DSTPORT */
         memcpy(mem, &host->port, sizeof(host->port));
         mem += sizeof(host->port);

         /* DSTIP */
         memcpy(mem, &host->addr.ipv4, sizeof(host->addr.ipv4));
         mem += sizeof(host->addr.ipv4);

         break;

      case PROXY_SOCKS_V5:
         /* ATYP */
         memcpy(mem, &host->atype, sizeof(host->atype));
         mem += sizeof(host->atype);

         switch (host->atype) {
            case SOCKS_ADDR_IPV4:
               memcpy(mem, &host->addr.ipv4.s_addr,
               sizeof(host->addr.ipv4.s_addr));
               mem += sizeof(host->addr.ipv4.s_addr);
               break;

            case SOCKS_ADDR_IPV6:
               memcpy(mem, &host->addr.ipv6.ip, sizeof(host->addr.ipv6.ip));
               mem += sizeof(host->addr.ipv6.ip);
               break;

            case SOCKS_ADDR_DOMAIN:
               /* first byte gives length of rest. */
               *mem = (unsigned char)strlen(host->addr.domain);

               memcpy(mem + 1, host->addr.domain, (size_t)*mem);
               mem += *mem + 1;
               break;

            default:
               SERRX(host->atype);
         }

         /* DST.PORT */
         memcpy(mem, &host->port, sizeof(host->port));
         mem += sizeof(host->port);

         break;

      default:
         SERRX(version);
   }

   return mem;
}

const unsigned char *
mem2sockshost(host, mem, len, version)
   sockshost_t *host;
   const unsigned char *mem;
   size_t len;
   int version;
{
   const char *function = "mem2sockshost()";

   switch (version) {
      case PROXY_SOCKS_V5:
         if (len < MINSOCKSHOSTLEN)
            return NULL;

         if (len < sizeof(host->atype))
            return NULL;

         memcpy(&host->atype, mem, sizeof(host->atype));
         mem += sizeof(host->atype);
         len -= sizeof(host->atype);

         switch (host->atype) {
            case SOCKS_ADDR_IPV4:
               if (len < sizeof(host->addr.ipv4))
                  return NULL;

               memcpy(&host->addr.ipv4, mem, sizeof(host->addr.ipv4));
               mem += sizeof(host->addr.ipv4);
               len -= sizeof(host->addr.ipv4);
               break;

            case SOCKS_ADDR_DOMAIN: {
               size_t domainlen = (size_t)*mem;

               mem += sizeof(*mem);

               OCTETIFY(domainlen);

               if (len < domainlen + 1) /* +1 for NUL to be added. */
                  return NULL;

               SASSERTX(domainlen < sizeof(host->addr.domain));

               memcpy(host->addr.domain, mem, domainlen);
               host->addr.domain[domainlen] = NUL;
               mem += domainlen;
               len -= domainlen + 1; /* +1 for added NUL. */
               break;
            }

            case SOCKS_ADDR_IPV6:
               if (len < sizeof(host->addr.ipv6.ip))
                  return NULL;

               memcpy(&host->addr.ipv6.ip, mem, sizeof(host->addr.ipv6.ip));
               mem += sizeof(host->addr.ipv6.ip);
               len -= sizeof(host->addr.ipv6.ip);

               host->addr.ipv6.scopeid = 0;

               break;

            default:
               slog(LOG_NEGOTIATE, "%s: unknown atype value: %d",
                    function, host->atype);
               return NULL;
         }

         if (len < sizeof(host->port))
            return NULL;

         memcpy(&host->port, mem, sizeof(host->port));
         mem += sizeof(host->port);
         len -= sizeof(host->port);

         break;

      default:
         SERRX(version);
   }

   return mem;
}

void
socks_set_responsevalue(response, value)
    response_t *response;
    unsigned int value;
{

    switch (response->version) {
      case PROXY_SOCKS_V4REPLY_VERSION:
      case PROXY_SOCKS_V5:
         response->reply.socks = (unsigned char)value;
         break;

      case PROXY_UPNP:
         response->reply.upnp = (unsigned char)value;
         break;

      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         response->reply.http = (unsigned short)value;
         break;

      default:
         SERRX(response->version);
   }
}

unsigned int
socks_get_responsevalue(response)
    const response_t *response;
{

    switch (response->version) {
      case PROXY_SOCKS_V4REPLY_VERSION:
      case PROXY_SOCKS_V5:
         return response->reply.socks;

      case PROXY_UPNP:
         return response->reply.upnp;

      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         return response->reply.http;

      default:
         SERRX(response->version);
   }

   /* NOTREACHED */
}

int
proxyprotocolisknown(version)
   const int version;
{

   switch (version) {
      /* don't include PROXY_SOCKS_V4REPLY_VERSION.  Stupid thing set to 0. */
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V5:
      case PROXY_UPNP:
      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         return 1;

      default:
         return 0;
   }
}

int
authmethodisknown(method)
   const int method;
{

   switch (method) {
      case AUTHMETHOD_NOTSET:
      case AUTHMETHOD_NONE:
      case AUTHMETHOD_GSSAPI:
      case AUTHMETHOD_UNAME:
      case AUTHMETHOD_NOACCEPT:
      case AUTHMETHOD_RFC931:
      case AUTHMETHOD_PAM_ANY:
      case AUTHMETHOD_PAM_ADDRESS:
      case AUTHMETHOD_PAM_USERNAME:
      case AUTHMETHOD_BSDAUTH:
#if HAVE_LDAP
      case AUTHMETHOD_LDAPAUTH:
#endif /* HAVE_LDAP */
         return 1;

      default:
         return 0;
   }
}

int *
charmethod2intmethod(methodc, charmethodv, intmethodv)
   const size_t methodc;
   const unsigned char charmethodv[];
   int intmethodv[];
{
   size_t i;

   for (i = 0; i < methodc; ++i)
      intmethodv[i] = (int)charmethodv[i];

   return intmethodv;
}
