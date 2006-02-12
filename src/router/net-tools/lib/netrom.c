/*
 * lib/netrom.c       This file contains an implementation of the "NET/ROM"
 *              support functions for the NET-2 base distribution.
 *
 * Version:     $Id: netrom.c,v 1.8 2000/03/05 11:26:03 philip Exp $
 *
 * NOTE:        I will redo this module as soon as I got the libax25.a
 *              library sorted out.  This library contains some useful
 *              and often used address conversion functions, database
 *              lookup stuff, and more of the like.
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 * 
 * Changes:
 * 980701 {1.21} Arnaldo Carvalho de Melo - GNU gettext instead of catgets,
 *                                          strncpy instead of strcpy for
 *                                          i18n strings
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_AFNETROM || HAVE_HWNETROM
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netax25/ax25.h>
#else
#include <linux/ax25.h>
#endif
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

static char netrom_errmsg[128];

extern struct aftype netrom_aftype;

static char *NETROM_print(unsigned char *ptr)
{
    static char buff[8];
    int i;

    for (i = 0; i < 6; i++) {
	buff[i] = ((ptr[i] & 0377) >> 1);
	if (buff[i] == ' ')
	    buff[i] = '\0';
    }
    buff[6] = '\0';
    i = ((ptr[6] & 0x1E) >> 1);
    if (i != 0)
	sprintf(&buff[strlen(buff)], "-%d", i);
    return (buff);
}


/* Display an AX.25 socket address. */
static char *NETROM_sprint(struct sockaddr *sap, int numeric)
{
    char buf[64];
    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return safe_strncpy(buf, _("[NONE SET]"), sizeof(buf));
    return (NETROM_print(((struct sockaddr_ax25 *) sap)->sax25_call.ax25_call));
}


static int NETROM_input(int type, char *bufp, struct sockaddr *sap)
{
    unsigned char *ptr;
    char *orig, c;
    unsigned int i;

    sap->sa_family = netrom_aftype.af;
    ptr = ((struct sockaddr_ax25 *) sap)->sax25_call.ax25_call;

    /* First, scan and convert the basic callsign. */
    orig = bufp;
    i = 0;
    while ((*bufp != '\0') && (*bufp != '-') && (i < 6)) {
	c = *bufp++;
	if (islower(c))
	    c = toupper(c);
	if (!(isupper(c) || isdigit(c))) {
	    safe_strncpy(netrom_errmsg, _("Invalid callsign"), sizeof(netrom_errmsg));
#ifdef DEBUG
	    fprintf(stderr, "netrom_input(%s): %s !\n", netrom_errmsg, orig);
#endif
	    errno = EINVAL;
	    return (-1);
	}
	*ptr++ = (unsigned char) ((c << 1) & 0xFE);
	i++;
    }

    /* Callsign too long? */
    if ((i == 6) && (*bufp != '-') && (*bufp != '\0')) {
	safe_strncpy(netrom_errmsg, _("Callsign too long"), sizeof(netrom_errmsg));
#ifdef DEBUG
	fprintf(stderr, "netrom_input(%s): %s !\n", netrom_errmsg, orig);
#endif
	errno = E2BIG;
	return (-1);
    }
    /* Nope, fill out the address bytes with blanks. */
    while (i++ < sizeof(ax25_address) - 1) {
	*ptr++ = (unsigned char) ((' ' << 1) & 0xFE);
    }

    /* See if we need to add an SSID field. */
    if (*bufp == '-') {
	i = atoi(++bufp);
	*ptr = (unsigned char) ((i << 1) & 0xFE);
    } else {
	*ptr = (unsigned char) '\0';
    }

    /* All done. */
#ifdef DEBUG
    fprintf(stderr, "netrom_input(%s): ", orig);
    for (i = 0; i < sizeof(ax25_address); i++)
	fprintf(stderr, "%02X ", sap->sa_data[i] & 0377);
    fprintf(stderr, "\n");
#endif

    return (0);
}


/* Display an error message. */
static void NETROM_herror(char *text)
{
    if (text == NULL)
	fprintf(stderr, "%s\n", netrom_errmsg);
    else
	fprintf(stderr, "%s: %s\n", text, netrom_errmsg);
}


static int NETROM_hinput(char *bufp, struct sockaddr *sap)
{
    if (NETROM_input(0, bufp, sap) < 0)
	return (-1);
    sap->sa_family = ARPHRD_NETROM;
    return (0);
}

#if 0
/* Set the line discipline of a terminal line. */
static int KISS_set_disc(int fd, int disc)
{
    if (ioctl(fd, TIOCSETD, &disc) < 0) {
	fprintf(stderr, "KISS_set_disc(%d): %s\n", disc, strerror(errno));
	return (-errno);
    }
    return (0);
}


/* Start the KISS encapsulation on the file descriptor. */
static int KISS_init(int fd)
{
    if (KISS_set_disc(fd, N_SLIP) < 0)
	return (-1);
    if (ioctl(fd, SIOCSIFENCAP, 4) < 0)
	return (-1);
    return (0);
}
#endif

struct hwtype netrom_hwtype =
{
    "netrom", NULL, /* "AMPR NET/ROM", */ ARPHRD_NETROM, 7,
    NETROM_print, NETROM_hinput, NULL, 0
};

struct aftype netrom_aftype =
{
    "netrom", NULL, /* "AMPR NET/ROM", */ AF_NETROM, 7,
    NETROM_print, NETROM_sprint, NETROM_input, NETROM_herror,
    NULL, NULL, NULL,
    -1,
    "/proc/net/nr"
};

#endif				/* HAVE_AFNETROM */
