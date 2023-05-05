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

#include "sf_types.h"

#include "sfdaq.h"

#include "perf_indicators.h"

#ifdef PI_PACKET_LATENCY_SUPPORT
static PreprocStats *latencyStats = { NULL };
#endif

static double ticks_per_microsec = 0.0;

static void getTicksPerMicrosec(void)
{
    if (ticks_per_microsec == 0.0)
    {
        ticks_per_microsec = get_ticks_per_usec();
    }
}

#ifdef PI_PACKET_LATENCY_SUPPORT
uint32_t GetPacketLatency(void)
{
    if( (latencyStats != NULL) && (latencyStats->exits != 0) )
    {
        getTicksPerMicrosec();

	/* The exit counter indicate the number of full enter-exit cycles
	 * have been added to the ticks count. */
        return( (uint32_t)((double)latencyStats->ticks /
			   (double)latencyStats->exits /
			   ticks_per_microsec));
    }
    else
        return( 0 );
}
#endif

#ifdef PI_PACKET_DROPS_SUPPORT
double GetPacketDropPortion(void)
{
    uint64_t drop, recv, sum;
    double portion;
    const DAQ_Stats_t* ps = DAQ_GetStats();

/* Debugging code:
    DAQ_Stats_t test_stats;
    test_stats.packets_received = 2000;
    test_stats.hw_packets_dropped = 1050;
    ps = &test_stats; */

    recv = ps->packets_received;
    drop = ps->hw_packets_dropped;

    sum = recv + drop;

    if( sum == 0 )
        portion = 0.0;
    else
        portion = ((double)drop / (double)sum);

    return( portion );
}
#endif

#ifdef PERF_PROFILING
int PerfIndicator_RegisterPreprocStat( PreprocStats *Stats,
                                       enum Perf_Indicator_Type Type )
{
    if( (Stats != NULL) && (Type == Perf_Indicator_Type_Packet_Latency) )
    {
        latencyStats = Stats;
        return( 0 );
    }
    else
        return( -1 );
}
#endif

int PerfIndicator_GetIndicators( Perf_Indicator_Descriptor_p_t PI_List )
{
    if( PI_List == NULL )
        return( -1 );

    for( ; PI_List != NULL; PI_List = PI_List->Next )
    {
        PI_List->Valid = false;
        switch( PI_List->Type )
        {
#ifdef PI_PACKET_LATENCY_SUPPORT
            case( Perf_Indicator_Type_Packet_Latency ):
            {
                PI_List->Indicator.Latency.Latency_us = GetPacketLatency();
                PI_List->Valid = true;
                break;
            }
#endif
#ifdef PI_PROCESSOR_UTILIZATION_SUPPORT
            case( Perf_Indicator_Type_Processor_Utilization ):
            {
                PI_List->Valid = false;
                break;
            }
#endif
#ifdef PI_PACKET_DROPS_SUPPORT
            case( Perf_Indicator_Type_DAQ_Drops ):
            {
                PI_List->Indicator.Drops.Drop_Portion = GetPacketDropPortion();
                PI_List->Valid = true;
                break;
            }
#endif
            default:
            {
                return( -1 );
            }
        }
    }

    return( 0 );
}
