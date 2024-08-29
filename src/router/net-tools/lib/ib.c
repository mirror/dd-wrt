/*
 * lib/ib.c        This file contains an implementation of the "Infiniband"
 *              support functions.
 *
 * Version:     $Id: ib.c,v 1.1 2008/10/03 01:52:03 ecki Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *		Tom Duffy <tduffy@sun.com>
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWIB
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/if_infiniband.h>
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

extern struct hwtype ib_hwtype;


/* Display an InfiniBand address in readable format. */
static const char *pr_ib(const char *ptr)
{
    static char buff[128];
    char *pos;
    unsigned int i;

    pos = buff;
    for (i = 0; i < INFINIBAND_ALEN; i++) {
	pos += sprintf(pos, "%02X:", (*ptr++ & 0377));
    }
    buff[strlen(buff) - 1] = '\0';
    fprintf(stderr, _("Infiniband hardware address can be incorrect! Please read BUGS section in ifconfig(8).\n"));
    /* snprintf(buff, sizeof(buff), "%02X:%02X:%02X:%02X:%02X:%02X",
	     (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
	     (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377)
	);
    */
    return (buff);
}

#ifdef DEBUG
#define _DEBUG 1
#else
#define _DEBUG 0
#endif

/* Input an Infiniband address and convert to binary. */
static int in_ib(char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr *sap = (struct sockaddr *)sasp;
    char *ptr;
    char c, *orig;
    int i;
    unsigned val;

    sap->sa_family = ib_hwtype.type;
    ptr = sap->sa_data;

    i = 0;
    orig = bufp;
    while ((*bufp != '\0') && (i < INFINIBAND_ALEN)) {
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
		fprintf(stderr, _("in_ib(%s): invalid infiniband address!\n"), orig);
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
#ifdef DEBUG
	    fprintf(stderr, _("in_ib(%s): invalid infiniband address!\n"), orig);
#endif
	    errno = EINVAL;
	    return (-1);
	}
	if (c != 0)
	    bufp++;
	*ptr++ = (unsigned char) (val & 0377);
	i++;

	/* We might get a semicolon here - not required. */
	if (*bufp == ':') {
	    if (i == INFINIBAND_ALEN) {
#ifdef DEBUG
		fprintf(stderr, _("in_ib(%s): trailing : ignored!\n"),
			orig)
#endif
		    ;		/* nothing */
	    }
	    bufp++;
	}
    }

    /* That's it.  Any trailing junk? */
    if ((i == INFINIBAND_ALEN) && (*bufp != '\0')) {
#ifdef DEBUG
	fprintf(stderr, _("in_ib(%s): trailing junk!\n"), orig);
	errno = EINVAL;
	return (-1);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "in_ib(%s): %s\n", orig, pr_ib(sap->sa_data));
#endif

    return (0);
}


struct hwtype ib_hwtype =
{
    "infiniband", NULL, ARPHRD_INFINIBAND, INFINIBAND_ALEN,
    pr_ib, in_ib, NULL
};


#endif				/* HAVE_HWIB */
