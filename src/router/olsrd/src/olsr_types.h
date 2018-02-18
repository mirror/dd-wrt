/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/*
 *Values and packet formats as proposed in RFC3626 and misc. values and
 *data structures used by the olsr.org OLSR daemon.
 */

#ifndef _OLSR_TYPES_H
#define	 _OLSR_TYPES_H

#if !defined __linux__ && !defined __APPLE__ && !defined _WIN32 && !defined __FreeBSD__ && !defined __FreeBSD_kernel__ && !defined __NetBSD__ && !defined __OpenBSD__
#       error "Unsupported system"
#endif /* !defined __linux__ && !defined __APPLE__ && !defined _WIN32 && !defined __FreeBSD__ && !defined __FreeBSD_kernel__ && !defined __NetBSD__ && !defined __OpenBSD__ */

/* types */
#ifdef _MSC_VER
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
#else /* _MSC_VER */
#include <inttypes.h>
#endif /* _MSC_VER */

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L

/* we have a C99 environment */
#include <stdbool.h>
#elif defined __GNUC__

/* we simulate a C99 environment */
#define bool _Bool
#define true 1
#define false 0
#define __bool_true_false_are_defined 1
#endif /* defined __GNUC__ */

/* add some safe-gaurds */
#ifndef _MSC_VER
#if !defined bool || !defined true || !defined false || !defined __bool_true_false_are_defined
#error You have no C99-like boolean types. Please extend src/olsr_type.h!
#endif /* !defined bool || !defined true || !defined false || !defined __bool_true_false_are_defined */
#endif /* _MSC_VER */

/* user defined cookies */
typedef uint16_t olsr_cookie_t;

#ifdef _WIN32
#include <winsock2.h>
#else /* _WIN32 */
/* manpage says: fd_set is in sys/select.h with posix (at least with the Android-NDK) */
#include <sys/select.h>
#endif /* _WIN32 */

/* OpenBSD wants this here */
#include <sys/types.h>
#include <sys/socket.h>

/* IPv6 address format in6_addr */
#ifndef _MSC_VER
#include <netinet/in.h>
#endif /* _MSC_VER */

union olsr_sockaddr {
  struct sockaddr_storage storage;
  struct sockaddr in;
  struct sockaddr_in in4;
  struct sockaddr_in6 in6;
};

union olsr_ip_addr {
  struct in_addr v4;
  struct in6_addr v6;
};

struct olsr_ip_prefix {
  union olsr_ip_addr prefix;
  uint8_t prefix_len;
};

typedef uint32_t olsr_linkcost;

#endif /* _OLSR_TYPES_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
