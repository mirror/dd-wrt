/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

#ifndef __HI_INCLUDE_H__
#define __HI_INCLUDE_H__

#include "sf_types.h"
#include "snort_debug.h"
#include "ipv6_port.h"

#define HI_UNKNOWN_METHOD 1
#define HI_POST_METHOD 2
#define HI_GET_METHOD 4

typedef struct _hi_stats {
    uint64_t unicode;
    uint64_t double_unicode;
    uint64_t non_ascii;        /* Non ASCII-representable character in URL */
    uint64_t dir_trav;         /* '../' */
    uint64_t slashes;          /* '//' */
    uint64_t self_ref;         /* './' */
    uint64_t post;             /* Number of POST methods encountered */
    uint64_t get;              /* Number of GETs */
    uint64_t post_params;      /* Number of successfully extract post parameters */
    uint64_t req_headers;      /* Number of successfully extracted request headers */
    uint64_t resp_headers;     /* Number of successfully extracted response headers */
#ifdef DEBUG
    uint64_t req_header_len;
    uint64_t resp_header_len;
#endif
    uint64_t req_cookies;          /* Number of successfully extracted request cookies */
    uint64_t resp_cookies;         /* Number of successfully extracted response cookies */
#ifdef DEBUG
    uint64_t req_cookie_len;
    uint64_t resp_cookie_len;
#endif
    uint64_t total;
    uint64_t gzip_pkts;
    uint64_t compr_bytes_read;
    uint64_t decompr_bytes_read;
    uint64_t h2_rebuilt_packets;
    uint64_t mem_used;
    uint64_t session_count;
} HIStats;

extern HIStats hi_stats;

#ifndef inline

#ifdef WIN32
#define inline __inline
#else
#define inline inline
#endif
#endif /* endif for inline */

#endif
