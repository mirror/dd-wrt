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


#ifndef __APP_ID_CONFIG_H___
#define __APP_ID_CONFIG_H___

/**
 * \file appIdConfig.h
 *
 * \brief AppId configuration data structures
 */

/****************************** INCLUDES **************************************/

#include <sf_types.h>

#include "appId.h"
#include "client_app_api.h"
#include "service_api.h"
#include "serviceConfig.h"
#include "httpCommon.h"
#include "clientAppConfig.h"
#include "detector_sip.h"


/******************************* DEFINES **************************************/

#define APP_ID_MAX_DIRS         16
#define APP_ID_PORT_ARRAY_SIZE  65536
#define MAX_ZONES               1024


/********************************* TYPES **************************************/

struct _AppInfoTableEntry;
struct DynamicArray;
struct ServicePortPattern;
struct ClientPortPattern;

typedef struct _port_ex
{
    int family;
    struct in6_addr ip;
    struct in6_addr netmask;
} PortExclusion;

/**
 * \typedef tAppidGenericConfigItem
 *
 * \brief AppId generic configuration item
 *
 * Modules can use this generic data structure to store their configuration.
 * All such generic configurations are stored in genericConfigList. Modules
 * are responsible for populating the configuration in init() and cleaning it
 * up in clean() function.
 *
 * Currently, IMAP, PO3 and MDNS use this data structure. Lua modules currently
 * do not have any configuration. They can use this data structure in the future,
 * if needed.
 */
typedef struct appidGenericConfigItem_
{
    char    *name;  ///< Module name
    void    *pData; ///< Module configuration data
} tAppidGenericConfigItem;

typedef enum
{
    APPID_REQ_UNINITIALIZED = 0,
    APPID_REQ_YES,
    APPID_REQ_NO
} tAppIdReq;

/**
 * \typedef tAppIdConfig
 *
 * \brief AppId dynamic configuration data structure
 *
 * Members of this data structure get populated during initialization and reload.
 * They get freed after reload swap and during exit.
 */
typedef struct appIdConfig_
{
    unsigned    max_service_info;
    unsigned    net_list_count;
    NetworkSet  *net_list_list;         ///< list of network sets
    NetworkSet  *net_list;              ///< list of networks we're analyzing
    NetworkSet  *net_list_by_zone[MAX_ZONES];    ///< list of networks we're analyzing
    tAppId      tcp_port_only[65536];       ///< Service IDs for port-only TCP services
    tAppId      udp_port_only[65536];       ///< Service IDs for port-only UDP services
    tAppId      ip_protocol[256];           ///< Service IDs for non-TCP / UDP protocol services

    SF_LIST     client_app_args;            ///< List of Client App arguments

    SF_LIST     *tcp_port_exclusions_src[APP_ID_PORT_ARRAY_SIZE];   ///< for each potential port, an sflist of PortExclusion structs
    SF_LIST     *udp_port_exclusions_src[APP_ID_PORT_ARRAY_SIZE];   ///< for each potential port, an sflist of PortExclusion structs
    SF_LIST     *tcp_port_exclusions_dst[APP_ID_PORT_ARRAY_SIZE];   ///< for each potential port, an sflist of PortExclusion structs
    SF_LIST     *udp_port_exclusions_dst[APP_ID_PORT_ARRAY_SIZE];   ///< for each potential port, an sflist of PortExclusion structs

    SFXHASH     *CHP_glossary;              ///< keep track of http multipatterns here

    SFXHASH     *AF_indicators;             ///< App Forecasting list of "indicator apps"
    SFXHASH     *AF_actives;                ///< App Forecasting list of hosts to watch for "forecast apps"

    sfaddr_t    *debugHostIp;

    struct _AppInfoTableEntry   *AppInfoList;
    struct _AppInfoTableEntry   *AppInfoTable[SF_APPID_MAX];
    struct _AppInfoTableEntry   *AppInfoTableByService[SF_APPID_MAX];
    struct _AppInfoTableEntry   *AppInfoTableByClient[SF_APPID_MAX];
    struct _AppInfoTableEntry   *AppInfoTableByPayload[SF_APPID_MAX];
    struct DynamicArray         *AppInfoTableDyn;
    SFGHASH                     *AppNameHash;

    SFXHASH                 *hostPortCache;
    SFXHASH                 *lengthCache;

    tDetectorHttpConfig     detectorHttpConfig;     ///< HTTP detector configuration
    tDetectorSipConfig      detectorSipConfig;      ///< SIP detector configuration
    tServiceConfig          serviceConfig;          ///< Common configuration for all services
    tServiceSslConfig       serviceSslConfig;       ///< SSL service configuration
    tServiceDnsConfig       serviceDnsConfig;       ///< DNS service configuration
    tClientAppConfig        clientAppConfig;        ///< Common configuration for all client applications
    HttpPatternLists        httpPatternLists;

    struct ServicePortPattern *servicePortPattern;
    struct ClientPortPattern  *clientPortPattern;

    SF_LIST                 genericConfigList;      ///< List of tAppidGenericConfigItem structures

    tAppIdReq isAppIdAlwaysRequired;
} tAppIdConfig;

#ifdef SIDE_CHANNEL
typedef struct _AppIdSSConfig
{
#ifdef REG_TEST
    char *startup_input_file;
    char *runtime_output_file;
#endif
    bool use_side_channel;
} AppIdSSConfig;
#endif

/**
 * \struct tAppidStaticConfig
 *
 * \brief AppId static configuration data structure
 *
 * Members of this data structure get populated during initialization and freed
 * during exit. They are not reloadable/reconfigurable.
 * Note: appid_tp_dir can be reconfigured but gets used by 3rd party reload. AppID
 * reload does not look at this variable.
 */
struct AppidStaticConfig
{
    unsigned    disable_safe_search;
    const char *appid_thirdparty_dir;         /* directory where thirdparty modules are located.*/
    char* tp_config_path;
    char* app_stats_filename;
    unsigned long app_stats_period;
    unsigned long app_stats_rollover_size;
    unsigned long app_stats_rollover_time;
    char* app_id_detector_path;
    unsigned long memcap;
    int app_id_dump_ports;
    int app_id_debug;
    uint32_t instance_id;
    char* conf_file;
    unsigned dns_host_reporting;
    unsigned referred_appId_disabled;
    unsigned rtmp_max_packets;
    unsigned mdns_user_reporting;
    unsigned ftp_userid_disabled;
    unsigned chp_userid_disabled;
    unsigned chp_body_collection_disabled;
    unsigned chp_fflow_disabled;
    unsigned chp_body_collection_max;
    unsigned max_tp_flow_depth;
    unsigned tp_allow_probes;
    unsigned host_port_app_cache_lookup_interval;
    unsigned host_port_app_cache_lookup_range;
    unsigned multipayload_max_packets;
    unsigned http_tunnel_detect;
    uint64_t max_bytes_before_service_fail;
    uint16_t max_packet_before_service_fail;
    uint16_t max_packet_service_fail_ignore_bytes;
    bool http2_detection_enabled;    // internal HTTP/2 detection
    bool is_host_port_app_cache_runtime;
    bool check_host_port_app_cache;
    bool check_host_cache_unknown_ssl;
    bool recheck_for_unknown_appid;
    bool send_state_sharing_updates;
    bool allow_port_wildcard_host_cache;
    bool recheck_for_portservice_appid;
    tAppIdConfig* newAppIdConfig;    // Used only during reload
#ifdef SIDE_CHANNEL
    AppIdSSConfig *appId_ss_config;
#endif
#ifdef REG_TEST
    bool appid_reg_test_mode;
#endif
};
typedef struct AppidStaticConfig tAppidStaticConfig;

void appIdConfigParse(tAppidStaticConfig* appidSC, char *args);


/************************** GLOBAL VARIABLES **********************************/

/// AppId static configuration data
extern tAppidStaticConfig* appidStaticConfig;

/**
 * \brief Pointer to AppId dynamic configuration data
 *
 * This variable always points to the current active configuration that needs
 * to be used during packet processing. Lower level functions should restrain
 * from using this variable directly since they need to be context-agnostic.
 * A lower-level function (for example, clientCreatePattern()) could be called
 * during initalization, reload and reconfiguration. Pointer to the right
 * context information needs to be provided to such functions.
 */
extern tAppIdConfig         *pAppidActiveConfig;
extern tAppIdConfig         *pAppidPassiveConfig;

/********************* GLOBAL FUNCTION PROTOTYPES ****************************/

/**
 * \brief Add generic configuration item to AppID configuration list
 *
 * @param pConfig AppID configuration to which this item needs to be added
 * @param name Module name - needs to be unique per-module
 * @param pData pointer to module configuration data
 * @return None
 */
void AppIdAddGenericConfigItem(tAppIdConfig *pConfig, const char *name, void *pData);

/**
 * \brief Find a module's configuration in AppID configuration list
 *
 * @param pConfig AppID configuration in which the module's configuration needs to be searched
 * @param name Module name
 * @return pointer to module configuration data
 */
void *AppIdFindGenericConfigItem(const tAppIdConfig *pConfig, const char *name);

/**
 * \brief Remove a module's configuration from AppID configuration list
 *
 * Note: This function has to be called after the config item's data (pData) is freed
 *
 * @param pConfig AppID configuration in which the module's configuration needs to be searched
 * @param name Module name
 * @return None
 */
void AppIdRemoveGenericConfigItem(tAppIdConfig *pConfig, const char *name);


/************************** LOCAL FUNCTIONS **********************************/

inline static tAppIdConfig *appIdActiveConfigGet(void)
{
    return pAppidActiveConfig;
}

inline static tAppIdConfig *appIdNewConfigGet(void)
{
    return pAppidPassiveConfig;
}

#endif // APPID_CONFIG_H_
