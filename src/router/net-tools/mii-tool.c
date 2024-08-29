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
#include <linux/sockios.h>

#include <linux/mii.h>
#include <linux/sockios.h>
#include "version.h"
#include "net-support.h"
#include "util.h"

static char *Release = RELEASE, *Signature = "David Hinds based on Donald Becker's mii-diag";

#define MAX_ETH		8		/* Maximum # of interfaces */
#define LPA_ABILITY_MASK	0x07e0

/* Table of known MII's */
static const struct {
    u_short	id1, id2;
    char	*name;
} mii_id[] = {
    { 0x0022, 0x5610, "AdHoc AH101LF" },
    { 0x0022, 0x5520, "Altimata AC101LF" },
    { 0x0000, 0x6b90, "AMD 79C901A HomePNA" },
    { 0x0000, 0x6b70, "AMD 79C901A 10baseT" },
    { 0x0181, 0xb800, "Davicom DM9101" },
    { 0x0043, 0x7410, "Enable EL40-331" },
    { 0x0243, 0x0c50, "ICPlus IP101A" },
    { 0x0015, 0xf410, "ICS 1889" },
    { 0x0015, 0xf420, "ICS 1890" },
    { 0x0015, 0xf430, "ICS 1892" },
    { 0x02a8, 0x0150, "Intel 82555" },
    { 0x7810, 0x0000, "Level One LXT970/971" },
    { 0x0022, 0x1510, "Micrel KSZ8041" },
    { 0x0022, 0x1610, "Micrel KSZ9021" },
    { 0x2000, 0x5c00, "National DP83840A" },
    { 0x2000, 0x5c70, "National DP83865" },
    { 0x0181, 0x4410, "Quality QS6612" },
    { 0x0282, 0x1c50, "SMSC 83C180" },
    { 0x0203, 0x8460, "STMicroelectronics ST802RT" },
    { 0x1c04, 0x0010, "STMicroelectronics STE100P" },
    { 0x0300, 0xe540, "TDK 78Q2120" },
    { 0x0141, 0x0c20, "Yukon 88E1011" },
    { 0x0141, 0x0cc0, "Yukon-EC 88E1111" },
    { 0x0141, 0x0c90, "Yukon-2 88E1112" },
};
#define NMII (sizeof(mii_id)/sizeof(mii_id[0]))

/*--------------------------------------------------------------------*/

struct option longopts[] = {
 /* { name  has_arg  *flag  val } */
    {"advertise",	1, 0, 'A'},	/* Advertise only specified media. */
    {"force",		1, 0, 'F'},	/* Force specified media technology. */
    {"phy",		1, 0, 'p'},	/* Set PHY (MII address) to report. */
    {"log",		0, 0, 'l'},	/* With --watch, write events to syslog. */
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
    struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;
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
    struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;
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
    u_short	value[2];
} media[] = {
    /* The order through 100baseT4 matches bits in the BMSR */
    { "10baseT-HD",	{LPA_10HALF} },
    { "10baseT-FD",	{LPA_10FULL} },
    { "100baseTx-HD",	{LPA_100HALF} },
    { "100baseTx-FD",	{LPA_100FULL} },
    { "100baseT4",	{LPA_100BASE4} },
    { "100baseTx",	{LPA_100FULL | LPA_100HALF} },
    { "10baseT",	{LPA_10FULL | LPA_10HALF} },

    { "1000baseT-HD",	{0, ADVERTISE_1000HALF} },
    { "1000baseT-FD",	{0, ADVERTISE_1000FULL} },
    { "1000baseT",	{0, ADVERTISE_1000HALF|ADVERTISE_1000FULL} },
};
#define NMEDIA (sizeof(media)/sizeof(media[0]))

/* Parse an argument list of media types */
static int parse_media(char *arg, unsigned *bmcr2)
{
    int mask, i;
    char *s;
    mask = strtoul(arg, &s, 16);
    if ((*arg != '\0') && (*s == '\0')) {
	if ((mask & LPA_ABILITY_MASK) &&
	    !(mask & ~LPA_ABILITY_MASK)) {
		*bmcr2 = 0;
		return mask;
	}
	goto failed;
    }
    mask = 0;
    *bmcr2 = 0;
    s = strtok(arg, ", ");
    do {
	    for (i = 0; i < NMEDIA; i++)
		if (s && strcasecmp(media[i].name, s) == 0) break;
	    if (i == NMEDIA) goto failed;
	    mask |= media[i].value[0];
	    *bmcr2 |= media[i].value[1];
    } while ((s = strtok(NULL, ", ")) != NULL);

    return mask;
failed:
    fprintf(stderr, "Invalid media specification '%s'.\n", arg);
    return -1;
}

/*--------------------------------------------------------------------*/

static const char *media_list(unsigned mask, unsigned mask2, int best)
{
    static char buf[100];
    int i;
    *buf = '\0';

    if (mask & BMCR_SPEED1000) {
	if (mask2 & ADVERTISE_1000HALF) {
	    strcat(buf, " ");
	    strcat(buf, "1000baseT-HD");
	    if (best) goto out;
	}
	if (mask2 & ADVERTISE_1000FULL) {
	    strcat(buf, " ");
	    strcat(buf, "1000baseT-FD");
	    if (best) goto out;
	}
    }

    mask >>= 5;
    for (i = 4; i >= 0; i--) {
	if (mask & (1<<i)) {
	    strcat(buf, " ");
	    strcat(buf, media[i].name);
	    if (best) break;
	}
    }
 out:
    if (mask & (1<<5))
	strcat(buf, " flow-control");
    return buf;
}

int show_basic_mii(int sock, int phy_id)
{
    char buf[200];
    int i, mii_val[32];
    unsigned bmcr, bmsr, advert, lkpar, bmcr2, lpa2;

    /* Some bits in the BMSR are latched, but we can't rely on being
       the only reader, so only the current values are meaningful */
    mdio_read(sock, MII_BMSR);
    for (i = 0; i < ((verbose > 1) ? 32 : (MII_STAT1000+1)); i++)
	switch (i & 0x1F) {
	    case MII_BMCR:
	    case MII_BMSR:
	    case MII_PHYSID1:
	    case MII_PHYSID2:
	    case MII_ADVERTISE:
	    case MII_LPA:
	    case MII_EXPANSION:
	    case MII_CTRL1000:
	    case MII_STAT1000:
	    case MII_ESTATUS:
	    case MII_DCOUNTER:
	    case MII_FCSCOUNTER:
	    case MII_NWAYTEST:
	    case MII_RERRCOUNTER:
	    case MII_SREVISION:
	    case MII_RESV1:
	    case MII_LBRERROR:
	    case MII_PHYADDR:
	    case MII_RESV2:
	    case MII_TPISTATUS:
	    case MII_NCONFIG:
		mii_val[i] = mdio_read(sock, i);
		break;
	    default:
		if (verbose > 2)
		    mii_val[i] = mdio_read(sock, i);
		else
		    mii_val[i] = 0;
		break;
        }

    if (mii_val[MII_BMCR] == 0xffff  || mii_val[MII_BMSR] == 0x0000) {
	fprintf(stderr, "  No MII transceiver present!.\n");
	return -1;
    }

    /* Descriptive rename. */
    bmcr = mii_val[MII_BMCR]; bmsr = mii_val[MII_BMSR];
    advert = mii_val[MII_ADVERTISE]; lkpar = mii_val[MII_LPA];
    bmcr2 = mii_val[MII_CTRL1000]; lpa2 = mii_val[MII_STAT1000];

    sprintf(buf, "%s: ", ifr.ifr_name);
    if (bmcr & BMCR_ANENABLE) {
	if (bmsr & BMSR_ANEGCOMPLETE) {
	    if (advert & lkpar) {
		strcat(buf, (lkpar & LPA_LPACK) ?
		       "negotiated" : "no autonegotiation,");
		strcat(buf, media_list(advert & lkpar, bmcr2 & lpa2>>2, 1));
		strcat(buf, ", ");
	    } else {
		strcat(buf, "autonegotiation failed, ");
	    }
	} else if (bmcr & BMCR_ANRESTART) {
	    strcat(buf, "autonegotiation restarted, ");
	}
    } else {
	sprintf(buf+strlen(buf), "%s Mbit, %s duplex, ",
		((bmcr2 & (ADVERTISE_1000HALF | ADVERTISE_1000FULL)) & lpa2 >> 2)
		? "1000"
		: (bmcr & BMCR_SPEED100) ? "100" : "10",
		(bmcr & BMCR_FULLDPLX) ? "full" : "half");
    }
    strcat(buf, (bmsr & BMSR_LSTATUS) ? "link ok" : "no link");

    if (opt_watch) {
	if (opt_log) {
	    syslog(LOG_INFO, "%s", buf);
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
	if (bmcr & BMCR_RESET)
	    printf("software reset, ");
	if (bmcr & BMCR_LOOPBACK)
	    printf("loopback, ");
	if (bmcr & BMCR_ISOLATE)
	    printf("isolate, ");
	if (bmcr & BMCR_CTST)
	    printf("collision test, ");
	if (bmcr & BMCR_ANENABLE) {
	    printf("autonegotiation enabled\n");
	} else {
	    printf("%s Mbit, %s duplex\n",
		   (bmcr & BMCR_SPEED100) ? "100" : "10",
		   (bmcr & BMCR_FULLDPLX) ? "full" : "half");
	}
	printf("  basic status: ");
	if (bmsr & BMSR_ANEGCOMPLETE)
	    printf("autonegotiation complete, ");
	else if (bmcr & BMCR_ANRESTART)
	    printf("autonegotiation restarted, ");
	if (bmsr & BMSR_RFAULT)
	    printf("remote fault, ");
	printf((bmsr & BMSR_LSTATUS) ? "link ok" : "no link");
	printf("\n  capabilities:%s", media_list(bmsr >> 6, bmcr2, 0));
	printf("\n  advertising: %s", media_list(advert, bmcr2, 0));
	if (lkpar & LPA_ABILITY_MASK)
	    printf("\n  link partner:%s", media_list(lkpar, lpa2 >> 2, 0));
	printf("\n");
    }
    fflush(stdout);
    return 0;
}

/*--------------------------------------------------------------------*/

static int do_one_xcvr(int skfd, char *ifname, int maybe)
{
    struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;

    /* Get the vitals from the interface. */
    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
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
	mdio_write(skfd, MII_BMCR, BMCR_RESET);
    }
    if (nway_advertise > 0) {
	mdio_write(skfd, MII_ADVERTISE, nway_advertise | 1);
	opt_restart = 1;
    }
    if (opt_restart) {
	printf("restarting autonegotiation...\n");
	mdio_write(skfd, MII_BMCR, 0x0000);
	mdio_write(skfd, MII_BMCR, BMCR_ANENABLE|BMCR_ANRESTART);
    }
    if (fixed_speed) {
	int bmcr = 0;
	if (fixed_speed & (LPA_100FULL|LPA_100HALF))
	    bmcr |= BMCR_SPEED100;
	if (fixed_speed & (LPA_100FULL|LPA_10FULL))
	    bmcr |= BMCR_FULLDPLX;
	mdio_write(skfd, MII_BMCR, bmcr);
    }

    if (!opt_restart && !opt_reset && !fixed_speed && !nway_advertise)
	show_basic_mii(skfd, mii->phy_id);

    return 0;
}

/*--------------------------------------------------------------------*/

static void watch_one_xcvr(int skfd, char *ifname, int index)
{
    struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr.ifr_data;
    static int status[MAX_ETH] = { 0, /* ... */ };
    int now;

    /* Get the vitals from the interface. */
    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGMIIPHY, &ifr) < 0) {
	if (errno != ENODEV)
	    fprintf(stderr, "SIOCGMIIPHY on '%s' failed: %s\n",
		    ifname, strerror(errno));
	return;
    }
    if (override_phy >= 0) {
        mii->phy_id = override_phy;
    }
    now = (mdio_read(skfd, MII_BMCR) |
	   (mdio_read(skfd, MII_BMSR) << 16));
    if (status[index] && (status[index] != now))
	show_basic_mii(skfd, mii->phy_id);
    status[index] = now;
}

/*--------------------------------------------------------------------*/

const char *usage =
"usage: %s [-VvRrwl] [-A media,... | -F media] [-p addr] <interface ...>\n"
"       -V, --version               display version information\n"
"       -v, --verbose               more verbose output\n"
"       -R, --reset                 reset MII to poweron state\n"
"       -r, --restart               restart autonegotiation\n"
"       -w, --watch                 monitor for link status changes\n"
"       -l, --log                   with -w, write events to syslog\n"
"       -A, --advertise=media,...   advertise only specified media\n"
"       -F, --force=media           force specified media technology\n"
"       -p, --phy=addr              set PHY (MII address) to report\n"
"media: 1000baseTx-HD, 1000baseTx-FD,\n"
"       100baseT4, 100baseTx-FD, 100baseTx-HD,\n"
"       10baseT-FD, 10baseT-HD,\n"
"       (to advertise both HD and FD) 1000baseTx, 100baseTx, 10baseT\n";


static void version(void)
{
    printf("%s\n%s\n", Release, Signature);
    exit(E_VERSION);
}


int main(int argc, char **argv)
{
    int i, c, ret, errflag = 0;
    unsigned ctrl1000 = 0;

    while ((c = getopt_long(argc, argv, "A:F:p:lrRvVw?", longopts, 0)) != EOF)
	switch (c) {
	case 'A': nway_advertise = parse_media(optarg, &ctrl1000); break;
	case 'F': fixed_speed = parse_media(optarg, &ctrl1000); break;
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

    if ((nway_advertise < 0) || (fixed_speed < 0))
    	return 2;

    if (errflag || (fixed_speed & (fixed_speed-1)) ||
	(fixed_speed && (opt_restart || nway_advertise))) {
	fprintf(stderr, usage, argv[0]);
	return 2;
    }

    if (opt_version)
	version();

    /* Open a basic socket. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM,0)) < 0) {
	perror("socket");
	exit(-1);
    }

    if (verbose > 1)
    	printf("Using SIOCGMIIPHY=0x%x\n", SIOCGMIIPHY);

    /* No remaining args means show all interfaces. */
    if (optind == argc) {
	fprintf(stderr, "No interface specified\n");
	fprintf(stderr, usage, argv[0]);
	close(skfd);
	return 2;
    } else {
	ret = 0;
	for (i = optind; i < argc; i++) {
	    ret |= do_one_xcvr(skfd, argv[i], 0);
	}
    }

    if (opt_watch && (ret == 0)) {
	while (1) {
	    sleep(1);
	    for (i = optind; i < argc; i++)
		    watch_one_xcvr(skfd, argv[i], i-optind);
	}
    }

    close(skfd);
    return ret;
}
