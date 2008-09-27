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

#include "ipcalc.h"

int
prefix_to_netmask(olsr_u8_t *a, int len, olsr_u8_t prefixlen)
{
#if !defined(NODEBUG) && defined(DEBUG)
  struct ipaddr_str buf;
  const olsr_u8_t *a_start = a;
#endif
  int p;
  const olsr_u8_t *a_end;

  a_end = a+len;
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

olsr_u8_t
netmask_to_prefix(const olsr_u8_t *adr, int len)
{
  struct ipaddr_str buf;
  const olsr_u8_t * const a_end = adr+len;
  olsr_u16_t prefix = 0;
  const olsr_u8_t *a;
  for (a = adr; a < a_end && *a == 0xff; a++) {
    prefix += 8;
  }
  if (a < a_end) {
    /* handle the last byte */
    switch (*a) {
    case   0: prefix += 0; break;
    case 128: prefix += 1; break;
    case 192: prefix += 2; break;
    case 224: prefix += 3; break;
    case 240: prefix += 4; break;
    case 248: prefix += 5; break;
    case 252: prefix += 6; break;
    case 254: prefix += 7; break;
    case 255: prefix += 8; break; /* Shouldn't happen */
    default:
      OLSR_PRINTF(0, "%s: Got bogus netmask %s\n", __func__, olsr_ip_to_string(&buf, (const union olsr_ip_addr *)adr));
      prefix = UCHAR_MAX;
      *(int *)0 = 0;
      break;
    }
  }
#ifdef DEBUG
  OLSR_PRINTF(3, "Netmask: %s = Prefix %d\n", olsr_ip_to_string(&buf, (const union olsr_ip_addr *)adr), prefix);
#endif
  return prefix;
}

const char *
olsr_ip_prefix_to_string(const struct olsr_ip_prefix *prefix)
{
  /* We need for IPv6 an IP address + '/' + prefix and for IPv4 an IP address + '/' + a netmask */
  static char buf[MAX(INET6_ADDRSTRLEN + 1 + 3, INET_ADDRSTRLEN + 1 + INET_ADDRSTRLEN)];
  const char *rv;

  if(olsr_cnf->ip_version == AF_INET) {
    /* IPv4 */
    int len;
    union olsr_ip_addr netmask;
    rv = inet_ntop(AF_INET, &prefix->prefix.v4, buf, sizeof(buf));
    len = strlen(buf);
    buf[len++] = '/';
    olsr_prefix_to_netmask(&netmask, prefix->prefix_len);
    inet_ntop(AF_INET, &netmask.v4, buf+len, sizeof(buf)-len);
  } else {
    /* IPv6 */
    int len;
    rv = inet_ntop(AF_INET6, &prefix->prefix.v6, buf, sizeof(buf));
    len = strlen(buf);
    buf[len++] = '/';
    snprintf(buf+len, sizeof(buf)-len, "/%d", prefix->prefix_len);
  }
  return rv;
}


/* see if the ipaddr is in the net. That is equivalent to the fact that the net part
 * of both are equal. So we must compare the first <prefixlen> bits.
 */
int ip_in_net(const union olsr_ip_addr *ipaddr, const struct olsr_ip_prefix *net)
{
  int rv;
  if(olsr_cnf->ip_version == AF_INET) {
    olsr_u32_t netmask = ~0 << (32 - net->prefix_len);
    rv = (ipaddr->v4.s_addr & netmask) == (net->prefix.v4.s_addr & netmask);
  } else {
    /* IPv6 */
    olsr_u32_t netmask;
    const olsr_u32_t *i = (const olsr_u32_t *)&ipaddr->v6;
    const olsr_u32_t *n = (const olsr_u32_t *)&net->prefix.v6;
    unsigned int prefix_len;
    for (prefix_len = net->prefix_len; prefix_len > 32; prefix_len -= 32) {
      if (*i != *n) {
        return OLSR_FALSE;
      }
      i++;
      n++;
    }
    netmask = ~0 << (32 - prefix_len);
    rv = (*i & netmask) == (*n & netmask);
  }
  return rv;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
