/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2019, 2020,
 *               2021, 2024
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

/* $Id: sockd.h,v 1.945.4.14.2.3.4.18.4.7 2024/11/21 10:22:41 michaels Exp $ */

#ifndef _SOCKD_H_
#define _SOCKD_H_

#if HAVE_SCHED_H
#include <sched.h>
#endif /* HAVE_SCHED_H */

#if HAVE_SOLARIS_PRIVS
#include <priv.h>
#endif /* HAVE_SOLARIS_PRIVS */

#include <sys/types.h>
#include <sys/ipc.h>
#if HAVE_SYS_SHM_H
#include <sys/shm.h>
#endif /* HAVE_SYS_SHM_H */

#include <regex.h>

/*
 * Throttling for objects we send to the monitor process for testing.
 * Each process can send objects to the monitor at the rate of one
 * object per <seconds given below>.
 */
#define SOCKD_ALARMTESTS_DELAY (60)

/*
 * number of seconds a to wait for a connect initiated on behalf of a
 * client to complete.  Can be changed in the config file.
 */
#define SOCKD_CONNECTTIMEOUT   (30)


/*
 * number of seconds a client can negotiate with server.
 * Can be changed in the config file.
 */
#define SOCKD_NEGOTIATETIMEOUT   (30)

/*
 * number of seconds a client can be connected after negotiation is completed
 * without sending/receiving any data.  Can be changed in the config file.
 */
#define SOCKD_IOTIMEOUT_TCP          (0) /* no timeout. */

#if SOCKS_SERVER
#define SOCKD_IOTIMEOUT_UDP          (86400) /* 24h */
#else /* BAREFOOTD */
#define SOCKD_IOTIMEOUT_UDP          (3600)  /* one hour */
#endif

/*
 * This is to handle a potential resource issue that can occur
 * in TCP when side 'A' of the TCP session closes it's end, but
 * the other end, side 'B', does not close it's end.  In this
 * situation, TCP will be forced to keep state for the TCP session
 * until side B closes it's end, or "never" if side B never closes.
 *
 * Some kernels have added kernel support for tuning this on a global
 * basis, but implementations may vary.
 *
 * If this value is set, it gives the number of seconds to wait
 * for B to close it's side.  Note that this may break the application
 * protocol, as there may be nothing wrong with B using a long time,
 * days even, to close it's end.  It may however produce an unfortunate
 * resource problem with both the Dante server and the kernels TCP having to
 * keep state for these sessions, which in 99.999% of the cases could
 * probably be closed as B will not send anything more.
 *
 * The default therefor is to not enable this "feature".
 */
#define SOCKD_FIN_WAIT_2_TIMEOUT  (0) /* Seconds.  Set to 0 to disable. */

/*
 * Prefer to not remove idle processes, even if possible, without
 * waiting for them to have handled at least this many clients, or
 * lived at least this long.
 * Nothing very special behind these numbers, just seems like reasonable
 * defaults at the time.  Note that they do not affect removal of idle
 * processes when we are running short of resources, then we do remove what
 * we can, regardless of these numbers.
 */

#define SOCKD_MIN_CLIENTS_HANDLED_NEGOTIATE (MAX(SOCKD_NEGOTIATEMAX * 10, 5000))
#define SOCKD_MIN_CLIENTS_HANDLED_REQUEST   (MAX(SOCKD_REQUESTMAX   * 10, 5000))
#define SOCKD_MIN_CLIENTS_HANDLED_IO        (MAX(SOCKD_IOMAX        * 10, 5000))

#define SOCKD_MIN_LIFETIME_SECONDS          (60 * 1)



#define SOCKD_SINGLEAUTH_LINGERTIMEOUT (60)  /* one minute */

#define SOCKD_CACHESTAT    (1000) /* how often to print info.     */

#define SOCKD_EXPLICIT_LDAP_PORT     (389)
#define SOCKD_EXPLICIT_LDAPS_PORT    (636)

#ifndef SOCKD_LDAP_DEADTIME
#define SOCKD_LDAP_DEADTIME          (30) /* server down cache timeout */
#endif /* SOCKD_LDAP_DEADTIME */
#define SOCKD_LDAP_SEARCHTIME        (30)
#define SOCKD_LDAP_TIMEOUT           (2)

/*
 * -1 is a valid value, so special-case.
 */
 #define LDAP_UNSET_DEBUG_VALUE   (-2)


/*
 * Depending on what kind of server we are, we will have different
 * phases to go through before we get to the i/o part.  This is
 * where we try to define the name of some generic phases, to reduce
 * the number of product-specific defines in the code.
 *
 * Also we provide wrappers to avoid linkage errors for functions that
 * are not used in different servers, to limit the amount of #ifdef's
 * in the code itself.
 */

#if BAREFOOTD
#define HAVE_NEGOTIATE_PHASE              (0)
/* XXX same as HAVE_NEGOTIATE_PHASE? */
#define HAVE_CONTROL_CONNECTION           (0)
#define HAVE_SOCKS_RULES                  (0)
#define HAVE_TCP_SUPPORT                  (1)
#define HAVE_UDP_SUPPORT                  (1)

#elif SOCKS_SERVER
#define HAVE_NEGOTIATE_PHASE              (1)
#define HAVE_CONTROL_CONNECTION           (1)
#define HAVE_SOCKS_RULES                  (1)
#define HAVE_TCP_SUPPORT                  (1)
#define HAVE_UDP_SUPPORT                  (1)

#elif COVENANT
#define HAVE_NEGOTIATE_PHASE              (1)
#define HAVE_CONTROL_CONNECTION           (0)
#define HAVE_SOCKS_RULES                  (1)
#define HAVE_TCP_SUPPORT                  (1)
#define HAVE_UDP_SUPPORT                  (0)

#else

#error "No product defined.  Who am I?"

#endif


/*
 * use caching versions, avoiding a lot of overhead.
 */
#ifndef DISABLE_GETHOSTBYNAME_CACHE

#undef gethostbyname
#define gethostbyname(name)            (cgethostbyname((name)))

#undef gethostbyaddr
#define gethostbyaddr(addr, len, type) (cgethostbyaddr((addr), (len), (type)))

#undef getnameinfo
#define getnameinfo(sa, salen, host, hostlen, serv, servlen, flags)  \
       cgetnameinfo(sa, salen, host, hostlen, serv, servlen, flags)

#endif /* DISABLE_GETHOSTBYNAME_CACHE */

/* Search for POSIX groups */
#define DEFAULT_LDAP_FILTER_GROUP "(&(memberuid=%s)(objectclass=posixgroup))"
/* Search for Active Directory groups */
#define DEFAULT_LDAP_FILTER_GROUP_AD "(&(cn=%s)(objectclass=group))"


/*
 * This is a bunch of macros that are part of the implementation that
 * makes the negotiate children able to handle reading an unlimited
 * number of concurrent requests from the clients, without blocking
 * in any read call.
 * We treat blocking on write as a fatal error as there is no normal
 * reason that should happen during the negotiation.
 */

#define INIT(length)                                                           \
   const size_t start   = state->start;      /* start of next block in mem. */ \
   const size_t end     = start + (length);  /* end of next block in mem.   */ \
   errno = 0

/*
 * Checks whether "object" has been filled with all data requested and
 * if so calls "function", if function is not NULL.
 * If "object" has not been filled it returns the number of bytes
 * that was added to object on this call, or error.
*/

#define CHECK(object, auth, nextfunction)                      \
do {                                                           \
   SASSERTX(state->reqread <= end);                            \
                                                               \
   errno = 0;                                                  \
                                                               \
   if (LEFT()) {                                               \
      ssize_t p;                                               \
                                                               \
      SASSERT(LEFT() > 0);                                     \
                                                               \
      if (LEFT() > MEMLEFT())                                  \
          OUTOFMEM();                                          \
                                                               \
      p = READ(s, LEFT(), auth);                               \
                                                               \
      if (p >= 0)                                              \
         errno = 0; /* remove any old cruft. */                \
                                                               \
      if (p <= 0) {                                            \
         if (ERRNOISTMP(errno))                                \
            return NEGOTIATE_CONTINUE;                         \
         else if (p == 0)                                      \
            return NEGOTIATE_EOF;                              \
         else                                                  \
            return NEGOTIATE_ERROR;                            \
      }                                                        \
                                                               \
      state->reqread += p;                                     \
                                                               \
      if (LEFT()) { /* read something, but not all. */         \
         errno = EWOULDBLOCK;                                  \
         return NEGOTIATE_CONTINUE;                            \
      }                                                        \
   }                                                           \
                                                               \
   OBJECTFILL((object));                                       \
                                                               \
   state->start    = end;                                      \
   state->rcurrent = nextfunction;                             \
                                                               \
   if (state->rcurrent != NULL)                                \
      return state->rcurrent(s, request, state);               \
} while (/*CONSTCOND*/0)

#define MEMLEFT()      (sizeof(state->mem) - state->reqread)

#define LEFT()   ((end) - state->reqread)
/*
 * Returns the number of bytes left to read.
 */

#define READ(s, length, auth)                    \
   (socks_recvfromn((s),                         \
                    &state->mem[state->reqread], \
                    (length),                    \
                    0,                           \
                    0,                           \
                    NULL,                        \
                    NULL,                        \
                    NULL,                        \
                    (auth)))                      \
/*
 * "s" is the descriptor to read from.
 * "length" is how much to read.
 * Returns the number of bytes read, -1 on error.
 */



#define OBJECTFILL(object)  (memmove((object), &state->mem[start], end - start))
/*
 * Fills "object" with data.
 */

/*
 * Handle out of memory error (i.e., no more space in our inbuffer).
 * In the socks case, the max length is known, so we should never
 * experience that error.
 * In the http proxy case, there is no limit, so we have to make sure
 * the size of our buffer, as set at compile time, is big enough.
 */
#if SOCKS_SERVER
#define OUTOFMEM()                                                             \
do {                                                                           \
   SERRX(LEFT());                                                              \
} while (/*CONSTCOND*/0)
#else /* COVENANT */
#define OUTOFMEM() \
do {                                                                           \
   snprintf(state->emsg, sizeof(state->emsg),                                  \
           "http request is too long (more than %lu bytes)."                   \
           "Either our compiled-in limit is too low, or the client is "        \
           "attempting something fishy",                                       \
           (unsigned long)state->reqread);                                     \
                                                                               \
   return NEGOTIATE_ERROR;                                                     \
} while (/*CONSTCOND*/0)
#endif /* COVENANT */



/*
 * computing the values to send to io_update() indicating how much
 * was read/written to/where in relation to the i/o object "_io".
 * "_io" contains the value now, while the other objects contain the
 * value before we read/wrote anything.
 */
#define DO_IOCOUNT(_src_read, _src_written, _dst_read, _dst_written, _io)     \
do {                                                                           \
   SASSERTX((_src_read)->bytes      <= (_io)->src.read.bytes);                 \
   SASSERTX((_src_read)->packets    <= (_io)->src.read.packets);               \
                                                                               \
   SASSERTX((_src_written)->bytes   <= (_io)->src.written.bytes);              \
   SASSERTX((_src_written)->packets <= (_io)->src.written.packets);            \
                                                                               \
   SASSERTX((_dst_read)->bytes      <= (_io)->dst.read.bytes);                 \
   SASSERTX((_dst_read)->packets    <= (_io)->dst.read.packets);               \
                                                                               \
   SASSERTX((_dst_written)->bytes   <= (_io)->dst.written.bytes);              \
   SASSERTX((_dst_written)->packets <= (_io)->dst.written.packets);            \
                                                                               \
   (_src_read)->bytes   = (_io)->src.read.bytes   - (_src_read)->bytes;        \
   (_src_read)->packets = (_io)->src.read.packets - (_src_read)->packets;      \
                                                                               \
   (_src_written)->bytes = (_io)->src.written.bytes - (_src_written)->bytes;   \
   (_src_written)->packets                                                     \
   = (_io)->src.written.packets - (_src_written)->packets;                     \
                                                                               \
   (_dst_read)->bytes   = (_io)->dst.read.bytes   - (_dst_read)->bytes;        \
   (_dst_read)->packets = (_io)->dst.read.packets - (_dst_read)->packets;      \
                                                                               \
   (_dst_written)->bytes = (_io)->dst.written.bytes - (_dst_written)->bytes;   \
   (_dst_written)->packets                                                     \
   = (_io)->dst.written.packets - (_dst_written)->packets;                     \
} while (/* CONSTCOND */ 0)

/* reset a throttle object. */
#define RESET_THROTTLE(object, timenow)                                        \
do {                                                                           \
   (object)->starttime  = (timenow);                                           \
   (object)->newclients = (0);                                                 \
} while (/*CONSTCOND*/ 0)


/* copies all monitor-related fields. */
#define COPY_MONITORFIELDS(from, to)                                           \
do {                                                                           \
   (to)->mstats               = (from)->mstats;                                \
   (to)->mstats_shmid         = (from)->mstats_shmid;                          \
   (to)->mstats_isinheritable = (from)->mstats_isinheritable;                  \
   (to)->alarmsconfigured     = (from)->alarmsconfigured;                      \
} while (/* CONSTCOND */ 0)


/* clears monitor-related fields that should be cleared. */
#define CLEAR_MONITORFIELDS(object)                                            \
do {                                                                           \
   (object)->mstats               = NULL;                                      \
   (object)->mstats_shmid         = 0;                                         \
   (object)->mstats_isinheritable = 1; /* always */                            \
   /* not this one: (object)->alarmsconfigured     = 0; */                     \
} while (/* CONSTCOND */ 0)


/*
 *
 * Macros related to shmem handling.
 *
 */

/*
 * Mostly used for asserts.  Check if something is set when nothing should
 * be set.
 */

#define SHMID_ISSET(object) \
   ((object)->bw_shmid || (object)->mstats_shmid || (object)->ss_shmid)

#define SHMID_ISATTACHED(object) \
   ((object)->bw != NULL || (object)->mstats != NULL || (object)->ss != NULL)

#define INIT_MSTATES(object, _type, _number)                                   \
do {                                                                           \
   if ((object)->bw != NULL)                                                   \
      INIT_MSTATE(&((object)->bw->mstate), (_type), (_number));                \
                                                                               \
   if ((object)->ss != NULL)                                                   \
      INIT_MSTATE(&((object)->ss->mstate), (_type), (_number));                \
                                                                               \
   if ((object)->mstats != NULL)                                               \
      INIT_MSTATE(&((object)->mstats->mstate), (_type), (_number));            \
} while (/* CONSTCOND */ 0)

#define INIT_MSTATE(mstate, _parenttype, _number)                              \
do {                                                                           \
   (mstate)->number     = (_number);                                           \
   (mstate)->parenttype = (_parenttype);                                       \
} while (/* CONSTCOND */ 0)


#if DO_SHMEMCHECK

#define MPROTECT_SHMEMHEADER(shmemobject)                                      \
do {                                                                           \
   SASSERTX((shmemobject)->mstate.shmid != 0);                                 \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: doing mprotect on address %p",  function, (shmemobject));         \
                                                                               \
   SASSERT(mprotect((shmemobject),                                             \
           sizeof(shmemobject)->mstate,                                        \
           PROT_READ) == 0);                                                   \
                                                                               \
   SASSERTX((shmemobject)->mstate.shmid != 0);                                 \
} while (/* CONSTCOND */ 0)

#define MUNPROTECT_SHMEMHEADER(shmemobject)                                    \
do {                                                                           \
   SASSERTX((shmemobject)->mstate.shmid != 0);                                 \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: doing munprotect on address %p",  function, (shmemobject));       \
                                                                               \
   SASSERT(mprotect((shmemobject),                                             \
                    sizeof(shmemobject)->mstate,                               \
                    PROT_READ | PROT_WRITE) == 0);                             \
                                                                               \
   SASSERTX((shmemobject)->mstate.shmid != 0);                                 \
} while (/* CONSTCOND */ 0)


#else  /* !DO_SHMEMCHECK */

#define MPROTECT_SHMEMHEADER(shmemobject)   do { } while (/* CONSTCOND */ 0)
#define MUNPROTECT_SHMEMHEADER(shmemobject) do { } while (/* CONSTCOND */ 0)

#endif /* !DO_SHMEMCHECK */

#if DEBUG || DIAGNOSTIC
/*
 * memory-mapped file contents may not be saved in coredumps.
 */
#define SHMEM_COPYOBJECT(src, dst) shmem_object_t (dst) = (*src)

#else

#define SHMEM_COPYOBJECT(src, dst)                                             \
do {                                                                           \
   SASSERTX(src != NULL);                                                      \
} while (/*CONSTCOND*/ 0)


#endif /* DEBUG */

/*
 * copies shmem stuff from rule "src" to rule "dst".
 */
#define SHMEM_MOVE(src, dst, objects)                                          \
do {                                                                           \
   SHMEM_COPY(src, dst, objects);                                              \
   SHMEM_CLEAR(src, objects, 1);                                               \
} while (/*CONSTCOND*/ 0)

/*
 * copies shmem stuff from rule "src" to rule "dst".
 */
#define SHMEM_COPY(src, dst, objects)                                          \
do {                                                                           \
   if (objects & SHMEM_BW) {                                                   \
      (dst)->bw       = (src)->bw;                                             \
      (dst)->bw_shmid = (src)->bw_shmid;                                       \
   }                                                                           \
                                                                               \
   if (objects & SHMEM_MONITOR) {                                              \
      COPY_MONITORFIELDS((src), (dst));                                        \
   }                                                                           \
                                                                               \
   if (objects & SHMEM_SS) {                                                   \
      (dst)->ss       = (src)->ss;                                             \
      (dst)->ss_shmid = (src)->ss_shmid;                                       \
   }                                                                           \
} while (/*CONSTCOND*/ 0)



#define HANDLE_SHMAT(object, memfield, idfield)                                \
do {                                                                           \
   char *fname;                                                                \
   int fd;                                                                     \
                                                                               \
   SASSERTX((object)->memfield == NULL);                                       \
                                                                               \
   if ((object)->idfield == 0)                                                 \
      break;                                                                   \
                                                                               \
   fname = sockd_getshmemname((object)->idfield, key_unset);                   \
   if ((fd = open(fname, O_RDWR)) == -1) {                                     \
      int failureisok;                                                         \
                                                                               \
      if (sockscf.state.mother.s == -1 || sockd_motherexists() == 0)           \
         failureisok = 1; /* presumably mother exited and deleted shmemfiles */\
      else                                                                     \
         failureisok = 0;                                                      \
                                                                               \
      slog(failureisok ? LOG_DEBUG: LOG_WARNING,                               \
           "%s: failed to open %s for attaching to " #idfield " %ld: %s",      \
           function, fname, (object)->idfield, strerror(errno));               \
                                                                               \
      (object)->memfield = NULL;                                               \
      (object)->idfield  = 0;                                                  \
      break;                                                                   \
   }                                                                           \
                                                                               \
   (object)->memfield = sockd_mmap(NULL,                                       \
                                   sizeof(*(object)->memfield),                \
                                   PROT_READ | PROT_WRITE,                     \
                                   MAP_SHARED,                                 \
                                   fd,                                         \
                                   0);                                         \
                                                                               \
   close(fd);                                                                  \
                                                                               \
   if ((object)->memfield == MAP_FAILED) {                                     \
      swarn("%s: failed to mmap " #idfield " shmem segment %ld of size %ld "   \
            "from file %s",                                                    \
            function,                                                          \
            (object)->idfield,                                                 \
            (unsigned long)sizeof(*(object)->memfield),                        \
            fname);                                                            \
                                                                               \
      (object)->memfield = NULL;                                               \
      (object)->idfield  = 0;                                                  \
      break;                                                                   \
   }                                                                           \
                                                                               \
   MPROTECT_SHMEMHEADER((object)->memfield);                                   \
                                                                               \
   SHMEM_COPYOBJECT((object)->memfield, _shmem);                               \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: attached to " #idfield " %ld of size %lu at %p, %lu clients, "    \
        "filename %s",                                                         \
        function,                                                              \
        (object)->idfield,                                                     \
        (unsigned long)sizeof(*(object)->memfield),                            \
        (object)->memfield,                                                    \
        (unsigned long)(object)->memfield->mstate.clients, fname);             \
                                                                               \
   SASSERTX((object)->idfield == (object)->memfield->mstate.shmid);            \
} while (/* CONSTCOND */ 0)

#define HANDLE_SHMDT(object, memfield, idfield)                                \
do {                                                                           \
   SASSERTX((object)->idfield  != 0);                                          \
   SASSERTX((object)->memfield != NULL);                                       \
                                                                               \
   SHMEM_COPYOBJECT((object)->memfield, _shmem);                               \
                                                                               \
   SASSERTX((object)->idfield == (object)->memfield->mstate.shmid);            \
                                                                               \
   slog(LOG_DEBUG,                                                             \
        "%s: detaching from " #idfield " %ld at %p with %lu clients",          \
        function,                                                              \
        (object)->idfield,                                                     \
        (object)->memfield,                                                    \
        (unsigned long)(object)->memfield->mstate.clients);                    \
                                                                               \
   if (munmap((object)->memfield, sizeof(*(object)->memfield)) != 0)           \
      swarn("%s: munmap of " #idfield " shmem segment %ld (%p) failed",        \
            function, (object)->idfield, (object)->memfield);                  \
                                                                               \
   (object)->memfield = NULL;                                                  \
} while (/* CONSTCOND */ 0)


/*
 * Unuses (decrements by one) any shared memory in use by "rule" for the
 * client address "addr", indicating one client-session has been closed.
 * If necessary, temporarily attaches to the shmem segments to do it,
 * before always detaching before returning.
 *
 */
#define SHMEM_UNUSE(rule, cinfo, lock, objects)                                \
do {                                                                           \
   int need_attach = SHMEM_NONE, detach_from = SHMEM_NONE;                     \
                                                                               \
   /*                                                                          \
    * First attach to everything we need to attach to ...                      \
    */                                                                         \
                                                                               \
   if (((objects) & SHMEM_BW) && (rule)->bw_shmid != 0) {                      \
      if ((rule)->bw == NULL)                                                  \
         need_attach |= SHMEM_BW;                                              \
                                                                               \
      detach_from |= SHMEM_BW;                                                 \
   }                                                                           \
                                                                               \
   if (((objects) & SHMEM_MONITOR) && (rule)->mstats_shmid != 0) {             \
      if ((rule)->mstats == NULL)                                              \
         need_attach |= SHMEM_MONITOR;                                         \
                                                                               \
      detach_from |= SHMEM_MONITOR;                                            \
   }                                                                           \
                                                                               \
   if (((objects) & SHMEM_SS) && (rule)->ss_shmid != 0) {                      \
      if ((rule)->ss == NULL)                                                  \
         need_attach |= SHMEM_SS;                                              \
                                                                               \
      detach_from |= SHMEM_SS;                                                 \
   }                                                                           \
                                                                               \
   if (need_attach != SHMEM_NONE)                                              \
      (void)sockd_shmat((rule), need_attach);                                  \
                                                                               \
   /*                                                                          \
    * ... and then unuse it.                                                   \
    */                                                                         \
                                                                               \
   if (((objects) & SHMEM_BW) && (rule)->bw_shmid != 0) {                      \
      SASSERTX((rule)->bw != NULL);                                            \
                                                                               \
      slog(LOG_DEBUG, "%s: unusing bw_shmid %lu at %p, cinfo %s, clients %lu", \
           function,                                                           \
           (unsigned long)(rule)->bw_shmid,                                    \
           (rule)->bw,                                                         \
           clientinfo2string(cinfo, NULL, 0),                                  \
           (unsigned long)(rule)->bw->mstate.clients);                         \
                                                                               \
      bw_unuse((rule)->bw, cinfo, lock);                                       \
   }                                                                           \
                                                                               \
   if (((objects) & SHMEM_MONITOR) && (rule)->mstats_shmid != 0) {             \
      SASSERTX((rule)->mstats != NULL);                                        \
                                                                               \
      slog(LOG_DEBUG,                                                          \
           "%s: unusing mstats_shmid %lu, cinfo %s, clients %lu",              \
           function,                                                           \
           (unsigned long)(rule)->mstats_shmid,                                \
           clientinfo2string(cinfo, NULL, 0),                                  \
           (unsigned long)(rule)->mstats->mstate.clients);                     \
                                                                               \
      monitor_unuse((rule)->mstats, cinfo, lock);                              \
   }                                                                           \
                                                                               \
   if (((objects) & SHMEM_SS) && (rule)->ss_shmid != 0) {                      \
      SASSERTX((rule)->ss != NULL);                                            \
                                                                               \
      slog(LOG_DEBUG, "%s: unusing ss_shmid %lu, cinfo %s, clients %lu",       \
           function,                                                           \
           (unsigned long)(rule)->ss_shmid,                                    \
           clientinfo2string(cinfo, NULL, 0),                                  \
           (unsigned long)(rule)->ss->mstate.clients);                         \
                                                                               \
      session_unuse((rule)->ss, cinfo, lock);                                  \
   }                                                                           \
                                                                               \
   if ((need_attach | detach_from) != SHMEM_NONE)                              \
      sockd_shmdt((rule), need_attach | detach_from);                          \
} while (/*CONSTCOND*/0)

/*
 * clears shmem stuff in "rule".
 */
#define SHMEM_CLEAR(rule, objects, idtoo)                                      \
do {                                                                           \
   if (objects & SHMEM_BW) {                                                   \
      (rule)->bw = NULL;                                                       \
                                                                               \
      if (idtoo)                                                               \
         (rule)->bw_shmid = 0;                                                 \
   }                                                                           \
                                                                               \
   if (objects & SHMEM_MONITOR) {                                              \
      const unsigned long shmid     = (rule)->mstats_shmid;                    \
      const size_t alarmsconfigured = (rule)->alarmsconfigured;                \
                                                                               \
      CLEAR_MONITORFIELDS((rule));                                             \
                                                                               \
      if (!idtoo) {                                                            \
         (rule)->mstats_shmid     = shmid;                                     \
         (rule)->alarmsconfigured = alarmsconfigured;                          \
      }                                                                        \
   }                                                                           \
                                                                               \
   if (objects & SHMEM_SS) {                                                   \
      (rule)->ss = NULL;                                                       \
                                                                               \
      if (idtoo)                                                               \
         (rule)->ss_shmid = 0;                                                 \
   }                                                                           \
} while (/*CONSTCOND*/0)

#define UNLOCK(lock)                                                           \
do {                                                                           \
   if (lock != -1)                                                             \
      socks_unlock(lock);                                                      \
} while (/*CONSTCOND*/0)

#if BAREFOOTD
#define ALL_UDP_BOUNCED()                                                      \
   (  sockscf.state.alludpbounced                                              \
    || pidismother(sockscf.state.pid) != 1 /* only main mother bounces. */)
#endif /* BAREFOOTD */

/*
 * build a string for the source and one for the destination that can
 * be used in iolog() and similar for logging the address related to
 * something.
 */
#define MAX_IOLOGADDR                                                          \
  (MAXSOCKADDRSTRING  + sizeof(" ")                  /* local */               \
 + MAXAUTHINFOLEN + MAXSOCKSHOSTSTRING + sizeof(" ") /* proxy */               \
 + MAXSOCKSHOSTSTRING + sizeof(" ")                  /* proxy's ext addr. */   \
 + MAXAUTHINFOLEN + MAXSOCKSHOSTSTRING + sizeof(" ") /* peer  */               \
 + (MAXSOCKADDRSTRING + sizeof(" ") * HAVE_MAX_HOSTIDS))

#if HAVE_SOCKS_HOSTID

#define CRULE_OR_HRULE(object)                                                 \
   ((object)->hrule_isset ? &((object)->hrule) : &((object)->crule))

#if HAVE_MAX_HOSTIDS == 0

#error "HAVE_MAX_HOSTIDS cannot be zero"

#endif /* HAVE_MAX_HOSTIDS == 0 */

#define DEFAULT_HOSTINDEX (1)

#else /* !HAVE_SOCKS_HOSTID */

#define CRULE_OR_HRULE(object)  (&((object)->crule))

#endif /* !HAVE_SOCKS_HOSTID */

/*
 * The rule that contains the bandwidth info.
 * For products with socks rules, the shmem-objects are eventually
 * inherited by the socks-rule.  For other products, they are eventually
 * inherited by the hostid-rule, if present, or client-rule, it not present.
 */


#if HAVE_SOCKS_RULES
#define SHMEMRULE(io)      (&((io)->srule))

#else /* !HAVE_SOCKS_RULES */
#define SHMEMRULE(io)      CRULE_OR_HRULE((io))

#endif /* !HAVE_SOCKS_RULES */

#define IORULE(io) SHMEMRULE(io)

/*
 * Some sessions have a separate control-connection, while others just
 * have one connection that serves as both the client connection and the
 * control connection.
 */
#define CONTROLIO(io) ((io)->control.s != -1 ?                                 \
   &((io)->control)                                                            \
:  (((io)->state.command == SOCKS_BINDREPLY) ? &((io)->dst) :  &((io)->src)))

#define INTERNALIO(io)                                                         \
(((io)->state.command == SOCKS_BINDREPLY) ? &((io)->dst) :  &((io)->src))

#define CLIENTIO(io) INTERNALIO(io)

#define EXTERNALIO(io)                                                         \
(((io)->state.command == SOCKS_BINDREPLY) ? &((io)->src) :  &((io)->dst))

#define TARGETIO(io) EXTERNALIO(io)

/*
 * info sent by sockd children to mother.
 */
#define SOCKD_NOP                (0)   /* No command/op.       */
#define SOCKD_FREESLOT_TCP       (1)   /* free'd a tcp slot.   */
#define SOCKD_FREESLOT_UDP       (2)   /* free'd a udp slot.   */

/*
 * info sent by mother to sockd children.
 */
#define SOCKD_EXITNORMALLY       (1)

/*
 * a request child can currently handle a maximum of one client, so can't
 * be changed and is therefor #defined here rather than in config.h.
 */
#define SOCKD_REQUESTMAX   1

/*
 * Max number of tests a monitor process can have queued.
 */
#define MAX_TESTS_QUEUED (256)

/*
 * types of child-processes.
 */
#define PROC_MOTHER        (0)    /* 0 so that it's correct pre-init too. */
#define PROC_MONITOR       (1)
#define PROC_NEGOTIATE     (2)
#define PROC_REQUEST       (3)
#define PROC_IO            (4)
#define PROC_NOTOURS       (255)

#if SOCKS_SERVER
#define FDPASS_MAX          3   /* max number of descriptors we send/receive. */
#else
#define FDPASS_MAX          2   /* max number of descriptors we send/receive. */
#endif

/*
 * If we are unable to add a new child due to lack of resources, this is the
 * max time to wait before trying again.
 */
#define MAX_ADDCHILD_SECONDS (10)


/* how long the alarm period should last by default. */
#define DEFAULT_ALARM_PERIOD     (5)

/*
 * alarmsides.  Bitmask.
 */
#define ALARM_INTERNAL           (1)
#define ALARM_INTERNAL_RECV      (2)
#define ALARM_INTERNAL_SEND      (4)
#define ALARM_EXTERNAL           (8)
#define ALARM_EXTERNAL_RECV      (16)
#define ALARM_EXTERNAL_SEND      (32)
#define ALARM_RECV               (64)
#define ALARM_SEND               (128)

/* alarmtypes set. Bitmask. */
#define ALARM_DATA               (1)
#define ALARM_DISCONNECT         (2)
#define ALARM_TEST               (4)
#define ALARM_PROTOCOL           (8)


   /*
    * config stuff
    */

#define VERDICT_BLOCKs     "block"
#define VERDICT_PASSs      "pass"

/* how to rotate addresses. */
#define ROTATION_NOTSET     0	/* illegal value. */
#define ROTATION_NONE       1
#define ROTATION_ROUTE      2
#define ROTATION_SAMESAME   3

#define SOCKS_LOG_CONNECTs       "connect"
#define SOCKS_LOG_DISCONNECTs    "disconnect"
#define SOCKS_LOG_DATAs          "data"
#define SOCKS_LOG_ERRORs         "error"
#define SOCKS_LOG_IOOPERATIONs   "iooperation"
#define SOCKS_LOG_TCPINFOs       "tcpinfo"

typedef enum { addrscope_global,
               addrscope_nodelocal,
               addrscope_linklocal } ipv6_addrscope_t;


/*
 * privilege stuff.
 */
#if !HAVE_PRIVILEGES
typedef enum { PRIV_ON, PRIV_OFF } priv_op_t;
#endif /* !HAVE_PRIVILEGES */

typedef enum { SOCKD_PRIV_NOTSET = 0,
               SOCKD_PRIV_FILE_READ,
               SOCKD_PRIV_FILE_WRITE,
               SOCKD_PRIV_NET_ADDR,
               SOCKD_PRIV_NET_ICMPACCESS,
               SOCKD_PRIV_NET_ROUTESOCKET,
               SOCKD_PRIV_PRIVILEGED,
               SOCKD_PRIV_UNPRIVILEGED,
               SOCKD_PRIV_LIBWRAP,
               SOCKD_PRIV_PAM,
               SOCKD_PRIV_BSDAUTH,
               SOCKD_PRIV_GSSAPI
} privilege_t;

typedef enum { IO_NOERROR,
               IO_TMPERROR,          /* temporary error; try again later.     */
               IO_IOERROR,           /* a network error.                      */
               IO_ERROR,             /* non-network error.                    */
               IO_EAGAIN,            /* no data available to read currently.  */
               IO_TIMEOUT,
               IO_CLOSE,
               IO_BLOCK,
               IO_TMPBLOCK,
               IO_ADMINTERMINATION
} iostatus_t;

#define IOSTATUS_FATALERROR(error)       \
(!  (  (error) == IO_NOERROR             \
    || (error) == IO_TMPERROR            \
    || (error) == IO_TMPBLOCK            \
    || (error) == IO_EAGAIN))

#if SOCKS_SERVER

#define IOSTATUS_UDP_SEND_FAILED(e)                                            \
(IO_TMPERROR)

#else /* !SOCKS_SERVER */

#define IOSTATUS_UDP_SEND_FAILED(e)                                            \
(((e) == EMSGSIZE || ERRNOISTMP(e)) ? IO_TMPERROR : IO_ERROR)

#endif /* !SOCKS_SERVER */


typedef enum {
   OPERATION_ACCEPT,
   OPERATION_BLOCK,        /* session blocked and closed.             */
   OPERATION_TMPBLOCK,     /* packet blocked, but session not closed. */
   OPERATION_CONNECT,
   OPERATION_DISCONNECT,
   OPERATION_ERROR,        /* session failed and closed.              */
   OPERATION_TMPERROR,     /* packet failed but session not closed.   */
   OPERATION_HOSTID,
   OPERATION_IO
} operation_t;

typedef enum { KEY_IPV4 = 1, KEY_IPV6, KEY_MAC, KEY_TIME } keytype_t;
typedef enum { ACKPIPE, DATAPIPE } whichpipe_t;
typedef enum { NEGOTIATE_EOF,
               NEGOTIATE_ERROR,     /* fatal error, wrong auth, etc.          */
               NEGOTIATE_CONTINUE,  /* have not finished, do continue.        */
               NEGOTIATE_FINISHED   /* have finished, read request ok so far. */
} negotiate_result_t;

#define fakesockaddr2sockshost sockaddr2sockshost/* no fakes in server. */

/* ok signals, i.e signals that do not indicate an error. */
#if HAVE_SIGNAL_SIGINFO
#define SIGNALISOK(sig) \
   (  (sig) == SIGHUP   \
   || (sig) == SIGINT   \
   || (sig) == SIGUSR1  \
   || (sig) == SIGINFO  \
   || (sig) == SIGQUIT  \
   || (sig) == SIGTERM  \
   || (sig) == SIGHUP)
#else /* !HAVE_SIGNAL_SIGINFO */
#define SIGNALISOK(sig) \
   (  (sig) == SIGHUP   \
   || (sig) == SIGINT   \
   || (sig) == SIGUSR1  \
   || (sig) == SIGQUIT  \
   || (sig) == SIGTERM  \
   || (sig) == SIGHUP)
#endif

/*
 * Called at the start of all signalhandlers.
 */

#define SIGNAL_PROLOGUE(sig, si, saved_errno)                                  \
do {                                                                           \
   char _b[1][32];                                                             \
   const char *_msgv[]                                                         \
   = { signal2string(-sig),                                                    \
       " [: processing signal ",                                               \
       ltoa((-sig), _b[0], sizeof(_b[0])),                                     \
       NULL                                                                    \
   };                                                                          \
                                                                               \
   if ((sig) > 0) {                                                            \
      const int old_insignal = sockscf.state.insignal;                         \
                                                                               \
      sockscf.state.insignal = (sig);                                          \
      sockd_pushsignal(sig, si);                                               \
      sockscf.state.insignal = old_insignal;                                   \
                                                                               \
      errno = (saved_errno);                                                   \
      return;                                                                  \
   }                                                                           \
                                                                               \
   (sig) = -(sig);                                                             \
   signalslog(LOG_DEBUG, _msgv);                                               \
} while (/* CONSTCOND */ 0)

#define SIGNAL_EPILOGUE(sig, si, saved_errno)                                  \
do {                                                                           \
   char _b[1][32];                                                             \
   const char *_msgv[]                                                         \
   = { signal2string(sig),                                                     \
       " ]: finished processing signal ",                                      \
       ltoa((sig), _b[0], sizeof(_b[0])),                                      \
       NULL                                                                    \
   };                                                                          \
                                                                               \
   signalslog(LOG_DEBUG, _msgv);                                               \
                                                                               \
   errno = (saved_errno);                                                      \
} while (/* CONSTCOND */ 0)

#if HAVE_UDP_SUPPORT

#define MIN_IPHLEN  (20)
#define MAX_IPHLEN  (60)

#define MIN_UDPLEN   (8)

#define MAX_ICMPUNREACHLEN (8 + MAX_IPHLEN + MIN_UDPLEN)
#define MIN_ICMPUNREACHLEN (8 + MIN_IPHLEN + MIN_UDPLEN)

#define ICMP_TYPE_TTLEXCEEDED (11)
#define ICMP_CODE_TTLEXCEEDED_TRANSIT      (1)

#define ICMP_TYPE_UNREACHABLE (3)
#define ICMP_CODE_UNREACHABLE_HOST             (1)
#define ICMP_CODE_UNREACHABLE_PORT             (3)
#define ICMP_CODE_UNREACHABLE_DESTHOSTUNKNOWN  (7)
#define ICMP_CODE_UNREACHABLE_HOSTPROHIBITED   (13)

#define IOOP(isblocked, iostatus)                                              \
   /* coverity[dead_error_begin] */                                            \
   ((isblocked) ?                                                              \
      (SOCKS_SERVER ? OPERATION_TMPBLOCK : OPERATION_BLOCK)                    \
   /* coverity[dead_error_begin] */                                            \
  : ((iostatus) == IO_TMPERROR ?  OPERATION_TMPERROR : OPERATION_ERROR))



typedef enum { RAWSOCKET_NOP,          /* nothing to care about.  */
               RAWSOCKET_IO_DELETED    /* an io was deleted.      */
} rawsocketstatus_t;


#define SOCKD_IO_PACKETSTATS           (128)
typedef struct {
   struct timeval latencyv[SOCKD_IO_PACKETSTATS];
   size_t         lastlatencyi;   /* index of last ts added.                  */

   size_t         latencyc;       /* current number of ts's in latencyv.      */

   /* min/max/last/etc. observed latency. */
   unsigned long   min_us;
   unsigned long   max_us;
   unsigned long   last_us;
   unsigned long   average_us;
   unsigned long   median_us;
   unsigned long   stddev_us;
} iostat_t;

#endif /* HAVE_UDP_SUPPORT */

typedef enum { TEST_MTU__WAITING_FOR_KEEPALIVE_ACK1 = 1,
               TEST_MTU__WAITING_FOR_KEEPALIVE_ACK2
} mtu_error_test_state_t;

typedef struct {
   int              tcp_keepalive;
   int              tcp_keepidle;

#if HAVE_TCP_INFO
   struct tcp_info  tcpinfo;
#endif /* HAVE_TCP_INFO */
} mtu_test_state_data_t;

typedef struct {
   struct timeval         start;       /* time we started checking.           */

   mtu_error_test_state_t state;       /* current state.                      */
   struct timeval         nextcheck;   /* time next check should be done.     */

   mtu_test_state_data_t initial;
   mtu_test_state_data_t current;
} mtu_test_state_t;

typedef struct {
   unsigned         dotest;      /* perform mtu test? */
   mtu_test_state_t state;       /* ... state if so.                  */
} mtutest_t;

typedef struct {
   mtutest_t   mtu;
} networktest_t;

/*
 * Similar to networktest_t, but without the state object.
 */
typedef struct {
   struct {
      unsigned tested;
   } mtu;
} networktest_tested_t;


typedef struct {
   unsigned char sameport;     /* always try to use same port as client?   */
   unsigned char draft_5_05;   /* try to support parts of socks 5.05 draft */
} compat_t;

typedef struct {
   unsigned char connect;
   unsigned char disconnect;
   unsigned char data;
   unsigned char error;
   unsigned char iooperation;
   unsigned char tcpinfo;
} log_t;

typedef struct {
   struct sockaddr_storage from; /* clients address.                          */
   struct hostid           hostid;
} clientinfo_t;

typedef enum { key_unset = 0, key_from, key_hostid } statekey_t;

typedef struct {
   /*
    * These variables are fixed at creation and are after that never to
    * be changed.
    */
   unsigned long      shmid;            /* shmid of this object.              */
   size_t             number;           /* rule/monitor # this object is for. */
   objecttype_t       parenttype;       /*
                                         * type of object this object belongs
                                         * to (rule/monitor/etc.).
                                         */

   size_t             clients;          /* # of clients using this object.    */
} shmem_header_t;

typedef struct {
   size_t              bytes;             /* bytes transferred since iotime.  */
   struct timeval      iotime;

   unsigned char       maxbps_isset;
   size_t              maxbps;            /* max  b/s allowed.                */
} bw_t;

typedef struct {
   /* max number of "clients" to accept within "seconds". */
   size_t  clients;
   time_t  seconds;
} sessionthrottle_t;

typedef struct {
   unsigned char max_isset;
   size_t        max;      /*
                            * max number of sessions allowed in total.
                            * -1 if there is no total limit for
                            * this object, only per-state limits.
                            */

   unsigned char throttle_isset;
   struct {
      sessionthrottle_t limit;
      struct timeval    starttime; /* time we last reset the client counter.  */
      size_t            newclients;/* new clients since we reset the counter. */
   } throttle;

   /*
    * Same as the above, but on a per-state basis, if key is set.
    */

   size_t            max_perstate;
   unsigned char     max_perstate_isset;

   unsigned char     throttle_perstate_isset;
   struct {
      sessionthrottle_t limit;
   } throttle_perstate;
} session_t;

typedef struct {
      statekey_t       key;

      /*
       * The rest is only applicable if key is not "key_unset".
       */

     time_t lastexpirescan;   /* time of last scan for entries to expire.     */

      /* extra info needed for some keys. */
      union {
         unsigned char hostindex;     /* index of hostid saved/to use.        */
      } keyinfo;

      /*
       * NOTE: we use a separate memory mapping for this and that must
       * always be temporary; we can map it after getting the lock, but
       * must release and NULL the pointer before releasing the lock.
       * The reason for this is that multiple processes will map this
       * object, and if one of the processes also maps they keymap,
       * the other processes will see that the keymap pointer is no
       * longer NULL, but that other process will not (yet) have mapped
       * the keymap.
       */
      struct {
         union { /* data for the key. */
            struct {
               sa_family_t        safamily; /* which union member is set.     */
               union {
                  struct in_addr  ipv4;
                  struct in6_addr ipv6;
               }                 addr;     /* address of this client.         */

               size_t            addrc;    /* # of clients from address.      */
            } from;

            struct {
               struct in_addr  ipv4;
               size_t          addrc;    /* # of clients from address.      */
            } hostid;
         } data;

         union { /* extra info for some keys/data. */
            struct {
               struct timeval starttime;
               size_t         newclients;
            } ss; /* for session-limits. */
         } info;

      } *keyv;
      size_t keyc;                  /* number of elements in keyv.            */
} keystate_t;

typedef struct {
   /* must transfer at least <bytes> bytes every <seconds> seconds. */
   size_t bytes;
   time_t seconds;
} alarm_data_limit_t;

typedef struct {
   unsigned char      isconfigured; /* alarm configured for this side?        */

   unsigned char      ison;         /* alarm is currently on?                 */

   struct timeval     alarmchange;  /* last time alarm switched status.       */

   size_t             bytes;        /* bytes transferred since last reset.    */
   struct timeval     lastio;       /* time of last i/o.                      */

   struct timeval     lastreset;    /* timestamp of last reset.               */

   alarm_data_limit_t limit;
} alarm_data_side_t;

typedef struct {
   alarm_data_side_t recv;      /* received from interface.                   */
   alarm_data_side_t send;      /* sent to interface.                         */
} alarm_data_t;

typedef struct {
   /*
    * if <disconnectc> or more clients or targets have disconnected during
    * the last <seconds> seconds, and the ratio of disconnects to the
    * current number of sessions is equal or higher than the limit set,
    * trigger the alarm.
    */
   size_t sessionc;
   size_t disconnectc;
   time_t seconds;
} alarm_disconnect_limit_t;

typedef struct {
   unsigned char  isconfigured;  /* disconnect-alarm configured.              */

   struct timeval lastreset;     /* time of last reset.                       */
   size_t         sessionc;      /* number of sessions currently established. */

   /*
    * number of disconnects done by peer and number of disconnects done by us
    * /since last reset/.
    */
   size_t         peer_disconnectc;
   size_t         self_disconnectc;

   alarm_disconnect_limit_t limit;
} alarm_disconnect_t;

typedef struct {
   struct {
      alarm_data_t       data;
      alarm_disconnect_t disconnect;

      /*
       * tests to perform.
       */
      networktest_t        test;
   } alarm;
} monitor_if_t;

typedef struct {
   monitor_if_t   internal;
   monitor_if_t   external;
} monitor_stats_t;

/* which shmem object.  Bitmask. */
#define SHMEM_NONE                   (0x0)
#define SHMEM_BW                     (0x1)
#define SHMEM_MONITOR                (0x2)
#define SHMEM_SS                     (0x4)
#define SHMEM_ALL                    (SHMEM_BW | SHMEM_MONITOR | SHMEM_SS)

typedef struct {
   shmem_header_t     mstate;
   keystate_t         keystate;

   size_t             type; /* which object in union this object is for. */
   union {
      bw_t            bw;
      session_t       ss;
      monitor_stats_t monitor;
   } object;
} shmem_object_t;


typedef struct {
   keytype_t key;

   union {
      struct in_addr   ipv4;
      struct in6_addr  ipv6;
      unsigned char     macaddr[ETHER_ADDR_LEN];
      time_t           time;
   } value;
} licensekey_t;

typedef struct monitor_t {
   objecttype_t     type;

   shmem_object_t   *mstats;
   unsigned long    mstats_shmid;
   unsigned char    mstats_isinheritable;

   /*
    * Should we aggregate counters on interface/sides?  Bitmask set if so.
    */
   size_t           alarm_data_aggregate;
   size_t           alarm_disconnect_aggregate;


   /*
    * used as an optimization to avoid needlessly attach/detach to figure out
    * the alarm is not relevant to the current action.
    */
   size_t           alarmsconfigured;           /* bitmask.                   */

   ruleaddr_t       src;
   ruleaddr_t       dst;

   unsigned char    hostidoption_isset;   /* any of the values below set?     */

#if HAVE_SOCKS_HOSTID

   ruleaddr_t       hostid;         /*
                                     * if atype is not SOCKS_ADDR_NOTSET,
                                     * this rule requires a matching hostid.
                                     */

   unsigned char    hostindex; /*
                                * address index to match hostid against.
                                * 0 means any, 1 means first index, etc.
                                */

#endif /* HAVE_SOCKS_HOSTID */

   size_t                  number;       /* rulenumber.                       */
   size_t                  linenumber;   /* linenumber; info/debugging only.  */

   serverstate_t           state;

   struct monitor_t        *next;          /* next monitor in list.           */
} monitor_t;

typedef struct {
   unsigned char              isconfigured;  /* any options here set?    */

   unsigned                   ecn;
   int                        ecn_loglevel;

   unsigned                   sack;
   int                        sack_loglevel;

   unsigned                   timestamps;
   int                        timestamps_loglevel;

   unsigned                   wscale;
   int                        wscale_loglevel;
} warn_protocol_tcp_options_t;

typedef struct {
   struct {
      warn_protocol_tcp_options_t enabled;
      warn_protocol_tcp_options_t disabled;
   } tcp;
} warn_protocol_t;



typedef struct {
   /*
    * Contains list of errno-values that should be logged additionally,
    * at loglevel LOG_EMERG (errno_loglevel[0], LOG_ALERT((errno_loglevel[1]),
    * etc.
    */
   int                     errno_loglevelv[MAXLOGLEVELS][UNIQUE_ERRNO_VALUES];
   size_t                  errno_loglevelc[MAXLOGLEVELS];

   /*
    * Same as above, but for dns-errors (getaddrinfo(3) and family).
    */
   int                     gaierr_loglevelv[MAXLOGLEVELS][UNIQUE_GAIERR_VALUES];
   size_t                  gaierr_loglevelc[MAXLOGLEVELS];

   /*
    * Warnings for certain protocol-spesific things.
    */
   warn_protocol_t         protocol;
} logspecial_t;

typedef struct rule_t {
   objecttype_t            type;         /* what kind of rule this is.        */
   int                     verdict;      /* verdict for this rule.            */

   socketoption_t          *socketoptionv;
   size_t                  socketoptionc;

#if COVENANT
   /* if block, why.  XXX why not a more general text string? */
   struct {
      unsigned char        missingproxyauth;
   } whyblock;
#endif /* COVENANT */


   ruleaddr_t       src;
   ruleaddr_t       dst;

   ruleaddr_t       rdr_from;
   ruleaddr_t       rdr_to;

   unsigned char    hostidoption_isset;   /* any of the values below set?     */

#if HAVE_SOCKS_HOSTID

   ruleaddr_t       hostid;         /*
                                     * if atype is not SOCKS_ADDR_NOTSET,
                                     * this rule requires a matching hostid.
                                     */

   unsigned char    hostindex; /*
                                * address index to match hostid against.
                                * 0 means any, 1 means first index, etc.
                                */

#endif /* HAVE_SOCKS_HOSTID */

   /*
    * Extra options only present in certain kind of rules.
    */
#if BAREFOOTD
   unsigned char           bounced;      /*
                                          * have we faked a request for the addr
                                          * "dst" already?  Only used for udp.
                                          */

   struct {
      /*
       * address packet from src to dst should be bounced to.
       * Is a ruleaddr_t and not sockshost_t because bounce-to port may be
       * different for tcp and udp clients if specified as a servicename.
       */
      ruleaddr_t              bounceto;
   } extra;
#endif /* BAREFOOTD */

   log_t                   log;          /* type of logging to do.            */

   struct {
      logspecial_t         log;          /* certain special logging.          */
   } internal;

   struct {
      logspecial_t         log;          /* certain special logging.          */
   } external;


   size_t                  number;       /* rulenumber.                       */
   size_t                  linenumber;   /* linenumber; info/debugging only.  */

   serverstate_t           state;
   timeout_t               timeout;      /* default or specific for this one. */

   linkedname_t            *user;        /* name of users allowed.            */
   linkedname_t            *group;       /* name of groups allowed.           */

   struct {
      in_port_t            start;
      in_port_t            end;
      enum operator_t      op;
   } udprange;                           /* udprange, if limited.             */

#if HAVE_LDAP

   linkedname_t            *ldapgroup;   /* name of ldap groups allowed.      */
   unsigned char           ldapsettingsfromuser;

#endif /* HAVE_LDAP */

#if HAVE_PAC
   linkedname_t            *objectsids;  /* name of sids(=AD groups) allowed. */
   unsigned                pacoff;       /* flag to use sids(=AD groups) */
#endif /* HAVE_PAC */

#if HAVE_LIBWRAP
   char                    libwrap[LIBWRAPBUF];   /* libwrapline.             */
#endif /* HAVE_LIBWRAP */

   shmem_object_t          *mstats;                /* Matching monitorstats.  */
   unsigned long           mstats_shmid;           /* shmid of monitorstats.  */
   unsigned char           mstats_isinheritable;
   size_t                  alarmsconfigured;       /* bitmask.                */

   shmem_object_t          *bw;            /* pointer, memory will be shared. */
   unsigned long           bw_shmid;       /* shmid of bw, if any.            */
   unsigned char           bw_isinheritable;/*
                                             * object is inheritable by
                                             * higher-level ACLs?
                                             */

   shmem_object_t          *ss;            /* pointer, memory will be shared. */
   unsigned long           ss_shmid;       /* shmid of ss, if any.            */
   unsigned char           ss_isinheritable;/*
                                             * object is inheritable by
                                             * higher-level ACLs?
                                             */

   struct rule_t           *next;      /* next rule in list.                  */
} rule_t;

typedef struct {
   int value;                     /* value of SCHED_foo define                */
   const char *name;              /* textual representation of scheduler name */
} cpupolicy_t;

typedef struct {

   /* scheduling configured for this process? */
   unsigned char        scheduling_isset;

#if HAVE_SCHED_SETSCHEDULER
   int                  policy;
   struct sched_param   param;
#endif /* HAVE_SCHED_SETSCHEDULER */

   /* affinity configured for this process? */
   unsigned char        affinity_isset;

#if HAVE_SCHED_SETAFFINITY
   cpu_set_t            mask;
#endif /* HAVE_SCHED_SETAFFINITY */

} cpusetting_t;

typedef struct {
   unsigned char nodnsmismatch;  /* deny if mismatch between dns claim/fact?  */
   unsigned char nodnsunknown;   /* deny if no dns record?                    */
   unsigned char checkreplyauth; /* check that method matches for replies?    */
} srchost_t;

/*
 * NOTE: commandline-options that can override config-file options must have
 * a matching _isset attribute and be added to the CMDLINE_OVERRIDE() macro.
 */
typedef struct {
   const char        *configfile;     /* name of config file.                 */

   unsigned char     daemon;          /* run as a daemon?                     */

   int               debug;           /* debug level.                         */
   unsigned char     debug_isset;

   int               hosts_access;    /* do hosts_access() lookup?            */

   int               directfallback;  /* fallback to direct connections?      */

   unsigned char     keepalive;       /* set SO_KEEPALIVE?                    */

   char              *pidfile;        /* name of pidfile.                     */
   unsigned char     pidfilewritten;  /* did we successfully write pidfile?   */

   size_t            serverc;         /* number of servers.                   */

   unsigned char     verifyonly;      /* syntax verification of config only.  */
   unsigned char     versiononly;     /* show version info only.              */
} option_t;


#if HAVE_PRIVILEGES
typedef struct {
   priv_set_t       *unprivileged;
   priv_set_t       *privileged;
} privileges_t;

#else /* !HAVE_PRIVILEGES */
typedef struct {
   unsigned char    privileged_isset;
   uid_t            privileged_uid;
   gid_t            privileged_gid;

   unsigned char    unprivileged_isset;
   uid_t            unprivileged_uid;
   gid_t            unprivileged_gid;

   unsigned char    libwrap_isset;
   uid_t            libwrap_uid;
   gid_t            libwrap_gid;

   unsigned :0;
} userid_t;
#endif /* !HAVE_PRIVILEGES */

typedef struct {
   int                  ack;            /* control-pipe to mother.            */
   int                  s;              /* data-pipe to mother.               */
} sockd_mother_t;


typedef struct {
   unsigned char       inited;

   sig_atomic_t        insignal;          /* executing in signal handler?     */
   struct {
      sig_atomic_t     signal;
      siginfo_t        siginfo;
   } signalv[SOCKS_NSIG];                 /* stacked signals.                 */
   sig_atomic_t        signalc;           /* number of stacked signals.       */

   /*
    * For mother processes only.
    */
   size_t              unexpected_deaths;
   time_t              firstdeath_time;
   time_t              lastdeath_time;

   /*
    * Sum of rusage for various process types.
    * For mother processes only.
    */
   /* struct rusage       rusage_mother; XXX not supported yet. */
   struct rusage       rusage_monitor;
   struct rusage       rusage_negotiate;
   struct rusage       rusage_request;
   struct rusage       rusage_io;

   cpusetting_t   cpu;                  /* current cpusettings, if any set.   */

   pid_t          *motherpidv;          /* pid of mothers.                    */
   pid_t          pid;                  /* pid of current process.            */

   int            reservedfdv[1];       /*
                                         * Dummy fd we reserve and keep around
                                         * so we can temporarily close it if we
                                         * need to make a library calls and
                                         * it failed because there were no
                                         * more free fd's.  E.g. getpwuid(3).
                                         */

   unsigned char  haveprivs;            /*
                                         * some sort of privileges/euid
                                         * switching available?
                                         */

   uid_t          euid;                 /* current euid.                      */
   gid_t          egid;                 /* current egid.                      */

   int            highestfdinuse;
   rlim_t         maxopenfiles;
   sockd_mother_t mother;               /* if child, mother info.             */
   long           pagesize;
   int            type;                 /* process type we are.               */

   int            monitor_ack;          /* control-pipe to monitor process.   */
   int            monitor_s;            /* data-pipe to monitor process.      */
   time_t         monitor_sent;         /*
                                         * last time this process sent an
                                         * object to the monitor process.
                                         */

#if BAREFOOTD

   unsigned char  alludpbounced;        /* bounced all udp addresses?         */

#endif /* BAREFOOTD */

   /*
    * The next set of objects allows us to optimize a few things based on
    * the currently running configuration.
    * If a value is unset it means values can vary from rule to rule.
    * Otherwise, the value is fixed and these variables contain the fixed
    * value.
    * We only care about attributes that can affect rulespermit().  I.e.,
    * is it possible that the value of a given attribute can change whether
    * rulespermit() will pass or block a session?  If not, no need to care
    * about that attribute here.
    */

#if HAVE_PAM

   char          pamservicename[MAXNAMELEN];

#endif /* HAVE_PAM */

#if HAVE_BSDAUTH

   char          bsdauthstylename[MAXNAMELEN];

#endif /* HAVE_BSDAUTH */

#if HAVE_GSSAPI

   char          gssapiservicename[MAXNAMELEN];
   char          gssapikeytab[MAXNAMELEN];

#endif /* HAVE_GSSAPI */

#if HAVE_LDAP

   ldapauthorisation_t     ldapauthorisation;
   ldapauthentication_t    ldapauthentication;

#endif /* HAVE_LDAP */

} configstate_t;

typedef struct {
   /*
    * Protocols/address-families operator has configured us to *look for*,
    * but not necessarily available.
    */
   unsigned ipv4;
   unsigned ipv6;

   /*
    * Protocols we have looked for and found to be available.
    */
   unsigned char              hasipv4;
   unsigned char              hasipv6;

   /* at least one of our IPv6 addresses has global scope. */
   unsigned char              hasipv6_globalscope;
} interfaceprotocol_t;

typedef struct {
   int                     s;           /* socket we listen on.               */
   struct sockaddr_storage addr;        /* address we listen on.              */

   int                     protocol;    /*
                                         * Is socket SOCKS_TCP or SOCKS_UDP?
                                         * UDP only applicable to barefoot.
                                         */
} listenaddress_t;

typedef struct {
   interfaceprotocol_t     protocol;

   listenaddress_t         *addrv;     /* addresses.                          */
   size_t                  addrc;

   logspecial_t            log; /* special logging; problems on internal side */
} internaladdress_t;

typedef struct {
   interfaceprotocol_t     protocol;

   ruleaddr_t              *addrv;           /* addresses.                    */
   size_t                  addrc;

   int                     rotation;         /* how to rotate, if at all.     */

   logspecial_t            log; /* special logging; problems on external side */
} externaladdress_t;

typedef struct {
   size_t                  accepted;         /* accepts done.                 */
   time_t                  boot;             /* time of server start.         */
   time_t                  configload;       /* time config was last loaded.  */

   struct {
      size_t               sendt;            /* clients sent to children.     */
      size_t               received;         /* clients received back.        */
   } negotiate;

   struct {
      size_t               sendt;            /* clients sent to children.     */
      size_t               received;         /* clients received back.        */
   } request;

   struct {
      size_t               sendt;            /* clients sent to children.     */
      size_t               received;         /* acks received back.           */
   } io;
} statistic_t;

typedef struct {
#ifdef HAVE_VOLATILE_SIG_ATOMIC_T
   sig_atomic_t            noaddchild;          /* okay to do a addchild()?   */
   sig_atomic_t            noaddchild_errno;    /* if not, why not.           */
#else
   volatile sig_atomic_t   noaddchild;          /* okay to do a addchild()?   */
   volatile sig_atomic_t   noaddchild_errno;    /* if not, why not.           */
#endif /* HAVE_VOLATILE_SIG_ATOMIC_T */
   const char              *noaddchild_reason;  /* and our own txt, or NULL.  */

   size_t                  maxrequests;         /*
                                                 * max # of requests to handle
                                                 * before quitting.
                                                 * 0 if unlimited.
                                                 */

   size_t                  maxlifetime;         /*
                                                 * max seconds to live
                                                 * before quitting.
                                                 * 0 if forever.
                                                 */
} childstate_t;

typedef struct {
   unsigned long  id;
   statekey_t     key;
   size_t         type; /* type of shmem-object.                              */
} oldshmeminfo_t;


/*
 * Make sure to keep in sync with resetconfig().
 *
 * Also try to avoid adding pointers to this object.  Any pointers added
 * *MUST* to be synced with the shmem config copy functions in
 * sockd_shmemconfig.c, while non-pointers are handled automatically.
 */

struct config {
   struct { /* initial/pre-config settings that we want to save. */
      option_t           cmdline;   /* original cmdline options. */
      cpusetting_t       cpu;

#if !HAVE_PRIVILEGES
      uid_t              euid;
      gid_t              egid;
#endif /* !HAVE_PRIVILEGES */

      res_options_type_t res_options;
   }                          initial;

   internaladdress_t          internal;   /* internal addresses.              */
   externaladdress_t          external;   /* external addresses.              */

   struct {
      cpusetting_t mother;
      cpusetting_t negotiate;
      cpusetting_t request;
      cpusetting_t io;
      cpusetting_t monitor;
   }                          cpu;

   rule_t                     *crule;              /* clientrules, list.      */
   rule_t                     *hrule;              /* hostidrules, list.      */
   rule_t                     *srule;              /* socksrules, list.       */

   routeoptions_t             routeoptions;        /* global route flags.     */
   route_t                    *route;

   monitor_t                  *monitor;            /* monitors, list.         */
   struct {
      /*
       * If a session has been idle this amount of seconds, mark it as
       * a candidate for testing MTU-related problems.
       */
      time_t                  mtu_timeout;
   } monitorspec;

   socketoption_t             *socketoptionv;      /* global socket options.  */
   size_t                     socketoptionc;

   int                        hostfd;              /*
                                                    * shmem file/lock for
                                                    * hostcache.
                                                    */

#if HAVE_LDAP

   int                        ldapfd;              /*
                                                    * shmem file/lock for
                                                    * ldap cache.
                                                    */
#endif /* HAVE_LDAP */

   int                        shmemfd;             /*
                                                    * shmem file/lock for
                                                    * shared memory, *and*
                                                    * for holding shmeminfo.
                                                    */

   int                        shmemconfigfd;       /*
                                                    * For mapping the shmem
                                                    * config.
                                                    */

   struct {
      /*
       * address of shmemconfig in mothers process.  Children
       * need to know so they can calculate the correct
       * offset of pointers in the shmem area they copy from
       * mother.
       */
      struct config *config;

      /* size of config, with all allocated pointer memory. */
      size_t        configsize;
   } *shmeminfo;
   char                       shmem_fnamebase[PATH_MAX];

   oldshmeminfo_t             *oldshmemv; /* old shmem, not yet deleted.      */
   size_t                     oldshmemc;

   compat_t                   compat;               /* compatibility options. */
   extension_t                extension;            /* extensions set.        */

   logtype_t                  errlog;               /* for errors only.       */
   logtype_t                  log;                  /* where to log.          */
   int                        loglock;              /* lockfile for logging.  */

   option_t                   option;               /*
                                                     * options that can be
                                                     * set on the commandline
                                                     * also.
                                                     */

   int                        resolveprotocol;      /* resolve protocol.      */
   srchost_t                  srchost;              /* relevant to srchost.   */
   statistic_t                stat;                 /* some statistics.       */
   configstate_t              state;
   timeout_t                  timeout;

#if HAVE_PRIVILEGES
    privileges_t              privileges;
#else /* !HAVE_PRIVILEGES */
   userid_t                   uid;
#endif /* !HAVE_PRIVILEGES */

   childstate_t               child;                   /* childstate.         */

   int                        cmethodv[METHODS_KNOWN]; /* clientmethods.      */
   size_t                     cmethodc;                /* methods set in list.*/

   int                        smethodv[METHODS_KNOWN];/* methods by priority. */
   size_t                     smethodc;               /* methods set in list. */

   unsigned char              udpconnectdst;          /* connect udp sockets? */

#if HAVE_LIBWRAP
   char                       *hosts_allow_original;/* original libwrap value */
   char                       *hosts_deny_original; /* original libwrap value */
#endif /* HAVE_LIBWRAP */
#if COVENANT
   char                       realmname[256];
#endif /* COVENANT */
};

typedef struct {
   int                  proxyprotocol; /* proxyprotocol used with this proxy. */
   sockshost_t          extaddr; /* external address proxyserver may be using */
} proxychaininfo_t;


typedef struct {
   int                  command;
   int                  protocol;
   int                  proxyprotocol;

   /*
    * The hostids on the control connection.
    * Assumes hostids can only be set on TCP sessions, which is currently 
    * the case.
    */
   struct hostid  hostid;

   proxychaininfo_t     proxychain;    /* only if proxyprotocol is not direct.*/
   extension_t          extension;     /* extensions set.                     */

   struct {
      struct timeval    accepted;      /* time connection accepted.           */
      struct timeval    negotiatestart;/* time negotiation started.           */
      struct timeval    negotiateend;  /* time negotiation ended.             */
      struct timeval    requestend;    /* time requestprocesssing ended.      */
      struct timeval    established;   /* time session was fully established. */
      struct timeval    firstio;       /* time of first i/o operation.        */
   } time;

   const char           *tcpinfo;      /* tcpinfo, if available/relevant.     */
} connectionstate_t;

typedef struct {
   uint64_t         bytes;        /* byte count.                              */
   /*
    * packet count.  For UDP this is the number of packets we have
    * sent (or at least handed of to the kernel), while for TCP it
    * corresponds to the number of i/o operations done.
    */
   uint64_t         packets;

   struct timeval   lastio;       /* time of last i/o operation.              */
} iocount_t;


/* common header for data packets to/from child. */
typedef struct {
   unsigned char command;     /*
                               * Command from child.  Same as command
                               * read over ack-pipe, but if sent here,
                               * will not be sent over ack-pipe.
                               */
} reqinfo_t;

typedef struct {
   int                     s;     /* socket we use to send/receive to remote. */

   struct sockaddr_storage laddr;     /* address we receive remote replies on.*/
   struct sockaddr_storage raddr;     /* target address we forward packets to.*/
   struct sockshost_t      raddrhost; /* raddr on sockshost_t form.           */

#if BAREFOOTD /* Dante has only one client, but Barefoot can have many. */
   struct sockaddr_storage client;     /* address of our client.              */
   sockshost_t             clienthost; /* client on sockshost_t form.         */

   /*
    * client-rule matched for this particular client.
    */
   rule_t          crule;
#endif /* BAREFOOTD */

   unsigned char   isconnected;    /* socket connected to target?             */

   /*
    * read from client in relation with this target.
    */
   iocount_t       client_read;
   iocount_t       client_written; /* written to client.                      */

   /*
    * read from target in relation with this client.
    */
   iocount_t       target_read;
   iocount_t       target_written;

   struct timeval  firstio;        /* time of first i/o operation.            */
   struct timeval  lastio;         /* time of last i/o operation.             */
} udptarget_t;


typedef struct {
   int                 s;          /* socket connection.               */
   struct sockaddr_storage laddr;  /* local address of s.              */
   struct sockaddr_storage raddr;  /* address of s's (last) peer.      */

   authmethod_t        auth;        /* authentication in use on s.     */
   sockshost_t         host;
   /*
    * Varies according to context.
    * src    : same as raddr.
    *
    * dst    : raddr as given by client.  Note that if a serverchain is used,
    *          raddr is the remote proxy's address, while host is still the
    *          raddr as given by client.  (Don't know what the resolved raddr
    *          was).
    *
    * control: same as raddr
   */

   /* these are socket counts, not from/to our internal userspace buffer. */
   iocount_t                  read;         /* read from socket s.            */
   iocount_t                  written;      /* written to socket s.           */

   int                        flags;        /* misc. flags                    */
   unsigned char              isclientside; /* is this the clientside?        */

   struct {
      unsigned char alarmdisconnectdone;
      unsigned char isconnected;    /* socket is connected?                   */
      int           err;            /* current errno.                         */
      unsigned char fin_received;   /* received FIN on this socket?           */
      unsigned char use_saved_srule;/*
                                     * should we try to reuse last rule result
                                     * for udp packets _received_ on this
                                     * socket?
                                     */
   } state;

#if HAVE_UDP_SUPPORT
   /*
    * For TCP, there is only one peer/target/destination, and that is
    * either an ipv4 or an ipv6 peer.
    *
    * For UDP things can be different.
    *
    * In Dante's case, if the client sends to both IPv4 and IPv6 targets,
    * we want to use a local IPv4 address/socket for sending to IPv4 targets
    * and an IPv4 address/socket for sending to IPv6 targets.
    * For this reason there are up to two dst objects if it's a udp session;
    * one for IPv4 and one for IPv6.  We allocate them as needed, so
    * we end up having one of following number of target sockets for a
    * client:
    *    - zero (no packets forwarded from client).
    *    - one (packets forwarded to only one type of address (ipv4 or ipv6,
    *      but not both).
    *    - two (packets forwarded to both ipv4 and ipv6 addresses).
    * This however assumes our external interface has both IPv4 and IPv6
    * addresses.  Does it not, at most one target socket will be created.
    *
    * In Barefoots case, we don't have a TCP control-session, so the scenario
    * is one-to-many; we receive all udp packets from different clients on
    * the one source socket, and we forward them to various targets based
    * what client it was received from.  Since the target is hardcoded in
    * the sockd.conf, there can be only one target for packets we receive
    * from a client (the same client can however send to us on different
    * internal addresses, in which case the target may also differ, but
    * that creates a separate session; one session per internal listen
    * address).
    */

   /* only used on the destination side, NULL otherwise. */
   udptarget_t                   *dstv;
   size_t                        dstcmax;  /* number of slots in dstv array.  */
   size_t                        dstc;     /* # of slots currently in use.    */
#endif /* HAVE_UDP_SUPPORT */
} sockd_io_direction_t;

/*
 * part of objects received between processes, but where the size is not
 * fixed.  Not used yet.
 */
typedef struct {
   void   *gssapidata;
   size_t gssapidatalen;

   void   *socketoptiondata;
   size_t socketoptdatalen;
} dynamicdata_t;

#define MAX_DYNAMICDATA()                                                      \
   (                                                                           \
      MAX_GSS_STATE                                                            \
    + sizeof(socketoption_t) * MAX_EXTERNAL_SOCKETOPTIONS                      \
   )


typedef struct sockd_io_t {
   unsigned char          allocated; /* object currently allocated?           */

   reqinfo_t              reqinfo;   /* info from child about this request.   */

   connectionstate_t      state;
   authmethod_t           cauth;     /* client authentication in use.         */
   requestflags_t         reqflags;  /* original client request flags.        */

   sockd_io_direction_t   control;  /* clients controlconnection.             */
   sockd_io_direction_t   src;      /* client we receive data from.           */
   sockd_io_direction_t   dst;      /* remote peer.                           */

   dynamicdata_t          data;

   /*
    * data received from the client that should be sent to the remote server,
    * but has not yet.
    */
#if HAVE_NEGOTIATE_PHASE
   char        clientdata[MAXREQLEN];
   size_t      clientdatalen;
#endif /* HAVE_NEGOTIATE_PHASE */


   rule_t              crule;       /* client rule matched.                   */

#if HAVE_SOCKS_HOSTID

   unsigned char       hrule_isset;
   rule_t              hrule;       /* rule matched for hostid().             */

#endif /* HAVE_SOCKS_HOSTID */

   rule_t              srule;       /* socks-rule matched.                    */

   socketoption_t      extsocketoptionv[MAX_EXTERNAL_SOCKETOPTIONS];
   size_t              extsocketoptionc;

    union {
      struct {
         /*
          * These are pointers since they are only used in the i/o processes.
          * Making them pointers lets us save on the size of the i/o object
          * when passing it around.
          *
          * If the corresponding "use_saved_rule" variable is set, it
          * is possible (given some other constraints) that we can
          * reuse a previous rulespermit(), the resulting rule of which
          * is stored in these objects.
          */
         rule_t *sfwdrule;   /* for packets forwarded from client  */
         rule_t *sreplyrule; /* for packets forwarded from target. */
      } udp;

#if SOCKS_SERVER
      struct {
         /*
          * the i/o-process only handles bind replies, but may need to log
          * information related to the bind session that initiated the
          * bind reply session also.
          */
         sockshost_t host;
         rule_t      rule;
      } bind;
#endif /* SOCKS_SERVER */
   } cmd; /* extra info required for certain commands. */

   struct timeval     lastio;          /* time of last i/o operation.         */

   /* tests already done/scheduled on this object. */
   struct {
      networktest_tested_t     internal;
      networktest_tested_t     external;
   } tested;

   struct sockd_io_t  *next;           /* for bind-extension.                 */
} sockd_io_t;


typedef struct {
   reqinfo_t         reqinfo;    /* info from child about this request. */
   struct timeval    accepted;   /* time client was accepted.           */

   int               s;          /* socket client was accepted on.      */
   struct sockaddr_storage from;       /* client's control address.           */
   struct sockaddr_storage to;         /* address we accepted client on.      */


#if COVENANT
   /*
    * if not zero, this is an "old" client that has been sent back
    * to the negotiate process from the i/o process, due to the client
    * changing it's target (remote http server).
    * "clientdata" contains the request received from the client,
    * already parsed into "request".
    */
   char                   clientdata[MAXREQLEN];
   size_t                 clientdatalen;

   authmethod_t           auth;
   request_t              request;
#endif /* COVENANT */
} sockd_client_t;

typedef struct negotiate_state_t {
   unsigned char        complete;     /* completed?                           */

#if SOCKS_SERVER
   unsigned char        mem[ 1                       /* VER                   */
                           + 1                       /* NMETHODS              */
                           + (AUTHMETHOD_MAX + 1)    /* METHODS               */
#if HAVE_GSSAPI
                           + MAXGSSAPITOKENLEN
#endif /* HAVE_GSSAPI */
                           + sizeof(request_t)
                           + sizeof(authmethod_t)   /*
                                                     * size authmethod uname
                                                     * really, but include the
                                                     * whole struct as if
                                                     * not we will surly forget
                                                     * to change this if a
                                                     * bigger authmethod is
                                                     * ever added.
                                                     */
                           ];
#elif COVENANT
   /* no fixed limit in the http protocol?  Try this for now. */
   unsigned char        mem[MAXREQLEN];

   unsigned char         haverequestedproxyauth;
   unsigned char         havedonerulespermit;
#endif /* COVENANT */

   size_t               reqread;                     /* read so far.          */
   size_t               start;                       /* start of current req  */
   char                 emsg[512];                   /* error message, if any.*/
   negotiate_result_t   (*rcurrent)(int s,
                                    request_t *request,
                                    struct negotiate_state_t *state);

   sockshost_t          src;          /* client's address.                    */
   sockshost_t          dst;          /* our address.                         */

   rule_t               *crule;        /* client-rule that permitted client.  */
#if HAVE_GSSAPI
   unsigned short       gssapitoken_len; /* length of token we're working on. */
#endif /* HAVE_GSSAPI */
} negotiate_state_t;

typedef struct {
   unsigned char       allocated;


   request_t           req;
   negotiate_state_t   negstate;

   rule_t              crule;       /* client-rule matched.            */
   authmethod_t        cauth;       /* authentication for clientrule.  */

#if HAVE_SOCKS_HOSTID

   unsigned char       hrule_isset;
   rule_t              hrule;       /* rule matched for hostid().      */

#endif /* HAVE_SOCKS_HOSTID */

#if COVENANT
   rule_t              srule;       /* rule matched at socks-level.    */
#endif /* COVENANT */
   authmethod_t        sauth;       /* authentication for socks-rule.  */

   int                 s;           /* client connection.              */
   connectionstate_t   state;       /* state of connection.            */
} sockd_negotiate_t;

typedef struct sockd_request_t {
   reqinfo_t           reqinfo;   /* info from child about this request. */
   struct sockaddr_storage from;  /* client's control address.           */
   struct sockaddr_storage to;    /* address client was accepted on.     */

   request_t           req;       /* request to perform.                 */

   rule_t              crule;     /* client-rule matched.                */
   authmethod_t        cauth;     /* client authentication in use.       */

#if HAVE_SOCKS_HOSTID

   rule_t              hrule;      /* hostid-rule matched, if any.       */
   unsigned char       hrule_isset;

#endif /* HAVE_SOCKS_HOSTID */


   rule_t              srule;      /* rule matched at socks-level.       */
   unsigned char       srule_isset;/* did we progress far enough to use srule?*/

   authmethod_t        sauth;      /* socks authentication in use.       */

   int                 s;         /* clients control connection.         */
   connectionstate_t   state;     /* state of connection.                */

#if HAVE_NEGOTIATE_PHASE          /* initial request from client.        */
   char                       clientdata[MAXREQLEN];
   size_t                     clientdatalen;
#endif /* HAVE_NEGOTIATE_PHASE */
} sockd_request_t;


typedef struct {
   unsigned char    waitingforexit; /*
                                     * waiting for child to exit.  If set,
                                     * pipes should be -1 and no more clients
                                     * should be sent to this child, nor
                                     * should it be included in any counts.
                                     *
                                     * Basically just waiting for the SIGCHLD
                                     * so we can add it's resourceusage to
                                     * our counters.
                                     */
   unsigned         exitingnormally;/* exiting normally, on our request?      */

   int              ack;            /* connection to child for acks.          */
   int              s;              /* connection to child for data.          */

   pid_t            pid;            /* childs pid.                            */
   int              type;           /* child type.                            */

   time_t           created;        /* time created.                          */
   size_t           freec;          /* free slots at the moment.              */
   size_t           sentc;          /* clients sent to this child.            */

#if BAREFOOTD
   unsigned char    hasudpsession;  /*
                                     * is one of the slots taken by an udp
                                     * session at the moment?
                                     */
#endif /* BAREFOOTD */
} sockd_child_t;

typedef struct {
   /*
    * Note that these are not necessarily the physical endpoints.  It could
    * be this object refers to a connection via a proxy server, in which
    * case "local" will be the address of the proxy server we connected to,
    * while "peer" will be the address (we believe) the proxy server is
    * using on our behalf.
    */
   unsigned char  local_isset;
   sockshost_t    local;  /* local endpoint if known.  */

   unsigned char  peer_isset;
   sockshost_t    peer;   /* remote endpoint if known. */

   /*
    * auth used on the connection between local and peer, if set.
    */
   unsigned char  auth_isset;
   authmethod_t   auth;

#if HAVE_SOCKS_HOSTID

   /*
    * if set, the hostids of servers/gateways between local and peer.
    * Not to be confused with a possible proxy server used by us to
    * establish a session with peer.
    */
   struct in_addr hostidv[HAVE_MAX_HOSTIDS];
   size_t         hostidc; /* how many hostids are actually present/set. */

#endif /* HAVE_SOCKS_HOSTID */
} iologaddr_t;

void
io_updatemonitor(sockd_io_t *io);
/*
 * Updates "io" according to the current monitor configuration.
 */

void
io_add_alarmdisconnects(sockd_io_t *io, const char *reason);
/*
 * Called when the session belonging to "io" is to be removed and adds
 * any alarm disconnects not yet done.  "reason" is the reason for the
 * disconnect.
 */

void
io_update(const struct timeval *timenow, const size_t bwused,
          const iocount_t *internal_read, const iocount_t *internal_written,
          const iocount_t *external_read, const iocount_t *external_written,
          rule_t *rule, rule_t *packetrule, const int lock);
/*
 * update the time/bw counters in "rule" and/or "packetrule" according to
 * the arguments.
 *
 * In the case of a TCP session, "rule" and "packetrule" are the same,
 * but in the case of a SOCKS UDP session, they can differ, with "packetrule"
 * being the rule matched for the given UDP packet transfer, while "rule"
 * is, as for TCP, the rule that controls the resources (bandwidth, session,
 * etc.).
 *
 * "bwused" is the bandwidth used at time "timenow", while
 * the iocount_t variables provide more detailed information about
 * what was read/written from/to where.
 */

iostatus_t
doio_tcp(sockd_io_t *io, fd_set *rset, fd_set *wset,
         const int flags, int *badfd);
/*
 * Does i/o over the descriptors in "io", in to out and out to in.
 * "io" is the object to do i/o over,
 * "flags" is the flags to set on the actual i/o calls
 * (read()/write(), recvfrom()/sendto()), currently only MSG_OOB.
 *
 * Returns the status of the doio() call, IO_NOERROR on success, or
 * some other value on error.  If "badfd" is not -1, it will have the value
 * of the file descriptor on which the error was detected.
 *
 * In most cases, io_delete() should be called upon error.
 */


#if HAVE_UDP_SUPPORT

iostatus_t
doio_udp(sockd_io_t *io, fd_set *rset, int *badfd);
/*
 * does io over the udp sockets in "io".  "rset" is a set where at least
 * one of the fds matches a readable fd in io
 *
 * Returns the status of the i/o operation.  If an error is detected,
 * "badfd" contains the fd related to the error (io->src.s or io->dst.s).
 */

iostatus_t
io_packet_received(const recvfrom_info_t *recvflags,
                   const size_t bytesreceived,
                   const struct sockaddr_storage *from,
                   const struct sockaddr_storage *receivedon);

/*
 * Called immedidately after a udp packet has been received.
 *
 * "recvflags" are the values set when receiving the packet.
 * "bytesreceived" gives the length of the packet received.
 * "from" is the address that sent the packet.
 * "receiveon" is the address we received the packet on.
 *
 * Normally returns IO_NOERROR, but if something is wrong with the packet,
 * the appropriate error is logged and a failurecode is returned.
 */

iostatus_t
io_packet_sent(const size_t bytestosend,
               const size_t bytessent,
               const struct timeval *tsreceived,
               const struct sockaddr_storage *from,
               const struct sockaddr_storage *to,
               char *emsg, size_t emsglen);
/*
 * Called immediately after a udp packet has been sent.
 *
 * "bytestosend" gives the number of bytes we were supposed to send, while
 * "bytessent" gives the number of bytes actually sent.
 *
 * "tsreceived" is the time when the bytes we just sent was received.
 *
 * "from" is the address from which the packet was received.
 *
 * "to" is the address to which we sent the packet.
 *
 * Returns IO_NOERROR if everything is ok, or a failurecode on error.
 * If error, the reason for error is stored in "emsg".
 */


in_port_t *udphdr_uh_dport(struct udphdr *udp);
in_port_t *udphdr_uh_sport(struct udphdr *udp);
uint16_t *udphdr_uh_ulen(struct udphdr *udp);
uint16_t *udphdr_uh_sum(struct udphdr *udp);
/*
 * Wrappers to access the member due to difference in naming between
 * BSD and linux.
 */



udptarget_t *
clientofsocket(const int s, const size_t udpclientc,
               udptarget_t *udpclientv);
/*
 * Returns the udpclient belonging to socket "s", or NULL if no
 * such client.
 */

void
io_syncudp(sockd_io_t *io, udptarget_t *udpclient);
/*
 * Syncs the i/o-object "io" based on the contents of "udpclient".
 * In Barefoot, udp is a point-to-multipoint case: we receive all client
 * packets on one socket, but use different outgoing sockets for each client.
 * We use this function to sync the necessary parts of the udpclient to the
 * io object before usage.
 */

void
io_syncudpdst(sockd_io_direction_t *dst, const udptarget_t *udptarget);
/*
 * syncs only the "dst" part of an i/o object with "udptarget".
 */


void
io_syncudpsrc(sockd_io_direction_t *src, const udptarget_t *udpclient);
/*
 * Syncs only the "src" part of an i/o object based on "udpclient".
 */

#if BAREFOOTD

rawsocketstatus_t
rawsocket_recv(const int s, const size_t ioc, sockd_io_t iov[]);
/*
 * Handles packets input on the raw socket "s".
 * Used to read icmp-errors concerning packets we send to/from udp clients.
 */

int
removeclient(const int s, size_t *clientc, udptarget_t *clientv);
/*
 * Removes the udpclient associated with the socket "s" from the
 * "clientv" array, which contains "clientc" elements, and decrements
 * "clientc".
 * Returns 0 on success, -1 on failure.
 */


udptarget_t *
clientofclientaddr(const struct sockaddr_storage *addr,
                   const size_t udpclientc, udptarget_t *udpclientv);
/*
 * Returns the udpclient that has the client address "addr", or NULL
 * if no such client exists.
 */

#endif /* BAREFOOTD */

size_t
io_udpclients(const size_t ioc, const sockd_io_t iov[], const ssize_t mincount);
/*
 * Returns the number of active udp clients.
 *
 * If "mincount" is not -1, returns as soon as at least "mincount" active
 * udp clients have been counted, rather than scanning till the end of
 * iov to count all possible clients.
 */


#if HAVE_SO_TIMESTAMP
void
io_addts(const struct timeval *ts, const struct sockaddr_storage *from,
         const struct sockaddr_storage *to);
/*
 * adds timestamp "ts" to the object we use to keep latency timestamps.
 * The ts is the time it took from a packet from "from" was received
 * by the kernel, until it was sent out on the correct socket, to "to".
 *
 * If "to" is NULL, it means this is only the time it took us to read
 * the packet out of the socket buffer.
 */

struct timeval *
io_calculatelatency(const struct timeval *ts_recv, const struct timeval *tnow,
             struct timeval *latency);
/*
 * Calculates the packet latency for a packet received at "ts_recv" until
 * the current time, given as "tnow".
 *
 * The calculated latency is stored in "latency".
 *
 * Returns a pointer to latency.
 */

#endif /* HAVE_SO_TIMESTAMP */

iostat_t *io_get_ro_stats(void);
iostat_t *io_get_io_stats(void);
/*
 * Returns a pointer to the current (updated) iostats for read-only latency,
 * or i/o latency, accordingly to the function called.
 *
 * Returns NULL if information is not yet available.
 */

void
send_icmperror(const int s, const struct sockaddr_storage *receivedonaddr,
               const struct sockaddr_storage *originalpacketsrc,
               const struct sockaddr_storage *packettarget,
               const int iplen, const int udplen,
               const int type, const int code);
/*
 * Handles packets output in on the raw socket "s".  Used to send icmp-errors
 * concerning packets we could not forward.
 *
 * "receivedonaddr" is the local address on which we received the packet we
 * could not forward.
 *
 * "originalpacketsrc" is the address which sent the packet we could not
 *
 * forward (client or remote target).
 *
 * "packettarget" it the target address the packet we could not forward
 * was intended for (client or remote target).
 *
 * "iplen" is the length of the IP packet containing the udp datagram we could
 * not send, or -1 if unknown.
 *
 * "udplen" is the length of the udp datagram we could not send, or -1 if
 * unknown.
 *
 * "type" and "code" is the icmp type and code we should use when sending the
 *  icmperror.
 */

udptarget_t *
initclient(const int control,
           const struct sockaddr_storage *client_laddr,
           const struct sockaddr_storage *client_raddr,
           const sockshost_t *tohost,
           const struct sockaddr_storage *toaddr, const rule_t *rule,
           char *emsg, const size_t emsglen, udptarget_t *client);
/*
 * Fills in "client" with the necessary info for a new udp client, as well
 * as creating a socket for the client to send packets to the destination
 * "toaddr".
 *
 * "control", if not -1, specifies the socket used for the control connection.
 *
 * "client_laddr" gives the local address the UDP pakcet was received on.
 *
 * "client_raddr" gives the remote address the UDP packet was received from.
 *
 * "toaddr" gives the resolved destination address.
 *
 * "tohost" gives the original address requested by the client, which may
 * be a hostname.
 *
 * "rule" is the rule that permits the client.
 *
 * Returns "client" on success.
 * Returns NULL on failure.  On failure, "emsg" of "emsglen" will contain the
 * reason the call failed.
 */

udptarget_t *
addclient(const struct sockaddr_storage *clientladdr,
          const udptarget_t *client,
          size_t *clientc, size_t *maxclientc, udptarget_t **clientv,
          const connectionstate_t *state, const rule_t *rule);
/*
 * Adds the udpclient "client" to the "clientv" array, which is large
 * enough to contain "maxclientc" clients.
 * "clientc" gives the index of the last slot in "clientv" that is
 * currently in use.
 * "clientladdr" is our local endpoint for packets from the client, and
 * "rule" is the rule that matched the client, and "state" is the state.
 *
 * Returns a pointer to the added client ("client"), or NULL if there
 * is no more room and clientv can not be expanded.
 */


#endif /* HAVE_UDP_SUPPORT */

int
freedescriptors(const char *message, int *highestfdinuse);
/*
 * Returns the number of currently unallocated descriptors, and also
 * writes the index of the highest fd in use to "highestfdinuse", if
 * not NULL.
 *
 * If "message" is not NULL, also logs the current status.
 */


int
sockd_unconnect(const int s, const struct sockaddr_storage *oldpeer);
/*
 * "unconnects" a socket.  Must only be used with udp sockets.
 *
 * If "oldpeer" is not NULL it indicates the peer we are currently
 * unconnecting from.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1 (something wrong with the socket).
 */

void
sockd_rstonclose(const int s);
/*
 * Tries to set the necessary socket options so that when socket "s" is
 * closed, a TCP RST packet will also be sent automatically.
 * Used where we want to indicate to one end of the session that we received
 * a RST from the other end.
 */

int
bindinternal(const int protocol);
/*
 * Binds all internal addresses using protocol "protocol".
 * Returns 0 on success, -1 on failure.
 */


int
pidismother(pid_t pid);
/*
 * If "pid" refers to a mother, the number of "pid" in
 * state.motherpidv is returned.  Numbers are counted from 1.
 * If "pid" is not a motherprocess, 0 is returned.
 */

int
pidismainmother(pid_t pid);
/*
 * If "pid" refers to main mother process, returns true.  Otherwise false.
 */


int
sockd_motherexists(void);
/*
 * Simply check for whether mother still exists or not.
 * Not 100%, but is only used to limit noise related to shmem files
 * mother removes upon exit, making child processes unable to open
 * them.
 * Returns true if mother exists, false otherwise.
 */

int
descriptorisreserved(int d);
/*
 * If "d" is a descriptor reserved for use globally, the function
 * returns true.
 * Otherwise, false.
 */

size_t
childcheck(int type);
/*
 * Calculates the number of free slots every child of type "type" has,
 * combined, and returns that number.
 *
 * If the childtype "type" is not a child related to free slots, the function
 * instead returns the number of child processes of type "type" that exists.
 *
 * If "type" is negated, the function instead returns the total number of
 * slots (free or not) in every child of that type.
 * This function also adjusts the number of children of type "type" if needed,
 * according to configured variables.
 */

int
childtype(const pid_t pid);
/*
 * Returns the type of child the child with pid "pid" is.
 */

void
removechild(const pid_t childpid);
/*
 * Removes the child "child" with pid "childpid" from our list of children.
 * If "childpid" is 0, removes all children.
 */

void
closechild(const pid_t childpid, const int isnormalexit);
/*
 * Closes our pipes to child "childpid" and marks it as unable to receive
 * further clients, but does not remove it from our list of children.
 *
 * If "isnormalexit" is set, we are closing the pipes to this child and
 * expect it to exit normally.  This notifies the child about the close,
 * telling it to exit normally when done serving it's clients, unlike what
 * happens if the child exiting by itself without us telling it to do so,
 * or closing it's pipes to us first.
 *
 * If "childpid" is 0, closes all children.
 */

void
setcommandprotocol(const objecttype_t type, command_t  *commands,
                   protocol_t *protocols);
/*
 * Sets "commands" and "protocols" in an object of type "type".
 * I.e., if "protocols" specifies the udp protocol, sets udp-based
 * commands in "commands", and vice versa.
 */

rule_t *
addclientrule(const rule_t *rule);
/*
 * Appends a copy of "rule" to our list of client rules, and returns
 * a pointer to the added rule.
 */

#if HAVE_SOCKS_HOSTID
int
hostidmatches(const struct hostid *hostid,
              const unsigned char hostindex, const ruleaddr_t *addr,
              const objecttype_t type, const size_t number);
/*
 * Returns true if "addr" matches the corresponding hostid in hostid.
 *
 * "type" and "number" is the type and number of the object (rule) we are 
 * matching against.  These are included for better debug logging and error 
 * messages only.
 */

rule_t *
addhostidrule(const rule_t *rule);
/*
 * Appends a copy of "rule" to our list of hostid rules, and returns a
 * pointer to the added rule.
 */
#endif /* HAVE_SOCKS_HOSTID */


rule_t *
addsocksrule(const rule_t *rule);
/*
 * Appends a copy of "rule" to our list of socks rules and returns a pointer
 * to the added rule.
 */

void
freerulelist(rule_t *rulehead);
/*
 * Frees all rules and their contents, starting at "rulehead", then
 * continuing with "rulehead->next", etc.
 */

void
addinternal(const ruleaddr_t *addr, const int protocol);
/*
 * Adds "addr" to the list of external addresses.
 * "protocol" gives the protocol to add, SOCKS_TCP or SOCKS_UDP.
 */

void
addexternal(const ruleaddr_t *addr);
/*
 * Adds "addr" to the list of internal addresses (to listen on).
 */

#if 0
void
external_set_safamily(unsigned char *hasipv4, unsigned char *hasipv6,
                      unsigned char *hasipv6_global_scope);
/*
 * Checks if the list of external addresses we are configured to
 * use contain the specified address-families and sets "hasipv4"
 * and "hasipv6" as appropriate.
 *
 * Should be called when there are address-related changes on any of the
 * external interfaces configured for use by Dante, but currently there
 * is no code for that, so we currently depend on receiving a SIGHUP
 * when that happens, and then set the global hasipv4 and hasipv6
 * variables as part of the normal config-parsing (and not via this
 * special function).
 *
 * If "hasipv4" is not NULL, it is set to true if we have at least one IPv4
 * address configured on the list of external addresses.
 *
 * If "hasipv6" is not NULL, it is set to true if we have at least one IPv6
 * address configured on the list of external addresses.
 *
 * If "hasipv6_global_scope" is not NULL, it is set to true if we have at
 * least one IPv6 address with global scope configured on the list of external
 * addresses.
 */
#endif

struct in_addr *
ipv4_mapped_to_regular(const struct in6_addr *ipv4_mapped,
                       struct in_addr  *ipv4_regular);
/*
 * Converts the IPv4-mapped IPv6 address "ipv4_mapped" to a regular
 * IPv4 address, and stores it in "ipv4_regular".
 *
 * Returns: ipv4_regular.
 */

void
add_internal_safamily(const sa_family_t safamily);
/*
 * Marks in global state that we have an address belonging to safamily
 * "safamily" on the internal side.
 */

int
internal_has_safamily(const sa_family_t safamily);
/*
 * Checks if the list of internal addresses we are configured to
 * use contain at list one address of the "safamily" family.
 *
 * Returns true if there such an address, or false otherwise.
 */

void
add_external_safamily(const sa_family_t safamily, const int globalscope);
/*
 * Marks in global state that we have an address belonging to safamily
 * "safamily" on the external side.
 *
 * If "globalscope" is true, the address is of global scope.
 */


int
external_has_safamily(const sa_family_t safamily);
/*
 * Checks if the list of external addresses we are configured to
 * use contain at list one address of the "safamily" family.
 *
 * Returns true if there such an address, or false otherwise.
 */

int
external_has_only_safamily(const sa_family_t safamily);
/*
 * Checks if the list of external addresses we are configured to
 * use contain addresses of the "safamily" family.
 *
 * Returns true if there at least one such an address, and only such an
 * address, or false otherwise.
 */

int
external_has_global_safamily(const sa_family_t safamily);
/*
 * Checks if the list of external addresses we are configured to
 * use contain at list one address of the "safamily" family, *and*
 * that address has global scope.
 *
 * Returns true if there such an address, or false otherwise.
 */


int
addrisbindable(const ruleaddr_t *addr);
/*
 * Checks whether "addr" is bindable.
 * Returns:
 *      On success: true.
 *      On failure: false.
 */

int
isreplycommandonly(const command_t *command);
/*
 * Returns true if "command" specifies reply-commands only (bind/udp-replies),
 * Returns false otherwise.
 */

int
hasreplycommands(const command_t *command);
/*
 * Returns true if "command" specifies any reply-commands.
 * Returns false otherwise.
 */



linkedname_t *
addlinkedname(linkedname_t **linkedname, const char *name);
/*
 * Adds a link with the name "name" to the list hanging of "linkedname".
 * Returns:
 *      On success: a pointer to linkedname.
 *      On failure: NULL.
 */

void
freelinkedname(linkedname_t *link);
/*
 * free(3)s all entries in the link starting at "link".
 */

void
showrule(const rule_t *rule, const objecttype_t ruletype);
/*
 * Prints the rule "rule" to logfile.  "ruletype" says what kind of rule
 * "rule" refers to.
 */

void
rule_detachfromlist(rule_t *head);
/*
 * sockd_shmdt() from rule-list starting at "head".
 */

void
showmonitor(const monitor_t *monitor);
/*
 * Prints the monitor "monitor" to logfile.
 */

void
showlist(const linkedname_t *list, const char *prefix);
/*
 * shows user names in "list".
 */

void
showlogspecial(const logspecial_t *log, const interfaceside_t isinternalside);
/*
 * Displays the logsettings in "log".  "isinternalside" should be true if
 * "log" is from the internal side, and false if from the external side.
 */


const char *
authname(const authmethod_t *auth);
/*
 * Returns a pointer to the name contained in "auth", or NULL if none.
 */

const char *
authinfo(const authmethod_t *auth, char *info, size_t infolen)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Fills in "info" with a printable representation of the "auth".
 * Returns a pointer to "info".
 */

#if HAVE_PAC
const char *
authsids(const authmethod_t *auth);
/*
 * Returns a pointer to the sids contained in "auth", or NULL if none.
 */
#endif /* HAVE_PAC */

int
rule_inheritoruse(struct rule_t *from, const clientinfo_t *cinfo_from,
                  struct rule_t *to, const clientinfo_t *cinfo,
                  const size_t sidesconnected,
                  char *emsg, const size_t emsglen);
/*
 * Checks whether rule "to" should inherit from the lower-level rule "from"
 * and uses unuses shmem and redirect settings in "from" and "to" as
 * appropriate.
 * "cinfo_from" is the clientinfo that was used when allocating resources
 * (if any) for rule "from", and "cinfo_to" is the clientinfo to use when
 * allocating resources (if any) for rule "to".
 *
 * "sidesconnected" gives the TCP session-sides currently connected
 * (ALARM_INTERNAL, ALARM_EXTERNAL.  Note that for udp there is only
 * one side possible, the internal).
 *
 * If limits (e.g., session-limits) prevents using one or more objects,
 * to->verdict will be set to VERDICT_BLOCK and the emsg will contain the
 * appropriate errormessage.  The function will then return -1.
 *
 * Returns 0 if there were no limits that prevented us from using the
 *           necessary objects.
 * Returns -1 otherwise, with the reason written to "emsg".
 */

int
rulespermit(int s, const struct sockaddr_storage *peer,
            const struct sockaddr_storage *local,
            const authmethod_t *clientauth, authmethod_t *srcauth,
            rule_t *rule, connectionstate_t *state,
            const sockshost_t *src, const sockshost_t *dst,
            sockshost_t *dstmatched,
            char *msg, size_t msgsize)
            __ATTRIBUTE__((__BOUNDED__(__buffer__, 11, 12)));
/*
 * Checks whether the rules permit data from "src" to "dst".
 *
 * #if !BAREFOOTD
 *
 * "s" is the socket the control connection from the SOCKS client is on,
 * from SOCKS client "peer", accepted on our internal address "local".
 *
 * #else: BAREFOOTD
 *
 * "s" is the socket the client was accepted on, or on which udp packets from
 * it was received.  "peer" gives the client address, and "local" gives our
 * own internal address on which the connection/packets were accepted on.
 *
 * #endif: BAREFOOTD
 *
 *
 * "clientauth" is the authentication established for the client-rule, or
 * NULL if no authentication has yet been established for the client rule.
 * "srcauth" is the current authentication established for communicating with
 * "src".  It may be AUTHMETHOD_NONE or AUTHMETHOD_NOTSET and may be updated
 * by this function if an authentication-method is successfully established.
 * "state" is the state of the connection.
 * "msg" is filled in with any message/information provided when checking
 * access, "msgsize" is the size of "msg".
 *
 * Wildcard fields are supported for the following fields;
 *      ipv4:         INADDR_ANY
 *      port:         none [enum]
 *
 * "rule" is filled in with the contents of the matching rule.
 *
 * Returns:
 *      True if request should be allowed.  If so, "dstresolved", if not NULL,
 *      will contain the address the request was allowed for, which may be
 *      different from "dst" if "dst" had to be resolved or reverse-mapped
 *      in order to decide on the verdict.
 *
 *      Returns false otherwise.
 */

int
command_matches(const int command, const command_t *commands);
/*
 * Returns true if "command" is set in "commands".
 * False otherwise.
 */

int
protocol_matches(const int protocol, const protocol_t *protocols);
/*
 * Returns true if "protocol" is set in "protocols".
 * False otherwise.
 */

int
proxyprotocol_matches(const int protocol, const proxyprotocol_t *protocols);
/*
 * Returns true if "protocol" is set in "protocols".
 * False otherwise.
 */


int
sockd_connect(int s, const sockshost_t *dst);
/*
 * Tries to connect socket "s" to the host given in "dst".
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

int
shmem2config(const struct config *old, struct config *new);
/*
 * Copies a config from "old", a config in shmem, to "new", a config
 * in regular memory.  The copy includes allocating memory for attributes
 * as necessary (i.e., it's a "deep copy").
 *
 * Note that memory that is only set once at startup and never changed
 * is not copied from "old".  Instead it is copied from the processes
 * current config object.
 *
 * Returns 0 on success, -1 on failure.
 */

void
pointer_free(struct config *config);
/*
 * Frees all pointers in "config".
 */

size_t
pointer_size(struct config *config);
/*
 * Calculates the size of all the memory pointed to by all pointers (and
 * pointers to pointers, etc.) in the config "config".
 *
 * If an object is of an opaque type, and the size of the memory it
 * points to can not be determined, it is not counted.
 *
 * Returns the size of the memory pointed to by all pointers in config.
 */

int
pointer_copy(struct config *src, const ptrdiff_t srcptroffset,
             struct config *dst, void *mem, const size_t memsize);
/*
 * Does a deep copy of the pointers in the config "src" to the config "dst".
 * If "srcptroffset" is not 0x0, it indicates that all the pointers in
 * "src" needs to be offset by this number of bytes to get the correct
 * address (i.e., the memory for the pointers were allocated in a different
 * process at a different address, based on one big block of memory).
 *
 * NOTE: The function call will update the pointers in "src" before copying,
 * and upon return, the pointers will point to the correct address.
 *
 * If "memsize" is not 0, it gives the size of "mem", which must point to a
 * memory area big enough to hold the contents of everything the pointers in
 * "src" point to
 *
 * Otherwise, if "memsize" is 0, the memory to hold the area each pointer
 * points to is allocated by this function via malloc(3) and must be
 * free(3)-ed in the usual way.
 *
 * Returns 0 on success, -1 on failure.
 */

size_t
compareconfigs(const struct config *a, const struct config *b);
/*
 * Checks if config "a" and the contents of all it points to
 * equals that of config "b" and all it points to.
 *
 * Returns 0 if the configs are not equal.
 * Otherwise the configs are equal and the return value indicates the number
 * of bytes compared.
 */



int
send_req(int s, sockd_request_t *req);
/*
 * Sends "req" to "s".
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

int
send_client(int s, const sockd_client_t *client,
            const char *req, const size_t reqlen);
/*
 * Sends the client "client" to the process connected to "s".
 * If "reqlen" is not 0, it is data that has already been read from the
 * client, but not forwarded.  This Can only happen in the case of COVENANT.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

/*
 * Returns a value indicating whether relaying from "src" to "dst" should
 * be permitted.
 */

int
selectmethod(const int *methodv, const size_t methodc,
      const unsigned char *offeredv, const size_t offeredc);
/*
 * Selects the best method based on available methods and given
 * priority.
 * "methodv" is a list over available methods, methodc in length.
 * "offerdv" is a list over offered methods, offeredc in length.
 * The function returns the value of the method that should be selected,
 * AUTMETHOD_NOACCEPT if none is acceptable.
 */

negotiate_result_t
method_uname(int s, request_t *request, negotiate_state_t *state);
/*
 * Enters username/password sub negotiation.  If successful,
 * "request->auth.mdata.uname" is filled in with values read from client.
 * If unsuccessful, the contents of "uname" is indeterminate.
 * After negotiation has finished and the response to client has been sent
 * the function returns.
 * Returns:
 *      On success: 0 (user/password accepted)
 *      On failure: -1  (user/password not accepted, communication failure,
 *                       or something else.)
 */

#if HAVE_GSSAPI
negotiate_result_t
method_gssapi(int s, request_t *request, negotiate_state_t *state);
/*
 * Enters gssapi sub negotiation.  If successful, "request->auth.mdata.gssapi"
 * is filled in with values read from client.
 * If unsuccessful, the contents of "gssapi" is indeterminate.
 *
 * After negotiation has finished and the response to client has been sent
 * the function returns.
 *
 * Returns:
 *      On success: 0 (authentication and encryption token accepted)
 *      On failure: -1  (authentication or encryption token not accepted,
 *                       communication failure, or something else.)
 */
#endif /* HAVE_GSSAPI */

/*
 * Functions for more consistent log messages.
 * XXX move to separate header file.
 */

void
sockd_freelogobject(logtype_t *logobject, const int closetoo);
/*
 * Frees memory associated with the logobject "logobject".
 * If "closetoo" is set, closes the files associated too.
 */

int
sockd_reopenlogfiles(logtype_t *log, const int docloseold);
/*
 * Reopens all logfiles in "log".  If "docloseold" is true, closes
 * old logfiles first.
 */


int loglevel_errno(const int e,  const interfaceside_t side);
int loglevel_gaierr(const int e, const interfaceside_t side);
/*
 * Returns the LOG_* level the error "e" has been configured to be
 * logged at, or LOG_DEBUG if no particular loglevel has been configured
 * for this error.
 *
 * "side" is the interface-side (internal/external) the error occurred on.
 *
 * Returns the appropriate loglevel, or -1 if no loglevel configured.
 */

const int *
errnovalue(const char *symbol);
/*
 * Returns a zero-terminated list of errnovalues corresponding to the
 * symbolic errno-name "symbol".
 *
 * "symbol" can also be a Dante-specific alias that will expand to
 * multiple error-values, which is the reason for returning a list.
 *
 * Returns NULL if "symbol" is unknown.
 */


const int *
gaivalue(const char *symbol);
/*
 * Returns a zero-terminated list of libresolv errorvalues (like those
 * returned by getaddrinfo(3) and family) corresponding to the
 * symbolic error-name "symbol".
 *
 * "symbol" can also be a Dante-specific alias that will expand to
 * multiple error-values, which is the reason for returning a list.
 *
 * Returns NULL if "symbol" is unknown.
 */


iologaddr_t *
init_iologaddr(iologaddr_t *addr,
               const objecttype_t local_type, const void *local,
               const objecttype_t peer_type, const void *peer,
               const authmethod_t *auth, const struct hostid *hostid);
/*
 * Inits "addr" based on the passed arguments.  If "local" or "peer" is not
 * NULL, "local_type" or "peer_type" indicates what kind of object "local"
 * or "peer" is.
 *
 * Returns a pointer to "addr".
 */


void
iolog(const rule_t *rule, const connectionstate_t *state, const operation_t op,
      const iologaddr_t *src, const iologaddr_t *dst,
      const iologaddr_t *tosrc_proxy, const iologaddr_t *todst_proxy,
      const char *data, size_t datalen);

/*
 * Called after each each complete io operation
 * (read then write, or read then block).
 * Does misc. logging based on the log options set in "log".
 * - "rule" is the rule that matched the iooperation, not "const" due to
 *    possible libwrap interaction.
 * - "state" is the state of the connection.
 * - "op" is the operation that was performed.
 * - "src" is where data was received from.
 * - "dst" is where data was written to.
 * - "tosrc_proxy", if not NULL, is the proxy used in the serverchain to
 *    reach "src.host".
 * - "todst_proxy", if not NULL, is the proxy used in the serverchain to
 *    reach "dst.host".
 * - "data" and "datalen" are interpreted depending on "operation".
 */

char *
build_addrstr_src(const struct hostid *hostid, 
                  const sockshost_t *peer, const sockshost_t *proxy_ext,
                  const sockshost_t *proxy, const sockshost_t *local,
                  const authmethod_t *peerauth, const authmethod_t *proxyauth,
                  char *str, size_t strsize);

char *
build_addrstr_dst(const sockshost_t *local, const sockshost_t *proxy,
                  const sockshost_t *proxy_ext, const sockshost_t *peer,
                  const authmethod_t *peerauth, const authmethod_t *proxyauth,
                  const struct hostid *hostid, 
                  char *str, size_t strsize);

void
io_delete(int mother, sockd_io_t *io, int fd, const iostatus_t status);
/*
 * deletes the io object "io".  "fd" is the descriptor on which "status"
 * was returned.  If "fd" is negative, it is ignored.
 * If "mother" is >= 0, the deletion of "io" is ACK'ed to her.
 * "status" is the reason for why the io was deleted.
 */


void
close_iodescriptors(const sockd_io_t *io);
/*
 * A subset of io_delete().  Will just close all descriptors in
 * "io".
 */

int
sockdnegotiate(int s);
/*
 * Sends the connection "s" to a negotiator child.
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

void
run_monitor(void);
/*
 * Sets a io child running.
 */

void
run_io(void);
/*
 * Sets a io child running.
 *
 * A child starts running with zero clients and waits
 * indefinitely for mother to send at least one.
 */

void
run_negotiate(void);
/*
 * Sets a negotiator child running.
 * A child starts running with zero clients and waits
 * indefinitely for mother to send at least one.
 */

void
run_request(void);
/*
 * Sets a request child running.
 * A child starts running with zero clients and waits
 * indefinitely for mother to send at least one.
 */

void mother_preconfigload(void);
void monitor_preconfigload(void);
void negotiate_preconfigload(void);
void request_preconfigload(void);
void io_preconfigload(void);

void mother_postconfigload(void);
void monitor_postconfigload(void);
void negotiate_postconfigload(void);
void request_postconfigload(void);
void io_postconfigload(void);
/*
 * Process-specific post/pre-processing after loading config.
 */

void detach_from_shmem(void);
/*
 * Detaches from shmem not all processes need to be attached to.
 * Processes who need it, will attach later as needed.
 */


void mother_envsetup(int argc, char *argv[]);
/*
 * Cleans up surrounding environment before we start forking of processes.
 * argc and argv are the argc/argv arguments passed main.
 */

char *
mother_getlimitinfo(void);
/*
 * returns a string with some information about current state and limits.
 */

void
log_rusage(const int childtype, const pid_t pid, const struct rusage *rusage);
/*
 * Logs the rusage in "rusage" for a child of type "childtype with the
 * pid "pid".  If pid is 0, assumes "rusage" is for all children of
 * the given childtype.
 */



void
checkconfig(void);
/*
 * Scans through the config, perhaps fixing some things and warning
 * about strange things, or errors out on serious mistakes.
 */


int
send_io(int s, sockd_io_t *io);
/*
 * Sends the io-object "io" to "s".
 * Returns
 *    On success: 0
 *    On failure: -1
 */


int
recv_io(int mother, sockd_io_t *io);
/*
 * Attempts to read a new io object from "mother".
 * If a io is received it is either copied into "io", or it's copied
 * Returns:
 *      On success: 0
 *      On failure: -1.  Errno will be set.
 */

int
recv_req(int s, sockd_request_t *req);
/*
 * Receives a request from the socket "s" and stores it in "req".
 * Returns:
 *      On success: 0
 *      On failure: -1
 */

negotiate_result_t
recv_clientrequest(int s, request_t *request, negotiate_state_t *state);
/*
 * Reads a request from the socket "s", which can be set to non-blocking.
 * "request" will be filled in as reading progresses but it should
 * be considered of indeterminate contents until the whole request
 * has been read.
 * Returns the result (continue, finished, error).
 */

negotiate_result_t
recv_sockspacket(int s, request_t *request, negotiate_state_t *state);
/*
 * When method negotiation has finished (if appropriate) this function
 * is called to receive the actual packet.
 */

void
disable_childcreate(int err, const char *reason);
/*
 * Disables creation of children.  "err" is the errno reason to do so, if
 * any, while "reason" is an additional textual description.
 */

void
enable_childcreate(void);
/*
 * Enables creation of children again.
 */


sockd_child_t *
getchild(pid_t pid);
/*
 * Attempts to find a child with pid "pid".
 * Returns:
 *      On success: a pointer to the found child.
 *      On failure: NULL.
 */

void
sigchildbroadcast(int sig);
/*
 * Sends signal "sig" to all children of type "childtype".
 */

int
fillset(fd_set *set, size_t *negc, size_t *reqc, size_t *ioc);
/*
 * Sets every child's descriptor in "set", as well as sockets we listen on.
 * "negc", "reqc", and "ioc" is upon return filled in with the number of
 * currently free negotiate slots, request slots, and io slots, respectively.
 *
 * Returns the number of the highest descriptor set, or -1 if none was set.
 */

void
clearset(whichpipe_t type, const sockd_child_t *child, fd_set *set);
/*
 * Clears every descriptor of type "type" in "child" from "set".
 * "type" gives the type of pipe that must be set.
 */

void
clearchildtype(const int childtype, whichpipe_t pipetype,
               const int nfds, fd_set *set);
/*
 * Like clearset(), but for all children of the type "childtype".
 */

sockd_child_t *
getset(whichpipe_t type, fd_set *set);
/*
 * If there is a child with a descriptor set in "set", a pointer to
 * the child is returned.
 * "type" gives the type of pipe that must be set.
 * The children returned are returned in prioritized order.
 * If no child is found, NULL is returned.
 */

sockd_child_t *
nextchild(const int type, const int protocol);
/*
 * Returns:
 *      On success: pointer to a child of correct type with at least one free
 *      slot of protocol type "protocol".
 *      On failure: NULL.
 */

#if HAVE_SCHED_SETAFFINITY
/*
 * Modelled after the CPU_SET() macros.
 */

size_t
cpu_get_setsize(void);

void
cpu_set(const int cpu, cpu_set_t *set);

void
cpu_zero(cpu_set_t *set);

int
cpu_isset(const int cpu, const cpu_set_t *set);

int
cpu_equal(const cpu_set_t *set1, const cpu_set_t *set2);

int
cpu_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *mask);

int
cpu_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *mask);

int
sockd_cpuset_isok(const cpu_set_t *set);
/*
 * Returns false if one or more of the cpus set in "set" is not valid
 * on the current system.  True otherwise.
 */
#endif /* HAVE_SCHED_SETAFFINITY */

#if HAVE_SCHED_SETSCHEDULER

int
cpupolicy2numeric(char *name);
/*
 * return cpupolicy id value for given policy name, or -1 if not found.
 */

char *
numeric2cpupolicy(int value);
/*
 * return pointer to name for given policy id value, or NULL if not found.
 */

#endif /* HAVE_SCHED_SETSCHEDULER */

int
sockd_setcpusettings(const cpusetting_t *old, const cpusetting_t *new);
/*
 * Applies the cpusettings in "new" to the current process.
 * "old", if not NULL, contains the current settings, and is used to avoid
 * reapplying existing settings.
 *
 * Returns 0 on success, -1 on failure.
 */

void
setsockoptions(const int s, const sa_family_t family, const int type,
               const int isclientside);
/*
 * Sets options _all_ server sockets should have set on the socket "s".
 *
 * "family" gives the socket family (AF_INET or AF_INET6),
 * "type" gives the socket type (SOCK_STREAM or SOCK_DGRAM),
 * "isclientside" says if the socket is to be used to receive data
 * from the client.
 */

void
sockdexit(const int exitcode)
   __ATTRIBUTE__((noreturn));
/*
 * Exits with the value of "exitcode".
 */


/*
 * DNS-functions.
 * Same API as libresolv, but uses internal cache if possible.
 */

struct hostent *
cgethostbyname(const char *name);

struct hostent *
cgethostbyaddr(const void *addr, socklen_t len, int type);

int
cgetnameinfo(const struct sockaddr *addr, const socklen_t addrlen,
             char *host, const socklen_t hostlen, char *service,
             const socklen_t servicelen, const int flags);

int
cgetaddrinfo(const char *name, const char *service,
             const struct addrinfo *hints, struct addrinfo **res,
             dnsinfo_t *resmem);
/*
 * Like getaddrinfo(3), but "resmem" is used to hold the contents of "res",
 * rather than allocating the memory for "res" dynamically and then
 * having to call freeaddrinfo(3).
 */


#if !HAVE_PRIVILEGES
int
sockd_seteugid(const uid_t uid, const gid_t gid);
/*
 * Sets the effective user/group-id to "uid" and "gid", and updates
 * sockscf.state.{euid,egid} if successful.
 *
 * Returns 0 on success, -1 on failure.
 */
#endif /* !HAVE_PRIVILEGES */

int
sockd_initprivs(void);
/*
 * Initializes things based on configured userid/privilege settings.
 * Returns 0 if inited ok, -1 if not.
 */

void
sockd_priv(const privilege_t privilege, const priv_op_t op);
/*
 * Acquires or releases the privilege associated with the privilege
 * "privilege".
 * "op" indicates whether the privilege should be acquired or relinquished,
 * and must have one of the values PRIV_ON or PRIV_OFF, correspondingly.
 */

void
resetprivileges(void);
/*
 * Resets privileges to correct state based on config.
 * Should be called when starting and after sighup.
 */


int
usermatch(const authmethod_t *auth, const linkedname_t *userlist);
/*
 * Checks whether the username in "auth" matches a name in the
 * list "userlist".
 * Returns:
 *    If match: true.
 *      Else: false.
 */

int
groupmatch(const authmethod_t *auth, const linkedname_t *grouplist);
/*
 * Checks whether the username in "auth" matches groupname listed in "userlist".
 * Returns:
 *    If match: true.
 *      Else: false.
 */

#if HAVE_LDAP
int
ldapgroupmatch(const authmethod_t *auth, const rule_t *rule);
/*
 * Checks whether the username in "auth" matches ldap groupname listed
 * in "userlist".
 * Returns:
 *    If match: true.
 *    Else: false.
 */

int
ldapgroupmatches(const authmethod_t *auth,
                 const char *username, const char *userdomain,
                 const char *group, const char *groupdomain,
                 const rule_t *rule);
/*
 * Checks if user "username" in Kerberos domain "userdomain" is member of
 * ldap group "group" in Kerberos domain "groupdomain".
 * Rule "rule" contains further ldap parameters.
 */

int
ldapauth_passwordcheck(int s, const struct sockaddr_storage *src,
                       const struct sockaddr_storage *dst,
                       authmethod_ldap_t *auth,
		       char *emsg, size_t emsgsize);
/*
 * Checks if user password against ldap server
 */


void
cache_ldap_user(const char *username, int result, size_t rulenumber);
/*
 * Add user "username" to cache for rule "rulenumber".
 * "retval" gives the result to cache.
 * XXX result should be enum, and used in ldap_user_is_cached() also?
 */

int
ldap_user_is_cached(const char *username, size_t rulenumber);
/*
 * Checks if user "username" is cached for rule "rulenumber".
 * Returns:
 *    If not cached: -1
 *    Else: 0 or 1
 */

char *
asciitoutf8(char *input);
/*
 * Checks if string contains character > 127 and converts them to UTF8
 */

char *
hextoutf8(const char *input, int flag);
/*
 * Convert hex input to UTF8 character string
 * flag = 2 convert input (all)
 * flag = 1 convert input (group name/basedn and realm)
 * flag = 0 convert input (only group name/basedn)
 * XXX flag should be enum.
 */

#endif /* HAVE_LDAP */

#if HAVE_PAC

#define MAX_BASE64_LEN 256

int
sidmatch(const authmethod_t *auth, const linkedname_t *objectsids);
/*
 * Checks whether the username in "auth" matches SID listed
 * in "userlist".
 * Returns:
 * If match: true.
 * Else: false.
 */

int
binsidtob64(const char *bsid, char *b64buf, int bsid_len, size_t b64buflen);
/*
 * Converts binary format SID to a base64 encoded string.
 * Returns:
 *  If success:  0
 *  If Failure: -1
 */

int
sidtob64(const char *sid, char *b64buf, size_t b64buflen);
/*
 * Converts a SID string to a base64 encoded string.
 * Returns:
 *   If success:  0
 *   If failure: -1
 */

int
b64tosid(const char *b64, char *sidbuf, size_t sidbuflen);
/*
 * Converts a base64 encoded string to a SID string.
 * Returns:
 *   If success:  0
 *   If failure: -1
 */

#endif /* HAVE_PAC */

unsigned long
medtv(struct timeval *tvarr, size_t tvsize);
/*
 * Return median time in microseconds from sorted timeval array with
 * tvsize entries.
 */

unsigned long
avgtv(struct timeval *tvarr, size_t tvsize);
/*
 * Return average time in microseconds from sorted timeval array with
 * tvsize entries.
 */

unsigned long
stddevtv(struct timeval *tvarr, size_t tvsize, unsigned long avg);
/*
 * Return standard deviation in microseconds for sorted timeval array with
 * tvsize entries and an average of avg.
 */

int
accesscheck(int s, authmethod_t *auth, const struct sockaddr_storage *src,
            const struct sockaddr_storage *dst, char *emsg, size_t emsgsize)
            __ATTRIBUTE__((__BOUNDED__(__buffer__, 5, 6)));
/*
 * Checks whether access matches according to supplied arguments.
 * "auth" is the authentication to be matched against,
 * "s" is the socket the client is connected to,
 * "src" is address client connected from, "dst" is address client
 * connected to.
 * "emsg" is a buffer that information can be written into, "emsgsize"
 * is the size of that buffer.
 *
 * Returns:
 *      If access is ok: true.
 *      Otherwise: false.  Writes the reason into "emsg".
 */

int
passwordcheck(const char *name, const char *cleartextpassword,
              char *emsg, size_t emsglen)
              __ATTRIBUTE__((__BOUNDED__(__buffer__, 3, 4)));
/*
 * First it checks whether "name" is in the password file.  If
 * "cleartextpassword" is NULL, as is the case if called as part of
 * rfc931/ident authentication, that is all that is done.
 *
 * If "cleartextpassword" is not NULL, also checks if "name"'s
 * password in the system passwordfile is "cleartextpassword".
 *
 * Returns:
 *      If "name" and "cleartextpassword" is matched: 0
 *      Otherwise: -1.  "emsg" is filled in with the error message.
 */

int
pam_passwordcheck(int s,
      const struct sockaddr_storage *src, const struct sockaddr_storage *dst,
      const authmethod_pam_t *auth, char *emsg, size_t emsglen)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 5, 6)));
/*
 * Checks whether pam grants access to the client connected to the socket "s".
 * "src" is the clients source address, "dst" is address we accepted the
 * clients connection on.
 *
 * Returns:
 *      If "name" and "cleartext password" is matched: 0
 *      Otherwise: -1.  "emsg" is filled in with the error message.
 */

int
bsdauth_passwordcheck(int s,
      const struct sockaddr_storage *src, const struct sockaddr_storage *dst,
      authmethod_bsd_t *auth, char *emsg, size_t emsgsize)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 5, 6)));
/*
 * Checks whether bsd authentication grants access to the client
 * connected to the socket "s".  "src" is the clients source address,
 * "dst" is address we accepted the clients connection on.
 *
 * Returns:
 *      If "name" and "cleartext password" is matched: 0
 *      Otherwise: -1.  "emsg" is filled in with the error message.
 */

void
redirectsetup(void);
/*
 * sets up things for using the redirect module.
 * Must be called at start and after sighup by main mother.
 */

int
redirect(int s, struct sockaddr_storage *addr,
#if !BAREFOOTD
         sockshost_t *host,
#endif /* !BAREFOOTD */
         int command, const ruleaddr_t *from
#if !BAREFOOTD
         , const ruleaddr_t *to
#endif /* !BAREFOOTD */
         );
/*
 * "s" is the socket to use for performing "command", while  "from"
 * and "to" are the redirect from/to values specified in the redirect
 * statement from the matching rule.
 *
 * The meaning of "addr" and "host" varies depending on what "command" is.
 * When passed to the redirect() function, they contain the values
 * that will be used if the redirect() function does not change them.
 *
 * The redirect() function supports all commands and will change
 * "addr" and host as follows for the different commands:
 *      SOCKS_BIND:
 *         "addr" is local address of "s", to accept remote connection on.
 *         "host" is ignored.
 *
 *      SOCKS_BINDREPLY:
 *         "addr" is the address to say bindreply is from.
 *         "host" is the address to send reply to (only for bind extension).
 *
 *      SOCKS_CONNECT:
 *         "addr" is local address of socket "s".
 *         "host" is host to connect to.
 *
 *      case SOCKS_UDPASSOCIATE:
 *         "addr" is the address to send the udp packet from.
 *         "host" is the address to send packet to.
 *
 *      case SOCKS_UDPREPLY:
 *         "addr" is the address to say the reply is from.
 *         "host" is the address to send the reply to.
 *
 * "host", "addr", and the local address of "s" will be changed if needed.
 *
 * Returns:
 *      On success: 0.
 *      On failure: -1.
 */

void
hostcachesetup(void);
/*
 * Initializes the hostcache.  Must be called before any calls to
 * cgethostby*().
 */

void
hostcacheinvalidate(void);
/*
 * Invalidates all entries in the hostcache.
 */

void
ldapcachesetup(void);
/*
 * Initializes the ldapcache.  Must be called before any calls to
 * ldap functions.
 */

void
ldapcacheinvalid(void);
/*
 * Clear all ldap cache entries.
 */

char *
sockd_getshmemname(const unsigned long id, const statekey_t key);
/*
 * Returns the shmemname corresponding to the id "id" and key "key".
 */

void
sockd_shmdt(rule_t *rule, const int which);
/*
 * Detaches shared memory segments in rule "rule" as indicated by which,
 * and sets the value of the detached objects to not in use.
 */

int
sockd_shmat(rule_t *rule, int which);
/*
 * Attaches shared memory segments in "rule" as indicated by which.
 *
 * The attachments that fail are set to NULL/-1.  It is therefore important
 * to check shmid/object twice if trying to map more than one (when which
 * does not indicate a single object); once before calling sockd_shmat(),
 * and once after, as the first sockd_shmat() call may have failed on some
 * or all the shmem segments and NULL-ed them if so.
 *
 * Returns 0 on success, -1 if one or more attachments failed.
 */

void
shmem_setup(void);
/*
 * sets up things for using shared memory.
 */

void
shmem_idupdate(struct config *config);
/*
 * Goes through "sockscf" and allocates shmemids for all objects that need
 * it.
 */


int
shmem_alloc(const size_t len, const key_t key);
/*
 * allocate shared memory of size "len" and the shmid of the memory.
 * Return -1 on failure.
 */

int
shmem_unuse(shmem_object_t *object, const clientinfo_t *cinfo, int lock);
/*
 * Says we are no longer using "object" on behalf of the client in "cinfo".
 * "lock" is used for locking.  If it is -1, no locking is enforced.
 *
 * Returns 0 on success, -1 on failure.
 */

int
shmem_use(shmem_object_t *object, const clientinfo_t *cinfo, const int lock,
          const int mapisopen);
/*
 * Marks "object" as in use on behalf of the client in "cinfo".
 * "lock" is used for locking.  If it is -1, no locking is enforced.
 *
 * If "mapisopen" is true, any keymap in shmem to be used for keystate
 * is already open.  Upon function return, the open/close state of the
 * keystate map should remain as it was.
 *
 * Returns 0 on success, -1 on failure.
 */

int
shmem_userule(rule_t *rule, const clientinfo_t *cinfo, char *emsg,
              const size_t emsglen);
/*
 * Uses shmem-resources set in rule "rule".
 * Returns 0 on success.
 * Returns -1 on error.  In this case, "emsg" contains the reason.
 */

void
monitor_use(shmem_object_t *stats, const clientinfo_t *cinfo, const int lock);
/*
 * Marks alarms in "stats" as in use on behalf of the client in "cinfo".
 * "lock" is used for locking.  If it is -1, no locking is enforced.
 */

void
monitor_unuse(shmem_object_t *stats, const clientinfo_t *cinfo, const int lock);
/*
 * Decrements usage count for alarms in "stats" on behalf of the client
 * in "cinfo".
 * "lock" is used for locking.  If it is -1, no locking is enforced.
 */

void
monitor_sync(shmem_object_t *alarm, const unsigned long shmid,
             const size_t alarmsconfigured, const size_t sidesconnected,
             const clientinfo_t *cinfo, const int lock);
/*
 * Called a sighup invalidates the old monitor and we want to transfer
 * the settings to the new monitor referenced by "alarm".
 * "alarmsconfigured" gives the alarms configured on the new monitor, and
 * "sidesconnected" is the session-sides currently connected (internal,
 * external, or both).
 */

void
alarm_inherit(rule_t *from, const clientinfo_t *cinfo_from,
              rule_t *to,   const clientinfo_t *cinfo_to,
              const size_t sidesconnected);
/*
 * Handles alarm inheritance from rule "from" into rule "to".
 * "sidesconnected" is the sides connected (internal/external).
 * Note that for udp, there is only one side possible, the internal.
 */


void
alarm_add_connect(rule_t *alarm, const size_t sides,
                  const clientinfo_t *cinfo, const int lock);
/*
 * Adds a connect to the alarmobject referenced by "rule", on the sides
 * indicated by sides.
 */


void
alarm_add_disconnect(const int weclosedfirst, rule_t *alarm,
                     const size_t sides, const clientinfo_t *cinfo,
                     const char *reason, const int lock);
/*
 * Adds a disconnect to the alarmobject referenced by "rule", on the sides
 * indicated by "sides".
 *
 * If "weclosefirst" is set, the disconnect/close by peer is considered a
 * response to a previous close by us.
 *
 * "reason" is the reason for for the disconnect.
 */

void
alarm_remove_session(rule_t *alarm, const size_t sides,
                     const clientinfo_t *cinfo, const int lock);
/*
 * Removes a previously added session on the sides "sides" from the
 * alarmobject referenced by "rule".
 *
 * Typically used when a session is to be inherited by another
 * monitor, and we want to remove the session from the old monitor
 * without affecting other things.
 */

void *
sockd_mmap(void *oldmap, size_t size, const int prot, const int flags,
           const int fd, const int docreate);
/*
 * Allocates shared memory of size "size", using "fd" for storage
 * and mmap()s it.
 * If "oldmap" is not NULL, it should be a previously mmap(2)-ed segment that
 * should now be re-mmap(2)-ed.
 *
 * If "docreate" is set, this is a call to create or extend the memory and
 * function will make sure to extend the file referenced by fd to at least
 * "size" bytes, but will not explicitly initialize the memory.
 * If "docreate" is not set, this is a remap of a previously created
 * shmem segment and the file referenced by fd will not be touched.
 *
 * Returns a pointer to the memory allocated, or MAP_FAILED on failure.
 */

ssize_t
keystate_index(shmem_object_t *shmem, const clientinfo_t *cinfo,
               const int expireoldtoo);
/*
 * Returns the index of "cinfo" in shmem's keystate array,
 * or -1 if no such index exists.
 * If "expireoldtoo" is set, expired old keystate entries in shmem,
 * if any, too.
 */

int
keystate_openmap(const unsigned long id, keystate_t *keystate,
                 size_t *sizemapped);
/*
 * opens and mmap's the file corresponding to "id" and "keystate".
 * If "mappedsize"  is not NULL, it will contain the size of the
 * object mapped.

 * Note that the lock corresponding to keystate should be taken
 * before calling this function, to make sure the correct size
 * is mmap'ed.
 *
 * Returns 0 on success, -1 on failure.
 */

void
keystate_closemap(const unsigned long id, keystate_t *keystate,
                  const size_t mappedsize, const ssize_t changedindex);
/*
 * Closes the previously opened keystate map of size "mappedsize".
 * Also truncates the file used if the current size is different from
 * the size mapped.
 *
 * If "datawaschanged" is true it means some of the data was changed, and
 * a msync(2) may have to be done.
 */

int
keystate_hasexpired(const shmem_object_t *shmem, const size_t keyindex,
                    const struct timeval *timenow);
/*
 * "timenow" is the current time.
 *
 * Returns true if the keystate at index "keyindex" in "shmem" has expired
 * or can be reset.
 * Returns false otherwise.
 */

void
disconnectalarm_use(shmem_object_t *disconnectalarm, const clientinfo_t *cinfo,
                    const int lock);
/*
 * Marks "disconnectalarm" as in use by client "cinfo".
 */


void
disconnectalarm_unuse(shmem_object_t *disconnectalarm,
                      const clientinfo_t *cinfo, const int lock);
/*
 * Marks "disconnectalarm" as no longer in use by client "cinfo".
 */

void
bw_use(shmem_object_t *bw, const clientinfo_t *cinfo, const int lock);
/*
 * Marks "bw" as in use by client "cinfo".
 */

void
bw_unuse(shmem_object_t *bw, const clientinfo_t *cinfo, const int lock);
/*
 * Says client "cinfo" is no longer using "bw".
 * If "bw" is NULL, nothing is done.
 */

ssize_t
bw_left(const shmem_object_t *bw, const int lock);
/*
 * Returns how many bytes we should read if the client is restricted
 * by "bw".
 */

void
bw_update(shmem_object_t *bw, size_t bwused, const struct timeval *bwusedtime,
          const int lock);
/*
 * Updates "bw".  "bwused" is the bandwidth used (in bytes) at time
 * "bwusedtime".
 */

int
bw_rulehasoverflown(const rule_t *rule, const struct timeval *tnow,
                    struct timeval *overflowok);
/*
 * Checks if the bandwidth limit for the bw-object in "rule" has overflown.
 * If it has overflown, "overflowok" is set to the time when we can again do
 * i/o over the io objects using this rule.
 *
 * Return true if bw has overflown.  In this case, "overflowok" will be set.
 * Returns false if bw has not overflown.
 */


int
session_use(shmem_object_t *ss, const clientinfo_t *cinfo, const int lock,
            char *emsg, const size_t emsglen);
/*
 * Allocates a session for client "cinfo".
 *
 * Returns true on success, false otherwise.  If false, the reason is
 * printed to "emsg".
 */

void
session_unuse(shmem_object_t *ss, const clientinfo_t *cinfo, const int lock);
/*
 * Says "cinfo" is no longer using "ss".
 */



void
checkmodule(const char *name);
/*
 * Checks that the system has the module "name" and permission to use it.
 * Aborts with an error message if not.
 */

char *
licensekey2string(const licensekey_t *key);
/*
 * Returns a printable representation of the licensekey "key".
 * Exits on error.
 */



int sockd_handledsignals(void);
/*
 * Check if we have received any signal, and calls the appropriate
 * signal handler if so.
 *
 * Returns 1 if a signal handler was called, 0 otherwise.
 */


struct sockaddr_storage *
getoutaddr(struct sockaddr_storage *laddr,
           const struct sockaddr_storage *client_laddr,
           const struct sockaddr_storage *client_raddr,
           const int command, const struct sockshost_t *request,
           char *emsg, const size_t emsglen);
/*
 * Gets the outgoing IP address to use.
 *
 * "client_laddr" is address we accepted the client on.
 " "client_raddr" is the address of the client, on whos behalf we are
 *  binding an address on the external side.
 *
 * "command" is the SOCKS command the client requested.
 * "reqhost" is the host in the SOCKS request from the client.
 *
 * The address to use on the external side is stored in "laddr".
 *
 * Returns: "laddr", or NULL on failure.  On failure, "emsg" contains
 * the reason.
 */

struct sockaddr_storage *
getinaddr(struct sockaddr_storage *laddr,
          const struct sockaddr_storage *client,
          char *emsg, const size_t emsglen);
/*
 * Gets the incoming address to use for a connection from the client address
 * "client" (which can be 0.0.0.0) if unknown.  The local address to
 * accept the connection on is saved in "laddr" on success.
 *
 * Only used to decide which address to bind for accepting udp packets *
 * currently.
 *
 * Returns:
 *    On success: laddr, populated appropriately.
 *    On failure: NULL.  Session should be aborted.  Reason for failure is
 *                then stored in "emsg", which must be of len "emsglen".
 */
void
sigserverbroadcast(int sig);
/*
 * Broadcasts "sig" to other "main" servers (started with "-N" option).
 *
 */

void
sockd_pushsignal(const int sig, const siginfo_t *siginfo);
/*
 * Adds the signal "sig" to the end of the internal signal stack.
 */


int
sockd_popsignal(siginfo_t *siginfo);
/*
 * Pops the first signal on the internal signal stack.
 * Returns the signal number, and stores the siginfo in "siginfo".
 */

ipv6_addrscope_t
ipv6_addrscope(const struct in6_addr *addr);
/*
 * Returns the address-scope of the ipv6 address "addr".
 */

unsigned char *
sockd_getmacaddr(const char *ifname, unsigned char *macaddr);
/*
 * Writes the mac-address of the interface named "ifname" to "macaddr",
 * which must be of at least length ETHER_ADDR_LEN.
 * Returns a pointer to macaddress, or NULL if no mac-address
 * is set for the interface.
 */

ssize_t
addrindex_on_listenlist(const size_t listc, const listenaddress_t *listv,
                        const struct sockaddr_storage *addr,
                        const int protocol);
/*
 * Checks if "addr" is on the list of internal addresses, as
 * given by "listc" and 'listv".
 * "protocol" gives the protocol to check for, SOCKS_TCP or SOCKS_UDP.
 *
 * Returns the index of addr in listv if addr is on the list, or -1 if not.
 */

ssize_t
addrindex_on_externallist(const externaladdress_t *external,
                          const struct sockaddr_storage *addr);
/*
 * Checks if "addr" is on the list of external addresses, as
 * given by "external".
 *
 * Returns the index of addr in listv if addr is on the list, or -1 if not.
 */


size_t
maxfreeslots(const int childtype);
/*
 * Returns the maximum number of free slots a child of type "childtype"
 * can have.
 */

int
child_should_retire( const sockd_child_t *child);
/*
 * Returns true if child "child" is ready for retirement, indicating
 * we should not send any more clients to it.
 *
 * Returns false if child "child" is not ready for retiring, meaning
 * it should continue to accept new clients as normal.
 */


void
sockd_print_child_ready_message(const size_t freefds);
/*
 * Prints the appropriate startup message for the child calling this
 * function.
 * "freefds" is the number of free fd's.
 */

int methodcanprovide(const int method, const methodinfo_t what);
/*
 * Returns true if method "method" can possibly provide "what".
 * It does not mean the method will always provide it, only
 * that it can in some cases.
 */

int
methodworkswith(const int method, const methodinfo_t what);
/*
 * Returns true if method "method" can possibly work with "what".
 */


int
rulerequires(const rule_t *rule, const methodinfo_t what);
/*
 * Returns true if rule "rule" requires "what".
 * Returns false otherwise.
 */


void
sighup_child(int sig, siginfo_t *si, void *sc);
/*
 * SIGHUP-handler for everyone but main mother.
 */

void
io_handlesighup(void);
/*
 * Called at sighup to let the i/o childs do what they need to do
 * upon receiving a sighup.
 */

int sockd_check_ipclatency(const char *description,
                           const struct timeval *tsent,
                           const struct timeval *treceived,
                           const struct timeval *tnow);
/*
 * checks whether the delay between a packet sent by process N (e.g.,
 * the negotiate-process) at time "tsent", and received by process
 * N + 1 (e.g., the request-process) at time "treceived" indicates
 * we are overloaded.
 *
 * "tnow" is the current time, and is used to avoid printing warnings
 * too frequently.  It is included in the API for this function in order
 * to avoid an unnecessary system call since "treceived" and "tnow" will
 * usually be the same.
 *
 * Returns true, and possibly prints a warning if we are overloaded.
 * Returns false if no overload condition is detected.
 */


      /*
       * for the i/o processes only.
       */

typedef struct {
   /*
    * number of currently free file descriptors.  Should never become
    * a problem for Dante or Covenant, but Barefoot needs to keep track
    * of it so it does not end up using up all fds for udp clients, then
    * becomes unable to receive a new io from mother.
    */
   size_t freefds;

   /*
    * do any active i/o objects have a connect that we do not know whether
    * have completed or not yet?
    */
   int haveconnectinprogress;

} iostate_t;


#if BAREFOOTD

void
update_clientpointers(const size_t dstc, udptarget_t *dstv);
/*
 * Updates the pointers in "dstv" after a realloc(3).
 */

int
io_remove_session(const size_t ioc, sockd_io_t *iov,
                  const struct sockaddr_storage *laddr, const int protocol,
                  const iostatus_t reason);
/*
 * This function tries to find a session in iov where the local address is
 * "addr" and is using protocol "protocol".  If found, the session is removed.
 * If "addr" is NULL, all sessions using protocol "protocol" are removed.
 *
 * "reason" gives the reason why the session should be removed.
 *
 * Returns:
 *    If a matching session was found: 0.
 *    If no matching session was found: -1.
 */

iostatus_t
io_udp_client2target(sockd_io_direction_t *in, sockd_io_direction_t *out,
                     const authmethod_t *cauth, connectionstate_t *state,
                     iologaddr_t *src, iologaddr_t *dst,
                     int *bad, rule_t *packetrule, size_t *bwused);
/*
 * Tries to read a udp packet from the socket in "in" and send it out on
 * the socket in "out".
 * "state" is the connection state of the io object "in" and "out" belong to.
 *
 * "lowerlevelrule" will be the rule matched at the previous (lower) level,
 * i.e., a clientrule or a hostidrule.
 *
 * "packetrule" is the rule used for sending a packet from "in" to "out".
 *
 * When called it will contain information necessary to call iolog() if
 * an error is detected before we get far enough to be be able to do a
 * full rule lookup.  Upon return, if there was no error, it will contain
 * the rule used for the forwarded packet.
 *
 * Returns a status code indicating whether the packet was forwarded or not.
 * If a fatal error occurred and the session should be removed, "bad" is
 * set to the socket where the error occurred (the socket in "in" or "out"
 * if it can be determined, or it will remain unchanged if not.
 */

iostatus_t
io_udp_target2client(sockd_io_direction_t *in,
                     sockd_io_direction_t *out,
                     const authmethod_t *cauth,
                     connectionstate_t *state,
                     iologaddr_t *src, iologaddr_t *dst,
                     int *bad, rule_t *packetrule, size_t *bwused);
/*
 * The opposite of io_udp_client2target().
 */


#elif SOCKS_SERVER

iostatus_t
io_udp_client2target(sockd_io_direction_t *control, sockd_io_direction_t *in,
                     sockd_io_direction_t *out, const authmethod_t *cauth,
                     connectionstate_t *state, iologaddr_t *clog,
                     iologaddr_t *dlog, int *bad,
                     rule_t *packetrule, size_t *bwused);
/*
 * Tries to read a udp packet from the socket in "in" and send it out on
 * the socket in "out".
 * "state" is the connection state of the io object "in" and "out" belong to.
 *
 * "packetrule" is the rule used for sending a packet from "in" to "out".
 * When called it will contain information necessary to call iolog() if
 * an error is detected before we get far enough to be be able to do a
 * full rule lookup.  If the sockets are connected, "packetrule" may contain
 * all the necessary information and no rule lookup may be necessary.
 * Upon return, if there was no error, it will contain the rule used for the
 * forwarded packet.
 *
 * Returns a status code indicating whether the packet was forwarded or not.
 * If a fatal error occurred and the session should be removed, "bad" is
 * set to the socket where the error occurred (the socket in "in" or "out"
 * if it can be determined, or it will remain unchanged if not.
 */

iostatus_t
io_udp_target2client(sockd_io_direction_t *control,
                     sockd_io_direction_t *in, sockd_io_direction_t *out,
                     connectionstate_t *state,
                     iologaddr_t *clog, iologaddr_t *dlog, int *bad,
                     rule_t *packetrule, size_t *bwused);
/*
 * The opposite of io_udp_client2target().
 */

#endif /* SOCKS_SERVER */

#if COVENANT

int
resend_client(sockd_io_t *io);
/*
 * Resends the client using "io" to mother, for renegotiation.
 * This happens when a http client wants to connect to a different
 * remote server.
 *
 * Returns 0 on success, -1 on error.
 */

int
recv_resentclient(int s, sockd_client_t *client);
/*
 * Receives the resent client from "s".  The resent client
 * stored in "client".
 *
 * Returns 0 on success, -1 on error.
 */

negotiate_result_t
recv_httprequest(int s, request_t *request,
                 negotiate_state_t *state);
/*
 * Reads a http request from the socket "s", which can be set to
 * non-blocking.
 * "request" will be filled in as reading progresses but it should
 * be considered of indeterminate contents until the whole request
 * has been read.
 * Returns:
 *    On success: > 0
 *    On failure: <= 0.  If errno does not indicate the request should be
 *                       be retried, the connection "s" should be dropped.
 */

int
parse_httprequest(request_t *reqinfo, const char *req,
                  char *emsg, size_t emsglen);
/*
 * Parses the http request present in the NUL-terminated string "req".
 * The information extracted from the request is stored in "reqinfo".
 *
 * Returns 0 on success, or -1 on error.  On error, "emsg", of size "emsglen"
 * is filled in with information about the error.
 */

size_t
httpresponse2mem(const int s, const response_t *response,
                 char *buf, const size_t buflen);

#endif /* COVENANT */

#if HAVE_NEGOTIATE_PHASE
int
send_connectresponse(const int s, const int error, sockd_io_t *io);
/*
 * Sends the response to a connect request issued by the client
 * connected to the socket "s".  "io" is the object we created for
 * the client.
 *
 * "error", if not 0, is the errno value corresponding to the failure.
 * If "error" is 0, the connect succeeded, otherwise "error" is the
 * errno value corresponding to the failed connect(2).
 *
 * Returns 0 if the response was sent, -1 if an i/o error occurred.
 */

response_t *
create_response(const sockshost_t *host, authmethod_t *auth,
                const int proxyprotocol, const int responsecode,
                response_t *response);
/*
 * Fills in the responseobject "response" based on data the passed data.
 * "responsecode" is responsecode, using proxyprotocol proxyprotocol" to
 * set the reply to.
 * If "host" is not NULL, it is the host to use in response.
 * If "host" is NULL, an all-zero ipv4 address is used instead.
 */

void
send_failure(const int s, response_t *response, const unsigned int failure);
/*
 * Wrapper around send_response() that sends a failure message to the
 * client connected to "s" and deletes gss state if in use.
 *
 * "response" is the packet we send,
 * "failure" is the errno reason for failure,
 * and "auth" is the agreed on authentication.
 */

int
send_response(int s, const response_t *response);
/*
 * Sends "response" to "s".
 *      On success: 0
 *      On failure: -1
 */

#else /* !HAVE_NEGOTIATE_PHASE */

#define send_failure(s, response, failure)
#define send_response(s, response)            (0)

#endif /* !HAVE_NEGOTIATE_PHASE */

#if DIAGNOSTIC
void doconfigtest(void);
void shmemcheck(void);
/*
 * Internal testing functions.
 */


void
checksockoptions(const int s, const sa_family_t family, const int type,
                 const int isclientside);
/*
 * checks that the expected options are set on the socket and prints
 * warnings if not.
 */
#endif /* DIAGNOSTIC */

#endif /* !_SOCKD_H_ */
