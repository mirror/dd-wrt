/*
 * lib/hw.c   This file contains the top-level part of the hardware
 *              support functions module.
 *
 * Version:     $Id: hw.c,v 1.17 2000/05/20 13:38:10 pb Exp $
 *
 * Maintainer:  Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 *980701 {1.21}  Arnaldo C. Melo       GNU gettext instead of catgets
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
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

extern struct hwtype unspec_hwtype;
extern struct hwtype loop_hwtype;

extern struct hwtype slip_hwtype;
extern struct hwtype cslip_hwtype;
extern struct hwtype slip6_hwtype;
extern struct hwtype cslip6_hwtype;
extern struct hwtype adaptive_hwtype;
extern struct hwtype strip_hwtype;

extern struct hwtype ether_hwtype;
extern struct hwtype fddi_hwtype;
extern struct hwtype hippi_hwtype;
extern struct hwtype tr_hwtype;
#ifdef ARPHRD_IEEE802_TR
extern struct hwtype tr_hwtype1;
#endif

extern struct hwtype ax25_hwtype;
extern struct hwtype rose_hwtype;
extern struct hwtype netrom_hwtype;
extern struct hwtype x25_hwtype;
extern struct hwtype tunnel_hwtype;

extern struct hwtype ash_hwtype;

extern struct hwtype ppp_hwtype;

extern struct hwtype arcnet_hwtype;

extern struct hwtype dlci_hwtype;
extern struct hwtype frad_hwtype;

extern struct hwtype hdlc_hwtype;
extern struct hwtype lapb_hwtype;

extern struct hwtype sit_hwtype;

extern struct hwtype irda_hwtype;

extern struct hwtype ec_hwtype;

static struct hwtype *hwtypes[] =
{

    &loop_hwtype,

#if HAVE_HWSLIP
    &slip_hwtype,
    &cslip_hwtype,
    &slip6_hwtype,
    &cslip6_hwtype,
    &adaptive_hwtype,
#endif
#if HAVE_HWSTRIP
    &strip_hwtype,
#endif
#if HAVE_HWASH
    &ash_hwtype,
#endif
#if HAVE_HWETHER
    &ether_hwtype,
#endif
#if HAVE_HWTR
    &tr_hwtype,
#ifdef ARPHRD_IEEE802_TR
    &tr_hwtype1, 
#endif
#endif
#if HAVE_HWAX25
    &ax25_hwtype,
#endif
#if HAVE_HWNETROM
    &netrom_hwtype,
#endif
#if HAVE_HWROSE
    &rose_hwtype,
#endif
#if HAVE_HWTUNNEL
    &tunnel_hwtype,
#endif
#if HAVE_HWPPP
    &ppp_hwtype,
#endif
#if HAVE_HWHDLCLAPB
    &hdlc_hwtype,
    &lapb_hwtype,
#endif
#if HAVE_HWARC
    &arcnet_hwtype,
#endif
#if HAVE_HWFR
    &dlci_hwtype,
    &frad_hwtype,
#endif
#if HAVE_HWSIT
    &sit_hwtype,
#endif
#if HAVE_HWFDDI
    &fddi_hwtype,
#endif
#if HAVE_HWHIPPI
    &hippi_hwtype,
#endif
#if HAVE_HWIRDA
    &irda_hwtype,
#endif
#if HAVE_HWEC
    &ec_hwtype,
#endif
#if HAVE_HWX25
    &x25_hwtype,
#endif
    &unspec_hwtype,
    NULL
};

static short sVhwinit = 0;

void hwinit()
{
    loop_hwtype.title = _("Local Loopback");
    unspec_hwtype.title = _("UNSPEC");
#if HAVE_HWSLIP
    slip_hwtype.title = _("Serial Line IP");
    cslip_hwtype.title = _("VJ Serial Line IP");
    slip6_hwtype.title = _("6-bit Serial Line IP");
    cslip6_hwtype.title = _("VJ 6-bit Serial Line IP");
    adaptive_hwtype.title = _("Adaptive Serial Line IP");
#endif
#if HAVE_HWETHER
    ether_hwtype.title = _("Ethernet");
#endif
#if HAVE_HWASH
    ash_hwtype.title = _("Ash");
#endif
#if HAVE_HWFDDI
    fddi_hwtype.title = _("Fiber Distributed Data Interface");
#endif
#if HAVE_HWHIPPI
    hippi_hwtype.title = _("HIPPI");
#endif
#if HAVE_HWAX25
    ax25_hwtype.title = _("AMPR AX.25");
#endif
#if HAVE_HWROSE
    rose_hwtype.title = _("AMPR ROSE");
#endif
#if HAVE_HWNETROM
    netrom_hwtype.title = _("AMPR NET/ROM");
#endif
#if HAVE_HWX25
    x25_hwtype.title = _("generic X.25");
#endif
#if HAVE_HWTUNNEL
    tunnel_hwtype.title = _("IPIP Tunnel");
#endif
#if HAVE_HWPPP
    ppp_hwtype.title = _("Point-to-Point Protocol");
#endif
#if HAVE_HWHDLCLAPB
    hdlc_hwtype.title = _("(Cisco)-HDLC");
    lapb_hwtype.title = _("LAPB");
#endif
#if HAVE_HWARC
    arcnet_hwtype.title = _("ARCnet");
#endif
#if HAVE_HWFR
    dlci_hwtype.title = _("Frame Relay DLCI");
    frad_hwtype.title = _("Frame Relay Access Device");
#endif
#if HAVE_HWSIT
    sit_hwtype.title = _("IPv6-in-IPv4");
#endif
#if HAVE_HWIRDA
    irda_hwtype.title = _("IrLAP");
#endif
#if HAVE_HWTR
    tr_hwtype.title = _("16/4 Mbps Token Ring");
#ifdef ARPHRD_IEEE802_TR
    tr_hwtype1.title = _("16/4 Mbps Token Ring (New)") ; 
#endif
#endif
#if HAVE_HWEC
    ec_hwtype.title = _("Econet");
#endif
    sVhwinit = 1;
}

/* Check our hardware type table for this type. */
struct hwtype *get_hwtype(const char *name)
{
    struct hwtype **hwp;

    if (!sVhwinit)
	hwinit();

    hwp = hwtypes;
    while (*hwp != NULL) {
	if (!strcmp((*hwp)->name, name))
	    return (*hwp);
	hwp++;
    }
    return (NULL);
}


/* Check our hardware type table for this type. */
struct hwtype *get_hwntype(int type)
{
    struct hwtype **hwp;

    if (!sVhwinit)
	hwinit();

    hwp = hwtypes;
    while (*hwp != NULL) {
	if ((*hwp)->type == type)
	    return (*hwp);
	hwp++;
    }
    return (NULL);
}

/* type: 0=all, 1=ARPable */
void print_hwlist(int type) {
    int count = 0;
    char * txt;
    struct hwtype **hwp;

    if (!sVhwinit)
	hwinit();

    hwp = hwtypes;
    while (*hwp != NULL) {
	if (((type == 1) && ((*hwp)->alen == 0)) || ((*hwp)->type == -1)) {
		hwp++; continue;
	}
	if ((count % 3) == 0) fprintf(stderr,count?"\n    ":"    "); 
        txt = (*hwp)->name; if (!txt) txt = "..";
	fprintf(stderr,"%s (%s) ",txt,(*hwp)->title);
	count++;
	hwp++;
    }
    fprintf(stderr,"\n");
}

/* return 1 if address is all zeros */
int hw_null_address(struct hwtype *hw, void *ap)
{
    unsigned int i;
    unsigned char *address = (unsigned char *)ap;
    for (i = 0; i < hw->alen; i++)
	if (address[i])
	    return 0;
    return 1;
}
