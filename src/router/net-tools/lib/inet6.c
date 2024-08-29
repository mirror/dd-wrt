/*
 * lib/inet6.c        This file contains an implementation of the "INET6"
 *              support functions for the net-tools.
 *              (most of it copied from lib/inet.c 1.26).
 *
 * Version:     $Id: inet6.c,v 1.13 2010-07-05 22:52:00 ecki Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 * Modified:
 *960808 {0.01} Frank Strauss :         adapted for IPv6 support
 *980701 {0.02} Arnaldo C. Melo:        GNU gettext instead of catgets
 *990824        Bernd Eckenfels:	clear members for selecting v6 address
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

#if HAVE_AFINET6
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

extern int h_errno;		/* some netdb.h versions don't export this */

static char *fix_v4_address(char *buf, const struct in6_addr *in6)
{
	if (IN6_IS_ADDR_V4MAPPED(in6->s6_addr)) {
			char *s =strchr(buf, '.');
			if (s) {
				while (s > buf && *s != ':')
					--s;
				if (*s == ':') ++s;
				else s = NULL;
			}
			if (s) return s;
	}
	return buf;
}

static int INET6_resolve(char *name, struct sockaddr_storage *sasp)
{
    struct addrinfo req, *ai;
    int s;

    memset (&req, '\0', sizeof req);
    req.ai_family = AF_INET6;
    if ((s = getaddrinfo(name, NULL, &req, &ai))) {
	fprintf(stderr, "getaddrinfo: %s: %d\n", name, s);
	return -1;
    }
    memcpy(sasp, ai->ai_addr, sizeof(struct sockaddr_in6));

    freeaddrinfo(ai);

    return (0);
}

#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a) \
        (((__u32 *) (a))[0] == 0 && ((__u32 *) (a))[1] == 0 && \
         ((__u32 *) (a))[2] == 0 && ((__u32 *) (a))[3] == 0)
#endif


static int INET6_rresolve(char *name, size_t namelen,
			  const struct sockaddr_storage *sasp, int numeric)
{
    const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)sasp;
    /* Grmpf. -FvK */
    if (sin6->sin6_family != AF_INET6) {
#ifdef DEBUG
	fprintf(stderr, _("rresolve: unsupport address family %d !\n"),
		sin6->sin6_family);
#endif
	errno = EAFNOSUPPORT;
	return (-1);
    }
    if (numeric & 0x7FFF) {
	inet_ntop( AF_INET6, &sin6->sin6_addr, name, namelen);
	return (0);
    }
    if (IN6_IS_ADDR_UNSPECIFIED(&sin6->sin6_addr)) {
        if (numeric & 0x8000)
	    safe_strncpy(name, "default", namelen);
	else
	    safe_strncpy(name, "[::]", namelen);
	return (0);
    }

    if (getnameinfo((const struct sockaddr *)sasp, sizeof(struct sockaddr_in6),
		    name, namelen , NULL, 0, 0)) {
	inet_ntop( AF_INET6, &sin6->sin6_addr, name, namelen);
    }
    return (0);
}


static void INET6_reserror(const char *text)
{
    herror(text);
}



/* Display an Internet socket address. */
static const char *INET6_print(const char *ptr)
{
    static char name[INET6_ADDRSTRLEN + 1];
    socklen_t len = sizeof(name) - 1;
    name[len] = '\0';
    inet_ntop(AF_INET6, ptr, name, len);
    return fix_v4_address(name, (struct in6_addr *)ptr);
}


/* Display an Internet socket address. */
/* dirty! struct sockaddr usually doesn't suffer for inet6 addresses, fst. */
static const char *INET6_sprint(const struct sockaddr_storage *sasp, int numeric)
{
    static char buff[128];

    if (sasp->ss_family == 0xFFFF || sasp->ss_family == 0)
	return safe_strncpy(buff, _("[NONE SET]"), sizeof(buff));
    if (INET6_rresolve(buff, sizeof(buff), sasp, numeric) != 0)
	return safe_strncpy(buff, _("[UNKNOWN]"), sizeof(buff));
    return (fix_v4_address(buff, &((struct sockaddr_in6 *)sasp)->sin6_addr));
}


static int INET6_getsock(char *bufp, struct sockaddr_storage *sasp)
{
    struct sockaddr_in6 *sin6;
	char *p;

    sin6 = (struct sockaddr_in6 *) sasp;
    sin6->sin6_family = AF_INET6;
    sin6->sin6_port = 0;
    sin6->sin6_scope_id = 0;
    sin6->sin6_flowinfo = 0;

    if (inet_pton(AF_INET6, bufp, sin6->sin6_addr.s6_addr) <= 0)
	return (-1);
    p = fix_v4_address(bufp, &sin6->sin6_addr);
    if (p != bufp)
        memmove(bufp, p, strlen(p) + 1);
    return 16;			/* ?;) */
}

static int INET6_input(int type, char *bufp, struct sockaddr_storage *sasp)
{
    switch (type) {
    case 1:
	return INET6_getsock(bufp, sasp);
    default:
	return INET6_resolve(bufp, sasp);
    }
}


struct aftype inet6_aftype =
{
    "inet6", NULL, /*"IPv6", */ AF_INET6, sizeof(struct in6_addr),
    INET6_print, INET6_sprint, INET6_input, INET6_reserror,
    INET6_rprint, INET6_rinput, NULL,

    -1,
    "/proc/net/if_inet6"
};


#endif				/* HAVE_AFINET6 */
