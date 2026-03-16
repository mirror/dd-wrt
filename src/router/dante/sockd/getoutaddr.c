/*
 * $Id: getoutaddr.c,v 1.140.4.3.2.3 2017/01/31 08:17:38 karls Exp $
 *
 * Copyright (c) 2001, 2002, 2006, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
 *               2016, 2017
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
"$Id: getoutaddr.c,v 1.140.4.3.2.3 2017/01/31 08:17:38 karls Exp $";

static int
addrscope_matches(const struct sockaddr_in6 *addr,
                  const ipv6_addrscope_t addrscope);
/*
 * Returns true if address-scope "addrscope" matches the address scope
 * of address "addr".
 */

static int
ifname_matches(const char *ifname, const uint32_t ifindex);
/*
 * Returns true if there is an interface with the name "ifname", and
 * the index of this interface is "ifinxex".
 *
 * Returns false otherwise.
 */

static sa_family_t
get_external_safamily(const struct sockaddr_storage *client,
                      const int command, const sockshost_t *reqhost);
/*
 * Returns the sa_family_t that should be used for a client with address
 * "client", requesting the SOCKS command "command" with the request host
 * "reqhost".
 *
 * On success the sa_family_t that should be used.
 * On failure returns AF_UNSPEC.
 */


static struct sockaddr_storage *
getdefaultexternal(const sa_family_t safamily, ipv6_addrscope_t addrscope,
                   const uint32_t ifindex, struct sockaddr_storage *addr);
/*
 * Returns the default IP address of sa_family_t "safamily" to use for
 * external connections.  The portnumber in the address returned should
 * be ignored.
 *
 * "safamily" can be set to AF_UNSPEC if the function can return an
 * address of any sa_family_t.  If there is no address of type safamily
 * available, the semantics are the same as if safamily was set to AF_UNSPEC.
 *
 * "addrscope" indicates the scopeid of the external addresses we want
 * returned.  "addrscope" only applies if "safamily" is AF_INET6.
 * If "addrscope" is addrscope_linklocal, "ifindex" indicates the interface
 * the returned address should be configured on.  Otherwise, "ifindex" is
 * ignored.
 *
 * The ipaddress to use is stored in "addr", and a pointer to it is returned.
 */

struct sockaddr_storage *
getoutaddr(laddr, client_l, client_r, cmd, reqhost, emsg, emsglen)
   struct sockaddr_storage *laddr;
   const struct sockaddr_storage *client_l;
   const struct sockaddr_storage *client_r;
   const int cmd;
   const sockshost_t *reqhost;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "getoutaddr()";
   struct sockaddr_storage raddr;
   char addrstr[MAXSOCKADDRSTRING], raddrstr[MAXSOCKSHOSTSTRING];

   slog(LOG_DEBUG,
        "%s: client %s, cmd %s, reqhost %s, external.rotation = %s",
        function,
        sockaddr2string(client_r, addrstr, sizeof(addrstr)),
        command2string(cmd),
        sockshost2string(reqhost, raddrstr, sizeof(raddrstr)),
        rotation2string(sockscf.external.rotation));

   bzero(&raddr, sizeof(raddr));

   /*
    * First figure out what /type/ of address (ipv4 or ipv6) we need to
    * bind on the external side.
    */
   switch (cmd) {
      case SOCKS_BIND:
         if (reqhost->atype            == SOCKS_ADDR_IPV4
         &&  reqhost->addr.ipv4.s_addr == htonl(BINDEXTENSION_IPADDR))
            SET_SOCKADDR(&raddr, external_has_global_safamily(AF_INET) ?
                                 AF_INET : AF_INET6);
         else if (reqhost->atype == SOCKS_ADDR_IPV4
         ||       reqhost->atype == SOCKS_ADDR_IPV6)
            sockshost2sockaddr(reqhost, &raddr);
         else {
            /*
             * Have to expect the bindreply from an address given as a
             * hostname by the client.  Since we may have multiple address
             * families configured on the external interface on which we
             * can accept the bindreply on, we need to bind an address of
             * the correct type.
             *
             * For now we assume that the type of address returned first
             * is the type of address we should bind.
             */
            int gaierr;

            SASSERTX(reqhost->atype == SOCKS_ADDR_DOMAIN);

            sockshost2sockaddr2(reqhost, &raddr, &gaierr, emsg, emsglen);
            if (gaierr != 0) {
               log_resolvefailed(reqhost->addr.domain, EXTERNALIF, gaierr);
               return NULL;
            }
         }

         break;

      case SOCKS_CONNECT:
      case SOCKS_UDPASSOCIATE: {
         int gaierr;

         sockshost2sockaddr2(reqhost, &raddr, &gaierr, emsg, emsglen);
         if (gaierr != 0) {
            SASSERTX(reqhost->atype == SOCKS_ADDR_DOMAIN);

            log_resolvefailed(reqhost->addr.domain, EXTERNALIF, gaierr);
            return NULL;
         }

         break;
      }

      default:
         SERRX(cmd);
   }

   switch (sockscf.external.rotation) {
      case ROTATION_NONE:
         /*
          * We have a complicating factor here regarding IPv6 scopeids.
          * If the target destination is specified with a non-global
          * scopeid (e.g. a link-local address), we need to bind an address
          * not only of the same type, but on the *same link/interface* too.
          */

         getdefaultexternal(raddr.ss_family,
                            raddr.ss_family == AF_INET6 ?
                                ipv6_addrscope(&TOIN6(&raddr)->sin6_addr)
                              : addrscope_global,
                            TOIN6(&raddr)->sin6_scope_id,
                            laddr);
         break;

      case ROTATION_SAMESAME:
         if (client_l->ss_family == raddr.ss_family)
            *laddr = *client_l;
         else {
            snprintf(emsg, emsglen,
                     "rotation for external addresses is set to %s, but "
                     "that can not work for this %s request.  "
                     "The internal address we accepted the client on is "
                     "a %s (%s), but the target address is a %s (%s)",
                     rotation2string(sockscf.external.rotation),
                     command2string(cmd),
                     safamily2string(client_l->ss_family),
                     sockaddr2string(client_l, NULL, 0),
                     safamily2string(raddr.ss_family),
                     sockaddr2string(&raddr, raddrstr, sizeof(raddrstr)));

            swarnx("%s: %s", function, emsg);
            return NULL;
         }

         break;

      case ROTATION_ROUTE: {
         if (IPADDRISBOUND(&raddr)) {
            /*
             * Connect a udp socket and check what local address was chosen
             * by the kernel for connecting to dst.  Idea from Quagga source.
             */
            sockshost_t host;
            int s;

            if ((s = socket(raddr.ss_family, SOCK_DGRAM, 0)) == -1) {
               snprintf(emsg, emsglen,
                        "could not create new %s UDP socket with socket(2): %s",
                        safamily2string(raddr.ss_family), strerror(errno));

               swarn("%s: %s", function, emsg);
               return NULL;
            }

            if (!PORTISBOUND(&raddr))
               /* use any valid portnumber. This is just a dry-run */
               SET_SOCKADDRPORT(&raddr, 1);

            sockaddr2sockshost(&raddr, &host);
            if (socks_connecthost(s,
                                  EXTERNALIF,
                                  &host,
                                  laddr,
                                  NULL,
                                  -1,
                                  emsg,
                                  emsglen) == -1) {
               slog(LOG_DEBUG, "%s: %s", function, emsg);
               close(s);

               if (cmd == SOCKS_UDPASSOCIATE)
                  return NULL;

               /*
                * Else: continue.
                * While highly unlikely it will work later, when we actually
                * do try to connect/send data, it could be the local (but
                * external to Dante) configuration is to block udp packets to
                * this destination, but allow tcp.
                */
               getdefaultexternal(raddr.ss_family,
                                  raddr.ss_family == AF_INET6 ?
                                       ipv6_addrscope(&TOIN6(&raddr)->sin6_addr)
                                    :  addrscope_global,
                                  TOIN6(&raddr)->sin6_scope_id,
                                  laddr);
               break;
            }

            close(s);
         }
         else
            getdefaultexternal(get_external_safamily(client_r, cmd, reqhost),
                               addrscope_global,
                               0,
                               laddr);

         break;
      }

      default:
         SERRX(sockscf.external.rotation);
   }

   if (addrindex_on_externallist(&sockscf.external, laddr) != -1)
      slog(LOG_DEBUG,
           "%s: local address %s selected for forwarding from client %s to %s",
           function,
           sockaddr2string2(laddr,  0, addrstr,  sizeof(addrstr)),
           sockaddr2string(client_r, NULL, 0),
           sockaddr2string(&raddr, raddrstr, sizeof(raddrstr)));
   else {
      char rotation[256];

      if (sockscf.external.rotation == ROTATION_NONE)
         *rotation = NUL; /* default.  Don't print anything confusing. */
      else
         snprintf(rotation, sizeof(rotation),
                  "using external.rotation = %s, ",
                  rotation2string(sockscf.external.rotation));

      if (IPADDRISBOUND(laddr))
         snprintf(emsg, emsglen,
                  "%slocal address %s was selected for forwarding from our "
                  "local client %s to target %s, but that local address is "
                  "not set on our external interface(s).  Configuration "
                  "error in %s?",
                  rotation,
                  sockaddr2string2(laddr,  0, addrstr,  sizeof(addrstr)),
                  sockaddr2string(client_r, NULL,     0),
                  sockaddr2string(&raddr, raddrstr, sizeof(raddrstr)),
                  sockscf.option.configfile);
      else
         snprintf(emsg, emsglen,
                  "%snone of the addresses configured on our external "
                  "interface(s) can be used for forwarding from our local "
                  "client %s to target %s.  Configuration error in %s?",
                  rotation,
                  sockaddr2string(client_r, NULL, 0),
                  sockaddr2string(&raddr, raddrstr, sizeof(raddrstr)),
                  sockscf.option.configfile);

      /*
       * Assume that if the user has not configured any such address,
       * his intention is to not support that af (e.g., support only
       * ipv4).  If he has configured such an address, but we could for
       * some reason not use it, it's more likely to be a configuration
       * error though, so warn.
       */
      slog(external_has_safamily(laddr->ss_family) ? LOG_WARNING : LOG_DEBUG,
           "%s: %s", function, emsg);

      return NULL;
   }

   /*
    * Try to set the local port to the best value also, though this is mostly
    * just guessing for all but the bind-case.
    */
   switch (cmd) {
#if SOCKS_SERVER
      case SOCKS_BIND:
         if (reqhost->atype            == SOCKS_ADDR_IPV4
         &&  reqhost->addr.ipv4.s_addr == htonl(BINDEXTENSION_IPADDR))
            SET_SOCKADDRPORT(laddr, GET_SOCKADDRPORT(client_r));
         else if (  (raddr.ss_family == AF_INET
                  && TOIN(&raddr)->sin_addr.s_addr == htonl(INADDR_ANY))
               ||    (raddr.ss_family == AF_INET6
                  && memcmp(&TOIN6(&raddr)->sin6_addr,
                            &in6addr_any,
                            sizeof(in6addr_any)) == 0))
            SET_SOCKADDRPORT(laddr, reqhost->port);
         else
            SET_SOCKADDRPORT(laddr, htons(0));

         break;
#endif /* SOCKS_SERVER */

      case SOCKS_CONNECT:
      case SOCKS_UDPASSOCIATE: /* reqhost is the target of the first packet. */
         SET_SOCKADDRPORT(laddr, GET_SOCKADDRPORT(client_r));
         break;

      default:
         SERRX(cmd);
   }

   SASSERTX(IPADDRISBOUND(laddr));

   return laddr;
}

struct sockaddr_storage *
getinaddr(laddr, _client, emsg, emsglen)
   struct sockaddr_storage *laddr;
   const struct sockaddr_storage *_client;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "getinaddr()";
   struct sockaddr_storage client;
   size_t i;
   int wildcard_address_found = 0;

   slog(LOG_DEBUG, "%s: client %s",
        function, sockaddr2string(_client, NULL, 0));

   SASSERTX(_client->ss_family == AF_INET || _client->ss_family == AF_INET6);

   sockaddrcpy(&client, _client, sizeof(client));

   /*
    * Just return the first address of the appropriate type from our
    * internal list and hope the best.
    */
   for (i = 0; i < sockscf.internal.addrc; ++i) {
      if (sockscf.internal.addrv[i].addr.ss_family == client.ss_family) {
         if (IPADDRISBOUND(&sockscf.internal.addrv[i].addr)) {
            sockaddrcpy(laddr, &sockscf.internal.addrv[i].addr, sizeof(*laddr));

            slog(LOG_DEBUG, "%s: address %s selected",
                 function, sockaddr2string(laddr, NULL, 0));

            return laddr;
         }
         else
            wildcard_address_found = 1;
      }
   }

   if (wildcard_address_found)
      snprintf(emsg, emsglen,
               "no specific %s found amongst the internal addresses, only "
               "an unbound wildcard address found.  This client requires "
               "an internal IP-address to be specified in %s however",
               safamily2string(client.ss_family), SOCKD_CONFIGFILE);
   else
      snprintf(emsg, emsglen, "no %s found amongst the internal addresses",
               safamily2string(client.ss_family));

   slog(wildcard_address_found ? LOG_NOTICE : LOG_DEBUG,
        "%s: %s", function, emsg);

   return NULL;
}

static struct sockaddr_storage *
getdefaultexternal(safamily, addrscope, ifindex, addr)
   const sa_family_t safamily;
   const ipv6_addrscope_t addrscope;
   const uint32_t ifindex;
   struct sockaddr_storage *addr;
{
   const char *function = "getdefaultexternal()";
   const char *safamilystring  = (safamily == AF_UNSPEC ?
                                   "<any address>" : safamily2string(safamily));
   const char *addrscopestring = addrscope2string(addrscope);
   size_t i, addrfound;

   slog(LOG_DEBUG, "%s: looking for an %s with scopeid %d/%s, ifindex %d",
        function, safamilystring, addrscope, addrscopestring, ifindex);

   for (i = 0, addrfound = 0; i < sockscf.external.addrc && !addrfound; ++i) {
      switch (sockscf.external.addrv[i].atype) {
         case SOCKS_ADDR_IFNAME: {
            struct sockaddr_storage mask;

            size_t ii = 0;
            while (ifname2sockaddr(sockscf.external.addrv[i].addr.ifname,
                                   ii++,
                                   addr,
                                   &mask) != NULL) {
               if (safamily == AF_UNSPEC || addr->ss_family == safamily) {
                  if (safamily == AF_INET6) {
                     if (!addrscope_matches(TOIN6(addr), addrscope))
                        continue;

                     if (addrscope == addrscope_linklocal
                     && !ifname_matches(sockscf.external.addrv[i].addr.ifname,
                                        ifindex))
                        continue;
                  }

                  addrfound = 1;
                  break;
               }
            }

            break;
         }

         case SOCKS_ADDR_IPV4:
         case SOCKS_ADDR_IPV6:
            if (safamily != AF_UNSPEC
            &&  safamily != atype2safamily(sockscf.external.addrv[i].atype)) {
               slog(LOG_DEBUG,
                    "%s: atype of address %s does not match atype %s",
                    function,
                    ruleaddr2string(&sockscf.external.addrv[i],
                                    ADDRINFO_ATYPE,
                                    NULL,
                                    0),
                    safamily2string(safamily));
               continue;
            }

            sockshost2sockaddr(ruleaddr2sockshost(&sockscf.external.addrv[i],
                                                  NULL,
                                                  SOCKS_TCP),
                               addr);

            if (addr->ss_family == AF_INET6) {
               const char *ifname;

               if (addrscope == addrscope_nodelocal)
                  /*
                   * a nodelocal address should be able to reach any
                   * other nodelocal address.
                   */
                  ;
               else if (!addrscope_matches(TOIN6(addr), addrscope))
                  continue;

                if ((ifname = sockaddr2ifname(addr, NULL, 0)) == NULL) {
                  swarnx("%s: could not find any interface with address %s",
                         function, sockaddr2string(addr, NULL, 0));

                  continue;
               }

               if (addrscope == addrscope_linklocal
               && !ifname_matches(ifname, ifindex))
                  continue;
            }

            addrfound = 1;
            break;

         default:
            SERRX((*sockscf.external.addrv).atype);
      }
   }

   if (addrfound)
      slog(LOG_DEBUG, "%s: matched %s %s",
           function,
           safamilystring,
           sockaddr2string2(addr, ADDRINFO_SCOPEID | ADDRINFO_PORT, NULL, 0));
   else {
      slog(LOG_DEBUG,
           "%s: no matching %s found on external list, using INADDR_ANY",
           function, safamilystring);

      bzero(addr, sizeof(*addr));
      SET_SOCKADDR(addr, safamily == AF_UNSPEC ? AF_INET : safamily);
   }

   return addr;
}

sa_family_t
get_external_safamily(client, command, reqhost)
   const struct sockaddr_storage *client;
   const int command;
   const sockshost_t *reqhost;
{
   const char *function = "get_external_safamily()";
   sa_family_t safamily;

   switch (command) {
      case SOCKS_BIND:
      case SOCKS_UDPASSOCIATE:
         switch (reqhost->atype) {
            case SOCKS_ADDR_IPV4:
            case SOCKS_ADDR_IPV6:
               safamily = atype2safamily(reqhost->atype);
               break;

            case SOCKS_ADDR_DOMAIN: {
               struct sockaddr_storage p;

               sockshost2sockaddr(reqhost, &p);
               if (IPADDRISBOUND(&p))
                  safamily = p.ss_family;
               else
                  safamily = client->ss_family;

               break;
            }

            default:
               SERRX(reqhost->atype);
         }
         break;

      case SOCKS_CONNECT: {
         struct sockaddr_storage p;

         sockshost2sockaddr(reqhost, &p);
         if (IPADDRISBOUND(&p))
            safamily = p.ss_family;
         else
            safamily = client->ss_family;

         break;
      }

      default:
         SERRX(command);
   }

   if (external_has_safamily(safamily))
      return safamily;

   /*
    * Do not have the optimal safamily.  Anything else we can try?
    */
   switch (safamily) {
      case AF_INET:
         if (external_has_safamily(AF_INET6))
            return AF_INET6;

         break;

      case AF_INET6:
         if (external_has_safamily(AF_INET))
            return AF_INET;

         break;

      default:
         SERRX(safamily);
   }

   swarnx("%s: strange ... could not find any address to bind on external side "
          "for command %s from client %s.  Reqhost is %s.  "
          "Have IPv4? %s.  IPv6? %s",
          function,
          command2string(command),
          sockaddr2string(client, NULL, 0),
          sockshost2string(reqhost, NULL, 0),
          external_has_safamily(AF_INET)  ? "Yes" : "No",
          external_has_safamily(AF_INET6) ?
                 external_has_global_safamily(AF_INET6) ?
                    "Yes (global)" : "Yes (local only)"
              :  "No");

   return AF_UNSPEC;
}

static int
addrscope_matches(addr, scope)
   const struct sockaddr_in6 *addr;
   const ipv6_addrscope_t scope;
{
   const char *function = "addrscope_matches()";
   const ipv6_addrscope_t addrscope = ipv6_addrscope(&addr->sin6_addr);

   if (addrscope == scope)
      return 1;

   if (scope      == addrscope_nodelocal
   &&  addrscope  == addrscope_global)
      /*
       * a locally configured global address should be able to connect
       * to any nodelocal address, but a linklocal address will not
       * necessarily be able to connect to a nodelocal one, for some
       * reason.  That at least appears to be the case on FreeBSD 9.1.
       */
      return 1;

   if (sockscf.option.debug)
      slog(LOG_DEBUG,
           "%s: skipping address %s with system scopeid "
           "0x%x (internal: %d/%s) while searching for "
           "address with internal scopeid %d/%s",
           function,
           sockaddr2string(TOCSS(addr), NULL, 0),
           addr->sin6_scope_id,
           ipv6_addrscope(&addr->sin6_addr),
           addrscope2string(ipv6_addrscope(&addr->sin6_addr)),
           (int)scope,
           addrscope2string(scope));

   return 0;
}

static int
ifname_matches(ifname, ifindex)
   const char *ifname;
   const uint32_t ifindex;
{
   const char *function = "ifname_matches()";
   uint32_t ifnameindex;
   int matches;

   if ((ifnameindex = if_nametoindex(ifname)) == 0) {
      swarn("%s: if_nametoindex(%s) failed", function, ifname);
      return 0;
   }

   matches = (ifnameindex == ifindex);

   slog(LOG_DEBUG, "%s: ifname %s/ifindex %u %s ifindex %u",
        function,
        ifname,
        ifnameindex,
        matches ? "matches" : "does not match",
        ifindex);

   return matches;
}
