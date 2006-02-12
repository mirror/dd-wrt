/*
 *    Tunnel.c, Alan Cox 1995.
 *
 */

#include "config.h"

#if HAVE_HWTUNNEL
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"

extern struct hwtype ether_hwtype;

static char *pr_tunnel(unsigned char *ptr)
{
    return ("");
}


static int in_tunnel(char *bufp, struct sockaddr *sap)
{
    return (-1);
}


struct hwtype tunnel_hwtype =
{
    "tunnel", NULL, /*"IPIP Tunnel", */ ARPHRD_TUNNEL, 0,
    pr_tunnel, in_tunnel, NULL, 0
};


#endif				/* HAVE_HWTUNNEL */
