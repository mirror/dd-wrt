/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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

/* Necessary hash/wrapper functions to put a .so rule's HdrOptCheck option
 * directly on the rule option tree. */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sp_hdr_opt_wrap.h"
#include "sf_engine/sf_snort_plugin_api.h"

//extern int checkHdrOpt(void *p, HdrOptCheck *optData);

uint32_t HdrOptCheckHash(void *d)
{
    uint32_t a, b, c;
    HdrOptCheck *hdrData = (HdrOptCheck *)d;

    a = (uint32_t)hdrData->hdrField;
    b = hdrData->op;
    c = hdrData->value;
    mix(a,b,c);

    a += hdrData->mask_value;
    b += hdrData->flags;
    final(a,b,c);

    return c;
}

int HdrOptCheckCompare(void *l, void *r)
{
    HdrOptCheck *left = (HdrOptCheck *)l;
    HdrOptCheck *right = (HdrOptCheck *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if ((left->hdrField == right->hdrField) &&
        (left->op == right->op) &&
        (left->value == right->value) &&
        (left->mask_value == right->mask_value) &&
        (left->flags == right->flags))
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}

/* This function is a wrapper to call the check function normally used in
 * .so rules */
int HdrOptEval(void *option_data, Packet *p)
{
   HdrOptCheck *hdrData = (HdrOptCheck *)option_data;

   return checkHdrOpt(p, hdrData);
}
