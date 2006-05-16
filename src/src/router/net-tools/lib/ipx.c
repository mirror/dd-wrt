/*
 *            IPX protocol output functions.
 *              [Not yet input]
 *
 *                      Alan Cox  <Alan.Cox@linux.org>
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 * Modifications:
 * 1998-07-01 - Arnaldo Carvalho de Melo - GNU gettext instead of catgets,
 *                                         snprintf instead of sprintf
 */
#include "config.h"

#if HAVE_AFIPX
#include <asm/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

#if (IPX_NODE_LEN != 6)
#error "IPX_NODE_LEN != 6"
#endif

/* Display a ipx domain address. */
static char *IPX_print(unsigned char *ptr)
{
    static char buff[64];
    struct sockaddr_ipx *sipx = (struct sockaddr_ipx *) (ptr - 2);
    int t;


    for (t = IPX_NODE_LEN; t; t--)
	if (sipx->sipx_node[t - 1])
	    break;

    if (t && ntohl(sipx->sipx_network))
	snprintf(buff, sizeof(buff), "%08lX:%02X%02X%02X%02X%02X%02X",
		 (long int) ntohl(sipx->sipx_network),
		 (int) sipx->sipx_node[0], (int) sipx->sipx_node[1],
		 (int) sipx->sipx_node[2], (int) sipx->sipx_node[3],
		 (int) sipx->sipx_node[4], (int) sipx->sipx_node[5]);
    else if (!t && ntohl(sipx->sipx_network))
	snprintf(buff, sizeof(buff), "%08lX", (long int) ntohl(sipx->sipx_network));
    else if (t && !ntohl(sipx->sipx_network))
	snprintf(buff, sizeof(buff), "%02X%02X%02X%02X%02X%02X",
		 (int) sipx->sipx_node[0], (int) sipx->sipx_node[1],
		 (int) sipx->sipx_node[2], (int) sipx->sipx_node[3],
		 (int) sipx->sipx_node[4], (int) sipx->sipx_node[5]);
    else
	buff[0] = '\0';
    return (buff);
}


/* Display a ipx domain address. */
static char *IPX_sprint(struct sockaddr *sap, int numeric)
{
    static char buf[64];

    if (sap->sa_family != AF_IPX)
	return safe_strncpy(buf, _("[NONE SET]"), sizeof(buf));
    return (IPX_print(sap->sa_data));
}


static int IPX_getsock(char *bufp, struct sockaddr *sap)
{
    char *sp = bufp, *bp;
    unsigned int i;
    unsigned char val;
    struct sockaddr_ipx *sipx = (struct sockaddr_ipx *) sap;

    sipx->sipx_port = 0;

    val = 0;
    bp = (char *) sipx->sipx_node;
    for (i = 0; i < sizeof(sipx->sipx_node); i++) {
	*sp = toupper(*sp);

	if ((*sp >= 'A') && (*sp <= 'F'))
	    bp[i] |= (int) (*sp - 'A') + 10;
	else if ((*sp >= '0') && (*sp <= '9'))
	    bp[i] |= (int) (*sp - '0');
	else
	    return (-1);

	bp[i] <<= 4;
	sp++;
	*sp = toupper(*sp);

	if ((*sp >= 'A') && (*sp <= 'F'))
	    bp[i] |= (int) (*sp - 'A') + 10;
	else if ((*sp >= '0') && (*sp <= '9'))
	    bp[i] |= (int) (*sp - '0');
	else
	    return (-1);

	sp++;
    }
    if ((memcmp(sipx->sipx_node, "\0\0\0\0\0\0\0\0", IPX_NODE_LEN) == 0) ||
	(memcmp(sipx->sipx_node, "\377\377\377\377\377\377", IPX_NODE_LEN) == 0))
	return (-1);

    return (0);
}

/* XXX define type which makes verbose format checks AF_input */

static int IPX_input(int type, char *bufp, struct sockaddr *sap)
{
    struct sockaddr_ipx *sai = (struct sockaddr_ipx *) sap;
    unsigned long netnum;
    char *ep;
    int nbo;

    sai->sipx_family = AF_IPX;
    sai->sipx_network = htonl(0);
    sai->sipx_node[0] = sai->sipx_node[1] = sai->sipx_node[2] =
	sai->sipx_node[3] = sai->sipx_node[4] = sai->sipx_node[5] = '\0';
    sai->sipx_port = 0;

    if (type & 4)
	nbo = 1;
    else
	nbo = 0;

    type &= 3;
    if (type <= 1) {
	netnum = strtoul(bufp, &ep, 16);
	if ((netnum == 0xffffffffL) || (netnum == 0L))
	    return (-1);
	if (nbo)
	    sai->sipx_network = netnum;
	else
	    sai->sipx_network = htonl(netnum);
    }
    if (type == 1) {
	if (*ep != '\0')
	    return (-2);
	return (0);
    }
    if (type == 0) {
	if (*ep != ':')
	    return (-3);
	bufp = ep + 1;
    }
    return (IPX_getsock(bufp, sap));
}


struct aftype ipx_aftype =
{
    "ipx", NULL, /*"IPX", */ AF_IPX, 0,
    IPX_print, IPX_sprint, IPX_input, NULL,
    NULL /*IPX_rprint */ , NULL, NULL,
    -1,
    "/proc/net/ipx"
};

#endif
