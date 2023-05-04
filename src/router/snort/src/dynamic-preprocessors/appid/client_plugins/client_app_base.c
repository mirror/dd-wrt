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


/**
 * @file   client_app_base.c
 * @author Ron Dempster <Ron.Dempster@sourcefire.com>
 *
 */

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "profiler.h"
#include "appIdApi.h"
#include "client_app_base.h"
#include "str_search.h"

#include "common_util.h"
#include "client_app_api.h"
#include "client_app_base.h"

#include "client_app_msn.h"
#include "client_app_aim.h"
#include "client_app_ym.h"
#include "detector_cip.h"
#include "detector_sip.h"
#include "luaDetectorModule.h"
#include "luaDetectorApi.h"
#include "httpCommon.h"
#include "fw_appid.h"
#include "service_ssl.h"
#include "appIdConfig.h"
#include "detector_dns.h"
#include "detector_pattern.h"

/*#define CLIENT_APP_DEBUG    1 */

#define BUFSIZE         512

/* If this is greater than 1, more than 1 client detector can be searched for
 * and tried per flow based on pattern (if a valid detector doesn't
 * already exist). */
#define MAX_CANDIDATE_CLIENTS 10

static void *client_app_flowdata_get(tAppIdData *flowp, unsigned client_id);
static int client_app_flowdata_add(tAppIdData *flowp, void *data, unsigned client_id, AppIdFreeFCN fcn);
static void AppIdAddClientAppInfo(tAppIdData *flowp, const char *info);

static const ClientAppApi client_app_api =
{
    .data_get = &client_app_flowdata_get,
    .data_add = &client_app_flowdata_add,
    .add_app= &AppIdAddClientApp,
    .add_info = &AppIdAddClientAppInfo,
    .add_user = &AppIdAddUser,
    .add_payload = &AppIdAddPayload
};


static void LuaClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                        const uint8_t * const pattern, unsigned size,
                                        int position, struct _Detector *userData);
static void CClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                      const uint8_t * const pattern, unsigned size,
                                      int position, tAppIdConfig *pConfig);
static void CClientAppRegisterPatternNoCase(RNAClientAppFCN fcn, uint8_t proto,
                                            const uint8_t * const pattern, unsigned size,
                                            int position, tAppIdConfig *pConfig);

static InitClientAppAPI client_init_api =
{
    .RegisterPattern = &CClientAppRegisterPattern,
    .RegisterPatternEx = &LuaClientAppRegisterPattern,
    .RegisterPatternNoCase = &CClientAppRegisterPatternNoCase,
    .RegisterAppId = &appSetClientValidator,
    .RegisterDetectorCallback = &appSetClientDetectorCallback,
};

static CleanClientAppAPI clean_api =
{
};

static FinalizeClientAppAPI finalize_api =
{
};

extern tRNAClientAppModule bit_client_mod;
extern tRNAClientAppModule bit_tracker_client_mod;
extern tRNAClientAppModule rtp_client_mod;
extern tRNAClientAppModule ssh_client_mod;
extern tRNAClientAppModule timbuktu_client_mod;
extern tRNAClientAppModule tns_client_mod;
extern tRNAClientAppModule vnc_client_mod;
extern tRNAClientAppModule pattern_udp_client_mod;
extern tRNAClientAppModule pattern_tcp_client_mod;
extern tRNAClientAppModule http_client_mod;

static tRNAClientAppModule *static_client_list[] =
{
    &msn_client_mod,
    &aim_client_mod,
    &ym_client_mod,
    &sip_udp_client_mod,
    &sip_tcp_client_mod,
    &bit_client_mod,
    &bit_tracker_client_mod,
    &rtp_client_mod,
    &ssh_client_mod,
    &timbuktu_client_mod,
    &tns_client_mod,
    &vnc_client_mod,
    &pattern_udp_client_mod,
    &pattern_tcp_client_mod,
    &dns_udp_client_mod,
    &dns_tcp_client_mod,
    &http_client_mod,
    &cip_client_mod,
    &enip_client_mod
};

/*static const char * const MODULE_NAME = "ClientApp"; */

const ClientAppApi *getClientApi(void)
{
    return &client_app_api;
}

RNAClientAppModuleConfig *getClientAppModuleConfig(const char *moduleName, tClientAppConfig *pClientAppConfig)
{
    RNAClientAppModuleConfig *mod_config;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&pClientAppConfig->module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&pClientAppConfig->module_configs))
    {
        if (strcasecmp(mod_config->name, moduleName) == 0) break;
    }
    return mod_config;
}

tRNAClientAppModule *ClientAppGetClientAppModule(RNAClientAppFCN fcn, struct _Detector *userdata,
                                                      tClientAppConfig *pClientAppConfig)
{
    RNAClientAppRecord *li;

    for (li=pClientAppConfig->tcp_client_app_list; li; li=li->next)
    {
        if ((li->module->validate == fcn) && (li->module->userData == userdata))
            return li->module;
    }
    for (li=pClientAppConfig->udp_client_app_list; li; li=li->next)
    {
        if ((li->module->validate == fcn) && (li->module->userData == userdata))
            return li->module;
    }
    return NULL;
}

static void clientCreatePattern(RNAClientAppFCN fcn, uint8_t proto,
                         const uint8_t * const pattern, unsigned size,
                         int position, unsigned nocase, struct _Detector *userData,
                         const tRNAClientAppModule *li, tClientAppConfig *pClientAppConfig)
{
    void **patterns;
    int *count;
    tClientPatternData *pd;

    if (!li)
    {
        _dpd.errMsg( "Invalid client app when registering a pattern");
        return;
    }

    if (proto == IPPROTO_TCP)
    {
        patterns = &pClientAppConfig->tcp_patterns;
        count = &pClientAppConfig->tcp_pattern_count;
    }
    else if (proto == IPPROTO_UDP)
    {
        patterns = &pClientAppConfig->udp_patterns;
        count = &pClientAppConfig->udp_pattern_count;
    }
    else
    {
        _dpd.errMsg("Invalid protocol when registering a pattern: %u\n",(unsigned)proto);
        return;
    }

    if (!(*patterns))
    {
        *patterns = _dpd.searchAPI->search_instance_new_ex(MPSE_ACF);
        if (!(*patterns))
        {
            _dpd.errMsg("Error initializing the pattern table for protocol %u\n",(unsigned)proto);
            return;
        }
    }

    pd = malloc(sizeof(*pd));
    if (pd)
    {
        pd->ca = li;
        pd->position = position;
        (*count)++;
        pd->next = pClientAppConfig->pattern_data_list;
        pClientAppConfig->pattern_data_list = pd;
        _dpd.searchAPI->search_instance_add_ex(*patterns, (const char *)pattern, size, pd, nocase);
    }
    else _dpd.errMsg( "Error allocating pattern data");
}

static void CClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                     const uint8_t * const pattern, unsigned size,
                                     int position, tAppIdConfig *pConfig)
{
    ClientAppRegisterPattern(fcn, proto, pattern, size, position, 0, NULL, &pConfig->clientAppConfig);
}

static void CClientAppRegisterPatternNoCase(RNAClientAppFCN fcn, uint8_t proto,
                                            const uint8_t * const pattern, unsigned size,
                                            int position, tAppIdConfig *pConfig)
{
    ClientAppRegisterPattern(fcn, proto, pattern, size, position, 1, NULL, &pConfig->clientAppConfig);
}

static void LuaClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                        const uint8_t * const pattern, unsigned size,
                                        int position, struct _Detector *userData)
{
    ClientAppRegisterPattern(fcn, proto, pattern, size, position, 0, userData, &userData->pAppidNewConfig->clientAppConfig);
}

void ClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                              const uint8_t * const pattern, unsigned size,
                              int position, unsigned nocase, struct _Detector *userData,
                              tClientAppConfig *pClientAppConfig)
{
    RNAClientAppRecord *list;
    RNAClientAppRecord *li;

    if (proto == IPPROTO_TCP)
        list = pClientAppConfig->tcp_client_app_list;
    else if (proto == IPPROTO_UDP)
        list = pClientAppConfig->udp_client_app_list;
    else
    {
        _dpd.errMsg("Invalid protocol when registering a pattern: %u\n",(unsigned)proto);
        return;
    }

    for (li=list; li; li=li->next)
    {
        if ((li->module->validate == fcn) && (li->module->userData == userData))
        {
            clientCreatePattern(fcn, proto, pattern, size, position, nocase, userData, li->module, pClientAppConfig);
            break;
        }
    }
}

int clientAppLoadForConfigCallback(void *symbol, tClientAppConfig *pClientAppConfig)
{
    static unsigned client_module_index = 0;
    tRNAClientAppModule *cam = (tRNAClientAppModule *)symbol;
    RNAClientAppRecord * *list = NULL;
    RNAClientAppRecord *li;

    _dpd.debugMsg(DEBUG_LOG,"Adding client %s for protocol %u\n",cam->name, (unsigned)cam->proto);

    if (client_module_index >= 65536)
    {
        _dpd.errMsg( "Maximum number of client modules exceeded");
        return -1;
    }

    if (cam->proto == IPPROTO_TCP)
    {
        list = &pClientAppConfig->tcp_client_app_list;
    }
    else if (cam->proto == IPPROTO_UDP)
    {
        list = &pClientAppConfig->udp_client_app_list;
    }
    else
    {
        _dpd.errMsg( "Client %s did not have a valid protocol (%u)",
                cam->name, (unsigned)cam->proto);
        return -1;
    }

    for (li=*list; li; li=li->next)
    {
        if (li->module == cam)
            break;
    }
    if (!li)
    {
        if (!(li = calloc(1, sizeof(*li))))
        {
            _dpd.errMsg( "Could not allocate a client app list element");
            return -1;
        }
        li->next = *list;
        *list = li;
        li->module = cam;

        cam->api = &client_app_api;

        cam->flow_data_index = client_module_index | APPID_SESSION_DATA_CLIENT_MODSTATE_BIT;
        client_module_index++;
    }
    /*Can't set cam->userData to NULL because Lua detectors use it although C detectors don't */
    /*cam->userData = NULL; */
    return 0;
}

int clientAppLoadCallback(void *symbol)
{
    return clientAppLoadForConfigCallback(symbol, &pAppidActiveConfig->clientAppConfig);
}

int LoadClientAppModules(const char **dir_list, tAppIdConfig *pConfig)
{
    unsigned i;

    for (i=0; i<sizeof(static_client_list)/sizeof(*static_client_list); i++)
    {
        if (clientAppLoadForConfigCallback(static_client_list[i], &pConfig->clientAppConfig))
            return -1;
    }

    return 0;
}

static void AddModuleConfigItem(char *module_name, char *item_name, char *item_value, tClientAppConfig *config)
{
    RNAClientAppModuleConfig *mod_config;
    RNAClientAppModuleConfigItem *item;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&config->module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&config->module_configs))
    {
        if (strcasecmp(mod_config->name, module_name) == 0) break;
    }
    if (!mod_config)
    {
        mod_config = calloc(1, sizeof(*mod_config));
        if (!mod_config)
        {
            _dpd.fatalMsg( "Failed to allocate a module configuration");
            exit(-1);
        }
        mod_config->name = strdup(module_name);
        if (!mod_config->name)
        {
            _dpd.fatalMsg( "Failed to allocate a module configuration name");
            exit(-1);
        }
        sflist_init(&mod_config->items);
        if (sflist_add_tail(&config->module_configs, mod_config))
        {
            _dpd.fatalMsg( "Failed to add a module configuration");
            exit(-1);
        }
    }
    for (item = (RNAClientAppModuleConfigItem *)sflist_first(&mod_config->items);
         item;
         item = (RNAClientAppModuleConfigItem *)sflist_next(&mod_config->items))
    {
        if (strcasecmp(item->name, item_name) == 0)
        {
            break;
        }
    }
    if (!item)
    {
        item = calloc(1, sizeof(*item));
        if (!item)
        {
            _dpd.fatalMsg( "Failed to allocate a module configuration item");
            exit(-1);
        }
        item->name = strdup(item_name);
        if (!item->name)
        {
            _dpd.fatalMsg( "Failed to allocate a module configuration item name");
            exit(-1);
        }
        if (sflist_add_tail(&mod_config->items, item))
        {
            _dpd.fatalMsg( "Failed to add a module configuration item");
            exit(-1);
        }
    }
    if (item->value)
    {
        free((void *)item->value);
        item->value = NULL;
    }
    item->value = strdup(item_value);
    if (!item->value)
    {
        _dpd.fatalMsg( "Failed to add a module configuration item value");
        exit(-1);
    }
}

static void ClientAppParseOption(tClientAppConfig *config,
                                 char *key, char *value)
{
    char *p;

    if(!strcasecmp(key, "enable"))
    {
        config->enabled = atoi(value);
    }
    else if ((p = strchr(key, ':')) && p[1])
    {
        *p = 0;
        AddModuleConfigItem(key, &p[1], value, config);
        *p = ':';
    }
    else
    {
        _dpd.debugMsg(DEBUG_LOG, "Unknown client app argument ignored: key(%s) value(%s)",
                key, value);
    }
}

static int ClientAppParseArgs(tClientAppConfig *config, SF_LIST *args)
{
    ConfigItem *ci;

    for (ci=(ConfigItem *)sflist_first(args);
         ci;
         ci=(ConfigItem *)sflist_next(args))
    {
        ClientAppParseOption(config, ci->name, ci->value);
    }

    return 0;
}

#define MAX_DISPLAY_SIZE   65536
static void DisplayClientAppConfig(tClientAppConfig *config)
{
    static char buffer[MAX_DISPLAY_SIZE];
    int position = 0;
    int tmp;
    RNAClientAppModuleConfig *mod_config;
    RNAClientAppModuleConfigItem *item;

    tmp = snprintf(&buffer[position], MAX_DISPLAY_SIZE-position,
                   "\n----------------------------------------------\nRNA Client App Config\n");
    if (tmp >= MAX_DISPLAY_SIZE-position) position = MAX_DISPLAY_SIZE;
    else if (tmp > 0) position += tmp;

    tmp = snprintf(&buffer[position], MAX_DISPLAY_SIZE-position,
                   "Enabled: %s\n", config->enabled?"Yes":"No");
    if (tmp >= MAX_DISPLAY_SIZE-position) position = MAX_DISPLAY_SIZE;
    else if (tmp > 0) position += tmp;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&config->module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&config->module_configs))
    {
        tmp = snprintf(&buffer[position], MAX_DISPLAY_SIZE-position,
                       "%s\n", mod_config->name);
        if (tmp >= MAX_DISPLAY_SIZE-position) position = MAX_DISPLAY_SIZE;
        else if (tmp > 0) position += tmp;
        for (item = (RNAClientAppModuleConfigItem *)sflist_first(&mod_config->items);
             item;
             item = (RNAClientAppModuleConfigItem *)sflist_next(&mod_config->items))
        {
            tmp = snprintf(&buffer[position], MAX_DISPLAY_SIZE-position,
                           "   %s: %s\n", item->name, item->value);
            if (tmp >= MAX_DISPLAY_SIZE-position) position = MAX_DISPLAY_SIZE;
            else if (tmp > 0) position += tmp;
        }
    }
    tmp = snprintf(&buffer[position], MAX_DISPLAY_SIZE-position,
                   "----------------------------------------------\n");
    if (tmp >= MAX_DISPLAY_SIZE-position) position = MAX_DISPLAY_SIZE;
    else if (tmp > 0) position += tmp;
    _dpd.debugMsg(DEBUG_LOG,"%s\n",buffer);
}


static void free_module_config_item(void *module_config_item)
{
    RNAClientAppModuleConfigItem *item = (RNAClientAppModuleConfigItem *)module_config_item;

    if (item)
    {
        if (item->name)
            free((void *)item->name);
        if (item->value)
            free((void *)item->value);
        free(item);
    }
}

static void free_module_config(void *module_config)
{
    RNAClientAppModuleConfig *config = (RNAClientAppModuleConfig *)module_config;
    if (config)
    {
        if (config->name)
            free((void *)config->name);
        sflist_static_free_all(&config->items, &free_module_config_item);
        free(config);
    }
}

static void initialize_module(RNAClientAppRecord *li, tClientAppConfig *pClientAppConfig)
{
    RNAClientAppModuleConfig *mod_config;
    int rval;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&pClientAppConfig->module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&pClientAppConfig->module_configs))
    {
        if (strcasecmp(mod_config->name, li->module->name) == 0) break;
    }
    if (li->module->init && (rval=li->module->init(&client_init_api, mod_config ? &mod_config->items:NULL)) != CLIENT_APP_SUCCESS)
    {
        _dpd.fatalMsg("Could not initialize the %s client app element: %d\n",li->module->name, rval);
        exit(-1);
    }
}

static void finalize_module(RNAClientAppRecord *li)
{
    int rval;

    if (li->module->finalize && (rval=li->module->finalize(&finalize_api)) != CLIENT_APP_SUCCESS)
    {
        _dpd.fatalMsg("Could not finlize the %s client app element: %d\n",li->module->name, rval);
        exit(-1);
    }
}

static void clean_module(RNAClientAppRecord *li)
{
    if (li->module->clean)
        li->module->clean(&clean_api);
}


void UnconfigureClientApp(tAppIdConfig *pConfig)
{
    tClientPatternData *pd;
    RNAClientAppRecord *li;

    clean_api.pAppidConfig = pConfig;
    for (li = pConfig->clientAppConfig.tcp_client_app_list; li; li = li->next)
        clean_module(li);
    for (li = pConfig->clientAppConfig.udp_client_app_list; li; li = li->next)
        clean_module(li);

    if (pConfig->clientAppConfig.tcp_patterns)
    {
        _dpd.searchAPI->search_instance_free(pConfig->clientAppConfig.tcp_patterns);
        pConfig->clientAppConfig.tcp_patterns = NULL;
    }

    if (pConfig->clientAppConfig.udp_patterns)
    {
        _dpd.searchAPI->search_instance_free(pConfig->clientAppConfig.udp_patterns);
        pConfig->clientAppConfig.udp_patterns = NULL;
    }

    while (pConfig->clientAppConfig.pattern_data_list)
    {
        pd = pConfig->clientAppConfig.pattern_data_list;
        pConfig->clientAppConfig.pattern_data_list = pd->next;
        free((void *)pd);
    }

    CleanHttpPatternLists(pConfig);
    ssl_detector_free_patterns(&pConfig->serviceSslConfig);
    dns_detector_free_patterns(&pConfig->serviceDnsConfig);
    CleanClientPortPatternList(pConfig);

    sflist_static_free_all(&pConfig->clientAppConfig.module_configs, &free_module_config);
}

/**
 * Initialize the configuration of the client app module
 *
 * @param args
 */
void ClientAppInit(tAppidStaticConfig* appidSC, tAppIdConfig *pConfig)
{
    RNAClientAppRecord *li;

    sflist_init(&pConfig->clientAppConfig.module_configs);
    pConfig->clientAppConfig.enabled = 1;

    ClientAppParseArgs(&pConfig->clientAppConfig, &pConfig->client_app_args);
    DisplayClientAppConfig(&pConfig->clientAppConfig);

    if (pConfig->clientAppConfig.enabled)
    {
        client_init_api.debug = app_id_debug;
        client_init_api.pAppidConfig = pConfig;
        client_init_api.instance_id = appidSC->instance_id;

        for (li = pConfig->clientAppConfig.tcp_client_app_list; li; li = li->next)
            initialize_module(li, &pConfig->clientAppConfig);
        for (li = pConfig->clientAppConfig.udp_client_app_list; li; li = li->next)
            initialize_module(li, &pConfig->clientAppConfig);

        luaModuleInitAllClients();

        for (li = pConfig->clientAppConfig.tcp_client_app_list; li; li = li->next)
             finalize_module(li);

        for (li = pConfig->clientAppConfig.udp_client_app_list; li; li = li->next)
            finalize_module(li);

    }
}

void ClientAppFinalize(tAppIdConfig *pConfig)
{
    if (pConfig->clientAppConfig.enabled)
    {
        if (pConfig->clientAppConfig.tcp_patterns)
        {
            _dpd.searchAPI->search_instance_prep(pConfig->clientAppConfig.tcp_patterns);
        }
        if (pConfig->clientAppConfig.udp_patterns)
        {
            _dpd.searchAPI->search_instance_prep(pConfig->clientAppConfig.udp_patterns);
        }
    }
}

typedef struct _CLIENT_APP_MATCH
{
    struct _CLIENT_APP_MATCH *next;
    unsigned count;
    const tRNAClientAppModule *ca;
} ClientAppMatch;

static ClientAppMatch *match_free_list;

/**
 * Clean up the configuration of the client app module
 */
void CleanupClientApp(tAppIdConfig *pConfig)
{
#ifdef APPID_FULL_CLEANUP
    ClientAppMatch *match;
    tClientPatternData *pd;
    RNAClientAppRecord *li;

    clean_api.pAppidConfig = pConfig;
    if (pConfig->clientAppConfig.tcp_patterns) {
         _dpd.searchAPI->search_instance_free(pConfig->clientAppConfig.tcp_patterns);
        pConfig->clientAppConfig.tcp_patterns = NULL;
    }
    if (pConfig->clientAppConfig.udp_patterns) {
         _dpd.searchAPI->search_instance_free(pConfig->clientAppConfig.udp_patterns);
        pConfig->clientAppConfig.udp_patterns = NULL;
    }
    while ((pd = pConfig->clientAppConfig.pattern_data_list) != NULL)
    {
        pConfig->clientAppConfig.pattern_data_list = pd->next;
        free(pd);
    }

    while ((li=pConfig->clientAppConfig.tcp_client_app_list) != NULL)
    {
        pConfig->clientAppConfig.tcp_client_app_list = li->next;
        if (li->module->clean)
            li->module->clean(&clean_api);
        free(li);
    }
    while ((li=pConfig->clientAppConfig.udp_client_app_list) != NULL)
    {
        pConfig->clientAppConfig.udp_client_app_list = li->next;
        if (li->module->clean)
            li->module->clean(&clean_api);
        free(li);
    }

    luaModuleCleanAllClients();

    CleanHttpPatternLists(pConfig);
    ssl_detector_free_patterns(&pConfig->serviceSslConfig);
    dns_detector_free_patterns(&pConfig->serviceDnsConfig);
    CleanClientPortPatternList(pConfig);

    sflist_static_free_all(&pConfig->clientAppConfig.module_configs, &free_module_config);
    while ((match=match_free_list) != NULL)
    {
        match_free_list = match->next;
        free(match);
    }
#endif
}

/*
 * Callback function for string search
 *
 * @param   id      id in array of search strings from pop_config.cmds
 * @param   index   index in array of search strings from pop_config.cmds
 * @param   data    buffer passed in to search function
 *
 * @return response
 * @retval 1        commands caller to stop searching
 */
static int pattern_match(void* id, void *unused_tree, int index, void* data, void *unused_neg)
{
    ClientAppMatch * *matches = (ClientAppMatch **)data;
    tClientPatternData *pd = (tClientPatternData *)id;
    ClientAppMatch *cam;

    if (pd->position >= 0 && pd->position != index)
        return 0;

    for (cam=*matches; cam; cam=cam->next)
    {
        if (cam->ca == pd->ca)
            break;
    }

    if (cam)
        cam->count++;
    else
    {
        if (match_free_list)
        {
            cam = match_free_list;
            match_free_list = cam->next;
            memset(cam, 0, sizeof(*cam));
        }
        else
            cam = calloc(1, sizeof(*cam));

        if (cam)
        {
            cam->count = 1;
            cam->ca = pd->ca;
            cam->next = *matches;
            *matches = cam;
        }
        else
        {
            _dpd.errMsg( "Error allocating a client app match structure");
        }
    }
    return 0;
}

void AppIdAddClientApp(SFSnortPacket *p, int direction, const tAppIdConfig *pConfig, tAppIdData *flowp, tAppId service_id, tAppId id, const char *version)
{
    tAppId tmpAppId = flowp->clientAppId;
    tAppId tmpServiceAppId = flowp->clientServiceAppId;

    if (version)
    {
        if (flowp->clientVersion)
        {
            if (strcmp(version, flowp->clientVersion))
            {
                free(flowp->clientVersion);
                flowp->clientVersion = strdup(version);
                if (!flowp->clientVersion)
                    _dpd.errMsg("failed to allocate client version name");
            }
        }
        else
        {
            flowp->clientVersion = strdup(version);
            if (!flowp->clientVersion)
                _dpd.errMsg("failed to allocate client version name");
        }
    }

    setAppIdFlag(flowp, APPID_SESSION_CLIENT_DETECTED);
    flowp->clientServiceAppId = service_id;
    flowp->clientAppId = id;
    checkSandboxDetection(id);

    if (id > APP_ID_NONE && tmpAppId != id)
        CheckDetectorCallback(p, flowp, (APPID_SESSION_DIRECTION) direction, id, pConfig);

    if (service_id > APP_ID_NONE && tmpServiceAppId != service_id)
        CheckDetectorCallback(p, flowp, (APPID_SESSION_DIRECTION) direction, service_id, pConfig);
}

static void AppIdAddClientAppInfo(tAppIdData *flowp, const char *info)
{
    if (flowp->hsession && !flowp->hsession->url)
    {
        flowp->hsession->url = strdup(info);
        if (!flowp->hsession->url)
            _dpd.errMsg("failed to allocate url");
    }
}

static ClientAppMatch *BuildClientPatternList(const SFSnortPacket *pkt,
                                              uint32_t protocol,
                                              const tClientAppConfig *pClientAppConfig)
{
    ClientAppMatch *match_list = NULL;
    void *patterns;

    if (protocol == IPPROTO_TCP)
        patterns = pClientAppConfig->tcp_patterns;
    else
        patterns = pClientAppConfig->udp_patterns;

    if (!patterns)
        return NULL;

    _dpd.searchAPI->search_instance_find_all(patterns,
            (char *)pkt->payload,
            pkt->payload_size,
            0,
            &pattern_match, (void*)&match_list
            );

    return match_list;
}

static const tRNAClientAppModule *GetNextFromClientPatternList(ClientAppMatch **match_list)
{
    ClientAppMatch *curr = NULL;
    ClientAppMatch *prev = NULL;
    ClientAppMatch *max_curr = NULL;
    ClientAppMatch *max_prev = NULL;
    unsigned max_count;
    unsigned max_precedence;

    curr = *match_list;
    max_count = 0;
    max_precedence = 0;
    while (curr)
    {
        if (curr->count >= curr->ca->minimum_matches
                && ((curr->count > max_count)
                    || (curr->count == max_count && curr->ca->precedence > max_precedence)))
        {
            max_count = curr->count;
            max_precedence = curr->ca->precedence;
            max_curr = curr;
            max_prev = prev;
        }
        prev = curr;
        curr = curr->next;
    }

    if (max_curr != NULL)
    {
        if (max_prev == NULL)
        {
            *match_list = (*match_list)->next;
        }
        else
        {
            max_prev->next = max_curr->next;
        }
        max_curr->next = match_free_list;
        match_free_list = max_curr;
        return max_curr->ca;
    }
    else
    {
        return NULL;
    }
}

static void FreeClientPatternList(ClientAppMatch **match_list)
{
    ClientAppMatch *cam;
    ClientAppMatch *tmp;

    cam = *match_list;
    while (cam)
    {
        tmp = cam;
        cam = tmp->next;
        tmp->next = match_free_list;
        match_free_list = tmp;
    }
    *match_list = NULL;
}

/**
 * The process to determine the running client app given the packet data.
 *
 * @param p packet to process
 */
static void ClientAppID(SFSnortPacket *p, const int dir, tAppIdData *flowp, const tAppIdConfig *pConfig)
{
    const tRNAClientAppModule *client = NULL;
    ClientAppMatch *match_list;

#ifdef CLIENT_APP_DEBUG
    _dpd.logMsg( "Client");
#endif

    if (!p->payload_size)
        return;

    if (flowp->clientData != NULL)
        return;

    if (flowp->candidate_client_list != NULL)
    {
        if (flowp->num_candidate_clients_tried > 0)
            return;
    }
    else
    {
        if (!(flowp->candidate_client_list = malloc(sizeof(SF_LIST))))
        {
            _dpd.errMsg("Could not allocate a candidate client list.");
            return;
        }
        sflist_init(flowp->candidate_client_list);
        flowp->num_candidate_clients_tried = 0;
    }

    match_list = BuildClientPatternList(p, flowp->proto, &pConfig->clientAppConfig);
    while (flowp->num_candidate_clients_tried < MAX_CANDIDATE_CLIENTS)
    {
        const tRNAClientAppModule *tmp = GetNextFromClientPatternList(&match_list);
        if (tmp != NULL)
        {
            client = sflist_first(flowp->candidate_client_list);
            while (client && (client != tmp))
                client = sflist_next(flowp->candidate_client_list);
            if (client == NULL)
            {
                sflist_add_tail(flowp->candidate_client_list, (void*)tmp);
                flowp->num_candidate_clients_tried++;
#ifdef CLIENT_APP_DEBUG
                _dpd.logMsg("Using %s from pattern match", tmp ? tmp->name:\n",ULL");
#endif
            }
        }
        else
        {
            break;
        }
    }
    FreeClientPatternList(&match_list);

}

int AppIdDiscoverClientApp(SFSnortPacket *p, int direction, tAppIdData *rnaData, const tAppIdConfig *pConfig)
{
    if (!pConfig->clientAppConfig.enabled)
        return APPID_SESSION_SUCCESS;

    if (direction == APP_ID_FROM_INITIATOR)
    {
        /* get out if we've already tried to validate a client app */
        if (!getAppIdFlag(rnaData, APPID_SESSION_CLIENT_DETECTED))
            ClientAppID(p, direction, rnaData, pConfig);
    }
    else if (rnaData->rnaServiceState != RNA_STATE_STATEFUL && getAppIdFlag(rnaData, APPID_SESSION_CLIENT_GETS_SERVER_PACKETS))
        ClientAppID(p, direction, rnaData, pConfig);

    return APPID_SESSION_SUCCESS;
}

DetectorAppUrlList *getAppUrlList(tAppIdConfig *pConfig)
{
    HttpPatternLists* patternLists = &pConfig->httpPatternLists;
    return (&patternLists->appUrlList);
}

static void *client_app_flowdata_get(tAppIdData *flowp, unsigned client_id)
{
    return AppIdFlowdataGet(flowp, client_id);
}

static int client_app_flowdata_add(tAppIdData *flowp, void *data, unsigned client_id, AppIdFreeFCN fcn)
{
    return AppIdFlowdataAdd(flowp, data, client_id, fcn);
}

