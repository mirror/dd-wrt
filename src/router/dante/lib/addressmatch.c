/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2005, 2008, 2009, 2010, 2011,
 *               2012, 2013, 2014, 2016, 2017
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
"$Id: addressmatch.c,v 1.97.4.3.2.6 2017/01/31 08:17:38 karls Exp $";

static int
ipv4_addrareeq(const struct in_addr *a, const struct in_addr *b,
               const struct in_addr *mask);
/*
 * Compares "a", bitwise and-ed with "mask", against "b", also bitwise
 * and-ed with "mask".
 *
 * Returns:
 *      If "against" matches "addr" and "mask": true
 *      else: false
 */

UNIT_TEST_STATIC_SCOPE int
ipv6_addrareeq(const struct in6_addr *a, const struct in6_addr *b,
               unsigned int maskbits);
/*
 * Same as previous two functions, but for IPv6.
 */

static int
hostareeq(const char *ruledomain, const char *addrdomain);
/*
 * Compares the rule-given domain "ruledomain" against "addrdomain".
 *
 * Note that if "ruledomain" starts with a dot, it will match "addrdomain" if
 * the last part of "addrdomain" matches the part after the dot in "ruledomain".
 *
 * Returns:
 *      if match   : true
 *      if no match: false
 */


int
addrmatch(rule, addr, addrmatched, protocol, alias)
   const ruleaddr_t *rule;
   const sockshost_t *addr;
   sockshost_t *addrmatched;
   int protocol;
   int alias;
{
   const char *function = "addrmatch()";
   struct addrinfo *ai_rule, *ai_addr, hints;
   struct sockaddr_storage sa;
   dnsinfo_t ai_rulemem, ai_addrmem;
   sockshost_t addrmatched_mem;
   in_port_t ruleport;
   const void *p;
   size_t i;
   char hostname[MAXHOSTNAMELEN];
   int doresolve, rc;

   if (sockscf.option.debug) {
      char rstring[MAXRULEADDRSTRING], astring[MAXSOCKSHOSTSTRING];

      slog(LOG_DEBUG,
           "%s: matching ruleaddress %s against %s for protocol %s, %s alias",
           function,
           ruleaddr2string(rule,
                           ADDRINFO_ATYPE | ADDRINFO_PORT,
                           rstring,
                           sizeof(rstring)),
           sockshost2string2(addr,
                             ADDRINFO_ATYPE | ADDRINFO_PORT,
                             astring,
                             sizeof(astring)),
           protocol2string(protocol),
           alias ? "with" : "without");
   }

   if (addrmatched == NULL)
      addrmatched = &addrmatched_mem;

   *addrmatched = *addr;

   /* test port first since we always have all info needed for that locally. */
   switch (protocol) {
      case SOCKS_TCP:
         ruleport = rule->port.tcp;
         break;

      case SOCKS_UDP:
         ruleport = rule->port.udp;
         break;

      default:
         SERRX(protocol);
   }

   switch (rule->operator) {
      case none:
         break;

      case eq:
         if (addr->port == ruleport)
            break;
         return 0;

      case neq:
         if (addr->port != ruleport)
            break;
         return 0;

      case ge:
         if (ntohs(addr->port) >= ntohs(ruleport))
            break;
         return 0;

      case le:
         if (ntohs(addr->port) <= ntohs(ruleport))
            break;
         return 0;

      case gt:
         if (ntohs(addr->port) > ntohs(ruleport))
            break;
         return 0;

      case lt:
         if (ntohs(addr->port) < ntohs(ruleport))
            break;
         return 0;

      case range:
         if (ntohs(addr->port) >= ntohs(ruleport)
         &&  ntohs(addr->port) <= ntohs(rule->portend))
            break;
         return 0;

      default:
         SERRX(rule->operator);
   }

   switch (sockscf.resolveprotocol) {
      case RESOLVEPROTOCOL_TCP:
      case RESOLVEPROTOCOL_UDP:
         doresolve = 1;
         break;

      case RESOLVEPROTOCOL_FAKE:
         doresolve = 0;
         break;

      default:
         SERRX(sockscf.resolveprotocol);
   }

   /*
    * Check the obvious go/no-gos first.
    */

   /*
    * No-go: different address families.  Make sure we check each
    * family separately, as we do not want to check against
    * SOCKS_ADDR_IPVANY here.
    */

   if (rule->atype == SOCKS_ADDR_IPV4 && addr->atype == SOCKS_ADDR_IPV6)
      return 0;

   if (rule->atype == SOCKS_ADDR_IPV6 && addr->atype == SOCKS_ADDR_IPV4)
      return 0;


   /*
    * Go: zero netmask.
    */

   if (rule->atype == SOCKS_ADDR_IPVANY) {
      SASSERTX(rule->addr.ipvany.mask.s_addr == htonl(0));
      return 1;
   }

   if (rule->atype == SOCKS_ADDR_IPV4
   &&  rule->addr.ipv4.mask.s_addr == htonl(0)
   &&  addr->atype == rule->atype)
      return 1;

   if (rule->atype == SOCKS_ADDR_IPV6
   &&  rule->addr.ipv6.maskbits == 0
   &&  addr->atype == rule->atype)
      return 1;

#if SOCKS_CLIENT
   /*
    * Wrong, but maintains compatability with 1.4.1 regarding
    * 0.0.0.0/0 matching everything.  Keep that compatability with the
    * the 1.4.1 socks.conf-files for now.
    */
   if (rule->atype                 == SOCKS_ADDR_IPV4
   &&  rule->addr.ipv4.mask.s_addr == htonl(0)
   &&  addr->atype                 == SOCKS_ADDR_DOMAIN) {

      *addrmatched = *addr;
      return 1;
   }
#endif /* SOCKS_CLIENT */


   /*
    * The hard work begins.
    */


   bzero(&hints, sizeof(hints));

   /*
    * if mask of rule is 0, it should match anything.  Try that, and other
    * things we can decide on quickly first, since it can save ourselves
    * lots of very heavy work.
    */
   if ((rule->atype == SOCKS_ADDR_IPV4 || rule->atype == SOCKS_ADDR_IPV6)
   &&   addr->atype == SOCKS_ADDR_DOMAIN) {
      /*
       * match(rule.ipaddr, addr.hostname)
       *
       * resolve addr.hostname to ipaddress(es) and try to match each
       * resolved ipaddress against rule.ipaddress:
       *      rule.ipaddr isin addr.hostname.ipaddr
       *      .
       */

      if (!doresolve)
         return 0;

      /*
       * Instead of setting hints.ai_family to atype2safamily(rule->atype),
       * we need to set it to zero.  Otherwise the getaddrinfo(3)-api will
       * not return IPv4-mapped IPv6 addresses.
       * If the rule is e.g. an IPv4 address, but the target resolves to an
       * IPv4-mapped IPv6 address, this would otherwise not work;
       * getaddrinfo(3) would return the regular IPv4-addresses, but not the
       * IPv4-mapped IPv6 address, then though that address (converted to
       * plain IPv4) is something we would match.
       */
      hints.ai_family = 0;

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      rc = cgetaddrinfo(addr->addr.domain, NULL, &hints, &ai_addr, &ai_addrmem);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      if (rc != 0) {
         log_resolvefailed(addr->addr.domain, EXTERNALIF, rc);
         return 0;
      }

      switch (rule->atype) {
         case SOCKS_ADDR_IPV4:
            if ((p = ipv4_addrisinlist(&rule->addr.ipv4.ip,
                                       &rule->addr.ipv4.mask,
                                       ai_addr)) == NULL)
               return 0;

            addrmatched->atype = SOCKS_ADDR_IPV4;
            memcpy(&addrmatched->addr.ipv4, p, sizeof(addrmatched->addr.ipv4));
            return 1;

         case SOCKS_ADDR_IPV6:
            if ((p = ipv6_addrisinlist(&rule->addr.ipv6.ip,
                                       rule->addr.ipv6.maskbits,
                                       ai_addr)) == NULL)
               return 0;

            addrmatched->atype = SOCKS_ADDR_IPV6;

            memcpy(&addrmatched->addr.ipv6.ip,
                   p,
                   sizeof(addrmatched->addr.ipv6.ip));

            return 1;

         default:
            SERRX(rule->atype);
      }
   }
   else if ((rule->atype == SOCKS_ADDR_IPV4 && addr->atype == SOCKS_ADDR_IPV4)
   ||       (rule->atype == SOCKS_ADDR_IPV6 && addr->atype == SOCKS_ADDR_IPV6)){
      /*
       * match(rule.ipaddr, addr.ipaddr)
       *
       * try simple comparison first: rule.ipaddr against addr.ipaddr.
       */

      switch (rule->atype) {
         case SOCKS_ADDR_IPV4:
            if (ipv4_addrareeq(&rule->addr.ipv4.ip,
                               &addr->addr.ipv4,
                               &rule->addr.ipv4.mask))
               return 1;
            break;

         case SOCKS_ADDR_IPV6:
            if (ipv6_addrareeq(&rule->addr.ipv6.ip,
                               &addr->addr.ipv6.ip,
                               rule->addr.ipv6.maskbits))
               return 1;
            break;

         default:
            SERRX(rule->atype);
      }

      /*
       * Did not match.  If alias is set, try to resolve addr.ipaddr
       * to hostname(s), the hostname back to IP address(es) and
       * then match those IP address(es) against rule.ipaddr:
       *
       *    rule.ipaddr isin addr.ipaddr->hostname(s)->ipaddr(eses)
       *    .
       *
       * Quite an expensive thing, so will hopefully not be used often,
       * but might be needed for accepting bind-replies from multihomed
       * hosts.  E.g. the client connects to IP i1, but the bindreply
       * comes from IP i2 on the same host.  If i1 and i2 are for the
       * same host, they should reversemap back to the same hostname,
       * and that hostname should resolve to both i1 and i2.
       */

      if (!doresolve)
         return 0;

      if (!alias)
         return 0;

      sockshost2sockaddr(addr, &sa);

      rc = sockaddr2hostname(&sa, hostname, sizeof(hostname));

      if (rc != 0) {
         log_resolvefailed(sockaddr2string2(&sa, 0, NULL, 0),
                           EXTERNALIF,
                           rc);

         return 0;
      }

      /*
       * Reverse-mapped addr.ipaddr to a hostname.  Now resolve that hostname
       * to ipaddress(es), and compare each of those against rule.ipaddr.
       */

      hints.ai_family = atype2safamily(rule->atype);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      rc = cgetaddrinfo(hostname, NULL, &hints, &ai_addr, &ai_addrmem);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      if (rc != 0) {
         log_resolvefailed(hostname, EXTERNALIF, rc);
         return 0;
      }

      /*
       * Should now have one or more ipaddresses in ai_addr.
       * Compare them against rule.ipaddr:
       *    rule.ipaddr isin addr->hostname->ipaddr
       *    .
       */
      switch (rule->atype) {
         case SOCKS_ADDR_IPV4:
            if ((p = ipv4_addrisinlist(&rule->addr.ipv4.ip,
                                       &rule->addr.ipv4.mask,
                                       ai_addr)) == NULL)
               return 0;

            addrmatched->atype = SOCKS_ADDR_IPV4;
            memcpy(&addrmatched->addr.ipv4, p, sizeof(addrmatched->addr.ipv4));
            return 1;

         case SOCKS_ADDR_IPV6:
            if ((p = ipv6_addrisinlist(&rule->addr.ipv6.ip,
                                       rule->addr.ipv6.maskbits,
                                       ai_addr)) == NULL)
               return 0;

            addrmatched->atype = SOCKS_ADDR_IPV6;

            memcpy(&addrmatched->addr.ipv6.ip,
                   p,
                   sizeof(addrmatched->addr.ipv6.ip));

            return 1;

         default:
            SERRX(rule->atype);
      }
   }
   else if (rule->atype == SOCKS_ADDR_DOMAIN
   &&       addr->atype == SOCKS_ADDR_DOMAIN) {
      /*
       * Try the simple match first:
       *    match(rule.hostname, addr.hostname)
       *
       * If no go and rule is a hostname (rather than a domain (.domain)),
       * resolve both rule.hostname and addr.hostname to IP address(es),
       * and then compare each IP address of resolved rule.hostname against
       * each IP address of resolved address.hostname:
       *
       *    rule->hostname->ipaddress(es) isin addr->hostname->ipaddress(es)
       *    .
       */
      struct addrinfo *next;

      /* Note: this also handles the '.domain'-case. */
      if (hostareeq(rule->addr.domain, addr->addr.domain))
         return 1;

      if (!doresolve)
         return 0;

      if (*rule->addr.domain == '.')
         /*
          * can not resolve rule.domain to any ipaddresses, and since
          * addr.hostname did not match, there is nothing to do.
          * (except possibly resolve addr.hostname to ipaddresses, and
          * reverse-map those back to hostnames.  But we don't do that.)
          */
         return 0;

      /*
       * resolve both rule.hostname and addr.hostname to ipaddresses, and
       * then match those ipaddresses against each others.
       */

      set_hints_ai_family(&hints.ai_family);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      rc = cgetaddrinfo(addr->addr.domain,
                        NULL,
                        &hints,
                        &ai_addr,
                        &ai_addrmem);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      if (rc != 0) {
         log_resolvefailed(addr->addr.domain, EXTERNALIF, rc);
         return 0;
      }

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      rc = cgetaddrinfo(rule->addr.domain, NULL, &hints, &ai_rule, &ai_rulemem);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      if (rc != 0) {
         log_resolvefailed(rule->addr.domain, INTERNALIF, rc);

         swarnx("%s: cgetaddrinfo(%s) failed: %s",
                function, rule->addr.domain, gai_strerror(rc));

         return 0;
      }

      /*
       * Ok, both hostname in rule and hostname to check resolved.
       * All set to go for comparing all ipaddresses from rule against
       * all ipaddresses from hostname to check against:
       *    rule->hostname->ipaddress(es) isin addr->hostname->ipaddress(es)
       *    .
       */

      next = ai_rule;
      do {
         SASSERT(next->ai_addr != NULL);

         switch (next->ai_family) {
            case AF_INET: {
               struct in_addr mask;

               mask.s_addr = htonl(IPV4_FULLNETMASK);

               if ((p = ipv4_addrisinlist(&TOIN(next->ai_addr)->sin_addr,
                                          &mask,
                                          ai_addr)) != NULL) {
                  addrmatched->atype = SOCKS_ADDR_IPV4;

                  memcpy(&addrmatched->addr.ipv4,
                         p,
                         sizeof(addrmatched->addr.ipv4));

                  return 1;
               }

               break;
            }

            case AF_INET6:
               if ((p = ipv6_addrisinlist(&TOIN6(next->ai_addr)->sin6_addr,
                                          IPV6_NETMASKBITS,
                                          ai_addr)) != NULL) {

                  addrmatched->atype = SOCKS_ADDR_IPV6;

                  memcpy(&addrmatched->addr.ipv6.ip,
                         p,
                         sizeof(addrmatched->addr.ipv6.ip));

                  return 1;
               }

               break;

            default:
               swarnx("%s: unexpected af_family in ai_rule for %s: %d",
                      function,
                      rule->addr.domain,
                      next->ai_family);
         }

         next = next->ai_next;
      } while (next != NULL);

      return 0;
   }
   else if (rule->atype == SOCKS_ADDR_DOMAIN
   &&      (addr->atype == SOCKS_ADDR_IPV4 || addr->atype == SOCKS_ADDR_IPV6)) {
      /*
       * match(rule.hostname, addr.ipaddress)
       *
       * If rule is not a domain (.domain) but a hostname, try resolving
       * rule.hostname to IP addresses and match each of those against
       * addr.ipaddress:
       *      addr.ipaddr isin rule.hostname->ipaddr
       *
       * If still no match, and alias is set, resolve addr.ipv4 to hostname(s),
       * those hostnames back to ip, and and match against
       * rule.hostame->ipaddr:
       *    rule.hostname->ipaddr isin addr->ipaddr->hostname(s)->ipaddr
       *    .
       *
       * Quite an expensive thing, so will hopefully not be used often,
       * but might be needed for accepting bind-replies from multihomed
       * hosts.  E.g. the client connects to IP i1, but the bindreply
       * comes from IP i2 on the same host.  If i1 and i2 are for the
       * same host, they should reversemap back to the same hostname,
       * and that hostname should resolve to both i1 and i2.
       *
       * Note that we do not attempt to
       * match(rule.hostname, addr->ipaddr->hostname), as addr->ipaddr
       * can resolve to whatever it wants to and is thus not safe.
       * We could consider it and depend on user setting, if he so wishes,
       * "srchost: nodnsmismatch" to avoid possible problems, but currently
       * we do not.
       *
       * If on the other hand rule *is* a domain, we have no other choice
       * but to try to match rule.domain against addr->ipaddr->hostname(s):
       *    addr.ipaddr->hostnames isin rule.domain
       *    .
       */
      struct addrinfo *next;

      if (!doresolve)
         return 0;

      if (*rule->addr.domain == '.') {
         /*
          * Reversemap addr.ipaddr to hostnames and compare against that.
          */

         sockshost2sockaddr(addr, &sa);

         rc = sockaddr2hostname(&sa, hostname, sizeof(hostname));

         if (rc != 0) {
            log_resolvefailed(sockaddr2string2(&sa, 0, NULL, 0),
                              EXTERNALIF,
                              rc);

            return 0;
         }

         return hostareeq(rule->addr.domain, hostname);
      }

      /*
       * Else: resolve rule.hostname to ipaddresses and match them against
       * addr.ipaddr:
       *    addr.ipaddr isin rule.hostname->ipaddr
       */

      /*
       * hints.ai_family = atype2safamily(addr->atype);
       *
       * Don't set ai_family in hints.  Even though we are only looking for
       * addresses of type addr->atype, specifying that in hints will make
       * getaddrinfo(3) fail, and there appears to unfortunately not be
       * any way to distinguish by the failure code whether the failure
       * is due to no addresses at all, or just no address for the specified
       * address family.  The former is likely an error that we want to
       * warn about, since the name we are trying to resolve is specified
       * in a rule, while the latter is more likely not a problem (unless
       * the specified address-family is also the only address-family
       * configured on the external interface.
       * Assume it's better to risk getaddrinfo() returning fewer than the
       * possible number of addr->atype addresses, in return for being able
       * to warn about what probably a configuration error somewhere.
       */

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      rc = cgetaddrinfo(rule->addr.domain, NULL, &hints, &ai_rule, &ai_rulemem);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      if (rc != 0) {
         log_resolvefailed(rule->addr.domain, INTERNALIF, rc);

         swarnx("%s: cgetaddrinfo(%s) failed: %s",
                function, rule->addr.domain, gai_strerror(rc));

         return 0;
      }

      switch (addr->atype) {
         case SOCKS_ADDR_IPV4: {
            struct in_addr mask;

            mask.s_addr = htonl(IPV4_FULLNETMASK);

            if ((p = ipv4_addrisinlist(&addr->addr.ipv4, &mask, ai_rule))
            != NULL) {
               addrmatched->atype = SOCKS_ADDR_IPV4;

               memcpy(&addrmatched->addr.ipv4,
                      p,
                      sizeof(addrmatched->addr.ipv4));

               return 1;
            }

            break;
         }

         case SOCKS_ADDR_IPV6: {
            if ((p = ipv6_addrisinlist(&addr->addr.ipv6.ip,
                                       IPV6_NETMASKBITS,
                                       ai_rule)) != NULL) {
               addrmatched->atype = SOCKS_ADDR_IPV6;

               memcpy(&addrmatched->addr.ipv6.ip,
                      p,
                      sizeof(addrmatched->addr.ipv6.ip));

               return 1;
            }

            break;
         }

         default:
            SERRX(addr->atype);
      }

      if (!alias)
         return 0;

      /*
       * Else: reversemap addr.ipaddr to hostname, hostname to ipaddr,
       * and then try to match that against rule.hostname->ipaddr:
       */

      sockshost2sockaddr(addr, &sa);

      rc = sockaddr2hostname(&sa, hostname, sizeof(hostname));

      if (rc != 0) {
         log_resolvefailed(sockaddr2string2(&sa, 0, NULL, 0),
                           EXTERNALIF,
                           rc);
         return 0;
      }

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnative("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      rc = cgetaddrinfo(hostname, NULL, &hints, &ai_addr, &ai_addrmem);

#if SOCKSLIBRARY_DYNAMIC
      socks_markasnormal("*");
#endif /* SOCKSLIBRARY_DYNAMIC */

      if (rc != 0) {
         log_resolvefailed(hostname, EXTERNALIF, rc);
         return 0;
      }


      /*
       * See if any of the addresses resolved from
       * rule->hostname->ipaddr match addr->ipaddr->hostname->ipaddr:
       *    rule.hostname->ipaddr isin addr.ipaddr->hostname(s)->ipaddr
       *    .
       */
      next = ai_rule;
      do {
         SASSERT(next->ai_addr != NULL);

         switch (next->ai_family) {
            case AF_INET: {
               struct in_addr mask;

               mask.s_addr = htonl(IPV4_FULLNETMASK);
               if ((p = ipv4_addrisinlist(&TOIN(next->ai_addr)->sin_addr,
                                          &mask,
                                          ai_addr)) != NULL) {
                  addrmatched->atype = SOCKS_ADDR_IPV4;

                  memcpy(&addrmatched->addr.ipv4,
                         p,
                         sizeof(addrmatched->addr.ipv4));

                  return 1;
               }

               break;
            }

            case AF_INET6:
               if ((p = ipv6_addrisinlist(&TOIN6(next->ai_addr)->sin6_addr,
                                          IPV6_NETMASKBITS,
                                          ai_addr)) != NULL) {
                  addrmatched->atype = SOCKS_ADDR_IPV6;

                  memcpy(&addrmatched->addr.ipv6.ip,
                         p,
                         sizeof(addrmatched->addr.ipv6.ip));

                  return 1;
               }

               break;

            default:
               swarnx("%s: unexpected af_family in ai_rule for %s: %d",
                      function,
                      rule->addr.domain,
                      next->ai_family);
         }

         next = next->ai_next;
      } while (next != NULL);

      return 0;
   }
   else if (rule->atype == SOCKS_ADDR_IFNAME) {
      /*
       * Like rule.ipaddress, just call it for each IP address of interface.
       *
       * match(rule.ifname2ipaddress, addr)
       */
       struct sockaddr_storage ss, mask;
       int matched;

       i = matched = 0;
       while (!matched
       && ifname2sockaddr(rule->addr.ifname, i++, &ss, &mask) != NULL) {
         ruleaddr_t ruleaddr;

         sockaddr2ruleaddr(&ss, &ruleaddr);
         if (addrmatch(&ruleaddr, addr, addrmatched, protocol, alias))
            return 1;
      }

      return 0;
   }
   else
      SERRX(0);

   /* NOTREACHED */
   SERRX(0);
   return 0;
}

const struct in_addr *
ipv4_addrisinlist(addr, mask, ailist)
   const struct in_addr *addr;
   const struct in_addr *mask;
   const struct addrinfo *ailist;
{
   const struct addrinfo *next = ailist;
   do {
      SASSERTX(next->ai_addr != NULL);

      if (next->ai_addr->sa_family == AF_INET) {
         if (ipv4_addrareeq(addr, &TOCIN(next->ai_addr)->sin_addr, mask))
            return &TOCIN(next->ai_addr)->sin_addr;
      }

      next = next->ai_next;
   } while (next != NULL);

   return NULL;
}

const struct in6_addr *
ipv6_addrisinlist(addr, maskbits, ailist)
   const struct in6_addr *addr;
   const unsigned int maskbits;
   const struct addrinfo *ailist;
{
   const struct addrinfo *next = ailist;
   do {
      SASSERTX(next->ai_addr != NULL);

      if (next->ai_addr->sa_family == AF_INET6) {
         if (ipv6_addrareeq(addr, &TOCIN6(next->ai_addr)->sin6_addr, maskbits))
            return &TOCIN6(next->ai_addr)->sin6_addr;
      }

      next = next->ai_next;
   } while (next != NULL);

   return NULL;
}


static int
ipv4_addrareeq(a, b, mask)
   const struct in_addr *a;
   const struct in_addr *b;
   const struct in_addr *mask;
{

   return (a->s_addr & mask->s_addr) == (b->s_addr & mask->s_addr);
}

UNIT_TEST_STATIC_SCOPE int
ipv6_addrareeq(a, b, maskbits)
   const struct in6_addr *a;
   const struct in6_addr *b;
   unsigned int maskbits;
{
   const unsigned int maskv[CHAR_BIT + 1] = { 0,   /* 00000000 */
                                              128, /* 10000000 */
                                              192, /* 11000000 */
                                              224, /* 11100000 */
                                              240, /* 11110000 */
                                              248, /* 11111000 */
                                              252, /* 11111100 */
                                              254, /* 11111110 */
                                              255  /* 11111111 */ };
   size_t i;

   SASSERTX(CHAR_BIT <= 8);
   SASSERTX(maskbits <= IPV6_NETMASKBITS);

   /*
    * unfortunately "s6_addr" is the only member defined in the RFC.
    * Perhaps later add some autoconf checks for whether we also have
    * s6_addr32, and do uint32_t compares on those platforms.
    *
    * Would be nice if some bit-twiddling wizard made some efficient
    * code available, but till then, this will hopefully work well enough.
    */

   SASSERTX((maskbits / CHAR_BIT) <= ELEMENTS(a->s6_addr));
   i = 0;
   while (maskbits > 0) {
      if (maskbits >= CHAR_BIT) {
         /* all bits of byte set, can do a direct compare. */
         if (a->s6_addr[i] != b->s6_addr[i])
            return 0;

         maskbits -= CHAR_BIT;
         ++i;
      }
      else {
         /*
          * only some bits of byte are set, mask out the remaining.
          * Also means this must be the last byte to compare.
          */
         return (a->s6_addr[i] & maskv[maskbits])
         ==     (b->s6_addr[i] & maskv[maskbits]);
      }
   }

   return 1;
}


static int
hostareeq(ruledomain, addrdomain)
   const char *ruledomain;
   const char *addrdomain;
{
   const char *function         = "hostareeq()";
   const size_t ruledomainlen   = strlen(ruledomain);
   const size_t remotedomainlen = strlen(addrdomain);

   slog(LOG_DEBUG, "%s: %s, %s", function, ruledomain, addrdomain);

   if  (*ruledomain == '.') { /* match everything ending in ruledomain */
      if (ruledomainlen - 1 /* '.' */ > remotedomainlen)
         return 0;   /* address to compare against too short, can't match. */

      return strcasecmp(ruledomain + 1,
                        addrdomain + (remotedomainlen - (ruledomainlen - 1)))
             == 0;
   }
   else /* need exact match. */
      return strcasecmp(ruledomain, addrdomain) == 0;
}
