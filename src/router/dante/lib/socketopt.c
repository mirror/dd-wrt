/*
 * Copyright (c) 2012, 2013, 2024
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
"$Id: socketopt.c,v 1.60.18.17 2024/08/19 04:00:31 michaels Exp $";

static void
setconfsockoption(const int in, const int out, const sa_family_t safamily,
                  const int protocol, const int isclientside,
                  const int whichtime, const socketoption_t *opt);


void
socketoptioncheck(const socketoption_t *option)
{

   if (option->info->level != option->level
   && !(   (option->level == IPPROTO_UDP || option->level == IPPROTO_TCP)
        &&  option->info->level == SOL_SOCKET))
      yywarnx("to our knowledge socket option \"%s\" is not valid at the "
              "protocol level given (%s/%d)",
              option->info->name,
              sockoptlevel2string(option->level),
              option->level);

   if (option->info->mask != 0) {
      SASSERTX(option->info->opttype == int_val
      ||       option->info->opttype == uchar_val);

      if ((~option->info->mask & option->optval.int_val) != 0)
         yywarnx("to our knowledge socket option %s can not have the value %d",
                 option->info->name, option->optval.int_val);
   }
}

int
addedsocketoption(optc, optv, newoption)
   size_t *optc;
   socketoption_t **optv;
   const socketoption_t *newoption;
{
   const char *function = "addedsocketoption()";
   void *newoptv;

   slog(LOG_DEBUG, "%s: adding socket option %s.  Currently have %lu options",
        function, sockopt2string(newoption, NULL, 0), (unsigned long)*optc);

   if (newoption->info != NULL && newoption->info->calltype == invalid) {
      yywarnx("option \"%s\" not user settable, ignoring",
              newoption->info->name);

      return 0;
   }

   if ((newoptv = realloc(*optv, sizeof(**optv) * (*optc + 1))) == NULL) {
      yywarn("could not allocate %lu bytes of memory to expand list of "
             "socket options",
             (unsigned long)(sizeof(**optv) * (*optc + 1)));

      return 0;
   }

   *optv              = newoptv;
   (*optv)[(*optc)++] = *newoption;

   return 1;
}

void
setconfsockoptions(target, in, protocol, isclientside, optc, optv,
                   whichlocals, whichglobals)
   const int target;
   const int in;
   const int protocol;
   const int isclientside;
   const size_t optc;
   const socketoption_t *optv;
   const int whichlocals;
   const int whichglobals;
{
   const char *function = "setconfsockoptions()";
   struct sockaddr addr;
   socklen_t len;
   size_t i;

   slog(LOG_DEBUG,
        "%s: going through options, looking for %s socket options for fd %d "
        "(in: %d) on the %s side",
        function,
        protocol2string(protocol),
        target,
        in,
        isclientside ? "internal" : "external");

   len = sizeof(addr);
   if (getsockname(target, &addr, &len) != 0) {
      slog(LOG_DEBUG, "%s: getsockname(2) on target-fd %d failed: %s",
           function, target, strerror(errno));

      return;
   }

   if (whichglobals) {
      /*
       * Set the globals first so that it is possible for the user to
       * override them locally in a rule/route.
       */

      slog(LOG_DEBUG,
           "%s: going through global array with %lu options, looking for "
           "globals matching %d (%s)",
           function,
           (unsigned long)sockscf.socketoptionc,
           whichglobals,
           socketsettime2string(whichglobals));

      for (i = 0; i < sockscf.socketoptionc; ++i)
         setconfsockoption(target,
                           in,
                           addr.sa_family,
                           protocol,
                           isclientside,
                           whichglobals,
                           &sockscf.socketoptionv[i]);
   }

   if (whichlocals) {
      slog(LOG_DEBUG,
           "%s: going through local array with %lu options, looking for "
           "locals matching %d",
           function, (unsigned long)optc, whichlocals);

      for (i = 0; i < optc; ++i)
         setconfsockoption(target,
                           in,
                           addr.sa_family,
                           protocol,
                           isclientside,
                           whichlocals,
                           &optv[i]);
   }
}

static void
setconfsockoption(target, in, safamily, protocol, isclientside, whichtime, opt)
   const int target;
   const int in;
   const sa_family_t safamily;
   const int protocol;
   const int isclientside;
   const int whichtime;
   const socketoption_t *opt;
{
   const char *function = "setconfsockoption()";
   socketoptvalue_t newvalue;
   socklen_t len;
   int rc;

   slog(LOG_DEBUG,
        "%s: checking protocol %s on the %s-side for whether socket option %s "
        "should be set at %s (%d) on %s target socket",
        function,
        protocol2string(protocol),
        isclientside ? "internal" : "external",
        sockopt2string(opt, NULL, 0),
        socketsettime2string(whichtime),
        whichtime,
        safamily2string(safamily));

   if (opt->info != NULL) {
      if (safamily == AF_INET  && !opt->info->ipv4_on)
         return;

      if (safamily == AF_INET6 && !opt->info->ipv6_on)
         return;

      if (((whichtime & SOCKETOPT_ANYTIME) && opt->info->calltype == anytime)
      ||  ((whichtime & SOCKETOPT_PRE)     && opt->info->calltype == preonly)
      ||  ((whichtime & SOCKETOPT_POST)    && opt->info->calltype == postonly))
         ;
      else
         return;
   }

   if (opt->isinternalside && !isclientside)
      return;

   if (!opt->isinternalside && isclientside)
      return;

/*
 * Does socketlevel "socketlevel" work with any l4 protocol?
 */
#define ANY_L4_PROTOCOL(socketlevel)                                           \
   ((socketlevel) == SOL_SOCKET || (socketlevel) == IPPROTO_IP)

   if (protocol    == SOCKS_TCP
   && !(ANY_L4_PROTOCOL(opt->level) || opt->level == IPPROTO_TCP))
      return;

   if (protocol    == SOCKS_UDP
   && !(ANY_L4_PROTOCOL(opt->level) || opt->level == IPPROTO_UDP))
      return;

   slog(LOG_DEBUG, "%s: setting %s", function, sockopt2string(opt, NULL, 0));

   if (opt->info != NULL) {
      if (opt->info->shift) {
         socketoptvalue_t oldvalue;
         const int mask = opt->info->mask << opt->info->shift;

         SASSERTX(opt->info->opttype == int_val
         ||       opt->info->opttype == uchar_val);

         bzero(&oldvalue, sizeof(oldvalue));

         switch (opt->opttype) {
            case int_val:
               newvalue.int_val  = opt->optval.int_val << opt->info->shift;
               oldvalue.int_val &= (~mask);
               newvalue.int_val |= oldvalue.int_val;
               break;

            case uchar_val:
               newvalue.uchar_val  = (unsigned char)
                                    (opt->optval.uchar_val << opt->info->shift);

               oldvalue.uchar_val &= (unsigned char)(~mask);
               newvalue.uchar_val |= oldvalue.uchar_val;
               break;

            default:
               SERRX(opt->opttype);
         }
      }
      else
         newvalue = opt->optval;
   }
   else
      newvalue = opt->optval;

#if !SOCKS_CLIENT

   if (opt->info != NULL && opt->info->needpriv)
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);

#endif /* !SOCKS_CLIENT */

   switch (opt->optname) {
      /*
       * Special cases.
       */

#if HAVE_SOCKS_HOSTID

#if HAVE_TCP_IPA

      case TCP_IPA:

#endif /* HAVE_TCP_IPA */

#if HAVE_TCP_EXP1

      case TCP_EXP1: 

#endif /* HAVE_TCP_EXP1 */

      {
         const struct in_addr *addr;
         struct sockaddr_storage raddr;
         struct hostid hostid_in, hostid_out;
         size_t i;
         int getraddr, gethostid;

         /*
          * In order to figure out what hostids to set on target, do we need 
          * to do getpeername(2)  Do we need to get currently set hostids? 
          */
         switch (newvalue.int_val) {
            case SOCKS_HOSTID_NONE:
               getraddr  = 0;
               gethostid = 0;
               break;

            case SOCKS_HOSTID_SETCLIENT:
               getraddr  = 1;
               gethostid = 0;
               break;

            case SOCKS_HOSTID_PASS:
               getraddr  = 0;
               gethostid = 1;
               break;

            case SOCKS_HOSTID_ADDCLIENT:
            case SOCKS_HOSTID_PASS_OR_SETCLIENT:
               getraddr  = 1;
               gethostid = 1;
               break;

            default:
               SERRX(0);
               /* NOTREACHED */
         }

         slog(LOG_DEBUG, "%s: int_val: %d, getraddr: %d, gethostid: %d", 
              function, newvalue.int_val, getraddr, gethostid);

         if (!getraddr && !gethostid) {
            slog(LOG_DEBUG, 
                 "%s: !getraddr && !gethostid; breaking out", function);

            rc = 0;
            break;
         }

         if (getraddr) {
            len = sizeof(raddr);

            if (getpeername(in, TOSA(&raddr), &len) == -1) {
               slog(LOG_DEBUG, "%s: getpeername(2) on client-fd %d failed: %s ",
                    function, in, strerror(errno));

               return;
            }
         }

         if (gethostid) {
            getsockethostid(in, &hostid_in);

            slog(LOG_DEBUG, 
                 "%s: retrieved %u type %d hostids on fd %d from client %s",
                 function,
                 (unsigned)hostid_in.addrc,
                 hostid_in.hostidtype,
                 in,
                 getraddr ? sockaddr2string(&raddr, NULL, 0) : "<N/A>");
         }
         else
            hostid_in.addrc = 0;

         bzero(&hostid_out, sizeof(hostid_out));

         if (hostid_in.addrc == 0) {
            /* 
             * if no hostids set on incomming connection, default the 
             * hostid-type to set on the outgoing connection, if setting 
             * any hostid, to the last version (TCP_EXP1).
             */

#if HAVE_TCP_EXP1

            hostid_out.hostidtype = SOCKS_HOSTID_TYPE_TCP_EXP1;

#elif HAVE_TCP_IPA 

            hostid_out.hostidtype = SOCKS_HOSTID_TYPE_TCP_IPA;

#else /* !HAVE_TCP_IPA */

#error "neither HAVE_TCP_EXP1 or HAVE_TCP_IPA set"

#endif /* !HAVE_TCP_EXP1 && !HAVE_TCP_IPA */

         }
         else
            hostid_out.hostidtype = hostid_in.hostidtype;

         /*
          * Do this again because there are some cases, where based on 
          * whether or not we got any hostids, what version of hostid, 
          * and what getpeername(2) returned, we may need to change 
          * some things.
          */
         switch (newvalue.int_val) {
            /* nothing changes for these cases. */
            case SOCKS_HOSTID_NONE:
            case SOCKS_HOSTID_PASS:
               break;

            case SOCKS_HOSTID_SETCLIENT:
               if (raddr.ss_family != AF_INET) {
                  slog(LOG_NOTICE, 
                       "%s: incoming client connection from %s is an %s "
                       "connection, while hostids are only specified for "
                       "%ses at the moment, so cannot set the address of "
                       "this client as a hostid",
                       function,
                       sockaddr2string(&raddr, NULL, 0),
                       safamily2string(raddr.ss_family),
                       safamily2string(AF_INET));

                  getraddr = 0;
               }

               break;

            case SOCKS_HOSTID_ADDCLIENT:
               if (raddr.ss_family != AF_INET) {
                  slog(LOG_NOTICE, 
                       "%s: incoming client connection from %s is an %s "
                       "connection, while hostids are only specified for "
                       "%ses at the moment, so cannot add this client to "
                       "the hostid list",
                       function,
                       sockaddr2string(&raddr, NULL, 0),
                       safamily2string(raddr.ss_family),
                       safamily2string(AF_INET));

                  getraddr = 0;
               }

               break; 

            case SOCKS_HOSTID_PASS_OR_SETCLIENT:
               /*
                *
                * This command is a bit clunky as the semantics depend on
                * whether we are using the older hostid version (TCP_IPA),
                * or the newer TCP_EXP1 version.
                *
                * For TCP_EXP1:
                *    - If hostids are present, pass them on, but don't add 
                *      this client.
                *
                * For TCP_IPA:
                *    - If hostids are present, pass them on, and add this 
                *      client.
                *
                * - If no hostids set, add this client, and use the TCP_EXP1
                *   hostid version.
                */
               if (hostid_in.addrc > 0) {
                  switch (hostid_in.hostidtype) {
                     case SOCKS_HOSTID_TYPE_TCP_EXP1:
                        getraddr = 0;
                        break;

                     case SOCKS_HOSTID_TYPE_TCP_IPA:
                        getraddr = 1;
                        break;

                     default:
                        SERRX(hostid_in.hostidtype);
                  }
               }

               if (raddr.ss_family != AF_INET) {
                  slog(LOG_NOTICE, 
                       "%s: incoming client connection from %s is an %s "
                       "connection and no hostids are set on that connection.  "
                       "Since hostids are only specified for %ses at the "
                       "moment, we cannot set this client hostid",
                       function,
                       sockaddr2string(&raddr, NULL, 0),
                       safamily2string(raddr.ss_family),
                       safamily2string(AF_INET));

                  getraddr = 0;
               }

               break;

            default:
               SERRX(newvalue.int_val);
               /* NOTREACHED */
         }

         if ((size_t)(hostid_in.addrc) + getraddr 
         >   ELEMENTS(hostid_out.addrv)) {
            char ntop[MAXSOCKADDRSTRING];

            for (i = ELEMENTS(hostid_out.addrv) - getraddr; 
            i < hostid_in.addrc; 
            ++i) {
               switch (hostid_out.hostidtype) {

#if HAVE_TCP_EXP1

                  case SOCKS_HOSTID_TYPE_TCP_EXP1:
                     addr 
                     = (struct in_addr *)&hostid_in.addrv[i].tcp_exp1.data.ip;

                     break;

#endif /* HAVE_TCP_EXP1 */

#if HAVE_TCP_IPA

                  case SOCKS_HOSTID_TYPE_TCP_IPA:
                     addr = (struct in_addr *)&hostid_in.addrv[i].tcp_ipa.ip;
                     break;

#endif /* HAVE_TCP_IPA */

                  default:
                     SERRX(hostid_out.hostidtype);
               }

               if (inet_ntop(AF_INET, addr, ntop, sizeof(ntop)) == NULL) {
                  swarn("%s: inet_ntop(3) failed on %s %x",
                       function,
                       atype2string(SOCKS_ADDR_IPV4),
                       addr->s_addr);

                  snprintf(ntop, sizeof(ntop), "<unknown>");
               }

               slog(LOG_NOTICE,
                    "%s: connection from %s has already reached the maximum "
                    "number of hostids (%u) to pass on upstream.  Can not "
                    "add any more, so discarding hostid %s%s",
                    function, 
                    getraddr ? sockaddr2string(&raddr, NULL, 0) : "<N/A>",
                    (unsigned)ELEMENTS(hostid_out.addrv),
                    ntop,
                    getraddr ? " before adding connecting client instead" : "");
            }

            /* truncate. */
            hostid_in.addrc = (unsigned char)(ELEMENTS(hostid_out.addrv)
                                              - getraddr);
         }

         slog(LOG_DEBUG, 
              "%s: %u hostids to copy from incoming connection, and %u to add",
              function, (unsigned)hostid_in.addrc, getraddr);

         SASSERTX(hostid_in.addrc  <= ELEMENTS(hostid_out.addrv));

         for (i = 0; i < hostid_in.addrc; ++i, ++hostid_out.addrc)
              hostid_out.addrv[i] = hostid_in.addrv[i];

         if (getraddr) {
            SASSERTX(hostid_out.addrc < ELEMENTS(hostid_out.addrv));

            switch (hostid_out.hostidtype) {

#if HAVE_TCP_EXP1

               case SOCKS_HOSTID_TYPE_TCP_EXP1: {
                  const size_t exidlen = 2; /* use default. */

                  hostid_out.addrv[hostid_out.addrc].tcp_exp1.len 
                  = tcp_exp1_len(exidlen);

                  switch (exidlen) {
                     case 2:
                        hostid_out.addrv[hostid_out.addrc].tcp_exp1.exid.exid_16
                        = htons(TCP_EXP1_EXID_IP);

                        break;

                     case 4:
                        hostid_out.addrv[hostid_out.addrc].tcp_exp1.exid.exid_32
                        = htonl(TCP_EXP1_EXID_IP);

                        break;

                     default:
                        SERRX(exidlen);
                  }

                  hostid_out.addrv[hostid_out.addrc].tcp_exp1.data.ip 
                  = TOIN(&raddr)->sin_addr.s_addr;

                  break;
               }

#endif /* HAVE_TCP_EXP1 */

#if HAVE_TCP_IPA

               case SOCKS_HOSTID_TYPE_TCP_IPA:
                  hostid_out.addrv[hostid_out.addrc].tcp_ipa.ip 
                  = TOIN(&raddr)->sin_addr.s_addr;

                  break;

#endif /* HAVE_TCP_IPA */

               default:
                  SERRX(hostid_out.hostidtype);
            }

            ++hostid_out.addrc;
         }

         rc  = setsockethostid(target, &hostid_out);

         if (rc != 0)
            swarn("%s: setsockethostid() on fd %d failed",
                  function, target);

         break;
      }

#endif /* HAVE_SOCKS_HOSTID */

      /*
       * The generic cases.
       */

      default:
         len = SOCKETOPTVALUETYPE2SIZE(opt->opttype);
         rc  = setsockopt(target,
                          opt->info == NULL ? opt->level : opt->info->level,
                          opt->optname,
                          &newvalue,
                          len);
   }

   if (rc != 0)
      swarn("%s: failed to set socket option %s of size %lu",
            function, sockopt2string(opt, NULL, 0), (unsigned long)len);
   else
      slog(LOG_DEBUG, "%s: set option %s to %s (len %d)",
           function,
           sockopt2string(opt, NULL, 0),
           sockoptval2string(newvalue, opt->opttype, NULL, 0),
           len);


#if !SOCKS_CLIENT

   if (opt->info != NULL && opt->info->needpriv)
      sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);

#endif /* !SOCKS_CLIENT */

}

