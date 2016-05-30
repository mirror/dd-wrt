/* src/vm/cycles-stats.c - functions for cycle count statistics

   Copyright (C) 1996-2005, 2006 R. Grafl, A. Krall, C. Kruegel,
   C. Oates, R. Obermaisser, M. Platter, M. Probst, S. Ring,
   E. Steiner, C. Thalinger, D. Thuernbeck, P. Tomsich, C. Ullrich,
   J. Wenninger, Institut f. Computersprachen - TU Wien

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

   Contact: cacao@cacaojvm.org

   Authors: Edwin Steiner

   Changes:

*/

#include "config.h"
#include "vm/types.hpp"
#include "vm/global.hpp"

#if defined(ENABLE_CYCLES_STATS)

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "vm/cycles-stats.hpp"

#include "toolbox/logging.hpp"

struct cycles_stats_percentile {
	int         pct;
	const char *name;
};

static struct cycles_stats_percentile cycles_stats_percentile_defs[] = {
	{ 10, "10%-percentile" },
	{ 50, "median"         },
	{ 90, "90%-percentile" },
	{ 99, "99%-percentile" },
	{  0, NULL             } /* sentinel */
};

static double cycles_stats_cpu_MHz = 0.0;

#define CYCLES_STATS_MAXLINE  100

static double cycles_stats_get_cpu_MHz(void)
{
	FILE *info;
	char line[CYCLES_STATS_MAXLINE + 1];

	if (cycles_stats_cpu_MHz != 0.0)
		return cycles_stats_cpu_MHz;

	info = fopen("/proc/cpuinfo","r");
	if (!info) {
		fprintf(log_get_logfile(),"error: could not open /proc/cpuinfo: %s\n",strerror(errno));
		goto got_no_cpuinfo;
	}

	while (!feof(info) && !ferror(info)) {
		if (fgets(line,CYCLES_STATS_MAXLINE,info)
			&& sscanf(line,"cpu MHz : %lf",&cycles_stats_cpu_MHz) == 1)
		{
			fclose(info);
			fprintf(log_get_logfile(),"CPU frequency used for statistics: %f MHz\n",
					cycles_stats_cpu_MHz);
			return cycles_stats_cpu_MHz;
		}
	}

	if (ferror(info)) {
		fprintf(log_get_logfile(),"error reading /proc/cpuinfo: %s\n",strerror(errno));
	}

	fclose(info);

got_no_cpuinfo:
	fprintf(log_get_logfile(),"warning: falling back to default CPU frequency for statistics\n");
	cycles_stats_cpu_MHz = 1800.0;
	return cycles_stats_cpu_MHz;
}

u8 cycles_stats_measurement_overhead = 0;

static void cycles_stats_print_percentile(FILE *file, const char *name,
										  double percentile, u8 count,
										  u8 cumul, double cumulcycles,
										  bool isoverhead, bool printforall)
{
	u8 forall;
	double cpuMHz = cycles_stats_get_cpu_MHz();
	u8 cycles_per_ms = cpuMHz * 1000;

	if (isoverhead) {
		fprintf(file,"\t\t%14s = %6.1f\n", name, percentile);
	}
	else {
		percentile -= cycles_stats_measurement_overhead;

		fprintf(file,"\t\t%14s = %14.1f (+%llu)",
				name, percentile,
				(unsigned long long)cycles_stats_measurement_overhead);
		if (printforall) {
			forall = cumulcycles - cumul * cycles_stats_measurement_overhead
				   + percentile * (count - cumul);
			fprintf(file," (%-23s: %15llu cycles = %6lu msec)",
					(count == cumul) ? "total" : "capped here & extrapol.",
					(unsigned long long)forall,
					(unsigned long)(forall / cycles_per_ms));
		}
		fprintf(file,"\n");
	}
}

void cycles_stats_print(FILE *file,
						const char *name, int nbins, int div,
						u4 *bins, u8 count, u8 total, u8 min, u8 max,
						int overhead)
{
        s4 i;
		struct cycles_stats_percentile *pcd;
		u8 floor, ceiling;
		u8 p;
		u8 cumul;
		double cumulcycles;
		double percentile;
		double cpuMHz = cycles_stats_get_cpu_MHz();
		u8 cycles_per_ms = cpuMHz * 1000;

        fprintf(file,"\t%s: %llu calls\n",
                (overhead) ? "measurement overhead determined by" : name,
				(unsigned long long)count);

        fprintf(file,"\t%s cycles distribution:\n",
				(overhead) ? "measurement overhead" : name);

		cycles_stats_print_percentile(file, "min", min, count, 0, 0, overhead, true);

		pcd = cycles_stats_percentile_defs;
		for (; pcd->name; pcd++) {
			floor   = (count * pcd->pct) / 100;
			ceiling = (count * pcd->pct + 99) / 100;
			cumul = 0;
			cumulcycles = 0;
			p = 0;
			percentile = -1.0;

			assert( ceiling <= floor + 1 );

			for (i=0; i<nbins; ++i) {

				/* { invariant: `cumul` samples are < `p` } */

				/* check if percentile lies exactly at the bin boundary */

				if (floor == cumul && floor == ceiling) {
					percentile = p;
					break;
				}

				/* check if percentile lies within this bin */

				if (cumul <= floor && ceiling <= cumul + bins[i]) {
					percentile = p + (double)div/2.0;
					break;
				}

				cumul += bins[i];
				p     += div;

				cumulcycles += bins[i] * (p - (double)div/2.0);

				/* { invariant: `cumul` samples are < `p` } */
			}

			/* check if percentile lies exactly at the bin boundary */

			if (floor == cumul && floor == ceiling) {
				percentile = p;
			}

			if (percentile >= 0) {
				if (overhead && pcd->pct == 50) {
					cycles_stats_measurement_overhead = percentile;
				}
				cycles_stats_print_percentile(file, pcd->name, percentile,
											  count, cumul, cumulcycles,
											  overhead, true);
			}
			else {
				if (!overhead)
					p -= cycles_stats_measurement_overhead;
				fprintf(file,"\t\t%14s = unknown (> %llu)\n", pcd->name, (unsigned long long)p);
			}
		}

		cycles_stats_print_percentile(file, "max", max, count, count,
									  total, overhead, true);

		if (!overhead) {
			fprintf(file,"\t\t(assuming %llu cycles per ms)\n",
					(unsigned long long)cycles_per_ms);
			fprintf(file,"\t\t(assuming %llu cycles measurement overhead)\n",
					(unsigned long long)cycles_stats_measurement_overhead);
		}

		fprintf(file,"\n");

		cumul = 0;
        for (i=0; i<nbins; ++i) {
			cumul += bins[i];
            fprintf(file,"\t\t<  %8d: %10lu (%3d%%) %10lu\n",
                    (int)((i+1) * div),
					(unsigned long) cumul,
					(count) ? (int)((cumul * 100) / count) : 0,
                    (unsigned long) bins[i]);
        }

        fprintf(file,"\t\t>= %8d: %10s (----) %10lu\n",
                (int)(nbins * div),
				"OVER",
                (unsigned long) bins[nbins]);
}

#endif /* defined(ENABLE_CYCLES_STATS) */

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
