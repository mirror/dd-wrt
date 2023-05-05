/*
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
 */

/*
 * File: ssl_session.h
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 * Brief: Header file for SSL session handling
 */

#ifndef SSL_SESSION_H
#define SSL_SESSION_H

#include "ssl_include.h"

typedef struct _SSL_SsnData
{
    uint32_t ssn_flags;
    uint32_t flow_id;
    bool is_ssl;
    uint16_t partial_rec_len[4];
} SSL_SsnData;

static inline void SSL_SetAppData(const SFSnortPacket *p, void *data, StreamAppDataFree sdfree)
{
    if( p->stream_session != NULL )
        _dpd.sessionAPI->set_application_data( p->stream_session, PP_SSL, data, sdfree );
}

static inline void * SSL_GetAppData(const SFSnortPacket *p)
{
    if( p->stream_session != NULL )
        return _dpd.sessionAPI->get_application_data( p->stream_session, PP_SSL );
    else
        return NULL;
}
#endif
