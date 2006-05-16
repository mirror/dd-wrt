/*
 * lib/hippi.c        This file contains an implementation of the "HIPPI"
 *              support functions for the NET-2 base distribution.
 *
 * Version:     $Id$
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              Modified for HIPPI by Jes Sorensen, <Jes.Sorensen@cern.ch>
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWHIPPI
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
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

/*
 *    HIPPI magic constants.
 */

#define HIPPI_ALEN	6	/* Bytes in one HIPPI hw-addr        */
#ifndef ARPHRD_HIPPI
#define ARPHRD_HIPPI    780
#warning "ARPHRD_HIPPI is not defined in <net/if_arp.h>. Using private value 708"
#endif

extern struct hwtype hippi_hwtype;


/* Display an HIPPI address in readable format. */
static char *pr_hippi(unsigned char *ptr)
{
    static char buff[64];

    sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
	    (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
	    (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377)
	);
    return (buff);
}


/* Input an HIPPI address and convert to binary. */
static int in_hippi(char *bufp, struct sockaddr *sap)
{
    unsigned char *ptr;
    char c, *orig;
    int i, val;

    sap->sa_family = hippi_hwtype.type;
    ptr = sap->sa_data;

    i = 0;
    orig = bufp;
    while ((*bufp != '\0') && (i < HIPPI_ALEN)) {
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
	    fprintf(stderr, _("in_hippi(%s): invalid hippi address!\n"), orig);
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
	    fprintf(stderr, _("in_hippi(%s): invalid hippi address!\n"), orig);
#endif
	    errno = EINVAL;
	    return (-1);
	}
	*ptr++ = (unsigned char) (val & 0377);
	i++;

	/* We might get a semicolon here - not required. */
	if (*bufp == ':') {
	    if (i == HIPPI_ALEN) {
#ifdef DEBUG
		fprintf(stderr, _("in_hippi(%s): trailing : ignored!\n"), orig)
#endif
		    ;		/* nothing */
	    }
	    bufp++;
	}
    }

    /* That's it.  Any trailing junk? */
    if ((i == HIPPI_ALEN) && (*bufp != '\0')) {
#ifdef DEBUG
	fprintf(stderr, _("in_hippi(%s): trailing junk!\n"), orig);
	errno = EINVAL;
	return (-1);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "in_hippi(%s): %s\n", orig, pr_hippi(sap->sa_data));
#endif

    return (0);
}


struct hwtype hippi_hwtype =
{
    "hippi", NULL, /*"HIPPI", */ ARPHRD_HIPPI, HIPPI_ALEN,
    pr_hippi, in_hippi, NULL, 0
};


#endif				/* HAVE_HWHIPPI */
