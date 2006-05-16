/*

    mii-tool: monitor and control the MII for a network interface

    Usage:

	mii-tool [-VvRrw] [-A media,... | -F media] [interface ...]

    This program is based on Donald Becker's "mii-diag" program, which
    is more capable and verbose than this tool, but also somewhat
    harder to use.

    Copyright (C) 2000 David A. Hinds -- dhinds@pcmcia.sourceforge.org

    mii-diag is written/copyright 1997-2000 by Donald Becker
        <becker@scyld.com>

    This program is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation.

    Donald Becker may be reached as becker@scyld.com, or C/O
    Scyld Computing Corporation, 410 Severn Av., Suite 210,
    Annapolis, MD 21403

    References
	http://www.scyld.com/diag/mii-status.html
	http://www.scyld.com/expert/NWay.html
	http://www.national.com/pf/DP/DP83840.html
*/

static char version[] =
"mii-tool.c 1.9 2000/04/28 00:56:08 (David Hinds)\n";

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#ifndef __GLIBC__
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#endif
#include "mii.h"

#define MAX_ETH		8		/* Maximum # of interfaces */

/* Table of known MII's */
static struct {
    u_short	id1, id2;
    char	*name;
} mii_id[] = {
    { 0x0022, 0x5610, "AdHoc AH101LF" },
    { 0x0022, 0x5520, "Altimata AC101LF" },
    { 0x0000, 0x6b90, "AMD 79C901A HomePNA" },
    { 0x0000, 0x6b70, "AMD 79C901A 10baseT" },
    { 0x0181, 0xb800, "Davicom DM9101" },
    { 0x0043, 0x7411, "Enable EL40-331" },
    { 0x0015, 0xf410, "ICS 1889" },
    { 0x0015, 0xf420, "ICS 1890" },
    { 0x0015, 0xf430, "ICS 1892" },
    { 0x02a8, 0x0150, "Intel 82555" },
    { 0x7810, 0x0000, "Level One LXT970/971" },
    { 0x2000, 0x5c00, "National DP83840A" },
    { 0x0181, 0x4410, "Quality QS6612" },
    { 0x0282, 0x1c50, "SMSC 83C180" },
    { 0x0300, 0xe540, "TDK 78Q2120" },
};
#define NMII (sizeof(mii_id)/sizeof(mii_id[0]))

/*--------------------------------------------------------------------*/

struct option longopts[] = {
 /* { name  has_arg  *flag  val } */
    {"advertise",	1, 0, 'A'},	/* Change capabilities advertised. */
    {"force",		1, 0, 'F'},	/* Change capabilities advertised. */
    {"phy",		1, 0, 'p'},	/* Set PHY (MII address) to report. */
    {"log",		0, 0, 'l'},	/* Set PHY (MII address) to report. */
    {"restart",		0, 0, 'r'},	/* Restart link negotiation */
    {"reset",		0, 0, 'R'},	/* Reset the transceiver. */
    {"verbose", 	0, 0, 'v'},	/* Report each action taken.  */
    {"version", 	0, 0, 'V'},	/* Emit version information.  */
    {"watch", 		0, 0, 'w'},	/* Constantly monitor the port.  */
    {"help", 		0, 0, '?'},	/* Give help */
    { 0, 0, 0, 0 }
};

static unsigned int
    verbose = 0,
    opt_version = 0,
    opt_restart = 0,
    opt_reset = 0,
    opt_log = 0,
    opt_watch = 0;
static int nway_advertise = 0;
static int fixed_speed = 0;
static int override_phy = -1;

static int skfd = -1;		/* AF_INET socket for ioctl() calls. */
static struct ifreq ifr;

/*--------------------------------------------------------------------*/

static int mdio_read(int skfd, int location)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    if (ioctl(skfd, SIOCGMIIREG, &ifr) < 0) {
	fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr.ifr_name,
		strerror(errno));
	return -1;
    }
    return mii->val_out;
}

static void mdio_write(int skfd, int location, int value)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    mii->reg_num = location;
    mii->val_in = value;
    if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
	fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
		strerror(errno));
    }
}

/*--------------------------------------------------------------------*/

const struct {
    char	*name;
    u_short	value;
} media[] = {
    /* The order through 100baseT4 matches bits in the BMSR */
    { "10baseT-HD",	MII_AN_10BASET_HD },
    { "10baseT-FD",	MII_AN_10BASET_FD },
    { "100baseTx-HD",	MII_AN_100BASETX_HD },
    { "100baseTx-FD",	MII_AN_100BASETX_FD },
    { "100baseT4",	MII_AN_100BASET4 },
    { "100baseTx",	MII_AN_100BASETX_FD | MII_AN_100BASETX_HD },
    { "10baseT",	MII_AN_10BASET_FD | MII_AN_10BASET_HD },
};
#define NMEDIA (sizeof(media)/sizeof(media[0]))
	
/* Parse an argument list of media types */
static int parse_media(char *arg)
{
    int mask, i;
    char *s;
    mask = strtoul(arg, &s, 16);
    if ((*arg != '\0') && (*s == '\0')) {
	if ((mask & MII_AN_ABILITY_MASK) &&
	    !(mask & ~MII_AN_ABILITY_MASK))
	    return mask;
	goto failed;
    } else {
	mask = 0;
	s = strtok(arg, ", ");
	do {
	    for (i = 0; i < NMEDIA; i++)
		if (strcasecmp(media[i].name, s) == 0) break;
	    if (i == NMEDIA) goto failed;
	    mask |= media[i].value;
	} while ((s = strtok(NULL, ", ")) != NULL);
    }
    return mask;
failed:
    fprintf(stderr, "Invalid media specification '%s'.\n", arg);
    return -1;
}

/*--------------------------------------------------------------------*/

static char *media_list(int mask, int best)
{
    static char buf[100];
    int i;
    *buf = '\0';
    mask >>= 5;
    for (i = 4; i >= 0; i--) {
	if (mask & (1<<i)) {
	    strcat(buf, " ");
	    strcat(buf, media[i].name);
	    if (best) break;
	}
    }
    if (mask & (1<<5))
	strcat(buf, " flow-control");
    return buf;
}

int show_basic_mii(int sock, int phy_id)
{
    char buf[100];
    int i, mii_val[32];
    int bmcr, bmsr, advert, lkpar;

    /* Some bits in the BMSR are latched, but we can't rely on being
       the only reader, so only the current values are meaningful */
    mdio_read(sock, MII_BMSR);
    for (i = 0; i < ((verbose > 1) ? 32 : 8); i++)
	mii_val[i] = mdio_read(sock, i);

    if (mii_val[MII_BMCR] == 0xffff) {
	fprintf(stderr, "  No MII transceiver present!.\n");
	return -1;
    }

    /* Descriptive rename. */
    bmcr = mii_val[MII_BMCR]; bmsr = mii_val[MII_BMSR];
    advert = mii_val[MII_ANAR]; lkpar = mii_val[MII_ANLPAR];

    sprintf(buf, "%s: ", ifr.ifr_name);
    if (bmcr & MII_BMCR_AN_ENA) {
	if (bmsr & MII_BMSR_AN_COMPLETE) {
	    if (advert & lkpar) {
		strcat(buf, (lkpar & MII_AN_ACK) ?
		       "negotiated" : "no autonegotiation,");
		strcat(buf, media_list(advert & lkpar, 1));
		strcat(buf, ", ");
	    } else {
		strcat(buf, "autonegotiation failed, ");
	    }
	} else if (bmcr & MII_BMCR_RESTART) {
	    strcat(buf, "autonegotiation restarted, ");
	}
    } else {
	sprintf(buf+strlen(buf), "%s Mbit, %s duplex, ",
	       (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
	       (bmcr & MII_BMCR_DUPLEX) ? "full" : "half");
    }
    strcat(buf, (bmsr & MII_BMSR_LINK_VALID) ? "link ok" : "no link");

    if (opt_watch) {
	if (opt_log) {
	    syslog(LOG_INFO, buf);
	} else {
	    char s[20];
	    time_t t = time(NULL);
	    strftime(s, sizeof(s), "%T", localtime(&t));
	    printf("%s %s\n", s, buf);
	}
    } else {
	printf("%s\n", buf);
    }

    if (verbose > 1) {
	printf("  registers for MII PHY %d: ", phy_id);
	for (i = 0; i < 32; i++)
	    printf("%s %4.4x", ((i % 8) ? "" : "\n   "), mii_val[i]);
	printf("\n");
    }

    if (verbose) {
	printf("  product info: ");
	for (i = 0; i < NMII; i++)
	    if ((mii_id[i].id1 == mii_val[2]) &&
		(mii_id[i].id2 == (mii_val[3] & 0xfff0)))
		break;
	if (i < NMII)
	    printf("%s rev %d\n", mii_id[i].name, mii_val[3]&0x0f);
	else
	    printf("vendor %02x:%02x:%02x, model %d rev %d\n",
		   mii_val[2]>>10, (mii_val[2]>>2)&0xff,
		   ((mii_val[2]<<6)|(mii_val[3]>>10))&0xff,
		   (mii_val[3]>>4)&0x3f, mii_val[3]&0x0f);
	printf("  basic mode:   ");
	if (bmcr & MII_BMCR_RESET)
	    printf("software reset, ");
	if (bmcr & MII_BMCR_LOOPBACK)
	    printf("loopback, ");
	if (bmcr & MII_BMCR_ISOLATE)
	    printf("isolate, ");
	if (bmcr & MII_BMCR_COLTEST)
	    printf("collision test, ");
	if (bmcr & MII_BMCR_AN_ENA) {
	    printf("autonegotiation enabled\n");
	} else {
	    printf("%s Mbit, %s duplex\n",
		   (bmcr & MII_BMCR_100MBIT) ? "100" : "10",
		   (bmcr & MII_BMCR_DUPLEX) ? "full" : "half");
	}
	printf("  basic status: ");
	if (bmsr & MII_BMSR_AN_COMPLETE)
	    printf("autonegotiation complete, ");
	else if (bmcr & MII_BMCR_RESTART)
	    printf("autonegotiation restarted, ");
	if (bmsr & MII_BMSR_REMOTE_FAULT)
	    printf("remote fault, ");
	printf((bmsr & MII_BMSR_LINK_VALID) ? "link ok" : "no link");
	printf("\n  capabilities:%s", media_list(bmsr >> 6, 0));
	printf("\n  advertising: %s", media_list(advert, 0));
	if (lkpar & MII_AN_ABILITY_MASK)
	    printf("\n  link partner:%s", media_list(lkpar, 0));
	printf("\n");
    }
    return 0;
}

/*--------------------------------------------------------------------*/

static int do_one_xcvr(int skfd, char *ifname, int maybe)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;

    /* Get the vitals from the interface. */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
	if (!maybe || (errno != ENODEV))
	    fprintf(stderr, "SIOCGMIIPHY on '%s' failed: %s\n",
		    ifname, strerror(errno));
	return 1;
    }

    if (override_phy >= 0) {
	printf("using the specified MII index %d.\n", override_phy);
	mii->phy_id = override_phy;
    }

    if (opt_reset) {
	printf("resetting the transceiver...\n");
	mdio_write(skfd, MII_BMCR, MII_BMCR_RESET);
    }
    if (nway_advertise) {
	mdio_write(skfd, MII_ANAR, nway_advertise | 1);
	opt_restart = 1;
    }
    if (opt_restart) {
	printf("restarting autonegotiation...\n");
	mdio_write(skfd, MII_BMCR, 0x0000);
	mdio_write(skfd, MII_BMCR, MII_BMCR_AN_ENA|MII_BMCR_RESTART);
    }
    if (fixed_speed) {
	int bmcr = 0;
	if (fixed_speed & (MII_AN_100BASETX_FD|MII_AN_100BASETX_HD))
	    bmcr |= MII_BMCR_100MBIT;
	if (fixed_speed & (MII_AN_100BASETX_FD|MII_AN_10BASET_FD))
	    bmcr |= MII_BMCR_DUPLEX;
	mdio_write(skfd, MII_BMCR, bmcr);
    }

    if (!opt_restart && !opt_reset && !fixed_speed && !nway_advertise)
	show_basic_mii(skfd, mii->phy_id);

    return 0;
}

/*--------------------------------------------------------------------*/

static void watch_one_xcvr(int skfd, char *ifname, int index)
{
    struct mii_data *mii = (struct mii_data *)&ifr.ifr_data;
    static int status[MAX_ETH] = { 0, /* ... */ };
    int now;

    /* Get the vitals from the interface. */
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
	if (errno != ENODEV)
	    fprintf(stderr, "SIOCGMIIPHY on '%s' failed: %s\n",
		    ifname, strerror(errno));
	return;
    }
    now = (mdio_read(skfd, MII_BMCR) |
	   (mdio_read(skfd, MII_BMSR) << 16));
    if (status[index] && (status[index] != now))
	show_basic_mii(skfd, mii->phy_id);
    status[index] = now;
}

/*--------------------------------------------------------------------*/

const char *usage =
"usage: %s [-VvRrwl] [-A media,... | -F media] [interface ...]
       -V, --version               display version information
       -v, --verbose               more verbose output
       -R, --reset                 reset MII to poweron state
       -r, --restart               restart autonegotiation
       -w, --watch                 monitor for link status changes
       -l, --log                   with -w, write events to syslog
       -A, --advertise=media,...   advertise only specified media
       -F, --force=media           force specified media technology
media: 100baseT4, 100baseTx-FD, 100baseTx-HD, 10baseT-FD, 10baseT-HD,
       (to advertise both HD and FD) 100baseTx, 10baseT\n";

int main(int argc, char **argv)
{
    int i, c, ret, errflag = 0;
    char s[6];
    
    while ((c = getopt_long(argc, argv, "A:F:p:lrRvVw?", longopts, 0)) != EOF)
	switch (c) {
	case 'A': nway_advertise = parse_media(optarg); break;
	case 'F': fixed_speed = parse_media(optarg); break;
	case 'p': override_phy = atoi(optarg); break;
	case 'r': opt_restart++;	break;
	case 'R': opt_reset++;		break;
	case 'v': verbose++;		break;
	case 'V': opt_version++;	break;
	case 'w': opt_watch++;		break;
	case 'l': opt_log++;		break;
	case '?': errflag++;
	}
    /* Check for a few inappropriate option combinations */
    if (opt_watch) verbose = 0;
    if (errflag || (fixed_speed & (fixed_speed-1)) ||
	(fixed_speed && (opt_restart || nway_advertise))) {
	fprintf(stderr, usage, argv[0]);
	return 2;
    }

    if (opt_version)
	printf(version);

    /* Open a basic socket. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
	perror("socket");
	exit(-1);
    }

    /* No remaining args means show all interfaces. */
    if (optind == argc) {
	ret = 1;
	for (i = 0; i < MAX_ETH; i++) {
	    sprintf(s, "eth%d", i);
	    ret &= do_one_xcvr(skfd, s, 1);
	}
	if (ret)
	    fprintf(stderr, "no MII interfaces found\n");
    } else {
	ret = 0;
	for (i = optind; i < argc; i++) {
	    ret |= do_one_xcvr(skfd, argv[i], 0);
	}
    }

    if (opt_watch && (ret == 0)) {
	while (1) {
	    sleep(1);
	    if (optind == argc) {
		for (i = 0; i < MAX_ETH; i++) {
		    sprintf(s, "eth%d", i);
		    watch_one_xcvr(skfd, s, i);
		}
	    } else {
		for (i = optind; i < argc; i++)
		    watch_one_xcvr(skfd, argv[i], i-optind);
	    }
	}
    }

    close(skfd);
    return ret;
}
