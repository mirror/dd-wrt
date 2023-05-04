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
 */

/* $Id$ */

#ifndef __SP_PREPROCOPT_H_
#define __SP_PREPROCOPT_H_

#include "sf_dynamic_engine.h"
#include "sfghash.h"

SFGHASH * PreprocessorRuleOptionsNew(void);
void PreprocessorRuleOptionsFree(SFGHASH *);

int RegisterPreprocessorRuleOption(
    struct _SnortConfig *,
    char *optionName,
    PreprocOptionInit initFunc,
    PreprocOptionEval evalFunc,
    PreprocOptionCleanup cleanupFunc,
    PreprocOptionHash hashFunc,
    PreprocOptionKeyCompare keyCompareFunc,
    PreprocOptionOtnHandler otnHandler,
    PreprocOptionFastPatternFunc fpFunc
);

void RegisterPreprocessorRuleOptionOverride(
    struct _SnortConfig *,
    char *keyword, char *option,
    PreprocOptionInit initFunc,
    PreprocOptionEval evalFunc,
    PreprocOptionCleanup cleanupFunc,
    PreprocOptionHash hashFunc,
    PreprocOptionKeyCompare keyCompareFunc,
    PreprocOptionOtnHandler otnHandler,
    PreprocOptionFastPatternFunc fpFunc
);

int GetPreprocessorRuleOptionFuncs(
    struct _SnortConfig *,
    char *optionName,
    PreprocOptionInit* initFunc,
    PreprocOptionEval* evalFunc,
    PreprocOptionOtnHandler* otnHandler,
    PreprocOptionFastPatternFunc* fpFunc,
    PreprocOptionCleanup* cleanupFunc
);


void RegisterPreprocessorRuleOptionByteOrder(char *keyword, PreprocOptionByteOrderFunc bo_func);

int AddPreprocessorRuleOption(struct _SnortConfig *sc, char *, OptTreeNode *, void *, PreprocOptionEval);

uint32_t PreprocessorRuleOptionHash(void *d);
int PreprocessorRuleOptionCompare(void *l, void *r);
void PreprocessorRuleOptionsFreeFunc(void *);
int GetPreprocFastPatterns(void *, int, int, FPContentInfo **);
int PreprocessorOptionFunc(void *option_data, Packet *p);

#endif  /* __SP_PREPROCOPT_H_ */

