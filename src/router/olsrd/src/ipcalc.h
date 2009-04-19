
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2007, Bernd Petrovitsch <berndæfirmix.at>
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

#ifndef _IPCALC
#define _IPCALC

#include "olsr_types.h"
#include "defs.h"

#include <limits.h>
#include <arpa/inet.h>

struct ipaddr_str {
  char buf[MAX(INET6_ADDRSTRLEN, INET_ADDRSTRLEN)];
} __attribute__ ((unused));

/*
 * Macros for comparing and copying IP addresses
 */
static INLINE int
ip4cmp(const struct in_addr *a, const struct in_addr *b)
{
  return a->s_addr > b->s_addr ? +1 : a->s_addr < b->s_addr ? -1 : 0;
}
static INLINE int
ip4equal(const struct in_addr *a, const struct in_addr *b)
{
  return a->s_addr == b->s_addr;
}

static INLINE int
ip6cmp(const struct in6_addr *a, const struct in6_addr *b)
{
  return memcmp(a, b, sizeof(*a));
}
static INLINE int
ip6equal(const struct in6_addr *a, const struct in6_addr *b)
{
  return ip6cmp(a, b) == 0;
}

#if 0
static INLINE int
ipcmp(const union olsr_ip_addr *a, const union olsr_ip_addr *b)
{
  return olsr_cnf->ip_version == AF_INET ? ip4cmp(&a->v4, &b->v4) : ip6cmp(&a->v6, &b->v6);
}
#endif
static INLINE int
ipequal(const union olsr_ip_addr *a, const union olsr_ip_addr *b)
{
  return olsr_cnf->ip_version == AF_INET ? ip4equal(&a->v4, &b->v4) : ip6equal(&a->v6, &b->v6);
}

/* Do not use this - this is as evil as the COPY_IP() macro was and only used in
 * source which also needs cleanups.
 */
static INLINE void
genipcopy(void *dst, const void *src)
{
  memcpy(dst, src, olsr_cnf->ipsize);
}

int ip_in_net(const union olsr_ip_addr *ipaddr, const struct olsr_ip_prefix *net);

int prefix_to_netmask(uint8_t *, int, uint8_t);

static INLINE int
olsr_prefix_to_netmask(union olsr_ip_addr *adr, uint8_t prefixlen)
{
  return prefix_to_netmask(adr->v6.s6_addr, olsr_cnf->ipsize, prefixlen);
}

uint8_t netmask_to_prefix(const uint8_t *, int);

static INLINE uint8_t
olsr_netmask_to_prefix(const union olsr_ip_addr *adr)
{
  return netmask_to_prefix(adr->v6.s6_addr, olsr_cnf->ipsize);
}

static INLINE uint8_t
olsr_netmask4_to_prefix(const uint32_t * a)
{
  return netmask_to_prefix((const uint8_t *)a, sizeof(*a));
}
static INLINE uint8_t
olsr_netmask6_to_prefix(const struct in6_addr *a)
{
  return netmask_to_prefix((const uint8_t *)a, sizeof(*a));
}

static INLINE const char *
ip4_to_string(struct ipaddr_str *const buf, const struct in_addr addr4)
{
  return inet_ntop(AF_INET, &addr4, buf->buf, sizeof(buf->buf));
}

static INLINE const char *
ip6_to_string(struct ipaddr_str *const buf, const struct in6_addr *const addr6)
{
  return inet_ntop(AF_INET6, addr6, buf->buf, sizeof(buf->buf));
}

static INLINE const char *
olsr_ip_to_string(struct ipaddr_str *const buf, const union olsr_ip_addr *addr)
{
  return inet_ntop(olsr_cnf->ip_version, addr, buf->buf, sizeof(buf->buf));
}

const char *olsr_ip_prefix_to_string(const struct olsr_ip_prefix *prefix);

static INLINE const char *
sockaddr4_to_string(struct ipaddr_str *const buf, const struct sockaddr *const addr)
{
  const struct sockaddr_in *const addr4 = (const struct sockaddr_in *)addr;
  return ip4_to_string(buf, addr4->sin_addr);
}

/* we need to handle one value specifically since shifting 32 bits of a 32 bit integer is the same as shifting 0 bits.
 * The result is in host-byte-order.
 */
static INLINE uint32_t
prefix_to_netmask4(uint8_t prefixlen)
{
  return prefixlen == 0 ? 0 : (~0U << (32 - prefixlen));
}

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
