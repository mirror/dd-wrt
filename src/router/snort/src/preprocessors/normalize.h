/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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
#ifndef __NORMALIZE_H__
#define __NORMALIZE_H__

#ifdef NORMALIZER
#include "decode.h"
#include "snort.h"
#include "spp_normalize.h"

// TBD encapsulate implementation within normalize.c!
struct _NormalizerContext;

// all normalizers look like this:
// the return is 1 if packet was changed, else 0
typedef int (*Normalizer)(
    struct _NormalizerContext*, Packet*, uint8_t layer, int changes);

typedef enum registerFunc 
{
        unIntialized = 0,
        init,
        reload
} registerFunc;

typedef struct _NormalizerContext
{
    uint32_t normalizer_flags;
    uint8_t normalizer_options[32];
    NormMode normMode;

    // these must be in the same order PROTO_IDs are defined!
    // if entry is NULL, proto doesn't have normalization or it is disabled
    Normalizer normalizers[PROTO_MAX];
    registerFunc regFunc;
} NormalizerContext;

int Norm_SetConfig(NormalizerContext*);
int Norm_Packet(NormalizerContext*, Packet*);

static inline void Norm_Enable(NormalizerContext* nc, NormFlags nf)
{
    nc->normalizer_flags |= nf;
}

static inline void Norm_Disable(NormalizerContext* nc, NormFlags nf)
{
    nc->normalizer_flags &= ~nf;
}

static inline int Norm_IsEnabled(const NormalizerContext* nc, NormFlags nf)
{
    return ( (nc->normalizer_flags & nf) != 0 );
}

static inline void Norm_TcpPassOption(NormalizerContext* nc, uint8_t opt)
{
    uint8_t byte = (opt >> 3), bit = (1 << (opt & 0x07));
    nc->normalizer_options[byte] |= bit;
}

static inline void Norm_TcpDropOption(NormalizerContext* nc, uint8_t opt)
{
    uint8_t byte = (opt >> 3), bit = (1 << (opt & 0x07));
    nc->normalizer_options[byte] &= ~bit;
}

static inline int Norm_TcpIsOptional(const NormalizerContext* nc, uint8_t opt)
{
    uint8_t byte = (opt >> 3), bit = (1 << (opt & 0x07));
    return ( (nc->normalizer_options[byte] & bit) != 0 );
}

void Norm_PrintStats(void);
void Norm_ResetStats(void);
#endif // NORMALIZER

#endif // __NORMAL_H__

