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


#include <glob.h>

#include "commonAppMatcher.h"
#include "common_util.h"
#include "httpCommon.h"
#include "service_state.h"
#include "service_base.h"
#include "service_api.h"
#include "client_app_base.h"
#include "client_app_api.h"
#include "detector_base.h"
#include "detector_api.h"
#include "detector_http.h"
#include "service_ssl.h"
#include "luaDetectorModule.h"
#include "fw_appid.h"
#include "hostPortAppCache.h"
#include "appInfoTable.h"
#include "appIdStats.h"
#include "appIdConfig.h"

unsigned appIdPolicyId;
tAppIdConfig appIdConfig;

/*static const char * const MODULE_NAME = "AppMatcher"; */
#define MAX_DISPLAY_SIZE   65536

static void DisplayConfig(tAppIdConfig *aic)
{
    if (aic->appid_directory)
    {
        _dpd.logMsg("   AppId Dir: %s\n", aic->appid_directory);
    }
}

static void DisplayPortConfig(tAppIdConfig *aic)
{
    unsigned i;
    int first;

    first = 1;
    for (i=0; i<sizeof(aic->tcp_port_only)/sizeof(*aic->tcp_port_only); i++)
    {
        if (aic->tcp_port_only[i])
        {
            if (first)
            {
                _dpd.logMsg("    TCP Port-Only Services\n");
                first = 0;
            }
            _dpd.logMsg("        %5u - %u\n", i, aic->tcp_port_only[i]);
        }
    }

    first = 1;
    for (i=0; i<sizeof(aic->udp_port_only)/sizeof(*aic->udp_port_only); i++)
    {
        if (aic->udp_port_only[i])
        {
            if (first)
            {
                _dpd.logMsg("    UDP Port-Only Services\n");
                first = 0;
            }
            _dpd.logMsg("        %5u - %u\n", i, aic->udp_port_only[i]);
        }
    }
}

typedef struct _PORT
{
    struct _PORT *next;
    uint16_t port;
} Port;

static void ReadPortDetectors(tAppIdConfig *aic, const char *files)
{
    int rval;
    glob_t globs;
    char pattern[PATH_MAX];
    uint32_t n;

    snprintf(pattern, sizeof(pattern), "%s/%s", appIdCommandConfig->app_id_detector_path, files);

    memset(&globs, 0, sizeof(globs));
    rval = glob(pattern, 0, NULL, &globs);
    if (rval != 0 && rval != GLOB_NOMATCH)
    {
        _dpd.errMsg("Unable to read directory '%s'\n",pattern);
        return;
    }

    for(n = 0; n < globs.gl_pathc; n++)
    {
        FILE *file;
        unsigned proto = 0;
        tAppId appId = APP_ID_NONE;
        char line[1024];
        Port *port = NULL;
        Port *tmp_port;

        if ((file = fopen(globs.gl_pathv[n], "r")) == NULL)
        {
            _dpd.errMsg("Unable to read port service '%s'\n",globs.gl_pathv[n]);
            continue;
        }

        while (fgets(line, sizeof(line), file))
        {
            char *key, *value, *p;
            size_t len;

            len = strlen(line);
            for (; len && (line[len - 1] == '\n' || line[len - 1] == '\r'); len--)
                line[len - 1] = 0;

            /* find key/value for lines of the format "key: value\n" */
            if ((value = strchr(line, ':')))
            {
                key = line;
                *value = '\0';
                value++;
                for (; *value && *value == ' '; value++);

                if (strcasecmp(key, "ports") == 0)
                {
                    char *context = NULL;
                    char *ptr;
                    unsigned long tmp;

                    for (ptr = strtok_r(value, ",", &context); ptr; ptr = strtok_r(NULL, ",", &context))
                    {
                        for (; *ptr && *ptr == ' '; ptr++);
                        len = strlen(ptr);
                        for (; len && ptr[len - 1] == ' '; len--)
                            ptr[len - 1] = 0;
                        tmp = strtoul(ptr, &p, 10);
                        if (!*ptr || *p || !tmp || tmp > 65535)
                        {
                            _dpd.errMsg("Invalid port, '%s', in lua detector '%s'\n",ptr, globs.gl_pathv[n]);
                            goto next;
                        }
                        if ((tmp_port = calloc(1, sizeof(*tmp_port))) == NULL)
                        {
                            _dpd.errMsg( "Failed to allocate a port struct");
                            goto next;
                        }
                        tmp_port->port = (uint16_t)tmp;
                        tmp_port->next = port;
                        port = tmp_port;
                    }
                }
                else if (strcasecmp(key, "protocol") == 0)
                {
                    if (strcasecmp(value, "tcp") == 0)
                        proto = 1;
                    else if (strcasecmp(value, "udp") == 0)
                        proto = 2;
                    else if (strcasecmp(value, "tcp/udp") == 0)
                        proto = 3;
                    else
                    {
                        _dpd.errMsg("Invalid protocol, '%s', in port service '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                }
                else if (strcasecmp(key, "appId") == 0)
                {
                    appId = (tAppId)strtoul(value, &p, 10);
                    if (!*value || *p || appId <= APP_ID_NONE)
                    {
                        _dpd.errMsg("Invalid app ID, '%s', in port service '%s'\n",value, globs.gl_pathv[n]);
                        goto next;
                    }
                }
            }
        }
        if (port && proto && appId > APP_ID_NONE)
        {
            while ((tmp_port = port))
            {
                port = tmp_port->next;
                if (proto & 1)
                    aic->tcp_port_only[tmp_port->port] = appId;
                if (proto & 2)
                    aic->udp_port_only[tmp_port->port] = appId;

                free(tmp_port);
                appInfoSetActive(appId, true);
            }
        }
        else
            _dpd.errMsg("Missing parameter(s) in port service '%s'\n",globs.gl_pathv[n]);

next:;
        while ((tmp_port = port))
        {
            port = tmp_port->next;
            free(tmp_port);
        }
        fclose(file);
    }

    globfree(&globs);
}

#define CISCO_PORT_DETECTORS "odp/port/*yaml"
#define CUSTOM_PORT_DETECTORS "custom/port/*yaml"

static void ReadPorts(tAppIdConfig *aic)
{
    memset(aic->tcp_port_only, 0, sizeof(aic->tcp_port_only));
    memset(aic->udp_port_only, 0, sizeof(aic->udp_port_only));
    ReadPortDetectors(aic, CISCO_PORT_DETECTORS);
    ReadPortDetectors(aic, CUSTOM_PORT_DETECTORS);
}

static void LoadModules(uint32_t instance_id)
{
    if (LoadServiceModules(NULL, instance_id))
        exit(-1);

    if (LoadClientAppModules(NULL))
        exit(-1);

    if (LoadDetectorModules(NULL))
        exit(-1);
}

typedef enum
{

    RNA_FW_CONFIG_STATE_UNINIT,
    RNA_FW_CONFIG_STATE_INIT,
    RNA_FW_CONFIG_STATE_PENDING,

} tRnaFwConfigState;

static tRnaFwConfigState rnaFwConfigState = RNA_FW_CONFIG_STATE_UNINIT;

static void ConfigItemFree(ConfigItem *ci)
{
    if (ci)
    {
        if (ci->name)
            free(ci->name);
        if (ci->value)
            free(ci->value);
        free(ci);
    }
}

static void AppIdCleanupConfig(void)
{
    sflist_static_free_all(&appIdConfig.client_app_args, (void (*)(void*))ConfigItemFree);
}

int AppIdCommonInit(unsigned long memcap)
{
    if (rnaFwConfigState == RNA_FW_CONFIG_STATE_UNINIT)
    {
        appIdPolicyId = 53;
        rnaFwConfigState = RNA_FW_CONFIG_STATE_PENDING;
        sflist_init(&appIdConfig.client_app_args);
        luaModuleInit();
        appInfoTableInit(appIdCommandConfig->app_id_detector_path);
        ReadPorts(&appIdConfig);
        LoadModules(0);
        hostPortAppCacheInit();
        luaDetectorsLoad();
        ClientAppInit();
        ServiceInit();
        http_detector_finalize();
        sipUaFinalize();
        ssl_detector_process_patterns();
        appIdStatsInit(appIdCommandConfig->app_stats_filename, appIdCommandConfig->app_stats_period, 
                appIdCommandConfig->app_stats_rollover_size, appIdCommandConfig->app_stats_rollover_time);
        DisplayConfig(&appIdConfig);
        DisplayPortConfig(&appIdConfig);
        if (AppIdServiceStateInit(memcap))
            exit(-1);
        rnaFwConfigState = RNA_FW_CONFIG_STATE_INIT;
        return 0;
    }
    return -1;
}

int AppIdCommonFini(void)
{
    if (rnaFwConfigState == RNA_FW_CONFIG_STATE_INIT)
    {
        rnaFwConfigState = RNA_FW_CONFIG_STATE_PENDING;

        if (appIdConfig.appid_directory)
        {
            free((void*)appIdConfig.appid_directory);
            appIdConfig.appid_directory = NULL;
        }

        CleanupServices();
        CleanupClientApp();
        luaModuleFini();
        hostPortAppCacheFini();
        AppIdServiceStateCleanup();
        appIdStatsFini();
        fwAppIdFini();
        http_detector_clean();
        service_ssl_clean();
        rnaFwConfigState = RNA_FW_CONFIG_STATE_UNINIT;
        return 0;
    }
    return -1;
}

int AppIdCommonReload()
{
    if (rnaFwConfigState == RNA_FW_CONFIG_STATE_INIT)
    {
        rnaFwConfigState = RNA_FW_CONFIG_STATE_PENDING;
        AppIdCleanupConfig();
        http_detector_clean();
        hostPortAppCacheFini();
        service_ssl_clean();
        appInfoTableInit(appIdCommandConfig->app_id_detector_path);
        ReadPorts(&appIdConfig);
        hostPortAppCacheInit();
        luaDetectorsLoad();
        ReconfigureClientApp();
        ReconfigureServices();
        http_detector_finalize();
        sipUaFinalize();
        ssl_detector_process_patterns();
        appIdStatsReinit();
        appIdPolicyId++;
        DisplayConfig(&appIdConfig);
        DisplayPortConfig(&appIdConfig);
        rnaFwConfigState = RNA_FW_CONFIG_STATE_INIT;
        return 0;
    }
    return -1;
}

