/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2005, 2006, 2008, 2009,
 *               2010, 2011, 2012, 2013
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
"$Id: Rbind.c,v 1.215 2013/10/27 15:24:42 karls Exp $";

int
Rbind(s, _name, namelen)
   int s;
   const struct sockaddr *_name;
   socklen_t namelen;
{
   const char *function = "Rbind()";
   struct sockaddr_storage namemem, *name = &namemem;
   authmethod_t auth;
   socksfd_t socksfd;
   socklen_t len;
   socks_t packet;
   char emsg[256];
   int val, rc, flags, errno_s;

   clientinit();

   /* hack for performance testing. */
   if (socks_getenv(ENV_SOCKS_BINDLOCALONLY, dontcare) != NULL)
      return bind(s, _name, namelen);

   if (_name == NULL) {
      slog(LOG_DEBUG, "%s: fd %d, _name = %p",
           function, s, _name);

      return bind(s, _name, namelen);
   }

   usrsockaddrcpy(name, TOCSS(_name), salen(_name->sa_family));

   slog(LOG_DEBUG, "%s, fd %d, address %s",
        function, s, sockaddr2string(TOSS(name), NULL, 0));

   /*
    * Nothing can be called before Rbind(), delete any old cruft.
    */
   socks_rmaddr(s, 1);

   if (TOSA(name)->sa_family != AF_INET) {
      slog(LOG_INFO, "%s: fd %d, unsupported af %s, system fallback",
           function, s, safamily2string(name->ss_family));

      return bind(s, _name, namelen);
   }

   if (socks_socketisforlan(s)) {
      slog(LOG_INFO, "%s: fd %d is for lan only, system bind fallback",
           function, s);

      return bind(s, _name, namelen);
   }

   bzero(&auth, sizeof(auth));
   auth.method               = AUTHMETHOD_NOTSET;

   bzero(&packet, sizeof(packet));
   packet.req.version        = PROXY_DIRECT;
   packet.req.command        = SOCKS_BIND;
   packet.req.host.atype     = SOCKS_ADDR_IPV4;
   packet.req.host.addr.ipv4 = TOIN(&sockscf.state.lastconnect)->sin_addr;
   packet.req.host.port      = GET_SOCKADDRPORT(name);
   packet.req.auth           = &auth;

   len = sizeof(val);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &val, &len) != 0) {
      swarn("%s: getsockopt(SO_TYPE)", function);
      return -1;
   }

   switch (val) {
      case SOCK_DGRAM:
         packet.req.protocol = SOCKS_UDP;
         break;

      case SOCK_STREAM:
         packet.req.protocol = SOCKS_TCP;
         break;

      default:
         swarnx("%s: unknown socket-type %d, falling back to system bind(2)",
                function, val);

         return bind(s, _name, namelen);
   }

   bzero(&socksfd, sizeof(socksfd));

   if ((socksfd.route = socks_requestpolish(&packet.req, NULL, NULL)) == NULL) {
      /*
       * If we just do a local bind, and pretend things are ok, that
       * means our client can fetch the local address we bound and
       * send it to a peer, which can then send data to us without going
       * via the proxy.  If that is not configured by routes (and direct
       * fallback is presumably disabled), we can not be sure our client
       * wants that to happen.  If the client us running in an environment
       * where it is more important to not leak traffic outside of the
       * proxy connection, it surly does not want us to make a best
       * effort to transfer the traffic; it has to flow via the proxy
       * or not flow at all.
       *
       * So what we do is pretend we've bound an address, but we have
       * not really done that, not even locally.
       * This should prevent us from receiving non-proxied traffic, but
       * might make things work for a subset of other operations, where
       * the client does not expect to receive traffic before it has sent
       * any itself (i.e., sendt UDP packets or performed a TCP (or UDP)
       * connect), and where it does not expect to use the local address it
       * thinks it has bound to set up a rendezvous of any sort.
       *
       * This should prevent us from leaking traffic directly, while still
       * having a chance of things working without requiring an explicit
       * direct route for e.g. UDP bind, which is not supported by most
       * of the proxy protocols we support, and perhaps even work in
       * cases where we are just using a primitive http proxy that only
       * supports connect(2).
       *
       * Note that this is not the same that happens for a direct route,
       * because we do not actually bind a local address and we will
       * not be able to accept any traffic on this socket until the
       * client first sends traffic on this socket (at which point some
       * sort of local address will have to be bound).
       */

      slog(LOG_INFO,
           "%s: no route found for binding %s address %s.  Pretending we've "
           "done it anyway.  This may result in strange errors later, but "
           "there is also a fair chance this will work, if the client does "
           "not actually plan to use the bound address for anything until "
           "after it has itself sendt the first traffic",
           function,
           protocol2string(packet.req.protocol),
           sockaddr2string(name, NULL, 0));

      errno = 0;
      return 0;
   }

   packet.version = packet.req.version;

   if (socksfd.route->gw.state.proxyprotocol.direct) {
      slog(LOG_DEBUG, "%s: using system bind(2) for fd %d", function, s);
      return bind(s, _name, namelen);
   }
   /*
    * Else: client requests us to bind a specific port, but if we are
    * socksifying that request, there is no need to bind any port locally.
    */

   /*
    * no separate control socket (except bind-extension case, but
    * that is dealt with in Raccept().
    */
   socksfd.control = s;

   if (socks_routesetup(socksfd.control, s, socksfd.route, emsg, sizeof(emsg))
   != 0) {
      swarnx("%s: socks_routesetup() failed: %s", function, emsg);

      if (socksfd.control != s)
         close(socksfd.control);

      return -1;
   }

   /*
    * we're not interested the extra hassle of negotiating over
    * a non-blocking socket, so make sure it's blocking while we
    * use it.
    */
   flags = setblocking(socksfd.control, "socket used for negotiation");

   socksfd.route = socks_connectroute(socksfd.control,
                                      &packet,
                                      NULL,
                                      NULL,
                                      emsg,
                                      sizeof(emsg));

   if (socksfd.route == NULL || socksfd.route->gw.state.proxyprotocol.direct) {
      if (flags != -1)
         if (fcntl(socksfd.control, F_SETFL, flags) == -1)
            swarn("%s: fcntl(s)", function);
   }

   if (socksfd.route == NULL) {
      swarnx("%s: could not connect route: %s", function, emsg);

      errno = EADDRNOTAVAIL;
      return -1;
   }

   if (socksfd.route->gw.state.proxyprotocol.direct) {
      slog(LOG_INFO,
           "%s: strange ... did our previously found route get blacklisted "
           "in the meantime?  Using system bind(2) for fd %d",
           function, s);

      return bind(s, _name, namelen);
   }

   rc = socks_negotiate(s,
                        socksfd.control,
                        &packet,
                        socksfd.route,
                        emsg,
                        sizeof(emsg));

   errno_s = errno;

   if (flags != -1)
      if (fcntl(socksfd.control, F_SETFL, flags) == -1)
         swarn("%s: fcntl(s)", function);

   if (rc != 0) {
      errno = errno_s;

      swarnx("%s: socks_negotiate() failed: %s", function, emsg);

      slog(LOG_DEBUG,
           "%s: returning after socks_negotiate() failure with errno = %d (%s)",
           function, errno, strerror(errno));

      return -1;
   }

   update_after_negotiate(&packet, &socksfd);

   if (packet.req.protocol == SOCKS_TCP)
      socksfd.state.protocol.tcp = 1;
   else if (packet.req.protocol == SOCKS_UDP)
      socksfd.state.protocol.udp = 1;

   socksfd.state.version = packet.req.version;
   sockshost2sockaddr(&packet.res.host, &socksfd.remote);

   switch (packet.req.version) {
      case PROXY_SOCKS_V4:
         if (TOIN(&socksfd.remote)->sin_addr.s_addr == htonl(0)) {
            /*
             * v4 specific; server doesn't say, so should set it to address
             * we connected to for the control connection.
             */
            struct sockaddr_in addr;

            len = sizeof(addr);
            if (getpeername(socksfd.control, TOSA(&addr), &len) != 0)
               SERR(-1);

            TOIN(&socksfd.remote)->sin_addr = addr.sin_addr;
         }
         /* FALLTHROUGH */

      case PROXY_SOCKS_V5:
         socksfd.reply                = socksfd.remote;   /* same IP address. */
         socksfd.state.acceptpending  = socksfd.route->gw.state.extension.bind;
         break;

      case PROXY_UPNP:
         /* don't know what address connection will be forwarded from yet. */
         socksfd.state.acceptpending = 1;
         break;

      default:
         SERRX(packet.req.version);
   }

   /* did we get the requested port? */
   if (TOIN(name)->sin_port != htons(0)
   &&  TOIN(name)->sin_port != TOIN(&socksfd.remote)->sin_port) { /* no. */
      socks_freebuffer(socksfd.control);

      slog(LOG_INFO,
           "%s: proxyserver did not let us bind the requested port, %u.  "
           "Proxyserver offered us instead port %u, so failing",
           function,
           ntohs(GET_SOCKADDRPORT(name)),
           ntohs(GET_SOCKADDRPORT(&socksfd.remote)));

      errno = EADDRINUSE;
      return -1;
   }

   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) != 0) {
      swarn("%s: getsockname() of fd %d failed", function, s);
      return -1;
   }

   slog(LOG_DEBUG, "%s: address of fd %d is %s",
        function, s, sockaddr2string(&socksfd.local, NULL, 0));


   if (socksfd.control != s) {
      len = sizeof(socksfd.server);
      if (getpeername(socksfd.control, TOSA(&socksfd.server), &len) != 0) {
         return -1;
      }
   }

   if (socksfd.state.acceptpending)
      /*
       * will accept(2) connection on control as for normal sockets,
       * so no need to use a buffer for the control/listening socket;
       * if a buffer is needed it will be for connection we later accept(2).
       */
      socks_freebuffer(socksfd.control);

   switch (socksfd.state.version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V5:
      case PROXY_UPNP:
         socks_addaddr(s, &socksfd, 1);
         break;

      default:
         SERRX(socksfd.state.version);
   }

   slog(LOG_INFO, "%s: successfully bound address %s for fd %d",
        function, sockaddr2string(&socksfd.remote, NULL, 0), s);

   return 0;
}

#if 0
/*
 * Disabled since this idea will not work due to NAT. :-/
 */

static int socks_bind_udp(const int s, struct sockaddr_storage *addr);
/*
 * Attempts to bind udp address "addr" at the socks server for socket "s".
 *
 * Returns 0 on success, -1 on failure, with errno set appropriately if so.
 */

static int
socks_bind_udp(s, addr)
   const int s;
   struct sockaddr_storage *addr;
{
   /*
    * The socks protocol does not support binding udp sockets, but we try
    * to implement the same by creating a normal socks udp association
    * and sending a packet to ourselves.  The senders address for that
    * packet should be the address the socks server is using on our
    * behalf.
    * Unfortunately this does not work in most personal cases due to
    * NAT - the socks server on the other side of the NAT-box can not send
    * data to our (private) address.
    */
   const char *function = "socks_bind_udp()";
   const int errno_s = errno;
   struct sockaddr_storage from;
   socksfd_t socksfd;
   socklen_t len;
   ssize_t rc;
   char *p, emsg[1024], buf[256];

   errno = EADDRNOTAVAIL; /* default errorcode. */

   slog(LOG_INFO, "%s: fd %d, address %s",
        function, s, sockaddr2string(addr, NULL, 0));

   /*
    * Hackish and fragile, but need to do the things necessary udpsetup()
    * can understand this really is our fd, and not something that slipped
    * through the cracks.
    */

   bzero(&socksfd, sizeof(socksfd));

   socksfd.state.command = SOCKS_BIND;

   len = sizeof(socksfd.local);
   if (getsockname(s, TOSA(&socksfd.local), &len) == -1)
      return -1;

   socks_addaddr(s, &socksfd, 1);

   socksfd.route = udpsetup(s,
                            &socksfd.local,
                            SOCKS_SEND,
                            0,
                            emsg,
                            sizeof(emsg));

   if (socksfd.route == NULL) {
      slog(LOG_INFO, "%s: udpsetup() for udp bind on fd %d failed: %s",
           function, s, emsg);

      return -1;
   }

   if (socks_getaddr(s, &socksfd, 1) == 0) {
      swarnx("%s: strange, socks_getaddr() on fd %d failed after udpsetup()",
             function, s);

      return -1;
   }

   slog(LOG_INFO,
        "%s: created udp association for data fd %d successfully.  Now trying "
        "to send a small packet to ourselves on address %s",
        function, s, sockaddr2string(&socksfd.local, NULL, 0));

   p = "I just want to know what address you are using on my behalf";
   if (Rsendto(s, p, strlen(p), 0, TOSA(&socksfd.local), sizeof(socksfd.local))
   !=  (ssize_t)strlen(p)) {
      slog(LOG_INFO,
           "%s: send of packet to ourselves at address %s to finish setting "
           "up udp bind for fd %d failed",
           function, sockaddr2string(&socksfd.local, NULL, 0), s);

      return -1;
   }

   slog(LOG_DEBUG,
        "%s: successfully sent udp packet to ourselves on address %s "
        "via proxy.  Now waiting for the reply",
        function, sockaddr2string(&socksfd.local, NULL, 0));

   len = sizeof(from);
   if ((rc = recvfrom(s, buf, sizeof(buf), 0, TOSA(&from), &len))
   != (ssize_t)strlen(p)) {
      swarn("%s: expected to receive our own udp packet of length %lu back, "
            "but received %ld byte%s instead",
            function,
            (unsigned long)strlen(p),
            (long)rc,
            rc == 1 ? "" : "s");

      return -1;
   }

   if (strcmp(buf, p) == 0)
      slog(LOG_INFO, "%s: received our own %lu bytes from %s: all as expected",
           function, rc, sockaddr2string(&from, NULL, 0));
   else {
      char visbuf[sizeof(buf) * 4];

      swarnx("%s: bytes received from %s not what we expected.  "
             "Expected %s, got %s",
             function,
             sockaddr2string(&from, NULL, 0),
             p,
             str2vis(buf, rc, visbuf, sizeof(visbuf)));

      return -1;
   }

   return -1;

   errno = errno_s;;
   return 0;
}

#endif
