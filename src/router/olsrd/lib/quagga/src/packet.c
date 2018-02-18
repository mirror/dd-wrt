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

/* -------------------------------------------------------------------------
 * File               : packet.c
 * Description        : zebra packet creation functions
 * ------------------------------------------------------------------------- */

#include "defs.h"
#include "olsr.h"

#include "common.h"
#include "packet.h"

unsigned char
*zpacket_route(uint16_t cmd, struct zroute *r)
{
  int count;
  uint8_t len;
  uint16_t size, safi;
  uint32_t ind, metric;
  unsigned char *cmdopt, *t;

  cmdopt = olsr_malloc(ZEBRA_MAX_PACKET_SIZ, "QUAGGA: New route packet");

  t = &cmdopt[2];
  switch (zebra.version) {
  case 0:
    *t++ = (unsigned char) cmd;
    break;
  case 1:
  case 2:
    *t++ = ZEBRA_HEADER_MARKER;
    *t++ = zebra.version;
    cmd = htons(cmd);
    memcpy(t, &cmd, sizeof cmd);
    t += sizeof cmd;
    break;
  default:
    olsr_exit("QUAGGA: Unsupported zebra packet version", EXIT_FAILURE);
    break;
  }
  *t++ = r->type;
  *t++ = r->flags;
  *t++ = r->message;
  switch (zebra.version) {
  case 0:
  case 1:
    break;
  case 2:
    safi = htons(r->safi);
    memcpy(t, &safi, sizeof safi);
    t += sizeof safi;
    break;
  default:
    olsr_exit("QUAGGA: Unsupported zebra packet version", EXIT_FAILURE);
    break;
  }
  *t++ = r->prefixlen;
  len = (r->prefixlen + 7) / 8;
  if (olsr_cnf->ip_version == AF_INET)
    memcpy(t, &r->prefix.v4.s_addr, len);
  else
    memcpy(t, r->prefix.v6.s6_addr, len);
  t = t + len;

  if (r->message & ZAPI_MESSAGE_NEXTHOP) {
    *t++ = r->nexthop_num + r->ifindex_num;

    for (count = 0; count < r->nexthop_num; count++) {
      if (olsr_cnf->ip_version == AF_INET) {
        *t++ = ZEBRA_NEXTHOP_IPV4;
        memcpy(t, &r->nexthop[count].v4.s_addr, sizeof r->nexthop[count].v4.s_addr);
        t += sizeof r->nexthop[count].v4.s_addr;
      } else {
        *t++ = ZEBRA_NEXTHOP_IPV6;
        memcpy(t, r->nexthop[count].v6.s6_addr, sizeof r->nexthop[count].v6.s6_addr);
        t += sizeof r->nexthop[count].v6.s6_addr;
      }
    }
    for (count = 0; count < r->ifindex_num; count++) {
      *t++ = ZEBRA_NEXTHOP_IFINDEX;
      ind = htonl(r->ifindex[count]);
      memcpy(t, &ind, sizeof ind);
      t += sizeof ind;
    }
  }
  if ((r->message & ZAPI_MESSAGE_DISTANCE) > 0)
    *t++ = r->distance;
  if ((r->message & ZAPI_MESSAGE_METRIC) > 0) {
    metric = htonl(r->metric);
    memcpy(t, &metric, sizeof metric);
    t += sizeof metric;
  }
  size = htons(t - cmdopt);
  memcpy(cmdopt, &size, sizeof size);

  return cmdopt;
}

unsigned char
*zpacket_redistribute (uint16_t cmd, unsigned char type)
{
  unsigned char *data, *pnt;
  uint16_t size;

  data = olsr_malloc(ZEBRA_MAX_PACKET_SIZ , "QUAGGA: New redistribute packet");

  pnt = &data[2];
  switch (zebra.version) {
  case 0:
    *pnt++ = (unsigned char) cmd;
    break;
  case 1:
  case 2:
    *pnt++ = ZEBRA_HEADER_MARKER;
    *pnt++ = zebra.version;
    cmd = htons(cmd);
    memcpy(pnt, &cmd, sizeof cmd);
    pnt += sizeof cmd;
    break;
  default:
    olsr_exit("QUAGGA: Unsupported zebra packet version", EXIT_FAILURE);
    break;
  }
  *pnt++ = type;
  size = htons(pnt - data);
  memcpy(data, &size, sizeof size);

  return data;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
