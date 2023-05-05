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


#ifndef _SF_MULTI_PART_MPSE_H_
#define _SF_MULTI_PART_MPSE_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct _tMlmpPattern
{
    /*binary pattern */
    const uint8_t *pattern;

    /*binary pattern length in bytes */
    size_t        patternSize;

    /**level of pattern. It should start from 0.*/
    uint32_t      level;

} tMlmpPattern;

struct tMlmpTree;

struct tMlmpTree* mlmpCreate(void);
int   mlmpAddPattern(struct tMlmpTree* root, const tMlmpPattern *patterns, void *metaData);
int   mlmpProcessPatterns(struct tMlmpTree* root);
void *mlmpMatchPatternUrl(struct tMlmpTree* root, tMlmpPattern *inputPatternList);
void *mlmpMatchPatternGeneric(struct tMlmpTree* root, tMlmpPattern *inputPatternList);
void  mlmpDestroy(struct tMlmpTree* root);
void  mlmpDump(struct tMlmpTree* root);

#endif
