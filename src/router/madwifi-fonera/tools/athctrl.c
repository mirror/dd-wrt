/*-
 * Copyright (c) 2002-2004 Gunter Burchardt, Local-Web AG
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
 * $Id: athctrl.c 1436 2006-02-03 12:03:03Z mrenzmann $
 */

/*
 * Simple Atheros-specific tool to inspect and set atheros specific values
 * athctrl [-i interface] [-d distance]
 * (default interface is wifi0).  
 */
#include <sys/types.h>
#include <sys/file.h>

#include <getopt.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>

#include <net/if.h>

#ifdef DOMULTI
#include "do_multi.h"
#endif

static int
setsysctrl(const char *dev, const char *control , u_long value)
{
	char buffer[256];
	FILE * fd;

	snprintf(buffer, sizeof(buffer), "/proc/sys/dev/%s/%s", dev, control);
	fd = fopen(buffer, "w");
	if (fd != NULL) {
		fprintf(fd, "%li", value);
		fclose(fd);
	} else
		fprintf(stderr, "Could not open %s for writing!\n", buffer);
	
	return 0;
}

static void usage(void)
{
    fprintf(stderr,
        "Atheros driver control\n"
        "Copyright (c) 2002-2004 Gunter Burchardt, Local-Web AG\n"
        "\n"
        "usage: athctrl [-i interface] [-d distance]\n"
        "\n"
        "options:\n"
        "   -h   show this usage\n"
	"   -i   interface (default interface is wifi0)\n"
        "   -d   specify the maximum distance of a sta or the distance\n"
	"        of the master\n");

    exit(1);
}

#ifdef DOMULTI

int
athctrl_init(int argc, char *argv[])
{

#else

int
main(int argc, char *argv[])
{

#endif
	char device[IFNAMSIZ + 1];
	int distance = -1;
	int c;

	strncpy(device, "wifi0", sizeof (device));

	for (;;) {
        	c = getopt(argc, argv, "d:i:h");
        	if (c < 0)
			break;
        	switch (c) {
        	case 'h':
			usage();
			break;
        	case 'd':
			distance = atoi(optarg);
			break;
        	case 'i':
			strncpy(device, optarg, sizeof (device));
			break;
        	default:
			usage();
			break;
        	}
	}

	if (distance >= 0) {
        	int slottime = 9 + (distance / 300) + ((distance % 300) ? 1 : 0);
		int acktimeout = slottime * 2 + 3;
		int ctstimeout = slottime * 2 + 3;
		
		printf("Setting distance on interface %s to %i meters\n",
			device, distance);
		setsysctrl(device, "slottime", slottime);
		setsysctrl(device, "acktimeout", acktimeout);
		setsysctrl(device, "ctstimeout", ctstimeout);
	} else
		usage();
	return 0;
}
