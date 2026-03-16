/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2004, 2005, 2008, 2009, 2010,
 *               2011, 2012, 2013, 2014
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

 /*
  * The gssapi code was contributed by
  * Markus Moeller (markus_moeller at compuserve.com).
  */


#include "common.h"

#if SOCKS_CLIENT || SOCKS_SERVER /* XXX is this correct? */
#include "interposition.h"
#endif /* SOCKS_CLIENT || SOCKS_SERVER */

static const char rcsid[] =
"$Id: clientprotocol.c,v 1.225.4.4.6.1 2021/01/07 15:46:46 karls Exp $";

static int
recv_sockshost(int s, sockshost_t *host, int version, authmethod_t *auth,
               char *emsg, const size_t emsglen);
/*
 * Fills "host" based on data read from "s".  "version" is the version
 * the remote peer is expected to send data in.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

int
socks_sendrequest(s, request, emsg, emsglen)
   int s;
   const request_t *request;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_sendrequest()";
   ssize_t rc;
   size_t len;
   unsigned char requestmem[sizeof(*request)], *p = requestmem;

   switch (request->version) {
      case PROXY_SOCKS_V4:
         /*
          * VN   CD  DSTPORT DSTIP USERID   0
          *  1 + 1  +   2   +  4  +  ?    + 1  = 9 + USERID
          */

         /* VN */
         memcpy(p, &request->version, sizeof(request->version));
         p += sizeof(request->version);

         /* CD */
         memcpy(p, &request->command, sizeof(request->command));
         p += sizeof(request->command);

         p = sockshost2mem(&request->host, p, request->version);

         *p = NUL; /* not bothering to send any userid.  Should we? */
         ++p;
         break;

       case PROXY_SOCKS_V5:
         /*
          * rfc1928 request:
          *
          *   +----+-----+-------+------+----------+----------+
          *   |VER | CMD |  FLAG | ATYP | DST.ADDR | DST.PORT |
          *   +----+-----+-------+------+----------+----------+
          *   | 1  |  1  |   1   |  1   | Variable |    2     |
          *   +----+-----+-------+------+----------+----------+
          *     1      1     1      1       > 0         2
          *
          *   Which gives a fixed size of minimum 7 octets.
          *   The first octet of DST.ADDR when it is SOCKS_ADDR_DOMAINNAME
          *   contains the length of DST.ADDR.
          */

         /* VER */
         memcpy(p, &request->version, sizeof(request->version));
         p += sizeof(request->version);

         /* CMD */
         memcpy(p, &request->command, sizeof(request->command));
         p += sizeof(request->command);

         /* FLAG */
         memcpy(p, &request->flag, sizeof(request->flag));
         p += sizeof(request->flag);

         p = sockshost2mem(&request->host, p, request->version);
         break;

       default:
         SERRX(request->version);
   }

   slog(LOG_NEGOTIATE, "%s: sending request to server: %s",
        function, socks_packet2string(request, 1));

   /*
    * Send the request to the server.
    */
   len = p - requestmem;
   if ((rc = socks_sendton(s,
                           requestmem,
                           len,
                           len,
                           0,
                           NULL,
                           0,
                           NULL,
                           request->auth)) != (ssize_t)len) {
      snprintf(emsg, emsglen,
               "could not send request to proxy server.  Sent %ld/%lu: %s",
               (long)rc, (unsigned long)len, strerror(errno));
      return -1;
   }

   return 0;
}

int
socks_recvresponse(s, response, version, emsg, emsglen)
   int s;
   response_t   *response;
   int version;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_recvresponse()";
   ssize_t rc;

   /* get the version specific data that prefixes the sockshost. */
   switch (version) {
      case PROXY_SOCKS_V4: {
         /*
          * The socks V4 reply length is fixed:
          * VN   CD  DSTPORT  DSTIP
          *  1 + 1  +   2   +   4
          */
         char responsemem[ sizeof(response->version)
                         + sizeof(response->reply.socks)
                         ];
         char *p = responsemem;

         if ((rc = socks_recvfromn(s,
                                   responsemem,
                                   sizeof(responsemem),
                                   sizeof(responsemem),
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   response->auth))
         != (ssize_t)sizeof(responsemem)) {
            fmtresponseerror(rc, sizeof(responsemem), emsg, emsglen);
            return -1;
         }

         /* VN */
         memcpy(&response->version, p, sizeof(response->version));
         p += sizeof(response->version);
         if (response->version != PROXY_SOCKS_V4REPLY_VERSION) {
            fmtversionerror(PROXY_SOCKS_V4REPLY_VERSION,
                            response->version,
                            emsg,
                            emsglen);
            return -1;
         }

         /* CD */
         memcpy(&response->reply.socks, p, sizeof(response->reply.socks));
         p += sizeof(response->reply.socks);
         break;
      }

      case PROXY_SOCKS_V5: {
         /*
          * rfc1928 reply:
          *
          * +----+-----+-------+------+----------+----------+
          * |VER | REP |  FLAG | ATYP | BND.ADDR | BND.PORT |
          * +----+-----+-------+------+----------+----------+
          * | 1  |  1  |   1   |  1   |  > 0     |    2     |
          * +----+-----+-------+------+----------+----------+
          *
          *   Which gives a size of >= 7 octets.
          *
          */
         char responsemem[sizeof(response->version)
                        + sizeof(response->reply.socks)
                        + sizeof(response->flag)
                        ];
         char *p = responsemem;

         if ((rc = socks_recvfromn(s,
                                   responsemem,
                                   sizeof(responsemem),
                                   sizeof(responsemem),
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   response->auth))
         != (ssize_t)sizeof(responsemem)) {
            fmtresponseerror(rc, sizeof(responsemem), emsg, emsglen);
            return -1;
         }

         /* VER */
         memcpy(&response->version, p, sizeof(response->version));
         p += sizeof(response->version);
         if (version != response->version) {
            fmtversionerror(version, response->version, emsg, emsglen);
            return -1;
         }

         /* REP */
         memcpy(&response->reply.socks, p, sizeof(response->reply.socks));
         p += sizeof(response->reply.socks);

         /* FLAG */
         memcpy(&response->flag, p, sizeof(response->flag));
         p += sizeof(response->flag);

         break;
      }

      default:
         SERRX(version);
   }

   if (recv_sockshost(s,
                      &response->host,
                      version,
                      response->auth,
                      emsg,
                      emsglen) != 0)
      return -1;

   slog(LOG_NEGOTIATE, "%s: received response from server: %s",
        function, socks_packet2string(response, 0));

   return 0;
}

int
socks_negotiate(s, control, packet, route, emsg, emsglen)
   int s;
   int control;
   socks_t *packet;
   route_t *route;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "socks_negotiate()";
   char sbuf[512], cbuf[512];
   int failed = 0;

   slog(LOG_NEGOTIATE,
        "%s: initiating %s negotiation with control-fd %d (%s), "
        "data-fd %d (%s), req.host = %s",
        function,
        proxyprotocol2string(packet->req.version),
        control,
        control == -1 ? "N/A" : socket2string(control, cbuf, sizeof(cbuf)),
        s,
        s == control ? "same" : socket2string(s, sbuf, sizeof(sbuf)),
        sockshost2string(&packet->req.host, NULL, 0));

   /* avoid false Valgrind warning later due to uninitialized padding bits. */
   bzero(&packet->res.host, sizeof(packet->res.host));

   packet->res.auth = packet->req.auth;

   switch (packet->req.version) {
      case PROXY_SOCKS_V4:
         if (packet->req.command == SOCKS_BIND) {
            if (route != NULL && route->gw.state.extension.bind)
               packet->req.host.addr.ipv4.s_addr = htonl(BINDEXTENSION_IPADDR);
#if SOCKS_CLIENT
            else {
               if (packet->req.version == PROXY_SOCKS_V4)
                   /*
                    * v4/v5 difference.  We always set up for v5 by default,
                    * but if v4 is what proxyserver us, modify the request
                    * slightly for v4.
                    */
                  if (ntohs(sockscf.state.lastconnect.port) != 0)
                     packet->req.host.port = sockscf.state.lastconnect.port;
            }
#endif /* SOCKS_CLIENT */
         }

         /* FALLTHROUGH */

      case PROXY_SOCKS_V5: {
         /*
          * Whatever these file descriptor-indexes were used for before, we
          * need to reset them now.
          */
         int rc;

#if SOCKS_CLIENT
         int original_s; /* false gcc warning: may be used uninitialized */
         int executingdnscode;

         socks_rmaddr(s, 1);
         socks_rmaddr(control, 1);

         /*
          * Some resolverlibrary have bugs whose side-effect leads to
          * them either closing the socket they created and called us
          * on (e.g., for connect(2)), or they (naturally) don't expect
          * that when they call connect(2), connect(2) will end up
          * calling them again. The later can happen when their connect(2)
          * is caught by our Rconnect(), and our Rconnect() needs to
          * call one of the dns-functions to reach the socks-server.
          *
          * We attempt to slightly increase the chances of this working
          * by dup(2)'ing the socket they called us with so that after
          * method_negotiate(), which may end up calling dns-functions,
          * returns, we still have the original socket they called us
          * with, even if the dns-calls made by method_negotiate() ended
          * up closing and recreating it.
          */

         SASSERTX(sockscf.state.executingdnscode >= 0);
         if (sockscf.state.executingdnscode
         &&  s != control
            /* workaround is only usable for udp. */
         &&  packet->req.command == SOCKS_UDPASSOCIATE)
            executingdnscode = 1;
         else
            executingdnscode = 0;

         if (executingdnscode) {
            slog(LOG_DEBUG,
                 "%s: preparing to call method_negotiate() from dns-code",
                 function);

            if ((original_s = dup(s)) == -1)
               swarn("%s: dup() failed on fd %d while executing dns-code",
                     function, s);
            else {
               int tmp_s = socketoptdup(s, -1);

               if (tmp_s == -1)
                  swarn("%s: socketoptdup() failed on fd %d while executing "
                        "dns-code",
                        function, s);
               else {
                  rc = dup2(tmp_s, s);
                  close(tmp_s);

                  if (rc == s) {
                     slog(LOG_DEBUG,
                          "%s: successfully prepared things.  Data-fd %d is "
                          "now a dummy-fd, while original data-fd is saved as "
                          "fd %d",
                          function, s, original_s);
                  }
                  else
                     swarn("%s: dup2() failed on fd %d, fd %d while executing "
                           "dns-code",
                           function, tmp_s, s);
               }
            }
         }
#endif /* SOCKS_CLIENT */

         rc = negotiate_method(control, packet, route, emsg, emsglen);

#if SOCKS_CLIENT
         if (executingdnscode && original_s != -1) {
            const int errno_s = errno;

            slog(LOG_DEBUG, "%s: restoring data fd %d from saved fd %d (%s)",
                 function, s, original_s, socket2string(original_s, NULL, 0));

            if (dup2(original_s, s) != s)
               swarn("%s: failed to restore data fd %d from saved fd %d",
                     function, s, original_s);

            close(original_s);
            errno = errno_s;
         }
#endif /* SOCKS_CLIENT */

         if (rc != 0) {
            if (errno == 0) /* something wrong.  If nothing else ... */
               errno = ECONNREFUSED;

            failed = 1;
            break;
         }

         slog(LOG_DEBUG,
              "%s: method negotiation successful.  Server selected method "
              "%d (%s)",
              function,
              packet->req.auth->method,
              method2string(packet->req.auth->method));

         if (socks_sendrequest(control, &packet->req, emsg, emsglen) != 0) {
            failed = 1;
            break;
         }

         if (socks_recvresponse(control,
                                &packet->res,
                                packet->req.version,
                                emsg,
                                emsglen) != 0) {
            socks_blacklist(route, emsg);

            if (errno == 0) /* something wrong.  If nothing else ... */
               errno = ECONNREFUSED;

            failed = 1;
         }

         break;
      }

      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         if (httpproxy_negotiate(control, packet, emsg, emsglen) != 0) {
            if (errno == 0)
               errno = ECONNREFUSED; /* something wrong.  If nothing else ... */

            failed = 1;
         }

         break;

#if HAVE_LIBMINIUPNP
      case PROXY_UPNP:
         if (upnp_negotiate(s, packet, &route->gw, emsg, emsglen) != 0) {
            if (errno == 0)
               errno = ECONNREFUSED; /* something wrong.  If nothing else ... */

            failed = 1;
         }

         break;
#endif /* HAVE_LIBMINIUPNP */

      default:
         SERRX(packet->req.version);
   }

   if (!failed) {
      if (serverreplyisok(packet->res.version,
                          packet->req.command,
                          socks_get_responsevalue(&packet->res),
                          route,
                          emsg,
                          emsglen)) {
         if (errno != EINPROGRESS)
            errno = 0; /* all should be ok. */
      }
      else {
         SASSERTX(errno != 0);
         failed = 1;
      }
   }

   if (failed) {
#if HAVE_GSSAPI
      if (packet->req.auth->method == AUTHMETHOD_GSSAPI
      &&  packet->req.auth->mdata.gssapi.state.id != GSS_C_NO_CONTEXT) {
         OM_uint32 major_status, minor_status;
         char buf[512];

         if ((major_status
         = gss_delete_sec_context(&minor_status,
                                  &packet->req.auth->mdata.gssapi.state.id,
                                  GSS_C_NO_BUFFER)) != GSS_S_COMPLETE) {
            if (!gss_err_isset(major_status, minor_status, buf, sizeof(buf)))
               *buf = NUL;

            swarnx("%s: gss_delete_sec_context() failed%s%s",
                   function,
                   *buf == NUL ? "" : ": ",
                   *buf == NUL ? "" : buf);
         }
      }
#endif /* HAVE_GSSAPI */

      return -1;
   }

   return 0;

}

#if SOCKS_CLIENT
void
update_after_negotiate(packet, socksfd)
   const socks_t *packet;
   socksfd_t *socksfd;
{

   socksfd->state.auth    = *packet->req.auth;
   socksfd->state.command = packet->req.command;
   socksfd->state.version = packet->req.version;
}

#endif /* SOCKS_CLIENT */

static int
recv_sockshost(s, host, version, auth, emsg, emsglen)
   int s;
   sockshost_t *host;
   int version;
   authmethod_t *auth;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "recv_sockshost()";
   ssize_t rc;

   switch (version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V4REPLY_VERSION: {
         /*
          * DSTPORT  DSTIP
          *   2    +   4
          */
         char hostmem[ sizeof(host->port)
                     + sizeof(host->addr.ipv4)
                     ];
         char *p = hostmem;

         if ((rc = socks_recvfromn(s,
                                   hostmem,
                                   sizeof(hostmem),
                                   sizeof(hostmem),
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   auth)) != (ssize_t)sizeof(hostmem)) {
            fmtresponseerror(rc, sizeof(hostmem), emsg, emsglen);
            return -1;
         }

         host->atype = SOCKS_ADDR_IPV4;

         /* BND.PORT */
         memcpy(&host->port, p, sizeof(host->port));
         p += sizeof(host->port);

         /* BND.ADDR */
         memcpy(&host->addr.ipv4, p, sizeof(host->addr.ipv4));
         p += sizeof(host->addr.ipv4);

         break;
      }

      case PROXY_SOCKS_V5:
         /*
          * +------+----------+----------+
          * | ATYP | BND.ADDR | BND.PORT |
          * +------+----------+----------+
          * |  1   |  > 0     |    2     |
          * +------+----------+----------+
          */

         /* ATYP */
         if ((rc = socks_recvfromn(s,
                                   &host->atype, sizeof(host->atype),
                                   sizeof(host->atype),
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   auth)) != (ssize_t)sizeof(host->atype)) {
            fmtresponseerror(rc, sizeof(host->atype), emsg, emsglen);
            return -1;
         }

         switch(host->atype) {
            case SOCKS_ADDR_IPV4:
               if ((rc = socks_recvfromn(s,
                                         &host->addr.ipv4,
                                         sizeof(host->addr.ipv4),
                                         sizeof(host->addr.ipv4),
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         auth))
               != (ssize_t)sizeof(host->addr.ipv4)) {
                  fmtresponseerror(rc, sizeof(host->addr.ipv4), emsg, emsglen);
                  return -1;
               }
               break;

            case SOCKS_ADDR_IPV6:
               if ((rc = socks_recvfromn(s,
                                         &host->addr.ipv6.ip,
                                         sizeof(host->addr.ipv6.ip),
                                         sizeof(host->addr.ipv6.ip),
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         auth))
               != (ssize_t)sizeof(host->addr.ipv6.ip)) {
                  fmtresponseerror(rc,
                                   sizeof(host->addr.ipv6.ip),
                                   emsg,
                                   emsglen);

                  return -1;
               }
               break;

            case SOCKS_ADDR_DOMAIN: {
               unsigned char alen;

               /* read length of domain name. */
               if ((rc = socks_recvfromn(s,
                                         &alen,
                                         sizeof(alen),
                                         sizeof(alen),
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         auth)) != (ssize_t)sizeof(alen)) {
                  fmtresponseerror(rc, sizeof(alen), emsg, emsglen);
                  return -1;
               }

               OCTETIFY(alen);

#if MAXHOSTNAMELEN < 0xff
               SASSERTX(alen < sizeof(host->addr.domain));
#endif /* MAXHOSTNAMELEN < 0xff */

               /* BND.ADDR, alen octets */
               if ((rc = socks_recvfromn(s,
                                         host->addr.domain,
                                         (size_t)alen,
                                         (size_t)alen,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL,
                                         auth)) != (ssize_t)alen) {
                  fmtresponseerror(rc, alen, emsg, emsglen);
                  return -1;
               }
               host->addr.domain[alen] = NUL;

               break;
            }

            default:
               swarnx("%s: unsupported address format %d in reply",
                      function, host->atype);
               return -1;
         }

         /* BND.PORT */
         if ((rc = socks_recvfromn(s,
                                   &host->port,
                                   sizeof(host->port),
                                   sizeof(host->port),
                                   0,
                                   NULL,
                                   NULL,
                                   NULL,
                                   auth)) != (ssize_t)sizeof(host->port)) {
            fmtresponseerror(rc, sizeof(host->port), emsg, emsglen);
            return -1;
         }

         break;
   }

   return 0;
}

int
serverreplyisok(version, command, reply, route, emsg, emsglen)
   const unsigned int version;
   const unsigned int command;
   const unsigned int reply;
   route_t *route;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "serverreplyisok()";

   slog(LOG_NEGOTIATE, "%s: version %d, command %d, reply %d",
        function, version, command, reply);

   switch (version) {
      case PROXY_SOCKS_V4REPLY_VERSION:
         switch (reply) {
            case SOCKSV4_SUCCESS:
               socks_clearblacklist(route);
               return 1;

            case SOCKSV4_FAIL:
               snprintf(emsg, emsglen, "generic proxy server failure");

               socks_clearblacklist(route);
               errno = ECONNREFUSED;
               break;

            case SOCKSV4_NO_IDENTD:
               snprintf(emsg, emsglen,
                        "proxy server says it could not get a ident (rfc931) "
                        "response from host we are running on");

               /* will probably fail next time too, so blacklist it. */
               socks_blacklist(route, emsg);

               errno = ECONNREFUSED;
               break;

            case SOCKSV4_BAD_ID:
               snprintf(emsg, emsglen,
                        "proxy server claims username/ident mismatch from us");

               socks_blacklist(route, emsg);
               errno = ECONNREFUSED;
               break;

            default:
               snprintf(emsg, emsglen,
                        "unknown v%d reply from proxy server.  Replycode: %d",
                        version, reply);

               socks_blacklist(route, emsg);
               errno = ECONNREFUSED;
               break;
         }
         break;

      case PROXY_SOCKS_V5:
         switch (reply) {
            case SOCKS_SUCCESS:
               socks_clearblacklist(route);
               return 1;

            case SOCKS_FAILURE:
               snprintf(emsg, emsglen,
                        "generic failure at remote proxy server");

               if (command == SOCKS_BIND) {
                  errno = EADDRINUSE;
                  socks_clearblacklist(route);
               }
               else
                  socks_blacklist(route, emsg);

               errno = ECONNREFUSED;
               break;

            case SOCKS_NOTALLOWED:
               snprintf(emsg, emsglen, "connection denied by proxy server");

               socks_clearblacklist(route);
               errno = ECONNREFUSED;
               break;

            case SOCKS_NETUNREACH:
               snprintf(emsg, emsglen, "net unreachable by proxy server");

               socks_clearblacklist(route);
               errno = ENETUNREACH;
               break;

            case SOCKS_HOSTUNREACH:
               snprintf(emsg, emsglen, "target unreachable by proxy server");

               socks_clearblacklist(route);
               errno = EHOSTUNREACH;
               break;

            case SOCKS_CONNREFUSED:
               snprintf(emsg, emsglen,
                        "target refused connection by proxy server");

               socks_clearblacklist(route);
               errno = ECONNREFUSED;
               break;

            case SOCKS_TTLEXPIRED:
               snprintf(emsg, emsglen,
                        "connection to target from proxy server timed out");

               socks_clearblacklist(route);
               errno = ETIMEDOUT;
               break;

            case SOCKS_CMD_UNSUPP:
               snprintf(emsg, emsglen, "command not supported by proxy server");

               swarnx("%s: %s", function, emsg);

               socks_blacklist(route, emsg);
               errno = ECONNREFUSED;
               break;

            case SOCKS_ADDR_UNSUPP:
               snprintf(emsg, emsglen,
                        "address format in the request we sent is not "
                        "supported by the proxy server");

               socks_blacklist(route, emsg);
               errno = ECONNREFUSED;
               break;

            default:
               snprintf(emsg, emsglen,
                        "unknown v%d reply from proxy server: %d",
                        version, reply);

               socks_blacklist(route, emsg);
               errno = ECONNREFUSED;
               break;
         }
         break;

      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         switch (reply) {
            case HTTP_SUCCESS:
               socks_clearblacklist(route);
               return 1;

            default:
               snprintf(emsg, emsglen, "unknown proxy server failure");

               socks_blacklist(route, emsg);
               errno = ECONNREFUSED;
               break;
         }
         break;

      case PROXY_UPNP:
         switch (reply) {
            case UPNP_SUCCESS:
               socks_clearblacklist(route);
               return 1;

            default:
               socks_blacklist(route, "UPNP failure");
               errno = ECONNREFUSED;
               break;
         }
         break;


      default:
         snprintf(emsg, emsglen, "unknown proxy version %d", version);
         break;
   }

   SASSERTX(*emsg != NUL);

   slog(LOG_DEBUG, "%s", emsg);

   return 0;
}

/* ARGSUSED */
int
clientmethod_uname(s, host, version, name, password, emsg, emsglen)
   int s;
   const sockshost_t *host;
   int version;
   unsigned char *name;
   unsigned char *password;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "clientmethod_uname()";
   static authmethod_uname_t uname;   /* cached userinfo.              */
   static sockshost_t unamehost;      /* host cache was gotten for.    */
   static int usecachedinfo;          /* cached data is ok?            */
   ssize_t rc;
   size_t len;
   unsigned char *offset;
   unsigned char request[ 1               /* version.          */
                        + 1               /* username length.  */
                        + MAXNAMELEN      /* username.         */
                        + 1               /* password length.  */
                        + MAXPWLEN        /* password.         */
   ];
   unsigned char response[ 1 /* version.  */
                         + 1 /* status.   */
   ];

   switch (version) {
      case PROXY_SOCKS_V5:
         break;

      default:
         SERRX(version);
   }

   if (memcmp(&unamehost, host, sizeof(unamehost)) != 0)
      usecachedinfo = 0;   /* not same host as cache was gotten for. */

   /* fill in request. */

   offset  = request;
   *offset = (unsigned char)SOCKS_UNAMEVERSION;
   ++offset;

   if (!usecachedinfo) {
      if (name == NULL
      && (name = (unsigned char *)socks_getusername(host,
                                                    (char *)offset + 1,
                                                    MAXNAMELEN)) == NULL) {
         snprintf(emsg, emsglen, "could not determine username of client");
         return -1;
      }

      if (strlen((char *)name) > sizeof(uname.name) - 1) {
         char visbuf[MAXNAMELEN];

         swarnx("%s: username \"%s ...\" is too long.  Max length is %lu.  "
                "Trying to continue anyway.",
                function,
                str2vis((char *)name,
                        strlen((char *)name),
                        visbuf,
                        sizeof(visbuf)),
                (unsigned long)(sizeof(uname.name) - 1));

         /* perhaps it will be truncated at proxy too. */
         name[sizeof(uname.name) - 1] = NUL;
      }

      SASSERTX(strlen((char *)name) < sizeof(uname.name));
      strcpy((char *)uname.name, (char *)name);
   }

   slog(LOG_DEBUG, "%s: usecachedinfo %d, name \"%s\"",
        function, usecachedinfo, uname.name);

   /* first byte gives length. */
   *offset = (unsigned char)strlen((char *)uname.name);
   OCTETIFY(*offset);
   strcpy((char *)offset + 1, (char *)uname.name);
   offset += *offset + 1;

   if (!usecachedinfo) {
      if (password == NULL
      && (password = (unsigned char *)socks_getpassword(host,
                                                       (char *)name,
                                                       (char *)offset + 1,
                                                       MAXPWLEN)) == NULL) {
         slog(LOG_NEGOTIATE,
              "%s: could not determine password of client, using an empty one",
              function);

         password = (unsigned char *)"";
      }

      if (strlen((char *)password) > sizeof(uname.password) - 1) {
         swarnx("%s: password is too long.  Max length is %lu.  "
                "Trying to continue anyway.",
                function, (unsigned long)(sizeof(uname.password) - 1));

         /* perhaps it will be truncated at proxy too. */
         password[sizeof(uname.password) - 1] = NUL;
      }

      SASSERTX(strlen((char *)password) < sizeof(uname.password));
      strcpy((char *)uname.password, (char *)password);
   }

   /* first byte gives length. */
   *offset = (unsigned char)strlen((char *)uname.password);
   OCTETIFY(*offset);
   strcpy((char *)offset + 1, (char *)uname.password);
   offset += *offset + 1;

   slog(LOG_NEGOTIATE, "%s: offering username \"%s\", password %s to server",
        function, uname.name, (*uname.password == NUL) ? "\"\"" : "********");

   len = offset - request;
   if ((rc = socks_sendton(s,
                           request,
                           len,
                           len,
                           0,
                           NULL,
                           0,
                           NULL,
                           NULL)) != (ssize_t)len) {
      snprintf(emsg, emsglen,
               "send of username/password to proxy server failed, "
               "sent %ld/%lu: %s",
               (long)rc, (unsigned long)(offset - request), strerror(errno));
      return -1;
   }

   if ((rc = socks_recvfromn(s,
                             response,
                             sizeof(response),
                             sizeof(response),
                             0,
                             NULL,
                             NULL,
                             NULL,
                             NULL)) != sizeof(response)) {
      snprintfn(emsg, emsglen,
                "failed to receive proxy server response, received %ld/%lu: %s",
                (long)rc, (unsigned long)sizeof(response), strerror(errno));
      return -1;
   }

   slog(LOG_NEGOTIATE, "%s: received server response: 0x%x, 0x%x",
        function, response[0], response[1]);

   if (request[UNAME_VERSION] != response[UNAME_VERSION]) {
      snprintf(emsg, emsglen,
               "sent a v%d uname request to proxy server, "
               "but got back a v%d response",
               request[0], response[1]);
      return -1;
   }

   if (response[UNAME_STATUS] == UNAME_STATUS_ISOK) { /* server accepted. */
      unamehost     = *host;
      usecachedinfo = 1;

      return 0;
   }

   snprintf(emsg, emsglen, "proxy server rejected our username/password");

   return -1;
}

#if HAVE_GSSAPI
 /*
  * This code was contributed by
  * Markus Moeller (markus_moeller at compuserve.com).
  */

int
clientmethod_gssapi(s, protocol, gw, version, auth, emsg, emsglen)
   int s;
   int protocol;
   const gateway_t *gw;
   int version;
   authmethod_t *auth;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "clientmethod_gssapi()";
   OM_uint32 ret_flags, major_status, minor_status;
   gss_name_t            client_name       = GSS_C_NO_NAME,
                         server_name       = GSS_C_NO_NAME;
   gss_cred_id_t         server_creds      = GSS_C_NO_CREDENTIAL;
   gss_buffer_desc       service           = GSS_C_EMPTY_BUFFER,
                         input_token       = GSS_C_EMPTY_BUFFER,
                         output_token      = GSS_C_EMPTY_BUFFER,
                         *context_token    = GSS_C_NO_BUFFER;
#if SOCKS_CLIENT
   sigset_t oldset;
#endif /* SOCKS_CLIENT */
   ssize_t rc;
   size_t len;
   unsigned short token_length;
   unsigned char request[GSSAPI_HLEN + MAXGSSAPITOKENLEN],
                 response[GSSAPI_HLEN + MAXGSSAPITOKENLEN],
                 gss_server_enc, gss_enc;
   char nameinfo[MAXNAMELEN + MAXNAMELEN], buf[sizeof(nameinfo)], tmpbuf[512];
   int conf_state;
   int have_slash;

#if SOCKSLIBRARY_DYNAMIC && SOCKS_CLIENT
   /*
    * Make sure the gssapi functions use the native connect(2)/bind(2)
    * and sendto(2)/recvfrom(2) system calls, even if the user has not
    * created a direct route for the related addresses.
    *
    * During the process of establishing a socks gssapi session with
    * server X, we will need to contact the kdc for a ticket to
    * use with server X, but if our only route to the kdc is via
    * server X, that doesn't work of course.  The correct thing
    * is for the user to create a direct route to kdc, but helping
    * him this way will hopefully save a ton of grief.
    *
    * It will not work in the (hopefully very rare) case where the
    * user actually has set things up correctly, but the route to
    * the kdc is via another, non-gssapi, proxy though.
    *
    * Based on an idea by Markus Moeller for forcing connect(2) to
    * the Kerberos kdc to use the native connect(2), rather than most
    * likely create a routing loop when the user neglects to mark the
    * route to the kdc as "direct" in socks.conf.
    */
   socks_mark_io_as_native();
#endif /* SOCKSLIBRARY_DYNAMIC && SOCKS_CLIENT */


   /*
    * Get the hostname of the gateway so we can convert it to a gss-name
    * and contact the kdc to get a ticket for using the socks-service at
    * hostname.
    */

   SASSERTX(gw != NULL);
   switch (gw->addr.atype) {
      case SOCKS_ADDR_IPV4: {
         struct sockaddr_storage addr;

         sockshost2sockaddr(&gw->addr, &addr);

         SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);
         rc = getnameinfo(TOSA(&addr),
                          salen(addr.ss_family),
                          nameinfo,
                          sizeof(nameinfo),
                          NULL,
                          0,
                          NI_NAMEREQD);
         SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

         if (rc != 0) {
            char ntop[MAXSOCKADDRSTRING];

            if (inet_ntop(addr.ss_family,
                          GET_SOCKADDRADDR(&addr),
                          ntop,
                          sizeof(ntop)) == NULL) {
               snprintf(emsg, emsglen, "inet_ntop(3) failed on addr %s: %s",
                         sockaddr2string2(&addr, 0, NULL, 0), strerror(errno));
            }
            else
               snprintf(emsg, emsglen, "getnameinfo(%s) failed: error %ld",
                        ntop, (long)rc);

            swarnx("%s: %s", emsg, function); /* likely generic config error. */
            goto error;
         }
         break;
      }

      case SOCKS_ADDR_DOMAIN:
         STRCPY_ASSERTSIZE(nameinfo, gw->addr.addr.domain);
         break;

      default:
         SERRX(gw->addr.atype);
   }

   have_slash = (strchr(gw->state.gssapiservicename, '/') != NULL);
   if (have_slash)
      snprintf(buf, sizeof(buf), "%s", gw->state.gssapiservicename);
   else
      snprintf(buf, sizeof(buf), "%s@%s", gw->state.gssapiservicename, nameinfo);
   service.value  = buf;
   service.length = strlen((char *)service.value);

   SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);
   major_status = gss_import_name(&minor_status,
                                  &service,
                                  have_slash ? (gss_OID)GSS_C_NULL_OID
                                             : (gss_OID)gss_nt_service_name,
                                  &server_name);
   SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

   if (gss_err_isset(major_status, minor_status, tmpbuf, sizeof(tmpbuf))) {
      snprintf(emsg, emsglen, "gss_import_name() failed: %s", emsg);

      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);
      goto error;
   }

   request[GSSAPI_VERSION]     = SOCKS_GSSAPI_VERSION;
   auth->mdata.gssapi.state.id = GSS_C_NO_CONTEXT;

   while (1) {
      SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);
      major_status = gss_init_sec_context(&minor_status,
                                          GSS_C_NO_CREDENTIAL,
                                          &auth->mdata.gssapi.state.id,
                                          server_name,
                                          GSS_C_NULL_OID,
                                            GSS_C_MUTUAL_FLAG
                                          | GSS_C_REPLAY_FLAG
                                          | (protocol == SOCKS_TCP ?
                                                      GSS_C_SEQUENCE_FLAG : 0),
                                          /*
                                           * | GSS_C_DELEG_FLAG
                                           * RFC 1961 says GSS_C_DELEG_FLAG
                                           * should also be set, but I can't
                                           * see any reason why the client
                                           * should want to forward it's
                                           * tickets to a socks server ...
                                           *
                                           * Don't set unless until we find
                                           * a reason to do so.
                                           */
                                          0,
                                          GSS_C_NO_CHANNEL_BINDINGS,
                                          context_token,
                                          NULL,
                                          &output_token,
                                          &ret_flags,
                                          NULL);
      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

      slog(LOG_DEBUG, "%s: length of output_token is %lu",
           function, (unsigned long)output_token.length);

      switch (major_status) {
         case GSS_S_COMPLETE:
            slog(LOG_NEGOTIATE, "%s: gssapi negotiation completed", function);
            break;

         case GSS_S_CONTINUE_NEEDED:
            slog(LOG_DEBUG, "%s: gssapi negotiation to be continued", function);
            break;

         default:
            if (!gss_err_isset(major_status,
                               minor_status,
                               tmpbuf,
                               sizeof(tmpbuf)))
               snprintf(tmpbuf, sizeof(tmpbuf), "unknown gss major_status %d",
                        major_status);

            snprintf(emsg, emsglen,
                     "gss_init_sec_context() failed: %s", tmpbuf);
            goto error;
      }

      if(output_token.length != 0) {
         request[GSSAPI_STATUS] = SOCKS_GSSAPI_AUTHENTICATION;

         token_length = htons((unsigned short)output_token.length);
         memcpy(request + GSSAPI_TOKEN_LENGTH, &token_length, sizeof(short));

         SASSERTX(output_token.length <= sizeof(request) - GSSAPI_HLEN);
         memcpy(request + GSSAPI_HLEN, output_token.value, output_token.length);

         slog(LOG_DEBUG, "%s: sending token of length %lu to server",
              function, (unsigned long)output_token.length);

         len = (size_t)(GSSAPI_HLEN + output_token.length);
         if ((rc = socks_sendton(s,
                                 request,
                                 len,
                                 len,
                                 0,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL)) != (ssize_t)len) {
            snprintf(emsg, emsglen,
                     "send of request to proxy server failed, sent %ld/%ld: %s",
                     (long)rc,
                     (long)(GSSAPI_HLEN + output_token.length),
                     strerror(errno));
            goto error;
         }

         CLEAN_GSS_TOKEN(output_token);
      }

      if (major_status == GSS_S_COMPLETE)
         break;

      if ((rc = socks_recvfromn(s,
                                response,
                                GSSAPI_HLEN,
                                GSSAPI_HLEN,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL)) != GSSAPI_HLEN) {
         snprintf(emsg, emsglen,
                  "read of response from proxy server failed, read %ld/%ld: %s",
                  (long)rc, (long)GSSAPI_HLEN, strerror(errno));
         goto error;
      }

      slog(LOG_DEBUG, "%s: read %ld bytes of response data from server",
           function, (long)rc);

      if(response[GSSAPI_VERSION] != SOCKS_GSSAPI_VERSION) {
         snprintf(emsg, emsglen,
                  "invalid GSSAPI authentication response type (%d, %d) from "
                  "proxy server",
                  response[GSSAPI_VERSION], response[GSSAPI_STATUS]);
         goto error;
      }

      if (response[GSSAPI_STATUS] == 0xff) {
         snprintf(emsg, emsglen, "user was rejected by proxy server (%d, %d).",
                  response[GSSAPI_VERSION], response[GSSAPI_STATUS]);
         goto error;
      }

      if(response[GSSAPI_STATUS] != SOCKS_GSSAPI_AUTHENTICATION) {
         snprintf(emsg, emsglen,
                  "invalid GSSAPI authentication response type (%d, %d) from "
                  "proxy server",
                  response[GSSAPI_VERSION], response[GSSAPI_STATUS]);
         goto error;
      }

      memcpy(&token_length, &response[GSSAPI_TOKEN_LENGTH], sizeof(short));
      token_length = ntohs(token_length);

      input_token.value = response + GSSAPI_HLEN;
      if ((input_token.length = token_length) > sizeof(response) - GSSAPI_HLEN){
         snprintf(emsg, emsglen,
                  "proxy server sent illegal token length of %u, max is %lu",
                  token_length, (long unsigned)sizeof(response) - GSSAPI_HLEN);
         goto error;
      }

      if ((rc = socks_recvfromn(s,
                                input_token.value,
                                input_token.length,
                                input_token.length,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL)) != (ssize_t)input_token.length) {
         snprintf(emsg, emsglen,
                  "read of response from proxy server failed, read %ld/%ld: %s",
                  (long)rc, (long)input_token.length, strerror(errno));

         goto error;
      }

      slog(LOG_DEBUG, "%s: read %lu byte token from server",
           function, (long)input_token.length);

      context_token = &input_token;
   }

   CLEAN_GSS_TOKEN(output_token);
   CLEAN_GSS_AUTH(client_name, server_name, server_creds);

   request[GSSAPI_STATUS] = SOCKS_GSSAPI_ENCRYPTION;

   /*
    * offer the best we are configured to support.
    * Later when we get the reply from the server, check that it is, if
    * not the one we offered, at least one we are configured to support
    * for this rule.
    */
   if (gw->state.gssapiencryption.confidentiality)
      gss_enc = SOCKS_GSSAPI_CONFIDENTIALITY;
   else if (gw->state.gssapiencryption.integrity)
      gss_enc = SOCKS_GSSAPI_INTEGRITY;
   else if (gw->state.gssapiencryption.clear)
      gss_enc = SOCKS_GSSAPI_CLEAR;
   else {
      swarnx("%s: no gssapi type to offer set", function);
      SERRX(0);
   }

   slog(LOG_NEGOTIATE,
        "%s: running in %s %s GSSAPI mode.  Offering server protection "
        "\"%s\" (%d)",
        function,
        gw->state.gssapiencryption.nec ? "nec"          : "rfc1961",
        gw->state.gssapiencryption.nec ? "non-standard" : "standard",
        gssapiprotection2string(gss_enc), gss_enc);

   if (gw->state.gssapiencryption.nec) {
      const size_t tosend = GSSAPI_HLEN + 1, toread = tosend;

      token_length = htons(1);
      memcpy(&request[GSSAPI_TOKEN_LENGTH], &token_length, sizeof(short));
      memcpy(request + GSSAPI_HLEN, &gss_enc, 1);

      if ((rc = socks_sendton(s,
                              request,
                              tosend,
                              tosend,
                              0,
                              NULL,
                              0,
                              NULL,
                              NULL)) != (ssize_t)tosend) {
         snprintf(emsg, emsglen,
                  "send of request to proxy server failed, sent %ld/%ld: %s",
                  (long)rc, (long)tosend, strerror(errno));
         goto error;
      }

      if ((rc = socks_recvfromn(s,
                                response,
                                toread,
                                toread,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL)) != (ssize_t)toread) {
         snprintf(emsg, emsglen,
                  "read of response from proxy server failed, read %ld/%ld: %s",
                  (long)rc, (long)(GSSAPI_HLEN + 1), strerror(errno));
         goto error;
      }

      if (response[GSSAPI_STATUS] != SOCKS_GSSAPI_ENCRYPTION) {
         snprintf(emsg, emsglen,
                  "invalid GSSAPI encryption response type (%d, %d) "
                  "from proxy server",
                  response[GSSAPI_VERSION], response[GSSAPI_STATUS]);
         goto error;
      }

      memcpy(&token_length, &response[GSSAPI_TOKEN_LENGTH], sizeof(short));
      token_length = ntohs(token_length);

      if (token_length != 1) {
         snprintf(emsg, emsglen,
                  "received invalid encryption token length from proxy server "
                  "(%d, not 1) ",
                  token_length);
         goto error;
      }

      gss_server_enc = response[GSSAPI_TOKEN];
   }
   else {
      unsigned char p;

      input_token.value  = &p;
      input_token.length = 1;

      memcpy(input_token.value, &gss_enc, input_token.length);

      SOCKS_SIGBLOCK_IF_CLIENT(SIGIO, &oldset);
      major_status = gss_wrap(&minor_status,
                              auth->mdata.gssapi.state.id,
                              0,
                              GSS_C_QOP_DEFAULT,
                              &input_token,
                              &conf_state,
                              &output_token);
      SOCKS_SIGUNBLOCK_IF_CLIENT(&oldset);

      if (gss_err_isset(major_status, minor_status, tmpbuf, sizeof(tmpbuf))) {
         snprintf(emsg, emsglen, "gss_wrap() failed: %s", tmpbuf);
         goto error;
      }

      token_length = htons((short)output_token.length);
      memcpy(&request[GSSAPI_TOKEN_LENGTH], &token_length, sizeof(short));

      if ((rc = socks_sendton(s,
                              request,
                              GSSAPI_TOKEN,
                              GSSAPI_TOKEN,
                              0,
                              NULL,
                              0,
                              NULL,
                              NULL)) != GSSAPI_TOKEN)  {
         snprintf(emsg, emsglen,
                  "send of request to proxy server failed, sent %ld/%ld: %s",
                  (long)rc, (long)GSSAPI_TOKEN, strerror(errno));
         goto error;
      }

      if ((rc = socks_sendton(s,
                              output_token.value,
                              output_token.length,
                              output_token.length,
                              0,
                              NULL,
                              0,
                              NULL,
                              NULL)) != (ssize_t)output_token.length) {
         snprintf(emsg, emsglen,
                  "send of request to proxy server failed, sent %ld/%ld: %s",
                  (long)rc, (long)output_token.length, strerror(errno));
         goto error;
      }

      CLEAN_GSS_TOKEN(output_token);

      if ((rc = socks_recvfromn(s,
                                response,
                                GSSAPI_HLEN,
                                GSSAPI_HLEN,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL)) != GSSAPI_HLEN) {
         snprintf(emsg, emsglen,
                  "read of response from proxy server failed, read %ld/%d: %s",
                  (long)rc, GSSAPI_HLEN, strerror(errno));
         goto error;
      }

      if (response[GSSAPI_STATUS] != SOCKS_GSSAPI_ENCRYPTION) {
         snprintf(emsg, emsglen,
                  "received invalid GSSAPI encryption response type from "
                  "proxy server (version %d, status %d)",
                  response[GSSAPI_VERSION], response[GSSAPI_STATUS]);
         goto error;
      }

      memcpy(&token_length, response + GSSAPI_TOKEN_LENGTH, sizeof(short));
      input_token.length = ntohs(token_length);

      if (input_token.length > sizeof(response) - GSSAPI_HLEN) {
         snprintf(emsg, emsglen,
                  "proxy server replied with too big a token of length %u, "
                  "but the max length is %lu",
                  (unsigned)token_length,
                  (unsigned long)sizeof(response) - GSSAPI_HLEN);
         goto error;
      }

      input_token.value = response + GSSAPI_HLEN;

      if ((rc = socks_recvfromn(s,
                                input_token.value,
                                input_token.length,
                                input_token.length,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL)) != (ssize_t)input_token.length) {
         snprintf(emsg, emsglen,
                  "read of response from proxy server failed, read %ld/%ld: %s",
                  (long)rc, (long)input_token.length, strerror(errno));
         goto error;
      }

      major_status = gss_unwrap(&minor_status,
                                auth->mdata.gssapi.state.id,
                                &input_token,
                                &output_token,
                                0,
                                GSS_C_QOP_DEFAULT);

      if (gss_err_isset(major_status, minor_status, tmpbuf, sizeof(tmpbuf))) {
         snprintf(emsg, emsglen,
                  "gss_unwrap() of token received from proxy server failed: %s",
                  tmpbuf);
         goto error;
      }

      if (output_token.length != 1)  {
         snprintf(emsg, emsglen,
                  "gssapi encryption output_token.length is not 1 as expected, "
                  "but instead %lu",
                  (unsigned long)output_token.length);
         goto error;
      }

      gss_server_enc = *(unsigned char *)output_token.value;

      CLEAN_GSS_TOKEN(output_token);
   }

   switch (gss_server_enc) {
      case SOCKS_GSSAPI_CLEAR:
         if (gw->state.gssapiencryption.clear)
            gss_enc = SOCKS_GSSAPI_CLEAR;
         break;

      case SOCKS_GSSAPI_INTEGRITY:
         if (gw->state.gssapiencryption.integrity)
            gss_enc = SOCKS_GSSAPI_INTEGRITY;
         break;

      case SOCKS_GSSAPI_CONFIDENTIALITY:
         if (gw->state.gssapiencryption.confidentiality)
            gss_enc = SOCKS_GSSAPI_CONFIDENTIALITY;
         break;

      default:
         snprintf(emsg, emsglen,
                  "proxy server responded with different encryption than we "
                  "are accepting for this proxy server.  "
                  "Our settings for this server are: "
                  "clear/%s, integrity/%s, confidentiality/%s, per message/%s, "
                  "but server instead offered us %s (%d)",
                  gw->state.gssapiencryption.clear           ? "yes" : "no",
                  gw->state.gssapiencryption.integrity       ? "yes" : "no",
                  gw->state.gssapiencryption.confidentiality ? "yes" : "no",
                  gw->state.gssapiencryption.permessage      ? "yes" : "no",
                  gssapiprotection2string(gss_server_enc), gss_server_enc);
         goto error;
   }

   slog(LOG_NEGOTIATE, "%s: using gssapi %s protection (%d)",
        function, gssapiprotection2string(gss_enc), gss_enc);

   auth->mdata.gssapi.state.protection = gss_enc;
   if (auth->mdata.gssapi.state.protection != SOCKS_GSSAPI_CLEAR)
       auth->mdata.gssapi.state.wrap = 1;

   major_status
   = gss_wrap_size_limit(&minor_status,
                         auth->mdata.gssapi.state.id,
                         auth->mdata.gssapi.state.protection
                         == GSSAPI_CONFIDENTIALITY ?
                         GSS_REQ_CONF : GSS_REQ_INT,
                         GSS_C_QOP_DEFAULT,
                         MAXGSSAPITOKENLEN - GSSAPI_HLEN,
                         &auth->mdata.gssapi.state.maxgssdata);

   if (gss_err_isset(major_status, minor_status, tmpbuf, sizeof(tmpbuf))) {
      snprintf(emsg, emsglen, "gss_wrap_size_limit() failed: %s", tmpbuf);
      goto error;
   }
   else {
      slog(LOG_DEBUG, "%s: max length of gssdata before encoding: %lu",
           function, (unsigned long)auth->mdata.gssapi.state.maxgssdata);

      if ((unsigned long)auth->mdata.gssapi.state.maxgssdata == 0) {
         snprintf(emsg, emsglen,
                  "for a token of length %lu gss_wrap_size_limit() returned "
                  "%lu, which does not make sense.  "
                  "Possibly the kerberos library in use does not fully support "
                  "the configured gssapi encoding type?",
                   (unsigned long)auth->mdata.gssapi.state.maxgssdata,
                   (unsigned long)auth->mdata.gssapi.state.maxgssdata);
         goto error;
      }
   }

#if SOCKSLIBRARY_DYNAMIC && SOCKS_CLIENT
   socks_mark_io_as_normal();
#endif /* SOCKSLIBRARY_DYNAMIC && SOCKS_CLIENT */

   return 0;

error:
   if (auth->mdata.gssapi.state.id != GSS_C_NO_CONTEXT) {
      if ((major_status
      = gss_delete_sec_context(&minor_status,
                               &auth->mdata.gssapi.state.id,
                               GSS_C_NO_BUFFER)) != GSS_S_COMPLETE) {
         if (!gss_err_isset(major_status, minor_status, tmpbuf, sizeof(tmpbuf)))
            *tmpbuf = NUL;

         swarnx("%s: gss_delete_sec_context() failed%s%s",
                function,
                *tmpbuf == NUL ? "" : ": ",
                *tmpbuf == NUL ? "" : tmpbuf);
      }
   }

   CLEAN_GSS_TOKEN(output_token);
   CLEAN_GSS_AUTH(client_name, server_name, server_creds);

#if SOCKSLIBRARY_DYNAMIC && SOCKS_CLIENT
   socks_mark_io_as_normal();
#endif /* SOCKSLIBRARY_DYNAMIC && SOCKS_CLIENT */

   slog(LOG_NEGOTIATE, "%s: failed, error is: %s", function, emsg);
   return -1;
}
#endif /* HAVE_GSSAPI and Markus' contributed code. */
