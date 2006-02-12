/*
 * lib/ppp_ac.c       This file contains the activation for the
 *              PPP line disciplines, called from activate_ld().
 *
 * Version:     $Id: ppp_ac.c,v 1.3 1998/11/15 20:11:50 freitag Exp $
 *
 * Author:      Bernd 'eckes' Eckenfels
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWPPP

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

/* Start the VJ-SLIP encapsulation on the file descriptor. */
int PPP_activate(int fd)
{
    fprintf(stderr, _("Sorry, use pppd!\n"));	/* FIXME */
    return (-1);
}

#endif				/* HAVE_HWPPP */
