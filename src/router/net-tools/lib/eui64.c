/*
 * lib/eui64.c  This file contains support for generic EUI-64 hw addressing
 *
 * Version:     $Id: eui64.c,v 1.1 2001/11/12 02:12:05 ecki Exp $
 *
 * Author:      Daniel Stodden <stodden@in.tum.de>
 *              Copyright 2001 Daniel Stodden
 *
 *              blueprinted from ether.c
 *              Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWEUI64

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"

/*
 * EUI-64 constants
 */

#define EUI64_ALEN	8

#ifndef ARPHRD_EUI64
#define ARPHRD_EUI64	27
#warning "ARPHRD_EUI64 not defined in <net/if_arp.h>. Using private value 27"
#endif

struct hwtype eui64_hwtype;

/* Display an EUI-64 address in readable format. */
static const char *pr_eui64(const char *ptr)
{
	static char buff[64];

	snprintf(buff, sizeof(buff), "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
		 (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377), (ptr[3] & 0377),
		 (ptr[4] & 0377), (ptr[5] & 0377), (ptr[6] & 0377), (ptr[7] & 0377)
		);
	return (buff);
}

#ifdef DEBUG
#define _DEBUG 1
#else
#define _DEBUG 0
#endif

/* Start the PPP encapsulation on the file descriptor. */
static int in_eui64(char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr *sap = (struct sockaddr *)sasp;
	char *ptr;
	char c, *orig;
	int i;
	unsigned val;

	sap->sa_family = eui64_hwtype.type;
	ptr = sap->sa_data;

	i = 0;
	orig = bufp;

	while ((*bufp != '\0') && (i < EUI64_ALEN)) {
		val = 0;
		c = *bufp++;
		if (isdigit(c))
			val = c - '0';
		else if (c >= 'a' && c <= 'f')
			val = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			val = c - 'A' + 10;
		else {
			if (_DEBUG)
				fprintf( stderr, _("in_eui64(%s): invalid eui64 address!\n"),
					 orig );
			errno = EINVAL;
			return (-1);
		}

		val <<= 4;
		c = *bufp;
		if (isdigit(c))
			val |= c - '0';
		else if (c >= 'a' && c <= 'f')
			val |= c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			val |= c - 'A' + 10;
		else if (c == ':' || c == 0)
			val >>= 4;
		else {
			if (_DEBUG)
				fprintf( stderr, _("in_eui64(%s): invalid eui64 address!\n"),
					 orig );
			errno = EINVAL;
			return (-1);
		}

		if (c != 0)
			bufp++;

		*ptr++ = (unsigned char) (val & 0377);
		i++;

		/* We might get a semicolon here - not required. */
		if (*bufp == ':') {
			if (_DEBUG && i == EUI64_ALEN)
				fprintf(stderr, _("in_eui64(%s): trailing : ignored!\n"),
					orig);
			bufp++;
		}
	}

	/* That's it.  Any trailing junk? */
	if (_DEBUG && (i == EUI64_ALEN) && (*bufp != '\0')) {
		fprintf(stderr, _("in_eui64(%s): trailing junk!\n"), orig);
		errno = EINVAL;
		return (-1);
	}
	if (_DEBUG)
		fprintf(stderr, "in_eui64(%s): %s\n", orig, pr_eui64(sap->sa_data));

    return (0);
}

struct hwtype eui64_hwtype =
{
	"eui64", NULL, /*"EUI-64 addressing", */ ARPHRD_EUI64, EUI64_ALEN,
	pr_eui64, in_eui64, NULL, 0
};


#endif				/* HAVE_EUI64 */
