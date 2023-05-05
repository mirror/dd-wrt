/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
**
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

#ifndef SDF_DETECTION_OPTION__H
#define SDF_DETECTION_OPTION__H

#include <stdint.h>
#include "treenodes.h"
#include "sf_dynamic_engine.h"
#include "spp_sdf.h"

int SDFOptionInit(struct _SnortConfig *sc, char *name, char *args, void **data);
int SDFOptionEval(void *p, const uint8_t **cursor, void *data);
int SDFOtnHandler(struct _SnortConfig *sc, void *potn);

/* Struct for SDF option data */
typedef struct _SDFOptionData
{
    char *pii;
    uint32_t counter_index;
    OptTreeNode *otn;
    int (*validate_func)(char *buf, uint32_t buflen, struct _SDFConfig *config);
    uint8_t count;
    uint8_t match_success;

    /* These are kept separately in case the OTN reference is freed */
    uint32_t sid;
    uint32_t gid;
} SDFOptionData;

#endif
