/*
 * lib/frame.c        This file contains the Frame Relay support.
 *
 * Version:     $Id: frame.c,v 1.4 2000/03/05 11:26:02 philip Exp $
 *
 * Maintainer:  Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author:      Mike McLagan <mike.mclagan@linux.org>
 *
 * Changes:
 *
 *962303 {0.01} Mike McLagan :          creation
 *960413 {0.02} Bernd Eckenfels :       included in net-lib
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_HWFR

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

static const char *pr_dlci(const char *ptr)
{
    static char buf[12];
    short i;
    memcpy(&i, ptr, sizeof(i));
    snprintf(buf, sizeof(buf), "%i", i);
    return (buf);
}

struct hwtype dlci_hwtype =
{
    "dlci", NULL, /*"Frame Relay DLCI", */ ARPHRD_DLCI, 3,
    pr_dlci, NULL, NULL, 0
};

struct hwtype frad_hwtype =
{
    "frad", NULL, /*"Frame Relay Access Device", */ ARPHRD_FRAD, 0,
    NULL, NULL, NULL, 0
};
#endif				/* HAVE_HWFR */
