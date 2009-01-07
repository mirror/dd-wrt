/*
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: linux_net.c,v 1.1.1.12 2007/05/31 08:00:41 michael Exp $
 */

#include <sys/ioctl.h>
#include <net/if.h>
//#include <linux/sockios.h>
#include <linux/socket.h>

#include "upnp_osl.h"
#include "upnp.h"
#include "../igd/igd.h"

char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

int get_dev_fields(char *bp, int versioninfo, if_stats_t *pstats )
{
    switch (versioninfo) {
    case 3:
	sscanf(bp,
	"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
	       &pstats->rx_bytes,
	       &pstats->rx_packets,
	       &pstats->rx_errors,
	       &pstats->rx_dropped,
	       &pstats->rx_fifo_errors,
	       &pstats->rx_frame_errors,
	       &pstats->rx_compressed,
	       &pstats->rx_multicast,

	       &pstats->tx_bytes,
	       &pstats->tx_packets,
	       &pstats->tx_errors,
	       &pstats->tx_dropped,
	       &pstats->tx_fifo_errors,
	       &pstats->collisions,
	       &pstats->tx_carrier_errors,
	       &pstats->tx_compressed);
	break;
    case 2:
	sscanf(bp, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
	       &pstats->rx_bytes,
	       &pstats->rx_packets,
	       &pstats->rx_errors,
	       &pstats->rx_dropped,
	       &pstats->rx_fifo_errors,
	       &pstats->rx_frame_errors,

	       &pstats->tx_bytes,
	       &pstats->tx_packets,
	       &pstats->tx_errors,
	       &pstats->tx_dropped,
	       &pstats->tx_fifo_errors,
	       &pstats->collisions,
	       &pstats->tx_carrier_errors);
	pstats->rx_multicast = 0;
	break;
    case 1:
	sscanf(bp, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
	       &pstats->rx_packets,
	       &pstats->rx_errors,
	       &pstats->rx_dropped,
	       &pstats->rx_fifo_errors,
	       &pstats->rx_frame_errors,

	       &pstats->tx_packets,
	       &pstats->tx_errors,
	       &pstats->tx_dropped,
	       &pstats->tx_fifo_errors,
	       &pstats->collisions,
	       &pstats->tx_carrier_errors);
	pstats->rx_bytes = 0;
	pstats->tx_bytes = 0;
	pstats->rx_multicast = 0;
	break;
    }
    return 0;
}

int procnetdev_version(char *buf)
{
    if (strstr(buf, "compressed"))
	return 3;
    if (strstr(buf, "bytes"))
	return 2;
    return 1;
}
