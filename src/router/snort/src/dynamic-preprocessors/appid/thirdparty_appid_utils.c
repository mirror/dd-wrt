/****************************************************************************
 *
 * Copyright (C) 2015-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

#include "thirdparty_appid_utils.h"

#include <stdbool.h>
#include <stdint.h>

#include "sf_dynamic_preprocessor.h"
#include "commonAppMatcher.h"

#define MODULE_SYMBOL "thirdparty_appid_impl_module"

static _PluginHandle module_handle = NULL;
static struct ThirdPartyConfig thirdpartyConfig;
ThirdPartyAppIDModule* thirdparty_appid_module = NULL;

static int LoadCallback(struct _SnortConfig *sc, const char * const path, int indent)
{
    _PluginHandle handle_tmp;
    ThirdPartyAppIDModule* module_tmp;
    DynamicPluginMeta meta;

    if (thirdparty_appid_module != NULL)
    {
        _dpd.errMsg("Ignoring additional 3rd party AppID module (%s)!\n", path ? : "");
        return 0;
    }

    handle_tmp = _dpd.openDynamicLibrary(path, 0);
    if (handle_tmp == NULL)
    {
        _dpd.errMsg("Could not load 3rd party AppID module (%s)!\n", path ? : "");
        return 0;
    }
    meta.libraryPath = (char *)path;

    module_tmp = (ThirdPartyAppIDModule*)_dpd.getSymbol(handle_tmp, MODULE_SYMBOL, &meta, 1);
    if (module_tmp == NULL)
    {
        _dpd.errMsg("Ignoring invalid 3rd party AppID module (%s)!\n", path ? : "");
        _dpd.closeDynamicLibrary(handle_tmp);
        return 0;
    }

    if (    (module_tmp->api_version != THIRD_PARTY_APP_ID_API_VERSION)
         || ((module_tmp->module_name == NULL) || (module_tmp->module_name[0] == 0))
         || (module_tmp->init == NULL)
         || (module_tmp->fini == NULL)
         || (module_tmp->session_create == NULL)
         || (module_tmp->session_delete == NULL)
         || (module_tmp->session_process == NULL)
         || (module_tmp->print_stats == NULL)
         || (module_tmp->reset_stats == NULL)
         || (module_tmp->disable_flags == NULL) )
    {
        _dpd.errMsg("Ignoring incomplete 3rd party AppID module (%s)!\n", path ? : "");
        _dpd.closeDynamicLibrary(handle_tmp);
        return 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "Found 3rd party AppID module (%s).\n", module_tmp->module_name ? : ""););
    module_handle = handle_tmp;
    thirdparty_appid_module = module_tmp;
    return 0;
}

static void getXffFields(void)
{
    static char* defaultXffFields[] = {HTTP_XFF_FIELD_X_FORWARDED_FOR, HTTP_XFF_FIELD_TRUE_CLIENT_IP};
    char** xffFields;
    int i;

    xffFields = _dpd.getHttpXffFields(&thirdpartyConfig.numXffFields);
    if (!xffFields)
    {
        xffFields = defaultXffFields;
        thirdpartyConfig.numXffFields = sizeof(defaultXffFields) / sizeof(defaultXffFields[0]);
    }
    thirdpartyConfig.xffFields = malloc(thirdpartyConfig.numXffFields * sizeof(char*));
    if(!thirdpartyConfig.xffFields)
    {
         _dpd.errMsg("getXffFields: Failed to allocate memory for xffFields in thirdpartyConfig\n");
    }
    for (i = 0; i < thirdpartyConfig.numXffFields; i++)
        thirdpartyConfig.xffFields[i] = strndup(xffFields[i], UINT8_MAX);
}

void ThirdPartyAppIDInit(struct AppidStaticConfig *appidStaticConfig)
{
    const char* thirdparty_appid_dir = appidStaticConfig->appid_thirdparty_dir;
    int ret;
    const char* dir = NULL;
    struct ThirdPartyUtils thirdpartyUtils;

    if (thirdparty_appid_module != NULL)
    {
        return;
    }

    if ((thirdparty_appid_dir == NULL) || (thirdparty_appid_dir[0] == 0))
    {
        return;
    }
    else
    {
        dir = thirdparty_appid_dir;
    }

    _dpd.loadAllLibs(NULL, dir, LoadCallback);
    if (thirdparty_appid_module == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "No 3rd party AppID module loaded.\n"););
        return;
    }

    memset(&thirdpartyConfig, 0, sizeof(thirdpartyConfig));
    thirdpartyConfig.chp_body_collection_max = appidStaticConfig->chp_body_collection_max;
    thirdpartyConfig.ftp_userid_disabled = appidStaticConfig->ftp_userid_disabled;
    thirdpartyConfig.chp_body_collection_disabled = appidStaticConfig->chp_body_collection_disabled;
    thirdpartyConfig.tp_allow_probes = appidStaticConfig->tp_allow_probes;
    if (appidStaticConfig->http2_detection_enabled)
        thirdpartyConfig.http_upgrade_reporting_enabled = 1;
    else
        thirdpartyConfig.http_upgrade_reporting_enabled = 0;

    if (appidStaticConfig->tp_config_path)
    {
        strncpy(thirdpartyConfig.tp_config_path, appidStaticConfig->tp_config_path, TP_PATH_MAX);
        thirdpartyConfig.tp_config_path[TP_PATH_MAX-1] = '\0';
    }
    else
        thirdpartyConfig.tp_config_path[0] = '\0'; // use default path

    thirdpartyUtils.logMsg           = _dpd.logMsg;
    thirdpartyUtils.getSnortInstance = _dpd.getSnortInstance;

    getXffFields();

    ret = thirdparty_appid_module->init(&thirdpartyConfig, &thirdpartyUtils);
    if (ret != 0)
    {
        _dpd.errMsg("Unable to initialize 3rd party AppID module (%d)!\n", ret);
        _dpd.closeDynamicLibrary(module_handle);
        module_handle = NULL;
        thirdparty_appid_module = NULL;
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "3rd party AppID module loaded and initialized OK (%s).\n", thirdparty_appid_module->module_name ? : ""););
}

void ThirdPartyAppIDReconfigure(void)
{
    int ret;
    int i;

    if (thirdparty_appid_module == NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "No 3rd party AppID module loaded.\n"););
        return;
    }

    thirdpartyConfig.oldNumXffFields = thirdpartyConfig.numXffFields;
    thirdpartyConfig.oldXffFields = thirdpartyConfig.xffFields;

    getXffFields();

    ret = thirdparty_appid_module->reconfigure(&thirdpartyConfig);

    for (i = 0; i < thirdpartyConfig.oldNumXffFields; i++)
        free(thirdpartyConfig.oldXffFields[i]);
    free(thirdpartyConfig.oldXffFields);

    if (ret != 0)
    {
        _dpd.errMsg("Unable to reconfigure 3rd party AppID module (%d)!\n", ret);
        return;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_APPID, "3rd party AppID module reconfigured OK (%s).\n", thirdparty_appid_module->module_name ? : ""););
}

void ThirdPartyAppIDFini(void)
{
    int ret;
    int i;

    if (thirdparty_appid_module != NULL)
    {
        ret = thirdparty_appid_module->fini();

        for (i = 0; i < thirdpartyConfig.numXffFields; i++)
            free(thirdpartyConfig.xffFields[i]);
        free(thirdpartyConfig.xffFields);

        if (ret != 0)
        {
            _dpd.errMsg("Could not finalize 3rd party AppID module (%d)!\n", ret);
        }

        _dpd.closeDynamicLibrary(module_handle);
        module_handle = NULL;
        thirdparty_appid_module = NULL;

        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "3rd party AppID module finalized and unloaded OK.\n"););
    }
}
