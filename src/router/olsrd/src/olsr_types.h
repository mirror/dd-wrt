/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
#define	_OLSR_TYPES_H

/* types */
#ifdef _MSC_VER
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef char            int8_t;
typedef short           int16_t;
typedef int             int32_t;
#else
#include <inttypes.h>
#endif


typedef enum {
    OLSR_FALSE = 0,
    OLSR_TRUE
} olsr_bool;

/* user defined cookies */
typedef uint16_t        olsr_cookie_t;

#if defined linux || defined __MacOSX__ || defined WIN32 || defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__

typedef uint8_t         olsr_u8_t;
typedef uint16_t        olsr_u16_t;
typedef uint32_t        olsr_u32_t;
typedef int8_t          olsr_8_t;
typedef int16_t         olsr_16_t;
typedef int32_t         olsr_32_t;

#else
#       error "Unsupported system"
#endif

/* OpenBSD wants this here */
#include <sys/types.h>
/* IPv6 address format in6_addr */
#ifndef _MSC_VER
#include <netinet/in.h>
#endif


union olsr_ip_addr {
  struct in_addr v4;
  struct in6_addr v6;
};

struct olsr_ip_prefix {
  union olsr_ip_addr prefix;
  olsr_u8_t prefix_len;
};

typedef olsr_u32_t olsr_linkcost;

#endif
