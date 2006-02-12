/*-
 * Copyright (c) 2001, 2003 Lev Walkin <vlm@lionet.info>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: pps.c,v 1.22 2004/05/06 15:17:45 vlm Exp $
 */


#include "rflow.h"
#include "opt.h"
#include "disp.h"

static double pst = 0.0;

double
time_difference() {
	struct timeval tv;
	double d;
	double tdif;

	gettimeofday(&tv, NULL);

	d = tv.tv_sec + (double)tv.tv_usec / 1000000;
	if(pst < 1) {
		/* First run */
		pst = d;
		return 0.0;
	}
	tdif = d - pst;
	if(tdif < 0.1) {
		return 0.0;       /* Too short period */
	}

	pst = d;

	return tdif;

}


int
adjust_stats(packet_source_t *pss) {
	packet_source_t *ps;
	unsigned long tmpb;
	unsigned long tmpp;
	double tdif;

	tdif = time_difference();
	if(tdif < 0.01)       /* Too short period */
		return -1;

	for(ps = pss; ps; ps = ps->next) {
		/* Semi-atomic */
		tmpb = ps->bytes_cur; ps->bytes_cur = 0;
		tmpp = ps->packets_cur; ps->packets_cur= 0;

		/*
		 * Bytes
		 */

		ps->bytes_prev = tmpb;
		if(ps->bytes_lp == -1) {
			ps->bytes_lp = tmpb * (ps->avg_period / tdif);
		} else {
			ps->bytes_lp = (1 - tdif / ps->avg_period)
				* ps->bytes_lp + tmpb;
		}

		/* Estimate bps */
		ps->bps_lp = ps->bytes_lp / ps->avg_period;

		/*
		 * Packets
		 */

		ps->packets_prev = tmpp;
		if(ps->packets_lp == -1) {
			ps->packets_lp = tmpp * (ps->avg_period / tdif);
		} else {
			ps->packets_lp = (1 - tdif / ps->avg_period)
				* ps->packets_lp + tmpp;
		}

		/* Estimate bps */
		ps->pps_lp = ps->packets_lp / ps->avg_period;

	}

	return 0;
}


int
process_packet_sources(packet_source_t *pss) {
	packet_source_t *ps;
	sigset_t set, oset;

	/*
	 * Block all signals.
	 * The signal mask is inherited from the creating thread.
	 */
	sigfillset(&set);
	sigdelset(&set, SIGSEGV);
	sigdelset(&set, SIGBUS);
	sigprocmask(SIG_BLOCK, &set, &oset);

	/* Run threads */
	for(ps = pss; ps; ps = ps->next) {
		if( pthread_create(&ps->thid, NULL, ps->process_ptr, ps) ) {
			fprintf(stderr,
				"Can't start thread for %s!\n",
				ps->ifName);
			signoff_now = 1;
			sigprocmask(SIG_SETMASK, &oset, NULL);
			return -1;
		} else {
			printf("Thread %ld processing %s started.\n",
				(long)ps->thid, ps->ifName);
		}
	}

	sigprocmask(SIG_SETMASK, &oset, NULL);

	/* Signal-processing loop */

#define	SLEEP_TIME	2	/* Seconds */

	while(1) {
		sleep(SLEEP_TIME);

		if(signoff_now)
			break;

		if(display_now) {
			if(daemon_mode) {   
				FILE *f = fopen("/dev/console", "w");
				if(f) {
					display(f, 0);
					fclose(f);
				}
			} else {
				display(stdout, DS_NETFLOW);
			}
			display_now = 0;
		}

		adjust_stats(pss);
	}

	return 0;
}

