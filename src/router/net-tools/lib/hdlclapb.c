/*
 * lib/hdlclapb.c 
 *              This file contains the HDLC/LAPB support for the NET-2 base
 *              distribution.
 *
 * Version:    $Id: hdlclapb.c,v 1.5 2000/03/05 11:26:02 philip Exp $
 *
 * Original Author:     
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

#if HAVE_HWHDLCLAPB

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

#ifndef ARPHRD_HDLC
#warning "No definition of ARPHRD_HDLC in <net/if_arp.h>, using private value 513"
#define ARPHRD_HDLC 513
#endif

#ifndef ARPHRD_LAPB
#warning "No definition of ARPHRD_HDLC in <net/if_arp.h>, using private value 516"
#define ARPHRD_LAPB 516
#endif

struct hwtype hdlc_hwtype =
{
    "hdlc", NULL, /*"(Cisco) HDLC", */ ARPHRD_HDLC, 0,
    NULL, NULL, NULL, 0
};
struct hwtype lapb_hwtype =
{
    "lapb", NULL, /*"LAPB", */ ARPHRD_LAPB, 0,
    NULL, NULL, NULL, 0
};

#endif				/* HAVE_HWHDLCLAPB */
