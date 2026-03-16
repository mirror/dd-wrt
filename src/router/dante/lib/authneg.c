/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2005, 2008, 2009, 2011, 2012,
 *               2013, 2019
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
"$Id: authneg.c,v 1.128.10.2 2020/11/11 16:11:52 karls Exp $";

int
negotiate_method(s, packet, route, emsg, emsglen)
   int s;
   socks_t *packet;
   route_t *route;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "negotiate_method()";
   ssize_t rc;
   size_t i, requestlen;
   unsigned char *name = NULL, *password = NULL;
   unsigned char request[ 1                  /* version                       */
                        + 1                  /* number of methods to offer.   */
                        + METHODS_KNOWN      /* methods we offer server.      */
                        ];
   unsigned char response[ 1   /* version.                     */
                         + 1   /* method selected by server.   */
                         ];
   char buf[256], lemsg[512] /* local emesg. */;
   int intmethodv[METHODS_KNOWN];

   if (sockscf.option.debug)
      slog(LOG_DEBUG, "%s: fd %d, %s", function, s, socket2string(s, NULL, 0));

#if !SOCKS_CLIENT && HAVE_GSSAPI

   switch (packet->req.auth->method) {
      case AUTHMETHOD_GSSAPI:
         /*
          * Nothing from gssapistate with client we are currently
          * offering upstream proxyserver, so reset authmethod to none
          * so as to not confuse things and make any part of the code
          * try to think this gssapi state relates to the upstream
          * proxy.
          */
         bzero(&packet->req.auth->mdata.gssapi,
               sizeof(packet->req.auth->mdata.gssapi));

         packet->req.auth->method = AUTHMETHOD_NOTSET;
         break;
   }
#endif /* !SOCKS_CLIENT && HAVE_GSSAPI */


   if (packet->req.version == PROXY_SOCKS_V4) {
      slog(LOG_DEBUG,
           "%s: no method negotiate in %s.  Setting authmethod to %s",
           function,
           proxyprotocol2string(packet->req.version),
           method2string(AUTHMETHOD_NONE));


      packet->req.auth->method = AUTHMETHOD_NONE;
      packet->res.auth->method = AUTHMETHOD_NONE;

      return 0;
   }

   SASSERTX(packet->gw.state.smethodc > 0);
   SASSERTX(packet->gw.state.smethodc <= METHODS_KNOWN);

   /*
    * create request packet.
    * version numberOfmethods methods ...
    */

   requestlen            = 0;
   request[requestlen++] = packet->req.version;

   SASSERTX(requestlen == AUTH_NMETHODS);
   request[requestlen++] = (unsigned char)0;
   SASSERTX(request[AUTH_NMETHODS] == 0);

   /*
    * Count and add the methods we support and are configured to offer
    * this server.
    */
   for (i = 0; i < packet->gw.state.smethodc; ++i) {
      if (packet->req.auth->method != AUTHMETHOD_NOTSET) {
         /*
          * Must be doing serverchaining.  Not all methods we are
          * configured to support for this route may be supported
          * for this particular client.  E.g., if the client has
          * not provided us with a username/password, we can not
          * provide the upstream proxy with it either, so don't
          * offer it.
          */

         SASSERTX(!SOCKS_CLIENT);

         switch (packet->gw.state.smethodv[i]) {
            case AUTHMETHOD_NONE:
               break; /* ok. */

            case AUTHMETHOD_UNAME:
               if (packet->req.auth->method != AUTHMETHOD_UNAME)
                  continue; /* don't offer this method. */

               break;

            case AUTHMETHOD_GSSAPI:
               break; /*
                       * ok?  Can't forward gssapi/kerberos credentials,
                       * but operator should be able to set up a
                       * things so we can initiate our own gssapi
                       * session to the upsteam proxy.
                       */

            default:
               SERRX(packet->gw.state.smethodv[i]);
         }
      }

      request[requestlen++] = (unsigned char)packet->gw.state.smethodv[i];
      ++request[AUTH_NMETHODS];
   }

   SASSERTX(request[AUTH_NMETHODS] > 0);
   SASSERTX(request[AUTH_NMETHODS] <= METHODS_KNOWN);
   SASSERTX(request[AUTH_NMETHODS] <= ELEMENTS(intmethodv));

   charmethod2intmethod((ssize_t)request[AUTH_NMETHODS],
                        &request[AUTH_FIRSTMETHOD],
                        intmethodv);

   slog(LOG_NEGOTIATE, "%s: offering proxy server #%d method%s: %s",
        function,
        request[AUTH_NMETHODS],
        request[AUTH_NMETHODS] == 1 ? "" : "s",
        methods2string(request[AUTH_NMETHODS], intmethodv, buf, sizeof(buf)));

   if (socks_sendton(s,
                     request,
                     requestlen,
                     requestlen,
                     0,
                     NULL,
                     0,
                     NULL,
                     NULL) != (ssize_t)requestlen) {
      snprintf(emsg, emsglen,
               "could not offer list of auth methods to proxy server: "
               "send failed: %s",
               strerror(errno));

      socks_blacklist(route, emsg);
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
      snprintf(emsg, emsglen,
               "could not read proxy server's response concerning method to "
               "use, read %ld/%lu: %s",
               (long)rc,
               (unsigned long)sizeof(response),
               rc == 0 ? "server closed connection" : strerror(errno));

      socks_blacklist(route, emsg);
      return -1;
   }

   /*
    * sanity check server's reply.
    */

   SASSERTX(AUTH_VERSION <= rc);
   if (request[AUTH_VERSION] != response[AUTH_VERSION]) {
      snprintf(emsg, emsglen,
               "got reply version %d from proxy server, but expected version "
               "%d.  Remote proxy server problem?",
               response[AUTH_VERSION], request[AUTH_VERSION]);

      socks_blacklist(route, emsg);
      return -1;
   }
   packet->version = request[AUTH_VERSION];

   SASSERTX(AUTH_SELECTEDMETHOD <= rc);
   if (!methodisset(response[AUTH_SELECTEDMETHOD],
                    intmethodv,
                    request[AUTH_NMETHODS])) {
      if (response[AUTH_SELECTEDMETHOD] == AUTHMETHOD_NOACCEPT)
         snprintf(emsg, emsglen,
                  "proxy server said we did not offer any acceptable "
                  "authentication methods");
      else {
         snprintf(emsg, emsglen,
                  "proxy server selected method 0x%x (%s), but that is not "
                  "among the methods we offered it",
                  response[AUTH_SELECTEDMETHOD],
                  method2string(response[AUTH_SELECTEDMETHOD]));

         swarnx("%s: %s", function, emsg);
      }

      socks_blacklist(route, emsg);
      return -1;
   }

   slog(LOG_NEGOTIATE, "%s: proxy server selected method %s",
        function, method2string(response[AUTH_SELECTEDMETHOD]));

   switch (response[AUTH_SELECTEDMETHOD]) {
      case AUTHMETHOD_NONE:
         rc = 0;
         break;

#if HAVE_GSSAPI
      case AUTHMETHOD_GSSAPI:
         if (clientmethod_gssapi(s,
                                 packet->req.protocol,
                                 &packet->gw,
                                 packet->req.version,
                                 packet->req.auth,
                                 lemsg,
                                 sizeof(lemsg)) == 0)
            rc = 0;
         else
            rc = -1;
         break;
#endif /* HAVE_GSSAPI */

      case AUTHMETHOD_UNAME:
         if (clientmethod_uname(s,
                                &packet->gw.addr,
                                packet->req.version,
                                name,
                                password,
                                lemsg,
                                sizeof(lemsg)) == 0)
            rc = 0;
         else
            rc = -1;
         break;

      case AUTHMETHOD_NOACCEPT:
         snprintf(lemsg, sizeof(lemsg),
                  "proxy server did not accept any of the authentication "
                  "methods we offered it");

         socks_blacklist(route, emsg);
         rc = -1;
         break;

      default:
         SERRX(packet->req.auth->method);
   }

   packet->req.auth->method = response[AUTH_SELECTEDMETHOD];

   if (rc == 0) {
      slog(LOG_NEGOTIATE, "%s: established v%d connection using method %d",
           function, packet->version, packet->req.auth->method);

      errno = 0; /* all is ok. */
   }
   else {
      snprintf(emsg, emsglen,
               "failed to establish v%d connection using method %d: %s",
               packet->version, packet->req.auth->method, lemsg);

      slog(LOG_DEBUG, "%s: %s", function, emsg);
   }

   return (int)rc;
}
