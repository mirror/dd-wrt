/*
 * sf_decompression_define.h
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Date: 3/8/2011
 *
 * Decompression API for Snort Plugins.
 *
 */

#ifndef SF_DECOMPRESSION_DEFINES_H
#define SF_DECOMPRESSION_DEFINES_H

#include <stdint.h>

/* Types */
typedef enum compression_type
{
    COMPRESSION_TYPE_DEFLATE = 1,
    COMPRESSION_TYPE_GZIP,
    COMPRESSION_TYPE_MAX
} compression_type_t;

/* Return Codes */
#define SNORT_DECOMPRESS_OK 0
#define SNORT_DECOMPRESS_OUTPUT_TRUNC  1
#define SNORT_DECOMPRESS_BAD_ARGUMENT (-1)
#define SNORT_DECOMPRESS_BAD_DATA     (-2)
#define SNORT_DECOMPRESS_ERROR (-3)

#endif /* SF_DECOMPRESSION_DEFINES_H */
