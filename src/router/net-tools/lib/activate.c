/*
 * lib/activate.c     This file contains a small interface function to
 *                      use the HW specific activate routines for line
 *                      disciplines
 *
 * NET-LIB      A collection of functions used from the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system. (net-tools, net-drivers)
 *
 * Version:     $Id: activate.c,v 1.3 1998/11/15 20:08:55 freitag Exp $
 *
 * Author:      Bernd 'eckes' Eckenfels <net-tools@lina.inka.de>
 *              Copyright 1996 Bernd Eckenfels, Germany
 *
 * Modifications:
 *
 *960322 {0.01} Bernd Eckenfels:        creation
 *980411 {0.01i} Arnaldo Carvalho:      i18n: now uses gettext
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

extern struct hwtype slip_hwtype;
extern struct hwtype cslip_hwtype;
extern struct hwtype slip6_hwtype;
extern struct hwtype cslip6_hwtype;
extern struct hwtype adaptive_hwtype;
extern struct hwtype ppp_hwtype;

extern int SLIP_activate(int fd);
extern int CSLIP_activate(int fd);
extern int SLIP6_activate(int fd);
extern int CSLIP6_activate(int fd);
extern int ADAPTIVE_activate(int fd);
extern int PPP_activate(int fd);

void activate_init(void)
{
#if HAVE_HWSLIP
    slip_hwtype.activate = SLIP_activate;
    cslip_hwtype.activate = CSLIP_activate;
    slip6_hwtype.activate = SLIP6_activate;
    cslip6_hwtype.activate = CSLIP6_activate;
    adaptive_hwtype.activate = ADAPTIVE_activate;
#endif
#if HAVE_HWPPP
    ppp_hwtype.activate = PPP_activate;
#endif
}

int activate_ld(const char *hwname, int fd)
{
    struct hwtype *hw;

    hw = get_hwtype(hwname);

    if (!hw) {
	fprintf(stderr, _("Hardware type `%s' not supported.\n"), hwname);
	return (E_NOSUPP);
    }
    if (!hw->activate) {
	fprintf(stderr, _("Cannot change line discipline to `%s'.\n"), hw->name);
	return (E_OPTERR);
    }
    return (hw->activate(fd));
}
