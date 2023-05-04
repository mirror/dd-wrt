/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2010-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __SPP_NORMALIZE_H__
#define __SPP_NORMALIZE_H__

#ifdef NORMALIZER

// these control protocol specific normalizations

typedef enum {
    NORM_IP4             = 0x00000001, // core ip4 norms
  //NORM_IP4_ID          = 0x00000002, // tbd:  encrypt ip id
    NORM_IP4_DF          = 0x00000004, // clear df
    NORM_IP4_RF          = 0x00000008, // clear rf
    NORM_IP4_TTL         = 0x00000010, // ensure min ttl
    NORM_ICMP4           = 0x00000020, // core icmp4 norms
    NORM_IP6             = 0x00000040, // core ip6 norms
    NORM_IP6_TTL         = 0x00000080, // ensure min hop limit
    NORM_ICMP6           = 0x00000100, // core icmp6 norms
    NORM_TCP_BLOCK       = 0x00000200, // enable tcp norms (used for normalizer indexing)
    NORM_TCP_RSV         = 0x00000400, // clear reserved bits
    NORM_TCP_PAD         = 0x00000800, // clear option padding bytes
    NORM_TCP_REQ_URG     = 0x00001000, // clear URP if URG = 0
    NORM_TCP_REQ_PAY     = 0x00002000, // clear URP/URG on no payload
    NORM_TCP_REQ_URP     = 0x00004000, // clear URG if URP is not set
    NORM_TCP_ECN_PKT     = 0x00008000, // clear ece and cwr
    NORM_TCP_ECN_STR     = 0x00010000, // clear if not negotiated (stream)
    NORM_TCP_URP         = 0x00020000, // trim urp to dsize
    NORM_TCP_OPT         = 0x00040000, // nop over non-essential options
    NORM_TCP_IPS         = 0x00080000, // enable stream normalization/pre-ack flushing
    NORM_IP4_TOS         = 0x00100000, // clear tos/diff-serv
    NORM_IP4_TRIM        = 0x00200000, // enforce min frame
    NORM_TCP_TRIM_SYN    = 0x00400000, // strip data from syn
    NORM_TCP_TRIM_RST    = 0x00800000, // strip data from rst
    NORM_TCP_TRIM_WIN    = 0x01000000, // trim to window
    NORM_TCP_TRIM_MSS    = 0x02000000, // trim to mss
    NORM_ALL             = 0x07FFFFFF  // all normalizations on
} NormFlags;

// if this != 0, tcp normalizer should be enabled
#define NORM_TCP   NORM_TCP_BLOCK | \
                   NORM_TCP_RSV | \
                   NORM_TCP_PAD | \
                   NORM_TCP_REQ_URG | \
                   NORM_TCP_REQ_PAY | \
                   NORM_TCP_REQ_URP | \
                   NORM_TCP_ECN_PKT | \
                   NORM_TCP_ECN_STR | \
                   NORM_TCP_URP | \
                   NORM_TCP_OPT | \
                   NORM_TCP_IPS | \
                   NORM_TCP_TRIM_SYN | \
                   NORM_TCP_TRIM_RST | \
                   NORM_TCP_TRIM_WIN | \
                   NORM_TCP_TRIM_MSS

// this can be used to index norm stat trackers
// ensure this aligns with structures in perf-base, normalize, and snort_stream_tcp
typedef enum {
    NORM_MODE_OFF       = -1,
    NORM_MODE_ON        = 0,
    NORM_MODE_WOULDA    = 1,
    NORM_MODE_MAX       = 2
} NormMode;

struct _SnortConfig;

typedef uint64_t PegCount;

void SetupNormalizer(void);
NormMode Normalize_GetMode(const struct _SnortConfig*, NormFlags);
#endif

#endif

