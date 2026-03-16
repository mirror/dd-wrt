
/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2020,
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
#include "config_parse.h"

static const char rcsid[] =
"$Id: sockd_io.c,v 1.1229.4.4.2.5.4.2.4.6 2024/11/23 22:41:09 michaels Exp $";

/*
 * IO-child:
 * Accept io objects from mother and do io on them.  Unless
 * Covenant, we never send back ancillary data, only ordinary data.
 *
 * XXX remove io_allocated()?  Add some variables instead that we
 * always keep updated.
 */

#if HAVE_UDP_SUPPORT

#define UDP_INITIALCLIENTCOUNT (16)   /*
                                       * Number of UDP clients to allocate
                                       * memory for initially.
                                       */

#endif /* HAVE_UDP_SUPPORT */

#if BAREFOOTD
#define RAW_SOCKETBUFFER   (100 * 1000) /* minsize for raw socket buffer. */
#endif /* BAREFOOTD  */

static void siginfo(int sig, siginfo_t *si, void *sc);
static void proctitleupdate(void);

static int
io_connectisinprogress(const sockd_io_t *io);
/*
 * Returns true if "io" belongs to a connect whos current state is marked
 * as being in progress, but does not check whether it's status has changed
 * since last time state was updated.
 * Returns false otherwise.
 */

static size_t
io_allocated(size_t *tcp_io, size_t *tcp_fd, size_t *udp_io, size_t *udp_fd);
/*
 * If "tcp_io" is not NULL, on return it will contain the number of i/os
 * allocated for tcp.
 *
 * If "udp_io" is not NULL, on return it will contain the number of i/os
 * allocated for udp.
 *
 * Likewise for tcp_fd and udp_fd, though in this case, it's the number
 * of fds in use for tcp and udp, rather than the number of i/o-objects.
 *
 * Returns the number of allocated (active) ios in total (udp and tcp).
 */

static sockd_io_t *
io_getset(const int nfds, const fd_set *set);
/*
 * Goes through our i/o list until it finds an io object where at least one
 * of the descriptors in "set" is set.  "nfds" gives the number of
 * descriptors in "set" to check
 *
 * Returns NULL if none found.
 *
 * Special notes for Barefoot:
 *    Does not go through io.dstv, because that is never used by callers
 *    of this function.
 */

static sockd_io_t *
io_finddescriptor(int d);
/*
 * Finds the io object where one of the descriptors matches "fd".
 */

static int
io_fillset(fd_set *set, int antiflags, fd_set *antiflags_set,
           struct timeval *bwoverflowtil);
/*
 * Sets all descriptors from our list, in "set".
 *
 * If "antiflags" is set, ios with any of the flags in "antiflags" set
 * will be excluded from "set", but set in "antiflags_set" instead.
 * If "antiflags" is not set, antiflags_set may be NULL.
 *
 * ios with state.fin_received set, ios that have not finished connecting,
 * and ios that have overflown the bandwidth limit, will not be set in any
 * set.
 *
 * If any ios were excluded due to having overflown the bandwidth limit,
 * the earliest time we can again do i/o over one of the bandwidth-excluded
 * ios will be stored in "bwoverflowtil", if not NULL.
 *
 * Returns the highest descriptor in our list, or -1 if we don't
 * have any descriptors we want to select() on currently.
 */

static int
io_fillset_connectinprogress(fd_set *set);
/*
 * Similar to io_fillset(), but fills "set" only with descriptors belonging
 * to connects that are marked as still being in progress.
 *
 * In addition, "set" may be NULL, in which case it can be used to simply
 * check whether there are *any* connects in progress (return code will
 * be the fd of the first connect in progress found).
 */

static void
io_clearset(const sockd_io_t *io, const int clearalltargets, fd_set *set);
/*
 * Clears all file descriptors in the i/o object "io" from set "set".
 *
 * If "clearalltargets" is set, the function also clears any fds
 * from the array io->dst.dstv, rather than just io->dst.s.
 */


#if HAVE_UDP_SUPPORT

/*
 * In Dante's case, we only use this for forwarding icmp errors.
 *
 * In Barefoot's case we also use it to listen for icmp errors so that
 * we can more quickly remove sessions that are (presumably) no longer in
 * use.  We are much more aggressive about this in Barefoot's case because
 * we have no control-connection we can use to check whether the session
 * is still active or not, and we want to remove expired session at the
 * first error.
 */
int rawsocket = -1;

#endif /* HAVE_UDP_SUPPORT */

static int
io_timeoutispossible(const sockd_io_t *io);
/*
 * Returns true if it's possible the io object "io" could time out, i.e.,
 * the config and state of the io object is such that it is possible.
 *
 * Returns false if it is not possible for the i/o object to time out
 * in it's current state with the current config.
 */

static time_t
io_timeuntiltimeout(sockd_io_t *io, const struct timeval *tnow,
                  timeouttype_t *type, const int doudpsync);
/*
 * "tnow" is the current time.
 * "type", if not NULL, is filled in with the type of timeout that will
 * occur at that time, if any.
 *
 * Returns the number of seconds til the io object "io" will timeout.
 *
 * 0 if the timeout has already been reached, or
 * -1 if no timeout on the io is currently set.
 *
 * Special notes for Barefoot:
 *    If "io" belongs to a udp-session and "doudpsync" is set, the function
 *    will sync "io" with the udpclient struct belonging to the udp session
 *    that has timed out.
 */

static struct timeval *
io_gettimeout(struct timeval *timeout);
/*
 * If there is an applicable timeout on the current clients for how long
 * we should wait for them to do i/o again, this function fills in "timeout"
 * with the time remaining until then.
 *
 * Returns:
 *      If there is a timeout: timeout.
 *      If there is no applicable timeout currently: NULL.
 */

static sockd_io_t *
io_gettimedout(void);
/*
 * Scans all clients for one that has timed out according to sockscf
 * settings.
 *
 * Returns:
 *      If timed out client found: pointer to timed out i/o object.
 *      Else: NULL.
 */

static int
getnewios(void);
/*
 * Receives new ios from mother.
 * Returns the number of new ios received, or -1 on error.
 */

static void
freebuffers(const sockd_io_t *io);
/*
 * Frees buffers, if any, used by "io".
 */

static int
connectstatus(sockd_io_t *io, int *badfd);
/*
 * Checks if the socket on "io->dst" has finished connecting, and fills
 * in status flags as appropriate.  Note that this function should only
 * be called once the connect has completed (successfully or otherwise).
 *
 * Note that this function must be called after the connect has completed,
 * as in the socks case (and some covenant cases) we need to send a
 * response back to the client before it will start sending us data.
 * We can thus not delay calling this function til we get ordinary i/o
 * from one side, as it's possible none will be coming til after we
 * have sent the response to the client.
 *
 * Returns 0 if the socket connected successfully.
 * Returns -1 if the socket was not connected successfully, or some other error
 *            occurred.  In this case, "badfd" has the value of the "bad" fd,
 *            otherwise it will be -1.
 */

#if SOCKS_SERVER

/*
 * instead of including memory for this as part of the i/o object, we
 * set up the pointer from the i/o object to the appropriate index into
 * this array when we receive the i/o object.  We can do that since
 * we only use the io.sreplyrule object in this process.
 * The only reason for going through with this hassle is so we can reduce
 * the size of the i/o object.  Since the i/o object is passed around between
 * processes, we want it to be as small as possible, reducing the min-size
 * of the socket buffers between mother and child.
 *
 * The rules are needed because while the original io.crule and io.rule
 * are used to establish the session, we also need to do a rulespermit()
 * on a per-packet basis (except in Dante when we have connected to the
 * destination).
 */
static rule_t fwdrulev[SOCKD_IOMAX];
static rule_t replyrulev[SOCKD_IOMAX];

/*
 * Each udpsession can have up to two target sockets.  One for IPv4 and
 * one for IPv6.
 */
udptarget_t udptargetv[SOCKD_IOMAX][2];

#endif /* SOCKS_SERVER */

/* calls io_cleartcpset() on all fd_sets. */
#define IO_CLEAR_ALL_SETS(io, clearalltargets)         \
do {                                                   \
   io_clearset((io), (clearalltargets), buffwset);     \
   io_clearset((io), (clearalltargets), bufrset);      \
   io_clearset((io), (clearalltargets), newrset);      \
   io_clearset((io), (clearalltargets), rset);         \
   io_clearset((io), (clearalltargets), tmpset);       \
   io_clearset((io), (clearalltargets), udprset);      \
   io_clearset((io), (clearalltargets), wset);         \
   io_clearset((io), (clearalltargets), xset);         \
} while (/* CONSTCOND */ 0)

sockd_io_t   iov[SOCKD_IOMAX];   /* each child has these ios. */
const size_t ioc = ELEMENTS(iov);
iostate_t iostate;

/*
 * if not 0, we have "overflowed" according to max bandwidth configured.
 * We can not attribute it to any given client though, so we penalize
 * all by delaying a little.  This object gives the earliest time at which we
 * can again do i/o over one of the object that has overflown it's bandwidth
 * limit.
 */
static struct timeval bwoverflowtil;

void
run_io()
{
   const char *function = "run_io()";
   struct sigaction sigact;
   fd_set *rset, *wset, *xset, *newrset, *tmpset, *bufrset, *buffwset, *udprset,
          *zeroset;
   int p, mayhavetimedout;
#if DIAGNOSTIC && 0 /* XXX not fully tested yet. */
   size_t freefds_initially, logfds_initially;
#endif /* DIAGNOSTIC */

   bzero(&sigact, sizeof(sigact));
   sigact.sa_flags     = SA_RESTART | SA_NOCLDSTOP | SA_SIGINFO;
   sigact.sa_sigaction = siginfo;

#if HAVE_SIGNAL_SIGINFO
   if (sigaction(SIGINFO, &sigact, NULL) != 0)
      serr("%s: sigaction(SIGINFO)", function);
#endif /* HAVE_SIGNAL_SIGINFO */

   /* same handler, for systems without SIGINFO. */
   if (sigaction(SIGUSR1, &sigact, NULL) != 0)
      serr("%s: sigaction(SIGUSR1)", function);

#if HAVE_UDP_SUPPORT
   sockd_priv(SOCKD_PRIV_NET_ICMPACCESS, PRIV_ON);
   if ((rawsocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
      slog(BAREFOOTD ? LOG_NOTICE : LOG_DEBUG,
           "%s: could not open raw socket for improved UDP compatibility.  "
           "Usually root privileges are required for this",
           function);
   else
      slog(LOG_DEBUG, "%s: created raw socket, fd %d", function, rawsocket);
   sockd_priv(SOCKD_PRIV_NET_ICMPACCESS, PRIV_OFF);

   if (rawsocket != -1)
      if (setnonblocking(rawsocket, "rawsocket") == -1)
         serr("%s: could not make rawsocket non-blocking", function);

#if HAVE_PRIVILEGES
   /* don't need this privilege any more, permanently loose it. */

   if (sockscf.state.haveprivs) {
      priv_delset(sockscf.privileges.privileged, PRIV_NET_ICMPACCESS);
      if (setppriv(PRIV_SET, PRIV_PERMITTED, sockscf.privileges.privileged)
      != 0)
         swarn("%s: setppriv() to relinquish PRIV_NET_ICMPACCESS failed",
               function);
   }
#endif /* HAVE_PRIVILEGES */

#if BAREFOOTD  /* only Barefoot reads from the raw socket. */
   if (rawsocket != -1) {
      socklen_t optlen = sizeof(p);
      if (getsockopt(rawsocket, SOL_SOCKET, SO_RCVBUF, &p, &optlen) != 0)
         swarn("%s: getsockopt(SO_RCVBUF)", function);
      else {
         if (p < RAW_SOCKETBUFFER) {
            p = RAW_SOCKETBUFFER;
            if (setsockopt(rawsocket, SOL_SOCKET, SO_RCVBUF, &p, sizeof(p))
            != 0)
               swarn("%s: failed setsockopt(SO_RCVBUF, %d) on raw socket",
               function, p);
            else
               slog(LOG_DEBUG, "%s: changed buffer size to %d bytes",
               function, p);
         }
         else
            slog(LOG_DEBUG, "%s: default buffer size is %d bytes, keeping it",
            function, p);
      }
   }
#endif /* BAREFOOTD */
#endif /* HAVE_UDP_SUPPORT */

   buffwset = allocate_maxsize_fdset();
   bufrset  = allocate_maxsize_fdset();
   newrset  = allocate_maxsize_fdset();
   rset     = allocate_maxsize_fdset();
   tmpset   = allocate_maxsize_fdset();
   udprset  = allocate_maxsize_fdset();
   wset     = allocate_maxsize_fdset();
   xset     = allocate_maxsize_fdset();
   zeroset  = allocate_maxsize_fdset();

   FD_ZERO(zeroset);

   proctitleupdate();

   iostate.freefds = (size_t)freedescriptors(NULL, NULL);

   mayhavetimedout = 0;

   sockd_print_child_ready_message(iostate.freefds);

#if DIAGNOSTIC && 0 /* XXX not fully tested yet. */
   freefds_initially = iostate.freefds;
   logfds_initially
   =   sockscf.log.filenoc    + (sockscf.log.type    & LOGTYPE_SYSLOG)
     + sockscf.errlog.filenoc + (sockscf.errlog.type & LOGTYPE_SYSLOG);
#endif /* DIAGNOSTIC */

   /* CONSTCOND */
   while (1) {
      /*
       * The heart and soul of the server.  This is the loop where
       * all i/o is done and involves some tricky stuff.
       *
       * We need to check for write separately to avoid busy-looping.
       * The problem is that if the descriptor is ready for reading but
       * the corresponding descriptor to write out on is not ready, we will
       * be busy-looping; above select will keep returning descriptors set,
       * but we will not be able to write (and thus won't read) them.
       *
       * Our solution to this is two select(2) calls.  One to see
       * what descriptors are readable, and another select(2) call to
       * block until at least one of the descriptors on the corresponding
       * write-side has become writable.
       * We therefore only set in wset the descriptors that have the
       * corresponding read descriptor readable, so that when the
       * second select() returns, the io objects we get from wset will
       * be both readable and writable.
       *
       * XXX Now that we have the iobuffers, perhaps we can improve on the
       * above by not bothering with the select(2) for write as long as
       * there is some room in the write buffer?
       *
       * Another problem is that if while we wait for writability, a new
       * descriptor becomes readable, we thus can't block forever here.
       * We solve this by in the second select() also checking for
       * readability, but now only the descriptors that were not found
       * to be readable in the previous select().
       * This means that a positive return from the second select does not
       * necessarily indicate we have i/o to do, but it does mean we
       * either have it or a new descriptor became readable; in either
       * case, something has happened.
       * Reason we do not check for exceptions in this second select is that
       * there is nothing we do about them until the descriptor becomes
       * readable too, thus any new exceptions will be in newrset before
       * we have reason to care about them.
       */
      iostatus_t iostatus;
      sockd_io_t *io;
      struct timeval timeout, *timeoutpointer;
      int i, bits, first_rbits, rbits, wbits, udprbits,
          newsocketsconnected, badfd;

#if DIAGNOSTIC && 0 /* XXX not fully tested yet. */
      size_t tcpfd, udpfd, freefds_now, logfds_now;

      io_allocated(NULL, &tcpfd, NULL, &udpfd);

      logfds_now
      =   sockscf.log.filenoc    + (sockscf.log.type    & LOGTYPE_SYSLOG)
        + sockscf.errlog.filenoc + (sockscf.errlog.type & LOGTYPE_SYSLOG);

      freefds_now = (size_t)freedescriptors(NULL, NULL);

      SASSERTX(freefds_now - logfds_now
      >=       freefds_initially - logfds_initially - (tcpfd + udpfd));
#endif /* DIAGNOSTIC */

      errno = 0; /* reset for each iteration. */

      /* look for timed-out clients and calculate the next timeout, if any. */
      if (mayhavetimedout) {
         while ((io = io_gettimedout()) != NULL) {
#if HAVE_NEGOTIATE_PHASE
            if (io_connectisinprogress(io)
            && (SOCKS_SERVER || io->reqflags.httpconnect)) {
               response_t response;

               create_response(NULL,
                               &io->src.auth,
                               io->state.proxyprotocol,
                               (int)sockscode(io->state.proxyprotocol,
                                              SOCKS_TTLEXPIRED),
                               &response);

               if (send_response(io->src.s, &response) != 0)
                  errno = 0; /* real error is the timeout. */
            }
#endif /* HAVE_NEGOTIATE_PHASE */

            io_delete(sockscf.state.mother.ack, io, -1, IO_TIMEOUT);
         }
      }

      mayhavetimedout = 0;
      rbits           = io_fillset(rset, MSG_OOB, xset, &bwoverflowtil);

      /*
       * buffwset.  What descriptors do we want to check for having data
       * buffered for write?  Having data buffered for write means we have
       * data to write on them, thus we want to know if they are writable.
       *
       * Pretty much any client-related descriptor we want to check for
       * having data buffered for write, except those specifically
       * skipped (due to e.g., bw overflow).
       */
      FD_COPY(buffwset, rset);

      /* likewise for having data buffered for read. */
      FD_COPY(bufrset, rset);

      if (sockscf.state.mother.s != -1) {
         FD_SET(sockscf.state.mother.s, rset);
         rbits = MAX(rbits, sockscf.state.mother.s);

         /* checked so we know if mother goes away.  */
         FD_SET(sockscf.state.mother.ack, rset);
         rbits = MAX(rbits, sockscf.state.mother.ack);
      }
      else { /* no mother.  Do we have any other descriptors to work with? */
         if (rbits == -1 && !timerisset(&bwoverflowtil)) {
            /* no clients in fd_sets, and not due to bwoverflow ... */
            SASSERTX(io_allocated(NULL, NULL, NULL, NULL) == 0);

            slog(LOG_DEBUG, "%s: no connection to mother, no clients; exiting",
                 function);

#if HAVE_VALGRIND_VALGRIND_H
         if (RUNNING_ON_VALGRIND) {
            /* for some reason Valgrind complains the rset pointer is lost. */
            free(rset);
         }
#endif /* HAVE_VALGRIND_VALGRIND_H */

            sockdexit(EXIT_SUCCESS);
         }
      }

#if BAREFOOTD
      if (rawsocket != -1 && (io_udpclients(ioc, iov, 1) > 0)) {
         /* raw socket is only of interest if we have udp clients. */

         FD_SET(rawsocket, rset);
         rbits = MAX(rbits, rawsocket);
      }
#endif /* BAREFOOTD */

      /*
       * In the first select(2) we check for descriptors that are readable;
       * we won't write if we can't read.
       *
       * Connects that are in progress is a special case that we also need
       * to check for here.  Once the connect(2) has completed, successfully
       * or not, the socket will become writable and we may need to send a
       * status response (if there is a negotiate phase) to the client.
       *
       * Also select(2) for exceptions so we later can tell the i/o function
       * if there's one pending.
       */

      wbits               = io_fillset_connectinprogress(wset);
      newsocketsconnected = 0;
      bits                = MAX(rbits, wbits) + 1;

      slog(LOG_DEBUG, "%s: first select; readable/connected?", function);
      switch (selectn(bits,
                      rset,
                      bufrset,
                      buffwset,
                      wset,
                      xset,
                      io_gettimeout(&timeout))) {
         case -1:
            SASSERT(ERRNOISTMP(errno));
            continue;

         case 0:
            mayhavetimedout = 1;
            continue; /* restart the loop. */
      }

      if (sockscf.state.mother.ack != -1
      && FD_ISSET(sockscf.state.mother.ack, rset)) { /* only eof expected. */
         sockd_readmotherscontrolsocket(function, sockscf.state.mother.ack);
         sockscf.state.mother.s = sockscf.state.mother.ack = -1;

#if BAREFOOTD
         /*
          * terminate all udp sessions as if we do not, a restart of mother
          * will not be able to rebind the ports used.
          * Not a problem for TCP, so those sessions can continue to run
          * until the session ends for other reasons.
          */
         io_remove_session(ioc, iov, NULL, SOCKS_UDP, IO_ADMINTERMINATION);
         proctitleupdate();

#else /* !BAREFOOTD */
         /*
          * this process can continue independent of mother as long as it
          * has clients, because each client has it's own unique
          * udp socket on the client-side also.
          */
#endif /* !BAREFOOTD */

         /*
          * safest to regenerate the fd_sets for this once in a life-time
          * event.
          */
         continue;
      }

      /*
       * this needs to be after check of ack-pipe to limit error messages,
       * because the ack-pipe is a stream pipe, so hopefully we will handle
       * the EOF from mother on the ack-pipe before we get the error on
       * the data-pipe.
       */
      if (sockscf.state.mother.s != -1
      && FD_ISSET(sockscf.state.mother.s, rset)) {
         if (getnewios() == -1)
            continue; /* loop around and check control connection again. */
         else {
            proctitleupdate();
            continue; /* need to scan rset again; should have a new client. */
                      /*
                       * XXX Or can we instead add it to newrset, and rescan as
                       * normal after that?
                       */
         }
      }

      first_rbits = bits;

      /*
       * First check if any new connect(2)s  to targets have finished.
       */
      if (FD_CMP(zeroset, wset) != 0) {
         for (p = 0; p < bits; ++p) {
            if (FD_ISSET(p, wset)) {

               io = io_finddescriptor(p);
               SASSERTX(io != NULL);
               SASSERTX(p == io->dst.s);

               if (connectstatus(io, &badfd) == 0)
                  ++newsocketsconnected;
               else {
                  SASSERTX(badfd != -1);
                  SASSERTX(badfd == io->src.s
                  ||       badfd == io->dst.s
                  ||       badfd == io->control.s);

                  IO_CLEAR_ALL_SETS(io, 1);
                  io_delete(sockscf.state.mother.ack, io, badfd, IO_IOERROR);
               }
            }
         }

         if (newsocketsconnected > 0)
            proctitleupdate();

         slog(LOG_DEBUG, "%s: %d new socket%s finished connecting",
              function,
              newsocketsconnected,
              newsocketsconnected == 1 ? "" : "s");
      }

      /*
       * Add bufrset to rset, so rset now contains all sockets we can
       * read from, whether it's from the socket or from our local buffer.
       */
      fdsetop(bits, '|', rset, bufrset, rset);

#if BAREFOOTD
      if (rawsocket != -1 && FD_ISSET(rawsocket, rset)) {
         FD_CLR(rawsocket, rset);

         if (rawsocket_recv(rawsocket, ioc, iov) == RAWSOCKET_IO_DELETED)
            /*
             * one or more ios were deleted.  Don't know which ones, so
             * need to regenerate the descriptor sets for select.
             */
            continue;
      }
#endif /* BAREFOOTD */

      /*
       * We now know what descriptors are readable; rset.
       *
       * Next prepare for the second select(2), where we want to
       * know which of the descriptors, paired with the above readable
       * descriptors, we can write to.  In that select(2) we also need to
       * check for read again, but only those descriptors that are not
       * already readable, as that constitutes at least a status change
       * which we should loop around for.  Likewise, we again need to
       * check whether some new sockets have finished connecting.
       */

      i     = io_fillset(tmpset, 0, NULL, &bwoverflowtil);
      rbits = fdsetop(i, '^', rset, tmpset, newrset);

      if (sockscf.state.mother.s != -1) { /* mother status may change too. */
         FD_SET(sockscf.state.mother.s, newrset);
         rbits = MAX(rbits, sockscf.state.mother.s);

         /* checked so we know if mother goes away.  */
         SASSERTX(sockscf.state.mother.ack != -1);
         FD_SET(sockscf.state.mother.ack, newrset);
         rbits = MAX(rbits, sockscf.state.mother.ack);
      }

#if BAREFOOTD
      /*
       * Checked before, and checked again now, as it's status too may
       * change and it may become readable (again).
       */
      if (rawsocket != -1) {
         FD_SET(rawsocket, newrset);
         rbits = MAX(rbits, rawsocket);
      }
#endif /* BAREFOOTD */

      /*
       * Use a separate set to to store all udp fds that should be writable.
       * We don't bother actually checking udp sockets for writability
       * because if the udp write ends up failing, it wouldn't make any
       * difference whether the socket was marked as writable or not; for
       * all we know it's "writability" could have have been limited to a
       * one byte write/packet, while the corresponding packet read was much
       * larger, in which case our write could have failed anyway.
       */
      FD_ZERO(udprset);
      udprbits = -1;

      /*
       * descriptors to check for writability:
       * - those with the corresponding read-descriptor set.
       * - those with data already buffered for write.
       * - the connects that are still in progress.
       *
       * Initialize with the set of connects still in progress, and then add
       * those fds that have the corresponding other side readable.
       */
      wbits = io_fillset_connectinprogress(wset);

      for (p = 0; p < MAX(bits, first_rbits); ++p) {
         int sockettoset;

         if (FD_ISSET(p, buffwset)) {
            /*
             * Descriptor has data buffered for write.  That means we should
             * mark the other side as readable.  Regardless of whether we
             * can read from the other side or not at the moment, we have
             * data that we previously read from it which which we need to
             * forward to the other side.
             */
            int other_side;

            io = io_finddescriptor(p);

            SASSERTX(io != NULL);
            SASSERTX(socks_bufferhasbytes(p, WRITE_BUF));
            SASSERTX(io->state.protocol == SOCKS_TCP); /* we only buffer tcp. */

            if (p == io->src.s)
               other_side = io->dst.s;
            else if (p == io->dst.s)
               other_side = io->src.s;
            else
               SERRX(p);

            slog(LOG_DEBUG,
                 "%s: fd %d has data buffered for write.  Checking it for "
                 "writability and marking other side, fd %d, as readable",
                 function, p, other_side);

            FD_SET(other_side, rset);
            rbits = MAX(other_side, rbits);

            /*
             * ok, we know we have data buffered for write, but /can/
             * we write?  For TCP, need to check.
             *
             * XXX possible optimization target: if we have enough room
             *     in the writebuffer, we can pretend the fd is writable
             *     as long as we do not read too much (gssapi encapsulation
             *     included).  As of now, we just use the buffer to even
             *     out differences between the two sides, but if one side
             *     stops reading completely, the fd will not be writable
             *     and we won't try to write anything, not even to our own
             *     buffer even though that might be possible.  No big deal.
             */

            FD_SET(p, wset);
            wbits = MAX(wbits, p);
         }
         else {
            /*
             * No data buffered for write.  Check is the socket is readable,
             * from the buffer or from the socket itself.
             */
            io = NULL;
         }

         if (!FD_ISSET(p, rset))
            continue; /* socket is not readable. */

         /*
          * Have data to read.  Figure out from what and to which socket.
          */

         if (io == NULL)
            io = io_finddescriptor(p);

         SASSERTX(io != NULL);

         /*
          * find out what corresponding socket we should check for writability.
          */

         sockettoset = -1;

         /*
          * In the case of udp, we have a one-to-many (many = 2 in the socks
          * case) scenario, where packets received on "in" can go to many
          * different "outs." and we don't know which out socket to use until
          * we have read the packet from the client.
          *
          * UDP sockets shouldn't normally block though, so selecting
          * for writability is not something we care about it this case.
          *
          * The reverse, when a packet comes in on one of the out sockets
          * is slightly more complicated. To detect that we need to select(2)
          * for readability on all the target/dst sockets.  This is handled
          * as usual in io_fillset().
          */

         if (p == io->src.s || p == io->dst.s) {
            if (io->state.protocol == SOCKS_UDP) {
               /*
                * Just set this fd in udprset.  We don't bother actually
                * select(2)'ing on the corresponding udp socket for
                * writability.
                */

               sockettoset = p;

               FD_SET(sockettoset, udprset);
               udprbits = MAX(udprbits, sockettoset);
            }
            else {
               /*
                * For TCP-sockets we need to know whether we can write or
                * not to the other (non-read) side.
                */

               if (p == io->src.s) {
                  /*
                   * read from src (client) requires writable out (target).
                   */
                  sockettoset = io->dst.s;
               }
               else {
                  SASSERTX(p == io->dst.s);
                  /*
                   * read from dst (target) requires writable in (client).
                   */
                  sockettoset = io->src.s;
               }

               SASSERTX(sockettoset != -1);

               FD_SET(sockettoset, wset);
               wbits = MAX(wbits, sockettoset);
            }
         }
#if HAVE_CONTROL_CONNECTION
         else {
            /*
             * control connection is also readable without matching
             * writable and is used to signal session close in udp and
             * bind extension cases.
             * Since it doesn't have any side to forward the data to
             * it is simpler to handle it here and now.
             */
            ssize_t r;
            char buf[1024];

            SASSERTX(io->control.s == p);
            SASSERTX(io->control.s != io->src.s);
            SASSERTX(io->control.s != io->dst.s);

            sockettoset = io->control.s;

            SASSERTX(io->state.command  == SOCKS_UDPASSOCIATE
            ||       (io->state.command == SOCKS_BINDREPLY
                 &&   io->state.extension.bind));

            /*
             * Only thing we expect from client's control connection is
             * an eof.
             */
            r = socks_recvfrom(io->control.s,
                               buf,
                               sizeof(buf),
                               0,
                               NULL,
                               NULL,
                               NULL,
                               &io->control.auth);

            if (r <= 0) {
               slog(LOG_DEBUG,
                    "%s: TCP control connection from client %s closed: %s",
                    function,
                    sockaddr2string(&CONTROLIO(io)->raddr, NULL, 0),
                    r == 0 ? "EOF" : strerror(errno));

               if (io->srule.mstats_shmid != 0
               &&  (io->srule.alarmsconfigured & ALARM_DISCONNECT)) {
                  clientinfo_t cinfo;

                  cinfo.from   = io->control.raddr;
                  cinfo.hostid = io->state.hostid; 

                  SASSERTX(!io->control.state.alarmdisconnectdone);
                  alarm_add_disconnect(0,
                                       &io->srule,
                                       ALARM_INTERNAL,
                                       &cinfo,
                                       r == 0 ? "EOF" : strerror(errno),
                                       sockscf.shmemfd);

                  io->control.state.alarmdisconnectdone = 1;
               }

               IO_CLEAR_ALL_SETS(io, 1);

               io_delete(sockscf.state.mother.ack,
                         io,
                         io->control.s,
                         r == 0 ? IO_CLOSE : IO_IOERROR);
            }
            else {
               char visbuf[256];

               slog(LOG_NOTICE, "%s: %ld unexpected byte%s over control "
                                "connection from client %s: %s",
                                function,
                                (long)r,
                                r == 1 ? "" : "s",
                                sockaddr2string(&CONTROLIO(io)->raddr, NULL, 0),
                                str2vis(buf,
                                        (size_t)r,
                                        visbuf,
                                        sizeof(visbuf)));

               FD_CLR(io->control.s, rset);

               FD_SET(io->control.s, newrset);
               rbits = MAX(rbits, io->control.s);
            }
         }
#endif /* HAVE_CONTROL_CONNECTION */

         SASSERTX(sockettoset != -1);
      }

      if (newsocketsconnected || udprbits > -1) {
         /*
          * Don't wait.  Handle what we can now and restart the loop,
          * which will then include handling of any new sockets.
          */
         timeoutpointer = &timeout;
         bzero(timeoutpointer, sizeof(*timeoutpointer));
      }
      else
         timeoutpointer = io_gettimeout(&timeout);

      ++rbits;
      ++wbits;

      bits = MAX(rbits, wbits);

      if (bits == 0) {
         slog(LOG_DEBUG,
              "%s: no fds to select(2) on ... restarting loop", function);

         continue;
      }

      if (FD_CMP(zeroset, newrset) == 0
      &&  FD_CMP(zeroset, wset)    == 0) {
         slog(LOG_DEBUG,
              "%s: no fds to select(2) on ... restarting loop", function);

         continue;
      }

      slog(LOG_DEBUG, "%s: second select; what is writable?", function);
      switch (selectn(bits, newrset, NULL, NULL, wset, NULL, timeoutpointer)) {
         case -1:
            SASSERT(ERRNOISTMP(errno));
            continue;

         case 0:
            mayhavetimedout = 1;
      }

      if (sockscf.state.mother.ack != -1
      && FD_ISSET(sockscf.state.mother.ack, newrset))
         continue; /* eof presumably, but handle it in one place, above. */

      if (sockscf.state.mother.s != -1
      && FD_ISSET(sockscf.state.mother.s, newrset)) {
         FD_CLR(sockscf.state.mother.s, newrset);
         getnewios();
      }

      /*
       * If newrset has any udp i/os, add them to udprset.
       */
      for (p = 0; p < rbits; ++p) {
         if (!FD_ISSET(p, newrset))
            continue;

#if BAREFOOTD
         /* specialcased as it's not part of any i/o object. */
         if (rawsocket != -1 && p == rawsocket)
            continue;
#endif /* BAREFOOTD */

         io = io_finddescriptor(p);
         SASSERTX(io != NULL);

         if (io->state.protocol == SOCKS_UDP
         &&  (p == io->src.s || p == io->dst.s)) {
            FD_SET(p, udprset);
            udprbits = MAX(udprbits, p);

            /* enough to have it in udprset. */
            FD_CLR(p, newrset);
         }
      }

      /*
       * Status now is as follows:
       *
       * newrset: tcp descriptors newly readable (from the second select).
       *          We don't do anything with them here as we don't know if
       *          the other side is writable, but instead loop around and
       *          check for writability first.
       *
       * rset: descriptors readable, from buffer or from socket.
       *
       * udprset: udp sockets that are readable.
       *
       * wset: descriptors writable with at least one of:
       *          a) a matching descriptor in rset.
       *          b) data buffered for write.
       *          c) a connect that was previously in progress, now completed.
       *
       *       a) and b) we can do i/o over, c) we can't know for sure.
       *
       * xset: subset of rset with exceptions pending.
       *
       * The sockets in udprset and wset are the sockets we can possibly
       * do i/o over.
       *
       * For wset we need check which of a), b), or c) is the case.
       * If it's a) or b), do i/o, remove the socket from wset, and
       * get the next socket set in wset.  If c), we can not do i/o now.
       *
       * For udprset, we have no corresponding write-side we bother checking
       * as we assume we can always write in the udp case.
       */

#if HAVE_UDP_SUPPORT
      /*
       * First handle UDP case; udprset.
       */

      ++udprbits;

      while ((io = io_getset(udprbits, udprset)) != NULL) {
         SASSERTX(io->state.protocol == SOCKS_UDP);

         /*
          * Since udp is a "point-to-multipoint" case, the descriptor in
          * udprset could be:
          * a) the one socket (io->src.s) we use to read from all clients.
          * b) any of the sockets in io->dst.dstv, which we use for
          *    sending/reading data to/from targets.
          *
          * In Barefoot's case, if it's a), there could be a lot of packets
          * queued up, since that one socket will be handling io->dst.dstc
          * number * of clients.  If so, we'll want to read from the socket
          * until there are no more packets, to reduce the chance of packets
          * overflowing the kernels socket buffer and becoming lost.
          * That we don't know how many packets there are in Barefoot's
          * case is no different from Dante's case, but in Barefoot's case
          * there is a good chance there will be many due to the
          * multipoint-to-point nature of things, so read packets until
          * there are no more to read to reduce the chance of packets *
          * being discarded due to the socket buffer filling up.
          *
          * If it's b), we need to find out which one(s) it is and set
          * io->dst.s appropriately, so that doio() reads from the correct
          * socket.  This is however done by the function that fetches us
          * the io above (io_getset()).
          *
          * The thing we need to be careful about is to not clear a
          * fd we have not read from.  If we read from the client-fd,
          * the target-fd should be -1, so we can't read or clear that.
          * If we read from the target-fd however, the client-fd can also
          * be set.  doio() handles this by reading from both sides,
          * if both sides are set, which is good enough for Dante.
          * In Barefoot we have the extra "read until we can read no more"
          * for the client-side however, to reduce the chance of packetloss,
          * which is the main reason for the difference here.
          *
          * XXX how to handle bwoverflow?
          */

#if BAREFOOTD
         if (FD_ISSET(io->src.s, udprset)) {
            /*
             * case a): packet from client to target.
             *
             * Don't know what the destination socket is until we've
             * read the packet and found the destination socket, based
             * on what client sent the packet.  Set it to the dummy socket
             * for now and let the i/o function demux it when the client
             * packet has been read.
             *
             * Try to read as many packets as we can as presumably there is
             * a much greater risk of loosing packets on on the client side,
             * since all the clients send to one address/socket.
             */

            do {
               io->dst.s = -1; /* don't yet know what dst will be. */
               iostatus  = doio_udp(io, udprset, &badfd);

               if (IOSTATUS_FATALERROR(iostatus) && badfd != -1) {
                  if (io->dst.s != -1)
                     FD_CLR(io->dst.s, udprset);

                  io_delete(sockscf.state.mother.ack, io, badfd, iostatus);
               }
            } while (iostatus     != IO_EAGAIN
              &&     io->dst.dstc  > 0);

            if (io->src.s != -1)
               FD_CLR(io->src.s, udprset);
         }
         else {
            /*
             * case b): reply from target to client.
             */
            SASSERTX(io->dst.s != -1);

            /*
             * can't be set as we first checked the client-side of this i/o
             * and then read till there were no more packets to be read.
             */
            SASSERTX(!FD_ISSET(io->src.s, udprset));

            iostatus = doio_udp(io, udprset, &badfd);

            if (io->dst.s != -1)
               FD_CLR(io->dst.s, udprset);

            if (IOSTATUS_FATALERROR(iostatus) && badfd != -1)
               io_delete(sockscf.state.mother.ack, io, badfd, iostatus);
         }

#else /* Dante */

         /*
          * With the advent of IPv6, Dante is also a p-to-mp now, not
          * completely unlike Barefoot, though in Dante's case the "mp"
          * refers to only up to two points per client-side socket,
          * one for ipv4 and one for ipv6.
          * Like for Barefoot, we know what client socket we're going to
          * read the packet from, as there is only one per i/o object,
          * but we don't know what target socket, if any, to write the
          * packet to until we've read the client packet.
          */

         iostatus = doio_udp(io, udprset, &badfd);

         io_clearset(io, IOSTATUS_FATALERROR(iostatus), udprset);

         if (IOSTATUS_FATALERROR(iostatus))
            io_delete(sockscf.state.mother.ack, io, badfd, iostatus);

#endif /* Dante */
      }
#endif /* HAVE_UDP_SUPPORT */

      /*
       * Next; TCP-case in wset.  In the tcp-case, we don't try to
       * read from one side unless we are sure we can write to the
       * other side.
       */

      FD_ZERO(tmpset);
      while ((io = io_getset(bits, wset)) != NULL) {
         int flags;

         SASSERTX(io->state.protocol == SOCKS_TCP);

         if (io->state.command == SOCKS_CONNECT
         && !io->dst.state.isconnected) {

            SASSERTX(FD_ISSET(io->dst.s, wset));
            FD_CLR(io->dst.s, wset);

            if (connectstatus(io, &badfd) == -1) {
               SASSERTX(badfd == io->src.s || badfd == io->dst.s);
               io_delete(sockscf.state.mother.ack, io, badfd, IO_IOERROR);
            }

            /*
             * regardless of whether the connect was successful or not we
             * can't do anything here as we don't know what the status of
             * src.s is, so have to loop around.
             * XXX we could check though --- if we coincidentally know that
             * src is readable, and the connect succeeded, we can do i/o, no?
             */
            continue;
         }

         if (FD_CMP(tmpset, xset) != 0
         && (FD_ISSET(io->src.s, xset) || FD_ISSET(io->dst.s, xset)))
            flags = MSG_OOB;
         else
            flags = 0;

         iostatus = doio_tcp(io, rset, wset, flags, &badfd);

         io_clearset(io, IOSTATUS_FATALERROR(iostatus), rset);
         io_clearset(io, IOSTATUS_FATALERROR(iostatus), wset);

         if (IOSTATUS_FATALERROR(iostatus))
            io_delete(sockscf.state.mother.ack, io, badfd, iostatus);
      }

#if BAREFOOTD
      /*
       * And lastly, raw socket.  Only Barefoot reads from it.
       */
      if (rawsocket != -1 && FD_ISSET(rawsocket, newrset)) {
         FD_CLR(rawsocket, newrset);
         (void)rawsocket_recv(rawsocket, ioc, iov);
      }
#endif /* BAREFOOTD */

      /*
       * XXX optimization target.
       * At this time, newrset should contain mostly the same tcp-sockets
       * that would be returned as writable by the next select(2) ("first
       * select"), no?  It would be very nice if we could somewhat "continue"
       * and use that, which would save us wasting a lot of time repeating
       * the same select(2) which we just performed.
       */
   }
}

void
io_preconfigload(void)
{
   const char *function = "io_preconfigload()";

   slog(LOG_DEBUG, "%s", function);
}

void
io_postconfigload(void)
{
   const char *function = "io_postconfigload()";
   size_t i;

   slog(LOG_DEBUG, "%s: ioc = %ld", function, (long)ioc);

   for (i = 0; i < ioc; ++i) {
      if (!iov[i].allocated)
         continue;

      io_updatemonitor(&iov[i]);

      if (iov[i].state.protocol == SOCKS_TCP)
         continue;

      SASSERTX(iov[i].state.protocol == SOCKS_UDP);

      /*
       * In the UDP case we should check rules on each packet, so indicate
       * we should not reuse the saved rules after sighup, which may have
       * changed the ACLs.
       * XXX ideally we would also do a rulespermit() to remove clients that
       * no longer have any hope if being allowed to pass packets through.
       */

#if !BAREFOOTD
      iov[i].src.state.use_saved_srule = 0;
      iov[i].dst.state.use_saved_srule = 0;

#else /* BAREFOOTD */
      /*
       * Nothing to do.  Those clients that were permitted we continue
       * to permit.
       */

      slog(LOG_DEBUG, "%s: iov #%ld, dstc = %ld",
           function, (long)ioc, (long)iov[i].dst.dstc);
#endif /* BAREFOOTD */
   }

#if BAREFOOTD
#warning "missing code to remove unused internal udp sessions."
/*
 * i.e. sessions belonging to internal addresses we should no longer
 * listen on.  Clients belonging to those sessions must also be
 * removed.  So perhaps all clients that are no longer permitted should
 * also be removed then?
 */
#if 0
         io_remove_session(ioc,
                           iov,
                           &oldinternal.addrv[i].addr,
                           oldinternal.addrv[i].protocol,
                           IO_ADMINTERMINATION);
         proctitleupdate();
#endif
#endif /* BAREFOOTD */
}

void
close_iodescriptors(io)
   const sockd_io_t *io;
{
   const char *function = "close_iodescriptors()";
   size_t i;
   int errno_s, fdv[] = { io->control.s, io->src.s, io->dst.s, -1 };

#if SOCKS_SERVER
   if (io->state.protocol == SOCKS_UDP) {
      /*
       * dst socket may be up to two sockets (ipv4 and ipv6),
       * not just io->dst.s.
       */
      const size_t dstoffset = 2; /* control = 0, src = 1, dst = 2. */

      SASSERTX(io->dst.dstc <= 2);

      for (i = 0; i < io->dst.dstc; ++i)
         fdv[dstoffset + i] = io->dst.dstv[i].s;

      for (i = dstoffset + i; i < ELEMENTS(fdv); ++i)
         fdv[i] = -1;
   }
#endif /* SOCKS_SERVER */

   errno_s = errno;

   for (i = 0; i < ELEMENTS(fdv); ++i) {
      if (fdv[i] != -1) {
         if (close(fdv[i]) == 0) {
            ++iostate.freefds;
            fdv[i] = -1;
         }
         else
            SWARN(fdv[i]);
      }
   }

   errno = errno_s;
}

int
recv_io(s, io)
   int s;
   sockd_io_t *io;
{
   const char *function = "recv_io()";
#if HAVE_GSSAPI
   gss_buffer_desc gssapistate;
   char gssapistatemem[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */
   struct iovec iovecv[2];
   struct timeval tnow;
   struct msghdr msg;
   sockd_io_t tmpio;
   ssize_t received;
   size_t ioi;
   int wearechild, fdexpect, fdreceived, iovecc;
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

   bzero(iovecv, sizeof(iovecv));
   iovecc = 0;

   iovecv[iovecc].iov_base = &tmpio;
   iovecv[iovecc].iov_len  = sizeof(tmpio);
   ++iovecc;

#if HAVE_GSSAPI
   iovecv[iovecc].iov_base = gssapistatemem;
   iovecv[iovecc].iov_len  = sizeof(gssapistatemem);
   ++iovecc;
#endif /* HAVE_GSSAPI */

   bzero(&msg, sizeof(msg));
   msg.msg_iov    = iovecv;
   msg.msg_iovlen = iovecc;
   msg.msg_name   = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   if (io == NULL) /* child semantics; find a free io ourselves. */
      wearechild = 1;
   else
      wearechild = 0;

   slog(LOG_DEBUG, "%s: %s",
        function, wearechild ? "we are child" : "we are mother");

   if ((received = recvmsgn(s, &msg, 0)) < (ssize_t)sizeof(*io)) {
      if (received == -1 && errno == EAGAIN)
         ;
      else
         slog(LOG_DEBUG, "%s: recvmsg(): unexpected short read on socket "
                         "%d (%ld < %lu): %s",
                         function,
                         s,
                         (long)received,
                         (unsigned long)(sizeof(*io)),
                         strerror(errno));

      return -1;
   }

   if (socks_msghaserrors(function, &msg))
      return -1;

   /*
    * ok, received a io.  Find out where to store it.
    */
   ioi = 0;
   if (wearechild) { /* child semantics; find a free io ourselves. */
      SASSERTX(io == NULL);

      for (; ioi < ioc; ++ioi)
         if (!iov[ioi].allocated) {
            io = &iov[ioi];
            break;
         }

      if (io == NULL) {
         /*
          * Only reason this should happen is if mother died/closed connection,
          * and what we are getting now is not actually an i/o, but just an
          * error indicator.
          */
         char buf;

         if (recv(s, &buf, sizeof(buf), MSG_PEEK) > 0)
            /*
             * not an error indicator, but a mismatch between us and mother.
             * Should never happen.
             */
            SERRX(io_allocated(NULL, NULL, NULL, NULL));

         return -1;
      }
   }

   SASSERTX(tmpio.allocated == 0);
   *io = tmpio;

   SASSERTX(io->crule.bw  == NULL);
   SASSERTX(io->crule.ss  == NULL);
   SASSERTX(io->srule.bw  == NULL);
   SASSERTX(io->srule.ss  == NULL);

   /* figure out how many descriptors we are supposed to be passed. */
   switch (io->state.command) {
      case SOCKS_BIND:
      case SOCKS_BINDREPLY:
         if (io->state.extension.bind)
            fdexpect = 3;   /* in, out, control. */
         else
            fdexpect = 2;   /* in and out. */
         break;

      case SOCKS_CONNECT:
         fdexpect = 2;   /* in and out */
         break;

#if HAVE_UDP_SUPPORT
      case SOCKS_UDPASSOCIATE:

#if BAREFOOTD
         fdexpect = 1;   /* in. */

#else /* SOCKS_SERVER */
         fdexpect = 2;   /* in and control. */

#endif /* SOCKS_SERVER */

         break;
#endif /* HAVE_UDP_SUPPORT */

      default:
         SERRX(io->state.command);
   }

   if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
      swarnx("%s: received control message has the invalid len of %d",
              function, (int)CMSG_TOTLEN(msg));

      return -1;
   }

   /*
    * Get descriptors sent us.  Should be at least two.
    */

   SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
   SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

   fdreceived = 0;

   CMSG_GETOBJECT(io->src.s, cmsg, sizeof(io->src.s) * fdreceived++);

   if (io->state.protocol == SOCKS_TCP)
      CMSG_GETOBJECT(io->dst.s, cmsg, sizeof(io->dst.s) * fdreceived++);

#if DIAGNOSTIC
   checksockoptions(io->src.s,
                    io->src.laddr.ss_family,
                    io->state.protocol == SOCKS_TCP ? SOCK_STREAM : SOCK_DGRAM,
                    1);

   if (io->state.protocol == SOCKS_TCP)
      checksockoptions(io->dst.s, io->dst.laddr.ss_family, SOCK_STREAM, 0);
#endif /* DIAGNOSTIC */

#if HAVE_GSSAPI
   gssapistate.value  = gssapistatemem;
   gssapistate.length = received - sizeof(*io);

   if (gssapistate.length > 0)
      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: read gssapistate of size %ld",
              function, (unsigned long)gssapistate.length);
#endif /* HAVE_GSSAPI */

   /* any more descriptors to expect? */
   switch (io->state.command) {
      case SOCKS_BINDREPLY:
#if HAVE_GSSAPI
         if (io->dst.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->dst.auth.mdata.gssapi.state.id,
                                    &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */

         if (io->state.extension.bind) {
            CMSG_GETOBJECT(io->control.s,
                           cmsg,
                           sizeof(io->control.s) * fdreceived++);

#if DIAGNOSTIC
            checksockoptions(io->control.s,
                             io->control.laddr.ss_family,
                             SOCK_STREAM,
                             1);
#endif /* DIAGNOSTIC */

         }
         else
            SASSERTX(io->control.s == -1);
         break;

      case SOCKS_CONNECT:
#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->src.auth.mdata.gssapi.state.id,
            &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */

         SASSERTX(io->control.s == -1);
         break;

      case SOCKS_UDPASSOCIATE:
#if SOCKS_SERVER
         /* LINTED pointer casts may be troublesome */
         CMSG_GETOBJECT(io->control.s,
                        cmsg,
                        sizeof(io->control.s) * fdreceived++);

#if HAVE_GSSAPI
         if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
            if (gssapi_import_state(&io->src.auth.mdata.gssapi.state.id,
            &gssapistate) != 0)
               return -1;
         }
#endif /* HAVE_GSSAPI */

#else /* !SOCKS_SERVER */
         SASSERTX(io->control.s == -1);

#endif /* !SOCKS_SERVER */

         SASSERTX(io->dst.s == -1); /* will be allocated when needed. */
         break;

      default:
         SERRX(io->state.command);
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE) {
#if DIAGNOSTIC
      struct sockaddr_storage addr;
      socklen_t len;
#endif /* DIAGNOSTIC */
      size_t bufused;
      char buf[1024];


      bufused =
      snprintf(buf, sizeof(buf),
               "received %d descriptor(s) for command %d.  Control: fd %d, "
               "src: fd %d, dst: fd %d.  Allocated to iov #%lu.\n",
               fdreceived,
               io->state.command,
               io->control.s,
               io->src.s,
               io->dst.s,
               (unsigned long)ioi);

      bufused +=
      snprintf(&buf[bufused], sizeof(buf) - bufused,
               "src fd %d (%s)", io->src.s, socket2string(io->src.s, NULL, 0));

      bufused +=
      snprintf(&buf[bufused], sizeof(buf) - bufused,
               ", dst fd %d (%s)",
               io->dst.s,
               io->dst.s == -1 ? "N/A" : socket2string(io->dst.s, NULL, 0));

      if (io->control.s != -1) {
         bufused +=
         snprintf(&buf[bufused], sizeof(buf) - bufused,
                  ", control fd %d (%s)",
                  io->control.s, socket2string(io->control.s, NULL, 0));
      }

      slog(LOG_DEBUG, "%s: %s", function, buf);

#if DIAGNOSTIC
      len = sizeof(addr);
      if (getsockname(io->src.s, TOSA(&addr), &len) == 0
      &&  IPADDRISBOUND(&addr))
         SASSERTX(sockaddrareeq(&io->src.laddr, &addr, 0));

      if (io->src.state.isconnected) {
         len = sizeof(addr);
         if (getpeername(io->src.s, TOSA(&addr), &len) == 0
         &&  IPADDRISBOUND(&addr))
            SASSERTX(sockaddrareeq(&io->src.raddr, &addr, 0));
      }

      if (io->dst.s != -1) {
         len = sizeof(addr);
         if (getsockname(io->dst.s, TOSA(&addr), &len) == 0
         &&  IPADDRISBOUND(&addr))
            SASSERTX(sockaddrareeq(&io->dst.laddr, &addr, 0));

         if (io->dst.state.isconnected) {
            len = sizeof(addr);
            if (getpeername(io->dst.s, TOSA(&addr), &len) == 0
            &&  IPADDRISBOUND(&addr))
               SASSERTX(sockaddrareeq(&io->dst.raddr, &addr, 0));
         }
      }

      if (io->control.s != -1) {
         len = sizeof(addr);
         if (getsockname(io->control.s, TOSA(&addr), &len) == 0
         &&  IPADDRISBOUND(&addr))
            SASSERTX(sockaddrareeq(&io->control.laddr, &addr, 0));

         len = sizeof(addr);
         if (getpeername(io->control.s, TOSA(&addr), &len) == 0)
            SASSERTX(sockaddrareeq(&io->control.raddr, &addr, 0));
      }
#endif /* DIAGNOSTIC */
   }

   if (wearechild) { /* only child does i/o, wait till then before initing. */
      size_t i;

      for (i = 0; i < io->extsocketoptionc; ++i)
         io->extsocketoptionv[i].info
         = optval2sockopt(io->extsocketoptionv[i].level,
                          io->extsocketoptionv[i].optname);

      gettimeofday_monotonic(&tnow);

      sockd_check_ipclatency("client object received from request process",
                             &io->state.time.requestend,
                             &tnow,
                             &tnow);

      /* needs to be set now for correct bandwidth calculation/limiting. */
      io->lastio = tnow;

      switch (io->state.command) {
#if SOCKS_SERVER
         case SOCKS_BINDREPLY:
            socks_allocbuffer(io->src.s, SOCK_STREAM);
            socks_allocbuffer(io->dst.s, SOCK_STREAM);

            io->dst.isclientside = 1;

            if (io->control.s != -1) {
               SASSERTX(io->state.extension.bind);
               socks_allocbuffer(io->control.s, SOCK_STREAM);
            }

            break;
#endif /* SOCKS_SERVER */

#if HAVE_UDP_SUPPORT
         case SOCKS_UDPASSOCIATE: {

            io->src.isclientside = 1;

#if SOCKS_SERVER
            io->cmd.udp.sfwdrule   = &fwdrulev[ioi];
            io->cmd.udp.sreplyrule = &replyrulev[ioi];
            io->dst.dstv           = udptargetv[ioi];
            io->dst.dstcmax        = ELEMENTS(udptargetv[ioi]);
            io->dst.dstc           = 0;

            if (ADDRISBOUND(&io->src.raddr))
               SASSERTX(io->src.state.isconnected);
            else
               SASSERTX(!io->src.state.isconnected);

            SASSERTX(io->control.s != -1);
            socks_allocbuffer(io->control.s, SOCK_STREAM);

#else /* BAREFOOTD */
            const size_t mallocsize
                         = UDP_INITIALCLIENTCOUNT * sizeof(*io->dst.dstv);

            SASSERTX(io->src.auth.method == AUTHMETHOD_NONE);

            SASSERTX(io->dst.s == -1);

            /* only used for select(2) regarding writability. */
            io->dst.s = io->src.s;

            if ((io->dst.dstv = malloc(mallocsize)) == NULL) {
               struct sockaddr_storage laddr;
               char buf[MAXSOCKADDRSTRING];
               socklen_t len = sizeof(laddr);

               if (getsockname(io->src.s, TOSA(&laddr), &len) == 0)
                  sockaddr2string(&laddr, buf, sizeof(buf));
               else {
                  SWARN(errno);
                  snprintf(buf, sizeof(buf), "<unknown>");
               }

               swarn("%s: failed to allocate %lu bytes of memory for initial "
                     "%d udp clients to accept on internal address %s",
                     function,
                     (unsigned long)(mallocsize),
                     UDP_INITIALCLIENTCOUNT,
                     buf);

               close(io->src.s);

               return -1;
            }

            io->dst.dstc    = 0;
            io->dst.dstcmax = UDP_INITIALCLIENTCOUNT;
#endif /* BAREFOOTD */

            /*
             * Each client will have it's own target/dst object, set once
             * we receive the first packet from it.  The client/source socket
             * is however the same for all clients.
             */
            socks_allocbuffer(io->src.s, SOCK_DGRAM);

            break;
         }
#endif /* HAVE_UDP_SUPPORT */

         case SOCKS_CONNECT:
            if (!io->dst.state.isconnected)
               iostate.haveconnectinprogress = 1;

            io->src.isclientside = 1;

            socks_allocbuffer(io->src.s, SOCK_STREAM);
            socks_allocbuffer(io->dst.s, SOCK_STREAM);
            break;

      }

#if HAVE_NEGOTIATE_PHASE
      if (io->clientdatalen != 0) {
         slog(LOG_DEBUG,
              "%s: adding initial data of size %ld from client %s to iobuf",
              function,
              (long)io->clientdatalen,
              sockshost2string(&io->src.host, NULL, 0));

         /*
          * XXX if covenant, this request has already been parsed and we
          * already know we need to forward it; should optimize away
          * re-parsing.
          */

         socks_addtobuffer(CONTROLIO(io)->s,
                           READ_BUF,
                           0,
                           io->clientdata,
                           io->clientdatalen);

         io->clientdatalen = 0;
      }
#endif /* HAVE_NEGOTIATE_PHASE */

      log_ruleinfo_shmid(CRULE_OR_HRULE(io), function, NULL);

      if (io->srule.type == object_none) {
         SASSERTX(io->state.protocol == SOCKS_UDP);
         SASSERTX(!HAVE_SOCKS_RULES);
      }
      else
         log_ruleinfo_shmid(&io->srule, function, NULL);

      /*
       * attach to shmem now and keep attached, so we don't have to
       * attach/detach for every i/o op later.  Session is not important
       * to attach to as we won't need it until we delete the client,
       * though it would still be preferable to attach to that also to
       * avoid having to wait for pagein of sessionmemory upon disconnect.
       */
      (void)sockd_shmat(SHMEMRULE(io), SHMEM_ALL);

      io_updatemonitor(io);

      /*
       * only update now, as it's added to us (us, the i/o process)
       * without problems.
       */
      io->allocated = 1;
   }

   iostate.freefds -= fdreceived;

   return 0;
}

static void
io_clearset(io, clearalltargets, set)
   const sockd_io_t *io;
   const int clearalltargets;
   fd_set *set;
{

   SASSERTX(io->src.s != -1);
   FD_CLR(io->src.s, set);

   if (io->state.protocol == SOCKS_TCP)
      SASSERTX(io->dst.s != -1);

   if (io->dst.s != -1)
      FD_CLR(io->dst.s, set);

   switch (io->state.command) {
      case SOCKS_CONNECT:
         break;

      case SOCKS_BIND:
      case SOCKS_BINDREPLY:
         if (io->state.extension.bind) {
            SASSERTX(io->control.s != -1);
            FD_CLR(io->control.s, set);
         }
         break;

#if HAVE_UDP_SUPPORT
      case SOCKS_UDPASSOCIATE: {
         size_t i;

#if HAVE_CONTROL_CONNECTION
         SASSERTX(io->control.s != -1);
         FD_CLR(io->control.s, set);
#endif /* HAVE_CONTROL_CONNECTION */


         if (clearalltargets)
            for (i = 0; i < io->dst.dstc; ++i)
               FD_CLR(io->dst.dstv[i].s, set);

         break;
      }
#endif /* HAVE_UDP_SUPPORT */

      default:
         SERRX(io->state.command);
   }
}

static size_t
io_allocated(tcpio, tcpfd, udpio, udpfd)
   size_t *tcpio;
   size_t *tcpfd;
   size_t *udpio;
   size_t *udpfd;
{
   const char *function = "io_allocated()";
   size_t i, tcpio_mem, tcpfd_mem, udpio_mem, udpfd_mem;

   if (tcpio == NULL)
      tcpio = &tcpio_mem;

   if (tcpfd == NULL)
      tcpfd = &tcpfd_mem;

   if (udpio == NULL)
      udpio = &udpio_mem;

   if (udpfd == NULL)
      udpfd = &udpfd_mem;

   *tcpio = *tcpfd = *udpio = *udpfd = 0;

   for (i = 0; i < ioc; ++i) {
      if (!iov[i].allocated)
         continue;

      switch (iov[i].state.protocol) {
#if HAVE_UDP_SUPPORT
         case SOCKS_UDP:
            ++(*udpio);

            ++(*udpfd);                   /* internal-side.  Always one. */
            (*udpfd) += iov[i].dst.dstc;  /* external side.  Varies.     */

#if HAVE_CONTROL_CONNECTION
            ++(*tcpfd);
#endif /* HAVE_CONTROL_CONNECTION */

            break;
#endif /* HAVE_UDP_SUPPORT */

         case SOCKS_TCP:
            ++(*tcpio);

            (*tcpfd) += 1 /* internal side */ + 1 /* external side. */;
            break;

         default:
            SERRX(iov[i].state.protocol);
      }

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: iov #%lu allocated for %s",
         function, (unsigned long)i, protocol2string(iov[i].state.protocol));
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: allocated for tcp: %lu, udp: %lu",
           function, (unsigned long)*tcpio, (unsigned long)*udpio);

   return *tcpio + *udpio;
}

static void
proctitleupdate(void)
{
   size_t inprogress;

   if (iostate.haveconnectinprogress) {
      size_t i;

      for (i = inprogress = 0; i < ioc; ++i)
         if (io_connectisinprogress(&iov[i]))
            ++inprogress;
   }
   else
      inprogress = 0;

   setproctitle("%s: %lu/%lu (%lu in progress)",
                childtype2string(sockscf.state.type),
                (unsigned long)io_allocated(NULL, NULL, NULL, NULL),
                (unsigned long)SOCKD_IOMAX,
                (unsigned long)inprogress);
}

static sockd_io_t *
io_getset(nfds, set)
   const int nfds;
   const fd_set *set;
{
   const char *function = "io_getset()";
   sockd_io_t *best, *evaluating;
   size_t i;
   int s;

   for (s = 0, best = NULL; s < nfds; ++s) {
      if (!FD_ISSET(s, set))
         continue;

      /*
       * find the io 's' is part of.
       */
      for (i = 0, evaluating = NULL; i < ioc; ++i) {
         if (!iov[i].allocated)
            continue;

         switch (iov[i].state.command) {
            case SOCKS_CONNECT:
               if (s == iov[i].src.s || s == iov[i].dst.s)
                  evaluating = &iov[i];

               break;

#if SOCKS_SERVER
            case SOCKS_BINDREPLY:
               if (s == iov[i].src.s || s == iov[i].dst.s)
                  evaluating = &iov[i];
               else if (iov[i].state.extension.bind && s == iov[i].control.s)
                  evaluating = &iov[i];

               break;
#endif /* SOCKS_SERVER */

#if HAVE_UDP_SUPPORT
            case SOCKS_UDPASSOCIATE: {
               udptarget_t *target;

               if (s == iov[i].src.s) {
                  /* will have to demux later based on packet read from src. */
                  iov[i].dst.s = -1;
                  evaluating = &iov[i];
               }
               else if ((target = clientofsocket(s,
                                                 iov[i].dst.dstc,
                                                 iov[i].dst.dstv)) != NULL) {
                     io_syncudp(&iov[i], target);

                     SASSERTX(iov[i].dst.s != -1);
                     SASSERTX(iov[i].dst.s == s);

                     evaluating = &iov[i];
               }
#if HAVE_CONTROL_CONNECTION
               else if (s == iov[i].control.s)
                  evaluating = &iov[i];
#endif /* HAVE_CONTROL_CONNECTION */

               break;
            }
#endif /* HAVE_UDP_SUPPORT */

            default:
               break;
         }

         if (evaluating != NULL)
            break;
      }

      SASSERTX(evaluating != NULL);

      /* want the i/o object that has least recently done i/o. */
      if (best == NULL || timercmp(&evaluating->lastio, &best->lastio, <))
         best = evaluating;
   }

   return best;
}

static sockd_io_t *
io_finddescriptor(d)
   int d;
{
   size_t i;

   for (i = 0; i < ioc; ++i) {
      if (!iov[i].allocated)
         continue;

      switch (iov[i].state.command) {
         case SOCKS_BIND:
         case SOCKS_BINDREPLY:
            if (d == iov[i].src.s || d == iov[i].dst.s)
               return &iov[i];
            else if (!iov[i].state.extension.bind) {
               if (d == iov[i].control.s)
                  return &iov[i];
            }
            break;

         case SOCKS_CONNECT:
            if (d == iov[i].src.s || d == iov[i].dst.s)
               return &iov[i];
            break;

#if HAVE_UDP_SUPPORT
         case SOCKS_UDPASSOCIATE: {
            udptarget_t *target;

            if (d == iov[i].src.s) {
               /* will have to demux later based on packet read from src. */
               iov[i].dst.s = -1;
               return &iov[i];
            }

            target = clientofsocket(d, iov[i].dst.dstc, iov[i].dst.dstv);
            if (target != NULL) {
               io_syncudp(&iov[i], target);

               SASSERTX(iov[i].dst.s != -1);
               SASSERTX(iov[i].dst.s == d);

               return &iov[i];
            }

#if HAVE_CONTROL_CONNECTION
            if (d == iov[i].control.s)
               return &iov[i];
#endif /* HAVE_CONTROL_CONNECTION */

            break;
         }
#endif /* HAVE_UDP_SUPPORT */

         default:
            SERRX(iov[i].state.command);
      }
   }

   return NULL;
}

static int
io_fillset(set, antiflags, antiflags_set, bwoverflowtil)
   fd_set *set;
   int antiflags;
   fd_set *antiflags_set;
   struct timeval *bwoverflowtil;
{
   const char *function = "io_fillset()";
   struct timeval tnow, firstbwoverflowok;
   size_t i;
   int max;

   if (antiflags != 0)
      SASSERTX(antiflags_set != NULL);

   gettimeofday_monotonic(&tnow);
   timerclear(&firstbwoverflowok);

   FD_ZERO(set);

   if (antiflags_set != 0)
      FD_ZERO(antiflags_set);

   for (i = 0, max = -1; i < ioc; ++i) {
      sockd_io_t *io = &iov[i];

      if (!io->allocated)
         continue;

      /* should have been removed already if so. */
      SASSERTX(!(io->src.state.fin_received && io->dst.state.fin_received));

#if HAVE_CONTROL_CONNECTION
      /*
       * Don't care about bandwidth-limits on control-connections.
       */
      if (io->control.s != -1) {
         if (antiflags & io->control.flags)
            FD_SET(io->control.s, antiflags_set);
         else
            FD_SET(io->control.s, set);

         max = MAX(max, io->control.s);
      }
#endif /* HAVE_CONTROL_CONNECTION */


#if BAREFOOTD
      /*
       * udp-clients need special handling in barefootd regarding bw,
       * but the tcp case is the same.
       */
      if (io->state.protocol == SOCKS_TCP) {
#endif /* BAREFOOTD */

      if (SHMEMRULE(io)->bw_shmid != 0) {
         struct timeval bwoverflowok, howlongtil;

         if (bw_rulehasoverflown(SHMEMRULE(io), &tnow, &bwoverflowok)) {
               if (!timerisset(&firstbwoverflowok)
               ||  timercmp(&bwoverflowok, &firstbwoverflowok, <))
                  firstbwoverflowok = bwoverflowok;

            SASSERTX(!timercmp(&bwoverflowok, &tnow, <));
            timersub(&bwoverflowok, &tnow, &howlongtil);
            slog(LOG_DEBUG,
                 "%s: skipping io #%lu belonging to rule #%lu/bw_shmid %lu "
                 "due to bwoverflow.  Have fd %d, fd %d, fd %d ctrl/src/dst.  "
                 "Have to wait for %ld.%06lds, until %ld.%06ld",
                 function,
                 (unsigned long)i,
                 (unsigned long)SHMEMRULE(io)->number,
                 (unsigned long)SHMEMRULE(io)->bw_shmid,
                 io->control.s,
                 io->src.s,
                 io->dst.s,
                 (long)howlongtil.tv_sec,
                 (long)howlongtil.tv_usec,
                 (long)bwoverflowok.tv_sec,
                 (long)bwoverflowok.tv_usec);

            continue;
         }
      }

#if BAREFOOTD
      }
#endif /* BAREFOOTD */

      switch (io->state.command) {
         case SOCKS_BINDREPLY:
         case SOCKS_CONNECT:
            if (!io->src.state.fin_received) {
               if (antiflags & io->src.flags)
                  FD_SET(io->src.s, antiflags_set);
               else
                  FD_SET(io->src.s, set);

               max = MAX(max, io->src.s);
            }

            if (io->dst.state.isconnected
            && !io->dst.state.fin_received) {
               if (antiflags & io->dst.flags)
                  FD_SET(io->dst.s, antiflags_set);
               else
                  FD_SET(io->dst.s, set);

               max = MAX(max, io->dst.s);
            }

            break;

#if HAVE_UDP_SUPPORT
         case SOCKS_UDPASSOCIATE: {
            size_t j;

            /* no flags for udp so far. */
            SASSERTX(io->src.flags == 0);
            SASSERTX(io->dst.flags == 0);

            FD_SET(io->src.s, set);
            max = MAX(max, io->src.s);

#if BAREFOOTD
            /*
             * the client-socket is shared among many clients, so set it
             * regardless of bw-limits as we don't know from what
             * client the packet is til we've read the packet.
             *
             * XXX But what do we do if the bw overflows?  We can't know
             * that until we've read the packet and seen what client it's
             * from.  Should we then drop the packet?  Probably.
             */

            for (j = 0; j < io->dst.dstc; ++j) {
               if (io->dst.dstv[j].crule.bw_shmid != 0) {
                  struct timeval bwoverflowok;

                  slog(LOG_DEBUG,
                       "%s: checking client %s for bw overflow "
                       "according to bw_shmid %lu ...",
                       function,
                       sockaddr2string(&io->dst.dstv[j].client, NULL, 0),
                       (unsigned long)io->dst.dstv[j].crule.bw_shmid);

                  SASSERTX(io->dst.dstv[j].crule.bw != NULL);
                  if (bw_rulehasoverflown(&io->dst.dstv[j].crule,
                                          &tnow,
                                          &bwoverflowok)) {
                     if (!timerisset(&firstbwoverflowok)
                     ||  timercmp(&bwoverflowok, &firstbwoverflowok, <))
                        firstbwoverflowok = bwoverflowok;

                     continue;
                  }
               }

               FD_SET(io->dst.dstv[j].s, set);
               max = MAX(max, io->dst.dstv[j].s);
            }

#else /* SOCKS_SERVER */
            /*
             * Each client can have up to two target sockets, one
             * IPv4 socket and one IPv6 socket.  Set all allocated.
             */

            if (io->src.state.isconnected) {
               for (j = 0; j < io->dst.dstc; ++j) {
                  SASSERTX(io->dst.dstv[j].s != -1);

                  FD_SET(io->dst.dstv[j].s, set);
                  max = MAX(max, io->dst.dstv[j].s);
               }
            }
            else {
               /*
                * means we don't yet know what address the client will send
                * us packets from, and it has not sent us any packets yet,
                * so we can hardly expect any reply.  Even if we got a
                * replypacket, we wouldn't know where to send it.
                */
               SASSERTX(io->dst.dstc == 0);
               SASSERTX(io->dst.s    == -1);
            }
#endif /* SOCKS_SERVER */

            break;
         }
#endif /* HAVE_UDP_SUPPORT */
      }
   }

   if (bwoverflowtil != NULL)
      *bwoverflowtil = firstbwoverflowok;

   return max;
}

static int
io_fillset_connectinprogress(set)
   fd_set *set;
{
   const char *function = "io_fillset_connectinprogress()";
   int i, bits, count;

   if (set != NULL)
      FD_ZERO(set);

   if (!iostate.haveconnectinprogress)
      return -1;

   for (i = count = 0, bits = -1; (size_t)i < ioc; ++i) {
      if (io_connectisinprogress(&iov[i])) {
         if (set == NULL)
            return iov[i].dst.s;

         FD_SET(iov[i].dst.s, set);

         bits = MAX(bits, iov[i].dst.s);

         slog(LOG_DEBUG, "%s: fd %d marked as still connecting",
              function, iov[i].dst.s);

         ++count;
      }
   }

   return bits;
}

static struct timeval *
io_gettimeout(timeout)
   struct timeval *timeout;
{
   const char *function = "io_gettimeout()";
   static struct timeval last_timeout;
   static time_t last_time;
   static int last_timeout_isset;
   struct timeval tnow, time_havebw;
   size_t i, tcpc, udpc;
   int havetimeout;

   gettimeofday_monotonic(&tnow);

   if (timerisset(&bwoverflowtil)) {
      const struct timeval shortenough = { 1, 0 };

      timersub(&bwoverflowtil, &tnow, &time_havebw);

      slog(LOG_DEBUG,
           "%s: bwoverflowtil is set until %ld.%06ld (in %ld.%06lds)",
           function,
           (long)bwoverflowtil.tv_sec,
           (long)bwoverflowtil.tv_usec,
           (long)time_havebw.tv_sec,
           (long)time_havebw.tv_usec);

      if (time_havebw.tv_sec < 0) {
         timerclear(&bwoverflowtil);
         timerclear(timeout);

         return timeout;
      }

      *timeout = time_havebw;
      if (timercmp(timeout, &shortenough, <))
         return timeout;

      havetimeout = 1;
   }
   else
      havetimeout = 0;

   slog(LOG_DEBUG,
        "%s: last_time = %ld, tnow = %ld, last_timeout_isset = %ld, "
        "last_timeout = %ld.%06ld",
        function,
        (long)last_time,
        (long)tnow.tv_sec,
        (long)last_timeout_isset,
        (long)last_timeout.tv_sec,
        (long)last_timeout.tv_usec);

   /*
    * If last timeout scan was the same second as now, see if we
    * can reuse it without scanning all clients again.
    */
   if (last_timeout_isset && last_time == tnow.tv_sec) {
      /*
       * Don't know if the last timeout is still valid.  Could be it was a
       * temporary timeout that no longer is valid (e.g., a timeout waiting
       * for connect(2) to complete.  Since we don't know, make sure we
       * don't wait too long, and not too little either.
       */
      last_timeout.tv_sec  = 1;
      last_timeout.tv_usec = 0;

      if (!havetimeout
      || (havetimeout && timercmp(timeout, &last_timeout, <)))
         *timeout = last_timeout;

      return timeout;
   }

   /*
    * Could perhaps add a "timeoutispossible" object also by checking
    * each io object as we receive it (and each udp client as we
    * add it).  If we find one where timeout is possible, set the
    * global timeoutispossible, if not, don't set it.  Each time
    * we io_delete(), we change timeoutispossible to true, and
    * upon scanning through all i/o's here, we may possible set it
    * to false again.  Since the default is to not have any timeout
    * in the i/o phase (except for FIN_WAIT), this might save time in
    * the common cases.
    */
   if (io_allocated(&tcpc, NULL, &udpc, NULL) == 0) {
      last_timeout_isset = 0;
      return NULL;
   }

   last_time = tnow.tv_sec;

   /*
    * go through all i/o-objects, finding the one who has least left
    * until timeout, or the first with a timeout that is soon enough.
    */
   for (i = 0; i < ioc && (tcpc > 0 || udpc > 0); ++i) {
      struct timeval timeout_found;

      if (!iov[i].allocated)
         continue;

      if (iov[i].state.protocol == SOCKS_TCP)
         --tcpc;
      else {
         SASSERTX(iov[i].state.protocol == SOCKS_UDP);
         --udpc;
      }

      timeout_found.tv_sec  = io_timeuntiltimeout(&iov[i], &tnow, NULL, 0);
      timeout_found.tv_usec = 0;

      slog(LOG_DEBUG, "%s: timeout for iov #%lu is in %lds",
           function, (unsigned long)i, (long)timeout_found.tv_sec);

      if (timeout_found.tv_sec != (time_t)-1) {
         if (!havetimeout || timercmp(&timeout_found, timeout, <)) {
            havetimeout = 1;
            *timeout    = timeout_found;
         }

         SASSERTX(havetimeout);
         if (timeout->tv_sec <= 0)
            break; /* timeout soon enough or already there. */
      }
   }

   if (havetimeout) {
      SASSERTX(timeout->tv_sec  >= 0);
      SASSERTX(timeout->tv_usec >= 0);

      /*
       * never mind sub-second accuracy, but do make sure we don't end up
       * with {0, 0} if there is less than one second till timeout.  If
       * there is more than one second, never mind if the timeout is a
       * little longer than necessary.
       */
      timeout->tv_usec = 999999;

      last_timeout = *timeout;
   }
   else
      timeout = NULL;

   last_timeout_isset = havetimeout;

   return timeout;
}

static sockd_io_t *
io_gettimedout(void)
{
   const char *function = "io_gettimedout()";
   struct timeval tnow;
   size_t i;

   gettimeofday_monotonic(&tnow);
   for (i = 0; i < ioc; ++i) {
      struct timeval timeout;

      if (!iov[i].allocated)
         continue;

      if ((timeout.tv_sec = io_timeuntiltimeout(&iov[i], &tnow, NULL, 1))
      == (time_t)-1)
         continue;  /* no timeout on this object. */

      timeout.tv_usec = 0; /* whole seconds is good enough. */
      if (timeout.tv_sec <= 0) { /* has timed out already. */
         slog(LOG_DEBUG,
              "%s: io #%lu with control %d, src %d, dst %d, has reached the "
              "timeout point.  I/O last done at %ld.%06ld",
              function,
              (unsigned long)i,
              iov[i].control.s,
              iov[i].src.s,
              iov[i].dst.s,
              (long)iov[i].lastio.tv_sec,
              (long)iov[i].lastio.tv_usec);

         return &iov[i];
      }
   }

   return NULL;
}

static int
io_timeoutispossible(io)
   const sockd_io_t *io;
{

   if (!io->allocated)
      return 0;

#if HAVE_UDP_SUPPORT
   if (io->state.protocol == SOCKS_UDP) {

#if BAREFOOTD
      size_t i;

      for (i = 0; i < io->dst.dstc; ++i) {
         if (io->dst.dstv[i].crule.timeout.udpio != 0)
            return 1;
      }

      return 0;

#else /* !BAREFOOTD */

      return io->srule.timeout.udpio != 0;

#endif /* !BAREFOOTD */

   }
#endif /* HAVE_UDP_SUPPORT */

   /*
    * TCP is the same for all.
    */
   SASSERTX(io->state.protocol == SOCKS_TCP);

   if (io->dst.state.isconnected) {
      if (io->srule.timeout.tcp_fin_wait != 0
      ||  io->srule.timeout.tcpio        != 0)
         return 1;
      else
         return 0;
   }
   else
      return io->srule.timeout.connect != 0;

   /* NOTREACHED */
}

static time_t
io_timeuntiltimeout(io, tnow, timeouttype, doudpsync)
   sockd_io_t *io;
   const struct timeval *tnow;
   timeouttype_t *timeouttype;
   const int doudpsync;
{
   const char *function = "io_timeuntiltimeout()";
   timeouttype_t timeouttype_mem;
   time_t *lastio;
   long protocoltimeout;

   if (timeouttype == NULL)
      timeouttype = &timeouttype_mem;

   *timeouttype = TIMEOUT_NOTSET;

   if (!io_timeoutispossible(io))
      return -1;

   /*
    * First find out what the correct timeoutobject to use for this
    * io at this time is, and then see if a timeout value has been
    * set in that object (i.e., is not 0).
    */
   if (io->state.protocol == SOCKS_UDP) {
#if BAREFOOTD
      size_t i;
      time_t timeout;

      slog(LOG_DEBUG, "%s: scanning %lu udp clients for nearest timeout",
           function, (unsigned long)io->dst.dstc);

      for (i = 0, timeout = -1; i < io->dst.dstc; ++i) {
         udptarget_t *udpclient = &io->dst.dstv[i];

         SASSERTX(tnow->tv_sec >= udpclient->lastio.tv_sec);

         timeout
        =  socks_difftime(udpclient->crule.timeout.udpio,
                        socks_difftime(tnow->tv_sec, udpclient->lastio.tv_sec));

         slog(LOG_DEBUG, "%s: time until timeout for udpclient %s is %ld",
              function,
              sockaddr2string(&udpclient->client, NULL, 0),
              (long)timeout);

         timeout      = MAX(0, timeout);
         *timeouttype = TIMEOUT_IO;

         if (timeout <= 0) {
            SASSERTX(udpclient != NULL);

            if (doudpsync)
               io_syncudp(io, udpclient);

            break; /* timeout is now. */
         }
      }

      return timeout;
#else /* SOCKS_SERVER */

      *timeouttype    = TIMEOUT_IO; /* only type possible for an udp client. */
      protocoltimeout = io->srule.timeout.udpio;
      lastio          = (time_t *)&io->lastio.tv_sec;
#endif /* SOCKS_SERVER */
   }
   else {
      SASSERTX(io->state.protocol == SOCKS_TCP);

      if (io->dst.state.isconnected) {
         if (io->src.state.fin_received || io->dst.state.fin_received) {
            if (io->srule.timeout.tcp_fin_wait == 0)
               *timeouttype = TIMEOUT_IO;
            else {
               if (io->srule.timeout.tcpio == 0
               ||  io->srule.timeout.tcpio > io->srule.timeout.tcp_fin_wait) {
                  *timeouttype = TIMEOUT_TCP_FIN_WAIT;
               }
               else
                  *timeouttype = TIMEOUT_IO;
            }
         }
         else
            *timeouttype = TIMEOUT_IO;

         if (*timeouttype == TIMEOUT_IO)
            protocoltimeout = io->srule.timeout.tcpio;
         else {
            SASSERTX(*timeouttype == TIMEOUT_TCP_FIN_WAIT);
            protocoltimeout = io->srule.timeout.tcp_fin_wait;
         }

         lastio = (time_t *)&io->lastio.tv_sec;
      }
      else {
         *timeouttype    = TIMEOUT_CONNECT;
         protocoltimeout = io->srule.timeout.connect;
         lastio          = (time_t *)&io->state.time.negotiateend.tv_sec;
      }
   }

   if (protocoltimeout == 0)
      return -1;

   SASSERTX(socks_difftime(*lastio, (time_t)tnow->tv_sec) <= 0);

   if (sockscf.option.debug)
      if (MAX(0, protocoltimeout - socks_difftime(tnow->tv_sec, *lastio)) == 0)
         slog(LOG_DEBUG,
              "%s: timeouttype = %d, protocoltimeout = %ld, tnow = %lu, "
              "lastio = %ld (%lds ago), timeout reached %lds ago",
              function,
              (int)*timeouttype,
              protocoltimeout,
              (unsigned long)tnow->tv_sec,
              (long)*lastio,
              (long)socks_difftime(tnow->tv_sec, *lastio),
              (long)(protocoltimeout - socks_difftime(tnow->tv_sec, *lastio)));

   return MAX(0, protocoltimeout - socks_difftime(tnow->tv_sec, *lastio));
}

static int
getnewios()
{
   const char *function = "getnewios()";
   const size_t freec = SOCKD_IOMAX - io_allocated(NULL, NULL, NULL, NULL);
   size_t receivedc;

   receivedc = errno = 0;
   while ( recv_io(sockscf.state.mother.s, NULL) == 0
   &&     receivedc < freec)
      ++receivedc;

   slog(LOG_DEBUG, "%s: received %lu new io%s, errno = %d (%s)",
        function,
        (long)receivedc,
        receivedc == 1 ? "" : "s",
        errno,
        strerror(errno));

   if (receivedc > 0) {
      errno = 0;
      return (int)receivedc;
   }
   else {
      slog(LOG_DEBUG,
           "%s: strange ... we were called to receive a new client (%lu/%lu), "
           "but no new client was there to receive: %s",
           function,
           (unsigned long)(freec + 1),
           (unsigned long)SOCKD_IOMAX,
           strerror(errno));

      return -1;
   }
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

#if HAVE_UDP_SUPPORT

   iostat_t *stats;

#endif /* HAVE_UDP_SUPPORT */

   unsigned long days, hours, minutes, seconds;
   time_t tnow;
   size_t i;

   SIGNAL_PROLOGUE(sig, si, errno_s);

   seconds = (unsigned long)socks_difftime(time_monotonic(&tnow),
                                           sockscf.stat.boot);

   seconds2days(&seconds, &days, &hours, &minutes);

   slog(LOG_INFO, "io-child up %lu day%s, %lu:%.2lu:%.2lu",
        days, days == 1 ? "" : "s", hours, minutes, seconds);

#if HAVE_UDP_SUPPORT
   if ((stats = io_get_ro_stats()) == NULL)
      slog(LOG_INFO, "no read-only latency information available (yet)");
   else
      slog(LOG_INFO,
           "read-only latency statistics based on last %lu packets: "
           "min/max/median/average/last/stddev: "
           "%lu/%lu/%lu/%lu/%lu/%lu (us)",
           (unsigned long)stats->latencyc,
           stats->min_us,
           stats->max_us,
           stats->median_us,
           stats->average_us,
           stats->last_us,
           stats->stddev_us);

   if ((stats = io_get_io_stats()) == NULL)
      slog(LOG_INFO, "no i/o latency information available (yet)");
   else
      slog(LOG_INFO,
           "i/o latency statistics based on last %lu packets: "
           "min/max/median/average/last/stddev: "
           "%lu/%lu/%lu/%lu/%lu/%lu (us)",
           (unsigned long)stats->latencyc,
           stats->min_us,
           stats->max_us,
           stats->median_us,
           stats->average_us,
           stats->last_us,
           stats->stddev_us);
#endif /* HAVE_UDP_SUPPORT */

   for (i = 0; i < ioc; ++i) {
      const int isreversed = (iov[i].state.command == SOCKS_BINDREPLY ? 1 : 0);
      uint64_t src_written, dst_written;
      sockd_io_direction_t *src, *dst;
      sockshost_t a, b;
      char srcstring[MAX_IOLOGADDR], dststring[MAX_IOLOGADDR],
           timeinfo[64], idlestr[64];

      if (!iov[i].allocated)
         continue;

      if (isreversed) {
         src = &iov[i].dst;
         dst = &iov[i].src;
      }
      else {
         src = &iov[i].src;
         dst = &iov[i].dst;
      }

      if (iov[i].state.protocol == SOCKS_UDP
      ||  dst->state.isconnected)
         snprintfn(idlestr, sizeof(idlestr), "%lds",
                   (long)socks_difftime(tnow, iov[i].lastio.tv_sec));
      else
         snprintfn(idlestr, sizeof(idlestr),
                   "%lds (waiting for connect to complete)",
                   (long)socks_difftime(tnow,
                                        iov[i].state.time.requestend.tv_sec));

      snprintf(timeinfo, sizeof(timeinfo),
               "age: %lds, idle: %s",
               (long)socks_difftime(tnow, iov[i].state.time.accepted.tv_sec),
               idlestr);


      /*
       * When printing current state display the IP-addresses in actual
       * use, rather than any hostnames the client may have provided.
       */

      if (iov[i].state.protocol == SOCKS_TCP) {
         size_t src_buffered, dst_buffered;
         char src_bufferinfo[64], dst_bufferinfo[sizeof(src_bufferinfo)],
              tcpinfo[MAXTCPINFOLEN];
         int havesocketinfo;

#if HAVE_RECVBUF_IOCTL
         int src_so_rcvbuf, dst_so_rcvbuf;
#endif /* HAVE_RECVBUF_IOCTL */

#if HAVE_SENDBUF_IOCTL
         int src_so_sndbuf, dst_so_sndbuf;
#endif /* HAVE_SENDBUF_IOCTL */

         build_addrstr_src(HAVE_SOCKS_HOSTID ? &iov[i].state.hostid : NULL,
                           &src->host,
                           NULL,
                           NULL,
                           sockaddr2sockshost(&src->laddr, &b),
                           &src->auth,
                           NULL,
                           srcstring,
                           sizeof(srcstring));

         build_addrstr_dst(sockaddr2sockshost(&dst->laddr, NULL),
                         iov[i].state.proxychain.proxyprotocol == PROXY_DIRECT ?
                              NULL : sockaddr2sockshost(&dst->raddr, &a),
                         iov[i].state.proxychain.proxyprotocol == PROXY_DIRECT ?
                                 NULL : &iov[i].state.proxychain.extaddr,
                         iov[i].state.proxychain.proxyprotocol == PROXY_DIRECT ?
                              sockaddr2sockshost(&dst->raddr, &a)
                           :  &dst->host,
                           &dst->auth,
                           NULL,
                           NULL,
                           dststring,
                           sizeof(dststring));


         src_buffered
         = socks_bytesinbuffer(src->s,
                               WRITE_BUF,
#if SOCKS_SERVER && HAVE_GSSAPI
                               src->auth.method == AUTHMETHOD_GSSAPI
                               && src->auth.mdata.gssapi.state.wrap ?
                                    1 : 0
#else /* !SOCKS_SERVER */
                               0
#endif /* !SOCKS_SERVER */
                              );

         dst_buffered = socks_bytesinbuffer(dst->s, WRITE_BUF, 0);

         *src_bufferinfo = NUL;
         *dst_bufferinfo = NUL;

#if HAVE_RECVBUF_IOCTL || HAVE_SENDBUF_IOCTL

         havesocketinfo = 1;

#else /* ! (HAVE_RECVBUF_IOCTL || HAVE_SENDBUF_IOCTL) */

         havesocketinfo = 0;

#endif /* ! (HAVE_RECVBUF_IOCTL || HAVE_SENDBUF_IOCTL) */

#if HAVE_RECVBUF_IOCTL
         if (ioctl(dst->s, RECVBUF_IOCTLVAL, &dst_so_rcvbuf) != 0) {
            swarn("%s: rcvbuf size ioctl() on dst-fd %d failed",
                  function, dst->s);
         }
         else
            havesocketinfo = 1;

         if (havesocketinfo) {
            if (ioctl(src->s, RECVBUF_IOCTLVAL, &src_so_rcvbuf) != 0) {
               swarn("%s: recvbuf size ioctl() on src-fd %d failed",
                     function, src->s);
               havesocketinfo = 0;
            }
         }
#endif /* HAVE_RECVBUF_IOCTL */

#if HAVE_SENDBUF_IOCTL
         if (havesocketinfo) {
            if (ioctl(src->s, SENDBUF_IOCTLVAL, &src_so_sndbuf) != 0) {
               swarn("%s: sendbuf size ioctl() on src-fd %d failed",
                     function, src->s);
               havesocketinfo = 0;
            }
         }

         if (havesocketinfo) {
            if (ioctl(dst->s, SENDBUF_IOCTLVAL, &dst_so_sndbuf) != 0) {
               swarn("%s: sendbuf size ioctl() on dst-fd %d failed",
                     function, dst->s);
               havesocketinfo = 0;
            }
         }
#endif /* HAVE_SENDBUF_IOCTL */

#if HAVE_SENDBUF_IOCTL && HAVE_RECVBUF_IOCTL
         if (havesocketinfo) {
            snprintf(src_bufferinfo, sizeof(src_bufferinfo),
                     "%lu buffered (%lu + %lu + %lu)",
                     (unsigned long)(src_buffered
                                   + dst_so_rcvbuf
                                   + src_so_sndbuf),
                     (unsigned long)dst_so_rcvbuf,
                     (unsigned long)src_buffered,
                     (unsigned long)src_so_sndbuf);

            snprintf(dst_bufferinfo, sizeof(dst_bufferinfo),
                     "%lu buffered (%lu + %lu + %lu)",
                     (unsigned long)(dst_buffered
                                   + src_so_rcvbuf
                                   + dst_so_sndbuf),
                     (unsigned long)src_so_rcvbuf,
                     (unsigned long)dst_buffered,
                     (unsigned long)dst_so_sndbuf);
         }

#elif HAVE_SENDBUF_IOCTL && !HAVE_RECVBUF_IOCTL
         if (havesocketinfo) {
            snprintf(src_bufferinfo, sizeof(src_bufferinfo),
                     "%lu buffered (? + %lu + %lu)",
                     (unsigned long)(src_buffered + src_so_sndbuf),
                     (unsigned long)src_buffered,
                     (unsigned long)src_so_sndbuf);

            snprintf(dst_bufferinfo, sizeof(dst_bufferinfo),
                     "%lu buffered (? + %lu + %lu)",
                     (unsigned long)(dst_buffered + dst_so_sndbuf),
                     (unsigned long)dst_buffered,
                     (unsigned long)dst_so_sndbuf);
         }

#elif !HAVE_SENDBUF_IOCTL && HAVE_RECVBUF_IOCTL
         if (havesocketinfo) {
            snprintf(src_bufferinfo, sizeof(src_bufferinfo),
                     "%lu buffered (%lu + %lu + ?)",
                     (unsigned long)(src_buffered + dst_so_rcvbuf),
                     (unsigned long)dst_so_rcvbuf,
                     (unsigned long)src_buffered);

            snprintf(dst_bufferinfo, sizeof(dst_bufferinfo),
                     "%lu buffered (%lu + %lu + ?)",
                     (unsigned long)(dst_buffered + src_so_rcvbuf),
                     (unsigned long)src_so_rcvbuf,
                     (unsigned long)dst_buffered);
         }
#endif /* !HAVE_SENDBUF_IOCTL && !HAVE_RECVBUF_IOCTL */

         if (!havesocketinfo) {
            snprintf(src_bufferinfo, sizeof(src_bufferinfo),
                     "%lu buffered (? + %lu + ?)",
                     (unsigned long)src_buffered,
                     (unsigned long)src_buffered);

            snprintf(dst_bufferinfo, sizeof(dst_bufferinfo),
                     "%lu buffered (? + %lu + ?)",
                     (unsigned long)dst_buffered,
                     (unsigned long)dst_buffered);
         }

         src_written = src->written.bytes;
         dst_written = dst->written.bytes;

         if (iov[i].srule.log.tcpinfo) {
            const char *info;
            int fdv[] = { src->s, dst->s };

            if ((info = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0)) == NULL)
               *tcpinfo = NUL;
            else {
#if DIAGNOSTIC
               if (strlen(info) >= sizeof(tcpinfo))
                  SWARNX(strlen(info));
#endif /* DIAGNOSTIC */

               snprintf(tcpinfo, sizeof(tcpinfo),
                        "\nTCP_INFO:\n"
                        "%s",
                        info);
            }
         }
         else
            *tcpinfo = NUL;

         slog(LOG_INFO,
              "%s: %s <-> %s: %s, bytes transferred: "
              "%"PRIu64" (+ %s) "
              "<-> "
              "%"PRIu64" (+ %s)"
              "%s",
              protocol2string(iov[i].state.protocol),
              srcstring,
              dststring,
              timeinfo,
              dst_written,
              dst_bufferinfo,
              src_written,
              src_bufferinfo,
              tcpinfo);
      }

#if HAVE_UDP_SUPPORT
      else if (iov[i].state.protocol == SOCKS_UDP) {
         uint64_t src_packetswritten, dst_packetswritten;
         size_t srci;

#define UDPLOG()                                                               \
do {                                                                           \
   if (*dststring == NUL) {                                                    \
      slog(LOG_INFO,                                                           \
           "%s: %s: %s, "                                                      \
           "bytes transferred: %"PRIu64" <-> %"PRIu64", "                      \
           "packets: %"PRIu64" <-> %"PRIu64"",                                 \
           protocol2string(iov[i].state.protocol),                             \
           srcstring,                                                          \
           timeinfo,                                                           \
           dst_written,                                                        \
           src_written,                                                        \
           dst_packetswritten,                                                 \
           src_packetswritten);                                                \
   }                                                                           \
   else {                                                                      \
      slog(LOG_INFO,                                                           \
           "%s: %s <-> %s: %s, "                                               \
           "bytes transferred: %"PRIu64" <-> %"PRIu64", "                      \
           "packets: %"PRIu64" <-> %"PRIu64"",                                 \
           protocol2string(iov[i].state.protocol),                             \
           srcstring,                                                          \
           dststring,                                                          \
           timeinfo,                                                           \
           dst_written,                                                        \
           src_written,                                                        \
           dst_packetswritten,                                                 \
           src_packetswritten);                                                \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#if SOCKS_SERVER

         if (dst->dstc == 0) {
            /*
             * Special-case for Dante.  Even though we have no target yet,
             * do print out the addresses on the internal side, as
             * we do have a client.  In Barefoot's case we on the other
             * hand do not have any control-connection, so if dstc is 0,
             * we have no clients either.
             */

            build_addrstr_src(HAVE_SOCKS_HOSTID ? &iov[i].state.hostid : NULL,
                              &src->host,
                              NULL,
                              NULL,
                              sockaddr2sockshost(&src->laddr, &b),
                              &src->auth,
                              NULL,
                              srcstring,
                              sizeof(srcstring));

            *dststring         = NUL;

            src_written        = src->written.bytes;
            src_packetswritten = src->written.packets;

            dst_written        = dst->written.bytes;
            dst_packetswritten = dst->written.packets;

            UDPLOG();
            continue;
         }

#endif /* SOCKS_SERVER */

         for (srci = 0; srci < dst->dstc; ++srci) {
            const udptarget_t *client = &dst->dstv[srci];

            src_written        = client->client_written.bytes,
            src_packetswritten = client->client_written.packets;

            dst_written        = client->target_written.bytes,
            dst_packetswritten = client->target_written.packets;


            build_addrstr_src(HAVE_SOCKS_HOSTID ? &iov[i].state.hostid : NULL,


#if BAREFOOTD

                              sockaddr2sockshost(&client->client, &a),

#else /* Dante */

                              sockaddr2sockshost(&iov[i].src.raddr, &a),

#endif /* Dante */
                              NULL,
                              NULL,
                              sockaddr2sockshost(&src->laddr, &b),
                              &src->auth,
                              NULL,
                              srcstring,
                              sizeof(srcstring));

            build_addrstr_dst(sockaddr2sockshost(&client->laddr, &a),
                              iov[i].state.proxychain.proxyprotocol
                              == PROXY_DIRECT ?
                                 NULL : sockaddr2sockshost(&client->raddr, &b),
                              iov[i].state.proxychain.proxyprotocol
                              == PROXY_DIRECT ?
                                 NULL : &iov[i].state.proxychain.extaddr,
                              sockaddr2sockshost(&client->raddr, NULL),
                              &dst->auth,
                              NULL,
                              NULL,
                              dststring,
                              sizeof(dststring));

            snprintf(timeinfo, sizeof(timeinfo), "age: %lus, idle: %lus",
                     (long)socks_difftime(tnow, client->firstio.tv_sec),
                     (long)socks_difftime(tnow, client->lastio.tv_sec));

            UDPLOG();
         }
      }
#endif /* HAVE_UDP_SUPPORT */
   }

   SIGNAL_EPILOGUE(sig, si, errno_s);
}

static void
freebuffers(io)
   const sockd_io_t *io;
{
   if (io->control.s != -1)
      socks_freebuffer(io->control.s);

   socks_freebuffer(io->src.s);

   switch (io->state.protocol) {
      case SOCKS_TCP:
         socks_freebuffer(io->dst.s);
         break;

#if HAVE_UDP_SUPPORT
      case SOCKS_UDP: {
         size_t i;

         SASSERTX(SOCKS_SERVER); /* only called for UDP in Dante. */

         for (i = 0; i < io->dst.dstc; ++i)
            socks_freebuffer(io->dst.dstv[i].s);
         break;
      }
#endif /* HAVE_UDP_SUPPORT */

      default:
         SERRX(io->state.protocol);
   }

}

static int
connectstatus(io, badfd)
   sockd_io_t *io;
   int *badfd;
{
   const char *function = "connectstatus()";
   clientinfo_t cinfo;
   socklen_t len;
   char src[MAXSOCKSHOSTSTRING], dst[MAXSOCKSHOSTSTRING], buf[2048];

   SASSERTX(io_connectisinprogress(io));
   SASSERTX(io->dst.state.err == 0);

   *badfd = -1;

   cinfo.from   = CONTROLIO(io)->raddr;
   cinfo.hostid = io->state.hostid;

   /*
    * Check if the socket connected successfully.
    */
   len = sizeof(io->dst.raddr);
   if (getpeername(io->dst.s, TOSA(&io->dst.raddr), &len) == 0) {
      iologaddr_t src, dst;

      gettimeofday_monotonic(&io->state.time.established);

      slog(LOG_DEBUG, "%s: connect to %s on fd %d completed successfully",
           function, sockshost2string(&io->dst.host, NULL, 0), io->dst.s);

      io->dst.state.isconnected = 1;

#if HAVE_NEGOTIATE_PHASE
      if (SOCKS_SERVER || io->reqflags.httpconnect) {
         errno = 0; /* make sure we don't reuse some old junk. */

         if (send_connectresponse(io->src.s, 0, io) != 0) {
            if (io->srule.mstats_shmid != 0
            && (io->srule.alarmsconfigured & ALARM_DISCONNECT)) {
               SASSERTX(!io->src.state.alarmdisconnectdone);

               alarm_add_disconnect(0,
                                    &io->srule,
                                    ALARM_INTERNAL,
                                    &cinfo,
                                    strerror(errno),
                                    sockscf.shmemfd);

               io->src.state.alarmdisconnectdone = 1;
            }

            *badfd = io->src.s;
            return -1;
         }
      }
#endif /* HAVE_NEGOTIATE_PHASE */

      setconfsockoptions(io->dst.s,
                         io->control.s,
                         io->state.protocol,
                         0,
                         io->extsocketoptionc,
                         io->extsocketoptionv,
                         SOCKETOPT_POST,
                         SOCKETOPT_POST);

      init_iologaddr(&src,
                     object_sockaddr,
                     &io->src.laddr,
                     object_sockshost,
                     &io->src.host,
                     &io->src.auth,
                     &io->state.hostid);

      init_iologaddr(&dst,
                     object_sockaddr,
                     &io->dst.laddr,
                     object_sockaddr,
                     &io->dst.raddr,
                     &io->dst.auth,
                     NULL);

      if (io->srule.log.tcpinfo) {
         int fdv[] = { io->src.s, io->dst.s };
         const char *tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);

         if (tcpinfo != NULL) {
            snprintf(buf, sizeof(buf),
                     "\nTCP_INFO:\n"
                     "%s",
                     tcpinfo);
         }
         else
            *buf = NUL;
      }
      else
         *buf = NUL;

      iolog(&io->srule,
            &io->state,
            OPERATION_CONNECT,
            &src,
            &dst,
            NULL,
            NULL,
            buf,
            strlen(buf));

      if (io_fillset_connectinprogress(NULL) == -1)
         iostate.haveconnectinprogress = 0;

      return 0;
   }

   /*
    * else: connect(2) failed.
    */

   slog(LOG_DEBUG, "%s: getpeername(2) on fd %d failed: %s",
        function, io->dst.s, strerror(errno));

   if (io->srule.mstats_shmid != 0
   && (io->srule.alarmsconfigured & ALARM_DISCONNECT)) {
      SASSERTX(!io->dst.state.alarmdisconnectdone);

      alarm_add_disconnect(0,
                           &io->srule,
                           ALARM_EXTERNAL,
                           &cinfo,
                           strerror(errno),
                           sockscf.shmemfd);

      io->dst.state.alarmdisconnectdone = 1;
   }

   len = sizeof(errno);
   (void)getsockopt(io->dst.s, SOL_SOCKET, SO_ERROR, &errno, &len);

   if (errno == 0) {
      swarnx("%s: strange ... getpeername(2) failed, but getsockopt(2) "
             "still says errno is 0",
             function);

      io->dst.state.err = errno = ECONNREFUSED; /* no idea. */
   }
   else
      io->dst.state.err = errno;

   slog(LOG_DEBUG,
        "%s: connect(2) to %s on fd %d, on behalf of client %s, failed: %s",
        function,
        sockshost2string(&io->dst.host, dst, sizeof(dst)),
        io->dst.s,
        sockshost2string(&io->src.host, src, sizeof(src)),
        strerror(errno));

   log_connectfailed(EXTERNALIF, dst); /* special-cased for customer. */

#if HAVE_NEGOTIATE_PHASE
   if (SOCKS_SERVER || io->reqflags.httpconnect) {
      SASSERTX(errno != 0);

      if (send_connectresponse(io->src.s, errno, io) != 0)
         errno = io->dst.state.err; /* let errno be errno from connect(2). */
   }
#endif /* HAVE_NEGOTIATE_PHASE */

   *badfd = io->dst.s;
   return -1;
}

void
io_update(timenow, bwused, i_read, i_written,
          e_read, e_written, rule, packetrule, lock)
   const struct timeval *timenow;
   const size_t bwused;
   const iocount_t *i_read;
   const iocount_t *i_written;
   const iocount_t *e_read;
   const iocount_t *e_written;
   rule_t *rule;
   rule_t *packetrule;
   const int lock;
{
   const char *function = "io_update()";
   const iocount_t zero = { 0 };
   monitor_stats_t *monitor;
   int didattach;

   slog(LOG_DEBUG, "%s: bwused %lu, bw_shmid %lu, mstats_shmid %lu",
        function,
        (unsigned long)bwused,
        (rule == NULL || rule->bw_shmid == 0) ?
            0 : (unsigned long)rule->bw_shmid,
        (unsigned long)packetrule->mstats_shmid);

   if (rule != NULL && rule->bw_shmid != 0 && bwused != 0) {
      SASSERTX(rule->bw != NULL);
      bw_update(rule->bw, bwused, timenow, lock);
   }

   if (packetrule->mstats_shmid == 0
   || !(packetrule->alarmsconfigured & ALARM_DATA))
      return;

   if (packetrule->mstats == NULL) {
      /*
       * Must be a Dante UDP session.
       */
      SASSERTX(packetrule != rule);
      SASSERTX(SOCKS_SERVER);

      if (sockd_shmat(packetrule, SHMEM_MONITOR) != 0)
         return;

      didattach = 1;
   }
   else
      didattach = 0;

   SASSERTX(packetrule->mstats != NULL);
   monitor = &packetrule->mstats->object.monitor;

   socks_lock(sockscf.shmemfd, (off_t)packetrule->mstats_shmid, 1, 1, 1);

   MUNPROTECT_SHMEMHEADER(packetrule->mstats);

   if (monitor->internal.alarm.data.recv.isconfigured
   &&  i_read != NULL
   &&  memcmp(&zero, i_read, sizeof(zero)) != 0) {
      monitor->internal.alarm.data.recv.bytes  += i_read->bytes;
      monitor->internal.alarm.data.recv.lastio  = *timenow;
   }

   if (monitor->internal.alarm.data.send.isconfigured
   && i_written != NULL
   && memcmp(&zero, i_written, sizeof(zero)) != 0) {
      monitor->internal.alarm.data.send.bytes  += i_written->bytes;
      monitor->internal.alarm.data.send.lastio  = *timenow;
   }

   if (monitor->external.alarm.data.recv.isconfigured
   && e_read != NULL
   && memcmp(&zero, e_read, sizeof(zero)) != 0) {
      monitor->external.alarm.data.recv.bytes  += e_read->bytes;
      monitor->external.alarm.data.recv.lastio  = *timenow;
   }

   if (monitor->external.alarm.data.send.isconfigured
   && e_written != NULL
   && memcmp(&zero, e_written, sizeof(zero)) != 0) {
      monitor->external.alarm.data.send.bytes  += e_written->bytes;
      monitor->external.alarm.data.send.lastio  = *timenow;
   }

   MPROTECT_SHMEMHEADER(packetrule->mstats);

   socks_unlock(sockscf.shmemfd, (off_t)packetrule->mstats_shmid, 1);

   slog(LOG_DEBUG,
        "%s: data sides configured in monitor with shmid: %lu.  "
        "Data added: i_recv/i_send/e_recv/e_send: %lu/%lu/%lu/%lu bytes",
        function,
        (unsigned long)packetrule->mstats_shmid,
        i_read    != NULL && monitor->internal.alarm.data.recv.isconfigured ?
            (unsigned long)i_read->bytes    : 0,
        i_written != NULL && monitor->internal.alarm.data.send.isconfigured ?
            (unsigned long)i_written->bytes : 0,
        e_read    != NULL && monitor->external.alarm.data.recv.isconfigured ?
            (unsigned long)e_read->bytes    : 0,
        e_written != NULL && monitor->external.alarm.data.send.isconfigured ?
            (unsigned long)e_written->bytes : 0);

   if (didattach)
      sockd_shmdt(packetrule, SHMEM_MONITOR);
}

void
io_delete(mother, io, badfd, status)
   int mother;
   sockd_io_t *io;
   int badfd;
   const iostatus_t status;
{
   const char *function = "io_delete()";
   const int isreversed = (io->state.command == SOCKS_BINDREPLY ? 1 : 0);
   const int errno_s = errno;
#if HAVE_GSSAPI
   OM_uint32 major_status, minor_status;
#endif /* HAVE_GSSAPI */
   struct timeval tnow;
   rule_t *rulev[] = {
                        &io->srule,
#if HAVE_SOCKS_HOSTID
                        io->hrule_isset ? &io->hrule : NULL,
#endif /* HAVE_SOCKS_HOSTID */
                        &io->crule,
                     };
   clientinfo_t cinfo;
   ssize_t rulei;
   char buf[512], tcpinfo[MAXTCPINFOLEN];
   int command, protocol;

   slog(LOG_DEBUG,
        "%s: command %s, bad-fd %d, controlfd %d, src-fd %d, dst-fd %d"
#if HAVE_UDP_SUPPORT
        " dstc = %lu"
#endif /* HAVE_UDP_SUPPORT */
        ,
        function,
        command2string(io->state.command),
        badfd,
        io->control.s,
        io->src.s,
        io->dst.s
#if HAVE_UDP_SUPPORT
        ,
        (unsigned long)io->dst.dstc
#endif /* HAVE_UDP_SUPPORT */
        );

   SASSERTX(  badfd == -1
           || badfd == io->src.s
           || badfd == io->control.s
           || badfd == io->dst.s);

   SASSERTX(io->allocated);

   gettimeofday_monotonic(&tnow);

#if SOCKS_SERVER
   /*
    * UDP in Dante's case needs some special handling here because each
    * udp client can have two target sockets, one for ipv4 and one for ipv6,
    * and we want to print both of them when logging the session-close, but
    * log the client/hostid-rule close only once.  We use an ugly hack
    * for this involving dsti.
    */
    size_t dsti = 0;
#endif /* SOCKS_SERVER */

   /* only log the disconnect if the rule says so. */
   for (rulei = 0; rulei < (ssize_t)ELEMENTS(rulev); ++rulei) {
      const rule_t *rule = rulev[rulei];
      sockshost_t a, b;
      uint64_t src_read, src_written, dst_read, dst_written;
      size_t bufused;
      char in[MAX_IOLOGADDR], out[MAX_IOLOGADDR],
           timeinfo[512],
           logmsg[sizeof(in) + sizeof(out) + 1024 + sizeof(timeinfo)];

      if (rule == NULL)
         continue;

#if !HAVE_SOCKS_RULES
      if (rule->type == object_crule && !sockscf.option.debug)
         continue; /* normally inherited by the auto-created socks-rule. */
#endif /* HAVE_SOCKS_RULES */

      if (rule->log.disconnect
      || (rule->log.error && (status == IO_IOERROR)))
         /* LINTED */ /* EMPTY */;
      else
         continue;

      protocol = io->state.protocol;

      if (protocol == SOCKS_TCP && rule->log.tcpinfo) {
         const char *info;
         int fdv[] = { CLIENTIO(io)->s, EXTERNALIO(io)->s };

         if ((info = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0)) == NULL)
            *tcpinfo = NUL;
         else {
#if DIAGNOSTIC
            if (strlen(info) >= sizeof(tcpinfo))
               SWARNX(strlen(info));
#endif /* DIAGNOSTIC */

            snprintf(tcpinfo, sizeof(tcpinfo),
                     "\nTCP_INFO:\n"
                     "%s",
                     info);
         }
      }
      else
         *tcpinfo = NUL;

      if (rule->type == object_srule) {
         build_addrstr_src(HAVE_SOCKS_HOSTID ? &io->state.hostid : NULL,
                           &io->src.host,
                           NULL,
                           NULL,
                           sockaddr2sockshost(&io->src.laddr, NULL),
                           &io->src.auth,
                           NULL,
                           in,
                           sizeof(in));

         switch (io->state.command) {
            case SOCKS_BIND:
            case SOCKS_BINDREPLY:
            case SOCKS_CONNECT:
               build_addrstr_dst(sockaddr2sockshost(&io->dst.laddr, &a),
                                 io->state.proxychain.proxyprotocol
                                 == PROXY_DIRECT ?
                                  NULL : sockaddr2sockshost(&io->dst.raddr, &b),
                                 io->state.proxychain.proxyprotocol
                                 == PROXY_DIRECT ?
                                       NULL : &io->state.proxychain.extaddr,
                                 io->state.proxychain.proxyprotocol
                                 == PROXY_DIRECT ?
                                       sockaddr2sockshost(&io->dst.raddr, NULL)
                                    :  &io->dst.host,
                                 &io->dst.auth,
                                 NULL,
                                 NULL,
                                 out,
                                 sizeof(out));
               break;

#if HAVE_UDP_SUPPORT
            case SOCKS_UDPASSOCIATE: {
               sockshost_t a, b;

               if (io->dst.dstc == 0) /* no targets created for this session. */
                  *out = NUL;
               else {
                  udptarget_t *udptarget;

#if SOCKS_SERVER
                  /*
                   * Can have up to two targets.  Make sure we log both.
                   */

                  SASSERTX(dsti < io->dst.dstc);
                  udptarget = &io->dst.dstv[dsti];

#else  /* BAREFOOTD */

                  SASSERTX(io->dst.s != -1);

                  udptarget = clientofsocket(io->dst.s,
                                             io->dst.dstc,
                                             io->dst.dstv);

                  SASSERTX(udptarget != NULL);

#endif /* BAREFOOTD */

                  io_syncudp(io, udptarget);

                  build_addrstr_dst(sockaddr2sockshost(&io->dst.laddr, &a),
                                    io->state.proxychain.proxyprotocol
                                    == PROXY_DIRECT ?
                                NULL : sockaddr2sockshost(&io->dst.raddr, &b),
                                    io->state.proxychain.proxyprotocol
                                    == PROXY_DIRECT ?
                                       NULL : &io->state.proxychain.extaddr,
                                    io->dst.state.isconnected ?
                                sockaddr2sockshost(&io->dst.raddr, &b) : NULL,
                                    &io->dst.auth,
                                    NULL,
                                    NULL,
                                    out,
                                    sizeof(out));

#if SOCKS_SERVER
                  if (++dsti < io->dst.dstc) {
                     /*
                      * re-log using the same rule but with next dsti.
                      */
                     --rulei;
                  }
#endif /* SOCKS_SERVER */

               }
               break;
            }
#endif /* HAVE_UDP_SUPPORT */

            default:
               SERRX(io->state.command);
         }

         command  = io->state.command;
      }
      else {
         /*
          * XXX if support for server chaining is added to bind, the
          * bindreply might involve a proxy on the src side.
          */

#if !HAVE_SOCKS_RULES
         /*
          * we don't want to log the crule close of a udp session,
          * but only the individual clients closing the srule.
          */
         if (protocol == SOCKS_UDP)
            continue;
#endif /* !HAVE_SOCKS_RULES. */

         build_addrstr_src(HAVE_SOCKS_HOSTID ? &io->state.hostid : NULL,
                           &CONTROLIO(io)->host,
                           NULL,
                           NULL,
                           sockaddr2sockshost(&CONTROLIO(io)->laddr, NULL),
                           &io->cauth,
                           NULL,
                           in,
                           sizeof(in));

#if HAVE_SOCKS_RULES
         *out = NUL; /* client-rule is from client to socks-server, and stop. */

#else /* !HAVE_SOCKS_RULES; destination address is know upon accepting client.*/
         build_addrstr_dst(NULL, /* now known, but was not upon accepting. */
                           NULL,
                           NULL,
                           &io->dst.host,
                           &io->dst.auth,
                           NULL,
                           NULL,
                           out,
                           sizeof(out));
#endif /* HAVE_SOCKS_RULES */

         SASSERTX(rule->type != object_srule);

#if HAVE_SOCKS_RULES
         command  = (rule->type == object_crule ? SOCKS_ACCEPT: SOCKS_HOSTID);
         protocol = SOCKS_TCP;   /* always tcp before socks-rules. */

#else /* !HAVE_SOCKS_RULES */
         switch (rule->type) {
            case object_crule:
               if (protocol == SOCKS_TCP)
                  command = SOCKS_ACCEPT;
               else
                  command = SOCKS_UDPASSOCIATE;
               break;

#if HAVE_SOCKS_HOSTID
            case object_hrule:
               command = SOCKS_HOSTID;
               break;
#endif /* HAVE_SOCKS_HOSTID */

            default:
               SERRX(rule->type);
         }

#endif /* !HAVE_SOCKS_RULES */
      }

      bufused = snprintf(logmsg, sizeof(logmsg), "%s(%lu): %s/%s ]: ",
                         rule->verdict == VERDICT_PASS ?
                         VERDICT_PASSs : VERDICT_BLOCKs,
#if !HAVE_SOCKS_RULES
                        /* use the number from the user-created rule. */
                         io->state.protocol == SOCKS_UDP ?
                          (unsigned long)io->crule.number
                        : (unsigned long)rule->number,
#else /* HAVE_SOCKS_RULES */
                        (unsigned long)rule->number,
#endif /* HAVE_SOCKS_RULES */
                         protocol2string(protocol),
                         command2string(command));

      src_read           = io->src.read.bytes;
      src_written        = io->src.written.bytes;
      dst_read           = io->dst.read.bytes;
      dst_written        = io->dst.written.bytes;

      if (protocol == SOCKS_TCP) {
         if (*out == NUL) {
            bufused += snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                                "%"PRIu64" -> %s -> %"PRIu64"",
                                (isreversed ? dst_written : src_written),
                                in,
                                (isreversed ? src_written  : dst_written));
         }
         else
            bufused += snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                                "%"PRIu64" -> %s -> %"PRIu64", "
                                "%"PRIu64" -> %s -> %"PRIu64"",
                                src_written, in, src_read,
                                dst_written, out, dst_read);
      }
      else {
         SASSERTX(protocol == SOCKS_UDP);

         if (rule->type == object_srule) {
            bufused +=
            snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                     "%"PRIu64"/%"PRIu64" -> %s -> %"PRIu64"/%"PRIu64"",
                     src_written,
                     io->src.written.packets,
                     in,
                     src_read,
                     io->src.read.packets);


            if (*out != NUL) { /* have a target address also. */
               bufused +=
               snprintf(&logmsg[bufused], sizeof(logmsg) - bufused,
                        ", %"PRIu64"/%"PRIu64" -> %s -> %"PRIu64"/%"PRIu64"",
                        dst_written,
                        io->dst.written.packets,
                        out,
                        dst_read,
                        io->dst.read.packets);
            }
         }
         else
            SASSERTX(*out == NUL);
      }

      bufused = snprintf(timeinfo, sizeof(timeinfo),
                        "Session duration: %lds",
                        (long)(  tnow.tv_sec
                               - io->state.time.accepted.tv_sec));

      /*
       * XXX probably better to add another log-option, "stats" or similar,
       * that can be used to log some extra information, including this
       * and buffer-usage perhaps?
       */

      if (sockscf.option.debug
#if BAREFOOTD
      &&  protocol == SOCKS_TCP
#endif /* BAREFOOTD */
      ) {
         struct timeval accept2neg, negstart2negfinish, sessionduration;
         char established2io_str[16], negfinish2established_str[16];

         timersub(&io->state.time.negotiatestart,
                  &io->state.time.accepted,
                  &accept2neg);

         timersub(&io->state.time.negotiateend,
                  &io->state.time.negotiatestart,
                  &negstart2negfinish);

         if (io->state.time.established.tv_sec == 0)
            STRCPY_ASSERTSIZE(negfinish2established_str, "N/A");
         else {
            struct timeval tdiff;

            timersub(&io->state.time.established,
                     &io->state.time.negotiateend,
                     &tdiff);

            snprintf(negfinish2established_str, sizeof(established2io_str),
                     "%ld.%06lds", (long)tdiff.tv_sec, (long)tdiff.tv_usec);
         }

         if (io->state.time.firstio.tv_sec == 0)
            STRCPY_ASSERTSIZE(established2io_str, "N/A");
         else {
            struct timeval tdiff;

            timersub(&io->state.time.firstio, &io->state.time.established,
                     &tdiff);

            snprintf(established2io_str, sizeof(established2io_str),
                     "%ld.%06lds", (long)tdiff.tv_sec, (long)tdiff.tv_usec);
         }

         timersub(&tnow, &io->state.time.accepted, &sessionduration);

         bufused += snprintf(&timeinfo[bufused], sizeof(timeinfo) - bufused,
                             "\n"
                             "accept to negotiate start       : %ld.%06lds\n"
                             "negotiate duration              : %ld.%06lds\n"
                             "negotiate finish to established : %s\n"
                             "session establish to first i/o  : %s\n"
                             "total session duration          : %ld.%06lds\n",
                             (long)accept2neg.tv_sec,
                             (long)accept2neg.tv_usec,
                             (long)negstart2negfinish.tv_sec,
                             (long)negstart2negfinish.tv_usec,
                             negfinish2established_str,
                             established2io_str,
                             (long)sessionduration.tv_sec,
                             (long)sessionduration.tv_usec);
      }

      errno = errno_s;
      switch (status) {
         case IO_BLOCK:
            slog(LOG_INFO, "%s: blocked.  %s%s", logmsg, timeinfo, tcpinfo);
            break;

         case IO_IOERROR:
         case IO_ERROR:
            if (errno != 0)
               snprintf(buf, sizeof(buf), " (%s)", strerror(errno));
            else
               *buf = NUL;

            slog(LOG_INFO, "%s: %s error%s.  %s%s",
                 logmsg,
                 badfd < 0 ? "session"
                        : (badfd == io->dst.s && !isreversed) ?
                              "remote peer" : "local client",
                 buf,
                 timeinfo,
                 tcpinfo);

            if (badfd >= 0 && (ERRNOISRST(errno))) {
               if (io->dst.s != -1 && badfd == io->dst.s)
                  sockd_rstonclose(io->src.s);
               else if (badfd == io->src.s && io->dst.s != -1)
                  sockd_rstonclose(io->dst.s);
            }
            break;

         case IO_CLOSE:
            slog(LOG_INFO, "%s: %s closed.  %s%s",
                logmsg,
                badfd < 0 ? "session" : badfd == io->dst.s ?
                "remote peer" : "local client",
                timeinfo,
                tcpinfo);
            break;

         case IO_TIMEOUT: {
            const char *timeoutinfo = NULL;
            timeouttype_t timeouttype;
            time_t timeuntiltimeout;

            timeuntiltimeout = io_timeuntiltimeout(io, &tnow, &timeouttype, 0);
            SASSERTX(timeuntiltimeout <= 0);
            SASSERTX(timeouttype != TIMEOUT_NOTSET);

            if (timeouttype == TIMEOUT_TCP_FIN_WAIT) {
               SASSERTX(io->src.state.fin_received
               ||       io->dst.state.fin_received);

               if (io->dst.state.fin_received)
                  timeoutinfo = " (waiting for client to close)";
               else
                  timeoutinfo = " (waiting for remote peer to close)";

            }

            slog(LOG_INFO, "%s: %s%s.  %s%s",
                 logmsg,
                 timeouttype2string(timeouttype),
                 timeoutinfo == NULL ? "" : timeoutinfo,
                 timeinfo,
                 tcpinfo);

            break;
         }

         case IO_ADMINTERMINATION:
            slog(LOG_INFO, "%s: administrative termination.  %s%s",
                 logmsg, timeinfo, tcpinfo);
            break;

         default:
            SERRX(status);
      }

#if SOCKS_SERVER
      if (io->state.command == SOCKS_BINDREPLY && rule->type == object_srule) {
         /*
          * log the close of the open'd bind session also.
          */
         const int original_command = io->state.command;
         iologaddr_t src, dst;

         init_iologaddr(&src,
                        object_sockaddr,
                        &io->src.laddr,
                        object_sockshost,
                        io->state.extension.bind ? NULL : &io->cmd.bind.host,
                        &io->src.auth,
                        &io->state.hostid);

         init_iologaddr(&dst,
                        object_sockaddr,
                        &io->dst.laddr,
                        object_sockaddr,
                        &io->dst.raddr,
                        &io->dst.auth,
                        NULL);

         io->state.command = SOCKS_BIND;
         /*
          * The bindreply src/dst order is reversed compared to that of the
          * bind as the src for bindreply is the client that connects to the
          * address bound.
          */
         iolog(&io->cmd.bind.rule,
               &io->state,
               OPERATION_DISCONNECT,
               &dst,
               &src,
               NULL,
               NULL,
               tcpinfo,
               strlen(tcpinfo));

         io->state.command = original_command;
      }
#endif /* SOCKS_SERVER */
   }

#if HAVE_GSSAPI
   if (io->src.auth.method == AUTHMETHOD_GSSAPI) {
      if ((major_status
      = gss_delete_sec_context(&minor_status,
                               &io->src.auth.mdata.gssapi.state.id,
                               GSS_C_NO_BUFFER)) != GSS_S_COMPLETE) {
         if (!gss_err_isset(major_status, minor_status, buf, sizeof(buf)))
            *buf = NUL;

         swarnx("%s: gss_delete_sec_context() of src failed%s%s",
                function,
                *buf == NUL ? "" : ": ",
                *buf == NUL ? "" : buf);
      }
   }

   if (io->dst.auth.method == AUTHMETHOD_GSSAPI) {
      if ((major_status
      = gss_delete_sec_context(&minor_status,
                               &io->dst.auth.mdata.gssapi.state.id,
                               GSS_C_NO_BUFFER)) != GSS_S_COMPLETE) {
         if (!gss_err_isset(major_status, minor_status, buf, sizeof(buf)))
            *buf = NUL;

         swarnx("%s: gss_delete_sec_context() of dst failed%s%s",
                function,
                *buf == NUL ? "" : ": ",
                *buf == NUL ? "" : buf);
      }
   }
#endif /* HAVE_GSSAPI */

#if BAREFOOTD
   if (io->state.protocol == SOCKS_UDP) {
      /*
       * The io itself is normally not freed in the udp-case, as we can
       * always get new clients; only this one client is removed.
       */

      removeclient(io->dst.s, &io->dst.dstc, io->dst.dstv);
      socks_freebuffer(io->dst.s);
      io->dst.s = -1;

      return;
   }
#endif /* BAREFOOTD */

   freebuffers(io);
   io_add_alarmdisconnects(io, "session delete");
   close_iodescriptors(io);

   if (mother != -1) {
      const char info = (io->state.command == SOCKS_UDPASSOCIATE ?
                                       SOCKD_FREESLOT_UDP : SOCKD_FREESLOT_TCP);

      /* ack io slot free. */
      if (socks_sendton(mother,
                        &info,
                        sizeof(info),
                        sizeof(info),
                        0,
                        NULL,
                        0,
                        NULL,
                        NULL) != sizeof(info))
         slog(sockd_motherexists() ? LOG_WARNING : LOG_DEBUG,
              "%s: sending ack to mother failed: %s",
              function, strerror(errno));
   }

   if (io->state.command == SOCKS_CONNECT)
      if (io_fillset_connectinprogress(NULL) == -1)
         iostate.haveconnectinprogress = 0;

   cinfo.hostid = io->state.hostid;

   log_ruleinfo_shmid(CRULE_OR_HRULE(io), function, "before SHMEM_UNUSE()");

   if (io->srule.type == object_none) {
      SASSERTX(io->state.protocol == SOCKS_UDP);
      SASSERTX(!HAVE_SOCKS_RULES);
   }
   else
      log_ruleinfo_shmid(&io->srule, function, "before SHMEM_UNUSE()");

   SASSERTX(!(SHMID_ISSET(CRULE_OR_HRULE(io)) && SHMID_ISSET(&io->srule)));

   cinfo.from = CONTROLIO(io)->raddr;

#if HAVE_CONTROL_CONNECTION

   SHMEM_UNUSE(SHMEMRULE(io), &cinfo, sockscf.shmemfd, SHMEM_ALL);

#else /* !HAVE_CONTROL_CONNECTION */

   if (io->state.protocol == SOCKS_TCP) /* UDP is free'd by removeclient(). */
      SHMEM_UNUSE(SHMEMRULE(io), &cinfo, sockscf.shmemfd, SHMEM_ALL);

#endif /* !HAVE_CONTROL_CONNECTION */

#if SOCKS_SERVER
   bzero(io->dst.dstv, sizeof(*io->dst.dstv) * io->dst.dstc);
#endif /* SOCKS_SERVER */

   bzero(io, sizeof(*io));

   proctitleupdate();
}


static int
io_connectisinprogress(io)
   const sockd_io_t *io;
{
   if (io->allocated
   &&  io->state.command == SOCKS_CONNECT
   && !io->dst.state.isconnected)
      return 1;
   else
      return 0;
}


#if COVENANT

int
recv_resentclient(s, client)
   int s;
   sockd_client_t *client;
{
   const char *function = "recv_resentclient()";
   struct iovec iov[2];
   struct msghdr msg;
   int ioc, fdexpect, fdreceived, r;
   CMSG_AALLOC(cmsg, sizeof(int));

   ioc = 0;
   bzero(iov, sizeof(iov));
   iov[ioc].iov_base = client;
   iov[ioc].iov_len  = sizeof(*client);
   ++ioc;

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ioc;
   msg.msg_name    = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   if ((r = recvmsgn(s, &msg, 0)) < (ssize_t)sizeof(*client)) {
      switch (r) {
         case -1:
            swarn("%s: recvmsg() failed", function);
            break;

         case 0:
            slog(LOG_DEBUG, "%s: recvmsg(): other side closed connection",
            function);
            break;

         default:
            swarnx("%s: recvmsg(): unexpected short read: %d/%ld",
                   function, r, (long)sizeof(*client));
      }

      return -1;
   }

   if (socks_msghaserrors(function, &msg))
      return -1;

   r       -= sizeof(*client);
   fdexpect = 1; /* client. */

   if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
      swarnx("%s: received control message has the invalid len of %d",
              function, (int)CMSG_TOTLEN(msg));

      return -1;
   }

   SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
   SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

   fdreceived = 0;
   if (fdexpect > 0) {
      CMSG_GETOBJECT(client->s, cmsg, sizeof(client->s) * fdreceived++);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: received fd %d (%s) ...",
         function, client->s, socket2string(client->s, NULL, 0));
   }

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG, "%s: received %d descriptors for client",
                      function, fdreceived);

   return 0;
}

#endif /* COVENANT */
