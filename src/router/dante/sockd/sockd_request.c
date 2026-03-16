/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2019,
 *               2020, 2024
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
"$Id: sockd_request.c,v 1.849.4.15.2.4.4.6.4.3 2024/11/20 22:05:41 karls Exp $";

/*
 * XXX Should fix things so this process too can support multiple clients.
 * Will also fix the terrible fact that we just sit around and wait if the
 * command is bind, wasting the whole process on practically nothing.
 */


/* start of siginfo struct for requestchild.  Just the basics for now. */
static struct req {
   unsigned char             allocated;

   rule_t                    crule;           /* crule matched for client.    */
#if HAVE_SOCKS_HOSTID
   rule_t                    hrule;           /* hrule matched for client.    */
   unsigned char             hrule_isset;
#endif /* HAVE_SOCKS_HOSTID */

   struct sockaddr_storage   client;          /* client's address.            */
   request_t                 request;         /* request to perform.          */
   unsigned char             request_isvalid; /*
                                               * request has been checked and
                                               * is if nothing else, valid.
                                               */
   int                       s;               /* control-connection to client */
   time_t                    starttime;       /* time we started handling req */
} reqv[1];
static const size_t reqc = ELEMENTS(reqv);

static void siginfo(int sig, siginfo_t *si, void *sc);

static int bindexternaladdr(struct sockd_io_t *io, const request_t *req,
                            char *emsg, const size_t emsglen);
/*
 * Binds an external address of the appropriate type for the request "req".
 * Updates io.dst.laddr with the bound external address.
 *
 * Returns 0 on success.
 * Returns -1 on failure, and fills in emsg with the reason.
 */

static void
auth2standard(const authmethod_t *auth, authmethod_t *stdauth);
/*
 * Converts auth "auth", possibly using a non-standard method (e.g.,
 * AUTHMETHOD_PAM_USERNAME) to a standard socks authmethod, stored in
 * "stdauth".
 */

static void init_req(struct req *req, const sockd_request_t *request);
static void delete_req(struct req *req);
/*
 * Inits and de-inits "req".  For siginfo printing.
 */

static void
initlogaddrs(const sockd_io_t *io, iologaddr_t *src, iologaddr_t *dst,
             iologaddr_t *proxy);
/*
 * inits "src", "dst", and "proxy", based on values in "io".
 * If any of src, dst, or proxy, is NULL, that is not inited.
 */

static void
convertresponse(const response_t *oldres, response_t *newres,
                const int newversion);
/*
 * Converts a response on form "oldres", using oldres->version,
 * to a new response on form "newversion".
 */

static iostatus_t
dorequest(const sockd_mother_t *mother, sockd_request_t *request,
          struct sockaddr_storage *clientudpaddr, int *weclosedfirst,
          char *emsg, const size_t emsglen);
/*
 * When a complete request has been read, this function can be
 * called.  It will perform the request "request->req" and send the
 * result to "mother".
 *
 * If the request is for a udp-associate, the address the client told
 * us it will be sending from, if any, is stored as resolved in
 * "clientudpaddr".
 *
 * "weclosedfirst" is on failure set to indicate whether we closed
 * the connection first, or whether the client closed it first.  The
 * latter is typically what happens during network errors or blocked
 * requests.  In this case, "emsg" is filled in with the a text describing
 * the problem.
 *
 * Returns:
 *    If request was successfully completed: IO_NOERROR.
 *    If request was not performed: an iostauts indicating the reason.
 *        In this case "weclosedfirst" is also set to true or false,
 *        indicating whether we or a peer first closed the connection.
 */

static int
reqhostisok(sockshost_t *host, const int cmd, char *emsg,
            const size_t emsglen);
/*
 * Checks that the host "host" specified in the client request with command
 * "cmd" is ok.
 * If necessary, it may "fixup" host also, which currently consists of
 * converting it from an ipv4-mapped ipv6 address to a ipv4 address.
 *
 * Returns SOCKS_SUCCESS if everything is ok.
 * Returns a socks failurecode if something is not ok.  "emsg" will then
 * contain the reason.
 */

static int
flushio(int mother, sockd_io_t *io);
/*
 * "flushes" a complete io object and free's any state/resources held by it.
 * Also iolog()s that the session was accepted.
 * "mother" is connection to mother for sending the io.
 * "io" is the io object to send to mother.
 *
 * Returns 0 if the i/o object was forwarded successfully.
 * Returns -1 if the i/o object was not forwarded.
 *
 */

static void
proctitleupdate(const struct sockaddr_storage *from);
/*
 * Updates the title of this process.
 */

static route_t *
getroute(const struct sockaddr_storage *client, request_t *req,
         char *emsg, const size_t emsglen);
/*
 * Returns the route that should be used for the request "req"
 * from client "client", if a route is found.
 *
 * Returns NULL if no route is found or an error occurred.  "emsg" will then
 * contain the reason.
 */


static int
serverchain(const int targetsocket, const int clientsocket,
            const struct sockaddr_storage *client, const request_t *req,
            proxychaininfo_t *proxychain, authmethod_t *proxychainauth,
            char *emsg, const size_t emsglen);
/*
 * Checks if we should create a serverchain to the target on  targetsocket
 * "targetsocket" for the request "req".
 *
 * "clientsocket" is the socket we accepted the client on,.
 * "client" is the clients address.
 *
 * If a serverchain was created, the proxyprotocol used in the chain is set
 * in "proxyprotocol", and further information is provided in "proxychain".
 * If proxychain->proxyprotocol is set to PROXY_DIRECT, no server-chaining
 * should be done.
 *
 * If a serverchain was established, "proxychainauth" is the authentication
 * used with the upsteream proxy.
 *
 * Returns:
 *       0: Call successful.  See proxychain for information about the results.
 *      -1: Some error occurred.  "emsg" will contain the details.
 */

#if SOCKS_SERVER

static sockd_io_t *
io_add(sockd_io_t *iolist, const sockd_io_t *newio);
/*
 * Adds _a copy_ of the object "newio" to the list "iolist".
 * Returns a pointer to the (new) iolist.
 */

static sockd_io_t *
io_remove(sockd_io_t *iolist, sockd_io_t *rmio);
/*
 * Removes the object "rmio" from the list "iolist".
 * Returns a pointer to the (new) iolist.
 */

static sockd_io_t *
io_find(sockd_io_t *iolist, const struct sockaddr_storage *addr);
/*
 * Scans "iolist" for a object that contains "addr" as a local address.
 * If "addr" is NULL, returns "iolist".
 * Returns:
 *      On success: pointer to the matching io object.
 *      On failure: NULL.
 */
#endif /* SOCKS_SERVER */

void
run_request()
{
   const char *function = "run_request()";
   sockd_request_t req;
   struct sigaction sigact;
   fd_set *rset;

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

   rset  = allocate_maxsize_fdset();
   req.s = -1;

   sockd_print_child_ready_message((size_t)freedescriptors(NULL, NULL));

   while (1) {
      /*
       * Get request from mother, perform it, get next request.
       */
#if DIAGNOSTIC
      const int freec
      = freedescriptors(sockscf.option.debug ? "start" : NULL, NULL);
#endif /* DIAGNOSTIC */
      iostatus_t iostatus;
      struct sockaddr_storage clientudpaddr;
      char command, emsg[2048];
      int fdbits, weclosedfirst;

      errno = 0; /* reset for each iteration. */

      proctitleupdate(NULL);

      FD_ZERO(rset);
      fdbits = -1;

      if (sockscf.state.mother.s != -1) {
         FD_SET(sockscf.state.mother.s, rset);
         fdbits = sockscf.state.mother.s;
      }

      /* checked so we know if mother goes away.  */
      FD_SET(sockscf.state.mother.ack, rset);
      fdbits = MAX(fdbits, sockscf.state.mother.ack);

      ++fdbits;
      switch (selectn(fdbits, rset, NULL, NULL, NULL, NULL, NULL)) {
         case -1:
            SASSERT(ERRNOISTMP(errno));
            continue;

         case 0:
            SERRX(0);
      }

      if (FD_ISSET(sockscf.state.mother.ack, rset)) { /* only eof expected. */
         sockd_readmotherscontrolsocket(function, sockscf.state.mother.ack);

#if HAVE_VALGRIND_VALGRIND_H
         if (RUNNING_ON_VALGRIND) {
            /* for some reason Valgrind complains the rset pointer is lost. */
            free(rset);
         }
#endif /* HAVE_VALGRIND_VALGRIND_H */

         sockdexit(EXIT_FAILURE);
      }

      if (FD_ISSET(sockscf.state.mother.s, rset)) {
         if (recv_req(sockscf.state.mother.s, &req) == -1)
            serr("%s: failed reading new request from mother: %s",
                 function, strerror(errno));
      }

      iostatus = dorequest(&sockscf.state.mother,
                           &req,
                           &clientudpaddr,
                           &weclosedfirst,
                           emsg,
                           sizeof(emsg));

      if (iostatus != IO_NOERROR) {
         /*
          * log the client-rule and hostid-rule close also if appropriate,
          * as this will not be logged on the normal session-close in the i/o
          * process because the session was not successfully established.
          */
         operation_t op;
         clientinfo_t cinfo;
         iologaddr_t src;
         rule_t *rule;
         size_t rused;
         char reason[sizeof(emsg) + 1024];

         slog(LOG_DEBUG,
              "%s: dorequest() failed.  srule_isset = %d, weclosedfirst = %d, "
              "iostatus = %d, error: %s",
              function, req.srule_isset, weclosedfirst, iostatus, emsg);

         log_ruleinfo_shmid(CRULE_OR_HRULE(&req), function, NULL);

         if (req.srule_isset)
            rule = &req.srule;
         else
            rule = CRULE_OR_HRULE(&req);

         log_ruleinfo_shmid(rule, function, NULL);

         if (req.s == -1) {
            SASSERTX(BAREFOOTD);
            SASSERTX(req.state.protocol == SOCKS_UDP);

            /*
             * nothing can have been used as we need to wait
             * for the udp clients.
             */
            SHMEM_CLEAR(CRULE_OR_HRULE(&req), SHMEM_ALL, 1);

            SASSERTX(!req.srule_isset);
         }

         /*
          * Clean up shmem usage, whether the request failed before or
          * after we progressed to the socks-rule level.  In the former case,
          * no shmid-fields will be set in req->srule.
          */

         cinfo.from   = req.from;
         cinfo.hostid = req.state.hostid;

         switch (iostatus) {
            case IO_BLOCK:
               rused = 0;
               op    = OPERATION_BLOCK;
               break;

            case IO_CLOSE:
               op    = OPERATION_DISCONNECT;
               rused = 0;
               break;

            case IO_IOERROR:
            case IO_ERROR:
               rused = snprintf(reason, sizeof(reason),
                                "request was not performed due to error");
               op    = OPERATION_ERROR;
               break;

            default:
               SWARNX(iostatus);

               rused = 0;
               op    = OPERATION_ERROR;
         }

         if (*emsg != NUL)
            snprintf(&reason[rused], sizeof(reason) - rused,
                     "%s%s",
                     rused > 0 ? ": " : "",
                     emsg);

         if (rule->mstats_shmid != 0
         && (rule->alarmsconfigured & ALARM_DISCONNECT))
            alarm_add_disconnect(weclosedfirst,
                                 rule,
                                 ALARM_INTERNAL,
                                 &cinfo,
                                 strerror(errno),
                                 sockscf.shmemfd);

         SHMEM_UNUSE(rule, &cinfo, sockscf.shmemfd, SHMEM_ALL);

         switch (req.state.protocol) {
            case SOCKS_TCP:
               command = SOCKD_FREESLOT_TCP;
               break;

            case SOCKS_UDP:
               command = SOCKD_FREESLOT_UDP;
               break;

            default:
               SERRX(req.state.protocol);
         }

         if (socks_sendton(sockscf.state.mother.ack,
                           &command,
                           sizeof(command),
                           sizeof(command),
                           0,
                           NULL,
                           0,
                           NULL,
                           NULL) != sizeof(command))
            slog(sockd_motherexists() ? LOG_WARNING : LOG_DEBUG,
                 "%s: sending ack to mother failed: %s",
                 function, strerror(errno));

         /*
          * dorequest() logs the failed socks-level session close with
          * appropriate details, but we need to log the client-level close.
          */
         init_iologaddr(&src,
                        object_sockaddr,
                        &req.to,
                        object_sockaddr,
                        &req.from,
                        &req.cauth,
                        &req.state.hostid);

#if HAVE_SOCKS_HOSTID
         if (req.hrule_isset) {
            req.state.command = SOCKS_HOSTID;
            iolog(&req.hrule,
                  &req.state,
                  op,
                  &src,
                  NULL,
                  NULL,
                  NULL,
                  reason,
                  strlen(reason));
         }
#endif /* HAVE_SOCKS_HOSTID */

         req.state.command  = SOCKS_ACCEPT;
         req.state.protocol = SOCKS_TCP;

#if !HAVE_SOCKS_RULES /* only logged if debug is enabled. */
         if (sockscf.option.debug)
#endif /* HAVE_SOCKS_RULES */
         iolog(&req.crule,
               &req.state,
               op,
               &src,
               NULL,
               NULL,
               NULL,
               reason,
               strlen(reason));
      }

      delete_req(&reqv[0]);

#if DIAGNOSTIC
      if (freec != freedescriptors(sockscf.option.debug ?  "end" : NULL, NULL))
         swarnx("%s: lost %d file descriptor%s in total while handling request",
                function,
                freec - freedescriptors(NULL, NULL),
                freec - freedescriptors(NULL, NULL) == 1 ? "" : "s");
#endif /* DIAGNOSTIC */
   }
}

void
request_preconfigload(void)
{
   const char *function = "request_preconfigload()";

   slog(LOG_DEBUG, "%s", function);
}

void
request_postconfigload(void)
{
   const char *function = "request_postconfigload()";

   slog(LOG_DEBUG, "%s", function);
}


int
recv_req(s, req)
   int s;
   sockd_request_t *req;
{
   const char *function = "recv_req()";
#if HAVE_GSSAPI
   char gssapistatemem[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */
   struct iovec iov[2];
   struct msghdr msg;
   struct timeval tnow;
   ssize_t r;
   int ioc, fdexpect, fdreceived;
   CMSG_AALLOC(cmsg, sizeof(int));

#if BAREFOOTD
   if (!ALL_UDP_BOUNCED()) {
      /*
       * We're the main mother.  Go through all client-rules that have a
       * bounce-to address that requires us to listen to udp addresses and
       * fake a request for it, without going through the negotiate child
       * (there is nothing to negotiate).
       */
      rule_t *rule;
      char sstr[MAXRULEADDRSTRING], tstr[MAXRULEADDRSTRING],
           dstr[MAXRULEADDRSTRING], emsg[1024];
      int foundruletobounce = 0;

      SASSERTX(pidismainmother(sockscf.state.pid));

      bzero(req, sizeof(*req));

      for (rule = sockscf.crule; rule != NULL; rule = rule->next) {
         struct sockaddr_storage addr;
         int gaierr;

         if (!rule->state.protocol.udp || rule->bounced)
            continue;

         sockshost2sockaddr2(ruleaddr2sockshost(&rule->dst, NULL, SOCKS_UDP),
                             &req->to,
                             &gaierr,
                             emsg,
                             sizeof(emsg));

         if (gaierr != 0 || !IPADDRISBOUND(&req->to)) {
            if (gaierr != 0)
               log_resolvefailed(rule->dst.addr.domain, INTERNALIF, gaierr);

            /*
             * this is serious since it means we won't know what address we
             * should listen for udp packets on.
             */
            serr("%s: could not convert \"%s\" in destination-portion "
                 "of %s #%lu to IP-address: %s",
                 function,
                 ruleaddr2string(&rule->dst, ADDRINFO_ATYPE, NULL, 0),
                 objecttype2string(rule->type),
                 (unsigned long)rule->number,
                 emsg);
         }

         if (rule->verdict == VERDICT_PASS) {
            ruleaddr2sockaddr2(&rule->extra.bounceto,
                               &addr,
                               SOCKS_UDP,
                               &gaierr,
                               emsg,
                               sizeof(emsg));

            if (gaierr != 0 || !IPADDRISBOUND(&addr)) {
               if (gaierr != 0)
                  log_resolvefailed(rule->extra.bounceto.addr.domain,
                                    EXTERNALIF,
                                    gaierr);

               swarn("%s: could not convert hostname \"%s\" in bounce-to "
                     "portion of %s #%lu to IP-address: %s",
                     function,
                     ruleaddr2string(&rule->extra.bounceto,
                                     ADDRINFO_ATYPE,
                                     NULL,
                                     0),
                     objecttype2string(rule->type),
                     (unsigned long)rule->number,
                     emsg);

               /*
                * but don't consider it fatal.  In particular if this is
                * after a sighup it would be embarrassing to exit on such
                * an error if there are hundreds of other rules working fine.
                * A warning should be enough, and do it now too, rather than
                * waiting for a client.
                */
            }
         }
         /* else; will not be using bounce-to address; packet is blocked. */

         sockshost2sockaddr2(ruleaddr2sockshost(&rule->src, NULL, SOCKS_UDP),
                             &req->from,
                             &gaierr,
                             emsg,
                             sizeof(emsg));

         if ((rule->src.atype == SOCKS_ADDR_DOMAIN
         ||   rule->src.atype == SOCKS_ADDR_IFNAME)
         && !IPADDRISBOUND(&req->from)) {
            if (rule->src.atype        == SOCKS_ADDR_DOMAIN
            &&  *rule->src.addr.domain == '.') {
               /*
                * Rule is on ".<domain>" form.  Not resolvable.
                * Just use zeros for now.  Not needed till we get an
                * actual client, and will be updated then.
                */
               bzero(&req->from, sizeof(req->from));
               SET_SOCKADDR(&req->from, AF_INET);
            }
            else {
               if (rule->src.atype == SOCKS_ADDR_DOMAIN && gaierr != 0)
                  log_resolvefailed(rule->src.addr.domain, INTERNALIF, gaierr);

               swarn("%s: could not convert \"%s\" in source-portion of "
                     "%s #%lu to IP-address: %s",
                     function,
                     ruleaddr2string(&rule->src, ADDRINFO_ATYPE, NULL, 0),
                     objecttype2string(rule->type),
                     (unsigned long)rule->number,
                     emsg);

            }
         }

         slog(LOG_DEBUG,
              "%s: creating new udp session for clients from %s to target %s, "
              "to be accepted on %s, for %s #%lu",
              function,
              ruleaddr2string(&rule->src, ADDRINFO_PORT, sstr, sizeof(sstr)),
              ruleaddr2string(&rule->extra.bounceto,
                              ADDRINFO_PORT,
                              tstr,
                              sizeof(tstr)),
              ruleaddr2string(&rule->dst, ADDRINFO_PORT, dstr, sizeof(dstr)),
              objecttype2string(rule->type),
              (unsigned long)rule->number);

         req->crule                     = *rule;

         req->cauth.method              = AUTHMETHOD_NONE;
         req->sauth.method              = AUTHMETHOD_NONE;
         req->s                         = -1;

         req->state.command             = SOCKS_ACCEPT;
         req->state.protocol            = SOCKS_UDP;

         req->req.version               = PROXY_SOCKS_V5;
         req->req.command               = SOCKS_UDPASSOCIATE;
         req->req.flag                  = 0;

         /*
          * We can obviously only accept IPvX packets on an IPvX socket,
          * so client must send from an IPvX address.
          */
         req->req.host.atype            = rule->dst.atype;

         switch (req->req.host.atype) {
            case SOCKS_ADDR_IPV4:
               req->req.host.addr.ipv4.s_addr = htonl(INADDR_ANY);
               break;

            case SOCKS_ADDR_IPV6:
               req->req.host.addr.ipv6.ip      = in6addr_any;
               req->req.host.addr.ipv6.scopeid = rule->dst.addr.ipv6.scopeid;
               break;

            default:
               SERRX(req->req.host.atype);
         }

         req->req.host.port             = htons(0);

         req->req.auth                  = &req->sauth;
         req->req.protocol              = SOCKS_UDP;

         /*
          * no negotiation going on here; what we want is what we get.
          */
         req->state.command       = req->req.command;
         req->state.proxyprotocol = req->req.version;

         gettimeofday_monotonic(&req->state.time.accepted);
         req->state.time.negotiatestart = req->state.time.accepted;
         req->state.time.negotiateend   = req->state.time.accepted;
         req->state.time.established    = req->state.time.accepted;

         rule->bounced     = 1;
         foundruletobounce = 1;
         break;
      }

      if (foundruletobounce) {
         SASSERTX(req->s == -1);
         return 0;
      }
      else {
         slog(LOG_DEBUG, "%s: no more addresses to bounce", function);
         sockscf.state.alludpbounced = 1;

         errno = EAGAIN;
         return -1;
      }
   }
#endif /* BAREFOOTD */

   ioc = 0;
   bzero(iov, sizeof(iov));
   iov[ioc].iov_base = req;
   iov[ioc].iov_len  = sizeof(*req);
   ++ioc;

#if HAVE_GSSAPI
   iov[ioc].iov_base = gssapistatemem;
   iov[ioc].iov_len  = sizeof(gssapistatemem);
   ++ioc;
#endif /* HAVE_GSSAPI */

   bzero(&msg, sizeof(msg));
   msg.msg_iov     = iov;
   msg.msg_iovlen  = ioc;
   msg.msg_name    = NULL;
   msg.msg_namelen = 0;

   /* LINTED pointer casts may be troublesome */
   CMSG_SETHDR_RECV(msg, cmsg, CMSG_MEMSIZE(cmsg));

   if ((r = recvmsgn(s, &msg, 0)) < (ssize_t)sizeof(*req)) {
      switch (r) {
         case -1:
            slog(LOG_DEBUG, "%s: recvmsg() failed: %s",
            function, strerror(errno));
            break;

         case 0:
            slog(LOG_DEBUG, "%s: recvmsg(): other side closed connection",
            function);
            break;

         default:
            swarnx("%s: recvmsg(): unexpected short read: %ld/%lu",
                   function, (long)r, (unsigned long)sizeof(*req));
      }

      return -1;
   }

   if (socks_msghaserrors(function, &msg))
      return -1;

   gettimeofday_monotonic(&tnow);
   sockd_check_ipclatency("client object received from negotiate process",
                          &req->state.time.negotiateend,
                          &tnow,
                          &tnow);

   SASSERTX(req->srule.bw == NULL);
   SASSERTX(req->srule.ss == NULL);

#if BAREFOOTD
   if (req->req.command == SOCKS_UDPASSOCIATE)
      fdexpect = 0; /* no client yet. */
   else
      fdexpect = 1; /* client. */

#else /* SOCKS_SERVER */
   fdexpect = 1; /* client. */
#endif /* SOCKS_SERVER */

   if (!CMSG_RCPTLEN_ISOK(msg, sizeof(int) * fdexpect)) {
      swarnx("%s: received control message has the invalid len of %d",
              function, (int)CMSG_TOTLEN(msg));

      return -1;
   }

   fdreceived = 0;
   if (fdexpect > 0) {
      SASSERTX(cmsg->cmsg_level == SOL_SOCKET);
      SASSERTX(cmsg->cmsg_type  == SCM_RIGHTS);

      CMSG_GETOBJECT(req->s, cmsg, sizeof(req->s) * fdreceived++);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: received fd %d (%s) ...",
         function, req->s, socket2string(req->s, NULL, 0));
   }

   req->req.auth = &req->sauth; /* pointer fixup */

#if HAVE_GSSAPI
   if (req->req.auth->method == AUTHMETHOD_GSSAPI) {
      gss_buffer_desc gssapistate;

      r -= sizeof(*req);
      SASSERTX(r > 0);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: read gssapistate of size %ld", function, (long)r);

      gssapistate.value  = gssapistatemem;
      gssapistate.length = r;

      if (gssapi_import_state(&req->req.auth->mdata.gssapi.state.id,
                              &gssapistate) != 0)
         return -1;
   }
#endif /* HAVE_GSSAPI */

   if (sockscf.option.debug >= DEBUG_VERBOSE)
      slog(LOG_DEBUG,
           "%s: received %d fds for request with method %d, req->s = %d",
           function, fdreceived, req->req.auth->method, s);

   return 0;
}

static iostatus_t
dorequest(mother, request, clientudpaddr, weclosedfirst, emsg, emsglen)
   const sockd_mother_t *mother;
   sockd_request_t *request;
   struct sockaddr_storage *clientudpaddr;
   int *weclosedfirst;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "dorequest()";
   iostatus_t iostatus;
   sockd_io_t io;
   response_t response;
   iologaddr_t *src, srcmem, *dst, dstmem;
   clientinfo_t cinfo;
   size_t msglen;
   char strhost[MAXSOCKSHOSTSTRING], buf[(MAXHOSTNAMELEN * 4) + 256];
   int rc;
#if SOCKS_SERVER
   sockshost_t expectedbindreply, bindreplydst;
#endif /* SOCKS_SERVER */

#if HAVE_SOCKS_RULES
   int permit;
#endif /* HAVE_SOCKS_RULES */

   *emsg = NUL;

   /*
    * If the request fails, we will typically close first, as otherwise
    * the reason for the request failing would typically be a network error,
    * which there are not many checks for (typically only detected when
    * we send the response to the client).
    */
   *weclosedfirst = 1;

   slog(LOG_DEBUG,
        "%s: command %d request received from client %s.  Accepted on %s, "
        "matched by %s #%lu, auth: %s.  Packet: %s ",
        function,
        request->state.command,
        sockaddr2string(&request->from, strhost, sizeof(strhost)),
        sockaddr2string(&request->to, NULL, 0),
        objecttype2string(request->crule.type),
        (unsigned long)request->crule.number,
        method2string(request->req.auth->method),
        socks_packet2string(&request->req, 1));

   proctitleupdate(&request->from);
   init_req(&reqv[0], request);

   bzero(&response, sizeof(response));
   response.host   = request->req.host;
   response.auth   = request->req.auth;

   bzero(&io, sizeof(io));

   io.control.auth.method = AUTHMETHOD_NOTSET;
   io.control.s           = -1;
   io.src                 = io.control;

   io.dst.s               = -1;

   io.reqflags            = request->req.flags;
   io.state               = request->state;

   SASSERTX(proxyprotocolisknown(io.state.proxyprotocol));

   io.crule               = request->crule;

#if HAVE_SOCKS_HOSTID
   io.hrule               = request->hrule;
   io.hrule_isset         = request->hrule_isset;
#endif /* HAVE_SOCKS_HOSTID */

   io.cauth               = request->cauth;

#if HAVE_NEGOTIATE_PHASE
   CTASSERT(sizeof(io.clientdata) == sizeof(request->clientdata));
   io.clientdatalen = request->clientdatalen;
   memcpy(io.clientdata, request->clientdata, io.clientdatalen);
#endif /* HAVE_NEGOTIATE_PHASE */

   src = &srcmem;
   init_iologaddr(src,
                  object_sockaddr,
                  &request->to,
                  object_sockaddr,
                  &request->from,
                  request->req.auth,
                  &request->state.hostid);

   dst = NULL; /* for now. */

   /*
    * examine client request.
    */

   /* supported version? */
   switch (request->req.version) {
#if SOCKS_SERVER || BAREFOOTD
      case PROXY_SOCKS_V4:
         response.version = PROXY_SOCKS_V4REPLY_VERSION;

         /* supported address format for this version? */
         switch (request->req.host.atype) {
            case SOCKS_ADDR_IPV4:
               break;

            default:
               msglen = snprintf(emsg, emsglen, "unrecognized v%d atype: %d",
                                 request->req.version, request->req.host.atype);

               iolog(&io.crule,
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     msglen);

               send_failure(request->s, &response, SOCKS_ADDR_UNSUPP);

               close(request->s);
               return IO_ERROR;
         }

         /* recognized command for this version? */
         switch (request->req.command) {
            case SOCKS_BIND:
            case SOCKS_CONNECT:
               io.state.protocol        = SOCKS_TCP;
               io.src.state.isconnected = 1;
               break;

            default:
               io.state.command = SOCKS_UNKNOWN;

               msglen = snprintf(emsg, emsglen,
                                 "unrecognized v%d command: %d",
                                 request->req.version, request->req.command);

               iolog(&io.crule,
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     msglen);

               send_failure(request->s, &response, SOCKS_CMD_UNSUPP);

               close(request->s);
               return IO_ERROR;
         }
         break; /* PROXY_SOCKS_V4 */

      case PROXY_SOCKS_V5:
         response.version = request->req.version;

         /* supported address format for this version? */
         switch (request->req.host.atype) {
            case SOCKS_ADDR_IPV4:
            case SOCKS_ADDR_IPV6:
            case SOCKS_ADDR_DOMAIN:
               break;

            default:
               msglen = snprintf(emsg, emsglen,
                                 "unrecognized v%d atype: %d",
                                 request->req.version, request->req.host.atype);

               iolog(&io.crule,
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     msglen);

               send_failure(request->s, &response, SOCKS_ADDR_UNSUPP);

               close(request->s);
               return IO_ERROR;
         }

         /* recognized command for this version? */
         switch (request->req.command) {
            case SOCKS_BIND:
            case SOCKS_CONNECT:
               io.state.protocol = SOCKS_TCP;
               break;

            case SOCKS_UDPASSOCIATE:
               io.state.protocol = SOCKS_UDP;
               break;

            default:
               io.state.command = SOCKS_UNKNOWN;

               msglen = snprintf(emsg, emsglen,
                                 "unrecognized v%d command: %d",
                                 request->req.version, request->req.command);

               iolog(&io.crule,
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     msglen);

               send_failure(request->s, &response, SOCKS_CMD_UNSUPP);

               close(request->s);
               return IO_ERROR;
         }

         break; /* PROXY_SOCKS_V5 */
#endif /* SOCKS_SERVER || BAREFOOTD */

#if COVENANT
      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         SASSERTX(request->req.command == SOCKS_CONNECT);
         SASSERTX (request->req.host.atype == SOCKS_ADDR_IPV4
         ||        request->req.host.atype == SOCKS_ADDR_IPV6
         ||        request->req.host.atype == SOCKS_ADDR_DOMAIN);

         response.version  = request->req.version;
         io.state.protocol = SOCKS_TCP;
         break;
#endif /* COVENANT */

      default:
         SERRX(request->req.version);
   }

   rc = reqhostisok(&request->req.host, request->req.command, emsg, emsglen);

   if (rc != SOCKS_SUCCESS) {
      iolog(&io.crule,
            &io.state,
            OPERATION_ERROR,
            src,
            dst,
            NULL,
            NULL,
            emsg,
            strlen(emsg));

      send_failure(request->s, &response, rc);

      close(request->s);
      return IO_ERROR;
   }

   /*
    * packet looks ok to, fill in remaining bits and check rules.
    */
   reqv[0].request_isvalid = 1;

   switch (request->req.command) {
#if SOCKS_SERVER
      case SOCKS_BIND:
         /*
          * Bind is a bit cumbersome.
          * We first need to check if the bind request is allowed, and then
          * we transform io.dst to something completely different to check
          * if the bindreply is allowed.
          */

         io.src.s                  = request->s;
         io.src.state.isconnected = 1;
         io.src.laddr             = request->to;
         io.src.raddr             = request->from;
         io.src.auth              = *request->req.auth;
         sockaddr2sockshost(&io.src.raddr, &io.src.host);

         io.dst.host = request->req.host;

         if (io.dst.host.atype            == SOCKS_ADDR_IPV4
         &&  io.dst.host.addr.ipv4.s_addr == htonl(BINDEXTENSION_IPADDR)) {
            slog(LOG_DEBUG, "%s: bind extension enabled for request from %s",
                 function, sockaddr2string(&request->from, NULL, 0));

            io.state.extension.bind = 1;
         }
         else
            io.state.extension.bind = 0;

         /*
          * Semantics in v4 and v5 are slightly different, so if v4
          * convert to corresponding v5 semantics, so we later can
          * treat v4 and v5 as the same (mostly).
          */
         switch (request->req.version) {
            case PROXY_SOCKS_V5:
               /*
                * One popular interpretation is that the port always gives
                * the desired port, rather than the port the client
                * previously connected to.  So that is what we assume,
                * and we convert the v4 request to something that can match
                * that.
                */
               break;

            case PROXY_SOCKS_V4:
               /*
                * If the address is 0, assume port is port client
                * wants to bind.  If not, best we can try for is to
                * use same port as client used for connecting to us.
                */
               SASSERTX(request->req.host.atype == SOCKS_ADDR_IPV4);

               if (request->req.host.addr.ipv4.s_addr != htonl(0))
                  request->req.host.port = GET_SOCKADDRPORT(&request->to);
               /* else; assume port already set to what client wants to bind. */
               break;

            default:
               SERRX(request->req.version);
         }

         break;
#endif /* SOCKS_SERVER */

      case SOCKS_CONNECT:
         io.src.s                 = request->s;
         io.src.state.isconnected = 1;
         io.src.laddr             = request->to;
         io.src.raddr             = request->from;
         io.src.auth              = *request->req.auth;
         sockaddr2sockshost(&io.src.raddr, &io.src.host);

         io.dst.host              = request->req.host;
         break;

      case SOCKS_UDPASSOCIATE:
         /*
          * some things will change, but some things, auth included, will
          * stay the same.
          */

#if HAVE_CONTROL_CONNECTION
         io.control.s                 = request->s;
         io.control.state.isconnected = 1;
         io.control.state.isconnected = 1;
         io.control.laddr             = request->to;
         io.control.raddr             = request->from;
         io.control.auth              = *request->req.auth;

         /* let src get the same credentials as control. */
         io.src.auth                  = *request->req.auth;

         sockaddr2sockshost(&io.control.raddr, &io.control.host);
#endif /* HAVE_CONTROL_CONNECTION */

         io.src.host = request->req.host;
         sockshost2sockaddr(&io.src.host, &io.src.raddr);

         bzero(&io.src.laddr, sizeof(io.src.laddr));
         SET_SOCKADDR(&io.src.laddr, io.src.raddr.ss_family);

         /* needs to be done so caller can shmem_unuse() correct cinfo. */
         *clientudpaddr = io.src.raddr;

         /*
          * for UDP_ASSOCIATE we are getting the clients UDP address in the
          * request, not the target destination.  Destination address will
          * be contained in each socks udp packet received, and checked in
          * the i/o process for each destination for each packet.
          */
         break;

      default:
         SERRX(request->req.command);
   }

   /*
    * Update now that we have parsed the request and know what is what.
    */
   init_iologaddr(src,
                  object_sockaddr,
                  &io.src.laddr,
                  object_sockshost,
                  &io.src.host,
                  &io.src.auth,
                  &io.state.hostid);

   if (io.state.protocol == SOCKS_TCP) { /* also know parts of dst now. */
      dst = &dstmem;

      init_iologaddr(dst,
                     object_none,
                     NULL,
                     object_sockshost,
                     &io.dst.host,
                     &io.dst.auth,
                     NULL);

      switch (request->req.command) {
#if SOCKS_SERVER
         case SOCKS_BIND:
            /*
             * Figure out what address to expect the bind reply from.
             * Unfortunately, this is a mishmash of different interpretations.
             *
             * The socks v4 standard is pretty strict about the meaning,
             * while the v5 is much more ambiguous.
             *
             * Unfortunately the meaning given in these standard provides
             * limited usability, so people "interpret" the standards more
             * loose to get more practical functionality out of them.
             *
             * - If the client provided an ip address when requesting the
             *   bind, we should only return remote connections matching
             *   that ip address.  The portnumber we should ignore.
             *
             * - If the client did not provide an ip address (set it to 0),
             *   we should probably try to match neither ip nor port,
             *   but should try to bind the requested portnumber locally.
             *
             * The standard is not very clear in this area, but the above
             * interpretation seems most practical.
             */

            if (io.state.extension.bind) { /* multiple connections. */
               sockaddr2sockshost(&request->from, &io.dst.host);
               bindreplydst           = io.dst.host;

               bzero(&expectedbindreply, sizeof(expectedbindreply));
               expectedbindreply.atype = SOCKS_ADDR_IPV4;
            }
            else { /* only one connection; bindreply over control-channel. */
               bindreplydst = io.src.host;

               if (io.dst.host.addr.ipv4.s_addr == htonl(0)) {
                  bzero(&expectedbindreply, sizeof(expectedbindreply));
                  expectedbindreply.atype = SOCKS_ADDR_IPV4;
               }
               else
                  expectedbindreply = io.dst.host;

               expectedbindreply.port = htons(0);
            }

            slog(LOG_DEBUG, "%s: expecting bindreply from %s",
                 function, sockshost2string(&expectedbindreply, NULL, 0));
            break;

#endif /* SOCKS_SERVER */

         case SOCKS_CONNECT:
#if 0
            /*
             * Set SO_REUSEADDR to limit the chances that we run out of
             * available TCP ports to use.
             * We however need to handle the case of us trying to connect
             * from the same port (due to the client connecting from the
             * same port) to the same destination multiple times, which
             * we don't do yet.  Thus disabled.
             */
            rc = 1;
            if (setsockopt(io.dst.s, SOL_SOCKET, SO_REUSEADDR, &rc, sizeof(rc))
            != 0)
               swarn("%s: setsockopt(SO_REUSEADDR)", function);
#endif
            break;

         default:
            SERRX(request->req.command);
      }
   }

#if HAVE_SOCKS_RULES
   /*
    * rules permit?
    */
   switch (request->req.command) {
#if SOCKS_SERVER
      case SOCKS_BIND: {
         sockshost_t boundhost;

         /*
          * For the bind-command, we need to bind the external
          * address before rulespermits(), as the external address
          * is the "dst" address that rulespermit() will check.
          */
         if (bindexternaladdr(&io, &request->req, buf, sizeof(buf)) == 0)
            SASSERTX(io.dst.s != -1);
         else {
            msglen = snprintf(emsg, emsglen,
                              "could not get address to use on external "
                              "side: %s",
                              buf);

            iolog(&io.crule,
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  msglen);

            send_failure(CONTROLIO(&io)->s, &response, SOCKS_NETUNREACH);

            close_iodescriptors(&io);
            return IO_ERROR;
         }

         sockaddr2sockshost(&io.dst.laddr, &boundhost);
         permit = rulespermit(request->s,
                              &request->from,
                              &request->to,
                              &io.cauth,
                              &io.src.auth,
                              &io.srule,
                              &io.state,
                              &io.src.host,
                              &boundhost,
                              NULL,
                              emsg,
                              emsglen);

         /*
          * XXX we should check whether it's possible receive any bindreply
          * also.  No need to stick around if no replies will be allowed.
          */

         break;
      }
#endif /* SOCKS_SERVER */

      case SOCKS_CONNECT: {
         sockshost_t dsthostmatched;

         permit = rulespermit(request->s,
                              &request->from,
                              &request->to,
                              &io.cauth,
                              &io.src.auth,
                              &io.srule,
                              &io.state,
                              &io.src.host,
                              &io.dst.host,
                              &dsthostmatched,
                              emsg,
                              emsglen);

         if (permit) {
            if (!sockshostareeq(&io.dst.host, &dsthostmatched)) {
               /*
                * this is the address we have to connect to.
                */
               slog(LOG_DEBUG,
                    "%s: client requested target %s, we will use %s",
                    function,
                    sockshost2string(&io.dst.host, strhost, sizeof(strhost)),
                    sockshost2string(&dsthostmatched, NULL, 0));

               request->req.host = io.dst.host = dsthostmatched;
            }
         }

         break;
      }

#if HAVE_UDP_SUPPORT
      case SOCKS_UDPASSOCIATE: {
         /*
          * UDP is slightly more complex since the client is allowed to send
          * an "incomplete" address when setting up the udp associate.
          * If it does that, we may not be able to give a conclusive verdict
          * here on whether any packets will be allowed from it or not.
          * If the verdict would be deny regardless of what address the
          * client would send the udp packet from we can however make a
          * conclusive verdict now.
          *
          * Destination address can vary for each packet so we use NULL for
          * that until we get an packet.
          *
          * Another complication is what to do about resource-limits and
          * which addresses to consider when doing the socks-level
          * rulespermit().  Since the udp destination address can vary
          * for each packet, it does not make sense to allocate/free
          * resources (sessionlimits, bandwidth limits, etc.) for each
          * packet, even though have to do a rulespermit() on each packet.
          * It seems better to consider the rule matched by rulespermit()
          * here, based on the client's TCP control address, to be "the"
          * rule.  That allows us to allocate resources here for udp
          * clients, the same way as we do it for tcp clients, rather than
          * have a lot of complicated stuff for handling it in the i/o
          * process.
          *
          * The procedure thus becomes that we use the clients TCP
          * address to evaluate what resources to allocate, and then
          * use the clients UDP address to evaluate whether each individual
          * packet should be passed through.  If the client sends us it's
          * UDP address in the udpassociate request, we can also evaluate
          * here whether it's at all possible that any udp packets from
          * it will be let through.  If not, we can just as well block it
          * now, rather than waste resources on a client that will never
          * be allowed to forward any packets.
          */
         sockshost_t *srchost;

         if (SOCKSHOSTISBOUND(&io.src.host)) {
            srchost = &io.src.host;

            slog(LOG_DEBUG,
                 "%s: got a complete UDP source address (%s) so can make a "
                 "conclusive verdict now",
                 function, sockshost2string(&io.src.host, NULL, 0));
         }
         else {
            slog(LOG_DEBUG,
                 "%s: got an incomplete UDP source address (%s) so may not "
                 "be able to make a conclusive verdict yet",
                 function, sockshost2string(&io.src.host, NULL, 0));

            srchost = NULL;
         }

         if (srchost != NULL) {
            /*
             * Don't know target address of course (can be vary), but
             * if no packets at all are permitted from the source address,
             * we can know for sure no packets will be permitted.
             */
            permit = rulespermit(request->s,
                                 &request->from,
                                 &request->to,
                                 &io.cauth,
                                 &io.control.auth, /* accept this for src too */
                                 &io.srule,
                                 &io.state,
                                 srchost,
                                 NULL,
                                 NULL,
                                 emsg,
                                 emsglen);
         }

         if (srchost == NULL
         || (srchost != NULL && permit)) {
            /*
             * Either don't know the source address, or the source address
             * is permitted, so go on and do the check against the
             * control-connection address to, to know what resources to
             * allocate.
             */
            permit = rulespermit(request->s,
                                 &request->from,
                                 &request->to,
                                 &io.cauth,
                                 &io.control.auth,
                                 &io.srule,
                                 &io.state,
                                 &io.control.host,
                                 NULL,
                                 NULL,
                                 emsg,
                                 emsglen);
         }

         break;
      }
#endif /* HAVE_UDP_SUPPORT */

      default:
         SERRX(request->req.command);
   }

   /*
    * Update now that auth, if any, is set.
    */
   src->auth_isset = 1;
   src->auth       = io.src.auth;

   /*
    * Let the socks-rule inherit the appropriate settings from the
    * lower level rule, but don't bother if rule does not permit anyway.
    */
   if (permit) {
      /*
       * Have to handle the monitor-rules with care now.
       * If the target was not known at the client-rule level (as is the
       * case for Dante), it may mean the results of the monitormatch()
       * done in the negotiate process is not the same as it would be now,
       * now that the target is know (for the tcp case).
       *
       * We don't want to let a session use more than one monitor, so
       * what we do is this:
       * - If the session matches a different monitor at this (socks-rule)
       *   level, we use that monitor and disregard the monitor we matched
       *   at the client-rule level, if any.
       *   How do we handle the monitor we disregard, though?  The same
       *   way we do when we normally close a side on our own (rather than
       *   as a response to the client closing).  This decrements the
       *   session counter in the monitor and increments the disconnect
       *   counter for disconnects done by ourselves, which should set
       *   things correct.
       *
       * - If the session does not match a different monitor at this level,
       *   we continue to use the one matched at the client-level, if any.
       *
       * This matches our hardcoded behaviour where we consider
       * monitor-settings to always be inherited.
       */

      cinfo.from   = request->from;
      cinfo.hostid = request->state.hostid;

      rc = rule_inheritoruse(CRULE_OR_HRULE(&io),
                             &cinfo,
                             &io.srule,
                             &cinfo,
                             ALARM_INTERNAL,
                             emsg,
                             emsglen);

      /* wait until here so we have the correct shmem-settings. */
      request->srule         = io.srule;
      request->srule_isset = 1;

      if (rc != 0) {
         /*
          * Use from lower-level rule, as rule_inheritoruse() will
          * have cleared the monitor fields, after unusing if necessary.
          */
         if (CRULE_OR_HRULE(request)->mstats_shmid != 0
         && (CRULE_OR_HRULE(request)->alarmsconfigured & ALARM_DISCONNECT))
            alarm_add_disconnect(0,
                                 CRULE_OR_HRULE(request),
                                 ALARM_INTERNAL,
                                 &cinfo,
                                 strerror(errno),
                                 sockscf.shmemfd);

         permit = 0;
      }
   }
   else {
      request->srule       = io.srule;
      request->srule_isset = 1;
      SHMEM_MOVE(CRULE_OR_HRULE(request), &request->srule, SHMEM_ALL);
   }

   if (!permit) {
      iolog(IORULE(&io),
            &io.state,
            OPERATION_BLOCK,
            src,
            dst,
            NULL,
            NULL,
            emsg,
            strlen(emsg));

#if HAVE_NEGOTIATE_PHASE
      send_failure(CONTROLIO(&io)->s, &response, SOCKS_NOTALLOWED);
#endif /* HAVE_NEGOTIATE_PHASE  */

      snprintf(buf, sizeof(buf),
               "blocked by higher-level %s #%lu%s%s",
               objecttype2string(IORULE(&io)->type),
               (unsigned long)IORULE(&io)->number,
               *emsg == NUL ? "" : ": ",
               emsg);

      strncpy(emsg, buf, emsglen - 1);
      emsg[emsglen - 1] = NUL;

      close_iodescriptors(&io);
      return IO_BLOCK;
   }

   SASSERTX(permit);


#else /* !HAVE_SOCKS_RULES */

   /*
    * copy over auth from lower level.
    */
   io.src.auth = io.control.auth = io.cauth;

   /*
    * no actually socks-rules, just copy over from crule to reduce
    * the amount of #ifdefs, as most is identical.
    *
    * Resourcelimits should always be set in the crule, so make sure
    * we clear that however.
    */
   io.srule       = io.crule;
   io.srule.type  = object_srule;

   SHMEM_CLEAR(&io.srule, SHMEM_ALL, 1);
#endif /* !HAVE_SOCKS_RULES */

   if (io.state.protocol == SOCKS_TCP) { /* udp src.s not created yet. */
      if (io.dst.s == -1) {
         if (bindexternaladdr(&io, &request->req, buf, sizeof(buf)) == 0)
            SASSERTX(io.dst.s != -1);
         else {
            msglen = snprintf(emsg, emsglen,
                              "could not bind address to use on external "
                              "side: %s",
                              buf);

            iolog(&io.crule,
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  msglen);

            send_failure(CONTROLIO(&io)->s, &response, SOCKS_NETUNREACH);

            close_iodescriptors(&io);
            return IO_ERROR;
         }

         setconfsockoptions(io.src.s,
                            io.src.s,
                            io.state.protocol,
                            1,
                            io.srule.socketoptionc,
                            io.srule.socketoptionv,
                            SOCKETOPT_ANYTIME | SOCKETOPT_POST,
                            0 /* should be set already. */);
      }

      sockaddr2sockshost(&io.dst.laddr, &dst->local);
      dst->local_isset = 1;
   }
   else
      io.dst.s = -1; /* will be set up in the i/o process when/if needed. */

   /*
    * Redirect if necessary and set socket options for target side.
    */
   if (io.state.protocol == SOCKS_TCP) {
      SASSERTX(io.dst.s != -1);

      if (redirect(io.dst.s,
                   &io.dst.laddr,
#if !BAREFOOTD
                   &io.dst.host,
#endif /* !BAREFOOTD */
                   request->req.command,
                   &io.srule.rdr_from
#if !BAREFOOTD
                 , &io.srule.rdr_to
#endif /* !BAREFOOTD */
                   ) != 0) {

         if (io.srule.log.error) {
            msglen = snprintf(emsg, emsglen,
                              "redirect() failed: %s", strerror(errno));
            iolog(&io.srule,
                   &io.state,
                   OPERATION_ERROR,
                   src,
                   dst,
                   NULL,
                   NULL,
                   emsg,
                   msglen);
         }

         send_failure(CONTROLIO(&io)->s,
                      &response,
                      errno2reply(errno, response.version));

         close_iodescriptors(&io);
         return IO_ERROR;
      }

      setsockoptions(io.dst.s, io.dst.laddr.ss_family, SOCK_STREAM, 0);

      setconfsockoptions(io.dst.s,
                         CONTROLIO(&io)->s,
                         SOCKS_TCP,
                         0,
                         io.srule.socketoptionc,
                         io.srule.socketoptionv,
                         SOCKETOPT_PRE | SOCKETOPT_ANYTIME,
                         SOCKETOPT_PRE | SOCKETOPT_ANYTIME);
   }

   errno = 0;
   if (serverchain(io.dst.s,
                   CONTROLIO(&io)->s,
                   &io.src.raddr,
                   &request->req,
                   &io.state.proxychain,
                   io.state.protocol == SOCKS_TCP ? &io.dst.auth : NULL,
                   buf,
                   sizeof(buf)) == 0) {
      if (io.state.proxychain.proxyprotocol != PROXY_DIRECT) {
         socklen_t sinlen;

         /*
          * In case a redirect statement in the route changed laddr.
          */
         sinlen = sizeof(io.dst.laddr);
         if (getsockname(io.dst.s, TOSA(&io.dst.laddr), &sinlen) != 0) {
            slog(LOG_DEBUG,
                 "%s: strange ... serverchain() succeeded, but now "
                 "getsockname(2) failed: %s",
                 function, strerror(errno));

            if (io.srule.log.error) {
               msglen = snprintf(emsg, emsglen,
                                 "getsockname(io.dst.s) failed: %s",
                                 strerror(errno));

               iolog(&io.srule,
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     msglen);
            }

            *weclosedfirst = 0;

            close_iodescriptors(&io);

            return IO_ERROR;
         }

         io.src.state.isconnected = 1;

         sinlen = sizeof(io.dst.raddr);
         if (getpeername(io.dst.s, TOSA(&io.dst.raddr), &sinlen) != 0) {
            slog(LOG_DEBUG,
                 "%s: strange ... serverchain() succeeded, but now "
                 "getpeername(2) failed: %s",
                 function, strerror(errno));

            if (io.srule.log.error) {
               msglen = snprintf(emsg, emsglen,
                                 "getpeername(io.dst.s) failed: %s",
                                 strerror(errno));

               iolog(&io.srule,
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     msglen);
            }

            *weclosedfirst = 0;

            close_iodescriptors(&io);

            return IO_ERROR;
         }

         io.dst.state.isconnected = 1;

         setconfsockoptions(io.dst.s,
                            request->s,
                            io.state.protocol,
                            0,
                            io.srule.socketoptionc,
                            io.srule.socketoptionv,
                            SOCKETOPT_POST,
                            SOCKETOPT_POST);

         if (SHMEMRULE(&io)->mstats_shmid != 0
         && (SHMEMRULE(&io)->alarmsconfigured & ALARM_DISCONNECT))
            alarm_add_connect(SHMEMRULE(&io),
                              ALARM_EXTERNAL,
                              &cinfo,
                              sockscf.shmemfd);

         io.reqinfo.command = (io.state.protocol == SOCKS_TCP ?
                                 SOCKD_FREESLOT_TCP : SOCKD_FREESLOT_UDP);

         if (flushio(mother->s, &io) == 0)
            return IO_NOERROR;
         else
            return IO_ERROR;
      }
      /* else: go direct.  */
   }
   else { /* some error occurred. */
      msglen = snprintf(emsg, emsglen, "serverchain failed: %s", buf);

      iolog(IORULE(&io),
            &io.state,
            OPERATION_ERROR,
            src,
            dst,
            NULL,
            NULL,
            emsg,
            msglen);

      send_failure(request->s,
                   &response,
                   errno2reply(errno,
                   response.version));

      close_iodescriptors(&io);
      return IO_ERROR;
   }

   SASSERTX(io.state.proxychain.proxyprotocol == PROXY_DIRECT);

   /*
    * Set up missing bits of io, and then and send it to mother.
    */

   socks_set_responsevalue(&response,
                           sockscode(response.version, SOCKS_SUCCESS));

   rc       = 0;
   iostatus = IO_NOERROR;

   switch (io.state.command) {
#if SOCKS_SERVER
      case SOCKS_BIND: {
         sockd_io_t *iolist;
         sockd_io_t bindio;         /* send this to iochild.  */
         socklen_t len;
         enum socketindex { client, childpipe, ourpipe, reply, remote };

         /* array of sockets, indexed by above enums.  -1 if not open. */
         int sv[(int)(remote) + 1] = { -1, -1, -1, -1, -1 }, emfile;

         /*
          * - io.dst gives the address bound on behalf of the client (io.src).
          * - expectedbindreply give the address to expect the bindreply from.
          * - bindreplydst give the address to send the bindreply to.
          */

         msglen = 0;

         if (io.state.extension.bind)
            dst->peer_isset = 0;
         else {
            dst->peer_isset = 1;
            dst->peer       = io.dst.host;
         }

         if (listen(io.dst.s, SOCKD_MAXCLIENTQUEUE) != 0) {
            msglen = snprintf(emsg, emsglen,
                              "listen(2) on bindreply socket failed: %s",
                              strerror(errno));

            swarnx("%s: %s", function, emsg);

            iostatus = IO_ERROR;
            rc       = -1;
         }

         if (rc == 0) {
            SASSERTX(sv[ELEMENTS(sv) - 1] == -1);
            if (io.state.extension.bind) {
               int pipev[2];

               /*
                * The problem is that both we and the process which receives
                * the io packet needs to know when the client closes it's
                * connection, but _we_ need to receive a query from the
                * client on the connection as well, and the io process would
                * get confused about that.  We try to hack around that
                * by making a "dummy" descriptor that the io process can
                * check as all other control connections and which we
                * can close when the client closes the real control connection,
                * so the io process can detect it.
                * Not very nice, no.
                */

               if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipev) == 0) {
                  sv[childpipe] = pipev[0];
                  sv[ourpipe]   = pipev[1];
               }
               else {
                  msglen = snprintf(emsg, emsglen,
                                    "socketpair() failed: %s", strerror(errno));

                  swarnx("%s: %s", function, emsg);

                  iostatus = IO_ERROR;
                  rc       = -1;
               }
            }
         }

         if (rc == 0) {
            /*
             * let client know what address we bound to on it's behalf.
             */

            sockaddr2sockshost(&io.dst.laddr, &response.host);
            if (send_response(request->s, &response) != 0) {
               msglen = snprintf(emsg, emsglen,
                                 "sending response to client failed: %s",
                                 strerror(errno));

               iostatus = IO_IOERROR;
               rc       = -1;
            }
         }

         if (rc != 0) {
            iolog(&io.srule,
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  msglen);

            send_failure(request->s, &response, SOCKS_FAILURE);
            close_iodescriptors(&io);
            break;
         }

         iolog(&io.srule,
               &io.state,
               OPERATION_CONNECT,
               src,
               dst,
               NULL,
               NULL,
               NULL,
               0);

         emfile = 0;
         iolist = NULL;

         bindio               = io; /* quick init of most stuff. */

         bindio.state.command = SOCKS_BINDREPLY;

         bindio.dst.host = bindreplydst;

         if (bindio.state.extension.bind) {
            sockshost2sockaddr(&bindio.dst.host, &bindio.dst.raddr);

            /* won't create socket for this til we connect to the client. */
            bzero(&bindio.dst.laddr, sizeof(bindio.dst.laddr));
            SET_SOCKADDR(&bindio.dst.laddr, AF_INET);
            TOIN(&bindio.dst.laddr)->sin_addr.s_addr = htonl(INADDR_ANY);
            TOIN(&bindio.dst.laddr)->sin_port        = htons(0);
         }
         else {
            bindio.dst.laddr           = io.src.laddr;
            bindio.dst.raddr           = io.src.raddr;
         }

         bindio.dst.auth = io.src.auth;

         bindio.src.auth.method  = AUTHMETHOD_NOTSET;
         bindio.src.laddr        = io.dst.laddr;
         sockaddr2sockshost(&bindio.src.laddr, &bindio.src.host);

         /*
          * don't know what peer will be til we accept(2) it.
          */
         bzero(&bindio.src.raddr, sizeof(bindio.src.raddr));
         SET_SOCKADDR(&bindio.src.raddr, bindio.src.laddr.ss_family);

         bindio.cmd.bind.host = io.dst.host;
         bindio.cmd.bind.rule = io.srule;

         /*
          * if we are using the bind extension, keep accepting connections
          * until client closes the control-connection.  If not, break
          * after the first.
          */

         sv[client] = io.src.s;

         /*
          * XXX this is to complicated and needs to be cleaned up.
          */
         while (1) {
            static fd_set *rset;
            iologaddr_t replysrc, replydst;
            ruleaddr_t ruleaddr;
            struct sockaddr_storage remoteaddr; /* remote address accepted.   */
            struct sockaddr_storage replyaddr;  /* addr of bindreply socket.  */
            int replyredirect, fdbits;

            init_iologaddr(&replysrc,
                           object_sockaddr,
                           &bindio.src.laddr,
                           object_none, /* will know after accept(2). */
                           NULL,
                           NULL,
                           &io.state.hostid);


            if (bindio.state.extension.bind) {
               init_iologaddr(&replydst,
                              object_none, /* will know when connecting back. */
                              NULL,
                              object_sockshost,
                              &bindio.dst.host,
                              NULL,
                              NULL);
            }
            else
               replydst = *src;

            if (rset == NULL)
               rset = allocate_maxsize_fdset();

            FD_ZERO(rset);

            /* some sockets change, most remain the same. */
            sv[reply]  = -1;
            sv[remote] = -1;

            /*
             * select(2) on ack-pipe to mother (eof only expected),
             * from client (eof only, unless bind extension),
             * and from socket we listen for a bindreply on.
             */

            fdbits = -1;

            FD_SET(mother->ack, rset);
            fdbits = MAX(fdbits, mother->ack);

            FD_SET(sv[client], rset);
            fdbits = MAX(fdbits, sv[client]);

            if (!emfile) {
               FD_SET(io.dst.s, rset);
               fdbits = MAX(fdbits, io.dst.s);
            }

            ++fdbits;
            if ((rc = selectn(fdbits, rset, NULL, NULL, NULL, NULL, NULL))
            <= 0) {
               SASSERT(ERRNOISTMP(errno));
               continue;
            }

            if (FD_ISSET(mother->ack, rset)) {
               slog(LOG_DEBUG,
                    "%s: socket to mother is readable.  Since we can only "
                    "handle one client at a time, this should only happen if "
                    "mother closes the connection.  ",
                    function);

               break; /* return to caller, and handle it there. */
            }

            if (FD_ISSET(sv[client], rset)) {
               /*
                * nothing is normally expected on control connection so
                * assume it's a bind extension query or eof.
                */
               request_t query;
               response_t queryresponse;
               negotiate_state_t state;
               struct sockaddr_storage queryaddr;
               negotiate_result_t res;

               bzero(&state, sizeof(state));
               bzero(&query, sizeof(query));
               bzero(&queryresponse, sizeof(queryresponse));

               query.auth         = request->req.auth;
               queryresponse.auth = query.auth;

               switch (res = recv_sockspacket(sv[client], &query, &state)) {
                  case NEGOTIATE_CONTINUE:
                     slog(LOG_DEBUG,
                          "%s: did not receive full request, continuing",
                          function);

                     continue;

                  case NEGOTIATE_EOF:
                  case NEGOTIATE_ERROR: {
                     operation_t op;

                     if (res == NEGOTIATE_ERROR) {
                        msglen
                        = snprintf(emsg, emsglen,
                                   "receiving request from client failed (%s)",
                                   strerror(errno));

                        op = OPERATION_ERROR;

                        if (ERRNOISNETWORK(errno)) {
                           iostatus = IO_IOERROR;
                           *weclosedfirst = 0;
                        }
                        else
                           iostatus = IO_ERROR;
                     }
                     else {
                        msglen = snprintf(emsg, emsglen, "eof from client");
                        op             = OPERATION_DISCONNECT;
                        iostatus       = IO_CLOSE;
                        *weclosedfirst = 0;
                     }

                     iolog(&io.srule,
                           &io.state,
                           op,
                           src,
                           dst,
                           NULL,
                           NULL,
                           emsg,
                           msglen);

                     rc = -1; /* session ended, nothing to forward. */
                     break;
                  }

                  case NEGOTIATE_FINISHED: {
                     sockd_io_t *fio;

                     slog(LOG_DEBUG, "received bind resolve request: %s",
                          socks_packet2string(&query, 1));

                     switch (query.version) {
                        case PROXY_SOCKS_V4:
                           queryresponse.version = PROXY_SOCKS_V4REPLY_VERSION;
                           break;

                        case PROXY_SOCKS_V5:
                           queryresponse.version = query.version;
                           break;

                        default:
                           SERRX(query.version);
                     }

                     sockshost2sockaddr(&query.host, &queryaddr);
                     if ((fio = io_find(iolist, &queryaddr)) == NULL) {
                        queryresponse.host.atype            = SOCKS_ADDR_IPV4;
                        queryresponse.host.addr.ipv4.s_addr = htonl(0);
                        queryresponse.host.port             = htons(0);
                     }
                     else {
                        SASSERTX(fio->state.command == SOCKS_BINDREPLY);
                        SASSERTX(sockaddrareeq(&fio->dst.laddr, &queryaddr, 0));

                        sockaddr2sockshost(&fio->src.raddr,
                                           &queryresponse.host);
                     }

                     if ((rc = send_response(sv[client], &queryresponse)) == 0){
                        if (fio != NULL) {
                           fio->reqinfo.command = SOCKD_NOP;
                           if (flushio(mother->s, fio) != 0)
                              return IO_ERROR;

                           emfile = MAX(0, emfile - 3); /* flushio() closes 3 */
                           iolist = io_remove(iolist, fio);
                        }
                        /* else; nothing to flush yet. */
                     }
                     else {
                        msglen
                        = snprintf(emsg, emsglen,
                                   "sending bind-response to client failed: %s",
                                   strerror(errno));

                        iolog(&io.srule,
                              &io.state,
                              OPERATION_ERROR,
                              src,
                              dst,
                              NULL,
                              NULL,
                              emsg,
                              msglen);

                        *weclosedfirst = 0;
                        iostatus       = IO_IOERROR;
                        rc             = -1; /* session ended. */
                     }

                     break;
                  }

                  default:
                     SERRX(res);
               }

               if (rc == -1)
                  break;
            }

            if (!FD_ISSET(io.dst.s, rset))
               continue;

            len = sizeof(remoteaddr);
            if ((sv[remote] = acceptn(io.dst.s, &remoteaddr, &len))
            == -1) {
               msglen = snprintf(emsg, emsglen,
                                 "failed to accept(2) bindreply: %s",
                                 strerror(errno));

                  iolog(&io.srule,
                        &io.state,
                        OPERATION_ERROR,
                        &replysrc,
                        &replydst,
                        NULL,
                        NULL,
                        emsg,
                        msglen);

               switch (errno) {
#ifdef EPROTO
                  case EPROTO:         /* overloaded SVR4 error */
#endif /* EPROTO */
                  case EWOULDBLOCK:    /* BSD */
                  case ECONNABORTED:   /* POSIX */

                  /* rest appears to be Linux stuff according to Apache src. */
#ifdef ECONNRESET
                  case ECONNRESET:
#endif /* ECONNRESET */
#ifdef ETIMEDOUT
                  case ETIMEDOUT:
#endif /* ETIMEDOUT */
#ifdef EHOSTUNREACH
                  case EHOSTUNREACH:
#endif /* EHOSTUNREACH */
#ifdef ENETUNREACH
                  case ENETUNREACH:
#endif /* ENETUNREACH */
                     continue;

                  case EMFILE:
                  case ENFILE:
                     ++emfile;
                     continue;
               }

               iostatus = IO_IOERROR;
               break; /* errno is not ok, end. */
            }

            slog(LOG_DEBUG, "%s: got a bindreply from %s",
                 function, sockaddr2string(&remoteaddr, NULL, 0));

            sockaddr2sockshost(&remoteaddr, &bindio.src.host);
            bindio.src.raddr    = remoteaddr;
            replysrc.peer_isset = 1;
            replysrc.peer       = bindio.src.host;

            /*
             * Accepted a connection.  Does remote address match requested?
             */

            if (io.state.extension.bind
            || (  expectedbindreply.atype            == SOCKS_ADDR_IPV4
               && expectedbindreply.addr.ipv4.s_addr == htonl(0))
            || addrmatch(sockshost2ruleaddr(&expectedbindreply, &ruleaddr),
                         &bindio.src.host,
                         NULL,
                         SOCKS_TCP,
                         1)) {
               permit = rulespermit(sv[remote],
                                    &remoteaddr,
                                    &io.dst.laddr,
                                    NULL,
                                    &bindio.src.auth,
                                    &bindio.srule,
                                    &bindio.state,
                                    &bindio.src.host,
                                    &bindio.dst.host,
                                    NULL,
                                    emsg,
                                    emsglen);
               replysrc.auth_isset = 1;
               replysrc.auth       = bindio.src.auth;
            }
            else {
               bindio.srule.number  = 0;
               bindio.srule.verdict = VERDICT_BLOCK;

               snprintf(emsg, emsglen,
                       "expected bindreply from %s, but got it from %s",
                        sockshost2string(&expectedbindreply,
                                         strhost,
                                         sizeof(strhost)),
                        sockshost2string(&bindio.src.host, NULL, 0));

               permit = 0;
            }

            if (permit) {
               /*
                * Now we need to decide whether we should inherit from
                * bindreply-rule to bind-request rule.
                *
                * If we are using the bind-extension, we will keep waiting
                * for new bindreplies, so we can not free the resources
                * allocated for the bindrequest, meaning the bindreply
                * rule has to allocate new resources for itself.
                *
                * If we are not using the bind extension, we will not
                * wait for any more replies and the bindreply-rule's
                * resource limits should replace the bindrequest-rule's
                * limits, or inherit from the bindrequest-rule.
                */
               const int doinherit = (bindio.state.extension.bind == 0);

               slog(LOG_DEBUG,
                    "%s: bindreply-limits in %s #%lu should %s "
                    "bindrequest-limits in %s #%lu",
                    function,
                    objecttype2string(bindio.srule.type),
                    (unsigned long)bindio.srule.number,
                    doinherit ? "consider inheriting" : "be aggregated with",
                    objecttype2string(io.srule.type),
                    (unsigned long)io.srule.number);

               if (doinherit) {
                  rc = rule_inheritoruse(&io.srule,
                                         &cinfo,
                                         &bindio.srule,
                                         &cinfo, /* always socks client */
                                         ALARM_INTERNAL,
                                         emsg,
                                         emsglen);

                  if (rc != 0) {
                     permit               = 0;
                     bindio.srule.verdict = VERDICT_BLOCK;
                  }
               }
               else {
                  if (shmem_userule(&bindio.srule, &cinfo, emsg, emsglen) != 0){
                     permit               = 0;
                     bindio.srule.verdict = VERDICT_BLOCK;
                  }
               }

               if (!permit) {
                  SHMEM_CLEAR(&bindio.srule, SHMEM_ALL, 1); /* did not use. */

                  if (!bindio.state.extension.bind) {
                     /*
                      * Will not wait for more connections.  Update the
                      * disconnect counter.
                      * Use settings in io.srule, as rule_inheritoruse() will
                      * have cleared the monitor fields, after unusing if
                      * necessary, while io.srule will still be containing
                      * the correct once, after the rulespermit() on the
                      * bind /request/.
                      */
                     if (io.srule.mstats_shmid != 0
                     && (io.srule.alarmsconfigured & ALARM_DISCONNECT))
                        alarm_add_disconnect(0,
                                             &io.srule,
                                             ALARM_INTERNAL,
                                             &cinfo,
                                             strerror(errno),
                                             sockscf.shmemfd);
                  }

                  /* wait until here so we get the correct shmem-settings. */
                  request->srule = bindio.srule;
                  SASSERTX(request->srule_isset);
               }
            }

            if (!permit) {
               iolog(&bindio.srule,
                     &bindio.state,
                     OPERATION_BLOCK,
                     &replysrc,
                     &replydst,
                     NULL,
                     NULL,
                     emsg,
                     strlen(emsg));

               if (!bindio.state.extension.bind) {
                  /*
                   * can only accept one client, and that one failed,
                   * so assume it's better to end it rather than possibly
                   * wait forever for another client.
                   */
                  size_t len = snprintf(emsg, emsglen,
                                        "bind-reply from %s blocked by "
                                        "%s #%lu",
                                        sockaddr2string(&bindio.src.raddr,
                                                        NULL,
                                                        0),
                                        objecttype2string(bindio.srule.type),
                                        (unsigned long)bindio.srule.number);

                  response.host = bindio.src.host;
                  send_failure(sv[client], &response, SOCKS_NOTALLOWED);

                  /*
                   * log the close of the opened bind session also.
                   */
                  errno = 0; /* in case send_failure() sets it. */
                  iolog(&io.srule,
                        &io.state,
                        OPERATION_BLOCK,
                        src,
                        dst,
                        NULL,
                        NULL,
                        emsg,
                        len);

                  iostatus = IO_BLOCK;
                  break;
               }
               else {
                  close(sv[remote]);
                  continue; /* wait for next client, but will there be one? */
               }
            }

            errno = 0;
            if (redirect(sv[reply],
                         &remoteaddr,
                         bindio.state.extension.bind ? &bindreplydst : NULL,
                         SOCKS_BINDREPLY,
                         &bindio.srule.rdr_from,
                         &bindio.srule.rdr_to) != 0) {

                  msglen = snprintf(emsg, emsglen,
                                    "redirect() failed: %s", strerror(errno));

                  iolog(&bindio.srule,
                        &bindio.state,
                        OPERATION_ERROR,
                        &replysrc,
                        &replydst,
                        NULL,
                        NULL,
                        emsg,
                        msglen);

               close(sv[remote]);
               close(sv[reply]);

               SHMEM_UNUSE(&bindio.srule,
                           &cinfo,
                           sockscf.shmemfd,
                           SHMEM_ALL);
               continue;
            }
            else {
               bindio.dst.host = bindreplydst;
               sockshost2sockaddr(&bindio.dst.host, &bindio.dst.raddr);
            }


            /*
             * Someone connected to socket we listen to on behalf of client.
             * If the destination to forward the reply is not the same
             * as the control address (bind extension), we need to connect
             * to it.  Otherwise we will pass the data over the same
             * connection as the client connection (which is the implicit
             * control * connection in this case).
             */

            if (bindio.state.extension.bind)
               replyredirect = 1;
            else {
               if (!sockshostareeq(&bindreplydst, &io.src.host)) {
                  slog(LOG_DEBUG,
                       "%s: need to forward reply to %s, "
                       "due to redirect-module?",
                       function, sockshost2string(&bindreplydst, NULL, 0));

                  replyredirect = 1; /* must be using redirect() module. */
               }
               else
                  replyredirect = 0;
            }

            if (bindio.state.extension.bind || replyredirect) {
               /*
                * need to create a new socket to use for connecting
                * to the destination address; not sending the data over
                * the control-socket.
                */
               if ((sv[reply] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                  if (io.srule.log.error)
                     swarn("%s: socket(SOCK_STREAM)", function);

                  switch (errno) {
                     case EMFILE:
                     case ENFILE:
                        ++emfile;
                        /* FALLTHROUGH */

                     case ENOBUFS:
                        close(sv[remote]);
                        SHMEM_UNUSE(&bindio.srule,
                                    &cinfo,
                                    sockscf.shmemfd,
                                    SHMEM_ALL);
                        continue;
                  }

                  iostatus = IO_IOERROR;
                  break; /* errno is not ok. */
               }

               setsockoptions(sv[reply], AF_INET, SOCK_STREAM, 1);
               setconfsockoptions(sv[client],
                                  sv[reply],
                                  bindio.state.protocol,
                                  1,
                                  bindio.srule.socketoptionc,
                                  bindio.srule.socketoptionv,
                                  SOCKETOPT_PRE | SOCKETOPT_ANYTIME,
                                  SOCKETOPT_PRE | SOCKETOPT_ANYTIME);

               replyaddr = io.src.laddr; /* just to init. */
               SET_SOCKADDRPORT(&replyaddr, htons(0));

               if (socks_bind(sv[reply], &replyaddr, 0) != 0) {
                  log_bind_failed(function, SOCKS_TCP, &replyaddr);

                  iostatus = IO_IOERROR;
                  break;
               }

               bindio.dst.laddr = replyaddr;
               replydst.peer_isset = 1;
               sockaddr2sockshost(&bindio.dst.laddr, &replydst.peer);

               slog(LOG_DEBUG,
                    "%s: connecting to %s to forward bindreply from %s",
                    function,
                    sockshost2string(&bindreplydst, strhost, sizeof(strhost)),
                    sockshost2string(&bindio.src.host, NULL, 0));

               if (socks_connecthost(sv[reply],
                                     INTERNALIF,
                                     &bindreplydst,
                                     NULL,
                                     NULL,
                                     bindio.srule.timeout.connect ?
                                  (long)bindio.srule.timeout.connect : (long)-1,
                                     emsg,
                                     emsglen) != 0) {
                  iolog(&bindio.srule,
                        &bindio.state,
                        OPERATION_ERROR,
                        &replysrc,
                        &replydst,
                        NULL,
                        NULL,
                        emsg,
                        strlen(emsg));

                  /* log the close of the opened bind session also. */
                  iolog(&io.srule,
                        &io.state,
                        OPERATION_ERROR,
                        src,
                        dst,
                        NULL,
                        NULL,
                        emsg,
                        strlen(emsg));

                  iostatus = IO_IOERROR;
                  break;
               }

               setconfsockoptions(sv[client],
                                  sv[reply],
                                  bindio.state.protocol,
                                  1,
                                  bindio.srule.socketoptionc,
                                  bindio.srule.socketoptionv,
                                  SOCKETOPT_POST,
                                  SOCKETOPT_POST);

               if (replyredirect) {
                  close(sv[client]);
                  sv[client] = sv[reply];
                  sv[reply]  = -1;
               }
            }

            if (bindio.state.extension.bind) {
               /*
                * flushio() will close all descriptors set in io packet,
                * so dup what we need to keep going.
                */

               if ((bindio.control.s = dup(sv[childpipe])) == -1) {
                  switch (errno) {
                     case EMFILE:
                     case ENFILE:
                        if (bindio.srule.log.error)
                           swarn("%s: dup()", function);

                        ++emfile;
                        close(sv[remote]);
                        continue;

                     default:
                        SERR(bindio.control.s);
                  }
               }
            }

            if (bindio.state.extension.bind || replyredirect) {
               if (bindio.state.extension.bind)
                  bindio.dst.s = sv[reply];
               else /* replyredirect */
                  bindio.dst.s = sv[client];
            }
            else
               bindio.dst = io.src;

            bindio.src.s = sv[remote];

            bindio.src.state.isconnected = 1;
            bindio.dst.state.isconnected = 1;

            if (bindio.srule.mstats_shmid != 0
            && (bindio.srule.alarmsconfigured & ALARM_DISCONNECT))
               alarm_add_connect(&bindio.srule,
                                 ALARM_EXTERNAL,
                                 &cinfo,
                                 sockscf.shmemfd);

            if (bindio.state.extension.bind)
               /* add to list, client will query. */
               iolist = io_add(iolist, &bindio);
            else {
               response.host = bindio.src.host;

               if (send_response(sv[client], &response) != 0) {
                  close_iodescriptors(&io);
                  iostatus = IO_IOERROR;
               }
               else {
                  bindio.reqinfo.command
                  = (bindio.state.protocol == SOCKS_TCP ?
                                       SOCKD_FREESLOT_TCP : SOCKD_FREESLOT_UDP);

                  if (flushio(mother->s, &bindio) != 0)
                     return IO_ERROR;
               }

               /* flushio() closes these, not closev(). */
               sv[client] = sv[remote] = -1;
            }

            if (!bindio.state.extension.bind)
               break; /* only one connection to relay; flushed on to mother. */
         }

         if (bindio.state.extension.bind) {
            sockd_io_t *rmio;

            /* delete any connections we have queued. */
            while ((rmio = io_find(iolist, NULL)) != NULL) {
               close_iodescriptors(rmio);
               iolist = io_remove(iolist, rmio);
            }
         }

         /* not accepting any more connections on this socket. */
         close(io.dst.s);

         closev(ELEMENTS(sv), sv);
         break;
      }
#endif /* SOCKS_SERVER */

      case SOCKS_CONNECT: {
         rc = socks_connecthost(io.dst.s,
                                EXTERNALIF,
                                &io.dst.host,
                                &io.dst.laddr,
                                &io.dst.raddr,
                                (long)0, /* wait for completion in i/o child. */
                                emsg,
                                emsglen);

         if (rc == 0) {
            io.dst.state.isconnected = 1;

#if HAVE_NEGOTIATE_PHASE
            if (SOCKS_SERVER || io.reqflags.httpconnect) {
               errno = 0;
               if (send_connectresponse(request->s, errno, &io) != 0) {
                  *weclosedfirst = 0;
                  iostatus       = IO_IOERROR;

                  snprintf(emsg, emsglen,
                           "could not send connect response to client: %s",
                           strerror(errno));

                  close_iodescriptors(&io);
                  break;
               }
            }
#endif /* HAVE_NEGOTIATE_PHASE */
         }
         else {
            if (errno == EINPROGRESS)
               /*
                * we don't wait for the result but instead push the io object
                * on to i/o process.  This allows the connect(2)-time to
                * overlap with the sending of the io-object to the io-process.
                */
               rc = 0;
            else {
               rc       = -1;
               iostatus = IO_IOERROR;

#if HAVE_NEGOTIATE_PHASE

               SASSERTX(errno != 0);
               if (send_connectresponse(request->s, errno, &io) != 0)
                  slog(LOG_DEBUG,
                       "%s: could not send connect response to client: %s",
                       function, strerror(errno));

#else /* !HAVE_NEGOTIATE_PHASE */

               if (ERRNOISRST(errno))
                  sockd_rstonclose(io.dst.s); /* no other way to tell client. */

#endif /* !HAVE_NEGOTIATE_PHASE */
            }
         }

         if (rc != 0) {
            iolog(&io.srule,
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  strlen(emsg));

            close_iodescriptors(&io);
         }
         else {
            if (SHMEMRULE(&io)->mstats_shmid != 0
            && (SHMEMRULE(&io)->alarmsconfigured & ALARM_DISCONNECT))
               /*
                * Connect(2) in progress or already completed, so add it now.
                */
               alarm_add_connect(SHMEMRULE(&io),
                                 ALARM_EXTERNAL,
                                 &cinfo,
                                 sockscf.shmemfd);

            io.src.state.isconnected = 1;

            SASSERTX(io.state.proxychain.proxyprotocol == PROXY_DIRECT);
            io.reqinfo.command = (io.state.protocol == SOCKS_TCP ?
                                       SOCKD_FREESLOT_TCP : SOCKD_FREESLOT_UDP);

            if (flushio(mother->s, &io) != 0)
               return IO_ERROR;
         }

         break;
      }

#if HAVE_UDP_SUPPORT
      case SOCKS_UDPASSOCIATE: {
         /*
          * create socket we will receive datagrams from client on.
          * Let the created socket be of the same address family as
          * the client tells us it will send packets from.  Note that
          * even if the client sends us an all-zero address now, even
          * the all-zero address will have to have it's family
          * specified, as either ipv4 or ipv6.
          *
          * Exception is if the client does something so strange as to
          * send us a hostname.  In that case base ourselves on the type
          * of the (possibly first of several) addresses the hostname
          * resolved to.
          */
         socklen_t boundlen;
         int gaierr;
#if SOCKS_SERVER
         int triesleft;
#endif /* SOCKS_SERVER */

         sockshost2sockaddr2(&io.src.host,
                             &io.src.raddr,
                             &gaierr,
                             buf,
                             sizeof(buf));

         if (gaierr != 0) {
            SASSERTX(io.src.host.atype == SOCKS_ADDR_DOMAIN);

            log_resolvefailed(io.src.host.addr.domain, INTERNALIF, gaierr);

            msglen = snprintf(emsg, emsglen,
                              "client says it will send us UDP packets from "
                              "hostname \"%s\", but we are unable to resolve "
                              "that hostname",
                              sockshost2string(&io.src.host, NULL, 0));

            rc = SOCKS_HOSTUNREACH;
         }
         else
            rc = SOCKS_SUCCESS;

         if (rc == SOCKS_SUCCESS) {
            /*
             * Ok so far.  Now figure out what address to bind on the
             * internal side (io.src.laddr).
             */

            if (request->to.ss_family == io.src.raddr.ss_family)
               /*
                * Client connected to us on the same type of address it wants
                * to send us packets from, so should be ok to use the same
                * ipaddress for receiving udp packets as it tcp-connected to us
                * on.  The portnumber we will set later.
                */
               io.src.laddr = request->to;
            else {
                if (getinaddr(&io.src.laddr, &io.src.raddr, emsg, emsglen)
                == NULL) {
                  slog(LOG_DEBUG,
                       "%s: could not find %s on internal side to bind "
                       "for accepting UDP packets from client %s: %s",
                       function,
                       safamily2string(io.src.raddr.ss_family),
                       sockaddr2string2(&io.src.raddr, 0, NULL, 0),
                       emsg);

                     rc = SOCKS_ADDR_UNSUPP;
               }
            }
         }

         if (rc == SOCKS_SUCCESS) {
            SASSERTX(io.src.laddr.ss_family == AF_INET
            ||       io.src.laddr.ss_family == AF_INET6);

            /*
             * create socket of the same type as client will send us data
             * from.
             */
            if ((io.src.s = socket(io.src.laddr.ss_family, SOCK_DGRAM, 0))
            == -1) {
               msglen = snprintf(emsg, emsglen,
                                 "could not create socket(): %s",
                                 strerror(errno));

               swarnx("%s: %s", function, emsg);

               rc = SOCKS_FAILURE;
            }
         }

         if (rc != SOCKS_SUCCESS) {
            iolog(IORULE(&io),
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  msglen);

            send_failure(request->s, &response, rc);

            close_iodescriptors(&io);

            iostatus = IO_IOERROR;
            break;
         }

         SASSERTX(IPADDRISBOUND(&io.src.laddr));
         SASSERTX(safamily_issupported(io.src.laddr.ss_family));

         setsockoptions(io.src.s, io.src.laddr.ss_family, SOCK_DGRAM, 1);

         setconfsockoptions(io.src.s,
                            io.control.s,
                            io.state.protocol,
                            1,
                            io.srule.socketoptionc,
                            io.srule.socketoptionv,
                            SOCKETOPT_PRE | SOCKETOPT_ANYTIME,
                            SOCKETOPT_PRE | SOCKETOPT_ANYTIME);

#if BAREFOOTD
         if ((rc = socks_bind(io.src.s, &io.src.laddr, 1)) == 0)
            rc = SOCKS_SUCCESS;
         else
            rc = SOCKS_FAILURE;

#else /* SOCKS_SERVER */
         /*
          * bind client-side address for receiving UDP packets, so we can tell
          * the client where to send it's packets.
          * XXX should perhaps have a global udprange option also, for clients
          *     that don't know what address they will be sending from.  This
          *     rule is possibly not the one the client will finaly end up
          *     using, in which the udprange specification would not apply,
          *     but how can we know if the client doesn't tell us it's source
          *     address.
          * XXX add check for privileges on startup if range is privileged
          */
         if (io.srule.udprange.op == range)
            triesleft = MIN(10,
            ntohs(io.srule.udprange.end) - ntohs(io.srule.udprange.start) + 1);
         else
            triesleft = 1;

         do {
            if (io.srule.udprange.op == range) {
               SET_SOCKADDRPORT(&io.src.laddr,
                                htons(ntohs(io.srule.udprange.start)
                                + (random()
                                   % (ntohs(io.srule.udprange.end)
                                      - ntohs(io.srule.udprange.start) + 1))));

               slog(LOG_DEBUG,
                    "%s: random port selected for udp in range %u - %u: %u.  "
                    "Trying that first",
                    function,
                    ntohs(io.srule.udprange.start),
                    ntohs(io.srule.udprange.end),
                    ntohs(GET_SOCKADDRPORT(&io.src.laddr)));
            }
            else
               SET_SOCKADDRPORT(&io.src.laddr, htons(0));

            rc = socks_bind(io.src.s, &io.src.laddr, 0);
         } while (rc == -1 && (errno == EADDRINUSE || ERRNOISACCES(errno))
         && io.srule.udprange.op == range && --triesleft > 0);

         if (rc == 0)
            rc = SOCKS_SUCCESS;
        else
            rc = SOCKS_FAILURE;
#endif /* SOCKS_SERVER */

         if (rc != SOCKS_SUCCESS) {
            if (io.srule.udprange.op != none) {
               /*
                * Sigh.  No luck.  Will need to try every port in range.
                */

               slog(LOG_DEBUG,
                     "%s: failed to bind udp port in range %u - %u by "
                     "random selection.  Doing a sequential search now ...",
                     function,
                     ntohs(io.srule.udprange.start),
                     ntohs(io.srule.udprange.end));

               rc = socks_bindinrange(io.src.s,
                                      &io.src.laddr,
                                      io.srule.udprange.start,
                                      io.srule.udprange.end,
                                      io.srule.udprange.op);
               if (rc != 0) {
                  rc = SOCKS_FAILURE;
                  snprintf(emsg, emsglen, "no free ports in range to bind");
               }
            }
            else
               snprintf(emsg, emsglen, "could not bind local address %s: %s",
                       sockaddr2string(&io.src.laddr, strhost, sizeof(strhost)),
                        strerror(errno));
         }

         if (rc != SOCKS_SUCCESS) {
            iolog(IORULE(&io),
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  strlen(emsg));

            send_failure(request->s, &response, rc);
            close_iodescriptors(&io);

            iostatus = IO_IOERROR;

            break;
         }

         if (ADDRISBOUND(&io.src.raddr)) {
            /*
             * more efficient to be connected to the client.
             */

            slog(LOG_DEBUG, "%s: connecting to udp client at address %s",
                 function, sockaddr2string(&io.src.raddr, NULL, 0));

            if (socks_connecthost(io.src.s,
                                  INTERNALIF,
                                  sockaddr2sockshost(&io.src.raddr, NULL),
                                  NULL,
                                  NULL,
                                  (long)-1,
                                  emsg,
                                  emsglen) != 0) {
               iolog(IORULE(&io),
                     &io.state,
                     OPERATION_ERROR,
                     src,
                     dst,
                     NULL,
                     NULL,
                     emsg,
                     strlen(emsg));

               send_failure(request->s, &response, SOCKS_HOSTUNREACH);

               close_iodescriptors(&io);

               iostatus = IO_IOERROR;
               break;
            }

            io.src.state.isconnected = 1;
         }

         io.dst.state.isconnected = 0;

         boundlen = sizeof(io.src.laddr);
         if (getsockname(io.src.s, TOSA(&io.src.laddr), &boundlen) != 0) {
            msglen = snprintf(emsg, emsglen,
                              "getsockname() failed: %s", strerror(errno));

            iolog(IORULE(&io),
                  &io.state,
                  OPERATION_ERROR,
                  src,
                  dst,
                  NULL,
                  NULL,
                  emsg,
                  msglen);

            send_failure(request->s, &response, SOCKS_FAILURE);
            close_iodescriptors(&io);

            iostatus = IO_IOERROR;
            break;
         }

         slog(LOG_DEBUG, "%s: address bound on client side for udp: %s",
              function, sockaddr2string(&io.src.laddr, NULL, 0));

         /* external side will be bound in i/o process. */

         if (request->req.flag & SOCKS_USECLIENTPORT
         &&  sockscf.compat.draft_5_05)
            if (GET_SOCKADDRPORT(&io.src.raddr)
            ==  GET_SOCKADDRPORT(&io.dst.laddr))
               response.flag |= SOCKS_USECLIENTPORT;

         sockaddr2sockshost(&io.src.laddr, &response.host);

         if (send_response(request->s, &response) == 0) {
            io.reqinfo.command = (io.state.protocol == SOCKS_TCP ?
                                       SOCKD_FREESLOT_TCP : SOCKD_FREESLOT_UDP);
            if (flushio(mother->s, &io) != 0)
               return IO_ERROR;
         }
         else {
            close_iodescriptors(&io);
            *weclosedfirst = 0;
            iostatus       = IO_IOERROR;
         }

         break;
      }
#endif /* HAVE_UDP_SUPPORT */

      default:
         SERRX(io.state.command);
   }

   rc = errno;
   SASSERTX(close(request->s) == -1);
   errno = rc;

   return iostatus;
}

static int
reqhostisok(host, cmd, emsg, emsglen)
   sockshost_t *host;
   const int cmd;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "reqhostisok()";
   struct sockaddr_storage addr;
   int gaierr, musthaveaddr, musthaveport;

   slog(LOG_DEBUG, "%s: host %s, command %s",
        function, sockshost2string(host, NULL, 0), command2string(cmd));

   if (host->atype == SOCKS_ADDR_IPV6
   && IN6_IS_ADDR_V4MAPPED(&host->addr.ipv6.ip)) {
      /*
       * We never use that.  Either we have IPv4 available on the interface,
       * in which case we create a regular IPv4 socket to use, or we do not,
       * in which case the request will not be performed.
       *
       * We do not mix IPv4 and IPv6 traffic on the same socket, even though
       * that is supported by some systems.  Instead we convert the address
       * to an IPv4 address and proceed as usual.
       */
      sockshost_t convertedhost;

      convertedhost = *host;
      ipv4_mapped_to_regular(&host->addr.ipv6.ip, &convertedhost.addr.ipv4);

      host->atype     = SOCKS_ADDR_IPV4;
      host->addr.ipv4 = convertedhost.addr.ipv4;
   }

   sockshost2sockaddr2(host, &addr, &gaierr, emsg, emsglen);

   if (host->atype == SOCKS_ADDR_DOMAIN) {
      if (addr.ss_family == AF_UNSPEC) {
         if (gaierr != 0)
            log_resolvefailed(host->addr.domain,
                              cmd == SOCKS_UDPASSOCIATE ?
                                                        INTERNALIF : EXTERNALIF,
                              gaierr);

         return SOCKS_HOSTUNREACH;
      }

      SASSERTX(gaierr == 0);

      slog(LOG_DEBUG, "%s: hostname %s in %s-request resolved to address %s",
           function,
           sockshost2string(host, NULL, 0),
           command2string(cmd),
           sockaddr2string(&addr, NULL, 0));
   }

   switch (cmd) {
      case SOCKS_BIND:
         if (host->atype            == SOCKS_ADDR_IPV4
         &&  host->addr.ipv4.s_addr == htonl(BINDEXTENSION_IPADDR))
            break;

         /* FALLTHROUGH */

      case SOCKS_CONNECT:
         if (host->atype == SOCKS_ADDR_IPV4
         ||  host->atype == SOCKS_ADDR_IPV6) {
            if (!external_has_global_safamily(atype2safamily(host->atype))) {
               snprintf(emsg, emsglen,
                        "connect to %s requested, but no %s configured "
                        "for our usage on the external interface",
                        atype2string(host->atype),
                        atype2string(host->atype));

               return SOCKS_ADDR_UNSUPP;
            }
         }

         break;

      case SOCKS_UDPASSOCIATE:
         if (host->atype == SOCKS_ADDR_IPV4
         ||  host->atype == SOCKS_ADDR_IPV6) {
            if (!internal_has_safamily(atype2safamily(host->atype))) {
               /*
                * If client insists on using an address-type we can not
                * accept packets from things will fail later of course,
                * but that is up to client.  If the client has provided a
                * real IP-address (rather than zero) here however, we can
                * abort things now as we cannot receive packets from that
                * address and our check for whether the client is sending
                * from the address it told us (not zero) will fail, if
                * we ever get that far.
                */

               snprintf(emsg, emsglen,
                        "client wants to send us UDP packets from %s, but no "
                        "%s configured for our usage on the internal interface",
                        atype2string(host->atype),
                        atype2string(host->atype));

               slog(LOG_DEBUG, "%s: %s", function, emsg);

               if (SOCKSHOST_ADDRISBOUND(host))
                  return SOCKS_ADDR_UNSUPP;
            }
         }

         break;

      default:
         SERRX(cmd);
   }

   switch (cmd) {
      case SOCKS_BIND:
         musthaveaddr = 0;
         musthaveport = 0;
         break;

      case SOCKS_CONNECT:
         musthaveaddr = 1;
         musthaveport = 1;
         break;

      case SOCKS_UDPASSOCIATE:
         musthaveaddr = 0;
         musthaveport = 0;
         break;

      default:
         SERRX(cmd);
   }

   if (musthaveaddr && !IPADDRISBOUND(&addr)) {
      snprintf(emsg, emsglen, "unbound ipaddress (%s) in request",
               sockaddr2string(&addr, NULL, 0));

      return SOCKS_ADDR_UNSUPP;
   }

   if (musthaveport && GET_SOCKADDRPORT(&addr) == htons(0)) {
      snprintf(emsg, emsglen, "unbound port (%s) in request",
               sockaddr2string(&addr, NULL, 0));

      return SOCKS_ADDR_UNSUPP;
   }

   /*
    * Anything special we need to check for this command or address?
    */
   switch (cmd) {
      case SOCKS_CONNECT:
         if (addrindex_on_listenlist(sockscf.internal.addrc,
                                     sockscf.internal.addrv,
                                     &addr,
                                     SOCKS_TCP /* only listen on TCP. */)
         != -1) {
            snprintf(emsg, emsglen,
                     "client is requesting a connection to the same address "
                     "we listen to new clients on: %s.  Very suspicious.   "
                     "For security reasons that is not allowed, as otherwise "
                     "one client could trivially use up all our resources "
                     "by going into a loop sending a stream of requests that "
                     "all asked us to connect to ourselves",
                     sockaddr2string(&addr, NULL, 0));

            return SOCKS_NOTALLOWED;
         }
         break;

      case SOCKS_UDPASSOCIATE: {
         break;

#if 0
         /*
          * Not a problem in Dante due to either one of:
          *
          * a) Dante does not bind any address on the external side
          *    until it has received the first UDP packet from the client.
          *
          * b) Dante does not listen for replies from the address it
          *    bound on the external side until is has forwarded at
          *    least one packet from the client.
          */
         int isoninternal = 0, isonexternal = 0;

         /* don't care about port when matching IP-address here. */
         SET_SOCKADDRPORT(&addr, htons(0));

         if (addrindex_on_listenlist(sockscf.internal.addrc,
                                     sockscf.internal.addrv,
                                     &addr,
                                     SOCKS_TCP /* only listen on TCP. */) != -1)
            isoninternal = 1;
         else if (addrindex_on_externallist(&sockscf.external, &addr) != -1)
            isonexternal = 1;

         if (isoninternal || isonexternal) {
            /*
             * We don't allow udpassociate requests that claim to come
             * from the same ipaddresses we listen to ourselves.
             * This is for safety reasons, as otherwise it's too easy
             * to create dos-problems by e.g. making us loop udp packets
             * internally forever.  Could happen like this:
             *
             * 1) Client sends udpassociate request with address ipaddrX.portY,
             *    indicating it will send Dante packets from this address,
             *    and also indicating Dante should send udp-replies to that
             *    address.
             *
             * 2) Dante accepts the request, and binds a random port on the,
             *    the external side.  This random port happens to be portY.
             *    So Dante has now bound ipaddrX.portY on the external side,
             *    planing to use it to forward packets from the socks client
             *    to the socksclient's target.
             *
             * 3) Dante receives an udp packet ("reply") on ipaddrX.portY.
             *    This packet Dante is supposed to send to the client,
             *    so it encapsulates it with an UDP header, and sends it to
             *    the address the client specified in 1), which is also
             *    the same address Dante bound in 2).
             *
             * 4) Dante receives a new udp packet, same way as in 3.  But
             *    this packet it packet which Dante sent.  Again Dante
             *    adds a socks header, and forwards it to what it thinks
             *    is the client address.
             *
             * 5) Goto 3.
             *
             * But not a problem in Dante due to a) or b).
             */
            snprintf(emsg, emsglen,
                     "local client claims to be listening on the same "
                     "IP-address as we use on the %s side (%s).  "
                     "For security reasons that is not allowed",
                     isoninternal ? "internal" : "external",
                     sockaddr2string(&addr, NULL, 0));

            return SOCKS_NOTALLOWED;
         }
#endif /* 0 no (longer) a problem i Dante. */

         break;
      }

      default:
         break;
   }

   return SOCKS_SUCCESS;
}


static int
flushio(mother, io)
   int mother;
   sockd_io_t *io;
{
   const char *function = "flushio()";
   int dolog;

   SASSERTX(io->allocated == 0);

   switch (io->state.command) {
      case SOCKS_CONNECT:
         SASSERTX(io->control.s == -1);

         SASSERTX(io->src.state.isconnected);
         SASSERTX(io->src.s     != -1);
         SASSERTX(io->dst.s     != -1);
         break;

      case SOCKS_BINDREPLY:
         if (io->state.extension.bind)
            SASSERTX(io->control.s != -1);
         else
            SASSERTX(io->control.s == -1);

         SASSERTX(io->src.s != -1);
         SASSERTX(io->dst.s != -1);

         SASSERTX(io->src.state.isconnected);
         SASSERTX(io->dst.state.isconnected);
         break;

      case SOCKS_UDPASSOCIATE:
         SASSERTX(!io->dst.state.isconnected);
         SASSERTX(io->dst.s == -1);

#if HAVE_CONTROL_CONNECTION
         SASSERTX(io->control.s != -1);
         SASSERTX(io->control.state.isconnected);

         SASSERTX(io->src.s     != -1);

#else /* !HAVE_CONTROL_CONNECTION */
         SASSERTX(io->control.s == -1);

#endif /* HAVE_CONTROL_CONNECTION */

         break;

      default:
         SERRX(io->state.command);
   }

#if HAVE_GSSAPI
   if (CONTROLIO(io)->auth.method == AUTHMETHOD_GSSAPI) {
      OM_uint32 minor_status, major_status, maxlen;
      char emsg[1024];

      major_status
      = gss_wrap_size_limit(&minor_status,
                            CONTROLIO(io)->auth.mdata.gssapi.state.id,
                            CONTROLIO(io)->auth.mdata.gssapi.state.protection
                            == GSSAPI_CONFIDENTIALITY ?
                                 GSS_REQ_CONF : GSS_REQ_INT,
                            GSS_C_QOP_DEFAULT,
                            (OM_uint32)(MAXGSSAPITOKENLEN - GSSAPI_HLEN),
                            &maxlen);

      if (gss_err_isset(major_status, minor_status, emsg, sizeof(emsg)))
         serrx("%s: gss_wrap_size_limit() failed: %s", function, emsg);

      if (maxlen == 0)
         serrx("%s: for a token of length %d, gss_wrap_size_limit() returned "
               "%d.  The kerberos library might not fully support the "
               "configured encoding type",
               function, MAXGSSAPITOKENLEN - GSSAPI_HLEN, maxlen);

      if (sockscf.option.debug >= DEBUG_VERBOSE)
         slog(LOG_DEBUG, "%s: gss_wrap_size_limit() for fd %d is %lu",
              function, CONTROLIO(io)->s, (unsigned long)maxlen);

      CONTROLIO(io)->auth.mdata.gssapi.state.maxgssdata = maxlen;

      if (io->src.auth.method == AUTHMETHOD_GSSAPI)
         io->src.auth.mdata.gssapi.state.maxgssdata = maxlen;

      if (io->dst.auth.method == AUTHMETHOD_GSSAPI)
         io->dst.auth.mdata.gssapi.state.maxgssdata = maxlen;
   }
#endif /* HAVE_GSSAPI */

   if (io->state.command == SOCKS_CONNECT) {
      /*
       * Need to save all the socketoptions that we can not set now but
       * with which we must wait until the connection has been fully
       * established.  The i/o child will have to set them if the connect(2)
       * completes successfully.
       */
      size_t i;

      for (i = 0; i < io->srule.socketoptionc; ++i) {
         if (!io->srule.socketoptionv[i].isinternalside
         &&   (io->srule.socketoptionv[i].info == NULL
           ||  io->srule.socketoptionv[i].info->calltype == postonly)) {
            if (io->srule.socketoptionc >= ELEMENTS(io->extsocketoptionv)) {
               swarnx("%s: one or more socket options from socks-rule #%lu "
                      "could not be set on fd %d because the hardcoded "
                      "limit for number of options that can be set on the "
                      "external side is %lu",
                      function,
                      (unsigned long)io->srule.number,
                      io->dst.s,
                      (unsigned long)ELEMENTS(io->extsocketoptionv));

               break;
            }

            io->extsocketoptionv[io->extsocketoptionc++]
            = io->srule.socketoptionv[i];
         }
      }
   }

   gettimeofday_monotonic(&io->state.time.requestend);

   if (io->state.command                  == SOCKS_CONNECT
   &&  io->state.proxychain.proxyprotocol == PROXY_DIRECT)
         ; /* not established yet. */
   else
      gettimeofday_monotonic(&io->state.time.established);

   /* only the shemid's should be set, not the memory. */
   SASSERTX(io->crule.bw == NULL);
   SASSERTX(io->crule.ss == NULL);
   SASSERTX(io->srule.bw == NULL);
   SASSERTX(io->srule.ss == NULL);

   slog(LOG_DEBUG,
        "%s: io->control.s = fd %d, io->src.s = fd %d, io->dst.s = fd %d",
        function, io->control.s, io->src.s, io->dst.s);

   if (io->state.command == SOCKS_UDPASSOCIATE)
#if BAREFOOTD
      dolog = 0; /* log when we get an actual client, not just the setup. */
#else /* Dante */
      dolog = 1;
#endif /* Dante */

   else if (io->src.state.isconnected && io->dst.state.isconnected)
      dolog = 1;
   else
      dolog = 0; /* don't know status yet.  I/O child will have to log it. */

   if (dolog) {
      iologaddr_t src, dst, proxy;

      if (io->state.protocol == SOCKS_TCP
      &&  io->srule.log.tcpinfo
      &&  io->dst.state.isconnected) {
         int fdv[] = { CLIENTIO(io)->s, EXTERNALIO(io)->s };

         io->state.tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
      }

      initlogaddrs(io,
                   &src,
                   io->state.protocol == SOCKS_TCP ? &dst : NULL,
                   &proxy);

      iolog(&io->srule,
            &io->state,
            OPERATION_CONNECT,
            &src,
            io->state.protocol == SOCKS_TCP ? &dst : NULL,
            NULL,
            io->state.proxychain.proxyprotocol == PROXY_DIRECT ? NULL : &proxy,
            NULL,
            0);

      io->state.tcpinfo = NULL;
   }

   if (send_io(mother, io) != 0) {
#if HAVE_NEGOTIATE_PHASE
      response_t response;
#endif /* HAVE_NEGOTIATE_PHASE */
      iologaddr_t src, dst, proxy;

      if (sockd_motherexists())
        swarn("%s: sending io object with local client %s to mother failed",
              function,
             sockaddr2string(&CONTROLIO(io)->raddr, NULL, 0));

#if HAVE_NEGOTIATE_PHASE
      if (io->state.proxychain.proxyprotocol == PROXY_DIRECT) {
         create_response(NULL,
                         &CONTROLIO(io)->auth,
                         io->state.proxyprotocol,
                         (int)errno2reply(errno, io->state.proxyprotocol),
                         &response);

         if (send_response(CONTROLIO(io)->s, &response) != 0) {
            slog(LOG_DEBUG,
                 "%s: send_response to client %s on fd %d failed: %s",
                 function,
                 sockshost2string(&CONTROLIO(io)->host, NULL, 0),
                 CONTROLIO(io)->s,
                 strerror(errno));
         }
      }
      /* else; success response should have been send already. */
#endif /* HAVE_NEGOTIATE_PHASE */

      initlogaddrs(io,
                   &src,
                   io->state.protocol == SOCKS_TCP ? &dst : NULL,
                   &proxy);

      iolog(&io->srule,
            &io->state,
            OPERATION_ERROR,
            &src,
            io->state.protocol == SOCKS_TCP ? &dst : NULL,
            NULL,
            io->state.proxychain.proxyprotocol == PROXY_DIRECT ? NULL : &proxy,
            NULL,
            0);

      close_iodescriptors(io);
      return -1;
   }

   close_iodescriptors(io);
   return 0;
}

static void
proctitleupdate(from)
   const struct sockaddr_storage *from;
{
   setproctitle("%s: %s",
                childtype2string(sockscf.state.type),
                from == NULL ?  "0/1" : "1/1");
}

static route_t *
getroute(client, req, emsg, emsglen)
   const struct sockaddr_storage *client;
   request_t *req;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "getroute()";
   const int originalreqversion = req->version;
   static route_t routemem;
   authmethod_t auth;
   route_t *route;

   slog(LOG_DEBUG, "%s: request: %s, authmethod %d",
        function, socks_packet2string(req, 1), req->auth->method);

   bzero(&routemem, sizeof(routemem));

   if (sockscf.route == NULL) {
      slog(LOG_DEBUG, "%s: no routes, faking direct route", function);

      routemem.gw.state.proxyprotocol.direct = 1;
      return &routemem;
   }

   /*
    * We can reuse the authentication the client provided to us when
    * authentication to an upstream proxy, unless the operator has
    * explicitly set the methods supported by this route to "none".
    *
    * In some cases, we need to convert the method from a non-standard
    * method to a standard socks method however.
    */

   auth2standard(req->auth, &auth);
   *req->auth = auth;

   switch (req->command) {
      case SOCKS_BIND:
      case SOCKS_CONNECT:
         break;

      case SOCKS_UDPASSOCIATE:
         /*
          * Client should send us the address it will send us
          * udp packets from.  Not related to target address,
          * so bzero it (INADDR_ANY) as the address given here
          * is nothing we can use in relation to a route-lookup.
          */
         bzero(&req->host.addr, sizeof(req->host.addr));
         req->host.port = htons(0);

         switch (req->host.atype) {
            case SOCKS_ADDR_IPV4:
            case SOCKS_ADDR_IPV6:
               break;

            case SOCKS_ADDR_DOMAIN:
               req->host.atype = SOCKS_ADDR_IPV4; /* any ipaddress type */
               break;

            default:
               SERRX(req->host.atype);
         }
         break;

      default:
         SERRX(req->command);
   }

   /* best if we can find a direct route, so look for that first. */
   req->version = PROXY_DIRECT;

   route = socks_requestpolish(req,
                               sockaddr2sockshost(client, NULL),
                               &req->host);
   if (route == NULL) {
      if (req->command    == SOCKS_CONNECT
      &&  req->host.atype == SOCKS_ADDR_DOMAIN) {
         /*
          * Possibly there is a route supporting an ipaddress destination,
          * even if there was no route supporting the hostname destination
          * (e.g., there is only socks v4 route).  Therefor try resolving the
          * destination locally before giving up on finding a route.
          *
          * We will need to resolve the destination sooner or later
          * anyway, so if it's not already in our hostcache, there should
          * not be a big penalty incurred by adding it now before doing
          * a route lookup again.
          */
         struct sockaddr_storage saddr;
         int gaierr;

         slog(LOG_DEBUG,
              "%s: no hostname-route for destination %s found.  Trying to "
              "resolve and do route lookup again",
              function, sockshost2string(&req->host, NULL, 0));

         sockshost2sockaddr2(&req->host, &saddr, &gaierr, emsg, emsglen);

         if (gaierr != 0) {
            log_resolvefailed(req->host.addr.domain, EXTERNALIF, gaierr);
            return NULL;
         }

         /*
          * Retry the request using the original request data, but
          * change the hostname to the resolved address.
          */

         sockaddr2sockshost(&saddr, &req->host);

         if (req->version   == PROXY_SOCKS_V4
         &&  saddr.ss_family != AF_INET)
            /*
             * v4 only supports ipv4, so no choice but changing it.
             */
            req->version = PROXY_SOCKS_V5;
         else
            req->version = originalreqversion;

         return getroute(client, req, emsg, emsglen);
      }
      else {
         if (req->command != SOCKS_CONNECT)
            snprintf(emsg, emsglen,
                     "command %s is not supported by serverchaining and no "
                     "direct route found",
                     command2string(req->command));
         else
            snprintf(emsg, emsglen,
                     "no usable serverchain route to target found");

         return NULL;
      }
   }

   SASSERTX(route != NULL);
   routemem = *route;

   return &routemem;
}

static int
serverchain(targetsocket, clientsocket, client, req,
            proxychain, proxychainauth, emsg, emsglen)
   const int targetsocket;
   const int clientsocket;
   const struct sockaddr_storage *client;
   const request_t *req;
   proxychaininfo_t *proxychain;
   authmethod_t *proxychainauth;
   char *emsg;
   size_t emsglen;
{
   const char *function = "serverchain()";
   response_t response;
   route_t *route;
   socks_t packet;
   char lemsg[512];
   int rc, flags;

   slog(LOG_DEBUG, "%s: client %s, auth %s, request %s",
        function,
        sockaddr2string(client, NULL, 0),
        method2string(req->auth->method),
        socks_packet2string(req, 1));

   bzero(&packet, sizeof(packet));
   packet.state.auth = *req->auth;
   packet.req        = *req;

   packet.req.auth = &packet.state.auth;
   packet.res.auth = &packet.state.auth;

   if ((route = getroute(client, &packet.req, emsg, emsglen)) == NULL)
      return -1;

   if (route->gw.state.proxyprotocol.direct) {
      slog(LOG_DEBUG, "%s: using direct system calls for fd %d",
           function, targetsocket);

      proxychain->proxyprotocol = PROXY_DIRECT;
      return 0;
   }

   if (socks_routesetup(targetsocket, targetsocket, route, emsg, emsglen) != 0){
      swarnx("%s: socks_routesetup() failed: %s", function, emsg);
      return -1;
   }

   if ((route = socks_connectroute(targetsocket,
                                   &packet,
                                   sockaddr2sockshost(client, NULL),
                                   &packet.req.host,
                                   lemsg,
                                   sizeof(lemsg))) == NULL) {
      snprintf(emsg, emsglen,
               "could not connect to upstream proxyserver: %s", lemsg);

      return -1;
   }

   proxychain->proxyprotocol = packet.req.version;

   /*
    * we're not interested the extra hassle of negotiating over a
    * non-blocking socket so set it to blocking while we use it.
    */
   if ((flags = setblocking(targetsocket, "server-chaining")) == -1)
      return -1;

   rc = socks_negotiate(targetsocket,
                        targetsocket,
                        &packet,
                        route,
                        emsg,
                        emsglen);

   if (fcntl(targetsocket, F_SETFL, flags) == -1)
      swarn("%s: fcntl(2) failed to restore flags on fd %d to %d",
            function, targetsocket, flags);

   if (rc != 0) {
      slog(LOG_DEBUG, "%s: socks_negotiate() failed: %s",
           function, strerror(errno));

      return -1;
   }

   /* Use request-version as response-version differs in v4 (is 0 there). */
   proxychain->proxyprotocol = (int)packet.req.version;

   switch (packet.req.command) {
      case SOCKS_CONNECT:
         proxychain->extaddr = packet.res.host;
         break;

      default:
         SERRX(packet.req.command);
   }

   *proxychainauth = *packet.req.auth;

   convertresponse(&packet.res, &response, req->version);

   response.auth   = req->auth; /* must use the auth negotiated with client. */

   if (send_response(clientsocket, &response) != 0)
      return -1;

   slog(LOG_DEBUG,
        "%s: external address used by upstream proxy is %s.  "
        "Authmethod upstream is %s, authmethod downstream is %s",
        function,
        sockshost2string(&proxychain->extaddr, NULL, 0),
        method2string(proxychainauth->method),
        method2string(response.auth->method));


   return 0;
}

static void
convertresponse(oldres, newres, newversion)
   const response_t *oldres;
   response_t *newres;
   const int newversion;
{
   const char *function = "convertresponse()";
   int genericreply;

   if (oldres->version == newversion
   || (   oldres->version == PROXY_SOCKS_V4REPLY_VERSION
       && newversion      == PROXY_SOCKS_V4)) {
      *newres = *oldres;
      return;
   }

   /*
    * first convert the genericreply code from whatever old version to the
    * corresponding socks v5 generic reply code.  Then convert from the
    * v5 replycode to whatever new version.
    */
   switch (oldres->version) {
      case PROXY_HTTP_10:
      case PROXY_HTTP_11:
         switch (oldres->reply.http) {
            case HTTP_SUCCESS:
               genericreply = SOCKS_SUCCESS;
               break;

            case HTTP_NOTALLOWED:
            case HTTP_FORBIDDEN:
            case HTTP_PROXYAUTHREQUIRED:
               genericreply = SOCKS_NOTALLOWED;
               break;

            case HTTP_HOSTUNREACH:
               genericreply = SOCKS_HOSTUNREACH;
               break;

            default:
               genericreply = SOCKS_FAILURE;
               break;
         }
         break;

      case PROXY_UPNP:
         switch (oldres->reply.upnp) {
            case UPNP_SUCCESS:
               genericreply = SOCKS_SUCCESS;
               break;

            default:
               genericreply = SOCKS_FAILURE;
               break;
         }
         break;

      case PROXY_SOCKS_V4REPLY_VERSION:
         switch (oldres->reply.socks) {
            case SOCKSV4_SUCCESS:
               genericreply = SOCKS_SUCCESS;
               break;

            case SOCKSV4_NO_IDENTD:
            case SOCKSV4_BAD_ID:
               genericreply = SOCKS_NOTALLOWED;
               break;

            default:
               genericreply = SOCKS_FAILURE;
               break;
         }
         break;

      case PROXY_SOCKS_V5: /* default; what we use as the generic replycode. */
         genericreply = oldres->reply.socks;
         break;

      default:
         swarnx("%s: unknown proxy protocol: %d", function, oldres->version);
         genericreply = SOCKS_FAILURE;
   }

   switch (newversion) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V4REPLY_VERSION:
         /*
          * Socks v4 only supports IPv4.
          */
         switch (oldres->host.atype) {
            case SOCKS_ADDR_IPV4:
               newres->host = oldres->host;
               break;

            case SOCKS_ADDR_IPV6:
               slog(LOG_DEBUG,
                    "%s: IPv6 is not supported by by SOCKS v4, so changing "
                    "address in response from %s to zero",
                    function,
                    sockshost2string(&oldres->host, NULL, 0));

               newres->host.atype = SOCKS_ADDR_IPV4;
               bzero(&newres->host.addr, sizeof(newres->host.addr));      
               break;

            default: {
               struct sockaddr_storage addr;

               slog(LOG_DEBUG,
                    "%s: SOCKS v4 only supports %s, so need to convert this "
                    "%s to %s",
                    function,
                    atype2string(SOCKS_ADDR_IPV4),
                    atype2string(oldres->host.atype),
                    atype2string(SOCKS_ADDR_IPV4));

               sockshost2sockaddr(&oldres->host, &addr);

               if (IPADDRISBOUND(&addr)) {
                  sockaddr2sockshost(&addr, &newres->host);

                  if (addr.ss_family == AF_INET)
                     sockaddr2sockshost(&addr, &newres->host);
                  else {
                     slog(LOG_DEBUG,
                          "%s: hostname %s did not resolve to IPv4 (resolved "
                          "to %s).  Setting address in response to zero "
                          "instead",
                          function,
                          sockshost2string(&oldres->host, NULL, 0),
                          sockaddr2string(&addr,          NULL, 0));

                     newres->host.atype = SOCKS_ADDR_IPV4;
                     bzero(&newres->host.addr, sizeof(newres->host.addr));
                  }
               }
               else {
                  swarnx("%s: can not resolve hostname %s",
                         function, sockshost2string(&oldres->host, NULL, 0));

                  genericreply = SOCKS_FAILURE;
               }
            }
         }

         newres->version = PROXY_SOCKS_V4REPLY_VERSION;
         break;

      case PROXY_SOCKS_V5:
         /*
          * only flagbits in v5, and old version was obviously not that,
          * so we have no flagbits to copy.
         */
         newres->flag    = 0;

         newres->host    = oldres->host;
         newres->version = newversion;
         break;

      default:
         SERRX(newversion);
   }

   socks_set_responsevalue(newres, sockscode(newversion, genericreply));

   slog(LOG_DEBUG,
        "%s: converted from proxy protocol version %d (%s) to %d (%s).  "
        "Old response value was %d, new is %d",
        function,
        oldres->version,
        proxyprotocol2string(oldres->version == PROXY_SOCKS_V4REPLY_VERSION ? 
                             PROXY_SOCKS_V4 : oldres->version),
        newres->version,
        proxyprotocol2string(newres->version == PROXY_SOCKS_V4REPLY_VERSION ? 
                             PROXY_SOCKS_V4 : newres->version),
        socks_get_responsevalue(oldres),
        socks_get_responsevalue(newres));
}

static void
init_req(req, request)
   struct req *req;
   const sockd_request_t *request;
{

   req->starttime        = time_monotonic(NULL);

   req->crule            = request->crule;
#if HAVE_SOCKS_HOSTID
   req->hrule            = request->hrule;
   req->hrule_isset      = request->hrule_isset;
#endif /* HAVE_SOCKS_HOSTID */

   req->client           = request->from;
   req->request_isvalid  = 1;
   req->request          = request->req;
   req->s                = request->s;

#if HAVE_NEGOTIATE_PHASE
   socks_allocbuffer(req->s, SOCK_STREAM);
#endif /* HAVE_NEGOTIATE_PHASE */

   req->allocated        = 1;
}

static void
delete_req(req)
   struct req *req;
{

#if HAVE_NEGOTIATE_PHASE
   socks_freebuffer(req->s);
#endif /* HAVE_NEGOTIATE_PHASE */

   req->allocated = req->request_isvalid = 0;
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

   slog(LOG_INFO, "request-child up %lu day%s, %lu:%.2lu:%.2lu",
                  days, days == 1 ? "" : "s", hours, minutes, seconds);

   for (i = 0; i < reqc; ++i) {
      char *tcpinfo, reqinfo[64];

      if (!reqv[i].allocated)
         continue;

      if (CRULE_OR_HRULE(&reqv[i])->log.tcpinfo) {
         int fdv[] = { reqv[i].s };

         tcpinfo = get_tcpinfo(ELEMENTS(fdv), fdv, NULL, 0);
      }
      else
         tcpinfo = NULL;

      if (reqv[i].request_isvalid)
         snprintf(reqinfo, sizeof(reqinfo),
                  "%s %s-request",
                  protocol2string(reqv[i].request.protocol),
                  command2string(reqv[i].request.command));
      else
         snprintf(reqinfo, sizeof(reqinfo), "request");

      slog(LOG_INFO,
           "%s: %s in progress for %lds"
           "%s%s%s",
           sockaddr2string(&reqv[i].client, NULL, 0),
           reqinfo,
           (long)socks_difftime(tnow, reqv[i].starttime),
           tcpinfo == NULL ? "" : "\n",
           tcpinfo == NULL ? "" : "   TCP_INFO:\n",
           tcpinfo == NULL ? "" : tcpinfo);
   }

   SIGNAL_EPILOGUE(sig, si, errno_s);
}

static void
initlogaddrs(io, src, dst, proxy)
      const sockd_io_t *io;
      iologaddr_t *src;
      iologaddr_t *dst;
      iologaddr_t *proxy;
{

   if (src != NULL)
      init_iologaddr(src,
                     object_sockaddr,
                     &io->src.laddr,
                     object_sockshost,
                     &io->src.host,
                     &io->src.auth,
                     &io->state.hostid);

   if (dst != NULL)
      init_iologaddr(dst,
                     object_sockaddr,
                     &io->dst.laddr,
                     object_sockshost,
                     &io->dst.host,
                     io->state.proxychain.proxyprotocol == PROXY_DIRECT ?
                        &io->dst.auth : NULL,
                     NULL);

   if (proxy != NULL && io->state.proxychain.proxyprotocol != PROXY_DIRECT)
      init_iologaddr(proxy,
                     object_sockaddr,
                     &io->dst.raddr,
                     object_sockshost,
                     &io->state.proxychain.extaddr,
                     &io->dst.auth,
                     NULL);
}

static int
bindexternaladdr(io, _req, emsg, emsglen)
   sockd_io_t *io;
   const request_t *_req;
   char *emsg;
   const size_t emsglen;
{
   const char *function = "bindexternaladdr()";
   const sockshost_t *target;
   request_t req;
   authmethod_t auth;
   route_t *route;
   int rc;


   slog(LOG_DEBUG, "%s: request: %s, authmethod %d",
        function, socks_packet2string(_req, 1), _req->auth->method);

   /*
    * Don't let getroute() modify our data.
    */
   auth     = *_req->auth;
   req      = *_req;
   req.auth = &auth;

   /*
    * If we have a non-direct route we need to use for the request,
    * the address we should bind is an address we can connect to the
    * upstream proxyserver from.  I.e., in this context, the target is
    * the proxyserver, not the address given by the client.
    */

   if ((route = getroute(&io->src.raddr, &req, emsg, emsglen)) == NULL)
      return -1;

   if (route->gw.state.proxyprotocol.direct)
      target = &req.host;
   else
      target = &route->gw.addr;

   /*
    * Find address to bind for client.  First the ipaddress.
    */
   if (getoutaddr(&io->dst.laddr,
                  &io->src.laddr,
                  &io->src.raddr,
                  req.command,
                  target,
                  emsg,
                  emsglen) == NULL)
      return -1;

   if (PORTISRESERVED(GET_SOCKADDRPORT(&io->dst.laddr))
   && !sockscf.compat.sameport) {
      slog(LOG_DEBUG,
           "%s: would normally try to bind the privileged port %u, but "
           "\"compatibility: sameport\" is not set, so just binding an porty",
           function, ntohs(GET_SOCKADDRPORT(&io->dst.laddr)));

      SET_SOCKADDRPORT(&io->dst.laddr, htons(0));
   }

   io->dst.s = socket(io->dst.laddr.ss_family,
                      io->state.protocol == SOCKS_TCP ?
                           SOCK_STREAM : SOCK_DGRAM,
                      0);

   if (io->dst.s == -1) {
      snprintf(emsg, emsglen, "could not create socket: %s", strerror(errno));
      return -1;
   }

   if (io->state.extension.bind && io->state.command == SOCKS_BIND) {
      rc = 1;
      if (setsockopt(io->dst.s, SOL_SOCKET, SO_REUSEADDR, &rc, sizeof(rc)) != 0)
         swarn("%s: setsockopt(SO_REUSEADDR)", function);
   }

   if ((rc = socks_bind(io->dst.s, &io->dst.laddr, 0)) != 0) {
      /*
       * no such luck.  Bind any port then.
       */
      const in_port_t port_desired = GET_SOCKADDRPORT(&io->dst.laddr);

      SET_SOCKADDRPORT(&io->dst.laddr, htons(0));

      if ((rc = socks_bind(io->dst.s, &io->dst.laddr, 0)) == 0)
         slog(LOG_DEBUG,
               "%s: bound different port than desired (bound %u, not %u)",
               function,
               ntohs(port_desired),
               ntohs(GET_SOCKADDRPORT(&io->dst.laddr)));
   }

   if (rc != 0) {
      log_bind_failed(function, SOCKS_TCP, &io->dst.laddr);

      snprintf(emsg, emsglen,
               "failed to bind address on external side: %s", strerror(errno));

      return -1;
   }

   log_boundexternaladdress(function, &io->dst.laddr);

   return 0;
}

static void
auth2standard(auth, stdauth)
   const authmethod_t *auth;
   authmethod_t *stdauth;
{
   const char *function = "auth2standard()";

   bzero(stdauth, sizeof(*stdauth));

   switch (auth->method) {
      case AUTHMETHOD_NONE:
         stdauth->method = auth->method;
         break;

      case AUTHMETHOD_UNAME:
         stdauth->mdata.uname = auth->mdata.uname;
         stdauth->method      = auth->method;
         break;

#if HAVE_GSSAPI
      case AUTHMETHOD_GSSAPI:
         stdauth->mdata.gssapi = auth->mdata.gssapi;
         stdauth->method       = auth->method;
         break;
#endif /* HAVE_GSSAPI */

#if HAVE_PAM
      case AUTHMETHOD_PAM_USERNAME: {
         const authmethod_pam_t *pam = &auth->mdata.pam;

         STRCPY_ASSERTSIZE(stdauth->mdata.uname.name, pam->name);

         STRCPY_ASSERTSIZE(stdauth->mdata.uname.password, pam->password);

         stdauth->method = AUTHMETHOD_UNAME;
         break;
      }
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
      case AUTHMETHOD_BSDAUTH: {
         const authmethod_bsd_t *bsd = &auth->mdata.bsd;

         STRCPY_ASSERTSIZE(stdauth->mdata.uname.name, bsd->name);

         STRCPY_ASSERTSIZE(stdauth->mdata.uname.password, bsd->password);

         stdauth->method = AUTHMETHOD_UNAME;
         break;
      }
#endif /* HAVE_BSDAUTH */

      default:
         stdauth->method = AUTHMETHOD_NONE;
   }

   if (auth->method != stdauth->method)
      slog(LOG_DEBUG, "%s: converted from authmethod %d (%s) to %d (%s)",
           function,
           auth->method,
           method2string(auth->method),
           stdauth->method,
           method2string(stdauth->method));
}


#if SOCKS_SERVER
static sockd_io_t *
io_add(iolist, newio)
   sockd_io_t *iolist;
   const sockd_io_t *newio;
{
   const char *function = "io_add()";
   sockd_io_t *io, *previo;

   SASSERTX(newio->next == NULL);

   previo = io = iolist;
   while (io != NULL) {
      previo = io;
      io = io->next;
   }

   if ((io = malloc(sizeof(*newio))) == NULL)
      swarnx("%s: %s", function, NOMEM);
   else {
      *io = *newio;

      if (previo == NULL)
         previo = io;
      else
         previo->next = io;
   }

   return iolist == NULL ? previo : iolist;
}

static sockd_io_t *
io_remove(iolist, rmio)
   sockd_io_t *iolist;
   sockd_io_t *rmio;
{
   sockd_io_t *io, *previo;

   SASSERTX(iolist != NULL);

   if (iolist == rmio) {
      iolist = rmio->next;
      free(rmio);
      return iolist;
   }

   previo = iolist;
   io = iolist->next;
   while (io != NULL) {
      if (io == rmio) {
         previo->next = rmio->next;
         free(rmio);
         break;
      }

      previo = io;
      io = io->next;
   }

   return iolist;
}

static sockd_io_t *
io_find(iolist, addr)
   sockd_io_t *iolist;
   const struct sockaddr_storage *addr;
{
   sockd_io_t *io;

   if (addr == NULL)
      return iolist;

   io = iolist;
   while (io != NULL)
      if (sockaddrareeq(&io->src.laddr, addr, 0)
      ||  sockaddrareeq(&io->dst.laddr, addr, 0)
      ||  sockaddrareeq(&io->control.laddr, addr, 0))
         return io;
      else
         io = io->next;

   return NULL;
}

#endif /* SOCKS_SERVER */
