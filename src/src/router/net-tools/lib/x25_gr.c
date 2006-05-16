/*
 * lib/x25_gr.c	This file contains an implementation of the "X.25"
 *		route print support functions.
 *
 * Version:	lib/x25_gr.c	1.00	08/15/98
 *
 * Author:	Stephane Fillod, <sfillod@charybde.gyptis.frmug.org>
 * 		based on ax25_gr.c by:
 *		Bernd Eckenfels, <ecki@lina.inka.de>
 *		Copyright 1999 Bernd Eckenfels, Germany
 *		base on Code from Jonathan Naylor <jsn@Cs.Nott.AC.UK>
 *
 *		This program is free software; you can redistribute it
 *		and/or  modify it under  the terms of  the GNU General
 *		Public  License as  published  by  the  Free  Software
 *		Foundation;  either  version 2 of the License, or  (at
 *		your option) any later version.
 */
#include "config.h"

#if HAVE_AFX25
#if 0
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/x25.h>
#include <linux/if_arp.h>	/* ARPHRD_X25 */
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "net-support.h"
#include "pathnames.h"
#define  EXTERN
#if 0
#include "net-locale.h"
#endif
#include "intl.h"

/* is in net/x25.h, not in the public header file linux/x25.h. Why?*/
#ifndef X25_ADDR_LEN
#define X25_ADDR_LEN 16
#endif

int X25_rprint(int options)
{
	FILE *f=fopen(_PATH_PROCNET_X25_ROUTE, "r");
	char buffer[256];
	char *p;
	int  digits;

	if(f==NULL)
	{
		printf( _("X.25 not configured in this system.\n")); /* xxx */
		return 1;
	}
	printf( _("Kernel X.25 routing table\n")); /* xxx */
	printf( _("Destination          Iface\n")); /* xxx */
	fgets(buffer,256,f);
	while(fgets(buffer,256,f))
	{
		p = strchr(buffer,'\n');
		if (p)
			*p=0;

		buffer[24]=0;
		buffer[35]=0;
		digits=atoi(buffer+17);
		if (digits < 0 || digits > 15)
			digits=15;
		buffer[digits]=0;
		if (digits == 0)
			printf("*                    %-5s\n", buffer+25);
		else
			printf("%s/%*d   %-5s\n",
				buffer,digits-17,digits,buffer+25);
	}
	fclose(f);
	return 0;
}

#endif	/* HAVE_AFX25 */
