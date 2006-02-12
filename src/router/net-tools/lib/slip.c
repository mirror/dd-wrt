/*
 * lib/slip.c This file contains the SLIP HW-type support.
 *
 * Version:     $Id: slip.c,v 1.4 1999/09/27 11:00:49 philip Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
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


struct hwtype slip_hwtype =
{
    "slip", NULL, /*"Serial Line IP", */ ARPHRD_SLIP, 0,
    NULL, NULL, NULL
};
struct hwtype cslip_hwtype =
{
    "cslip", NULL, /*"VJ Serial Line IP", */ ARPHRD_CSLIP, 0,
    NULL, NULL, NULL
};
struct hwtype slip6_hwtype =
{
    "slip6", NULL, /*"6-bit Serial Line IP", */ ARPHRD_SLIP6, 0,
    NULL, NULL, NULL
};
struct hwtype cslip6_hwtype =
{
    "cslip6", NULL, /*"VJ 6-bit Serial Line IP", */ ARPHRD_CSLIP6, 0,
    NULL, NULL, NULL
};
struct hwtype adaptive_hwtype =
{
    "adaptive", NULL, /*"Adaptive Serial Line IP", */ ARPHRD_ADAPT, 0,
    NULL, NULL, NULL
};
#endif				/* HAVE_HWSLIP */
