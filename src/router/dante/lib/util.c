/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008,
 *               2009, 2010, 2011, 2012, 2013, 2014, 2019, 2020
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

#include "vis_compat.h"

static const char rcsid[] =
"$Id: util.c,v 1.416.4.5.6.5.4.1 2024/11/21 10:22:43 michaels Exp $";

const char *
strcheck(string)
   const char *string;
{
   return string == NULL ? NOMEM : string;
}

unsigned int
sockscode(version, code)
   const int version;
   const int code;
{

   SASSERTX(code >= 0);

   switch (version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V4REPLY_VERSION:
         switch (code) {
            case SOCKS_SUCCESS:
               return SOCKSV4_SUCCESS;

            default:
               return SOCKSV4_FAIL; /* v4 is not very specific. */
         }
         /* NOTREACHED */

      case PROXY_SOCKS_V5:
         return (unsigned char)code; /* current codes are all V5. */

      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         switch (code) {
            case SOCKS_SUCCESS:
               return HTTP_SUCCESS;

            case SOCKS_FAILURE:
               return HTTP_FAILURE;

            case SOCKS_NOTALLOWED:
               return HTTP_NOTALLOWED;

            case SOCKS_NETUNREACH:
            case SOCKS_HOSTUNREACH:
            case SOCKS_CONNREFUSED:
               return HTTP_HOSTUNREACH;

            default:
               return HTTP_FAILURE;
         }
         /* NOTREACHED */

      case PROXY_UPNP:
         switch (code) {
            case SOCKS_SUCCESS:
               return UPNP_SUCCESS;

            case SOCKS_FAILURE:
               return UPNP_FAILURE;

            default:
               return UPNP_FAILURE;
         }
         /* NOTREACHED */


      default:
         SERRX(version);
   }

   /* NOTREACHED */
}

unsigned int
errno2reply(errnum, version)
   int errnum;
   int version;
{

   switch (errnum) {
      case ENETDOWN:
      case ENETUNREACH:
         return sockscode(version, SOCKS_NETUNREACH);

      case EHOSTDOWN:
      case EHOSTUNREACH:
         return sockscode(version, SOCKS_HOSTUNREACH);

      case ECONNREFUSED:
      case ECONNRESET:
         return sockscode(version, SOCKS_CONNREFUSED);

      case ETIMEDOUT:
         return sockscode(version, SOCKS_TTLEXPIRED);
   }

   return sockscode(version, SOCKS_FAILURE);
}

struct sockaddr_storage *
int_sockshost2sockaddr2(host, addr, addrlen, gaierr, emsg, emsglen)
   const sockshost_t *host;
   struct sockaddr_storage *addr;
   size_t addrlen;
   int *gaierr;
   char *emsg;
   size_t emsglen;
{
   const char *function = "int_sockshost2sockaddr2()";
   char emsgmem[1024 + MAXHOSTNAMELEN * 4];

   if (emsg == NULL || emsglen == 0) {
      emsg    = emsgmem;
      emsglen = sizeof(emsgmem);
   }

   *emsg   = NUL;
   *gaierr = 0;

   if (addr == NULL) {
      static struct sockaddr_storage addrmem;

      addr    = &addrmem;
      addrlen = sizeof(addrmem);
   }

   bzero(addr, addrlen);

   switch (host->atype) {
      case SOCKS_ADDR_IPV4:
      case SOCKS_ADDR_IPV6: {
         struct sockaddr_storage ss;

         bzero(&ss, sizeof(ss));

         if (host->atype == SOCKS_ADDR_IPV4) {
            SET_SOCKADDR(&ss, AF_INET);
            TOIN(&ss)->sin_addr = host->addr.ipv4;
         }
         else {
            SET_SOCKADDR(&ss, AF_INET6);
            TOIN6(&ss)->sin6_addr     = host->addr.ipv6.ip;
            TOIN6(&ss)->sin6_scope_id = host->addr.ipv6.scopeid;
         }

         sockaddrcpy(addr, &ss, salen(ss.ss_family));
         SET_SOCKADDRPORT(addr, host->port);
         break;
      }

      case SOCKS_ADDR_DOMAIN: {
         struct addrinfo hints, *res;
         dnsinfo_t resmem;

         bzero(&hints, sizeof(hints));

         set_hints_ai_family(&hints.ai_family);

         *gaierr = cgetaddrinfo(host->addr.domain, NULL, &hints, &res, &resmem);

         if (*gaierr == 0) {
            if (res->ai_addrlen <= addrlen) {
               sockaddrcpy(addr, TOSS(res->ai_addr), addrlen);
               SET_SOCKADDRPORT(addr, host->port);
            }
            else {
               snprintf(emsg, emsglen,
                        "strange dns reply.  res->ai_addrlen (%lu) > "
                        "addrlen (%lu)",
                        (unsigned long)res->ai_addrlen,
                        (unsigned long)addrlen);

               swarnx("%s: %s", function, emsg);
               addr->ss_family = AF_UNSPEC;
            }
         }
         else {
            char visbuf[MAXHOSTNAMELEN * 4];

            snprintf(emsg, emsglen,
                     "could not resolve hostname \"%s\": %s",
                     str2vis(host->addr.domain,
                             strlen(host->addr.domain),
                             visbuf,
                             sizeof(visbuf)),
                     gai_strerror(*gaierr));

            slog(LOG_NEGOTIATE, "%s: %s", function, emsg);
            addr->ss_family = AF_UNSPEC;
         }

         break;
      }

      case SOCKS_ADDR_IFNAME: {
         struct sockaddr_storage a, m;

         if (ifname2sockaddr(host->addr.ifname, 0, &a, &m) != NULL) {
            sockaddrcpy(addr, &a, addrlen);
            SET_SOCKADDRPORT(addr, host->port);
         }
         else {
            snprintf(emsg, emsglen,
                     "could not resolve %s to IP-address",
                     sockshost2string2(host, ADDRINFO_ATYPE, NULL, 0));

            slog(LOG_NEGOTIATE, "%s: %s", function, emsg);

            addr->ss_family = AF_UNSPEC;
         }

         break;
      }

      case SOCKS_ADDR_URL:
         urlstring2sockaddr(host->addr.urlname, addr, gaierr, emsg, emsglen);
         break;

      default:
         SERRX(host->atype);
   }

   SASSERTX(addr->ss_family == AF_UNSPEC
   ||       addr->ss_family == AF_INET
   ||       addr->ss_family == AF_INET6);

   return addr;
}

struct sockaddr_storage *
int_sockshost2sockaddr(host, addr, addrlen)
   const sockshost_t *host;
   struct sockaddr_storage *addr;
   size_t addrlen;
{
   int p;

   return int_sockshost2sockaddr2(host, addr, addrlen, &p, NULL, 0);
}


sockshost_t *
sockaddr2sockshost(addr, host)
   const struct sockaddr_storage *addr;
   sockshost_t *host;
{

   if (host == NULL) {
      static sockshost_t _host;

      host = &_host;
   }

   switch (addr->ss_family) {
      case AF_INET:
         host->atype     = SOCKS_ADDR_IPV4;
         host->addr.ipv4 = TOCIN(addr)->sin_addr;
         host->port      = TOCIN(addr)->sin_port;
         break;

      case AF_INET6:
         host->atype             = SOCKS_ADDR_IPV6;
         host->addr.ipv6.ip      = TOCIN6(addr)->sin6_addr;
         host->addr.ipv6.scopeid = TOCIN6(addr)->sin6_scope_id;
         host->port              = TOCIN6(addr)->sin6_port;
         break;

      default:
         SERRX(addr->ss_family);
   }

   return host;
}

int
sockaddr2hostname(sa, hostname, hostnamelen)
   const struct sockaddr_storage *sa;
   char *hostname;
   const size_t hostnamelen;
{
   const char *function = "sockaddr2hostname()";
   char vbuf[MAXHOSTNAMELEN * 4];
   int rc;

   rc = getnameinfo(TOCSA(sa),
                    salen(sa->ss_family),
                    hostname,
                    (socklen_t)hostnamelen,
                    NULL,
                    0,
                    NI_NAMEREQD);

   if (rc != 0) {
      slog(LOG_DEBUG, "%s: getnameinfo(%s) failed: %s",
           function,
           sockaddr2string2(sa, 0, NULL, 0),
           gai_strerror(rc));

      return rc;
   }

   slog(LOG_DEBUG, "%s: %s resolved to \"%s\"",
        function,
        sockaddr2string2(sa, 0, NULL, 0),
        str2vis(hostname, strlen(hostname), vbuf, sizeof(vbuf)));

   return rc;
}


struct sockaddr_storage *
int_ruleaddr2sockaddr2(address, sa, len, protocol, gaierr, emsg, emsglen)
   const ruleaddr_t *address;
   struct sockaddr_storage *sa;
   size_t len;
   const int protocol;
   int *gaierr;
   char *emsg;
   const size_t emsglen;
{
   sockshost_t host;

   if (sa == NULL) {
      static struct sockaddr_storage samem;

      sa  = &samem;
      len = sizeof(samem);
   }

   ruleaddr2sockshost(address, &host, protocol);
   return int_sockshost2sockaddr2(&host, sa, len, gaierr, emsg, emsglen);
}

struct sockaddr_storage *
int_ruleaddr2sockaddr(address, sa, len, protocol)
   const ruleaddr_t *address;
   struct sockaddr_storage *sa;
   size_t len;
   const int protocol;
{
   int gaierr;

   return int_ruleaddr2sockaddr2(address, sa, len, protocol, &gaierr, NULL, 0);
}

sockshost_t *
ruleaddr2sockshost(address, host, protocol)
   const ruleaddr_t *address;
   sockshost_t *host;
   int protocol;
{
   const char *function = "ruleaddr2sockshost()";

   if (host == NULL) {
      static sockshost_t hostmem;

      host = &hostmem;
   }

   switch (host->atype = address->atype) {
      case SOCKS_ADDR_IPV4:
         host->addr.ipv4 = address->addr.ipv4.ip;
         break;

      case SOCKS_ADDR_IPV6:
         host->addr.ipv6.ip      = address->addr.ipv6.ip;
         host->addr.ipv6.scopeid = address->addr.ipv6.scopeid;
         break;

      case SOCKS_ADDR_DOMAIN:
         STRCPY_ASSERTSIZE(host->addr.domain, address->addr.domain);
         break;

      case SOCKS_ADDR_IFNAME: {
         struct sockaddr_storage addr, p;

         if (ifname2sockaddr(address->addr.ifname, 0, &addr, &p) == NULL){
            swarnx("%s: cannot find interface named %s with IP configured.  "
                   "Using address %d instead",
                   function, address->addr.ifname, INADDR_ANY);

            host->atype            = SOCKS_ADDR_IPV4;
            host->addr.ipv4.s_addr = htonl(INADDR_ANY);
         }
         else {
            switch (addr.ss_family) {
               case AF_INET:
                  host->addr.ipv4 = TOIN(&addr)->sin_addr;
                  break;

               case AF_INET6:
                  host->addr.ipv6.ip      = TOIN6(&addr)->sin6_addr;
                  host->addr.ipv6.scopeid = TOIN6(&addr)->sin6_scope_id;
                  break;

               default:
                  SERRX(addr.ss_family);

            }

            host->atype = (unsigned char)safamily2atype(addr.ss_family);
         }

         break;
      }

      default:
         SERRX(address->atype);
   }

   switch (protocol) {
      case SOCKS_TCP:
         host->port = address->port.tcp;
         break;

      case SOCKS_UDP:
         host->port = address->port.udp;
         break;

      default:
         SERRX(protocol);
   }

   return host;
}

ruleaddr_t *
sockshost2ruleaddr(host, addr)
   const sockshost_t *host;
   ruleaddr_t *addr;
{

   if (addr == NULL) {
      static ruleaddr_t addrmem;

      addr = &addrmem;
   }

   switch (addr->atype = host->atype) {
      case SOCKS_ADDR_IPV4:
         addr->addr.ipv4.ip            = host->addr.ipv4;
         addr->addr.ipv4.mask.s_addr   = htonl(IPV4_FULLNETMASK);
         break;

      case SOCKS_ADDR_IPV6:
         addr->addr.ipv6.ip        = host->addr.ipv6.ip;
         addr->addr.ipv6.maskbits  = IPV6_NETMASKBITS;
         addr->addr.ipv6.scopeid   = host->addr.ipv6.scopeid;
         break;

      case SOCKS_ADDR_DOMAIN:
         STRCPY_ASSERTSIZE(addr->addr.domain, host->addr.domain);
         break;

      default:
         SERRX(host->atype);
   }


   if (host->port == htons(0)) {
      addr->operator   = none;
      addr->port.tcp   = addr->port.udp = addr->portend = htons(0);
   }
   else {
      addr->operator  = eq;
      addr->port.tcp  = host->port;
      addr->port.udp  = host->port;
      addr->portend   = host->port;
   }

   return addr;
}

ruleaddr_t *
sockaddr2ruleaddr(addr, ruleaddr)
   const struct sockaddr_storage *addr;
   ruleaddr_t *ruleaddr;
{
   sockshost_t host;

   sockaddr2sockshost(addr, &host);
   sockshost2ruleaddr(&host, ruleaddr);

   return ruleaddr;
}

struct sockaddr_storage *
int_hostname2sockaddr(name, index, addr, addrlen)
   const char *name;
   size_t index;
   struct sockaddr_storage *addr;
   size_t addrlen;
{
   int rc;

   return int_hostname2sockaddr2(name, index, addr, addrlen, &rc, NULL, 0);
}


struct sockaddr_storage *
int_hostname2sockaddr2(name, index, addr, addrlen, gaierr, emsg, emsglen)
   const char *name;
   size_t index;
   struct sockaddr_storage *addr;
   size_t addrlen;
   int *gaierr;
   char *emsg;
   size_t emsglen;
{
   const char *function = "int_hostname2sockaddr()";
   struct addrinfo *ai, hints;
   dnsinfo_t aimem;
   size_t i;
   char emsgmem[1024 + MAXHOSTNAMELEN * 4];

   if (emsg == NULL || emsglen == 0) {
      emsg    = emsgmem;
      emsglen = sizeof(emsgmem);
   }

   *emsg   = NUL;
   *gaierr = 0;

   bzero(addr, addrlen);
   SET_SOCKADDR(addr, AF_UNSPEC);

   bzero(&hints, sizeof(hints));
   if ((*gaierr = cgetaddrinfo(name, NULL, &hints, &ai, &aimem)) != 0) {
      char visbuf[MAXHOSTNAMELEN * 4];

      snprintf(emsg, emsglen, "could not resolve hostname \"%s\": %s",
               str2vis(name, strlen(name), visbuf, sizeof(visbuf)),
               gai_strerror(*gaierr));

      slog(LOG_DEBUG, "%s: could not resolve hostname \"%s\": %s",
           function, visbuf, gai_strerror(*gaierr));

      return NULL;
   }

   i = 0;
   do {
      SASSERTX(ai->ai_addr != NULL);

      if (i == index) {
         sockaddrcpy(addr, TOSS(ai->ai_addr), addrlen);
         return addr;
      }

      ++i;
      ai = ai->ai_next;
   } while (ai != NULL);

   return NULL;
}


struct sockaddr_storage *
int_ifname2sockaddr(ifname, index, addr, addrlen, mask, masklen)
   const char *ifname;
   size_t index;
   struct sockaddr_storage *addr;
   size_t addrlen;
   struct sockaddr_storage *mask;
   size_t masklen;
{
   const char *function = "int_ifname2sockaddr()";
   struct ifaddrs ifa, *ifap = &ifa, *iface;
   size_t i, realindex;
   int foundifname, foundaddr;

   if (getifaddrs(&ifap) != 0) {
      swarn("%s: getifaddrs() failed", function);
      return NULL;
   }

   for (iface = ifap, i = 0, realindex = 0, foundifname = foundaddr = 0;
   i <= index && iface != NULL;
   iface = iface->ifa_next, ++realindex) {
      if (strcmp(iface->ifa_name, ifname) != 0)
         continue;

      foundifname = 1;

      if (iface->ifa_addr == NULL) {
         slog(LOG_DEBUG,
              "%s: interface %s missing address on index %lu ... skipping",
              function, iface->ifa_name, (unsigned long)realindex);

         continue;
      }

      if (iface->ifa_netmask == NULL) {
         slog(LOG_DEBUG,
              "%s: interface %s missing netmask for address %s, skipping",
              function,
              iface->ifa_name,
              sockaddr2string(TOSS(iface->ifa_addr), NULL, 0));

         continue;
      }

      if (iface->ifa_addr->sa_family != AF_INET
      &&  iface->ifa_addr->sa_family != AF_INET6) {
         slog(LOG_DEBUG,
              "%s: interface %s has neither AF_INET nor AF_INET6 configured "
              "at index %lu, skipping",
              function, iface->ifa_name, (unsigned long)index);

         continue;
      }

      /*
       * this address-index looks usable.  Does it match the requested
       * index?
       */
      if (i != index) {
         ++i;        /* we only count usable indexes. */
         continue;
      }

      foundaddr = 1;

      sockaddrcpy(addr, TOSS(iface->ifa_addr), addrlen);

      if (mask != NULL)
         sockaddrcpy(mask, TOSS(iface->ifa_netmask), masklen);

      break;
   }

   freeifaddrs(ifap);

   if (!foundifname) {
      slog(LOG_DEBUG, "%s: no interface with the name \"%s\" found",
           function, ifname);

      return NULL;
   }

   if (!foundaddr) {
      if (index == 0) {
         char visbuf[MAXIFNAMELEN * 4];

         swarnx("%s: interface \"%s\" has no usable IP-addresses configured",
                function,
                str2vis(ifname, strlen(ifname), visbuf, sizeof(visbuf)));

      }

      return NULL;
   }

   return addr;
}

const char *
sockaddr2ifname(addr, ifname, iflen)
   struct sockaddr_storage *addr;
   char *ifname;
   size_t iflen;
{
   const char *function = "sockaddr2ifname()";
   struct ifaddrs ifa, *ifap = &ifa, *iface;
   size_t nocompare;

   if (ifname == NULL || iflen == 0) {
      static char ifname_mem[MAXIFNAMELEN];

      ifname = ifname_mem;
      iflen  = sizeof(ifname_mem);
   }

   /*
    * port is irrelevant as far as an interface-address is concerned,
    * so make a copy of the address and zero it before we start
    * comparing.
    */
   nocompare = ADDRINFO_PORT;

   if (addr->ss_family == AF_INET6
   &&  TOIN6(addr)->sin6_scope_id == 0)
      /*
       * no particular scope requested, match all.
       */
      nocompare |= ADDRINFO_SCOPEID;

   if (getifaddrs(&ifap) != 0)
      return NULL;

   for (iface = ifap; iface != NULL; iface = iface->ifa_next) {
      if (iface->ifa_addr != NULL
      &&  sockaddrareeq(TOSS(iface->ifa_addr), addr, nocompare)) {
         strncpy(ifname, iface->ifa_name, iflen - 1);
         ifname[iflen - 1] = NUL;

         slog(LOG_DEBUG, "%s: address %s belongs to interface %s (af: %s)",
              function,
              sockaddr2string(addr, NULL, 0),
              ifname,
              safamily2string(iface->ifa_addr->sa_family));

         freeifaddrs(ifap);
         return ifname;
      }
      else
         slog(LOG_DEBUG,
              "%s: address %s does not belong to interface %s (af: %s)",
              function,
              sockaddr2string(addr, NULL, 0),
              iface->ifa_name,
              iface->ifa_addr == NULL ?
                  "<no address>" : safamily2string(iface->ifa_addr->sa_family));
   }

   freeifaddrs(ifap);
   return NULL;
}

int
sockshostareeq(a, b)
   const sockshost_t *a;
   const sockshost_t *b;
{

   if (a->atype != b->atype)
      return 0;

   if (a->port != b->port)
      return 0;

   switch (a->atype) {
      case SOCKS_ADDR_IPV4:
         if (memcmp(&a->addr.ipv4, &b->addr.ipv4, sizeof(a->addr.ipv4)) != 0)
            return 0;
         break;

      case SOCKS_ADDR_IPV6:
         if (memcmp(&a->addr.ipv6, &b->addr.ipv6, sizeof(a->addr.ipv6)) != 0)
            return 0;
         break;

      case SOCKS_ADDR_DOMAIN:
         if (strcmp(a->addr.domain, b->addr.domain) != 0)
            return 0;
         break;

      default:
         SERRX(a->atype);
   }

   return 1;
}

int
fdsetop(highestfd, op, a, b, result)
   int highestfd;
   int op;
   const fd_set *a;
   const fd_set *b;
   fd_set *result;
{
   int i, bits;

   bits = -1;
   switch (op) {
      case '&':
         FD_ZERO(result);
         for (i = 0; i <= highestfd; ++i)
            if (FD_ISSET(i, a) && FD_ISSET(i, b)) {
               FD_SET(i, result);
               bits = MAX(i, bits);
            }

         break;

      case '^':
         FD_ZERO(result);
         for (i = 0; i <= highestfd; ++i)
            if (FD_ISSET(i, a) != FD_ISSET(i, b)) {
               FD_SET(i, result);
               bits = MAX(i, bits);
            }
            else
               FD_CLR(i, result);

         break;

      case '|':
         /*
          * no FD_ZERO() required.  Allows caller to call us without using
          * a temporary object for result if he wants to do result = a | b.
          */
         for (i = 0; i <= highestfd; ++i)
            if (FD_ISSET(i, a) || FD_ISSET(i, b)) {
               FD_SET(i, result);
               bits = MAX(i, bits);
            }
         break;

      default:
         SERRX(op);
   }

   return bits;
}

int
methodisset(method, methodv, methodc)
   int method;
   const int *methodv;
   size_t methodc;
{
   const char *function = "methodisset()";
   size_t i;

   if (sockscf.option.debug)
      slog(LOG_DEBUG,
           "%s: checking if method %s is set in the list (%lu) \"%s\"",
           function,
           method2string(method),
           (unsigned long)methodc,
           methods2string(methodc, methodv, NULL, 0));

   for (i = 0; i < methodc; ++i)
      if (methodv[i] == method)
         return 1;

   return 0;
}

char *
str2vis(string, len, visstring, vislen)
   const char *string;
   size_t len;
   char *visstring;
   size_t vislen;
{
   const int visflag = VIS_SP | VIS_TAB | VIS_NL | VIS_CSTYLE | VIS_OCTAL;

   if (visstring == NULL) {
      SERRX(0); /* should never be used. */

      /* see vis(3) for "* 4" */
      if ((visstring = malloc((sizeof(*visstring) * len * 4) + 1)) == NULL)
         return NULL;

      vislen = len * 4 + 1;
   }

   len = MIN(len, (vislen / 4) - 1);
   strvisx(visstring, string, len, visflag);

   return visstring;
}

int
socks_mklock(template, newname, newnamelen)
   const char *template;
   char *newname;
   const size_t newnamelen;
{
   const char *function = "socks_mklock()";
   const char *prefix;
   static char newtemplate[PATH_MAX];
   size_t len;
   int s, flag;

   if ((prefix = socks_getenv(ENV_TMPDIR, dontcare)) != NULL)
      if (*prefix == NUL)
         prefix = NULL;

   if (prefix == NULL)
      prefix = "/tmp";

   len = strlen(prefix) + strlen("/") + strlen(template) + 1;
   if (len > sizeof(newtemplate))
      serr("%s: the combination of \"%s\" and \"%s\""
           "is longer than the system max path length of %lu",
           function, prefix, template, (unsigned long)sizeof(newtemplate));

   if (newnamelen != 0 && len > newnamelen)
      serr("%s: the combination of \"%s\" and \"%s\""
           "is longer than the passed maxlength length of %lu",
           function, prefix, template, (unsigned long)newnamelen);

   if (*prefix != NUL)
      snprintf(newtemplate, len, "%s/%s", prefix, template);
   else
      snprintf(newtemplate, len, "%s", template);

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: newtemplate = \"%s\", prefix = \"%s\" "
           "uid = %d, euid = %d, gid = %d, egid = %d",
           function, newtemplate, prefix,
           (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());

   if (strstr(newtemplate, "XXXXXX") != NULL) {
      const mode_t oldumask = umask(S_IWGRP | S_IWOTH);

      if ((s = mkstemp(newtemplate)) == -1)
         swarn("%s: mkstemp(%s) using euid/egid %d/%d failed",
               function, newtemplate, (int)geteuid(), (int)getegid());

      (void)umask(oldumask);

#if HAVE_SOLARIS_BUGS
      if (s == -1 && *newtemplate == NUL) {
          /*
           * Solaris 5.11 sometimes loses the template on failure. :-/
           */
         if (*prefix != NUL)
            snprintf(newtemplate, len, "%s/%s", prefix, template);
         else
            snprintf(newtemplate, len, "%s", template);
      }
#endif /* HAVE_SOLARIS_BUGS */
   }
   else {
      s = open(newtemplate, O_RDWR | O_CREAT | O_EXCL, 0600);
      swarn("%s: open(%s)", function, newtemplate);
   }

   if (s == -1) {
      if (*prefix == NUL) {
         slog(LOG_DEBUG, "%s: failed to create \"%s\" (%s) and TMPDIR is not "
                         "set.  Trying again with TMPDIR set to \"/tmp\"",
                         function, newtemplate, strerror(errno));

         if (setenv("TMPDIR", "/tmp", 1) != 0)
            serr("%s: could not setenv(\"TMPDIR\", \"/tmp\")", function);

         SASSERT(socks_getenv(ENV_TMPDIR, dontcare) != NULL);

         return socks_mklock(template, newname, newnamelen);
      }

      return -1;
   }
   else
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: created file %s", function, newtemplate);

   if (newnamelen == 0) {
      if (unlink(newtemplate) == -1) {
         swarn("%s: unlink(%s)", function, newtemplate);
         close(s);

         return -1;
      }
   }
   else
      strcpy(newname, newtemplate);

   if ((flag = fcntl(s, F_GETFD, 0))       == -1
   || fcntl(s, F_SETFD, flag | FD_CLOEXEC) == -1)
      swarn("%s: fcntl(F_GETFD/F_SETFD)", function);

   return s;
}

int
socks_lock(d, offset, len, exclusive, wait)
   const int d;
   const off_t offset;
   const off_t len;
   const int exclusive;
   const int wait;
{
   const char *function = "socks_lock()";
   struct flock lock;
   int rc;

/*   slog(LOG_DEBUG, "%s: %d", function, d);  */

   if (d == -1)
      return 0;

   lock.l_start  = offset;
   lock.l_len    = len;
   lock.l_whence = SEEK_SET;
   lock.l_type   = exclusive ? F_WRLCK : F_RDLCK;

#if DIAGNOSTIC && 0
{
   struct flock diaginfo = lock;

   if (d != sockscf.loglock && fcntl(d, F_GETLK, &diaginfo) != -1)
      if (diaginfo.l_type != F_UNLCK)
         slog(LOG_DEBUG, "%s: lock %d is currently held by pid %ld",
              function, d, (long)diaginfo.l_pid);
}
#endif /* DIAGNOSTIC && 0*/

   do
      rc = fcntl(d, wait ? F_SETLKW : F_SETLK, &lock);
   while (rc == -1 && wait && (ERRNOISTMP(errno) || errno == EACCES));

   if (rc == -1) {
      if (!sockscf.state.inited
      &&  sockscf.loglock == d
      &&  sockscf.loglock == 0) { /* have not yet inited lockfile. */
         sockscf.loglock = -1;
         return 0;
      }

      SASSERT(ERRNOISTMP(errno) || errno == EACCES);
      SASSERT(!wait);
   }

   return rc;
}

void
socks_unlock(d, offset, len)
   int d;
   const off_t offset;
   const off_t len;
{
/*   const char *function = "socks_unlock()";  */
   struct flock lock;

/*   slog(LOG_DEBUG, "%s: %d", function, d);  */

   if (d == -1)
      return;

   lock.l_start  = offset;
   lock.l_len    = len;
   lock.l_type   = F_UNLCK;
   lock.l_whence = SEEK_SET;

   if (fcntl(d, F_SETLK, &lock) == -1)
      SERR(errno);
}

int
fdisopen(fd)
   const int fd;
{
   const int errno_s = errno;
   const int rc = fcntl(fd, F_GETFD, 0);

   errno = errno_s;
   return rc != -1;
}

int
fdisblocking(fd)
   const int fd;
{
   const char *function = "fdisblocking()";
   int p;

   if ((p = fcntl(fd, F_GETFL, 0)) == -1) {
      swarn("%s: fcntl(F_GETFL)", function);
      return 1;
   }

   return !(p & O_NONBLOCK);
}

void
closev(ic, iv)
   size_t ic;
   int *iv;
{
   size_t i;

   for (i = 0; i < ic; ++i)
      if (iv[i] >= 0)
         if (close(iv[i]) != 0)
            SWARN(iv[i]);
}

/*
 * Posted by Kien Ha (Kien_Ha@Mitel.COM) in comp.lang.c once upon a
 * time.
*/
int
bitcount(number)
   unsigned long number;
{
   int bitsset;

   for (bitsset = 0; number > 0; number >>= 1)
      if (number & 1)
         ++bitsset;

   return bitsset;
}

int
bitcount_in6addr(in6addr)
   const struct in6_addr *in6addr;
{
   size_t i;
   int bitsset;

   for (i = 0, bitsset = 0; i < ELEMENTS(in6addr->s6_addr); ++i)
      bitsset += bitcount((unsigned long)in6addr->s6_addr[i]);

   return bitsset;
}


fd_set *
allocate_maxsize_fdset(void)
{
   const char *function = "allocate_maxsize_fdset()";
   fd_set *set;

#if SOCKS_CLIENT
   sockscf.state.maxopenfiles = getmaxofiles(hardlimit);
   if (sockscf.state.maxopenfiles == (rlim_t)RLIM_INFINITY)
      /*
       * In the client the softlimit can vary at any time, so this is not
       * 100%, but see no other practical solution at the moment.
       */
      sockscf.state.maxopenfiles = getmaxofiles(softlimit);
#endif /* !SOCKS_CLIENT */

   SASSERTX(sockscf.state.maxopenfiles < (rlim_t)RLIM_INFINITY);
   SASSERTX(sockscf.state.maxopenfiles > 0);

   if ((set = malloc(MAX(sizeof(fd_set), SOCKD_FD_SIZE()))) == NULL)
      serr("%s: malloc() of %lu bytes for fd_set failed",
           function, (unsigned long)MAX(sizeof(fd_set), SOCKD_FD_SIZE()));

#if DEBUG
   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: allocated %lu bytes",
           function, (unsigned long)SOCKD_FD_SIZE());
#endif /* DEBUG */

   return set;
}

rlim_t
getmaxofiles(limittype_t type)
{
   const char *function = "getmaxofiles()";
   struct rlimit rlimit;
   rlim_t limit;

   if (getrlimit(RLIMIT_OFILE, &rlimit) != 0)
         serr("%s: getrlimit(RLIMIT_OFILE)", function);

   if (type == softlimit)
      limit = rlimit.rlim_cur;
   else if (type == hardlimit)
#if HAVE_DARWIN /* documented os x bug.  What on earth are they thinking? */
      limit = MIN(rlimit.rlim_max, OPEN_MAX);
#else /* !HAVE_DARWIN */
      limit = rlimit.rlim_max;
#endif /* !HAVE_DARWIN */
   else
      SERRX(type);
      /* NOTREACHED */

#if !SOCKS_CLIENT && FD_SETSIZE_LIMITS_SELECT
   /*
    * we don't mess with the clients limits.  Not our business whether
    * select(2) will work or not if a fd has a index that would overflow
    * FD_SETSIZE.
    */
   if (limit >= FD_SETSIZE) {
      static int logged;

      if (!logged) {
         slog(LOG_INFO,
              "%s: max open file limit is %lu, but we need to shrink it "
              "down to below FD_SETSIZE (%lu) for select(2) to work",
              function, (unsigned long)limit, (unsigned long)FD_SETSIZE);

         logged = 1;
      }

      limit = FD_SETSIZE - 1;
   }
#endif /* !SOCKS_CLIENT && FD_SETSIZE_LIMITS_SELECT */

   if (type == softlimit && limit == (rlim_t)RLIM_INFINITY) {
      static int logged;
      const rlim_t reduced = 65356;

      if (!logged) {
         slog(LOG_INFO,
              "%s: maxopenfiles is RLIM_INFINITY (%lu), reducing to %lu",
              function, (unsigned long)RLIM_INFINITY, (unsigned long)reduced);

         logged = 1;
      }

      limit = reduced;
   }

   return limit;
}

void
socks_sigblock(sig, oldset)
   const int sig;
   sigset_t *oldset;
{
   const char *function = "socks_sigblock()";
   sigset_t newmask;

   if (sig == -1)
      (void)sigfillset(&newmask);
   else {
      (void)sigemptyset(&newmask);
      (void)sigaddset(&newmask, sig);
   }

   if (sigprocmask(SIG_BLOCK, &newmask, oldset) != 0)
      swarn("%s: sigprocmask()", function);
}

void
socks_sigunblock(oldset)
   const sigset_t *oldset;
{
   const char *function = "socks_sigunblock()";

   if (sigprocmask(SIG_SETMASK, oldset, NULL) != 0)
      swarn("%s: sigprocmask()", function);
}

int
socks_msghaserrors(prefix, msg)
   const char *prefix;
   const struct msghdr *msg;
{
   if (msg->msg_flags & MSG_TRUNC) {
      swarnx("%s: msg is truncated ... message discarded", prefix);

      if (CMSG_TOTLEN(*msg) > 0)
         swarnx("%s: XXX should close received descriptors", prefix);

      return 1;
   }

   if (msg->msg_flags & MSG_CTRUNC) {
      swarnx("%s: cmsg was truncated ... message discarded", prefix);
      return 1;
   }

   return 0;
}

void
seconds2days(seconds, days, hours, minutes)
   unsigned long *seconds;
   unsigned long *days;
   unsigned long *hours;
   unsigned long *minutes;
{

   if (*seconds >= 3600 * 24) {
      *days     = *seconds / (3600 * 24);
      *seconds -= (time_t)(*days * 3600 * 24);
   }
   else
      *days = 0;

   if (*seconds >= 3600) {
      *hours    = *seconds / 3600;
      *seconds -= (time_t)(*hours * 3600);
   }
   else
      *hours = 0;

   if (*seconds >= 60) {
      *minutes  = *seconds / 60;
      *seconds -= (time_t)(*minutes * 60);
   }
   else
      *minutes = 0;

#if DIAGNOSTIC
   SASSERTX(*seconds < 60);
   SASSERTX(*minutes < 60);
   SASSERTX(*hours   < 24);
#endif /* DIAGNOSTIC */

}

struct sockaddr_storage *
int_urlstring2sockaddr(string, saddr, saddrlen, gaierr, emsg, emsglen)
   const char *string;
   struct sockaddr_storage *saddr;
   size_t saddrlen;
   int *gaierr;
   char *emsg;
   size_t emsglen;
{
   const char *function = "int_urlstring2sockaddr()";
   const char *httpprefix = "http://";
   char buf[1024], vbuf[sizeof(buf) * 4], vstring[sizeof(vbuf)],
        emsgmem[1024], *port, *s;
   long portnumber;
   int haveportsep;

   *gaierr = 0;

   bzero(saddr, saddrlen);
   SET_SOCKADDR(saddr, AF_UNSPEC);

   if (emsg == NULL) {
      emsg    = emsgmem;
      emsglen = sizeof(emsgmem);
   }

   slog(LOG_DEBUG, "%s: string to parse is \"%s\"",
        function, str2vis(string, strlen(string), vstring, sizeof(vstring)));

   if (strstr(string, httpprefix) == NULL) {
      snprintf(emsg, emsglen,
               "could not find http prefix (%s) in http address \"%s\"",
               httpprefix, vstring);

      slog(LOG_DEBUG, "%s: %s", function, emsg);
      return NULL;
   }

   string += strlen(httpprefix);

   snprintf(buf, sizeof(buf), "%s", string);

   if ((s = strchr(buf, ':')) == NULL) {
      slog(LOG_DEBUG, "%s: could not find port separator in \"%s\"",
           function, vstring);

      haveportsep = 0;
   }
   else {
      haveportsep = 1;
      *s = NUL;
   }

   if (*buf == NUL) {
      snprintf(emsg, emsglen,
               "could not find address string in \"%s\"", vstring);

      slog(LOG_DEBUG, "%s: %s", function, emsg);
      return NULL;
   }

   slog(LOG_DEBUG, "%s: pre-portnumber string (%s): \"%s\"",
        function,
        haveportsep ? "portnumber comes later" : "no portnumber given",
        str2vis(buf, strlen(buf), vbuf, sizeof(vbuf)));

   if (socks_inet_pton(saddr->ss_family, buf, &(TOIN(saddr)->sin_addr), NULL)
   != 1) {
      struct hostent *hostent;
      char *ep;

      errno = 0;
      (void)strtol(buf, &ep, 10);

      if (*ep == NUL || errno == ERANGE) {
         /* only digits, but inet_pton() failed. */
         snprintf(emsg, emsglen,
                 "\"%s\" does not appear to be a valid IP address",
                 str2vis(buf, strlen(buf), vbuf, sizeof(vbuf)));

         slog(LOG_DEBUG, "%s: %s", function, emsg);
         return NULL;
      }

      if ((hostent = gethostbyname(buf)) == NULL
      ||   hostent->h_addr               == NULL) {
         snprintf(emsg, emsglen, "could not resolve hostname \"%s\"",
                  str2vis(buf, strlen(buf), vbuf, sizeof(vbuf)));

         slog(LOG_DEBUG, "%s: %s", function, emsg);
         return NULL;
      }

      SET_SOCKADDR(saddr, (uint8_t)hostent->h_addrtype);
      memcpy(&TOIN(saddr)->sin_addr,
             hostent->h_addr_list[0],
             (size_t)hostent->h_length);
   }


   if (haveportsep) {
      if ((port = strchr(string, ':')) == NULL) {
         snprintf(emsg, emsglen,
                 "could not find start of port number in \"%s\"",
                 str2vis(string, strlen(string), vbuf, sizeof(vbuf)));

         return NULL;
      }
      ++port; /* skip ':' */

      if ((portnumber = string2portnumber(port, emsg, emsglen)) == -1)
         return NULL;
   }
   else
      portnumber = SOCKD_HTTP_PORT;

   TOIN(saddr)->sin_port = htons((in_port_t)portnumber);

   slog(LOG_DEBUG, "%s: returning addr %s",
        function, sockaddr2string(saddr, NULL, 0));

   return saddr;
}

#undef snprintf
size_t
snprintfn(char *str, size_t size, const char *format, ...)
{
   const int errno_s = errno;
   va_list ap;
   ssize_t rc;

   if (size <= 0 || str == NULL)
      return 0;

   va_start(ap, format);
   rc = vsnprintf(str, size, format, ap);
   va_end(ap);

   errno = errno_s; /* don't want snprintf(3) to change errno. */

   if (rc <= 0) {
      *str = NUL;
      rc   = 0;
   }
   else if (rc >= (ssize_t)size) {
      rc = (ssize_t)(size - 1);
      str[rc] = NUL; /* we never return non-terminated strings. */
   }

   if (size > 0)
      SASSERTX(str[rc] == NUL);

   return (size_t)rc;
}

/*
 * NOTE: close() macro undefined; closen() function needs to be at the
 * end of the file.
 */
int
closen(d)
   int d;
{
   int rc;

#undef close  /* we redefine close() to closen() for convenience. */
   while ((rc = close(d)) == -1 && errno == EINTR)
      ; /* LINTED empty */

   if (rc == -1 && errno != EBADF) {
      /*
       * Some people don't understand one should not introduce random
       * extra return codes into standard system calls without thinking
       * about it carefully first.
       * E.g. FreeBSD seems to think it's perfectly ok to let close(2)
       * close the socket, yet return -1 and set errno to ECONNRESET.
       * Never mind this breaks all sort of applications who keep track
       * of their fd's well enough to consider a failure from close(2)
       * an indication of something being wrong in their code, rather
       * than a TCP connection being reset.
       */
      errno = 0;
      rc    = 0;
   }

   return rc;
}

int
linkednamesareeq(a, b)
   const linkedname_t *a;
   const linkedname_t *b;
{

   /*
    * Check that they have the same contents and in the same order.
    */
   while (1) {
      if (a == b)
         return 1;

      if (a == NULL || b == NULL)
         return 0;

      if (strcmp(a->name, b->name) != 0)
         return 0;

      a = a->next;
      b = b->next;
   }

   /* NOTREACHED */
   SERRX(0);
}
