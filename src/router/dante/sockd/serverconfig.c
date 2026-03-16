/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2019, 2020, 2021,
 *               2024
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
"$Id: serverconfig.c,v 1.567.4.12.6.14.4.2 2024/11/20 22:05:40 karls Exp $";

static int
safamily_isenabled(const sa_family_t family, const char *addrstr,
                   const interfaceside_t side);
/*
 * Returns true if the address family "family" is enabled on the
 * interface-side "side".  "addrstr" is a printable representation of
 * the address we tried to add.
 *
 * Returns false if the address family "family" is not enabled.
 */

static int addexternaladdr(const struct ruleaddr_t *ra);
/*
 * Returns 0 if the address "ra" was addedd to the list of external addresses.
 *
 * Returns -1 if the address "ra" was not added for a non-fatal reason,
 * after loging a message if apropriate.
 */

static int addinternaladdr(const char *ifname,
                           const struct sockaddr_storage *sa,
                           const int protocol);
/*
 * Returns 0 if the address "ra" was addedd to the list of internal addresses.
 *
 * Returns -1 if the address "ra" was not added for a non-fatal reason,
 * after loging a message if apropriate.
 */


static void
add_more_old_shmem(struct config *config, const size_t memc,
                   const oldshmeminfo_t memv[]);
/*
 * Adds "memv" to the list of old shmem entries stored in "config".
 */

struct config sockscf;        /* current config.   */

void
addinternal(addr, protocol)
   const ruleaddr_t *addr;
   const int protocol;
{
   const char *function = "addinternal()";
   struct sockaddr_storage sa;
   char ifname[MAXIFNAMELEN];
   int changesupported;

   if (sockscf.option.serverc == 1
   ||  sockscf.state.inited   == 0
   ||  protocol               == SOCKS_UDP)
      changesupported = 1;
   else
      changesupported = 0;

   slog(LOG_DEBUG, "%s: (%s, %s).  Change supported: %d",
        function,
        ruleaddr2string(addr,
                        ADDRINFO_PORT | ADDRINFO_ATYPE,
                        NULL,
                        0),
        protocol2string(protocol),
        changesupported);

   switch (addr->atype) {
       case SOCKS_ADDR_IPV4:
       case SOCKS_ADDR_IPV6:
         if (addr->atype == SOCKS_ADDR_IPV4)
            SASSERTX(addr->addr.ipv4.mask.s_addr == htonl(IPV4_FULLNETMASK));
         else if (addr->atype == SOCKS_ADDR_IPV6)
            SASSERTX(addr->addr.ipv6.maskbits    == IPV6_NETMASKBITS);

         ruleaddr2sockaddr(addr, &sa, protocol);

         if (!PORTISBOUND(&sa))
            yyerrorx("%s: address %s does not specify a portnumber to bind",
                     function, sockaddr2string(&sa, NULL, 0));

         if (addrindex_on_listenlist(sockscf.internal.addrc,
                                     sockscf.internal.addrv,
                                     &sa,
                                     protocol) == -1) {
            if (!changesupported) {
               yywarnx("cannot change internal addresses once running.  "
                       "%s looks like a new address and will be ignored",
                       sockaddr2string(&sa, NULL, 0));

               break;
            }
         }
         else {
            /*
             * Already here, but do make sure to update globalstate to reflect
             * it too.
             */
            add_internal_safamily(sa.ss_family);
            break;
         }

         if (sa.ss_family == AF_INET
         &&  TOIN(&sa)->sin_addr.s_addr == htonl(INADDR_ANY))
            STRCPY_ASSERTSIZE(ifname, "<any IPv4-interface>");
         else if (sa.ss_family == AF_INET6
         &&  memcmp(&TOIN6(&sa)->sin6_addr,
                    &in6addr_any,
                    sizeof(in6addr_any)) == 0)
            STRCPY_ASSERTSIZE(ifname, "<any IPv6-interface>");
         else if (sockaddr2ifname(&sa, ifname, sizeof(ifname)) == NULL) {
            /*
             * Probably config-error, but could be a bug in sockaddr2ifname(),
             * so don't error out yet.  Will know for sure when we try to bind
             * the address later.
             */
            strncpy(ifname, "<unknown>", sizeof(ifname) - 1);
            ifname[sizeof(ifname) - 1] = NUL;

            yywarn("%s: could not find address %s on any network interface",
                   function, sockaddr2string2(&sa, 0, NULL, 0));
         }

         addinternaladdr(ifname, &sa, protocol);
         break;

      case SOCKS_ADDR_DOMAIN: {
         size_t i;
         char emsg[1024];
         int gaierr;

         for (i = 0;
              hostname2sockaddr2(addr->addr.domain,
                                 i,
                                 &sa,
                                 &gaierr,
                                 emsg,
                                 sizeof(emsg)) != NULL;
              ++i) {
            SET_SOCKADDRPORT(&sa,
                             protocol == SOCKS_TCP ?
                                       addr->port.tcp : addr->port.udp);

            if (addrindex_on_listenlist(sockscf.internal.addrc,
                                        sockscf.internal.addrv,
                                        &sa,
                                        protocol) == -1) {
               if (!changesupported) {
                  swarnx("cannot change internal addresses once running "
                         "and %s looks like a new address.  Ignored",
                         sockaddr2string(&sa, NULL, 0));

                  continue;
               }
            }
            else {
               /*
                * Already here, but do make sure to update globalstate to
                * reflect it too.
                */
               add_internal_safamily(sa.ss_family);
               continue;
            }

            if (sockaddr2ifname(&sa, ifname, sizeof(ifname)) == NULL) {
               /*
                * Probably config-error, but could be bug in our
                * sockaddr2ifname().
                * Will know for sure when we try to bind the address later,
                * so don't error out quite yet.
                */

               yywarn("%s: could not find address %s (resolved from %s) on "
                      "any network interface",
                      function,
                      sockaddr2string(&sa, NULL, 0),
                      addr->addr.domain);

               STRCPY_ASSERTSIZE(ifname, "<unknown>");
            }

            addinternaladdr(ifname, &sa, protocol);
         }

         if (i == 0)
            yyerrorx("%s", emsg);

         break;
      }

      case SOCKS_ADDR_IFNAME: {
         struct ifaddrs *ifap, *iface;
         int isvalidif;

         ifap = NULL;

         if (getifaddrs(&ifap) != 0)
            serr("getifaddrs()");

         SASSERTX(ifap != NULL);

         for (isvalidif = 0, iface = ifap;
         iface != NULL;
         iface = iface->ifa_next) {
            if (iface->ifa_addr == NULL)
               continue;

            if (!safamily_issupported(iface->ifa_addr->sa_family))
               continue;

            if (strcmp(iface->ifa_name, addr->addr.ifname) != 0)
               continue;

            isvalidif = 1;

            sockaddrcpy(&sa, TOSS(iface->ifa_addr), sizeof(sa));

            SET_SOCKADDRPORT(&sa, protocol == SOCKS_TCP ?
                                       addr->port.tcp : addr->port.udp);

            if (addrindex_on_listenlist(sockscf.internal.addrc,
                                        sockscf.internal.addrv,
                                        &sa,
                                        protocol) == -1) {
               if (!changesupported) {
                  swarnx("cannot change internal addresses once running, "
                         "and %s, expanded from the ifname \"%s\" looks "
                         "like a new address.  Ignored",
                         sockaddr2string(&sa, NULL, 0),
                         addr->addr.ifname);

                  continue;
               }
            }
            else {
               /*
                * Already here, but do make sure to update globalstate to
                * reflect it too.
                */
               add_internal_safamily(sa.ss_family);
               continue;
            }

            addinternaladdr(addr->addr.ifname, &sa, protocol);
         }

         freeifaddrs(ifap);

         if (!isvalidif)
            swarnx("cannot find interface/address for %s", addr->addr.ifname);

         break;
      }

      default:
         SERRX(addr->atype);
   }
}

void
addexternal(addr)
   const ruleaddr_t *addr;
{
   const char *function = "addexternal()";
   ruleaddr_t ra;
   int added_ipv4 = 0, added_ipv6 = 0, added_ipv6_gs = 0;

   SASSERTX(ntohs(addr->port.tcp) == 0);
   SASSERTX(ntohs(addr->port.udp) == 0);

   switch (addr->atype) {
      case SOCKS_ADDR_DOMAIN: {
         /*
          * XXX this is not good.  It is be better to not resolve this now,
          * but resolve it when using.  Since we have a hostcache, that
          * should not add too much expense.  Sending servers a SIGHUP
          * when local addresses change is quite common though, so
          * assume it's good enough for now.
          */
         struct sockaddr_storage sa;
         size_t i;
         char emsg[1024];
         int gaierr;

         for (i = 0;
         hostname2sockaddr2(addr->addr.domain,
                            i,
                            &sa,
                            &gaierr,
                            emsg,
                            sizeof(emsg)) != NULL;
          ++i) {
            SET_SOCKADDRPORT(&sa, addr->port.tcp);

            sockaddr2ruleaddr(&sa, &ra);

            if (addexternaladdr(&ra) == 0) {
               switch (sa.ss_family) {
                  case AF_INET:
                     added_ipv4 = 1;
                     break;

                  case AF_INET6:
                     added_ipv6 = 1;

                     if (!IN6_IS_ADDR_LINKLOCAL(&TOIN6(&sa)->sin6_addr))
                        added_ipv6_gs = 1;

                     break;

                  default:
                     SERRX(sa.ss_family);
               }
            }
         }

         if (i == 0)
            yyerrorx("%s", emsg);

         break;
      }

      case SOCKS_ADDR_IPV4:
         if (addr->addr.ipv4.ip.s_addr == htonl(INADDR_ANY))
            yyerrorx("external address (%s) to connect out from cannot "
                     "be a wildcard address",
                     ruleaddr2string(addr, 0, NULL, 0));

         ra                       = *addr;
         ra.addr.ipv4.mask.s_addr = htonl(IPV4_FULLNETMASK);

         if (addexternaladdr(&ra) == 0)
            added_ipv4 = 1;

         break;

      case SOCKS_ADDR_IPV6:
         if (memcmp(&addr->addr.ipv6.ip, &in6addr_any, sizeof(in6addr_any))
         == 0)
            yyerrorx("external address (%s) cannot be a wildcard address",
                     ruleaddr2string(addr, 0, NULL, 0));

         ra                    = *addr;
         ra.addr.ipv6.maskbits = IPV6_NETMASKBITS;

         if (addexternaladdr(&ra) == 0) {
            added_ipv6 = 1;

            if (!IN6_IS_ADDR_LINKLOCAL(&ra.addr.ipv6.ip))
               added_ipv6_gs = 1;
         }

         break;

      case SOCKS_ADDR_IFNAME: {
         /*
          * Would be nice if this could be cached, e.g. by monitoring a
          * routing socket for changes.  Have no code for that however.
          */
         struct sockaddr_storage sa, t;
         size_t i;

         /*
          * We add the interface, not the addresses.  But we want to
          * know whether the addresses, at least currently, resolve
          * to ipv4 or ipv6 so we can resolve hostnames appropriately.
          * E.g., no need to resolve hostname to ipv6 address if we do
          * not have ipv6 on the external interface.
          */
         for (i = 0;
         ifname2sockaddr(addr->addr.ifname, i, &sa, &t) != NULL;
         ++i) {
            const int enabled
            = safamily_isenabled(sa.ss_family,
                                 sockaddr2string(&sa, NULL, 0),
                                 EXTERNALIF);

            slog(LOG_DEBUG, "%s: ifname %s resolved to address %s.  %s",
                 function,
                 addr->addr.ifname,
                 sockaddr2string2(&sa, ADDRINFO_ATYPE, NULL, 0),
                 enabled ? "enabled" : "not enabled due to address family");

            if (!enabled)
               continue;

            switch (sa.ss_family) {
               case AF_INET:
                  added_ipv4 = 1;
                  break;

               case AF_INET6:
                  added_ipv6 = 1;

                  if (!IN6_IS_ADDR_LINKLOCAL(&TOIN6(&sa)->sin6_addr))
                     added_ipv6_gs = 1;

                  break;

               default:
                  SERRX(sa.ss_family);
            }
         }

         /*
          * Not resolving but adding the ifname itself.
          */
         (void)addexternaladdr(addr);

         break;
      }

      default:
         SERRX(addr->atype);
   }

   if (added_ipv4)
      add_external_safamily(AF_INET, 1);

   if (added_ipv6)
      add_external_safamily(AF_INET6, added_ipv6_gs);
}

void
resetconfig(config, exiting)
   struct config *config;
   const int exiting;
{
   const char *function = "resetconfig()";
   const int ismainmother = pidismainmother(config->state.pid);
   rule_t *rulev[] = { config->crule, config->hrule, config->srule };
   monitor_t *monitor;
   size_t oldc, i;

   slog(LOG_DEBUG, "%s: exiting? %s, ismainmother? %s",
        function,
        exiting ?       "yes" : "no",
        ismainmother?   "yes" : "no");

   if (!exiting) {
#if !HAVE_NO_RESOLVESTUFF
      _res.options = config->initial.res_options;
#endif /* !HAVE_NO_RESOLVSTUFF */
   }

   switch (sockscf.state.type) {
      case PROC_MOTHER:
         mother_preconfigload();
         break;

      case PROC_MONITOR:
         monitor_preconfigload();
         break;

      case PROC_NEGOTIATE:
         negotiate_preconfigload();
         break;

      case PROC_REQUEST:
         request_preconfigload();
         break;

      case PROC_IO:
         io_preconfigload();
         break;
   }

   /*
    * config->initial: nothing to do here.
    */

   /*
    * Internal interface.
    */

   if (config->option.serverc == 1 || !ismainmother) {
      /*
       * We only support changing these as long as we only have one mother
       * process.
       * If we are not the main mother, we do need to free the memory as
       * usual however, so it will not be leaked when we realloc based on
       * the config in shmem that main mother has now installed and from
       * which we will update.
       */

      free(config->internal.addrv);
      config->internal.addrv = NULL;
      config->internal.addrc = 0;

      bzero(&config->internal.protocol, sizeof(config->internal.protocol));
   }

   bzero(&config->internal.log, sizeof(config->internal.log));

   /*
    * External interface.
    */

   /* external addresses can always be changed. */
   free(config->external.addrv);
   config->external.addrv = NULL;
   config->external.addrc = 0;

   config->external.rotation = ROTATION_NOTSET;
   bzero(&config->external.log, sizeof(config->external.log));
   bzero(&config->external.protocol, sizeof(config->external.protocol));

   /* can always be changed from config. */
   bzero(&config->cpu, sizeof(config->cpu));

   for (i = 0; i < ELEMENTS(rulev); ++i) {
      rule_t *rule, prevrule, *next;
      int haveprevrule;

      haveprevrule = 0;
      rule         = rulev[i];

      while (rule != NULL) {
         /*
          * Free normal process-local memory.
          */
         int rule_is_autoexpanded;

#if !HAVE_SOCKS_RULES
         if (rule->type == object_srule) {
            /*
             * All pointers are pointers to the same memory in the clientrule,
             * so it has already been freed and only the rule itself remains
             * to be freed.
             */
            next = rule->next;
            free(rule);
            rule = next;

            continue;
         }
#endif /* !HAVE_SOCKS_RULES */

         if (haveprevrule
         &&  prevrule.type   == rule->type
         &&  prevrule.number == rule->number) {
            slog(LOG_DEBUG,
                 "%s: another %s with same rule-number (%lu).  "
                 "Must be expanded from the same \"to\"-address",
                 function,
                 objecttype2string(rule->type),
                 (unsigned long)rule->number);

            rule_is_autoexpanded = 1;
            SASSERTX(BAREFOOTD);
         }
         else
            rule_is_autoexpanded = 0;

         if (!rule_is_autoexpanded)
            /*
             * We don't bother allocating identical memory for auto-expanded
             * rules, so only need to free it if it is not auto-expanded.
             */
            freelinkedname(rule->user);
         rule->user = NULL;

         if (!rule_is_autoexpanded)
            freelinkedname(rule->group);
         rule->group = NULL;

         if (!rule_is_autoexpanded)
            free(rule->socketoptionv);
         rule->socketoptionv = NULL;
         rule->socketoptionc = 0;

         if (ismainmother) {
            /*
             * Next go through the shmem in this rule.  It's possible
             * we have children that are still using, or about to use,
             * these segments, so don't delete them now, but save
             * them for later.  Only upon exit we delete them all.
             *
             * This means we may have a lot of unneeded shmem segments
             * laying around, but since they are just files, rather
             * than the, on some systems very scarce, sysv-style shmem
             * segments, that should not be any problem.  It allows
             * us to ignore a lot of nasty locking issues.
             */
            size_t moreoldshmemc = 0;
            oldshmeminfo_t moreoldshmemv[   1 /* bw             */
                                          + 1 /* session        */
                                          + 1 /* session state. */
                                        ];

            if (rule_is_autoexpanded) {
               SASSERTX(BAREFOOTD);
               SHMEM_CLEAR(rule, SHMEM_ALL, 1);
            }
            else {
               if (rule->bw_shmid != 0) {
                  moreoldshmemv[moreoldshmemc].id   = rule->bw_shmid;
                  moreoldshmemv[moreoldshmemc].key  = key_unset;
                  moreoldshmemv[moreoldshmemc].type = SHMEM_BW;

                  ++moreoldshmemc;
               }

               if (rule->ss_shmid != 0) {
                  /*
                   * session-module supports statekeys too, so need to save that
                   * too.
                   */
                  if (sockd_shmat(rule, SHMEM_SS) == 0) {
                     moreoldshmemv[moreoldshmemc].id   = rule->ss_shmid;
                     moreoldshmemv[moreoldshmemc].key  = rule->ss->keystate.key;
                     moreoldshmemv[moreoldshmemc].type = SHMEM_SS;

                     ++moreoldshmemc;

                     sockd_shmdt(rule, SHMEM_SS);
                  }
               }

               if (moreoldshmemc > 0)
                  add_more_old_shmem(config, moreoldshmemc, moreoldshmemv);
            }
         }

         prevrule = *rule;
         haveprevrule = 1;

         next     = rule->next;
         free(rule);
         rule     = next;
      }
   }

   config->crule = config->hrule = config->srule = NULL;

   /* routeoptions, read from config file. */
   bzero(&config->routeoptions, sizeof(config->routeoptions));

   /* free routes. */
   freeroutelist(config->route);
   config->route = NULL;

   /* and monitors. */
   monitor = sockscf.monitor;
   while (monitor != NULL) {
      monitor_t *next = monitor->next;

      if (ismainmother && monitor->mstats_shmid != 0) {
         oldshmeminfo_t moreoldshmemv[   1 /* just the monitor shmid. */ ];

         moreoldshmemv[0].id    = monitor->mstats_shmid;
         moreoldshmemv[0].key   = key_unset;
         moreoldshmemv[0].type  = SHMEM_MONITOR;

         add_more_old_shmem(config, ELEMENTS(moreoldshmemv), moreoldshmemv);
      }

      free(monitor);
      monitor = next;
   }
   config->monitor = NULL;

   /* monitoroptions.  Reset on each reload. */
   bzero(&config->monitorspec, sizeof(config->monitorspec));

   free(config->socketoptionv);
   config->socketoptionv = NULL;
   config->socketoptionc = 0;

   /* compat, read from config file. */
   bzero(&config->compat, sizeof(config->compat));

   /* extensions, read from config file. */
   bzero(&config->extension, sizeof(config->extension));

   /*
    * log, errlog; handled specially when parsing.
    */

   /*
    * option; some only settable at commandline, some only read from config
    * file.  Those only read from config file will be reset to default in
    * optioninit().
    */

   /* resolveprotocol, read from config file. */
   bzero(&config->resolveprotocol, sizeof(config->resolveprotocol));

   /*
    * socketconfig, read from config file, but also has defaults set by
    * optioninit(), so don't need to touch it.
    */

   /* srchost, read from config file. */
   bzero(&config->srchost, sizeof(config->srchost));

   /* stat: not touch.  Accumulated continously. */

   /*
    * state; keep most of it, with the following exceptions:
    */
   /* don't want to have too much code for tracking this, so regen this now. */
   config->state.highestfdinuse = 0;

   /* timeout, read from config file. */
   bzero(&config->timeout, sizeof(config->timeout));

#if HAVE_SOLARIS_PRIVS
   /* uid; is special.  Needs clearing, but must reopen config-file first. */
#endif /* HAVE_SOLARIS_PRIVS */

   /* (child)state: not touched. */


   /*
    * various method settings.  All read from config file.
    */

   bzero(config->cmethodv, sizeof(config->cmethodv));
   config->cmethodc = 0;

   bzero(config->smethodv, sizeof(config->smethodv));
   config->smethodc = 0;

   /* udpconnectdst.  No need to touch.  Reset to default on reload. */

#if HAVE_LIBWRAP
   if (config->hosts_allow_original != NULL
   && hosts_allow_table             != config->hosts_allow_original) {
      free(hosts_allow_table);
      hosts_allow_table = config->hosts_allow_original;
   }

   if (config->hosts_deny_original != NULL
   && hosts_deny_table             != config->hosts_deny_original) {
      free(hosts_deny_table);
      hosts_deny_table = config->hosts_deny_original;
   }
#endif /* HAVE_LIBWRAP */

   if (exiting && ismainmother && config->oldshmemc > 0) {
      /*
       * Go through the list of saved segments and delete them.
       * Any (io) children using them should already have them open,
       * and nobody not already using them should need to attach to them
       * after we exit.  The exception is clients using the session module,
       * where we do not keep attached to the segment, but who need to attach
       * to it when removing the client.  Unfortunately failure to attach
       * to a shmem segment is normally a serious error and logged as thus,
       * but if mother has removed the segment, then obviously the other
       * processes can not attach to it again.
       *
       * There is some code to only debug log failure to attach to the
       * shmem segments (or rather, failure to open the file) if mother
       * does not exist (presumably having deleted the files before exiting),
       * rather than warn.  It depends on mother having exited before the
       * child process tries to remove the client though, which may not
       * be the case even though we do a little work to increase the odds.
       * Worst case is that we end up with some useless warnings though,
       * so not worth going overboard with it.
       */

      SASSERTX(ismainmother);
      SASSERTX(sockscf.state.type == PROC_MOTHER);

      /*
       * Lock to increase the chance of us having time to exit before
       * any children try to attach/detach (they will be blocked waiting for
       * the lock).  Don't unlock ourselves, but let the kernel release the
       * lock when we exit, further reducing gap between us exiting and
       * a child process being able to detect it.
       */
      socks_lock(config->shmemfd, 0, 0, 1, 1);

      slog(LOG_DEBUG, "%s: %ld old shmem entr%s saved.  Deleting now",
                      function, (unsigned long)config->oldshmemc,
                      config->oldshmemc == 1 ? "y" : "ies");

      for (oldc = 0; oldc < config->oldshmemc; ++oldc) {
         char fname[PATH_MAX];

         snprintf(fname, sizeof(fname), "%s",
                  sockd_getshmemname(config->oldshmemv[oldc].id, key_unset));

         slog(LOG_DEBUG,
              "%s: deleting shmem segment shmid %lu in file %s at index #%lu",
              function,
              (unsigned long)config->oldshmemv[oldc].id,
              fname,
              (unsigned long)oldc);

         if (unlink(fname) != 0)
            swarn("%s: failed to unlink shmem segment %ld in file %s",
                  function, config->oldshmemv[oldc].id, fname);

         if (config->oldshmemv[oldc].key != key_unset) {
            snprintf(fname, sizeof(fname), "%s",
                     sockd_getshmemname(config->oldshmemv[oldc].id,
                                        config->oldshmemv[oldc].key));

            slog(LOG_DEBUG,
                 "%s: deleting shmem segment shmid %lu/key %lu in file %s",
                 function,
                 (unsigned long)config->oldshmemv[oldc].id,
                 (unsigned long)config->oldshmemv[oldc].key,
                 fname);

            if (unlink(fname) != 0)
               swarn("%s: failed to unlink shmem segment %ld.%d in file %s",
                     function,
                     config->oldshmemv[oldc].id,
                     (int)config->oldshmemv[oldc].key,
                     fname);
         }
      }
   }
}

void
freeroutelist(routehead)
   route_t *routehead;
{

   while (routehead != NULL) {
      route_t *next = routehead->next;

      free(routehead->socketoptionv);
      free(routehead);
      routehead = next;
   }
}

int
addrisbindable(addr)
   const ruleaddr_t *addr;
{
   const char *function = "addrisbindable()";
   struct sockaddr_storage saddr;
   int rc, s;

   switch (addr->atype) {
      case SOCKS_ADDR_IPV4:
      case SOCKS_ADDR_IPV6:
         sockshost2sockaddr(ruleaddr2sockshost(addr, NULL, SOCKS_TCP), &saddr);
         break;

      case SOCKS_ADDR_IFNAME: {
         struct sockaddr_storage mask;

         if (ifname2sockaddr(addr->addr.ifname, 0, &saddr, &mask) == NULL) {
            swarn("%s: cannot find interface named %s with ip configured",
                  function, addr->addr.ifname);

            return 0;
         }

         break;
      }

      case SOCKS_ADDR_DOMAIN: {
         sockshost_t host;

         sockshost2sockaddr(ruleaddr2sockshost(addr, &host, SOCKS_TCP), &saddr);
         if (!IPADDRISBOUND(&saddr)) {
            swarnx("%s can not resolve host %s: %s",
                  function,
                  sockshost2string(&host, NULL, 0),
                  hstrerror(h_errno));

            return 0;
         }

         break;
      }

      default:
         SERRX(addr->atype);
   }

   if ((s = socket(saddr.ss_family, SOCK_STREAM, 0)) == -1) {
      swarn("%s: socket(SOCK_STREAM)", function);
      return 0;
   }

   rc = socks_bind(s, &saddr, 0);
   close(s);

   if (rc != 0)
      swarn("%s: cannot bind address: %s (from address specification %s)",
            function,
            sockaddr2string(&saddr, NULL, 0),
            ruleaddr2string(addr, 0, NULL, 0));

   return rc == 0;
}

int
isreplycommandonly(command)
   const command_t *command;
{

   if ((command->bindreply || command->udpreply)
   && !(command->connect || command->bind || command->udpassociate))
      return 1;
   else
      return 0;
}

int
hasreplycommands(command)
   const command_t *command;
{

   if (command->bindreply || command->udpreply)
      return 1;
   else
      return 0;
}


ssize_t
addrindex_on_listenlist(listc, listv, _addr, protocol)
   const size_t listc;
   const listenaddress_t *listv;
   const struct sockaddr_storage *_addr;
   const int protocol;
{
   size_t i;

   for (i = 0; i < listc; ++i) {
      struct sockaddr_storage addr = *(const struct sockaddr_storage *)_addr;

      if (listv[i].protocol != protocol)
         continue;

      if (GET_SOCKADDRPORT(&addr) == htons(0)) /* match any internal port. */
         SET_SOCKADDRPORT(&addr, GET_SOCKADDRPORT(&listv[i].addr));

      if (sockaddrareeq(&addr, &listv[i].addr, 0))
         return (ssize_t)i;
   }

   return (ssize_t)-1;
}

ssize_t
addrindex_on_externallist(external, _addr)
   const externaladdress_t *external;
   const struct sockaddr_storage *_addr;
{
   const char *function = "addrindex_on_externallist()";
   struct sockaddr_storage sa, addr;
   size_t i;

   /*
    * Not interested in comparing portnumber.
    */
   sockaddrcpy(&addr, _addr, sizeof(addr));
   SET_SOCKADDRPORT(&addr, htons(0));

   slog(LOG_DEBUG,
        "%s: checking if address %s is a configured external address",
        function, sockaddr2string(&addr, NULL, 0));

   for (i = 0; i < external->addrc; ++i) {
      slog(LOG_DEBUG, "%s: external address #%lu: %s",
           function,
           (unsigned long)i,
           ruleaddr2string(&external->addrv[i], ADDRINFO_ATYPE, NULL, 0));

      switch (external->addrv[i].atype) {
         case SOCKS_ADDR_IPV4:
         case SOCKS_ADDR_IPV6: {
            sockshost_t host;

            sockshost2sockaddr(ruleaddr2sockshost(&external->addrv[i],
                                                  &host,
                                                  SOCKS_TCP),
                               &sa);
#if DIAGNOSTIC
            SASSERTX(safamily_isenabled(sa.ss_family,
                                        sockaddr2string(&sa, NULL, 0),
                                        EXTERNALIF));
#endif /* DIAGNOSTIC */

            if (sockaddrareeq(&addr, &sa, 0))
               return (ssize_t)i;

            break;
         }
         case SOCKS_ADDR_DOMAIN: {
            char emsg[1024];
            int gaierr;
            size_t ii;

            ii = 0;
            while (hostname2sockaddr2(external->addrv[i].addr.domain,
                                      ii++,
                                      &sa,
                                      &gaierr,
                                      emsg,
                                      sizeof(emsg)) != NULL) {
               slog(LOG_DEBUG, "%s: checking resolved address %s ...",
                    function, sockaddr2string(&sa, NULL, 0));

               if (!safamily_isenabled(sa.ss_family,
                                       sockaddr2string(&sa, NULL, 0),
                                       EXTERNALIF))
                  continue;

               if (sockaddrareeq(&addr, &sa, 0))
                  return (ssize_t)i;
            }

            if (ii == 0)
               swarnx("%s: problem with address on external interface: %s",
                      function, emsg);

            break;
         }

         case SOCKS_ADDR_IFNAME: {
            struct sockaddr_storage mask;
            size_t ii;

            ii = 0;
            while (ifname2sockaddr(external->addrv[i].addr.ifname,
                                   ii++,
                                   &sa,
                                   &mask) != NULL) {
               if (!safamily_isenabled(sa.ss_family,
                                       sockaddr2string(&sa, NULL, 0),
                                       EXTERNALIF))
                  continue;

               if (sockaddrareeq(&addr, &sa, 0))
                  return (ssize_t)i;
            }

            break;
         }

         default:
            SERRX(external->addrv[i].atype);
      }
   }

   return (ssize_t)-1;
}

void
checkconfig(void)
{
   const char *function = "checkconfig()";

#if HAVE_PAM
   char *pamservicename = NULL;
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   char *bsdauthstylename = NULL;
#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP

   ldapauthentication_t ldapauthentication =
   { .ldapurl    = NULL,
/*     .ldapbasedn = NULL, */
     .domain     = { NUL },
     .keytab     = { NUL },
     .filter     = { NUL },
     .certfile   = { NUL },
     .certpath   = { NUL },
     .debug      = LDAP_UNSET_DEBUG_VALUE,
     .auto_off   = -1,
     .ssl        = -1,
     .certcheck  = -1,
     .port       = -1,
     .portssl    = -1,
   };

   /*
    * XXX need same for ldap_t?
    */

   int ldom = -1, lfil = -1;

#endif /* HAVE_LDAP */

#if HAVE_GSSAPI

   char *gssapiservicename = NULL, *gssapikeytab = NULL;

#endif /* HAVE_GSSAPI */


   rule_t *rulebasev[]   =  { sockscf.crule,
                              sockscf.hrule,
                              sockscf.srule
                            };
   size_t i, basec;
   int usinglibwrap = 0;

   for (i = 0; i < sockscf.cmethodc; ++i) {
      SASSERTX(sockscf.cmethodv[i] >= AUTHMETHOD_NONE);
      SASSERTX(sockscf.cmethodv[i] <= AUTHMETHOD_MAX);

      SASSERTX(methodisvalid(sockscf.cmethodv[i], object_crule));

      if (sockscf.cmethodv[i] == AUTHMETHOD_RFC931)
         usinglibwrap = 1;
   }

#if HAVE_SOCKS_RULES
   if (sockscf.smethodc == 0)
      swarnx("%s: no socks authentication methods enabled.  This means all "
             "socks requests will be blocked after negotiation.  "
             "Perhaps this is not intended?",
             function);
   else {
      for (i = 0; i < sockscf.smethodc; ++i) {
         SASSERTX(sockscf.smethodv[i] >= AUTHMETHOD_NONE);
         SASSERTX(sockscf.smethodv[i] <= AUTHMETHOD_MAX);

         if (sockscf.smethodv[i] == AUTHMETHOD_RFC931)
            usinglibwrap = 1;

         if (sockscf.smethodv[i] == AUTHMETHOD_NONE
         &&  i + 1               < sockscf.smethodc)
            yywarnx("authentication method \"%s\" is configured in the "
                    "global socksmethod list, but since authentication "
                    "methods are selected by the priority given, we will "
                    "never try to match any of the subsequent authentication "
                    "methods.  I.e., no match will ever be attempted on the "
                    "next method, method \"%s\"",
                    method2string(sockscf.smethodv[i]),
                    method2string(sockscf.smethodv[i + 1]));

      }
   }
#endif /* HAVE_SOCKS_RULES */

   /*
    * Check rules, including if some rule-specific settings vary across
    * rules.
    *
    * If they don't vary we can optimize things when running and set the
    * corresponding variable in the global sockscf object to the constant
    * value.
    * If they vary, we set the corresponding global variable in sockscf to
    * NULL/NUL to indicate we don't have a constant value for this
    * variable/setting.
    */
   basec = 0;
   while (basec < ELEMENTS(rulebasev)) {
      rule_t *rule = rulebasev[basec++];

      if (rule == NULL)
         continue;

      for (; rule != NULL; rule = rule->next) {
         size_t methodc;
         int *methodv;

         slog(LOG_DEBUG, "%s: %s %u",
              function, objecttype2string(rule->type), (unsigned)rule->number);

#if HAVE_LIBWRAP

         if (*rule->libwrap != NUL)
            usinglibwrap = 1;

#endif /* HAVE_LIBWRAP */

         /*
          * What methods do we need to check?  clientmethods for
          * client-rules, socksmethods for socks-rules.
          */
         switch (rule->type) {
            case object_crule:

#if HAVE_SOCKS_HOSTID

            case object_hrule:

#endif /* HAVE_SOCKS_HOSTID */

               methodc = rule->state.cmethodc;
               methodv = rule->state.cmethodv;
               break;

            case object_srule:
               methodc = rule->state.smethodc;
               methodv = rule->state.smethodv;
               break;

            default:
               SERRX(rule->type);
         }

         for (i = 0; i < methodc; ++i) {
            switch (methodv[i]) {

#if HAVE_PAM
               case AUTHMETHOD_PAM_ANY:
               case AUTHMETHOD_PAM_ADDRESS:
               case AUTHMETHOD_PAM_USERNAME:
                  if (*sockscf.state.pamservicename == NUL)
                     break; /* already found to vary. */

                  if (pamservicename == NULL) /* first pam rule. */
                     pamservicename = rule->state.pamservicename;
                  else if (strcmp(pamservicename, rule->state.pamservicename)
                  != 0) {
                     slog(LOG_DEBUG, "%s: pam.servicename varies, %s ne %s",
                          function,
                          pamservicename,
                          rule->state.pamservicename);

                     *sockscf.state.pamservicename = NUL;
                  }

                  break;

#endif /* HAVE_PAM */

#if HAVE_BSDAUTH

               case AUTHMETHOD_BSDAUTH:
                  if (*sockscf.state.bsdauthstylename == NUL)
                     break; /* already found to vary. */

                  if (bsdauthstylename == NULL) /* first bsdauth rule. */
                     bsdauthstylename = rule->state.bsdauthstylename;
                  else if (strcmp(bsdauthstylename,
                                  rule->state.bsdauthstylename) != 0) {
                     slog(LOG_DEBUG,
                          "%s: bsdauth.stylename varies, %s ne %s",
                          function,
                          bsdauthstylename,
                          rule->state.bsdauthstylename);

                     *sockscf.state.bsdauthstylename = NUL;
                  }

                  break;

#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP

                  case AUTHMETHOD_LDAPAUTH:
                     if (sockscf.state.ldapauthentication.ldapurl == NULL)
                        ; /* varies. */
                     else {
                        if (ldapauthentication.ldapurl == NULL)
                           ldapauthentication.ldapurl
                           = rule->state.ldapauthentication.ldapurl;
                        else if (!linkednamesareeq(ldapauthentication.ldapurl,
                                      rule->state.ldapauthentication.ldapurl)) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.ldapurl varies",
                                function);

                           sockscf.state.ldapauthentication.ldapurl = NULL;
                        }
                     }

                     if (*sockscf.state.ldapauthentication.certfile == NUL)
                        ; /* varies. */
                     else {
                        if (*ldapauthentication.certfile == NUL)
                           STRCPY_ASSERTSIZE(ldapauthentication.certfile,
                                       rule->state.ldapauthentication.certfile);
                        else if (strcmp(ldapauthentication.certfile,
                                        rule->state.ldapauthentication.certfile)
                                 != 0) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.certfile varies, "
                                "%s ne %s",
                                function,
                                ldapauthentication.certfile,
                                rule->state.ldapauthentication.certfile);

                           *sockscf.state.ldapauthentication.certfile = NUL;
                        }
                     }

                     if (*sockscf.state.ldapauthentication.certpath == NUL)
                        ; /* varies. */
                     else {
                        if (*ldapauthentication.certpath == NUL)
                           STRCPY_ASSERTSIZE(ldapauthentication.certpath,
                                       rule->state.ldapauthentication.certpath);
                        else if (strcmp(ldapauthentication.certpath,
                                        rule->state.ldapauthentication.certpath)
                        != 0) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.certpath varies, "
                                "%s ne %s",
                                function,
                                ldapauthentication.certpath,
                                rule->state.ldapauthentication.certpath);

                           *sockscf.state.ldapauthentication.certpath = NUL;
                        }
                     }

                     if (*sockscf.state.ldapauthentication.keytab == NUL)
                        ; /* varies. */
                     else {
                        if (*ldapauthentication.keytab == NUL)
                           STRCPY_ASSERTSIZE(ldapauthentication.keytab,
                                         rule->state.ldapauthentication.keytab);
                        else if (strcmp(ldapauthentication.keytab,
                                        rule->state.ldapauthentication.keytab)
                                 != 0) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.keytab varies, "
                                "%s ne %s",
                                function,
                                ldapauthentication.keytab,
                                rule->state.ldapauthentication.keytab);

                           *sockscf.state.ldapauthentication.keytab = NUL;
                        }
                     }

                     if (ldom != 0
                     &&  *rule->state.ldapauthentication.domain != NUL) {
                        if (*ldapauthentication.domain == NUL)
                           STRCPY_ASSERTSIZE(ldapauthentication.domain,
                                         rule->state.ldapauthentication.domain);
                        else if (strcmp(ldapauthentication.domain,
                                        rule->state.ldapauthentication.domain)
                                 != 0) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.domain varies, "
                                "%s ne %s",
                                function,
                                ldapauthentication.domain,
                                rule->state.ldapauthentication.domain);

                           ldom = 0;
                        }
                     }

                     if (lfil != 0
                     &&  *rule->state.ldapauthentication.filter != NUL) {
                        if (*ldapauthentication.filter == NUL)
                           STRCPY_ASSERTSIZE(ldapauthentication.filter,
                                         rule->state.ldapauthentication.filter);
                        else if (strcmp(ldapauthentication.filter,
                                        rule->state.ldapauthentication.filter)
                                 != 0) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.filter varies, "
                                "%s ne %s",
                                function,
                                ldapauthentication.filter,
                                rule->state.ldapauthentication.filter);

                           lfil = 0;
                        }
                     }

                     if (sockscf.state.ldapauthentication.debug
                     != LDAP_UNSET_DEBUG_VALUE) {
                        if (ldapauthentication.debug == LDAP_UNSET_DEBUG_VALUE)
                           ldapauthentication.debug
                           = rule->state.ldapauthentication.debug;
                        else if (ldapauthentication.debug
                        != rule->state.ldapauthentication.debug) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.debug varies, %d ne %d",
                                function,
                                (int)ldapauthentication.debug,
                                (int)rule->state.ldapauthentication.debug);

                           sockscf.state.ldapauthentication.debug
                           = LDAP_UNSET_DEBUG_VALUE;
                        }
                     }
		     
                     if (sockscf.state.ldapauthentication.auto_off != -1) {
                        if (ldapauthentication.auto_off == -1)
                           ldapauthentication.auto_off
                           = rule->state.ldapauthentication.auto_off;
                        else if (ldapauthentication.auto_off
                        != rule->state.ldapauthentication.auto_off) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.auto_off varies, "
                                "%d ne %d",
                                function,
                                (int)ldapauthentication.auto_off,
                                (int)rule->state.ldapauthentication.auto_off);

                           sockscf.state.ldapauthentication.auto_off = -1;
                        }
                     }

                     if (sockscf.state.ldapauthentication.ssl != -1) {
                        if (ldapauthentication.ssl == -1)
                           ldapauthentication.ssl
                           = rule->state.ldapauthentication.ssl ;
                       else if (ldapauthentication.ssl
                       != rule->state.ldapauthentication.ssl) {
                          slog(LOG_DEBUG,
                               "%s: ldapauthentication.ssl varies, %d ne %d",
                               function,
                               (int)ldapauthentication.ssl ,
                               (int)rule->state.ldapauthentication.ssl);

                           sockscf.state.ldapauthentication.ssl = -1;
                        }
                     }

                     if (sockscf.state.ldapauthentication.certcheck != -1) {
                        if (ldapauthentication.certcheck == -1)
                           ldapauthentication.certcheck
                           = rule->state.ldapauthentication.certcheck;
                        else if (ldapauthentication.certcheck
                        != rule->state.ldapauthentication.certcheck) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.certcheck varies, "
                                "%d ne %d",
                                function,
                                (int)ldapauthentication.certcheck,
                                (int)rule->state.ldapauthentication.certcheck);

                           sockscf.state.ldapauthentication.certcheck = -1;
                        }
                     }

                     if (sockscf.state.ldapauthentication.port != -1) {
                        if (ldapauthentication.port == -1)
                           ldapauthentication.port
                           = rule->state.ldapauthentication.port;
                        else if (ldapauthentication.port
                        != rule->state.ldapauthentication.port) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.port varies, %d ne %d",
                                function,
                                ldapauthentication.port,
                                rule->state.ldapauthentication.port);

                           sockscf.state.ldapauthentication.port = -1;
                        }
                     }

                     if (sockscf.state.ldapauthentication.portssl != -1) {
                        if (ldapauthentication.portssl == -1)
                           ldapauthentication.portssl
                           = rule->state.ldapauthentication.portssl;
                        else if (ldapauthentication.portssl
                        != rule->state.ldapauthentication.portssl) {
                           slog(LOG_DEBUG,
                                "%s: ldapauthentication.portssl varies, "
                                "%d ne %d",
                                function,
                                ldapauthentication.portssl,
                                rule->state.ldapauthentication.portssl);

                           sockscf.state.ldapauthentication.portssl = -1;
                        }
                     }

                     break;

#endif /* HAVE_LDAP */

#if HAVE_GSSAPI
               case AUTHMETHOD_GSSAPI:
                  if (*sockscf.state.gssapiservicename != NUL) {
                     if (gssapiservicename == NULL) /* first gssapi rule. */
                        gssapiservicename = rule->state.gssapiservicename;
                     else if (strcmp(gssapiservicename,
                              rule->state.gssapiservicename) != 0) {
                        slog(LOG_DEBUG,
                             "%s: gssapi.servicename varies, %s ne %s",
                             function,
                             gssapiservicename,
                             rule->state.gssapiservicename);

                        *sockscf.state.gssapiservicename = NUL;
                     }
                  }
                  /* else; already found to vary. */

                  if (*sockscf.state.gssapikeytab != NUL) {
                     if (gssapikeytab == NULL) /* first gssapi rule. */
                        gssapikeytab = rule->state.gssapikeytab;
                     else if (strcmp(gssapikeytab, rule->state.gssapikeytab)
                     != 0) {
                        slog(LOG_DEBUG, "%s: gssapi.keytab varies, %s ne %s",
                             function,
                             gssapikeytab,
                             rule->state.gssapikeytab);

                        *sockscf.state.gssapikeytab = NUL;
                     }
                  }
                  /* else; already found to vary. */

                  break;
#endif /* HAVE_GSSAPI */

               default:
                  break;
            }
         }

#if BAREFOOTD
         if (rule->type == object_crule) {
            if (rule->state.protocol.tcp)
               /*
                * Add all "to:" addresses to the list of internal interfaces;
                * barefootd doesn't use a separate "internal:" keyword for it.
                */
                addinternal(&rule->dst, SOCKS_TCP);

            if (rule->state.protocol.udp)
               sockscf.state.alludpbounced = 0;
         }
#endif /* BAREFOOTD */

      }
   }

   /*
    * Check that the main configured privileges work.
    */
   sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
   sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);

   sockd_priv(SOCKD_PRIV_UNPRIVILEGED, PRIV_ON);
   sockd_priv(SOCKD_PRIV_UNPRIVILEGED, PRIV_OFF);

   sockd_priv(SOCKD_PRIV_LIBWRAP, PRIV_ON);
   sockd_priv(SOCKD_PRIV_LIBWRAP, PRIV_OFF);

#if !HAVE_PRIVILEGES
   SASSERTX(sockscf.state.euid == geteuid());
   SASSERTX(sockscf.state.egid == getegid());

   if (sockscf.uid.unprivileged_uid == 0)
      swarnx("%s: setting the unprivileged uid to %d is not recommended "
             "for security reasons",
             function, sockscf.uid.unprivileged_uid);

#if HAVE_LIBWRAP
   if (usinglibwrap && sockscf.uid.libwrap_uid == 0)
      swarnx("%s: setting the libwrap uid to %d is almost never needed, and "
             "is not recommended for security reasons",
             function, sockscf.uid.libwrap_uid);
#endif /* HAVE_LIBWRAP */
#endif /* !HAVE_PRIVILEGES */

#if HAVE_PAM
   if (*sockscf.state.pamservicename != NUL
   &&  pamservicename                != NULL) {
      /*
       * pamservicename does not vary, but is not necessarily the
       * the same as sockscf.state.pamservicename (default).
       * If it is not, set sockscf.state.pamservicename to
       * what the user used in one or more of the rules, since
       * it is the same in all rules, i.e. making it that value
       * we use to make passworddbisunique() work as expected.
       *
       * Likewise for bsdauth, gssapi, etc.
      */

      if (strcmp(pamservicename, sockscf.state.pamservicename) != 0)
         STRCPY_CHECKLEN(sockscf.state.pamservicename,
                         pamservicename,
                         sizeof(sockscf.state.pamservicename) - 1,
                         yyerrorx);
   }
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   if (*sockscf.state.bsdauthstylename != NUL
   &&  bsdauthstylename                != NULL) {
      if (strcmp(bsdauthstylename, sockscf.state.bsdauthstylename) != 0)
         STRCPY_CHECKLEN(sockscf.state.bsdauthstylename,
                         bsdauthstylename,
                         sizeof(sockscf.state.bsdauthstylename) - 1,
                         yyerrorx);
   }
#endif /* HAVE_BSDAUTH */

# if HAVE_LDAP

   if (sockscf.state.ldapauthentication.ldapurl != NULL
   &&  ldapauthentication.ldapurl               != NULL)
       sockscf.state.ldapauthentication.ldapurl = ldapauthentication.ldapurl;

   if (*sockscf.state.ldapauthentication.certfile != NUL
   &&  *ldapauthentication.certfile               != NUL)
      if (strcmp(ldapauthentication.certfile,
                 sockscf.state.ldapauthentication.certfile) != 0)
         STRCPY_CHECKLEN(sockscf.state.ldapauthentication.certfile,
                         ldapauthentication.certfile,
                         sizeof(sockscf.state.ldapauthentication.certfile) - 1,
                         yyerrorx);

   if (*sockscf.state.ldapauthentication.certpath != NUL
   &&  *ldapauthentication.certpath               != NUL)
      if (strcmp(ldapauthentication.certpath,
                 sockscf.state.ldapauthentication.certpath) != 0)
         STRCPY_CHECKLEN(sockscf.state.ldapauthentication.certpath,
                         ldapauthentication.certpath,
                         sizeof(sockscf.state.ldapauthentication.certpath) - 1,
                         yyerrorx);

   if (*sockscf.state.ldapauthentication.keytab != NUL
   &&  *ldapauthentication.keytab               != NUL)
      if (strcmp(ldapauthentication.keytab,
                 sockscf.state.ldapauthentication.keytab) != 0)
         STRCPY_CHECKLEN(sockscf.state.ldapauthentication.keytab,
                         ldapauthentication.keytab,
                         sizeof(sockscf.state.ldapauthentication.keytab) - 1,
                         yyerrorx);

   if (ldom             == -1
   &&  *ldapauthentication.domain != NUL)
      if (strcmp(ldapauthentication.domain,
                 sockscf.state.ldapauthentication.domain) != 0)
         STRCPY_CHECKLEN(sockscf.state.ldapauthentication.domain,
                         ldapauthentication.domain,
                         sizeof(sockscf.state.ldapauthentication.domain) - 1,
                         yyerrorx);

   if (lfil              == -1
   &&  *ldapauthentication.filter  != NUL)
      if (strcmp(ldapauthentication.filter,
                 sockscf.state.ldapauthentication.filter) != 0)
         STRCPY_CHECKLEN(sockscf.state.ldapauthentication.filter,
                         ldapauthentication.filter,
                         sizeof(sockscf.state.ldapauthentication.filter) - 1,
                         yyerrorx);

   if (sockscf.state.ldapauthentication.debug != LDAP_UNSET_DEBUG_VALUE
   &&  ldapauthentication.debug               != LDAP_UNSET_DEBUG_VALUE)
      sockscf.state.ldapauthentication.debug = ldapauthentication.debug;

   if (sockscf.state.ldapauthentication.auto_off != -1
    && ldapauthentication.auto_off               != -1)
      sockscf.state.ldapauthentication.auto_off = ldapauthentication.auto_off;

   if (sockscf.state.ldapauthentication.ssl != -1
   &&  ldapauthentication.ssl               != -1)
      sockscf.state.ldapauthentication.ssl = ldapauthentication.ssl;

   if (sockscf.state.ldapauthentication.certcheck != -1
   &&  ldapauthentication.certcheck               != -1)
      sockscf.state.ldapauthentication.certcheck = ldapauthentication.certcheck;

   if (sockscf.state.ldapauthentication.port != -1
   &&  ldapauthentication.port               != -1)
      sockscf.state.ldapauthentication.port = ldapauthentication.port;

   if (sockscf.state.ldapauthentication.portssl != -1
   &&  ldapauthentication.portssl               != -1)
      sockscf.state.ldapauthentication.portssl = ldapauthentication.portssl;

#endif /* HAVE_LDAP */

#if HAVE_GSSAPI
   if (*sockscf.state.gssapiservicename != NUL
   &&  gssapiservicename                != NULL) {
      if (strcmp(gssapiservicename, sockscf.state.gssapiservicename) != 0)
         STRCPY_CHECKLEN(sockscf.state.gssapiservicename,
                         gssapiservicename,
                         sizeof(sockscf.state.gssapiservicename) - 1,
                         yyerrorx);
   }

   if (*sockscf.state.gssapikeytab != NUL
   &&  gssapikeytab                != NULL) {
      if (strcmp(gssapikeytab, sockscf.state.gssapikeytab) != 0)
         STRCPY_CHECKLEN(sockscf.state.gssapikeytab,
                         gssapikeytab,
                         sizeof(sockscf.state.gssapikeytab) - 1,
                         yyerrorx);
   }
#endif /* HAVE_GSSAPI */

   /*
    * Go through all rules again and set default values for
    * authentication-methods based on the global method-lines, if none set.
    */
#if BAREFOOTD
   if (sockscf.internal.addrc == 0 && ALL_UDP_BOUNCED())
      serrx("%s: no client-rules to accept clients on specified", function);

#else /* !BAREFOOTD */
   if (sockscf.internal.addrc == 0)
      serrx("%s: no internal address given for server to listen for clients on",
            function);
#endif /* !BAREFOOTD */


   if (sockscf.external.addrc == 0)
      serrx("%s: no external address specified for server to use when "
            "forwarding data on behalf of clients",
            function);

   if (sockscf.external.rotation == ROTATION_NONE) {
      size_t ipv4c = 0, ipv6c = 0;

      for (i = 0; i < sockscf.external.addrc; ++i) {
         switch (sockscf.external.addrv[i].atype) {
            case SOCKS_ADDR_IPV4:
               ++ipv4c;
               break;

            case SOCKS_ADDR_IPV6:
               ++ipv6c;
               break;

            case SOCKS_ADDR_IFNAME:
            case SOCKS_ADDR_DOMAIN:
               break;

            default:
               SERRX(sockscf.external.addrv[i].atype);
         }
      }

      if (ipv4c > 1 || ipv6c > 1)
         swarnx("%s: more than one external address has been specified, but "
                "as long as external.rotation has the default value %s "
                "only the first address specified will be used",
                function, rotation2string(sockscf.external.rotation));
   }

   if (sockscf.external.rotation == ROTATION_SAMESAME
   &&  sockscf.external.addrc    == 1)
      swarnx("%s: rotation for external addresses is set to same-same, but "
             "the number of external addresses is only one, so this does "
             "not make sense",
             function);

   if (sockscf.routeoptions.maxfail == 0 && sockscf.routeoptions.badexpire != 0)
      swarnx("%s: it does not make sense to set \"route.badexpire\" "
             "when \"route.maxfail\" is set to zero",
             function);

#if COVENANT
   if (*sockscf.realmname == NUL)
      STRCPY_ASSERTSIZE(sockscf.realmname, DEFAULT_REALMNAME);
#endif /* COVENANT */

#if HAVE_SCHED_SETAFFINITY
{
   const cpusetting_t *cpuv[] = { &sockscf.cpu.mother,
                                  &sockscf.cpu.monitor,
                                  &sockscf.cpu.negotiate,
                                  &sockscf.cpu.request,
                                  &sockscf.cpu.io };

   const int proctypev[]      = { PROC_MOTHER,
                                  PROC_MONITOR,
                                  PROC_NEGOTIATE,
                                  PROC_REQUEST,
                                  PROC_IO };
   size_t i;

   for (i = 0; i < ELEMENTS(cpuv); ++i)
   if (cpuv[i]->affinity_isset && !sockd_cpuset_isok(&cpuv[i]->mask))
      serrx("%s: invalid cpu mask configured for %s process: %s",
            function,
            childtype2string(proctypev[i]),
            cpuset2string(&cpuv[i]->mask, NULL, 0));
}
#endif /* HAVE_SCHED_SETAFFINITY */

   for (i = 0; i < sockscf.external.addrc; ++i)
      if (!addrisbindable(&sockscf.external.addrv[i]))
         serrx("%s: cannot bind external address #%ld: %s",
               function,
               (long)i,
               ruleaddr2string(&sockscf.external.addrv[i], 0, NULL, 0));
}

void
add_external_safamily(safamily, globalscope)
   const sa_family_t safamily;
   const int globalscope;
{
   switch (safamily) {
      case AF_INET:
         sockscf.external.protocol.hasipv4 = 1;
         break;

      case AF_INET6:
         sockscf.external.protocol.hasipv6 = 1;

         if (globalscope)
            sockscf.external.protocol.hasipv6_globalscope = 1;

         break;

      default:
         SERRX(safamily);
   }
}


int
external_has_safamily(safamily)
   const sa_family_t safamily;
{
   const char *function = "external_has_safamily()";

   SASSERTX(sockscf.shmeminfo != NULL);

#if 0
   slog(LOG_DEBUG, "%s: hasipv4: %d, hasipv6: %d, hasipv6_globalscope: %d",
        function,
        sockscf.shmeminfo->state.external_hasipv4,
        sockscf.shmeminfo->state.external_hasipv6,
        sockscf.shmeminfo->state.external_hasipv6_globalscope);
#endif

   switch (safamily) {
      case AF_INET:
         return sockscf.external.protocol.hasipv4;

      case AF_INET6:
         return sockscf.external.protocol.hasipv6;

      default:
         SERRX(safamily);
   }
}

void
add_internal_safamily(safamily)
   const sa_family_t safamily;
{
   switch (safamily) {
      case AF_INET:
         sockscf.internal.protocol.hasipv4 = 1;
         break;

      case AF_INET6:
         sockscf.internal.protocol.hasipv6 = 1;
         break;

      default:
         SERRX(safamily);
   }
}


int
internal_has_safamily(safamily)
   const sa_family_t safamily;
{
   const char *function = "internal_has_safamily()";

   SASSERTX(sockscf.shmeminfo != NULL);

#if 0
   slog(LOG_DEBUG, "%s: hasipv4: %d, hasipv6: %d",
        function,
        sockscf.internal.protocol.hasipv4,
        sockscf.internal.protocol.hasipv6);
#endif

   switch (safamily) {
      case AF_INET:
         return sockscf.internal.protocol.hasipv4;

      case AF_INET6:
         return sockscf.internal.protocol.hasipv6;

      default:
         SERRX(safamily);
   }
}

int
external_has_only_safamily(safamily)
   const sa_family_t safamily;
{

   switch (safamily) {
      case AF_INET:
         return   (external_has_safamily(AF_INET)
               && !external_has_safamily(AF_INET6));

      case AF_INET6:
         return   (external_has_safamily(AF_INET6)
               && !external_has_safamily(AF_INET));

      default:
         SERRX(safamily);
   }
}


int
external_has_global_safamily(safamily)
   const sa_family_t safamily;
{

   SASSERTX(sockscf.shmeminfo != NULL);

   switch (safamily) {
      case AF_INET: /* don't care about scope for IPv4. */
         return sockscf.external.protocol.hasipv4;

      case AF_INET6:
         return sockscf.external.protocol.hasipv6_globalscope;

      default:
         SERRX(safamily);
   }
}




static void
add_more_old_shmem(config, memc, memv)
   struct config *config;
   const size_t memc;
   const oldshmeminfo_t memv[];
{
   const char *function = "add_more_old_shmem()";
   void *old;
   size_t i;

   if ((old = realloc(config->oldshmemv,
                      sizeof(*config->oldshmemv) * (config->oldshmemc + memc)))
   == NULL) {
      swarn("%s: could not allocate %lu bytes of memory to "
            "hold old shmids for later removal",
            function,
            (unsigned long)(sizeof(*config->oldshmemv)
                            * (config->oldshmemc + memc)));
      return;
   }
   config->oldshmemv = old;

   for (i = 0; i < memc; ++i) {
      const char *type;

      switch (memv[i].type) {
         case SHMEM_BW:
            type = "bw";
            break;

         case SHMEM_MONITOR:
            type = "monitor";
            break;

         case SHMEM_SS:
            type = "session";
            break;

         default:
            SERRX(memv[i].type);
      }

      slog(LOG_DEBUG,
           "%s: saving old shmem-object of type %lu (%s), with "
           "shmid %lu/key %lu, at index #%lu, for removal upon exit",
           function,
           (unsigned long)memv[i].type,
           type,
           (unsigned long)memv[i].id,
           (unsigned long)memv[i].key,
           (unsigned long)i);

      config->oldshmemv[config->oldshmemc++] = memv[i];
   }
}

static int
safamily_isenabled(safamily, addrstr, side)
   const sa_family_t safamily;
   const char *addrstr;
   const interfaceside_t side;
{
   const char *function = "safamily_isenabled()";
   interfaceprotocol_t *interface;
   int isenabled;

   switch (side) {
      case INTERNALIF:
         interface = &sockscf.internal.protocol;
         break;

      case EXTERNALIF:
         interface = &sockscf.external.protocol;
         break;

      default:
         SERRX(side);
   }

   isenabled = 0;
   switch (safamily) {
      case AF_INET:
         if (interface->ipv4)
            isenabled = 1;
         break;

      case AF_INET6:
         if (interface->ipv6)
            isenabled = 1;
         break;

      default:
         SERRX(safamily);
   }

   if (!isenabled)
      slog(LOG_DEBUG,
           "%s: address family %s option is not enabled on the %s-side "
           "interface. Can not use address \"%s\"",
           function,
           safamily2string(safamily),
           interfaceside2string(side),
           addrstr);

   return isenabled;
}

static int
addexternaladdr(ra)
   const struct ruleaddr_t *ra;
{
   void *old;

   switch (ra->atype) {
      case SOCKS_ADDR_IPV4:
      case SOCKS_ADDR_IPV6:
         if (!safamily_isenabled(atype2safamily(ra->atype),
                                 ruleaddr2string(ra, ADDRINFO_ATYPE, NULL, 0),
                                 EXTERNALIF))
            return -1;
         break;

      default:
         /*
          * Will be resolved when needed.
          */
         break;
   }

   old = realloc(sockscf.external.addrv,
                sizeof(*sockscf.external.addrv) * (sockscf.external.addrc + 1));

   if (old == NULL)
      yyerror(NOMEM);

   sockscf.external.addrv = old;

   sockscf.external.addrv[sockscf.external.addrc++] = *ra;
   return 0;
}


static int
addinternaladdr(ifname, sa, protocol)
   const char *ifname;
   const struct sockaddr_storage *sa;
   const int protocol;
{
   const char *function = "addinternaladdr()";
   void *old;

   if (!safamily_isenabled(sa->ss_family,
                           sockaddr2string2(sa, ADDRINFO_ATYPE, NULL, 0),
                           INTERNALIF))
      return -1;

   slog(LOG_DEBUG, "%s: adding address %s on nic %s to the internal list",
        function, sockaddr2string(sa, NULL, 0), ifname);

   old = realloc(sockscf.internal.addrv,
                sizeof(*sockscf.internal.addrv) * (sockscf.internal.addrc + 1));

   if (old == NULL)
      yyerror(NOMEM);

   sockscf.internal.addrv = old;

#if 0
   bzero(&sockscf.internal.addrv[sockscf.internal.addrc],
         sizeof(sockscf.internal.addrv[sockscf.internal.addrc]));
#endif
   sockaddrcpy(&sockscf.internal.addrv[sockscf.internal.addrc].addr,
               sa,
               sizeof(sockscf.internal.addrv[sockscf.internal.addrc].addr));

   sockscf.internal.addrv[sockscf.internal.addrc].protocol = protocol;
   sockscf.internal.addrv[sockscf.internal.addrc].s        = -1;

   ++sockscf.internal.addrc;

   add_internal_safamily(sa->ss_family);

   return 0;
}
