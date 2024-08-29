/*
 * lib/rose.c This file contains an implementation of the "ROSE"
 *              support functions for the NET-2 base distribution.
 *
 * Version:     $Id: rose.c,v 1.7 2000/03/05 11:26:03 philip Exp $
 *
 * Author:      Terry Dawson, VK2KTJ, <terry@perf.no.itg.telstra.com.au>
 *              based on ax25.c by:
 *              Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_AFROSE || HAVE_HWROSE
#include <features.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>		/* ARPHRD_ROSE */
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
#include "util.h"

#ifndef _NETROSE_ROSE_H
#include <linux/ax25.h>
#include <linux/rose.h>
/* this will check for the broken #define PF_ROSE AF_ROSE define in some older kernel headers */
#undef AF_ROSE
#if PF_ROSE == AF_ROSE
#warning "Your <linux/rose.h> is broken and defines PF_ROSE, better remove the define in /usr/include/linux/rose.h (using private define for PF_ROSE meanwhile)"
#undef PF_ROSE
#define PF_ROSE         11      /* Amateur Radio X.25 PLP       */
#endif
/* now restore the value of AF_ROSE (which had to be deleted to catch the case where #define AF_ROSE PF_ROSE) */
#define AF_ROSE         PF_ROSE
#endif

static char ROSE_errmsg[128];

extern struct aftype rose_aftype;

static const char *
 ROSE_print(const char *ptr)
{
    static char buff[12];

    snprintf(buff, sizeof(buff), "%02hhx%02hhx%02hhx%02hhx%02hhx",
	     ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]);
    buff[10] = '\0';
    return (buff);
}

/* Display a ROSE socket address. */
static const char *
 ROSE_sprint(const struct sockaddr_storage *sasp, int numeric)
{
    const struct sockaddr_rose *rose_sap = (const struct sockaddr_rose *)sasp;
    const struct sockaddr *sap = (const struct sockaddr *)sasp;
    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return _("[NONE SET]");

    return ROSE_print(rose_sap->srose_addr.rose_addr);
}

static int ROSE_input(int type, char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr *sap = (struct sockaddr *)sasp;
    char *ptr;
    int i, o;

    sap->sa_family = rose_aftype.af;
    ptr = ((struct sockaddr_rose *) sasp)->srose_addr.rose_addr;

    /* Node address the correct length ? */
    if (strlen(bufp) != 10) {
	safe_strncpy(ROSE_errmsg, _("Node address must be ten digits"), sizeof(ROSE_errmsg));
	errno = EINVAL;
	return (-1);
    }
    /* Ok, lets set it */
    for (i = 0, o = 0; i < 5; i++) {
	o = i * 2;
	ptr[i] = (((bufp[o] - '0') << 4) | (bufp[o + 1] - '0'));
    }

    /* All done. */
    return (0);
}


/* Display an error message. */
static void ROSE_herror(const char *text)
{
    if (text == NULL)
	fprintf(stderr, "%s\n", ROSE_errmsg);
    else
	fprintf(stderr, "%s: %s\n", text, ROSE_errmsg);
}


static int ROSE_hinput(char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr *sap = (struct sockaddr *)sasp;
    if (ROSE_input(0, bufp, sasp) < 0)
	return (-1);
    sap->sa_family = ARPHRD_ROSE;
    return (0);
}

struct hwtype rose_hwtype =
{
    "rose", NULL, /*"AMPR ROSE", */ ARPHRD_ROSE, 10,
    ROSE_print, ROSE_hinput, NULL
};

struct aftype rose_aftype =
{
    "rose", NULL, /*"AMPR ROSE", */ AF_ROSE, 10,
    ROSE_print, ROSE_sprint, ROSE_input, ROSE_herror,
    NULL, NULL, NULL,
    -1,
    "/proc/net/rose"
};

#endif				/* HAVE_xxROSE */
