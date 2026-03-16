/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014, 2019, 2024
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
"$Id: showconfig.c,v 1.34.4.3.6.2.4.2 2024/11/20 22:03:27 karls Exp $";

void
showtimeout(timeout)
   const timeout_t *timeout;
{

   slog(LOG_DEBUG, "connect timeout: %lds%s",
        (long)timeout->connect,
        timeout->connect == 0 ? " (use kernel default)": "");

#if !SOCKS_CLIENT
   slog(LOG_DEBUG, "negotiate timeout: %lds%s",
        (long)timeout->negotiate,
        timeout->negotiate == 0 ? " (use kernel default)" : "");

   slog(LOG_DEBUG, "i/o timeout: tcp: %lds, udp: %lds",
                   (long)timeout->tcpio, (long)timeout->udpio);

   slog(LOG_DEBUG, "tcp fin-wait-2 timeout: %lds%s",
        (long)timeout->tcp_fin_wait,
        timeout->tcp_fin_wait == 0 ? " (use kernel default)" : "");
#endif /* !SOCKS_CLIENT */

}

void
showstate(state)
   const serverstate_t *state;
{
   char buf[1024];
   size_t bufused;

   slog(LOG_DEBUG, "command(s): %s",
        commands2string(&state->command, buf, sizeof(buf)));

   bufused = snprintf(buf, sizeof(buf), "extension(s): ");
   if (state->extension.bind)
      snprintf(&buf[bufused], sizeof(buf) - bufused, "bind");
   slog(LOG_DEBUG, "%s", buf);

   bufused = snprintf(buf, sizeof(buf), "protocol(s): ");
   protocols2string(&state->protocol,
   &buf[bufused], sizeof(buf) - bufused);
   slog(LOG_DEBUG, "%s", buf);

   showmethod(object_crule, state->cmethodc, state->cmethodv);
   showmethod(object_srule, state->smethodc, state->smethodv);

   slog(LOG_DEBUG, "proxyprotocol(s): %s",
   proxyprotocols2string(&state->proxyprotocol, buf, sizeof(buf)));

#if HAVE_GSSAPI
   if (methodisset(AUTHMETHOD_GSSAPI, state->smethodv, state->smethodc)) {
      if (*state->gssapiservicename != NUL)
         slog(LOG_DEBUG, "gssapi.servicename: %s", state->gssapiservicename);

      if (*state->gssapikeytab != NUL)
         slog(LOG_DEBUG, "gssapi.keytab: %s", state->gssapikeytab);

      if (state->gssapiencryption.clear
      ||  state->gssapiencryption.integrity
      ||  state->gssapiencryption.confidentiality
      || state->gssapiencryption.permessage)
         slog(LOG_DEBUG, "gssapi.encryption:%s%s%s%s",
         state->gssapiencryption.clear?           " clear"           :"",
         state->gssapiencryption.integrity?       " integrity"       :"",
         state->gssapiencryption.confidentiality? " confidentiality" :"",
         state->gssapiencryption.permessage?      " permessage"      :"");

      if (state->gssapiencryption.nec)
         slog(LOG_DEBUG, "clientcompatibility: necgssapi enabled");
   }
#endif /* HAVE_GSSAPI */
}

void
showmethod(type, methodc, methodv)
   objecttype_t type;
   size_t methodc;
   const int *methodv;
{
   char buf[1024];

   slog(LOG_DEBUG, "%s(s): %s",
        type == object_crule ? "clientmethod" : "socksmethod",
        methods2string(methodc, methodv, buf, sizeof(buf)));
}

void
showconfig(sockscf)
   const struct config *sockscf;
{
#if !SOCKS_CLIENT
   static int firsttime = 1;
   size_t bufused;
#if HAVE_SCHED_SETAFFINITY
   size_t setsize;
#endif /* HAVE_SCHED_SETAFFINITY */
#endif /* !SOCKS_CLIENT */

   char buf[4096];
   size_t i;

#if !SOCKS_CLIENT
   slog(LOG_DEBUG, "cmdline options:\n%s",
        options2string(&sockscf->option, "", buf, sizeof(buf)));

   if (sockscf->child.maxrequests != 0)
      slog(LOG_DEBUG, "max requests for a child to handle: %lu",
           (unsigned long)sockscf->child.maxrequests);

   if (sockscf->child.maxlifetime != 0)
      slog(LOG_DEBUG, "max lifetime for a child: %lu seconds",
           (unsigned long)sockscf->child.maxlifetime);

   slog(LOG_DEBUG, "address-families to look for on internal interface: %s",
        interfaceprotocol2string(&sockscf->internal.protocol, NULL, 0));

   slog(LOG_DEBUG, "internal addresses (%lu):",
        (unsigned long)sockscf->internal.addrc);
   for (i = 0; i < sockscf->internal.addrc; ++i)
      slog(LOG_DEBUG, "\t%s %s",
           protocol2string(sockscf->internal.addrv[i].protocol),
           sockaddr2string2(&sockscf->internal.addrv[i].addr,
                            ADDRINFO_ATYPE | ADDRINFO_PORT,
                            NULL,
                            0));

#if HAVE_SCHED_SETSCHEDULER
   bufused = 0;
   if (sockscf->cpu.mother.scheduling_isset)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%s: %s/%d, ",
                          childtype2string(PROC_MOTHER),
                          numeric2cpupolicy(sockscf->cpu.mother.policy),
                          sockscf->cpu.mother.param.sched_priority);

   if (sockscf->cpu.monitor.scheduling_isset)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%s: %s/%d, ",
                          childtype2string(PROC_MONITOR),
                          numeric2cpupolicy(sockscf->cpu.monitor.policy),
                          sockscf->cpu.monitor.param.sched_priority);


   if (sockscf->cpu.negotiate.scheduling_isset)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%s: %s/%d, ",
                          childtype2string(PROC_NEGOTIATE),
                          numeric2cpupolicy(sockscf->cpu.negotiate.policy),
                          sockscf->cpu.negotiate.param.sched_priority);

   if (sockscf->cpu.request.scheduling_isset)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%s: %s/%d, ",
                          childtype2string(PROC_REQUEST),
                          numeric2cpupolicy(sockscf->cpu.request.policy),
                          sockscf->cpu.request.param.sched_priority);

   if (sockscf->cpu.io.scheduling_isset)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                          "%s: %s/%d, ",
                          childtype2string(PROC_IO),
                          numeric2cpupolicy(sockscf->cpu.io.policy),
                          sockscf->cpu.io.param.sched_priority);

   if (bufused != 0)
      slog(LOG_DEBUG, "cpu scheduling settings: %s", buf);
#endif /* HAVE_SCHED_SETSCHEDULER */

#if HAVE_SCHED_SETAFFINITY
   bufused = 0;
   setsize = cpu_get_setsize();
   if (sockscf->cpu.mother.affinity_isset) {
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused, "%s: ",
                          childtype2string(PROC_MOTHER));

      for (i = 0; i < setsize; ++i)
         if (cpu_isset(i, &sockscf->cpu.mother.mask))
            bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                                "%d ", (int)i);
   }

   if (sockscf->cpu.monitor.affinity_isset) {
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused, "%s%s: ",
                          bufused ? ", " : "",
                          childtype2string(PROC_MONITOR));

      for (i = 0; i < setsize; ++i)
         if (cpu_isset(i, &sockscf->cpu.monitor.mask))
            bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                                "%d ", (int)i);
   }

   if (sockscf->cpu.negotiate.affinity_isset) {
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused, "%s%s: ",
                          bufused ? ", " : "",
                          childtype2string(PROC_NEGOTIATE));

      for (i = 0; i < setsize; ++i)
         if (cpu_isset(i, &sockscf->cpu.negotiate.mask))
            bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                                "%d ", (int)i);
   }

   if (sockscf->cpu.request.affinity_isset) {
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused, "%s%s: ",
                          bufused ? ", " : "",
                          childtype2string(PROC_REQUEST));

      for (i = 0; i < setsize; ++i)
         if (cpu_isset(i, &sockscf->cpu.request.mask))
            bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                                "%d ", (int)i);
   }

   if (sockscf->cpu.io.affinity_isset) {
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused, "%s%s: ",
                          bufused ? ", " : "",
                          childtype2string(PROC_IO));

      for (i = 0; i < setsize; ++i)
         if (cpu_isset(i, &sockscf->cpu.io.mask))
            bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                                "%d ", (int)i);
   }

   if (bufused != 0)
      slog(LOG_DEBUG, "cpu affinity settings: %s", buf);
#endif /* HAVE_SCHED_SETAFFINITY */

   slog(LOG_DEBUG, "global socket options on the internal side:");
   for (i = 0; i < sockscf->socketoptionc; ++i)
      if (sockscf->socketoptionv[i].isinternalside)
         slog(LOG_DEBUG, "   option #%lu: %s, value %s\n",
              (unsigned long)i,
              sockopt2string(&sockscf->socketoptionv[i], NULL, 0),
              sockoptval2string(sockscf->socketoptionv[i].optval,
                                sockscf->socketoptionv[i].opttype,
                                NULL,
                                0));


   slog(LOG_DEBUG, "address-families to look for on external interface: %s",
        interfaceprotocol2string(&sockscf->external.protocol, NULL, 0));

   slog(LOG_DEBUG, "external addresses (%lu):",
        (unsigned long)sockscf->external.addrc);
   for (i = 0; i < sockscf->external.addrc; ++i) {
      slog(LOG_DEBUG, "\t%s",
           ruleaddr2string(&sockscf->external.addrv[i],
                           ADDRINFO_ATYPE,
                           NULL,
                           0));
   }

   slog(LOG_DEBUG, "\thave IPv4 on external address list?  %s.  IPv6?  %s",
        external_has_safamily(AF_INET)  ? "Yes" : "No",
        external_has_safamily(AF_INET6) ?
               external_has_global_safamily(AF_INET6) ?
                  "Yes (global)" : "Yes (local only)"
            :  "No");

   slog(LOG_DEBUG, "global socket options on the external side:");
   for (i = 0; i < sockscf->socketoptionc; ++i)
      if (!sockscf->socketoptionv[i].isinternalside)
         slog(LOG_DEBUG, "   option #%lu: %s, value %s\n",
              (unsigned long)i,
              sockopt2string(&sockscf->socketoptionv[i], NULL, 0),
              sockoptval2string(sockscf->socketoptionv[i].optval,
                                sockscf->socketoptionv[i].opttype, NULL, 0));

   slog(LOG_DEBUG, "external address rotation: %s",
        rotation2string(sockscf->external.rotation));

   slog(LOG_DEBUG, "compatibility options: %s",
        compats2string(&sockscf->compat, buf, sizeof(buf)));

   slog(LOG_DEBUG, "extensions enabled: %s",
        extensions2string(&sockscf->extension, buf, sizeof(buf)));

   slog(LOG_DEBUG, "connect udp sockets to destination: %s",
        sockscf->udpconnectdst ? "yes" : "no");

   showlogspecial(&sockscf->internal.log, INTERNALIF);
   showlogspecial(&sockscf->external.log, EXTERNALIF);

#endif /* !SOCKS_CLIENT */

   slog(LOG_DEBUG, "logoutput goes to: %s",
        logtypes2string(&sockscf->log, buf, sizeof(buf)));

   slog(LOG_DEBUG, "resolveprotocol: %s",
        resolveprotocol2string(sockscf->resolveprotocol));

   showtimeout(&sockscf->timeout);

   slog(LOG_DEBUG, "global route options: %s",
        routeoptions2string(&sockscf->routeoptions, buf, sizeof(buf)));

   slog(LOG_DEBUG, "direct route fallback: %s",
        sockscf->option.directfallback ? "enabled" : "disabled");

#if !SOCKS_CLIENT
   srchosts2string(&sockscf->srchost, "", buf, sizeof(buf));
   if (*buf != NUL)
      slog(LOG_DEBUG, "srchost:\n%s", buf);

#if COVENANT
   slog(LOG_DEBUG, "proxy realm: %s", sockscf->realmname);
#endif /* COVENANT */

#if HAVE_LIBWRAP
   if (sockscf->option.hosts_access)
      slog(LOG_DEBUG, "libwrap.hosts_access: yes");
   else
      slog(LOG_DEBUG, "libwrap.hosts_access: no");
#endif /* HAVE_LIBWRAP */

#if !HAVE_PRIVILEGES
   slog(LOG_DEBUG, "euid: %d", (int)sockscf->initial.euid);

   slog(LOG_DEBUG, "userid:\n%s",
        userids2string(&sockscf->uid, "", buf, sizeof(buf)));
#endif /* !HAVE_PRIVILEGES */

   bufused = snprintf(buf, sizeof(buf), "clientmethod(s): ");
   for (i = 0; (size_t)i < sockscf->cmethodc; ++i)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused, "%s%s",
      i > 0 ? ", " : "", method2string(sockscf->cmethodv[i]));

   slog(LOG_DEBUG, "%s", buf);

   bufused = snprintf(buf, sizeof(buf), "socksmethod(s): ");
   for (i = 0; (size_t)i < sockscf->smethodc; ++i)
      bufused += snprintf(&buf[bufused], sizeof(buf) - bufused,
                         "%s%s",
                         i > 0 ? ", " : "",
                         method2string(sockscf->smethodv[i]));

   slog(LOG_DEBUG, "%s", buf);

#endif /* !SOCKS_CLIENT */


   if (sockscf->option.debug) {
      route_t *route;
#if !SOCKS_CLIENT
      rule_t *rule;
      monitor_t *monitor;

      for (i = 0, rule = sockscf->crule; rule != NULL; rule = rule->next)
         ++i;
      slog(LOG_DEBUG, "%ss (%lu): ",
           objecttype2string(object_crule), (unsigned long)i);

      for (rule = sockscf->crule; rule != NULL; rule = rule->next)
         showrule(rule, object_crule);

      for (i = 0, rule = sockscf->hrule; rule != NULL; rule = rule->next)
         ++i;

      slog(LOG_DEBUG, "%ss (%lu): ",
           objecttype2string(object_hrule), (unsigned long)i);

      for (rule = sockscf->hrule; rule != NULL; rule = rule->next)
         showrule(rule, object_hrule);

      for (i = 0, rule = sockscf->srule; rule != NULL; rule = rule->next)
         ++i;

      slog(LOG_DEBUG, "%ss (%lu): ",
           objecttype2string(object_srule), (unsigned long)i);

      for (rule = sockscf->srule; rule != NULL; rule = rule->next)
         showrule(rule, object_srule);

#endif /* !SOCKS_CLIENT */

      for (i = 0, route = sockscf->route; route != NULL; route = route->next)
         ++i;
      slog(LOG_DEBUG, "routes (%lu): ", (unsigned long)i);
      for (route = sockscf->route; route != NULL; route = route->next)
         socks_showroute(route);

#if !SOCKS_CLIENT
      for (i = 0, monitor = sockscf->monitor;
           monitor != NULL;
           monitor = monitor->next)
              ++i;

      slog(LOG_DEBUG, "monitors (%lu): ", (unsigned long)i);
      for (monitor = sockscf->monitor; monitor != NULL; monitor = monitor->next)
         showmonitor(monitor);
#endif /* !SOCKS_CLIENT */

   }

#if !SOCKS_CLIENT
   if (firsttime) {
      slog(LOG_DEBUG, "shmemconfigfd: %d, hostfd: %d, "
#if HAVE_LDAP
                      "ldapfd: %d, "
#endif /* HAVE_LDAP */
                      "loglock: %d, shmemfd: %d, ",
                      sockscf->shmemconfigfd,
                      sockscf->hostfd,
#if HAVE_LDAP
                      sockscf->ldapfd,
#endif /* HAVE_LDAP */
                      sockscf->loglock,
                      sockscf->shmemfd);

      firsttime = 0;
   }
#endif /* !SOCKS_CLIENT */
}

void
socks_showroute(route)
   const route_t *route;
{
   char gwstring[MAXGWSTRING], addr[MAXRULEADDRSTRING];
   size_t i;

   slog(LOG_DEBUG, "route #%d", route->number);

   slog(LOG_DEBUG, "src: %s",
        ruleaddr2string(&route->src, ADDRINFO_PORT, addr, sizeof(addr)));

   slog(LOG_DEBUG, "dst: %s",
        ruleaddr2string(&route->dst, ADDRINFO_PORT, addr, sizeof(addr)));

   slog(LOG_DEBUG, "gateway: %s",
        sockshost2string(&route->gw.addr, gwstring, sizeof(gwstring)));

   showstate(&route->gw.state);

   if (route->rdr_from.atype != SOCKS_ADDR_NOTSET)
      slog(LOG_DEBUG, "redirect from: %s",
          ruleaddr2string(&route->rdr_from, ADDRINFO_PORT, addr, sizeof(addr)));

   for (i = 0; i < route->socketoptionc; ++i)
      slog(LOG_DEBUG, "socketoption %s", route->socketoptionv[i].info->name);

   slog(LOG_DEBUG, "route state: autoadded: %s, failed: %lu, badtime: %ld",
                   route->state.autoadded ? "yes" : "no",
                   (long)route->state.failed,
                   (long)route->state.badtime);
}

#if !SOCKS_CLIENT

void
showlogspecial(log, side)
   const logspecial_t *log;
   const interfaceside_t side;
{
   size_t i;

   if (log->protocol.tcp.disabled.isconfigured) {
      slog(LOG_DEBUG,
           "warn if the following %s options are not enabled on the %s side: "
           "ECN: %swarning, SACK: %swarning, TIMESTAMPS: %swarning, "
           "WSCALE: %swarning",
           protocol2string(SOCKS_TCP),
           interfaceside2string(side),
           log->protocol.tcp.disabled.ecn        ? "" : "no ",
           log->protocol.tcp.disabled.sack       ? "" : "no ",
           log->protocol.tcp.disabled.timestamps ? "" : "no ",
           log->protocol.tcp.disabled.wscale     ? "" : "no ");
   }

   if (log->protocol.tcp.enabled.isconfigured) {
      slog(LOG_DEBUG,
           "warn if the following %s options are enabled on the %s side: "
           "ECN: %swarning, SACK: %swarning, TIMESTAMPS: %swarning, "
           "WSCALE: %swarning",
           protocol2string(SOCKS_TCP),
           interfaceside2string(side),
           log->protocol.tcp.enabled.ecn        ? "" : "no ",
           log->protocol.tcp.enabled.sack       ? "" : "no ",
           log->protocol.tcp.enabled.timestamps ? "" : "no ",
           log->protocol.tcp.enabled.wscale     ? "" : "no ");
   }

   for (i = 0; i < ELEMENTS(log->errno_loglevelv); ++i) {
      if (log->errno_loglevelc[i] > 0) {
         size_t ii;

         slog(LOG_DEBUG,
              "extra errno values on the %s side for loglevel %s (%lu):",
              interfaceside2string(side),
              loglevel2string(i), (unsigned long)i);

         for (ii = 0; ii < log->errno_loglevelc[i]; ++ii)
            slog(LOG_DEBUG, "%d", log->errno_loglevelv[i][ii]);
      }
   }

   for (i = 0; i < ELEMENTS(log->gaierr_loglevelv); ++i) {
      if (log->gaierr_loglevelc[i] > 0) {
         size_t ii;

         slog(LOG_DEBUG,
              "extra dnserror values on the %s side for loglevel %s (%lu):",
              interfaceside2string(side),
              loglevel2string(i),
              (unsigned long)i);

         for (ii = 0; ii < log->gaierr_loglevelc[i]; ++ii)
            slog(LOG_DEBUG, "%d", log->gaierr_loglevelv[i][ii]);
      }
   }

}

#endif /* !SOCKS_CLIENT */
