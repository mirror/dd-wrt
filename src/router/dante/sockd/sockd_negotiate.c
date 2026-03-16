/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2008,
 *               2009, 2010, 2011, 2012, 2013, 2014, 2016, 2020, 2024
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
"$Id: sockd_negotiate.c,v 1.477.4.5.2.2.4.2.4.6 2024/11/25 20:59:07 michaels Exp $";

static sockd_negotiate_t negv[SOCKD_NEGOTIATEMAX];
static const size_t negc = ELEMENTS(negv);

static void siginfo(int sig, siginfo_t *si, void *sc);

static void
logdisconnect(const int s, sockd_negotiate_t *neg, const operation_t op,
              const iologaddr_t *src, const iologaddr_t *dst,
              const char *buf, const size_t buflen);
/*
 * Logs the disconnect of the client connected to "s", using "neg".
 * - "op" indicates the reason for the disconnect.
 * - "src" and "dst" give information about the source and destination of
 *   the client.
 * - "buf" and "buflen" an optional string giving more info about
 *   the disconnect.
 */

static int
send_negotiate(sockd_negotiate_t *neg);
/*
 * Sends "neg" to "mother".
 * Returns:
 *      On success: 0
 *      On error: -1.  If error was in relation to sending to mother,
 *                     errno will be set.
 */

static int
recv_negotiate(void);
/*
 * Tries to receive a client from mother.
 * Returns: the number of new clients received on success, or -1 on error.
 *
 * Note that a return of 0 does not indicate an error, but does mean
 * no new clients were added, most likely because none of the clients
 * received passed the ACL-checks.
 */

static void
delete_negotiate(sockd_negotiate_t *neg, const int forwardedtomother);
/*
 * Frees any state occupied by "neg", including closing any descriptors.
 * If "forwardedtomother" is set, we are deleting the negotiate after
 * forwarding the client object to mother (and thus need not ack mother).
 */

static int
neg_fillset(fd_set *set, const int skipcompleted, const int skipinprogress);
/*
 * Sets all client descriptors in our list in the set "set".
 *
 * If "skipcompleted" is set, skip those descriptors that belong to
 * clients that have completed negotiation.
 *
 * If "skipinprogress" is set, skip those descriptors that belong to
 * clients that have not yet completed negotiation.
 *
 * Returns the highest descriptor in our list, or -1 if we don't
 * have any descriptors open currently.
 */

static void
neg_clearset(sockd_negotiate_t *neg, fd_set *set);
/*
 * Clears all file descriptors in "neg" from "set".
 */

static sockd_negotiate_t *
neg_getset(fd_set *set);
/*
 * Goes through our list until it finds a negotiate object where at least
 * one of the descriptors is set, or where the negotiation has completed,
 * which always implies the descriptor is "set".
 *
 * Returns:
 *      On success: pointer to the found object.
 *      On failure: NULL.
 */

static size_t
neg_completed(const size_t howmany);
/*
 * Returns the number of objects completed and ready to be sent currently.
 * The function stops counting when a count of "howmany" is reached.
 */

static size_t
neg_allocated(void);
/*
 * Returns the number of objects currently allocated for use.
 */

static void
proctitleupdate(void);
/*
 * Updates the title of this process.
 */

static struct timeval *
neg_gettimeout(struct timeval *timeout);
/*
 * Fills in "timeout" with time til the first clients connection
 * expires.
 * Returns:
 *      If there is a timeout: pointer to filled in "timeout".
 *      If there is no timeout: NULL.
 */

#if HAVE_NEGOTIATE_PHASE
static sockd_negotiate_t *
neg_gettimedout(const struct timeval *tnow);
/*
 * Scans all clients for one that has timed out according to sockscf settings.
 * "tnow" is the current time.
 *
 * Returns:
 *      If a timed out client found: pointer to it.
 *      Else: NULL.
 */
#endif /* HAVE_NEGOTIATE_PHASE */

static void
neg_clearset(sockd_negotiate_t *neg, fd_set *set);
/*
 * Clears all file descriptors in "neg" from "set".
 */

static sockd_negotiate_t *
neg_getset(fd_set *set);
/*
 * Goes through our list until it finds a negotiate object where at least
 * one of the descriptors is set, or where the negotiation has completed,
 * which always implies the descriptor is "set".
 *
 * Returns:
 *      On success: pointer to the found object.
 *      On failure: NULL.
 */

static size_t
neg_completed(const size_t howmany);
/*
 * Returns the number of objects completed and ready to be sent currently.
 * The function stops counting when a count of "howmany" is reached.
 */

static size_t
neg_allocated(void);
/*
 * Returns the number of objects currently allocated for use.
 */

static void
proctitleupdate(void);
/*
 * Updates the title of this process.
 */

static struct timeval *
neg_gettimeout(struct timeval *timeout);
/*
 * Fills in "timeout" with time til the first clients connection expires.
 * Returns:
 *      If there is a timeout: pointer to filled in "timeout".
 *      If there is no timeout: NULL.
 */


void
run_negotiate()
{
   const char *function = "run_negotiate()";
   struct sigaction sigact;
   fd_set *rset, *rsetbuf, *tmpset, *wsetmem;
   int sendfailed;

   bzero(&sigact, sizeof(sigact));
   sigact.sa_flags     = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;
   sigact.sa_sigaction = siginfo;

#if HAVE_SIGNAL_SIGINFO
   if (sigaction(SIGINFO, &sigact, NULL) != 0)
      serr("%s: sigaction(SIGINFO)", function);
#endif /* HAVE_SIGNAL_SIGINFO */

   /* same handler, for systems without SIGINFO. */
   if (sigaction(SIGUSR1, &sigact, NULL) != 0)
      serr("%s: sigaction(SIGINFO)", function);

   rset     = allocate_maxsize_fdset();
   rsetbuf  = allocate_maxsize_fdset();
   tmpset   = allocate_maxsize_fdset();
   wsetmem  = allocate_maxsize_fdset();

   proctitleupdate();

   sockd_print_child_ready_message((size_t)freedescriptors(NULL, NULL));

   sendfailed  = 0;
   while (1 /* CONSTCOND */) {
      negotiate_result_t negstatus;
      struct timeval *timeout, timeoutmem;
      sockd_negotiate_t *neg;
      struct timeval tnow;
      fd_set *wset;
      int fdbits;

      errno = 0; /* reset for each iteration. */


#if HAVE_NEGOTIATE_PHASE
      gettimeofday_monotonic(&tnow);
      while ((neg = neg_gettimedout(&tnow)) != NULL) {
         const time_t duration
         = socks_difftime(tnow.tv_sec, neg->state.time.accepted.tv_sec);

         iologaddr_t src;
         size_t buflen;
         char buf[512];

         init_iologaddr(&src,
                        object_sockshost,
                        &neg->negstate.dst,
                        object_sockshost,
                        &neg->negstate.src,
                        &neg->cauth,
                        &neg->state.hostid);

         if (neg->negstate.complete) {
            struct sockaddr_storage p;

            log_clientdropped(sockshost2sockaddr(&neg->negstate.src, &p));
         }
         /*
          * else; nothing special.  Negotiation simply did not complete in time.
          */

         buflen = snprintf(buf, sizeof(buf),
                           "%s after %ld second%s%s",
                           timeouttype2string(TIMEOUT_NEGOTIATE),
                           (long)duration,
                           duration == 1 ? "" : "s",
                           neg->negstate.complete ?
                        " while waiting for mother to become available" : "");


         logdisconnect(neg->s, neg, OPERATION_ERROR, &src, NULL, buf, buflen);
         delete_negotiate(neg, 0);
      }

      fdbits = neg_fillset(rset,
                           sendfailed, /*
                                        * If we've previously failed sending
                                        * the completed clients back to mother,
                                        * don't bother select(2)ing on them
                                        * for readability now; can't send them
                                        * until we know mother is writable.
                                        */
                           0);         /*
                                        * clients where negotiation is not
                                        * yet completed we want to continue
                                        * negotiating with.
                                        */
#else /* !HAVE_NEGOTIATE_PHASE */

      /*
       * don't bother checking here.  All, if any, should be completed.
       * Meaning, neg_completed(1) will be true and no timeout will be set
       * on select(2), unless sending to mother failed.
       */
      fdbits = -1;
      FD_ZERO(rset);

#endif /* HAVE_NEGOTIATE_PHASE */

      FD_COPY(rsetbuf, rset);

      if (neg_completed(1) && !sendfailed) {
         timeout         = &timeoutmem;
         timerclear(timeout);
      }
      else
         timeout = neg_gettimeout(&timeoutmem);

      FD_SET(sockscf.state.mother.s, rset);
      fdbits = MAX(fdbits, sockscf.state.mother.s);

      if (sendfailed) {
         /*
          * Previously failed sending a request to mother.
          * Pull in select this time to check; normally we don't
          * bother and just assume mother will be able to accept it.
          */
         FD_ZERO(wsetmem);
         FD_SET(sockscf.state.mother.s, wsetmem);
         wset = wsetmem;

         fdbits     = MAX(fdbits, sockscf.state.mother.s);
         sendfailed = 0;
      }
      else
         wset = NULL;

      /* also check ack-pipe so we know if mother goes away.  */
      FD_SET(sockscf.state.mother.ack, rset);
      fdbits = MAX(fdbits, sockscf.state.mother.ack);

      SASSERTX(fdbits >= 0);

      ++fdbits;
      switch (selectn(fdbits, rset, rsetbuf, NULL, wset, NULL, timeout)) {
         case -1:
            SASSERT(ERRNOISTMP(errno));
            continue;

         case 0:
            if (neg_completed(1)) /* XXX why? */
               break;
            else
               continue;
      }

#if !HAVE_NEGOTIATE_PHASE
      /*
       * since we have no negotiate phase, any fd's should be "readable", or
       * in practice, "completed", so just add them to what is reported
       * as readable by selectn() (could only be mother).
       */
      fdbits = MAX(fdbits, neg_fillset(tmpset, 0, 1));

      fdsetop(fdbits, '|', rset, tmpset, rset);
#else /* HAVE_NEGOTIATE_PHASE */

      fdsetop(fdbits, '|', rset, rsetbuf, rset);

      /*
       * Those descriptors that have completed negotiation we want to
       * consider readable/ready, so we know to call recv_clientrequest()
       * on them.
       */
      fdbits = MAX(fdbits, neg_fillset(tmpset, 0, 1));
      fdsetop(fdbits, '|', rset, tmpset, rset);
#endif /* HAVE_NEGOTIATE_PHASE */

      if (FD_ISSET(sockscf.state.mother.ack, rset)) {
         sockd_readmotherscontrolsocket(function, sockscf.state.mother.ack);

#if HAVE_VALGRIND_VALGRIND_H
         if (RUNNING_ON_VALGRIND) {
            /* for some reason Valgrind complains the rset pointer is lost. */
            free(rset);
         }
#endif /* HAVE_VALGRIND_VALGRIND_H */

         sockdexit(EXIT_SUCCESS);
      }

      if (FD_ISSET(sockscf.state.mother.s, rset)) {
         if (recv_negotiate() == -1)
            continue; /* loop around and check control-connection. */
         else
            proctitleupdate();

         FD_CLR(sockscf.state.mother.s, rset);
      }

      while ((neg = neg_getset(rset)) != NULL) {
         int io_errno;

         neg_clearset(neg, rset);

         errno     = 0; /* reset before each client. */
         negstatus = recv_clientrequest(neg->s, &neg->req, &neg->negstate);
         io_errno  = errno;

         slog(LOG_DEBUG,
              "%s: recv_clientrequest() from client %s returned %d, "
              "errno is %d (%s)",
              function,
              sockshost2string(&neg->negstate.src, NULL, 0),
              negstatus,
              errno,
              errno == 0 ?
                  "no error" : ERRNOISTMP(errno) ?
                        "temp error" : "fatal error");

         switch (negstatus) {
            case NEGOTIATE_CONTINUE:
               break;

            case NEGOTIATE_FINISHED: {
               if (!timerisset(&neg->state.time.negotiateend))
                  gettimeofday_monotonic(&neg->state.time.negotiateend);

               neg->state.command       = neg->req.command;
               neg->state.protocol      = neg->req.protocol;
               neg->state.proxyprotocol = neg->req.version;

#if COVENANT
               if (neg->negstate.havedonerulespermit)
                  slog(LOG_DEBUG,
                       "%s: must have failed to send client %s to mother "
                       "before trying again",
                       function,
                       sockshost2string(&neg->negstate.src, NULL, 0));
               else {
                  /*
                   * Need to do rulespermit() of second-level acl in this
                   * process as well, as if it fails due to missing proxy
                   * authentication we need to inform the client and restart
                   * negotiation (i.e., wait for the client to repeat the
                   * request, but this time with proxy authentication).
                   */
                  struct sockaddr_storage src, dst;
                  int permit;

                  sockshost2sockaddr(&neg->negstate.src, &src);
                  sockshost2sockaddr(&neg->negstate.dst, &dst);

                  slog(LOG_DEBUG, "%s: second-level acl check on %s -> %s",
                       function,
                       sockaddr2string(&src, NULL, 0),
                       sockshost2string(&neg->req.host, NULL, 0));

                  permit = rulespermit(neg->s,
                                       &src,
                                       &dst,
                                       &neg->cauth,
                                       &neg->sauth,
                                       &neg->srule,
                                       &neg->state,
                                       &neg->negstate.src,
                                       &neg->req.host,
                                       NULL,
                                       neg->negstate.emsg,
                                       sizeof(neg->negstate.emsg));

                  if (permit)
                     neg->negstate.havedonerulespermit = 1;
                  else {
                     iologaddr_t src;
                     response_t response;

                     if (neg->srule.whyblock.missingproxyauth) {
                        if (!neg->negstate.haverequestedproxyauth) {
                           slog(LOG_DEBUG,
                                "%s: telling client at %s to provide proxy "
                                 "authentication before restarting negotiation",
                                  function,
                                 sockshost2string(&neg->negstate.src, NULL, 0));

                           bzero(&response, sizeof(response));
                           response.version    = neg->state.proxyprotocol;
                           response.host       = neg->req.host;
                           response.reply.http
                           = response.version == PROXY_HTTP_10 ?
                              HTTP_NOTALLOWED : HTTP_PROXYAUTHREQUIRED;

                           send_response(neg->s, &response);

                           neg->negstate.complete               = 0;
                           neg->negstate.haverequestedproxyauth = 1;
                           bzero(neg->negstate.mem, sizeof(neg->negstate.mem));

                           continue;
                        }

                        slog(LOG_DEBUG,
                             "%s: already told client at %s to provide "
                             "proxy authentication, but again it sent "
                             "us a request without authentication",
                             function,
                             sockshost2string(&neg->negstate.src, NULL, 0));
                     }

                     /*
                      * only log on deny.  Pass will be logged as usual later.
                      */

                     init_iologaddr(&src,
                                    object_sockshost,
                                    &neg->negstate.dst,
                                    object_sockshost,
                                    &neg->negstate.src,
                                    &neg->cauth,
                                    &neg->state.hostid);

                     if (neg->srule.log.tcpinfo) {
                        int fdv[] = { neg->s };

                        neg->state.tcpinfo
                        = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
                     }

                     iolog(&neg->srule,
                           &neg->state,
                           OPERATION_CONNECT,
                           &src,
                           NULL,
                           NULL,
                           NULL,
                           neg->negstate.emsg,
                           strlen(neg->negstate.emsg));

                     iolog(&neg->crule,
                           &neg->state,
                           OPERATION_CONNECT,
                           &src,
                           NULL,
                           NULL,
                           NULL,
                           neg->negstate.emsg,
                           strlen(neg->negstate.emsg));

                     neg->state.tcpinfo = NULL;

                     bzero(&response, sizeof(response));
                     response.version    = neg->state.proxyprotocol;
                     response.host       = neg->req.host;
                     response.reply.http = HTTP_FORBIDDEN;

                     send_response(neg->s, &response);
                     delete_negotiate(neg, 0);

                     continue;
                  }
               }
#endif /* COVENANT */

               if (wset != NULL && !FD_ISSET(sockscf.state.mother.s, wset)) {
                  sendfailed = 1;
                  continue; /* don't bother trying to send to mother now. */
               }

               errno = 0;
               if (send_negotiate(neg) == 0) {
                  delete_negotiate(neg, 1);
                  sendfailed = 0;
               }
               else if (ERRNOISTMP(errno))
                  sendfailed = 1; /* we will retry sending this object later. */
               else {
                  struct sockaddr_storage p;

                  log_clientdropped(sockshost2sockaddr(&neg->negstate.src, &p));

                  delete_negotiate(neg, 0);

                  /*
                   * assume what failed was not related to the send to mother,
                   * but some (network) error related to the connection between
                   * us and the client.  If the error is between us and mother,
                   * it will be picked up on the control-pipe.
                   */
                  sendfailed = 0;
               }

               break;
            }

            case NEGOTIATE_ERROR:
            case NEGOTIATE_EOF: {
               const char *error;
               const time_t duration = socks_difftime(time_monotonic(NULL),
                                               neg->state.time.accepted.tv_sec);
               iologaddr_t src;
               char reason[256];
               int takingtoolong = 0, erroriseof = 0;

               if (negstatus == NEGOTIATE_EOF) {
                  error      = "eof from local client";
                  erroriseof = 1;
               }
               else {
                  if (*neg->negstate.emsg == NUL) {
                     if (io_errno == 0)
                        error = "local client protocol error";
                     else
                        error = strerror(io_errno);
                  }
                  else
                     error = neg->negstate.emsg;
               }

               if (CRULE_OR_HRULE(neg)->mstats_shmid != 0
               && (CRULE_OR_HRULE(neg)->alarmsconfigured & ALARM_DISCONNECT)) {
                  clientinfo_t cinfo;
                  int weclosed;

                  if (negstatus == NEGOTIATE_EOF || ERRNOISNETWORK(errno))
                     weclosed = 0;
                  else
                     weclosed = 1;

                  sockshost2sockaddr(&neg->negstate.src, &cinfo.from);
                  cinfo.hostid = neg->state.hostid;

                  alarm_add_disconnect(weclosed,
                                       CRULE_OR_HRULE(neg),
                                       ALARM_INTERNAL,
                                       &cinfo,
                                       error,
                                       sockscf.shmemfd);
               }

               if (!erroriseof && neg->negstate.complete) {
                  struct timeval tdiff;

                  gettimeofday_monotonic(&tnow);
                  timersub(&tnow, &neg->state.time.negotiateend, &tdiff);

                  if (tdiff.tv_sec >= CRULE_OR_HRULE(neg)->timeout.negotiate)
                     takingtoolong = 1;
               }

               snprintf(reason, sizeof(reason),
                       "error after reading %lu byte%s%s in %ld second%s: %s",
                       (unsigned long)neg->negstate.reqread,
                       (unsigned long)neg->negstate.reqread == 1 ? "" : "s",
                       takingtoolong ?
                           ", possibly due to us taking too "
                           "long to send the clientobject to mother"  :  "",
                       (long)duration,
                       duration == 1 ? "" : "s",
                       error);

               init_iologaddr(&src,
                              object_sockshost,
                              &neg->negstate.dst,
                              object_sockshost,
                              &neg->negstate.src,
                              &neg->cauth,
                              &neg->state.hostid);

               if (neg->crule.log.tcpinfo) {
                  int fdv[] = { neg->s };

                  neg->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
               }

               logdisconnect(neg->s,
                             neg,
                             OPERATION_ERROR,
                             &src,
                             NULL,
                             reason,
                             strlen(reason));

               delete_negotiate(neg, 0);
               break;
            }

            default:
               SERRX(negstatus);
         }
      }
   }
}

void
negotiate_preconfigload(void)
{
   const char *function = "negotiate_preconfigload()";

   slog(LOG_DEBUG, "%s", function);
}

void
negotiate_postconfigload(void)
{
   const char *function = "negotiate_postconfigload()";
   size_t i;

   slog(LOG_DEBUG, "%s", function);

   /*
    * update monitor shmids in rules used by current clients.
    */
   for (i = 0; i < negc; ++i) {
      const monitor_t *p;
      monitor_t oldmonitor, newmonitor;
      clientinfo_t cinfo;

      if (!negv[i].allocated)
         continue;

      SASSERTX(CRULE_OR_HRULE(&negv[i])->mstats == NULL);

      bzero(&oldmonitor, sizeof(oldmonitor));
      if (CRULE_OR_HRULE(&negv[i])->mstats_shmid != 0)
         COPY_MONITORFIELDS(CRULE_OR_HRULE(&negv[i]), &oldmonitor);

      p = monitormatch(&negv[i].negstate.src,
                       &negv[i].negstate.dst,
                       &negv[i].cauth,
                       &negv[i].state);

      if (p == NULL)
         bzero(&newmonitor, sizeof(newmonitor));
      else {
         SASSERTX(p->mstats == NULL);
         newmonitor = *p;
      }

      if (oldmonitor.mstats_shmid == 0 && newmonitor.mstats_shmid == 0)
         continue; /* no monitors for this client before, and no now. */

      sockshost2sockaddr(&negv[i].negstate.src, &cinfo.from);
      cinfo.hostid = negv[i].state.hostid;

      monitor_move(&oldmonitor,
                   &newmonitor,
                   0,            /* negotiate process does not stay attached. */
                   ALARM_INTERNAL,
                   &cinfo,
                   sockscf.shmemfd);

      COPY_MONITORFIELDS(&newmonitor, CRULE_OR_HRULE(&negv[i]));

#if COVENANT
#warning "missing monitor code"
#endif /* COVENANT */
   }
}


static int
send_negotiate(neg)
   sockd_negotiate_t *neg;
{
   const char *function = "send_negotiate()";
#if HAVE_GSSAPI
   gss_buffer_desc gssapistate = { 0, NULL };
   char gssapistatemem[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */
   struct iovec iov[2];
   sockd_request_t req;
   clientinfo_t cinfo;
   struct msghdr msg;
   ssize_t w;
   size_t length, ioc, fdsendt;
   CMSG_AALLOC(cmsg, sizeof(int));
   int unuse = 0;

   sockshost2sockaddr(&neg->negstate.src, &cinfo.from);
   cinfo.hostid = neg->state.hostid;

   if (CRULE_OR_HRULE(neg)->bw_shmid != 0
   && !CRULE_OR_HRULE(neg)->bw_isinheritable)
      unuse |= SHMEM_BW;

   if (CRULE_OR_HRULE(neg)->ss_shmid != 0
   && !CRULE_OR_HRULE(neg)->ss_isinheritable)
      unuse |= SHMEM_SS;

   if (unuse != 0)
      slog(LOG_DEBUG,
           "%s: unusing and clearing shmem %d; not marked as inheritable",
           function, unuse);
   else
      slog(LOG_DEBUG, "%s: no shmem to unuse/clear", function);

   SHMEM_UNUSE(CRULE_OR_HRULE(neg), &cinfo, sockscf.shmemfd, unuse);
   SHMEM_CLEAR(CRULE_OR_HRULE(neg), unuse, 1);
   SHMEM_CLEAR(CRULE_OR_HRULE(neg), SHMEM_ALL, 0);

   /*
    * copy needed fields from negotiate.
    */
   bzero(&req, sizeof(req)); /* silence valgrind warning */
   sockshost2sockaddr(&neg->negstate.src, &req.from);
   sockshost2sockaddr(&neg->negstate.dst, &req.to);

   req.req             = neg->req;
   req.reqinfo.command = (neg->state.protocol == SOCKS_TCP ?
                              SOCKD_FREESLOT_TCP : SOCKD_FREESLOT_UDP);

#if HAVE_NEGOTIATE_PHASE
   /*
    * save initial requestdata from client, if any.
    */

#if COVENANT
   if (neg->req.flags.httpconnect)
      length = 0;
   else {
      /* XXX should probably strip out any authorization headers if present. */
      length = neg->negstate.reqread;
      memcpy(req.clientdata, neg->negstate.mem, length);
   }

#else /* SOCKS_SERVER */
   if ((length = socks_bytesinbuffer(neg->s, READ_BUF, 0)) != 0) {
      slog(length > sizeof(req.clientdata) ? LOG_INFO : LOG_DEBUG,
           "%s: local socks client at %s sent us %lu bytes of payload before "
           "we told it that it can do that.  Not permitted by the SOCKS "
           "standard.  %s",
           function,
           socket2string(neg->s, NULL, 0),
           (unsigned long)length,
           length > sizeof(req.clientdata) ?
                 "Too much unexpected data for us to handle"
               : "Trying to handle it however");

      if (length > sizeof(req.clientdata))
        return -1;

      socks_getfrombuffer(neg->s, 0, READ_BUF, 0, req.clientdata, length);
   }
#endif /* SOCKS_SERVER */

    req.clientdatalen = length;

    if (req.clientdatalen > 0)
       slog(LOG_DEBUG,
            "%s: saving local client data of length %lu for later forwarding",
            function, (unsigned long)req.clientdatalen);
#endif /* HAVE_NEGOTIATE_PHASE */

   req.crule       = neg->crule;

#if HAVE_SOCKS_HOSTID
   req.hrule       = neg->hrule;
   req.hrule_isset = neg->hrule_isset;
#endif /* HAVE_SOCKS_HOSTID */

#if COVENANT
   req.srule         = neg->srule;
#endif /* COVENANT */

   req.cauth         = neg->cauth;
   req.sauth         = neg->sauth;

   req.state         = neg->state;

   SASSERTX(proxyprotocolisknown(req.state.proxyprotocol));

   slog(LOG_DEBUG,
        "%s: client %s finished negotiate phase for command %s using "
        "proxyprotocol %s",
        function,
        sockaddr2string(&req.from, NULL, 0),
        command2string(req.state.command),
        proxyprotocol2string(req.state.proxyprotocol));

   bzero(iov, sizeof(iov));
   ioc               = 0;
   length            = 0;
   iov[ioc].iov_base = &req;
   iov[ioc].iov_len  = sizeof(req);
   length           += iov[ioc].iov_len;
   ++ioc;

#if HAVE_GSSAPI
   if (req.sauth.method == AUTHMETHOD_GSSAPI) {
      gssapistate.value   = gssapistatemem;
      gssapistate.length  = sizeof(gssapistatemem);

      if (gssapi_export_state(&req.sauth.mdata.gssapi.state.id, &gssapistate)
      != 0)
         return -1;

      iov[ioc].iov_base = gssapistate.value;
      iov[ioc].iov_len  = gssapistate.length;
      length           += iov[ioc].iov_len;
      ++ioc;

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: gssapistate has length %lu",
              function, (long unsigned)gssapistate.length);
   }
#endif /* HAVE_GSSAPI */

   fdsendt = 0;

#if BAREFOOTD
   if (req.state.command != SOCKS_UDPASSOCIATE)
      /* udp has no control/client socket until set up in request-child. */
      CMSG_ADDOBJECT(neg->s, cmsg, sizeof(neg->s) * fdsendt++);

#else /* !BAREFOOTD */

   CMSG_ADDOBJECT(neg->s, cmsg, sizeof(neg->s) * fdsendt++);

#endif /* !BAREFOOTD */

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ioc;
   msg.msg_name    = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdsendt);

   if (sockscf.option.debug >= DEBUG_VERBOSE && neg->s != -1)
      slog(LOG_DEBUG, "%s: sending fd %d (%s) ...",
           function, neg->s, socket2string(neg->s, NULL, 0));

   if ((w = sendmsgn(sockscf.state.mother.s, &msg, 0, 100)) != (ssize_t)length){
      slog(LOG_DEBUG, "%s: sending client to mother failed: %s",
           function, strerror(errno));

#if HAVE_GSSAPI
      /*
       * re-import the gssapi state so we can delete if needed.
       */
      if (gssapistate.value != NULL) {
         if (gssapi_import_state(&req.sauth.mdata.gssapi.state.id, &gssapistate)
         != 0)
            swarnx("%s: could not re-import gssapi state", function);
      }
#endif /* HAVE_GSSAPI */
   }
   else {
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG,
              "%s: sent %ld descriptors for command %d.  cauth %s, sauth %s, "
              "neg->s %d",
              function,
              (unsigned long)fdsendt,
              req.state.command,
              method2string(req.cauth.method),
              method2string(req.sauth.method),
              neg->s);
   }

   return (size_t)w == length ? 0 : -1;
}

static int
recv_negotiate(void)
{
   const char *function = "recv_negotiate()";
   sockd_client_t client;
   sockd_negotiate_t *neg;
   struct iovec iov[1];
   struct msghdr msg;
   ssize_t r;
   size_t i, freec, newc, failedc;
   CMSG_AALLOC(cmsg, sizeof(int));
   char ruleinfo[256];
   int permit, fdreceived;

   bzero(iov, sizeof(iov));
   iov[0].iov_base = &client;
   iov[0].iov_len  = sizeof(client);

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ELEMENTS(iov);
   msg.msg_name    = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   newc  = failedc = errno = 0;
   freec = SOCKD_NEGOTIATEMAX - neg_allocated();

   while (newc < freec) {
      const size_t fdexpect = 1;
      iologaddr_t src, dst;
      clientinfo_t cinfo;

#if DIAGNOSTIC /* for internal debugging/testing. */
      shmemcheck();
#endif /* DIAGNOSTIC */

      if ((r = recvmsgn(sockscf.state.mother.s, &msg, 0)) != sizeof(client)) {
         switch (r) {
            case -1:
            case 0:
               slog(LOG_DEBUG,
                    "%s: recvmsg() from mother returned %ld after having "
                    "received %lu new clients (%lu failed/blocked clients).  "
                    "errno = %d (%s)",
                    function, (long)r,
                    (unsigned long)newc, (unsigned long)failedc,
                    errno, strerror(errno));
               break;

            default:
               swarnx("%s: recvmsg(): unexpected short read from mother after "
                      "having received %lu new clients.  Got %ld/%lu bytes",
                      function,
                      (unsigned long)newc,
                      (long)r,
                      (unsigned long)sizeof(client));
         }

         break;
      }

      if (socks_msghaserrors(function, &msg)) {
         ++failedc;
         continue;
      }

      /*
       * Got packet of expected size, now find a free slot for it.
       */
      for (i = 0, neg = NULL; i < negc; ++i)
         if (!negv[i].allocated) {
            /*
             * don't allocate it yet, so siginfo() doesn't print before ready.
             */
            neg = &negv[i];
            break;
         }

      if (neg == NULL)
         SERRX(neg_allocated());

      if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
         swarnx("%s: received control message has the invalid len of %d",
                 function, (int)CMSG_TOTLEN(msg));

         ++failedc;
         continue;
      }

      SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
      SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

      fdreceived = 0;
      CMSG_GETOBJECT(neg->s, cmsg, sizeof(neg->s) * fdreceived++);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: received fd %d (%s) ...",
              function, neg->s, socket2string(neg->s, NULL, 0));

      neg->state.time.accepted = client.accepted;
      gettimeofday_monotonic(&neg->state.time.negotiatestart);

      sockd_check_ipclatency("client object received from mother",
                             &neg->state.time.accepted,
                             &neg->state.time.negotiatestart,
                             &neg->state.time.negotiatestart);

      /*
       * init state correctly for checking a connection to us.
       */

      sockaddr2sockshost(&client.from, &neg->negstate.src);
      sockaddr2sockshost(&client.to,   &neg->negstate.dst);

      neg->state.command  = SOCKS_ACCEPT;
      neg->state.protocol = SOCKS_TCP;

      neg->req.auth       = &neg->sauth;       /* pointer fixup    */
      neg->cauth.method   = AUTHMETHOD_NOTSET; /* nothing so far   */

#if PRERELEASE
#warning "XXX add call to sockd_handledsignals() here.  This loop could be long"
#endif /* PRERELEASE */

      permit = rulespermit(neg->s,
                           &client.from,
                           &client.to,
                           NULL,
                           &neg->cauth,
                           &neg->crule,
                           &neg->state,
                           &neg->negstate.src,
                           &neg->negstate.dst,
                           NULL,
                           ruleinfo,
                           sizeof(ruleinfo));

      cinfo.from   = client.from;
      cinfo.hostid = neg->state.hostid;

      setconfsockoptions(neg->s,
                         neg->s,
                         SOCKS_TCP,
                         1,
                         neg->crule.socketoptionc,
                         neg->crule.socketoptionv,
                         SOCKETOPT_ANYTIME | SOCKETOPT_POST,
                         SOCKETOPT_ANYTIME | SOCKETOPT_POST);

      /*
       * Might need to use some values from cauth when negotiating,
       * i.e. gssapi or pam-values.  Also, in some cases (gssapi), the
       * authmethod to be used for socks negotiation needs to be
       * set at the client acl pass, so start by copying the current
       * cauth into what will become the sauth proper.
       */
      neg->sauth = neg->cauth;

      /* but don't actually set the authmethod.  rulespermit() will do that. */
      neg->sauth.method = AUTHMETHOD_NOTSET;

      if (permit) {
         if (shmem_userule(&neg->crule, &cinfo, ruleinfo, sizeof(ruleinfo))
         != 0) {
            permit             = 0;
            neg->crule.verdict = VERDICT_BLOCK;

            SASSERTX(!SHMID_ISATTACHED(&neg->crule));
            SHMEM_CLEAR(&neg->crule, SHMEM_ALL, 1); /* could not use it. */
         }
      }

      init_iologaddr(&src,
                     object_sockaddr,
                     &client.to,
                     object_sockshost,
                     &neg->negstate.src,
                     &neg->cauth,
                     &neg->state.hostid);

      init_iologaddr(&dst,
                     object_none,
                     NULL,
                     object_sockshost,
#if BAREFOOTD
                     &neg->crule.extra.bounceto,
#else /* !BAREFOOTD */
                     NULL,
#endif /* !BAREFOOTD */
                     NULL,
                     NULL);


#if !HAVE_SOCKS_RULES
      /*
       * If it's a block, this is the only logging that will be done, so
       * do it now.
       */

      if (!permit) /* really SOCKS_ACCEPT, but user does not know about that. */
         neg->state.command = SOCKS_CONNECT;

      if (!permit || sockscf.option.debug) {
#endif /* !HAVE_SOCKS_RULES */

      logdisconnect(neg->s,
                    neg,
                    permit ? OPERATION_ACCEPT  : OPERATION_BLOCK,
                    &src,
#if BAREFOOTD
                    &dst,
#else /* !HAVE_SOCKS_RULES */
                    NULL,
#endif /* !HAVE_SOCKS_RULES */

                    ruleinfo,
                    strlen(ruleinfo));

#if !HAVE_SOCKS_RULES
      }
#endif /* !HAVE_SOCKS_RULES */

      if (!permit) {
         ++failedc;
         delete_negotiate(neg, 0);

         continue;
      }

#if HAVE_NEGOTIATE_PHASE
    socks_allocbuffer(neg->s, SOCK_STREAM);

    /*
     * We don't want this buffer to be bigger than MAXREQLEN, as that is
     * the amount of memory we have allocated to hold possible client data.
     *
     * Normally there is no client data in Dante's case, but some clients
     * may piggy-back the payload together with the socks request, without
     * waiting for our response.  That is not legal to do, but some clients
     * do it anyway, so we better support it.
     * We therefor need to make sure we never read more of the payload than
     * we can send on to the i/o process, which will eventually need to
     * forward it to the destination.
     */
    socks_setbufferfd(neg->s, _IONBF, MAXREQLEN);
#endif /* HAVE_NEGOTIATE_PHASE */

#if HAVE_PAM
      /* copy over pam-values from matched rule. */
      STRCPY_ASSERTSIZE(neg->sauth.mdata.pam.servicename,
                        CRULE_OR_HRULE(neg)->state.pamservicename);
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
      /* copy over bsdauth-values from matched rule. */
      STRCPY_ASSERTSIZE(neg->sauth.mdata.bsd.style,
                        CRULE_OR_HRULE(neg)->state.bsdauthstylename);
#endif /* HAVE_BSDAUTH */

#if HAVE_GSSAPI
      /* copy over gssapi-values from matched rule. */
      STRCPY_ASSERTSIZE(neg->sauth.mdata.gssapi.servicename,
                        CRULE_OR_HRULE(neg)->state.gssapiservicename);

      STRCPY_ASSERTSIZE(neg->sauth.mdata.gssapi.keytab,
                        CRULE_OR_HRULE(neg)->state.gssapikeytab);

      neg->sauth.mdata.gssapi.encryption
      = CRULE_OR_HRULE(neg)->state.gssapiencryption;
#endif /* HAVE_GSSAPI */

#if HAVE_SOCKS_HOSTID
      if (sockscf.hrule != NULL) {
         /*
          * Unlike the client- and socks-rules, we assume that if no
          * hostid-rules are configured, we should simply skip/pass
          * that step.  The reason for this is that hostid may not be
          * available or in use on the network, or the user may not want
          * to care about it.  So unless the user explicitly enables
          * hostid-usage by providing at least one hostid-rule, the most
          * sensible default is to not require hostid-rules permitting the
          * client.
          */
         size_t p;

         slog(LOG_DEBUG, "%s: checking access through hostid rules", function);

         neg->state.command = SOCKS_HOSTID;

         permit = rulespermit(neg->s,
                              &client.from,
                              &client.to,
                              &neg->cauth,
                              &neg->cauth,
                              &neg->hrule,
                              &neg->state,
                              &neg->negstate.src,
                              &neg->negstate.dst,
                              NULL,
                              ruleinfo,
                              sizeof(ruleinfo));

         /*
          * if no hostids, no rules will have been checked.
          */
         if (!permit || (permit && neg->state.hostid.addrc > 0)) {
            neg->hrule_isset = 1;

            setconfsockoptions(neg->s,
                               -1,
                               SOCKS_TCP,
                               1,
                               neg->hrule.socketoptionc,
                               neg->hrule.socketoptionv,
                               SOCKETOPT_PRE | SOCKETOPT_ANYTIME,
                               0 /* should already be set. */);

            if (neg->state.hostid.addrc > 0)
               src.hostidc = gethostidipv(&neg->state.hostid, 
                                          src.hostidv, 
                                          ELEMENTS(src.hostidv));

            if (permit) {
               /*
                * Let the hostid-rule inherit settings from the client-rule,
                * or use it's own.
                */
               if (rule_inheritoruse(&neg->crule,
                                     &cinfo,
                                     &neg->hrule,
                                     &cinfo,
                                     ALARM_INTERNAL,
                                     ruleinfo,
                                     sizeof(ruleinfo)) != 0) {
                  SHMEM_CLEAR(&neg->hrule, SHMEM_ALL, 1);
                  permit = 0;
               }
            }

            if (neg->hrule.log.tcpinfo) {
               int fdv[] = { neg->s };

               neg->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
            }

            iolog(&neg->hrule,
                  &neg->state,
                  permit ? OPERATION_HOSTID  : OPERATION_BLOCK,
                  &src,

#if BAREFOOTD
                  &dst,
#else /* !HAVE_SOCKS_RULES */
                  NULL,
#endif /* !HAVE_SOCKS_RULES */

                  NULL,
                  NULL,
                  ruleinfo,
                  strlen(ruleinfo));

            if (!permit) {
               /*
                * log the client-rule close too.
                */
               p = snprintf(ruleinfo, sizeof(ruleinfo),
                            "blocked by higher-level %s #%lu",
                            objecttype2string(neg->hrule.type),
                            (unsigned long)neg->hrule.number);

               if (neg->crule.log.tcpinfo && neg->state.tcpinfo == NULL) {
                  int fdv[] = { neg->s };

                  neg->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
               }

               neg->state.command = HAVE_SOCKS_RULES ?
                                       SOCKS_ACCEPT : SOCKS_CONNECT;

               iolog(&neg->crule,
                     &neg->state,
                     OPERATION_BLOCK,
                     &src,

#if BAREFOOTD
                     &dst,
#else /* !HAVE_SOCKS_RULES */
                     NULL,
#endif /* !HAVE_SOCKS_RULES */

                     NULL,
                     NULL,
                     ruleinfo,
                     p);

#if !HAVE_SOCKS_RULES
               if (sockscf.option.debug) {
                  neg->state.command = SOCKS_ACCEPT;
                  /*
                   * normally not logging tcp/accept, but if debug is enabled,
                   * we do.
                   */
                  iolog(&neg->crule,
                        &neg->state,
                        OPERATION_BLOCK,
                        &src,
                        &dst,
                        NULL,
                        NULL,
                        ruleinfo,
                        p);
               }
#endif /* !HAVE_SOCKS_RULES */

               neg->state.tcpinfo = NULL;

               ++failedc;
               delete_negotiate(neg, 0);

               continue;
            }

            neg->state.tcpinfo = NULL;
         }
      }
#endif /* HAVE_SOCKS_HOSTID */

#if BAREFOOTD

      neg->req.version       = PROXY_SOCKS_V5;
      neg->req.command       = SOCKS_CONNECT;
      neg->req.flag          = 0;
      neg->req.protocol      = SOCKS_TCP;

      ruleaddr2sockshost(&neg->crule.extra.bounceto,
                         &neg->req.host,
                         neg->req.protocol);

      neg->negstate.complete = 1; /* nothing to do in barefoot's case. */

#elif COVENANT /* !BAREFOOTD */

      if (client.clientdatalen > 0) {
         slog(LOG_DEBUG,
              "%s: received client already has %lu bytes read from it.  "
              "Must be a client that is changing it's http server target "
              "to %s, so request should already be parsed",
              function,
              (unsigned long)client.clientdatalen,
              sockshost2string(&client.request.host, NULL, 0));

         SASSERTX(client.clientdatalen <= sizeof(neg->negstate.mem));
         memcpy(neg->negstate.mem, client.clientdata, client.clientdatalen);
         neg->negstate.reqread  = client.clientdatalen;
         neg->negstate.complete = 1;
         neg->req               = client.request;
         *neg->req.auth         = client.auth; /* pointer fixup. */
      }
#endif /* COVENANT */

      /*
       * wait with this until the end, when we know that the client was
       * accepted and by what rule (crule or hrule).
       */
      if (CRULE_OR_HRULE(neg)->mstats_shmid != 0
      &&  CRULE_OR_HRULE(neg)->alarmsconfigured & ALARM_DISCONNECT)
        alarm_add_connect(CRULE_OR_HRULE(neg),
                          ALARM_INTERNAL,
                          &cinfo,
                          sockscf.shmemfd);

      neg->negstate.crule = &neg->crule;

      neg->allocated = 1;
      ++newc;
   }

   if (newc == 0 && failedc == 0) {
      slog(LOG_DEBUG,
           "%s: strange ... we were called to receive a new client (%lu/%lu), "
           "but no new client was there to receive: %s",
           function,
           (unsigned long)(freec + 1),
           (unsigned long)SOCKD_NEGOTIATEMAX,
           strerror(errno));

      return -1;
   }

   if (ERRNOISTMP(errno))
      errno = 0;

   return newc;
}

static void
delete_negotiate(neg, forwardedtomother)
   sockd_negotiate_t *neg;
   const int forwardedtomother;
{
   const char *function = "delete_negotiate()";

   slog(LOG_DEBUG, "%s: forwardedtomother: %d", function, forwardedtomother);

   if (!forwardedtomother) {
      const unsigned char cmd = (neg->state.protocol == SOCKS_TCP ?
                                       SOCKD_FREESLOT_TCP : SOCKD_FREESLOT_UDP);
      clientinfo_t cinfo;

#if HAVE_GSSAPI
      if (neg->sauth.method == AUTHMETHOD_GSSAPI
      &&  neg->sauth.mdata.gssapi.state.id != GSS_C_NO_CONTEXT) {
         OM_uint32 major_status, minor_status;

         if ((major_status
         = gss_delete_sec_context(&minor_status,
                                  &neg->sauth.mdata.gssapi.state.id,
                                  GSS_C_NO_BUFFER)) != GSS_S_COMPLETE){
            char buf[512];

            if (!gss_err_isset(major_status,
                               minor_status,
                               buf,
                               sizeof(buf))) {
               *buf = NUL;

               swarn("%s: gss_delete_sec_context() failed%s%s",
                     function,
                      *buf == NUL ? "" : ": ",
                      *buf == NUL ? "" : buf);
            }
         }
      }
#endif /* HAVE_GSSAPI */

      sockshost2sockaddr(&neg->negstate.src, &cinfo.from);
      cinfo.hostid = neg->state.hostid;

      if (socks_sendton(sockscf.state.mother.ack,
                        &cmd,
                        sizeof(cmd),
                        sizeof(cmd),
                        0,
                        NULL,
                        0,
                        NULL,
                        NULL) != sizeof(cmd))
         slog(sockd_motherexists() ? LOG_WARNING : LOG_DEBUG,
              "%s: sending ack to mother failed: %s",
              function, strerror(errno));

      SHMEM_UNUSE(CRULE_OR_HRULE(neg),
                  &cinfo,
                  sockscf.shmemfd,
                  SHMEM_ALL);
   }

#if HAVE_NEGOTIATE_PHASE
   socks_freebuffer(neg->s);
#endif /* HAVE_NEGOTIATE_PHASE */

   close(neg->s);
   bzero(neg, sizeof(*neg));
   proctitleupdate();
}

static int
neg_fillset(set, skipcompleted, skipinprogress)
   fd_set *set;
   const int skipcompleted;
   const int skipinprogress;
{
   size_t i;
   int max;

   FD_ZERO(set);
   for (i = 0, max = -1; i < negc; ++i) {
      if (!negv[i].allocated)
         continue;

#if !HAVE_NEGOTIATE_PHASE
      SASSERTX(negv[i].negstate.complete);
#endif /* HAVE_NEGOTIATE_PHASE */

      if (skipcompleted && negv[i].negstate.complete)
         continue;

      if (skipinprogress && !negv[i].negstate.complete)
         continue;

      FD_SET(negv[i].s, set);
      max = MAX(max, negv[i].s);
   }

   return max;
}

static void
neg_clearset(neg, set)
   sockd_negotiate_t *neg;
   fd_set *set;
{

   FD_CLR(neg->s, set);
}

static sockd_negotiate_t *
neg_getset(set)
   fd_set *set;
{
   size_t i;

   for (i = 0; i < negc; ++i) {
      if (!negv[i].allocated)
         continue;

      if (FD_ISSET(negv[i].s, set))
         return &negv[i];
   }

   return NULL;
}

static size_t
neg_allocated(void)
{
   size_t i, alloc;

   for (i = 0, alloc = 0; i < negc; ++i)
      if (negv[i].allocated)
         ++alloc;

   return alloc;
}

static size_t
neg_completed(count)
   const size_t count;
{
   size_t i, completec;

   for (i = 0, completec = 0; i < negc; ++i)
      if (negv[i].allocated && negv[i].negstate.complete)
         if (++completec >= count)
            return completec;

   return completec;
}

static void
proctitleupdate(void)
{

   setproctitle("%s: %lu/%lu",
                childtype2string(sockscf.state.type),
                (unsigned long)neg_allocated(),
                (unsigned long)SOCKD_NEGOTIATEMAX);
}

static void
logdisconnect(s, neg, op, src, dst, buf, buflen)
   const int s;
   sockd_negotiate_t *neg;
   const operation_t op;
   const iologaddr_t *src;
   const iologaddr_t *dst;
   const char *buf;
   const size_t buflen;
{

   if (neg->crule.log.tcpinfo) {
      int fdv[] = { neg->s };

      neg->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
   }

   iolog(&neg->crule,
         &neg->state,
         op,
         src,
         dst,
         NULL,
         NULL,
         buf,
         buflen);

#if HAVE_SOCKS_HOSTID
   if (neg->hrule_isset) {
      if (neg->state.tcpinfo == NULL && neg->hrule.log.tcpinfo) {
         int fdv[] = { neg->s };

         neg->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
      }

      iolog(&neg->hrule,
            &neg->state,
            op,
            src,
            dst,
            NULL,
            NULL,
            buf,
            buflen);
   }
#endif /* HAVE_SOCKS_HOSTID */

   neg->state.tcpinfo = NULL;
}


static struct timeval *
neg_gettimeout(timeout)
   struct timeval *timeout;
{
#if HAVE_NEGOTIATE_PHASE
   const char *function = "neg_gettimeout()";
   time_t timenow;
   size_t i;
   int havetimeout;

   havetimeout = 0;
   time_monotonic(&timenow);

   for (i = 0; i < negc; ++i) {
      if (!negv[i].allocated)
         continue;

      if (CRULE_OR_HRULE(&negv[i])->timeout.negotiate == 0)
         continue;

      if (havetimeout)
         timeout->tv_sec
         = MAX(0,
               MIN(timeout->tv_sec,
                   socks_difftime(CRULE_OR_HRULE(&negv[i])->timeout.negotiate,
                                  socks_difftime(timenow,
                                   negv[i].state.time.negotiatestart.tv_sec))));
      else {
         timeout->tv_sec
         = MAX(0,
               socks_difftime(CRULE_OR_HRULE(&negv[i])->timeout.negotiate,
                              socks_difftime(timenow,
                                    negv[i].state.time.negotiatestart.tv_sec)));
         havetimeout = 1;
      }
   }

   if (!havetimeout)
      timeout = NULL;
   else {
      /*
       * never mind sub-second accuracy, but be sure we don't end up with
       * {0, 0}.
       */
      timeout->tv_usec = 999999;
   }

#else /* !HAVE_NEGOTIATE_PHASE */
   timeout = NULL;
#endif /* !HAVE_NEGOTIATE_PHASE */

   return timeout;
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
   time_t tnow;
   unsigned long days, hours, minutes, seconds;
   size_t i;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   seconds = (unsigned long)socks_difftime(time_monotonic(&tnow),
                                           sockscf.stat.boot);

   seconds2days(&seconds, &days, &hours, &minutes);

   slog(LOG_INFO, "negotiate-child up %lu day%s, %lu:%.2lu:%.2lu",
                  days, days == 1 ? "" : "s", hours, minutes, seconds);

   for (i = 0; i < negc; ++i) {
      char srcstring[MAX_IOLOGADDR], *tcpinfo;

      if (!negv[i].allocated)
         continue;

      build_addrstr_src(HAVE_SOCKS_HOSTID ? &negv[i].state.hostid : NULL,
                        &negv[i].negstate.src,
                        NULL,
                        NULL,
                        &negv[i].negstate.dst,
                        &negv[i].cauth,
                        NULL,
                        srcstring,
                        sizeof(srcstring));

      if (CRULE_OR_HRULE(&negv[i])->log.tcpinfo) {
         int fdv[] = { negv[i].s };

         tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
      }
      else
         tcpinfo = NULL;

      slog(LOG_INFO,
           "%s: negotiating for %lds"
           "%s%s%s",
           srcstring,
           (long)socks_difftime(tnow, negv[i].state.time.negotiatestart.tv_sec),
           tcpinfo == NULL ? "" : "\n",
           tcpinfo == NULL ? "" : "   TCP_INFO:\n",
           tcpinfo == NULL ? "" : tcpinfo);
   }

   SIGNAL_EPILOGUE(sig, si, errno_s);
}

#if HAVE_NEGOTIATE_PHASE
static sockd_negotiate_t *
neg_gettimedout(const struct timeval *tnow)
{
   size_t i;

   for (i = 0; i < negc; ++i) {
      if (!negv[i].allocated
      || CRULE_OR_HRULE(&negv[i])->timeout.negotiate == 0)
         continue;

      if (socks_difftime(tnow->tv_sec, negv[i].state.time.negotiatestart.tv_sec)
      >= CRULE_OR_HRULE(&negv[i])->timeout.negotiate)
         return &negv[i];
   }

   return NULL;
}
#endif /* HAVE_NEGOTIATE_PHASE */
