/*
 * lib/ax25_gr.c      This file contains an implementation of the "AX.25"
 *                      route print support functions.
 *
 * Version:     $Id: ax25_gr.c,v 1.4 1999/01/05 20:53:21 philip Exp $
 *
 * Author:      Bernd Eckenfels, <ecki@lina.inka.de>
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

#if HAVE_AFAX25
#if 0
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/ax25.h>
#include <linux/if_arp.h>	/* ARPHRD_AX25 */
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

int AX25_rprint(int options)
{
    FILE *f = fopen(_PATH_PROCNET_AX25_ROUTE, "r");
    char buffer[256];
    int use;

    if (f == NULL) {
        perror(_PATH_PROCNET_AX25_ROUTE);
	printf(_("AX.25 not configured in this system.\n"));	/* xxx */
	return 1;
    }
    printf(_("Kernel AX.25 routing table\n"));	/* xxx */
    printf(_("Destination  Iface    Use\n"));	/* xxx */
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

#endif				/* HAVE_AFAX25 */
