/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2005, 2008, 2009, 2010, 2011,
 *               2012, 2013, 2014, 2021, 2024
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
"$Id: httpproxy.c,v 1.73.4.2.6.2.4.2 2024/11/20 22:03:27 karls Exp $";

int
httpproxy_negotiate(s, packet, emsg, emsglen)
   int s;
   socks_t *packet;
   char *emsg;
   size_t emsglen;
{
   const char *function = "httpproxy_negotiate()";
   char buf[MAXHOSTNAMELEN + 512] /* The + 512 is for http babble. */,
        visbuf[sizeof(buf) * 4 + 1], *p;
   char host[MAXSOCKSHOSTSTRING];
   int checked, eof;
   ssize_t rc;
   size_t len, readsofar;
   struct sockaddr_storage addr;
   socklen_t addrlen;

   slog(LOG_DEBUG, "%s", function);

   sockshost2string(&packet->req.host, host, sizeof(host));

   /*
    * replace the dot that sockshost2string uses to separate port from host
    * with http's ':'.
    */
   if ((p = strrchr(host, '.')) == NULL) {
      snprintf(emsg, emsglen,
               "did not find portnumber separator ('.') in string \"%s\"",
               host);

      swarnx("%s: %s", function, emsg);
      return -1;
   }

   *p = ':';

   len = snprintf(buf, sizeof(buf),
                  "CONNECT %s %s\r\n"
                  "User-agent: %s/client v%s\r\n"
                  "\r\n",
                  host,
                  proxyprotocol2string(packet->req.version),
                  PRODUCT,
                  VERSION);

   slog(LOG_NEGOTIATE, "%s: sending to server: %s",
        function, str2vis(buf, len, visbuf, sizeof(visbuf)));

   if ((rc = socks_sendton(s, buf, len, len, 0, NULL, 0, NULL, NULL))
   != (ssize_t)len) {
      snprintf(emsg, emsglen,
               "could not send request to proxy server.  Sent %ld/%lu: %s",
               (long)rc, (unsigned long)len, strerror(errno));

      return -1;
   }

   /*
    * read til we get the eof response so there's no junk left in buffer
    * for client, then return the response code.
    */
   eof = checked = readsofar = 0;
   do {
      const char *eofresponse_str = "\r\n\r\n"; /*
                                                 * the CRLF terminating the
                                                 * line, and the CRLF
                                                 * terminating the entity body.
                                                 */
      const char *eol_str = "\r\n";
      char *eol, *bufp;
      size_t linelen;

      if ((rc = read(s, &buf[readsofar], sizeof(buf) - readsofar - 1)) <= 0) {
         snprintf(emsg, emsglen,
                  "could not read response from proxy server.  "
                  "read(2) returned %ld after having read %lu bytes",
                  (long)rc, (unsigned long)readsofar);
         return -1;
      }

      len = (size_t)rc;

      buf[readsofar + len] = NUL;

      slog(LOG_NEGOTIATE, "%s: read from server: %s",
           function, str2vis(&buf[readsofar], len, visbuf, sizeof(visbuf)));

      readsofar += len;

      if ((strstr(buf, eofresponse_str)) == NULL)
         continue; /* don't bother to start parsing til we've got it all. */
      else
         eof = 1;

      bufp = buf;
      while ((eol = strstr(bufp, eol_str)) != NULL) {
         /* check each line for the response we are looking for. */
         *eol   = NUL;
         linelen = eol - bufp;

         slog(LOG_DEBUG, "%s: checking line \"%s\"",
              function, str2vis(bufp, linelen, visbuf, sizeof(visbuf)));

         if (!checked) {
            int error = 0;

            switch (packet->req.version) {
               case PROXY_HTTP_10:
               case PROXY_HTTP_11: {
                  const char *ver_str
                  = proxyprotocol2string(packet->req.version);
                  size_t offset = strlen(ver_str);

                  if (linelen < offset + strlen(" 200")) {
                     snprintf(emsg, emsglen,
                              "response from proxy server is too short to"
                              "indicate success: \"%s\"",
                              visbuf);

                     error = 1;
                     break;
                  }

                  if (strncmp(bufp, ver_str, offset) != 0) {
                     snprintf(emsg, emsglen,
                              "HTTP version (\"%s\") in response from proxy "
                              "server does not match expected (\"%s\").  "
                              "Continuing anyway and hoping for the best ...",
                              visbuf, ver_str);
                  }

                  while (isspace((unsigned char)bufp[offset]))
                        ++offset;

                  if (!isdigit((unsigned char)bufp[offset])) {
                     char tmp[sizeof(visbuf)];

                     snprintf(emsg, emsglen,
                              "response from proxy server does not match.  "
                              "Expected a number at offset %lu, but got \"%s\"",
                              (unsigned long)offset,
                              str2vis(&bufp[offset],
                                      linelen - offset,
                                      tmp,
                                      sizeof(tmp)));
                     error = 1;
                     break;
                  }

                  packet->res.version = packet->req.version;

                  /* not really a portnumber, but close enough. */
                  if ((rc = string2portnumber(&bufp[offset], emsg, emsglen))
                  == -1) {
                     swarn("%s: could not find response code in http "
                           "response (\"%s\"): %s",
                           function, visbuf, emsg);

                     rc = HTTP_UNSUPPORTEDVERSION;
                  }
                  else {
                     snprintf(emsg, emsglen,
                              "response code %ld from http server indicates "
                              "%s: \"%s\"",
                              (long)rc,
                              rc == HTTP_SUCCESS ? "success" : "failure",
                              visbuf);

                     slog(LOG_DEBUG, "%s: %s", function, emsg);
                  }

                  socks_set_responsevalue(&packet->res, (unsigned int)rc);

                  /*
                   * we have no idea what address the server will use on
                   * our behalf, so set it to what we use.  Better than
                   * nothing, perhaps. :-/
                   */
                  addrlen = sizeof(addr);
                  if (getsockname(s, TOSA(&addr), &addrlen) != 0)
                     SWARN(s);

                  sockaddr2sockshost(&addr, &packet->res.host);

                  checked = 1;
                  break;
               }

               default:
                  SERRX(packet->req.version);
            }

            if (error) {
               snprintf(emsg, emsglen,
                        "unknown response from proxy server: \"%s\"",
                        str2vis(bufp, linelen, visbuf, sizeof(visbuf)));
               return -1;
            }
         }

         /* shift out the line we just parsed, nothing of interest there. */
         bufp += linelen;
      }
   } while (!eof);

   if (checked)
      return socks_get_responsevalue(&packet->res) == HTTP_SUCCESS ? 0 : -1;

   slog(LOG_INFO, "%s: didn't get status code from proxy", function);
   return -1;
}
