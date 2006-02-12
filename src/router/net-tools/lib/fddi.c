/*
 * lib/fddi.c This file contains an implementation of the "FDDI"
 *              support functions.
 *
 * Version:     $Id: fddi.c,v 1.7 2000/03/05 11:26:02 philip Exp $
 *
 * Author:      Lawrence V. Stefani, <stefani@lkg.dec.com>
 *
 * 1998-07-01 - Arnaldo Carvalho de Melo <acme@conectiva.com.br> GNU gettext
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#include <features.h>

#if HAVE_HWFDDI
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#ifndef ARPHRD_FDDI
#error "No FDDI Support in your current Kernelsource Tree."
#error "Disable HW Type FDDI"
#endif
#if __GLIBC__ >= 2
#include <netinet/if_fddi.h>
#else
#include <linux/if_fddi.h>
#endif
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

extern struct hwtype fddi_hwtype;


/* Display an FDDI address in readable format. */
static char *pr_fddi(unsigned char *ptr)
{
    static char buff[64];

    snprintf(buff, sizeof(buff), "%02X-%02X-%02X-%02X-%02X-%02X",
	     (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
	     (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377)
	);
    return (buff);
}


/* Input an FDDI address and convert to binary. */
static int in_fddi(char *bufp, struct sockaddr *sap)
{
    unsigned char *ptr;
    char c, *orig;
    int i, val;

    sap->sa_family = fddi_hwtype.type;
    ptr = sap->sa_data;

    i = 0;
    orig = bufp;
    while ((*bufp != '\0') && (i < FDDI_K_ALEN)) {
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
	    fprintf(stderr, _("in_fddi(%s): invalid fddi address!\n"), orig);
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
	    fprintf(stderr, _("in_fddi(%s): invalid fddi address!\n"), orig);
#endif
	    errno = EINVAL;
	    return (-1);
	}
	*ptr++ = (unsigned char) (val & 0377);
	i++;

	/* We might get a semicolon here - not required. */
	if (*bufp == ':') {
	    if (i == FDDI_K_ALEN) {
#ifdef DEBUG
		fprintf(stderr, _("in_fddi(%s): trailing : ignored!\n"),
			orig)
#endif
		    ;		/* nothing */
	    }
	    bufp++;
	}
    }

    /* That's it.  Any trailing junk? */
    if ((i == FDDI_K_ALEN) && (*bufp != '\0')) {
#ifdef DEBUG
	fprintf(stderr, _("in_fddi(%s): trailing junk!\n"), orig);
	errno = EINVAL;
	return (-1);
#endif
    }
#ifdef DEBUG
    fprintf(stderr, "in_fddi(%s): %s\n", orig, pr_fddi(sap->sa_data));
#endif

    return (0);
}


struct hwtype fddi_hwtype =
{
    "fddi", NULL, /*"Fiber Distributed Data Interface (FDDI)", */ ARPHRD_FDDI, FDDI_K_ALEN,
    pr_fddi, in_fddi, NULL
};


#endif				/* HAVE_HWFDDI */
