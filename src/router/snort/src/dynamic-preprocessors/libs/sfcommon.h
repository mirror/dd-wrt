/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc.
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

#ifndef DYN_PP_PARSER_H
#define DYN_PP_PARSER_H

#include "snort_bounds.h"
#include "snort_debug.h"

#define SFP_MIN_ERR_STR 128

/* Convert port value into an index */
#define PORT_INDEX(port) port/8

/* Convert port value into a value for bitwise operations */
#define CONV_PORT(port) 1<<(port%8)

typedef enum _SFP_ret {
    SFP_SUCCESS,
    SFP_ERROR
} SFP_ret_t;

typedef uint8_t ports_tbl_t[MAXPORTS/8];

typedef char SFP_errstr_t[SFP_MIN_ERR_STR + 1];

static inline char *SFP_GET_ERR(SFP_errstr_t err)
{
    return (char*)err;
}

SFP_ret_t SFP_ports(ports_tbl_t ports, char *str, SFP_errstr_t errstr);

SFP_ret_t SFP_snprintfa(char *buf, size_t buf_size, const char *format, ...);

#endif
