/*
 * lib/loopback.c     This file contains the general hardware types.
 *
 * Version:     $Id: loopback.c,v 1.7 1999/09/27 11:00:48 philip Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 * Modifications:
 * 1998-07-01 - Arnaldo Carvalho de Melo - GNU gettext instead of catgets
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
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

/* Display an UNSPEC address. */
static char *pr_unspec(unsigned char *ptr)
{
    static char buff[64];
    char *pos;
    unsigned int i;

    pos = buff;
    for (i = 0; i < sizeof(struct sockaddr); i++) {
	pos += sprintf(pos, "%02X-", (*ptr++ & 0377));
    }
    buff[strlen(buff) - 1] = '\0';
    return (buff);
}

struct hwtype unspec_hwtype =
{
    "unspec", NULL, /*"UNSPEC", */ -1, 0,
    pr_unspec, NULL, NULL
};

struct hwtype loop_hwtype =
{
    "loop", NULL, /*"Local Loopback", */ ARPHRD_LOOPBACK, 0,
    NULL, NULL, NULL
};
