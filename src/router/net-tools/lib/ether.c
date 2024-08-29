/*
 * lib/ether.c        This file contains an implementation of the "Ethernet"
 *              support functions.
 *
 * Version:     $Id: ether.c,v 1.8 2002/07/30 05:17:29 ecki Exp $
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

#if HAVE_HWETHER
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

extern struct hwtype ether_hwtype;


/* Display an Ethernet address in readable format. */
static const char *pr_ether(const char *ptr)
{
    static char buff[64];

    snprintf(buff, sizeof(buff), "%02x:%02x:%02x:%02x:%02x:%02x",
	     (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
	     (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377)
	);
    return (buff);
}

#ifdef DEBUG
#define _DEBUG 1
#else
#define _DEBUG 0
#endif

/* Input an Ethernet address and convert to binary. */
static int in_ether(char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr *sap = (struct sockaddr *)sasp;
    char *ptr;
    char c, *orig;
    int i;
    unsigned val;

    sap->sa_family = ether_hwtype.type;
    ptr = sap->sa_data;

    i = 0;
    orig = bufp;
    while ((*bufp != '\0') && (i < ETH_ALEN)) {
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
		fprintf(stderr, _("in_ether(%s): invalid ether address!\n"), orig);
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
		fprintf(stderr, _("in_ether(%s): invalid ether address!\n"), orig);
	    errno = EINVAL;
	    return (-1);
	}
	if (c != 0)
	    bufp++;
	*ptr++ = (unsigned char) (val & 0377);
	i++;

	/* We might get a semicolon here - not required. */
	if (*bufp == ':') {
	    if (_DEBUG && i == ETH_ALEN)
		fprintf(stderr, _("in_ether(%s): trailing : ignored!\n"),
			orig);
	    bufp++;
	}
    }

    /* That's it.  Any trailing junk? */
    if (_DEBUG && (i == ETH_ALEN) && (*bufp != '\0')) {
	fprintf(stderr, _("in_ether(%s): trailing junk!\n"), orig);
	errno = EINVAL;
	return (-1);
    }
    if (_DEBUG)
	fprintf(stderr, "in_ether(%s): %s\n", orig, pr_ether(sap->sa_data));

    return (0);
}


struct hwtype ether_hwtype =
{
    "ether", NULL, /*"10Mbps Ethernet", */ ARPHRD_ETHER, ETH_ALEN,
    pr_ether, in_ether, NULL
};


#endif				/* HAVE_HWETHER */
