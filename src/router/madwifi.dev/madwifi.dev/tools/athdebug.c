/*-
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 *
 * $Id: athdebug.c 3008 2007-12-07 07:00:31Z br1 $
 */

/*
 * athdebug [-i interface] flags
 * (default interface is wifi0).
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#ifdef DOMULTI
#include "do_multi.h"
#endif

#undef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static const char *progname;

enum {
	ATH_DEBUG_XMIT = 0x00000001,	/* basic xmit operation */
	ATH_DEBUG_XMIT_DESC = 0x00000002,	/* xmit descriptors */
	ATH_DEBUG_RECV = 0x00000004,	/* basic recv operation */
	ATH_DEBUG_RECV_DESC = 0x00000008,	/* recv descriptors */
	ATH_DEBUG_RATE = 0x00000010,	/* rate control */
	ATH_DEBUG_RESET = 0x00000020,	/* reset processing */
	ATH_DEBUG_SKB_REF = 0x00000040,	/* skb ref counting */
	ATH_DEBUG_BEACON = 0x00000080,	/* beacon handling */
	ATH_DEBUG_WATCHDOG = 0x00000100,	/* watchdog timeout */
	ATH_DEBUG_INTR = 0x00001000,	/* ISR */
	ATH_DEBUG_TX_PROC = 0x00002000,	/* tx ISR proc */
	ATH_DEBUG_RX_PROC = 0x00004000,	/* rx ISR proc */
	ATH_DEBUG_BEACON_PROC = 0x00008000,	/* beacon ISR proc */
	ATH_DEBUG_CALIBRATE = 0x00010000,	/* periodic calibration */
	ATH_DEBUG_KEYCACHE = 0x00020000,	/* key cache management */
	ATH_DEBUG_STATE = 0x00040000,	/* 802.11 state transitions */
	ATH_DEBUG_TSF = 0x00080000,	/* timestamp processing */
	ATH_DEBUG_LED = 0x00100000,	/* led management */
	ATH_DEBUG_FF = 0x00200000,	/* fast frames */
	ATH_DEBUG_TURBO = 0x00400000,	/* turbo/dynamic turbo */
	ATH_DEBUG_UAPSD = 0x00800000,	/* uapsd */
	ATH_DEBUG_DOTH = 0x01000000,	/* 11.h */
	ATH_DEBUG_DOTHFILT = 0x02000000,	/* 11.h radar pulse analysis */
	ATH_DEBUG_DOTHFILTVBSE = 0x04000000,	/* 11.h radar pulse analysis - verbose */
	ATH_DEBUG_DOTHFILTNOSC = 0x08000000,	/* 11.h radar pulse analysis - don't short circuit analysis when detected */
	ATH_DEBUG_DOTHPULSES = 0x10000000,	/* 11.h radar pulse events */
	ATH_DEBUG_TXBUF = 0x20000000,	/* TX buffer usage/leak debugging */
	ATH_DEBUG_SKB = 0x40000000,	/* SKB usage/leak debugging [applies to all vaps] */
	ATH_DEBUG_FATAL = 0x80000000,	/* fatal errors */
	ATH_DEBUG_ANY = 0xffffffff,
	ATH_DEBUG_GLOBAL = (ATH_DEBUG_SKB | ATH_DEBUG_SKB_REF)
};

static struct {
	const char *name;
	u_int bit;
	const char *desc;
} flags[] = {
	{ "xmit", ATH_DEBUG_XMIT, "transmission of packets before out to HW" },
	{ "xmit_desc", ATH_DEBUG_XMIT_DESC, "transmit descriptors" },
	{ "recv", ATH_DEBUG_RECV, "received packets directly from HW" },
	{ "recv_desc", ATH_DEBUG_RECV_DESC, "recv descriptors" },
	{ "rate", ATH_DEBUG_RATE, "rate control modules" },
	{ "reset", ATH_DEBUG_RESET, "reset processing and initialization" },
	{ "beacon", ATH_DEBUG_BEACON, "beacon handling" },
	{ "watchdog", ATH_DEBUG_WATCHDOG, "watchdog timer" },
	{ "intr", ATH_DEBUG_INTR, "interrupt processing" },
	{ "xmit_proc", ATH_DEBUG_TX_PROC, "processing of transmit descriptors" },
	{ "recv_proc", ATH_DEBUG_RX_PROC, "processing of receive descriptors" },
	{ "beacon_proc", ATH_DEBUG_BEACON_PROC, "beacon processing" },
	{ "calibrate", ATH_DEBUG_CALIBRATE, "periodic re-calibration" },
	{ "keycache", ATH_DEBUG_KEYCACHE, "key cache management" },
	{ "state", ATH_DEBUG_STATE, "802.11 state transitions" },
	{ "tsf", ATH_DEBUG_TSF, "TSF and timestamp processing" },
	{ "txbuf", ATH_DEBUG_TXBUF, "ath_buf management" },
	{ "skb", ATH_DEBUG_SKB, "skb management (affects all devs)" },
	{ "skb_ref", ATH_DEBUG_SKB_REF, "skb ref counting (affects all devs)" },
	{ "led", ATH_DEBUG_LED, "led management" },
	{ "ff", ATH_DEBUG_FF, "fast frame handling" },
	{ "turbo", ATH_DEBUG_TURBO, "dynamic turbo handling" },
	{ "uapsd", ATH_DEBUG_UAPSD, "WME/UAPSD handling" },
	{ "doth", ATH_DEBUG_DOTH, "802.11h handling" },
	{ "dothfilt", ATH_DEBUG_DOTHFILT, "802.11h radar pulse filter" },
	{ "dothfiltvbse", ATH_DEBUG_DOTHFILTVBSE, "802.11h radar pulse filter - verbose" },
	{ "dothfiltnosc", ATH_DEBUG_DOTHFILTNOSC, "802.11h radar pulse filter - disable short circuit" },
	{ "dothpulses", ATH_DEBUG_DOTHPULSES, "802.11h radar pulse events" },
	{ "fatal", ATH_DEBUG_FATAL, "fatal errors" },
};

static u_int getflag(const char *name, int len)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(flags); i++)
		if (strncasecmp(flags[i].name, name, len) == 0)
			return flags[i].bit;
	return 0;
}

static void usage(void)
{
	int i;

	fprintf(stderr, "usage: %s [-i device] [(+/-) flags]\n", progname);
	fprintf(stderr, "\twhere flags are:\n\n");
	for (i = 0; i < ARRAY_SIZE(flags); i++)
		printf("\t%12s\t0x%08x\t%s\n", flags[i].name, flags[i].bit, flags[i].desc);
	exit(-1);
}

#ifdef __linux__
static int sysctlbyname(const char *oid0, void *oldp, size_t *oldlenp, void *newp, size_t newlen)
{
	char oidcopy[1024], *oid;
	char path[1024];
	FILE *fd;

	strncpy(oidcopy, oid0, sizeof(oidcopy));
	oid = oidcopy;
	strcpy(path, "/proc/sys");
	do {
		char *cp, *tp;

		for (cp = oid; *cp != '\0' && *cp != '.'; cp++) ;
		if (*cp == '.')
			*cp++ = '\0';
		tp = strchr(path, '\0');
		snprintf(tp, sizeof(path) - (tp - path), "/%s", oid);
		oid = cp;
	} while (*oid != '\0');
	if (oldp != NULL) {
		fd = fopen(path, "r");
		if (fd == NULL)
			return -1;
		/* XXX: only handle ints */
		if (fscanf(fd, "%u", (int *)oldp) != 1) {
			fclose(fd);
			return -1;
		}
	} else {
		fd = fopen(path, "w");
		if (fd == NULL)
			return -1;
		/* XXX: only handle ints */
		(void)fprintf(fd, "%u", *(int *)newp);
	}
	fclose(fd);
	return 0;
}
#endif				/* __linux__ */

#ifdef DOMULTI

int athdebug_init(int argc, char *argv[])
{

#else

int main(int argc, char *argv[])
{

#endif

#ifdef __linux__
	const char *ifname = "wifi0";
#else
	const char *ifname = "ath0";
#endif
	const char *cp, *tp;
	const char *sep;
	int op, i;
	u_int32_t debug, ndebug;
	size_t debuglen;
	char oid[256];

	progname = argv[0];
	if (argc > 1) {
		if (strcmp(argv[1], "-i") == 0) {
			if (argc < 2)
				errx(1, "missing interface name for -i option");
			ifname = argv[2];
			argc -= 2;
			argv += 2;
		} else if (strcmp(argv[1], "-?") == 0)
			usage();
	}

#ifdef __linux__
	snprintf(oid, sizeof(oid), "dev.%s.debug", ifname);
#else
	snprintf(oid, sizeof(oid), "dev.ath.%s.debug", ifname + 3);
#endif
	debuglen = sizeof(debug);
	if (sysctlbyname(oid, &debug, &debuglen, NULL, 0) < 0) {
		if (argc <= 1)
			usage();	/* no user input, device not found - show usage instead of error message */
		else
			err(1, "sysctl-get(%s)", oid);	/* user specified arguments indicating a command, show error message */
	}
	ndebug = debug;
	for (; argc > 1; argc--, argv++) {
		cp = argv[1];
		do {
			u_int bit;

			if (*cp == '-') {
				cp++;
				op = -1;
			} else if (*cp == '+') {
				cp++;
				op = 1;
			} else
				op = 0;
			for (tp = cp; *tp != '\0' && *tp != '+' && *tp != '-';)
				tp++;
			bit = getflag(cp, tp - cp);
			if (op < 0)
				ndebug &= ~bit;
			else if (op > 0)
				ndebug |= bit;
			else {
				if (bit == 0) {
					if (isdigit(*cp))
						bit = strtoul(cp, NULL, 0);
					else
						errx(1, "unknown flag %.*s", (int)(tp - cp), cp);
				}
				ndebug = bit;
			}
		} while (*(cp = tp) != '\0');
	}
	if (debug != ndebug) {
		printf("%s: 0x%08x => ", oid, debug);
		if (sysctlbyname(oid, NULL, NULL, &ndebug, sizeof(ndebug)) < 0)
			err(1, "sysctl-set(%s)", oid);
		printf("0x%08x", ndebug);
		debug = ndebug;
	} else
		printf("%s: 0x%08x", oid, debug);
	sep = "<";
	for (i = 0; i < ARRAY_SIZE(flags); i++)
		if (debug & flags[i].bit) {
			printf("%s%s", sep, flags[i].name);
			sep = ",";
		}
	printf("%s\n", *sep != '<' ? ">" : "");
	printf("Details:\n");
	for (i = 0; i < ARRAY_SIZE(flags); i++)
		printf("%12s %s 0x%08x - %s\n", flags[i].name, debug & flags[i].bit ? "+" : " ", flags[i].bit, flags[i].desc);
	return 0;
}
