/*
 * arp                This file contains an implementation of the command
 *              that maintains the kernel's ARP cache.  It is derived
 *              from Berkeley UNIX arp(8), but cleaner and with sup-
 *              port for devices other than Ethernet.
 *
 * NET-TOOLS    A collection of programs that form the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system.
 *
 * Version:     $Id: arp.c,v 1.27 2009/09/06 22:50:11 vapier Exp $
 *
 * Maintainer:  Bernd 'eckes' Eckenfels, <net-tools@lina.inka.de>
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *
 * Changes:
 *              (based on work from Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>)
 *              Alan Cox        :       modified for NET3
 *              Andrew Tridgell :       proxy arp netmasks
 *              Bernd Eckenfels :       -n option
 *              Bernd Eckenfels :       Use only /proc for display
 *       {1.60} Bernd Eckenfels :       new arpcode (-i) for 1.3.42 but works
 *                                      with 1.2.x, too
 *       {1.61} Bernd Eckenfels :       more verbose messages
 *       {1.62} Bernd Eckenfels :       check -t for hw adresses and try to
 *                                      explain EINVAL (jeff)
 *970125 {1.63} Bernd Eckenfels :       -a print hardwarename instead of tiltle
 *970201 {1.64} Bernd Eckenfels :       net-features.h support
 *970203 {1.65} Bernd Eckenfels :       "#define" in "#if",
 *                                      -H|-A additional to -t|-p
 *970214 {1.66} Bernd Eckenfels :       Fix optarg required for -H and -A
 *970412 {1.67} Bernd Eckenfels :       device=""; is default
 *970514 {1.68} Bernd Eckenfels :       -N and -D
 *970517 {1.69} Bernd Eckenfels :       usage() fixed
 *970622 {1.70} Bernd Eckenfels :       arp -d priv
 *970106 {1.80} Bernd Eckenfels :       new syntax without -D and with "dev <If>",
 *                                      ATF_MAGIC, ATF_DONTPUB support.
 *                                      Typo fix (Debian Bug#5728 Giuliano Procida)
 *970803 {1.81} Bernd Eckenfels :       removed junk comment line 1
 *970925 {1.82} Bernd Eckenfels :       include fix for libc6
 *980213 (1.83) Phil Blundell:          set ATF_COM on new entries
 *980629 (1.84) Arnaldo Carvalho de Melo: gettext instead of catgets
 *990101 {1.85} Bernd Eckenfels		fixed usage and return codes
 *990105 (1.86) Phil Blundell:		don't ignore EINVAL in arp_set
 *991121 (1.87) Bernd Eckenfels:	yes --device has a mandatory arg
 *010404 (1.88) Arnaldo Carvalho de Melo: use setlocale
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
/* #include <linux/netdevice.h> */
/* #include <linux/if_arp.h>    */
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "config.h"
#include "intl.h"
#include "util.h"

#define DFLT_AF	"inet"
#define DFLT_HW	"ether"

#define FEATURE_ARP
#include "lib/net-features.h"

static char *Release = RELEASE;

int opt_n = 0;			/* do not resolve addresses     */
int opt_N = 0;			/* use symbolic names           */
int opt_v = 0;			/* debugging output flag        */
int opt_D = 0;			/* HW-address is devicename     */
int opt_e = 0;			/* 0=BSD output, 1=new linux    */
int opt_a = 0;			/* all entries, substring match */
const struct aftype *ap;/* current address family       */
const struct hwtype *hw;/* current hardware type        */
int sockfd = 0;			/* active socket descriptor     */
int hw_set = 0;			/* flag if hw-type was set (-H) */
char device[16] = "";		/* current device               */
static void usage(int);

/* Delete an entry from the ARP cache. */
static int arp_del(char **args)
{
    char host[128];
    struct arpreq req;
    struct sockaddr_storage ss;
    struct sockaddr *sa = (struct sockaddr *)&ss;
    int flags = 0;
    int deleted = 0;

    memset((char *) &req, 0, sizeof(req));

    /* Resolve the host name. */
    if (*args == NULL) {
	fprintf(stderr, _("arp: need host name\n"));
	return (-1);
    }
    safe_strncpy(host, *args, (sizeof host));
    if (ap->input(0, host, &ss) < 0) {
	ap->herror(host);
	return (-1);
    }
    /* If a host has more than one address, use the correct one! */
    memcpy((char *) &req.arp_pa, (char *) sa, sizeof(struct sockaddr));

    if (hw_set)
	req.arp_ha.sa_family = hw->type;

    req.arp_flags = ATF_PERM;
    args++;
    while (*args != NULL) {
	if (opt_v)
	    fprintf(stderr, "args=%s\n", *args);
	if (!strcmp(*args, "pub")) {
	    flags |= 1;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "priv")) {
	    flags |= 2;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "temp")) {
	    req.arp_flags &= ~ATF_PERM;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "trail")) {
	    req.arp_flags |= ATF_USETRAILERS;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "dontpub")) {
#ifdef ATF_DONTPUB
	    req.arp_flags |= ATF_DONTPUB;
#else
	    ENOSUPP("arp", "ATF_DONTPUB");
#endif
	    args++;
	    continue;
	}
	if (!strcmp(*args, "auto")) {
#ifdef ATF_MAGIC
	    req.arp_flags |= ATF_MAGIC;
#else
	    ENOSUPP("arp", "ATF_MAGIC");
#endif
	    args++;
	    continue;
	}
	if (!strcmp(*args, "dev")) {
	    if (*++args == NULL)
		usage(E_OPTERR);
	    safe_strncpy(device, *args, sizeof(device));
	    args++;
	    continue;
	}
	if (!strcmp(*args, "netmask")) {
	    if (*++args == NULL)
		usage(E_OPTERR);
	    if (strcmp(*args, "255.255.255.255") != 0) {
		safe_strncpy(host, *args, (sizeof host));
		if (ap->input(0, host, &ss) < 0) {
		    ap->herror(host);
		    return (-1);
		}
		memcpy((char *) &req.arp_netmask, (char *) sa,
		       sizeof(struct sockaddr));
		req.arp_flags |= ATF_NETMASK;
	    }
	    args++;
	    continue;
	}
	usage(E_OPTERR);
    }

    // if neighter priv nor pub is given, work on both
    if (flags == 0)
	flags = 3;

    safe_strncpy(req.arp_dev, device, sizeof(req.arp_dev));

    /* unfortuatelly the kernel interface does not allow us to
       delete private entries anlone, so we need this hack
       to avoid "not found" errors if we try both. */
    deleted = 0;

    /* Call the kernel. */
    if (flags & 2) {
	if (opt_v)
	    fprintf(stderr, "arp: SIOCDARP(dontpub)\n");
	if (ioctl(sockfd, SIOCDARP, &req) < 0) {
	    if ((errno == ENXIO) || (errno == ENOENT)) {
		if (flags & 1)
		    goto dontpub;
		printf(_("No ARP entry for %s\n"), host);
		return (-1);
	    }
	    perror("SIOCDARP(dontpub)");
	    return (-1);
	} else
	  deleted = 1;
    }
    if (!deleted && (flags & 1)) {
      dontpub:
	req.arp_flags |= ATF_PUBL;
	if (opt_v)
	    fprintf(stderr, "arp: SIOCDARP(pub)\n");
	if (ioctl(sockfd, SIOCDARP, &req) < 0) {
	    if ((errno == ENXIO) || (errno == ENOENT)) {
		printf(_("No ARP entry for %s\n"), host);
		return (-1);
	    }
	    perror("SIOCDARP(pub)");
	    return (-1);
	}
    }
    return (0);
}

/* Get the hardware address to a specified interface name */
static int arp_getdevhw(char *ifname, struct sockaddr *sa, const struct hwtype *hw)
{
    struct ifreq ifr;
    const struct hwtype *xhw;

    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
	fprintf(stderr, _("arp: cant get HW-Address for `%s': %s.\n"), ifname, strerror(errno));
	return (-1);
    }
    if (hw && (ifr.ifr_hwaddr.sa_family != hw->type)) {
	fprintf(stderr, _("arp: protocol type mismatch.\n"));
	return (-1);
    }
    memcpy((char *) sa, (char *) &(ifr.ifr_hwaddr), sizeof(struct sockaddr));

    if (opt_v) {
	if (!(xhw = get_hwntype(ifr.ifr_hwaddr.sa_family)) || (xhw->print == 0)) {
	    xhw = get_hwntype(-1);
	}
	fprintf(stderr, _("arp: device `%s' has HW address %s `%s'.\n"), ifname, xhw->name, xhw->print((char *)&ifr.ifr_hwaddr.sa_data));
    }
    return (0);
}

/* Set an entry in the ARP cache. */
static int arp_set(char **args)
{
    char host[128];
    struct arpreq req;
    struct sockaddr_storage ss;
    struct sockaddr *sa = (struct sockaddr *)&ss;
    int flags;

    memset((char *) &req, 0, sizeof(req));

    /* Resolve the host name. */
    if (*args == NULL) {
	fprintf(stderr, _("arp: need host name\n"));
	return (-1);
    }
    safe_strncpy(host, *args++, (sizeof host));
    if (ap->input(0, host, &ss) < 0) {
	ap->herror(host);
	return (-1);
    }
    /* If a host has more than one address, use the correct one! */
    memcpy((char *) &req.arp_pa, (char *) sa, sizeof(struct sockaddr));

    /* Fetch the hardware address. */
    if (*args == NULL) {
	fprintf(stderr, _("arp: need hardware address\n"));
	return (-1);
    }
    if (opt_D) {
	if (arp_getdevhw(*args++, &req.arp_ha, hw_set ? hw : NULL) < 0)
	    return (-1);
    } else {
	if (hw->input(*args++, &ss) < 0) {
	    fprintf(stderr, _("arp: invalid hardware address\n"));
	    return (-1);
	}
	memcpy(&req.arp_ha, sa, sizeof(*sa));
    }

    /* Check out any modifiers. */
    flags = ATF_PERM | ATF_COM;
    while (*args != NULL) {
	if (!strcmp(*args, "temp")) {
	    flags &= ~ATF_PERM;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "pub")) {
	    flags |= ATF_PUBL;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "priv")) {
	    flags &= ~ATF_PUBL;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "trail")) {
	    flags |= ATF_USETRAILERS;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "dontpub")) {
#ifdef ATF_DONTPUB
	    flags |= ATF_DONTPUB;
#else
	    ENOSUPP("arp", "ATF_DONTPUB");
#endif
	    args++;
	    continue;
	}
	if (!strcmp(*args, "auto")) {
#ifdef ATF_MAGIC
	    flags |= ATF_MAGIC;
#else
	    ENOSUPP("arp", "ATF_MAGIC");
#endif
	    args++;
	    continue;
	}
	if (!strcmp(*args, "dev")) {
	    if (*++args == NULL)
		usage(E_OPTERR);
	    safe_strncpy(device, *args, sizeof(device));
	    args++;
	    continue;
	}
	if (!strcmp(*args, "netmask")) {
	    if (*++args == NULL)
		usage(E_OPTERR);
	    if (strcmp(*args, "255.255.255.255") != 0) {
		safe_strncpy(host, *args, (sizeof host));
		if (ap->input(0, host, &ss) < 0) {
		    ap->herror(host);
		    return (-1);
		}
		memcpy((char *) &req.arp_netmask, (char *) sa,
		       sizeof(struct sockaddr));
		flags |= ATF_NETMASK;
	    }
	    args++;
	    continue;
	}
	usage(E_OPTERR);
    }

    /* Fill in the remainder of the request. */
    req.arp_flags = flags;

    safe_strncpy(req.arp_dev, device, sizeof(req.arp_dev));

    /* Call the kernel. */
    if (opt_v)
	fprintf(stderr, "arp: SIOCSARP()\n");
    if (ioctl(sockfd, SIOCSARP, &req) < 0) {
        perror("SIOCSARP");
	return (-1);
    }
    return (0);
}


/* Process an EtherFile */
static int arp_file(char *name)
{
    char buff[1024];
    char *sp, *args[32];
    int linenr, argc;
    FILE *fp;

    if ((fp = fopen(name, "r")) == NULL) {
	fprintf(stderr, _("arp: cannot open etherfile %s !\n"), name);
	return (-1);
    }
    /* Read the lines in the file. */
    linenr = 0;
    while (fgets(buff, sizeof(buff), fp) != (char *) NULL) {
	linenr++;
	if (opt_v == 1)
	    fprintf(stderr, ">> %s", buff);
	if ((sp = strchr(buff, '\n')) != (char *) NULL)
	    *sp = '\0';
	if (buff[0] == '#' || buff[0] == '\0')
	    continue;

	argc = getargs(buff, args);
	if (argc < 2) {
	    fprintf(stderr, _("arp: format error on line %u of etherfile %s !\n"),
		    linenr, name);
	    continue;
	}
	if (strchr (args[0], ':') != NULL) {
	    /* We have a correct ethers file, switch hw adress and hostname
	       for arp */
	    char *cp;
	    cp = args[1];
	    args[1] = args[0];
	    args[0] = cp;
	}
	if (arp_set(args) != 0)
	    fprintf(stderr, _("arp: cannot set entry on line %u of etherfile %s !\n"),
		    linenr, name);
    }

    (void) fclose(fp);
    return (0);
}


/* Print the contents of an ARP request block. */
static void arp_disp_2(const char *name, int type, int arp_flags, const char *hwa, const char *mask, const char *dev)
{
    static int title = 0;
    const struct hwtype *xhw;
    char flags[10];

    xhw = get_hwntype(type);
    if (xhw == NULL)
	xhw = get_hwtype(DFLT_HW);

    if (title++ == 0) {
	printf(_("Address                  HWtype  HWaddress           Flags Mask            Iface\n"));
    }
    /* Setup the flags. */
    flags[0] = '\0';
    if (arp_flags & ATF_COM)
	strcat(flags, "C");
    if (arp_flags & ATF_PERM)
	strcat(flags, "M");
    if (arp_flags & ATF_PUBL)
	strcat(flags, "P");
#ifdef ATF_MAGIC
    if (arp_flags & ATF_MAGIC)
	strcat(flags, "A");
#endif
#ifdef ATF_DONTPUB
    if (arp_flags & ATF_DONTPUB)
	strcat(flags, "!");
#endif
    if (arp_flags & ATF_USETRAILERS)
	strcat(flags, "T");

    if (!(arp_flags & ATF_NETMASK))
	mask = "";

    printf("%-23.23s  ", name);

    if (!(arp_flags & ATF_COM)) {
	if (arp_flags & ATF_PUBL)
	    printf("%-8.8s%-20.20s", "*", _("<from_interface>"));
	else
	    printf("%-8.8s%-20.20s", "", _("(incomplete)"));
    } else {
	printf("%-8.8s%-20.20s", xhw->name, hwa);
    }

    printf("%-6.6s%-15.15s %s\n", flags, mask, dev);
}

/* Print the contents of an ARP request block. */
static void arp_disp(const char *name, const char *ip, int type, int arp_flags, const char *hwa, const char *mask, const char *dev)
{
    const struct hwtype *xhw;

    xhw = get_hwntype(type);
    if (xhw == NULL)
	xhw = get_hwtype(DFLT_HW);

    printf(_("%s (%s) at "), name, ip);

    if (!(arp_flags & ATF_COM)) {
	if (arp_flags & ATF_PUBL)
	    printf("<from_interface> ");
	else
	    printf(_("<incomplete> "));
    } else {
	printf("%s [%s] ", hwa, xhw->name);
    }

    if (arp_flags & ATF_NETMASK)
	printf(_("netmask %s "), mask);

    if (arp_flags & ATF_PERM)
	printf("PERM ");
    if (arp_flags & ATF_PUBL)
	printf("PUB ");
#ifdef ATF_MAGIC
    if (arp_flags & ATF_MAGIC)
	printf("AUTO ");
#endif
#ifdef ATF_DONTPUB
    if (arp_flags & ATF_DONTPUB)
	printf("DONTPUB ");
#endif
    if (arp_flags & ATF_USETRAILERS)
	printf("TRAIL ");

    printf(_("on %s\n"), dev);
}


/* Display the contents of the ARP cache in the kernel. */
static int arp_show(char *name)
{
    char host[100];
    struct sockaddr_storage ss;
    char ip[100];
    char hwa[100];
    char mask[100];
    char line[200];
    char dev[100];
    int type, flags;
    FILE *fp;
    const char *hostname;
    int num, entries = 0, showed = 0;

    host[0] = '\0';

    if (name != NULL) {
	/* Resolve the host name. */
	safe_strncpy(host, name, (sizeof host));
	if (ap->input(0, host, &ss) < 0) {
	    ap->herror(host);
	    return (-1);
	}
	safe_strncpy(host, ap->sprint(&ss, 1), sizeof(host));
    }
    /* Open the PROCps kernel table. */
    if ((fp = fopen(_PATH_PROCNET_ARP, "r")) == NULL) {
	perror(_PATH_PROCNET_ARP);
	return (-1);
    }
    /* Bypass header -- read until newline */
    if (fgets(line, sizeof(line), fp) != (char *) NULL) {
	safe_strncpy(mask, "-", sizeof(mask));
	safe_strncpy(dev, "-", sizeof(dev));
	/* Read the ARP cache entries. */
	for (; fgets(line, sizeof(line), fp);) {
	    num = sscanf(line, "%s 0x%x 0x%x %99s %99s %99s\n",
			 ip, &type, &flags, hwa, mask, dev);
	    if (num < 4)
		break;
	    if (num == 5) {
		/*
		 * This happens for incomplete ARP entries for which there is
		 * no hardware address in the line.
		 */
		num = sscanf(line, "%s 0x%x 0x%x %99s %99s\n",
			     ip, &type, &flags, mask, dev);
		hwa[0] = 0;
	    }

	    entries++;
	    /* if the user specified hw-type differs, skip it */
	    if (hw_set && (type != hw->type))
		continue;

	    /* if the user specified address differs, skip it */
	    if (host[0] && strcmp(ip, host))
		continue;

	    /* if the user specified device differs, skip it */
	    if (device[0] && strcmp(dev, device))
		continue;

	    showed++;
	    /* This IS ugly but it works -be */
	    if (opt_n)
		hostname = "?";
	    else {
		if (ap->input(0, ip, &ss) < 0)
		    hostname = ip;
		else
		    hostname = ap->sprint(&ss, opt_n | 0x8000);
		if (strcmp(hostname, ip) == 0)
		    hostname = "?";
	    }

	    if (opt_e)
		arp_disp_2(hostname[0] == '?' ? ip : hostname, type, flags, hwa, mask, dev);
	    else
		arp_disp(hostname, ip, type, flags, hwa, mask, dev);
	}
    }
    if (opt_v)
	printf(_("Entries: %d\tSkipped: %d\tFound: %d\n"), entries, entries - showed, showed);

    if (!showed) {
	if (host[0] && !opt_a)
	    printf(_("%s (%s) -- no entry\n"), name, host);
	else if (hw_set || host[0] || device[0]) {
	    printf(_("arp: in %d entries no match found.\n"), entries);
	}
    }
    (void) fclose(fp);
    return (0);
}

static void version(void)
{
    printf("%s\n%s\n", Release, Features);
    exit(E_VERSION);
}

static void usage(int rc)
{
    FILE *fp = rc ? stderr : stdout;
    fprintf(fp, _("Usage:\n  arp [-vn]  [<HW>] [-i <if>] [-a] [<hostname>]             <-Display ARP cache\n"));
    fprintf(fp, _("  arp [-v]          [-i <if>] -d  <host> [pub]               <-Delete ARP entry\n"));
    fprintf(fp, _("  arp [-vnD] [<HW>] [-i <if>] -f  [<filename>]            <-Add entry from file\n"));
    fprintf(fp, _("  arp [-v]   [<HW>] [-i <if>] -s  <host> <hwaddr> [temp]            <-Add entry\n"));
    fprintf(fp, _("  arp [-v]   [<HW>] [-i <if>] -Ds <host> <if> [netmask <nm>] pub          <-''-\n\n"));

    fprintf(fp, _("        -a                       display (all) hosts in alternative (BSD) style\n"));
    fprintf(fp, _("        -e                       display (all) hosts in default (Linux) style\n"));
    fprintf(fp, _("        -s, --set                set a new ARP entry\n"));
    fprintf(fp, _("        -d, --delete             delete a specified entry\n"));
    fprintf(fp, _("        -v, --verbose            be verbose\n"));
    fprintf(fp, _("        -n, --numeric            don't resolve names\n"));
    fprintf(fp, _("        -i, --device             specify network interface (e.g. eth0)\n"));
    fprintf(fp, _("        -D, --use-device         read <hwaddr> from given device\n"));
    fprintf(fp, _("        -A, -p, --protocol       specify protocol family\n"));
    fprintf(fp, _("        -f, --file               read new entries from file or from /etc/ethers\n\n"));

    fprintf(fp, _("  <HW>=Use '-H <hw>' to specify hardware address type. Default: %s\n"), DFLT_HW);
    fprintf(fp, _("  List of possible hardware types (which support ARP):\n"));
    print_hwlist(1); /* 1 = ARPable */
    exit(rc);
}

int main(int argc, char **argv)
{
    int i, lop, what;
    struct option longopts[] =
    {
	{"verbose", 0, 0, 'v'},
	{"version", 0, 0, 'V'},
	{"all", 0, 0, 'a'},
	{"delete", 0, 0, 'd'},
	{"file", 0, 0, 'f'},
	{"numeric", 0, 0, 'n'},
	{"set", 0, 0, 's'},
	{"protocol", 1, 0, 'A'},
	{"hw-type", 1, 0, 'H'},
	{"device", 1, 0, 'i'},
	{"help", 0, 0, 'h'},
	{"use-device", 0, 0, 'D'},
	{"symbolic", 0, 0, 'N'},
	{NULL, 0, 0, 0}
    };

#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif

    /* Initialize variables... */
    if ((hw = get_hwtype(DFLT_HW)) == NULL) {
	fprintf(stderr, _("%s: hardware type not supported!\n"), DFLT_HW);
	return (-1);
    }
    if ((ap = get_aftype(DFLT_AF)) == NULL) {
	fprintf(stderr, _("%s: address family not supported!\n"), DFLT_AF);
	return (-1);
    }
    what = 0;

    /* Fetch the command-line arguments. */
    /* opterr = 0; */
    while ((i = getopt_long(argc, argv, "A:H:adfp:nsei:t:vh?DNV", longopts, &lop)) != EOF)
	switch (i) {
	case 'a':
	    what = 1;
	    opt_a = 1;
	    break;
	case 'f':
	    what = 2;
	    break;
	case 'd':
	    what = 3;
	    break;
	case 's':
	    what = 4;
	    break;


	case 'e':
	    opt_e = 1;
	    break;
	case 'n':
	    opt_n = FLAG_NUM;
	    break;
	case 'D':
	    opt_D = 1;
	    break;
	case 'N':
	    opt_N = FLAG_SYM;
	    fprintf(stderr, _("arp: -N not yet supported.\n"));
	    break;
	case 'v':
	    opt_v = 1;
	    break;

	case 'A':
	case 'p':
	    ap = get_aftype(optarg);
	    if (ap == NULL) {
		fprintf(stderr, _("arp: %s: unknown address family.\n"),
			optarg);
		exit(-1);
	    }
	    break;
	case 'H':
	case 't':
	    hw = get_hwtype(optarg);
	    if (hw == NULL) {
		fprintf(stderr, _("arp: %s: unknown hardware type.\n"),
			optarg);
		exit(-1);
	    }
	    hw_set = 1;
	    break;
	case 'i':
	    safe_strncpy(device, optarg, sizeof(device));
	    break;

	case 'V':
	    version();
	case 'h':
	    usage(E_USAGE);
	case '?':
	default:
	    usage(E_OPTERR);
	}

    if (ap->af != AF_INET) {
	fprintf(stderr, _("arp: %s: kernel only supports 'inet'.\n"),
		ap->name);
	exit(-1);
    }

    /* If not hw type specified get default */
    if(hw_set==0)
	if ((hw = get_hwtype(DFLT_HW)) == NULL) {
	  fprintf(stderr, _("%s: hardware type not supported!\n"), DFLT_HW);
	  return (-1);
	}

    if (hw->alen <= 0) {
	fprintf(stderr, _("arp: %s: hardware type without ARP support.\n"),
		hw->name);
	exit(-1);
    }
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("socket");
	exit(-1);
    }
    /* Now see what we have to do here... */
    switch (what) {
    case 0:
	opt_e = 1;
	what = arp_show(argv[optind]);
	break;

    case 1:			/* show an ARP entry in the cache */
	what = arp_show(argv[optind]);
	break;

    case 2:			/* process an EtherFile */
	what = arp_file(argv[optind] ? argv[optind] : "/etc/ethers");
	break;

    case 3:			/* delete an ARP entry from the cache */
	what = arp_del(&argv[optind]);
	break;

    case 4:			/* set an ARP entry in the cache */
	what = arp_set(&argv[optind]);
	break;

    default:
	usage(E_OPTERR);
    }

    exit(what);
}
