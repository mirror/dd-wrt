/*
 * Copyright (c) 2013, 2014, 2016, 2024
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
#include "monitor.h"
#include "config_parse.h"

static const char rcsid[] =
"$Id: monitor.c,v 1.125.4.8.2.2.8.2 2024/11/20 22:05:39 karls Exp $";


static void showalarms(const monitor_if_t *iface);
static void siginfo(int sig, siginfo_t *sip, void *scp);

static void
alarmcheck_disconnect(const monitor_t *monitor, alarm_disconnect_t *alarm,
                      const size_t alarmside, const struct timeval *tnow,
                      int *alarmtriggered);
/*
 * Checks the disconnect alarm "alarm", part of monitor "monitor".
 * Triggers or resets the alarm if appropriate.
 * "alarmside" is the side "alarm" belongs to (used for debug printing only).
 * "tnow" is the time now.
 *
 * If "alarmtriggered" is set upon return, a disconnect alarm was triggered
 * on this call.  If not, no disconnect alarm was triggered.
 */

/* bitmask. */
#define OP_CONNECT         (1)
#define OP_DISCONNECT      (2)
#define OP_REMOVE_SESSION  (4)

static char *op2string(const size_t op);


static void
disconnect_alarm_body(const int weclosedfirst, const size_t op,
                      rule_t *alarm, const size_t sides,
                      const clientinfo_t *cinfo,
                      const char *reason, const int lock);
/*
 * The operation for the different op values is given by the "op" value,
 * which should be a bitmask of one or more of the following.:
 *  - OP_CONNECT:  adds a connect to the disconnect alarm referenced
 *                 by shmem. "reason" should be NULL in this case.
 *
 *  - OP_DISCONNECT: adds a disconnect, decrementing the connect counter at
 *                   the same time.  In this case, "reason" must be a string
 *                   describing the reason for the disconnect, and
 *                   "weclosedfirst" indicates whether the disconnect is
 *                   considered a response to our close towards peer or not.
 *
 *  - OP_REMOVE_SESSION: decrements the session counter.
 */


static void
alarmcheck_data(const monitor_t *monitor, alarm_data_side_t *alarm,
                const size_t alarmside, const size_t sessionc,
                const struct timeval *tnow);
/*
 * Checks the data alarm "alarm", part of monitor "monitor".
 * Triggers or resets the alarm if appropriate.
 * "tnow" is the time now.
 */

static void
alarm_data_toggle(const monitor_t *monitor, const size_t alarmside,
                  alarm_data_side_t *alarm, const struct timeval *tnow);
/*
 * toggles the data-alarm "alarm", part of the monitor "monitor", on or off,
 * with appropriate log messages.
 */

static void checkmonitors(void);
/*
 * Checks all monitors, turns on/off alarms, resets, etc.
 */

static struct timeval *
timetillcheck(const monitor_t *monitor, const struct timeval *tnow,
            struct timeval *tfirstcheck, size_t *timedout);
/*
 * Calculates the time remaining till the next check specified in "monitor"
 * should be done, or NULL if no timeout is configured for this monitor.
 * The latter would likely only happen if there is a configuration error
 * by the user.
 *
 * The time till the first timeout is saved in "tfirstcheck.
 *
 * "tnow" is the current time.
 *
 * "timedout", if not NULL, is a bitmask indicating which, if any, of the
 * alarm elements for "monitor" have already timed out and should be checked
 * now.
 *
 * Returns "tfirstcheck", filled out correctly.
 */

static struct timeval *
gettimeout(struct timeval *timeout);
/*
 * Returns how long it is until the next monitor should be checked, or
 * NULL if there is no monitor that should be checked.
 */

static void proctitleupdate(void);
/*
 * Updates the process title.  "monitorc" is the number of objects
 * we are monitoring.
 */

static struct {
   size_t         alarmsactive;    /* number of alarms currently active.      */
   size_t         monitorc;        /* number of monitors we are monitoring.   */
} state;

monitor_t *
addmonitor(newmonitor)
   const monitor_t *newmonitor;
{
   const char *function = "addmonitor()";
   monitor_stats_t *mstats;
   monitor_t *monitor;

   SASSERTX(newmonitor->mstats != NULL);

   if (newmonitor->alarmsconfigured == 0)
      yywarnx("monitor has no alarms configured.  Is this intended?");

   if (newmonitor->alarm_data_aggregate & ALARM_INTERNAL) {
      SASSERTX(newmonitor->alarm_data_aggregate & ALARM_EXTERNAL);

      if (newmonitor->alarm_data_aggregate & ALARM_RECV)
         SASSERTX(newmonitor->alarm_data_aggregate & ALARM_SEND);
   }
   else if (newmonitor->alarm_data_aggregate & ALARM_RECV) {
      SASSERTX(newmonitor->alarm_data_aggregate & ALARM_SEND);
      SASSERTX(!(newmonitor->alarm_data_aggregate & ALARM_INTERNAL));
      SASSERTX(!(newmonitor->alarm_data_aggregate & ALARM_EXTERNAL));
   }

   mstats = &newmonitor->mstats->object.monitor;

   if (newmonitor->alarmsconfigured & ALARM_DATA)
      SASSERTX(mstats->internal.alarm.data.recv.isconfigured
      ||       mstats->internal.alarm.data.send.isconfigured
      ||       mstats->external.alarm.data.recv.isconfigured
      ||       mstats->external.alarm.data.send.isconfigured);

   if (newmonitor->alarmsconfigured & ALARM_DISCONNECT)
      SASSERTX(mstats->internal.alarm.disconnect.isconfigured
      ||       mstats->external.alarm.disconnect.isconfigured);


   if (mstats->internal.alarm.data.recv.isconfigured) {
      /* check data alarm settings, recv side. */
   }

   if (mstats->internal.alarm.data.send.isconfigured) {
      /* check data alarm settings, send side. */
   }

   if (mstats->internal.alarm.disconnect.isconfigured) {
      /* check disconnect alarm settings, internal side. */
      if (mstats->internal.alarm.disconnect.limit.disconnectc
      >   mstats->internal.alarm.disconnect.limit.sessionc)
         yyerrorx("internal session limit value (%lu) cannot be lower "
                  "than disconnect limit (%lu)",
                  (unsigned long)
                     mstats->internal.alarm.disconnect.limit.disconnectc,
                  (unsigned long)
                     mstats->internal.alarm.disconnect.limit.sessionc);
    }

   if (mstats->external.alarm.disconnect.isconfigured) {
      /* check disconnect alarm settings, external side. */
      if (mstats->external.alarm.disconnect.limit.disconnectc
      >   mstats->external.alarm.disconnect.limit.sessionc)
         yyerrorx("external session limit value (%lu) cannot be lower "
                  "than disconnect limit (%lu)",
                  (unsigned long)
                     mstats->external.alarm.disconnect.limit.disconnectc,
                  (unsigned long)
                     mstats->external.alarm.disconnect.limit.sessionc);
   }

   if ((monitor = malloc(sizeof(*monitor))) == NULL)
      yyerror("could not allocate %lu bytes for monitor",
              (unsigned long)sizeof(*monitor));

   *monitor                      = *newmonitor;
   monitor->type                 = object_monitor;
   monitor->mstats_isinheritable = 1;  /* always. */

   setcommandprotocol(monitor->type,
                      &monitor->state.command,
                      &monitor->state.protocol);

   if (sockscf.monitor == NULL) { /* first monitor. */
      monitor->number = 1;
      sockscf.monitor = monitor;
   }
   else { /* append this monitor to the end of our list. */
      monitor_t *last;

      last = sockscf.monitor;
      while (last->next != NULL)
         last = last->next;

      monitor->number = last->number + 1;
      last->next = monitor;
   }

   INIT_MSTATE(&monitor->mstats->mstate, monitor->type, monitor->number);
   monitor->next = NULL;

   return monitor;
}

void
showmonitor(_monitor)
   const monitor_t *_monitor;
{
   const char *function = "showmonitor()";
   monitor_t monitor = *_monitor;
   rule_t rule;

   slog(LOG_DEBUG, "monitor #%lu, line #%lu, shmid %lu",
        (unsigned long)monitor.number,
        (unsigned long)monitor.linenumber,
        (unsigned long)monitor.mstats_shmid);

   slog(LOG_DEBUG, "src: %s",
        ruleaddr2string(&monitor.src,
                        ADDRINFO_PORT | ADDRINFO_ATYPE,
                        NULL,
                        0));

   slog(LOG_DEBUG, "dst: %s",
        ruleaddr2string(&monitor.dst,
                        ADDRINFO_PORT | ADDRINFO_ATYPE,
                        NULL,
                        0));

#if HAVE_SOCKS_HOSTID
   if (monitor.hostid.atype != SOCKS_ADDR_NOTSET)
      slog(LOG_DEBUG, "hostindex: %d, hostid: %s",
           monitor.hostindex, ruleaddr2string(&monitor.hostid, 0, NULL, 0));
#endif /* HAVE_SOCKS_HOSTID */

   bzero(&rule, sizeof(rule));
   COPY_MONITORFIELDS(&monitor, &rule);
   (void)sockd_shmat(&rule, SHMEM_MONITOR);
   COPY_MONITORFIELDS(&rule, &monitor);

   if (monitor.mstats_shmid != 0) {
      const monitor_stats_t *mstats;

      SASSERTX(monitor.mstats != NULL);

      mstats = &monitor.mstats->object.monitor;

      if (mstats->internal.alarm.data.recv.isconfigured
      ||  mstats->internal.alarm.data.send.isconfigured
      ||  mstats->internal.alarm.disconnect.isconfigured) {
         slog(LOG_DEBUG, "alarms on internal interface:");
         showalarms(&mstats->internal);
      }

      if (mstats->external.alarm.data.recv.isconfigured
      ||  mstats->external.alarm.data.send.isconfigured
      ||  mstats->external.alarm.disconnect.isconfigured) {
         slog(LOG_DEBUG, "alarms on external interface:");
         showalarms(&monitor.mstats->object.monitor.external);
      }

      slog(LOG_DEBUG,
           "alarm_data_aggregate: %lu, alarm_disconnect_aggregate: %lu",
           (unsigned long)monitor.alarm_data_aggregate,
           (unsigned long)monitor.alarm_disconnect_aggregate);
   }

   sockd_shmdt(&rule, SHMEM_MONITOR);
   COPY_MONITORFIELDS(&rule, &monitor);

#if 0
   /*
    * Not implemented.
    */

   showlist(monitor.user, "user: ");
   showlist(monitor.group, "group: ");

#if HAVE_PAM
   if (methodisset(AUTHMETHOD_PAM,
                   monitor.state.methodv,
                   monitor.state.methodc))
      slog(LOG_DEBUG, "pam.servicename: %s", monitor.state.pamservicename);
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   if (methodisset(AUTHMETHOD_BSDAUTH,
                   monitor.state.methodv,
                   monitor.state.methodc))
      slog(LOG_DEBUG, "bsdauth.stylename: %s", monitor.state.bsdauthstylename);
#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP
   showlist(monitor.ldapgroup, "ldap.group: ");
   if (monitor.ldapgroup) {
      if (*monitor.state.ldap.domain != NUL)
         slog(LOG_DEBUG, "ldap.domain: %s",monitor.state.ldap.domain);

      slog(LOG_DEBUG, "ldap.auto.off: %s",
      monitor.state.ldap.auto_off ? "yes" : "no");
#if HAVE_OPENLDAP
      slog(LOG_DEBUG, "ldap.debug: %d", monitor.state.ldap.debug);
#endif
      slog(LOG_DEBUG, "ldap.keeprealm: %s",
      monitor.state.ldap.keeprealm ? "yes" : "no");

      if (*monitor.state.ldap.keytab != NUL)
         slog(LOG_DEBUG, "ldap.keytab: %s", monitor.state.ldap.keytab);

      showlist(monitor.state.ldap.ldapurl, "ldap.url: ");

      showlist(monitor.state.ldap.ldapbasedn, "ldap.basedn: ");

      if (*monitor.state.ldap.filter != NUL)
         slog(LOG_DEBUG, "ldap.filter: %s", monitor.state.ldap.filter);

      if (*monitor.state.ldap.filter_AD != NUL)
         slog(LOG_DEBUG, "ldap.filter.ad: %s", monitor.state.ldap.filter_AD);

      if (*monitor.state.ldap.attribute != NUL)
         slog(LOG_DEBUG, "ldap.attribute: %s", monitor.state.ldap.attribute);

      if (*monitor.state.ldap.attribute_AD != NUL)
         slog(LOG_DEBUG, "ldap.attribute.ad: %s",
              monitor.state.ldap.attribute_AD);

      slog(LOG_DEBUG, "ldap.mdepth: %d", monitor.state.ldap.mdepth);
      slog(LOG_DEBUG, "ldap.port: %d", monitor.state.ldap.port);
      slog(LOG_DEBUG, "ldap.ssl: %s", monitor.state.ldap.ssl ? "yes" : "no");
      slog(LOG_DEBUG, "ldap.certcheck: %s",
           monitor.state.ldap.certcheck ? "yes" : "no");

      if (*monitor.state.ldap.certfile != NUL)
         slog(LOG_DEBUG, "ldap.certfile: %s", monitor.state.ldap.certfile);

      if (*monitor.state.ldap.certpath != NUL)
         slog(LOG_DEBUG, "ldap.certpath: %s", monitor.state.ldap.certpath);
   }
#endif /* HAVE_LDAP */
#endif /* not implemented */

   showstate(&monitor.state);
}


void
run_monitor(void)
{
   const char *function = "run_monitor()";
   struct sigaction sigact;
   fd_set *rset;

   bzero(&sigact, sizeof(sigact));
   sigact.sa_flags     = SA_RESTART | SA_SIGINFO;
   sigact.sa_sigaction = siginfo;

#if HAVE_SIGNAL_SIGINFO
   if (sigaction(SIGINFO, &sigact, NULL) != 0)
      serr("%s: sigaction(SIGINFO)", function);
#endif /* HAVE_SIGNAL_SIGINFO */

   /* same handler, for systems without SIGINFO. */
   if (sigaction(SIGUSR1, &sigact, NULL) != 0)
      serr("%s: sigaction(SIGINFO)", function);

   rset = allocate_maxsize_fdset();

   sockd_print_child_ready_message((size_t)freedescriptors(NULL, NULL));

   while (1) {
      struct timeval timeout;
      int fdbits;

      errno = 0; /* reset for each iteration. */

      proctitleupdate();

      FD_ZERO(rset);
      fdbits = -1;

      /* checked so we know if mother goes away.  */
      SASSERTX(sockscf.state.mother.ack != -1);
      FD_SET(sockscf.state.mother.ack, rset);
      fdbits = MAX(fdbits, sockscf.state.mother.ack);

      ++fdbits;
      switch (selectn(fdbits,
                      rset,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      gettimeout(&timeout))) {
         case -1:
            SASSERT(ERRNOISTMP(errno));
            continue;

         case 0:
            break;

         default:
            if (FD_ISSET(sockscf.state.mother.ack, rset)) {
               /*
                * only eof expected.  Read and exit.
                */
               sockd_readmotherscontrolsocket(function,
                                              sockscf.state.mother.ack);

               /*
                * XXX for some reason Valgrind complains rset pointer is
                * lost, but this free(3)-call seems to make Valgrind
                * understand it is wrong.
                */
               free(rset);
               sockdexit(EXIT_FAILURE);
            }
      }

      checkmonitors();
   }

   /* NOTREACHED */
}

void
monitor_preconfigload(void)
{
   const char *function = "monitor_preconfigload()";

   slog(LOG_DEBUG, "%s", function);

   /*
    * will load new monitor objects (if any) from new config.
    * Disregard any old ones.
    */
   monitor_detachfromlist(sockscf.monitor);
}

void
monitor_postconfigload(void)
{
   const char *function = "monitor_postconfigload()";
   struct timeval tnow;
   monitor_t *monitor;

   /*
    * Permanently attach to all monitor objects and initialize the timestamps
    * in them according to the current time.
    */

   bzero(&state, sizeof(state));
   gettimeofday_monotonic(&tnow);

   for (monitor = sockscf.monitor; monitor != NULL; monitor = monitor->next) {
      monitor_stats_t *mstats;
      rule_t rule;
      int rc;

      SASSERTX(monitor->mstats_shmid != 0);
      SASSERTX(monitor->mstats       == NULL);

      bzero(&rule, sizeof(rule));
      COPY_MONITORFIELDS(monitor, &rule); /* tmp; sockd_shmat() expects rule */
      rc = sockd_shmat(&rule, SHMEM_MONITOR);
      COPY_MONITORFIELDS(&rule, monitor);

      if (rc != 0) {
         slog(LOG_DEBUG, "%s: could not attach to shmem segment of monitor #%lu",
              function, (unsigned long)monitor->number);

         continue;
      }

      SASSERTX(monitor->mstats != NULL);

      /*
       * Reset all counters and remain attached so we do not have to
       * constantly attach/detach for checking.
       */

      mstats = &monitor->mstats->object.monitor;

      socks_lock(sockscf.shmemfd, (off_t)monitor->mstats_shmid, 1, 1, 1);

#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
      monitor_stats_t _mstats = *mstats;
#endif /* DEBUG */

      MUNPROTECT_SHMEMHEADER(monitor->mstats);

      mstats->internal.alarm.data.recv.ison
      = mstats->internal.alarm.data.send.ison
      = mstats->external.alarm.data.recv.ison
      = mstats->external.alarm.data.send.ison = 0;

      mstats->internal.alarm.data.recv.bytes
      = mstats->internal.alarm.data.send.bytes
      = mstats->external.alarm.data.recv.bytes
      = mstats->external.alarm.data.send.bytes = 0;

      timerclear(&mstats->internal.alarm.data.recv.lastio);
      timerclear(&mstats->internal.alarm.data.send.lastio);
      timerclear(&mstats->external.alarm.data.recv.lastio);
      timerclear(&mstats->external.alarm.data.send.lastio);

      mstats->internal.alarm.data.recv.lastreset
      = mstats->internal.alarm.data.send.lastreset
      = mstats->external.alarm.data.recv.lastreset
      = mstats->external.alarm.data.send.lastreset
      = mstats->internal.alarm.disconnect.lastreset
      = mstats->external.alarm.disconnect.lastreset
      = tnow;

      MPROTECT_SHMEMHEADER(monitor->mstats);

      socks_unlock(sockscf.shmemfd, (off_t)monitor->mstats_shmid, 1);

      ++state.monitorc;
   }
}

void
monitor_detachfromlist(head)
   monitor_t *head;
{
   monitor_t *m;

   for (m = head; m != NULL; m = m->next) {
      rule_t rule;

      if (m->mstats_shmid == 0)
         continue;

      bzero(&rule, sizeof(rule));
      COPY_MONITORFIELDS(m, &rule); /* tmp; sockd_shmat() expects rule */
      sockd_shmdt(&rule, SHMEM_MONITOR);
      COPY_MONITORFIELDS(&rule, m); /* don't forget to update monitor-list. */
   }
}

void
alarm_inherit(from, cinfo_from, to, cinfo_to, sidesconnected)
   rule_t *from;
   const clientinfo_t *cinfo_from;
   rule_t *to;
   const clientinfo_t *cinfo_to;
   const size_t sidesconnected;
{
   const char *function = "alarm_inherit";

   slog(LOG_DEBUG, "%s: sidesconnected: %lu",
        function, (unsigned long)sidesconnected);

   log_ruleinfo_shmid(from, function, "from-rule");
   log_ruleinfo_shmid(to, function,   "to-rule");

   if (sidesconnected == 0)
      return;

   if (from->mstats_shmid == 0) { /* nothing to inherit from. */
      if (to->mstats_shmid != 0 && (to->alarmsconfigured & ALARM_DISCONNECT))
         /*
          * Add connect.
          */
         alarm_add_connect(to, sidesconnected, cinfo_to, sockscf.shmemfd);

      /* else; nothing to do.  No monitor before, no monitor now. */
   }
   else { /* disregard old monitor, inherit it, or nothing? */
      if (to->mstats_shmid == from->mstats_shmid) { /* nothing */
         slog(LOG_DEBUG, "%s: new rule uses same monitor as old one: shmid %lu",
              function, to->mstats_shmid);
      }
      else if (to->mstats_shmid == 0) { /* inherit */
         COPY_MONITORFIELDS(from, to);
      }
      else { /* disregard, old, use new. */
         SASSERTX(to->mstats_shmid   != 0);
         SASSERTX(from->mstats_shmid != 0);

         if (from->alarmsconfigured & ALARM_DISCONNECT) {
            /*
             * Remove connect.
             */
            alarm_remove_session(from,
                                 sidesconnected,
                                 cinfo_from,
                                 sockscf.shmemfd);
         }

         SASSERTX(to->mstats_shmid != 0);
         if (to->alarmsconfigured & ALARM_DISCONNECT)
            alarm_add_connect(to, sidesconnected, cinfo_to, sockscf.shmemfd);
      }
   }
}


const monitor_t *
monitormatch(src, dst, auth, state)
   const sockshost_t *src;
   const sockshost_t *dst;
   const authmethod_t *auth;
   const connectionstate_t *state;
{
   const char *function = "monitormatch()";
   monitor_t *monitor;
   char srcstr[MAXSOCKSHOSTSTRING], dststr[sizeof(srcstr)];

   slog(LOG_DEBUG, "%s: %s -> %s",
        function,
        src == NULL ? "N/A" : sockshost2string(src, srcstr, sizeof(srcstr)),
        dst == NULL ? "N/A" : sockshost2string(dst, dststr, sizeof(dststr)));

   for (monitor = sockscf.monitor; monitor != NULL; monitor = monitor->next) {
      if (!protocol_matches(state->protocol, &monitor->state.protocol)) {
         slog(LOG_DEBUG, "%s: monitor #%lu monitors protocols %s, but not %s",
              function,
              (unsigned long)monitor->number,
              protocols2string(&monitor->state.protocol, NULL, 0),
              protocol2string(state->protocol));

         continue;
      }

      if (!command_matches(state->command, &monitor->state.command)) {
         slog(LOG_DEBUG, "%s: monitor #%lu monitor commands %s, but not %s",
              function,
              (unsigned long)monitor->number,
              commands2string(&monitor->state.command, NULL, 0),
              command2string(state->command));

         continue;
      }

#if HAVE_SOCKS_HOSTID
      if (monitor->hostid.atype != SOCKS_ADDR_NOTSET) {
         slog(LOG_DEBUG,
              "%s: monitor #%lu requires hostid to be present on the "
              "connection ... checking ...",
              function,
              (unsigned long)monitor->number);

         if (!hostidmatches(&state->hostid,
                            monitor->hostindex,
                            &monitor->hostid,
                            object_monitor,
                            monitor->number))
            continue;
      }
#endif /* HAVE_SOCKS_HOSTID */

      if (src != NULL)
         if (!addrmatch(&monitor->src, src, NULL, state->protocol, 0))
            continue;

      if (dst != NULL)
         if (!addrmatch(&monitor->dst, dst, NULL, state->protocol, 0))
            continue;

      return monitor;
   }

   return NULL;
}

void
monitor_use(mstats, cinfo, lock)
   shmem_object_t *mstats;
   const clientinfo_t *cinfo;
   const int lock;
{
   const char *function = "monitor_use()";
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *mstats;
#endif /* DEBUG */

   SASSERTX(mstats != NULL);

   slog(LOG_DEBUG,
        "%s: cinfo = %s, lock = %d, mstats object: %p, current clientc: %ld.  "
        "Should become one more",
        function,
        clientinfo2string(cinfo, NULL, 0),
        lock,
        mstats,
        (unsigned long)mstats->mstate.clients);

   shmem_use(mstats, cinfo, lock, 0);
}

void
monitor_move(oldmonitor, newmonitor, stayattached, sidesconnected, cinfo, lock)
   monitor_t *oldmonitor;
   monitor_t *newmonitor;
   const int stayattached;
   const size_t sidesconnected;
   const clientinfo_t *cinfo;
   const int lock;
{
   const char *function = "monitor_move()";
   rule_t alarm;

   slog(LOG_DEBUG,
        "%s: shmid %lu (%p) -> shmid %lu (%p).  Old alarmsconfigured: %lu, "
        "new alarmsconfigured: %lu, sides connected: %lu",
        function,
        (unsigned long)oldmonitor->mstats_shmid,
        oldmonitor->mstats,
        (unsigned long)newmonitor->mstats_shmid,
        newmonitor->mstats,
        (unsigned long)oldmonitor->alarmsconfigured,
        (unsigned long)newmonitor->alarmsconfigured,
        (unsigned long)sidesconnected);

   /*
    * First: clean up old monitorstate.
    */

   bzero(&alarm, sizeof(alarm));
   COPY_MONITORFIELDS(oldmonitor, &alarm);

   if (alarm.mstats_shmid != 0 && alarm.mstats == NULL)
      if (sockd_shmat(&alarm, SHMEM_MONITOR) != 0)
         COPY_MONITORFIELDS(&alarm, oldmonitor);

   if (alarm.mstats_shmid != 0) {
      SASSERTX(alarm.mstats != NULL);

      socks_lock(lock, (off_t)alarm.mstats_shmid, 1, 1, 1);

      if (alarm.alarmsconfigured & ALARM_DISCONNECT)
         alarm_remove_session(&alarm, sidesconnected, cinfo, -1);

      monitor_unuse(alarm.mstats, cinfo, -1);

      socks_unlock(lock, (off_t)alarm.mstats_shmid, 1);

      SASSERTX(alarm.mstats != NULL);

      /*
       * Regardless of whether we were attached when called or not,
       * no need to remain attached to old monitor state now as we
       * are moving to the new monitor.
       */
      sockd_shmdt(&alarm, SHMEM_MONITOR);
   }

   CLEAR_MONITORFIELDS(oldmonitor);

   /*
    * Then: update to new monitorstate.
    */

   bzero(&alarm, sizeof(alarm));
   COPY_MONITORFIELDS(newmonitor, &alarm);

   if (alarm.mstats_shmid != 0 && alarm.mstats == NULL)
      if (sockd_shmat(&alarm, SHMEM_MONITOR) != 0)
         COPY_MONITORFIELDS(&alarm, newmonitor);

   if (alarm.mstats_shmid != 0) {
      SASSERTX(alarm.mstats != NULL);

      socks_lock(lock, (off_t)alarm.mstats_shmid, 1, 1, 1);

      monitor_use(alarm.mstats, cinfo, -1);

      if (alarm.alarmsconfigured & ALARM_DISCONNECT)
         alarm_add_connect(&alarm, sidesconnected, cinfo, -1);

      socks_unlock(lock, (off_t)alarm.mstats_shmid, 1);

      SASSERTX(alarm.mstats != NULL);

      if (stayattached)
         SASSERTX(alarm.mstats == newmonitor->mstats);
      else
         sockd_shmdt(&alarm, SHMEM_MONITOR);
   }
   else
      CLEAR_MONITORFIELDS(newmonitor);
}


void
monitor_unuse(mstats, cinfo, lock)
   shmem_object_t *mstats;
   const clientinfo_t *cinfo;
   const int lock;
{
   const char *function = "monitor_unuse()";
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *mstats;
#endif /* DEBUG */

   SASSERTX(mstats != NULL);

   slog(LOG_DEBUG,
        "%s: cinfo %s, lock %d, mstats object %p, shmid %lu, "
        "current clientc %lu.  Should become one less",
        function,
        clientinfo2string(cinfo, NULL, 0),
        lock,
        mstats,
        (unsigned long)mstats->mstate.shmid,
        (unsigned long)mstats->mstate.clients);

   shmem_unuse(mstats, cinfo, lock);
}

void
alarm_remove_session(alarm, sides, cinfo, lock)
   rule_t *alarm;
   const size_t sides;
   const clientinfo_t *cinfo;
   const int lock;
{

   disconnect_alarm_body(0,
                         OP_REMOVE_SESSION,
                         alarm,
                         sides,
                         cinfo,
                         NULL,
                         lock);
}

void
alarm_add_connect(alarm, sides, cinfo, lock)
   rule_t *alarm;
   const size_t sides;
   const clientinfo_t *cinfo;
   const int lock;
{

   disconnect_alarm_body(0,
                         OP_CONNECT,
                         alarm,
                         sides,
                         cinfo,
                         NULL,
                         lock);
}

void
alarm_add_disconnect(weclosedfirst, alarm, sides, cinfo, reason, lock)
   const int weclosedfirst;
   rule_t *alarm;
   const size_t sides;
   const clientinfo_t *cinfo;
   const char *reason;
   const int lock;
{

   SASSERTX(reason != NULL);
   disconnect_alarm_body(weclosedfirst,
                         OP_DISCONNECT,
                         alarm,
                         sides,
                         cinfo,
                         reason,
                         lock);
}

static void
disconnect_alarm_body(weclosedfirst, op, alarm, sides, cinfo, reason, lock)
   const int weclosedfirst;
   const size_t op;
   rule_t *alarm;
   const size_t sides;
   const clientinfo_t *cinfo;
   const char *reason;
   const int lock;
{
   const char *function = "disconnect_alarm_body()";
   const size_t sidev[] = { sides & ALARM_INTERNAL ? ALARM_INTERNAL : 0,
                            sides & ALARM_EXTERNAL ? ALARM_EXTERNAL : 0 };

   const size_t opv[]   = { (op & OP_CONNECT)        ? OP_CONNECT        : 0,
                            (op & OP_DISCONNECT)     ? OP_DISCONNECT     : 0,
                            (op & OP_REMOVE_SESSION) ? OP_REMOVE_SESSION : 0 };

   alarm_disconnect_t *disconnect;
   size_t sidec, opc, locktaken;
   int didattach;

   slog(LOG_DEBUG, "%s: op %lu, sides %lu, shmid %lu, shmem %p, cinfo %s",
        function,
        (unsigned long)op,
        (unsigned long)sides,
        (unsigned long)alarm->mstats_shmid,
        alarm->mstats,
        clientinfo2string(cinfo, NULL, 0));

   SASSERTX(alarm->mstats_shmid != 0);

   if (alarm->mstats == NULL) {
      if (sockd_shmat(alarm, SHMEM_MONITOR) != 0)
         return;

      didattach = 1;
   }
   else
      didattach = 0;

   SASSERTX(alarm->mstats != NULL);

#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   shmem_object_t _shmem = *alarm->mstats;
#endif /* DEBUG */

   locktaken = 0;

   for (opc = 0; opc < ELEMENTS(opv); ++opc) {
      if (opv[opc] == 0)
         continue;

      for (sidec = 0; sidec < ELEMENTS(sidev); ++sidec) {
         if (sidev[sidec] == 0)
            continue;

         if (sidev[sidec] == ALARM_INTERNAL)
            disconnect
            = &alarm->mstats->object.monitor.internal.alarm.disconnect;
         else {
            SASSERTX(sidev[sidec] == ALARM_EXTERNAL);

            disconnect
            = &alarm->mstats->object.monitor.external.alarm.disconnect;
         }

         slog(LOG_DEBUG,
              "%s: cinfo %s, operation %s, %s alarm configured in monitor "
              "shmid %lu for %s side.  Reason: %s.  Weclosedfirst?  %s.  "
              "Current sessionc: %lu, peer_disconnectc: %lu",
              function,
              clientinfo2string(cinfo, NULL, 0),
              op2string(opv[opc]),
              disconnect->isconfigured ? "Disconnect" : "No disconnect",
              alarm->mstats_shmid,
              alarmside2string(sidev[sidec]),
              reason == NULL ? "N/A" : reason,
              (opv[opc] & OP_DISCONNECT) ?
                  (weclosedfirst ? "Yes" : "No") : "N/A",
              (unsigned long)disconnect->sessionc,
              (unsigned long)disconnect->peer_disconnectc);

         if (!disconnect->isconfigured)
            continue;

         if (!locktaken) {
            socks_lock(lock, (off_t)alarm->mstats_shmid, 1, 1, 1);
            locktaken = 1;
         }

         MUNPROTECT_SHMEMHEADER(alarm->mstats);

         switch (opv[opc]) {
            case OP_CONNECT:
               SASSERTX(reason == NULL);

               disconnect->sessionc += 1;
               break;

            case OP_DISCONNECT:
               SASSERTX(reason != NULL);
               SASSERTX(disconnect->sessionc > 0);

               disconnect->sessionc -= 1;

               if (weclosedfirst)
                  disconnect->self_disconnectc += 1;
               else
                  disconnect->peer_disconnectc += 1;

               break;

            case OP_REMOVE_SESSION:
               SASSERTX(reason == NULL);
               SASSERTX(disconnect->sessionc > 0);

               disconnect->sessionc -= 1;
               break;

            default:
               SERRX(opv[opc]);

         }

         MPROTECT_SHMEMHEADER(alarm->mstats);
      }
   }

   if (locktaken)
      socks_unlock(lock, (off_t)alarm->mstats_shmid, 1);

   if (didattach)
      sockd_shmdt(alarm, SHMEM_MONITOR);
}

static void
checkmonitors(void)
{
   const char *function = "checkmonitors()";
   const struct timeval tzero = { 0, 0 };
   struct timeval tnow;
   monitor_t *monitor;

   slog(LOG_DEBUG, "%s: [", function);

   gettimeofday_monotonic(&tnow);

   for (monitor = sockscf.monitor; monitor != NULL; monitor = monitor->next) {
      monitor_stats_t *mstats;
      struct timeval timeout;
      size_t sessionc, hastimedout;
      int rang;

      if  (monitor->mstats_shmid == 0)
         continue;

      SASSERTX(monitor->mstats != NULL);

      (void)timetillcheck(monitor, &tnow, &timeout, &hastimedout);

      if (hastimedout == 0)
         continue;

      SASSERTX(!timercmp(&timeout, &tzero, >));
      SASSERTX(monitor->alarmsconfigured != 0);

      slog(LOG_DEBUG,
           "%s: time to check alarm on monitor #%lu, shmid %lu.  "
           "Configured alarms: %lu, hastimedout: %lu",
           function,
           (unsigned long)monitor->number,
           (unsigned long)monitor->mstats_shmid,
           (unsigned long)monitor->alarmsconfigured,
           (unsigned long)hastimedout);

      mstats = &monitor->mstats->object.monitor;
#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
      monitor_stats_t _mstats = *mstats;
#endif /* DEBUG */

      sessionc = monitor->mstats->mstate.clients,

      /*
       * Should have taken the lock before checking the timeout, but
       * seems wasteful as the only difference should be that on
       * boundary conditions, we might trigger an alarm or not trigger
       * an alarm, deciding on the exact timing of when another process
       * gets to update the alarm counters.
       */
      socks_lock(sockscf.shmemfd, (off_t)monitor->mstats_shmid, 1, 1, 1);

      if (hastimedout
      & (  ALARM_INTERNAL_RECV
         | ALARM_INTERNAL_SEND
         | ALARM_EXTERNAL_RECV
         | ALARM_EXTERNAL_SEND)) {
         if (monitor->alarm_data_aggregate != 0) {
            /*
             * Don't go through the interfaces individually, but instead
             * aggregate their values together into one.
             */
            alarm_data_side_t agg;
            struct timeval lio;
            size_t bytes, alarmsides;

            alarmsides = 0;
            bytes      = 0;
            timerclear(&lio);

            if (mstats->internal.alarm.data.recv.isconfigured) {
               agg         = mstats->internal.alarm.data.recv;
               bytes      += mstats->internal.alarm.data.recv.bytes;
               alarmsides |= ALARM_INTERNAL_RECV;

               if (timercmp(&mstats->internal.alarm.data.recv.lastio, &lio, >))
                  lio = mstats->internal.alarm.data.recv.lastio;
            }

            if (mstats->internal.alarm.data.send.isconfigured) {
               agg         = mstats->internal.alarm.data.send;
               bytes      += mstats->internal.alarm.data.send.bytes;
               alarmsides |= ALARM_INTERNAL_SEND;

               if (timercmp(&mstats->internal.alarm.data.send.lastio, &lio, >))
                  lio = mstats->internal.alarm.data.send.lastio;
            }

            if (mstats->external.alarm.data.recv.isconfigured) {
               agg         = mstats->external.alarm.data.recv;
               bytes      += mstats->external.alarm.data.recv.bytes;
               alarmsides |= ALARM_EXTERNAL_RECV;

               if (timercmp(&mstats->external.alarm.data.recv.lastio, &lio, >))
                  lio = mstats->external.alarm.data.recv.lastio;
            }

            if (mstats->external.alarm.data.send.isconfigured) {
               agg         = mstats->external.alarm.data.send;
               bytes      += mstats->external.alarm.data.send.bytes;
               alarmsides |= ALARM_EXTERNAL_SEND;

               if (timercmp(&mstats->external.alarm.data.send.lastio, &lio, >))
                  lio = mstats->external.alarm.data.send.lastio;
            }

            agg.bytes  = bytes;
            agg.lastio = lio;

            switch (monitor->alarm_data_aggregate) {
               case ALARM_INTERNAL | ALARM_EXTERNAL | ALARM_RECV |ALARM_SEND:
                  alarmsides = (size_t)-1; /* all. */
                  break;

               case ALARM_INTERNAL | ALARM_EXTERNAL:
                  if (alarmsides & (ALARM_INTERNAL_RECV | ALARM_EXTERNAL_RECV))
                     alarmsides = ALARM_RECV;
                  else {
                     SASSERTX(alarmsides
                              & (ALARM_INTERNAL_SEND | ALARM_INTERNAL_RECV));
                     alarmsides = ALARM_SEND;
                  }
                  break;

               case ALARM_RECV | ALARM_SEND:
                  if (alarmsides & (ALARM_INTERNAL_RECV | ALARM_INTERNAL_SEND))
                     alarmsides = ALARM_INTERNAL;
                  else {
                     SASSERTX(alarmsides
                              & (ALARM_EXTERNAL_RECV | ALARM_EXTERNAL_SEND));
                     alarmsides = ALARM_EXTERNAL;
                  }
                  break;

               default:
                  SERRX(monitor->alarm_data_aggregate);
            }

            alarmcheck_data(monitor, &agg, alarmsides, sessionc, &tnow);

            /*
             * Now reset all to the updated values in agg.
             */

            if (mstats->internal.alarm.data.recv.isconfigured)
               mstats->internal.alarm.data.recv = agg;

            if (mstats->internal.alarm.data.send.isconfigured)
               mstats->internal.alarm.data.send = agg;

            if (mstats->external.alarm.data.recv.isconfigured)
               mstats->external.alarm.data.recv = agg;

            if (mstats->external.alarm.data.send.isconfigured)
               mstats->external.alarm.data.send = agg;
         }
         else {
            if (hastimedout & ALARM_EXTERNAL_RECV) {
               alarm_data_side_t *alarm = &mstats->external.alarm.data.recv;

               alarmcheck_data(monitor,
                               alarm,
                               ALARM_EXTERNAL_RECV,
                               sessionc,
                               &tnow);
            }

            if (hastimedout & ALARM_EXTERNAL_SEND) {
               alarm_data_side_t *alarm = &mstats->external.alarm.data.send;

               alarmcheck_data(monitor,
                               alarm,
                               ALARM_EXTERNAL_SEND,
                               sessionc,
                               &tnow);
            }

            if (hastimedout & ALARM_INTERNAL_RECV) {
               alarm_data_side_t *alarm = &mstats->internal.alarm.data.recv;

               alarmcheck_data(monitor,
                               alarm,
                               ALARM_INTERNAL_RECV,
                               sessionc,
                               &tnow);
            }

            if (hastimedout & ALARM_INTERNAL_SEND) {
               alarm_data_side_t *alarm = &mstats->internal.alarm.data.send;

               alarmcheck_data(monitor,
                               alarm,
                               ALARM_INTERNAL_SEND,
                               sessionc,
                               &tnow);
            }
         }
      }

#define RESET_DISCONNECT(_alarm, _tnow)                                        \
do {                                                                           \
   (_alarm)->lastreset        = (_tnow);                                       \
   (_alarm)->peer_disconnectc = 0;                                             \
   (_alarm)->self_disconnectc = 0;                                             \
} while (/* CONSTCOND */ 0)

      MUNPROTECT_SHMEMHEADER(monitor->mstats);

      if (hastimedout & (ALARM_INTERNAL | ALARM_EXTERNAL)) {
         if (monitor->alarm_disconnect_aggregate != 0) {
            alarm_disconnect_t agg;

            SASSERTX(mstats->internal.alarm.disconnect.isconfigured);
            SASSERTX(mstats->external.alarm.disconnect.isconfigured);

            agg = mstats->internal.alarm.disconnect;

            agg.peer_disconnectc
            += mstats->external.alarm.disconnect.peer_disconnectc;

            agg.self_disconnectc
            += mstats->external.alarm.disconnect.self_disconnectc;

            alarmcheck_disconnect(monitor,
                                  &agg,
                                  (size_t)-1, /* all. */
                                  &tnow,
                                  &rang);

            RESET_DISCONNECT(&mstats->internal.alarm.disconnect, tnow);
            RESET_DISCONNECT(&mstats->external.alarm.disconnect, tnow);
         }
         else {
            if (hastimedout & ALARM_EXTERNAL) {
               alarm_disconnect_t *alarm = &mstats->external.alarm.disconnect;

               alarmcheck_disconnect(monitor,
                                     alarm,
                                     ALARM_EXTERNAL,
                                     &tnow,
                                     &rang);

               RESET_DISCONNECT(alarm, tnow);
            }

            if (hastimedout & ALARM_INTERNAL) {
               alarm_disconnect_t *alarm = &mstats->internal.alarm.disconnect;

               alarmcheck_disconnect(monitor,
                                     alarm,
                                     ALARM_INTERNAL,
                                     &tnow,
                                     &rang);

               RESET_DISCONNECT(alarm, tnow);
            }
         }

      }

      MPROTECT_SHMEMHEADER(monitor->mstats);

      socks_unlock(sockscf.shmemfd, (off_t)monitor->mstats_shmid, 1);
   }

   slog(LOG_DEBUG, "%s: ]", function);
}

static void
alarmcheck_disconnect(monitor, alarm, alarmside, tnow, alarmtriggered)
   const monitor_t *monitor;
   alarm_disconnect_t *alarm;
   const size_t alarmside;
   const struct timeval *tnow;
   int *alarmtriggered;
{
   const char *function = "alarmcheck_disconnect()";
   struct timeval tdiff;
   size_t sessions_this_period;
   char src[MAXRULEADDRSTRING], dst[sizeof(src)];

   *alarmtriggered = 0;

   timersub(tnow, &(alarm)->lastreset, &tdiff);

   /* we should only be called when enough time has passed. */
   SASSERTX(tdiff.tv_sec >= alarm->limit.seconds);

   slog(LOG_DEBUG,
        "%s: disconnect alarm on side %s in monitor #%lu was last reset "
        "%ld.%06lds ago.  Limit is %lds, current sessionc/peer_disconnectc/"
        "self_disconnectc is %lu/%lu/%lu",
        function,
        alarmside == (size_t)-1 ? "<all>" : alarmside2string(alarmside),
        (unsigned long)monitor->number,
        (long)tdiff.tv_sec,
        (long)tdiff.tv_usec,
        (long)alarm->limit.seconds,
        (unsigned long)alarm->sessionc,
        (unsigned long)alarm->peer_disconnectc,
        (unsigned long)alarm->self_disconnectc);

   if (alarm->peer_disconnectc < alarm->limit.disconnectc)
      return; /* no alarm.  */

   /*
    * Else: enough disconnects by absolute numbers to trigger alarm.
    * Enough disconnects by ratio also?
    */

   sessions_this_period
   = alarm->sessionc + alarm->peer_disconnectc + alarm->self_disconnectc;

   if (sessions_this_period == 0)
      return;

   slog(LOG_DEBUG, "%s: comparing %lu/%lu (%lu) vs %lu/%lu (%lu)",
        function,
        (unsigned long)alarm->peer_disconnectc,
        (unsigned long)sessions_this_period,
        (unsigned long)(alarm->peer_disconnectc  / sessions_this_period),
        (unsigned long)alarm->limit.disconnectc,
        (unsigned long)alarm->limit.sessionc,
        (unsigned long)(alarm->limit.disconnectc / alarm->limit.sessionc));

   if ((alarm->peer_disconnectc  / sessions_this_period)
   >=  (alarm->limit.disconnectc / alarm->limit.sessionc)) {
      slog(LOG_ALARM,
           "monitor(%lu): alarm/disconnect ]: %s -> %s%s%s: %lu/%lu "
           "disconnects during last %lds.  Session count: %lu",
           (unsigned long)monitor->number,
           ruleaddr2string(&monitor->src, ADDRINFO_PORT, src, sizeof(src)),
           ruleaddr2string(&monitor->dst, ADDRINFO_PORT, dst, sizeof(dst)),
           alarmside == (size_t)-1 ? "" : " ",
           alarmside == (size_t)-1 ? "" : alarmside2string(alarmside),
           (unsigned long)alarm->peer_disconnectc,
           (unsigned long)sessions_this_period,
           (long)tdiff.tv_sec,
           (unsigned long)alarm->sessionc);

      *alarmtriggered = 1;
   }
}

static void
alarmcheck_data(monitor, alarm, alarmside, sessionc, tnow)
   const monitor_t *monitor;
   alarm_data_side_t *alarm;
   const size_t alarmside;
   const size_t sessionc;
   const struct timeval *tnow;
{
   const char *function = "alarmcheck_data()";

   slog(LOG_DEBUG,
        "%s: checking monitor #%lu on side %s.  Alarm is currently %s, "
        "alarm->bytes is %lu, lastio is %ld.%06ld",
        function,
        (unsigned long)monitor->number,
        alarmside == (size_t)-1 ? "<all>" : alarmside2string(alarmside),
        alarm->ison ? "on" : "off",
        (unsigned long)alarm->bytes,
        (unsigned long)alarm->lastio.tv_sec,
        (unsigned long)alarm->lastio.tv_usec);

   MUNPROTECT_SHMEMHEADER(monitor->mstats);

   if (alarm->bytes <= alarm->limit.bytes) {
      /*
       * Alarm should trigger for this timeperiod.
       */

      if (!alarm->ison)
         alarm_data_toggle(monitor, alarmside, alarm, tnow);
      /* else; was on, leave on. */

      alarm->lastreset = *tnow;
   }
   else {
      /*
       * No alarm should trigger for this timeperiod.
       */

      if (alarm->ison) {
         alarm_data_toggle(monitor, alarmside, alarm, tnow);
         alarm->lastreset = *tnow;
      }
      else { /*
              * else; was off, leave off.
              * Next check at most one timeperiod after last i/o.
              */
         alarm->lastreset = alarm->lastio;
      }
   }

   alarm->bytes = 0;

   MPROTECT_SHMEMHEADER(monitor->mstats);
}

static void
alarm_data_toggle(monitor, alarmside, alarm, tnow)
   const monitor_t *monitor;
   const size_t alarmside;
   alarm_data_side_t *alarm;
   const struct timeval *tnow;
{
   const char *function = "alarm_data_toggle()";
   struct timeval tdiff;
   char src[MAXRULEADDRSTRING], dst[sizeof(src)], alarmduration[256];

   SASSERTX(monitor->mstats != NULL);
   SASSERTX(alarm->isconfigured);

   if (alarm->ison) {
      alarm->ison = 0;

      SASSERTX(timerisset(&alarm->alarmchange));

      timersub(tnow, &alarm->alarmchange, &tdiff);
      snprintf(alarmduration, sizeof(alarmduration),
               "Alarm duration: %lds", (long)tdiff.tv_sec);
   }
   else {
      alarm->ison    = 1;
      *alarmduration = NUL;
   }

   alarm->alarmchange = *tnow;

   timersub(tnow, &alarm->lastreset, &tdiff);

   slog(LOG_ALARM,
        "monitor(%lu): alarm/data %c: %s -> %s%s%s: %lu/%lu in %lds.  "
        "Session count: %lu%s%s",
        (unsigned long)monitor->number,
        alarm->ison ? '[' : ']',
        ruleaddr2string(&monitor->src, ADDRINFO_PORT, src, sizeof(src)),
        ruleaddr2string(&monitor->dst, ADDRINFO_PORT, dst, sizeof(dst)),
        alarmside == (size_t)-1 ? "" : " ",
        alarmside == (size_t)-1 ? "" : alarmside2string(alarmside),
        (unsigned long)alarm->bytes,
        (unsigned long)alarm->limit.bytes,
        (long)tdiff.tv_sec,
        (unsigned long)monitor->mstats->mstate.clients,
        *alarmduration == NUL ? "" : ".  ",
        alarmduration);
}

static struct timeval *
gettimeout(timeout)
   struct timeval *timeout;
{
   const char *function = "gettimeout()";
   const monitor_t *monitor;
   const struct timeval tzero = { 0, 0 };
   struct timeval tnow;

   int timeout_isset = 0;
   gettimeofday_monotonic(&tnow);

   for (monitor = sockscf.monitor; monitor != NULL; monitor = monitor->next) {
      struct timeval monitortimeout;

      if (monitor->mstats_shmid == 0)
         continue;

      SASSERTX(monitor->mstats != NULL);

      if (timetillcheck(monitor, &tnow, &monitortimeout, NULL) == NULL)
         continue;

      if (!timeout_isset || timercmp(&monitortimeout, timeout, <)) {
         *timeout      = monitortimeout;
         timeout_isset = 1;
      }
   }

   if (timeout_isset) {
      if (timercmp(timeout, &tzero, <))
         timerclear(timeout);

      return timeout;
   }
   else
      return NULL;
}

static struct timeval *
timetillcheck(monitor, tnow, tfirstcheck, hastimedout)
   const monitor_t *monitor;
   const struct timeval *tnow;
   struct timeval *tfirstcheck;
   size_t *hastimedout;
{
   const char *function = "timetillcheck()";
   const struct timeval tzero = { 0, 0 };
   struct timeval tnextcheck;
   monitor_stats_t *mstats;
   size_t hastimedout_mem;
   int tfirstcheck_isset;

   if (hastimedout == NULL)
      hastimedout = &hastimedout_mem;

   SASSERTX(monitor->mstats != NULL);
   mstats = &monitor->mstats->object.monitor;

#if DEBUG /* memory-mapped file contents may not be saved in coredumps. */
   monitor_stats_t _mstats = *mstats;
#endif /* DEBUG */

#define PRINT_DATALIMIT(monitornumber, interface, ioop, timeout)               \
do {                                                                           \
   slog(LOG_DEBUG,                                                             \
        "%s: monitor #%lu has a data-alarm configured on the %s interface "    \
        "for %s data.  Should be checked in %ld.%06lds %s%s%s",                \
        function,                                                              \
        (unsigned long)(monitornumber),                                        \
        (interface),                                                           \
        (ioop),                                                                \
        (long)(timeout)->tv_sec,                                               \
        (long)(timeout)->tv_usec,                                              \
        (long)(timeout)->tv_sec < 0 ? "("   : "",                              \
        (long)(timeout)->tv_sec < 0 ? "now" : "",                              \
        (long)(timeout)->tv_sec < 0 ? ")"   : "");                             \
} while (/* CONSTCOND */ 0)

#define PRINT_DISCONNECTLIMIT(monitornumber, interface, timeout)               \
do {                                                                           \
   slog(LOG_DEBUG,                                                             \
        "%s: monitor #%lu has a disconnect-alarm configured on the %s "        \
        "interface.  Should be checked in %ld.%06lds %s%s%s",                  \
        function,                                                              \
        (unsigned long)(monitornumber),                                        \
        (interface),                                                           \
        (long)(timeout)->tv_sec,                                               \
        (long)(timeout)->tv_usec,                                              \
        (long)(timeout)->tv_sec < 0 ? "("   : "",                              \
        (long)(timeout)->tv_sec < 0 ? "now" : "",                              \
        (long)(timeout)->tv_sec < 0 ? ")"   : "");                             \
} while (/* CONSTCOND */ 0)


#define TIMERCALC(object, tnextcheck, tfirstcheck, tnow)                       \
do {                                                                           \
   const struct timeval limit = { (object)->limit.seconds, 0 };                \
   struct timeval tshouldbechecked;                                            \
                                                                               \
   SASSERTX((object)->limit.seconds > 0);                                      \
   SASSERTX(timerisset(&limit));                                               \
                                                                               \
   timeradd(&((object)->lastreset), &limit, &tshouldbechecked);                \
   timersub(&tshouldbechecked, (tnow), (tnextcheck));                          \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: TIMERCALC: tshouldbechecked = %ld.%06ld, tnow = %ld.%06ld",       \
        function,                                                              \
        (long)tshouldbechecked.tv_sec,                                         \
        (long)tshouldbechecked.tv_usec,                                        \
        (long)(tnow)->tv_sec,                                                  \
        (long)(tnow)->tv_usec);                                                \
                                                                               \
   if (!tfirstcheck_isset || timercmp((tnextcheck), (tfirstcheck), <)) {       \
      *(tfirstcheck)    = *(tnextcheck);                                       \
      tfirstcheck_isset = 1;                                                   \
   }                                                                           \
} while (/* CONSTCOND */ 0)

   timerclear(tfirstcheck);
   tfirstcheck_isset = 0; /* used by TIMERCALC() macro. */
   *hastimedout      = 0;

   if (monitor->alarmsconfigured & ALARM_DATA) {
      if (mstats->internal.alarm.data.recv.isconfigured) {
         TIMERCALC(&mstats->internal.alarm.data.recv,
                   &tnextcheck,
                   tfirstcheck,
                   tnow);

         if (timercmp(&tzero, &tnextcheck, >))
            *hastimedout |= ALARM_INTERNAL_RECV;

         PRINT_DATALIMIT(monitor->number, "internal", "receiving", &tnextcheck);
      }

      if (mstats->internal.alarm.data.send.isconfigured) {
         TIMERCALC(&mstats->internal.alarm.data.send,
                   &tnextcheck,
                   tfirstcheck,
                   tnow);

         if (timercmp(&tzero, &tnextcheck, >))
            *hastimedout |= ALARM_INTERNAL_SEND;

         PRINT_DATALIMIT(monitor->number, "internal", "sending", &tnextcheck);
      }

      if (mstats->external.alarm.data.recv.isconfigured) {
         TIMERCALC(&mstats->external.alarm.data.recv,
                   &tnextcheck,
                   tfirstcheck,
                   tnow);

         if (timercmp(&tzero, &tnextcheck, >))
            *hastimedout |= ALARM_EXTERNAL_RECV;

         PRINT_DATALIMIT(monitor->number, "external", "receiving", &tnextcheck);
      }

      if (mstats->external.alarm.data.send.isconfigured) {
         TIMERCALC(&mstats->external.alarm.data.send,
                   &tnextcheck,
                   tfirstcheck,
                   tnow);

         if (timercmp(&tzero, &tnextcheck, >))
            *hastimedout |= ALARM_EXTERNAL_SEND;

         PRINT_DATALIMIT(monitor->number, "external", "sending", &tnextcheck);
      }
   }

   if (monitor->alarmsconfigured & ALARM_DISCONNECT) {
      if (mstats->internal.alarm.disconnect.isconfigured) {
         TIMERCALC(&mstats->internal.alarm.disconnect,
                   &tnextcheck,
                   tfirstcheck,
                   tnow);

         if (timercmp(&tzero, &tnextcheck, >))
            *hastimedout |= ALARM_INTERNAL;

         PRINT_DISCONNECTLIMIT(monitor->number, "internal", &tnextcheck);
      }

      if (mstats->external.alarm.disconnect.isconfigured) {
         TIMERCALC(&mstats->external.alarm.disconnect,
                   &tnextcheck,
                   tfirstcheck,
                   tnow);

         if (timercmp(&tzero, &tnextcheck, >))
            *hastimedout |= ALARM_EXTERNAL;

         PRINT_DISCONNECTLIMIT(monitor->number, "external", &tnextcheck);
      }
   }

   if (tfirstcheck_isset)
      slog(LOG_DEBUG, "%s: first check for monitor #%lu is in %ld.%06lds",
           function,
           (unsigned long)monitor->number,
           (long)tfirstcheck->tv_sec,
           (long)tfirstcheck->tv_usec);
   else {
      slog(LOG_DEBUG, "%s: no check scheduled for monitor #%lu",
           function, (unsigned long)monitor->number);

      tfirstcheck = NULL;
   }

   return tfirstcheck;
}

static void
siginfo(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "siginfo()";
   const int errno_s = errno;
   time_t tnow;
   unsigned long days, hours, minutes, seconds;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   seconds = (unsigned long)socks_difftime(time_monotonic(&tnow),
                                           sockscf.stat.boot);

   seconds2days(&seconds, &days, &hours, &minutes);

   slog(LOG_INFO, "%s up %lu day%s, %lu:%.2lu:%.2lu",
        childtype2string(sockscf.state.type),
        days, days == 1 ? "" : "s",
        hours,
        minutes,
        seconds);

   SIGNAL_EPILOGUE(sig, si, errno_s);
}

static void
proctitleupdate(void)
{

#if 1
   setproctitle("%s", childtype2string(sockscf.state.type));
#else /* XXX fix. */
   setproctitle("%s: %lu alarm%s active",
                childtype2string(sockscf.state.type),
                (unsigned long)state.alarmsactive,
                state.alarmsactive == 1 ? "" : "s");
#endif

}

static char *
op2string(op)
   const size_t op;
{

   switch (op) {
      case OP_CONNECT:
         return "connect";

      case OP_DISCONNECT:
         return "disconnect";

      case OP_REMOVE_SESSION:
         return "session-remove";
   }

   SERRX(op);
}

static void
showalarms(iface)
   const monitor_if_t *iface;
{

#define DOPRINT_DATA(attr, op)                                                 \
do {                                                                           \
   slog(LOG_DEBUG,                                                             \
        "alarm if %s less or equal to %lu byte%s every %ld second%s",          \
        (op),                                                                  \
        (unsigned long)iface->alarm.data.attr.limit.bytes,                     \
        iface->alarm.data.attr.limit.bytes == 1 ? "" : "s",                    \
        (long)iface->alarm.data.attr.limit.seconds,                            \
        iface->alarm.data.attr.limit.seconds == 1 ? "" : "s");                 \
} while (/* CONSTCOND */ 0)


   if (iface->alarm.data.recv.isconfigured)
      DOPRINT_DATA(recv, "receiving");

   if (iface->alarm.data.send.isconfigured)
      DOPRINT_DATA(send, "sending");

   if (iface->alarm.disconnect.isconfigured)
      slog(LOG_DEBUG,
           "alarm if more than %lu/%lu disconnects occur during %ld second%s",
           (unsigned long)iface->alarm.disconnect.limit.disconnectc,
           (unsigned long)iface->alarm.disconnect.limit.sessionc,
           (long)iface->alarm.disconnect.limit.seconds,
           iface->alarm.disconnect.limit.seconds == 1 ? "" : "s");
}
