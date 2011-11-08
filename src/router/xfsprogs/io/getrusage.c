/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "init.h"

static cmdinfo_t getrusage_cmd;

/*
 * Report process resource utilisation.  Formatting options:
 * "Shell" format: 0.000u 0.000s 0:00.00 0.0%      0+0k 0+0io 0pf+0w
 * Verbose format:
 * 0.00user 0.00system 0:00.00elapsed 0%CPU (0avgtext+0avgdata 0maxresident)k
 * 0inputs+0outputs (0major+0minor)pagefaults 0swaps
 * Comma Separated Value format: 0.000,0.000,00:00:00.00,0.0,0,0,0,0,0,0
 */
static int
getrusage_f(
	int		argc,
	char		**argv)
{
	struct timeval	wallclk, timenow;
	struct rusage	rusage;
	double		usrtime, systime, elapsed, pct_cpu;
	char		ts[64];
	int		Cflag, vflag;
	int		c;

	Cflag = vflag = 0;
	while ((c = getopt(argc, argv, "Cv")) != EOF) {
		switch (c) {
		case 'C':
			Cflag = 1;
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			return command_usage(&getrusage_cmd);
		}
	}
	if (optind != argc)
		return command_usage(&getrusage_cmd);

	if (getrusage(RUSAGE_SELF, &rusage) < 0) {
		perror("getrusage");
		return 0;
	}

	gettimeofday(&timenow, NULL);
	wallclk = tsub(timenow, stopwatch);
	elapsed = (double)wallclk.tv_sec +
		  ((double)wallclk.tv_usec / 1000000.0);
	usrtime = (double)rusage.ru_utime.tv_sec +
		  ((double)rusage.ru_utime.tv_usec / 1000000.0);
	systime = (double)rusage.ru_stime.tv_sec +
		  ((double)rusage.ru_stime.tv_usec / 1000000.0);
	if (elapsed < usrtime + systime)
		pct_cpu = 100.0;
	else
		pct_cpu = ((usrtime + systime) / elapsed) * 100;
	c = Cflag ? VERBOSE_FIXED_TIME : TERSE_FIXED_TIME;
	timestr(&wallclk, ts, sizeof(ts), c);

	if (Cflag)
		printf("%.3f,%.3f,%s,%.1f,%ld,%ld,%ld,%ld,%ld,%ld,%ld\n",
			usrtime, systime, ts, pct_cpu,
			rusage.ru_majflt, rusage.ru_minflt, rusage.ru_nswap,
			rusage.ru_inblock, rusage.ru_oublock,
			rusage.ru_nvcsw, rusage.ru_nivcsw);
	else if (vflag)
		printf("%.2fuser %.2fsystem %selapsed %.0f%%CPU "
		       "(%ldavgtext+%ldavgdata %ldmaxresident)k\n"
		       "%ldinputs+%ldoutputs "
			"(%ldmajor+%ldminor)pagefaults %ldswaps\n",
			usrtime, systime, ts, pct_cpu,
			rusage.ru_ixrss, rusage.ru_idrss, rusage.ru_maxrss,
			rusage.ru_inblock, rusage.ru_oublock,
			rusage.ru_majflt, rusage.ru_minflt, rusage.ru_nswap);
	else
		printf("%.3fu %.3fs %s %.1f%%\t"
		       "%ld+%ldk %ld+%ldio %ldpf+%ldw\n",
			usrtime, systime, ts, pct_cpu,
			rusage.ru_maxrss, rusage.ru_ixrss,
			rusage.ru_inblock, rusage.ru_oublock,
			rusage.ru_majflt, rusage.ru_nswap);
	return 0;
}

void
getrusage_init(void)
{
	getrusage_cmd.name = "getrusage";
	getrusage_cmd.altname = "g";
	getrusage_cmd.argmin = 0;
	getrusage_cmd.argmax = -1;
	getrusage_cmd.cfunc = getrusage_f;
	getrusage_cmd.flags = CMD_NOFILE_OK | CMD_NOMAP_OK | CMD_FOREIGN_OK;
	getrusage_cmd.oneline = _("report process resource usage");

	if (expert)
		add_command(&getrusage_cmd);
}
