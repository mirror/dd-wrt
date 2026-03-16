/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2008, 2009, 2010, 2011, 2012,
 *               2013, 2014, 2020
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

#include "interposition.h"

#include "upnp.h"

#ifndef __USE_GNU
#define __USE_GNU /* XXX for RTLD_NEXT on Linux */
#endif /* !__USE_GNU */
#include <dlfcn.h>

static const char rcsid[] =
"$Id: address.c,v 1.288.4.4.6.4.4.1 2024/11/21 10:22:42 michaels Exp $";

/*
 * During init, we need to let all system calls resolve to the native
 * version.  I.e., socks_shouldcallasnative() need to always return
 * true as long as we are initing. Use this object for holding that
 * knowledge.
 */
#ifdef HAVE_VOLATILE_SIG_ATOMIC_T
sig_atomic_t doing_addrinit;
#else
volatile sig_atomic_t doing_addrinit;
#endif /* HAVE_VOLATILE_SIG_ATOMIC_T */


/*
 * "fake" ip addresses for clients that want/need to use that.
 * Note that this is process-specific, so it will not work with
 * programs that fork of "dns-helper".  Shared memory might have worked,
 * but even that would have depended on us being able to set up the
 * shared memory early enough, so just say we don't support that.
 */
static char **ipv;
static in_addr_t ipc;

#define FDV_INITSIZE    64 /* on init allocate memory for first 64 fd indexes */
static socksfd_t socksfdinit;
static int *dv;
static size_t dc;
static socksfd_t *socksfdv;
static size_t socksfdc;

#if HAVE_PTHREAD_H

static int socks_pthread_mutex_init(pthread_mutex_t *mutex,
                                    const pthread_mutexattr_t *attr);
static int socks_pthread_mutexattr_init(pthread_mutexattr_t *attr);
static int socks_pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type);
static int socks_pthread_mutex_lock(pthread_mutex_t *mutex);
static int socks_pthread_mutex_unlock(pthread_mutex_t *mutex);

typedef int (*PT_INIT_FUNC_T)(pthread_mutex_t *mutex,
                                     const pthread_mutexattr_t *attr);
static PT_INIT_FUNC_T pt_init;

typedef int (*PT_ATTRINIT_FUNC_T)(pthread_mutexattr_t *attr);
static PT_ATTRINIT_FUNC_T pt_attrinit;

typedef int (*PT_SETTYPE_FUNC_T)(pthread_mutexattr_t *attr, int type);
static PT_SETTYPE_FUNC_T pt_settype;

typedef int (*PT_LOCK_FUNC_T)(pthread_mutex_t *mutex);
static PT_LOCK_FUNC_T pt_lock;

typedef int (*PT_UNLOCK_FUNC_T)(pthread_mutex_t *mutex);
static PT_LOCK_FUNC_T pt_unlock;

typedef pthread_t (*PT_SELF_FUNC_T)(void);
static PT_SELF_FUNC_T pt_self;

static pthread_mutex_t addrmutex;
#endif /* HAVE_PTHREAD_H */

int
socks_isaddr(const int fd, const int takelock);
/*
 * If "takelock" is true, it means the function should take the
 * socksfdv/addrlock.
 *
 * Returns true if there is a address registered for the socket "fd", false
 * otherwise.
 */


static int
socks_addfd(const int d);
/*
 * adds the file descriptor "fd" to an internal table.
 * If it is already in the table the  request is ignored.
 * Returns:
 *    On success: 0
 *    On failure: -1
 */

static int
socks_isfd(const int fd);
/*
 * returns 1 if "fd" is a file descriptor in our internal table, 0 if not.
 */

static void
socks_rmfd(const int fd);
/*
 * removes the file descriptor "fd" from our internal table.
 */

socksfd_t *
socks_addaddr(clientfd, socksfd, takelock)
   const int clientfd;
   const socksfd_t *socksfd;
   const int takelock;
{
   const char *function = "socks_addaddr()";
   addrlockopaque_t lock;

   clientinit();

   SASSERTX(clientfd >= 0);
   SASSERTX(!(socksfd->state.protocol.tcp && socksfd->state.protocol.udp));

#if 0 /* DEBUG */
   if (socksfd->state.command != -1 && !socksfd->state.system)
      slog(LOG_DEBUG, "%s: %d", function, clientfd);
#endif

   SASSERTX(socksfd->state.command == -1
   ||       socksfd->state.command == SOCKS_BIND
   ||       socksfd->state.command == SOCKS_CONNECT
   ||       socksfd->state.command == SOCKS_UDPASSOCIATE);

   if (takelock)
      socks_addrlock(F_WRLCK, &lock);

   if (socks_addfd(clientfd) != 0)
      serrx("%s: error adding descriptor %d", function, clientfd);

   if (socksfdc < dc) { /* init/reallocate */
#if HAVE_GSSAPI
      int i;
#endif /* HAVE_GSSAPI */

      slog(LOG_DEBUG,
           "%s: realloc(3)-ing socksfdv array.  Increasing length from "
           "%d to %d",
           function, (int)socksfdc, (int)dc);

      if (socksfdinit.control == 0) { /* not initialized */
         socksfdinit.control = -1;
         /* other members have ok default value. */
      }

      if ((socksfdv = realloc(socksfdv, sizeof(*socksfdv) * dc)) == NULL)
         serr("%s: could not allocate %lu bytes",
              function, (unsigned long)(sizeof(*socksfdv) * dc));

#if HAVE_GSSAPI
      /* update internal pointers in previously existing objects. */
      for (i = 0; i < (int)socksfdc; ++i) {
         socksfd_t *sfd = &socksfdv[i];

         if (!socks_isaddr(i, 0))
            continue;

         sfd->state.gssapistate.value = sfd->state.gssapistatemem;
      }
#endif /* HAVE_GSSAPI */

      /* init new unallocated objects */
      while (socksfdc < dc)
         socksfdv[socksfdc++] = socksfdinit;
   }

   /*
    * On one machine gcc 4.1.2 expands the below to a memcpy() call with
    * overlapping destinations, according to valgrind:
    *    socksfdv[clientfd]           = *socksfd;
    *
    * Presumably a gcc bug, but what can we do ... :-/
    */
   memmove(&socksfdv[clientfd], socksfd, sizeof(*socksfd));

#if HAVE_GSSAPI

   socksfdv[clientfd].state.gssapistate.value
   = socksfdv[clientfd].state.gssapistatemem;

#endif

   socksfdv[clientfd].allocated = 1;

   if (takelock)
      socks_addrunlock(&lock);

#ifdef THREAD_DEBUG
   if (sockscf.log.fpv != NULL) {
      char buf[80];

      snprintf(buf, sizeof(buf), "%s: allocating fd %d for command %d\n",
               function, clientfd, socksfdv[clientfd].state.command);

      syssys_write(fileno(sockscf.log.fpv[0]), buf, strlen(buf) + 1);
   }
#endif /* THREAD_DEBUG */

   if (socksfd->state.auth.method == AUTHMETHOD_GSSAPI)
      sockscf.state.havegssapisockets = 1;

   return &socksfdv[clientfd];
}

socksfd_t *
socks_getaddr(d, socksfd, takelock)
   const int d;
   socksfd_t *socksfd;
   const int takelock;
{
#if HAVE_GSSAPI
   const char *function = "socks_getaddr()";
#endif /* HAVE_GSSAPI */
   socksfd_t *sfd;
   addrlockopaque_t lock;

   if (socksfd == NULL) {
      static socksfd_t ifnullsocksfd;

      socksfd = &ifnullsocksfd;
   }

   if (takelock)
      socks_addrlock(F_RDLCK, &lock);

   if (socks_isaddr(d, 0)) {
      sfd = &socksfdv[d];

#if HAVE_GSSAPI
      if (sfd->state.gssimportneeded) {
         if (sockscf.state.insignal) {
            char buf[32];
            const char *msgv[] =
            { function,
              ": ",
              "not importing gssapistate for fd ",
              ltoa((long)d, buf, sizeof(buf)),
              NULL
            };

            signalslog(LOG_DEBUG, msgv);
         }
         else {
            slog(LOG_DEBUG, "%s: importing gssapistate for fd %d", function, d);

            if (gssapi_import_state(&sfd->state.auth.mdata.gssapi.state.id,
                                    &sfd->state.gssapistate) != 0) {
               swarnx("%s: failed to import gssapi context of length %lu for  "
                      "fd %d",
                      function,
                      (unsigned long)sfd->state.gssapistate.length, d);

               socks_rmaddr(d, 0);
               sfd = NULL;
            }
            else {
               sfd->state.gssimportneeded = 0;
               slog(LOG_DEBUG,
                    "%s: imported gssapistate for fd %d using ctxid %ld",
                    function, d, (long)sfd->state.auth.mdata.gssapi.state.id);
            }
         }
      }
#endif /* HAVE_GSSAPI */
   }
   else
      sfd = NULL;

   if (takelock)
      socks_addrunlock(&lock);

   if (sfd == NULL)
      return NULL;

   *socksfd = *sfd;
   return socksfd;
}

void
socks_rmaddr(d, takelock)
   const int d;
   const int takelock;
{
   const char *function = "socks_rmaddr()";
   addrlockopaque_t lock;

   if (d < 0 || (size_t)d >= socksfdc)
      return; /* not a socket of ours. */

   if (takelock)
      socks_addrlock(F_WRLCK, &lock);

   socks_rmfd(d);

   if (!socksfdv[d].state.issyscall) /* syscall adds/removes all the time. */
      socks_freebuffer(d);

   switch (socksfdv[d].state.version) {
      case PROXY_SOCKS_V4:
      case PROXY_SOCKS_V5:
         if (socksfdv[d].state.issyscall)
            break;

#if HAVE_GSSAPI

         if (socksfdv[d].state.auth.method == AUTHMETHOD_GSSAPI
         &&  socksfdv[d].state.auth.mdata.gssapi.state.id != GSS_C_NO_CONTEXT) {
            OM_uint32 major_status, minor_status;
            char buf[512];

            major_status = gss_delete_sec_context(&minor_status,
                                  &socksfdv[d].state.auth.mdata.gssapi.state.id,
                                                  GSS_C_NO_BUFFER);

            if (major_status != GSS_S_COMPLETE) {
               if (!gss_err_isset(major_status, minor_status, buf, sizeof(buf)))
                  *buf = NUL;

               swarnx("%s: gss_delete_sec_context() for fd %d failed%s%s",
                      function,
                      d,
                      *buf == NUL ? "" : ": ",
                      *buf == NUL ? "" : buf);
            }
            else {
               slog(LOG_DEBUG, "%s: deleted GSSAPI context for fd %d",
                    function, d);

               SASSERTX(socksfdv[d].state.auth.mdata.gssapi.state.id
               == GSS_C_NO_CONTEXT);
            }
         }
#endif /* HAVE_GSSAPI */

         switch (socksfdv[d].state.command) {
            case SOCKS_BIND:
               if (socksfdv[d].control == -1
               ||  socksfdv[d].control == d)
                  break;

               /*
                * If we are using the bind extension it's possible
                * that this control connection is shared with other
                * (accept()'ed) addresses, if so we must leave it
                * open for the other connections.
               */
               if (socks_addrcontrol(-1, d, 0) != -1)
                  break;

               close(socksfdv[d].control);
               break;

            case SOCKS_CONNECT:
               break; /* no separate control connection. */

            case SOCKS_UDPASSOCIATE:
               if (socksfdv[d].control != -1)
                  close(socksfdv[d].control);
               break;

            default:
               SERRX(socksfdv[d].state.command);
         }
         break;

      case PROXY_UPNP:
         if (socksfdv[d].state.issyscall)
            break;

         upnpcleanup(d);
         break;
   }

#ifdef THREAD_DEBUG
   if (sockscf.log.fpv != NULL) {
      char buf[80];

      snprintf(buf, sizeof(buf),
               "%s: deallocating fd %d, was allocated for command %d\n",
               function, d, socksfdv[d].state.command);

      syssys_write(fileno(sockscf.log.fpv[0]), buf, strlen(buf) + 1);
   }
#endif /* THREAD_DEBUG */

   socksfdv[d] = socksfdinit;

#if DIAGNOSTIC
   SASSERTX(socks_isaddr(d, 0) == 0);
   SASSERTX(socks_getaddr(d, NULL, 0) == NULL);
#endif /* DIAGNOSTIC */

   if (takelock)
      socks_addrunlock(&lock);
}

int
socks_isaddr(d, takelock)
   const int d;
   const int takelock;
{

   if (d < 0 || (size_t)d >= socksfdc)
      return 0;

   return socksfdv[d].allocated;
}

int
socks_addrisours(s, socksfdmatch, takelock)
   const int s;
   socksfd_t *socksfdmatch;
   const int takelock;
{
   const char *function = "socks_addrisours()";
   const char *breakreason = NULL;
   const int errno_s = errno;
   addrlockopaque_t lock;
   struct sockaddr_storage local, remote;
   socklen_t locallen, remotelen;
   int matched, type;

   slog(LOG_DEBUG, "%s: fd %d", function, s);

   if (takelock)
      socks_addrlock(F_RDLCK, &lock);

   locallen = sizeof(local);
   if (getsockname(s, TOSA(&local), &locallen) != 0) {
      slog(LOG_DEBUG,
           "%s: no match due to fd %d not having a local addr (errno = %d, %s)",
           function, s, errno, strerror(errno));

      if (takelock)
         socks_addrunlock(&lock);

      errno = errno_s;
      return 0;
   }
   else
      slog(LOG_DEBUG, "%s: local address of fd %d is %s",
           function, s, sockaddr2string(&local, NULL, 0));

   /* only network-sockets can be proxied. */
   if (local.ss_family != AF_INET
#ifdef AF_INET6
   &&  local.ss_family != AF_INET6
#endif /* AF_INET6 */
   ) {
      slog(LOG_DEBUG,
           "%s: no match due to fd %d not being an AF_INET/AF_INET6 socket",
           function, s);

      if (takelock)
         socks_addrunlock(&lock);

      errno = errno_s;
      return 0;
   }

   locallen = sizeof(type);
   if (getsockopt(s, SOL_SOCKET, SO_TYPE, &type, &locallen) != 0) {
      slog(LOG_DEBUG,
           "%s: no match due to getsockopt(SO_TYPE) failing on fd %d "
           "(errno = %d, %s)",
           function, s, errno, strerror(errno));

      if (takelock)
         socks_addrunlock(&lock);

      errno = errno_s;
      return 0;
   }

   if (type != SOCK_DGRAM && type != SOCK_STREAM) {
      slog(LOG_DEBUG,
           "%s: no match due to fd %d being neither SOCK_DGRAM nor SOCK_STREAM",
           function, s);

      if (takelock)
         socks_addrunlock(&lock);

      errno = errno_s;
      return 0;
   }

   errno   = 0;
   matched = 0;

   do {
      socksfd_t socksfd;

      if (socks_getaddr(s, &socksfd, 0) != NULL) {
         if ((socksfd.state.protocol.udp && type != SOCK_DGRAM)
         ||  (socksfd.state.protocol.tcp && type != SOCK_STREAM)) {
            breakreason = "protocol mismatch between registered and current";
            break;
         }

         if (TOCIN(&socksfd.local)->sin_addr.s_addr == htonl(0)) {
            /*
             * if address was not bound before, it might have become
             * later, after client did a send(2) or similar.
             * It's also possible accept(2) was called, so check
             * for that first.
             */
            socksfd_t nsocksfd, *socksfdptr;
            int duped;

            remotelen = sizeof(remote);
            if (getpeername(s, TOSA(&remote), &remotelen) == 0
            && (duped = socks_addrmatch(&local, &remote, NULL, 0)) != -1) {
               if ((socksfdptr = socks_addrdup(socks_getaddr(duped, NULL, 0),
                                               &nsocksfd)) == NULL) {
                  swarn("%s: socks_addrdup()", function);

                  if (errno == EBADF)
                     socks_rmaddr(duped, 0);

                  breakreason = "known fd, but unbound; socks_addrdup() failed";
                  break;
               }

               socksfd = *socksfdptr;
               socks_addaddr(s, &nsocksfd, 0);
               matched = 1;

               if (!fdisopen(duped))
                  socks_rmaddr(duped, 0);
            }
            else {
               nsocksfd = socksfd;
               TOIN(&nsocksfd.local)->sin_addr = TOIN(&local)->sin_addr;
               socksfd = *socks_addaddr(s, &nsocksfd, 0);
            }
         }

         if (!sockaddrareeq(&local, &socksfd.local, 0)) {
            breakreason = "local neq socksfd.local";
            break;
         }

         /* check remote endpoint too? */
         matched = 1;
      }
      else { /* unknown descriptor.  Try to check whether it's a dup. */
         int duped;

         if (!PORTISBOUND(&local)) {
            breakreason = "unknown fd and no local IP-address bound for it";
            break;
         }

         /* XXX check remote endpoint also. */
         if ((duped = socks_addrmatch(&local, NULL, NULL, 0)) != -1
         && ((socksfdv[duped].state.protocol.udp && type == SOCK_DGRAM)
          || (socksfdv[duped].state.protocol.tcp && type == SOCK_STREAM))) {
            socksfd_t nsocksfd;

            slog(LOG_DEBUG, "%s: fd %d appears to be a dup of fd %d (%s)",
                 function,
                 s,
                 duped,
                 socket2string(duped, NULL, 0));

            if (socks_addrdup(socks_getaddr(duped, NULL, 0), &nsocksfd)
            == NULL) {
               swarn("%s: socks_addrdup()", function);

               if (errno == EBADF)
                  socks_rmaddr(duped, 0);

               breakreason = "unknown fd and socks_addrdup() failed";
               break;
            }

            socks_addaddr(s, &nsocksfd, 0);

            if (!fdisopen(duped))
               socks_rmaddr(duped, 0);

            matched = 1;
         }

         breakreason = "unknown fd and no socks_addrmatch()";
         break;
      }
   } while (/* CONSTCOND */ 0);

   if (matched) {
      socksfd_t socksfd;

      socks_getaddr(s, &socksfd, 0);
      SASSERTX(!(socksfd.state.protocol.tcp && socksfd.state.protocol.udp));

      if ((socksfd.state.protocol.udp && type != SOCK_DGRAM)
      ||  (socksfd.state.protocol.tcp && type != SOCK_STREAM)) {
         breakreason = "protocol mismatch between registered and current";
         matched = 0;
      }
      else {
         if (socksfdmatch != NULL)
            *socksfdmatch = socksfd;
      }
   }

   if (takelock)
      socks_addrunlock(&lock);

   if (!matched && breakreason != NULL)
      slog(LOG_DEBUG, "%s: no match due to %s", function, breakreason);

   errno = errno_s;
   return matched;
}

int
socks_addrcontrol(controlsent, controlinuse, takelock)
   const int controlsent;
   const int controlinuse;
   const int takelock;
{
   const char *function = "socks_addrcontrol()";
   addrlockopaque_t lock;
   char fdsentstr[1024], fdinusestr[sizeof(fdsentstr)];
   int i;

   slog(LOG_DEBUG, "%s: sent fd %d (%s), in use fd %d (%s)",
        function,
        controlsent,
        controlsent == -1 ?
            "<none>" : socket2string(controlsent, fdsentstr, sizeof(fdsentstr)),
        controlinuse,
        socket2string(controlinuse, fdinusestr, sizeof(fdinusestr)));

   SASSERTX(controlinuse >= 0);

   if (takelock)
      socks_addrlock(F_RDLCK, &lock);

   if (socks_isaddr(controlsent, 0)) {
      /*
       * First check the index corresponding to what the descriptor should
       * be, if nothing tricky (dup(2) or similar) happened between the time
       * we sent the descriptor to the connect-process, and now.
       * If it doesn't match, we will have to go through all of the indexes.
       */
      if (fdisdup(controlsent, socksfdv[controlsent].control)) {
         if (takelock)
            socks_addrunlock(&lock);

         return controlsent;
      }
   }

   for (i = 0; i < (int)socksfdc; ++i) {
      if (!socks_isaddr(i, 0))
         continue;

      if (socksfdv[i].state.command == -1)
         continue;

      if (fdisdup(controlinuse, socksfdv[i].control))
         break;
   }

   if (takelock)
      socks_addrunlock(&lock);

   if (i < (int)socksfdc)
      return i;

   return -1;
}

int
socks_addrmatch(local, remote, state, takelock)
   const struct sockaddr_storage *local;
   const struct sockaddr_storage *remote;
   const socksstate_t *state;
   const int takelock;
{
   const char *function = "socks_addrmatch()";
   addrlockopaque_t lock;
   char lstr[MAXSOCKADDRSTRING], rstr[MAXSOCKADDRSTRING];
   int i;

   slog(LOG_DEBUG, "%s: local = %s, remote = %s",
        function,
        local  == NULL ?
           "NULL" : sockaddr2string(local, lstr, sizeof(lstr)),
        remote == NULL ?
           "NULL" : sockaddr2string(remote, rstr, sizeof(rstr)));

   if (takelock)
      socks_addrlock(F_RDLCK, &lock);

   for (i = 0; i < (int)socksfdc; ++i) {
      if (!socks_isaddr(i, 0))
         continue;

      /*
       * only compare fields that have a valid value in request to compare
       * against.
       */

      if (local != NULL) {
         if (sockaddrareeq(local, &socksfdv[i].local, 0))
            slog(LOG_DEBUG, "%s: local address %s matches %s for socksfdv[%d]",
                 function,
                 sockaddr2string(local, lstr, sizeof(lstr)),
                 sockaddr2string(&socksfdv[i].local, NULL, 0),
                 i);
         else
            continue;
      }

      if (remote != NULL) {
         if (sockaddrareeq(remote, &socksfdv[i].remote, 0))
            slog(LOG_DEBUG, "%s: remote address %s matches %s for socksfdv[%d]",
                 function,
                 sockaddr2string(remote, rstr, sizeof(rstr)),
                 sockaddr2string(&socksfdv[i].remote, NULL, 0),
                 i);
         else
            continue;
      }

      if (state != NULL) {
         if (state->version != -1)
            if (state->version != socksfdv[i].state.version)
               continue;

         if (state->command != -1)
            if (state->command != socksfdv[i].state.command)
               continue;

         if (state->inprogress != -1)
            if (state->inprogress != socksfdv[i].state.inprogress)
               continue;

         if (state->acceptpending != -1)
            if (state->acceptpending != socksfdv[i].state.acceptpending)
               continue;
      }

      break;
   }

   if (takelock)
      socks_addrunlock(&lock);

   if (i < (int)socksfdc)
      return i;

   return -1;
}

socksfd_t *
socks_addrdup(old, new)
   const socksfd_t *old;
   socksfd_t *new;
{
/*   const char *function = "socks_addrdup()"; */

   *new = *old;   /* init most stuff. */

   switch (old->state.command) {
      case SOCKS_BIND:
      case SOCKS_UDPASSOCIATE:
         if ((new->control = socketoptdup(old->control, -1)) == -1)
            return NULL;
         break;

      case SOCKS_CONNECT:
         /* only descriptor for connect is the one client has. */
         break;

      default:
         break;
   }

   return new;
}

void
socks_addrlock(locktype, lock)
   const int locktype;
   addrlockopaque_t *lock;
{

   socks_sigblock(-1, (sigset_t *)lock);

#if HAVE_PTHREAD_H
   /*
    * With the OpenBSD thread implementation, if a thread is interrupted,
    * calling pthread_mutex_lock() seems to clear the interrupt flag, so
    * that e.g. select(2) will restart rather than returning EINTR.
    * We don't wont that to happen since we depend on select(2)/etc.
    * being interrupted by the process used to handle non-blocking connects.
    * We instead take the risk of not taking the thread-lock in this case.
    */
   if (!sockscf.state.insignal)
      /* XXX set based on locktype. */
      socks_pthread_mutex_lock(&addrmutex);
#endif /* HAVE_PTHREAD_H */
}

void
socks_addrunlock(lock)
   const addrlockopaque_t *lock;
{

#if HAVE_PTHREAD_H
   if (!sockscf.state.insignal)
      socks_pthread_mutex_unlock(&addrmutex);
#endif /* HAVE_PTHREAD_H */

   socks_sigunblock((const sigset_t *)lock);
}

in_addr_t
socks_addfakeip(host)
   const char *host;
{
   const char *function = "socks_addfakeip()";
   addrlockopaque_t lock;
   struct in_addr addr;
   char **tmpmem;
   int ipc_added;

   socks_addrlock(F_WRLCK, &lock);

   if (socks_getfakeip(host, &addr)) {
      socks_addrunlock(&lock);
      return addr.s_addr;
   }

#if FAKEIP_END < FAKEIP_START
error "\"FAKEIP_END\" cannot be smaller than \"FAKEIP_START\""
#endif

   if (ipc >= FAKEIP_END - FAKEIP_START) {
      swarnx("%s: fakeip range (%d - %d) exhausted",
      function, FAKEIP_START, FAKEIP_END);

      socks_addrunlock(&lock);
      return INADDR_NONE;
   }

   if ((tmpmem      = realloc(ipv, sizeof(*ipv) * (ipc + 1)))        == NULL
   ||  (tmpmem[ipc] = malloc(sizeof(**tmpmem) * (strlen(host) + 1))) == NULL) {
      swarn("%s: could not allocate %lu bytes",
           function,
           (unsigned long)((sizeof(*ipv) * (ipc + 1))
                         + (sizeof(**tmpmem) * (strlen(host) + 1))));

      if (tmpmem != NULL)
         free(tmpmem);

      socks_addrunlock(&lock);
      return INADDR_NONE;
   }
   ipv = tmpmem;

   ipc_added = ipc;
   strcpy(ipv[ipc++], host);

   socks_addrunlock(&lock);

   return htonl(ipc_added + FAKEIP_START);
}

const char *
socks_getfakehost(addr)
   in_addr_t addr;
{
   const char *function = "socks_getfakehost()";
   addrlockopaque_t lock;
   const char *host;

   if (ntohl(addr) - FAKEIP_START < ipc) {
      socks_addrlock(F_RDLCK, &lock);
      host = ipv[ntohl(addr) - FAKEIP_START];
      socks_addrunlock(&lock);
   }
   else {
      if (ntohl(addr) >= FAKEIP_START &&  ntohl(addr) <= FAKEIP_END)
         swarnx("%s: looks like ip address %s might be a \"fake\" ip address, "
                "but we have no knowledge of that address in this process.  "
                "Possibly this client is forking a \"dns-helper\"-style "
                "program for resolving hostnames.  We unfortunately do not "
                "support using fake ip addresses in that case.",
                function, inet_ntoa(*(struct in_addr *)&addr));

      host = NULL;
   }

   return host;
}

int
socks_getfakeip(host, addr)
   const char *host;
   struct in_addr *addr;
{
   addrlockopaque_t lock;
   unsigned int i;

   socks_addrlock(F_RDLCK, &lock);

   for (i = 0; i < ipc; ++i)
      if (strcasecmp(host, ipv[i]) == 0) {
         addr->s_addr = htonl(i + FAKEIP_START);
         break;
      }

   socks_addrunlock(&lock);

   if (i < ipc)
      return 1;

   return 0;
}

sockshost_t *
fakesockaddr2sockshost(_addr, host)
   const struct sockaddr_storage *_addr;
   sockshost_t *host;
{
   const char *function = "fakesockaddr2sockshost()";
   struct sockaddr_storage addr;
   char string[MAXSOCKADDRSTRING];

   clientinit(); /* may be called before normal init, log to right place. */

   sockaddrcpy(&addr, _addr, salen(_addr->ss_family));

   slog(LOG_DEBUG, "%s: %s -> %s",
        function,
        sockaddr2string(&addr, string, sizeof(string)),
        socks_getfakehost(TOIN(&addr)->sin_addr.s_addr) == NULL ?
            string : socks_getfakehost(TOIN(&addr)->sin_addr.s_addr));

   if (socks_getfakehost(TOIN(&addr)->sin_addr.s_addr) != NULL) {
      const char *ipname = socks_getfakehost(TOIN(&addr)->sin_addr.s_addr);

      SASSERTX(ipname != NULL);

      host->atype = SOCKS_ADDR_DOMAIN;

      STRCPY_ASSERTLEN(host->addr.domain, ipname);

      host->port = TOIN(&addr)->sin_port;
   }
   else
      sockaddr2sockshost(&addr, host);

   return host;
}

struct sockaddr_storage *
int_fakesockshost2sockaddr(host, _addr, _addrlen)
   const sockshost_t *host;
   struct sockaddr_storage *_addr;
   size_t _addrlen;
{
   const char *function = "int_fakesockshost2sockaddr()";
   struct sockaddr_storage addr;
   char string[MAXSOCKSHOSTSTRING];

   clientinit(); /* may be called before normal init, log to right place. */

   slog(LOG_DEBUG, "%s: %s",
        function, sockshost2string(host, string, sizeof(string)));

   bzero(&addr, sizeof(addr));

   switch (host->atype) {
      case SOCKS_ADDR_DOMAIN:
         SET_SOCKADDR(&addr, AF_INET);
         if (socks_getfakeip(host->addr.domain, &(TOIN(&addr)->sin_addr)))
            break;
         /* else; */ /* FALLTHROUGH */

      default:
         sockshost2sockaddr(host, &addr);
   }

   TOIN(&addr)->sin_port = host->port;

   bzero(_addr, _addrlen);
   sockaddrcpy(_addr, &addr, MIN(salen(addr.ss_family), _addrlen));

   return _addr;
}

static int
socks_addfd(d)
   const int d;
{
   const char *function = "socks_addfd()";

   clientinit();

   SASSERTX(d >= 0);

   if ((unsigned int)d >= dc) { /* init/reallocate */
      size_t newfdc;
      int *newfdv;

      newfdc = (d + 1) * 2; /* add some extra at the same time. */

      slog(LOG_DEBUG,
           "%s: realloc(3)-ing dv array for fd %d.  Increasing length "
           "from %d to %d",
           function, d, (int)dc, (int)newfdc);

      if ((newfdv = realloc(dv, sizeof(*dv) * newfdc)) == NULL)
         serr("%s: could not allocate %lu bytes",
              function, (unsigned long)(sizeof(*dv) * newfdc));

      dv = newfdv;

      /* init all to -1, a illegal value for a descriptor. */
      while (dc < newfdc)
         dv[dc++] = -1;
   }

   dv[d] = d;

   return 0;
}

static int
socks_isfd(d)
   const int d;
{
   if (d < 0 || (unsigned int)d >= dc || dv[d] == -1)
      return 0;
   return 1;
}

static void
socks_rmfd(d)
   const int d;
{
   if (socks_isfd(d))
      dv[d] = -1;
}


void
socks_addrinit(void)
{
   const char *function = "socks_addrinit()";
#if HAVE_PTHREAD_H
   pthread_mutexattr_t attr;
   void *lpt;
#endif /* HAVE_PTHREAD_H */

#ifdef HAVE_VOLATILE_SIG_ATOMIC_T
   static sig_atomic_t inited;
#else
   static volatile sig_atomic_t inited;
#endif /* HAVE_VOLATILE_SIG_ATOMIC_T */

   if (inited)
      return;

   if (doing_addrinit)
      /*
       * XXX should really be sched_yield() or similar if initing, unless
       * the thread initing is ours.  If the thread initing is ours,
       * we can just return, to handle recursive problems during init.
       */
      return;

   doing_addrinit = 1; /*
                        * XXX should be pthread_self() or similar, but how can
                        * we call that before we have finished initing? :-/
                        */

   SASSERTX(socksfdv == NULL && dv == NULL);

   if ((socksfdv = malloc(sizeof(*socksfdv) * FDV_INITSIZE)) == NULL)
      serr("%s: failed to alloc %lu bytes for socksify socksfd memory",
           function, (unsigned long)(sizeof(*socksfdv) * 64));

   if ((dv = malloc(sizeof(*dv) * FDV_INITSIZE)) == NULL)
      serr("%s: failed to alloc %lu bytes for socksify dv memory",
           function, (unsigned long)(sizeof(*dv) * 64));

   /* init new objects */
   while (socksfdc < FDV_INITSIZE)
      socksfdv[socksfdc++] = socksfdinit;

   /* init all to -1, a illegal value for a descriptor. */
   while (dc < FDV_INITSIZE)
      dv[dc++] = -1;


#if HAVE_PTHREAD_H
   if (socks_getenv(ENV_SOCKS_DISABLE_THREADLOCK, istrue) != NULL)
      slog(LOG_DEBUG, "pthread locking off, manually disabled in environment");
   else {
#if HAVE_RTLD_NEXT
      /*
       * XXX following test will always perceive the application as being
       * threaded if lib(d)socks depends on libpthread, which might be the
       * case if e.g., some gssapi libs require this library.
       */
      if (dlsym(RTLD_NEXT, SYMBOL_PT_ATTRINIT) != NULL) {
         /*
          * appears to be a threaded application, obtain function pointers.
          */

         lpt = RTLD_NEXT;
         slog(LOG_DEBUG,
              "%s: pthread locking desired: threaded program (rtld)", function);
      }
      else {
         slog(LOG_DEBUG,
              "%s: pthread locking off: non-threaded program (rtld)", function);

         lpt = NULL;
      }

#else
      /* load libpthread */
      if ((lpt = dlopen(LIBRARY_PTHREAD, RTLD_LAZY)) == NULL) {
         swarn("%s: compile time configuration error?  Failed to open "
               "\"%s\": %s",
               function, LIBRARY_PTHREAD, dlerror());
      }
#endif /* !HAVE_RTLD_NEXT */

      if (lpt != NULL) {
         /*
          * resolve pthread symbols.
          */

         if ((pt_init = (PT_INIT_FUNC_T)dlsym(lpt, SYMBOL_PT_INIT)) == NULL)
            swarn("%s: compile time configuration error?  "
                  "Failed to find \"%s\" in \"%s\": %s",
                  function, SYMBOL_PT_INIT, LIBRARY_PTHREAD, dlerror());

         if ((pt_attrinit = (PT_ATTRINIT_FUNC_T)dlsym(lpt, SYMBOL_PT_ATTRINIT))
         == NULL)
            swarn("%s: compile time configuration error?  "
                  "Failed to find \"%s\" in \"%s\": %s",
                  function, SYMBOL_PT_ATTRINIT, LIBRARY_PTHREAD, dlerror());

         if ((pt_settype = (PT_SETTYPE_FUNC_T)dlsym(lpt, SYMBOL_PT_SETTYPE))
         == NULL)
            swarn("%s: compile time configuration error?  "
                  "Failed to find \"%s\" in \"%s\": %s",
                  function, SYMBOL_PT_SETTYPE, LIBRARY_PTHREAD, dlerror());

         if ((pt_lock = (PT_LOCK_FUNC_T)dlsym(lpt, SYMBOL_PT_LOCK)) == NULL)
            swarn("%s: compile time configuration error?  "
                  "Failed to find \"%s\" in \"%s\": %s",
                  function, SYMBOL_PT_LOCK, LIBRARY_PTHREAD, dlerror());

         if ((pt_unlock = (PT_UNLOCK_FUNC_T)dlsym(lpt, SYMBOL_PT_UNLOCK))
         == NULL)
            swarn("%s: compile time configuration error?  "
                  "Failed to find \"%s\" in \"%s\": %s",
                  function, SYMBOL_PT_UNLOCK, LIBRARY_PTHREAD, dlerror());

         if ((pt_self = (PT_SELF_FUNC_T)dlsym(lpt, SYMBOL_PT_SELF)) == NULL)
            swarn("%s: compile time configuration error?  "
                  "Failed to find \"%s\" in \"%s\": %s",
                  function, SYMBOL_PT_SELF, LIBRARY_PTHREAD, dlerror());
      }

      if (pt_init == NULL || pt_attrinit == NULL || pt_settype == NULL
      ||  pt_lock == NULL || pt_unlock   == NULL || pt_self    == NULL) {
         pt_init     = NULL;
         pt_attrinit = NULL;
         pt_settype  = NULL;
         pt_lock     = NULL;
         pt_unlock   = NULL;
         pt_self     = NULL;
      }

      if (pt_init == NULL) {
         slog(LOG_INFO, "%s: pthread locking disabled", function);
         sockscf.state.threadlockenabled = 0;
      }
      else {
         slog(LOG_INFO, "%s: pthread locking enabled", function);
         sockscf.state.threadlockenabled = 1;

         if (socks_pthread_mutexattr_init(&attr) != 0)
            serr("%s: mutexattr_init() failed", function);

         if (socks_pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)
         != 0)
            swarn("%s: mutex_settype(PTHREAD_MUTEX_ERRORCHECK) failed",
                  function);

         if (socks_pthread_mutex_init(&addrmutex, &attr) != 0) {
            swarn("%s: mutex_init() failed", function);

            if (socks_pthread_mutex_init(&addrmutex, NULL) != 0)
               serr("%s: mutex_init() failed", function);
         }
      }
#endif /* HAVE_PTHREAD_H */
   }

   inited         = 1;
   doing_addrinit = 0;
}

socks_id_t *
socks_whoami(id)
   socks_id_t *id;
{

#if HAVE_PTHREAD_H
   if (pt_self != NULL) {
      id->whichid   = thread;
      id->id.thread = pt_self();

      return id;
   }
#endif /* HAVE_PTHREAD_H */

   id->whichid = pid;
   id->id.pid = getpid();

   return id;
}

#if HAVE_PTHREAD_H
/* pthread lock wrapper functions */
static int
socks_pthread_mutex_init(mutex, attr)
   pthread_mutex_t *mutex;
   const pthread_mutexattr_t *attr;
{
   if (pt_init != NULL)
      return pt_init(mutex, attr);
   else
      return 0;
}

static int
socks_pthread_mutexattr_init(attr)
   pthread_mutexattr_t *attr;
{
   if (pt_attrinit != NULL)
      return pt_attrinit(attr);
   else
      return 0;
}

static int
socks_pthread_mutexattr_settype(attr, type)
   pthread_mutexattr_t *attr;
   int type;
{
   if (pt_settype != NULL)
      return pt_settype(attr, type);
   else
      return 0;
}

static int
socks_pthread_mutex_lock(mutex)
   pthread_mutex_t *mutex;
{
   if (pt_lock != NULL)
      return pt_lock(mutex);
   else
      return 0;
}

static int
socks_pthread_mutex_unlock(mutex)
   pthread_mutex_t *mutex;
{
   if (pt_unlock != NULL)
      return pt_unlock(mutex);
   else
      return 0;
}
#endif /* HAVE_PTHREAD_H */
