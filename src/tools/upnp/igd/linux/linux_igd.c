/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: linux_igd.c,v 1.9 2005/03/07 08:35:32 kanki Exp $
 */

#include <sys/ioctl.h>
#include <net/if.h>
//#include <linux/sockios.h>
#include <linux/socket.h>
#include <signal.h>
#include <signal.h>

#define __KERNEL__
#include <asm/types.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "../igd/igd.h"
#include "shutils.h"
#include "bcmnvram.h"
#include <shutils.h>


#define _PATH_PROCNET_DEV           "/proc/net/dev"

//static int dump_ecmd(struct ethtool_cmd *ep);

int osl_ifstats(char *ifname, if_stats_t *pstats )
{
    extern int get_dev_fields(char *, int , if_stats_t *);
    extern int procnetdev_version(char *);
    extern char *get_name(char *, char *);

    FILE *fh;
    char buf[512];
    int err;
    int procnetdev_vsn;  /* version information */

    memset(pstats, 0, sizeof(*pstats));
    
    fh = fopen(_PATH_PROCNET_DEV, "r");
    if (!fh) {
	fprintf(stderr, "Warning: cannot open %s (%s). Limited output.\n",
		_PATH_PROCNET_DEV, strerror(errno)); 
	return 0;
    }	

    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);

    procnetdev_vsn = procnetdev_version(buf);
    
    err = 0;
    while (fgets(buf, sizeof buf, fh)) {
	char *s;
	char name[50];
	
	s = get_name(name, buf);
	if (strcmp(name, ifname) == 0) {
	    get_dev_fields(s, procnetdev_vsn, pstats);
	    break;
	}
    }
    if (ferror(fh)) {
	perror(_PATH_PROCNET_DEV);
	err = -1;
    }
    fclose(fh);

    return err;
}


void osl_igd_disable(char *ifname)
{
    /* release the lease on the external interface. */
    char sigusr2[] = "-XX";
    
    sprintf(sigusr2, "-%d", SIGUSR2);
    eval("killall", sigusr2, "udhcpc");
}


void osl_igd_enable(char *ifname)
{
    /* renew the lease on the external interface. */
    char sigusr1[] = "-XX";
    
    sprintf(sigusr1, "-%d", SIGUSR1);
    eval("killall", sigusr1, "udhcpc");
}



/* Return OSL_LINK_UP if the link status given interface is UP, OSL_LINK_DOWN otherwise. */
osl_link_t osl_link_status(char *devname)
{
    struct ifreq ifr;
    int fd, err;
    uint if_up = OSL_LINK_DOWN;
    struct ethtool_cmd ecmd;

    if_up = OSL_LINK_UP;	// Our ethernet driver isn't support SIOCETHTOOL, so we must force to UP, by honor
    return if_up;

    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, devname);

    /* Open control socket. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err >= 0) {
	    switch (ecmd.speed) {
	    case SPEED_10:
	    case SPEED_100:
	    case SPEED_1000:
		if_up = OSL_LINK_UP;
		break;
	    };
	}

	/* close the control socket */
	close(fd);
    }

    return if_up;
}


uint osl_max_bitrates(char *devname, ulong *rx, ulong *tx)
{
    struct ethtool_cmd ecmd;
    struct ifreq ifr;
    int fd, err;
    long speed = 0;

    *rx = *tx = 100*1000000;	// Our ethernet driver isn't support SIOCETHTOOL, so we must force to 100Mb, by honor
    return TRUE;

    /* Setup our control structures. */
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, devname);

    /* Open control socket. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0) {
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err >= 0) {
	    u_int32_t mask = ecmd.supported;

	    // dump_ecmd(&ecmd);

	    if ((mask & (ADVERTISED_1000baseT_Half|ADVERTISED_1000baseT_Full))) {
		speed = (1000 * 1000000);
   	    } else if ((mask & (ADVERTISED_100baseT_Half|ADVERTISED_100baseT_Full))) {
		speed = (100 * 1000000);
	    } else if ((mask & (ADVERTISED_10baseT_Half|ADVERTISED_10baseT_Full))) {
		speed = (10 * 1000000);
	    } else {
		speed = 0;
	    }
	} else {
	    UPNP_ERROR(("ioctl(SIOCETHTOOL) failed in %s", __FUNCTION__));
	}

	/* close the control socket */
	close(fd);
    } else {
	UPNP_ERROR(("cannot open socket in %s", __FUNCTION__));
    }

    *rx = *tx = speed;

    return TRUE;
}


/* not used
static void dump_supported(struct ethtool_cmd *ep)
{
	u_int32_t mask = ep->supported;
	int did1;

	fprintf(stdout, "	Supported ports: [ ");
	if (mask & SUPPORTED_TP)
		fprintf(stdout, "TP ");
	if (mask & SUPPORTED_AUI)
		fprintf(stdout, "AUI ");
#ifdef notdef
	if (mask & SUPPORTED_BNC)
		fprintf(stdout, "BNC ");
#endif
	if (mask & SUPPORTED_MII)
		fprintf(stdout, "MII ");
	if (mask & SUPPORTED_FIBRE)
		fprintf(stdout, "FIBRE ");
	fprintf(stdout, "]\n");

	fprintf(stdout, "	Supported link modes:   ");
	did1 = 0;
	if (mask & SUPPORTED_10baseT_Half) {
		did1++; fprintf(stdout, "10baseT/Half ");
	}
	if (mask & SUPPORTED_10baseT_Full) {
		did1++; fprintf(stdout, "10baseT/Full ");
	}
	if (did1 && (mask & (SUPPORTED_100baseT_Half|SUPPORTED_100baseT_Full))) {
		fprintf(stdout, "\n");
		fprintf(stdout, "	                        ");
	}
	if (mask & SUPPORTED_100baseT_Half) {
		did1++; fprintf(stdout, "100baseT/Half ");
	}
	if (mask & SUPPORTED_100baseT_Full) {
		did1++; fprintf(stdout, "100baseT/Full ");
	}
	if (did1 && (mask & (SUPPORTED_1000baseT_Half|SUPPORTED_1000baseT_Full))) {
		fprintf(stdout, "\n");
		fprintf(stdout, "	                        ");
	}
	if (mask & SUPPORTED_1000baseT_Half) {
		did1++; fprintf(stdout, "1000baseT/Half ");
	}
	if (mask & SUPPORTED_1000baseT_Full) {
		did1++; fprintf(stdout, "1000baseT/Full ");
	}
	fprintf(stdout, "\n");

	fprintf(stdout, "	Supports auto-negotiation: ");
	if (mask & SUPPORTED_Autoneg)
		fprintf(stdout, "Yes\n");
	else
		fprintf(stdout, "No\n");
}

static void dump_advertised(struct ethtool_cmd *ep)
{
	u_int32_t mask = ep->advertising;
	int did1;

	fprintf(stdout, "	Advertised link modes:  ");
	did1 = 0;
	if (mask & ADVERTISED_10baseT_Half) {
		did1++; fprintf(stdout, "10baseT/Half ");
	}
	if (mask & ADVERTISED_10baseT_Full) {
		did1++; fprintf(stdout, "10baseT/Full ");
	}
	if (did1 && (mask & (ADVERTISED_100baseT_Half|ADVERTISED_100baseT_Full))) {
		fprintf(stdout, "\n");
		fprintf(stdout, "	                        ");
	}
	if (mask & ADVERTISED_100baseT_Half) {
		did1++; fprintf(stdout, "100baseT/Half ");
	}
	if (mask & ADVERTISED_100baseT_Full) {
		did1++; fprintf(stdout, "100baseT/Full ");
	}
	if (did1 && (mask & (ADVERTISED_1000baseT_Half|ADVERTISED_1000baseT_Full))) {
		fprintf(stdout, "\n");
		fprintf(stdout, "	                        ");
	}
	if (mask & ADVERTISED_1000baseT_Half) {
		did1++; fprintf(stdout, "1000baseT/Half ");
	}
	if (mask & ADVERTISED_1000baseT_Full) {
		did1++; fprintf(stdout, "1000baseT/Full ");
	}
	if (did1 == 0)
		 fprintf(stdout, "Not reported");
	fprintf(stdout, "\n");

	fprintf(stdout, "	Advertised auto-negotiation: ");
	if (mask & ADVERTISED_Autoneg)
		fprintf(stdout, "Yes\n");
	else
		fprintf(stdout, "No\n");
}

static int dump_ecmd(struct ethtool_cmd *ep)
{
	dump_supported(ep);
	dump_advertised(ep);

	fprintf(stdout, "	Speed: ");
	switch (ep->speed) {
	case SPEED_10:
		fprintf(stdout, "10Mb/s\n");
		break;
	case SPEED_100:
		fprintf(stdout, "100Mb/s\n");
		break;
	case SPEED_1000:
		fprintf(stdout, "1000Mb/s\n");
		break;
	default:
		fprintf(stdout, "Unknown! (%i)\n", ep->speed);
		break;
	};

	fprintf(stdout, "	Duplex: ");
	switch (ep->duplex) {
	case DUPLEX_HALF:
		fprintf(stdout, "Half\n");
		break;
	case DUPLEX_FULL:
		fprintf(stdout, "Full\n");
		break;
	default:
		fprintf(stdout, "Unknown! (%i)\n", ep->duplex);
		break;
	};

	fprintf(stdout, "	Port: ");
	switch (ep->port) {
	case PORT_TP:
		fprintf(stdout, "Twisted Pair\n");
		break;
	case PORT_AUI:
		fprintf(stdout, "AUI\n");
		break;
	case PORT_BNC:
		fprintf(stdout, "BNC\n");
		break;
	case PORT_MII:
		fprintf(stdout, "MII\n");
		break;
	case PORT_FIBRE:
		fprintf(stdout, "FIBRE\n");
		break;
	default:
		fprintf(stdout, "Unknown! (%i)\n", ep->port);
		break;
	};

	fprintf(stdout, "	PHYAD: %d\n", ep->phy_address);
	fprintf(stdout, "	Transceiver: ");
	switch (ep->transceiver) {
	case XCVR_INTERNAL:
		fprintf(stdout, "internal\n");
		break;
	case XCVR_EXTERNAL:
		fprintf(stdout, "externel\n");
		break;
	default:
		fprintf(stdout, "Unknown!\n");
		break;
	};

	fprintf(stdout, "	Auto-negotiation: %s\n",
		(ep->autoneg == AUTONEG_DISABLE) ?
		"off" : "on");
	return 0;
}
*/

void osl_sys_restart()
{
    kill(1, SIGHUP);
}

void osl_sys_reboot()
{
    kill(1, SIGTERM);
}

bool osl_wan_isup(char *devname)
{
    struct in_addr inaddr = {0};
    bool status = FALSE;

    if (strcasecmp(nvram_safe_get("wan_proto"), "disabled") != 0) {
	if (osl_ifaddr(nvram_safe_get("wan_iface"), &inaddr)) {
	    if (inaddr.s_addr != 0) {
		status = TRUE;
	    }
	} 
    }
    return status;
}

bool osl_lan_isup(char *devname)
{
    UPNP_ERROR(("%s is not implemented yet.\n", __FUNCTION__));
    return TRUE;
}

bool osl_set_macaddr(char *devname, char spoofed[]) 
{
    UPNP_ERROR(("%s is not implemented yet.\n", __FUNCTION__));
    return TRUE;
}
