/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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
#include "client_app_base.h"
#include "str_search.h"

#include "common_util.h"
#include "client_app_api.h"
#include "client_app_base.h"

#include "client_app_smtp.h"
#include "client_app_msn.h"
#include "client_app_aim.h"
#include "client_app_ym.h"
#include "detector_sip.h"
#include "luaDetectorModule.h"
#include "luaDetectorApi.h"
#include "httpCommon.h"
#include "fw_appid.h"
#include "service_ssl.h"
#include "appIdConfig.h"

/*#define CLIENT_APP_DEBUG    1 */

#define BUFSIZE         512

typedef struct _RNA_CLIENT_APP_CONFIG
{
    int enabled;
    SF_LIST module_configs;
} RNA_CLIENT_APP_CONFIG;

static RNAClientAppRecord *tcp_client_app_list = NULL;
static RNAClientAppRecord *udp_client_app_list = NULL;

static void *client_app_flowdata_get(tAppIdData *flowp);
static int client_app_flowdata_add(tAppIdData *flowp, void *data, AppIdFreeFCN fcn);
static void AppIdAddClientApp(FLOW *flowp, tAppId service_id, tAppId id, const char *version);
static void AppIdAddClientAppInfo(FLOW *flowp, const char *info);

static const ClientAppApi client_app_api =
{
    .data_get = &client_app_flowdata_get,
    .data_add = &client_app_flowdata_add,
    .add_app= &AppIdAddClientApp,
    .add_info = &AppIdAddClientAppInfo,
    .add_user = &AppIdAddUser,
    .add_payload = &AppIdAddPayload
};

static void *tcp_patterns;
static int tcp_pattern_count;
static void *udp_patterns;
static int udp_pattern_count;

static RNA_CLIENT_APP_CONFIG ca_config;

static void LuaClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                        const uint8_t * const pattern, unsigned size,
                                        int position, struct _Detector *userData);
static void CClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                     const uint8_t * const pattern, unsigned size,
                                     int position);
static void CClientAppRegisterPatternNoCase(RNAClientAppFCN fcn, uint8_t proto,
                                            const uint8_t * const pattern, unsigned size,
                                            int position);

static InitClientAppAPI init_api =
{
    .RegisterPattern = &CClientAppRegisterPattern,
    .RegisterPatternEx = &LuaClientAppRegisterPattern,
    .RegisterPatternNoCase = &CClientAppRegisterPatternNoCase,
    .RegisterAppId = &appSetClientValidator
};

static CleanClientAppAPI clean_api =
{
};

static FinalizeClientAppAPI finalize_api =
{
};

typedef struct _PATTERN_DATA
{
    struct _PATTERN_DATA *next;
    int position;
    const RNAClientAppModule *ca;
} PatternData;

static PatternData *pattern_data_list;

extern RNAClientAppModule bit_client_mod;
extern RNAClientAppModule bit_tracker_client_mod;
extern RNAClientAppModule rtp_client_mod;
extern RNAClientAppModule ssh_client_mod;
extern RNAClientAppModule timbuktu_client_mod;
extern RNAClientAppModule tns_client_mod;
extern RNAClientAppModule vnc_client_mod;


static RNAClientAppModule *static_client_list[] =
{
    &smtp_client_mod,
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
    &vnc_client_mod
};

/*static const char * const MODULE_NAME = "ClientApp"; */

const ClientAppApi *getClientApi(void)
{
    return &client_app_api;
}

RNAClientAppModuleConfig *getClientAppModuleConfig(const char *moduleName)
{
    RNAClientAppModuleConfig *mod_config;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&ca_config.module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&ca_config.module_configs))
    {
        if (strcasecmp(mod_config->name, moduleName) == 0) break;
    }
    return mod_config;
}

const RNAClientAppModule *ClientAppGetClientAppModule(RNAClientAppFCN fcn, struct _Detector *userdata)
{
    RNAClientAppRecord *li;

    for (li=tcp_client_app_list; li; li=li->next)
    {
        if ((li->module->validate == fcn) && (li->module->userData == userdata))
            return li->module;
    }
    for (li=udp_client_app_list; li; li=li->next)
    {
        if ((li->module->validate == fcn) && (li->module->userData == userdata))
            return li->module;
    }
    return NULL;
}

void clientCreatePattern(RNAClientAppFCN fcn, uint8_t proto,
                         const uint8_t * const pattern, unsigned size,
                         int position, unsigned nocase, struct _Detector *userData,
                         const RNAClientAppModule *li)
{
    void **patterns;
    int *count;
    PatternData *pd;

    if (!li)
    {
        _dpd.errMsg( "Invalid client app when registering a pattern");
        return;
    }

    if (proto == IPPROTO_TCP)
    {
        patterns = &tcp_patterns;
        count = &tcp_pattern_count;
    }
    else if (proto == IPPROTO_UDP)
    {
        patterns = &udp_patterns;
        count = &udp_pattern_count;
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
        pd->next = pattern_data_list;
        pattern_data_list = pd;
        _dpd.searchAPI->search_instance_add_ex(*patterns, (const char *)pattern, size, pd, nocase);
    }
    else _dpd.errMsg( "Error allocating pattern data");
}

static void CClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                     const uint8_t * const pattern, unsigned size,
                                     int position)
{
    ClientAppRegisterPattern(fcn, proto, pattern, size, position, 0, NULL);
}

static void CClientAppRegisterPatternNoCase(RNAClientAppFCN fcn, uint8_t proto,
                                            const uint8_t * const pattern, unsigned size,
                                            int position)
{
    ClientAppRegisterPattern(fcn, proto, pattern, size, position, 1, NULL);
}

static void LuaClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                        const uint8_t * const pattern, unsigned size,
                                        int position, struct _Detector *userData)
{
    ClientAppRegisterPattern(fcn, proto, pattern, size, position, 0, userData);
}

void ClientAppRegisterPattern(RNAClientAppFCN fcn, uint8_t proto,
                                     const uint8_t * const pattern, unsigned size,
                                     int position, unsigned nocase, struct _Detector *userData)
{
    RNAClientAppRecord *list;
    RNAClientAppRecord *li;

    if (proto == IPPROTO_TCP)
        list = tcp_client_app_list;
    else if (proto == IPPROTO_UDP)
        list = udp_client_app_list;
    else
    {
        _dpd.errMsg("Invalid protocol when registering a pattern: %u\n",(unsigned)proto);
        return;
    }

    for (li=list; li; li=li->next)
    {
        if ((li->module->validate == fcn) && (li->module->userData == userData))
        {
            clientCreatePattern(fcn, proto, pattern, size, position, nocase, userData, li->module);
            break;
        }
    }
}

int clientAppLoadCallback(void *symbol)
{
    RNAClientAppModule *cam = (RNAClientAppModule *)symbol;
    RNAClientAppRecord * *list = NULL;
    RNAClientAppRecord *li;

    _dpd.debugMsg(DEBUG_LOG,"Adding client %s for protocol %u\n",cam->name, (unsigned)cam->proto);
    if (cam->proto == IPPROTO_TCP)
    {
        list = &tcp_client_app_list;
    }
    else if (cam->proto == IPPROTO_UDP)
    {
        list = &udp_client_app_list;
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
    }
    /*Can't set cam->userData to NULL because Lua detectors use it although C detectors don't */
    /*cam->userData = NULL; */
    return 0;
}


int LoadClientAppModules(const char **dir_list)
{
    unsigned i;

    for (i=0; i<sizeof(static_client_list)/sizeof(*static_client_list); i++)
    {
        if (clientAppLoadCallback(static_client_list[i]))
            return -1;
    }

    return 0;
}

static void AddModuleConfigItem(char *module_name, char *item_name, char *item_value)
{
    RNAClientAppModuleConfig *mod_config;
    RNAClientAppModuleConfigItem *item;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&ca_config.module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&ca_config.module_configs))
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
        if (sflist_add_tail(&ca_config.module_configs, mod_config))
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

static void ClientAppParseOption(RNA_CLIENT_APP_CONFIG *config,
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
        AddModuleConfigItem(key, &p[1], value);
        *p = ':';
    }
    else
    {
        _dpd.debugMsg(DEBUG_LOG, "Unknown client app argument ignored: key(%s) value(%s)",
                key, value);
    }
}

static int ClientAppParseArgs(RNA_CLIENT_APP_CONFIG *config, SF_LIST *args)
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
static void DisplayClientAppConfig(RNA_CLIENT_APP_CONFIG *config)
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

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&ca_config.module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&ca_config.module_configs))
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

static void initialize_module(RNAClientAppRecord *li)
{
    RNAClientAppModuleConfig *mod_config;
    int rval;

    for (mod_config = (RNAClientAppModuleConfig *)sflist_first(&ca_config.module_configs);
         mod_config;
         mod_config = (RNAClientAppModuleConfig *)sflist_next(&ca_config.module_configs))
    {
        if (strcasecmp(mod_config->name, li->module->name) == 0) break;
    }
    if (li->module->init && (rval=li->module->init(&init_api, mod_config ? &mod_config->items:NULL)) != CLIENT_APP_SUCCESS)
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


/**
 * Reconfigure the protocol id module
 *
 * @param args arguments
 */
void ReconfigureClientApp(void)
{
    RNAClientAppRecord *li;
    PatternData *pd;

    if (tcp_patterns) {
        _dpd.searchAPI->search_instance_free(tcp_patterns);
        tcp_patterns = NULL;
    }
    if (udp_patterns) {
        _dpd.searchAPI->search_instance_free(udp_patterns);
        udp_patterns = NULL;
    }

    while (pattern_data_list)
    {
        pd = pattern_data_list;
        pattern_data_list = pd->next;
        free((void *)pd);
    }

    for (li = tcp_client_app_list; li; li = li->next)
        clean_module(li);
    for (li = udp_client_app_list; li; li = li->next)
        clean_module(li);

    CleanHttpPatternLists(0);
    ssl_detector_free_patterns();

    ca_config.enabled = 1;

    sflist_static_free_all(&ca_config.module_configs, &free_module_config);

    ClientAppParseArgs(&ca_config, &appIdConfig.client_app_args);
    DisplayClientAppConfig(&ca_config);

    if (ca_config.enabled)
    {
        init_api.debug = appIdCommandConfig->app_id_debug;

        for (li = tcp_client_app_list; li; li = li->next)
            initialize_module(li);
        for (li = udp_client_app_list; li; li = li->next)
            initialize_module(li);

        luaModuleInitAllClients();

        for (li = tcp_client_app_list; li; li = li->next)
            finalize_module(li);
        for (li = udp_client_app_list; li; li = li->next)
            finalize_module(li);

        if (tcp_patterns)
        {
            _dpd.searchAPI->search_instance_prep(tcp_patterns);
        }
        if (udp_patterns)
        {
            _dpd.searchAPI->search_instance_prep(udp_patterns);
        }
    }
}


/**
 * Initialize the configuration of the client app module
 *
 * @param args
 */
void ClientAppInit(void)
{
    sflist_init(&ca_config.module_configs);
    ReconfigureClientApp();
}


typedef struct _CLIENT_APP_MATCH
{
    struct _CLIENT_APP_MATCH *next;
    unsigned count;
    const RNAClientAppModule *ca;
} ClientAppMatch;

static ClientAppMatch *match_free_list;

/**
 * Clean up the configuration of the client app module
 */
void CleanupClientApp(void)
{
#ifdef RNA_FULL_CLEANUP
    ClientAppMatch *match;
    PatternData *pd;
    RNAClientAppRecord *li;
#endif

#ifdef RNA_FULL_CLEANUP
    if (tcp_patterns) {
        search_instance_free(tcp_patterns);
        tcp_patterns = NULL;
    }
    if (udp_patterns) {
        search_instance_free(udp_patterns);
        udp_patterns = NULL;
    }
    while ((pd = pattern_data_list) != NULL)
    {
        pattern_data_list = pd->next;
        free(pd);
    }

    while ((li=tcp_client_app_list) != NULL)
    {
        tcp_client_app_list = li->next;
        if (li->module->clean)
            li->module->clean(&clean_api);
        free(li);
    }
    while ((li=udp_client_app_list) != NULL)
    {
        udp_client_app_list = li->next;
        if (li->module->clean)
            li->module->clean(&clean_api);
        free(li);
    }

    luaModuleCleanAllClients();

    CleanHttpPatternLists(1);
    ssl_detector_free_patterns();

    sflist_static_free_all(&ca_config.module_configs, &free_module_config);
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
    PatternData *pd = (PatternData *)id;
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

static void AppIdAddClientApp(FLOW *flowp, tAppId service_id, tAppId id, const char *version)
{
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

    flowp->clientServiceAppId = service_id;
    flowp->clientAppId = id;
}

static void AppIdAddClientAppInfo(FLOW *flowp, const char *info)
{
    if (flowp->url)
        return;
    flowp->url = strdup(info);
    if (!flowp->url)
        _dpd.errMsg("failed to allocate url");
}

static void getAppidByClientPattern(const SFSnortPacket *pkt, uint32_t protocol, const RNAClientAppModule **rnaData)
{
    ClientAppMatch *match_list = NULL;
    void *patterns;
    ClientAppMatch *cam;
    ClientAppMatch *tmp;
    unsigned max_count;
    unsigned max_precedence;

    *rnaData = NULL;

    if (protocol == IPPROTO_TCP)
        patterns = tcp_patterns;
    else
        patterns = udp_patterns;

    if (!patterns)
        return;

    _dpd.searchAPI->search_instance_find_all(patterns,
            (char *)pkt->payload,
            pkt->payload_size,
            0,
            &pattern_match, (void*)&match_list
            );
    cam = match_list;
    max_count = 0;
    max_precedence = 0;
    while (cam)
    {
        if (cam->count >= cam->ca->minimum_matches
                && ((cam->count > max_count)
                    || (cam->count == max_count && cam->ca->precedence > max_precedence)))
        {
            max_count = cam->count;
            max_precedence = cam->ca->precedence;
            *rnaData = cam->ca;
        }
        tmp = cam;
        cam = tmp->next;
        tmp->next = match_free_list;
        match_free_list = tmp;
    }
}

/**
 * The process to determine the running client app given the packet data.
 *
 * @param p packet to process
 */
static void ClientAppID(const SFSnortPacket *p, const int dir, FLOW *flowp)
{
    const RNAClientAppModule *client = NULL;
    CLIENT_APP_RETCODE ret;

#ifdef CLIENT_APP_DEBUG
    _dpd.logMsg( "Client");
#endif

    if (!p->payload_size)
        return;

    if ((client = flowp->clientData))
    {
#ifdef CLIENT_APP_DEBUG
        _dpd.logMsg("Using %s from flow state", client ? client->name:\n",ULL");
#endif
    }
    else
    {
        getAppidByClientPattern(p, flowp->proto, &client);
        flowp->clientData = client;
#ifdef CLIENT_APP_DEBUG
        _dpd.logMsg("Using %s from pattern match", client ? client->name:\n",ULL");
#endif
    }

    if (client)
    {
        ret = client->validate((uint8_t *)p->payload,
                                       p->payload_size, dir,
                                       flowp, p, client->userData);
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s %s client detector returned %d\n", app_id_debug_session,
                        client->name ? client->name:"UNKNOWN", ret);
        if (ret < 0)
            flow_mark(flowp, FLOW_CLIENTAPPDETECTED);
    }
    else
        flow_mark(flowp, FLOW_CLIENTAPPDETECTED);
}

int AppIdDiscoverClientApp(const SFSnortPacket *p, int direction, tAppIdData *rnaData)
{
    if (!ca_config.enabled)
        return FLOW_SUCCESS;

    if (direction == APP_ID_FROM_INITIATOR)
    {
        /* get out if we've already tried to validate a client app */
        if (!flow_checkflag(rnaData, FLOW_CLIENTAPPDETECTED))
            ClientAppID(p, direction, rnaData);
    }
    else if (rnaData->rnaServiceState != RNA_STATE_STATEFUL && flow_checkflag(rnaData, FLOW_CLIENT_GETS_SERVER_PACKETS))
        ClientAppID(p, direction, rnaData);

    return FLOW_SUCCESS;
}

DetectorAppUrlList *getAppUrlList(void)
{
    HttpPatternLists* patternLists = &httpPatternLists;
    return (&patternLists->appUrlList);
}

static void *client_app_flowdata_get(tAppIdData *flowp)
{
    return AppIdFlowdataGet(flowp, FLOW_DATA_CLIENT_APP_MODSTATE);
}

static int client_app_flowdata_add(tAppIdData *flowp, void *data, AppIdFreeFCN fcn)
{
    return AppIdFlowdataAdd(flowp, data, FLOW_DATA_CLIENT_APP_MODSTATE, fcn);
}

