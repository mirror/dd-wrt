/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
 *
 ****************************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_dynamic_define.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_meta.h"
#include "detection_lib_meta.h"
#include "stdio.h"
#include "string.h"
#include "sfsnort_dynamic_detection_lib.h"

extern Rule *rules[];

DETECTION_LINKAGE int InitializeDetection(struct _SnortConfig *sc)
{
    return RegisterRules(sc, rules);
}

DETECTION_LINKAGE int DumpSkeletonRules(void)
{
    return DumpRules(DETECTION_LIB_NAME, rules);
}


DETECTION_LINKAGE int LibVersion(DynamicPluginMeta *dpm)
{
    dpm->type  = TYPE_DETECTION;
    dpm->major = DETECTION_LIB_MAJOR;
    dpm->minor = DETECTION_LIB_MINOR;
    dpm->build = DETECTION_LIB_BUILD;
    strncpy(dpm->uniqueName, DETECTION_LIB_NAME, MAX_NAME_LEN-1);
    dpm->uniqueName[MAX_NAME_LEN-1] = '\0';
    return 0;
}

DETECTION_LINKAGE int EngineVersion(DynamicPluginMeta *dpm)
{

    dpm->type  = TYPE_ENGINE;
    dpm->major = REQ_ENGINE_LIB_MAJOR;
    dpm->minor = REQ_ENGINE_LIB_MINOR;
    dpm->build = 0;
    strncpy(dpm->uniqueName, REQ_ENGINE_LIB_NAME, MAX_NAME_LEN-1);
    dpm->uniqueName[MAX_NAME_LEN-1] = '\0';
    return 0;
}
