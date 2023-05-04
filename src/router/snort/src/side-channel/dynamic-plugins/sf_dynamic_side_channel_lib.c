/*
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
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
 *
 * Author: Michael Altizer <maltizer@sourcefire.com>
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "sf_dynamic_define.h"
#include "sf_dynamic_meta.h"
#include "sf_dynamic_common.h"
#include "sf_dynamic_side_channel_lib.h"
#include "sf_dynamic_side_channel.h"
#include "sf_side_channel_info.h"

DynamicSideChannelData _dscd;

SIDE_CHANNEL_LINKAGE int InitializeSideChannel(DynamicSideChannelData *dscd)
{
    if (dscd->version < SIDE_CHANNEL_DATA_VERSION)
    {
        printf("ERROR version %d < %d\n", dscd->version, SIDE_CHANNEL_DATA_VERSION);
        return -1;
    }

    if (dscd->size != sizeof(DynamicSideChannelData))
    {
        printf("ERROR size %d != %u\n", dscd->size, (unsigned)sizeof(*dscd));
        return -2;
    }

    _dscd = *dscd;
    DYNAMIC_SIDE_CHANNEL_SETUP();
    return 0;
}

SIDE_CHANNEL_LINKAGE int LibVersion(DynamicPluginMeta *dpm)
{
    dpm->type  = TYPE_SIDE_CHANNEL;
    dpm->major = MAJOR_VERSION;
    dpm->minor = MINOR_VERSION;
    dpm->build = BUILD_VERSION;
    strncpy(dpm->uniqueName, SIDE_CHANNEL_NAME, MAX_NAME_LEN-1);
    dpm->uniqueName[MAX_NAME_LEN-1] = '\0';
    return 0;
}
