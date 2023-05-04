/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
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

#ifndef __PCAP_PKTHDR32_H__
#define __PCAP_PKTHDR32_H__

#include "sf_types.h"

/* we must use fixed size of 32 bits, because on-disk
 * format of savefiles uses 32-bit tv_sec (and tv_usec)
 */
struct sf_timeval32
{
    uint32_t tv_sec;      /* seconds */
    uint32_t tv_usec;     /* microseconds */
};

/* this is equivalent to the pcap pkthdr struct, but we need
 * a 32 bit one for unified output
 */
struct pcap_pkthdr32
{
    struct sf_timeval32 ts;   /* packet timestamp */
    uint32_t caplen;          /* packet capture length */
    uint32_t len;             /* packet "real" length */
};


#endif // __PCAP_PKTHDR32_H__

