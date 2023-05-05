/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
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

#ifndef _PERF_INDICATORS_H_
#define _PERF_INDICATORS_H_

#include "profiler.h"

/*#define PI_PROCESSOR_UTILIZATION_SUPPORT*/

#ifdef PERF_PROFILING
#define PI_PACKET_LATENCY_SUPPORT
#endif

#define PI_PACKET_DROPS_SUPPORT

/* Force the collection of packet latency profiler data */
#ifdef PI_PACKET_LATENCY_SUPPORT
#define PI_PREPROCS (1)
#endif

enum Perf_Indicator_Type
{
    Perf_Indicator_Type_None = 0,
    Perf_Indicator_Type_Packet_Latency,
    Perf_Indicator_Type_Processor_Utilization,
    Perf_Indicator_Type_DAQ_Drops
};

struct Perf_Indicator_Descriptor;

struct Perf_Indicator_Latency
{
    uint32_t Latency_us;
};

struct Perf_Indicator_Processor
{
    double Processor_Load;
};

struct Perf_Indicator_Drops
{
    double Drop_Portion;
};

struct _Perf_Indicator_Descriptor
{
    enum Perf_Indicator_Type Type;
    struct _Perf_Indicator_Descriptor *Next;
    bool Valid;
    union
    {
        struct Perf_Indicator_Latency Latency;
        struct Perf_Indicator_Processor Processor;
        struct Perf_Indicator_Drops Drops;
    } Indicator;
};

typedef struct _Perf_Indicator_Descriptor
        Perf_Indicator_Descriptor_t, *Perf_Indicator_Descriptor_p_t;

#ifdef PERF_PROFILING
int PerfIndicator_RegisterPreprocStat( PreprocStats *Stats,
                                       enum Perf_Indicator_Type Type );
#endif

int PerfIndicator_GetIndicators( Perf_Indicator_Descriptor_p_t PI_List );

#ifdef PI_PACKET_LATENCY_SUPPORT
uint32_t GetPacketLatency(void);
#endif

#ifdef PI_PACKET_DROPS_SUPPORT
double GetPacketDropPortion(void);
#endif

static inline Perf_Indicator_Descriptor_p_t PerfIndicator_NewDescriptor(void)
{
    /* Caller must check for NULL */
    return( (Perf_Indicator_Descriptor_p_t)calloc(sizeof(Perf_Indicator_Descriptor_t), sizeof(char)));
}

static inline void PerfIndicator_FreeDescriptorList( Perf_Indicator_Descriptor_p_t PI_List )
{
    if( PI_List->Next != NULL )
        PerfIndicator_FreeDescriptorList( PI_List->Next );
    free( PI_List );
}

static inline int PerfIndicator_CreateAndInsertDescriptor( Perf_Indicator_Descriptor_p_t *PI_List_Ptr,
                                                    enum Perf_Indicator_Type Type )
{
    Perf_Indicator_Descriptor_p_t elem;

    if( (PI_List_Ptr == NULL) || (elem = PerfIndicator_NewDescriptor()) == NULL )
        return( -1 );

    elem->Type = Type;
    elem->Next = *PI_List_Ptr;
    *(PI_List_Ptr) = elem;
    return( 0 );
}

#endif
