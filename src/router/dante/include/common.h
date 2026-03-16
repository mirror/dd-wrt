/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016, 2017, 2019,
 *               2020, 2021, 2024
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

/* $Id: common.h,v 1.931.4.7.2.8.4.20.4.21 2024/12/05 12:06:05 michaels Exp $ */

#ifndef _COMMON_H_
#define _COMMON_H_

/* ifdef, not if, defined on command line */
#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif /* HAVE_CONFIG_H */

#ifndef NO_OSDEP
#include "osdep.h"
#endif /* !NO_OSDEP */

#include "yacconfig.h"

#include "config.h"

/* global variables needed by everyone. */
extern struct config sockscf;
extern char *__progname;

   /*
    * defines
    */

/*
 * If we are compiling for unit-tests, there are functions
 * that normally have static/file-local scope, but which we
 * want to test in the unit-tests.  The below #define is used as
 * an easy way to make those functions be compiled with global
 * scope for unit-tests.
 */
#if STANDALONE_UNIT_TEST
#define UNIT_TEST_STATIC_SCOPE
#else
#define UNIT_TEST_STATIC_SCOPE static
#endif

#if DIAGNOSTIC
/*
 * Takes too much time, leading to one or more shmem-files usually having
 * been deleted already by mother before child-processes get around to
 * checking them.  Instead enable this only if we (again) suspect there
 * are bugs related to corruption of the shmem-header.
 */
#define DO_SHMEMCHECK (0)
#else
#define DO_SHMEMCHECK (0)
#endif


/*
 * We base ourselves on RFC 5424. (LOG_ALERT (0) - LOG_DEBUG (7))
 */
#define MAXLOGLEVELS        (8)


#define PIPEBUFFER_IS_SEND_BASED       (0)
#define PIPEBUFFER_IS_RECV_BASED       (1)
#define PIPEBUFFER_IS_UNKNOWN_BASED    (0)

#if HAVE_SOLARIS_BUGS
#define HAVE_UNIQUE_SOCKET_INODES   (0)
#else /* !HAVE_SOLARIS_BUGS */
#define HAVE_UNIQUE_SOCKET_INODES   (1)
#endif /* HAVE_SOLARIS_BUGS */

#define SOCKS_IGNORE_SIGNALSAFETY   (0)

#if PRERELEASE

/*
 * Solaris 2.5.1 and it's stream stuff is broken and puts the processes
 * into never-never land forever on half the sendmsg() calls if they
 * involve ancillary data.  (it seems to deadlock the processes.)
 * XXX need to retest what the current status of this is.
 */
/* always enable if PRERELEASE */
#undef HAVE_SENDMSG_DEADLOCK
#define HAVE_SENDMSG_DEADLOCK 1

#endif /* PRERELEASE */

#define TOIN(addr) ((struct sockaddr_in *)(addr))
#define TOCIN(addr) ((const struct sockaddr_in *)(addr))

#define TOIN6(addr) ((struct sockaddr_in6 *)(addr))
#define TOCIN6(addr) ((const struct sockaddr_in6 *)(addr))

#define TOSA(addr) ((struct sockaddr *)addr)
#define TOCSA(addr) ((const struct sockaddr *)addr)
#define TOSS(addr) ((struct sockaddr_storage *)addr)
#define TOCSS(addr) ((const struct sockaddr_storage *)addr)

#define IP_MAXPORT (65535)   /* max value for ip port number. */


/*
 * redefine system limits to match that of socks protocol.
 * No need for these to be bigger than protocol allows, but they
 * _must_ be at least as big as protocol allows.
 */

#ifdef   MAXHOSTNAMELEN
#undef   MAXHOSTNAMELEN
#endif /* MAXHOSTNAMELEN */
#define  MAXHOSTNAMELEN    (255 + 1)      /* socks5: 255, +1 for len. */

#ifndef  MAXSERVICELEN
#define  MAXSERVICELEN    (255 + 1)       /* aka /etc/services. */
#endif /* MAXSERVICELEN */


#ifdef   MAXURLLEN
#undef   MAXURLLEN
#endif /* MAXURLLEN */
#define  MAXURLLEN         (255 + 1)      /* whatever. */

#ifdef   MAXNAMELEN
#undef   MAXNAMELEN
#endif /* MAXNAMELEN */
#define  MAXNAMELEN        (255 + 1)      /* socks5: 255, +1 for len. */

#ifdef   MAXPWLEN
#undef   MAXPWLEN
#endif /* MAXPWLEN */
#define  MAXPWLEN          (255 + 1)      /* socks5: 255, +1 for len. */

#define MAXTCPINFOLEN      (2048)

#if HAVE_GSSAPI
#ifdef MAXGSSAPITOKENLEN
#undef MAXGSSAPITOKENLEN
#endif /* MAXGSSAPITOKENLEN */
#define MAXGSSAPITOKENLEN (1024 * 64 - 1) /* socks5: up to 2^16 - 1 */

/*
 * GSSAPI headerlen (NOTE: SOCKS GSSAPI headerlen.  After stripping of
 * the socks gssapi header, there is another non-socks gssapi header
 * also on the token).
 */
#define GSSAPI_HLEN       (4)

/*
 * XXX should be max-size of exported state, but we don't know what it is.
 * Is there any way to find out?
 */
#define MAX_GSS_STATE     (4000)

#endif /* HAVE_GSSAPI */

#if HAVE_PAC

#ifdef MAXSIDSLEN
#undef MAXSIDSLEN
#endif /* MAXSIDSLEN */
#define MAXSIDSLEN         (200*60 + 1)      /* assume max 40 sids (groups) */

#endif /* HAVE_PAC */

/* max number of socket options to set on the external side, per rule. */
#define MAX_EXTERNAL_SOCKETOPTIONS (5)

#define   MAXIFNAMELEN      (255)

#define   MAXSOCKADDRSTRING  \
               (sizeof("0000:0000:0000:0000:0000:0000:0000:0000.65535"))

/*                                             "." + "65535" + NUL */
#define   MAXSOCKSHOSTSTRING (MAXHOSTNAMELEN + 1  +    5)
#define   MINSOCKSHOSTLEN    (   1 /* ATYPE              */      \
                             +   2 /* DST.ADDR (LEN + 1) */      \
                             +   2 /* DST.PORT           */)

#define   MAXSOCKSHOSTLEN    (   1               /* ATYPE              */      \
                             +   1               /* DST.ADDR LEN       */      \
                             +   MAXHOSTNAMELEN  /* DST.ADDR           */      \
                             +   2               /* DST.PORT           */)

#define   MINSOCKSUDPHLEN    (   2 /* RSV                */      \
                             +   1 /* FRAG               */      \
                             +   MINSOCKSHOSTLEN)

#define   MAXSOCKSUDPHLEN    (   2 /* RSV                */      \
                             +   1 /* FRAG               */      \
                             +   MAXSOCKSHOSTLEN)

#define   MAXRULEADDRSTRING  (MAXSOCKSHOSTSTRING * 2 + 32 /* atype, etc. */)
#define   MAXGWSTRING        (MAXSOCKSHOSTSTRING)


#define MAXSUBDOMAINS        (10) /* a.b.c.d.e.f ... */


#define MAXAUTHINFOLEN      (((sizeof("(") - 1) + MAXMETHODSTRING) \
                           + (sizeof(")") - 1) + (sizeof("@") - 1) + MAXNAMELEN)


#define MAXFACILITYNAMELEN    (8 + 1) /* max length of syslog facility name. */

#ifndef NUL
#define NUL '\0'
#endif /* !NUL */

#define CONFIGTYPE_SERVER      1
#define CONFIGTYPE_CLIENT      2

#define PROTOCOL_TCPs         "tcp"
#define PROTOCOL_UDPs         "udp"
#define PROTOCOL_UNKNOWNs      "unknown"

#define SOCKS_TCP             (1)
#define SOCKS_UDP             (2)

#define RESOLVEPROTOCOL_UDP   (SOCKS_UDP)
#define RESOLVEPROTOCOL_TCP   (SOCKS_TCP)
#define RESOLVEPROTOCOL_FAKE  (3)

#define LOGTYPE_SYSLOG        0x1
#define LOGTYPE_FILE          0x2

/*
 * Some things we may want to log at different levels in the server and
 * the client.  Use #defines here that map to the appropriate level.
 */
#if SOCKS_CLIENT
#define LOG_NEGOTIATE         (LOG_INFO)
#else /* SOCKS_SERVER */
#define LOG_NEGOTIATE         (LOG_DEBUG)
#endif


#define NOMEM                 "<memory exhausted>"

/* environment variables used. */
#define ENV_HTTP_PROXY                     "HTTP_CONNECT_PROXY"
#define ENV_SOCKS4_SERVER                  "SOCKS4_SERVER"
#define ENV_SOCKS5_PASSWD                  "SOCKS5_PASSWD"
#define ENV_SOCKS5_SERVER                  "SOCKS5_SERVER"
#define ENV_SOCKS5_USER                    "SOCKS5_USER"
#define ENV_SOCKS_BINDLOCALONLY            "SOCKS_BINDLOCALONLY"
#define ENV_SOCKS_CONF                     "SOCKS_CONF"
#define ENV_SOCKS_DEBUG                    "SOCKS_DEBUG"
#define ENV_SOCKS_DIRECTROUTE_FALLBACK     "SOCKS_DIRECTROUTE_FALLBACK"
#define ENV_SOCKS_DISABLE_THREADLOCK       "SOCKS_DISABLE_THREADLOCK"
#define ENV_SOCKS_ERRLOGOUTPUT             "SOCKS_ERRLOGOUTPUT"
#define ENV_SOCKS_FORCE_BLOCKING_CONNECT   "SOCKS_FORCE_BLOCKING_CONNECT"
#define ENV_SOCKS_LOGOUTPUT                "SOCKS_LOGOUTPUT"
#define ENV_SOCKS_PASSWD                   "SOCKS_PASSWD"
#define ENV_SOCKS_PASSWORD                 "SOCKS_PASSWORD"
#define ENV_SOCKS_ROUTE_                   "SOCKS_ROUTE_"   /* _<number> */
#define ENV_SOCKS_SERVER                   "SOCKS_SERVER"
#define ENV_SOCKS_USER                     "SOCKS_USER"
#define ENV_SOCKS_USERNAME                 "SOCKS_USERNAME"
#define ENV_TMPDIR                         "TMPDIR"
#define ENV_UPNP_IGD                       "UPNP_IGD"
#define ENV_SOCKS_AUTOADD_LANROUTES        "SOCKS_AUTOADD_LANROUTES"
#define ENV_SOCKS_REDIRECT_FROM            "SOCKS_REDIRECT_FROM"

   /*
    * macros
    */

/*
 * CompileTime assert.  Based on an article in DrDobbs by Ralf Holly.
 * (http://www.drdobbs.com/compile-time-assertions/184401873)
 */
#define CTASSERT(exp)                     \
do {                                      \
   enum { assert_static__ = 1/(exp) };    \
} while (/* CONSTCOND */ 0)

/*
 * Error macros.
 */

#if HAVE_LIVEDEBUG /* try to generate a coredump and continue if server. */

#if SOCKS_CLIENT
#define SET_INTERNAL_ERROR() do {                                              \
   sockscf.state.internalerrordetected = 1;                                    \
} while (/* CONSTCOND */ 0)

#else /* !SOCKS_CIENT */
#define SET_INTERNAL_ERROR() do { } while (/* CONSTCOND */ 0)
#endif /* !SOCKS_CLIENT */

#define HANDLE_RINGBUFFER()                                                    \
do {                                                                           \
   SET_INTERNAL_ERROR();                                                       \
                                                                               \
   if (!sockscf.option.debug)                                                  \
      socks_flushrb();                                                         \
} while (/* CONSTCOND */ 0)

#else  /* !HAVE_LIVEDEBUG */
#define HANDLE_RINGBUFFER() do { } while (/* CONSTCOND */ 0)

#endif /* !HAVE_LIVEDEBUG */

#define SASSERT_STARTSTRING "an internal error was detected at "
#define SASSERT_ENDSTRING   \
"Please report this to Inferno Nettverk A/S at \"" LCPRODUCT "-bugs@inet.no\"."\
"  Please check for a coredump too."

#define SIGNALSLOG_WITH_ERRNO(value, expstr, err)                              \
do {                                                                           \
   char _b[10][32];                                                            \
   const char *_msgv[]                                                         \
   = { SASSERT_STARTSTRING,                                                    \
       __FILE__,                                                               \
       ":",                                                                    \
       ltoa(__LINE__, _b[0], sizeof(_b[0])),                                   \
       ", value ",                                                             \
       ltoa((value), _b[1], sizeof(_b[1])),                                    \
       ", expression \"",                                                      \
       (expstr),                                                               \
       "\", errno ",                                                           \
       ltoa(err, _b[2], sizeof(_b[2])),                                        \
       " (",                                                                   \
       strerror(errno),                                                        \
       ").  Version: ",                                                        \
       rcsid,                                                                  \
       ".  ",                                                                  \
       SASSERT_ENDSTRING,                                                      \
       NULL                                                                    \
     };                                                                        \
                                                                               \
   signalslog(LOG_WARNING, _msgv);                                             \
} while (/* CONSTCOND */ 0)                                                    \

#define SIGNALSLOG_WITHOUT_ERRNO(value, expstr)                                \
do {                                                                           \
   char _b[10][32];                                                           \
   const char *_msgv[]                                                         \
   = { SASSERT_STARTSTRING,                                                    \
       __FILE__,                                                               \
       ":",                                                                    \
       ltoa(__LINE__, _b[0], sizeof(_b[0])),                                   \
       ", value ",                                                             \
       ltoa((value), _b[1], sizeof(_b[1])),                                    \
       ", expression \"",                                                      \
       (expstr),                                                               \
       "\"",                                                                   \
       ".  Version: ",                                                         \
       rcsid,                                                                  \
       ".  ",                                                                  \
       SASSERT_ENDSTRING,                                                      \
       NULL                                                                    \
     };                                                                        \
                                                                               \
   signalslog(LOG_WARNING, _msgv);                                             \
} while (/* CONSTCOND */ 0)

#define SIGNALSLOG_WITH_ERRNO_FAD(_value, expstr, err)                         \
do {                                                                           \
   char _b[10][32];                                                           \
   const char *_msgv[]                                                         \
   = { SASSERT_STARTSTRING,                                                    \
       __FILE__,                                                               \
       ":",                                                                    \
       ltoa(__LINE__, _b[0], sizeof(_b[0])),                                   \
       ", by pid ",                                                            \
       ltoa(getppid(), _b[1], sizeof(_b[1])),                                  \
       ".  Value ",                                                            \
       ltoa((_value), _b[2], sizeof(_b[2])),                                   \
       ", expression \"",                                                      \
       (expstr),                                                               \
       "\", errno ",                                                           \
       ltoa(err, _b[3], sizeof(_b[3])),                                        \
       " (",                                                                   \
       strerror(errno),                                                        \
       ").  Version: ",                                                        \
       rcsid,                                                                  \
       ".  ",                                                                  \
       SASSERT_ENDSTRING,                                                      \
       NULL                                                                    \
     };                                                                        \
                                                                               \
   signalslog(LOG_WARNING, _msgv);                                             \
} while (/* CONSTCOND */ 0)                                                    \

#define SIGNALSLOG_WITHOUT_ERRNO_FAD(_value, expstr)                           \
do {                                                                           \
   char _b[10][32];                                                           \
   const char *_msgv[]                                                         \
   = { SASSERT_STARTSTRING,                                                    \
       __FILE__,                                                               \
       ":",                                                                    \
       ltoa(__LINE__, _b[0], sizeof(_b[0])),                                   \
       ", by pid ",                                                            \
       ltoa(getppid(), _b[1], sizeof(_b[1])),                                  \
       ".  Value ",                                                            \
       ltoa((_value), _b[2], sizeof(_b[2])),                                   \
       ", expression \"",                                                      \
       (expstr),                                                               \
       "\"",                                                                   \
       ".  Version: ",                                                         \
       rcsid,                                                                  \
       ".  ",                                                                  \
       SASSERT_ENDSTRING,                                                      \
       NULL                                                                    \
     };                                                                        \
                                                                               \
   signalslog(LOG_WARNING, _msgv);                                             \
} while (/* CONSTCOND */ 0)

#define SERR_BODY(_value, expstr, _err)                                        \
do {                                                                           \
   const int err = (_err);                                                     \
                                                                               \
   SIGNALSLOG_WITH_ERRNO(_value, expstr, err);                                 \
   HANDLE_RINGBUFFER();                                                        \
   abort();                                                                    \
} while (/* CONSTCOND */ 0)

                                                                               \
#define SERRX_BODY(_value, expstr)                                             \
do {                                                                           \
   SIGNALSLOG_WITHOUT_ERRNO(_value, expstr);                                   \
   HANDLE_RINGBUFFER();                                                        \
   abort();                                                                    \
} while (/* CONSTCOND */ 0)


#define SWARN_BODY(_value, expstr, _err)                                       \
do {                                                                           \
   pid_t forked;                                                               \
   const int err = (_err);                                                     \
                                                                               \
   switch ((forked = fork())) {                                                \
      case -1:                                                                 \
         SIGNALSLOG_WITH_ERRNO(_value, expstr, err);                           \
         break;                                                                \
                                                                               \
      case 0:                                                                  \
         newprocinit();                                                        \
         SIGNALSLOG_WITH_ERRNO_FAD(_value, expstr, err);                       \
         HANDLE_RINGBUFFER();                                                  \
         abort();                                                              \
         break; /* NOTREACHED */                                               \
                                                                               \
      default:                                                                 \
         SIGNALSLOG_PARENT_CONTINUING(forked);                                 \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#define SWARNX_BODY(_value, expstr)                                            \
do {                                                                           \
   pid_t forked;                                                               \
                                                                               \
   switch ((forked = fork())) {                                                \
      case -1:                                                                 \
         SIGNALSLOG_WITHOUT_ERRNO(_value, expstr);                             \
         break;                                                                \
                                                                               \
      case 0:                                                                  \
         newprocinit();                                                        \
         SIGNALSLOG_WITHOUT_ERRNO_FAD(_value, expstr);                         \
         HANDLE_RINGBUFFER();                                                  \
         abort();                                                              \
         break; /* NOTREACHED */                                               \
                                                                               \
      default:                                                                 \
         SIGNALSLOG_PARENT_CONTINUING(forked);                                 \
   }                                                                           \
} while (/* CONSTCOND */ 0)

#define SIGNALSLOG_PARENT_CONTINUING(childpid)                                 \
do {                                                                           \
   char _b[10][32];                                                            \
   const char *_msgv[]                                                         \
   = { "continuing after internal error.  Unless disabled on system we "       \
       "should have a coredump from pid ",                                     \
       ltoa(getpid(), _b[0], sizeof(_b[0])),                                   \
       " by way of pid ",                                                      \
       ltoa((childpid), _b[1], sizeof(_b[1])),                                 \
       " now",                                                                 \
       NULL                                                                    \
     };                                                                        \
                                                                               \
   signalslog(LOG_WARNING, _msgv);                                             \
} while (/* CONSTCOND */ 0)


#define SWARN(expression)                                                      \
do {                                                                           \
    const long _value = (const long)(expression);                              \
    const char *expstr = #expression;                                          \
                                                                               \
    SWARN_BODY(_value, expstr, errno);                                         \
} while (/* CONSTCOND */ 0)

#define SWARNX(expression)                                                     \
do {                                                                           \
    const long _value = (const long)(expression);                              \
    const char *expstr = #expression;                                          \
                                                                               \
    SWARNX_BODY(_value, expstr);                                               \
} while (/* CONSTCOND */ 0)

#define SERR(expression)                                                       \
do {                                                                           \
    const long _value = (const long)(expression);                              \
    const char *expstr = #expression;                                          \
                                                                               \
    SERR_BODY(_value, expstr, errno);                                          \
} while (/* CONSTCOND */ 0)

#define SERRX(expression)                                                      \
do {                                                                           \
    const long _value = (const long)(expression);                              \
    const char *expstr = #expression;                                          \
                                                                               \
    SERRX_BODY(_value, expstr);                                                \
} while (/* CONSTCOND */ 0)

#define SASSERT(expression)                                                    \
do {                                                                           \
   const long _value  = (const long)(expression);                              \
   const char *expstr = #expression;                                           \
                                                                               \
   if (_value == 0)                                                            \
      SERR_BODY(_value, expstr, errno);                                        \
} while (/* CONSTCOND */ 0)

#define SASSERTX(expression)                                                   \
do {                                                                           \
   const long _value  = (const long)(expression);                              \
   const char *expstr = #expression;                                           \
                                                                               \
   if (_value == 0)                                                            \
      SERRX_BODY(_value, expstr);                                              \
} while (/* CONSTCOND */ 0)

#if 0
/* so we can attach to the process while it's alive ... */
#define abort() do { sleep(60); } while (1)
#endif


/*
 * Make sure length of "_src" is not larger than the size of "_dst".
 */
#define STRCPY_ASSERTLEN(__dst, __src)                                         \
do {                                                                           \
   const void *_src   = (__src);                                               \
   const size_t _len = strlen((const char *)_src);                             \
   void *_dst        = (__dst);                                                \
                                                                               \
   SASSERTX(_len <= (sizeof((__dst)) - 1));                                    \
   memcpy((_dst), (_src), _len + 1);                                           \
} while (/* CONSTCOND */ 0)


/*
 * Round up or down a struct timeval to whole seconds.
 */
#define timeval2seconds(tval)                                                  \
   ((tval)->tv_sec + ((tval)->tv_usec >= 500000 ? 1 : 0))


/*
 * Make sure length of "_src" is not lager than "maxlen", and copy it to _dst.
 */
#define STRCPY_CHECKLEN(__dst, __src, _maxlen, _function)                      \
do {                                                                           \
   const void *_src  = (__src);                                                \
   const size_t _len = strlen((const char *)_src);                             \
   void *_dst        = (__dst);                                                \
                                                                               \
   if (_len >= (_maxlen) - 1) {                                                \
      _function("the value given is %lu bytes long, but the maximum length, "  \
                "set at compiletime, is %lu",                                  \
                (unsigned long)(_len),                                         \
                (unsigned long)((_maxlen)));                                   \
       break;                                                                  \
   }                                                                           \
                                                                               \
   memcpy((_dst), (_src), _len + 1);                                           \
} while (/* CONSTCOND */ 0)


#define STRCPY_ASSERTSIZE(__dst, __src)                                        \
do {                                                                           \
   const void *_src  = (__src);                                                \
   const size_t _len = strlen((const char *)_src);                             \
   void *_dst        = (__dst);                                                \
                                                                               \
   CTASSERT(sizeof((__dst)) >= sizeof((__src)));                               \
   SASSERTX(_len + 1 <= sizeof((__dst)));                                      \
   SASSERTX(_len + 1 <= sizeof((__src)));                                      \
                                                                               \
   memcpy((_dst), (_src), _len + 1);                                           \
} while (0 /* CONSTCOND */)


#define STRCPY_CHECKUTFLEN(__dst, __src, _maxlen, _function)                   \
do {                                                                           \
   const void *_src  = (__src);                                                \
   const size_t _len = strlen((const char *)__src);                            \
   void *_dst        = (__dst);                                                \
   char *utfsrc;                                                               \
                                                                               \
   if (_len / 2 >= ((_maxlen) - 1)) {                                          \
      _function("the value given is %lu bytes long, but the maximum length, "  \
               "set at compiletime, is %lu",                                   \
               (unsigned long)(_len / 2),                                      \
               (unsigned long)((_maxlen) - 1));                                \
       break;                                                                  \
   }                                                                           \
                                                                               \
   if ((utfsrc = hextoutf8((const char *)_src, 2)) == NULL)                    \
      _function("failed to convert string \"%s\" to UTF-8",                    \
                (const char *)_src);                                           \
                                                                               \
   strncpy((char *)_dst, utfsrc, (_maxlen) - 1);                               \
   ((char *)(_dst))[(_maxlen) - 1] = NUL;                                      \
} while (/* CONSTCOND */ 0)


#define TVMIN(a, b) \
   (timercmp((a), (b), <) ? (a) : (b))

#define TVMAX(a, b) \
   (timercmp((a), (b), >) ? (a) : (b))


/*
 * Can not call these function directly since we need to make sure unused
 * bytes in the destination are zero (requirement of the socket API),
 * but the size of the destination can vary. :-/
 */

#define sockshost2sockaddr2(host, addr, gaierr, emsg, emsglen)                 \
   int_sockshost2sockaddr2((host),                                             \
                           (addr),                                             \
                           sizeof((*addr)),                                    \
                           (gaierr),                                           \
                           (emsg),                                             \
                           (emsglen))                                          \

#define sockshost2sockaddr(host, addr)                                         \
   int_sockshost2sockaddr((host), addr, sizeof((*addr)))

#define fakesockshost2sockaddr(host, addr)                                     \
   int_fakesockshost2sockaddr((host), (addr), sizeof((*addr)))

#define urlstring2sockaddr(string, addr, gaierr, emsg, elen)                   \
   int_urlstring2sockaddr((string),                                            \
                          (addr),                                              \
                          sizeof((*addr)),                                     \
                          (gaierr),                                            \
                          (emsg),                                              \
                          (elen))

#define ruleaddr2sockaddr(ruleaddr, addr, protocol)                            \
   int_ruleaddr2sockaddr((ruleaddr), (addr), sizeof((*addr)), (protocol))

#define ruleaddr2sockaddr2(ruleaddr, addr, protocol, gaierr, emsg, emsglen)    \
   int_ruleaddr2sockaddr2((ruleaddr),                                          \
                          (addr),                                              \
                          (sizeof((*addr))),                                   \
                          (protocol),                                          \
                          (gaierr),                                            \
                          (emsg),                                              \
                          (emsglen))

#define hostname2sockaddr(name, index, addr)                                   \
   int_hostname2sockaddr((name), (index), addr, sizeof((*addr)))

#define hostname2sockaddr2(name, index, addr, gaierr, emsg, emsglen)           \
   int_hostname2sockaddr2(name,                                                \
                          index,                                               \
                          addr,                                                \
                          sizeof((*addr)),                                     \
                          gaierr,                                              \
                          emsg,                                                \
                          emsglen)

#define ifname2sockaddr(ifname, index, addr, mask)                             \
   int_ifname2sockaddr((ifname),                                               \
                       (index),                                                \
                       (addr),                                                 \
                       sizeof((*addr)),                                        \
                       (mask),                                                 \
                       sizeof((*mask)))


#if HAVE_GSSAPI
#define GSSAPI_OVERHEAD(gssapistate) \
   ((MAXGSSAPITOKENLEN - GSSAPI_HLEN) - (gssapistate)->maxgssdata)

#define GSSERR_IS_OK(e)                                                        \
   (  (e) == GSS_S_CONTEXT_EXPIRED                                             \
   || (e) == GSS_S_CREDENTIALS_EXPIRED)
#endif /* HAVE_GSSAPI */

/*
 * Matched against sockscf.option.debug.  If the value there is
 * >= to DEBUG_NORMAL, do normal debug logging.  If >= DEBUG_VERBOSE,
 * do verbose, possibly expensive, debug logging also.
 */
#define DEBUG_NORMAL    (1)
#define DEBUG_VERBOSE   (2)
#define DEBUG_DEBUG     (9)   /* only for debuging problems. */

/*
 * If client, it might need to call malloc(3) to expand socksfdv
 * from the signal handler upon SIGIO, but if we are in a gssapi-function
 * that also is calling malloc(3) ... Still not safe of course, as we
 * have no idea if client is in a function that has called malloc(3).
 */
#if SOCKS_CLIENT

#define SOCKS_SIGBLOCK_IF_CLIENT(sig, oldset) \
do { socks_sigblock(sig, oldset); } while (/* CONSTCOND */ 0)

#define SOCKS_SIGUNBLOCK_IF_CLIENT(oldset) \
do { socks_sigunblock(oldset); } while (/* CONSTCOND */ 0)

#define SIGSET_ALLOCATE(name) sigset_t name

#else /* !SOCKS_CLIENT */
#define SIGSET_ALLOCATE(name)
#define SOCKS_SIGBLOCK_IF_CLIENT(sig, oldset)
#define SOCKS_SIGUNBLOCK_IF_CLIENT(oldset)
#endif /* !SOCKS_CLIENT */

/* due to external libraries/software trying to log to stdout/stderr. :-( */
#define FD_IS_RESERVED_EXTERNAL(fd)    \
   ((fd) == STDOUT_FILENO || (fd) == STDERR_FILENO)


/*
 * when using very large numbers (e.g., 9223372036854775807 on a 64 bit cpu),
 * difftime() returns strange results, even when the second arg is 0.
 * Don't know why, but converting the time_t value to double and
 * then back to time_t changes 9223372036854775807 to -9223372036854775808,
 * which seems to be what happens when we do the equivalent of
 * difftime(9223372036854775807, 0)
 *
 * The below seems to work better, so use it until we encounter a platform
 * where it does not work better.
 */
#define socks_difftime(t1, t2)  ((t1) - (t2))

/*
 * Wrappers for our own functions that modify things a little in a way
 * that should not have any negative effects.
 */
#define close(n)              closen((n))
#define socket(d, t, p)       socks_socket((d), (t), (p))
#define strerror(e)           socks_strerror((e))
#define getifaddrs(ifap)      socks_getifaddrs((ifap))
#define gai_strerror(errcode) socks_gai_strerror((errcode))

#undef snprintf
#define snprintf   snprintfn

/*
 * If "str", of size "strused", contains characters present in
 * "strip", strips them off from "str".
 */
#define STRIPTRAILING(str, strused, strip)         \
do {                                               \
   ssize_t _i;                                     \
                                                   \
   for (_i = (ssize_t)(strused) - 1; _i > 0; --_i) \
      if (strchr((strip), str[_i]) != NULL)        \
         (str)[_i] = NUL;                          \
      else                                         \
         break;                                    \
} while (/* CONSTCOND */ 0)

#define SKIPLEADING(str, strip)                 \
do {                                            \
   while (*(str) != NUL)                        \
      if (strchr((strip), *(str)))              \
         ++(str);                               \
      else                                      \
         break;                                 \
} while (/* CONSTCOND */ 0)


/*
 * for dynamically-sized fd_sets.  Note that this means all our fd_set's
 * must be maxsize, or the macros we define will write over memory not
 * belonging to them.
 */

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif /* !howmany */

#define SOCKD_FD_SIZE() \
((size_t)(howmany((sockscf.state.maxopenfiles + 1), NFDBITS) * sizeof(fd_mask)))

#ifdef FD_ZERO
#undef FD_ZERO
#endif /* FD_ZERO */

#define FD_ZERO(p)                      \
do {                                    \
   memset((p), 0, SOCKD_FD_SIZE());     \
} while (/* CONSTCOND */ 0)

#ifdef FD_CMP
#undef FD_CMP
#endif /* FD_CMP */

#define FD_CMP(a, b) (memcmp((a), (b), SOCKD_FD_SIZE()))

#ifdef FD_COPY
#undef FD_COPY
#endif /* FD_COPY */

#define FD_COPY(dst, src)                 \
do {                                      \
   memcpy((dst), (src), SOCKD_FD_SIZE()); \
} while (/* CONSTCOND */ 0)



#define ERRNOISNOFILE(errno) \
   ((errno) == EMFILE || (errno) == ENFILE)

#define ERRNOISRST(errno) \
   ((errno) == ECONNREFUSED || (errno) == ECONNRESET)

#define ERRNOISPREVIOUSPACKET(errno)                                           \
(     ERRNOISRST(errno)                                                        \
   || ERRNOISNOROUTE(errno)   /* Linux ... */                                  \
   || ERRNOISACCES(errno)     /* Linux ... */                                  \
   || (errno) == EMSGSIZE     /* Linux ... */                                  \
   || (errno) == ETIMEDOUT                                                     \
)

#define ERRNOISTMP(errno)      \
   (  (errno) == EAGAIN        \
   || (errno) == EWOULDBLOCK   \
   || (errno) == EINTR         \
   || (errno) == ENOBUFS       \
   || (errno) == ENOMEM        \
   || (errno) == ENOMSG)

#define ERRNOISACCES(errno) ((errno) == EPERM || (errno) == EACCES)

#define ERRNOISNOROUTE(errno) \
   ((errno) == ENETUNREACH || (errno) == EHOSTUNREACH || (errno) == ENETDOWN)

#define ERRNOISNETWORK(errno) (\
   ERRNOISNOROUTE(errno)       \
|| ERRNOISRST(errno)           \
)

#define PORTISRESERVED(port)   \
   (ntohs((port)) != 0 && ntohs((port)) < IPPORT_RESERVED)

#define IPADDRISBOUND(addr) \
(TOSA((addr))->sa_family == AF_UNSPEC ?                                        \
   0 : (TOSA((addr))->sa_family == AF_INET ?                                   \
         (TOIN((addr))->sin_addr.s_addr != htonl(INADDR_ANY))                  \
      :  (memcmp(TOIN6((addr))->sin6_addr.s6_addr,                             \
                 &in6addr_any,                                                 \
                 sizeof(in6addr_any)) != 0))                                   \
)

#define PORTISBOUND(addr) \
(TOSA((addr))->sa_family == AF_UNSPEC ?                                        \
   0 : (TOSA((addr))->sa_family == AF_INET ?                                   \
        (ntohs(TOIN((addr))->sin_port) != 0)                                   \
      : (ntohs(TOIN6((addr))->sin6_port)) != 0)                                \
)

#define ADDRISBOUND(addr) \
   (IPADDRISBOUND((addr)) && PORTISBOUND((addr)))

#define SOCKSHOSTISNOTBOUND(host)                                              \
  ((host)->port == htons(0) || !SOCKSHOST_ADDRISBOUND(host))

#define SOCKSHOSTISBOUND(host)  (!(SOCKSHOSTISNOTBOUND((host))))

#define SOCKSHOST_ADDRISBOUND(host)                                            \
(  ((host)->atype == SOCKS_ADDR_DOMAIN && *host->addr.domain != NUL)           \
|| ((host)->atype == SOCKS_ADDR_IPV4                                           \
   && (host)->addr.ipv4.s_addr != htonl(INADDR_ANY))                           \
|| ((host)->atype == SOCKS_ADDR_IPV6                                           \
   && memcmp(&(host)->addr.ipv6, &in6addr_any, sizeof(in6addr_any))) != 0)

#define RULEPORT_START(addr, protocol) (                                       \
   ((protocol) == SOCKS_TCP ? (addr)->port.tcp : (addr)->port.udp))

#if HAVE_SOCKADDR_STORAGE_SS_LEN

#define SET_SOCKADDRLEN(ss, len)       \
do {                                   \
   ((ss)->ss_len = (len));             \
} while (/* CONSTCOND */ 0)

#else /* !HAVE_SOCKADDR_STORAGE_SS_LEN */

#define SET_SOCKADDRLEN(ss, len)

#endif /* !HAVE_SOCKADDR_STORAGE_SS_LEN */

#define SET_SOCKADDR(sa, family)                                               \
do {                                                                           \
   ((sa)->ss_family = (family));                                               \
   SET_SOCKADDRLEN((sa), salen((family)));                                     \
} while (/* CONSTCOND */ 0)

#define SET_SOCKADDRPORT(sa, port)                                             \
do {                                                                           \
   switch ((sa)->ss_family) {                                                  \
      case AF_INET:                                                            \
         (TOIN(sa))->sin_port   = (port);                                      \
         break;                                                                \
                                                                               \
      case AF_INET6:                                                           \
         (TOIN6(sa))->sin6_port = (port);                                      \
         break;                                                                \
                                                                               \
      default:                                                                 \
         SERRX((sa)->ss_family);                                               \
   }                                                                           \
} while (/* CONSTCOND */ 0)


#define GET_SOCKADDRPORT(sa)                                                   \
   (((sa)->ss_family) == AF_INET ?                                             \
         TOCIN(sa)->sin_port : TOCIN6(sa)->sin6_port)                          \

#define SET_SOCKADDRADDR(sa, addr)                                             \
do {                                                                           \
   switch ((sa)->ss_family) {                                                  \
      case AF_INET:                                                            \
         memcpy(&(TOIN(sa))->sin_addr, addr, sizeof(TOIN(sa))->sin_addr);      \
         break;                                                                \
                                                                               \
      case AF_INET6:                                                           \
         memcpy(&(TOIN6(sa))->sin6_addr, addr, sizeof(TOIN6(sa))->sin6_addr);  \
         break;                                                                \
                                                                               \
      default:                                                                 \
         SERRX((sa)->ss_family);                                               \
   }                                                                           \
} while (/* CONSTCOND */ 0)


#define GET_SOCKADDRADDR(sa)                                                   \
   (((sa)->ss_family) == AF_INET ?                                             \
         ((const void *)&(TOCIN(sa)->sin_addr))                                \
      :  ((const void *)&(TOCIN6(sa)->sin6_addr)))                             \



#define ELEMENTS(array) (sizeof(array) / sizeof(array[0]))

#define OCTETIFY(a) ((a) &= 0xff)
/*
 * Note that the argument will be truncated, not just the return value.
 */


/*
 * Stuff for messages between our processes.
 */

/* padding for each message between mother/child, including separation. */
#define SENDMSG_PADBYTES   (sizeof(long) * 64) /* just a guess. */


/*
 * macros to manipulate ancillary data depending on if we're on sysv or BSD.
 */

/*
 * Modern CMSG alignment macros. Use them if the platform has them,
 * if not we get the default behavior.
 */

#if HAVE_CMSGHDR

#if !HAVE_CMSG_LEN
#define CMSG_LEN(len) (sizeof(struct cmsghdr) + (len))
#endif /* !HAVE_CMSG_LEN */

#if !HAVE_CMSG_SPACE
#define CMSG_SPACE(len) (sizeof(struct cmsghdr) + (len))
#endif /* !HAVE_CMSG_SPACE */

#else /* HAVE_CMSGHDR */

#if !HAVE_CMSG_LEN
#define CMSG_LEN(len) (len)
#endif /* !HAVE_CMSG_LEN */

#if !HAVE_CMSG_SPACE
#define CMSG_SPACE(len) (len)
#endif /* !HAVE_CMSG_SPACE */

#endif /* HAVE_CMSGHDR */

/*
 * allocate memory for a control message of size "size".  "name" is the
 * name of the allocated memory.
 */
#if HAVE_CMSGHDR

#define CMSG_AALLOC(name, size)           \
   union {                                \
      char   cmsgmem[CMSG_SPACE(size)];   \
      struct cmsghdr align;               \
   } __CONCAT3(_, name, mem) = { { 0 } };    /* cleared to appease valgrind */ \
   struct cmsghdr *name = (struct cmsghdr *)__CONCAT3(_, name, mem).cmsgmem;

#else /* !HAVE_CMSGHDR */

#define CMSG_AALLOC(name, size)           \
   char name[(size)] = NUL;

#endif /* !HAVE_CMSGHDR */


/*
 * Returns the size of the previously allocated control message named
 * "name"
 */
#if HAVE_CMSGHDR
#define CMSG_MEMSIZE(name) (sizeof(__CONCAT3(_, name, mem)))
#else /* !HAVE_CMSGHDR */
#define CMSG_MEMSIZE(name) (sizeof((name)))
#endif /* HAVE_CMSGHDR */

/*
 * Verifies length of received control message.
 *
 * Final padding might not be present in received message,
 * expected length can be either value of CMSG_SPACE() or CMSG_LEN().
 */
#define CMSG_RCPTLEN_ISOK(msg, datalen)                                        \
    ((datalen) == 0 ? ((size_t)(CMSG_TOTLEN(msg) == 0))                        \
            :   ((size_t)CMSG_TOTLEN((msg)) == (size_t)(CMSG_SPACE((datalen))) \
              || (size_t)CMSG_TOTLEN((msg)) == (size_t)(CMSG_LEN((datalen)))))

/*
 * Returns the control data member of "msg".
 */
#if HAVE_CMSGHDR
/*
 * cast is necessary on AIX, due to buggy headers there?
 * needs additional testing on AIX, disable for now.
 */
#define CMSG_CONTROLDATA(msg)   ((msg).msg_control)
#else /* !HAVE_CMSGHDR */
#define CMSG_CONTROLDATA(msg)   ((msg).msg_accrights)
#endif /* HAVE_CMSGHDR */

/*
 * add "object" to "data".  "object" is the object to add to "data" at
 * offset "offset".
 */
#if HAVE_CMSGHDR
#define CMSG_ADDOBJECT(object, data, offset)                         \
   do                                                                \
      memcpy(CMSG_DATA(data) + (offset), &(object), sizeof(object)); \
   while (/* CONSTCOND */ 0)
#else /* !HAVE_CMSGHDR */
#define CMSG_ADDOBJECT(object, data, offset)                         \
   do                                                                \
      memcpy(data + (offset), &(object), sizeof((object)));          \
   while (/* CONSTCOND */ 0)
#endif /* !HAVE_CMSGHDR */

/*
 * get a object from control data "data".
 * "object" is the object to fill with data gotten from "data" at offset
 * "offset".
 */
#if HAVE_CMSGHDR
#define CMSG_GETOBJECT(object, data, offset)                               \
   do                                                                      \
      memcpy(&(object), CMSG_DATA((data)) + (offset), sizeof((object)));   \
   while (/* CONSTCOND */ 0)
#else /* !HAVE_CMSGHDR */
#define CMSG_GETOBJECT(object, data, offset)                               \
   do                                                                      \
      memcpy(&(object), ((data) + (offset)), sizeof((object)));            \
   while (/* CONSTCOND */ 0)
#endif /* !HAVE_CMSGHDR */

/*
 * Sets up "object" for sending a control message of size "size".
 * "controlmem" is the memory the control message is stored in.
 *
 * CMSG_SPACE() rather than CMSG_LEN() apparently correct value
 * for msg_controllen.
 */
#if HAVE_CMSGHDR
#define CMSG_SETHDR_SEND(object, controlmem, size)                             \
do {                                                                           \
   if (size == 0) {                                                            \
      object.msg_control      = NULL;                                          \
      object.msg_controllen   = 0;                                             \
   }                                                                           \
   else {                                                                      \
      bzero(controlmem, sizeof(*controlmem));                                  \
                                                                               \
      controlmem->cmsg_level  = SOL_SOCKET;                                    \
      controlmem->cmsg_type   = SCM_RIGHTS;                                    \
      controlmem->cmsg_len    = CMSG_LEN(size);                                \
                                                                               \
      object.msg_control      = (caddr_t)controlmem;                           \
      object.msg_controllen   = (size) == 0 ? 0 : CMSG_SPACE((size));          \
  }                                                                            \
} while (/* CONSTCOND */ 0)
#else /* !HAVE_CMSGHDR */
#define CMSG_SETHDR_SEND(object, controlmem, size)                             \
do {                                                                           \
  object.msg_accrights      = (caddr_t)controlmem;                             \
  object.msg_accrightslen   = (size);                                          \
} while (/* CONSTCOND */ 0)
#endif /* !HAVE_CMSGHDR */

/*
 * Sets up "object" for receiving a control message of size "size".
 * "controlmem" is the memory set aside for the control message.
 */
#if HAVE_CMSGHDR
#define CMSG_SETHDR_RECV(object, controlmem, size)             \
   do {                                                        \
      object.msg_control      = (caddr_t)controlmem;           \
      object.msg_controllen   = (size);                        \
   } while (/* CONSTCOND */ 0)
#else /* !HAVE_CMSGHDR */
#define CMSG_SETHDR_RECV(object, controlmem, size)             \
   do {                                                        \
      object.msg_accrights      = (caddr_t)controlmem;         \
      object.msg_accrightslen   = (size);                      \
   } while (/* CONSTCOND */ 0)
#endif /* !HAVE_CMSGHDR */

/* returns length of control data actually sent. */
#if HAVE_CMSGHDR
#define CMSG_GETLEN(msg)   ((msg).msg_controllen - CMSG_LEN(0))
#else
#define CMSG_GETLEN(msg)   ((msg).msg_accrightslen)
#endif /* HAVE_CMSGHDR */

#if HAVE_CMSGHDR
#define CMSG_TOTLEN(msg)   ((msg).msg_controllen)
#else
#define CMSG_TOTLEN(msg)   ((msg).msg_accrightslen)
#endif /* HAVE_CMSGHDR */


/* the size of a UDP header "packet" (no padding) */
#define HEADERSIZE_UDP(packet) (                                               \
   sizeof((packet)->flag) + sizeof((packet)->frag)                             \
   + sizeof((packet)->host.atype) + sizeof((packet)->host.port)                \
   + (ADDRESSIZE_V5(packet)))


/*
 * returns the length of the current address field in socks packet "packet".
 * "packet" can be one of pointer to response_t, request_t or udpheader_t.
 */
#define ADDRESSIZE(packet) (                                                   \
     ((packet)->version == SOCKS_V4 ?                                          \
     (ADDRESSIZE_V4(packet)) : (ADDRESSIZE_V5(packet))))

/*
 *   version specifics
 */
#define ADDRESSIZE_V5(packet) (                                                \
  (packet)->host.atype == SOCKS_ADDR_IPV4 ?                                    \
        sizeof((packet)->host.addr.ipv4)                                       \
      : (packet)->host.atype  == (unsigned char)SOCKS_ADDR_IPV6 ?              \
            sizeof((packet)->host.addr.ipv6.ip)                                \
          : (strlen((packet)->host.addr.domain) + 1))

#define ADDRESSIZE_V4(packet) (                                                \
   (packet)->atype == SOCKS_ADDR_IPV4 ?                                        \
   sizeof((packet)->addr.ipv4) : (strlen((packet)->addr.host) + 1))


/*
 * This is for Rgethostbyname() support for clients without access to
 * DNS.
 * FAKEIP_START is the first address in the range of "fake" IP addresses,
 * FAKEIP_END is the last.
 * There can thus be FAKEIP_END - FAKEIP_START number of fake IP addresses
 * supported per program.
 *
 * INADDR_NONE and INADDR_ANY may not be part of the range.
 */
#define FAKEIP_START 0x00000001
#define FAKEIP_END   0x000000ff

#define PROXY_UPNP                  3
#define PROXY_UPNPs                 "UPNP"
#define PROXY_BROADCASTs            "broadcast"    /* subset of upnp. */
#define PROXY_SOCKS_V4               4
#define PROXY_SOCKS_V4s              "socks_v4"
#define PROXY_SOCKS_V4REPLY_VERSION  0
#define PROXY_SOCKS_V5               5
#define PROXY_SOCKS_V5s              "socks_v5"
#define PROXY_DIRECT                 6
#define PROXY_DIRECTs               "direct"
#define PROXY_HTTP_10               7
#define PROXY_HTTP_10s              "HTTP/1.0"
#define PROXY_HTTP_11               8
#define PROXY_HTTP_11s              "HTTP/1.1"

/* sub negotiation. */
#define SOCKS_UNAMEVERSION              1
#define SOCKS_GSSAPI_VERSION            1
#define SOCKS_GSSAPI_AUTHENTICATION     1
#define SOCKS_GSSAPI_ENCRYPTION         2
#define SOCKS_GSSAPI_PACKET             3
#define SOCKS_GSSAPI_CLEAR              0
#define SOCKS_GSSAPI_INTEGRITY          1
#define SOCKS_GSSAPI_CONFIDENTIALITY    2
#define SOCKS_GSSAPI_PERMESSAGE         3

/* authentication METHOD values. */
#define AUTHMETHOD_NOTSET      -1
#define AUTHMETHOD_NOTSETs      "notset"
#define AUTHMETHOD_NONE         0
#define AUTHMETHOD_NONEs        "none"
#define AUTHMETHOD_GSSAPI       1
#define AUTHMETHOD_GSSAPIs      "gssapi"
#define AUTHMETHOD_UNAME        2
#define AUTHMETHOD_UNAMEs      "username"

/* X'03' to X'7F' IANA ASSIGNED                  */

/* X'80' to X'FE' RESERVED FOR PRIVATE METHODS   */

#define AUTHMETHOD_NOACCEPT   255
#define AUTHMETHOD_NOACCEPTs   "<no acceptable method>"

/* non-standard methods.  Must be > AUTHMETHOD_NOACCEPT. */
#define AUTHMETHOD_RFC931          (AUTHMETHOD_NOACCEPT + 1)
#define AUTHMETHOD_RFC931s         "rfc931"

#define AUTHMETHOD_PAM_ANY         (AUTHMETHOD_RFC931 + 1)
#define AUTHMETHOD_PAM_ANYs        "pam.any"

#define AUTHMETHOD_PAM_ADDRESS     (AUTHMETHOD_PAM_ANY + 1)
#define AUTHMETHOD_PAM_ADDRESSs    "pam.address"

#define AUTHMETHOD_PAM_USERNAME    (AUTHMETHOD_PAM_ADDRESS + 1)
#define AUTHMETHOD_PAM_USERNAMEs   "pam.username"

#define AUTHMETHOD_BSDAUTH     (AUTHMETHOD_PAM_USERNAME + 1)
#define AUTHMETHOD_BSDAUTHs    "bsdauth"

#define AUTHMETHOD_LDAPAUTH         (AUTHMETHOD_BSDAUTH + 1)
#define AUTHMETHOD_LDAPAUTHs        "ldapauth"

#define AUTHMETHOD_MAX         (AUTHMETHOD_LDAPAUTH)

#define MAXMETHODSTRING      (MAX(sizeof(AUTHMETHOD_NONEs),             \
                              MAX(sizeof(AUTHMETHOD_GSSAPIs),           \
                              MAX(sizeof(AUTHMETHOD_UNAMEs),            \
                              MAX(sizeof(AUTHMETHOD_RFC931s),           \
                              MAX(sizeof(AUTHMETHOD_PAM_ANYs),          \
                              MAX(sizeof(AUTHMETHOD_PAM_ADDRESSs),      \
                              MAX(sizeof(AUTHMETHOD_PAM_USERNAMEs),     \
                              MAX(sizeof(AUTHMETHOD_BSDAUTHs),          \
                              sizeof(AUTHMETHOD_LDAPAUTHs))))))))))

/* number of supported methods. */
#define METHODS_KNOWN  (  1  /* NONE      */   \
                        + 1  /* GSSAPI    */   \
                        + 1  /* UNAME     */   \
                        + 1  /* RFC931    */   \
                        + 1  /* PAM       */   \
                        + 1  /* BSDAUTH   */   \
                        + 1) /* LDAPAUTH  */

#define MAXMETHODS     (255)  /*
                               * max number of methods we can be offered, and
                               * potentially support.
                               */

/*
 *  Response commands/codes
 */
#define SOCKS_CONNECT            1
#define SOCKS_CONNECTs           "connect"
#define SOCKS_BIND               2
#define SOCKS_BINDs              "bind"
#define SOCKS_UDPASSOCIATE       3
#define SOCKS_UDPASSOCIATEs      "udpassociate"

/* pseudo commands */

#define SOCKS_COMMANDEND         0xff

#define SOCKS_BINDREPLY          (SOCKS_COMMANDEND + 1)
#define SOCKS_BINDREPLYs         "bindreply"

#define SOCKS_UDPREPLY           (SOCKS_BINDREPLY + 1)
#define SOCKS_UDPREPLYs          "udpreply"

/* misc. stuff */
#define SOCKS_ACCEPT             (SOCKS_UDPREPLY + 1)
#define SOCKS_ACCEPTs            "accept"

#define SOCKS_DISCONNECT         (SOCKS_ACCEPT + 1)
#define SOCKS_DISCONNECTs        "disconnect"

#define SOCKS_BOUNCETO            (SOCKS_DISCONNECT + 1)
#define SOCKS_BOUNCETOs           "bounce-to"

#define SOCKS_HOSTID              (SOCKS_BOUNCETO + 1)
#define SOCKS_HOSTIDs             "hostid"

#define SOCKS_UNKNOWN            (SOCKS_HOSTID + 1)
#define SOCKS_UNKNOWNs           "unknown"


/* reply field values */
#define SOCKS_SUCCESS         0x00
#define SOCKS_FAILURE         0x01
#define SOCKS_NOTALLOWED      0x02
#define SOCKS_NETUNREACH      0x03
#define SOCKS_HOSTUNREACH     0x04
#define SOCKS_CONNREFUSED     0x05
#define SOCKS_TTLEXPIRED      0x06
#define SOCKS_CMD_UNSUPP      0x07
#define SOCKS_ADDR_UNSUPP     0x08

/* version 4 codes. */
#define SOCKSV4_SUCCESS        90
#define SOCKSV4_FAIL           91
#define SOCKSV4_NO_IDENTD      92
#define SOCKSV4_BAD_ID         93

/* http stuff. */
#define HTTP_SUCCESS           200
#define HTTP_NOTALLOWED        401
#define HTTP_FORBIDDEN         403
#define HTTP_PROXYAUTHREQUIRED 407
#define HTTP_HOSTUNREACH       504
#define HTTP_FAILURE           501
#define HTTP_UNSUPPORTEDVERSION  505

#define SOCKD_HTTP_PORT        80

/* upnp stuff. */
#define UPNP_DISCOVERYTIME_MS              (1000)
#define UPNP_IP_TTL                        (2)
#define DEFAULT_SSDP_BROADCAST_IPV4ADDR    "239.255.255.250"
#define DEFAULT_SSDP_PORT                  (1900)

/* return codes from UPNP_GetValidIGD(). */
#define UPNP_NO_IGD           (0)
#define UPNP_CONNECTED_IGD    (1)

#if HAVE_LIBMINIUPNP228

#define UPNP_RESERVED_IGD     (2)
#define UPNP_DISCONNECTED_IGD (3)
#define UPNP_UNKNOWN_DEVICE   (4)

#else /* !HAVE_LIBMINIUPNP_228 */

#define UPNP_DISCONNECTED_IGD (2)
#define UPNP_UNKNOWN_DEVICE   (3)

#endif /* !HAVE_LIBMINIUPNP_228 */

#define UPNP_SUCCESS          (1)
#define UPNP_FAILURE          (2)

/* flag _bits_ */
#define SOCKS_INTERFACEREQUEST   0x01
#define SOCKS_USECLIENTPORT      0x04

/* subcommands */
#define SOCKS_INTERFACEDATA      0x01

#define SOCKS_RECV      0
#define SOCKS_SEND      1

/* offsets into authentication packet */
#define AUTH_VERSION         (0)   /* version of method packet.               */

/* request */
#define AUTH_NMETHODS        (1)   /* offset of number of methods offered.    */
#define AUTH_FIRSTMETHOD     (2)   /* offset of first method offered.         */

/* reply */
#define AUTH_SELECTEDMETHOD  (1)  /* offset for selected method in response. */

/* offsets into username/password negotiation packet */
#define UNAME_VERSION      (0)
#define UNAME_STATUS       (1)


/* uname status values. */
#define UNAME_STATUS_ISOK    (0)
#define UNAME_STATUS_ISNOK   (1)

/* offsets into gssapi negotiation packet */
#define GSSAPI_VERSION          0
#define GSSAPI_STATUS           1
#define GSSAPI_TOKEN_LENGTH     2
#define GSSAPI_TOKEN            4


#define GSSAPI_CLEAR            0
#define GSSAPI_INTEGRITY        1
#define GSSAPI_CONFIDENTIALITY  2

#define GSS_REQ_INT             0
#define GSS_REQ_CONF            1

#define SOCKS_IPV6_ALEN          16
#define IPV6_NETMASKBITS         (128)
#define IPV4_NETMASKBITS         (32)
#define IPV4_FULLNETMASK         (0xffffffff)

#define IP6_ELEMENTS(ip6)     \
(ip6)->s6_addr[0],            \
(ip6)->s6_addr[1],            \
(ip6)->s6_addr[2],            \
(ip6)->s6_addr[3],            \
(ip6)->s6_addr[4],            \
(ip6)->s6_addr[5],            \
(ip6)->s6_addr[6],            \
(ip6)->s6_addr[7],            \
(ip6)->s6_addr[8],            \
(ip6)->s6_addr[9],            \
(ip6)->s6_addr[10],           \
(ip6)->s6_addr[11],           \
(ip6)->s6_addr[12],           \
(ip6)->s6_addr[13],           \
(ip6)->s6_addr[14],           \
(ip6)->s6_addr[15]

#define IP6_FMTSTR "%02x%02x:%02x%02x:%02x%02x:%02x%02x"\
                   "%02x%02x:%02x%02x:%02x%02x:%02x%02x"

#define BINDEXTENSION_IPADDR     (0xffffffff)



/*
 * hostid related defines
 */

/* socket option hostid types */
#define SOCKS_HOSTID_TYPE_NONE 0
#define SOCKS_HOSTID_TYPE_TCP_IPA 1
#define SOCKS_HOSTID_TYPE_TCP_EXP1 2

#if (SOCKS_HOSTID_TYPE) == SOCKS_HOSTID_TYPE_NONE
#define HAVE_SOCKS_HOSTID (0)
#else
#define HAVE_SOCKS_HOSTID (1)
#endif

/* supported commands/command strings for parsing */
#define SOCKS_HOSTID_NONE                          (0)
#define SOCKS_HOSTID_NONE_SYMNAME                  "none"
#define SOCKS_HOSTID_PASS                          (1)
#define SOCKS_HOSTID_PASS_SYMNAME                  "pass"
#define SOCKS_HOSTID_ADDCLIENT                     (2)
#define SOCKS_HOSTID_ADDCLIENT_SYMNAME             "add-client"
#define SOCKS_HOSTID_SETCLIENT                     (3)
#define SOCKS_HOSTID_SETCLIENT_SYMNAME             "set-client"
#define SOCKS_HOSTID_PASS_OR_SETCLIENT              (4)
#define SOCKS_HOSTID_PASS_OR_SETCLIENT_SYMNAME     "pass-or-set-client"

typedef enum { NONESETIF = 0, INTERNALIF, EXTERNALIF } interfaceside_t;

enum operator_t { none = 0, eq, neq, ge, le, gt, lt, range };
typedef enum { dontcare, istrue, isfalse } value_t;
typedef enum { username, udpreplies, tcpreplies, replies } methodinfo_t;
typedef enum { softlimit, hardlimit } limittype_t;
typedef enum { type_global, type_rule, type_route } opttype_t;


#define SOCKS_ADDR_NOTSET   (0)
#define SOCKS_ADDR_IPV4     (1)
#define SOCKS_ADDR_IFNAME   (2)
#define SOCKS_ADDR_DOMAIN   (3)
#define SOCKS_ADDR_IPV6     (4)
#define SOCKS_ADDR_URL      (5)
#define SOCKS_ADDR_IPVANY   (6) /* for 0/0 address matching ipv4 and ipv6. */

typedef enum { object_none = 0,     /* no object set. */
               object_sockaddr,
               object_sockshost,
               object_crule,
               object_hrule,
               object_srule,
               object_route,
               object_monitor } objecttype_t;


#define MAXLOGLEVELLEN           (sizeof("warning"))
typedef struct {
   const char name[MAXLOGLEVELLEN];
   const int  value;
} loglevel_t;


typedef struct {
   /*
    * if we mark a route/proxy server as bad, how many seconds to wait
    * until we expire the badmarking so it will be tried again for new
    * connections.  A value of zero means never.
    */
   time_t badexpire;

   /*
    * how many times a route can fail before being marked as bad.
    * A value of zero means it will never be marked as bad.
    */
   size_t maxfail;
} routeoptions_t;

typedef struct {
   int           type;        /* type of logging (where to).                  */

   char          **fnamev;    /* name of file, if logging to file.            */
   unsigned char *createdv;   /* did we create this logfile ourselves?        */
   int           *filenov;    /* if logging is to file, the file descriptor.  */
   size_t        filenoc;     /* number of files.                             */

   int           facility;    /* if logging to syslog, this is the facility.  */
   char          facilityname[MAXFACILITYNAMELEN]; /* facilityname.           */
} logtype_t;

typedef struct linkedname_t {
   char                  *name;
   struct linkedname_t   *next;   /* next name in list.                       */
} linkedname_t;

#if HAVE_LDAP

/*
 * This struct contains variables used for LDAP authorization.
 */

typedef struct {
       linkedname_t *ldapurl;                      /* name of ldap urls.      */
       linkedname_t *ldapbasedn;                   /* name of ldap basedns.   */
       linkedname_t *ldapserver;                   /* name of predefined ldap servers.  */

       char         attribute[MAXNAMELEN];
       char         attribute_AD[MAXNAMELEN];
       int          auto_off;
       int          certcheck;
       char         certfile[MAXNAMELEN];
       char         certpath[MAXNAMELEN];
       int          debug;
       char         domain[MAXNAMELEN];
       char         filter[MAXNAMELEN];
       char         filter_AD[MAXNAMELEN];
       int          keeprealm;
       char         keytab[MAXNAMELEN];
       int          mdepth;
       int          port;
       int          portssl;
       int          ssl;
} ldapauthorisation_t;

/*
 * This struct contains variables used for LDAP authentication.
 */

typedef struct {
       linkedname_t *ldapurl;                      /* name of ldap urls.      */
       linkedname_t *ldapbasedn;                   /* name of ldap basedns.   */
       linkedname_t *ldapserver;                   /* name of predefined ldap servers.*/

       int          auto_off;
       int          certcheck;
       char         certfile[MAXNAMELEN];
       char         certpath[MAXNAMELEN];
       int          debug;
       char         domain[MAXNAMELEN];
       char         filter[MAXNAMELEN];
       char         filter_AD[MAXNAMELEN];
       char         keytab[MAXNAMELEN];
       int          keeprealm;
       int          port;
       int          portssl;
       int          ssl;
} ldapauthentication_t;

#endif /* HAVE_LDAP */


/* extensions supported by us. */
typedef struct {
   unsigned char bind;      /* use bind extension? */
} extension_t;

typedef enum { TIMEOUT_NOTSET = 0,
               TIMEOUT_NEGOTIATE,
               TIMEOUT_CONNECT,
               TIMEOUT_IO,
               TIMEOUT_TCP_FIN_WAIT,
               TIMEOUT_NETWORKTEST
} timeouttype_t;

typedef struct {
   /*
    * type should match the struct timeval.tv_sec used by select(2).
    * POSIX says it's time_t.
    */

   time_t connect;   /* how long to wait before giving up connect(2). */
#if !SOCKS_CLIENT
   time_t negotiate; /* how long negotiation can last.                */
   time_t tcpio;     /* how long session can last without i/o.        */
   time_t udpio;     /* how long session can last without i/o.        */

   time_t tcp_fin_wait; /* how long to wait after one end closes.     */
#endif /* !SOCKS_CLIENT */
} timeout_t;

/* method rfc931 */
typedef struct {
   unsigned char   name[MAXNAMELEN];
} authmethod_rfc931_t;

/* method pam. */
typedef struct {
   char            servicename[MAXNAMELEN];   /* servicename to use with pam. */
   unsigned char   name[MAXNAMELEN];
   unsigned char   password[MAXPWLEN];
} authmethod_pam_t;

/* method bsdauth. */
typedef struct {
   char            style[MAXNAMELEN];   /* style to use. */
   unsigned char   name[MAXNAMELEN];
   unsigned char   password[MAXPWLEN];
} authmethod_bsd_t;

#if HAVE_LDAP

/* method ldapauth. */
typedef struct {
   char                    name[MAXNAMELEN];
   char                    password[MAXPWLEN];
   ldapauthentication_t    ldapauthentication;
} authmethod_ldap_t;

#endif /* HAVE_LDAP */

/* method username */
typedef struct {
   unsigned char   version;
   unsigned char   name[MAXNAMELEN];
   unsigned char   password[MAXPWLEN];
} authmethod_uname_t;

#if HAVE_GSSAPI
typedef struct {
       unsigned char nec;
       unsigned char clear;
       unsigned char integrity;
       unsigned char confidentiality;
       unsigned char permessage;
} gssapi_enc_t;

#ifndef BUFSIZ
#define BUFSIZ 1024
#endif /* !BUFSIZ */
typedef struct {
    int            read;
    int            rpos;
    int            wpos;
    int            isbuffered;
    unsigned char  rbuffer[GSSAPI_HLEN + MAXGSSAPITOKENLEN];
    unsigned char  wbuffer[BUFSIZ];
} gssapi_buf_t;

typedef struct {
   int                 wrap;        /* gssapi-wrapped, or clear?              */
   gss_ctx_id_t        id;          /* gssapi context id.                     */
   int                 protection;  /* selected protection mechanism.         */

   OM_uint32           maxgssdata;  /* max length of gss data pre-encoding.   */
   size_t              gssoverhead; /*
                                     * overhead in bytes of gssapi given the
                                     * current mechanism/context/etc.
                                     * Don't know what the practical max
                                     * actually is, so this contains the
                                     * max overhead experienced so far.
                                     */
} gssapi_state_t;

/* method gssapi */
typedef struct {
       char           servicename[MAXNAMELEN];
       char           keytab[MAXNAMELEN];
       unsigned char  name[MAXNAMELEN];
#if HAVE_PAC
       unsigned char  sids[MAXSIDSLEN];
#endif /* HAVE_PAC */
       gssapi_enc_t   encryption;                  /* encryption details      */
       gssapi_state_t state;                       /* gssapi state details    */
} authmethod_gssapi_t;

#endif /* HAVE_GSSAPI */


/* this must be big enough to hold a complete method request. */
typedef struct {
   int                  method;                /* method in use.              */

   /*
    * Methods authenticated at some stage of the process.
    */
   int                  methodv[METHODS_KNOWN];
   size_t               methodc;  /* number of methods authenticated. */

   /*
    * Methods that failed authentication at some stage of the process.
    */
   int                  badmethodv[METHODS_KNOWN];
   size_t               badmethodc;  /* number of methods that failed. */

   union {
      authmethod_uname_t   uname;

#if HAVE_GSSAPI
      authmethod_gssapi_t  gssapi;
#endif /* HAVE_GSSAPI */
#if HAVE_LIBWRAP
      authmethod_rfc931_t  rfc931;
#endif /* HAVE_LIBWRAP */
#if HAVE_PAM
      authmethod_pam_t     pam;
#endif /* HAVE_PAM */
#if HAVE_BSDAUTH
      authmethod_bsd_t     bsd;
#endif /* HAVE_BSDAUTH */

#if HAVE_LDAP

      authmethod_ldap_t    ldap;

#endif /* HAVE_LDAP */

   } mdata;
} authmethod_t;

typedef union {
   int                       int_val;
   struct linger             linger_val;
   struct timeval            timeval_val;
   struct in_addr            in_addr_val;
   unsigned char             uchar_val;
   struct sockaddr_storage   sockaddr_val;
   struct ipoption           ipoption_val;

#if HAVE_TCP_IPA

   struct tcp_ipa_raw        option28_val;

#endif /* HAVE_TCP_IPA */

#if HAVE_TCP_EXP1

   struct tcp_exp1_raw       option253_val;

#endif /* HAVE_TCP_EXP1 */

} socketoptvalue_t;

/*
 * make sure to keep this in sync with the size calculation in
 * setusersockoptions().
 */
typedef enum { int_val = 1, linger_val, timeval_val, in_addr_val, uchar_val,
               sockaddr_val, ipoption_val, option28_val, option253_val 
             } socketoptvalue_type_t;

#define SOCKETOPTVALUETYPE2SIZE_BASE(type)                                     \
   ((type) == int_val      ? sizeof(int)                     :                 \
    (type) == linger_val   ? sizeof(struct linger)           :                 \
    (type) == timeval_val  ? sizeof(struct timeval)          :                 \
    (type) == in_addr_val  ? sizeof(struct in_addr)          :                 \
    (type) == uchar_val    ? sizeof(u_char)                  :                 \
    (type) == sockaddr_val ? sizeof(struct sockaddr_storage) :                 \
    (type) == ipoption_val ? sizeof(struct ipoption)         :                 \
    0)

#if HAVE_SOCKS_HOSTID

#if HAVE_TCP_IPA && HAVE_TCP_EXP1

#define SOCKETOPTVALUETYPE2SIZE(type)                                          \
   (SOCKETOPTVALUETYPE2SIZE_BASE(type) ? SOCKETOPTVALUETYPE2SIZE_BASE(type) :  \
   (type) == option28_val              ? sizeof(struct tcp_ipa_raw)         :  \
   (type) == option253_val             ? sizeof(struct tcp_exp1_raw)        :  \
   0) 

#elif HAVE_TCP_IPA 

#define SOCKETOPTVALUETYPE2SIZE(type)                                          \
   (SOCKETOPTVALUETYPE2SIZE_BASE(type) ? SOCKETOPTVALUETYPE2SIZE_BASE(type) :  \
   (type) == option28_val              ? sizeof(struct tcp_ipa_raw)         :  \
   0)

#elif HAVE_TCP_EXP1 

#define SOCKETOPTVALUETYPE2SIZE(type)                                         
   (SOCKETOPTVALUETYPE2SIZE_BASE(type) ? SOCKETOPTVALUETYPE2SIZE_BASE(type) :  \
   (type) == option253_val             ? sizeof(struct tcp_exp1_raw)        :  \
   0)

#endif /* HAVE_TCP_EXP1 */

#if HAVE_TCP_IPA 

struct tcp_ipa {
   u_int32_t ip; 
};

#endif /* HAVE_TCP_IPA  */

#if HAVE_TCP_EXP1

#define TCP_EXP1_EXID_IP                    (0x348)
#define TCP_EXP1_EXID_INFERNO_FOURBYTE_TEST (0xaabbccdd)

struct tcp_exp1 {
   u_int8_t  len;

   /*
    * For some reason exid can be 16 bits or 32 bits, and the len field seems 
    * to be the determinator of this.  A len of 7 bytes indicates exid is 
    * 16 bits, while a len of 9 bytes indicates exid is 32 bits.  Yeah. :-/
    */
   union {
      u_int16_t exid_16;
      u_int32_t exid_32;
   } exid;

   union {
      u_int32_t ip; /* TCP_EXP1_EXID_IP. */ 
   } data;
};

#endif /* HAVE_TCP_EXP1 */

struct hostid {

   int hostidtype;

   union {

#if HAVE_TCP_IPA 

      struct tcp_ipa tcp_ipa;

#endif /* HAVE_TCP_IPA */

#if HAVE_TCP_EXP1

      struct tcp_exp1 tcp_exp1;

#endif /* HAVE_TCP_EXP1 */

   } addrv[HAVE_MAX_HOSTIDS];

   size_t addrc; /* actually set/used. */
};


#else /* !HAVE_SOCKS_HOSTID */

struct hostid {
   unsigned char notused_just_to_avoid_countless_ifdefs_in_the_code;
};

#define SOCKETOPTVALUETYPE2SIZE(type) SOCKETOPTVALUETYPE2SIZE_BASE(type) 

#endif /* !HAVE_SOCKS_HOSTID */


#define SOCKETOPT_PRE     (0x1)
#define SOCKETOPT_POST    (0x2)
#define SOCKETOPT_ANYTIME (0x4)
#define SOCKETOPT_ALL     (SOCKETOPT_PRE | SOCKETOPT_POST | SOCKETOPT_ANYTIME)

typedef enum { preonly = 1, anytime, postonly, invalid } sockopt_calltype_t;
typedef struct {
   size_t                optid;   /* option identifier                        */
   socketoptvalue_type_t opttype; /* socket option argument type              */
   int                   value;   /* value of SO_foo define                   */
   int                   level;   /* protocol level option applies to         */

   unsigned char         ipv4_on; /* settable for ipv4?                       */
   unsigned char         ipv6_on; /* settable for ipv6?                       */

   /*
    * XXX currently assumed that getsockopt() only called for options
    *     where shift/mask is set
    */
   sockopt_calltype_t calltype;   /* when option can be set.                  */
   unsigned int       shift;      /* number of bits to shift argument value.  */
   unsigned int       mask;       /* if set, mask specifying valid values.    */
   unsigned char      dodup;      /* whether option should be duplicated      */
   unsigned char      needpriv;   /* whether privileges are required          */
   char               name[SOCKOPTNAME_MAXLEN]; /* optionname as string.      */
} sockopt_t;

typedef struct {
   size_t optid;                  /* sockopt_t id symbol is valid for         */
   socketoptvalue_t symval;       /* value of symbolic constant               */
   const char *name;              /* textual representation of constant value */
} sockoptvalsym_t;

typedef struct {
   const sockopt_t       *info;          /* NULL if unknown option.           */
   int                   level;          /*
                                          * socket level to set option at.
                                          * Not necessarily the same as
                                          * info->level as we allow the user
                                          * to specify e.g. "tcp" instead
                                          * of level sol_socket, indicating
                                          * the option should only be set
                                          * for tcp sockets.  The value
                                          * in "info" is the value we need
                                          * use when setting the option.
                                          */

   int                   optname;        /* numeric of option to set.         */
   socketoptvalue_t      optval;         /* value set.                    */
   socketoptvalue_type_t opttype;        /* socket option argument type.      */

   unsigned char         isinternalside; /* option for the internal side?     */
} socketoption_t;


union socksaddr_t {
   char               domain[MAXHOSTNAMELEN];
   char               urlname[MAXURLLEN];
   char               ifname[MAXIFNAMELEN];
   struct in_addr     ipv4;
   struct {
      struct in6_addr  ip;
      uint32_t         scopeid;
   } ipv6;
};

typedef struct sockshost_t {
   unsigned char        atype;
   union socksaddr_t    addr;
   in_port_t            port;
} sockshost_t;

typedef struct {
   unsigned char httpconnect;    /* session is the result of a http connect. */
} requestflags_t;

typedef struct request_t {
   authmethod_t   *auth;   /* pointer to level above. */
   unsigned char  command;
   unsigned char  flag;
   sockshost_t    host;
   int            protocol;
   unsigned char  version;

   requestflags_t flags;
} request_t;


typedef struct {
   unsigned char         version;

   union {
      unsigned char         socks;
      unsigned char         upnp;
      unsigned short        http;
   } reply;

   unsigned char  flag;
   sockshost_t    host;
   authmethod_t   *auth;   /* pointer to level above. */
} response_t;

/* encapsulation for UDP packets. */
typedef struct {
   unsigned char flag[2];
   unsigned char frag;
   sockshost_t   host;
} udpheader_t;

typedef struct {
   unsigned char tcp;
   unsigned char udp;
} protocol_t;


typedef struct {
   unsigned char bind;
   unsigned char connect;
   unsigned char udpassociate;

   /* not real commands as per standard, but they have their use. */
   unsigned char bindreply;      /* reply to bind command.   */
   unsigned char udpreply;       /* reply to UDP packet.     */
} command_t;


typedef struct {
   unsigned char direct;
   unsigned char socks_v4;
   unsigned char socks_v5;
   unsigned char http;
   unsigned char upnp;
} proxyprotocol_t ;

/* values in parentheses designate "don't care" values when searching.  */
typedef struct {
   int                     acceptpending; /* a accept pending?      (-1)      */
   authmethod_t            auth;          /* authentication in use.           */
   int                     command;       /* command (-1)                     */
   int                     err;           /* if request failed, errno.        */
#if HAVE_GSSAPI
   int                     gssimportneeded;
   gss_buffer_desc         gssapistate;   /* if gssimportneeded, data for it. */
   unsigned char           gssapistatemem[MAX_GSS_STATE];
#endif /* HAVE_GSSAPI */
   int                     inprogress;    /* operation in progress? (-1)      */
   unsigned char           issyscall;     /* started out as a real system call*/
   protocol_t              protocol;      /* protocol in use.                 */
   unsigned char           udpconnect;    /* connected UDP socket?            */
   int                     syscalldepth;
   int                     version;       /* version (-1)                     */
} socksstate_t;

typedef struct ruleaddr_t {
   unsigned char         atype;
   union {
      char               domain[MAXHOSTNAMELEN];

      char               ifname[MAXIFNAMELEN];

      struct {
         struct in_addr   ip;
         struct in_addr   mask;
      } ipv4;

      struct {
         struct in6_addr   ip;
         unsigned int      maskbits; /* host order. */
         uint32_t          scopeid;  /* host order. */
      } ipv6;

      struct {
         /*
          * both should always be zero as this is only meaningful for
          * a rule that should match all and any kind of ipaddress.
          */
         struct in_addr   ip;
         struct in_addr   mask;
      } ipvany;

   } addr;

   struct {
      in_port_t         tcp;      /* TCP portstart or field to operator on.   */
      in_port_t         udp;      /* UDP portstart or field to operator on.   */
   } port;
   in_port_t            portend;   /* only used if operator is range.         */
   enum operator_t      operator;  /* operator to compare ports via.          */
} ruleaddr_t;

#ifndef MINIUPNPC_URL_MAXSIZE
#define MINIUPNPC_URL_MAXSIZE (128)
#endif
typedef union {
   struct {
      char    controlurl[MINIUPNPC_URL_MAXSIZE];
      char    servicetype[MINIUPNPC_URL_MAXSIZE];
   } upnp;
} proxystate_t;

typedef struct {
   command_t        command;
   extension_t      extension;
   protocol_t       protocol;

   int              cmethodv[METHODS_KNOWN]; /* clientmethods to offer.       */
   size_t           cmethodc;                /* number of methods to offer.   */
   int              smethodv[METHODS_KNOWN]; /* socksmethods to offer.        */
   size_t           smethodc;                /* number of methods to offer.   */

   proxyprotocol_t  proxyprotocol;

#if HAVE_PAM
   char             pamservicename[MAXNAMELEN];
#endif /* HAVE_PAM */

#if HAVE_BSDAUTH
   char             bsdauthstylename[MAXNAMELEN];
#endif /* HAVE_BSDAUTH */

#if HAVE_GSSAPI
   char             gssapiservicename[MAXNAMELEN];
   char             gssapikeytab[MAXNAMELEN];
   gssapi_enc_t     gssapiencryption;       /* encryption status.      */
#endif /* HAVE_GSSAPI */

#if HAVE_LDAP
   /*
    * new ldap server details.  Used for checking if an already
    * authenticated user is member of the appropriate LDAP group.  I.e.,
    * authorization rather than authentication.
    */
   ldapauthorisation_t     ldapauthorisation;

   /*
    * new ldap auth server details.   Used for performing LDAP-based
    * authentication of a new client (similar to username auth,
    * gssapi auth, etc.).  Is independent of the ldap-object above that
    * is only used for authorization of an already authenticated user.
    */
   ldapauthentication_t   ldapauthentication;
#endif

#if HAVE_LIBMINIUPNP
   proxystate_t     data;
#endif /* HAVE_LIBMINIUPNP */
} serverstate_t;

typedef struct {
   sockshost_t     addr;
   serverstate_t   state;
} gateway_t;


typedef struct {
   unsigned char     version;
                     /*
                      * Negotiated version.  Each request and
                      * response will also contain a version number, that
                      * is the version number given for that particular
                      * packet and should be checked to make sure it is
                      * the same as the negotiated version.
                      */
   request_t         req;
   response_t        res;
   gateway_t         gw;
   socksstate_t      state;
} socks_t;

enum portcmp { e_lt = 1, e_gt, e_eq, e_neq, e_le, e_ge, e_nil };



/*
 * for use in generic functions that take either reply or request
 * packets, include a field indicating what it is.
 */
#define SOCKS_REQUEST   0x1
#define SOCKS_RESPONSE  0x2

/*
 * This object is either used for straightforward buffering, or
 * in the case the data is gssapi-encapsulated, to handle gssapi-data.
 * In the case of simple, non-gssapi, buffering,
 * no further explanation is given; the len field simply holds
 * the number of bytes currently buffered.
 *
 * Next we describe how it is used in the case of gssapi.
 *
 * In the case of gssapi, the buffer is divided into two parts,
 * the first part holding not-encoded data, and the latter part
 * holding encoded data.
 *
 * The operation when reading is as follows:
 *    1) Read into buf as much data as is needed to be able to
 *       decode the data (in the case of gssapi, the whole token).
 *       While doing this, we keep incrementing "enclen", to indicate
 *       how much encoded data has been stored in the buffer.  "len"
 *       is not touch during this.
 *
 *    2) When 1) has completed, we replace the data in buf with
 *       the decoded version of data in "buf", reset "enclen", and
 *       set "len" to indicate how much decoded data is stored in the
 *       buffer.
 *
 *    3) On subsequent read operations on the socket corresponding to
 *       the data (s), return data from buf instead of reading it from
 *       the socket.
 *
 *    4) When all data in buf has been returned, clear the iobuffer.
 *
 * The operation when writing is more complicated, because
 * we can get multiple write requests that we fail to send down
 * to the socket buffer, which in sum may be bigger than the
 * the iobuffer set aside to hold buffered unwritten data.
 *
 * The only way to prevent that situation from occurring is to
 * put a cap on how much we read, and never read more data than
 * we can store in our write-buffer, encoded.
 * We can use gss_wrap_size_limit() in combination with the amount
 * of data free in the buffer to find out the max amount of data to
 * read, and read no more than that in the tcp case.
 *
 * The operation for writing thus becomes:
 * 1) Encode the data received and write it to the socket.
 *
 * 2) If we fail to write all the data, and it is a tcp socket,
 *    store the remaining data in the iobuffer, setting encodedlen
 *    to the size of data remaining, and used to zero.
 *    If it's a udp socket, there is not much to do, so return the
 *    error.
 *
 * 3) On subsequent write operations on the socket, try to write the
 *    data we had previously buffered.  Then continue with 1).
 *
 $ 4) When all data has been written, clear the iobuffer.
 *
 */

typedef enum { READ_BUF  = 0 /* MUST BE 0 or 1 */,
               WRITE_BUF = 1 /* MUST BE 0 or 1 */ } whichbuf_t;

typedef struct {
   unsigned char allocated;
   int           s;

#if HAVE_GSSAPI
#  if (SOCKD_BUFSIZE) < (2 * (MAXGSSAPITOKENLEN + GSSAPI_HLEN))
 #     error "SOCKD_BUFSIZE too small."
#  endif
#endif /* HAVE_GSSAPI */

   char         buf[2][SOCKD_BUFSIZE];

   struct {
      size_t   len;        /* length of decoded/plaintext data in buffer      */
      size_t   enclen;     /* length of encoded data in buffer.               */

      int      mode;       /* buffering mode.  Default is no buffering.       */
      ssize_t  size;       /*
                            * size of buffer to use.  Can not be larger than
                            * SOCKD_BUFSIZE.  Default is SOCKD_BUFSIZE.
                            */

#if SOCKS_CLIENT
      size_t   readalready;/*
                            * # of bytes we have already read from socket and
                            * should ignore.
                            */
#endif /* SOCKS_CLIENT */
   } info[2];

   int      stype;         /* socket type; tcp or udp                         */
} iobuffer_t;

typedef struct route_t {
   int              number;   /* route number.                                */

   struct {
      unsigned char autoadded;/* autoadded route?                             */
      size_t        failed;   /* route is bad?  How many times it has failed. */
      time_t        badtime;  /* if route is bad, time last marked as such.   */
   } state;

   socketoption_t   *socketoptionv;
   size_t           socketoptionc;

   ruleaddr_t       src;
   ruleaddr_t       dst;
   gateway_t        gw;

   ruleaddr_t       rdr_from;

   struct route_t   *next;      /* next route in list.               */
} route_t;

typedef struct {
   unsigned char        allocated;  /* allocated?                             */
   int                  control;    /* control connection to server.          */
   socksstate_t         state;      /* state of this connection.              */

   struct sockaddr_storage local;   /* local address of control connection.   */
                                    /* XXX does not look to be case for udp.  */

   struct sockaddr_storage server;  /* remote address of control connection.  */
   struct sockaddr_storage remote;  /* address server is using on our behalf. */
   struct sockaddr_storage reply;   /*
                                     * address to expect reply from, if not
                                     * same as control.
                                     */

   union {
      sockshost_t   accepted;   /* address server accepted for us.     */
      sockshost_t   connected;  /* address server connected to for us. */
   } forus;

   route_t      *route;
} socksfd_t;

/*
 * getaddrinfo(3) returns separate entries for udp and tcp, even when
 * everything else is the same.  That means we effectively only get half
 * MAX_ADDRINFO_NEXT unique ipaddresses at most.  We used 5 when we were
 * using the gethostby*(3) api, so double for getaddrinfo(3).
 * Might think about filtering out otherwise duplicate tcp/udp entries to
 * save memory.  Would make the cache less general though.
 *
 * An additional thing to consider is that there is no way to specify in
 * the getaddrinfo(3)-api that we want ipv4-mapped ipv6 addresses
 * returned too, when we set ai_family to AF_INET in hints.  The only way
 * to getr the ipv4-mapped addresses returned to is to set ai_family to
 * zero, but then we get the regular ipv6-addresses also.  Since there
 * are cases when we want the ipv4-mapped addresses returned also, we
 * need to make the size set here able to accomodate that too.
 */
#define MAX_ADDRINFO_NEXT (10)

typedef enum { id_name = 1, id_addr } hostent_id_t;

typedef struct {
   unsigned       allocated:1;      /* entry allocated?                       */
   time_t         written;          /* time this entry was created.           */

   /* if looked up address/name was found, 0.  Otherwise errorcode.  */
   int            gai_rc;

   /*
    * address or hostname used with DNS for this entry.
    * Note that gethostbyname(x) may return address k, but gethostbyaddr(k)
    * need not return the hostname 'x', so we need different entries for
    * hostnames and addresses and can not reuse one as the other.
    */
   hostent_id_t key;
   union {
      char                     name[MAXHOSTNAMELEN];
      struct sockaddr_storage  addr;
   } id;


   char            service[MAXSERVICELEN];

   union {
      struct {
            struct addrinfo addrinfo;
            /*
             * The pointers in addrinfo are set to point to the below memory,
             * to avoid having to do dynamic allocation/free repeatedly.
             */

            /*
             * there is only one hostname returned in struct addrinfo
             * by getnameinfo(3).
             */
            char            ai_canonname_mem[MAXHOSTNAMELEN];

            /* but there can be multiple addresses. */
            struct sockaddr_storage ai_addr_mem[MAX_ADDRINFO_NEXT];

            struct addrinfo         ai_next_mem[MAX_ADDRINFO_NEXT];

            struct addrinfo   *hints;
            struct addrinfo   hints_mem; /* memory for hints, if not NULL. */
      } getaddr;

      struct {
         char name[MAXHOSTNAMELEN];
         int  flags;
      } getname;
   } data;
} dnsinfo_t;


#define HOSTENT_MAX_ALIASES (5)
typedef struct {
   unsigned       allocated:1;      /* entry allocated?                       */
   unsigned       notfound:1;       /* looked up address/name was not found.  */
   time_t         written;          /* time this entry was created.           */


   /*
    * address or hostname used with DNS for this entry.
    * Note that gethostbyname(x) may return address k, but gethostbyaddr(k)
    * need not return the hostname 'x', so we need different entries for
    * hostnames and addresses and can not reuse one as the other.
    */
   hostent_id_t key;
   union {
      char            name[MAXHOSTNAMELEN];
      struct in_addr  ipv4;
   } id;

   struct hostent hostent;

   /*
    * The contents of hostent is set to point to the corresponding area here,
    * rather than allocating it on the stack.
    * We add one to HOSTENT_MAX_ALIASES because the last index is used
    * for NULL-terminating, as per the libresolv (rfc?) spec.
    */
   char h_name[MAXHOSTNAMELEN];
   char *h_aliases[HOSTENT_MAX_ALIASES + 1];
   char *h_addr_list[HOSTENT_MAX_ALIASES + 1];

   /* memory for above arrays. */
   char h_aliasesmem[HOSTENT_MAX_ALIASES + 1][MAXHOSTNAMELEN];
   char h_addr_listmem[HOSTENT_MAX_ALIASES + 1][MAX(sizeof(struct in_addr),
                                                    sizeof(struct in6_addr))];

} hostentry_t;

typedef struct {
#if !SOCKS_CLIENT
   interfaceside_t side;        /* what interface-side we are sending out on. */
   struct sockaddr_storage peer;/* peer we are receiving from.                */
#endif /* !SOCKS_CLIENT */

   int            type;       /* socket type; SOCK_DGRAM or SOCK_STREAM.      */
   int            flags;      /* flags set on the received data.              */

   size_t         fromsocket; /*
                               * number of bytes read from socket.  This
                               * may differ from the return value of the
                               * function if some sort of encapsulation
                               * is used (currently only gssapi is
                               * possible), or parts or all of the data was
                               * read from an internal buffer rather than
                               * from the socket itself.
                               * It may also be less than the return value,
                               * if more was read from the socket than was
                               * returned at this time.
                               */

   struct timeval ts;      /* time packet was received (for datagram only). */
} recvfrom_info_t;

typedef struct {
   interfaceside_t side; /*
                          * what interface-side we are sending out on.
                          * Only used by server.
                          */

   size_t          tosocket; /*
                              * number of bytes written to socket.  This
                              * may differ from the return value of the
                              * function if some sort of encapsulation
                              * (currently only gssapi is possible) is
                              * used, or parts or all of the data was
                              * written from an internal buffer rather
                              * than to the socket itself.
                              */
} sendto_info_t;




/*
 * versions of BSD's error functions that log via slog() instead.
 */

void serr(const char *fmt, ...)
          __ATTRIBUTE__((noreturn)) __ATTRIBUTE__((FORMAT(printf, 1, 2)));

void serrx(const char *fmt, ...)
          __ATTRIBUTE__((noreturn)) __ATTRIBUTE__((FORMAT(printf, 1, 2)));

void swarn(const char *fmt, ...)
           __ATTRIBUTE__((FORMAT(printf, 1, 2)));

void swarnx(const char *fmt, ...)
            __ATTRIBUTE__((FORMAT(printf, 1, 2)));


void
runenvcheck(void);
/*
 * Verify that run environment corresponds to build environment.
 */

long long
maxvalueoftype(const size_t typelen);
/*
 * Gives the maxvalue for a signed integer-type stored in a object of length
 * "typelen".
 */

long long
minvalueoftype(const size_t typelen);
/*
 * Gives the minvalue for a signed integer-type stored in a object of length
 * "typelen".
 */

unsigned long long
umaxvalueoftype(const size_t typelen);
/*
 * Gives the maxvalue for an unsigned integer-type stored in a object of length
 * "typelen".
 */

unsigned long long
uminvalueoftype(const size_t typelen);
/*
 * Gives the minvalue for an unsigned integer-type stored in a object of length
 * "typelen".
 */


void
genericinit(void);
/*
 * Generic init, called after clientinit()/serverinit().
 */

void
optioninit(void);
/*
 * sets options to a reasonable default.
 */

void
postconfigloadinit(void);
/*
 * To be called after config is loaded.
 */

int
socks_initupnp(gateway_t *gw, char *emsg, const size_t emsglen);
/*
 * Inits upnp for interface corresponding to the gateway "gw".
 * If successful, the necessary information to later use the found
 * upnp router is saved in "data", which should normally be part of a
 * route object.
 *
 * Returns:
 *    On success: 0.
 *    On failure: -1 (no appropriate upnp devices found, or some other error.)
 */

void
newprocinit(void);
/*
 * After a new process is started/forked, this function should
 * be called.  It will initialize various things, open needed
 * descriptors, etc. and can be called as many times as wanted.
 */

void *
udpheader_add(const sockshost_t *host, void *msg, size_t *len,
              const size_t msgsize);
/*
 * Prefixes the udpheader_t version of "host" to a copy of "msg",
 * which is of length "len".
 * "msgsize" gives the total size of the memory pointed to by "msg",
 * which may be up to "len" big.
 *
 * If "msgsize" is large enough the function will prepend the socks
 * udpheader to "msg", moving the old contents in "msg" to the right.
 * If not, NULL will be returned with errno set to EMSGSIZE.  This
 * should only happen if the payload + the socks udpheader is larger
 * than the maxsize of a UDP (IP) packet.
 *
 * Returns:
 *   On success: "msg" with the udpheader prepended.  The length of the new
       message is stored in "len".
 *   On failure: NULL (message to large).
 */

int
fdisopen(const int fd);
/*
 * returns true if the file descriptor "fd" currently references a open fd,
 * false otherwise.
 */

int
fdisblocking(const int fd);
/*
 * returns true if the file descriptor "fd" is blocking, false otherwise.
 */

void
closev(size_t ic, int *iv);
/*
 * Goes through "iv", which contains "ic" elements.
 * Each element that does not have a negative value is closed.
 */

const loglevel_t *
loglevel(const char *name);
/*
 * Returns the loglevel value having the name "name".
 * Returns NULL if there is no such loglevel.
 */


int
socks_logmatch(int d, const logtype_t *log);
/*
 * Returns true if "d" is a descriptor matching any descriptor in "log".
 * Returns false otherwise.
 */

struct sockaddr_storage *
int_sockshost2sockaddr2(const sockshost_t *shost, struct sockaddr_storage *addr,
                        size_t addrlen, int *gaierr,
                        char *emsg, size_t emsglen);
/*
 * Converts the sockshost_t "shost" to a sockaddr struct and stores it
 * in "addr".  If conversion fails, 0/0 is stored in "addr" and "gaierr"
 * is set to the resolver errorcode.  Otherwise, "gaierr" is set to 0.
 *
 * If "addr" is NULL, the function uses a static buffer which may be
 * overwritten on the next call to this function.
 *
 * If conversion fails, emsg contains the reason.  If failure is related
 * to DNS, "gaierr" is in addition set.
 *
 * Returns: "addr".
 *
 */

struct sockaddr_storage *
int_sockshost2sockaddr(const sockshost_t *shost, struct sockaddr_storage *addr,
                       size_t addrlen);
/*
 * like int_sockshost2sockaddr(), but without the "gaierr" argument.
 */


struct sockaddr_storage *
int_fakesockshost2sockaddr(const sockshost_t *host,
                           struct sockaddr_storage *addr, size_t addrlen);
/*
 * Like sockshost2sockaddr(), but checks whether the address in
 * "host" is fake when converting.
 */

struct sockaddr_storage *
int_urlstring2sockaddr(const char *string, struct sockaddr_storage *addr,
                       size_t addrlen, int *gaierr,
                       char *emsg, size_t emsglen);
/*
 * Converts the address given in "string", on URL:// format, to
 * a sockaddr address stored in "saddr".
 *
 * Returns "saddr" on success.
 *
 * Returns NULL on failure with the reason written to "emsg", which must
 * be of at least "emsglen" size.  If failure is due to resolver failure,
 * "gaierr" will contain the resolver errorcode, or 0 if not.
 */

sockshost_t *
sockaddr2sockshost(const struct sockaddr_storage *addr, sockshost_t *host);
/*
 * Converts the sockaddr struct "shost" to a sockshost_t struct and stores it
 * in "host".  If "host" is NULL, a static host object is used instead.
 *
 * Returns: a pointer to the object containing the sockshost_t representation.
 */

int
sockaddr2hostname(const struct sockaddr_storage *sa, char *hostname,
                  const size_t hostnamelen);
/*
 * reversemaps "sa" to hostname and stores it "hostname", which is at least
 * "hostnamelen" bytes big.
 *
 * Returns:
 *    0 on success.
 *    The corresponding getnameinf(3) error on failure.
 */

sockshost_t *
ruleaddr2sockshost(const ruleaddr_t *address, sockshost_t *host, int protocol);
/*
 * Converts the ruleaddr_t "address" to a sockshost_t struct and stores it
 * in "host".
 * Returns: "host".
 */

struct sockaddr_storage *
int_ruleaddr2sockaddr(const ruleaddr_t *address, struct sockaddr_storage *sa,
                      size_t salen, const int protocol);
/*
 * Converts the ruleaddr_t "address" to a sockshost_t struct and stores it
 * in "sa" if not NULL.
 *
 * Returns: "sa".
 */

struct sockaddr_storage *
int_ruleaddr2sockaddr2(const ruleaddr_t *address, struct sockaddr_storage *sa,
                       size_t salen, const int protocol, int *gaierr,
                       char *emsg, const size_t emsglen);
/*
 * Converts the ruleaddr_t "address" to a sockshost_t struct and stores it
 * in "sa" if not NULL.
 *
 * On error, "gaierr" may be set, and "emsg" will be set if not NULL.
 *
 * Returns: "sa".
 */



ruleaddr_t *
sockshost2ruleaddr(const sockshost_t *host, ruleaddr_t *addr);
/*
 * Converts the sockshost_t "host" to a ruleaddr_t struct and stores it
 * in "addr".
 * Returns: "addr".
 */

ruleaddr_t *
sockaddr2ruleaddr(const struct sockaddr_storage *addr, ruleaddr_t *ruleaddr);
/*
 * Converts the struct sockaddr "addr" to a ruleaddr_t struct and stores
 * it in "ruleaddr".
 * Returns: "addr".
 */

struct sockaddr_storage *
int_hostname2sockaddr(const char *name, size_t index,
                      struct sockaddr_storage *addr, size_t addrlen);
/*
 * Retrieves the address with index "index" for the hostname named "name".
 * Returns:
 *      On success: "addr", filled in with the address found.
 *      On failure: NULL (no address found).
 */

struct sockaddr_storage *
int_hostname2sockaddr2(const char *name, size_t index,
                       struct sockaddr_storage *addr, size_t addrlen,
                       int *gaierr, char *emsg, const size_t emsglen);
/*
 * Retrieves the address with index "index" for the hostname named "name".
 * Returns:
 *      On success: "addr", filled in with the address found.
 *      On failure: NULL (no address found).  "gaierr" contains the
 *                  resolver error, if available, and "emsg" contains
 *                  a textual description of the error.
 */


struct sockaddr_storage *
int_ifname2sockaddr(const char *ifname, size_t index,
                    struct sockaddr_storage *addr, size_t addrlen,
                    struct sockaddr_storage *mask, size_t masklen);
/*
 * Retrieves the address with index "index" on the interface named "ifname".
 * If "mask" is not NULL, the mask of the interface is stored here.
 *
 * Returns:
 *      On success: "addr", and possibly "netmask", filled in with the
 *                  address found.
 *      On failure: NULL (no address found on the interface at that index.
 */

const char *
sockaddr2ifname(struct sockaddr_storage *addr, char *ifname, size_t iflen);
/*
 * Retrieves the name of the interface the address "addr" belongs to.
 * The name is written to "ifname", which must be of len "iflen".
 * If "ifname" or "iflen" is NULL, the name is written to a local
 * buffer instead.
 *
 * Returns a pointer to the memory containing the interface name, or
 * NULL if no matching interface is found.
 *
 */

int
sockatmark(int s);
/*
 * If "s" is at oob mark, return 1, otherwise 0.
 * Returns -1 if a error occurred.
 */

ssize_t
recvmsgn(int s, struct msghdr *msg, int flags);
/*
 * Like recvmsg(), but handles some os-specific bugs.
 */

ssize_t
sendmsgn(int s, const struct msghdr *msg, int flags, const time_t timeoutms);
/*
 * Like sendmsg(), but retries on temporary errors, including blocking
 * with select(2) for up to "timeoutms" milliseconds.
 *
 * If "timeout" is -1, block forever, or until we've failed a predefined
 * number of maxtimes, whatever comes first.
 */

ssize_t
readn(int, void *, size_t, const size_t minread, authmethod_t *auth)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * Like read() but with two additional arguments:
 * minread - the minimum amount of bytes to read before returning, or error.
 * auth    - authentication info for the file descriptor.  May be NULL.
 */

ssize_t
writen(int, const void *, size_t, const size_t minwrite,
       authmethod_t *auth)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * like write() but if with two additional arguments:
 * minwrite - the minimum amount of bytes to write before returning, or error.
 * auth     - authentication info for the file descriptor.  May be NULL.
 */

ssize_t
socks_recvfrom(int s, void *buf, size_t len, int flags,
               struct sockaddr_storage *from,
               socklen_t *fromlen, recvfrom_info_t *recvflags,
               authmethod_t *auth)
               __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * Similar to recvfrom(), but with two additional arguments:
 *  - auth:       if not NULL, the authentication used for this session.
 *  - recvflags:  if not NULL, information about the data received.
 */

ssize_t
socks_recvfromn(const int s, void *buf, size_t len, const size_t minread,
                const int flags, struct sockaddr_storage *from,
                socklen_t *fromlen, recvfrom_info_t *recvflags,
                authmethod_t *auth)
                __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * Like socks_recvfrom(), but retries until minread has been read, or failure.
 */

ssize_t
socks_sendto(int s, const void *msg, size_t len, int flags,
             const struct sockaddr_storage *, socklen_t,
             sendto_info_t *sendflags, authmethod_t *auth)
             __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * Like sendto(), but with two additional arguments:
 * - sendflags: if not NULL, updated with sendto_info upon return.
 */

ssize_t
socks_sendton(int s, const void *buf, size_t len, const size_t minwrite,
              int flags, const struct sockaddr_storage *to, socklen_t tolen,
              sendto_info_t *sendflags, authmethod_t *auth)
              __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * Like socks_sendto(), but retries until "minwrite" is written, or failure.
 */

int
linkednamesareeq(const linkedname_t *a, const linkedname_t *b);
/*
 * Checks if the contents and order of the linked lists "a" and "b"
 * is equal.
 *
 * Returns true if a and b are equal.
 * Returns false if a and b are not equal.
 */


int
closen(int);
/*
 * Wrapper around close().  Retries on EINTR.
 */

struct timeval *
gettimeofday_monotonic(struct timeval *tv);
/*
 * Similar to gettimeofday(2), but time is monotonic.
 * Returns "tv", filled with the current time.
 */

time_t
time_monotonic(time_t *tloc);
/*
 * Similar to time(3), but time is monotonic.
 */

unsigned long
tv2us(const struct timeval *tv);
/*
 * converts "tv" to microseconds and returns the result.
 */

struct timeval *
us2tv(const unsigned long us, struct timeval *tv);
/*
 * converts "usec" to struct timeval and stores the result in "tv".
 * Returns "tv".
 */


int
selectn(int nfds, fd_set *rset, fd_set *bufrset, fd_set *buffwset,
         fd_set *wset, fd_set *xset, struct timeval *);
/*
 * Wrapper around select() that takes two additional arguments:
 * bufrset  - if not NULL, set to contain descriptors with data buffered
 *            for reading.
 * buffwset - if not NULL, set to contain descriptors with data buffered
 *            for writing (buffered-for-writing).
 *
 * In addition, if it's called by the server, it checks whether we
 * have a signal queued internally, and if so calls the appropriate
 * signal handler.
 */

int
acceptn(int, struct sockaddr_storage *, socklen_t *);
/*
 * Wrapper around accept().  Retries on EINTR.
 */

int
socks_socket(const int domain, const int type, const int protocol);
/*
 * Like socket(2), but also sets some options we always want to set.
 */


int
setblocking(const int fd, const char *context);
/*
 * Configures "fd" to use blocking i/o.
 * "context" provides the context "fd" will be used in.
 *
 * Returns:
 *    On success: the original fd-flags set on fd "fd" before the change.
 *    On failure: -1.  In this case errno will be set and a warning will
 *                     be logged using "context".
 */

int
setnonblocking(const int fd, const char *context);
/*
 * Configures "fd" to use non-blocking i/o.
 * "context" provides the context "fd" will be used in.
 *
 * Returns:
 *    On success: the original fd-flags set on fd "fd" before the change.
 *    On failure: -1.  In this case errno will be set and a warning will
 *                     be logged using "context".
 */

int
socks_socketisforlan(const int s);
/*
 * If we can determine that the socket "s" is for lan use only, i.e. should
 * not be proxied, returns true.  Otherwise, returns false.
 */

struct sockaddr_storage *
socketisconnected(const int s, struct sockaddr_storage *addr, socklen_t alen);
/*
 * If the socket "s" is connected to a peer, returns peer's address.
 * If "addr" is not NULL, the peer address is stored in "addr", and a
 * pointer to "addr" is returned.  Otherwise a static buffer is used
 * and a pointer to that static buffer is returned.
 *
 * If the socket "s" is not connected, NULL is returned.
 */

int
fdisdup(const int fd1, const int fd2);
/*
 * Tries to determine if file descriptor fd1 is a dup of fd2.
 * Returns true if yes, false if not.
 */


int
socks_rebind(int s, int protocol, struct sockaddr_storage *from,
             const struct ruleaddr_t *to, char *emsg, const size_t emsglen);
/*
 * Tries to rebind the socket 's", currently bound to address "from", if
 * any, to match the range given by "to".
 *
 * Returns 0 on success.
 * Returns 0 on failure.  Error will be written to emsg and errno will be set.
 */

int
socks_bindinrange(int s, struct sockaddr_storage *addr,
                  in_port_t first, in_port_t last, const enum operator_t op);
/*
 * Like sockd_bind(), but keeps trying to sockd_bind a address in the
 * range given by "addr", as indicated by "first", "last" and "op",
 * until whole range has been tried.
*/

int
socks_bind(int s, struct sockaddr_storage *addr, size_t retries);
/*
 * Binds the address "addr" to the socket "s".  The bind call will
 * be tried "retries" + 1 times if the error is EADDRINUSE, or until
 * successful, whatever comes first.
 *
 * If the port number is privileged, it will set and reset the euid
 * as appropriate.  (Only applies when called called by server.)
 *
 * If successful, "addr" is filled in with the bound address.
 * Returns:
 *      On success: 0.
 *      On failure:   -1
 */

const struct in_addr *
ipv4_addrisinlist(const struct in_addr *addr, const struct in_addr *mask,
                  const struct addrinfo *ailist);

const struct in6_addr *
ipv6_addrisinlist(const struct in6_addr *addr, const unsigned int maskbits,
                  const struct addrinfo *ailist);

/*
 * Compares "addr", bitwise AND-ed with "mask", against each IPv4 or IPv6
 * address * in "list", also bitwise AND-ed with "mask".
 *
 * Returns:
 *      If "list" contains a element matching "addr": pointer to the matching
                                                      address in ailist.
 *      otherwise: NULL.
 */


int
sockaddrareeq(const struct sockaddr_storage *a,
              const struct sockaddr_storage *b, const size_t nocompare);
/*
 * Compares the address "a" against "b".
 * If "nocompare" is not zero, the attributes set (ADDRINFO_PORT, etc.)
 * are excluded from the comparison.
 *
 * Returns:
 *      If "a" and "b" are equal: true
 *      else: false
 */


void
usrsockaddrcpy(struct sockaddr_storage *dst, const struct sockaddr_storage *src,
               const size_t len);
/*
 * Duplicate contents of sockaddr structure, up to len bytes.
 * Variant of sockaddrcpy() for sockaddr copying sockaddr data
 * from clients in Rfoo() functions.
 */

void
sockaddrcpy(struct sockaddr_storage *dst, const struct sockaddr_storage *src,
            const size_t len);
/*
 * Duplicate contents of sockaddr structure, up to len bytes.
 */

sa_len_type
salen(const sa_family_t family);
/*
 * returns the sockaddrlen of a sockaddr struct of family "family".
 */

sa_len_type
inaddrlen(const sa_family_t family);
/*
 * returns the ipaddresslen of an address of the familytype "family".
 */


int
safamily_issupported(const sa_family_t family);
/*
 * Returns true if we support the sockaddr family "family".
 * Returns false if we do not.
 */

sa_family_t
atype2safamily(const int atype);
/*
 * Returns the sa_family_t equivalent of the SOCKS address type
 * "atype", which must be one of SOCKS_ADDR_IPV4 or SOCKS_ADDR_IPV6.
 */

int
safamily2atype(const sa_family_t safamily);
/*
 * Returns the atype equivalent of the sa_family_t "safamily".
 */


int
addrinfo_issupported(const struct addrinfo *ai);
/*
 * Returns true if we support the addrinfo in "ai".
 * Returns false if we do not.
 */


/*
 * Wrapper around standard functions.
 */
const char *socks_strerror(const int err);
const char *socks_gai_strerror(const int err);

size_t
snprintfn(char *str, size_t size, const char *format, ...)
      __ATTRIBUTE__((FORMAT(printf, 3, 4)))
      __ATTRIBUTE__((__NONNULL__(3)))
      __ATTRIBUTE__((__BOUNDED__(__string__, 1, 2)));
/*
 * Wrapper around snprintf() for consistent behavior.  Same as stdio
 * snprintf() but the following are also enforced:
 *      returns 0 instead of -1 (rawterminates *str) on error.
 *      never returns a value greater than size - 1.
 */

void
socks_sigblock(const int sig, sigset_t *oldset);
/*
 * If "sig" is -1, blocks all signals.  If not, adds only "sig" to
 * the list of currently blocked signals.
 *
 * The old signal mask is returned in "oldset".
 */

void
socks_sigunblock(const sigset_t *oldset);
/*
 * Restores the current signal mask to "oldset".
 */

const char *
strcheck(const char *string);
/*
 * Checks "string".  If it is NULL, returns a string indicating memory
 * exhausted, if not, returns the same string it was passed.
 */

unsigned char *
sockshost2mem(const sockshost_t *host, unsigned char *mem, int version);
/*
 * Writes "host" out to "mem".  The caller must make sure "mem"
 * is big enough to hold the contents of "host".
 * "version" gives the socks version "host" is to be written out in.
 * Returns a pointer to one element past the last byte written to "mem".
 */

const unsigned char *
mem2sockshost(sockshost_t *host, const unsigned char *mem, size_t len,
      int version)
      __ATTRIBUTE__((__BOUNDED__(__buffer__, 2, 3)));
/*
 * Writes "mem", which is assumed to be a sockshost string
 * of length "len" in version "version" in network order, out to "host".
 * Returns:
 *      On success: pointer to one element past last byte used of mem
 *                  and fills in "host" appropriately.
 *      On failure: NULL ("mem" is not a valid sockshost.)
 */

unsigned int socks_get_responsevalue(const response_t *response);
void socks_set_responsevalue(response_t *response, unsigned int value);
/*
 * Functions to fetch or set the value of the response, depending on what
 * version the response belongs to.
 */

int *
charmethod2intmethod(const size_t methodc,
                     const unsigned char charmethodv[], int intmethodv[]);
/*
 * converts char method array "charmethodv" with "methodc" elements,
 * to a integer method array, storing the result in "intmethodv".
 *
 * Returns "intmethodv".
 */


int
proxyprotocolisknown(const int version);
/*
 * Returns true if "version" is a known proxy version.  0 if not.
 */

int
authmethodisknown(const int method);
/*
 * Returns true if "method" is a known authmethod.  0 if not.
 */

int
socks_addlogfile(logtype_t *logcf, const char *logfile);
/*
 * Adds the file "logfile" to the list of files we log to, stored in "logcf".
 *
 * Returns 0 on success.
 * Returns -1 on failure.
 */

void slog(int priority, const char *fmt, ...)
      __ATTRIBUTE__((FORMAT(printf, 2, 3)));
/*
 * Logs message "fmt" at priority "priority" to previously configured
 * output device.
 * Checks settings and ignores message if it's of to low a priority.
 */

void vslog(int priority, const char *fmt, va_list ap, va_list apcopy)
      __ATTRIBUTE__((FORMAT(printf, 2, 0)));
/*
 * Same as slog() but assumes varargs/stdargs have already processed
 * the arguments.
 */

void
signalslog(const int priority, const char *msgv[]);
/*
 * Similar to slog(), but simpler.
 *
 * "msgv" is an array of NUL-terminated strings.  The last element in this
 * array must be NULL.
 * The function logs all the strings in "msgv", as one.  Caller must take
 * care of any desired space between the strings.
 *
 * Can be called from a signalhandler.
 */



int
parseconfig(const char *filename);
/*
 * Parses the config stored in in the filename "filename", as well
 * as environment-variables related.
 *
 * Returns:
 *      On success: 0.
 *      On failure: -1.
 */

void
resetconfig(struct config *config, const int exiting);
/*
 * resets the config "config" back to default, freeing memory as well.
 * If "exiting" is true, we are exiting and don't need to save
 * anything.
 */


int
addrmatch(const ruleaddr_t *rule, const sockshost_t *address,
          sockshost_t *addrmatched, int protocol, int ipalias);
/*
 * Tries to match "address" against "rule".  "address" is resolved
 * if necessary.  "rule" supports the wildcard INADDR_ANY and port of 0.
 * "protocol" is the protocol to compare under.
 *
 * If "ipalias" is true and "address" is an IP-address, the function will
 * try to reverse-map "address" to hostname, the hostname to ip, and match
 * those ipaddresses against "rule".
 *
 * Returns true if "rule" matched "address".  In this case, if "addrmatched"
 * is not NULL, it is updated to reflect the address that matched, which may
 * be different from "address" if "address" had to be resolved or
 * reversemapped.
 */

int
socks_connecthost(int s,
#if !SOCKS_CLIENT
                  const interfaceside_t side,
#endif /* !SOCKS_CLIENT */
                  const sockshost_t *host,
                  struct sockaddr_storage *laddr,
                  struct sockaddr_storage *raddr,
                  const long timeout, char *emsg, const size_t emsglen);
/*
 * Tries to connect to "host".  If "host"'s address is not a IP address,
 * the function also tries to connect to any alias for "host"'s
 * name.  The connection is done using the open descriptor "s".
 *
 * If "laddr" is not NULL, it is filled in with the address we connected to
 * "host" from, if a connect(2) was initiated.
 *
 * If "raddr" is not NULL, it is filled in with the address connected to if
 * successful.  If "host" is a an ip address, it will be identical to that
 * ip address, but if "host" is a hostname, they will of course differ.
 *
 * If "timeout" is not negative, it gives the timeout for how long to wait
 * for the connect to complete.  A value of zero means no wait will be
 * done, and the the function may return with errno set to EINPROGRESS.
 * A negative value for timeout means wait the kernel/system default.
 *
 * If the function fails, the reason is written to emsg, which must be
 * at least "emsglen" long.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1.  Reason for error is written to emsg.
 */

route_t *
socks_connectroute(const int s, socks_t *packet,
                   const sockshost_t *src, const sockshost_t *dst,
                   char *emsg, const size_t emsglen);
/*
 * Finds a route from "src" to "dst" and connects to it "s".
 * If src or dst is NULL, that argument is ignored.
 *
 * The route used may take into account the contents of "packet->req",
 * which is assumed to be the packet that will be sent to a socks server,
 * so it is recommended that it's contents be as conservative as possible.
 *
 * When it has successfully connected to a gateway it will set
 * the packet->method members to point to the methods the gateway
 * should be offered.
 *
 * Returns:
 *      On success: the route that was used.
 *      On failure: NULL.  See emsg for reason.
 */

route_t *
socks_requestpolish(request_t *req, const sockshost_t *src,
                    const sockshost_t *dst);
/*
 * Tries to "polish" (modify) the request "req" for a session from "src"
 * to "dst", so that a later socks_getroute() will succeed.
 *
 * Returns:
 *      On success: the route that will match the polished request.
 *      On failure: NULL.
 */

int
socks_routesetup(const int control, const int data, const route_t *route,
                 char *emsg, const size_t emsglen);
/*
 * Prepares for establishing a session via route "route".
 * "control" gives the controlsocket that will be used.
 * "data" gives the data socket that will be used.
 *
 * Return 0 on success.
 * Returns -1 on failure.  Reason for failure will be stored in "emsg", and
 *                         errno will be set appropriately.
 */

void
showstate(const serverstate_t *state);
/*
 * logs a printable representation of "state" to the logfile.
 */

void
showmethod(objecttype_t type, size_t methodc, const int *methodv);
/*
 * Shows methods set in "methodv" array.
 * "type" indicates whether the methods are for a client/hostid-rule,
 * and thus are clientmethods, or for a socksrule, and thus are socksmethods.
 */

void
showtimeout(const timeout_t *timeout);
/*
 * shows timeouts set in "timeout".
 */


void
freeroutelist(route_t *routehead);
/*
 * Frees a list of routes and their contents, starting at "routehead".
 */

route_t *
socks_addroute(const route_t *route, const int last);
/*
 * Appends a copy of "route" to our list of routes.
 * If "last" is true, the route is added to the end of our list.
 * If not, it's added to the start, and existing rule numbers are updated
 * correspondingly.
 *
 * Returns a pointer to the added route.
 */

route_t *
socks_autoadd_directroute(const command_t *commands,
                          const protocol_t *protocols,
                          const struct sockaddr_storage *saddr,
                          const struct sockaddr_storage *netmask);
/*
 * Adds a direct route to "saddr", netmask "netmask", for the commands
 * "commands" and protocols "protocols".
 *
 * Intended to be used for routes that are added automatically,
 * and not via socks.conf.
 */

void
socks_showroute(const route_t *route);
/*
 * prints the route "route".
 */

route_t *
socks_getroute(const request_t *req, const sockshost_t *src,
               const sockshost_t *dst);
/*
 * Tries to find a  route to be used for a connection going from
 * "src" to "dst".
 * If src or dst is NULL, that argument is ignored.
 *
 * The route used may take into account the contents of "req", which is
 * assumed to be the packet that will be sent to a socks server, so it is
 * recommended that it's contents be as conservative as possible.
 *
 * Returns:
 *      On success: pointer to route that should be used.
 *      On failure: NULL (no route found).
 */

unsigned int
sockscode(const int version, const int code);
/*
 * Maps the socks reply code "code", which is in non-version specific format,
 * to the equivalent reply code in version "version".
 */

unsigned int
errno2reply(int errnum, int version);
/*
 * Returns the proxy version "version" reply code for a error of type "errno".
 */

char *
str2vis(const char *string, size_t len, char *visstring, size_t vislen)
      __ATTRIBUTE__((__BOUNDED__(__string__, 3, 4)));
/*
 * Visually encodes exactly "len" chars of "string" and stores the
 * result in "visstring", which is of length "vislen".  "vislen" should
 * be at least "len" * 4 + 1.
 * Note that the function really will encode len characters, including
 * any NUL-characters.
 *
 * Returns: the visually encoded string, "visstring".
 */

in_addr_t
socks_addfakeip(const char *name);
/*
 * Adds "name" to a internal table indexed by (fake)IP addresses.
 * Returns:
 *      On success: "name"'s index.
 *      On failure:   INADDR_NONE
 */

const char *
socks_getfakehost(in_addr_t addr);
/*
 * If "addr" is a "fake" (non-resolved) addr, it returns the name
 * corresponding to it.
 * Else, NULL is returned.
 */

int
socks_getfakeip(const char *host, struct in_addr *addr);
/*
 * If "host" has a fake address entry, the address is written into
 * "addr".
 * Returns:
 *      If a fake address exits: true
 *      Else: false
 */

sockshost_t *
fakesockaddr2sockshost(const struct sockaddr_storage *addr, sockshost_t *host);
/*
 * Identical to sockaddr2sockshost, but checks whether
 * the address in "addr" is a "fake" one when converting.
 */

int
sockshostareeq(const sockshost_t *a, const sockshost_t *b);
/*
 * Compares the address "a" against "b".
 * Returns:
 *      If "a" and "b" are equal: true
 *      else: false
 */

int
fdsetop(int highestfd, int op, const fd_set *a, const fd_set *b,
        fd_set *result);
/*
 * Performs operation on descriptor sets.
 * "highestfd" is the descriptors with the highest index to perform operation
 * "op" on in the sets "a" and "b".
 *
 * The result of the operation is stored in "result".
 *
 * Returns the number of the highest descriptor set in "result".
 * NOTES:
 *      - operators supported is: AND ('&'), XOR ('^'), and OR ('|').
 */

int
methodisset(int method, const int *methodv, size_t methodc);
/*
 * Returns true if the method "method" is set in "methodv", false otherwise.
 * "methodc" is the length of "methodv".
 */


char *
get_tcpinfo(const size_t fdc, int fdv[], char *buf, size_t buflen);
/*
 * Retrieves tcp_info for the sockets in "fdv" and stores it in "buf", if
 * buf is not NULL.
 * If buf or buflen is not set, stores it in locally allocated memory
 * and return a pointer to it rather than to buf.
 *
 * If tcpinfo can not be retrieved for any of the sockets in fdv,
 * that index in fdv is set to -1.
 *
 * Returns:
 *    String with tcpinfo values, or NULL if no tcpinfo could be retrieved.
 */

int
socketoptdup(int s, int new_s);
/*
 * Duplicates the settings of "s" onto "new_s".  If "new_s" is -1,
 * a new socket is created for it.
 *
 * Returns:
 *      On success: new_s (or the descriptor for the new socket if new_s is -1).
 *      On failure: -1
 */

void
socketoptioncheck(const socketoption_t *option);
/*
 * Check socketoption arguments against sockopt_t entry.
 */

int
addedsocketoption(size_t *optc, socketoption_t **optv,
                  const socketoption_t *newoption);
/*
 * Adds the socketoption "newoption" to the list of current options
 * in the socketoption array "optv".
 *
 * Returns true on success. false on failure.
 */

void setconfsockoptions(const int target, const int in, const int protocol,
                        const int isclientside,
                        const size_t optc, const socketoption_t *optv,
                        const int whichlocals, const int whichglobals);
/*
 * Sets the options in "optv" on the socket "target", presumably loaded from
 * the sockd.conf.  "target" should be a socket of the type indicated by
 * protocol (SOCKS_TCP or SOCKS_UDP).
 *
 * If "in" is not -1, it indicates the socket the socket a connection from
 * a client came in from, and perhaps the reason "target" was created.
 * This is used in some special cases where we need to copy some special
 * options from the client connection (e.g., hostids).
 *
 * "isclientside" indicates whether "s" is a socket for the internal (client)
 * or external interface.
 *
 * "whichglobals" indicates what global (not rule/route-specific) options
 * configured should be checked at this time, and "whichlocals" the
 * same for the options in optv.
 */

int
socks_mklock(const char *template, char *newname, const size_t newnamelen);
/*
 * Creates a file that can be used with socks_lock() and
 * socks_unlock().  Returns the file descriptor of the created file.
 * If "newname" or "newnamelen" is zero, the created file is unlinked.
 * Otherwise the file is not unlinked and the name of the created file is
 * is saved to newname.
 *
 * Returns:
 *      On success: file descriptor
 *      On failure: -1
 */

int
socks_lock(const int fd, const off_t offset, const off_t len,
           const int exclusive, const int wait);
/*
 * Looks the file descriptor "fd" at offset "offset", length "len".
 * If "exclusive" is true, the lock is exclusive.  If not, it is shared.
 * If "wait" is true, wait for the lock.  If not, return if the lock
 * can not be taken.
 *
 * Upgrade/downgrade to/from exclusive is permitted.
 *
 * Returns:
 *      On success: 0
 *      On error  : -1
 */

void
socks_unlock(int d, const off_t offset, const off_t len);
/*
 * Unlocks the file descriptor "d", previously locked by this process,
 * at offset "offset", length "len".
 */

int
bitcount(unsigned long number);
/*
 * Returns the number of bits set in "number".
 */

int
bitcount_in6addr(const struct in6_addr *in6addr);
/*
 * Returns the number of bits set in "in6addr".
 */


#if SOCKSLIBRARY_DYNAMIC
/*
 * Here because they may be indirectly used by the server too, when it
 * executes external library code (e.g., libwrap).
 */

struct hostent *sys_gethostbyaddr(const char *addr, socklen_t len, int af);
struct hostent *sys_gethostbyname(const char *);
struct hostent *sys_gethostbyname2(const char *, int);

#if HAVE_GETADDRINFO
int sys_getaddrinfo(const char *nodename, const char *servname,
                    const struct addrinfo *hints, struct addrinfo **res);
#endif /* HAVE_GETADDRINFO */

#if HAVE_GETIPNODEBYNAME
struct hostent *sys_getipnodebyname(const char *name, int af, int flags,
                                    int *error_num);
#endif /* HAVE_GETIPNODEBYNAME */

#if SOCKS_CLIENT
#if HAVE___FPRINTF_CHK
HAVE_PROT_FPRINTF_0 __fprintf_chk(HAVE_PROT_FPRINTF_1 stream, int dummy,
              HAVE_PROT_FPRINTF_2 format, ...);
#endif /* HAVE___FPRINTF_CHK */

#if HAVE___VFPRINTF_CHK
HAVE_PROT_VFPRINTF_0 __vfprintf_chk(HAVE_PROT_VFPRINTF_1 stream,
      int dummy, HAVE_PROT_VFPRINTF_2 format, HAVE_PROT_VFPRINTF_3 ap);
#endif /* HAVE___VFPRINTF_CHK */
#endif /* SOCKS_CLIENT */

#if HAVE___READ_CHK
HAVE_PROT__READ_CHK_0
__read_chk(HAVE_PROT__READ_CHK_1 d, HAVE_PROT__READ_CHK_2 buf,
           HAVE_PROT__READ_CHK_3 nbytes, HAVE_PROT__READ_CHK_4 buflen);
#endif /* HAVE___READ_CHK */

#endif /* SOCKSLIBRARY_DYNAMIC */

int
httpproxy_negotiate(int control, socks_t *packet,
                    char *emsg, const size_t emsglen);
/*
 * Negotiates a session to be used with the server connected to "control".
 * "packet" is the packet with information about what we want the
 * server to do for us.
 * packet->res.reply will be set according to the result of negotiation.
 * Returns:
 *      On success: 0 (server accepted our request).
 *      On failure: -1.  "emsg" will contain the details.
 */

int
upnp_negotiate(const int s, socks_t *packet, gateway_t *gw,
               char *emsg, const size_t emsglen);
/*
 * Negotiates a session to be used with the upnp server.
 * If the request is for a i/o operation, socket is the socket to be used
 * for performing the i/o.
 *
 * "packet" is the packet with information about what we want the
 * server to do for us.
 *
 * "gw" is the upnp gateway to used.
 *
 * packet->res.reply will be set according to the result of negotiation.
 *
 * Returns:
 *      On success: 0 (server accepted our request).
 *                  Note that we do not need to contact the UPNP router
 *                  for all requests.  If we do not need to contact it for
 *                  the given request, we will just pretend everything is ok.
 *
 *      On failure: -1.  "emsg" will contain the details.
 */

int
socks_negotiate(int s, int control, socks_t *packet, route_t *route,
                char *emsg, const size_t emsglen);
/*
 * "s" is the socket data will flow over.
 * "control" is the control connection to the socks server.
 * "packet" is a socks packet containing the request.
 *   "route" is the connected route.
 * Negotiates method and fills the response to the request into packet->res.
 *
 * Returns:
 *      On success: 0 (server replied to our request).
 *      On failure: -1.  Reason is stored in "emsg" and errno is set.
 */

int
serverreplyisok(const unsigned int version, const unsigned int command,
                const unsigned int reply, route_t *route,
                char *emsg, const size_t emsglen);
/*
 * "replycode" is the reply code returned by a socks server of version
 * "version".
 * "route" is the route that was used for the socks server.
 * If the error code indicates a server failure, the route might be
 * "blacklisted".
 *
 *  On success: true.
 *  On failure: false.  Reason is stored in "emsg" and errno is set.
 */

route_t *
socks_nbconnectroute(int s, int control, socks_t *packet,
                     const sockshost_t *src, const sockshost_t *dst,
                     char *emsg, const size_t emsglen);
/*
 * The non-blocking version of socks_connectroute(), only used by client.
 * Takes one additional argument, "s", which is the socket to connect
 * and not necessarily the same as "control" (msproxy case).
 */

void
socks_blacklist(route_t *route, const char *reason);
/*
 * Marks route "route" as bad.
 * "reason" is a short reason describing why we are blacklisting this route.
 */

void
socks_clearblacklist(route_t *route);
/*
 * Clears bad marks on route.
 */

int
methodisvalid(const int method, objecttype_t ruletype);
/*
 * Returns true if "method" is a valid method for rules of type
 * "ruletype".
 */

int
negotiate_method(int s, socks_t *packet, route_t *route,
                 char *emsg, const size_t emsglen);
/*
 * Negotiates a method to be used when talking with the server connected
 * to "s".
 * "packet" is the packet that will later be sent to server, and only
 * the "auth" element in it will be set but other elements are needed
 * for reading too.
 * "route" is the route selected for connecting to the socks-server.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1.  "emsg" will contain the details.
 */

int
clientmethod_uname(int s, const sockshost_t *host, int version,
                   unsigned char *name, unsigned char *password,
                   char *emsg, size_t emsglen);
/*
 * Enters username/password negotiation with the socks server connected to
 * the socket "s".
 * "host" gives the name of the server.
 * "version" gives the socks version established to use.
 * "name", if not NULL, gives the name to use for authenticating.
 * "password", if not NULL, gives the name to use for authenticating.
 * Returns:
 *      On success: 0
 *      On failure: -1.  "emsg" will contain the details.
 */

#if HAVE_GSSAPI
int
clientmethod_gssapi(int s, int protocol, const gateway_t *gw,
                    int version, authmethod_t *auth,
                    char *emsg, const size_t emsglen);
/*
 * Enters gssapi negotiation with the socks server connected to
 * the socket "s".
 * "gw" gives the name of the gateway.
 * "version" gives the socks version established to use.
 * "*auth", authentication structure
 * Returns:
 *      On success: 0
 *      On failure: -1.  "emsg" will contain the details.
 */

int
gssapi_encode(const gss_buffer_t in, gssapi_state_t *gs, gss_buffer_t out);
/*
 * gssapi encodes the data in "in", storing the encoded message
 * in "out", which contains a pointer to the previously allocated
 * memory of the specified length.
 *
 * "gs" contains details about gssapi context.
 *
 * Returns:
 *    On success: 0
 *    On failure: -1
 */

int
gssapi_decode(const gss_buffer_t in, gssapi_state_t *gs, gss_buffer_t out);
/*
 * gssapi decodes the data in "in", storing the decoded message
 * in "out", which contains a pointer to the previously allocated
 * memory of the specified length.
 *
 * "gs" contains details about gssapi context.
 *
 * Returns:
 *    On success: 0
 *    On failure: -1
 */

#endif /* HAVE_GSSAPI */


int socks_yyparse(void);
int socks_yylex(void);

int
socks_sendrequest(int s, const request_t *request,
                  char *emsg, const size_t emsglen);
/*
 * Sends the request "request" to the socks server connected to "s".
 * Returns:
 *      On success: 0
 *      On failure: -1.  Reason is stored in "emsg" and errno is set.
 */

int
socks_recvresponse(int s, response_t *response, int version,
                   char *emsg, const size_t emsglen);
/*
 * Receives a socks response from the "s".  "response" is filled in with
 * the data received.
 * "version" is the protocol version negotiated.
 *
 * Returns:
 *      On success: 0
 *      On failure: -1.  Reason is stored in "emsg" and errno is set.
 */

iobuffer_t *
socks_allocbuffer(const int s, const int type);
/*
 * Returns the iobuffer allocated to file descriptor "s", or
 * a new free one if none is allocated.
 * "type" gives the type of socket "s" is, SOCK_STREAM or SOCK_DGRAM.
 *
 * It is an error if a new buffer is allocated to "s" before the old
 * one has been freed.
 */

void
socks_initbuffer(const int fd, const int stype, iobuffer_t *iobuf);

iobuffer_t *
socks_getbuffer(const int s);
/*
 * Returns the iobuffer allocated to file descriptor "s".
 */

void
socks_freebuffer(const int s);
/*
 * Marks the iobuffer allocated to file descriptor "s" as free.
 * It is not an error if no iobuffer is currently allocate dto "s".
 */

void
socks_reallocbuffer(const int old, const int new);
/*
 * Reallocs the buffer assigned to "old", if any, to instead be assigned
 * to "new".
 */

void
socks_clearbuffer(const int s, const whichbuf_t type);
/*
 * Clears the iobuffer belonging to "s".
 * "type" gives the buffer-type that should be cleared, READ_BUF or WRITE_BUF.
 */

int socks_flushbuffer(const int s, const ssize_t len,
                      sendto_info_t *sendtoflags);
/*
 * Tries to flush the data buffered for file descriptor "s".  If "s" is -1,
 * data for all descriptors is flushed.
 *
 * If "len" is -1, tries to flush all data on that fd, otherwise only flushes
 * up to "len" bytes.
 *
 * If "sendtoflags" is not NULL, it is updated appropriately.
 *
 * Returns 0 if all data, if any was flushed.
 * Returns -1 otherwise.
 */

void
socks_setbuffer(iobuffer_t *iobuf, const int mode, ssize_t bufsize);

void
socks_setbufferfd(const int fd, const int mode, ssize_t bufsize);

/*
 * The above two functions perform the same operation, but one takes
 * a fd as the id to identify the iobuf and results in a no-op if no iobuf
 * is allocated to the fd, while the other takes the iobuf directly.
 *
 * Sets a flag in the iobuf belonging to "fd", indicating data should
 * not be be written before a flush is done, the buffer becomes full,
 * or "another good reason" is given, according to "mode".
 * "mode" can take the same values as the corresponding argument
 * to setvbuf(3).
 *
 * "bufsize" is the size of buffer to use.  "bufsize" for the read buffer
 * and "bufsize" for the writebuffer.  Can not be larger than SOCKD_BUFSIZE.
 * Use -1 for a default value (SOCKD_BUFSIZE).
 */

size_t socks_addtobuffer(const int s, const whichbuf_t which,
                         const int encoded, const void *data,
                         const size_t datalen)
       __ATTRIBUTE__((__BOUNDED__(__buffer__, 4, 5)));
/*
 * Adds "data", of length "datalen" to the buffer belonging to "s".
 * "which" must have one of the values WRITE_BUF or READ_BUF, to
 * indicate what part of the buffer to add the data to;
 * READ_BUF : data that has been read from the socket.
 * WRITE_BUF: data that should be written to the socket.
 *
 * Returns the number of bytes added.
 */

size_t
socks_getfrombuffer(const int s, const size_t flags, const whichbuf_t which,
                    const int encoded, void *data, size_t datalen)
                    __ATTRIBUTE__((__BOUNDED__(__buffer__, 5, 6)));

/*
 * Copies up to "datalen" bytes from the iobuf belonging to "s".
 *
 * Flags can be either 0 or MSG_PEEK.  If MSG_PEEK, the data read
 * from the buffer will not be removed.
 *
 * "which" must have one of the values WRITE_BUF or READ_BUF, to
 * indicate what part of the buffer to copy the data from.
 *
 * Returns the number of bytes copied.
 */

size_t
socks_bytesinbuffer(const int s, const whichbuf_t which, const int encoded);
/*
 * Returns the number of bytes currently in the iobuf belonging to "s".
 */

int
socks_bufferhasbytes(const int s, const whichbuf_t which);
/*
 * Returns true if any of the buffers (encoded or decoded) belonging
 * to "s" has data in it.
 * Intended to be faster than calling socks_bytesinbuffer() twice,
 * once for each buffer (encoded/decoded).
 */

size_t
socks_freeinbuffer(const int s, const whichbuf_t which);
/*
 * Returns the number of bytes free in the iobuf belonging to "s".
 */

size_t
socks_buffersize(const int s, const whichbuf_t which);
/*
 * Returns the total size of the buffer belonging to "s".
 */


#if HAVE_LIVEDEBUG

void
socks_flushrb(void);
/*
 * Flushes the ringbuffer to log(s).
 */

#endif /* HAVE_LIVEDEBUG */


fd_set *
allocate_maxsize_fdset(void);
/*
 * Allocate a fd_set big enough to hold the highest file descriptor
 * we could possibly use.
 * Returns a pointer to the allocated fd_set, or exits on failure.
 */

rlim_t
getmaxofiles(limittype_t type);
/*
 * Return max number of open files for process.
 * If type is softlimit, the current limit is returned.
 * If type is hardlimit, the absolute maximum value is returned.
 */

char *
socks_getusername(const sockshost_t *host, char *buf, size_t buflen)
      __ATTRIBUTE__((__BOUNDED__(__string__, 2, 3)));
/*
 * Tries to determine the username of the current user, to be used
 * when negotiating with the server "host".
 * The NUL-terminated username is written to "buf", which is of size
 * "buflen".
 * Returns:
 *      On success: pointer to "buf" with the username.
 *      On failure: NULL.
 */

char *
socks_getpassword(const sockshost_t *host, const char *user,
      char *buf, size_t buflen);
/*
 * Tries to determine the password of user "user", to be used
 * when negotiating with the server "host".
 * The NUL-terminated password is written to "buf", which is of length
 * "buflen"
 * Returns:
 *      On success: pointer to "buf" with the password.
 *      On failure: NULL.
 */

const char *
socks_getenv(const char *name, value_t value);
/*
 * Depending on how the program was ./configured and on what
 * platform it runs, getenv(3) may or may not be disabled for
 * some names, for security reasons.
 *
 * This wrapper will return NULL if getenv(3) is disabled,
 * otherwise it will return the result of getenv(3).
 *
 * In addition, if "value" is not "dontcare", the function will
 * also compare the value returned by getenv(3), if any, to
 * see it it matches the value described by "value".  If they don't
 * match, the function will return NULL.
 */

int
socks_msghaserrors(const char *prefix, const struct msghdr *msg);
/*
 * Checks if "msg", as received via recvmsg(2), was truncated or
 * had other detectable errors, and reports it if so.
 * If reporting, "prefix" should contain information about where
 * the message was received.
 *
 * Returns true if "msg" has errors, "false" if not.
 */

void seconds2days(unsigned long *seconds, unsigned long *days,
                  unsigned long *hours, unsigned long *minutes);
/*
 * Converts "seconds" to the corresponding number of days, hours, minutes,
 * and seconds.
 * Upon return, the days, hours, minutes, and seconds are stored in the
 * passed arguments.
 */

void
showconfig(const struct config *config);
/*
 * prints out config "config".
 */

void
sockopts_dump(void);
/*
 * list all known socket option information
 */

const sockopt_t *
optname2sockopt(char *optname);
/*
 * return pointer to the socket option with the given name or NULL on failure.
 */

const sockopt_t *
optval2sockopt(int level, int optval);
/*
 * return pointer to the socket option with the the name "optname"
 * at the socket level "level", or NULL if no such option is known at
 * the given socket level.
 */

const sockopt_t *
optid2sockopt(size_t optid);
/*
 * return a pointer to the sockopt_t entry identified by "optid".
 */

const sockoptvalsym_t *
optval2valsym(size_t optid, char *name);
/*
 * returns a pointer to the sockoptvalsym entry if "name" is a valid symbolic
 * name for the socketoption indicated by "optid", or NULL if no matching
 * entry is found.
 */

#if HAVE_SOCKS_HOSTID

unsigned char
getsockethostid(const int s, struct hostid *hostid);
/*
 * Gets the hostids set on socket "s" and stores them in "hostid".
 *
 * Returns the number of hostids set on socket "s".
 * If none are set, 0 is returned.
 */

int
setsockethostid(const int s, const struct hostid *hostid);
/*
 * Sets the hostids in "hostidv", which contains "hostidc" hostids, 
 * on socket "s".
 *
 * Returns 0 on success, -1 on failure.
 */

const struct in_addr *
gethostidip(const struct hostid *hostid, const size_t index);
/*
 * Returns the ip-address at index "index" "hostid"'s addrv[] array.
 *
 * It is considered a fatal error (assert()) if there is no such index.
 */

size_t
gethostidipv(const struct hostid *hostid, struct in_addr *addrv, size_t addrc);
/*
 * Copies the IP-address parts of the "addrv" array in "hostid" to
 * "addrv", which must be large enough to contain "addrc" elements.
 * 
 * Returns the number of elements copied to "addrc".
 */


#if HAVE_TCP_EXP1

u_int8_t tcp_exp1_len(const int exidlen);
/*
 * Returns the len length of a TCP_EXP1 object where the length of the 
 * "ExID" field is "exidlen" bytes long.
 */

#endif /* HAVE_TCP_EXP1 */

#endif /* HAVE_SOCKS_HOSTID */


#if COVENANT
char *socks_decode_base64(char *in, char *out, size_t outlen);
char *socks_strcasestr(const char *a, const char *b);
#endif /* COVENANT */

#if HAVE_GETNAMEINFO && HAVE_PRELOAD

HAVE_PROT_GETNAMEINFO_0
sys_getnameinfo(HAVE_PROT_GETNAMEINFO_1 sa, HAVE_PROT_GETNAMEINFO_2 salen,
                HAVE_PROT_GETNAMEINFO_3 host, HAVE_PROT_GETNAMEINFO_4 hostlen,
                HAVE_PROT_GETNAMEINFO_5 serv, HAVE_PROT_GETNAMEINFO_6 servlen,
                HAVE_PROT_GETNAMEINFO_7 flags);

#endif /* HAVE_GETNAMEINFO && HAVE_PRELOAD */

#if SOCKS_CLIENT

#include "socks.h"

#else /* SOCKS_SERVER */

#include "sockd.h"

#endif /* SOCKS_SERVER */

#include "interposition.h"
#include "tostring.h"
#include "fmt.h"

#if HAVE_GSSAPI
#include "socks_gssapi.h"
#endif /* HAVE_GSSAPI */

#if HAVE_KRB5

#if !SOCKS_CLIENT

#include "socks_krb5.h"

#endif /* !SOCKS_CLIENT */

#endif /* HAVE_KRB5 */

void
slogstack(void);
/*
 * Prints the current stack.
 */

int
socks_getifaddrs(struct ifaddrs **ifap);
/*
 * Wrapper around getifaddrs(3) that does some extra work that should
 * not cause any problems.
 */


int
socks_inet_pton(const int af, const void *src, void *dst, uint32_t *dstscope);
/*
 * Like inet_pton(3), but calls getaddrinfo(3) to get the address instead
 * if "src" contains something that looks like a scope id (%id).  If so,
 * the scopeid is stored in "dstscope", provided "dstscope" is not NULL.
 *
 * Returns the same values as inet_pton().
 */

void
set_hints_ai_family(int *ai_family);
/*
 * Sets the "ai_family" member of a strut addrinfo "hints" object
 * appropriately for our use, according to whether we have usable
 * IPv6 addresses configured or not.
 */

int
makedummyfd(const sa_family_t safamily, const int socktype);
/*
 * Creates a dummy socket of the appropriate type, or the easiest/fastest
 * type if both safamily and socktype are zero.
 */


/*
 * Makes a dummy filedescriptor and returns it's index, or -1 on failure.
 */

#if DEBUG

void
printsocketopts(const int s);
/*
 * prints socket options and other flags set on the socket "s".
 */

int
fd_isset(int fd, fd_set *fdset);
/* function version of FD_ISSET() */

#endif /* DEBUG */

#endif /* !_COMMON_H_ */
