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

#ifndef _OLSR_LQ_PACKET_H
#define _OLSR_LQ_PACKET_H

#include "olsr_types.h"
#include "packet.h"
#include "mantissa.h"
#include "ipcalc.h"

#define LQ_HELLO_MESSAGE      201
#define LQ_TC_MESSAGE         202

/* deserialized OLSR header */

struct olsr_common {
  uint8_t type;
  olsr_reltime vtime;
  uint16_t size;
  union olsr_ip_addr orig;
  uint8_t ttl;
  uint8_t hops;
  uint16_t seqno;
};

/* serialized IPv4 OLSR header */

struct olsr_header_v4 {
  uint8_t type;
  uint8_t vtime;
  uint16_t size;
  uint32_t orig;
  uint8_t ttl;
  uint8_t hops;
  uint16_t seqno;
};

/* serialized IPv6 OLSR header */

struct olsr_header_v6 {
  uint8_t type;
  uint8_t vtime;
  uint16_t size;
  unsigned char orig[16];
  uint8_t ttl;
  uint8_t hops;
  uint16_t seqno;
};

/* deserialized LQ_HELLO */

struct lq_hello_neighbor {
  uint8_t link_type;
  uint8_t neigh_type;
  union olsr_ip_addr addr;
  struct lq_hello_neighbor *next;
  uint32_t linkquality[0];
};

struct lq_hello_message {
  struct olsr_common comm;
  olsr_reltime htime;
  uint8_t will;
  struct lq_hello_neighbor *neigh;
};

/* serialized LQ_HELLO */
struct lq_hello_info_header {
  uint8_t link_code;
  uint8_t reserved;
  uint16_t size;
};

struct lq_hello_header {
  uint16_t reserved;
  uint8_t htime;
  uint8_t will;
};

/* deserialized LQ_TC */
struct lq_tc_message {
  struct olsr_common comm;
  union olsr_ip_addr from;
  uint16_t ansn;
  struct tc_mpr_addr *neigh;
};

/* serialized LQ_TC */

struct lq_tc_header {
  uint16_t ansn;
  uint8_t lower_border;
  uint8_t upper_border;
};

static INLINE void
pkt_get_u8(const uint8_t ** p, uint8_t * var)
{
  *var = *(const uint8_t *)(*p);
  *p += sizeof(uint8_t);
}
static INLINE void
pkt_get_u16(const uint8_t ** p, uint16_t * var)
{
  *var = ntohs(**((const uint16_t **)p));
  *p += sizeof(uint16_t);
}
static INLINE void
pkt_get_u32(const uint8_t ** p, uint32_t * var)
{
  *var = ntohl(**((const uint32_t **)p));
  *p += sizeof(uint32_t);
}
static INLINE void
pkt_get_s8(const uint8_t ** p, int8_t * var)
{
  *var = *(const int8_t *)(*p);
  *p += sizeof(int8_t);
}
static INLINE void
pkt_get_s16(const uint8_t ** p, int16_t * var)
{
  *var = ntohs(**((const int16_t **)p));
  *p += sizeof(int16_t);
}
static INLINE void
pkt_get_s32(const uint8_t ** p, int32_t * var)
{
  *var = ntohl(**((const int32_t **)p));
  *p += sizeof(int32_t);
}
static INLINE void
pkt_get_reltime(const uint8_t ** p, olsr_reltime * var)
{
  *var = me_to_reltime(**p);
  *p += sizeof(uint8_t);
}
static INLINE void
pkt_get_ipaddress(const uint8_t ** p, union olsr_ip_addr *var)
{
  memcpy(var, *p, olsr_cnf->ipsize);
  *p += olsr_cnf->ipsize;
}
static INLINE void
pkt_get_prefixlen(const uint8_t ** p, uint8_t * var)
{
  *var = netmask_to_prefix(*p, olsr_cnf->ipsize);
  *p += olsr_cnf->ipsize;
}

static INLINE void
pkt_ignore_u8(const uint8_t ** p)
{
  *p += sizeof(uint8_t);
}
static INLINE void
pkt_ignore_u16(const uint8_t ** p)
{
  *p += sizeof(uint16_t);
}
static INLINE void
pkt_ignore_u32(const uint8_t ** p)
{
  *p += sizeof(uint32_t);
}
static INLINE void
pkt_ignore_s8(const uint8_t ** p)
{
  *p += sizeof(int8_t);
}
static INLINE void
pkt_ignore_s16(const uint8_t ** p)
{
  *p += sizeof(int16_t);
}
static INLINE void
pkt_ignore_s32(const uint8_t ** p)
{
  *p += sizeof(int32_t);
}
static INLINE void
pkt_ignore_ipaddress(const uint8_t ** p)
{
  *p += olsr_cnf->ipsize;
}
static INLINE void
pkt_ignore_prefixlen(const uint8_t ** p)
{
  *p += olsr_cnf->ipsize;
}

static INLINE void
pkt_put_u8(uint8_t ** p, uint8_t var)
{
  **((uint8_t **)p) = var;
  *p += sizeof(uint8_t);
}
static INLINE void
pkt_put_u16(uint8_t ** p, uint16_t var)
{
  **((uint16_t **)p) = htons(var);
  *p += sizeof(uint16_t);
}
static INLINE void
pkt_put_u32(uint8_t ** p, uint32_t var)
{
  **((uint32_t **)p) = htonl(var);
  *p += sizeof(uint32_t);
}
static INLINE void
pkt_put_s8(uint8_t ** p, int8_t var)
{
  **((int8_t **)p) = var;
  *p += sizeof(int8_t);
}
static INLINE void
pkt_put_s16(uint8_t ** p, int16_t var)
{
  **((int16_t **)p) = htons(var);
  *p += sizeof(int16_t);
}
static INLINE void
pkt_put_s32(uint8_t ** p, int32_t var)
{
  **((int32_t **)p) = htonl(var);
  *p += sizeof(int32_t);
}
static INLINE void
pkt_put_reltime(uint8_t ** p, olsr_reltime var)
{
  **p = reltime_to_me(var);
  *p += sizeof(uint8_t);
}
static INLINE void
pkt_put_ipaddress(uint8_t ** p, const union olsr_ip_addr *var)
{
  memcpy(*p, var, olsr_cnf->ipsize);
  *p += olsr_cnf->ipsize;
}

void olsr_output_lq_hello(void *para);

void olsr_output_lq_tc(void *para);

void olsr_input_lq_hello(union olsr_message *ser, struct interface_olsr *inif, union olsr_ip_addr *from);

extern bool lq_tc_pending;

#endif /* _OLSR_LQ_PACKET_H */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
