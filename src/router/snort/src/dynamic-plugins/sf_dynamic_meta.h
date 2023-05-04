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
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steven Sturges
 *
 * Dynamic Library Loading for Snort
 *
 */
#ifndef _SF_DYNAMIC_META_H_
#define _SF_DYNAMIC_META_H_

/* Required version and name of the engine */
#ifndef REQ_ENGINE_LIB_MAJOR
#define REQ_ENGINE_LIB_MAJOR 3
#endif
#ifndef REQ_ENGINE_LIB_MINOR
/* FIXTHIS need to update dynamic-plugins/sf_engine/examples/sfsnort_dynamic_detection_lib.c */
#define REQ_ENGINE_LIB_MINOR 2
#endif
#define REQ_ENGINE_LIB_NAME "SF_SNORT_DETECTION_ENGINE"

#define MAX_NAME_LEN 1024

#define TYPE_ENGINE 0x01
#define TYPE_DETECTION 0x02
#define TYPE_PREPROCESSOR 0x04
#define TYPE_SIDE_CHANNEL 0x08

typedef struct _DynamicPluginMeta
{
    int type;
    int major;
    int minor;
    int build;
    char uniqueName[MAX_NAME_LEN];
    char *libraryPath;
} DynamicPluginMeta;

typedef int (*LibVersionFunc)(DynamicPluginMeta *);

#endif /* _SF_DYNAMIC_META_H_ */
