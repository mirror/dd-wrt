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
 * $Id: 80211debug.c 1470 2006-03-10 13:23:50Z kelmo $
 */

/*
 * 80211debug [-i interface] flags
 * (default interface is ath0).
 */
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#ifdef DOMULTI
#include "do_multi.h"
#endif

#define	N(a)	(sizeof(a)/sizeof(a[0]))

const char *progname;

enum {
	IEEE80211_MSG_DEBUG	= 0x40000000,	/* IFF_DEBUG equivalent */
	IEEE80211_MSG_DUMPPKTS	= 0x20000000,	/* IFF_LINK2 equivalant, dump packets */
	IEEE80211_MSG_CRYPTO	= 0x10000000,	/* crypto modules */
	IEEE80211_MSG_INPUT	= 0x08000000,	/* input handling */
	IEEE80211_MSG_XRATE	= 0x04000000,	/* rate set handling */
	IEEE80211_MSG_ELEMID	= 0x02000000,	/* element id parsing */
	IEEE80211_MSG_NODE	= 0x01000000,	/* node handling */
	IEEE80211_MSG_ASSOC	= 0x00800000,	/* association handling */
	IEEE80211_MSG_AUTH	= 0x00400000,	/* authentication handling */
	IEEE80211_MSG_SCAN	= 0x00200000,	/* scanning */
	IEEE80211_MSG_OUTPUT	= 0x00100000,	/* output handling */
	IEEE80211_MSG_STATE	= 0x00080000,	/* state machine */
	IEEE80211_MSG_POWER	= 0x00040000,	/* power save handling */
	IEEE80211_MSG_DOT1X	= 0x00020000,	/* 802.1x authenticator */
	IEEE80211_MSG_DOT1XSM	= 0x00010000,	/* 802.1x state machine */
	IEEE80211_MSG_RADIUS	= 0x00008000,	/* 802.1x radius client */
	IEEE80211_MSG_RADDUMP	= 0x00004000,	/* dump 802.1x radius packets */
	IEEE80211_MSG_RADKEYS	= 0x00002000,	/* dump 802.1x keys */
	IEEE80211_MSG_WPA	= 0x00001000,	/* WPA/RSN protocol */
	IEEE80211_MSG_ACL	= 0x00000800,	/* ACL handling */
	IEEE80211_MSG_WME	= 0x00000400,	/* WME protocol */
	IEEE80211_MSG_SUPG	= 0x00000200,	/* SUPERG */
	IEEE80211_MSG_DOTH	= 0x00000100,	/* 11.h */
	IEEE80211_MSG_INACT	= 0x00000080,	/* inactivity handling */
	IEEE80211_MSG_ROAM	= 0x00000040,	/* sta-mode roaming */
	IEEE80211_MSG_ANY	= 0xffffffff
};

static struct {
	const char	*name;
	u_int		bit;
	const char *desc;
} flags[] = {
	{ "debug",	IEEE80211_MSG_DEBUG, "IFF_DEBUG equivalent" },
	{ "dumppkts",	IEEE80211_MSG_DUMPPKTS,  "dump packets" },
	{ "crypto",	IEEE80211_MSG_CRYPTO, "crypto modules" },
	{ "input",	IEEE80211_MSG_INPUT, "packet input handling" },
	{ "xrate",	IEEE80211_MSG_XRATE, "rate set handling" },
	{ "elemid",	IEEE80211_MSG_ELEMID, "element id parsing"},
	{ "node",	IEEE80211_MSG_NODE, "node management" },
	{ "assoc",	IEEE80211_MSG_ASSOC, "association handling" },
	{ "auth",	IEEE80211_MSG_AUTH, "authentication handling" },
	{ "scan",	IEEE80211_MSG_SCAN, "scanning" },
	{ "output",	IEEE80211_MSG_OUTPUT, "packet output handling" },
	{ "state",	IEEE80211_MSG_STATE, "802.11 state machine" },
	{ "power",	IEEE80211_MSG_POWER, "power save functions" },
	{ "dot1x",	IEEE80211_MSG_DOT1X, "802.1x authenticator" },
	{ "dot1xsm",	IEEE80211_MSG_DOT1XSM, "802.1x state machine" },
	{ "radius",	IEEE80211_MSG_RADIUS, "802.1x radius client" },
	{ "raddump",	IEEE80211_MSG_RADDUMP, "802.1x radius packet dump" },
	{ "radkeys",	IEEE80211_MSG_RADKEYS, "802.1x key dump" },
	{ "wpa",	IEEE80211_MSG_WPA, "WPA/RSN protocol" },
	{ "acl",	IEEE80211_MSG_ACL, "ACL handling" },
	{ "wme",	IEEE80211_MSG_WME, "WME protocol" },
	{ "superg",	IEEE80211_MSG_SUPG, "super G turbo mode" },
	{ "doth",	IEEE80211_MSG_DOTH, "802.11h (DFS/TPC) handling" },
	{ "inact",	IEEE80211_MSG_INACT, "timeout of inactive nodes"},
	{ "roam",	IEEE80211_MSG_ROAM, "station mode roaming" },
};

static u_int
getflag(const char *name, int len)
{
	int i;

	for (i = 0; i < N(flags); i++)
		if (strncasecmp(flags[i].name, name, len) == 0)
			return flags[i].bit;
	return 0;
}

static void
usage(void)
{
	int i;

	fprintf(stderr, "usage: %s [-i interface] [(+/-) flags]\n", progname);
	fprintf(stderr, "\twhere flags are:\n\n");
	for (i = 0; i < N(flags); i++)
		printf("\t%12s\t0x%08x\t%s\n", flags[i].name, flags[i].bit, flags[i].desc);
	exit(-1);
}

#ifdef __linux__
static int
sysctlbyname(const char *oid0, void *oldp, size_t *oldlenp,
	void *newp, size_t newlen)
{
	char oidcopy[1024], *oid;
	char path[1024];
	FILE *fd;

	strncpy(oidcopy, oid0, sizeof(oidcopy));
	oid = oidcopy;
	strcpy(path, "/proc/sys");
	do {
		char *cp, *tp;

		for (cp = oid; *cp != '\0' && *cp != '.'; cp++);
		if (*cp == '.')
			*cp++ = '\0';
		tp = strchr(path, '\0');
		snprintf(tp, sizeof(path) - (tp-path), "/%s", oid);
		oid = cp;
	} while (*oid != '\0');
	if (oldp != NULL) {
		fd = fopen(path, "r");
		if (fd == NULL)
			return -1;
		/* XXX only handle int's */
		if (fscanf(fd, "%u", (int *) oldp) != 1) {
			fclose(fd);
			return -1;
		}
	} else {
		fd = fopen(path, "w");
		if (fd == NULL)
			return -1;
		/* XXX only handle int's */
		(void) fprintf(fd, "%u", *(int *)newp);
	}
	fclose(fd);
	return 0;
}
#endif /* __linux__ */

#ifdef DOMULTI

int
a80211debug_init(int argc, char *argv[])
{

#else

int
main(int argc, char *argv[])
{

#endif
	const char *ifname = "ath0";
	const char *cp, *tp;
	const char *sep;
	int op, i;
	u_int32_t debug, ndebug;
	size_t debuglen;
	char oid[256];

	progname = argv[0];
	if (argc > 1) {
		if (strcmp(argv[1], "-i") == 0) {
			if (argc <= 2)
				errx(1, "missing interface name for -i option");
			ifname = argv[2];
			argc -= 2, argv += 2;
		} else if (strcmp(argv[1], "-?") == 0)
			usage();
	}

#ifdef __linux__
	snprintf(oid, sizeof(oid), "net.%s.debug", ifname);
#else
	snprintf(oid, sizeof(oid), "dev.wlan.%s.debug", ifname+4);
#endif
	debuglen = sizeof(debug);
	if (sysctlbyname(oid, &debug, &debuglen, NULL, 0) < 0)
		err(1, "sysctl-get(%s)", oid);
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
			bit = getflag(cp, tp-cp);
			if (op < 0)
				ndebug &= ~bit;
			else if (op > 0)
				ndebug |= bit;
			else {
				if (bit == 0) {
					if (isdigit(*cp))
						bit = strtoul(cp, NULL, 0);
					else
						errx(1, "unknown flag %.*s",
							(int)(tp-cp), cp);
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
	for (i = 0; i < N(flags); i++)
		if (debug & flags[i].bit) {
			printf("%s%s", sep, flags[i].name);
			sep = ",";
		}
	printf("%s\n", *sep != '<' ? ">" : "");
	return 0;
}


