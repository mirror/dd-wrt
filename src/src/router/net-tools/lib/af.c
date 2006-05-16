/*
 * lib/af.c   This file contains the top-level part of the protocol
 *              support functions module for the NET-2 base distribution.
 *
 * Version:     $Id: af.c,v 1.13 2000/05/20 13:38:10 pb Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "config.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

int flag_unx;
int flag_ipx;
int flag_ax25;
int flag_ddp;
int flag_netrom;
int flag_inet;
int flag_inet6;
int flag_econet;
int flag_x25 = 0;
int flag_ash;


struct aftrans_t {
    char *alias;
    char *name;
    int *flag;
} aftrans[] = {

    {
	"ax25", "ax25", &flag_ax25
    },
    {
	"ip", "inet", &flag_inet
    },
    {
	"ip6", "inet6", &flag_inet6
    },
    {
	"ipx", "ipx", &flag_ipx
    },
    {
	"appletalk", "ddp", &flag_ddp
    },
    {
	"netrom", "netrom", &flag_netrom
    },
    {
	"inet", "inet", &flag_inet
    },
    {
	"inet6", "inet6", &flag_inet6
    },
    {
	"ddp", "ddp", &flag_ddp
    },
    {
	"unix", "unix", &flag_unx
    },
    {
	"tcpip", "inet", &flag_inet
    },
    {
	"econet", "ec", &flag_econet
    },
    {
	"x25", "x25", &flag_x25
    },
    {
        "ash", "ash", &flag_ash
    },
    {
	0, 0, 0
    }
};

char afname[256] = "";

extern struct aftype unspec_aftype;
extern struct aftype unix_aftype;
extern struct aftype inet_aftype;
extern struct aftype inet6_aftype;
extern struct aftype ax25_aftype;
extern struct aftype netrom_aftype;
extern struct aftype ipx_aftype;
extern struct aftype ddp_aftype;
extern struct aftype ec_aftype;
extern struct aftype x25_aftype;
extern struct aftype rose_aftype;
extern struct aftype ash_aftype;

static short sVafinit = 0;

struct aftype *aftypes[] =
{
#if HAVE_AFUNIX
    &unix_aftype,
#endif
#if HAVE_AFINET
    &inet_aftype,
#endif
#if HAVE_AFINET6
    &inet6_aftype,
#endif
#if HAVE_AFAX25
    &ax25_aftype,
#endif
#if HAVE_AFNETROM
    &netrom_aftype,
#endif
#if HAVE_AFROSE
    &rose_aftype,
#endif
#if HAVE_AFIPX
    &ipx_aftype,
#endif
#if HAVE_AFATALK
    &ddp_aftype,
#endif
#if HAVE_AFECONET
    &ec_aftype,
#endif
#if HAVE_AFASH
    &ash_aftype,
#endif
#if HAVE_AFX25
    &x25_aftype,
#endif
    &unspec_aftype,
    NULL
};

void afinit()
{
    unspec_aftype.title = _("UNSPEC");
#if HAVE_AFUNIX
    unix_aftype.title = _("UNIX Domain");
#endif
#if HAVE_AFINET
    inet_aftype.title = _("DARPA Internet");
#endif
#if HAVE_AFINET6
    inet6_aftype.title = _("IPv6");
#endif
#if HAVE_AFAX25
    ax25_aftype.title = _("AMPR AX.25");
#endif
#if HAVE_AFNETROM
    netrom_aftype.title = _("AMPR NET/ROM");
#endif
#if HAVE_AFIPX
    ipx_aftype.title = _("Novell IPX");
#endif
#if HAVE_AFATALK
    ddp_aftype.title = _("Appletalk DDP");
#endif
#if HAVE_AFECONET
    ec_aftype.title = _("Econet");
#endif
#if HAVE_AFX25
    x25_aftype.title = _("CCITT X.25");
#endif
#if HAVE_AFROSE
    rose_aftype.title = _("AMPR ROSE");
#endif
#if HAVE_AFASH
    ash_aftype.title = _("Ash");
#endif
    sVafinit = 1;
}

/* set the default AF list from the program name or a constant value    */
void aftrans_def(char *tool, char *argv0, char *dflt)
{
    char *tmp;
    char *buf;

    strcpy(afname, dflt);

    if (!(tmp = strrchr(argv0, '/')))
	tmp = argv0;		/* no slash?! */
    else
	tmp++;

    if (!(buf = strdup(tmp)))
	return;

    if (strlen(tool) >= strlen(tmp)) {
	free(buf);
	return;
    }
    tmp = buf + (strlen(tmp) - strlen(tool));

    if (strcmp(tmp, tool) != 0) {
	free(buf);
	return;
    }
    *tmp = '\0';
    if ((tmp = strchr(buf, '_')))
	*tmp = '\0';

    afname[0] = '\0';
    if (aftrans_opt(buf))
	strcpy(afname, buf);

    free(buf);
}


/* Check our protocol family table for this family. */
struct aftype *get_aftype(const char *name)
{
    struct aftype **afp;

    if (!sVafinit)
	afinit();

    afp = aftypes;
    while (*afp != NULL) {
	if (!strcmp((*afp)->name, name))
	    return (*afp);
	afp++;
    }
    if (index(name, ','))
	fprintf(stderr, _("Please don't supply more than one address family.\n"));
    return (NULL);
}


/* Check our protocol family table for this family. */
struct aftype *get_afntype(int af)
{
    struct aftype **afp;

    if (!sVafinit)
	afinit();

    afp = aftypes;
    while (*afp != NULL) {
	if ((*afp)->af == af)
	    return (*afp);
	afp++;
    }
    return (NULL);
}

/* Check our protocol family table for this family and return its socket */
int get_socket_for_af(int af)
{
    struct aftype **afp;

    if (!sVafinit)
	afinit();

    afp = aftypes;
    while (*afp != NULL) {
	if ((*afp)->af == af)
	    return (*afp)->fd;
	afp++;
    }
    return -1;
}

int aftrans_opt(const char *arg)
{
    struct aftrans_t *paft;
    char *tmp1, *tmp2;
    char buf[256];

    safe_strncpy(buf, arg, sizeof(buf));

    tmp1 = buf;

    while (tmp1) {

	tmp2 = index(tmp1, ',');

	if (tmp2)
	    *(tmp2++) = '\0';

	paft = aftrans;
	for (paft = aftrans; paft->alias; paft++) {
	    if (strcmp(tmp1, paft->alias))
		continue;
	    if (strlen(paft->name) + strlen(afname) + 1 >= sizeof(afname)) {
		fprintf(stderr, _("Too much address family arguments.\n"));
		return (0);
	    }
	    if (paft->flag)
		(*paft->flag)++;
	    if (afname[0])
		strcat(afname, ",");
	    strcat(afname, paft->name);
	    break;
	}
	if (!paft->alias) {
	    fprintf(stderr, _("Unknown address family `%s'.\n"), tmp1);
	    return (1);
	}
	tmp1 = tmp2;
    }

    return (0);
}

/* type: 0=all, 1=getroute */
void print_aflist(int type) {
    int count = 0;
    char * txt;
    struct aftype **afp;

    if (!sVafinit)
	afinit();

    afp = aftypes;
    while (*afp != NULL) {
	if ((type == 1 && ((*afp)->rprint == NULL)) || ((*afp)->af == 0)) {
		afp++; continue;
	}
	if ((count % 3) == 0) fprintf(stderr,count?"\n    ":"    "); 
        txt = (*afp)->name; if (!txt) txt = "..";
	fprintf(stderr,"%s (%s) ",txt,(*afp)->title);
	count++;
	afp++;
    }
    fprintf(stderr,"\n");
}
