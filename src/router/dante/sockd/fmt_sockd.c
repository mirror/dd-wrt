/*
 * Copyright (c) 2012, 2013, 2014
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
 */


#include "common.h"

static const char rcsid[] =
"$Id: fmt_sockd.c,v 1.26.4.3 2014/08/15 18:16:42 karls Exp $";

void
sockd_readmotherscontrolsocket(prefix, s)
   const char *prefix;
   const int s;
{
   ssize_t rc;
   char buf[256];

   switch (rc = read(s, buf, sizeof(buf))) {
      case -1:
         slog(LOG_ALERT,
              "%s: unexpected error %d on IPC control channel "
              "to mother: %s%s%s",
              prefix,
              errno,
              strerror(errno),
              sockd_motherexists() ? "" : ": ",
              sockd_motherexists() ? "" : "mother unexpectedly exited");
         break;

      case 0:
         slog(LOG_ALERT,
              "%s: mother unexpectedly closed the IPC control channel%s%s",
              prefix,
              sockd_motherexists() ? "" : ": ",
              sockd_motherexists() ? "" : "mother unexpectedly exited");
         break;

      default:
         if (rc == 1 && *buf == SOCKD_EXITNORMALLY)
            slog(LOG_DEBUG, "%s: mother telling us to exit normally when done",
                 prefix);
         else
            swarn("%s: unexpectedly read %ld byte%s over IPC control channel "
                  "from mother",
                  prefix, (long)rc, rc == 1 ? "" : "s");
   }
}

void
log_clientsend(client, child, isresend)
   const struct sockaddr_storage *client;
   const sockd_child_t *child;
   const int isresend;
{

   slog(LOG_DEBUG, "%s client %s to %s (pid %ld with %lu slots free)",
        isresend ? "trying again to send" : "sending",
        sockaddr2string(client, NULL, 0),
        childtype2string(child->type),
        (long)child->pid,
        (unsigned long)child->freec);
}

void
log_probablytimedout(client, child)
   const struct sockaddr_storage *client;
   const sockd_child_t *child;
{

   swarn("client %s probably timed out waiting for us to send it to a %s",
         sockaddr2string(client, NULL, 0), childtype2string(child->type));
}

void
log_sendfailed(client, s, child, isfirsttime)
   const struct sockaddr_storage *client;
   const int s;
   const sockd_child_t *child;
   const int isfirsttime;
{
   const int errno_s = errno;

   if (ERRNOISTMP(errno))
      slog(isfirsttime? LOG_DEBUG : LOG_NOTICE,
           "sending client %s to %s %ld with %lu free slots failed%s: %s",
           sockaddr2string(client, NULL, 0),
           childtype2string(child->type),
           (long)child->pid,
           (unsigned long)child->freec,
           isfirsttime ? "" : " again",
           strerror(errno));
   else {
      /*
       * permanent error.
       */
      if (socketisconnected(s, NULL, 0) == NULL)
         log_probablytimedout(client, child);
      else
         swarn("sending client %s to %s %ld with %lu free slots failed",
               sockaddr2string(client, NULL, 0),
               childtype2string(child->type),
               (long)child->pid,
               (unsigned long)child->freec);
   }

   errno = errno_s;
}

void
log_noclientrecv(child)
   const sockd_child_t *child;
{

   slog(LOG_DEBUG,
        "already have a previously received, but not sent, %s-object, "
        "so not trying to receive a new client object from %s %ld now",
        childtype2string(child->type),
        childtype2string(child->type),
        (long)child->pid);
}

void
log_truncatedudp(function, from, len)
   const char *function;
   const struct sockaddr_storage *from;
   const ssize_t len;
{

   swarn("%s: UDP packet from %s was truncated (received bytes: %ld).  "
         "This indicates our UDP socket receive buffer is too small to "
         "handle all packets from this client.  You can increase the buffer "
         "size in %s if you expect to receive unusually large packets",
         function,
         sockaddr2string(from, NULL, 0),
         (long)len,
         sockscf.option.configfile);
}

void
log_ruleinfo_shmid(rule, function, context)
   const rule_t *rule;
   const char *function;
   const char *context;
{

   slog(LOG_DEBUG,
        "%s: %s%sshmids in %s #%lu: "
        "bw_shmid %lu (%p), mstats_shmid %lu (%p), ss_shmid %lu (%p)",
        function,
        context == NULL ? "" : context,
        context == NULL ? "" : ": ",
        objecttype2string(rule->type),
        (unsigned long)rule->number,
        (unsigned long)rule->bw_shmid,
        rule->bw,
        (unsigned long)rule->mstats_shmid,
        rule->mstats,
        (unsigned long)rule->ss_shmid,
        rule->ss);
}

void
log_boundexternaladdress(function, addr)
   const char *function;
   const struct sockaddr_storage *addr;
{

   slog(LOG_DEBUG, "%s: bound address on external side is %s",
        function, sockaddr2string(addr, NULL, 0));
}

void
log_unexpected_udprecv_error(function, fd, error, side)
   const char *function;
   const int fd;
   const int error;
   const interfaceside_t side;
{

   swarnx("%s: unknown error %d on %s-side when reading UDP from fd %d.  "
          "Assuming fatal: %s",
          function,
          error,
          side == INTERNALIF ? "client" : "target",
          fd,
          strerror(error));
}

void
log_bind_failed(function, protocol, address)
   const char *function;
   const int protocol;
   const struct sockaddr_storage *address;
{
   const int nofreeports
   = (ntohs(GET_SOCKADDRPORT(address)) == (in_port_t)0 && errno == EADDRINUSE);

   slog(nofreeports ? LOG_WARNING : LOG_DEBUG,
        "%s: could not bind %s port %u of %s: %s",
        function,
        protocol2string(protocol),
        ntohs(GET_SOCKADDRPORT(address)),
        sockaddr2string2(address, ADDRINFO_ATYPE, NULL, 0),
        nofreeports ?
            "this probably means we have run out of free ports in the "
            "ephemeral port range configured on this system (see sysctl(8))"
        :   strerror(errno));

}

void
log_interfaceprotocol_set_too_late(side)
   const interfaceside_t side;
{

   yyerrorx("protocols and address-families on the %s-side interface must be "
            "specified before addresses for the same interface are specified, "
            "so that we can know which addresses to ignore",
            interfaceside2string(side));
}

void
log_getsockopt_failed(function, option, fd, interfaceside)
   const char *function;
   const char *option;
   const int fd;
   const interfaceside_t interfaceside;
{

   swarn("%s: could not get the current %s value for fd %d on the %s side",
         function,
         option,
         fd,
         interfaceside2string(interfaceside));
}

void
log_setsockopt_failed(function, option, value, fd, interfaceside)
   const char *function;
   const char *option;
   const int value;
   const int fd;
   const interfaceside_t interfaceside;
{

   swarn("%s: could not set the %s value for fd %d on the %s side to %d",
         function,
         option,
         fd,
         interfaceside2string(interfaceside),
         value);
}
