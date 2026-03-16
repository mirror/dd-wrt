/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2003, 2004, 2005, 2008, 2009,
 *               2010, 2011, 2012, 2013, 2014, 2016, 2019, 2020, 2021, 2024
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
"$Id: connectchild.c,v 1.397.4.3.2.3.4.7.4.4 2024/11/21 10:22:42 michaels Exp $";

/*
 * This sets things up for performing a non-blocking connect for the client.
 * We do this by initiating a connect on a non-blocking socket.
 * If the initial response is positive, we then save the endpoint
 * addresses of the socket and send it our "connect-child", which then
 * handles the socks negotiation and returns the proxy server's response back
 * to us.
 *
 * To avoid the client stepping on our (or rather our connect-childs) toes
 * while it negotiates with the proxy server, we temporarily let the
 * fd-index the client is using point at at dummy socket, while we use
 * the real socket to negotiate.  Then we set the clients fd to point back
 * at the real socket.
 *
 * When the connect-child is done, it will send us back the same socket
 * we sent it, and we will try to match our address-table (socksfdv)
 * for an identical socket (the control socket).
 */


#define MOTHER  (0)   /* descriptor mother reads/writes on.  */
#define CHILD   (1)   /* descriptor child reads/writes on.   */

/*
 * Number of unhandled packets pipe between mother and child should be
 * dimensioned for.
 */
#define MAXPACKETSQUEUED   (10)

static void sigio(int sig, siginfo_t *sip, void *scp);
static void run_connectchild(const int mother_data, const int mother_ack)
            __ATTRIBUTE__((noreturn));

static struct sigaction       originalhandler;
static volatile sig_atomic_t  reqoutstanding;

route_t *
socks_nbconnectroute(s, control, packet, src, dst, emsg, emsglen)
   int s;
   int control;
   socks_t *packet;
   const sockshost_t *src, *dst;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_nbconnectroute()";
   const char *p;
   route_t *route;
   socksfd_t socksfd;
   childpacket_t childreq;
   struct sigaction currentsig;
   struct iovec iov[1];
   struct sockaddr_storage local;
   struct msghdr msg;
   socklen_t len;
   size_t fdsent;
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);
   char srcstr[MAXSOCKSHOSTSTRING], dststr[MAXSOCKSHOSTSTRING];
   int tmp, flags, isourhandler;

   slog(LOG_DEBUG, "%s: fd %d", function, s);

   if ((route = socks_getroute(&packet->req, src, dst)) == NULL) {
      snprintf(emsg, emsglen, "no route from %s to %s found",
               src == NULL ?
                     "<any>" : sockshost2string(src, srcstr, sizeof(srcstr)),
               dst == NULL ?
                     "<any>" : sockshost2string(dst, dststr, sizeof(dststr)));

      errno = ENETUNREACH;
      return NULL;
   }

   if (route->gw.state.proxyprotocol.direct)
      return route; /* nothing more to do. */

   if (sigaction(SIGIO, NULL, &currentsig) != 0) {
      snprintf(emsg, emsglen, 
               "could not fetch existing signal handler for SIGIO: %s",
               strerror(errno));

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   if (currentsig.sa_flags & SA_SIGINFO) { /* sa_sigaction. */
      isourhandler = (currentsig.sa_sigaction == sigio);

      if (!isourhandler) {
         if (currentsig.sa_sigaction == NULL) { /* OpenBSD threads weirdness. */
            slog(LOG_NOTICE,
                 "%s: hmm, that's strange ... sa_flags set to 0x%x, "
                 "but sa_sigaction is NULL",
                 function, currentsig.sa_flags);
         }
         else
            slog(LOG_NOTICE, "%s: a SIGIO sa_sigaction is already installed, "
                             "but not ours ... wonder how this will work out",
                             function);
      }
   }
   else { /* sa_handler. */
      isourhandler = 0; /* we install with SA_SIGINFO. */

      if (currentsig.sa_handler != SIG_IGN
      &&  currentsig.sa_handler != SIG_DFL)
         slog(LOG_DEBUG,
              "%s: a handler is installed, but it's not ours ...", function);
      else
         slog(LOG_DEBUG,
              "%s: no SIGIO handler previously installed", function);
   }

   if (!isourhandler) {
      slog(LOG_DEBUG, "%s: our signal handler is not installed, installing ...",
           function);

      if (install_sigio(emsg, emsglen) != 0)
         return NULL;
   }
   else
      slog(LOG_DEBUG, "%s: our signal handler already installed", function);

   if (sockscf.connectchild == 0) {
      /*
       * Create child process that will do our connections.
       */
      int datapipev[2], ackpipev[2];
      int sndbuf, sndbuf_set, rcvbuf, rcvbuf_set;
      socklen_t optlen;

      /* Should have been SOCK_SEQPACKET, but that's not portable. :-/ */
      if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, datapipev) != 0) {
         snprintf(emsg, emsglen, "socketpair(AF_LOCAL, SOCK_DGRAM) failed: %s",
                  strerror(errno));

         swarnx("%s: %s", function, emsg);
         return NULL;
      }
      else
         slog(LOG_DEBUG, "%s: socketpair(SOCK_DGRAM) returned %d, %d",
         function, datapipev[0], datapipev[1]);

      if (socketpair(AF_LOCAL, SOCK_STREAM, 0, ackpipev) != 0) {
         snprintf(emsg, emsglen, "socketpair(AF_LOCAL, SOCK_STREAM) failed: %s",
                  strerror(errno));

         swarnx("%s: %s", function, emsg);
         return NULL;
      }
      else
         slog(LOG_DEBUG, "%s: socketpair(SOCK_STREAM) returned %d, %d",
              function, ackpipev[0], ackpipev[1]);

      p = "pipe between mother and connect-child";
      if (setnonblocking(datapipev[0], p) == -1
      ||  setnonblocking(datapipev[1], p) == -1
      ||  setnonblocking(ackpipev[0],  p) == -1
      ||  setnonblocking(ackpipev[1],  p) == -1) {
         snprintf(emsg, emsglen, "could not set %s non-blocking: %s",
                  p, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return NULL;
      }

      rcvbuf  = (sizeof(childpacket_t)
              + sizeof(struct msghdr)
              + CMSG_SPACE(sizeof(int) * FDPASS_MAX)
              + SENDMSG_PADBYTES);
#if HAVE_GSSAPI
      rcvbuf += MAX_GSS_STATE + sizeof(struct iovec);
#endif /* HAVE_GSSAPI */

      sndbuf = rcvbuf * (MAXPACKETSQUEUED + 1 /* +1 for response from child */);

      if (HAVE_PIPEBUFFER_RECV_BASED) {
         /*
          * reverse of our assumption that how much we can write to the pipe
          * depends on the pipe's sndbuf.
          */
         const size_t _tmp  = sndbuf;
                     sndbuf = rcvbuf;
                     rcvbuf = _tmp;
      }
      else if (HAVE_PIPEBUFFER_UNKNOWN) { /* wastes a lot of memory. */
         rcvbuf = MAX(sndbuf, rcvbuf);
         sndbuf = MAX(sndbuf, rcvbuf);
      }

      optlen = sizeof(sndbuf);
      if (setsockopt(datapipev[MOTHER],
                     SOL_SOCKET,
                     SO_SNDBUF,
                     &sndbuf,
                     optlen) != 0
      || setsockopt(datapipev[CHILD],
                    SOL_SOCKET,
                    SO_SNDBUF,
                    &sndbuf,
                    optlen) != 0
      || setsockopt(datapipev[MOTHER],
                    SOL_SOCKET,
                    SO_RCVBUF,
                    &rcvbuf,
                    optlen) != 0
      || setsockopt(datapipev[CHILD],
                    SOL_SOCKET,
                    SO_RCVBUF,
                    &rcvbuf,
                    optlen) != 0) {
         snprintf(emsg, emsglen,
                  "setsockopt(SO_SNDBUF/RCVBUF, %d/%d) on %s failed: %s",
                  sndbuf, rcvbuf, p, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return NULL;
      }

      optlen = sizeof(sndbuf_set);
      if (getsockopt(datapipev[MOTHER],
                     SOL_SOCKET,
                     SO_SNDBUF,
                     &sndbuf_set,
                     &optlen) == -1
      || getsockopt(datapipev[CHILD],
                    SOL_SOCKET,
                    SO_RCVBUF,
                    &rcvbuf_set,
                    &optlen) == -1) {
         snprintf(emsg, emsglen,
                  "getsockopt(SO_SNDBUF/SO_RCVBUF) on %s failed: %s",
                  p, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return NULL;
      }

      if (sndbuf_set < sndbuf || rcvbuf_set < rcvbuf) {
         swarnx("%s: could not set SNDBUF/mother and RCVBUF/child "
                "on %s appropriately.  Requested size %d and %d, "
                "but got %d and %d",
                function, p, sndbuf, rcvbuf, sndbuf_set, rcvbuf_set);

         /* but continue anyway.  Hopefully things will still work. */
      }
      else
         slog(LOG_DEBUG,
              "%s: SNDBUF/mother and RCVBUF/child on %s set to %d and %d, "
              "minimum is %d and %d",
              function, p, sndbuf, rcvbuf, sndbuf_set, rcvbuf_set);

      optlen = sizeof(rcvbuf_set);
      if (getsockopt(datapipev[MOTHER],
                     SOL_SOCKET,
                     SO_RCVBUF,
                     &rcvbuf_set,
                     &optlen) == -1
      || getsockopt(datapipev[CHILD],
                    SOL_SOCKET,
                    SO_SNDBUF,
                    &sndbuf_set,
                    &optlen) == -1) {
         snprintf(emsg, emsglen,
                  "getsockopt(SO_SNDBUF/SO_RCVBUF) on %s failed: %s",
                  p, strerror(errno));

         swarnx("%s: %s", function, emsg);
         return NULL;
      }

      if (sndbuf_set < sndbuf || rcvbuf_set < rcvbuf) {
         swarnx("%s: could not set SNDBUF/child and RCVBUF/mother on %s"
                "appropriately.  Requested %d and %d, but is %d and %d",
                function, p, sndbuf, rcvbuf, sndbuf_set, rcvbuf_set);

         /* but continue anyway.  Hopefully things will still work. */
      }
      else
         slog(LOG_DEBUG,
              "%s: SNDBUF/child and RCVBUF/mother on %s set to %d and %d, "
              "minimum is %d and %d",
              function, p, sndbuf, rcvbuf, sndbuf_set, rcvbuf_set);

      switch (sockscf.connectchild = fork()) {
         case -1:
            snprintf(emsg, emsglen, "fork(2) failed: %s", strerror(errno));

            swarnx("%s: %s", function, emsg);
            return NULL;

         case 0: {
            const int maxbadfdc = 256;
            int i, maxfd, badfdc;

            slog(LOG_INFO,
                 "%s: connectchild forked, our pid is %ld, mother is %ld",
                 function, (long)getpid(), (long)getppid());

            /* 
             * We want to close all unknown descriptors, but since there can
             * be tens of thousands of them, if that is the open file limit,
             * that can take many seconds.  What we do instead is to assume
             * that after we have got maxbadfd number of EBADF in a row,
             * there are no more open fd's.
             */

            maxfd  = (int)getmaxofiles(softlimit) + 1;
            badfdc = 0;

            for (i = 0; i <= maxfd && badfdc < maxbadfdc; ++i) {
               if (socks_logmatch(i, &sockscf.log)
               ||  socks_logmatch(i, &sockscf.errlog)
               ||  i == datapipev[CHILD]
               ||  i == ackpipev[CHILD]
               ||  FD_IS_RESERVED_EXTERNAL(i))
                  continue;
               else if (isatty(i))
                  continue;
               else {
                  if (fdisopen(i)) {
                     close(i);
                     badfdc = 0;
                  }
                  else
                     ++badfdc;
               }
            }

            newprocinit();

            run_connectchild(datapipev[CHILD], ackpipev[CHILD]);

            /* NOTREACHED */
         }

         default:
            slog(LOG_INFO, "%s: connectchild forked with pid %lu",
                 function, (unsigned long)sockscf.connectchild);

            sockscf.child_data = datapipev[MOTHER];
            sockscf.child_ack  = ackpipev[MOTHER];

            close(datapipev[CHILD]);
            close(ackpipev[CHILD]);

            if (fcntl(sockscf.child_data, F_SETOWN, getpid()) == -1
            ||  fcntl(sockscf.child_ack, F_SETOWN, getpid())  == -1)
               serr("%s: fcntl(F_SETOWN)", function);

            if ((flags = fcntl(sockscf.child_data, F_GETFL, 0))    == -1
            ||  fcntl(sockscf.child_data, F_SETFL, flags | FASYNC) == -1
            ||  fcntl(sockscf.child_ack, F_SETFL, flags | FASYNC)  == -1)
               serr("%s: fcntl(F_SETFL, FASYNC)", function);
      }
   }

   /*
    * Control socket is what later becomes data socket.
    * We don't want to allow the client to read/write/select etc.
    * on the socket yet, since we need to read/write on it
    * ourselves to setup the connection to the socks server.
    *
    * We therefore create a new unconnected socket and assign it the
    * same filedescriptor index as the client uses. This way the clients
    * select(2)/poll(2) will not mark the descriptor as ready for anything
    * while we are working on it.
    *
    * When the connection has been set up, by the child, we dup2(2)
    * back the socket we were passed here and close the temporarily
    * created socket.
    */

   SASSERTX(control == s);
   if ((control = makedummyfd(AF_INET, SOCK_STREAM)) == -1) {
      snprintf(emsg, emsglen,
               "could not create a temporary dummy socket to use while "
               "connecting to %s: %s",
               sockshost2string(dst, NULL, 0), strerror(errno));

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   if (socketoptdup(s, control) == -1) {
      snprintf(emsg, emsglen,
               "failed to duplicate socketoptions on dummy socket to use "
               "while connecting to %s: %s",
               sockshost2string(dst, NULL, 0), strerror(errno));

      swarnx("%s: %s", function, emsg);

      close(control);
      return NULL;
   }

#if HAVE_GSSAPI
   if (socks_allocbuffer(s, SOCK_STREAM) == NULL) {
      snprintf(emsg, emsglen, "socks_allocbuffer() failed: %s",
               strerror(errno));

      close(control);

      swarnx("%s: %s", function, emsg);
      return NULL;
   }
#endif /* HAVE_GSSAPI */

   if ((tmp = dup(s)) == -1) { /* dup2() would close it. */
      snprintf(emsg, emsglen,
               "could not dup(2) data-fd %d: %s", s, strerror(errno));

      close(control);

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   if (dup2(control, s) == -1) { /* give the client the dummy socket. */
      snprintf(emsg, emsglen, "could not dup2(2) control-fd %d: %s",
               control, strerror(errno));

      close(control);
      close(tmp);

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   /*
    * use the clients original socket (but at a different fd-index) to
    * connect.
    */
   close(control);
   control = tmp;

   slog(LOG_DEBUG,
        "%s: socket used for non-blocking connect on behalf of fd %d is fd %d",
        function, s, control);

   /*
    * Now the status is:
    * s       - new (temporary) socket using original index of "s".
    * control -  original "s" socket, but temporarily using a new fd-index.
    */

   /* if used for something else before, free now. */
   socks_rmaddr(control, 1);

   bzero(&socksfd, sizeof(socksfd));
   if ((socksfd.route = socks_connectroute(control,
                                           packet,
                                           src,
                                           dst,
                                           emsg,
                                           emsglen)) == NULL) {
      close(control);
      return NULL;
   }

   if (route->gw.state.proxyprotocol.direct)
      return route;

   /*
    * data socket probably unbound.  If so we need to bind it so
    * we can get a (hopefully) unique local address for it.
    */

   len = sizeof(local);
   if (getsockname(s, TOSA(&local), &len) != 0) {
      snprintf(emsg, emsglen, "getsockname(2) on control-fd %d failed: %s",
               s, strerror(errno));

      close(control);

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   if (!PORTISBOUND(TOIN(&local))) {
      bzero(&local, sizeof(local));

      /* bind same IP as control, any fixed address would do though. */

      len = sizeof(local);
      if (getsockname(control, TOSA(&local), &len) != 0) {
         int new_control;

         socks_blacklist(socksfd.route, strerror(errno));

         if ((new_control = socketoptdup(control, -1)) == -1) {
            close(control);

            snprintf(emsg, emsglen, "could not dup(2) control socket: %s",
                     strerror(errno));

            swarnx("%s: %s", function, emsg);
            return NULL;
         }

         switch (packet->req.version) {
            case PROXY_SOCKS_V4:
            case PROXY_SOCKS_V5:
            case PROXY_HTTP_10:
            case PROXY_HTTP_11:
            case PROXY_UPNP:
               close(control); /* created in this function. */
               control = s;
               break;

            default:
               SERRX(packet->req.version);
         }

         if (dup2(new_control, control) != -1) {
            close(new_control);

            /* try again, hopefully there's a backup route. */
            return socks_nbconnectroute(s,
                                        control,
                                        packet,
                                        src,
                                        dst,
                                        emsg,
                                        emsglen);
         }

         snprintf(emsg, emsglen, "could not dup2(2) control-fd %d: %s",
                 control, strerror(errno));

         close(new_control);

         swarnx("%s: %s", function, emsg);
         return NULL;
      }

      SASSERTX(PORTISBOUND(TOIN(&local)));
      TOIN(&local)->sin_port = htons(0);

      if (socks_bind(s, TOSS(&local), 0) != 0) {
         snprintf(emsg, emsglen,
                  "could not bind address (%s) for control-fd %d: %s",
                  sockaddr2string(&local, NULL, 0), s, strerror(errno));

         close(control);

         swarnx("%s: %s", function, emsg);
         return NULL;
      }
   }

   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) != 0) {
      close(control);

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   /*
    * this has to be done here or there would be a race against the signal
    * we receive when our connect-child is done.
    */
   socksfd.control             = control;
   socksfd.state.command       = packet->req.command;
   socksfd.state.version       = packet->req.version;
   socksfd.state.protocol.tcp  = 1;
   socksfd.state.inprogress    = 1;
   socksfd.forus.connected     = packet->req.host;

   slog(LOG_DEBUG, "%s: registering with socks_addaddr() using fd %d",
        function, s);

   /*
    * When we check the status of socket "s", we will see that it
    * belongs to a connect in progress done over fd-index control.
    */
   socks_addaddr(s, &socksfd, 1);

   /*
    * send the request to our connect process and let it do the rest.
    * When it's done, we get a signal and dup "s" over "socksfd.control"
    * in the handler.
    */

   fdsent = 0;
   CMSG_ADDOBJECT(control, cmsg, sizeof(control) * fdsent++);

   switch (packet->req.version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V5:
      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
      case PROXY_UPNP:
         break;

      default:
         SERRX(packet->req.version);
   }

   bzero(&childreq, sizeof(childreq)); /* silence valgrind warning */
   childreq.s       = s;
   childreq.packet  = *packet;

   iov[0].iov_base  = &childreq;
   iov[0].iov_len   = sizeof(childreq);
   len              = sizeof(childreq);

   bzero(&msg, sizeof(msg));
   msg.msg_iov      = iov;
   msg.msg_iovlen   = ELEMENTS(iov);
   msg.msg_name     = NULL;
   msg.msg_namelen  = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdsent);

   slog(LOG_INFO,
        "%s: sending request of size %lu + %lu to connectchild.  "
        "%d requests outstanding",
        function,
        (unsigned long)len,
        (unsigned long)CMSG_TOTLEN(msg),
        (int)reqoutstanding);

   if (sendmsgn(sockscf.child_data, &msg, 0, -1) != (ssize_t)len) {
      snprintf(emsg, emsglen,
               "could not send client to connect-child: %s", strerror(errno));

      swarnx("%s: %s", function, emsg);
      return NULL;
   }

   ++reqoutstanding;
   return socksfd.route;
}


int
install_sigio(emsg, emsglen)
   char *emsg;
   const size_t emsglen;
{
   const char *function = "install_sigio()";
   struct sigaction newsig, currenthandler;

   if (sigaction(SIGIO, NULL, &currenthandler) != 0) {
      snprintf(emsg, emsglen, "could not fetch existing SIGIO handler: %s",
               strerror(errno));

      return -1;
   }

   newsig               = currenthandler; /* keep same as much as possible. */
   newsig.sa_sigaction  = sigio;
   newsig.sa_flags     |= SA_SIGINFO;

   originalhandler      = currenthandler;

   if (sigaction(SIGIO, &newsig, NULL) != 0) {
      snprintf(emsg, emsglen, "could not install SIGIO-handler: %s",
               strerror(errno));

      return -1;
   }

   slog(LOG_DEBUG, "%s: SIGIO-handler installed", function);
   return 0;
}

int
our_sigio_is_installed(void)
{
   const char *function = "our_sigio_is_installed()";
   struct sigaction currenthandler;

   if (sigaction(SIGIO, NULL, &currenthandler) != 0) {
      swarn("could not fetch existing SIGIO-handler");
      return 0;
   }

   if (currenthandler.sa_flags & SA_SIGINFO) { /* sa_sigaction. */
      if (currenthandler.sa_sigaction == sigio) {
         slog(LOG_DEBUG, "%s: our SIGIO-handler is installed", function);

         return 1;
      }
      else {
         if (currenthandler.sa_sigaction == NULL) {
            /* OpenBSD threads weirdness. */
            slog(LOG_NOTICE,
                 "%s: hmm, that's strange ... sa_flags set to 0x%x, "
                 "but sa_sigaction is NULL",
                 function, currenthandler.sa_flags);
         }
         else
            slog(LOG_NOTICE,
                 "%s: a SIGIO sa_sigaction is already installed, but it's not "
                 "ours",
                 function);

         return 0;
      }
   }
   else { /* we use sa_sigaction, so this is for sure not our handler. */
      if (currenthandler.sa_handler != SIG_IGN
      &&  currenthandler.sa_handler != SIG_DFL) 
         slog(LOG_DEBUG,
              "%s: a SIGIO-handler is already installed, but it's not ours ...",
              function);
      else
         slog(LOG_DEBUG, "%s: no SIGIO-handler installed", function);

      return 0;
   }
}


/*
 * XXX should have more code so we could handle multiple requests at
 * a time.
 */
static void
run_connectchild(mother_data, mother_ack)
   const int mother_data;
   const int mother_ack;
{
   const char *function = "run_connectchild()";
   fd_set *rset, *wset;
#if HAVE_GSSAPI
   gss_buffer_desc gssapistate;
   char gssapistatemem[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */
   ssize_t p;
   int fdexpect, fdbits;

   slog(LOG_DEBUG, "%s: data %d, ack %d",
        function, mother_data, mother_ack);

#if 0
   sleep(20);
#endif

   SASSERTX(sockscf.state.insignal == 0);

   setproctitle("connectchild");

#if HAVE_GSSAPI
   gssapistate.value  = gssapistatemem;
   gssapistate.length = sizeof(gssapistatemem);
#endif /* HAVE_GSSAPI */

   rset = allocate_maxsize_fdset();
   wset = allocate_maxsize_fdset();

   /* CONSTCOND */
   while (1) {
      static struct {
         struct msghdr msg;
         size_t        msglen;
         struct iovec  iov[2];

         childpacket_t req;
#if HAVE_GSSAPI
         char          gssdata[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */

#if HAVE_CMSGHDR

      union {
         char   cmsgmem[CMSG_SPACE(sizeof(int) * FDPASS_MAX )];
         struct cmsghdr align;
      } cmsgdata;

#else /* !HAVE_CMSGHDR */

      char cmsgdata[CMSG_SPACE(sizeof(int) * FDPASS_MAX )];

#endif /* !HAVE_CMSGHDR */
      } *finishedv;

      static size_t finishedc;
      int wset_isset;

      errno = 0; /* reset for each iteration. */

      FD_ZERO(rset);
      FD_SET(mother_data, rset);
      FD_SET(mother_ack, rset);
      fdbits = MAX(mother_data, mother_ack);

      if (finishedc > 0) {
         slog(LOG_DEBUG,
              "%s: %lu finished requests waiting to be sent to mother",
              function, (unsigned long)finishedc);

         FD_ZERO(wset);
         FD_SET(mother_data, wset);

         fdbits = MAX(fdbits, mother_data);
         wset_isset = 1;
      }
      else
         wset_isset = 0;

      ++fdbits;

      switch (selectn(fdbits,
                      rset,
                      wset_isset ? wset : NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL)) {
         case -1:
            SASSERT(ERRNOISTMP(errno));
            continue;
      }

      if (FD_ISSET(mother_ack, rset)) {
         char buf[256];

         switch ((p = read(mother_ack, buf, sizeof(buf)))) {
            case -1:
               slog(LOG_INFO, "%s: read(): mother exited: %s",
                    function, strerror(errno));

#if HAVE_COVERAGE
               __gcov_flush();
#endif /* HAVE_COVERAGE */
               _exit(EXIT_SUCCESS);
               /* NOTREACHED */

            case 0:
               slog(LOG_INFO, "%s: read(): mother closed", function);
#if HAVE_COVERAGE
               __gcov_flush();
#endif /* HAVE_COVERAGE */
               _exit(EXIT_SUCCESS);
               /* NOTREACHED */

            default:
               SERRX(p);
         }
      }

      if (FD_ISSET(mother_data, wset)) {
         /*
          * Have finished requests we should send to mother.
          */

         while (finishedc > 0) {
            if (sendmsgn(mother_data, &finishedv[finishedc - 1].msg, 0, 0)
            == -1) {
               slog(LOG_DEBUG,
                    "%s: failed to send response to mother, again", function);
               break;
            }

            --finishedc;
         }
      }

      if (FD_ISSET(mother_data, rset)) {
         /*
          * Mother sending us a connected (or in the process of being
          * connected) socket and necessary info to negotiate with
          * proxy server.
          */
         childpacket_t req;
         struct iovec iov[2];
         socklen_t len;
         size_t tosend, fdsent;
         struct sockaddr_storage local, remote;
         struct msghdr msg;
         struct timeval tval = { sockscf.timeout.connect, (long)0 };
         int data, control, ioc, flags, savedforlater;
         CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

         ioc = 0;
         bzero(iov, sizeof(iov));
         iov[ioc].iov_base = &req;
         iov[ioc].iov_len  = sizeof(req);
         len               = (socklen_t)iov[ioc].iov_len;
         ++ioc;

         bzero(&msg, sizeof(msg));
         msg.msg_iov      = iov;
         msg.msg_iovlen   = ioc;
         msg.msg_name     = NULL;
         msg.msg_namelen  = 0;

         CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));
         if ((p = recvmsgn(mother_data, &msg, 0)) != (ssize_t)len) {
            switch (p) {
               case -1:
                  slog(LOG_INFO, "%s: recvmsgn() failed: %s",
                       function, strerror(errno));
                  break;

               case 0:
                  slog(LOG_INFO, "%s: recvmsg(): mother closed", function);
#if HAVE_COVERAGE
                  __gcov_flush();
#endif /* HAVE_COVERAGE */
                  _exit(EXIT_SUCCESS);
                  /* NOTREACHED */

               default:
                  swarn("%s: recvmsg(): got %ld of %ld",
                  function, (long)p, (long)len);
            }

            continue;
         }

         slog(LOG_DEBUG,
              "%s: received request of size %ld + %lu from mother.  ",
              function, (long)p, (unsigned long)CMSG_TOTLEN(msg));

         if (socks_msghaserrors(function, &msg))
            continue;

         /* how many descriptors are we supposed to receive? */
         switch (req.packet.req.version) {
            case PROXY_SOCKS_V4:
            case PROXY_SOCKS_V5:
            case PROXY_HTTP_10:
            case PROXY_HTTP_11:
            case PROXY_UPNP:
               fdexpect = 1;
               break;

            default:
               SERRX(req.packet.req.version);
         }

         if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
            swarnx("%s: received control message has the invalid len of %d",
                    function, (int)CMSG_TOTLEN(msg));

            continue;
         }

         SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
         SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

         len = 0;
         CMSG_GETOBJECT(control, cmsg, sizeof(control) * len++);

         switch (req.packet.req.version) {
            case PROXY_SOCKS_V4:
            case PROXY_SOCKS_V5:
            case PROXY_HTTP_10:
            case PROXY_HTTP_11:
            case PROXY_UPNP:
               data = control;   /* data channel is control channel. */
               break;

            default:
               SERRX(req.packet.req.version);
         }

         slog(LOG_DEBUG, "%s: controlfd %d, datafd %d, req.s fd %d",
              function, control, data, req.s);

         /*
          * set a default failure now, in case we don't even get a valid
          * response from the server.
          */

         if (req.packet.req.version == PROXY_SOCKS_V4)
            req.packet.res.version = PROXY_SOCKS_V4REPLY_VERSION;
         else
            req.packet.res.version = req.packet.req.version;

         socks_set_responsevalue(&req.packet.res,
                                 sockscode(req.packet.res.version,
                                 SOCKS_FAILURE));

         req.packet.req.auth         = &req.packet.state.auth;
         req.packet.req.auth->method = AUTHMETHOD_NOTSET;

         /*
          * we're not interested the extra hassle of negotiating over
          * a non-blocking socket, so set it to blocking while we
          * use it.
          */
         flags = setblocking(control, "socket used for proxy negotiation");

         errno = 0;

         len = sizeof(local);
         if ((p = getsockname(control, TOSA(&local), &len)) == 0
         && ADDRISBOUND(&local)) /* can happen on Solaris on failure. */ {
            slog(LOG_DEBUG, "%s: control local: %s",
                 function, sockaddr2string(&local, NULL, 0));

            /*
             * On Solaris 5.11, it seems to be possible for a socket,
             * used for a non-blocking connect, to fail to become
             * writable if the connect fails.  Don't know why it happens,
             * but must be a kernel bug:
             * getpeername(1, 0x080333E0, 0x0803340C, SOV_DEFAULT)
             *            Err#134 ENOTCONN
             * fcntl(1, F_SETFL, FWRITE)                       = 0
             * pollsys(0x08033220, 1, 0x00000000, 0x00000000) (sleeping...)
             *         fd=1  ev=POLLOUT rev=0
             * <never returns>
             *
             * In that case, getsockname(2) seems to return a zero address
             * also, so we try to use that to detect the problem.
             */

            slog(LOG_INFO, "%s: waiting for connect response ...", function);

            FD_ZERO(wset);
            FD_SET(control, wset);
            switch (selectn(control + 1,
                            NULL,
                            NULL,
                            NULL,
                            wset,
                            NULL,
                            sockscf.timeout.connect == 0 ? NULL : &tval)) {
               case -1:
                  if (errno == EINTR)
                     continue;

                  SERR(-1);
                  /* NOTREACHED */

               case 0:
                  swarnx("%s: select(2) timed out waiting for connect(2)",
                         function);

                  errno = ETIMEDOUT;
                  break;
            }
         }
         else {
            if (p == 0) {
              SASSERT(!ADDRISBOUND(&local));
               slog(LOG_DEBUG,
                    "%s: getsockname(control) returned an unbound address.  "
                    "Running Solaris are we?",
                    function);
            }
            else
               slog(LOG_DEBUG, "%s: getsockname(control) failed: %s",
                    function, strerror(errno));
         }

         if (errno == 0) {
            len = sizeof(errno);
            getsockopt(control, SOL_SOCKET, SO_ERROR, &errno, &len);

            slog(errno == 0 ? LOG_DEBUG : LOG_WARNING,
                 "%s: SO_ERROR says errno after connect is %d (%s)",
                 function, errno, strerror(errno));
         }

         req.packet.state.err = errno;

         len = sizeof(remote);
         if (getpeername(control, TOSA(&remote), &len) != 0) {
            if (req.packet.state.err == 0) {
               swarn("%s: no error detected, but getpeername(control) failed",
                     function);

               req.packet.state.err = errno; /* better than nothing. */
            }
            else
               swarn("%s: getpeername(control) failed", function);
         }

         slog(LOG_INFO, "%s: connect %s (errno = %d)",
              function,
              req.packet.state.err == 0 ? "succeeded" : "failed",
              req.packet.state.err);


         if (req.packet.state.err == 0) { /* connected ok. */
            char emsg[1024];

            if (socks_negotiate(data,
                                control,
                                &req.packet,
                                NULL,
                                emsg,
                                sizeof(emsg)) != 0) {
               swarnx("%s: socks_negotiate() failed: %s", function, emsg);

               req.packet.res.auth->method = AUTHMETHOD_NOTSET;
               req.packet.state.err        = errno;
            }
            else {
               slog(LOG_INFO, "%s: socks_negotiate() succeeded", function);
               req.packet.state.err = 0;
            }
         }

         /* back to original. */
         if (flags != -1)
            if (fcntl(control, F_SETFL, flags) == -1)
               swarn("%s: failed to restore flags on control-fd %d",
                     function, control);

         ioc = 0;
         bzero(iov, sizeof(iov));

         iov[ioc].iov_base  = &req;
         iov[ioc].iov_len   = sizeof(req);
         tosend             = iov[ioc].iov_len;
         ++ioc;

#if HAVE_GSSAPI
         if (req.packet.state.err == 0
         &&  req.packet.state.auth.method == AUTHMETHOD_GSSAPI) {
            gssapi_export_state(&req.packet.state.auth.mdata.gssapi.state.id,
                                &gssapistate);

            iov[ioc].iov_base  = gssapistate.value;
            iov[ioc].iov_len   = gssapistate.length;
            tosend            += iov[ioc].iov_len;
            ++ioc;

            slog(LOG_DEBUG,
                 "%s: exporting gssapistate of size %lu (start: 0x%x, 0x%x)",
                 function,
                 (unsigned long)gssapistate.length,
                 ((char *)gssapistate.value)[0],
                 ((char *)gssapistate.value)[1]);

         }
#endif /* HAVE_GSSAPI */

         bzero(&msg, sizeof(msg));
         msg.msg_iov      = iov;
         msg.msg_iovlen   = ioc;
         msg.msg_name     = NULL;
         msg.msg_namelen  = 0;

         fdsent = 0;
         CMSG_ADDOBJECT(control, cmsg, sizeof(control) * fdsent++);
         CMSG_SETHDR_SEND(msg, cmsg, sizeof(int) * fdsent);

         savedforlater = 0;
         if (sendmsgn(mother_data, &msg, 0, 0) == -1) {
            void *tmp;

            slog(LOG_INFO,
                 "%s: sending response to mother, size %ld, received fd %d, "
                 "failed: %s",
                 function, (long)tosend, req.s, strerror(errno));

            if ((tmp = realloc(finishedv, sizeof(*finishedv) * (finishedc + 1)))
            == NULL) {
               swarnx("%s: could not expand finishedv array for connect to %s",
                      function,
                      sockshost2string(&req.packet.req.host, NULL, 0));
               goto exit;
            }

            finishedv = tmp;
            bzero(&finishedv[finishedc], sizeof(finishedv[finishedc]));

            finishedv[finishedc].req = req;

            ioc = 0;
            finishedv[finishedc].iov[ioc].iov_base
            = &finishedv[finishedc].req;

            finishedv[finishedc].iov[ioc].iov_len
            = sizeof(finishedv[finishedc].req);

            SASSERTX(msg.msg_iovlen <= 2);
#if HAVE_GSSAPI
            if (msg.msg_iovlen == 2) {
               /*
                * Have gss-data too.
                */

               ++ioc;

               SASSERTX(msg.msg_iov[ioc].iov_len
               <=       sizeof(finishedv[finishedc].gssdata));

               finishedv[finishedc].iov[ioc].iov_base
               = &finishedv[finishedc].gssdata;

               memcpy(finishedv[finishedc].iov[ioc].iov_base,
                      msg.msg_iov[ioc].iov_base,
                      msg.msg_iov[ioc].iov_len);

               finishedv[finishedc].iov[ioc].iov_len
               = msg.msg_iov[ioc].iov_len;
            }
#endif /* HAVE_GSSAPI */

            SASSERTX(msg.msg_controllen
            <=       sizeof(finishedv[finishedc].cmsgdata));

            finishedv[finishedc].msg.msg_control
            = &finishedv[finishedc].cmsgdata;

            memcpy(finishedv[finishedc].msg.msg_control,
                   msg.msg_control,
                   msg.msg_controllen);

            finishedv[finishedc].msg.msg_controllen
            = msg.msg_controllen;

            SASSERTX(msg.msg_name    == NULL);
            SASSERTX(msg.msg_namelen == 0);
            finishedv[finishedc].msg.msg_name    = msg.msg_name;
            finishedv[finishedc].msg.msg_namelen = msg.msg_namelen;

            finishedv[finishedc].msg.msg_iov   = finishedv[finishedc].iov;
            finishedv[finishedc].msg.msg_iovlen = ioc;

            finishedv[finishedc].msg.msg_flags = msg.msg_flags;

            ++finishedc;
            savedforlater = 1;

            slog(LOG_DEBUG,
                 "%s: successfully saved finished connect to %s for later",
                 function,
                 sockshost2string(&req.packet.req.host, NULL, 0));
         }
         else
            slog(LOG_INFO,
                 "%s: sent response to mother, size %ld, received fd %d",
                 function, (long)tosend, req.s);

exit:
         if (!savedforlater) {
            close(control);
            if (data != control)
               close(data);
         }
      }
   }
}

static void
sigio(sig, sip, scp)
   int sig;
   siginfo_t *sip;
   void *scp;
{
   const char *function = "sigio()";
   const int errno_s = errno;
   socksfd_t socksfd;
   childpacket_t childres;
   struct msghdr msg;
   struct iovec iov[2];
   socklen_t len;
   ssize_t rc;
   size_t gotpackets;
   char emsg[256], b[5][256];
   int fdexpect, s, ioc;
#if HAVE_GSSAPI
   char gssapistatemem[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */
   CMSG_AALLOC(cmsg, sizeof(int) * FDPASS_MAX);

#ifdef HAVE_LINUX_BUGS
   /*
    * Don't know how, but on Linux, it seems possible for this to
    * happen, even though we only have one signal handler:
    *
    * #29 <signal handler called>
    * ...
    * #21 0x0000003de487cb1b in _L_lock_9857 () at hooks.c:126
    * #20 __lll_lock_wait_private ()
    *  at ../nptl/sysdeps/unix/sysv/linux/x86_64/lowlevellock.S:97
    * #19 <signal handler called>
    */
   if (sockscf.state.insignal) {
      slog(LOG_DEBUG, "%s: this shouldn't happen ..."
                      "in signal %d, and got signal %d",
                      function, sockscf.state.insignal, sig);

      return;
   }
#else /* !HAVE_LINUX_BUGS */
   SASSERTX(!sockscf.state.insignal);
#endif /* !HAVE_LINUX_BUGS */

   sockscf.state.insignal = sockscf.state.handledsignal = sig;

   if (sockscf.option.debug) {
      const char *msgv[] =
      { function,
        ": ",
        "got signal, requests outstanding: ",
        ltoa((long)reqoutstanding, b[0], sizeof(b[0])),
        NULL
      };

      signalslog(LOG_DEBUG, msgv);
   }

   /*
    * Nothing is expected over the ack pipe, but it's a stream pipe
    * so we can use it to know when our connect-child has died.
    */
   if ((rc = recv(sockscf.child_ack, &msg, sizeof(msg), 0)) != -1
   && !ERRNOISTMP(errno)) {
      const char *msgv[] =
      { function,
        ": ",
        "ick ick ick.  It seems our dear connect-child has suffered "
        "unrepairable problems and sent us a message of size ",
        ltoa(rc, b[0], sizeof(b[0])),
        "over the ack-pipe.  Probably we will just hang now",
        NULL
      };

      signalslog(LOG_DEBUG, msgv);

      sockscf.connectchild = 0;
      close(sockscf.child_ack);
      close(sockscf.child_data);

      /*
       * Should try to go through all in-progress sessions sent to
       * connectchild, via socks_addrmatch() or similar, and either
       * invalidate them or fork a new connectchild and try again,
       * but that's a lot of work for something that should never
       * happen.
       */
      sockscf.state.insignal = 0;
      return;
   }

   if (originalhandler.sa_flags & SA_SIGINFO
   &&  originalhandler.sa_sigaction != NULL) {
      const char *msgv[] =
      { function,
        ": ",
        "calling original sa_sigaction()",
        NULL
      };

      signalslog(LOG_DEBUG, msgv);

      originalhandler.sa_sigaction(sig, sip, scp);
   }
   else {
      if (originalhandler.sa_handler != SIG_IGN
      &&  originalhandler.sa_handler != SIG_DFL) {
         const char *msgv[] =
         { function,
           ": ",
           "calling original sa_handler()",
           NULL
         };

         signalslog(LOG_DEBUG, msgv);

         originalhandler.sa_handler(sig);
      }
   }

   if (sockscf.connectchild == 0) {
      sockscf.state.insignal = 0;
      return;
   }

   bzero(iov, sizeof(iov));
   ioc = 0;

   iov[ioc].iov_base = &childres;
   iov[ioc].iov_len  = sizeof(childres);
   ++ioc;

#if HAVE_GSSAPI
   iov[ioc].iov_base = gssapistatemem;
   iov[ioc].iov_len  = sizeof(gssapistatemem);
   ++ioc;
#endif /* HAVE_GSSAPI */

   bzero(&msg, sizeof(msg));
   msg.msg_iov      = iov;
   msg.msg_iovlen   = ioc;
   msg.msg_name     = NULL;
   msg.msg_namelen  = 0;

   if (sockscf.option.debug) {
      const char *msgv[] =
      { function,
        ": ",
        "trying to receive msg from child ...",
        NULL
      };

      signalslog(LOG_DEBUG, msgv);
   }

   gotpackets = 0;

   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));
   while ((rc = recvmsgn(sockscf.child_data, &msg, 0))
   >= (ssize_t)sizeof(childres)) {
      const char *msgv[] =
      { function,
        ": ",
        "received msg of size ",
        ltoa((long)rc, b[0], sizeof(b[0])),
        " + ",
        ltoa((long)CMSG_TOTLEN(msg), b[1], sizeof(b[1])),
        " from child for childres.s.  fd = ",
        ltoa((long)childres.s, b[2], sizeof(b[2])),
        ".  Requests outstanding: ",
        ltoa((long)reqoutstanding, b[3], sizeof(b[3])),
        NULL
      };

      struct sockaddr_storage localmem,  *local  = &localmem;
      struct sockaddr_storage remotemem, *remote = &remotemem;
      int child_s;

      ++gotpackets;
      --reqoutstanding;

      signalslog(LOG_DEBUG, msgv);

      if (socks_msghaserrors(function, &msg))
         continue;

      fdexpect = 1;
      if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
         const char *msgv[] =
         { function,
           ": ",
           "received controlmessage has the invalid length of ",
           ltoa((long)CMSG_TOTLEN(msg), b[0], sizeof(b[0])),
           NULL,
         };

         signalslog(LOG_WARNING, msgv);
         continue;
      }

      SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
      SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

      len = 0;
      CMSG_GETOBJECT(child_s, cmsg, sizeof(child_s) * len++);

      if (sockscf.option.debug) {
         const char *msgv[] =
         { function,
           ": ",
           "child_s = ",
           ltoa((long)child_s, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }

      SASSERTX(fdisopen(child_s));

      rc -= sizeof(childres);

      /*
       * if an address has been associated with fdindex child_s before,
       * it can't possibly be valid any more.
       */

      socks_rmaddr(child_s, 0);

      len = sizeof(*local);
      if (getsockname(child_s, TOSA(local), &len) == 0) {
         const char *msgv[] =
         { function,
           ": local = ",
           "sockaddr2string(local, NULL, 0)",
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }
      else {
         const char *msgv[] =
         { function,
           ": ",
           "getsockname() on socket failed, errno ",
           ltoa((long)errno, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }

      len = sizeof(*remote);
      if (getpeername(child_s, TOSA(remote), &len) == 0) {
         const char *msgv[] =
         { function,
           ": ",
           "remote = ",
           "sockaddr2string(remote, NULL, 0)",
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }
      else {
         const char *msgv[] =
         { function,
           ": ",
           "getpeername() on socket failed, errno ",
           ltoa((long)errno, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }

      childres.packet.req.auth
      = childres.packet.res.auth
      = &childres.packet.state.auth;

      if (childres.packet.state.err != 0) {
         const char *msgv[] =
         { function,
           ": ",
           "child failed to establish a session: errno = ",
           ltoa((long)childres.packet.state.err, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_WARNING, msgv);
      }
      else {
         const char *msgv[] =
         { function,
           ": ",
           "auth method child negotiated is ",
           ltoa((long)childres.packet.res.auth->method, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_INFO, msgv);
      }

      s = socks_addrcontrol(childres.s, child_s, 0);
      close(child_s);

      if (s == -1) {
         const char *msgv[] =
         { function,
           ": ",
           "socks_addrcontrol() for fd ",
           ltoa((long)childres.s, b[0], sizeof(b[0])),
           " failed.  Assuming the fd has been recycled and is used for "
           "something else now",
           NULL
         };

         signalslog(LOG_INFO, msgv);

         /*
          * XXX if not, and something prevented socks_addrcontrol() from
          * working, the client may hang forever.
          * Problem on OpenBSD due to bug mentioned in fdisdup().
          */

         CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg)); /* for next msg. */
         continue;
      }

      if (sockscf.option.debug) {
         const char *msgv[] =
         { function,
           ": ",
           "packet belongs to fd ",
           ltoa((long)s, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }

      if (socks_getaddr(s, &socksfd, 0) == NULL) {
         const char *msgv[] =
         { function,
           ": ",
           "could not socks_getaddr() fd ",
           ltoa((long)s, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_WARNING, msgv);
         break;
      }

      socksfd.state.err = childres.packet.state.err;

      switch (socksfd.state.version) {
         case PROXY_SOCKS_V4:
         case PROXY_SOCKS_V5:
         case PROXY_HTTP_10:
         case PROXY_HTTP_11:
         case PROXY_UPNP:
            if (socksfd.control == s) {
               const char *msgv[] =
               { function,
                 ": ",
                 "duping fd ",
                 ltoa((long)socksfd.control, b[0], sizeof(b[0])),
                 " not needed",
                 NULL
               };

               signalslog(LOG_DEBUG, msgv);
               break;
            }

            if (sockscf.option.debug) {
               const char *msgv[] =
               { function,
                 ": ",
                 "duping fd ",
                 ltoa((long)socksfd.control, b[0], sizeof(b[0])),
                 " to fd ",
                 ltoa((long)s, b[1], sizeof(b[1])),
                 NULL
               };

               signalslog(LOG_DEBUG, msgv);
            }

            if (dup2(socksfd.control, s) == -1) {
               const char *msgv[] =
               { function,
                 ": ",
                 "dup2(",
                 ltoa((long)socksfd.control, b[0], sizeof(b[0])),
                 ", ",
                 ltoa((long)s, b[1], sizeof(b[1])),
                 " failed with errno",
                 ltoa((long)errno, b[2], sizeof(b[2])),
                 NULL
               };

               signalslog(LOG_WARNING, msgv);

               SASSERT(errno != EBADF);

               socks_addaddr(s, &socksfd, 0);
               break;
            }

            close(socksfd.control);
            socksfd.control = s;
            socks_addaddr(s, &socksfd, 0);
            break;

         default:
            SERRX(socksfd.state.version);
      }

      /*
       * it's possible endpoint changed/got fixed.  Update in case.
       */

      len = sizeof(socksfd.local);
      if (getsockname(s, TOSA(&socksfd.local), &len) != 0) {
         const char *msgv[] =
         { function,
           ": ",
           "getsockname() failed with errno ",
           ltoa((long)errno, b[0], sizeof(b[0])),
           NULL
         };

         signalslog(LOG_DEBUG, msgv);

         socks_rmaddr(s, 0);

         CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg)); /* for next. */
         continue;
      }
      else {
         const char *msgv[] =
         { function,
           ": ",
           "socksfd.local: ",
           "sockaddr2string(&socksfd.local, NULL, 0)",
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }

      len = sizeof(socksfd.server);
      if (getpeername(s, TOSA(&socksfd.server), &len) == 0) {
         const char *msgv[] =
         { function,
           ": ",
           "socksfd.server: ",
           "sockaddr2string(&socksfd.server, NULL, 0)",
           NULL
         };

         signalslog(LOG_DEBUG, msgv);
      }
      else {
         const char *msgv[] =
         { function,
           ": ",
           "second getpeername() on socket ",
           ltoa((long)s, b[0], sizeof(b[0])),
           " failed, errno ",
           ltoa((long)errno, b[1], sizeof(b[1])),
           NULL
         };

         signalslog(LOG_WARNING, msgv);
      }

      socksfd.state.inprogress = 0;
      socks_addaddr(s, &socksfd, 0);

      if (socksfd.state.err != 0
      ||  !serverreplyisok(childres.packet.res.version,
                           childres.packet.req.command,
                           socks_get_responsevalue(&childres.packet.res),
                           socksfd.route,
                           emsg,
                           sizeof(emsg))) {
         const char *msgv[] =
         { function,
           ": ",
           proxyprotocolisknown(childres.packet.res.version) ?
                   "proxy server did not perform request"
              :    "communication with proxy server failed",
           socksfd.state.err == 0 ? emsg : NULL,
           NULL
         };

         signalslog(LOG_WARNING, msgv);

         if (socksfd.state.err == 0
         && !proxyprotocolisknown(childres.packet.res.version)) /* no idea. */
            socksfd.state.err = EPROTONOSUPPORT;

         if (socksfd.state.err == 0)
            socksfd.state.err = errno;
         else
            errno = socksfd.state.err;

         if (errno == 0)
            socksfd.state.err = EPROTONOSUPPORT; /* no idea. */
         else {
            const char *msgv[] =
            { function,
              ": ",
              "connectchild failed to set up connection.  "
              "Error mapped to errno ",
              ltoa((long)errno, b[0], sizeof(b[0])),
              NULL
            };

            signalslog(LOG_WARNING, msgv);
         }

         socks_addaddr(s, &socksfd, 0);

         /*
          * XXX If it's a server error it would be nice to retry, could
          * be there's a backup route.
          */
         break;
      }

      if (1) {
         const char *msgv[] =
         { function,
           ": ",
           "server reply is ok, server will use as source "
           "sockshost2string(&childres.packet.res.host, NULL, 0)",
           NULL,
         };

         signalslog(LOG_INFO, msgv);
      }

      socksfd.state.auth         = *childres.packet.res.auth;
      sockshost2sockaddr(&childres.packet.res.host, &socksfd.remote);

#if HAVE_GSSAPI
      if (socksfd.state.auth.method == AUTHMETHOD_GSSAPI) {
         /*
          * can't import gssapi state here.  We're in a signal handler
          * and that is not safe.  Will be imported upon first call to
          * socks_getaddr() later, so just save it for now.
          */
         const char *msgv[] =
         { function,
           ": ",
           "read gssapistate of size ",
           ltoa((long)rc, b[0], sizeof(b[0])),
           " for fd ",
           ltoa((long)s, b[1], sizeof(b[1])),
           " (startvalues: ",
           ltoa((long)gssapistatemem[0], b[2], sizeof(b[2])),
           ", ",
           ltoa((long)gssapistatemem[1], b[3], sizeof(b[3])),
           ").  Will import later",
           NULL
         };

         signalslog(LOG_DEBUG, msgv);

         SASSERTX(rc > 0);

         socksfd.state.gssimportneeded    = 1;
         socksfd.state.gssapistate.value  = socksfd.state.gssapistatemem;
         socksfd.state.gssapistate.length = rc;

         SASSERTX(sizeof(socksfd.state.gssapistatemem) >= (size_t)rc);
         memcpy(socksfd.state.gssapistate.value, gssapistatemem, rc);

      }
#endif /* HAVE_GSSAPI */

      socks_addaddr(s, &socksfd, 0);

      /* needed for standard socks bind. */
      sockscf.state.lastconnect = socksfd.forus.connected;

#if 0
      {
         static int init;

         if (!init) {
            slog(LOG_DEBUG, "%s: XXX sleeping", function);
            init = 1;
            sleep(20);
         }
      }
#endif

      CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));
   }

   if (gotpackets) {
      const char *msgv[] =
      { function,
        ": ",
        "returning after having received ",
        ltoa((long)gotpackets, b[0], sizeof(b[0])),
        " packets.  Requests still outstanding: ",
        ltoa((long)reqoutstanding, b[1], sizeof(b[1])),
        NULL
      };

      signalslog(LOG_DEBUG, msgv);
   }
   else {
      const char *msgv[] =
      { function,
        ": ",
        "received no packets from child this time (read the packet last "
        "time presumably), rc = ",
        ltoa((long)rc, b[0], sizeof(b[0])),
        NULL
      };

      signalslog(LOG_DEBUG, msgv);
   }

   errno = errno_s;
   sockscf.state.insignal = 0;
}

