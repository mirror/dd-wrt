/* src/vm/cycles-stats.hpp - macros for cycle count statistics

   Copyright (C) 1996-2005, 2006, 2009
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/

#ifndef CYCLES_STATS_HPP_
#define CYCLES_STATS_HPP_

#include "config.h"
#include "vm/types.hpp"

#if defined(ENABLE_CYCLES_STATS)

#include <stdio.h>

#include "md.hpp"

#define CYCLES_STATS_DECLARE(name,nbins,divisor)                            \
    static const int CYCLES_STATS_##name##_MAX = (nbins);                   \
    static const int CYCLES_STATS_##name##_DIV = (divisor);                 \
    static u4 cycles_stats_##name##_bins[(nbins) + 1] = { 0 };              \
    static u4 cycles_stats_##name##_count = 0;                              \
    static u8 cycles_stats_##name##_total = 0;                              \
    static u8 cycles_stats_##name##_max = 0;                                \
    static u8 cycles_stats_##name##_min = 1000000000;

#define CYCLES_STATS_GET(var)                                               \
	(var) = md_get_cycle_count()                                            \

#define CYCLES_STATS_COUNT(name,cyclesexpr)                                 \
    do {                                                                    \
        u8 cyc = (cyclesexpr);                                              \
		cycles_stats_##name##_total += cyc;                                 \
        if (cyc > cycles_stats_##name##_max)                                \
            cycles_stats_##name##_max = cyc;                                \
        if (cyc < cycles_stats_##name##_min)                                \
            cycles_stats_##name##_min = cyc;                                \
        cyc /= CYCLES_STATS_##name##_DIV;                                   \
        if (cyc < CYCLES_STATS_##name##_MAX)                                \
            cycles_stats_##name##_bins[cyc]++;                              \
        else                                                                \
            cycles_stats_##name##_bins[CYCLES_STATS_##name##_MAX]++;        \
        cycles_stats_##name##_count++;                                      \
    } while (0)

#define CYCLES_STATS_COUNT_OVER(name,ovname,cyclesexpr)                     \
    do {                                                                    \
        u8 cyc = (cyclesexpr);                                              \
        if (cyc / CYCLES_STATS_##name##_DIV >= CYCLES_STATS_##name##_MAX)   \
            CYCLES_STATS_COUNT(ovname,cyc);                                 \
    } while (0)

#define CYCLES_STATS_PRINT(name,file)                                       \
    do {                                                                    \
        cycles_stats_print((file), #name,                                   \
            CYCLES_STATS_##name##_MAX, CYCLES_STATS_##name##_DIV,           \
            cycles_stats_##name##_bins, cycles_stats_##name##_count,        \
            cycles_stats_##name##_total,                                    \
            cycles_stats_##name##_min, cycles_stats_##name##_max, 0);       \
    } while (0)

#define CYCLES_STATS_PRINT_OVERHEAD(name,file)                              \
    do {                                                                    \
        cycles_stats_print((file), #name,                                   \
            CYCLES_STATS_##name##_MAX, CYCLES_STATS_##name##_DIV,           \
            cycles_stats_##name##_bins, cycles_stats_##name##_count,        \
            cycles_stats_##name##_total,                                    \
            cycles_stats_##name##_min, cycles_stats_##name##_max, 1);       \
    } while (0)

#define CYCLES_STATS_DECLARE_AND_START                                      \
    u8 cycles_start = md_get_cycle_count();                                 \
    u8 cycles_end;

#define CYCLES_STATS_DECLARE_AND_START_WITH_OVERHEAD                        \
    u8 cycles_start = md_get_cycle_count();                                 \
    u8 cycles_overhead = md_get_cycle_count();                              \
    u8 cycles_end;

#define CYCLES_STATS_END(name)                                              \
    cycles_end = md_get_cycle_count();                                      \
    CYCLES_STATS_COUNT(name, cycles_end - cycles_start);

#define CYCLES_STATS_END_WITH_OVERHEAD(name,ovname)                         \
    cycles_end = md_get_cycle_count();                                      \
    CYCLES_STATS_COUNT(ovname, cycles_overhead - cycles_start);             \
    CYCLES_STATS_COUNT(name, cycles_end - cycles_overhead);

void cycles_stats_print(FILE *file,
					    const char *name, int nbins, int div,
					    u4 *bins, u8 count, u8 total, u8 min, u8 max,
						int overhead);

#else /* !defined(ENABLE_CYCLES_STATS) */

#define CYCLES_STATS_DECLARE(name,nbins,divisor)
#define CYCLES_STATS_GET(var)
#define CYCLES_STATS_COUNT(name,cyclesexpr)
#define CYCLES_STATS_COUNT_OVER(name,ovname,cyclesexpr)
#define CYCLES_STATS_PRINT(name,file)
#define CYCLES_STATS_PRINT_OVERHEAD(name,file)
#define CYCLES_STATS_DECLARE_AND_START
#define CYCLES_STATS_DECLARE_AND_START_WITH_OVERHEAD
#define CYCLES_STATS_END(name)
#define CYCLES_STATS_END_WITH_OVERHEAD(name,ovname)

#endif /* defined(ENABLE_CYCLES_STATS) */

#endif // CYCLES_STATS_HPP_

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
