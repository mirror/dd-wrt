
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

#include "defs.h"
#include "ipcalc.h"

/* ipv4 prefix 0.0.0.0/0 */
const struct olsr_ip_prefix ipv4_internet_route =
{
    .prefix.v4.s_addr = 0,
    .prefix_len = 0
};

/* ipv6 prefix ::ffff:0:0/96 */
const struct olsr_ip_prefix ipv6_mappedv4_route =
{
    .prefix.v6.s6_addr = { 0,0,0,0,0,0,0,0,0,0,0xff,0xff,0,0,0,0 },
    .prefix_len = 96
};

/* ipv6 prefix 2000::/3 */
const struct olsr_ip_prefix ipv6_internet_route =
{
    .prefix.v6.s6_addr = { 0x20, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
    .prefix_len = 3
};

/* ip address zero */
const union olsr_ip_addr olsr_ip_zero =
{
    .v6.s6_addr = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
};

/* Default IPv6 multicast addresses FF02::6D(linklocal manet routers, see RFC 5498) */
const union olsr_ip_addr ipv6_def_multicast = {
    .v6.s6_addr = { 0xFF, 0x02, 0,0,0,0,0,0,0,0,0,0,0,0,0, 0x6D }
};

/* Host-byte-order! */
int
prefix_to_netmask(uint8_t * a, int len, uint8_t prefixlen)
{
#if !defined(NODEBUG) && defined(DEBUG)
  struct ipaddr_str buf;
  const uint8_t *a_start = a;
#endif
  int p;
  const uint8_t *a_end;

  a_end = a + len;
  for (p = prefixlen; a < a_end && p > 8; p -= 8) {
    *a++ = 0xff;
  }
  if (a >= a_end) {
    return 0;
  }
  *a++ = 0xff << (8 - p);
  while (a < a_end) {
    *a++ = 0;
  }

#ifdef DEBUG
  OLSR_PRINTF(3, "Prefix %d = Netmask: %s\n", prefixlen, inet_ntop(olsr_cnf->ip_version, a_start, buf.buf, sizeof(buf.buf)));
#endif
  return 1;
}

uint8_t
netmask_to_prefix(const uint8_t * adr, int len)
{
  struct ipaddr_str buf;
  const uint8_t *const a_end = adr + len;
  uint16_t prefix = 0;
  const uint8_t *a;
  for (a = adr; a < a_end && *a == 0xff; a++) {
    prefix += 8;
  }
  if (a < a_end) {
    /* handle the last byte */
    switch (*a) {
    case 0:
      prefix += 0;
      break;
    case 128:
      prefix += 1;
      break;
    case 192:
      prefix += 2;
      break;
    case 224:
      prefix += 3;
      break;
    case 240:
      prefix += 4;
      break;
    case 248:
      prefix += 5;
      break;
    case 252:
      prefix += 6;
      break;
    case 254:
      prefix += 7;
      break;
    case 255:
      prefix += 8;
      break;                    /* Shouldn't happen */
    default:
      OLSR_PRINTF(0, "%s: Got bogus netmask %s\n", __func__, olsr_ip_to_string(&buf, (const union olsr_ip_addr *)(const void *)adr));
      prefix = UCHAR_MAX;
      break;
    }
  }
#ifdef DEBUG
  OLSR_PRINTF(3, "Netmask: %s = Prefix %d\n", olsr_ip_to_string(&buf, (const union olsr_ip_addr *)(const void *)adr), prefix);
#endif
  return prefix;
}

const char *
olsr_ip_prefix_to_string(const struct olsr_ip_prefix *prefix)
{
  /* We need for IPv6 an IP address + '/' + prefix and for IPv4 an IP address + '/' + a netmask */
  static char buf[MAX(INET6_ADDRSTRLEN + 1 + 3, INET_ADDRSTRLEN + 1 + INET_ADDRSTRLEN)];
  const char *rv;

  if (olsr_cnf->ip_version == AF_INET) {
    /* IPv4 */
    int len;
    union olsr_ip_addr netmask;
    rv = inet_ntop(AF_INET, &prefix->prefix.v4, buf, sizeof(buf));
    len = strlen(buf);
    buf[len++] = '/';
    olsr_prefix_to_netmask(&netmask, prefix->prefix_len);
    inet_ntop(AF_INET, &netmask.v4, buf + len, sizeof(buf) - len);
  } else {
    /* IPv6 */
    int len;
    rv = inet_ntop(AF_INET6, &prefix->prefix.v6, buf, sizeof(buf));
    len = strlen(buf);
    buf[len++] = '/';
    snprintf(buf + len, sizeof(buf) - len, "/%d", prefix->prefix_len);
  }
  return rv;
}

int
olsr_string_to_prefix(int ipversion, struct olsr_ip_prefix *dst, const char *string) {
  static char buf[MAX(INET6_ADDRSTRLEN + 1 + 3, INET_ADDRSTRLEN + 1 + INET_ADDRSTRLEN)];
  char *ptr;

  strscpy(buf, string, sizeof(buf));
  dst->prefix_len = ipversion == AF_INET ? 32 : 128;

  ptr = strchr(buf, '/');
  if (!ptr) {
    ptr = strchr(buf, ' ');
  }

  if (ptr) {
    *ptr++ = 0;
    if (olsr_cnf->ip_version == AF_INET && strchr(ptr, '.')) {
      uint8_t subnetbuf[4];
      if (inet_pton(AF_INET, ptr, subnetbuf) != 1) {
        return -1;
      }

      dst->prefix_len = netmask_to_prefix(subnetbuf, sizeof(subnetbuf));
    }
    else {
      dst->prefix_len = atoi(ptr);
    }
  }
  return inet_pton(ipversion, buf, &dst->prefix) == 1 ? 0 : -1;
}

/* we need to handle one value specifically since shifting
 * 32 bits of a 32 bit integer is the same as shifting 0 bits.
 * The result is in host-byte-order.
 */
static INLINE uint32_t
prefix_to_netmask4(uint8_t prefixlen)
{
  return prefixlen == 0 ? 0 : (~0U << (32 - prefixlen));
}

/* see if the ipaddr is in the net. That is equivalent to the fact that the net part
 * of both are equal. So we must compare the first <prefixlen> bits. Network-byte-order!
 */
int
ip_in_net(const union olsr_ip_addr *ipaddr, const struct olsr_ip_prefix *net)
{
  int rv;
  if (olsr_cnf->ip_version == AF_INET) {
    uint32_t netmask = htonl(prefix_to_netmask4(net->prefix_len));
    rv = (ipaddr->v4.s_addr & netmask) == (net->prefix.v4.s_addr & netmask);
  } else {
    /* IPv6 */
    uint32_t netmask;
    const uint32_t *i = (const uint32_t *)&ipaddr->v6;
    const uint32_t *n = (const uint32_t *)&net->prefix.v6;
    unsigned int prefix_len;
    /* First we compare whole unsigned int's */
    for (prefix_len = net->prefix_len; prefix_len > 32; prefix_len -= 32) {
      if (*i != *n) {
        return false;
      }
      i++;
      n++;
    }
    /* And the remaining is the same as in the IPv4 case */
    netmask = htonl(prefix_to_netmask4(prefix_len));
    rv = (*i & netmask) == (*n & netmask);
  }
  return rv;
}

bool is_prefix_inetgw(const struct olsr_ip_prefix *prefix) {
  if (olsr_cnf->ip_version == AF_INET && ip_prefix_is_v4_inetgw(prefix)) {
    return true;
  }
  if (olsr_cnf->ip_version == AF_INET6) {
    if (ip_prefix_is_v6_inetgw(prefix) || ip_prefix_is_mappedv4_inetgw(prefix)) {
      return true;
    }
  }
  return false;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
