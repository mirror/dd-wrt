/*
 * lib/net-features.h This file contains the definitions of all kernel
 *                      dependend features.
 *
 * Version:     features.h 0.03 (1996-03-22)
 *
 * Author:      Bernd Eckenfels <net-tools@lina.inka.de>
 *              Copyright 1996 Bernd Eckenfels, Germany
 *
 * Modifications:
 *960201 {0.01} Bernd Eckenfels:        creation
 *960202 {0.02} Bernd Eckenfels:        HW and AF added
 *960322 {0.03} Bernd Eckenfels:        moved into the NET-LIB
 *980630 {0.04} Arnaldo Carvalho de Melo: changed NLS for I18N
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */

/* 
 *    This needs to be included AFTER the KErnel Header Files
 *      one of the FEATURE_ should be defined to get the Feature Variable
 *      definition included
 */

#ifndef _NET_FEATURES_H
#define _NET_FEATURES_H

/* detect the present features */

#if defined (SIOCADDRTOLD) || defined (RTF_IRTT)	/* route */
#define HAVE_NEW_ADDRT 1
#endif

#ifdef RTF_IRTT			/* route */
#define HAVE_RTF_IRTT 1
#endif

#ifdef RTF_REJECT		/* route */
#define HAVE_RTF_REJECT 1
#endif

/* compose the feature information string */

#if defined (FEATURE_ARP) || defined (FEATURE_ROUTE) || defined (FEATURE_NETSTAT)
static char *Features =

/* ---------------------------------------------------- */
#ifdef FEATURE_ROUTE

#if HAVE_NEW_ADDRT
"+"
#else
"-"
#endif
"NEW_ADDRT "

#if HAVE_RTF_IRTT
"+"
#else
"-"
#endif
"RTF_IRTT "

#if HAVE_RTF_REJECT
"+"
#else
"-"
#endif
"RTF_REJECT "

#endif				/* FEATURE_ROUTE */
/* ---------------------------------------------------- */


/* ---------------------------------------------------- */
#ifdef FEATURE_NETSTAT

#if HAVE_NEW_ADDRT
"+"
#else
"-"
#endif
"NEW_ADDRT "

#if HAVE_RTF_IRTT
"+"
#else
"-"
#endif
"RTF_IRTT "

#if HAVE_RTF_REJECT
"+"
#else
"-"
#endif
"RTF_REJECT "

#if HAVE_FW_MASQUERADE
"+"
#else
"-"
#endif
"FW_MASQUERADE "

#endif				/* FEATURE_NETSTAT */
/* ---------------------------------------------------- */


#if I18N
"+I18N"
#else
"-I18N"
#endif				/* I18N */


"\nAF: "
#ifdef DFLT_AF
"(" DFLT_AF ")"
#endif

#if HAVE_AFUNIX
" +"
#else
" -"
#endif
"UNIX "
#if HAVE_AFINET
"+"
#else
"-"
#endif
"INET "
#if HAVE_AFINET6
"+"
#else
"-"
#endif
"INET6 "
#if HAVE_AFIPX
"+"
#else
"-"
#endif
"IPX "
#if HAVE_AFAX25
"+"
#else
"-"
#endif
"AX25 "
#if HAVE_AFNETROM
"+"
#else
"-"
#endif
"NETROM "
#if HAVE_AFX25
"+"
#else
"-"
#endif
"X25 "
#if HAVE_AFATALK
"+"
#else
"-"
#endif
"ATALK "
#if HAVE_AFECONET
"+"
#else
"-"
#endif
"ECONET "
#if HAVE_AFROSE
"+"
#else
"-"
#endif
"ROSE "

"\nHW: "

#ifdef DFLT_HW
"(" DFLT_HW ")"
#endif

#if HAVE_HWETHER
" +"
#else
" -"
#endif
"ETHER "
#if HAVE_HWARC
"+"
#else
"-"
#endif
"ARC "
#if HAVE_HWSLIP
"+"
#else
"-"
#endif
"SLIP "
#if HAVE_HWPPP
"+"
#else
"-"
#endif
"PPP "
#if HAVE_HWTUNNEL
"+"
#else
"-"
#endif
"TUNNEL "
#if HAVE_HWTR
"+"
#else
"-"
#endif
"TR "
#if HAVE_HWAX25
"+"
#else
"-"
#endif
"AX25 "

#if HAVE_HWNETROM
"+"
#else
"-"
#endif
"NETROM "

#if HAVE_HWX25
"+"
#else
"-"
#endif
"X25 "

#if HAVE_HWFR
"+"
#else
"-"
#endif
"FR "

#if HAVE_HWROSE
"+"
#else
"-"
#endif
"ROSE "

#if HAVE_HWASH
"+"
#else
"-"
#endif
"ASH "

#if HAVE_HWSIT
"+"
#else
"-"
#endif
"SIT "

#if HAVE_HWFDDI
"+"
#else
"-"
#endif
"FDDI "

#if HAVE_HWHIPPI
"+"
#else
"-"
#endif
"HIPPI "

#if HAVE_HWHDLCLAPB
"+"
#else
"-"
#endif
"HDLC/LAPB "
;


#endif				/* FEATURE_* */

#endif				/* _NET_FEATURES_H */
/* End of features.h */
