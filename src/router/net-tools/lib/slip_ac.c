/*
 * lib/slip_ac.c      This file contains the activation for the
 *                      SLIP line disciplines, called from activate_ld().
 *
 * Version:     $Id: slip_ac.c,v 1.3 1998/11/15 20:12:20 freitag Exp $
 *
 * Author:      Bernd 'eckes' Eckenfels
 *              Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              Modified by Alan Cox, May 94 to cover NET-3
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWSLIP

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


/* Set the line discipline of a terminal line. */
static int SLIP_set_disc(int fd, int disc)
{
    if (ioctl(fd, TIOCSETD, &disc) < 0) {
	fprintf(stderr, "SLIP_set_disc(%d): %s\n", disc, strerror(errno));
	return (-errno);
    }
    return (0);
}


/* Set the encapsulation type of a terminal line. */
static int SLIP_set_encap(int fd, int encap)
{
    if (ioctl(fd, SIOCSIFENCAP, &encap) < 0) {
	fprintf(stderr, "SLIP_set_encap(%d): %s\n", encap, strerror(errno));
	return (-errno);
    }
    return (0);
}


/* Start the SLIP encapsulation on the file descriptor. */
int SLIP_activate(int fd)
{
    if (SLIP_set_disc(fd, N_SLIP) < 0)
	return (-1);
    if (SLIP_set_encap(fd, 0) < 0)
	return (-1);
    return (0);
}


/* Start the VJ-SLIP encapsulation on the file descriptor. */
int CSLIP_activate(int fd)
{
    if (SLIP_set_disc(fd, N_SLIP) < 0)
	return (-1);
    if (SLIP_set_encap(fd, 1) < 0)
	return (-1);
    return (0);
}


/* Start the SLIP-6 encapsulation on the file descriptor. */
int SLIP6_activate(int fd)
{
    if (SLIP_set_disc(fd, N_SLIP) < 0)
	return (-1);
    if (SLIP_set_encap(fd, 2) < 0)
	return (-1);
    return (0);
}


/* Start the VJ-SLIP-6 encapsulation on the file descriptor. */
int CSLIP6_activate(int fd)
{
    if (SLIP_set_disc(fd, N_SLIP) < 0)
	return (-1);
    if (SLIP_set_encap(fd, 3) < 0)
	return (-1);
    return (0);
}


/* Start adaptive encapsulation on the file descriptor. */
int ADAPTIVE_activate(int fd)
{
    if (SLIP_set_disc(fd, N_SLIP) < 0)
	return (-1);
    if (SLIP_set_encap(fd, 8) < 0)
	return (-1);
    return (0);
}
#endif				/* HAVE_HWSLIP */
