/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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

#ifndef __DNS_DEFS_H__
#define __DNS_DEFS_H__

#include <sys/types.h>

#define MAX_OPCODE     5
#define INVALID_OPCODE 3

#define MAX_RCODE 10

#define RCODE_NXDOMAIN 3

#define DNS_LENGTH_FLAGS 0xC0

#define PATTERN_A_REC      1
#define PATTERN_AAAA_REC  28
#define PATTERN_CNAME_REC  5
#define PATTERN_SRV_REC   33
#define PATTERN_TXT_REC   16
#define PATTERN_MX_REC    15
#define PATTERN_SOA_REC    6
#define PATTERN_NS_REC     2
#define PATTERN_PTR_REC   12
#define PATTERN_ANY_REC  255

#pragma pack(1)

typedef struct _DNS_HEADER {
    uint16_t id;
#if defined(SF_BIGENDIAN)
    uint8_t  QR:1,
              Opcode:4,
              AA:1,
              TC:1,
              RD:1;
    uint8_t  RA:1,
              Z:1,
              AD:1,
              CD:1,
              RCODE:4;
#else
    uint8_t  RD:1,
              TC:1,
              AA:1,
              Opcode:4,
              QR:1;
    uint8_t  RCODE:4,
              CD:1,
              AD:1,
              Z:1,
              RA:1;
#endif
    uint16_t QDCount;
    uint16_t ANCount;
    uint16_t NSCount;
    uint16_t ARCount;
} DNSHeader;

typedef struct _DNS_TCP_HEADER {
    uint16_t length;
} DNSTCPHeader;

typedef struct _DNS_LABEL {
    uint8_t len;
    uint8_t name;
} DNSLabel;

typedef struct _DNS_LABEL_POINTER {
    uint16_t position;
    uint8_t data;
} DNSLabelPtr;

typedef struct _DNS_LABEL_BITFIELD {
    uint8_t id;
    uint8_t len;
    uint8_t data;
} DNSLabelBitfield;

typedef struct _DNS_QUERY_FIXED {
    uint16_t QType;
    uint16_t QClass;
} DNSQueryFixed;

typedef struct _DNS_ANSWER_DATA {
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t r_len;
} DNSAnswerData;

#pragma pack()

#endif  /* __DNS_DEFS_H__ */
