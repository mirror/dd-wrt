/*
 * lib/getroute.c     This file contains a small interface function to
 *                      use the AF specific print routine for the routing
 *                      table.
 *
 * NET-LIB      A collection of functions used from the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system. (net-tools, net-drivers)
 *
 * Version:     $Id: getroute.c,v 1.6 2000/05/20 13:38:10 pb Exp $
 *
 * Author:      Bernd 'eckes' Eckenfels <net-tools@lina.inka.de>
 *              Copyright 1999 Bernd Eckenfels, Germany
 *
 * Modifications:
 *
 *951020 {0.10} Bernd Eckenfels:        creation
 *960202 {0.90} Bernd Eckenfels:        rewrite to use getaftype.
 *960204 {0.91} Bernd Eckenfels:        takes constant list of AFs
 *960206 {1.01} Bernd Eckenfels:        route_init will enable routing
 *                                      support in the AF handlers
 *960221 {1.02} Bernd Eckenfels:        renamed from route_info to getroute.c
 *960413 {1.03} Bernd Eckenfels:        new RTACTION support
 *980701 {1.04} Arnaldo C. Melo:        GNU gettext instead of catgets
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <stdio.h>
#include <string.h>
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "config.h"
#include "intl.h"
#include "util.h"

extern struct aftype unspec_aftype;
extern struct aftype unix_aftype;
extern struct aftype inet_aftype;
extern struct aftype inet6_aftype;
extern struct aftype ax25_aftype;
extern struct aftype netrom_aftype;
extern struct aftype ipx_aftype;
extern struct aftype ddp_aftype;
extern struct aftype x25_aftype;

void getroute_init(void)
{
#if HAVE_AFINET
    inet_aftype.rprint = INET_rprint;
#endif
#if HAVE_AFINET6
    inet6_aftype.rprint = INET6_rprint;
#endif
#if HAVE_AFNETROM
    netrom_aftype.rprint = NETROM_rprint;
#endif
#if HAVE_AFAX25
    ax25_aftype.rprint = AX25_rprint;
#endif
#if HAVE_AFIPX
    ipx_aftype.rprint = IPX_rprint;
#endif
#if HAVE_AFATALK
    ddp_aftype.rprint = DDP_rprint;
#endif
#if HAVE_AFX25
    x25_aftype.rprint = X25_rprint;
#endif
}

int route_info(const char *afname, int options)
{
    struct aftype *ap;
    char *tmp1, *tmp2;
    int found = E_NOTFOUND, rc;
    char buf[256];

    safe_strncpy(buf, afname, sizeof(buf));

    tmp1 = buf;

    while (tmp1) {

	ap = NULL;

	if ((tmp2 = index(tmp1, ',')))
	    *tmp2++ = '\0';

	if (!tmp1[0]) {
	    tmp1 = tmp2;
	    continue;
	}
	ap = get_aftype(tmp1);

	if (!ap) {
	    fprintf(stderr, _("Address family `%s' not supported.\n"), tmp1);
	    return (E_OPTERR);
	}
	tmp1 = tmp2;

	if (!ap->rprint) {
	    fprintf(stderr, _("No routing for address family `%s'.\n"), ap->name);
	    return (E_OPTERR);
	}
	found = 0;

	if ((rc = ap->rprint(options)))
	    return (rc);

    }
    return (found);
}
