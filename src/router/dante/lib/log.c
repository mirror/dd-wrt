/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014, 2024
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

static const char rcsid[] =
"$Id: log.c,v 1.373.4.8.6.1.4.4 2024/11/21 12:27:27 michaels Exp $";

#include "common.h"
#include "config_parse.h"

#if HAVE_EXECINFO_H && HAVE_BACKTRACE
#include <execinfo.h>
#endif /* HAVE_EXECINFO_H && HAVE_BACKTRACE */

#if DEBUG && PRERELEASE
#undef SOCKS_IGNORE_SIGNALSAFETY
#define SOCKS_IGNORE_SIGNALSAFETY 1
#endif /* DEBUG && PRERELEASE*/

#if SOCKS_CLIENT
#define SRCDSTLEN     (1024)
#else /* !SOCKS_CLIENT */
#define SRCDSTLEN     (MAX_IOLOGADDR + sizeof(" -> ") + MAX_IOLOGADDR)
#endif /* !SOCKS_CLIENT */

#define DATALEN       (2048)  /* payload data or message */
#define REGULARBUFLEN (  SRCDSTLEN                                             \
                       + (DATALEN * 4) /* x 4 for strivs(3) */                 \
                       + 1024          /* whatever. */)

struct syslogfacility {
   const char name[MAXFACILITYNAMELEN];
   const int  value;
};


/*
 * if possible (enough space) and necessary, add a newline to the
 * NUL-terminated buf (of size buflen, of which buflen has been used so far),
 * and increment bufused if so.
 */
#define ADDNL(bufused, buf, buflen)                                            \
do {                                                                           \
   SASSERTX((buf)[(*bufused) - 1] == NUL);                                     \
   if ((buf)[(*bufused) - 2] != '\n') {                                        \
      if ((*bufused) + 1 <= (buflen)) {                                        \
         buf[(*bufused) - 1] = '\n';                                           \
         buf[(*bufused)++]   = NUL;                                            \
      }                                                                        \
      else                                                                     \
         buf[(*bufused) - 2] = '\n'; /* truncated.  */                         \
   }                                                                           \
   SASSERTX((buf)[(*bufused) - 1] == NUL);                                     \
} while (0 /* CONSTCOND */)

static const struct syslogfacility *
syslogfacility(const char *name);
/*
 * Returns the syslog facility value having the name "name".
 * Returns NULL if there is no such facility.
 */


static void
dolog(const int priority, const char *buf,
      const size_t logprefixlen, const size_t messagelen);
/*
 * Does the actual logging of the formated logmessage for slog()/vslog().
 *
 * The last character in "buf", before the NUL, must be a newline.
 *
 * "prefixlen" is the length of the logprefix at the start of "buf", and
 * is not used if logging to syslog.
 *
 * "messagelen" gives the length of the message that follows after the
 * logprefix.
 */

static int
openlogfile(const char *logfile, int *wecreated);
/*
 * Calls open(2) with the correct flags for a logfile named "logfile",
 * as well as sets any fd flags we want.
 *
 * If "created" is true upon return, it means the openlog() created
 * the logfile.  Otherwise, no logfile was created, either because it
 * already existed, or because the function failed.
 *
 * Returns the fd associated with the open logfile.
 */

static size_t
getlogprefix(const int priority, char *buf, size_t buflen);
/*
 * Writes a logprefix, similar to that used by syslog, which should be used
 * if logging a message with level "priority" at the current time.
 * Should not be used if logging via syslog.
 *
 * The prefix is written to "buf", which should be of at least size "buflen".
 * Returns the length of the prefix, *NOT* including the terminating NUL.
 */


#if HAVE_LIVEDEBUG

/*
 * Apart from messages where payload data is included we never expect
 * the message to log being very large.  Since we don't normally care
 * much about the payload data, it's better to lose some/most of it and
 * have room for more relevant data instead.
 */
#define SOCKS_RINGBUF_MAXLINELEN (1024)

#ifndef SOCKS_RINGBUFLEN
#define SOCKS_RINGBUFLEN   (SOCKS_RINGBUF_MAXLINELEN * 100)
#endif /* SOCKS_RINGBUFLEN */


static int dont_add_to_rb;
static void
socks_addtorb(const char *str, const size_t strlen);
/*
 * Adds the NUL-terminated string "str", of length "strlen" (including NUL)
 * to the ringbuffer.
 */

#if SOCKS_CLIENT
static void atexit_flushrb(void);
#endif /* SOCKS_CLIENT */

static char ringbuffer[SOCKS_RINGBUFLEN];
static size_t ringbuf_curroff;
#endif /* HAVE_LIVEDEBUG */

#if !SOCKS_CLIENT

#define DO_BUILD(srcdst_str, dst_too)                                          \
do {                                                                           \
   char srcstr[MAX_IOLOGADDR];                                                 \
                                                                               \
   build_addrstr_src(HAVE_SOCKS_HOSTID ? &state->hostid : NULL,                \
                     src != NULL && src->peer_isset ?                          \
                        &src->peer : NULL,                                     \
                     tosrc_proxy != NULL && tosrc_proxy->peer_isset ?          \
                        &tosrc_proxy->peer : NULL,                             \
                     tosrc_proxy != NULL && tosrc_proxy->local_isset ?         \
                        &tosrc_proxy->local : NULL,                            \
                     src != NULL && src->local_isset ?                         \
                        &src->local : NULL,                                    \
                     src != NULL && src->auth_isset  ?                         \
                        &src->auth : NULL,                                     \
                     tosrc_proxy != NULL && tosrc_proxy->auth_isset ?          \
                        &tosrc_proxy->auth  : NULL,                            \
                     (dst_too) ? srcstr         : srcdst_str,                  \
                     (dst_too) ? sizeof(srcstr) : sizeof(srcdst_str));         \
                                                                               \
   if (dst_too) {                                                              \
      char dststr[MAX_IOLOGADDR];                                              \
                                                                               \
      build_addrstr_dst(dst != NULL && dst->local_isset ?                      \
                           &dst->local : NULL,                                 \
                        todst_proxy != NULL && todst_proxy->local_isset ?      \
                           &todst_proxy->local : NULL,                         \
                        todst_proxy != NULL && todst_proxy->peer_isset  ?      \
                           &todst_proxy->peer  : NULL,                         \
                        dst != NULL && dst->peer_isset ?                       \
                           &dst->peer : NULL,                                  \
                        dst != NULL && dst->auth_isset ?                       \
                           &dst->auth : NULL,                                  \
                        todst_proxy != NULL && todst_proxy->auth_isset ?       \
                           &todst_proxy->auth  : NULL,                         \
                        HAVE_SOCKS_HOSTID ? &state->hostid : NULL,             \
                        dststr,                                                \
                        sizeof(dststr));                                       \
                                                                               \
      snprintf(srcdst_str, sizeof(srcdst_str), "%s -> %s", srcstr, dststr);    \
   }                                                                           \
} while (/* CONSTCOND */ 0)


iologaddr_t *
init_iologaddr(addr, local_type, local, peer_type, peer, auth, hostid)
   iologaddr_t *addr;
   const objecttype_t local_type;
   const void *local;
   const objecttype_t peer_type;
   const void *peer;
   const authmethod_t *auth;
   const struct hostid *hostid;
{

   if (local == NULL || local_type == object_none)
      addr->local_isset = 0;
   else {
      switch (local_type) {
         case object_sockaddr:
            sockaddr2sockshost(local, &addr->local);
            break;

         case object_sockshost:
            addr->local = *(const sockshost_t *)local;
            break;

         default:
            SERRX(local_type);
      }

      addr->local_isset = 1;
   }

   if (peer == NULL || peer_type == object_none)
      addr->peer_isset  = 0;
   else {
      switch (peer_type) {
         case object_sockaddr:
            sockaddr2sockshost(peer, &addr->peer);
            break;

         case object_sockshost:
            addr->peer = *(const sockshost_t *)peer;
            break;

         default:
            SERRX(peer_type);
      }

      addr->peer_isset = 1;
   }

   if (auth == NULL)
      addr->auth_isset = 0;
   else {
      addr->auth       = *auth;
      addr->auth_isset = 1;
   }

#if HAVE_SOCKS_HOSTID

   if (hostid == NULL || hostid->addrc == 0)
      addr->hostidc = 0;
   else {
      SASSERTX(ELEMENTS(addr->hostidv) >= hostid->addrc);

      addr->hostidc = gethostidipv(hostid, 
                                   addr->hostidv, 
                                   ELEMENTS(addr->hostidv));
   }

#endif /* HAVE_SOCKS_HOSTID */

   return addr;
}

void
iolog(rule, state, op, src, dst, tosrc_proxy, todst_proxy, data, datalen)
   const rule_t *rule;
   const connectionstate_t *state;
   const operation_t op;
   const iologaddr_t *src;
   const iologaddr_t *dst;
   const iologaddr_t *tosrc_proxy;
   const iologaddr_t *todst_proxy;
   const char *data;
   size_t datalen;
{
   const char *function      = "iolog()";
   const char *tcpinfoprefix = "\nTCP_INFO:\n";
   const int dologtcpinfo    = (rule->log.tcpinfo && state->tcpinfo != NULL) ?
                               1 : 0;
   const int dologdstinfo    = (dst == NULL) ? 0 : 1;
   const char *verdict;
   size_t buflen;
   char srcdst_str[SRCDSTLEN], rulecommand[256],
        *buf, *bigbuf, regbuf[REGULARBUFLEN];

   if (rule->log.data && datalen > DATALEN) {
      const size_t bigbufsize =   (size_t)datalen * 4 /* x 4 for strvis(3) */
                                + sizeof(regbuf);

      slog(LOG_DEBUG,
           "%s: a datalen of %ld is too large to fit in the stack-allocated "
           "buffer.  Allocating %lu bytes of memory dynamically",
           function, (unsigned long)datalen, (unsigned long)bigbufsize);

      if ((bigbuf = malloc(bigbufsize)) != NULL) {
         buf    = bigbuf;
         buflen = bigbufsize;
      }
      else {
         swarn("%s: failed to allocate %lu bytes of memory",
               function, (unsigned long)bigbufsize);

         buf    = regbuf;
         buflen = sizeof(regbuf);
      }
   }
   else {
      buflen = sizeof(regbuf);
      buf    = regbuf;
      bigbuf = NULL;
   }

/*
 * If the datastring we are passed starts with a newline, don't include an
 * extra ':' to separate it from the addressinfo.  We do not need to worry
 * about the payload data starting with a newline, as we strvis(3) it before
 * passing it here, so it should not start with a newline when we get it.
 */
#define DATASEPARATOR(data)                                                    \
(((data) == NULL || *(data) == NUL || *(data) == '\n') ? "" : ": ")

#define DATASTRING(data)                                                       \
(((data) == NULL || *(data) == NUL) ? "" : data)

   verdict = NULL;
   switch (op) {
      case OPERATION_ACCEPT:
      case OPERATION_HOSTID:
      case OPERATION_CONNECT:
         if (rule->log.connect) {
            DO_BUILD(srcdst_str, dologdstinfo);
            snprintf(buf, buflen,
                    "[: %s%s%s",
                     srcdst_str,
                     DATASEPARATOR(data),
                     DATASTRING(data));
         }
         else {
            free(bigbuf);
            return;
         }

         break;

      case OPERATION_BLOCK:
      case OPERATION_TMPBLOCK:
         if (rule->log.connect || rule->log.disconnect
         ||  rule->log.data    || rule->log.iooperation) {
            DO_BUILD(srcdst_str, dologdstinfo);
            snprintf(buf, buflen,
                     "%c: %s%s%s",
                     op == OPERATION_BLOCK ? ']' : '-',
                     srcdst_str,
                     DATASEPARATOR(data),
                     DATASTRING(data));
         }
         else {
            free(bigbuf);
            return;
         }

         verdict = verdict2string(VERDICT_BLOCK);
         break;

      case OPERATION_DISCONNECT:
         if (rule->log.disconnect) {
            DO_BUILD(srcdst_str, dologdstinfo);
            snprintf(buf, buflen,
                     "]: %s%s%s",
                     srcdst_str,
                     DATASEPARATOR(data),
                     DATASTRING(data));
         }
         else {
            free(bigbuf);
            return;
         }

         break;

      case OPERATION_ERROR:
      case OPERATION_TMPERROR:
         if (rule->log.error || rule->log.disconnect) {
            DO_BUILD(srcdst_str, dologdstinfo);
            snprintf(buf, buflen,
                     "%c: %s%s%s",
                     op == OPERATION_ERROR ? ']' : '-',
                     srcdst_str,
                     DATASEPARATOR((data == NULL || *data == NUL) ?
                                          strerror(errno) : data),
                     (data == NULL || *data == NUL) ? strerror(errno) : data);
         }
         else {
            free(bigbuf);
            return;
         }

         verdict = verdict2string(VERDICT_BLOCK);
         break;

      case OPERATION_IO:
         if (rule->log.data || rule->log.iooperation) {
            if (rule->log.data && datalen != 0) {
               size_t lastbyteused;

               DO_BUILD(srcdst_str, dologdstinfo);
               lastbyteused = snprintf(buf, buflen,
                                       "-: %s (%lu): ",
                                       srcdst_str, (unsigned long)datalen);
               str2vis(data,
                       datalen,
                       &buf[lastbyteused],
                       buflen - lastbyteused);
            }
            else {
               DO_BUILD(srcdst_str, dologdstinfo);
               snprintf(buf, buflen,
                        "-: %s (%lu)", srcdst_str, (unsigned long)datalen);
            }
         }
         else {
            free(bigbuf);
            return;
         }

         break;

      default:
         SERRX(op);
   }

   snprintf(rulecommand, sizeof(rulecommand), "%s(%lu): %s/%s",
            verdict == NULL ? verdict2string(rule->verdict) : verdict,
            (unsigned long)rule->number,
            protocol2string(state->protocol),
            command2string(state->command));

   slog(LOG_INFO, "%s %s%s%s",
        rulecommand,
        buf,
        dologtcpinfo ? tcpinfoprefix  : "",
        dologtcpinfo ? state->tcpinfo : "");

   free(bigbuf);
}

void
sockd_freelogobject(logobject, closetoo)
   logtype_t *logobject;
   const int closetoo;
{
   const char *function = "sockd_freelogobject()";
   sigset_t all, oldmask;
   size_t i;

   (void)sigfillset(&all);
   if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK)", function);

  for (i = 0; i < logobject->filenoc; ++i) {
      if (closetoo && !FD_IS_RESERVED_EXTERNAL(logobject->filenov[i]))
         close(logobject->filenov[i]);

      free(logobject->fnamev[i]);
   }

   free(logobject->fnamev);
   free(logobject->filenov);
   free(logobject->createdv);

   bzero(logobject, sizeof(*logobject));

   if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);
}

int
loglevel_errno(e, side)
   const int e;
   const interfaceside_t side;
{
   const logspecial_t *log;
   static interfaceside_t last_side;
   static int last_loglevel = -1, last_errno;
   size_t i, ii;

   CTASSERT(LOG_EMERG == 0);
   CTASSERT(LOG_DEBUG == 7);

   if (last_loglevel != -1 && last_errno == e && last_side == side)
      return last_loglevel;

   if (side == INTERNALIF)
      log = &sockscf.internal.log;
   else {
      SASSERTX(side == EXTERNALIF);
      log = &sockscf.external.log;
   }

   /*
    * Go through all loglevels and see if we find a match for 'e'.
    * Presumably there will not be too many errnovalues set, so
    * this should be fast enough.  The other obvious alternative,
    * index by errnovalue, would unfortunately depend on us never
    * running on a system with large errnovalues, which we have no
    * control over.  The loglevels on the other side are defined by
    * rfc.
    */
   for (i = 0; i < ELEMENTS(log->errno_loglevelv); ++i)
      for (ii = 0; ii < log->errno_loglevelc[i]; ++ii)
         if (log->errno_loglevelv[i][ii] == e) {
            last_side     = side;
            last_errno    = errno;
            last_loglevel = i;

            return i;
         }

   return LOG_DEBUG;
}

int
loglevel_gaierr(e, side)
   const int e;
   const interfaceside_t side;
{
   const char *function = "loglevel_gaierr()";
   static interfaceside_t last_side;
   static int last_loglevel = -1, last_gaierr;
   const logspecial_t *log;
   size_t i, ii;

   CTASSERT(LOG_EMERG == 0);
   CTASSERT(LOG_DEBUG == 7);

   slog(LOG_DEBUG, "%s: error %d, side %s",
        function, e, side == EXTERNALIF ? "external" : "internal");

   if (last_loglevel != -1 && last_gaierr == e && last_side == side)
      return last_loglevel;

   if (side == INTERNALIF)
      log = &sockscf.internal.log;
   else {
      SASSERTX(side == EXTERNALIF);
      log = &sockscf.external.log;
   }

   for (i = 0; i < ELEMENTS(log->gaierr_loglevelv); ++i) {
      for (ii = 0; ii < log->gaierr_loglevelc[i]; ++ii)
         if (log->gaierr_loglevelv[i][ii] == e) {
            last_loglevel = i;
            last_gaierr   = e;
            last_side     = side;
            return i;
         }
   }

   return LOG_DEBUG;
}

#endif /* !SOCKS_CLIENT */

void
newprocinit(void)
{

#if SOCKS_CLIENT
   return;
#else /* !SOCKS_CLIENT */
   const char *function = "newprocinit()";

   if ((sockscf.log.type    & LOGTYPE_SYSLOG)
   ||  (sockscf.errlog.type & LOGTYPE_SYSLOG)
   ||  HAVE_LIBWRAP /* libwrap may also log to syslog. */) {
      closelog();

      /*
       * LOG_NDELAY so we don't end up in a situation where we
       * have no free descriptors and haven't yet syslog-ed anything.
       */
      openlog(__progname,
              LOG_NDELAY | LOG_PID
#ifdef LOG_NOWAIT
              | LOG_NOWAIT
#endif /* LOG_NOWAIT */
              , 0);

      errno = 0; /* syslog() stuff might set errno on some platforms. */
   }

   /*
    * not using this for client, since if e.g. the client forks, we'd
    * end up printing the wrong pid.
    */
   sockscf.state.pid = getpid();

   srandom((unsigned int)sockscf.state.pid);

   /* don't want children to inherit mother's signal queue. */
   sockscf.state.signalc = 0;
#endif /* !SOCKS_CLIENT */
}

int
socks_addlogfile(logcf, logfile)
   logtype_t *logcf;
   const char *logfile;
{
   const char *function = "socks_addlogfile()";
   const char *syslogname = "syslog";
   void *p1, *p2, *p3;
   sigset_t all, oldmask;
   char *fname;
   int fd, logfilewascreated;

   if (strncmp(logfile, syslogname, strlen(syslogname)) == 0
   && ( logfile[strlen(syslogname)] == NUL
     || logfile[strlen(syslogname)] == '/')) {
      const char *sl;

      logcf->type |= LOGTYPE_SYSLOG;

      if (*(sl = &(logfile[strlen(syslogname)])) == '/') { /* facility. */
         const struct syslogfacility *facility;

         ++sl;
         if ((facility = syslogfacility(sl)) == NULL) {
            yywarnx("unknown syslog facility \"%s\"", sl);
            return -1;
         }

         logcf->facility = facility->value;
         STRCPY_ASSERTSIZE(logcf->facilityname, facility->name);
      }
      else {   /* use default. */
         logcf->facility = LOG_DAEMON;
         STRCPY_ASSERTSIZE(logcf->facilityname, "daemon");
      }

      if (!sockscf.state.inited)
         newprocinit(); /* to setup syslog correctly asap. */

      return 0;
   }

   /* else: filename. */
   logcf->type |= LOGTYPE_FILE;
   if ((fd = openlogfile(logfile, &logfilewascreated)) == -1)
      return -1;

   /*
    * Don't want to receive signals, which could lead to us accessing the
    * logobject, at this point.
    */

   (void)sigfillset(&all);
   if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK)", function);

   if ((fname = strdup(logfile)) == NULL) {
      yywarn("%s: could not allocate %lu bytes of memory for logfile \"%s\"",
             function, (unsigned long)strlen(logfile), logfile);

      if (fd != fileno(stdout) && fd != fileno(stderr))
         close(fd);

      if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
         swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

      return -1;
   }

   p1 = p2 = p3 = NULL;

   p1 = realloc(logcf->filenov, sizeof(*logcf->filenov) * (logcf->filenoc + 1));
   p2 = realloc(logcf->fnamev,  sizeof(*logcf->fnamev)  * (logcf->filenoc + 1));
   p3 = realloc(logcf->createdv,
                               sizeof(*logcf->createdv) * (logcf->filenoc + 1));


   if (p1 != NULL)
      logcf->filenov  = p1;

   if (p2 != NULL)
      logcf->fnamev   = p2;

   if (p3 != NULL)
      logcf->createdv = p3;


   if (p1 == NULL || p2 == NULL || p3 == NULL) {
      yywarn("%s: failed to allocate memory for log filenames", function);

      free(fname);

      /*
       * Don't bother free(3)'ing what we actually managed to realloc(3).
       * Only means some of the elements in logcf now have a little more
       * memory available than strictly needed.
       */

      if (fd != fileno(stdout) && fd != fileno(stderr))
         close(fd);

      if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
         swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

      return -1;
   }

   logcf->filenov[logcf->filenoc]  = fd;
   logcf->fnamev[logcf->filenoc]   = fname;
   logcf->createdv[logcf->filenoc] = logfilewascreated;
   ++logcf->filenoc;

   if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

   return 0;
}

#if !SOCKS_CLIENT

int
sockd_reopenlogfiles(log, docloseold)
   logtype_t *log;
   const int docloseold;
{
   const char *function = "sockd_reopenlogfiles()";
   sigset_t all, oldmask;
   size_t i;
   int p;

   (void)sigfillset(&all);
   if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK)", function);

   for (i = 0; i < log->filenoc; ++i) {
      if (docloseold) {
         if (close(log->filenov[i]) != 0)
            swarn("%s: could not close logfile \"%s\" using fd %d",
                  function, log->fnamev[i], log->filenov[i]);
      }

      /*
       * When reopening we don't care if we created the logfile or not,
       * as it is only used for the first startup, when we may create
       * logfiles before we've inited our userids properly, in case we
       * may need to change the logfile owner after userids are inited.
       */
      if ((log->filenov[i] = openlogfile(log->fnamev[i], &p)) == -1) {
         swarn("%s: could not reopen logfile \"%s\"", function, log->fnamev[i]);

         if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
            swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

         return -1;
      }
   }

   if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

   return 0;
}
#endif /* !SOCKS_CLIENT */

void
slog(int priority, const char *message, ...)
{
   va_list ap, apcopy;

   /*
    * not all systems may have va_copy().  Idea from a news post by
    * Chris Torek.
    */
   va_start(ap, message);
   va_start(apcopy, message);

   vslog(priority, message, ap, apcopy);

   va_end(apcopy);
   va_end(ap);
}

void
vslog(priority, message, ap, apcopy)
   int priority;
   const char *message;
   va_list ap;
   va_list apcopy;
{
   const char *function = "vslog()";
   const int errno_s = errno;
   ssize_t p;
   size_t datalen /* no NUL */, prefixlen /* no NUL */, buflen,
          loglen, oldloglen;
   char *buf, *bigbuf, regbuf[REGULARBUFLEN];

   buf    = regbuf;
   buflen = sizeof(regbuf);
   bigbuf = NULL;

#if !SOCKS_IGNORE_SIGNALSAFETY
   if (sockscf.state.insignal
   /* && priority > LOG_WARNING */) /* > pri means < serious */
      /*
       * Note that this can be the case even if insignal is not set.
       * This can happen in the client if the application has
       * installed a signal handler, and that signal handler ends
       * up making calls that involve us.
       */
      return;
#endif /* !SOCKS_IGNORE_SIGNALSAFETY */

   if (priority == LOG_DEBUG && !sockscf.option.debug) {
#if HAVE_LIVEDEBUG
      const int insignal = sockscf.state.insignal;

#if SOCKS_CLIENT
      static int atexit_registered;

      if (!atexit_registered) {
         if (atexit(atexit_flushrb) != 0)
            swarn("%s: atexit() failed to register atexit function", function);
         else
            atexit_registered = 1;
      }
#endif /* SOCKS_CLIENT */

      if (!dont_add_to_rb) {
         /*
          * don't have getlogprefix() bother with calling localtime(3);  too
          * slow.
          */
          ssize_t maxtoprint;

         sockscf.state.insignal = 1;
         prefixlen = getlogprefix(priority, buf, buflen);
         sockscf.state.insignal = insignal;

         maxtoprint = MIN(buflen - prefixlen, SOCKS_RINGBUF_MAXLINELEN);
         SASSERTX(maxtoprint >= 0);

         p = vsnprintf(&buf[prefixlen],
                       (size_t)maxtoprint,
                       message,
                       ap);

         if (p <= 0) { /* well, that's strange. */
            errno = errno_s;
            return;
         }

         if (p >= maxtoprint) {
            buf[(prefixlen + maxtoprint) - 1] = NUL;
            p = maxtoprint - 1;
         }

         datalen = (size_t)p;

         SASSERTX(buf[prefixlen + datalen] == NUL);

         if (prefixlen + datalen < buflen)
            loglen = prefixlen + datalen + 1 /* + 1: NUL terminator */;
         else /* truncated, but never mind when debug-logging to ringbuffer. */
            loglen = buflen;

#if DIAGNOSTIC
         SASSERTX(loglen == strlen(buf) + 1);
#endif /* DIAGNOSTIC */

         SASSERTX(loglen >= 2);
         SASSERTX(buf[loglen - 1] == NUL);

         ADDNL(&loglen, buf, buflen);

         SASSERTX(buf[loglen - 1] == NUL);
         socks_addtorb(buf, loglen);
      }
#endif /* HAVE_LIVEDEBUG */

      errno = errno_s;
      return;
   }

   prefixlen = getlogprefix(priority, buf, buflen);
   SASSERTX(prefixlen < buflen);

   p = vsnprintf(&buf[prefixlen], buflen -prefixlen, message, ap);
   if (p <= 0) { /* well, that's strange. */
      errno = errno_s;
      return;
   }

   datalen = p;

   if (prefixlen + datalen >= buflen && !sockscf.state.insignal) {
      /*
       * not enough room in our regular buffer (and not in signal,
       * so can malloc(3)).
       */
      const size_t toalloc = datalen + prefixlen + sizeof("\n");

      if ((bigbuf = malloc(toalloc)) != NULL) {
         memcpy(bigbuf, buf, prefixlen);
         buf    = bigbuf;
         buflen = toalloc;

         p = vsnprintf(&buf[prefixlen], buflen - prefixlen, message, apcopy);
         if (p <= 0) { /* well, that's strange. */
            free(bigbuf);
            errno = errno_s;

            return;
         }

         datalen = p;
      }
   }

   /* check again, after possible malloc(3)/vsnprintf(3). */
   if (prefixlen + datalen >= buflen) {
      /* still not enough room.  Must truncate. */
      datalen         = buflen - prefixlen - 1;
      buf[buflen - 1] = NUL;
   }

   loglen = prefixlen + datalen + 1 /* NUL */;
   SASSERTX(loglen <= buflen);
   SASSERTX(buf[loglen - 1] == NUL);

#if DIAGNOSTIC
   SASSERTX(loglen == strlen(buf) + 1);
#endif /* DIAGNOSTIC */

   oldloglen = loglen;
   ADDNL(&loglen, buf, buflen);
   if (loglen != oldloglen) {
      SASSERTX(loglen = oldloglen + 1); /* newline added. */
      ++datalen;
   }

   SASSERTX(loglen <= buflen);
   SASSERTX(buf[loglen - 1] == NUL);

#if DIAGNOSTIC
   SASSERTX(loglen == strlen(buf) + 1);
#endif /* DIAGNOSTIC */

   dolog(priority, buf, prefixlen, datalen);

   free(bigbuf);
   errno = errno_s;
}

void
signalslog(priority, msgv)
   const int priority;
   const char *msgv[];
{
   const char *function = "signalslog()";
   const int errno_s = errno;
   size_t bufused, msglen, prefixlen, i;
#if HAVE_LIVEDEBUG
   char buf[SOCKS_RINGBUFLEN];

#else /* !HAVE_LIVEDEBUG */
   char buf[REGULARBUFLEN];

#endif /* !HAVE_LIVEDEBUG */

   prefixlen = bufused = getlogprefix(priority, buf, sizeof(buf));

   if (msgv == NULL)
      return;

   for (i = 0; msgv[i] != NULL; ++i) {
      msglen = MIN(strlen(msgv[i]), sizeof(buf) - bufused - 1);
      memcpy(&buf[bufused], msgv[i], msglen);
      bufused += msglen;
   }

   SASSERTX(bufused < sizeof(buf));
   buf[bufused++] = NUL;

   ADDNL(&bufused, buf, sizeof(buf));

   SASSERTX(buf[bufused - 1] == NUL);
   SASSERTX(buf[bufused - 2] == '\n');

   msglen = bufused - 1 /* don't count NUL */ - prefixlen;

   if (priority == LOG_DEBUG && !sockscf.option.debug) {
#if HAVE_LIVEDEBUG
      if (!dont_add_to_rb)
         socks_addtorb(buf, bufused);
#endif /* HAVE_LIVEDEBUG */

      errno = errno_s;
      return;
   }

   dolog(priority, buf, prefixlen, msglen);

#if HAVE_LIVEDEBUG /* always save to ring buffer too. */
   if (!dont_add_to_rb)
      socks_addtorb(buf, bufused);
#endif /* HAVE_LIVEDEBUG */

   errno = errno_s;
}


static void
dolog(priority, buf, prefixlen, messagelen)
   const int priority;
   const char *buf;
   const size_t prefixlen;
   const size_t messagelen;
{
   int needlock = 0, logged = 0;

   /*
    * syslog first ...
    */
   if ((sockscf.errlog.type & LOGTYPE_SYSLOG)
   ||  (sockscf.log.type    & LOGTYPE_SYSLOG)) {
      if (priority <= LOG_WARNING) { /* lower pri value means more serious */
         if (sockscf.errlog.type & LOGTYPE_SYSLOG) {
            /*
             * Unfortunately it's not safe to call syslog(3) from a signal
             * handler.  Do make an exception for the most serious warnings
             * however.
             */
            if (!sockscf.state.insignal || priority <= LOG_CRIT) {
               syslog(priority | sockscf.errlog.facility,
                      "%s: %s", loglevel2string(priority), &buf[prefixlen]);

               logged = 1;
            }
         }
      }

      if (sockscf.log.type & LOGTYPE_SYSLOG) {
         if (!sockscf.state.insignal || priority <= LOG_CRIT) {
            syslog(priority | sockscf.log.facility,
                   "%s: %s", loglevel2string(priority), &buf[prefixlen]);

            logged = 1;
         }
      }

#if SOCKS_CLIENT
      /*
       * We don't have control over what sockets the application is using, 
       * and some applications may try to close all open fds, including the 
       * socket used by syslog.  If that happens, the syslog(3) call will 
       * probably fail, which is not critical.  A critical problem is however
       * that some libc-libraries (e.g., glibc-2.26 and up) will in that 
       * case close(2) the syslog-socket they previously opened, but
       * that socket may now be in use by the application.  The latter problem
       * will make things go bad as the application will for some unknown
       * reason suddenly lose it's fd, or even worse, the fd may now be
       * connected to syslog after the libc-library decides to recreate the
       * syslog-socket.
       *
       * To avoid that problem we call closelog(3) after every syslog(3) 
       * call, to make the syslog library code close any fd in use for 
       * syslog(3)-ing.  Next time we then call syslog(3) (including the 
       * first time we call syslog(3)), the syslog code should create a new 
       * syslog-socket, syslog what we want, and then we call closelog(3)
       * again.  The effect should be that the syslog-socket only exists 
       * during the time we need to log something, which should avoid any
       * conflicts with the application code. 
       *
       * It is obviously not very optimal creating a new socket every time 
       * we syslog(3), but since this is only a problem for the client (in  
       * the Dante server, we have better control over sockets in use), the 
       * overhead is deemed acceptable.
       */
      
      closelog();

#endif /* SOCKS_CLIENT */
   }

   /*
    * ... and then logging to file.
    */


#if 0
   if ((sockscf.log.type & LOGTYPE_FILE)
   ||  (priority <= LOG_WARNING && (sockscf.errlog.type & LOGTYPE_FILE))
   ||  HAVE_LIVEDEBUG) {
      if (sockscf.loglock != -1) {
         needlock = 1;
         socks_lock(sockscf.loglock, 0, 0, 1, 1);
      }
   }
#else /* the file is opened with O_APPEND - no locking should be required. */
   needlock = 0;
#endif

   /* error-related logging first ...  */
   if (priority <= LOG_WARNING) { /* lower pri value means more serious */
      if (sockscf.errlog.type & LOGTYPE_FILE) {
         size_t i;

         for (i = 0; i < sockscf.errlog.filenoc; ++i) {
            while (write(sockscf.errlog.filenov[i], buf, prefixlen + messagelen)
            == -1 && errno == EINTR)
               ;

            logged = 1;
         }
      }
   }

   /* ... and then normal logging. */
   if (sockscf.log.type & LOGTYPE_FILE) {
      size_t i;

      for (i = 0; i < sockscf.log.filenoc; ++i) {
         size_t retries = 0;

         while (write(sockscf.log.filenov[i], buf, prefixlen + messagelen) == -1
         && errno     == EINTR
         && retries++ <  10)
            ;

         logged = 1;
      }
   }

   if (needlock)
      socks_unlock(sockscf.loglock, 0, 0);

   /*
    * If we need to log something serious but have not inited logfiles
    * yet, take the risk of logging it to stderr.
    */
   if (!logged) {
      if (!sockscf.state.inited && priority <= LOG_WARNING) {
#if SOCKS_CLIENT

         if (isatty(fileno(stderr))) /* don't take the risk otherwise. */
            (void)write(fileno(stderr), buf, prefixlen + messagelen);

#else /* server */

         (void)write(fileno(stderr), buf, prefixlen + messagelen);

#endif /* server */
      }
   }
}

static int
openlogfile(logfile, wecreated)
   const char *logfile;
   int *wecreated;
{
   const char *function = "openlogfile()";
   int p, flagstoadd, fd;

   *wecreated = 0;

   if (strcmp(logfile, "stdout") == 0) {
      fd         = fileno(stdout);
      flagstoadd = 0;
   }
   else if (strcmp(logfile, "stderr") == 0) {
      fd         = fileno(stderr);
      flagstoadd = 0;
   }
   else {
      const mode_t openmode  = S_IRUSR  | S_IWUSR  | S_IRGRP;
      const int    openflags = O_WRONLY | O_APPEND;

#if !SOCKS_CLIENT
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
#endif /* !SOCKS_CLIENT */

      if ((fd = open(logfile, openflags, openmode)) == -1)
         if ((fd = open(logfile, openflags | O_CREAT, openmode)) != -1)
            *wecreated = 1;

#if !SOCKS_CLIENT
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);
#endif /* !SOCKS_CLIENT */

      flagstoadd = FD_CLOEXEC;
   }

   if (fd == -1) {
      swarn("%s: could not open or create logfile \"%s\" for writing",
            function, logfile);

      return -1;
   }

   if ((p = fcntl(fd, F_GETFD, 0)) == -1)
      swarn("%s: fcntl(F_GETFD) on logfile \"%s\", fd %d, failed",
            function, logfile, fd);
   else {
      if (fcntl(fd, F_SETFD, p | flagstoadd) == -1)
         swarn("%s: fcntl(F_SETFD, 0x%x) on logfile \"%s\", fd %d, failed",
               function, p | flagstoadd, logfile, fd);
   }

   return fd;
}

static size_t
getlogprefix(priority, buf, buflen)
   const int priority;
   char *buf;
   size_t buflen;
{
   const char *p;
   static time_t last_secondsnow;
   static char laststr[128];
   static size_t laststr_lenused;
   struct timeval timenow;
   size_t i, tocopy, lenused;
   time_t secondsnow;
   pid_t pid;
   char s_string[22 /* see ltoa() doc. */],
        us_string[sizeof(s_string)], pid_string[sizeof(s_string)];

   if (buflen == 0)
      return 0;

   gettimeofday(&timenow, NULL);

   if (sockscf.state.pid == 0)
      pid = getpid(); /* don't change sockscf.state.pid; probably client. */
   else
      pid = sockscf.state.pid;

   lenused = 0;

   if ((secondsnow = (time_t)timenow.tv_sec) == last_secondsnow) {
      const size_t tocopy = MIN(buflen - lenused, laststr_lenused);

      memcpy(&buf[lenused], laststr, tocopy);
      lenused += tocopy; /* not counting NUL. */
   }
   else {
      struct tm *tm;

      if (sockscf.state.insignal
      ||  (tm = localtime(&secondsnow)) == NULL) {
         /*
          * can not call strftime(3) from signalhandler.
          */
         const char p[] = "<no localtime available> ";
         const size_t tocopy = MIN(buflen - lenused, sizeof(p) - 1);

         memcpy(&buf[lenused], p, tocopy);
         lenused += tocopy;
      }
      else {
         const size_t len = strftime(&buf[lenused], buflen - lenused,
                                     "%h %e %T ", tm);

         laststr_lenused = MIN(sizeof(laststr) - 1, len);
         memcpy(laststr, &buf[lenused], laststr_lenused);
         last_secondsnow = secondsnow;

         lenused += len;
      }
   }

#if 0
   /*
    * The rest of the code should produce the equivalent of the following
    * snprintf(3) call, but we don't want to call snprintf(3) as we may
    * be in a signalhandler.
    */
    snprintf(&buf[lenused], buflen - lenused,
             "(%ld.%06ld) %s[%ld]: %s: ",
             (long)timenow.tv_sec,
             (long)timenow.tv_usec,
             __progname,
             (long)pid,
             loglevel2string(priority));
#endif


   ltoa((long)timenow.tv_sec,  s_string,   sizeof(s_string));
   ltoa((long)timenow.tv_usec, us_string,  sizeof(us_string));
   ltoa((long)pid,             pid_string, sizeof(pid_string));

#define WANTED_DIGITS (6)  /* always want six digits. */

#if DIAGNOSTIC
   SASSERTX(strlen(us_string) <= WANTED_DIGITS);
#endif /* DIAGNOSTIC */

   if ((i = strlen(us_string)) < WANTED_DIGITS) {
      const size_t zeros_to_add = WANTED_DIGITS - i;
      size_t added;
      char debug[sizeof(us_string)];

      memcpy(debug, us_string, sizeof(debug));

      SASSERTX(us_string[i] == NUL);
      memmove(&us_string[zeros_to_add], us_string, i + 1);

      for (added = 0; added < zeros_to_add; ++added)
         us_string[added] = '0';

      SASSERTX(us_string[i + zeros_to_add] == NUL);

#if DIAGNOSTIC
      SASSERTX(strlen(us_string) == WANTED_DIGITS);
#endif /* DIAGNOSTIC */
   }

#if DIAGNOSTIC
   SASSERTX(strlen(us_string) == WANTED_DIGITS);

   SASSERTX(buflen - lenused >
            strlen("(")
               + strlen(s_string) + strlen(".") + strlen(us_string)
          + strlen(") ")
          + strlen(__progname)
          + strlen("[") + strlen(pid_string) + strlen("]: ")
          + strlen(loglevel2string(priority)) + strlen(": "));
#endif /* DIAGNOSTIC */

   buf[lenused++] = '(';

   p      = s_string;
   tocopy = MIN(buflen - lenused, strlen(p));
   memcpy(&buf[lenused], p, tocopy);
   lenused += tocopy;

   buf[lenused++] = '.';

   p      = us_string;
   tocopy = MIN(buflen - lenused, strlen(p));
   memcpy(&buf[lenused], p, tocopy);
   lenused += tocopy;

   buf[lenused++] = ')';
   buf[lenused++] = ' ';

   p      = __progname;
   tocopy = MIN(buflen - lenused, strlen(p));
   memcpy(&buf[lenused], p, tocopy);
   lenused += tocopy;

   buf[lenused++] = '[';

   p      = pid_string;
   tocopy = MIN(buflen - lenused, strlen(p));
   memcpy(&buf[lenused], p, tocopy);
   lenused += tocopy;

   buf[lenused++] = ']';
   buf[lenused++] = ':';
   buf[lenused++] = ' ';

   p      = loglevel2string(priority);
   tocopy = MIN(buflen - lenused, strlen(p));
   memcpy(&buf[lenused], p, tocopy);

   lenused += tocopy;
   buf[lenused++] = ':';
   buf[lenused++] = ' ';

   buf[lenused++] = NUL;

   --lenused;
   SASSERTX(buf[lenused] == NUL);

   return lenused;
}

int
socks_logmatch(d, log)
   int d;
   const logtype_t *log;
{
   size_t i;

   if (d < 0)
      return 0;

   for (i = 0; i < log->filenoc; ++i)
      if (d == log->filenov[i])
         return 1;

   return 0;
}


void
slogstack(void)
{
#if HAVE_BACKTRACE
   const char *function = "slogstack()";
   void *array[20];
   size_t i, size;
   char **strings;

   size    = backtrace(array, (int)ELEMENTS(array));
   strings = backtrace_symbols(array, size);

   if (strings == NULL)  {
      swarn("%s: strings = NULL", function);
      return;
   }

   for (i = 1; i < size; i++)
      slog(LOG_INFO, "%s: stackframe #%lu: %s\n",
           function, (unsigned long)i, strings[i]);

   free(strings);
#endif /* HAVE_BACKTRACE */
}

const loglevel_t *
loglevel(name)
   const char *name;
{
   static const loglevel_t loglevelv[MAXLOGLEVELS] = {
      { "emerg",   LOG_EMERG    },
      { "alert",   LOG_ALERT    },
      { "crit",    LOG_CRIT     },
      { "err",     LOG_ERR      },
      { "warning", LOG_WARNING  },
      { "notice",  LOG_NOTICE   },
      { "info",    LOG_INFO     },
      { "debug",   LOG_DEBUG    },
   };
   size_t i;

   CTASSERT(LOG_EMERG == 0);
   CTASSERT(LOG_DEBUG == 7);
   CTASSERT(LOG_DEBUG < MAXLOGLEVELS);
   CTASSERT(LOG_DEBUG < sizeof(loglevelv));

   for (i = 0; i < ELEMENTS(loglevelv); ++i)
      if (strcmp(name, loglevelv[i].name) == 0)
         return &loglevelv[i];

   return NULL;
}


static const struct syslogfacility *
syslogfacility(name)
   const char *name;
{
   static struct syslogfacility syslogfacilityv[] = {
         /* POSIX */
      { "user",     LOG_USER   },
      { "auth",     LOG_AUTH   },
      { "daemon",   LOG_DAEMON },
      { "local0",   LOG_LOCAL0 },
      { "local1",   LOG_LOCAL1 },
      { "local2",   LOG_LOCAL2 },
      { "local3",   LOG_LOCAL3 },
      { "local4",   LOG_LOCAL4 },
      { "local5",   LOG_LOCAL5 },
      { "local6",   LOG_LOCAL6 },
      { "local7",   LOG_LOCAL7 },

         /*
          * Non-POSIX.
          */
#ifdef LOG_AUTHPRIV
      { "authpriv",   LOG_AUTHPRIV  },
#endif /* LOG_AUTHPRIV */
   };

   size_t i;

   for (i = 0; i < ELEMENTS(syslogfacilityv); ++i)
      if (strcmp(name, syslogfacilityv[i].name) == 0)
         return &syslogfacilityv[i];

   return NULL;
}


#if HAVE_LIVEDEBUG

static void
socks_addtorb(str, lenopt)
   const char *str;
   const size_t lenopt;
{
   ssize_t overshoot;
   size_t len;

   if (dont_add_to_rb)
      return;

   if (lenopt <= 1)
      return;

#if DIAGNOSTIC
   SASSERTX(lenopt == strlen(str) + 1);
#endif /* DIAGNOSTIC */

   if (lenopt >= sizeof(ringbuffer))
      len = sizeof(ringbuffer) - 1 - 1; /* two strings, extra NUL */
   else
      len = lenopt - 1; /* ignore trailing NUL */

   overshoot = (ringbuf_curroff + len) - (sizeof(ringbuffer) - 1);
   if (overshoot > 0) {
      memmove(ringbuffer + ringbuf_curroff, str, len - overshoot);
      memmove(ringbuffer, str + len - overshoot, (size_t)overshoot);
      ringbuf_curroff = overshoot;
   }
   else {
      memmove(ringbuffer + ringbuf_curroff, str, len);
      ringbuf_curroff += len;
      if (ringbuf_curroff >= sizeof(ringbuffer) - 1)
         ringbuf_curroff = 0;
   }

   ringbuffer[ringbuf_curroff] = NUL;
   SASSERTX(ringbuffer[sizeof(ringbuffer) - 1] == NUL);

   return;
}

void
socks_flushrb(void)
{
   const char *function = "socks_flushrb()";
   const char *msgv[]
   = { function,
       ": flushing log buffer.  This should only happen upon fatal error.  "
       "\n\"\"\"",
       ringbuffer + ringbuf_curroff + 1,
       ringbuffer,
       "\n\"\"\"",
       NULL
     };
   const int old_dont_add_to_rb = dont_add_to_rb;

   dont_add_to_rb = 1; /* don't add this one to rb ... flushing. */

   signalslog(LOG_WARNING, msgv);

   dont_add_to_rb = old_dont_add_to_rb;
}

#if SOCKS_CLIENT
static void
atexit_flushrb(void)
{
   const char *function = "atexit_flushrb()";

   if (sockscf.state.internalerrordetected && !sockscf.option.debug) {
      slog(LOG_DEBUG, "%s", function);
      socks_flushrb();
   }
}
#endif /* SOCKS_CLIENT */

#endif /* HAVE_LIVEDEBUG */
