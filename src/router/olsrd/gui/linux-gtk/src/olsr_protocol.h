
/*
 * OLSR ad-hoc routing table management protocol
 * Copyright (C) 2003 Andreas Tonnesen (andreto@ifi.uio.no)
 *
 * This file is part of the UniK OLSR daemon.
 *
 * The UniK OLSR daemon is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The UniK OLSR daemon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the UniK OLSR daemon; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 *Values and packet formats as proposed in RFC3626 and misc. values and
 *data structures used by the UniK olsr daemon.
 */

#ifndef _PROTOCOLS_OLSR_H
#define	_PROTOCOLS_OLSR_H

/* Port for OLSR to use */

#define OLSRPORT       698

/* Default IPv6 multicast address */

#define OLSR_IPV6_MULTICAST_ADDR "ff05::15"

/* types */
#include <sys/types.h>

#ifdef WIN32
typedef unsigned char olsr_u8_t;
typedef unsigned short olsr_u16_t;
typedef unsigned int olsr_u32_t;
typedef char olsr_8_t;
typedef short olsr_16_t;
typedef int olsr_32_t;
#else
typedef u_int8_t olsr_u8_t;
typedef u_int16_t olsr_u16_t;
typedef u_int32_t olsr_u32_t;
typedef int8_t olsr_8_t;
typedef int16_t olsr_16_t;
typedef int32_t olsr_32_t;
#endif

/* IPv6 address format in6_addr */
#include <netinet/in.h>

union olsr_ip_addr {
  olsr_u32_t v4;
  struct in6_addr v6;
};

/*
 *Emission Intervals
 */

#define HELLO_INTERVAL        2
#define HELLO_INTERVAL_NW     HELLO_INTERVAL * 2
#define REFRESH_INTERVAL      2
#define TC_INTERVAL           5
#define MID_INTERVAL          TC_INTERVAL
#define HNA_INTERVAL          TC_INTERVAL

/*
 *Holding Time
 */

#define NEIGHB_HOLD_TIME      3 * REFRESH_INTERVAL

/*extra: time to delete for non-wireless interfaces */
#define NEIGHB_HOLD_TIME_NW   NEIGHB_HOLD_TIME * 2
#define TOP_HOLD_TIME         3 * TC_INTERVAL
#define DUP_HOLD_TIME         30
#define MID_HOLD_TIME         3 * MID_INTERVAL
#define HNA_HOLD_TIME         3 * HNA_INTERVAL

/*
 * Scaling factor
 */

#define VTIME_SCALE_FACTOR    0.0625

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
#define MAX_LINK              3

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
 *Misc. Constants
 */

#define TC_REDUNDANCY         0
#define MPR_COVERAGE          1
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
  olsr_u8_t link_code;
  olsr_u8_t reserved;
  olsr_u16_t size;
  olsr_u32_t neigh_addr[1];            /* neighbor IP address(es) */
};

struct hellomsg {
  olsr_u16_t reserved;
  olsr_u8_t htime;
  olsr_u8_t willingness;
  struct hellinfo hell_info[1];
};

/*
 *IPv6
 */

struct hellinfo6 {
  olsr_u8_t link_code;
  olsr_u8_t reserved;
  olsr_u16_t size;
  struct in6_addr neigh_addr[1];       /* neighbor IP address(es) */
};

struct hellomsg6 {
  olsr_u16_t reserved;
  olsr_u8_t htime;
  olsr_u8_t willingness;
  struct hellinfo6 hell_info[1];
};

/*
 * Topology Control packet
 */

struct neigh_info {
  olsr_u32_t addr;
};

struct olsr_tcmsg {
  olsr_u16_t ansn;
  olsr_u16_t reserved;
  struct neigh_info neigh[1];
};

/*
 *IPv6
 */

struct neigh_info6 {
  struct in6_addr addr;
};

struct olsr_tcmsg6 {
  olsr_u16_t ansn;
  olsr_u16_t reserved;
  struct neigh_info6 neigh[1];
};

/*
 *Multiple Interface Declaration message
 */

/*
 * Defined as s struct for further expansion
 * For example: do we want to tell what type of interface
 * is associated whit each address?
 */
struct midaddr {
  olsr_u32_t addr;
};

struct midmsg {
  struct midaddr mid_addr[1];
};

/*
 *IPv6
 */
struct midaddr6 {
  struct in6_addr addr;
};

struct midmsg6 {
  struct midaddr6 mid_addr[1];
};

/*
 * Host and Network Association message
 */
struct hnapair {
  olsr_u32_t addr;
  olsr_u32_t netmask;
};

struct hnamsg {
  struct hnapair hna_net[1];
};

/*
 *IPv6
 */

struct hnapair6 {
  struct in6_addr addr;
  struct in6_addr netmask;
};

struct hnamsg6 {
  struct hnapair6 hna_net[1];
};

/*
 * OLSR message (several can exist in one OLSR packet)
 */

struct olsrmsg {
  olsr_u8_t olsr_msgtype;
  olsr_u8_t olsr_vtime;
  olsr_u16_t olsr_msgsize;
  olsr_u32_t originator;
  olsr_u8_t ttl;
  olsr_u8_t hopcnt;
  olsr_u16_t seqno;

  union {
    struct hellomsg hello;
    struct olsr_tcmsg tc;
    struct hnamsg hna;
    struct midmsg mid;
  } message;

};

/*
 *IPv6
 */

struct olsrmsg6 {
  olsr_u8_t olsr_msgtype;
  olsr_u8_t olsr_vtime;
  olsr_u16_t olsr_msgsize;
  struct in6_addr originator;
  olsr_u8_t ttl;
  olsr_u8_t hopcnt;
  olsr_u16_t seqno;

  union {
    struct hellomsg6 hello;
    struct olsr_tcmsg6 tc;
    struct hnamsg6 hna;
    struct midmsg6 mid;
  } message;

};

/*
 * Generic OLSR packet
 */

struct olsr {
  olsr_u16_t olsr_packlen;             /* packet length */
  olsr_u16_t olsr_seqno;
  struct olsrmsg olsr_msg[1];          /* variable messages */
};

struct olsr6 {
  olsr_u16_t olsr_packlen;             /* packet length */
  olsr_u16_t olsr_seqno;
  struct olsrmsg6 olsr_msg[1];         /* variable messages */
};

/* IPv4 <-> IPv6 compability */

union olsr_message {
  struct olsrmsg v4;
  struct olsrmsg6 v6;
};

union olsr_packet {
  struct olsr v4;
  struct olsr6 v6;
};

#endif

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
