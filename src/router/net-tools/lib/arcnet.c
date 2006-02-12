/*
 * lib/arcnet.c       This file contains an implementation of the "ARCnet"
 *              support functions for the NET-2 base distribution.
 *
 * Version:     $Id: arcnet.c,v 1.6 2000/03/05 11:26:02 philip Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWARC
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

extern struct hwtype arcnet_hwtype;


/* Display an ARCnet address in readable format. */
static char *pr_arcnet(unsigned char *ptr)
{
    static char buff[64];

    snprintf(buff, sizeof(buff), "%02X", (ptr[0] & 0377));
    return (buff);
}


/* Input an ARCnet address and convert to binary. */
static int in_arcnet(char *bufp, struct sockaddr *sap)
{
    unsigned char *ptr;
    char c, *orig;
    int i, val;

    sap->sa_family = arcnet_hwtype.type;
    ptr = sap->sa_data;

    i = 0;
    orig = bufp;
    while ((*bufp != '\0') && (i < 1)) {
	val = 0;
	c = *bufp++;
	if (isdigit(c))
	    val = c - '0';
	else if (c >= 'a' && c <= 'f')
	    val = c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
	    val = c - 'A' + 10;
	else {
#ifdef DEBUG
	    fprintf(stderr, _("in_arcnet(%s): invalid arcnet address!\n"), orig);
#endif
	    errno = EINVAL;
	    return (-1);
	}
	val <<= 4;
	c = *bufp++;
	if (isdigit(c))
	    val |= c - '0';
	else if (c >= 'a' && c <= 'f')
	    val |= c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
	    val |= c - 'A' + 10;
	else {
#ifdef DEBUG
	    fprintf(stderr, _("in_arcnet(%s): invalid arcnet address!\n"), orig);
#endif
	    errno = EINVAL;
	    return (-1);
	}
	*ptr++ = (unsigned char) (val & 0377);
	i++;

	/* We might get a semicolon here - not required. */
	if (*bufp == ':') {
	    if (i == ETH_ALEN) {
#ifdef DEBUG
		fprintf(stderr, _("in_arcnet(%s): trailing : ignored!\n"),
			orig)
#endif
		    ;		/* nothing */
	    }
	    bufp++;
	}
    }

    /* That's it.  Any trailing junk? */
    if ((i == ETH_ALEN) && (*bufp != '\0')) {
#ifdef DEBUG
	fprintf(stderr, _("in_arcnet(%s): trailing junk!\n"), orig);
	errno = EINVAL;
	return (-1);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "in_arcnet(%s): %s\n", orig, pr_arcnet(sap->sa_data));
#endif

    return (0);
}


struct hwtype arcnet_hwtype =
{
    "arcnet", NULL, /*"2.5Mbps ARCnet", */ ARPHRD_ARCNET, 1,
    pr_arcnet, in_arcnet, NULL
};


#endif				/* HAVE_HWARC */
