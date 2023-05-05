/* $Id$ */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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

#ifndef __SP_REPLACE_H__
#define __SP_REPLACE_H__

#include "sp_pattern_match.h"

void PayloadReplaceInit(struct _SnortConfig *, char *, OptTreeNode *, int);

extern void Replace_ResetQueue(void);
extern void Replace_QueueChange(PatternMatchData*);
extern void Replace_ModifyPacket(Packet*);

static inline void Replace_ResetOffset(PatternMatchData* pmd)
{
    pmd->replace_depth = -1;
}

static inline void Replace_StoreOffset(PatternMatchData* pmd, int detect_depth)
{
    pmd->replace_depth = detect_depth;
}

static inline int Replace_OffsetStored(PatternMatchData* pmd)
{
    return pmd->replace_depth >= 0;
}

#endif  /* __SP_REACT_H__ */

