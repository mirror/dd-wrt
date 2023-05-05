/*
**  mpse.h
**
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
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
** GNU Gener*
*/

#ifndef _MPSE_METHODS_H_
#define _MPSE_METHODS_H_

/*
*  Pattern Matching Methods
*/
//#define MPSE_MWM       1
#define MPSE_AC        2
//#define MPSE_KTBM      3
#define MPSE_LOWMEM    4
//#define MPSE_AUTO      5
#define MPSE_ACF       6
#define MPSE_ACS       7
#define MPSE_ACB       8
#define MPSE_ACSB      9
#define MPSE_AC_BNFA   10
#define MPSE_AC_BNFA_Q 11
#define MPSE_ACF_Q     12
#define MPSE_LOWMEM_Q  13

#ifdef INTEL_SOFT_CPM
#define MPSE_INTEL_CPM 14
#endif /* INTEL_SOFT_CPM */

typedef enum {
    MPSE_PATTERN_CASE,
    MPSE_PATTERN_NOCASE
} tMpseCaseEnum;
#endif

