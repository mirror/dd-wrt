/*
 * lib/ash.c  This file contains an implementation of the Ash
 *              support functions for the NET-2 base distribution.
 * $Id: ash.c,v 1.11 1999/09/27 11:00:45 philip Exp $
 */

#include "config.h"

#if HAVE_HWASH || HAVE_AFASH

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

#define ASH_ALEN		64

static unsigned char hamming[16] =
{
    0x15, 0x02, 0x49, 0x5e, 0x64, 0x73, 0x38, 0x2f,
    0xd0, 0xc7, 0x8c, 0x9b, 0xa1, 0xb6, 0xfd, 0xea
};

/* Display an Ash address in readable format. */
static char *
pr_ash(unsigned char *ptr)
{
    static char buff[128];
    char *p = buff;
    unsigned int i = 0;

    p[0] = '[';
    p++;
    while (ptr[i] != 0xc9 && ptr[i] != 0xff && (i < ASH_ALEN))
	sprintf(p++, "%1x", ptr[i++]);
    *(p++) = ']';
    *p = 0;

    return buff;
}

#if HAVE_HWASH

#ifndef ARPHRD_ASH
#warning "No definition of ARPHRD_ASH in <net/if_arp.h>, using private value 517"
#define ARPHRD_ASH 517
#endif

struct hwtype ash_hwtype;

static int 
in_ash(char *bufp, struct sockaddr *sap)
{
    unsigned char *ptr;
    unsigned int i = 0;

    sap->sa_family = ash_hwtype.type;
    ptr = sap->sa_data;

    while (bufp && i < ASH_ALEN) {
	char *next;
	int hop = strtol(bufp, &next, 16);
	ptr[i++] = hamming[hop];
	switch (*next) {
	case ':':
	    bufp = next + 1;
	    break;
	case 0:
	    bufp = NULL;
	    break;
	default:
	    fprintf(stderr, _("Malformed Ash address"));
	    memset(ptr, 0xc9, ASH_ALEN);
	    return -1;
	}
    }

    while (i < ASH_ALEN)
	ptr[i++] = 0xc9;

    return 0;
}

struct hwtype ash_hwtype =
{
    "ash", NULL, ARPHRD_ASH, ASH_ALEN,
    pr_ash, in_ash, NULL,
    1
};

#endif	/* HAVE_HWASH */

#if HAVE_AFASH

/* Display an Ash socket address. */
static char *
pr_sash(struct sockaddr *sap, int numeric)
{
    static char buf[64];

    if (sap->sa_family != AF_ASH)
	return safe_strncpy(buf, "[NONE SET]", 64);
    return pr_ash(sap->sa_data);
}

struct aftype ash_aftype =
{
    "ash", NULL, AF_ASH, 0,
    pr_ash, pr_sash, NULL, NULL,
    NULL, NULL, NULL,
    -1,
    "/proc/sys/net/ash"
};

#endif	/* HAVE_AFASH */

#endif	/* HAVE_AFASH || HAVE_HWASH */
