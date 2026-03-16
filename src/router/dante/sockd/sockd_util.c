/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2006, 2008,
 *               2009, 2010, 2011, 2012, 2013, 2014, 2019
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
"$Id: sockd_util.c,v 1.263.4.3.6.2 2020/11/11 16:12:03 karls Exp $";

#include "common.h"

int
selectmethod(methodv, methodc, offeredv, offeredc)
   const int *methodv;
   const size_t methodc;
   const unsigned char *offeredv;
   const size_t offeredc;
{
   const unsigned char *methodokv;
   size_t i, methodokc;
   int intmethodv[MAXMETHODS];

   if (offeredc == 0)
      return AUTHMETHOD_NOACCEPT;

   charmethod2intmethod(offeredc, offeredv, intmethodv);

   for (i = 0; i < methodc; ++i) {
      if (methodv[i] > AUTHMETHOD_NOACCEPT) {
         /*
          * non-socks method.  Can select any of the standard methods
          * that can provide the necessary information.
          */
         const unsigned char rfc931methodv[] = { AUTHMETHOD_NONE,
                                                 AUTHMETHOD_UNAME,
#if HAVE_GSSAPI
                                                 AUTHMETHOD_GSSAPI
#endif /* HAVE_GSSAPI */
                                               };

         const unsigned char pam_any_methodv[] = {   AUTHMETHOD_UNAME,
                                                     AUTHMETHOD_NONE,
#if HAVE_GSSAPI
                                                     AUTHMETHOD_GSSAPI,
#endif /* HAVE_GSSAPI */
                                            };

         const unsigned char pam_address_methodv[] = {   AUTHMETHOD_UNAME,
                                                         AUTHMETHOD_NONE,
#if HAVE_GSSAPI
                                                         AUTHMETHOD_GSSAPI,
#endif /* HAVE_GSSAPI */
                                            };

         const unsigned char pam_password_methodv[] = {   AUTHMETHOD_UNAME };

         const unsigned char bsdmethodv[] = {    AUTHMETHOD_UNAME,
#if HAVE_GSSAPI
                                                 AUTHMETHOD_GSSAPI,
#endif /* HAVE_GSSAPI */
                                            };

#if HAVE_LDAP
         const unsigned char ldapmethodv[] = {    AUTHMETHOD_UNAME,
#if HAVE_GSSAPI
                                                 AUTHMETHOD_GSSAPI,
#endif /* HAVE_GSSAPI */
                                            };
#endif /* HAVE_LDAP */

         size_t ii;

         /* find the correct array to use for selecting the method. */
         switch (methodv[i]) {
            case AUTHMETHOD_RFC931:
               methodokc = ELEMENTS(rfc931methodv);
               methodokv = rfc931methodv;
               break;

            case AUTHMETHOD_PAM_ANY:
               methodokc = ELEMENTS(pam_any_methodv);
               methodokv = pam_any_methodv;
               break;

            case AUTHMETHOD_PAM_ADDRESS:
               methodokc = ELEMENTS(pam_address_methodv);
               methodokv = pam_address_methodv;
               break;

            case AUTHMETHOD_PAM_USERNAME:
               methodokc = ELEMENTS(pam_password_methodv);
               methodokv = pam_password_methodv;
               break;

            case AUTHMETHOD_BSDAUTH:
               methodokc = ELEMENTS(bsdmethodv);
               methodokv = bsdmethodv;
               break;

#if HAVE_LDAP
            case AUTHMETHOD_LDAPAUTH:
               methodokc = ELEMENTS(ldapmethodv);
               methodokv = ldapmethodv;
               break;
#endif /* HAVE_LDAP */

            default:
               SERRX(methodv[i]);
         }

         for (ii = 0; ii < methodokc; ++ii)
            if (methodisset(methodokv[ii], intmethodv, offeredc))
               return methodokv[ii];

         continue;
      }

      if (memchr(offeredv, (unsigned char)methodv[i], offeredc) != NULL)
         return methodv[i];
   }

   return AUTHMETHOD_NOACCEPT;
}

int
pidismother(pid)
   pid_t pid;
{
   size_t i;

   if (sockscf.state.motherpidv == NULL)
      return 1; /* so early we haven't forked yet. */

   for (i = 0; i < sockscf.option.serverc; ++i)
      if (sockscf.state.motherpidv[i] == pid)
         return i + 1;

   return 0;
}

int
pidismainmother(pid)
   pid_t pid;
{

   return pidismother(pid) == 1;
}

int
descriptorisreserved(d)
   int d;
{
   size_t i;

   if (d == sockscf.hostfd
   ||  d == sockscf.shmemfd
   ||  d == sockscf.loglock

#if HAVE_LDAP
   ||  d == sockscf.ldapfd
#endif /* HAVE_LDAP */

   ||  d == sockscf.shmemconfigfd
   || FD_IS_RESERVED_EXTERNAL(d))
      return 1;

   /* don't close log files. */
   if (socks_logmatch(d, &sockscf.log)
   ||  socks_logmatch(d, &sockscf.errlog))
      return 1;

   for (i = 0; i < ELEMENTS(sockscf.state.reservedfdv); ++i)
      if (d == sockscf.state.reservedfdv[i])
         return 1;

   return 0;
}

void
sigserverbroadcast(sig)
   int sig;
{
   const char *function = "sigserverbroadcast()";
   size_t i;

   if (sockscf.state.motherpidv == NULL)
      return; /* so early we haven't forked yet. */

   for (i = 1; i < sockscf.option.serverc; ++i)
      if (sockscf.state.motherpidv[i] != 0) {
         slog(LOG_DEBUG, "%s: sending signal %d to mother %lu",
              function, sig, (unsigned long)(sockscf.state.motherpidv[i]));

         if (kill(sockscf.state.motherpidv[i], sig) != 0)
            swarn("%s: could not send signal %d to mother process %lu",
                  function,
                  sig,
                  (unsigned long)sockscf.state.motherpidv[i]);
   }
}

void
sockd_pushsignal(sig, siginfo)
   const int sig;
   const siginfo_t *siginfo;
{
   const char *function = "sockd_pushsignal()";
   sigset_t all, oldmask;
   size_t i, alreadythere;

   SASSERTX(sig > 0);

   (void)sigfillset(&all);
   if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0) {
      const char *msgv[]
      = { function,
          ": sigprocmask(SIG_SETMASK) failed with errno ",
          ltoa(errno, NULL, 0),
          NULL
        };

      signalslog(LOG_WARNING, msgv);
   }

   /* go through currently pending signals.  If already there, don't add. */
   for (i = alreadythere = 0; i < (size_t)sockscf.state.signalc; ++i)
      if (sockscf.state.signalv[i].signal == sig) {
         alreadythere = 1;
         break;
      }

   if (!alreadythere) {
      if (i < ELEMENTS(sockscf.state.signalv)) {
         sockscf.state.signalv[sockscf.state.signalc].signal = sig;

         if (siginfo != NULL)
            sockscf.state.signalv[sockscf.state.signalc].siginfo = *siginfo;
         else /* Solaris ...  */
            bzero(&sockscf.state.signalv[sockscf.state.signalc].siginfo,
                  sizeof(sockscf.state.signalv[sockscf.state.signalc].siginfo));

         ++sockscf.state.signalc;

         if (1) {
            char sigbuf[32], pidbuf[32], numbuf[32];
            const char *msgv[]
            = { function,
                ": pushed signal ",
                ltoa(sig, sigbuf, sizeof(sigbuf)),
                " from pid ",
                ltoa(sockscf.state.signalv[sockscf.state.signalc - 1]
                     .siginfo.si_pid,
                     pidbuf,
                     sizeof(pidbuf)),
                ".  Number of signals on the stack is now ",
                ltoa(sockscf.state.signalc, numbuf, sizeof(numbuf)),
                NULL
              };

            signalslog(LOG_DEBUG, msgv);
         }
      }
      else
         SWARNX(i);
   }

   if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0) {
      const char *msgv[]
      = { function,
          ": sigprocmask(SIG_SETMASK) restoration failed with errno ",
          ltoa(errno, NULL, 0),
          NULL
        };

      signalslog(LOG_WARNING, msgv);
   }
}

int
sockd_popsignal(siginfo_t *siginfo)
{
   const char *function = "sockd_popsignal()";
   sigset_t all, oldmask;
   int sig;

   (void)sigfillset(&all);
   if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK)", function);

   SASSERTX(sockscf.state.signalc > 0);

   sig      = sockscf.state.signalv[0].signal;
   *siginfo = sockscf.state.signalv[0].siginfo;

   memmove(sockscf.state.signalv,
           &sockscf.state.signalv[1],
           sizeof(*sockscf.state.signalv) * (--sockscf.state.signalc));

   if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
      swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);

   return sig;
}

int
sockd_handledsignals()
{
   const char *function = "sockd_handledsignals()";
   const int errno_s = errno;
   struct sigaction oact;
   int i, rc = 0;

   if (sockscf.state.signalc == 0)
      return 0;

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      for (i = 0, rc = 0; i < sockscf.state.signalc; ++i)
         slog(LOG_DEBUG, "%s: signal #%d on the stack is signal %d",
              function, i + 1, (int)sockscf.state.signalv[i].signal);

   while (sockscf.state.signalc) {
      siginfo_t siginfo;
      const int signal = sockd_popsignal(&siginfo);

      slog(LOG_DEBUG,
            "%s: signals left on the stack after popping signal %d: %d",
            function, signal, sockscf.state.signalc);

      if (sigaction(signal, NULL, &oact) != 0)
         SERR(0);


      if (oact.sa_handler != SIG_IGN && oact.sa_handler != SIG_DFL) {
         oact.sa_sigaction(-signal, &siginfo, NULL);
         ++rc;
      }
      else
         /*
          * can happen when a child temporarily changes the
          * signal disposition while starting up.
          */
         slog(LOG_DEBUG, "%s: no handler for signal %d at the moment",
              function, signal);
   }

   errno = errno_s;
   return rc;
}

int
freedescriptors(message, hfd)
   const char *message;
   int *hfd;
{
   const int errno_s = errno;
   int i, freefds, tmp_hfd, max;

   if (sockscf.state.highestfdinuse != 0 && hfd == NULL)
      /* not asked to recalculate, so assume previous result is still valid. */
      max = sockscf.state.highestfdinuse;
  else
      max = sockscf.state.maxopenfiles;

   if (hfd == NULL) {
      tmp_hfd = 0;
      hfd     = &tmp_hfd;
   }

   for (i = freefds = 0; i < max; ++i)
      if (fdisopen((int)i))
         *hfd = MAX(i, *hfd);
      else
         ++freefds;

   freefds += (sockscf.state.maxopenfiles - max);

   if (message != NULL)
      slog(LOG_DEBUG, "freedescriptors(%s): free : %d/%ld, highest in use: %d",
           message, freefds, (long)sockscf.state.maxopenfiles, *hfd);

   errno = errno_s;
   return freefds;
}

int
sockd_motherexists(void)
{
   const pid_t pid = getppid();
   size_t i;

   if (sockscf.state.motherpidv == NULL)
      return 1; /* so early we haven't forked yet.  We must be mother then. */

   /*
    * A simple getppid(2) unfortunately no longer works on all platforms.
    * E.g. Solaris apparently, annoyingly enough, returns the pid of the
    * zone scheduler process, and not initd. :-(
    * So go through the list of all motherpids to see if getppid() matches
    * any of them and use that to decide if mother still exists.
    */
   for (i = 0; i < sockscf.option.serverc; ++i)
      if (sockscf.state.motherpidv[i] == pid)
         return 1;

   return 0;
}

void
sockdexit(code)
   const int code;
{
   const char *function = "sockdexit()";
   struct sigaction sigact;
   static int exiting;

   if (exiting) /* this must have gotten really screwed up. */
      abort();
   else
      exiting = 1;

   /*
    * we are terminating; don't want to receive SIGTERM or SIGCHLD now.
    */
   bzero(&sigact, sizeof(sigact));
   sigact.sa_handler = SIG_IGN;
   if (sigaction(SIGTERM, &sigact, NULL) != 0
   ||  sigaction(SIGCHLD, &sigact, NULL) != 0)
      swarn("%s: sigaction()", function);

   slog(LOG_DEBUG, "%s: insignal = %d", function, (int)sockscf.state.insignal);

   if (sockscf.state.type == PROC_MOTHER) {
      struct rusage rusage;

      closechild(0, 1); /* tell them we are shutting down orderly now. */

#if HAVE_ENABLED_PIDFILE
      if (sockscf.option.pidfilewritten
      && pidismainmother(sockscf.state.pid)) {
         sockd_priv(SOCKD_PRIV_FILE_WRITE, PRIV_ON);

         if (truncate(sockscf.option.pidfile, 0) != 0)
            swarn("%s: truncate(%s)", function, sockscf.option.pidfile);

         sockd_priv(SOCKD_PRIV_FILE_WRITE, PRIV_OFF);
      }
#endif /* HAVE_ENABLED_PIDFILE */

      if (getrusage(RUSAGE_SELF, &rusage) == 0)
         log_rusage(PROC_MOTHER,  0, &rusage);

      log_rusage(PROC_MONITOR,   0, &sockscf.state.rusage_monitor);
      log_rusage(PROC_NEGOTIATE, 0, &sockscf.state.rusage_negotiate);
      log_rusage(PROC_REQUEST,   0, &sockscf.state.rusage_request);
      log_rusage(PROC_IO,        0, &sockscf.state.rusage_io);
   }
   else {
      if (sockscf.state.insignal)
         slog(LOG_DEBUG, "%s: shutting down on signal %d",
              childtype2string(sockscf.state.type), sockscf.state.insignal);
      else
         slog(LOG_DEBUG, "%s: shutting down",
              childtype2string(sockscf.state.type));
   }

#if HAVE_PROFILING
   if (chdir(SOCKS_PROFILEDIR) != 0) {
      if (sockscf.state.type == PROC_MOTHER)
         slog(LOG_ERR,
              "%s: profiling is enabled, but could not chdir(2) to it (%s).  "
              "If you wish profiling output to be saved, create a directory "
              "named \"%s\" in the same as directory as you start %s",
              function, strerror(errno), SOCKS_PROFILEDIR, PRODUCT);
   }
   else {
      char dir[80];

      snprintf(dir, sizeof(dir), "%s.%ld",
              childtype2string(sockscf.state.type), (long)getpid());

      if (mkdir(dir, S_IRWXU) != 0)
         swarn("%s: mkdir(%s)", function, dir);
      else
         if (chdir(dir) != 0)
            swarn("%s: chdir(%s)", function, dir);
   }
#endif /* HAVE_PROFILING */

   if (sockscf.state.type == PROC_MOTHER) {
      if (pidismainmother(sockscf.state.pid)) {
         sigserverbroadcast(SIGTERM); /* signal other mothers too. */

         /*
          * mainly for removing old shared memory stuff and temporary files.
          * Do this last to reduce the chance of a child trying to use a
          * shmemfile we have already removed.
          */
         resetconfig(&sockscf, 1);
      }

      if (sockscf.state.insignal)
         slog(LOG_ALERT, "%s[%d/%lu]: shutting down on signal %d",
              childtype2string(sockscf.state.type),
              pidismother(sockscf.state.pid),
              (unsigned long)sockscf.option.serverc,
              sockscf.state.insignal);
      else
         slog(LOG_ALERT, "%s[%d/%lu]: shutting down",
              childtype2string(sockscf.state.type),
              pidismother(sockscf.state.pid),
              (unsigned long)sockscf.option.serverc);

      exit(code);
   }

   /*
    * Else; we are a child.
    */

#if HAVE_PROFILING
   exit(code);
#else
   fflush(NULL);
   _exit(code);
#endif /* HAVE_PROFILING */
}
