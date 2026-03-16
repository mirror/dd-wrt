/*
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
 *               2008, 2009, 2010, 2011, 2012, 2013, 2019, 2020, 2024
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

/* $Id: osdep.h,v 1.98.6.3.4.3.4.4 2024/11/20 22:06:31 karls Exp $ */

/*
 * ensure pam/libwrap defines are only set when compiling server
 */

#if HAVE_COND_PAM && !SOCKS_CLIENT
#define HAVE_PAM (1)
#else
#define HAVE_PAM (0)
#endif /* HAVE_COND_PAM && !SOCKS_CLIENT */

#if HAVE_COND_LIBWRAP && !SOCKS_CLIENT
#define HAVE_LIBWRAP (1)
#else
#define HAVE_LIBWRAP (0)
#endif /* HAVE_COND_LIBWRAP && !SOCKS_CLIENT */


#if HAVE_COND_LIVEDEBUG
#define HAVE_LIVEDEBUG (1)
#else
#define HAVE_LIVEDEBUG (0)
#endif /* HAVE_COND_LIVEDEBUG */


#include <sys/types.h>
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#if HAVE_SYS_FILE_H
#include <sys/file.h>
#endif /* HAVE_SYS_FILE_H */
#include <sys/resource.h>
#include <sys/ioctl.h>
#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif /* HAVE_SYS_IPC_H */
#if HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif /* HAVE_SYS_SEM_H */
#if (defined __sun) && (defined _NO_SUN_PRAGMA)
/* want to avoid connect/listen etc. being remapped to _xnet_foo */
#undef __PRAGMA_REDEFINE_EXTNAME
#endif /* __sun */
#include <sys/socket.h>
#if (defined __sun) && (defined _NO_SUN_PRAGMA)
/* avoid collision with config_parse.y keywords */
#undef bind
#undef connect
#endif /* __sun */
#include <net/if.h>
#if NEED_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* NEED_SYS_SOCKIO_H */
#include <sys/mman.h>
#include <sys/un.h>
#include <sys/stat.h>
#ifdef SOCKS_DLIB_OSF
#undef __DECC
#endif /* SOCKS_DLIB_OSF */
#include <sys/uio.h>
#ifdef SOCKS_DLIB_OSF
#define __DECC
#endif /* SOCKS_DLIB_OSF */
#include <sys/wait.h>
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */
#include <netinet/in.h>
#include <netinet/in_systm.h>
#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif /* HAVE_NETINET_IP_H */
#if HAVE_NETINET_IP_VAR_H
#include <netinet/ip_var.h>
#endif /* HAVE_NETINET_IP_VAR_H */
#include <arpa/inet.h>
#if HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif /* HAVE_ARPA_NAMESER_H */

#include <netinet/tcp.h>

#if HAVE_RPC_RPC_H
#include <rpc/rpc.h>
#endif /* HAVE_RPC_RPC_H */

#if !HAVE_TIMER_MACROS
#include "timers.h"
#endif /* !HAVE_TIMER_MACROS */

#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include <inttypes.h>

#include <assert.h>
#if HAVE_CRYPT_H
#include <crypt.h>
#endif /* HAVE_CRYPT_H */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#if HAVE_LIMITS_H
#include <limits.h>
#endif /* HAVE_LIMITS_H */
#include <netdb.h>
#include <pwd.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_STRINGS_H
#include <strings.h>
#endif /* HAVE_STRINGS_H */
#include <string.h>

#include <syslog.h>
#if HAVE_LIBWRAP && HAVE_TCPD_H
#include <tcpd.h>
#endif /* HAVE_LIBWRAP && HAVE_TCPD_H */
#if TIME_WITH_SYS_TIME || (!TIME_WITH_SYS_TIME && !HAVE_SYS_TIME_H)
#include <time.h>
#endif /* TIME_WITH_SYS_TIME || (!TIME_WITH_SYS_TIME && !HAVE_SYS_TIME_H) */
#if HAVE_UNISTD_H
#ifdef HAVE_DEC_PROTO
#undef _XOPEN_SOURCE_EXTENDED
#endif /* HAVE_DEC_PROTO */
#include <unistd.h>
#ifdef HAVE_DEC_PROTO
#define _XOPEN_SOURCE_EXTENDED 1
#endif /* HAVE_DEC_PROTO */
#endif /* HAVE_UNISTD_H */
#if HAVE_RESOLV_H
#include <resolv.h>
#endif /* HAVE_RESOLV_H */
#if HAVE_PAM
#include <security/pam_appl.h>
#endif /* HAVE_PAM */
#include <grp.h>

#if HAVE_GSSAPI

#if HAVE_GSSAPI_GSSAPI_KRB5_H
#include <gssapi/gssapi_krb5.h>
#elif HAVE_GSSAPI_GSSAPI_H
#include <gssapi/gssapi.h>
#elif HAVE_GSSAPI_H
#include <gssapi.h>
#endif /* HAVE_GSSAPI_GSSAPI_KRB5_H */

#if !HAVE_HEIMDAL_KERBEROS
#if HAVE_GSSAPI_GSSAPI_EXT_H
#include <gssapi/gssapi_ext.h>
#endif /* HAVE_GSSAPI_GSSAPI_EXT_H */
#if HAVE_GSSAPI_GSSAPI_GENERIC_H
#include <gssapi/gssapi_generic.h>
#endif /* HAVE_GSSAPI_GSSAPI_GENERIC_H */
#endif /* !HAVE_HEIMDAL_KERBEROS */
#endif /* HAVE_GSSAPI */

#if HAVE_KRB5_H
#include <krb5.h>
#elif HAVE_KERBEROSV5_KRB5_H
#include <kerberosv5/krb5.h>
#endif /* HAVE_KERBEROSV5_KRB5_H */

#if HAVE_STDDEF_H
#include <stddef.h>
#endif /* HAVE_STDDEF_H */

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif /* HAVE_PTHREAD_H */

#if HAVE_GETIFADDRS && HAVE_IFADDRS_H
#include <ifaddrs.h>
#else
#include "ifaddrs.h"
#endif /* HAVE_IFADDRS_H */

#if HAVE_VALGRIND_VALGRIND_H
#include <valgrind/valgrind.h>
#endif /* HAVE_VALGRIND_VALGRIND_H */

/*
 *  End of system headers
 */

/* XXX should have way to control use on client/server */
#if HAVE_LIBCFAIL && !SOCKS_CLIENT
#include "cfail.h"
#endif /* HAVE_LIBCFAIL && !SOCKS_CLIENT*/

#if HAVE_PRAGMA_SUPPORT
#define NOWARN_ADDRESS_PRAGMA() _Pragma("GCC diagnostic ignored \"-Waddress\"")
#else
#define NOWARN_ADDRESS_PRAGMA()
#endif /* HAVE_PRAGMA_SUPPORT */

#if HAVE_DECL_ATTRIBUTE && !defined lint
#define __ATTRIBUTE__(x) __attribute__(x)
#else
#define __ATTRIBUTE__(x)
#endif /* HAVE_DECL_ATTRIBUTE */

#if HAVE_DECL_CONSTRUCTOR
#define ATTRIBUTE_CONSTRUCTOR constructor
#else
#define ATTRIBUTE_CONSTRUCTOR
#endif /* HAVE_DECL_CONSTRUCTOR */

#if HAVE_DECL_NONNULL
#define __NONNULL__(x) __nonnull__(x)
#else
#define __NONNULL__(x)
#endif /* HAVE_DECL_NONNULL */

#if HAVE_DECL_FORMAT
#define FORMAT(x, y, z) format(x, y, z)
#else
#define FORMAT(x, y, z)
#endif /* HAVE_DECL_FORMAT */

#if HAVE_DECL_BOUNDED
#define __BOUNDED__(x, y, z) __bounded__(x, y, z)
#else
#define __BOUNDED__(x, y, z)
#endif /* HAVE_DECL_BOUNDED */

#if HAVE_LINUX_BUGS
#if (defined __bswap_16) && (!defined __bswap_32)
#undef ntohl
#undef ntohs
#undef htonl
#undef htons
#endif
#endif /* HAVE_LINUX_BUGS */

#ifndef RLIMIT_OFILE
#define RLIMIT_OFILE RLIMIT_NOFILE
#endif /* !RLIMIT_OFILE */

#ifndef WAIT_ANY
#define WAIT_ANY -1
#endif /* !WAIT_ANY */

/* libscompat replacement function prototypes */

#if !HAVE_SETPROCTITLE
void setproctitle(const char *fmt, ...)
   __ATTRIBUTE__((FORMAT(__printf__, 1, 2)));
void initsetproctitle(int, char **);
#endif /* !HAVE_SETPROCTITLE */

#if !HAVE_SOCKATMARK
int sockatmark(int);
#endif /* !HAVE_SOCKATMARK */

#if !HAVE_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t size);
size_t strlcat(char *dst, const char *src, size_t size);
#endif /* !HAVE_STRLCPY */

#if !HAVE_HSTRERROR
const char *hstrerror(int);
#endif /* !HAVE_HSTRERROR */

#if !HAVE_MEMMOVE
void * memmove(void *, const void *, register size_t);
#endif /* !HAVE_MEMMOVE */

#if !HAVE_INET_PTON
int inet_pton(int af, const char *src, void *dst);
#endif /* !HAVE_INET_PTON */

#if !HAVE_ISSETUGID
int issetugid(void);
#endif /* !HAVE_ISSETUGID */

#if !HAVE_PSELECT
inline int pselect(int nfds, fd_set *rset, fd_set *wset, fd_set *xset,
    const struct timespec *ts, const sigset_t *sigmask);
#endif /* !HAVE_ISSETUGID */

#if !HAVE_VSYSLOG
void vsyslog(int, const char *, va_list);
#endif /* !HAVE_VSYSLOG */

#if NEED_GETSOCKOPT_CAST
#define getsockopt(a,b,c,d,e) getsockopt((a),(b),(c),(char *)(d),(e))
#define setsockopt(a,b,c,d,e) setsockopt((a),(b),(c),(char *)(d),(e))
#endif /* NEED_GETSOCKOPT_CAST */

#if !HAVE_SIG_ATOMIC_T
typedef int sig_atomic_t;
#endif /* !HAVE_SIG_ATOMIC_T */

#define BYTEWIDTH_8BITS 1
#define BYTEWIDTH_16BITS 2
#define BYTEWIDTH_32BITS 4

#if SIZEOF_CHAR == BYTEWIDTH_8BITS
 typedef unsigned char ubits_8;
 typedef          char sbits_8;
#else
#error "no known 8 bits wide data type"
#endif /* SIZEOF_CHAR == BYTEWIDTH_8BITS */

#if SIZEOF_SHORT == BYTEWIDTH_16BITS
 typedef unsigned short ubits_16;
 typedef          short sbits_16;
#else
# if SIZEOF_INT == BYTEWIDTH_16BITS
  typedef unsigned int ubits_16;
  typedef          int sbits_16;
# else
#error "no known 16 bits wide data type"
# endif /* SIZEOF_INT == BYTEWIDTH_16BITS */
#endif

#if SIZEOF_INT == BYTEWIDTH_32BITS
 typedef unsigned int ubits_32;
 typedef          int sbits_32;
#else
# if SIZEOF_SHORT == BYTEWIDTH_32BITS
   typedef unsigned short ubits_32;
   typedef          short sbits_32;
# else
#  if SIZEOF_LONG == BYTEWIDTH_32BITS
    typedef unsigned long ubits_32;
    typedef          long sbits_32;
#  else
#error "no known 32 bits wide data type"
#  endif /* SIZEOF_LONG == BYTEWIDTH_32BITS */
# endif /* SIZEOF_SHORT == BYTEWIDTH_32BITS */
#endif /* SIZEOF_INT == BYTEWIDTH_32BITS */

#if !HAVE_INT8_T
#define int8_t sbits_8
#endif /* HAVE_INT8_T */

#if !HAVE_INT16_T
#define int16_t sbits_16
#endif /* HAVE_INT16_T */

#if !HAVE_INT32_T
#define int32_t sbits_32
#endif /* HAVE_INT32_T */

#if !HAVE_UINT8_T
#define uint8_t ubits_8
#endif /* HAVE_UINT8_T */

#if !HAVE_UINT16_T
#define uint16_t ubits_16
#endif /* HAVE_UINT16_T */

#if !HAVE_UINT32_T
#define uint32_t ubits_32
#endif /* HAVE_UINT32_T */

#if !HAVE_IN_PORT_T
#define in_port_t ubits_16
#endif /* HAVE_IN_PORT_T */

#if !HAVE_IN_ADDR_T
#define in_addr_t ubits_32
#endif /* HAVE_IN_ADDR_T */

#ifndef INADDR_NONE
#define INADDR_NONE (ubits_32)0xffffffff
#endif /* !INADDR_NONE */

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif /* !MAX */

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif /* !MIN */

#if NEED_EXIT_FAILURE
/* assumes EXIT_SUCCESS is undefined too */
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif /* NEED_EXIT_FAILURE */

#if NEED_SA_RESTART
#define SA_RESTART SV_INTERRUPT
#endif /* NEED_SA_RESTART */

#if NEED_AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif /* NEED_AF_LOCAL */

#if HAVE_NOMALLOC_REALLOC
#define realloc(p,s) (((p) == NULL) ? (malloc(s)) : (realloc((p),(s))))
#endif /* HAVE_NOMALLOC_REALLOC */

#if HAVE_NONULL_FREE
#define free(p)                                                                \
do {                                                                           \
   if ((p) != NULL)                                                            \
      free((p));                                                               \
} while (/* CONSTCOND */ 0)
#endif /* HAVE_NONULL_FREE */

#ifndef __CONCAT
#define __CONCAT(x,y) x ## y
#endif /* !__CONCAT */

#ifndef __CONCAT3
#define __CONCAT3(x,y,z) x ## y ## z
#endif /* !__CONCAT3 */

#ifndef ROUNDDOWN
#define ROUNDDOWN(x, size)  (((x) / (size) ) * (size))
#endif

#ifndef ROUNDUP
#define ROUNDUP(x, size)    ((((x) + (size) - 1) / (size)) * (size))
#endif

#define ROUNDFLOAT(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x) - 0.5))

#ifndef EPROTO
#define EPROTO EPROTOTYPE
#endif /* !EPROTO */

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN  (6)
#endif /* !ETHER_ADDR_LEN */

#if !HAVE_STRUCT_IPOPTS
#define   MAX_IPOPTLEN   40
struct ipoption {
   struct   in_addr ipopt_dst;
   sbits_8   ipopt_list[MAX_IPOPTLEN];
};
#endif /* !HAVE_STRUCT_IPOPTS */

#if !HAVE_FDMASK
typedef unsigned long fd_mask;
#endif /* !HAVE_FDMASK */

#if !HAVE_IN6_ADDR
/* from OpenBSD netinet6/in6.h */
struct in6_addr {
   union {
      u_int8_t   __u6_addr8[16];
      u_int16_t  __u6_addr16[8];
      u_int32_t  __u6_addr32[4];
   } __u6_addr;/* 128-bit IP6 address */
};
#define s6_addr   __u6_addr.__u6_addr8
#endif /* !HAVE_IN6_ADDR */


/*
 * hostid structure (defined here as linux/tcp.h cannot be included)
 */
#if HAVE_MAX_HOSTIDS

#if HAVE_LINUX_BBKERNEL_H

#include <linux/bbkernel.h>

#endif /* HAVE_LINUX_BBKERNEL_H */

#if SOCKS_HOSTID_VERSION == 1

struct tcp_ipa_raw {
   u_int32_t ipa_ipaddress[HAVE_MAX_HOSTIDS];
};

struct tcp_exp1_raw {
   ubits_8 data[MAX_TCP_OPTION_SPACE];
};

#else

#error "error: unsupported IPA_VERSION"

#endif /* SOCKS_HOSTID_VERSION */

#endif /* HAVE_MAX_HOSTIDS */

/* replace select() macros on Linux to avoid FORTIFY related bug */
#if HAVE_LINUX_BUGS
#undef __FD_ELT
#define __FD_ELT(d) ((d) / (__NFDBITS))
#endif /* HAVE_LINUX_BUGS */

/*
 * libscompat functions
 */

#if !HAVE_DAEMON
int daemon(int, int);
#endif /* !HAVE_DAEMON */

#if !HAVE_DIFFTIME
double difftime(const time_t time1, const time_t time0);
#endif /* !HAVE_DIFFTIME */

#include "in_cksum.h"
