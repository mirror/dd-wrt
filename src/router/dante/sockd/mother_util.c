/*
 * Copyright (c) 2013, 2014, 2019, 2020
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
"$Id: mother_util.c,v 1.22.4.8.6.3 2020/11/11 16:11:59 karls Exp $";

/*
 * signal handler functions.  Upon reception of signal, "sig" is the real
 * signal value (> 0).  We then set a flag indicating we got a signal,
 * but we don't do anything and return immediately.  Later we are called
 * again, with "sig" having the value -(sig), to indicate we are not
 * executing in the signal handler and it's safe to do whatever we
 * need to do.
 */
static void sigterm(int sig, siginfo_t *si, void *sc);
static void siginfo(int sig, siginfo_t *si, void *sc);
static void sigchld(int sig, siginfo_t *si, void *sc);
static void sigalrm(int sig, siginfo_t *si, void *sc);
static void  sighup(int sig, siginfo_t *si, void *sc);

static void unexpecteddeath(void);
/*
 * Should be called after any unexpected child death / child removal.
 * May disable creation of further children for a while and log a warning
 * if appropriate, or the enable creation of further children.
 */

void
mother_preconfigload(void)
{

}

void
mother_postconfigload(void)
{
   const char *function = "mother_postconfigload()";

   if (pidismainmother(sockscf.state.pid))
      shmem_idupdate(&sockscf);  /* only main mother does this. */
}

void
mother_envsetup(argc, argv)
   int argc;
   char *argv[];
{
   const char *function = "mother_envsetup()";
   const int exitsignalv[] = {
      SIGINT, SIGQUIT, SIGBUS, SIGSEGV, SIGTERM, SIGILL, SIGFPE
#ifdef SIGSYS
      , SIGSYS
#endif /* SIGSYS */
   };
   const size_t pipetomotherfds = 2; /* fds needed for pipe to mother. */
   const size_t exitsignalc = ELEMENTS(exitsignalv);
   const int ignoresignalv[] = {
      SIGPIPE
   };
   const size_t ignoresignalc = ELEMENTS(ignoresignalv);
   struct sigaction sigact;
   struct rlimit rlimit;
#ifdef RLIMIT_NPROC
   struct rlimit maxproc;
#endif /* RLIMIT_NPROC */
   rlim_t maxopenfd, minfd_neg, minfd_req, minfd_io, minfd;
   size_t i, fdreserved;

   for (fdreserved = 0;
   fdreserved < ELEMENTS(sockscf.state.reservedfdv);
   ++fdreserved) {
      if (sockscf.state.reservedfdv[fdreserved] == -1) {
         if ((sockscf.state.reservedfdv[fdreserved] = makedummyfd(0, 0)) == -1)
            serr("%s: could not reserve fd #%lu for later use",
                 function, (unsigned long)fdreserved + 1);
      }
   }

   /*
    * close any descriptor we don't need, both in case of chroot(2)
    * and for needing every descriptor we can get.
    */

   /* assume syslog uses one */
   fdreserved += sockscf.log.type & LOGTYPE_SYSLOG ? 0 : 1;

   /*
    * shmem-segments we may need to attach to temporarily in relation
    * to doing rulespermit() and similar for a client.
    */
   fdreserved += 1  /* bw fd      */
               + 1  /* session fd */
               + 1; /* monitor fd */

   for (i = 0, maxopenfd = getmaxofiles(softlimit); (rlim_t)i < maxopenfd; ++i){
      size_t j;

      if (descriptorisreserved((int)i)) {
         ++fdreserved;
         continue;
      }

      /* sockets we listen on. */
      for (j = 0; j < sockscf.internal.addrc; ++j) {
         if ((int)i == sockscf.internal.addrv[j].s)
            break;
      }

      if (j < sockscf.internal.addrc) /* listening on this socket. */
         continue;

      close((int)i);
   }

   errno = 0;
   newprocinit(); /* just in case the above closed a syslog(3) fd. */

   /*
    * Check system limits against what we need.
    * Enough descriptors for each child process? + 2 for the pipes from
    * the child to mother.
    */

   minfd_neg = (SOCKD_NEGOTIATEMAX * 1)          + pipetomotherfds + fdreserved;

   minfd_req = (SOCKD_REQUESTMAX   * FDPASS_MAX) + pipetomotherfds + fdreserved;

   minfd_io  = (SOCKD_IOMAX        * FDPASS_MAX) + pipetomotherfds + fdreserved;

   /* i/o process stays attached to bw and monitor shmem all the time. */
   minfd_io  += SOCKD_IOMAX * (1 + 1);

#if BAREFOOTD
   minfd_io += MIN(10, MIN_UDPCLIENTS);
#endif

   slog(LOG_DEBUG,
        "%s: minfd_negotiate: %lu, minfd_request: %lu, minfd_io: %lu",
        function,
        (unsigned long)minfd_neg,
        (unsigned long)minfd_req,
        (unsigned long)minfd_io);
   /*
    * need to know max number of open files so we can allocate correctly
    * sized fd_sets.  Also, try to set both it and the max number of
    * processes to the hard limit.
    */

   sockscf.state.maxopenfiles = getmaxofiles(hardlimit);

   slog(LOG_DEBUG,
        "hard limit for max number of open files is %lu, soft limit is %lu",
        (unsigned long)sockscf.state.maxopenfiles,
        (unsigned long)getmaxofiles(softlimit));

   if (sockscf.state.maxopenfiles == RLIM_INFINITY) {
      sockscf.state.maxopenfiles = getmaxofiles(softlimit);
      SASSERTX(sockscf.state.maxopenfiles != RLIM_INFINITY);
   }

   minfd = MAX(minfd_neg, MAX(minfd_req, minfd_io));

   if (sockscf.state.maxopenfiles < minfd) {
      slog(LOG_INFO,
           "have only %lu file descriptors available, but need at least %lu "
           "according to the configuration.  Trying to increase it ...",
           (unsigned long)sockscf.state.maxopenfiles,
           (unsigned long)minfd);

      sockscf.state.maxopenfiles = minfd;
   }

   rlimit.rlim_cur = rlimit.rlim_max = sockscf.state.maxopenfiles;

   if (setrlimit(RLIMIT_OFILE, &rlimit) == 0)
      slog(LOG_DEBUG, "max number of file descriptors is now %lu",
           (unsigned long)sockscf.state.maxopenfiles);
  else {
      const char *problem;

      sockscf.state.maxopenfiles = getmaxofiles(hardlimit);

      if (sockscf.state.maxopenfiles      < minfd_neg)
         problem = "SOCKD_NEGOTIATEMAX";
      else if (sockscf.state.maxopenfiles < minfd_req)
         problem = "SOCKD_REQUESTMAX";
      else if (sockscf.state.maxopenfiles < minfd_io)
         problem = "SOCKD_IOMAX";
      else
         SERRX(sockscf.state.maxopenfiles);

      serrx("%s: failed to increase the max number of open file descriptors "
            "for ourselves via setrlimit(RLIMIT_OFILE) to %lu: %s.  "
            "Increase the kernel/shell's max open files limit, or reduce "
            "the %s value in %s's include/config.h, or we will be unable to "
            "start up",
            function,
            (unsigned long)rlimit.rlim_max,
            strerror(errno),
            problem,
            PRODUCT);
   }

#ifdef RLIMIT_NPROC
   if (getrlimit(RLIMIT_NPROC, &maxproc) != 0)
      swarn("getrlimit(RLIMIT_NPROC) failed");
   else {
      maxproc.rlim_cur = maxproc.rlim_max;

      if (setrlimit(RLIMIT_NPROC, &maxproc) != 0)
         swarn("setrlimit(RLIMIT_NPROC, { %lu, %lu }) failed",
               (unsigned long)rlimit.rlim_cur,
               (unsigned long)rlimit.rlim_max);
   }
#endif /* !RLIMIT_NPROC */


   /*
    * set up signal handlers.
    */

   bzero(&sigact, sizeof(sigact));
   sigact.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;

   sigact.sa_sigaction = siginfo;
#if HAVE_SIGNAL_SIGINFO
   if (sigaction(SIGINFO, &sigact, NULL) != 0)
      serr("sigaction(SIGINFO)");
#endif /* HAVE_SIGNAL_SIGINFO */

   /*
    * same handler, for systems without SIGINFO, as well as systems with
    * broken ("more secure") signal semantics.
    */
   if (sigaction(SIGUSR1, &sigact, NULL) != 0)
      serr("sigaction(SIGUSR1)");

   sigact.sa_sigaction = sighup;
   if (sigaction(SIGHUP, &sigact, NULL) != 0)
      serr("sigaction(SIGHUP)");

   sigact.sa_sigaction = sigchld;
   if (sigaction(SIGCHLD, &sigact, NULL) != 0)
      serr("sigaction(SIGCHLD)");

   sigact.sa_sigaction = sigterm;
   for (i = 0; (size_t)i < exitsignalc; ++i)
      if (sigaction(exitsignalv[i], &sigact, NULL) != 0)
         serr("sigaction(%d)", exitsignalv[i]);

   sigact.sa_handler = SIG_IGN;
   for (i = 0; (size_t)i < ignoresignalc; ++i)
      if (sigaction(ignoresignalv[i], &sigact, NULL) != 0)
         serr("sigaction(%d)", ignoresignalv[i]);

   sigact.sa_flags     = SA_SIGINFO;   /* want to be interrupted. */
   sigact.sa_sigaction = sigalrm;
   if (sigaction(SIGALRM, &sigact, NULL) != 0)
      serr("sigaction(SIGALRM)");

   if (sockscf.option.daemon) {
      if (daemon(1, 0) != 0)
         serr("daemon()");

      /*
       * leave stdout/stderr, but close stdin.
       * Note that this needs to be done before newprocinit() (which
       * sets up syslog-thing), as it may happen that the syslog socket
       * is 0, and we don't want to close that.
       */
      close(STDIN_FILENO);
      newprocinit(); /* for daemon(). */

      *sockscf.state.motherpidv = getpid();   /* we are the main mother. */
   }

   if (HAVE_ENABLED_PIDFILE) {
      const mode_t openmode  = S_IRUSR  | S_IWUSR  | S_IRGRP | S_IROTH;
      const int    openflags = O_WRONLY | O_CREAT;
      FILE *fp;
      int fd;

      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);
      if ((fd = open(sockscf.option.pidfile, openflags, openmode)) == -1) {
         swarn("could not open pidfile %s for writing", sockscf.option.pidfile);
         errno = 0;
      }
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);

      if (fd != -1) {
         fp = fdopen(fd, "w");

         if (fp != NULL) {
            if (fprintf(fp, "%lu\n", (unsigned long)sockscf.state.pid) == EOF)
               swarn("failed writing pid to pidfile %s",
                     sockscf.option.pidfile);
            else
               sockscf.option.pidfilewritten = 1;

            fclose(fp);
         }

         close(fd);
      }

   }

   enable_childcreate();

   freedescriptors(NULL, &sockscf.state.highestfdinuse);
}

char *
mother_getlimitinfo(void)
{
   const char *function = "mother_getlimitinfo()";
   static char buf[2048];
   const int fds_per_proc = 2; /* two pipes */
   const char *limiter, *prefix = "max clients calculation will not be done";
   struct rlimit maxfd, maxproc;
   char maxprocstr[64], maxfdstr[64];
   unsigned long negc_proc, negc_fd, reqc_proc, reqc_fd, ioc_proc, ioc_fd,
                 negc_limit, reqc_limit, ioc_limit,
                 proc_free, proc_used, procs, fds_free;

   if (getrlimit(RLIMIT_NOFILE, &maxfd) != 0) {
      swarn("%s: getrlimit(RLIMIT_NOFILE) failed", function);
      return "";
   }

#ifdef RLIMIT_NPROC
   if (getrlimit(RLIMIT_NPROC, &maxproc) != 0) {
      swarn("%s: %s: getrlimit(RLIMIT_NPROC) failed", function, prefix);
      return "";
   }
#else /* !RLIMIT_NPROC */
   if ((maxproc.rlim_cur = (rlim_t)sysconf(_SC_CHILD_MAX)) == (rlim_t)-1) {
      swarn("%s: %s: sysconf(_SC_CHILD_MAX) failed", function, prefix);
      return "";
   }

   maxproc.rlim_max = maxproc.rlim_cur;
#endif /* !RLIMIT_NPROC */

   if (maxfd.rlim_cur   == RLIM_INFINITY
   &&  maxproc.rlim_cur == RLIM_INFINITY)
      return "no applicable environment resource limits configured";

   proc_used   = sockscf.option.serverc
               + childcheck(-PROC_NEGOTIATE) / SOCKD_NEGOTIATEMAX
               + childcheck(-PROC_REQUEST)   / SOCKD_REQUESTMAX
               + childcheck(-PROC_IO)        / SOCKD_IOMAX;
   proc_free   = maxproc.rlim_cur - proc_used;

   if (maxproc.rlim_cur == RLIM_INFINITY)
      snprintf(maxprocstr, sizeof(maxprocstr), "no limit");
   else
      snprintf(maxprocstr, sizeof(maxprocstr),
               "%lu (%lu free)",
               (unsigned long)maxproc.rlim_cur, proc_free);

   fds_free = freedescriptors(NULL, NULL) - FDPASS_MAX;
   if (maxfd.rlim_cur == RLIM_INFINITY)
      snprintf(maxfdstr, sizeof(maxfdstr), "no limit");
   else
      snprintf(maxfdstr, sizeof(maxfdstr),
               "%lu (%lu free)", (unsigned long)maxfd.rlim_cur, fds_free);

   /*
    * Calculate the max number of new clients we can handle based on both
    * the process resource limit and the fd limit.
    */

   /*
    * Process-based limit, disregarding any other limits.
    * Each process can handle SOCKD_{NEGOTIATE,REQUEST,IO}MAX clients.
    * We can create a max number of proc_free additional processes, so
    * the number of additional clients we can handle is the number
    * of additional clients multiplied by the number of clients each
    * process can handle.
    */
   negc_proc = proc_free * SOCKD_NEGOTIATEMAX;
   reqc_proc = proc_free * SOCKD_REQUESTMAX;
   ioc_proc  = proc_free * SOCKD_IOMAX;

   /*
    * FD-based limit, disregarding any other limits.
    * With the fds we have, we can create a given number of additional
    * processes (procs).
    * Each process needs fds_per_proc, and an additional
    * SOCKD_{NEGOTIATE,REQUEST,IO}MAX * <number of fds per client in this
    * phase> fds to handle the max number of clients, meaning we can handle
    * the following number of additional clients:
    */
   procs   = fds_free / fds_per_proc;
   negc_fd = MIN(((fds_free - fds_per_proc) / 1), SOCKD_NEGOTIATEMAX)
           * procs;
   reqc_fd = MIN(((fds_free - fds_per_proc) / FDPASS_MAX), SOCKD_REQUESTMAX)
           * procs;
   ioc_fd  = MIN(((fds_free - fds_per_proc) / FDPASS_MAX), SOCKD_IOMAX)
           * procs;

   /*
    * Different process-types could be limited by different things, but
    * ignore that here.
    */
   if (negc_proc < negc_fd
   ||  reqc_proc < reqc_fd
   ||  ioc_proc  < ioc_fd) {
      limiter = "process";

      negc_limit = negc_proc;
      reqc_limit = reqc_proc;
      ioc_limit  = ioc_proc;
   }
   else {
      limiter = "open file";

      negc_limit = negc_fd;
      reqc_limit = reqc_fd;
      ioc_limit  = ioc_fd;
   }

   snprintf(buf, sizeof(buf), "max limits: processes: %s, files: %s, "
            "%s-slots: %lu, %s-slots: %lu, %s-slots: %lu "
            "(max clients limited by %s limit)",
            maxprocstr,
            maxfdstr,
            childtype2string(PROC_NEGOTIATE),
            negc_limit,
            childtype2string(PROC_REQUEST),
            reqc_limit,
            childtype2string(PROC_IO),
            ioc_limit,
            limiter);

   return buf;
}

void
log_rusage(childtype, pid, rusage)
   const int childtype;
   const pid_t pid;
   const struct rusage *rusage;
{
   char prefix[256];

   if (pid == 0)
      snprintf(prefix, sizeof(prefix),
               "sum of resource usage for all %s processes",
               childtype2string(childtype));
   else
      snprintf(prefix, sizeof(prefix),
               "resource usage for %s-child %lu",
               childtype2string(childtype),
               (unsigned long)pid);


   slog(LOG_DEBUG,
        "%s\n"
        "  ru_utime   : %ld.%06lds\n"
        "  ru_stime   : %ld.%06lds\n"
        "  ru_minflt  : %ld\n"
        "  ru_majflt  : %ld\n"
        "  ru_nswap   : %ld\n"
        "  ru_inblock : %ld\n"
        "  ru_oublock : %ld\n"
        "  ru_msgsnd  : %ld\n"
        "  ru_msgrcv  : %ld\n"
        "  ru_nsignals: %ld\n"
        "  ru_nvcsw   : %ld\n"
        "  ru_nivcsw  : %ld\n",
        prefix,
        (long)rusage->ru_utime.tv_sec, (long)rusage->ru_utime.tv_usec,
        (long)rusage->ru_stime.tv_sec, (long)rusage->ru_stime.tv_usec,
        rusage->ru_minflt,
        rusage->ru_majflt,
        rusage->ru_nswap,
        rusage->ru_inblock,
        rusage->ru_oublock,
        rusage->ru_msgsnd,
        rusage->ru_msgrcv,
        rusage->ru_nsignals,
        rusage->ru_nvcsw,
        rusage->ru_nivcsw);
}



static void
unexpecteddeath(void)
{
   const char *function = "unexpecteddeath()";
   const size_t maxdeaths = 10;


   ++sockscf.state.unexpected_deaths;
   time_monotonic(&sockscf.state.lastdeath_time);

   if (sockscf.state.unexpected_deaths == 1)
      sockscf.state.firstdeath_time = sockscf.state.lastdeath_time;

   if (sockscf.state.unexpected_deaths < maxdeaths)
      return;

   if (sockscf.state.unexpected_deaths == maxdeaths) { /* log once only. */
      slog(LOG_ERR,
          "%s: %lu child deaths in %ld seconds.  Not something we are "
          "expecting so possibly something is wrong.  Locking childcount "
          "for a while (%ld seconds), hoping things will stabilize",
          function,
          (unsigned long)sockscf.state.unexpected_deaths,
          (long)socks_difftime(sockscf.state.lastdeath_time,
                               sockscf.state.firstdeath_time),
          (long)MAX_ADDCHILD_SECONDS);

      disable_childcreate(0, "large amount of processes suddenly died");
      alarm(MAX_ADDCHILD_SECONDS);
   }
}

/* ARGSUSED */
static void
sigalrm(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "sigalrm()";
   const int errno_s = errno;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   enable_childcreate();

   SIGNAL_EPILOGUE(sig, si, errno_s);
}

/* ARGSUSED */
static void
sigterm(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "sigterm()";
   const int errno_s = errno;

   if (sig > 0) {
      if (SIGNALISOK(sig))
         SIGNAL_PROLOGUE(sig, si, errno_s);
      else {
         /*
          * A bad signal, something has crashed.  Can't count
          * on it being possible to continue from here, have
          * to exit now.
          */
         const char *msgv[] = { function,
                                ": terminating on unexpected signal ",
                                ltoa(sig, NULL, 0),
                                NULL
                              };
         sigset_t clearmask;

         sockscf.state.insignal = sig;

         signalslog(LOG_WARNING, msgv);

#if HAVE_LIVEDEBUG
         if (!sockscf.option.debug)
            socks_flushrb();
#endif /* HAVE_LIVEDEBUG */

         /*
          * Reinstall default signal handler for this signal and raise it
          * again, assuming we will terminate and get a coredump if that is
          * the default behavior.
          */
         if (signal(sig, SIG_DFL) == SIG_ERR) {
            const char *msgv[]
            = { function,
                ": failed to reinstall original handler for signal %d",
                ltoa(sig, NULL, 0),
                NULL
              };

            signalslog(LOG_WARNING, msgv);
         }

         bzero(&clearmask, sizeof(clearmask));
         (void)sigprocmask(SIG_SETMASK, &clearmask, NULL);
         raise(sig);

         return; /* need to exit this signal handler so the default can run. */
      }
   }
   else
      sig = -sig;

   slog(LOG_INFO, "%s: exiting on signal %d", function, sig);

   SIGNAL_EPILOGUE(sig, si, errno_s);

   sockdexit(EXIT_SUCCESS);
}

/* ARGSUSED */
static void
siginfo(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "siginfo()";
   const int errno_s = errno;
   unsigned long days, hours, minutes, seconds,
                 free_negc, free_reqc, free_ioc,
                 max_negc,  max_reqc,  max_ioc;
   size_t clients;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   clients = 0;
   clients += (max_negc = childcheck(-PROC_NEGOTIATE));
   clients += (max_reqc = childcheck(-PROC_REQUEST));
   clients += (max_ioc  = childcheck(-PROC_IO));

   clients -= (free_negc = childcheck(PROC_NEGOTIATE));
   clients -= (free_reqc = childcheck(PROC_REQUEST));
   clients -= (free_ioc  = childcheck(PROC_IO));

   seconds = (unsigned long)socks_difftime(time_monotonic(NULL),
                                           sockscf.stat.boot);
   seconds2days(&seconds, &days, &hours, &minutes);

   slog(LOG_INFO, "%s v%s up %lu day%s, %lu:%.2lu, a: %lu, h: %lu c: %lu",
         PRODUCT,
         VERSION,
         days,
         days == 1 ? "" : "s",
         hours,
         minutes,
         (unsigned long)sockscf.stat.accepted,
         (unsigned long)sockscf.stat.negotiate.sendt,
         (unsigned long)clients);

   slog(LOG_INFO, "negotiators (%lu): a: %lu, h: %lu, c: %lu, f: %lu",
        max_negc / SOCKD_NEGOTIATEMAX,
        (unsigned long)sockscf.stat.negotiate.sendt,
        (unsigned long)sockscf.stat.negotiate.received,
        max_negc - free_negc,
        free_negc);

   slog(LOG_INFO, "requesters (%lu): a: %lu, h: %lu, c: %lu, f: %lu",
        max_reqc / SOCKD_REQUESTMAX,
        (unsigned long)sockscf.stat.request.sendt,
        (unsigned long)sockscf.stat.request.received,
        max_reqc - free_reqc,
        free_reqc);

   slog(LOG_INFO, "iorelayers (%lu): a: %lu, h: %lu, c: %lu, f: %lu",
        max_ioc / SOCKD_IOMAX,
        (unsigned long)sockscf.stat.io.sendt,
        (unsigned long)sockscf.stat.io.received,
        max_ioc - free_ioc,
        free_ioc);

   slog(LOG_INFO, "%s", mother_getlimitinfo());

   /*
    * Regarding kill(2), the OpenBSD manpage says this:
    *
    * """
    * Setuid and setgid processes are dealt with slightly differently.
    * For the non-root user, to prevent attacks against such processes,
    * some signal deliveries are not permitted and return the error
    * EPERM.  The following signals are allowed through to this class
    * of processes: SIGKILL, SIGINT, SIGTERM, SIGSTOP, SIGTTIN, SIGTTOU,
    * SIGTSTP, SIGHUP, SIGUSR1, SIGUSR2.
    * """
    *
    * The practical effect of this seems to be that if we use different
    * userids, we, when running with the euid of something other than root,
    * may not be able to send the SIGINFO signal to our own children. :-/
    * Simlar problem exists for FreeBSD.
    *
    * To workaround the problem, send SIGUSR1 to the children instead of
    * SIGINFO, as SIGUSR1 has always been treated the same way as SIGINFO
    * by Dante due to some platforms not having the SIGINFO signal.
    */
#ifdef SIGINFO
   if (sig == SIGINFO)
      sig = SIGUSR1;
#endif

   if (pidismainmother(sockscf.state.pid))   /* main mother */
      sigserverbroadcast(sig);

   sigchildbroadcast(sig);

   SIGNAL_EPILOGUE(sig, si, errno_s);
}

/* ARGSUSED */
static void
sighup(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "sighup()";
   const int errno_s = errno;
   struct config *newshmemconfig;
   internaladdress_t oldinternal;
   size_t i, pointersize;
   int rc;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   slog(LOG_INFO, "SIGHUP [: reloading config");

   /*
    * Copy the current addresses on the internal interfaces so that after
    * we have read in the new configuration, we can compare the old list
    * against the new to know which addresses/sockets are longer in use,
    * and stop listening on them.
    *
    * We can not simply clear them before reading in the new config
    * and then start listening on them (again) after we in read the new
    * config, as that would mean we could lose clients in the time-gap
    * between unbinding and rebinding the addresses.
    *
    * This is mainly for barefootd, where adding/removing bounce-to
    * addresses is probably not uncommon.  In the case of barefootd,
    * we additionally have udp addresses we listen on constantly that
    * we need to handle in a similar way.
    *
    * We also have a slight problem with udp rules, as we need to
    * know if the rule existed before the reload.  If it did,
    * we will fail when we try to bind on the internal side,
    * and also waste time trying to set up bouncing for the same
    * udp addresses several times.  More importantly, we will not
    * know whether the error is expected, or if we should tell the
    * user he is trying to use an address already in use by
    * somebody else.
    * The same problem occurs if we have multiple rules with the
    * same "to:" address, which can make sense provided "from:"
    * differs.  We then have multiple acls for the same "to:" address,
    * but of course only one "to:" address/socket.
    *
    * Our solution for this is to also save the unique udp addresses we
    * need to listen to, and compare against them upon config reload.
    * If one of the udp address is the same as before, we consider the
    * session to be "bounced" already, and if one of the addresses
    * present on the old list is not present on the new list, we know
    * we have an old session/socket to terminate.
    */

    oldinternal.addrc      = sockscf.internal.addrc;
    if ((oldinternal.addrv = malloc(sizeof(*oldinternal.addrv)
                                           * oldinternal.addrc)) == NULL) {
      swarn("%s: failed to allocate memory for saving state before "
            "configuration reload",
            function);

      return;
   }

   for (i = 0; i < oldinternal.addrc; ++i)
      oldinternal.addrv[i] = sockscf.internal.addrv[i];

   resetconfig(&sockscf, 0);
   genericinit();
   checkconfig();

   showconfig(&sockscf);

#if DIAGNOSTIC
   if (sockscf.monitor != NULL)
      SASSERTX(sockscf.monitor->mstats == NULL);

   if (sockscf.crule != NULL)
      SASSERTX(!SHMID_ISATTACHED(sockscf.crule));

   if (sockscf.srule != NULL)
      SASSERTX(!SHMID_ISATTACHED(sockscf.srule));
#endif /* DIAGNOSTIC */

   for (i = 0; i < oldinternal.addrc; ++i) {
      ssize_t p;

      p = addrindex_on_listenlist(sockscf.internal.addrc,
                                  sockscf.internal.addrv,
                                  &oldinternal.addrv[i].addr,
                                  oldinternal.addrv[i].protocol);

      if (p >= 0) {
         /*
          * this socket/session should continue to exist.
          */
         sockscf.internal.addrv[p].s = oldinternal.addrv[i].s;
         continue;
      }

      /*
       * this socket should be removed.
       */

      if (oldinternal.addrv[i].protocol == SOCKS_TCP) {
         close(oldinternal.addrv[i].s);
         continue;
      }

#if BAREFOOTD
      /* else; udp. */
      slog(LOG_DEBUG, "%s: child should remove udp session for %s",
           function,
           sockaddr2string(&oldinternal.addrv[i].addr, NULL, 0));
#endif /* BAREFOOTD */
   }

#if BAREFOOTD
   if (!ALL_UDP_BOUNCED()) {
      /*
       * Go through all rules and see if the current udp addresses
       * to bind matches any of the old ones so we know which addresses
       * are new and need to be bounced.  Those already bounced we should
       * ignore.
       */
      rule_t *rule;

      /*
       * Assume there are no new addresses to bounce initially.
       */
      sockscf.state.alludpbounced = 1;

      for (rule = sockscf.crule; rule != NULL; rule = rule->next) {
         sockshost_t hosttobind;
         struct sockaddr_storage addrtobind;

         if (!rule->state.protocol.udp)
            continue;

         switch (rule->dst.atype) {
            case SOCKS_ADDR_IPV4:
            case SOCKS_ADDR_IPV6:
               ruleaddr2sockshost(&rule->dst, &hosttobind, SOCKS_UDP);
               sockshost2sockaddr(&hosttobind, &addrtobind);

               if (addrindex_on_listenlist(oldinternal.addrc,
                                           oldinternal.addrv,
                                           &addrtobind,
                                           SOCKS_UDP) != -1) {
                  slog(LOG_DEBUG,
                       "%s: marking address %s in rule %lu as bounced; "
                       "previously bounced",
                       function,
                       sockaddr2string(&addrtobind, NULL, 0),
                       (unsigned long)rule->number);

                  rule->bounced = 1;
               }
               break;

            case SOCKS_ADDR_DOMAIN: {
               size_t i;

               i = 0;
               while (hostname2sockaddr(rule->dst.addr.domain,
                                        i++,
                                        &addrtobind) != NULL) {
                  if (addrindex_on_listenlist(oldinternal.addrc,
                                              oldinternal.addrv,
                                              &addrtobind,
                                              SOCKS_UDP) != -1) {
                     slog(LOG_DEBUG,
                          "%s: marking address %s in rule %lu "
                          "as bounced; previously bounced",
                          function,
                          sockaddr2string(&addrtobind, NULL, 0),
                          (unsigned long)rule->number);

                     rule->bounced = 1;
                     break;
                  }
               }

               break;
            }

            case SOCKS_ADDR_IFNAME: {
               struct sockaddr_storage mask;
               size_t i;

               i = 0;
               while (ifname2sockaddr(rule->dst.addr.ifname,
                                      i++,
                                      &addrtobind,
                                      &mask) != NULL) {
                  if (addrindex_on_listenlist(oldinternal.addrc,
                                              oldinternal.addrv,
                                              &addrtobind,
                                              SOCKS_UDP) != -1) {
                     slog(LOG_DEBUG,
                          "%s: marking address %s in rule %lu "
                          "as bounced; previously bounced",
                          function,
                          sockaddr2string(&addrtobind, NULL, 0),
                         (unsigned long)rule->number);

                     rule->bounced = 1;
                     break;
                  }
               }
               break;
            }

            default:
               SERRX(rule->dst.atype);
         }

         if (!rule->bounced)
            sockscf.state.alludpbounced = 0;
      }
   }
#endif /* BAREFOOTD */

   free(oldinternal.addrv);

   /* may have added addresses in new config, rebind if necessary. */
   if (bindinternal(SOCKS_TCP) != 0)
      serr("%s: failed to bind internal addresses", function);

   /*
    * Now comes the tricky part: copy the config to shared memory and forward
    * the SIGHUP to our children so they know to copy the config we put in
    * shared memory to their own local memory.
    *
    * The procedure works like this:
    * 1) We lock shmemconfigfd, copy the config to shmem (mapped by the
    *    file referenced by shmemconfigfd), and unlock shmemconfigfd.
    *
    * 2) We then forward the children the SIGHUP.
    *
    * 3) When the children receive the SIGHUP, they lock shmemconfigfd,
    *    copy the config, and unlock shmemconfigfd.
    *
    * The only reason for locking shmemconfig is so we do not update it (due
    * to another SIGHUP) while the children try to read it (children lock it
    * read-only, we lock it it read-write).
    */

   socks_lock(sockscf.shmemconfigfd, 0, 0, 1, 1);

   pointersize = pointer_size(&sockscf);

   slog(LOG_DEBUG,
        "%s: current config is of size %lu + %lu (%lu).  Trying to mmap(2) ...",
        function,
        (unsigned long)sizeof(sockscf),
        (unsigned long)pointersize,
        (unsigned long)(sizeof(sockscf) + pointersize));

   if ((newshmemconfig = sockd_mmap(NULL,
                                    sizeof(sockscf) + pointersize,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    sockscf.shmemconfigfd,
                                    1)) == MAP_FAILED) {
      swarn("%s: could not create shared memory segment of size %lu",
            function, (unsigned long)(sizeof(sockscf) + pointersize));

      socks_unlock(sockscf.shmemconfigfd, 0, 0);
      return;
   }

   /*
    * First shallow copy what we can.
    */
   *newshmemconfig = sockscf;

   /*
    * Then the more complicated deep copy.
    */
   if (pointer_copy(&sockscf,
                    0,
                    newshmemconfig,
                    (void *)((uintptr_t)newshmemconfig + sizeof(sockscf)),
                    pointersize) != 0) {

      swarn("%s: could not copy pointers to shared memory", function);

      munmap(newshmemconfig, sizeof(sockscf) + pointersize);

      socks_unlock(sockscf.shmemconfigfd, 0, 0);

      return;
   }

   /*
    * Successfully mapped new config and everything looks ok.  Now remove
    * the old mapping, if any.
    */
   if (sockscf.shmeminfo->config != NULL) {
      rc = munmap(sockscf.shmeminfo->config, sockscf.shmeminfo->configsize);
      SASSERTX(rc == 0);
   }

   sockscf.shmeminfo->config     = newshmemconfig;
   sockscf.shmeminfo->configsize = sizeof(sockscf) + pointersize;

   socks_unlock(sockscf.shmemconfigfd, 0, 0);

   slog(LOG_DEBUG,
        "%s: updated config in shmem.  Total size %lu. Doing compare test ...",
        function, (unsigned long)sockscf.shmeminfo->configsize);

   if ((i = compareconfigs(&sockscf, sockscf.shmeminfo->config)) == 0) {
      swarnx("%s: config in shmem not identical to running config", function);
      return;
   }
   else
      slog(LOG_DEBUG,
           "%s: shmem config identical to running config.  %lu bytes compared",
           function, (unsigned long)i);

   /*
    * Not necessarily necessary, but the config-change could imply we
    * should no longer use (and thus resolve for) ipv4 or ipv6 addresses.
    * Also, might be we have cached something the admin no longer wants us to
    * cache.  Safest to invalidate the cache too at this point.
    */
   hostcacheinvalidate();

#if HAVE_LDAP

   /*
    * LDAP cache entries depend on rule configuration so old cached entries
    * might no longer be valid after a SIGHUP.  Need to invalidate cache
    * at this point.
    */
   ldapcacheinvalid();

#endif /* HAVE_LDAP */

   time_monotonic(&sockscf.stat.configload);

   slog(LOG_INFO,
        "SIGHUP ]: config reloaded.  Broadcasting to children to do the same");

   sigserverbroadcast(sig);
   sigchildbroadcast(sig);

   resetprivileges();

   SIGNAL_EPILOGUE(sig, si, errno_s);
}



/* ARGSUSED */
static void
sigchld(sig, si, sc)
   int sig;
   siginfo_t *si;
   void *sc;
{
   const char *function = "sigchld()";
   const int errno_s = errno;
   time_t tnow;
   pid_t pid;
   int status;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   if (socks_difftime(time_monotonic(&tnow), sockscf.state.firstdeath_time)
   >= (time_t)MAX_ADDCHILD_SECONDS) { /* enough time has passed; reset. */
      sockscf.state.unexpected_deaths = 0;
      sockscf.state.firstdeath_time   = 0;
      sockscf.state.lastdeath_time    = 0;

      enable_childcreate();
   }

   while (1) {
      struct rusage thisrusage;
      sockd_child_t *child;
      int isunexpected, proctype;

      /*
       * On Solaris wait4(2) expects WAIT_ANY to be 0, not -1 as it is
       * on other systems.  Using -1 ends up calling waitid(2) as
       * waitid(P_PGID, 1, ...
       * Not what we want, so use wait3(2) instead, as it does not have
       * that bug.
       */
      if ((pid = wait3(&status, WNOHANG, &thisrusage)) == -1
      && ERRNOISTMP(errno))
         continue;

      if (pid <= 0)
         break;

      slog(LOG_DEBUG, "%s: process %ld exited", function, (long)pid);

      if (pidismother(pid)) {
         sockscf.state.motherpidv[pidismother(pid) - 1] = 0;
         isunexpected = 1;
         proctype     = PROC_MOTHER;
      }
      else {
         /*
          * Must be a regular childprocess.
          * XXX merge motherprocesses into getchild() code.
          */
         struct rusage *rusage, sum;

         if ((child = getchild(pid)) == NULL) {
            /*
             * Note that this might be a pid from our former self also
             * if we failed on an internal error, fork(2)-ed process to
             * get the coredump and continue.  When the fork(2)-ed process
             * exits after generating the coredump, we will receive it's
             * SIGCHLD, but no account of it.  To avoid that, hopefully
             * never happening, problem generating a recursive error, let
             * this be a swarnx(), and not a SWARNX().
             */

            swarnx("%s: unknown child pid %lu exited",
                   function, (unsigned long)pid);

            continue;
         }

         if (child->exitingnormally)
            isunexpected = 0;
         else
            isunexpected = 1;

         proctype = child->type;
         switch (child->type) {
            case PROC_MONITOR:
               rusage = &sockscf.state.rusage_monitor;
               break;

            case PROC_NEGOTIATE:
               rusage = &sockscf.state.rusage_negotiate;
               break;

            case PROC_REQUEST:
               rusage = &sockscf.state.rusage_request;
               break;

            case PROC_IO:
               rusage = &sockscf.state.rusage_io;
               break;

            default:
               SERRX(child->type);
         }

         sum = *rusage;

         timeradd(&rusage->ru_utime, &thisrusage.ru_utime, &sum.ru_utime);
         timeradd(&rusage->ru_stime, &thisrusage.ru_stime, &sum.ru_stime);
         sum.ru_minflt   += thisrusage.ru_minflt;
         sum.ru_majflt   += thisrusage.ru_majflt;
         sum.ru_nswap    += thisrusage.ru_nswap;
         sum.ru_inblock  += thisrusage.ru_inblock;
         sum.ru_oublock  += thisrusage.ru_oublock;
         sum.ru_msgsnd   += thisrusage.ru_msgsnd;
         sum.ru_msgrcv   += thisrusage.ru_msgrcv;
         sum.ru_nsignals += thisrusage.ru_nsignals;
         sum.ru_nvcsw    += thisrusage.ru_nvcsw;
         sum.ru_nivcsw   += thisrusage.ru_nivcsw;

         *rusage = sum;

         slog(LOG_DEBUG, "%s: %s-child %lu exiting after %lds and %lu client%s",
              function,
              childtype2string(proctype),
              (unsigned long)pid,
              (long)socks_difftime(tnow, child->created),
              (unsigned long)child->sentc,
              (unsigned long)child->sentc == 1 ? "" : "s");

         log_rusage(child->type, pid, &thisrusage);

         removechild(pid);
      }

      if (isunexpected) {
         swarnx("%s: %s %lu exited unexpectedly %s %s",
                function,
                childtype2string(proctype),
                (unsigned long)pid,
                WIFSIGNALED(status) ? "on signal" : "",
                WIFSIGNALED(status) ? signal2string(WTERMSIG(status)) : "");

         unexpecteddeath();
      }
   }

   SIGNAL_EPILOGUE(sig, si, errno_s);
}
