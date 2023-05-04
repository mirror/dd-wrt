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



#include <stdint.h>
#include <stdbool.h>
#include <strings.h>
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <inttypes.h>

#include "sf_dynamic_preprocessor.h"
#include "appIdConfig.h"
#include "common_util.h"
#ifdef SIDE_CHANNEL
#include "appId_ss.h"
#endif

#define APP_ID_MEMCAP_DEFAULT       (256*1024*1024ULL)
#define APP_ID_MEMCAP_UPPER_BOUND   (3*1024*1024*1024ULL)
#define APP_ID_MEMCAP_LOWER_BOUND   (32*1024*1024ULL)
#define APP_ID_MEMCAP_ABSOLUTE_LOWER_BOUND (1*1024*1024UL + 16UL)

#define DEFAULT_APPID_DETECTOR_PATH "/usr/local/etc/appid"

static void appIdConfigDump(tAppidStaticConfig* appidSC)
{
    _dpd.logMsg("AppId Configuration\n");

    _dpd.logMsg("    Detector Path:          %s\n", appidSC->app_id_detector_path ? appidSC->app_id_detector_path : "NULL");
    _dpd.logMsg("    appStats Files:         %s\n", appidSC->app_stats_filename ? appidSC->app_stats_filename : "NULL");
    _dpd.logMsg("    appStats Period:        %d secs\n", appidSC->app_stats_period);
    _dpd.logMsg("    appStats Rollover Size: %d bytes\n", appidSC->app_stats_rollover_size);
    _dpd.logMsg("    appStats Rollover time: %d secs\n", appidSC->app_stats_rollover_time);

#ifdef SIDE_CHANNEL
     AppIdPrintSSConfig(appidSC->appId_ss_config);
#endif

#ifdef REG_TEST
    _dpd.logMsg("    AppID Reg Test Mode:    %s\n", appidSC->appid_reg_test_mode ? "true" : "false");
#endif
    _dpd.logMsg("\n");
}

void appIdConfigParse(tAppidStaticConfig* appidSC, char *args)
{
    char **toks;
    int num_toks;
    int i;
    char **stoks;
    int s_toks;
    char *endPtr;
    char *ro_app_detector_dir;

    memset (appidSC, 0, sizeof(*appidSC));

#ifdef SIDE_CHANNEL
    if (NULL == (appidSC->appId_ss_config = (AppIdSSConfig *)_dpd.snortAlloc(1,
        sizeof(*appidSC->appId_ss_config), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)))
    {
        _dpd.fatalMsg("Appid failed to allocate memory for state sharing configuration\n");
    }
    if (_dpd.isSCEnabled())
    {
        appidSC->appId_ss_config->use_side_channel = true;
    }
#endif

    if ((args == NULL) || (strlen(args) == 0))
        return;

    ro_app_detector_dir = getenv("APPID_DETECTOR_DIR");
    if (ro_app_detector_dir)
    {   
        if (NULL == (appidSC->app_id_detector_path = strdup(ro_app_detector_dir)))
        {   
            _dpd.fatalMsg("Appid failed to allocate RO detector path\n");
        }
    }

    toks = _dpd.tokenSplit(args, ",", 0, &num_toks, 0);
    i = 0;

    for (i = 0; i < num_toks; i++)
    {
        stoks = _dpd.tokenSplit(toks[i], " ", 2, &s_toks, 0);

        if (s_toks == 0)
        {
            _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Missing AppId configuration");
        }

        if(!strcasecmp(stoks[0], "conf"))
        {
            if ((s_toks != 2) || !*stoks[1])
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid rna_conf");
            }

            if (NULL == (appidSC->conf_file = strdup(stoks[1])))
            {
                _dpd.fatalMsg("Appid failed to allocate configuration file name\n");
            }
        }
        else if(!strcasecmp(stoks[0], "debug"))
        {
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid debug");
            }
            if (!strcasecmp(stoks[1], "yes"))
                appidSC->app_id_debug = 1;
        }
        else if(!strcasecmp(stoks[0], "dump_ports"))
        {
            if (s_toks > 1)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid dump ports specified");
            }
            appidSC->app_id_dump_ports = 1;
        }
        else if(!strcasecmp(stoks[0], "memcap"))
        {
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid memcap");
            }

            appidSC->memcap = strtoul(stoks[1], &endPtr, 10);
            if (!*stoks[1] || *endPtr)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid memcap");
            }

            if (appidSC->memcap == 0)
                appidSC->memcap = APP_ID_MEMCAP_LOWER_BOUND;
            if (appidSC->memcap > APP_ID_MEMCAP_UPPER_BOUND)
                appidSC->memcap = APP_ID_MEMCAP_UPPER_BOUND;
            if (APP_ID_MEMCAP_ABSOLUTE_LOWER_BOUND > appidSC->memcap)
            {
                _dpd.errMsg("AppId invalid memory cap, %lu, overridden with %lu",
                            appidSC->memcap, APP_ID_MEMCAP_ABSOLUTE_LOWER_BOUND);
                appidSC->memcap = APP_ID_MEMCAP_ABSOLUTE_LOWER_BOUND;
            }
        }
        else if(!strcasecmp(stoks[0], "app_stats_filename"))
        {
            if ((s_toks != 2) || !*stoks[1])
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid stats_filename");
            }

            if (NULL == (appidSC->app_stats_filename = strdup(stoks[1])))
            {
                _dpd.fatalMsg("Appid failed to allocate stats file name\n");
            }
        }
        else if(!strcasecmp(stoks[0], "app_stats_period"))
        {
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_stats_period");
            }

            appidSC->app_stats_period = strtoul(stoks[1], &endPtr, 10);
            if (!*stoks[1] || *endPtr)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_stats_period");
            }
        }
        else if(!strcasecmp(stoks[0], "app_stats_rollover_size"))
        {
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_stats_rollover_size");
            }

            appidSC->app_stats_rollover_size = strtoul(stoks[1], &endPtr, 10);
            if (!*stoks[1] || *endPtr)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_stats_rollover_size");
            }
        }
        else if(!strcasecmp(stoks[0], "app_stats_rollover_time"))
        {
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_stats_rollover_time");
            }

            appidSC->app_stats_rollover_time = strtoul(stoks[1], &endPtr, 10);
            if (!*stoks[1] || *endPtr)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_stats_rollover_time");
            }
        }
        else if(!strcasecmp(stoks[0], "app_detector_dir"))
        {
            if (!ro_app_detector_dir)
            {
                if ((s_toks != 2) || !*stoks[1])
                {
                    _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid app_detector_dir");
                }

                if (NULL == (appidSC->app_id_detector_path = strdup(stoks[1])))
                {
                    _dpd.fatalMsg("Appid failed to allocate detector path\n");
                }
            }
        }
        else if(!strcasecmp(stoks[0], "instance_id"))
        {
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid instance id");
            }
            appidSC->instance_id = strtoul(stoks[1], &endPtr, 10);
            if (!*stoks[1] || *endPtr)
            {
                _dpd.fatalMsg("Invalid instance id specified");
            }
        }
        else if(!strcasecmp(stoks[0], "thirdparty_appid_dir"))
        {
            if (appidSC->appid_thirdparty_dir)
            {
                free((void *)appidSC->appid_thirdparty_dir);
                appidSC->appid_thirdparty_dir = NULL;
            }
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid ThirdpartyDirectory");
            }
            if (!(appidSC->appid_thirdparty_dir = strdup(stoks[1])))
            {
                _dpd.errMsg("Failed to allocate a module directory");
                return;
            }
        }
        else if(!strcasecmp(stoks[0], "tp_config_path"))
        {
            if (appidSC->tp_config_path)
            {
                free((void *)appidSC->tp_config_path);
                appidSC->tp_config_path = NULL;
            }
            if (s_toks != 2)
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid TP configuration");
            }
            if (!(appidSC->tp_config_path = strdup(stoks[1])))
            {
                _dpd.errMsg("Failed to allocate a module file");
                return;
            }
        }
#ifdef REG_TEST
#ifdef SIDE_CHANNEL
        else if(!strcasecmp(stoks[0], "ss_startup_input_file"))
        {
            if ((s_toks != 2) || !*stoks[1])
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid AppId state sharing startup input file");
            }

            if (NULL == (appidSC->appId_ss_config->startup_input_file = strdup(stoks[1])))
            {
                _dpd.fatalMsg("Appid failed to allocate memory for state sharing startup input file\n");
            }
        }
        else if(!strcasecmp(stoks[0], "ss_runtime_output_file"))
        {
            if ((s_toks != 2) || !*stoks[1])
            {
                _dpd.fatalMsg("%s(%d) => %s\n", *(_dpd.config_file), *(_dpd.config_line), "Invalid AppId state sharing runtime output file");
            }

            if (NULL == (appidSC->appId_ss_config->runtime_output_file = strdup(stoks[1])))
            {
                _dpd.fatalMsg("Appid failed to allocate memory for state sharing runtime output file\n");
            }
        }
#endif
        else if(!strcasecmp(stoks[0], "appid_reg_test_mode"))
        {
            appidSC->appid_reg_test_mode = true;
        }
#endif
        else
        {
            DynamicPreprocessorFatalMessage("%s(%d) => Unknown AppId configuration option \"%s\"\n",
                                            *(_dpd.config_file), *(_dpd.config_line), toks[i]);
        }

        _dpd.tokenFree(&stoks, s_toks);
    }

    if (!appidSC->memcap)
        appidSC->memcap = APP_ID_MEMCAP_DEFAULT;
    if (!appidSC->app_stats_period)
        appidSC->app_stats_period = 5*60;
    if (!appidSC->app_stats_rollover_size)
        appidSC->app_stats_rollover_size = 20 * 1024 * 1024;
    if (!appidSC->app_stats_rollover_time)
        appidSC->app_stats_rollover_time = 24*60*60;

    if (!appidSC->app_id_detector_path)
    {
        if (NULL == (appidSC->app_id_detector_path = strdup(DEFAULT_APPID_DETECTOR_PATH)))
        {
            _dpd.fatalMsg("Appid failed to allocate detector path\n");
        }
    }

    _dpd.tokenFree(&toks, num_toks);

    appIdConfigDump(appidSC);
}

void AppIdAddGenericConfigItem(tAppIdConfig *pConfig, const char *name, void *pData)
{
    tAppidGenericConfigItem *pConfigItem;

    if (!(pConfigItem = (tAppidGenericConfigItem*)_dpd.snortAlloc(1,
        sizeof(*pConfigItem), PP_APP_ID, PP_MEM_CATEGORY_CONFIG)) ||
        !(pConfigItem->name = strdup(name)))
    {
        if (pConfigItem)
            _dpd.snortFree(pConfigItem, sizeof(*pConfigItem), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
        _dpd.errMsg("Failed to allocate a config item.");
        return;
    }
    pConfigItem->pData = pData;
    sflist_add_tail(&pConfig->genericConfigList, pConfigItem);
}

void *AppIdFindGenericConfigItem(const tAppIdConfig *pConfig, const char *name)
{
    tAppidGenericConfigItem *pConfigItem;

    // Search a module's configuration by its name
    for (pConfigItem = (tAppidGenericConfigItem *) sflist_first((SF_LIST*)&pConfig->genericConfigList);
         pConfigItem;
         pConfigItem = (tAppidGenericConfigItem *) sflist_next((SF_LIST*)&pConfig->genericConfigList))
    {
        if (strcmp(pConfigItem->name, name) == 0)
        {
            return pConfigItem->pData;
        }
    }

    return NULL;
}

void AppIdRemoveGenericConfigItem(tAppIdConfig *pConfig, const char *name)
{
    SF_LNODE                *pNode;

    // Search a module's configuration by its name
    for (pNode = sflist_first_node(&pConfig->genericConfigList);
         pNode;
         pNode = sflist_next_node(&pConfig->genericConfigList))
    {
        tAppidGenericConfigItem *pConfigItem = SFLIST_NODE_TO_DATA(pNode);
        if (strcmp(pConfigItem->name, name) == 0)
        {
            free(pConfigItem->name);
            _dpd.snortFree(pConfigItem, sizeof(*pConfigItem), PP_APP_ID, PP_MEM_CATEGORY_CONFIG);
            sflist_remove_node(&pConfig->genericConfigList, pNode);
            break;
        }
    }
}


