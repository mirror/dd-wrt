/*
 ** Copyright (C) 2002-2011 Sourcefire, Inc.
 ** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

#include "detection_util.h"

const uint8_t *file_data_ptr;

uint8_t base64_decode_buf[DECODE_BLEN];
uint32_t base64_decode_size;

uint32_t mime_decode_size;
uint8_t mime_present;

const uint8_t *doe_ptr;
uint8_t doe_buf_flags;

HttpUri UriBufs[HTTP_BUFFER_MAX];
DataBuffer DecodeBuffer;

#ifdef DEBUG
const char* uri_buffer_name[HTTP_BUFFER_MAX] =
{
    "http_uri",
    "http_raw_uri",
    "http_header",
    "http_raw_header",
    "http_client_body",
    "http_method",
    "http_cookie",
    "http_raw_cookie",
    "http_stat_code",
    "http_stat_msg"
};
#endif

