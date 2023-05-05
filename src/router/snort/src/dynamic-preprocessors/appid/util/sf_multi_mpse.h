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


#ifndef _SF_MULTI_MPSE_H_
#define _SF_MULTI_MPSE_H_

#include <stdlib.h>
#include <stdint.h>

typedef struct _tPattern
{
    const uint8_t *pattern;
    size_t patternSize;
} tMlpPattern;

void *mlpCreate(void);
int   mlpAddPattern(void *root, const tMlpPattern **patterns, void *metaData);
int   mlpProcessPatterns(void *root);
void *mlpMatchPatternLongest(void *root, tMlpPattern **inputPatternList);
void *mlpMatchPatternUrl(void *root, tMlpPattern **inputPatternList);
void *mlpMatchPatternCustom(void *root, tMlpPattern **inputPatternList, int (*callback)(void*, void*, int, void*, void*));
void  mlpDestroy(void *root);
void  mlpDump(void *root);
void *mlpGetPatternMatcherTree(void *root, tMlpPattern **inputPatternList);

#endif
