/*
 * Copyright (c) 1997, 1998, 1999, 2001, 2003, 2008, 2009, 2010, 2011, 2012,
 *               2013, 2014, 2016
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
"$Id: sockd_socket.c,v 1.170.4.1.2.2 2017/01/31 08:17:39 karls Exp $";

#define MAXSOCKETOPTIONS ( 1 /* TCP_NODELAY || SO_BROADCAST                  */\
                         + 1 /* SO_TIMESTAMP                                 */\
                         + 1 /* SO_OOBINLINE                                 */\
                         + 1 /* SO_KEEPALIVE                                 */\
                         + 1 /* SO_SNDBUF   || SO_SNDBUFFORCE                */\
                         + 1 /* SO_RCVBUF   || SO_RCVBUFFORCE                */\
                         + 1 /* IPV6_V6ONLY                                  */\
          )
#define MAXOPTIONNAME   (16) /* max length of any of the option names above. */
typedef struct {
   unsigned char     wantprivileges;
   int               level;
   int               optname;
   int               optval;
   size_t            optlen;
   char              textname[MAXOPTIONNAME];
} socketoptions_t;

static size_t
getoptions(const sa_family_t family, const int type, const int isclientside,
           socketoptions_t *optionsv, const size_t optionsc);
/*
 * Fills in "optionsv" with the correct values for a socket of family "family"
 * and type "type".
 * "isclientside" indicates if the socket is to be used on the client side
 * or not.
 *
 * Returns the number of options set (always <= MAXSOCKETOPTIONS).
 */

int
sockd_unconnect(s, oldpeer)
   const int s;
   const struct sockaddr_storage *oldpeer;
{
   const char *function = "sockd_unconnect()";
   struct sockaddr_storage local, remote, newlocal;
   socklen_t addrlen;
   char buf[MAXSOCKADDRSTRING];
   int havepeer;

   addrlen = sizeof(local);
   if (getsockname(s, TOSA(&local), &addrlen) != 0) {
      swarn("%s: getsockname()", function);
      return -1;
   }

   if (getpeername(s, TOSA(&remote), &addrlen) != 0) {
#if HAVE_LINUX_BUGS
      if (oldpeer != NULL && GET_SOCKADDRPORT(oldpeer) == htons(0))
         /*
          * Linux bug; accepts udp connect(2) to port 0, but getpeername(2)
          * on the same socket afterwards fails.
          */
         ;
      else
#endif /* HAVE_LINUX_BUGS */

      {
         swarn("%s: could not unconnect fd %d from %s",
               function,
               s,
               oldpeer == NULL ? "N/A" : sockaddr2string(oldpeer, NULL, 0));

         if (!ERRNOISTMP(errno))
            SWARN(errno); /* not bound?  Should not happen. */
      }

      havepeer = 0;
   }
   else
      havepeer = 1;

   slog(LOG_DEBUG, "%s: unconnecting fd %d, currently connected from %s to %s",
        function,
        s,
        sockaddr2string(&local, buf, sizeof(buf)),
        havepeer ? sockaddr2string(&remote, NULL, 0) : "nothing");

   bzero(&remote, sizeof(remote));
   SET_SOCKADDR(&remote, AF_UNSPEC);

   if (connect(s, TOSA(&remote), salen(remote.ss_family)) != 0)
      slog(LOG_DEBUG, "%s: connect to %s failed: %s",
           function, sockaddr2string(&remote, NULL, 0), strerror(errno));

   /*
    * May need to re-bind the socket after unconnect to make sure we get the
    * same address as we had before, as some systems only keep the portnumber
    * if not. :-/  Check first.
    */
   addrlen = sizeof(newlocal);
   if (getsockname(s, TOSA(&newlocal), &addrlen) != 0) {
      swarn("%s: getsockname() failed the second time", function);
      return -1;
   }

   if (sockaddrareeq(&local, &newlocal, 0))
      return 0; /* ok, no problem on this system. */

   /*
    * Ack, need to try to rebind the socket. :-/
    */

   if (socks_bind(s, &local, 1) != 0) {
      char a[MAXSOCKADDRSTRING], b[MAXSOCKADDRSTRING];
      int new_s;

      slog(LOG_DEBUG, "%s: re-bind(2) after unconnecting failed: %s.  "
                      "Current address is %s (was %s).  Trying to create a "
                      "new socket instead, though we might loose some packets "
                      "doing so",
                      function,
                      strerror(errno),
                      sockaddr2string(&newlocal, a, sizeof(a)),
                      sockaddr2string(&local, b, sizeof(b)));

      /*
       * There is an unfortunate race here, as while we create the new
       * socket packets could come in on the old socket, and those packets
       * will be lost.
       */

      new_s = 1;
      if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &new_s, sizeof(new_s)) != 0)
         swarn("%s: setsockopt(SO_REUSEADDR)", function);

      if ((new_s = socketoptdup(s, -1)) == -1) {
         swarn("%s: socketoptdup(%d) failed", function, s);
         return -1;
      }

      if (socks_bind(new_s, &local, 1) != 0) {
         slog(LOG_DEBUG, "%s: bind of new socket also failed: %s",
              function, strerror(errno));

         close(new_s);
         return 0;
      }

      slog(LOG_DEBUG, "%s: bind of new socket to address %s succeeded",
           function, sockaddr2string(&local, NULL, 0));

      if (dup2(new_s, s) == -1) {
         swarn("%s: dup2() failed", function);

         close(new_s);
         return 0;
      }

      close(new_s);
   }

   return 0;
}

void
sockd_rstonclose(s)
   const int s;
{
   const char *function = "sockd_rstonclose()";
   /* try to make the kernel generate a TCP RST for the other end upon close. */
   const struct linger linger = { 1, 0 };

   if (setsockopt(s,
                  SOL_SOCKET,
                  SO_LINGER,
                  &linger,
                  sizeof(linger)) != 0)
      slog(LOG_DEBUG,
           "%s: setsockopt(fd %d, SO_LINGER) failed: %s",
           function, s, strerror(errno));
}

int
bindinternal(protocol)
   const int protocol;
{
   const char *function = "bindinternal()";
   size_t i;

   for (i = 0; i < sockscf.internal.addrc; ++i) {
      listenaddress_t *l = &sockscf.internal.addrv[i];
      int val;

      if (l->protocol != protocol)
         continue;

      if (l->s != -1) {
         slog(LOG_DEBUG, "%s: address %s should be bound to fd %d already",
              function,
              sockaddr2string(&l->addr, NULL, 0),
              l->s);

         SASSERTX(fdisopen(l->s));

         /*
          * config-based socket options need to be (re)set though.
          * XXX missing code to unset any previously set options.
          */
         setconfsockoptions(l->s,
                            l->s,
                            SOCKS_TCP,
                            1,
                            0,
                            NULL,
                            0,
                            SOCKETOPT_PRE | SOCKETOPT_ANYTIME);

         continue;
      }

      if ((l->s = socket(l->addr.ss_family, SOCK_STREAM, 0)) == -1) {
         swarn("%s: could not create %s socket(2)",
               function, safamily2string(l->addr.ss_family));

         return -1;
      }

      setsockoptions(l->s, l->addr.ss_family, SOCK_STREAM, 1);

      /*
       * This breaks the principle that we should only set socket options
       * on sockets used for data (rather than sockets used for the control
       * messages), but some options can only be set at pre-connect time,
       * so if we do not set them here, we will never be able to set them.
       * Possibly we should limit the settings here to the options that
       * can _only_ be set at pre-connect time, so that at least other
       * options are not set unnecessarily.
       */

       /* XXX missing code to unset any previously set options. */
      setconfsockoptions(l->s,
                         l->s,
                         SOCKS_TCP,
                         1,
                         0,
                         NULL,
                         0,
                         SOCKETOPT_PRE | SOCKETOPT_ANYTIME);

      val = 1;
      if (setsockopt(l->s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0)
         swarn("%s: setsockopt(SO_REUSEADDR)", function);

#if !HAVE_PRIVILEGES
{
      /*
       * Specialcase this so that if we are started as root we do bind
       * the listen port requested, regardless of whether user.privileged is
       * set to root or not.  Allows a user who does not want Dante to
       * run as root, ever, but does want Dante to bind a privileged port
       * when initially starting, get what he wants.
       */
      socklen_t len;
      uid_t old_euid;
      int changed_euid = 0;

      len = salen(l->addr.ss_family);

      if (PORTISRESERVED(GET_SOCKADDRPORT(&l->addr))) {
         old_euid = geteuid();

         if (old_euid != 0) {
            if (seteuid(0) == 0) {
               sockscf.state.euid = 0;
               changed_euid       = 1;
            }
         }
      }

      if ((val = bind(l->s, TOSA(&l->addr), len)) == -1 && errno == EADDRINUSE)
         /* retry once. */
         val = bind(l->s, TOSA(&l->addr), len);

      if (changed_euid) {
         if (seteuid(old_euid) == 0)
            sockscf.state.euid = old_euid;
      }
}
#else /* HAVE_PRIVILEGES */

      val = socks_bind(l->s, &l->addr, 1);

#endif /* HAVE_PRIVILEGES */

      if (val != 0) {
         swarn("%s: bind of address %s (address #%lu/%lu) for server to "
               "listen on failed",
               function,
               sockaddr2string(&l->addr, NULL, 0),
               (unsigned long)i + 1,
               (unsigned long)sockscf.internal.addrc);

         return -1;
      }

      val = 1;
      if (setsockopt(l->s, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) != 0)
         swarn("%s: setsockopt(SO_REUSEADDR)", function);

      if (listen(l->s, SOCKD_MAXCLIENTQUEUE) == -1) {
         swarn("%s: listen(%d) failed", function, SOCKD_MAXCLIENTQUEUE);
         return -1;
      }

      /*
       * We want to accept(2) the client on a non-blocking descriptor.
       */
      if (setnonblocking(l->s, "accepting new clients") == -1)
         return -1;
   }

   return 0;
}

void
setsockoptions(s, family, type, isclientside)
   const int s;
   const sa_family_t family;
   const int type;
   const int isclientside;
{
   const char *function = "setsockoptions()";
   socketoptions_t optionsv[MAXSOCKETOPTIONS];
   size_t optc, i;

   slog(LOG_DEBUG, "%s: fd %d, type = %d, isclientside = %d",
        function, s, type, isclientside);

   /*
    * Our default builtin options.
    */
   optc = getoptions(family, type, isclientside, optionsv, ELEMENTS(optionsv));
   slog(LOG_DEBUG, "%s: %lu options to set", function, (unsigned long)optc);

   for (i = 0; i < optc; ++i) {
      int rc;

      SASSERTX(*optionsv[i].textname  != NUL);

      if (optionsv[i].wantprivileges)
         sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_ON);

      rc = setsockopt(s,
                      optionsv[i].level,
                      optionsv[i].optname,
                      &optionsv[i].optval,
                      optionsv[i].optlen);

      if (optionsv[i].wantprivileges)
         sockd_priv(SOCKD_PRIV_PRIVILEGED, PRIV_OFF);

      if (rc != 0) {
         if (optionsv[i].optname == SO_BROADCAST
         &&  type                == SOCK_DGRAM
         &&  errno               == EPROTO)
            ; /* SO_BROADCAST is not always supported. */
         else
            swarn("%s: setsockopt(%s) to value %d on fd %d failed",
                  function, optionsv[i].textname, optionsv[i].optval, s);
      }
   }

   (void)setnonblocking(s, function);

   if (sockscf.option.debug) {
      socklen_t len;
      int sndbuf, rcvbuf;

      len = sizeof(sndbuf);
      if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, &sndbuf, &len) != 0) {
         swarn("%s: could not get the size of SO_SNDBUF for fd %d",
               function, s);

         sndbuf = -1;
      }

      len = sizeof(rcvbuf);
      if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcvbuf, &len) != 0) {
         swarn("%s: could not get the size of SO_RCVBUF for fd %d",
               function, s);

         rcvbuf = -1;
      }

      slog(LOG_DEBUG,
           "%s: buffer sizes for fd %d are: SO_SNDBUF: %d, SO_RCVBUF: %d",
           function, s, sndbuf, rcvbuf);
   }

#if DIAGNOSTIC
   checksockoptions(s, family, type, isclientside);
#endif /* DIAGNOSTIC */
}

#if DIAGNOSTIC
void
checksockoptions(s, family, type, isclientside)
   const int s;
   const sa_family_t family;
   const int type;
   const int isclientside;
{
   const char *function = "checksockoptions()";
   socketoptions_t optionsv[MAXSOCKETOPTIONS];
   size_t optc, i;
   int val;

   slog(LOG_DEBUG, "%s: fd %d, type = %d, isclientside = %d",
        function, s, type, isclientside);

   optc = getoptions(family, type, isclientside, optionsv, ELEMENTS(optionsv));
   for (i = 0; i < optc; ++i) {
      socklen_t vallen = sizeof(val);
      int rc;

#if HAVE_LINUX_BUGS
#if defined(SO_SNDBUFFORCE) || defined(SO_RCVBUFFORCE)
      /*
       * Crazy Linux ... one name to set it, another one to get it.
       */
      if (optionsv[i].optname == SO_SNDBUFFORCE)
         optionsv[i].optname = SO_SNDBUF;
      else if (optionsv[i].optname == SO_RCVBUFFORCE)
         optionsv[i].optname = SO_RCVBUF;
#endif /* SO_SNDBUFFORCE || SO_RCVBUFFORCE */
#endif /* HAVE_LINUX_BUGS */

      rc = getsockopt(s,
                      optionsv[i].level,
                      optionsv[i].optname,
                      &val,
                      &vallen);

      if (rc != 0) {
         if (type == SOCK_STREAM && errno == ECONNRESET)
            continue; /* presumably failed while transferring the descriptor. */

         if (optionsv[i].optname == SO_BROADCAST
         &&  type                == SOCK_DGRAM
         &&  errno               == EPROTO)
            continue; /* SO_BROADCAST is not always supported. */

         swarn("%s: could not get socket option %s on fd %d",
               function, optionsv[i].textname, s);
      }

      if (val != optionsv[i].optval) {
         if ((optionsv[i].optval == 1) && val)
            ; /* assume it's a boolean; true, but not necessarily 1. */
         else if ((   optionsv[i].optname == SO_SNDBUF
                   || optionsv[i].optname == SO_RCVBUF)) {
            if (val < optionsv[i].optval)
               slog(LOG_INFO,
                    "%s: socketoption %s on fd %d should be %d, but is %d",
                    function,
                    optionsv[i].textname,
                    s,
                    optionsv[i].optval,
                    val);
         }
         else
            slog((type == SOCK_DGRAM && optionsv[i].optname == SO_BROADCAST) ?
                 LOG_DEBUG : LOG_WARNING,
                 "%s: socket option %s on fd %d should be %d, but is %d",
                 function,
                 optionsv[i].textname,
                 s,
                 optionsv[i].optval,
                 val);
      }
   }

   if ((val = fcntl(s, F_GETFL, 0)) == -1)
      swarn("%s: fcntl() failed to get descriptor flags of fd %d",
            function, s);
   else {
      if (! (val & O_NONBLOCK))
         swarn("%s: fd %d is blocking", function, s);
   }
}

#endif /* DIAGNOSTIC */

static size_t
getoptions(family, type, isclientside, optionsv, optionsc)
   const sa_family_t family;
   const int type;
   const int isclientside;
   socketoptions_t *optionsv;
   const size_t optionsc;
{
   size_t optc;

   optc = 0;
   switch (type) {
      case SOCK_STREAM:
         optionsv[optc].wantprivileges = 0;
         optionsv[optc].level          = IPPROTO_TCP;
         optionsv[optc].optname        = TCP_NODELAY;
         optionsv[optc].optval         = 1;
         optionsv[optc].optlen         = sizeof(optionsv[optc].optval);
         STRCPY_ASSERTSIZE(optionsv[optc].textname, "TCP_NODELAY");
         ++optc;
         SASSERTX(optc <= optionsc);

         optionsv[optc].wantprivileges = 0;
         optionsv[optc].level          = SOL_SOCKET;
         optionsv[optc].optname        = SO_OOBINLINE;
         optionsv[optc].optval         = 1;
         optionsv[optc].optlen         = sizeof(optionsv[optc].optval);
         STRCPY_ASSERTSIZE(optionsv[optc].textname, "SO_OOBINLINE");
         ++optc;
         SASSERTX(optc <= optionsc);

         if (sockscf.option.keepalive) {
            optionsv[optc].wantprivileges = 0;
            optionsv[optc].level          = SOL_SOCKET;
            optionsv[optc].optname        = SO_KEEPALIVE;
            optionsv[optc].optval         = 1;
            optionsv[optc].optlen         = sizeof(optionsv[optc].optval);
            STRCPY_ASSERTSIZE(optionsv[optc].textname, "SO_KEEPALIVE");
            ++optc;
            SASSERTX(optc <= optionsc);
         }

         break;

      case SOCK_DGRAM:
         optionsv[optc].wantprivileges = 0;
         optionsv[optc].level          = SOL_SOCKET;
         optionsv[optc].optname        = SO_BROADCAST;
         optionsv[optc].optval         = 1;
         optionsv[optc].optlen         = sizeof(optionsv[optc].optval);
         STRCPY_ASSERTSIZE(optionsv[optc].textname, "SO_BROADCAST");
         ++optc;
         SASSERTX(optc <= optionsc);

#if HAVE_SO_TIMESTAMP
         optionsv[optc].wantprivileges = 0;
         optionsv[optc].level          = SOL_SOCKET;
         optionsv[optc].optname        = SO_TIMESTAMP;
         optionsv[optc].optval         = 1;
         optionsv[optc].optlen         = sizeof(optionsv[optc].optval);
         STRCPY_ASSERTSIZE(optionsv[optc].textname, "SO_TIMESTAMP");
         ++optc;
         SASSERTX(optc <= optionsc);
#endif /* HAVE_SO_TIMESTAMP */

         break;

      default:
         SERRX(type);
   }

   SASSERTX(optc <= optionsc);
   return optc;
}
