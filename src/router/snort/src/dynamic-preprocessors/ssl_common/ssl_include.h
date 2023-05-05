/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2013-2013 Sourcefire, Inc.
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
 * File: ssl_include.h
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 * Brief: Header file with all includes required by SSL
 */

#ifndef SSL_INCLUDE_H
#define SSL_INCLUDE_H

/******************INCLUDES************************/

#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#include <sys/time.h>
#include <stdbool.h>
#else
#include "sf_types.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include "ssl.h"
#include "sfcommon.h"
#include "profiler.h"
#include "sfPolicy.h"
#include "sfPolicyUserData.h"
#include "sf_snort_plugin_api.h"
#include "snort_debug.h"
#include "preprocids.h"
#include "sf_preproc_info.h"
#include "sf_snort_packet.h"

typedef void (*PP_Set_Flow_Id_Callback_Func) (void *app_data, uint32_t flow_context);

typedef struct _ssl_callback_interface
{

    int (*policy_initialize)(void *, bool);
    void (*policy_free)(void **, bool);
    void (*session_initialize)(SFSnortPacket* p, void *app_data, PP_Set_Flow_Id_Callback_Func pp_callback);
    void (*session_free)(uint32_t fid);
    bool (*is_session_ssl)(SFSnortPacket* p);
    int  (*get_ssl_flow_flags)(SFSnortPacket* p, void *sd, uint32_t *ssn_flags);
    void (*register_ha_funcs)(void);
    bool (*reload_mem_adjust_available)(void);
    void (*register_reload_mem_adjust)(struct _SnortConfig *sc, void *reload_config);
} ssl_callback_interface_t;


extern bool IsTlsClientHello(const uint8_t *ptr, const uint8_t *end);
extern bool IsTlsServerHello(const uint8_t *ptr, const uint8_t *end);
extern bool IsSSL(const uint8_t *ptr, int len, int pkt_flags);

#endif
