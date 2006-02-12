
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
    FILE *f = NULL;
    char buffer[256];
    int use;

    f=fopen(_PATH_PROCNET_ROSE_ROUTE, "r");
    if (f == NULL) {
	perror(_PATH_PROCNET_ROSE_ROUTE);
	printf(_("ROSE not configured in this system.\n"));	/* xxx */
	return 1;
    }
    printf(_("Kernel ROSE routing table\n"));
    printf(_("Destination  Iface    Use\n"));
    fgets(buffer, 256, f);
    while (fgets(buffer, 256, f)) {
	buffer[9] = 0;
	buffer[14] = 0;
	use = atoi(buffer + 15);
	printf("%-9s    %-5s  %5d\n",
	       buffer, buffer + 10, use);
    }
    fclose(f);
    return 0;
}

#endif				/* HAVE_AFROSE */
