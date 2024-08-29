
/*
 * lib/rose_gr.c      This file contains an implementation of the "ROSE"
 *                      route print support functions.
 *
 * Version:     $Id: rose_gr.c,v 1.4 1999/01/05 20:54:07 philip Exp $
 *
 * Author:      Terry Dawson, VK2KTJ, <terry@perf.no.itg.telstra.com.au>
 *              based on ax25_gr.c by:
 *              Bernd Eckenfels, <ecki@lina.inka.de>
 *              Copyright 1999 Bernd Eckenfels, Germany
 *              base on Code from Jonathan Naylor <jsn@Cs.Nott.AC.UK>
 *
 *              Bernard Pidoux f6bvp@amsat.org added ROSE_NEIGH proc
 *              completing ROSE routing table - November 2009.
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_AFROSE
#if 0
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/rose.h>
#include <sys/socket.h>
#include <net/if_arp.h>		/* ARPHRD_ROSE */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"

int ROSE_rprint(int options)
{
    FILE *f1, *f2;
    char buffer1[256], buffer2[256];
    int i, neigh, use;

    f2 = fopen(_PATH_PROCNET_ROSE_NEIGH, "r");
    f1 = fopen(_PATH_PROCNET_ROSE_NODES, "r");
    if (f1 == NULL) {
	perror(_PATH_PROCNET_ROSE_NODES);
	fprintf(stderr, _("ROSE not configured in this system.\n"));	/* xxx */
	return 1;
    }
    printf(_("Kernel ROSE routing table\n"));
    printf(_("Destination  neigh1 callsign  device  neigh2 callsign  device  neigh3 callsign  device\n"));
    if (fgets(buffer1, 256, f1))
	/* eat line */;
    while (fgets(buffer1, 256, f1)) {
	buffer1[10] = 0; /* address */
	buffer1[15] = 0; /* mask */
	buffer1[17] = 0; /* use */
	buffer1[23] = 0; /* neigh 1 */
	buffer1[29] = 0; /* neigh 2 */
	buffer1[35] = 0; /* neigh 3 */
	use = atoi(buffer1 + 16);
	neigh = atoi(buffer1 + 18);
	printf("%-10s   ", buffer1);
	for (i = 0; i < use; i++) {
		neigh = atoi(buffer1 + 6 * (i + 3));
		printf("%05d  ", neigh);
		rewind(f2);
		if (fgets(buffer2, 256, f2))
			/* eat line */;
		while (fgets(buffer2, 256, f2)) {
			buffer2[15] = 0;
			buffer2[21] = 0;
			if (atoi(buffer2) == neigh)
				printf("%-10s   %-4s", buffer2 + 6, buffer2 + 16);
		}
	}
	printf("\n");
    }
    fclose(f1);
    fclose(f2);
    return 0;
}

#endif				/* HAVE_AFROSE */
