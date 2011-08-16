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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
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
    char *optionName,
    PreprocOptionInit* initFunc,
    PreprocOptionEval* evalFunc,
    PreprocOptionOtnHandler* otnHandler,
    PreprocOptionFastPatternFunc* fpFunc,
    PreprocOptionCleanup* cleanupFunc
);


void RegisterPreprocessorRuleOptionByteOrder(char *keyword, PreprocOptionByteOrderFunc bo_func);

int AddPreprocessorRuleOption(char *, OptTreeNode *, void *, PreprocOptionEval);

u_int32_t PreprocessorRuleOptionHash(void *d);
int PreprocessorRuleOptionCompare(void *l, void *r);
void PreprocessorRuleOptionsFreeFunc(void *);
int GetPreprocFastPatterns(void *, int, int, FPContentInfo **);

#endif  /* __SP_PREPROCOPT_H_ */

