/* Read, and possibly modify, the Linux kernel `timex' variables.
 *
 * Copyright (C) 1997, 2000, 2003  Larry Doolittle <larry@doolittle.boa.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2,
 * June 1991) as published by the Free Software Foundation.  At the
 * time of writing, that license was published by the FSF with the URL
 * http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 * reference.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * This adjtimex(1) is very similar in intent to adjtimex(8) by Steven
 * Dick <ssd@nevets.oau.org> and Jim Van Zandt <jrv@vanzandt.mv.com>
 * (see http://metalab.unc.edu/pub/Linux/system/admin/time/adjtimex*).
 * That version predates this one, and is _much_ bigger and more
 * featureful.  My independently written version was very similar to
 * Steven's from the start, because they both follow the kernel timex
 * structure.  I further tweaked this version to be equivalent to Steven's
 * where possible, but I don't like getopt_long, so the actual usage
 * syntax is incompatible.
 *
 * Amazingly enough, my Red Hat 5.2 sys/timex (and sub-includes)
 * don't actually give a prototype for adjtimex(2), so building
 * this code (with -Wall) gives a warning.  Later versions of
 * glibc fix this issue.
 *
 * This program is too simple for a Makefile, just build with:
 *  gcc -Wall -O adjtimex_1.c -o adjtimex
 */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timex.h>

/* Status codes (timex.status) */
static struct {
	int bit;
	const char *name;
} statlist[] = {
	{
	STA_PLL, "PLL"},	/* enable PLL updates (rw)           */
	{
	STA_PPSFREQ, "PPSFREQ"},	/* enable PPS freq discipline (rw)   */
	{
	STA_PPSTIME, "PPSTIME"},	/* enable PPS time discipline (rw)   */
	{
	STA_FLL, "FFL"},	/* select frequency-lock mode (rw)   */
	{
	STA_INS, "INS"},	/* insert leap (rw)                  */
	{
	STA_DEL, "DEL"},	/* delete leap (rw)                  */
	{
	STA_UNSYNC, "UNSYNC"},	/* clock unsynchronized (rw)         */
	{
	STA_FREQHOLD, "FREQHOLD"},	/* hold frequency (rw)               */
	{
	STA_PPSSIGNAL, "PPSSIGNAL"},	/* PPS signal present (ro)           */
	{
	STA_PPSJITTER, "PPSJITTER"},	/* PPS signal jitter exceeded (ro)   */
	{
	STA_PPSWANDER, "PPSWANDER"},	/* PPS signal wander exceeded (ro)   */
	{
	STA_PPSERROR, "PPSERROR"},	/* PPS signal calibration error (ro) */
	{
	STA_CLOCKERR, "CLOCKERR"},	/* clock hardware fault (ro)         */
	{
	0, NULL}
};

static const char *ret_code_descript[] = {
	"clock synchronized",
	"insert leap second",
	"delete leap second",
	"leap second in progress",
	"leap second has occurred",
	"clock not synchronized"
};

static int usage(int code)
{
	fprintf(stderr, "Usage: adjtimex [-q] [-o offset] [-f frequency] [-p timeconstant] [-t tick]\n");

	return code;
}

int main(int argc, char **argv)
{
	struct timex txc;
	int quiet = 0;
	int c, i, ret, sep;

	txc.modes = 0;
	while (1) {
		c = getopt(argc, argv, "f:ho:p:qt:");
		if (c == EOF)
			break;

		switch (c) {
		case 'f':
			txc.freq = atoi(optarg);
			txc.modes |= ADJ_FREQUENCY;
			break;

		case 'h':
			return usage(0);

		case 'o':
			txc.offset = atoi(optarg);
			txc.modes |= ADJ_OFFSET_SINGLESHOT;
			break;

		case 'p':
			txc.constant = atoi(optarg);
			txc.modes |= ADJ_TIMECONST;
			break;

		case 'q':
			quiet = 1;
			break;

		case 't':
			txc.tick = atoi(optarg);
			txc.modes |= ADJ_TICK;
			break;

		default:
			return usage(1);
		}
	}

	/* Check for valid non-option parameters */
	if (argc != optind)
		return usage(1);

	ret = adjtimex(&txc);
	if (ret < 0)
		perror("adjtimex");

	if (!quiet && ret >= 0) {
		printf("    mode:         %d\n"
		       "-o  offset:       %ld\n"
		       "-f  frequency:    %ld\n" "    maxerror:     %ld\n" "    esterror:     %ld\n" "    status:       %d ( ", txc.modes, txc.offset, txc.freq, txc.maxerror, txc.esterror, txc.status);

		/*
		 * representative output of next code fragment:
		 * "PLL | PPSTIME"
		 */
		sep = 0;
		for (i = 0; statlist[i].name; i++) {
			if (txc.status & statlist[i].bit) {
				if (sep)
					fputs(" | ", stdout);
				fputs(statlist[i].name, stdout);
				sep = 1;
			}
		}

		printf(" )\n"
		       "-p  timeconstant: %ld\n"
		       "    precision:    %ld\n"
		       "    tolerance:    %ld\n"
		       "-t  tick:         %ld\n"
		       "    time.tv_sec:  %ld\n"
		       "    time.tv_usec: %ld\n"
		       "    return value: %d (%s)\n", txc.constant, txc.precision, txc.tolerance, txc.tick, txc.time.tv_sec, txc.time.tv_usec, ret, (ret >= 0 && ret <= 5) ? ret_code_descript[ret] : "error");
	}

	return ret < 0;
}

/**
 * Local Variables:
 *  compile-command: "make adjtimex"
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
