
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

#ifndef _PROTOCOLS_OLSR_H
#define	_PROTOCOLS_OLSR_H

#include "olsr_types.h"
#include "olsr_cfg.h"

#include <string.h>

/* Default IPv6 multicast addresses */

#define OLSR_IPV6_MCAST_SITE_LOCAL "ff05::15"
#define OLSR_IPV6_MCAST_GLOBAL     "ff0e::1"

#define OLSR_HEADERSIZE (sizeof(uint16_t) + sizeof(uint16_t))

#define OLSR_MSGHDRSZ_IPV4 12
#define OLSR_MSGHDRSZ_IPV6 24

/*
 *Emission Intervals
 */

#define HELLO_INTERVAL        2
#define REFRESH_INTERVAL      2
#define TC_INTERVAL           5
#define MID_INTERVAL          TC_INTERVAL
#define HNA_INTERVAL          TC_INTERVAL

/* Emission Jitter */
#define HELLO_JITTER         25 /* percent */
#define HNA_JITTER           25 /* percent */
#define MID_JITTER           25 /* percent */
#define TC_JITTER            25 /* percent */

/*
 *Holding Time
 */

#define NEIGHB_HOLD_TIME      3 * REFRESH_INTERVAL
#define TOP_HOLD_TIME         3 * TC_INTERVAL
#define DUP_HOLD_TIME         30
#define MID_HOLD_TIME         3 * MID_INTERVAL
#define HNA_HOLD_TIME         3 * HNA_INTERVAL

/*
 *Message Types
 */

#define HELLO_MESSAGE         1
#define TC_MESSAGE            2
#define MID_MESSAGE           3
#define HNA_MESSAGE           4
#define MAX_MESSAGE           4

/*
 *Link Types
 */

#define UNSPEC_LINK           0
#define ASYM_LINK             1
#define SYM_LINK              2
#define LOST_LINK             3
#define HIDE_LINK             4
#define MAX_LINK              4

/*
 *Neighbor Types
 */

#define NOT_NEIGH             0
#define SYM_NEIGH             1
#define MPR_NEIGH             2
#define MAX_NEIGH             2

/*
 *Neighbor status
 */

#define NOT_SYM               0
#define SYM                   1

/*
 *Link Hysteresis
 */

#define HYST_THRESHOLD_HIGH   0.8
#define HYST_THRESHOLD_LOW    0.3
#define HYST_SCALING          0.5

/*
 *Willingness
 */

#define WILL_NEVER            0
#define WILL_LOW              1
#define WILL_DEFAULT          3
#define WILL_HIGH             6
#define WILL_ALWAYS           7

/*
 *Redundancy defaults
 */
#define TC_REDUNDANCY         2
#define MPR_COVERAGE          1

/*
 *Misc. Constants
 */
#define MAXJITTER             HELLO_INTERVAL / 4
#define MAX_TTL               0xff

/*
 *Sequence numbering
 */

/* Seqnos are 16 bit values */

#define MAXVALUE 0xFFFF

/* Macro for checking seqnos "wraparound" */
#define SEQNO_GREATER_THAN(s1, s2)                \
        (((s1 > s2) && (s1 - s2 <= (MAXVALUE/2))) \
     || ((s2 > s1) && (s2 - s1 > (MAXVALUE/2))))

/*
 * Macros for creating and extracting the neighbor
 * and link type information from 8bit link_code
 * data as passed in HELLO messages
 */

#define CREATE_LINK_CODE(status, link) (link | (status<<2))

#define EXTRACT_STATUS(link_code) ((link_code & 0xC)>>2)

#define EXTRACT_LINK(link_code) (link_code & 0x3)

/***********************************************
 *           OLSR packet definitions           *
 ***********************************************/

/*
 *The HELLO message
 */

/*
 *Hello info
 */
struct hellinfo {
  uint8_t link_code;
  uint8_t reserved;
  uint16_t size;
  uint32_t neigh_addr[1];              /* neighbor IP address(es) */
} __attribute__ ((packed));

struct hellomsg {
  uint16_t reserved;
  uint8_t htime;
  uint8_t willingness;
  struct hellinfo hell_info[1];
} __attribute__ ((packed));

/*
 *IPv6
 */

struct hellinfo6 {
  uint8_t link_code;
  uint8_t reserved;
  uint16_t size;
  struct in6_addr neigh_addr[1];       /* neighbor IP address(es) */
} __attribute__ ((packed));

struct hellomsg6 {
  uint16_t reserved;
  uint8_t htime;
  uint8_t willingness;
  struct hellinfo6 hell_info[1];
} __attribute__ ((packed));

/*
 * Topology Control packet
 */

struct neigh_info {
  uint32_t addr;
} __attribute__ ((packed));

struct olsr_tcmsg {
  uint16_t ansn;
  uint16_t reserved;
  struct neigh_info neigh[1];
} __attribute__ ((packed));

/*
 *IPv6
 */

struct neigh_info6 {
  struct in6_addr addr;
} __attribute__ ((packed));

struct olsr_tcmsg6 {
  uint16_t ansn;
  uint16_t reserved;
  struct neigh_info6 neigh[1];
} __attribute__ ((packed));

/*
 *Multiple Interface Declaration message
 */

/*
 * Defined as s struct for further expansion
 * For example: do we want to tell what type of interface
 * is associated whit each address?
 */
struct midaddr {
  uint32_t addr;
} __attribute__ ((packed));

struct midmsg {
  struct midaddr mid_addr[1];
} __attribute__ ((packed));

/*
 *IPv6
 */
struct midaddr6 {
  struct in6_addr addr;
} __attribute__ ((packed));

struct midmsg6 {
  struct midaddr6 mid_addr[1];
} __attribute__ ((packed));

/*
 * Host and Network Association message
 */
struct hnapair {
  uint32_t addr;
  uint32_t netmask;
} __attribute__ ((packed));

struct hnamsg {
  struct hnapair hna_net[1];
} __attribute__ ((packed));

/*
 *IPv6
 */

struct hnapair6 {
  struct in6_addr addr;
  struct in6_addr netmask;
} __attribute__ ((packed));

struct hnamsg6 {
  struct hnapair6 hna_net[1];
} __attribute__ ((packed));

/*
 * OLSR message (several can exist in one OLSR packet)
 */

struct olsrmsg {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  uint32_t originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  union {
    struct hellomsg hello;
    struct olsr_tcmsg tc;
    struct hnamsg hna;
    struct midmsg mid;
  } message;

} __attribute__ ((packed));

/*
 *IPv6
 */

struct olsrmsg6 {
  uint8_t olsr_msgtype;
  uint8_t olsr_vtime;
  uint16_t olsr_msgsize;
  struct in6_addr originator;
  uint8_t ttl;
  uint8_t hopcnt;
  uint16_t seqno;

  union {
    struct hellomsg6 hello;
    struct olsr_tcmsg6 tc;
    struct hnamsg6 hna;
    struct midmsg6 mid;
  } message;

} __attribute__ ((packed));

/*
 * Generic OLSR packet
 */

struct olsr {
  uint16_t olsr_packlen;               /* packet length */
  uint16_t olsr_seqno;
  struct olsrmsg olsr_msg[1];          /* variable messages */
} __attribute__ ((packed));

struct olsr6 {
  uint16_t olsr_packlen;               /* packet length */
  uint16_t olsr_seqno;
  struct olsrmsg6 olsr_msg[1];         /* variable messages */
} __attribute__ ((packed));

/* IPv4 <-> IPv6 compability */

union olsr_message {
  struct olsrmsg v4;
  struct olsrmsg6 v6;
} __attribute__ ((packed));

union olsr_packet {
  struct olsr v4;
  struct olsr6 v6;
} __attribute__ ((packed));

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
