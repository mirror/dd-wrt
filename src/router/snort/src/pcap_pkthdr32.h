/*
** Copyright (C) 2007-2011 Sourcefire, Inc.
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __PCAP_PKTHDR32_H__
#define __PCAP_PKTHDR32_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

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

