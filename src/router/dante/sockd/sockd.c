/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2019,
 *               2020, 2021, 2024
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
"$Id: sockd.c,v 1.925.4.5.2.4.4.10.4.6 2024/12/02 20:46:57 karls Exp $";



#define DOTEST 1

#if DEBUG && DOTEST

static void dotest(void);
/*
 * runs some internal tests.
 */

#endif

static void
serverinit(int argc, char *argv[]);
/*
 * Initializes options/sockscf.  "argc" and "argv" should be
 * the arguments passed to main().
 * Exits on failure.
 */

static void
usage(int code)
      __ATTRIBUTE__((noreturn));
/*
 * print usage.
 */

static void
showversion(const int level);
/*
 * shows version info.
 */

static void
showlicense(void)
            __ATTRIBUTE__((noreturn));
/*
 * shows license and exits.
 */

static void
handlechildcommand(const unsigned int command, sockd_child_t *child,
                   int *finished);
/*
 * Handles childcommand "command", received from child "child".
 * "finished" is set to true if the child has now finished serving
 * requests and should be removed, or false otherwise.
 */

const int socks_configtype = CONFIGTYPE_SERVER;

#if DIAGNOSTIC && HAVE_MALLOC_OPTIONS
extern char *malloc_options;
#endif /* DIAGNOSTIC && HAVE_MALLOC_OPTIONS */

#if HAVE_PROGNAME
extern char *__progname;
#elif SOCKS_SERVER
char *__progname = "sockd";   /* default. */
#elif BAREFOOTD
char *__progname = "barefootd";   /* default. */
#elif COVENANT
char *__progname = "covenantd";   /* default. */
#else
#error "who are we?"
#endif /* HAVE_PROGNAME */

extern char *optarg;

#if !HAVE_SETPROCTITLE
char **argv_cpy;
int argc_cpy;
#endif /* !HAVE_SETPROCTITLE */

#define ELECTRICFENCE   0

#if ELECTRICFENCE
   extern int EF_PROTECT_FREE;
   extern int EF_ALLOW_MALLOC_0;
   extern int EF_ALIGNMENT;
   extern int EF_PROTECT_BELOW;
#endif /* ELECTRICFENCE */

int main(int argc, char *argv[]);

#if !STANDALONE_UNIT_TEST

int
main(argc, argv)
   int   argc;
   char   *argv[];
{
   const char *function = "main()";
   fd_set *rset;
   ssize_t p;
   size_t i;
   sockd_client_t  saved_client;
   sockd_io_t      saved_io;
   sockd_request_t saved_req;
   int have_saved_client = 0,
       have_saved_req    = 0,
       have_saved_io     = 0;

#if DIAGNOSTIC && HAVE_MALLOC_OPTIONS
   malloc_options = "S";
#endif /* DIAGNOSTIC && HAVE_MALLOC_OPTIONS */

#if ELECTRICFENCE
   EF_PROTECT_FREE         = 1;
   EF_ALLOW_MALLOC_0       = 1;
   EF_ALIGNMENT            = 0;
   EF_PROTECT_BELOW        = 0;
#endif /* ELECTRICFENCE */

   runenvcheck();

#if !HAVE_SETPROCTITLE
   argc_cpy = argc;
   if ((argv_cpy = malloc(sizeof(*argv_cpy) * (argc + 1))) == NULL)
      serr("%s: could not allocate %lu bytes of memory",
           function, (unsigned long)(sizeof(*argv_cpy) * (argc + 1)));

   for (i = 0; i < (size_t)argc; i++)
      if ((argv_cpy[i] = strdup(argv[i])) == NULL)
         serr("%s: %s", function, NOMEM);
   argv_cpy[i] = NULL;

   initsetproctitle(argc, argv);

   serverinit(argc_cpy, argv_cpy);

#else

   serverinit(argc, argv);

#endif /* !HAVE_SETPROCTITLE*/

#if DEBUG && DOTEST
   dotest();
#endif

   showconfig(&sockscf);

   if (sockscf.option.verifyonly) {
      resetconfig(&sockscf, 1);
      return 0;
   }

   mother_envsetup(argc, argv);

   /*
    * The monitor-child is special, as there is only one and it
    * is shared/used by all processes, the mother processes
    * included.  It therefor needs to be the first one created,
    * even before the mother processes.
    */
   if (childcheck(PROC_MONITOR) < 1) {
      serr("could not create monitor process: %s",
           sockscf.child.noaddchild_reason != NULL ?
                  sockscf.child.noaddchild_reason
               :  strerror(sockscf.child.noaddchild_errno));
   }


   time_monotonic(&sockscf.stat.boot);

   if (sockscf.option.serverc > 1) {
      /*
       * Temporarily block signals to avoid mixing up signals to us
       * and to child created as well as any races between child receiving
       * signal and child having finished setting up signalhandlers.
       */
      sigset_t all, oldmask;

      (void)sigfillset(&all);
      if (sigprocmask(SIG_SETMASK, &all, &oldmask) != 0)
         swarn("%s: sigprocmask(SIG_SETMASK)", function);


      /* fork of requested number of servers.  Start at one, 'cause we are it */
      for (i = 1; (size_t)i < sockscf.option.serverc; ++i) {
         struct sigaction sigact;
         pid_t pid;

         if ((pid = fork()) == -1)
            swarn("fork()");
         else if (pid == 0) {
            newprocinit();

            bzero(&sigact, sizeof(sigact));
            sigact.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;

            sigact.sa_sigaction = sighup_child;
            if (sigaction(SIGHUP, &sigact, NULL) != 0) {
               swarn("sigaction(SIGHUP)");
               _exit(EXIT_FAILURE);
            }

            sockscf.state.motherpidv[i] = sockscf.state.pid;
            break;
         }
         else {
            slog(LOG_DEBUG, "forked of %s[%lu/%lu] with pid %lu",
                childtype2string(PROC_MOTHER),
                (unsigned long)i + 1,
                (unsigned long)sockscf.option.serverc,
                (unsigned long)pid);

            sockscf.state.motherpidv[i] = pid;
         }
      }

      if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0)
         swarn("%s: sigprocmask(SIG_SETMASK, &oldmask, NULL)", function);
   }

   if (pidismainmother(sockscf.state.pid)) {
      if (sockscf.option.debug)
         sockopts_dump();
   }

   if (childcheck(PROC_NEGOTIATE) < SOCKD_FREESLOTS_NEGOTIATE
   ||  childcheck(PROC_REQUEST)   < SOCKD_FREESLOTS_REQUEST
   ||  childcheck(PROC_IO)        < SOCKD_FREESLOTS_IO)
      serr("initial childcheck() failed: %s",
           sockscf.child.noaddchild_reason != NULL ?
                  sockscf.child.noaddchild_reason
               :  strerror(sockscf.child.noaddchild_errno));

#if HAVE_PROFILING && HAVE_MONCONTROL /* XXX is this only needed on Linux? */
moncontrol(1);
#endif /* HAVE_PROFILING && HAVE_MONCONTROL*/

   if (sockscf.option.debug)
      slog(LOG_DEBUG, "%s", mother_getlimitinfo());

#if PRERELEASE
   slog(LOG_INFO, "\n"
   "   ******************************************************************\n"
#if BAREFOOTD
   "   *** Thank you for testing this %s pre-release.          ***\n"
#elif COVENANT
   "   *** Thank you for testing this %s pre-release.           ***\n"
#elif SOCKS_SERVER
   "   *** Thank you for testing this %s pre-release.              ***\n"
#else
#error "hmm, who are we?"
#endif /* SOCKS_SERVER */
   "   *** Please note pre-releases are always configured in a way    ***\n"
   "   *** that puts a considerably larger load on the running system ***\n"
   "   *** system than the standard releases.                         ***\n"
   "   *** This is to help simulate high-load situations and aid in   ***\n"
   "   *** finding bugs before a full release is done.                ***\n"
   "   ******************************************************************",
   PRODUCT);
#endif /* PRERELEASE */

   rset = allocate_maxsize_fdset();

   slog(LOG_INFO, "%s/server[%d/%d] v%s running\n",
        PRODUCT,
        (int)pidismother(sockscf.state.pid),
        (int)sockscf.option.serverc,
        VERSION);

   /*
    * main loop; accept new connections and handle our children.
    * CONSTCOND
    */
   while (1) {
#if HAVE_NEGOTIATE_PHASE
      response_t response;
#endif /* HAVE_NEGOTIATE_PHASE */
      size_t free_negc, free_reqc, free_ioc;
      sockd_child_t *child;
      struct timeval *timeout = NULL, zerotimeout  = { 0, 0 };
      int rbits;

      errno = 0; /* reset for each iteration. */
      rbits = fillset(rset, &free_negc, &free_reqc, &free_ioc);

      if (free_negc  < SOCKD_FREESLOTS_NEGOTIATE
      ||  free_reqc  < SOCKD_FREESLOTS_REQUEST
      ||  free_ioc   < SOCKD_FREESLOTS_IO) {
         static time_t lastloggedtime;
         time_t tnow;

         if (socks_difftime(time_monotonic(&tnow), lastloggedtime) > 10) {
            slog(LOG_WARNING,
                 "need to add a new child process to handle client load, but "
                 "unable to do so at the moment%s%s (%s)",
                 sockscf.child.noaddchild_reason == NULL ? "" : ": ",
                 sockscf.child.noaddchild_reason == NULL ?
                     "" : sockscf.child.noaddchild_reason,
                 strerror(sockscf.child.noaddchild_errno));

            lastloggedtime = tnow;
         }
      }

#if BAREFOOTD
      if (!ALL_UDP_BOUNCED()) {
         slog(LOG_DEBUG,
              "have not bounced all udp sessions yet, setting timeout to 0");

         timeout = &zerotimeout;
      }
#endif /* BAREFOOTD */

      if (have_saved_client || have_saved_req || have_saved_io) {
         slog(LOG_DEBUG,
              "have previously unsent clientobjects; setting timeout to zero");

         timeout = &zerotimeout;
      }

      slog(LOG_DEBUG, "calling select().  Free negc: %lu, reqc: %lu, ioc: %lu",
           (unsigned long)free_negc,
           (unsigned long)free_reqc,
           (unsigned long)free_ioc);

      p = selectn(++rbits,
                  rset,
                  NULL,
                  NULL,
                  NULL,
                  NULL,
                  timeout);

      slog(LOG_DEBUG, "%s: selectn() returned %d (%s)",
           function, (int)p, strerror(errno));

      if (p == -1) {
         SASSERT(ERRNOISTMP(errno));
         continue;
      }

      /*
       * First get standalone ack of free slots for requests that did not
       * require the children to send us any new client objects.
       */
      while ((child = getset(ACKPIPE, rset)) != NULL) {
         unsigned char command;
         int childisbad = 0, childhasfinished = 0;

         errno = 0;
         p     = socks_recvfromn(child->ack,
                                 &command,
                                 sizeof(command),
                                 0,
                                 0,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);
         clearset(ACKPIPE, child, rset);

         if (p != sizeof(command)) {
            switch (p) {
               case -1:
                  swarn("socks_recvfrom(child->ack) from %s %ld failed",
                        childtype2string(child->type), (long)child->pid);
                  break;

               case 0:
                  swarnx("EOF from %s %ld",
                         childtype2string(child->type), (long)child->pid);
                  break;

               default:
                  swarnx("unexpected byte count from %s %ld.  "
                         "Expected %lu, got %lu",
                         childtype2string(child->type), (long)child->pid,
                         (unsigned long)sizeof(command), (unsigned long)p);
            }

            childisbad = 1;
         }
         else
            handlechildcommand(command, child, &childhasfinished);

         if (childhasfinished || childisbad) {
            closechild(child->pid, childhasfinished ? 1 : 0);

           /*
            * Can no longer be sure we have any free slots to handle
            * new clients accept(2)-ed, so break out restart the loop so
            * we can recalculate.
            */
           break;
         }
      }

      /*
       * Check if we have any client objects previously received but
       * which we failed to send to a child at that time due to temporary
       * resource shortage.  Not that this is not related to a child not
       * having slots available, but only to a temporal failure, such
       * as the child not having yet drained the socket it receives
       * new objects on.
       */
      if (have_saved_client) {
         if ((child = nextchild(PROC_NEGOTIATE, SOCKS_TCP)) == NULL) {
            log_clientdropped(&saved_client.from);

            close(saved_client.s);
            continue;
         }

         SASSERTX(child != NULL);
         SASSERTX(child->freec > 0);

         log_clientsend(&saved_client.from, child, 1);
         p = send_client(child->s, &saved_client, NULL, 0);

         if (p == 0) {
            --child->freec;
            ++child->sentc;
            ++sockscf.stat.negotiate.sendt;

            close(saved_client.s);
            have_saved_client = 0;
         }
         else {
            log_sendfailed(&saved_client.from, saved_client.s, child, 0);

            if (!ERRNOISTMP(errno)) {
               close(saved_client.s);
               have_saved_client = 0;
            }
         }
      }

      if (have_saved_req && free_reqc > 0) {
         child = nextchild(PROC_REQUEST, SOCKS_TCP);

         SASSERTX(child != NULL);
         SASSERTX(child->freec > 0);

         log_clientsend(&saved_req.from, child, 1);

         if (send_req(child->s, &saved_req) == 0) {
            --free_reqc;
            --child->freec;
            ++child->sentc;
            ++sockscf.stat.request.sendt;

            close(saved_req.s);
            have_saved_req = 0;
         }
         else {
            log_sendfailed(&saved_req.from, saved_req.s, child, 0);

            if (!ERRNOISTMP(errno)) {
               clientinfo_t cinfo;

               if (SHMID_ISSET(CRULE_OR_HRULE(&saved_req))) {
                  cinfo.from   = saved_req.from;
                  cinfo.hostid = saved_req.state.hostid;
               }

               if (socketisconnected(saved_req.s, NULL, 0) == NULL) {
                  if (CRULE_OR_HRULE(&saved_req)->mstats_shmid != 0
                  && (CRULE_OR_HRULE(&saved_req)->alarmsconfigured
                      & ALARM_DISCONNECT))
                        alarm_add_disconnect(0,
                                             CRULE_OR_HRULE(&saved_req),
                                             ALARM_INTERNAL,
                                             &cinfo,
                                             strerror(errno),
                                             sockscf.shmemfd);
               }
               else { /* error with child? */
#if HAVE_NEGOTIATE_PHASE
                  create_response(NULL,
                                  &saved_req.cauth,
                                  saved_req.req.version,
                                  (int)errno2reply(errno,
                                                   saved_req.req.version),
                                  &response);

                  if (send_response(saved_req.s, &response) != 0) {
                     slog(LOG_DEBUG,
                          "%s: send_response(%d) to %s failed: %s",
                          function,
                          saved_req.s,
                          sockaddr2string(&saved_req.from, NULL, 0),
                          strerror(errno));
                  }
#endif /* HAVE_NEGOTIATE_PHASE */
               }

               if (SHMID_ISSET(CRULE_OR_HRULE(&saved_req)))
                  SHMEM_UNUSE(CRULE_OR_HRULE(&saved_req),
                              &cinfo,
                              sockscf.shmemfd,
                              SHMEM_ALL);

               close(saved_req.s);
               have_saved_req = 0;
            }
         }
      }

      if (have_saved_io && free_ioc > 0) {
         child = nextchild(PROC_IO, saved_io.state.protocol);
         SASSERTX(child != NULL);
         SASSERTX(child->freec > 0);

#if BAREFOOTD
         if (saved_io.state.protocol == SOCKS_UDP)
            SASSERTX(child->hasudpsession == 0);
#endif /* BAREFOOTD */

         log_clientsend(&INTERNALIO(&saved_io)->raddr, child, 1);
         if (send_io(child->s, &saved_io) == 0) {
#if BAREFOOTD
            if (saved_io.state.protocol == SOCKS_UDP) {
               SASSERTX(child->hasudpsession == 0);
               ++child->hasudpsession;

               slog(LOG_DEBUG,
                    "sent udp session for local address %s to %s %ld",
                    sockaddr2string(&saved_io.src.laddr, NULL, 0),
                    childtype2string(child->type),
                    (long)child->pid);
            }
#endif /* BAREFOOTD */

            --free_ioc;
            --child->freec;
            ++child->sentc;
            ++sockscf.stat.io.sendt;

            close_iodescriptors(&saved_io);
            have_saved_io = 0;
         }
         else {
            log_sendfailed(&CONTROLIO(&saved_io)->raddr,
                           CONTROLIO(&saved_io)->s,
                           child,
                           0);

            if (!ERRNOISTMP(errno)) {
               clientinfo_t cinfo;

               if (SHMID_ISSET(&saved_io.srule)) {
                  cinfo.from   = CONTROLIO(&saved_io)->raddr;
                  cinfo.hostid = saved_io.state.hostid;
               }

               if (socketisconnected(CONTROLIO(&saved_io)->s, NULL, 0) == NULL){
                  if (saved_io.srule.mstats_shmid != 0
                  && (saved_io.srule.alarmsconfigured & ALARM_DISCONNECT))
                     alarm_add_disconnect(0,
                                          &saved_io.srule,
                                          ALARM_INTERNAL,
                                          &cinfo,
                                          strerror(errno),
                                          sockscf.shmemfd);
               }
               else { /* error with target socket? */
                  if (saved_io.state.command != SOCKS_UDPASSOCIATE
                  &&  saved_io.dst.s         != -1
                  &&  !socketisconnected(saved_io.dst.s, NULL, 0)) {
                     if (saved_io.srule.mstats_shmid != 0
                     && (saved_io.srule.alarmsconfigured & ALARM_DISCONNECT))
                        alarm_add_disconnect(0,
                                             &saved_io.srule,
                                             ALARM_EXTERNAL,
                                             &cinfo,
                                             strerror(errno),
                                             sockscf.shmemfd);
                  }

#if HAVE_NEGOTIATE_PHASE
                  create_response(NULL,
                                  &saved_io.src.auth,
                                  saved_io.state.proxyprotocol,
                                  (int)errno2reply(errno,
                                                  saved_io.state.proxyprotocol),
                                  &response);

                  if (send_response(CONTROLIO(&saved_io)->s, &response) != 0) {
                     slog(LOG_DEBUG,
                          "%s: send_response(%d) to %s failed: %s",
                          function,
                          CONTROLIO(&saved_io)->s,
                          sockshost2string(&CONTROLIO(&saved_io)->host,
                                           NULL,
                                           0),
                          strerror(errno));
                  }
#endif /* HAVE_NEGOTIATE_PHASE */
               }

               if (SHMID_ISSET(&saved_io.srule))
                  SHMEM_UNUSE(&saved_io.srule,
                              &cinfo,
                              sockscf.shmemfd,
                              SHMEM_ALL);

               close_iodescriptors(&saved_io);
               have_saved_io = 0;
            }
         }
      }

      /*
       * Next, get new client objects from the children.
       * Don't try get more requests than we've calculated we can handle at
       * the start, or we could end up needlessly forking a lot of new
       * processes, while at the same time having a lot of unread
       * SOCKD_FREESLOT messages pending.
       */

#define HAVE_FREE_SLOTS() ((free_negc > 0) || (free_reqc > 0) || (free_ioc > 0))

      while (HAVE_FREE_SLOTS() && (child = getset(DATAPIPE, rset)) != NULL) {
         /*
          * Can not be a pointer as nextchild() may need to create a new
          * child, and that may cause the memory used to hold all our
          * childrens to be realloc(3)-ed, changing what the childpointer
          * returned above points to.
          */
         const sockd_child_t fromchild = *child;
         int childhasfinished;

         if (sockd_handledsignals())
            /* some child could have been removed from rset.  Can't use it. */
            break;

         if (sockscf.option.debug >= DEBUG_VERBOSE)
            slog(LOG_DEBUG,
                 "free negc = %lu, reqc = %lu, ioc = %lu.  Next child: %s %d",
                 (unsigned long)free_negc,
                 (unsigned long)free_reqc,
                 (unsigned long)free_ioc,
                 childtype2string(fromchild.type),
                 (int)fromchild.pid);

         clearset(DATAPIPE, &fromchild, rset);
         errno = 0;

         switch (fromchild.type) {
            /*
             * in the order a packet travels between children;
             * negotiate -> request -> io
             * (and in Covenants case, -> io -> negotiate again, sometimes).
             */

            case PROC_NEGOTIATE: {
               sockd_request_t req;

               if (free_reqc <= 0) {
                  clearchildtype(fromchild.type, DATAPIPE, rbits, rset);
                  continue;
               }

               if (have_saved_req) {
                  log_noclientrecv(&fromchild);
                  continue;
               }

               if ((child = nextchild(PROC_REQUEST, SOCKS_TCP)) == NULL) {
                  slog(LOG_DEBUG, "no request slot available for new client");
                  free_reqc = 0;
                  continue;
               }

               SASSERTX(child->freec > 0);

               slog(LOG_DEBUG, "trying to receive request from %s %ld",
                    childtype2string(fromchild.type), (long)fromchild.pid);

               if ((p = recv_req(fromchild.s, &req)) != 0) {
                  slog(LOG_DEBUG, "recv_req() on fd %d failed with %ld: %s",
                       fromchild.s, (long)p, strerror(errno));

                  continue;
               }

               ++sockscf.stat.negotiate.received;

               handlechildcommand(req.reqinfo.command,
                                  getchild(fromchild.pid),
                                  &childhasfinished);

               log_clientsend(&req.from, child, 0);

               if (send_req(child->s, &req) == 0) {
                  --free_reqc;
                  --child->freec;
                  ++child->sentc;
                  ++sockscf.stat.request.sendt;

                  close(req.s);
                  break;
               }

               log_sendfailed(&req.from, req.s, child, 1);

               if (ERRNOISTMP(errno)) {
                  saved_req      = req;
                  have_saved_req = 1;
               }
               else {
                  clientinfo_t cinfo;

                  if (SHMID_ISSET(CRULE_OR_HRULE(&req))) {
                     cinfo.from   = req.from;
                     cinfo.hostid = req.state.hostid;
                  }

                  if (!socketisconnected(req.s, NULL, 0)) {
                     if (CRULE_OR_HRULE(&saved_req)->mstats_shmid != 0
                     && (CRULE_OR_HRULE(&saved_req)->alarmsconfigured
                         & ALARM_DISCONNECT))
                        alarm_add_disconnect(0,
                                             CRULE_OR_HRULE(&req),
                                             ALARM_INTERNAL,
                                             &cinfo,
                                             strerror(errno),
                                             sockscf.shmemfd);
                  }
                  else {
#if HAVE_NEGOTIATE_PHASE
                     create_response(NULL,
                                     &req.cauth,
                                     req.req.version,
                                     errno2reply(errno, req.req.version),
                                     &response);

                     if (send_response(req.s, &response) != 0) {
                        slog(LOG_DEBUG,
                             "%s: send_response(%d) to %s failed: %s",
                             function,
                             req.s,
                             sockaddr2string(&req.from, NULL, 0),
                             strerror(errno));
                     }
#endif /* HAVE_NEGOTIATE_PHASE */
                  }

                  close(req.s);

                  if (SHMID_ISSET(CRULE_OR_HRULE(&req)))
                     SHMEM_UNUSE(CRULE_OR_HRULE(&req),
                                 &cinfo,
                                 sockscf.shmemfd,
                                 SHMEM_ALL);
               }

               continue;
            }

            case PROC_REQUEST: {
               sockd_io_t io;
               pid_t pid_tcp, pid_udp;

               if (free_ioc <= 0) {
                  clearchildtype(fromchild.type, DATAPIPE, rbits, rset);
                  continue;
               }

               if (have_saved_io) {
                  log_noclientrecv(&fromchild);
                  continue;
               }

               if ((child = nextchild(PROC_IO, SOCKS_TCP)) == NULL) {
                  slog(LOG_DEBUG, "no tcp io slot available for new client");
                  free_ioc = 0;
                  continue;
               }

               pid_tcp = child->pid;

#if BAREFOOTD
               /*
                * don't know which protocol the request we receive is for
                * until we receive it, so make sure we have space for
                * either possibility, as in Barefoot one i/o child can
                * only handle one (multiplexed) udp session.
                */
               if ((child = nextchild(PROC_IO, SOCKS_UDP)) == NULL) {
                  slog(LOG_DEBUG, "no udp io slot available for new client");
                  free_ioc = 0;
                  continue;
               }

               SASSERTX(child->hasudpsession == 0);

               pid_udp = child->pid;

#else /* !BAREFOOTD */
               /* any child with a free slot can handle a udp session. */
               pid_udp = pid_tcp;
#endif /* !BAREFOOTD */

               slog(LOG_DEBUG, "trying to receive request from %s %ld",
                    childtype2string(fromchild.type), (long)fromchild.pid);

               if ((p = recv_io(fromchild.s, &io)) != 0) {
                  slog(LOG_DEBUG, "recv_io() on fd %d failed with %ld: %s",
                       fromchild.s, (long)p, strerror(errno));

                  continue;
               }

               ++sockscf.stat.request.received;

               handlechildcommand(io.reqinfo.command,
                                  getchild(fromchild.pid),
                                  &childhasfinished);

               switch (io.state.protocol) {
                  case SOCKS_TCP:
                     child = getchild(pid_tcp);
                     break;

                  case SOCKS_UDP:
                     child = getchild(pid_udp);
                     break;

                  default:
                     SWARNX(io.state.protocol);
                     continue;
               }

               SASSERTX(child != NULL);
               SASSERTX(child->freec > 0);

               log_clientsend(&CONTROLIO(&io)->raddr, child, 0);
               if (send_io(child->s, &io) == 0) {
                  --free_ioc;
                  --child->freec;
                  ++child->sentc;
                  ++sockscf.stat.io.sendt;

                  SASSERTX(child->freec  < maxfreeslots(child->type));
                  close_iodescriptors(&io);
#if BAREFOOTD
                  if (io.state.protocol == SOCKS_UDP) {
                     SASSERTX(child->hasudpsession == 0);
                     ++child->hasudpsession;

                     slog(LOG_DEBUG,
                          "sent udp session for local address %s to %s %ld",
                         sockaddr2string(&io.src.laddr, NULL, 0),
                          childtype2string(child->type),
                          (long)child->pid);
                  }
#endif /* BAREFOOTD */

                  break;
               }

               log_sendfailed(&CONTROLIO(&io)->raddr,
                              CONTROLIO(&io)->s,
                              child,
                              0);

               if (ERRNOISTMP(errno)) {
                  saved_io      = io;
                  have_saved_io = 1;
               }
               else {
                  clientinfo_t cinfo;
                  if (SHMID_ISSET(&io.srule)) {
                     cinfo.from   = CONTROLIO(&io)->raddr;
                     cinfo.hostid = io.state.hostid;
                  }

                  if (socketisconnected(CONTROLIO(&io)->s, NULL, 0) == NULL) {
                     if (io.srule.mstats_shmid != 0
                     && (io.srule.alarmsconfigured & ALARM_DISCONNECT))
                        alarm_add_disconnect(0,
                                             &io.srule,
                                             ALARM_INTERNAL,
                                             &cinfo,
                                             strerror(errno),
                                             sockscf.shmemfd);
                  }
                  else {
                     if (io.state.command != SOCKS_UDPASSOCIATE
                     &&  io.dst.s         != -1
                     && socketisconnected(io.dst.s, NULL, 0)) {
                        if (io.srule.mstats_shmid != 0
                        && (io.srule.alarmsconfigured & ALARM_DISCONNECT))
                           alarm_add_disconnect(0,
                                                &io.srule,
                                                ALARM_EXTERNAL,
                                                &cinfo,
                                                strerror(errno),
                                                sockscf.shmemfd);
                     }

#if HAVE_NEGOTIATE_PHASE
                     create_response(NULL,
                                     &io.src.auth,
                                     io.state.proxyprotocol,
                                     (int)errno2reply(errno,
                                                      io.state.proxyprotocol),
                                     &response);

                     if (send_response(CONTROLIO(&io)->s, &response) != 0) {
                        slog(LOG_DEBUG,
                             "%s: send_response(%d) to %s failed: %s",
                             function,
                             CONTROLIO(&io)->s,
                             sockshost2string(&io.src.host, NULL, 0),
                             strerror(errno));
                     }
#endif /* HAVE_NEGOTIATE_PHASE */
                  }

                  close_iodescriptors(&io);

                  if (SHMID_ISSET(&io.srule))
                     SHMEM_UNUSE(&io.srule, &cinfo, sockscf.shmemfd, SHMEM_ALL);
               }

               continue;
            }

            case PROC_IO: {
#if COVENANT
               sockd_client_t client;

               if (free_negc <= 0) {
                  clearchildtype(fromchild.type, DATAPIPE, rbits, rset);
                  continue;
               }

               if (have_saved_client) {
                  log_noclientrecv(&fromchild);
                  continue;
               }

               if ((child = nextchild(PROC_NEGOTIATE, SOCKS_TCP)) == NULL) {
                  slog(LOG_DEBUG, "no %s available to accept old client",
                       childtype2string(fromchild.type));

                  continue;
               }

               SASSERTX(child->freec > 0);

               slog(LOG_DEBUG, "trying to receive request from %s %ld",
                    childtype2string(fromchild.type),
                    (long)fromchild.pid);

               if ((p = recv_resentclient(fromchild.s, &client)) != 0) {
                  slog(LOG_DEBUG,
                       "recv_resentclient() on fd %d failed with %ld: %s",
                       fromchild.s, (long)p, strerror(errno));

                  continue;
               }

               ++sockscf.stat.io.received;

               handlechildcommand(client.reqinfo.command,
                                  getchild(fromchild.pid),
                                  &childhasfinished);


               log_clientsend(&client.from, child, 0);
               p = send_client(child->s, &client, NULL, 0);

               if (p == 0) {
                  --child->freec;
                  ++child->sentc;
                  ++sockscf.stat.negotiate.sendt;

                  close(client.s);
                  break;
               }

               log_sendfailed(&client.from, client.s, child, 1);

               if (ERRNOISTMP(errno)) {
                  saved_client      = client;
                  have_saved_client = 1;
               }
               else {
                  /*
                   * XXX
                   * Should have done a monitormatch() here, but want to
                   * minimize the amount of code in this process.
                   */

                  if (socketisconnected(client.s, NULL, 0) != NULL) {
#if HAVE_NEGOTIATE_PHASE
                     /* XXX missing stuff here. */
                     create_response(NULL,
                                     &client.auth,
                                     client.request.version,
                                     errno2reply(errno, client.request.version),
                                     &response);

                     if (send_response(client.s, &response) != 0) {
                        slog(LOG_DEBUG,
                             "%s: send_response(%d) to %s failed: %s",
                             function,
                             client.s,
                             sockshost2string(&client.request.host, NULL, 0),
                             strerror(errno));
                     }
#endif /* HAVE_NEGOTIATE_PHASE */
                  }

                  close(client.s);
               }
#endif /* COVENANT */

               continue;
            }

            default:
               SERRX(fromchild.type);
         }

         if (childhasfinished)
            closechild(fromchild.pid, 1);
      }

      /*
       * handled our children.  Is there a new connection pending now?
       */
      for (i = 0; i < sockscf.internal.addrc && !have_saved_client; ++i) {
         char astr[MAXSOCKADDRSTRING];
         sockd_client_t client;

         if (sockd_handledsignals())
            break; /* don't know what happened; restart loop. */

#if BAREFOOTD
         if (sockscf.internal.addrv[i].protocol != SOCKS_TCP)
            continue; /* udp handled by io children. */
#endif /* BAREFOOTD */

         /* clear client to silence valgrind */
         bzero(&client, sizeof(client));

         if (FD_ISSET(sockscf.internal.addrv[i].s, rset)) {
            /*
             * Run until there are no more clients pending on the socket,
             * to somewhat reduce the chance of the listen queue filling
             * up before we have time to shrink it.
             */
            do {
               socklen_t len;
               int nomoreclients = 0;

               len       = sizeof(client.from);

               client.s  = acceptn(sockscf.internal.addrv[i].s,
                                   &client.from,
                                   &len);

               client.to = sockscf.internal.addrv[i].addr;

               if (client.s  == -1) {
                  switch (errno) {
#ifdef EPROTO
                     case EPROTO:         /* overloaded SVR4 error */
#endif /* EPROTO */
                     case EWOULDBLOCK:    /* BSD   */
                     case ENOBUFS:        /* HPUX  */
                     case ECONNABORTED:   /* POSIX */

                     case ECONNRESET:
#ifdef ETIMEDOUT
                     case ETIMEDOUT:
#endif /* ETIMEDOUT */
                     case EHOSTUNREACH:
                     case ENETUNREACH:
                     case ENETDOWN:
                     case EPERM: /* linux craziness. */
                        if (ERRNOISTMP(errno))
                           nomoreclients = 1;
                        else
                           slog(LOG_DEBUG, "accept(2) of new client failed: %s",
                                strerror(errno));
                        break;

                     case ENFILE:
                     case EMFILE:
                        swarn("could not accept new client");
                        break;

                     case EBADF:
                     case EFAULT:
                     case EINVAL:
                        SERRX(errno);

                     default:
                        if (ERRNOISTMP(errno)) {
                           nomoreclients = 1;
                           break;
                        }

                        SWARN(client.s);
                  }

                  if (nomoreclients)
                     break;
                  else
                     continue; /* check if there are more connections pending */
               }

               if (IPADDRISBOUND(&sockscf.internal.addrv[i].addr))
                  client.to = sockscf.internal.addrv[i].addr;
               else {
                  /*
                   * wildcard address.  Don't know what address client was
                   * accepted on.
                   */
                  socklen_t len = sizeof(client.to);

                  if (getsockname(client.s, TOSA(&client.to), &len) != 0) {
                     slog(LOG_DEBUG, "getsockname(2) failed after accept(2)");
                     continue; /* check if there are more connections pending */
                  }
               }

               gettimeofday_monotonic(&client.accepted);
               ++sockscf.stat.accepted;

               slog(LOG_DEBUG, "accepted tcp client %s on address %s, fd %d",
                    sockaddr2string(&client.from, astr, sizeof(astr)),
                    sockaddr2string(&sockscf.internal.addrv[i].addr, NULL, 0),
                    sockscf.internal.addrv[i].s);

               if ((child = nextchild(PROC_NEGOTIATE, SOCKS_TCP)) == NULL) {
                  log_clientdropped(&client.from);

                  close(client.s);
                  continue;
               }

               log_clientsend(&client.from, child, 0);

               p = send_client(child->s, &client, NULL, 0);

               if (p == 0) {
                  --free_negc;
                  --child->freec;
                  ++child->sentc;
                  ++sockscf.stat.negotiate.sendt;

                  close(client.s);
               }
               else {
                  log_sendfailed(&client.from, client.s, child, 1);

                  if (ERRNOISTMP(errno)) {
                     saved_client      = client;
                     have_saved_client = 1;
                  }
                  else
                     close(client.s);

                  break;
               }
            } while (1);
         }
      }
   }

   /* NOTREACHED */
}
#endif /* STANDALONE_UNIT_TEST  */


static void
usage(code)
   int code;
{

   (void)fprintf(code == 0 ? stdout : stderr,
"%s v%s.  Copyright (c) 1997 - 2024, Inferno Nettverk A/S, Norway.\n"
"usage: %s [-DLNVdfhnv]\n"
"   -D             : run in daemon mode\n"
"   -L             : shows the license for this program\n"
"   -N <number>    : fork of <number> servers [1]\n"
"   -V             : verify configuration and exit\n"
"   -d <number>    : set degree of debugging\n"
"   -f <filename>  : use <filename> as configuration file [%s]\n"
"   -h             : print this information\n"
"   -n             : disable TCP keep-alive\n"
"   -p <filename>  : write pid to <filename> [%s]\n"
"   -v             : print version info\n",
                 PRODUCT,
                 VERSION,
                __progname,
                SOCKD_CONFIGFILE,
                SOCKD_PIDFILE);

   exit(code);
}


extern const licensekey_t module_redirect_keyv[];
extern const size_t       module_redirect_keyc;
extern const char         module_redirect_version[];

extern const licensekey_t module_bandwidth_keyv[];
extern const size_t       module_bandwidth_keyc;
extern const char         module_bandwidth_version[];

#if HAVE_LDAP
extern const licensekey_t module_ldap_keyv[];
extern const size_t       module_ldap_keyc;
extern const char         module_ldap_version[];
#endif /* HAVE_LDAP */

#if HAVE_PAC
extern const licensekey_t module_pac_keyv[];
extern const size_t       module_pac_keyc;
extern const char         module_pac_version[];
#endif /* HAVE_PAC */

static void
showversion(level)
   const int level;
{
   struct {
      const char         *name;

      const licensekey_t *keyv;
      const size_t       keyc;
   } licenseinfov[] = {
                           {  "Bandwidth",
                              module_bandwidth_keyv,
                              module_bandwidth_keyc
                           },

#if HAVE_LDAP
                           {  "LDAP",
                              module_ldap_keyv,
                              module_ldap_keyc
                           },
#endif /* HAVE_LDAP */

#if HAVE_PAC
                           {  "PAC",
                              module_pac_keyv,
                              module_pac_keyc
                           },
#endif /* HAVE_PAC */

                           {  "Redirect",
                              module_redirect_keyv,
                              module_redirect_keyc
                           }
                        };
   const size_t licenseinfoc = ELEMENTS(licenseinfov);

   size_t i;

   printf("%s v%s.  Copyright (c) 1997 - 2024 Inferno Nettverk A/S, Norway\n",
          PRODUCT, VERSION);

   for (i = 0; i < licenseinfoc; ++i) {
      if (licenseinfov[i].keyc > 0) {
         size_t keyc;

         printf("%lu address%s licensed for the %s %s module:\n",
                (unsigned long)licenseinfov[i].keyc,
                (unsigned long)licenseinfov[i].keyc == 1 ? "" : "es",
                PRODUCT,
                licenseinfov[i].name);

         for (keyc = 0; keyc < licenseinfov[i].keyc; ++keyc)
            printf("address #%-5lu: %s\n",
                   (unsigned long)(keyc + 1),
                   licensekey2string(&licenseinfov[i].keyv[keyc]));
      }
   }

   if (level > 1) {
      if (strlen(DANTE_BUILD) > 0)
         printf("build: %s\n", DANTE_BUILD);

      if (strlen(DANTE_SOCKOPTS_SO) > 0)
         printf("socket options (socket level): %s\n", DANTE_SOCKOPTS_SO);

      if (strlen(DANTE_SOCKOPTS_IPV4) > 0)
         printf("socket options (ipv4 level): %s\n", DANTE_SOCKOPTS_IPV4);

      if (strlen(DANTE_SOCKOPTS_IPV6) > 0)
         printf("socket options (ipv6 level): %s\n", DANTE_SOCKOPTS_IPV6);

      if (strlen(DANTE_SOCKOPTS_TCP) > 0)
         printf("socket options (tcp level): %s\n", DANTE_SOCKOPTS_TCP);

      if (strlen(DANTE_SOCKOPTS_UDP) > 0)
         printf("socket options (udp level): %s\n", DANTE_SOCKOPTS_UDP);

      if (strlen(DANTE_COMPATFILES) > 0)
         printf("compat: %s\n", DANTE_COMPATFILES);
   }
}

static void
showlicense(void)
{

   printf("%s: %s v%s\n%s\n", __progname, PRODUCT, VERSION,
"\
/*\n\
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,\n\
 *               2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,\n\
 *               2017, 2018, 2019, 2020, 2021, 2024\n\
 *      Inferno Nettverk A/S, Norway.  All rights reserved.\n\
 *\n\
 * Redistribution and use in source and binary forms, with or without\n\
 * modification, are permitted provided that the following conditions\n\
 * are met:\n\
 * 1. The above copyright notice, this list of conditions and the following\n\
 *    disclaimer must appear in all copies of the software, derivative works\n\
 *    or modified versions, and any portions thereof, aswell as in all\n\
 *    supporting documentation.\n\
 * 2. All advertising materials mentioning features or use of this software\n\
 *    must display the following acknowledgement:\n\
 *      This product includes software developed by\n\
 *      Inferno Nettverk A/S, Norway.\n\
 * 3. The name of the author may not be used to endorse or promote products\n\
 *    derived from this software without specific prior written permission.\n\
 *\n\
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n\
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n\
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. \n\
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,\n\
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n\
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n\
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n\
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT \n\
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF\n\
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\
 *\n\
 * Inferno Nettverk A/S requests users of this software to return to\n\
 * \n\
 *  Software Distribution Coordinator  or  sdc@inet.no\n\
 *  Inferno Nettverk A/S\n\
 *  Oslo Research Park\n\
 *  Gaustadalleen 21\n\
 *  NO-0349 Oslo\n\
 *  Norway\n\
 * \n\
 * any improvements or extensions that they make and grant Inferno Nettverk A/S\n\
 * the rights to redistribute these changes.\n\
 *\n\
 */");

   exit(EXIT_SUCCESS);
}

static void
serverinit(argc, argv)
   int argc;
   char *argv[];
{
   const char *function = "serverinit()";
   size_t i;
   int ch;

#if !HAVE_PROGNAME
   if (argv[0] != NULL) {
      if ((__progname = strrchr(argv[0], '/')) == NULL)
         __progname = argv[0];
      else
         ++__progname;
   }
#endif /* !HAVE_PROGNAME */

#if !HAVE_PRIVILEGES
   sockscf.state.euid = sockscf.initial.euid = geteuid();
   sockscf.state.egid = sockscf.initial.egid = getegid();
#endif /* !HAVE_PRIVILEGES */

   sockscf.state.type     = PROC_MOTHER;
   sockscf.option.serverc = 1;   /* ourselves. ;-) */

   if ((sockscf.state.pagesize = sysconf(_SC_PAGESIZE)) == -1)
      serr("%s: could not get the pagesize via sysconf(SC_PAGESIZE)", function);

   sockscf.hostfd         = -1;
   sockscf.loglock        = -1;
   sockscf.shmemconfigfd  = -1;
   sockscf.shmemfd        = -1;

   for (i = 0; i < ELEMENTS(sockscf.state.reservedfdv); ++i)
      sockscf.state.reservedfdv[i] = -1;

   while ((ch = getopt(argc, argv, "DLN:Vd:f:hnp:v")) != -1) {
      switch (ch) {
         case 'D':
            sockscf.option.daemon = 1;
            break;

         case 'L':
            showlicense();
            /* NOTREACHED */

         case 'N': {
            char *endptr;

            if ((sockscf.option.serverc = (int)strtol(optarg, &endptr, 10)) < 1
            ||  *endptr != NUL)
               serr("%s: illegal value for -%c option: %s.  "
                    "Must be a number and greater or equal to one",
                    function, ch, optarg);

            break;
         }

         case 'V':
            sockscf.option.verifyonly = 1;
            break;

         case 'd': {
            char *endptr;

            if ((sockscf.option.debug = (int)strtol(optarg, &endptr, 10)) < 0
            ||  *endptr != NUL)
               serr("%s: illegal value for -%c option: %s.  "
                    "Must be a number and greater or equal to zero",
                    function, ch, optarg);

            sockscf.option.debug_isset = 1;
            break;
         }

         case 'f':
            sockscf.option.configfile = optarg;
            break;

         case 'h':
            usage(0);
            /* NOTREACHED */

         case 'n':
            sockscf.option.keepalive = 0;
            break;

         case 'p':
            sockscf.option.pidfile = optarg;
            break;

         case 'v':
            ++sockscf.option.versiononly;
            break;

         default:
            usage(1);
      }
   }

   /*
    * save original commandline so we can use it to override
    * sockd.conf-settings later if needed.
    */
   sockscf.initial.cmdline = sockscf.option;

   argc -= optind;
   argv += optind;

   if (sockscf.option.versiononly) {
      showversion(sockscf.option.versiononly);
      exit(EXIT_SUCCESS);
   }

   if ((sockscf.state.motherpidv = malloc(sizeof(*sockscf.state.motherpidv)
                                          * sockscf.option.serverc)) == NULL)
      serrx("%s", NOMEM);

   bzero(sockscf.state.motherpidv, sizeof(*sockscf.state.motherpidv)
                                   * sockscf.option.serverc);

   /* we are the main server. */
   *sockscf.state.motherpidv = sockscf.state.pid = getpid();

   if (argc > 0)
      serrx("%s: unknown argument %s", function, *argv);

   if (sockscf.option.configfile == NULL)
      sockscf.option.configfile = SOCKD_CONFIGFILE;

   if (sockscf.option.pidfile == NULL)
      sockscf.option.pidfile = SOCKD_PIDFILE;

#if HAVE_LIBWRAP
   /*
    * Save original libwrap settings.  User may want to change them in
    * sockd.conf.
    */
   sockscf.hosts_allow_original = hosts_allow_table;
   sockscf.hosts_deny_original  = hosts_deny_table;
#endif /* HAVE_LIBWRAP */

#if HAVE_SCHED_SETSCHEDULER
   if ((sockscf.initial.cpu.policy = sched_getscheduler(0)) == -1)
      serr("%s: sched_getscheduler(2): failed to retrieve current cpu"
           "scheduling policy",
           function);

   if (sched_getparam(0, &sockscf.initial.cpu.param) != 0)
      serr("%s: sched_getparam(2): failed to retrieve current cpu scheduling "
           "parameters",
           function);

   sockscf.initial.cpu.scheduling_isset = 1;
   sockscf.state.cpu = sockscf.initial.cpu;
#endif /* HAVE_SCHED_SETSCHEDULER */

#if HAVE_SCHED_SETAFFINITY
   if (cpu_getaffinity(0,
                       sizeof(sockscf.initial.cpu.mask),
                       &sockscf.initial.cpu.mask) == -1)
      serr("%s: could not get current cpu scheduling affinity", function);

   sockscf.initial.cpu.affinity_isset = 1;
#endif /* HAVE_SCHED_SETAFFINITY */

   /*
    * needs to be before config file read, as parsing functions may things
    * in shmem.
    */
   shmem_setup();

   genericinit();
   checkconfig();
   newprocinit();

   if (!sockscf.option.verifyonly) {
      if (bindinternal(SOCKS_TCP) != 0)
         serr("%s: failed to bind internal addresses", function);

      resetprivileges();
   }

   sockscf.state.inited = 1;
}

static void
handlechildcommand(command, child, finished)
   const unsigned int command;
   sockd_child_t *child;
   int *finished;
{
   const char *function = "handlechildcommand()";

   SASSERTX(child != NULL);

   slog(LOG_DEBUG, "%s: command %d from %s %ld",
        function, command, childtype2string(child->type), (long)child->pid);

   switch(command) {
      case SOCKD_NOP:
         break;

      case SOCKD_FREESLOT_TCP:
      case SOCKD_FREESLOT_UDP:
         ++child->freec;

         slog(LOG_DEBUG,
              "%s: %s %ld has freed a %s slot, now has %lu slot%s free",
              function,
              childtype2string(child->type),
              (long)child->pid,
              command == SOCKD_FREESLOT_TCP ?  "TCP" : "UDP",
              (unsigned long)child->freec,
              child->freec == 1 ? "" : "s");

         SASSERTX(child->freec <= maxfreeslots(child->type));

         if (child->type == PROC_IO) {
            /*
             * don't receive anything back from i/o childs
             * except the freeslot ack, as i/o childs are the
             * last in the chain, so need to update this stat her.
             */
            ++sockscf.stat.io.received;
#if COVENANT
#warning   "does not always apply to covenant"
#endif /* COVENANT */

#if BAREFOOTD
            if (command == SOCKD_FREESLOT_UDP) {
               --child->hasudpsession;
               SASSERTX(child->hasudpsession == 0);
            }
#endif /* BAREFOOTD */
         }

         break;

      default:
         SWARNX(command);
   }

   if (child->freec == maxfreeslots(child->type)
   &&  child_should_retire(child)) {
      time_t tnow;

      slog(LOG_DEBUG,
           "%s: %s %ld is ready for retirement: %lu requests handled "
           "during %lu seconds",
           function,
           childtype2string(child->type),
           (long)child->pid,
           (unsigned long)child->sentc,
           socks_difftime(time_monotonic(&tnow), child->created));

      *finished = 1;
   }
   else
      *finished = 0;
}

#if DEBUG && DOTEST
static void
dotest(void)
{
/*   const char *function = "dotest()"; */

   doconfigtest();

#if 0
   sockd_child_t *child;
   sockd_client_t client;
   sockd_request_t request;
   sockd_io_t io;

   slog(LOG_INFO, "%s: starting send_client() test ...", function);

   if ((child = nextchild(PROC_NEGOTIATE, SOCKS_TCP)) == NULL)
      serr("%s: nextchild(PROC_NEGOTIATE) failed", function);

   if (kill(child->pid, SIGSTOP) != 0)
      serr("%s: kill(SIGSTOP) of child %ld failed", function, (long)child->pid);

   bzero(&client, sizeof(client));
   if ((client.s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      serr("%s: failed to create a SOCK_STREAM socket", function);

   i = 0;
   while (send_client(child->s, &client, NULL, 0) == 0)
      ++i;

   if (kill(child->pid, SIGTERM) != 0)
      serr("%s: kill(SIGTERM) of child %ld failed", function, (long)child->pid);

   if (i >= SOCKD_NEGOTIATEMAX)
      slog(LOG_INFO, "%s: send_client() test completed ok, sent %d requests",
      function, i);
   else
      swarn("%s: send_client() test failed after %d requests", function, i);


   slog(LOG_INFO, "%s: starting send_req() test ...", function);

   if ((child = nextchild(PROC_REQUEST, SOCKS_TCP)) == NULL)
      serr("%s: nextchild(PROC_REQUEST) failed", function);

   if (kill(child->pid, SIGSTOP) != 0)
      serr("%s: kill(SIGSTOP) of child %ld failed", function, (long)child->pid);

   bzero(&request, sizeof(request));
   if ((request.s = socket(AF_INET, SOCK_STREAM, 0)) == -1)
      serr("%s: failed to create a SOCK_STREAM socket", function);

   i = 0;
   while (send_req(child->s, &request) == 0)
      ++i;

   if (kill(child->pid, SIGTERM) != 0)
      serr("%s: kill(SIGTERM) of child %ld failed", function, (long)child->pid);

   if (i >= SOCKD_REQUESTMAX)
      slog(LOG_INFO, "%s: send_req() test completed ok, sent %d requests",
      function, i);
   else
      swarn("%s: send_req() test failed after %d requests", function, i);

   slog(LOG_INFO, "%s: starting send_io() test ...", function);

   if ((child = nextchild(PROC_IO, SOCKS_TCP)) == NULL)
      serr("%s: nextchild(PROC_IO) failed", function);

   if (kill(child->pid, SIGSTOP) != 0)
      serr("%s: kill(SIGSTOP) of child %ld failed", function, (long)child->pid);

   bzero(&io, sizeof(io));
   io.state.command = SOCKS_UDPASSOCIATE;
   if ((io.control.s = socket(AF_INET, SOCK_STREAM, 0)) == -1
   ||  (io.src.s     = socket(AF_INET, SOCK_STREAM, 0)) == -1
   ||  (io.dst.s     = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
      serr("%s: failed to create a SOCK_STREAM socket", function);

   i = 0;
   while (send_io(child->s, &io) == 0)
      ++i;

   if (kill(child->pid, SIGTERM) != 0)
      serr("%s: kill(SIGTERM) of child %ld failed", function, (long)child->pid);

   if (i >= SOCKD_IOMAX)
      slog(LOG_INFO, "%s: send_io() test completed ok, sent %d requests",
      function, i);
   else
      swarn("%s: send_io() test failed after %d requests", function, i);

#if 0
   socks_iobuftest();
#endif /* 0 */

#endif /* 0 */
}

#endif /* DEBUG */
