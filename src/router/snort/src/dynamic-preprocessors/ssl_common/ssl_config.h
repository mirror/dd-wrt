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
 * File: ssl_config.h
 * Author: Bhagyashree Bantwal <bbantwal@cisco.com>
 * Brief: Configuration header file
*/

#ifndef SSL_CONFIG_H
#define SSL_CONFIG_H

/******************INCLUDES************************/
#include "ssl_include.h"
#include "ssl_session.h"


/******************DEFINES************************/
/* Configuration flags */
#define SSLPP_DISABLE_FLAG    0x0001 
#define SSLPP_TRUSTSERVER_FLAG  0x0002


/******************DATASTRUCTURES************************/
typedef struct _SslRuleOptData
{
    int flags;
    int mask;
} SslRuleOptData;

#ifdef ENABLE_HA
typedef struct _SSLHAConfig
{
    struct timeval min_sync_interval;
    char *startup_input_file;
    char *runtime_output_file;
    char *shutdown_output_file;
# ifdef SIDE_CHANNEL
uint8_t use_side_channel;
# endif
} SSLHAConfig;
#endif

typedef struct _SSLPP_config 
{
    ports_tbl_t ports;
    uint16_t flags;
    char *ssl_rules_dir;
    char *pki_dir;
    int memcap;
    int decrypt_memcap;
    int max_heartbeat_len;
    bool enable_ssl_ha;
#ifdef ENABLE_HA
    SSLHAConfig *ssl_ha_config;
#endif
    void *current_handle;
    void *reload_handle;
} SSLPP_config_t;

typedef struct 
{
    /*
     * SSL preprocessor global configuration structure.
     */
    SSLPP_config_t config;

} tSslPolicyConfig;

#ifdef TARGET_BASED
extern int16_t ssl_app_id;
#endif

#ifdef PERF_PROFILING
extern PreprocStats sslpp_perf_stats;
#endif

/*****************EXTERNS************************/
/* Prototypes for public interface */
extern tSslPolicyConfig sslPolicyConfig[];
extern tSfPolicyUserContextId ssl_config;
extern void SSLPP_init(struct _SnortConfig *sc, char *args);
#ifdef SNORT_RELOAD
extern void SSLReload(struct _SnortConfig *, char *, void **);
extern int SSLReloadVerify(struct _SnortConfig *, void *);
extern void * SSLReloadSwap(struct _SnortConfig *, void *);
extern void SSLReloadSwapFree(void *);
#endif


/*****************FUNCTIONS************************/
void SSL_InitGlobals(void);
int SSLPP_rule_eval(void *raw_packet, const uint8_t **cursor, void *data);
void SSLPP_process(void *raw_packet, void *context);
void SSLPP_drop_stats(int exiting);
void DisplaySSLPPStats (uint16_t type, void *old_context, struct _THREAD_ELEMENT *te, ControlDataSendFunc f);
#endif /* SSL_CONFIG_H */
