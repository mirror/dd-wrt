/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014, 2016, 2019, 2020, 2024
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
"$Id: config.c,v 1.464.4.2.2.3.4.11.4.3 2024/11/21 10:22:42 michaels Exp $";

void
genericinit(void)
{
   const char *function = "genericinit()";
#if BAREFOOTD
   rule_t *rule;
#endif /* BAREFOOTD */
#if !SOCKS_CLIENT
   sigset_t set, oset;
#endif /* !SOCKS_CLIENT */

#if SOCKS_CLIENT
   SASSERTX(sockscf.loglock == -1);
#else /* !SOCKS_CLIENT */
   if (sockscf.loglock == -1) {
      if ((sockscf.loglock = socks_mklock(SOCKS_LOCKFILE, NULL, 0)) == -1)
         serr("%s: socks_mklock() failed to create lockfile using base %s",
              function, SOCKS_LOCKFILE);
   }
#endif /* !SOCKS_CLIENT */

   if (!sockscf.state.inited) {
#if !HAVE_SETPROCTITLE
      /* create a backup to avoid setproctitle replacement overwriting it. */
      if ((__progname = strdup(__progname)) == NULL)
         serrx("%s: %s", function, NOMEM);
#endif /* !HAVE_SETPROCTITLE */
   }

#if !SOCKS_CLIENT
   sigemptyset(&set);
   sigaddset(&set, SIGHUP);
   sigaddset(&set, SIGTERM);
   if (sigprocmask(SIG_BLOCK, &set, &oset) != 0)
      swarn("%s: sigprocmask(SIG_BLOCK)", function);
#endif /* !SOCKS_CLIENT */

   optioninit();

   if (parseconfig(sockscf.option.configfile) != 0)
      return;

#if !SOCKS_CLIENT
   if (sigprocmask(SIG_SETMASK, &oset, NULL) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK)", function);
#endif /* !SOCKS_CLIENT */

   postconfigloadinit();

#if BAREFOOTD
   rule = sockscf.crule;
   while (rule != NULL) {
      if (rule->state.protocol.udp) {
         sockscf.state.alludpbounced = 0;
         break;
      }

      rule = rule->next;
   }
#endif /* BAREFOOTD */

#if SOCKSLIBRARY_DYNAMIC
   symbolcheck();
#endif /* SOCKSLIBRARY_DYNAMIC */
}

void
postconfigloadinit(void)
{
   const char *function = "postconfigloadinit()";
#if !SOCKS_CLIENT
   cpusetting_t newcpu;
   void (*procfunc)(void);
#endif /* !SOCKS_CLIENT */

   slog(LOG_DEBUG, "%s: I am a %s",
        function,
#if SOCKS_CLIENT
        "client"
#else /* !SOCKS_CLIENT */
        childtype2string(sockscf.state.type)
#endif /* !SOCKS_CLIENT */
        );

#if !HAVE_NO_RESOLVESTUFF
   if (!(_res.options & RES_INIT)) {
      res_init();

#if !SOCKS_CLIENT
      sockscf.initial.res_options = _res.options;
#endif /* !SOCKS_CLIENT */
   }
#endif /* !HAVE_NO_RESOLVSTUFF */

   switch (sockscf.resolveprotocol) {
      case RESOLVEPROTOCOL_TCP:
#if HAVE_NO_RESOLVESTUFF
         SERRX(sockscf.resolveprotocol);

#else /* !HAVE_NO_RESOLVESTUFF */
         _res.options |= RES_USEVC;
         slog(LOG_DEBUG,
              "%s: configured resolver for resolving over tcp", function);
#endif /* HAVE_NO_RESOLVESTUFF */

         break;

      case RESOLVEPROTOCOL_UDP:
      case RESOLVEPROTOCOL_FAKE:
         break;

      default:
         SERRX(sockscf.resolveprotocol);
   }

#if !HAVE_NO_RESOLVESTUFF && 0
   _res.options |= RES_DEBUG;
   slog(LOG_DEBUG, "%s: configured resolver for debugging", function);
#endif /* !HAVE_NO_RESOLVESTUFF && 0 */

#if !SOCKS_CLIENT

   switch (sockscf.state.type) {
      case PROC_MOTHER:
         newcpu   = sockscf.cpu.mother;
         procfunc = mother_postconfigload;
         break;

      case PROC_MONITOR:
         newcpu   = sockscf.cpu.monitor;
         procfunc = monitor_postconfigload;
         break;

      case PROC_NEGOTIATE:
         newcpu   = sockscf.cpu.negotiate;
         procfunc = negotiate_postconfigload;
         break;

      case PROC_REQUEST:
         newcpu   = sockscf.cpu.request;
         procfunc = request_postconfigload;
         break;

      case PROC_IO:
         newcpu   = sockscf.cpu.io;
         procfunc = io_postconfigload;
         break;

      default:
         SERRX(sockscf.state.type);
   }

#if HAVE_SCHED_SETSCHEDULER
   if (!newcpu.scheduling_isset) {
      newcpu.policy           = sockscf.initial.cpu.policy;
      newcpu.param            = sockscf.initial.cpu.param;
      newcpu.scheduling_isset = sockscf.initial.cpu.scheduling_isset;
   }
#endif /* HAVE_SCHED_SETSCHEDULER */

#if HAVE_SCHED_SETAFFINITY
   if (!newcpu.affinity_isset) {
      newcpu.mask           = sockscf.initial.cpu.mask;
      newcpu.affinity_isset = sockscf.initial.cpu.affinity_isset;
   }
#endif /* HAVE_SCHED_SETAFFINITY */

   if (!sockscf.option.verifyonly)
      sockd_setcpusettings(&sockscf.state.cpu, &newcpu);

#if HAVE_SCHED_SETSCHEDULER
   /*
    * does not work on FreeBSD version 8.2 (only one checked).
    * For some reason the sched_priority, as returned by sched_getparam(2),
    * after we receive a signal (e.g. SIGHUP) is not the same it was before
    * the signal, as well as other weirdness.  There are some bug-reports
    * related to what looks similar e.g.: kern/157657.  That bug has apparently
    * been fixed in in a commit in June 2011, after the FreeBSD 8.2 release
    * running on our testmachine.
    */
   struct sched_param param;

   SASSERTX(sockscf.state.cpu.policy               == sched_getscheduler(0));
   SASSERTX(sched_getparam(0, &param)              == 0);
   SASSERTX(sockscf.state.cpu.param.sched_priority == param.sched_priority);
#endif /* HAVE_SCHED_SETSCHEDULER */

   if (procfunc != NULL)
      procfunc();
#endif /* !SOCKS_CLIENT */
}


route_t *
socks_addroute(newroute, last)
   const route_t *newroute;
   const int last;
{
   const char *function = "socks_addroute()";
   serverstate_t nilstate;
   route_t *route, *nextroute;
   size_t i, ifb, isoneshot;

   if ((route = malloc(sizeof(*route))) == NULL)
      yyerrorx("could not allocate %lu bytes for route",
               (unsigned long)sizeof(*nextroute));

   *route = *newroute;
   bzero(&nilstate, sizeof(nilstate));

   if (route->gw.state.proxyprotocol.upnp) {
      if (route->gw.addr.atype == SOCKS_ADDR_DOMAIN
      &&  strcmp(route->gw.addr.addr.domain, PROXY_BROADCASTs) == 0)
         ;
      else {
         if (route->gw.addr.atype != SOCKS_ADDR_IFNAME
         &&  route->gw.addr.atype != SOCKS_ADDR_URL)
            yyerrorx("gateway for upnp proxy has to be an interface or url.  "
                     "The %s %s is not a valid gateway",
                     atype2string(route->gw.addr.atype),
                     sockshost2string(&route->gw.addr, NULL, 0));

         if (route->gw.addr.port == htons(0)) {
            if (route->gw.addr.atype != SOCKS_ADDR_URL) {
               slog(LOG_DEBUG, "%s: upnp gw port not set, using default (%d)",
                    function, DEFAULT_SSDP_PORT);

               route->gw.addr.port = htons(DEFAULT_SSDP_PORT);
            }
         }
         else if (route->gw.addr.port != htons(DEFAULT_SSDP_PORT))
            yyerrorx("sorry, the upnp library %s is tested with does"
                     "not support setting the upnp/ssdp port",
                     PRODUCT);
      }
   }
   else {
      switch (route->gw.addr.atype) {
         case SOCKS_ADDR_IPV4:
#if !SOCKS_CLIENT
         case SOCKS_ADDR_IPV6:
#endif /* !SOCKS_CLIENT */
            break;

         case SOCKS_ADDR_DOMAIN:
            if (memcmp(&nilstate.proxyprotocol,
                       &route->gw.state.proxyprotocol,
                       sizeof(nilstate.proxyprotocol)) == 0) {
               if (strcmp(route->gw.addr.addr.domain, PROXY_DIRECTs) == 0)
                  route->gw.state.proxyprotocol.direct = 1;
               else if (strcmp(route->gw.addr.addr.domain, PROXY_BROADCASTs)
               == 0)
                  route->gw.state.proxyprotocol.upnp = 1;
            }
            break;

         default:
            yyerrorx("gateway must be an IP-address or qualified domainname, "
                     "but type of %s is %s",
                     sockshost2string(&route->gw.addr, NULL, 0),
                     atype2string(route->gw.addr.atype));
      }
   }

   /* if no proxy protocol set, set socks v4 and v5. */
   if (memcmp(&nilstate.proxyprotocol, &route->gw.state.proxyprotocol,
   sizeof(nilstate.proxyprotocol)) == 0) {
      memset(&route->gw.state.proxyprotocol, 0,
      sizeof(route->gw.state.proxyprotocol));

      route->gw.state.proxyprotocol.socks_v4 = 1;
      route->gw.state.proxyprotocol.socks_v5 = 1;
   }
   else { /* proxy protocol set, do they make sense? */
      proxyprotocol_t proxy;

      if (route->gw.state.proxyprotocol.socks_v4
      ||  route->gw.state.proxyprotocol.socks_v5) {
         if (route->gw.state.proxyprotocol.http
         ||  route->gw.state.proxyprotocol.upnp)
         yyerrorx("%s: cannot combine proxyprotocol socks with other "
                  "proxyprotocols",
                  function);
      }
      else if (route->gw.state.proxyprotocol.http) {
         memset(&proxy, 0, sizeof(proxy));
         proxy.http = 1;

         if (memcmp(&proxy, &route->gw.state.proxyprotocol, sizeof(proxy)) != 0)
            yyerrorx("%s: cannot combine proxyprotocol http with other "
                     "proxyprotocols",
                     function);
      }
      else if (route->gw.state.proxyprotocol.upnp) {
#if !HAVE_LIBMINIUPNP
         serrx("%s: not configured for using upnp", function);
#else /* HAVE_LIBMINIUPNP */

         memset(&proxy, 0, sizeof(proxy));
         proxy.upnp = 1;

         if (memcmp(&proxy, &route->gw.state.proxyprotocol, sizeof(proxy)) != 0)
            yyerrorx("%s: cannot combine proxyprotocol upnp with other "
                     "proxyprotocols",
                     function);
#endif /* HAVE_LIBMINIUPNP */
      }
   }

   if (route->gw.addr.port == htons(0)) { /* no port, set default if known. */
      if (route->gw.state.proxyprotocol.socks_v4
      ||  route->gw.state.proxyprotocol.socks_v5)
         route->gw.addr.port = htons((in_port_t)SOCKD_PORT);

      else if (route->gw.state.proxyprotocol.http)
         route->gw.addr.port = htons((in_port_t)SOCKD_HTTP_PORT);
   }

   if (memcmp(&nilstate.command, &route->gw.state.command,
   sizeof(nilstate.command)) == 0) {
      if (route->gw.state.proxyprotocol.direct) {
#if SOCKS_CLIENT
         route->gw.state.command.udpassociate   = 1;
         route->gw.state.command.udpreply       = 1;
#endif /* SOCKS_CLIENT */
         route->gw.state.command.connect        = 1;

         /*
          * in a normal client configuration, it makes more sense
          * to not enable bind for direct routes, unless the user
          * explicitly enables it.
          * If not, bind(2) will always be local, which in most
          * cases is probably not what the user wanted, even
          * though he implied it by not specifying what commands
          * the direct route should handle, meaning "all".
          */
         route->gw.state.command.bind            = 0;
         route->gw.state.command.bindreply       = 0;
      }

      /*
       * Now go through the proxy protocol(s) supported by this route,
       * and enable the appropriate protocols and commands, if the
       * user has not already done so.
       */
      if (route->gw.state.proxyprotocol.socks_v5) {
#if SOCKS_CLIENT
         route->gw.state.command.udpassociate  = 1;
         route->gw.state.command.udpreply      = 1;
         route->gw.state.command.bind          = 1;
         route->gw.state.command.bindreply     = 1;
#endif /* SOCKS_CLIENT */
         route->gw.state.command.connect       = 1;
      }

      if (route->gw.state.proxyprotocol.socks_v4) {
#if SOCKS_CLIENT
         route->gw.state.command.bind       = 1;
         route->gw.state.command.bindreply  = 1;
#endif /* SOCKS_CLIENT */
         route->gw.state.command.connect    = 1;
      }

      if (route->gw.state.proxyprotocol.http) {
         route->gw.state.command.connect = 1;
      }

      if (route->gw.state.proxyprotocol.upnp) {
#if SOCKS_CLIENT
         route->gw.state.command.udpassociate = 1;
         route->gw.state.command.udpreply     = 1;
         route->gw.state.command.bind         = 1;
         route->gw.state.command.bindreply    = 1;
#endif /* SOCKS_CLIENT */
         route->gw.state.command.connect      = 1;
      }

   }
#if !SOCKS_CLIENT
   else {
      if (!route->gw.state.proxyprotocol.direct) {
         if (route->gw.state.command.bind
         ||  route->gw.state.command.bindreply
         ||  route->gw.state.command.udpassociate
         ||  route->gw.state.command.udpreply
         ||  route->gw.state.protocol.udp)
            yyerrorx("serverchaining only supported for the connect command");
      }
   }
#endif /* !SOCKS_CLIENT */

   if (memcmp(&nilstate.protocol, &route->gw.state.protocol,
   sizeof(nilstate.protocol)) == 0) {
      if (route->gw.state.proxyprotocol.direct) {
         route->gw.state.protocol.udp = 1;
         route->gw.state.protocol.tcp = 1;
      }

      if (route->gw.state.proxyprotocol.socks_v5) {
#if SOCKS_CLIENT
         route->gw.state.protocol.udp = 1;
#endif /* SOCKS_CLIENT */
         route->gw.state.protocol.tcp = 1;
      }

      if (route->gw.state.proxyprotocol.socks_v4)
         route->gw.state.protocol.tcp = 1;

      if (route->gw.state.proxyprotocol.http)
         route->gw.state.protocol.tcp = 1;

      if (route->gw.state.proxyprotocol.upnp) {
#if SOCKS_CLIENT
         route->gw.state.protocol.udp = 1;
#endif /* SOCKS_CLIENT */
         route->gw.state.protocol.tcp = 1;
      }
   }

#if HAVE_GSSAPI
   /*
    * if no gssapienctype set, or only nec-compatibility set,
    * set all except per-message.
    */
   if (route->gw.state.gssapiencryption.clear            == 0
   &&  route->gw.state.gssapiencryption.integrity        == 0
   &&  route->gw.state.gssapiencryption.confidentiality  == 0
   &&  route->gw.state.gssapiencryption.permessage       == 0) {
      route->gw.state.gssapiencryption.integrity       = 1;
      route->gw.state.gssapiencryption.confidentiality = 1;
      route->gw.state.gssapiencryption.permessage      = 0;
   }

   /* if no gssapiservicename set, set to default. */
   if (strcmp((char *)&nilstate.gssapiservicename,
   (char *)&route->gw.state.gssapiservicename) == 0)
      STRCPY_ASSERTSIZE(route->gw.state.gssapiservicename,
                       DEFAULT_GSSAPISERVICENAME);

   /* if no gssapiservicename set, set to default. */
   if (strcmp((char *)&nilstate.gssapikeytab,
   (char *)&route->gw.state.gssapikeytab) == 0)
      STRCPY_ASSERTSIZE(route->gw.state.gssapikeytab, DEFAULT_GSSAPIKEYTAB);
#endif /* HAVE_GSSAPI */

   /* if no method is set, set all we support for the proxyprotocols. */
   if (route->gw.state.smethodc == 0) {
      int *methodv    =  route->gw.state.smethodv;
      size_t *methodc = &route->gw.state.smethodc;

      /* all proxyprotocols support this. */
      methodv[(*methodc)++] = AUTHMETHOD_NONE;

#if SOCKS_CLIENT
/*
 * currently only supported for routes in the  client, not for
 * serverchaining.
 */

#if HAVE_GSSAPI
      if (route->gw.state.proxyprotocol.socks_v5)
         methodv[(*methodc)++] = AUTHMETHOD_GSSAPI;
#endif /* HAVE_GSSAPI */

#endif /* SOCKS_CLIENT */

#if SOCKS_CLIENT
      /*
       * If it's a socks_v5 route, we could set AUTMHETHOD_UNAME in the
       * server case also, but that means we forward the clients password
       * to an upstream proxy.
       * Don't do that by default, but insist the operator configures
       * this route as such if he really wants us to do that.
       */
      if (route->gw.state.proxyprotocol.socks_v5)
         methodv[(*methodc)++] = AUTHMETHOD_UNAME;
#endif
   }

   /* Checks the methods set make sense for the given proxy protocols. */
   for (i = 0; i < route->gw.state.smethodc; ++i)
      switch (route->gw.state.smethodv[i]) {
         case AUTHMETHOD_NONE:
            break;

#if !SOCKS_CLIENT

         case AUTHMETHOD_GSSAPI:
            yyerrorx("sorry, %s authentication is not supported for "
                     "serverchaining",
                     method2string(route->gw.state.smethodv[i]));

#else /* SOCKS_CLIENT */

         case AUTHMETHOD_GSSAPI:

#endif /* SOCKS_CLIENT */

         case AUTHMETHOD_UNAME:
            if (!route->gw.state.proxyprotocol.socks_v5)
               yyerrorx("rule specifies method %s, but that is not supported "
                        "by the specified proxy protocol(s): %s",
                        method2string(route->gw.state.smethodv[i]),
                        proxyprotocols2string(&route->gw.state.proxyprotocol,
                                              NULL,
                                              0));

#if !SOCKS_CLIENT
            if (route->gw.state.smethodv[i] == AUTHMETHOD_UNAME) {
               size_t j, have_username_methods;


               have_username_methods = 0;
               for (j = 0; j < sockscf.smethodc; ++j) {
                  if (methodcanprovide(sockscf.smethodv[j], username)) {
                     have_username_methods = 1;
                     break;
                  }
               }

               if (!have_username_methods)
                  yyerrorx("route specifies method %s in the proxychain "
                           "route, but that can't work because no methods "
                           "that can provide a username are set in the "
                           "global socksmethod list; no client using "
                           "username/password authentication will ever be "
                           "accepted by us",
                           method2string(route->gw.state.smethodv[i]));

               yylog(LOG_NOTICE,
                     "this serverchain route specifies authmethod %s, "
                     "indicating that we should forward username/password-"
                     "credentials received by our own user to an upstream "
                     "proxy.  Unless the upstream proxy is also under your "
                     "control, this may not be what you want to do",
                     method2string(route->gw.state.smethodv[i]));
            }
#endif /* !SOCKS_CLIENT */

            break;

         case AUTHMETHOD_BSDAUTH:
         case AUTHMETHOD_PAM_ANY:
         case AUTHMETHOD_PAM_ADDRESS:
         case AUTHMETHOD_PAM_USERNAME:
         case AUTHMETHOD_LDAPAUTH:
         case AUTHMETHOD_RFC931:
            yyerrorx("method %s is only valid for ACL rules",
                     method2string(route->gw.state.smethodv[i]));
            break; /* NOTREACHED */

         default:
            SERRX(route->gw.state.smethodv[i]);
      }

   if (route->src.atype == SOCKS_ADDR_IFNAME)
      yyerrorx("interface names not supported for src address");

#if SOCKS_CLIENT
   if (route->rdr_from.atype == SOCKS_ADDR_IPV4) {
      if (route->rdr_from.addr.ipv4.mask.s_addr != htonl(IPV4_FULLNETMASK))
         yyerror("netmask for redirect from address must be %d, not %d",
                 IPV4_FULLNETMASK,
                bitcount((unsigned long)route->rdr_from.addr.ipv4.mask.s_addr));
   }

   if (route->rdr_from.atype == SOCKS_ADDR_IPV6) {
      if (route->rdr_from.addr.ipv6.maskbits != IPV6_NETMASKBITS)
         yyerror("netmask for redirect from address must be %d, not %d",
                 IPV6_NETMASKBITS, route->rdr_from.addr.ipv6.maskbits);
   }
#endif /* SOCKS_CLIENT */

   isoneshot = (route->dst.atype == SOCKS_ADDR_IFNAME ? 0 : 1);
   ifb       = 0;
   nextroute = NULL;
   while (1) {
      /*
       * This needs to be a loop to handle the case where route->dst
       * expands to multiple ip addresses, which can happen when it is
       * e.g. a ifname with several addresses configured on it.  In that
       * case want to add almost identical routes for all the addresses
       * configured on the interface, with the only difference being
       * the "dst" field (one dst for each address/mask on the interface).
       */
      struct sockaddr_storage addr, mask;

      if (route->dst.atype == SOCKS_ADDR_IFNAME) {
         if (ifname2sockaddr(route->dst.addr.ifname, ifb, &addr, &mask)
         == NULL) {
            if (ifb == 0) {
               char visbuf[MAXIFNAMELEN * 4];

               yyerrorx("could not create route with destination interface %s: "
                        "no addresses found on interface",
                        str2vis(route->dst.addr.ifname,
                                strlen(route->dst.addr.ifname),
                                visbuf,
                                sizeof(visbuf)));
            }

            break; /* no more addresses on this interface. */
         }
      }

      if (nextroute == NULL)
         nextroute = route; /* first iteration. */
      else {
         if ((nextroute = malloc(sizeof(*nextroute))) == NULL)
            yyerrorx("could not allocate %lu bytes for route",
                     (unsigned long)sizeof(*nextroute));

         *nextroute = *route;/* stays identical to original except dst addr. */
      }

      if (route->dst.atype == SOCKS_ADDR_IFNAME) {
         SASSERTX(nextroute->dst.atype == SOCKS_ADDR_IFNAME);

         sockaddr2ruleaddr(&addr, &nextroute->dst);

         SASSERTX(nextroute->dst.atype != SOCKS_ADDR_IFNAME);

         switch (addr.ss_family) {
            case AF_INET:
               nextroute->dst.addr.ipv4.mask = TOIN(&mask)->sin_addr;
               break;

            case AF_INET6:
               nextroute->dst.addr.ipv6.maskbits
               = bitcount_in6addr(&TOIN6(&mask)->sin6_addr);
               break;

            default:
               SERRX(addr.ss_family);
         }
      }

      /*
       * place next route in list.  Last or first?
       */
      if (!last || sockscf.route == NULL) { /* first */
         route_t *p;

         nextroute->next = sockscf.route;
         sockscf.route   = nextroute;

         if (nextroute->state.autoadded)
            nextroute->number = 0;
         else
            if (ifb == 0) {
               /*
                * only update following route numbers for first
                * ip-block on interface.
                */
               for (i = 1, p = sockscf.route; p != NULL; p = p->next, ++i)
                  p->number = (int)i;
            }
      }
      else { /* last */
         route_t *lastroute;

         lastroute = sockscf.route;
         if (nextroute->state.autoadded)
            nextroute->number = 0;
         else {
            while (lastroute->next != NULL)
               lastroute = lastroute->next;

            if (ifb == 0)
               /*
                * only update route numbers for first
                * ip-block on interface.
                */
               nextroute->number = lastroute->number + 1;
         }

         lastroute->next = nextroute;
         nextroute->next = NULL;
      }

      if (isoneshot)
         break;

      ++ifb;
   }

   if (!route->gw.state.proxyprotocol.direct
   && !(    route->gw.state.proxyprotocol.upnp
         && route->gw.addr.atype == SOCKS_ADDR_DOMAIN
         &&  strcmp(route->gw.addr.addr.domain, PROXY_BROADCASTs) == 0)) {
      /*
       * A proxyserver, so make sure we add a direct route to it also.
       */
      struct sockaddr_storage saddr, smask;
      command_t commands;
      protocol_t protocols;

      bzero(&commands, sizeof(commands));
      bzero(&protocols, sizeof(protocols));

      bzero(&smask, sizeof(smask));
      SET_SOCKADDR(&smask, AF_INET);
      TOIN(&smask)->sin_port        = htons(0);
      TOIN(&smask)->sin_addr.s_addr = htonl(IPV4_FULLNETMASK);

      if (route->gw.state.proxyprotocol.upnp
      &&  route->gw.addr.atype == SOCKS_ADDR_IFNAME) {
         /*
          * Add direct route for the SSDP broadcast addr, only reachable
          * by lan, so should always be there.
          */
         static int already_done;

         if (!already_done) {
            struct servent *service;

            bzero(&saddr, sizeof(saddr));
            SET_SOCKADDR(&saddr, AF_INET);
            if (socks_inet_pton(AF_INET,
                                DEFAULT_SSDP_BROADCAST_IPV4ADDR,
                                &TOIN(&saddr)->sin_addr.s_addr,
                                NULL) != 1)
               serr("%s: inet_pton(3): could not convert %s to IPv4 address",
                    function, DEFAULT_SSDP_BROADCAST_IPV4ADDR);

            if ((service = getservbyname("ssdp", "udp")) == NULL)
               TOIN(&saddr)->sin_port = htons(DEFAULT_SSDP_PORT);
            else
               TOIN(&saddr)->sin_port = (in_port_t)service->s_port;

            protocols.udp = 1;

            socks_autoadd_directroute(&commands, &protocols, &saddr, &smask);

            already_done = 1;
         }
      }
      else {
         sockshost2sockaddr(&route->gw.addr, &saddr);

         commands.connect = 1;
         protocols.tcp    = 1;

         socks_autoadd_directroute(&commands, &protocols, &saddr, &smask);
      }
   }

   return route;
}

route_t *
socks_autoadd_directroute(command, protocol, saddr, netmask)
   const command_t *command;
   const protocol_t *protocol;
   const struct sockaddr_storage *saddr;
   const struct sockaddr_storage *netmask;
{
   route_t route;

   memset(&route, 0, sizeof(route));

   route.src.atype                            = SOCKS_ADDR_IPV4;
   route.src.operator                         = none;

   route.dst.atype                            = SOCKS_ADDR_IPV4;
   route.dst.addr.ipv4.ip                     = TOCIN(saddr)->sin_addr;
   route.dst.addr.ipv4.mask.s_addr            = TOCIN(netmask)->sin_addr.s_addr;
   route.dst.port.tcp = route.dst.port.udp    = TOCIN(saddr)->sin_port;
   route.dst.operator                         = htons(TOCIN(saddr)->sin_port)
                                                == 0 ? none : eq;

   route.gw.addr.atype                        = SOCKS_ADDR_DOMAIN;
   STRCPY_ASSERTSIZE(route.gw.addr.addr.domain, "direct");

   route.gw.state.command                     = *command;
   route.gw.state.protocol                    = *protocol;

   route.gw.state.proxyprotocol.direct        = 1;

   route.state.autoadded                      = 1;

   return socks_addroute(&route, 0);
}

route_t *
socks_getroute(req, src, dst)
   const request_t *req;
   const sockshost_t *src;
   const sockshost_t *dst;
{
   const char *function = "socks_getroute()";
   route_t *route;
   char srcbuf[MAXSOCKSHOSTSTRING], dstbuf[MAXSOCKSHOSTSTRING];

#if SOCKS_CLIENT
   clientinit();
#endif /* SOCKS_CLIENT */

   slog(LOG_DEBUG,
        "%s: searching for %s route for %s, protocol %s, src %s, dst %s, ...",
        function,
        proxyprotocol2string(req->version),
        command2string(req->command), protocol2string(req->protocol),
        src == NULL ? "<NONE>" : sockshost2string(src, srcbuf, sizeof(srcbuf)),
        dst == NULL ? "<NONE>" : sockshost2string(dst, dstbuf, sizeof(dstbuf)));

   for (route = sockscf.route; route != NULL; route = route->next) {
      socks_showroute(route);

      if (sockscf.routeoptions.maxfail != 0
      &&  route->state.failed >= sockscf.routeoptions.maxfail) {
         if (sockscf.routeoptions.badexpire == 0
         ||  socks_difftime(time_monotonic(NULL), route->state.badtime)
             <= sockscf.routeoptions.badexpire) {
            slog(LOG_DEBUG, "%s: route does not match; badtime", function);
            continue;
         }
         else
            route->state.failed = 0; /* reset. */
      }

      switch (req->version) {
         /*
          * First check if this rule can provide requested proxyprotocol
          * version with necessary functionality.
          */

         case PROXY_SOCKS_V4:
            if (!route->gw.state.proxyprotocol.socks_v4) {
               slog(LOG_DEBUG, "%s: route does not match; version", function);
               continue;
            }

            switch (req->host.atype) {
               case SOCKS_ADDR_IPV4:
                  break;

               default:
                  slog(LOG_DEBUG, "%s: route does not match; atype", function);
                  continue;  /* not supported by v4. */
            }

            switch (req->command) {
               case SOCKS_BIND:
               case SOCKS_CONNECT:
                  break;

               default:
                  slog(LOG_DEBUG, "%s: route does not match; cmd", function);
                  continue; /* not supported by v4. */
            }

            break;

         case PROXY_SOCKS_V5:
            if (!route->gw.state.proxyprotocol.socks_v5) {
               slog(LOG_DEBUG, "%s: route does not match; version", function);
               continue;
            }

            switch (req->host.atype) {
               case SOCKS_ADDR_IPV4:
               case SOCKS_ADDR_IPV6:
               case SOCKS_ADDR_DOMAIN:
                  break;

               default:
                  SERRX(req->host.atype); /* failure, nothing else exists. */
            }
            break;

         case PROXY_HTTP_10:
         case PROXY_HTTP_11:
            if (!route->gw.state.proxyprotocol.http) {
               slog(LOG_DEBUG, "%s: route does not match; version", function);
               continue;
            }

            switch (req->command) {
               case SOCKS_CONNECT:
                  break;

               default:
                  slog(LOG_DEBUG, "%s: route does not match; cmd", function);
                  continue; /* not supported by http. */
            }

            break;

         case PROXY_UPNP:
            if (!route->gw.state.proxyprotocol.upnp) {
               slog(LOG_DEBUG, "%s: route does not match; version", function);
               continue;
            }
            break;

         case PROXY_DIRECT:
            if (!route->gw.state.proxyprotocol.direct) {
               slog(LOG_DEBUG, "%s: route does not match; version", function);
               continue;
            }
            break;

         default:
            SERRX(req->version);
      }

      switch (req->command) {
         case SOCKS_BIND:
            if (!route->gw.state.command.bind) {
               slog(LOG_DEBUG, "%s: route does not match; cmd", function);
               continue;
            }

            /*
             * Need to check protocol and proxyprotocol also.  Even if
             * bind is supported for one protocol (e.g., tcp) it does
             * not mean it is supported for another protocol (e.g., udp).
             */
            switch (req->version) {
               case PROXY_SOCKS_V4:
               case PROXY_SOCKS_V5:
                  if (req->protocol == SOCKS_TCP)
                     break; /* yes, supported. */

                  /* Else: not supported. */

                  SASSERTX(req->protocol == SOCKS_UDP);
                  /* FALLTHROUGH */

               case PROXY_HTTP_10:
               case PROXY_HTTP_11:
                  slog(LOG_DEBUG,
                       "%s: route does not match; bind command is not "
                       "supported by proxyprotocol",
                       function);

                  continue;

               case PROXY_UPNP:
               case PROXY_DIRECT:
                  break;

               default:
                  SERRX(req->version);
            }

            break;

         case SOCKS_CONNECT:
            if (!route->gw.state.command.connect) {
               slog(LOG_DEBUG, "%s: route does not match; cmd", function);
               continue;
            }
            break;

         case SOCKS_UDPASSOCIATE:
            if (!route->gw.state.command.udpassociate) {
               slog(LOG_DEBUG, "%s: route does not match; cmd", function);
               continue;
            }
            break;

         default:
            SERRX(req->command);
      }

      /*
       * server supports protocol?
       */
      if ((req->protocol == SOCKS_TCP && !route->gw.state.protocol.tcp)
      ||  (req->protocol == SOCKS_UDP && !route->gw.state.protocol.udp)) {
         slog(LOG_DEBUG, "%s: route does not match; protocol", function);
         continue;
      }

      /*
       * server supports method?
       */
      switch (req->version) {
         /*
          * These proxyprotocols do not support authentication, or
          * we do not support the authentication part of them.
          * So if the requested proxyprotocol is one of the below, the
          * the route must not require authentication if we should be
          * able to use it.  If it does not, there is no point in
          * checking what auth we have set for the client (if any).
          */
         case PROXY_DIRECT:
         case PROXY_HTTP_10:
         case PROXY_HTTP_11:
         case PROXY_SOCKS_V4:
         case PROXY_UPNP:
            if (!methodisset(AUTHMETHOD_NONE,
                             route->gw.state.smethodv,
                             route->gw.state.smethodc)) {
               slog(LOG_DEBUG, "%s: route does not match; method", function);
               continue;
            }

            break;

         case PROXY_SOCKS_V5: /* v5 supports all methods we can support. */
            break;

         default:
            SERRX(req->version);
      }

      if (req->auth != NULL && req->auth->method != AUTHMETHOD_NOTSET) {
         /*
          * authmethod is already set.  Must be serverchaining.
          */
         SASSERTX(!SOCKS_CLIENT);

         slog(LOG_DEBUG,
              "%s: authmethod already set to %s.  Checking route for support",
              function, method2string(req->auth->method));

         if (!methodisset(req->auth->method,
                          route->gw.state.smethodv,
                          route->gw.state.smethodc)) {
            if (methodisset(AUTHMETHOD_NONE,
                             route->gw.state.smethodv,
                             route->gw.state.smethodc)) {
               slog(LOG_DEBUG,
                    "%s: route #%lu supports authmethod %s, meaning it "
                    "should work regardless of the authinfo we have "
                    "received from the client",
                    function,
                    (unsigned long)route->number,
                    method2string(AUTHMETHOD_NONE));
            }
            else {
               slog(LOG_DEBUG, "%s: route does not match; method", function);
               continue;
            }
         }
      }

      if (src != NULL) {
         slog(LOG_DEBUG, "%s: checking for src match ...", function);
         if (!addrmatch(&route->src, src, NULL, req->protocol, 0)) {
            slog(LOG_DEBUG, "%s: route does not match; src addr", function);
            continue;
         }
      }

      if (dst != NULL) {
         slog(LOG_DEBUG, "%s: checking for dst match ...", function);
         if (!addrmatch(&route->dst, dst, NULL, req->protocol, 0)) {
            slog(LOG_DEBUG, "%s: route does not match; dst addr", function);
            continue;
         }
      }

      break;   /* all matched */
   }

   if (route == NULL)
      slog(LOG_DEBUG, "%s: no %s route found",
           function, proxyprotocol2string(req->version));
   else {
      slog(LOG_DEBUG, "%s: %s route found, route #%d",
           function, proxyprotocol2string(req->version), route->number);

      if (!route->gw.state.proxyprotocol.direct
      &&  dst != NULL) {
         /* simple but non-robust attempt at check for routing loop. */
         if (sockshostareeq(&route->gw.addr, dst))
            serrx("%s: route to gw %s is itself.  Route loop in config\n",
                  function, sockshost2string(&route->gw.addr, NULL, 0));
      }
   }

   return route;
}

int
socks_routesetup(control, data, route, emsg, emsglen)
   int control;
   int data;
   const route_t *route;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_routesetup()";
   struct sockaddr_storage controladdr, dataaddr;
   socklen_t len;
   int control_type, data_type, rc;

   if (route->rdr_from.atype == SOCKS_ADDR_NOTSET)
      return 0;

   if (control == -1)
      control = data;
   else if (data == -1)
      data = control;

   SASSERTX(control != -1);
   SASSERTX(data    != -1);

   len = sizeof(controladdr);
   if (getsockname(control, TOSA(&controladdr), &len) != 0) {
      snprintf(emsg, emsglen, "getsockname(2) on fd %d (control) failed: %s",
               control, strerror(errno));

      swarnx("%s: %s", function, emsg);
      return -1;
   }

   if (data == control)
      dataaddr = controladdr;
   else {
      len = sizeof(dataaddr);
      if (getsockname(data, TOSA(&dataaddr), &len) != 0) {
         snprintf(emsg, emsglen, "getsockname(2) on fd %d (data) failed: %s",
                  data, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return -1;
      }
   }

   len = sizeof(control_type);
   if (getsockopt(control, SOL_SOCKET, SO_TYPE, &control_type, &len) != 0) {
      snprintf(emsg, emsglen, "getsockopt(2) on fd %d (control) failed: %s",
               control, strerror(errno));

      swarnx("%s: %s", function, emsg);
      return -1;
   }

   if (data == control)
      data_type = control_type;
   else {
      len = sizeof(data_type);
      if (getsockopt(data, SOL_SOCKET, SO_TYPE, &data_type, &len) != 0) {
         snprintf(emsg, emsglen, "getsockopt(2) on fd %d (data) failed: %s",
                  data, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return -1;
      }
   }

   slog(LOG_DEBUG,
        "%s: control-fd: %d (%s), data-fd: %d (%s), proxyprotocols: %s, "
        "redirect from: %s",
        function,
        control,
        control_type == SOCK_STREAM ? "stream" : "dgram",
        data,
        data_type    == SOCK_STREAM ? "stream" : "dgram",
        proxyprotocols2string(&route->gw.state.proxyprotocol, NULL, 0),
        ruleaddr2string(&route->rdr_from, ADDRINFO_PORT, NULL, 0));

   rc = socks_rebind(control,
                     control_type == SOCK_STREAM ? SOCKS_TCP : SOCKS_UDP,
                     &controladdr,
                     &route->rdr_from,
                     emsg,
                     emsglen);

   if (rc != 0) {
      snprintf(emsg, emsglen, "socks_rebind() of control-fd %d failed: %s",
               control, strerror(errno));

      swarnx("%s: %s", emsg, function);
      return -1;
   }

   if (control == data)
      return 0;

   if (data_type == SOCK_DGRAM) { /* only support udp now. */
      if (socks_rebind(data,
                       data_type == SOCK_STREAM ? SOCKS_TCP : SOCKS_UDP,
                       &dataaddr,
                       &route->rdr_from,
                       emsg,
                       emsglen) != 0) {
         snprintf(emsg, emsglen, "rebind() of data-fd %d failed: %s",
                  data, strerror(errno));

         swarnx("%s: %s", emsg, function);
         return -1;
      }
   }

   return 0;
}

route_t *
socks_connectroute(s, packet, src, dst, emsg, emsglen)
   const int s;
   socks_t *packet;
   const sockshost_t *src;
   const sockshost_t *dst;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_connectroute()";
   route_t *route;
   char dststring[MAXSOCKSHOSTSTRING], gwstring[MAXSOCKSHOSTSTRING];
   int rc;

   slog(LOG_DEBUG, "%s: fd %d, command %s",
        function, s, command2string(packet->req.command));

   if ((route = socks_getroute(&packet->req, src, dst)) == NULL)
      SERRX(0); /* should only be called if there is a route. */

   slog(LOG_NEGOTIATE, "%s: have %s route (route #%d) to %s via %s",
        function,
        proxyprotocols2string(&route->gw.state.proxyprotocol, NULL, 0),
        route->number,
        dst == NULL ?
            "<UNKNOWN>" : sockshost2string(dst, dststring, sizeof(dststring)),
        sockshost2string(&route->gw.addr, gwstring, sizeof(gwstring)));

   if (route->gw.state.proxyprotocol.direct)
      return route; /* nothing to do. */

#if HAVE_LIBMINIUPNP
   if (route->gw.state.proxyprotocol.upnp) {
      if (route->gw.addr.atype == SOCKS_ADDR_DOMAIN
      &&  strcmp(route->gw.addr.addr.domain, PROXY_BROADCASTs) == 0) {
         /*
          * Interface igd is reachable on was not specified, so need to
          * try all interfaces to see which one we can use.
          */
         struct ifaddrs *ifap, *iface;
         gateway_t gw;

         if (getifaddrs(&ifap) == -1) {
            snprintf(emsg, emsglen,
                     "getifaddrs() failed to get list of network interfaces on "
                     "this machine via getifaddrs(3).  This is necessary for "
                     "supporting setting \"%s\" to the value \"%s\": %s",
                     ENV_UPNP_IGD, route->gw.addr.addr.domain, strerror(errno));

            swarnx("%s: %s", function, emsg);

            socks_blacklist(route, emsg);

            return NULL;
         }

         gw            = route->gw;
         gw.addr.atype = SOCKS_ADDR_IFNAME;

         for (iface = ifap; iface != NULL; iface = iface->ifa_next) {
            if (iface->ifa_addr                          == NULL
            ||  iface->ifa_addr->sa_family               != AF_INET
            ||  TOIN(iface->ifa_addr)->sin_addr.s_addr   == htonl(0)
            ||  !(iface->ifa_flags & (IFF_UP | IFF_MULTICAST))
            ||  iface->ifa_flags & (IFF_LOOPBACK | IFF_POINTOPOINT))
               continue;

            if (strlen(iface->ifa_name) >= sizeof(gw.addr.addr.ifname)) {
               swarn("%s: ifname \"%s\" is too long according to our "
                     "compile-time limit and will be skipped.  Max length: %lu",
                     function,
                     iface->ifa_name,
                     (unsigned long)sizeof(gw.addr.addr.ifname));

               continue;
            }

            STRCPY_ASSERTLEN(gw.addr.addr.ifname, iface->ifa_name);

            if (socks_initupnp(&gw, emsg, emsglen) == 0) {
               slog(LOG_NEGOTIATE, "%s: socks_initupnp() succeeded on iface %s",
                    function, gw.addr.addr.ifname);

               /*
                * nothing more to do for now.  Once we get the actual request
                * (connect(2), bind(2), etc.) we can set up the rest.
                * XXX cache this somehow, so we don't need to broadcast again
                * next time.
                */
               packet->gw      = gw;
               return route;
            }
            else
               slog(LOG_NEGOTIATE,
                    "%s: socks_initupnp() failed on ifname %s: %s",
                    function, gw.addr.addr.ifname, emsg);
         }

         snprintf(emsg, emsglen,
                  "could not find an UPNP router on any interface");

         swarnx("%s: %s", function, emsg);

         if (errno == 0)
            errno = ENETUNREACH;

         socks_blacklist(route, emsg);
         return NULL;
      }
      else {
         /*
          * nothing more to do for now.  Once we get the actual request
          * (connect(2), bind(2), etc.) we can try connecting to the
          * igd and set up the rest.
          */
         packet->gw = route->gw;
         return route;
      }

   }
#endif /* HAVE_LIBMINIUPNP */

   if ((rc = socks_connecthost(s,
#if !SOCKS_CLIENT
                               EXTERNALIF,
#endif /* !SOCKS_CLIENT */
                               &route->gw.addr,
                               NULL,
                               NULL,
                               sockscf.timeout.connect ?
                                 (long)sockscf.timeout.connect : (long)-1,
                               emsg,
                               emsglen)) == 0
   || (rc == -1 && errno == EINPROGRESS)) {
      packet->gw = route->gw;
      return route;
   }

   swarnx("%s: failed to connect route to %s on fd %d: %s",
          function, sockshost2string(&route->gw.addr, NULL, 0), s, emsg);

   if (errno == EINVAL) {
      struct sockaddr_in laddr;
      socklen_t len = sizeof(laddr);

      if (getsockname(s, TOSA(&laddr), &len) == 0
      &&  laddr.sin_addr.s_addr              == htonl(INADDR_LOOPBACK)) {
         static route_t directroute;

         slog(LOG_NEGOTIATE,
              "%s: failed to connect route, but that appears to be due to the "
              "socket (fd %d) having been bound to the loopback interface.  "
              "Assuming this socket should not proxied, but a direct "
              "connection should be made instead",
              function, s);

         directroute.gw.state.proxyprotocol.direct = 1;
         slog(LOG_DEBUG, "%s: XXX, line %d", function, __LINE__);
         return &directroute;
      }
   }
   else
      socks_blacklist(route, emsg);

   return NULL;
}

void
socks_clearblacklist(route)
   route_t *route;
{

   if (route != NULL)
      route->state.failed = route->state.badtime = 0;
}

void
socks_blacklist(route, reason)
   route_t *route;
   const char *reason;
{
   const char *function = "socks_blacklist()";

   if (route == NULL || sockscf.routeoptions.maxfail == 0)
      return;

   slog(LOG_INFO,
        "%s: blacklisting %sroute #%d.  Reason: %s",
        function,
        route->state.autoadded ? "autoadded " : "",
        route->number,
        reason);

#if HAVE_LIBMINIUPNP
   bzero(&route->gw.state.data, sizeof(route->gw.state.data));
#endif /* HAVE_LIBMINIUPNP */

   ++route->state.failed;
   time_monotonic(&route->state.badtime);
}

route_t *
socks_requestpolish(req, src, dst)
   request_t *req;
   const sockshost_t *src;
   const sockshost_t *dst;
{
   const char *function = "socks_requestpolish()";
   const unsigned char originalversion = req->version;
   static route_t directroute;
   route_t *route;
   char srcbuf[MAXSOCKSHOSTSTRING], dstbuf[MAXSOCKSHOSTSTRING];

   if (sockscf.route == NULL) {
      slog(LOG_DEBUG,
           "%s: no routes configured.  Going direct for all", function);

      directroute.gw.state.proxyprotocol.direct = 1;
      return &directroute;
   }

   slog(LOG_NEGOTIATE,
        "%s: searching for %s route for %s, protocol %s, src %s, dst %s, "
        "authmethod %d",
        function,
        proxyprotocol2string(req->version),
        command2string(req->command), protocol2string(req->protocol),
        src == NULL ? "<NONE>" : sockshost2string(src, srcbuf, sizeof(srcbuf)),
        dst == NULL ? "<NONE>" : sockshost2string(dst, dstbuf, sizeof(dstbuf)),
        req->auth->method);

   directroute.gw.state.proxyprotocol.direct = 1;

   if ((route = socks_getroute(req, src, dst)) != NULL)
      return route;

   /*
    * no route found.  Can we "polish" the request and then find a route?
    * Try all proxy protocols we support.
    */

   /*
    * To simplify making sure we are trying all versions, for now,
    * make an assumption about what we start with.
    */
   SASSERTX(req->version == PROXY_DIRECT);

   req->version = PROXY_SOCKS_V4;
   if ((route = socks_getroute(req, src, dst)) != NULL)
      return route;

   req->version = PROXY_SOCKS_V5;
   if ((route = socks_getroute(req, src, dst)) != NULL)
      return route;

   req->version = PROXY_HTTP_10;
   if ((route = socks_getroute(req, src, dst)) != NULL)
      return route;

   req->version = PROXY_HTTP_11;
   if ((route = socks_getroute(req, src, dst)) != NULL)
      return route;

   req->version = PROXY_UPNP;
   if ((route = socks_getroute(req, src, dst)) != NULL)
      return route;

   req->version = originalversion;

   if (sockscf.option.directfallback) {
      slog(LOG_NEGOTIATE,
           "%s: no route found for request %s, but direct fallback is enabled",
           function, command2string(req->command));

      req->version = PROXY_DIRECT;
      return &directroute;
   }

   slog(LOG_NEGOTIATE,
        "%s: no route found to handle request %s and direct route fallback "
        "disabled.  Nothing we can do",
        function, command2string(req->command));

   errno = ENETUNREACH;
   return NULL;
}

void
optioninit(void)
{
   /*
    * initialize misc. options to sensible default.  Some may be
    * overridden later by user in the sockd.conf.
    */

#if SOCKS_SERVER && HAVE_LDAP

   ldapauthorisation_t *ldapauthorisation   = &sockscf.state.ldapauthorisation;
   ldapauthentication_t *ldapauthentication = &sockscf.state.ldapauthentication;

#endif /* SOCKS_SERVER &&HAVE_LDAP */

   sockscf.resolveprotocol          = RESOLVEPROTOCOL_UDP;

#if SOCKS_DIRECTROUTE_FALLBACK

   if (socks_getenv(ENV_SOCKS_DIRECTROUTE_FALLBACK, isfalse) != NULL)
      sockscf.option.directfallback = 0;
   else
      sockscf.option.directfallback = 1;

#else /* !SOCKS_DIRECTROUTE_FALLBACK */

   if (socks_getenv(ENV_SOCKS_DIRECTROUTE_FALLBACK, istrue) != NULL)
      sockscf.option.directfallback = 1;
   else
      sockscf.option.directfallback = 0;

#endif /* SOCKS_DIRECTROUTE_FALLBACK */

   sockscf.routeoptions.maxfail   = 1;
   sockscf.routeoptions.badexpire = (time_t)(ROUTEBLACKLIST_SECONDS);

#if !SOCKS_CLIENT
   sockscf.option.debug          = 0;
   sockscf.option.keepalive      = 1;
   sockscf.option.hosts_access   = 0;

   sockscf.udpconnectdst         = 1;

   sockscf.timeout.connect       = SOCKD_CONNECTTIMEOUT;
   sockscf.timeout.negotiate     = SOCKD_NEGOTIATETIMEOUT;
   sockscf.timeout.tcpio         = SOCKD_IOTIMEOUT_TCP;
   sockscf.timeout.udpio         = SOCKD_IOTIMEOUT_UDP;
   sockscf.timeout.tcp_fin_wait  = SOCKD_FIN_WAIT_2_TIMEOUT;

   sockscf.external.rotation     = ROTATION_NONE;

#if HAVE_PAM
   STRCPY_ASSERTSIZE(sockscf.state.pamservicename, DEFAULT_PAMSERVICENAME);
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   STRCPY_ASSERTSIZE(sockscf.state.bsdauthstylename, DEFAULT_BSDAUTHSTYLENAME);
#endif /* HAVE_BSDAUTH */

#if HAVE_GSSAPI

   STRCPY_ASSERTSIZE(sockscf.state.gssapiservicename,
                     DEFAULT_GSSAPISERVICENAME);

   STRCPY_ASSERTSIZE(sockscf.state.gssapikeytab,
                     DEFAULT_GSSAPIKEYTAB);

#endif /* HAVE_GSSAPI */

#if SOCKS_SERVER && HAVE_LDAP

   /*
    * LDAP authorization.
    */

   STRCPY_ASSERTSIZE(ldapauthorisation->attribute, DEFAULT_LDAP_ATTRIBUTE);

   STRCPY_ASSERTSIZE(ldapauthorisation->attribute_AD,
                     DEFAULT_LDAP_ATTRIBUTE_AD);

   ldapauthorisation->auto_off                      = 0;
   ldapauthorisation->certcheck                     = 0;

   STRCPY_ASSERTSIZE(ldapauthorisation->certfile,   DEFAULT_LDAP_CACERTFILE);
   STRCPY_ASSERTSIZE(ldapauthorisation->certpath,   DEFAULT_LDAP_CERTDBPATH);

   ldapauthorisation->debug                         = 0;
   *ldapauthorisation->domain                       = NUL;

   STRCPY_ASSERTSIZE(ldapauthorisation->filter,     DEFAULT_LDAP_FILTER);
   STRCPY_ASSERTSIZE(ldapauthorisation->filter_AD,  DEFAULT_LDAP_FILTER_AD);

   ldapauthorisation->keeprealm                     = 0;

   STRCPY_ASSERTSIZE(ldapauthorisation->keytab,     DEFAULT_GSSAPIKEYTAB);

   ldapauthorisation->mdepth                        = 0;
   ldapauthorisation->port                          = SOCKD_EXPLICIT_LDAP_PORT;
   ldapauthorisation->portssl                       = SOCKD_EXPLICIT_LDAPS_PORT;
   ldapauthorisation->ssl                           = 0;

   /*
    * LDAP authentication.
    */

   ldapauthentication->auto_off                     = 0;
   ldapauthentication->certcheck                    = 1;

   STRCPY_ASSERTSIZE(ldapauthentication->certfile,  DEFAULT_LDAP_CACERTFILE);
   STRCPY_ASSERTSIZE(ldapauthentication->certpath,  DEFAULT_LDAP_CERTDBPATH);

   ldapauthentication->debug                        = 0;
   *ldapauthentication->domain                      = NUL;

   *ldapauthentication->filter                      = NUL;
   *ldapauthentication->filter_AD                   = NUL;

   STRCPY_ASSERTSIZE(ldapauthentication->keytab,    DEFAULT_GSSAPIKEYTAB);

   ldapauthentication->keeprealm                    = 0;

   ldapauthentication->port                         = SOCKD_EXPLICIT_LDAP_PORT;
   ldapauthentication->portssl                      = SOCKD_EXPLICIT_LDAPS_PORT;
   ldapauthentication->ssl                          = 1;

#endif /* SOCKS_SERVER && HAVE_LDAP */

#if BAREFOOTD
   /*
    * initially there is no udp traffic to bounce.
    * Changed later if we discover there are udp rules configured also.
    */
   sockscf.state.alludpbounced = 1;
#endif /* BAREFOOTD */

#endif /* !SOCKS_CLIENT */
}
