/*
 * ifconfig   This file contains an implementation of the command
 *              that either displays or sets the characteristics of
 *              one or more of the system's networking interfaces.
 *
 * Version:     $Id: ifconfig.c,v 1.59 2011-01-01 03:22:31 ecki Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              and others.  Copyright 1993 MicroWalt Corporation
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 *
 * Patched to support 'add' and 'del' keywords for INET(4) addresses
 * by Mrs. Brisby <mrs.brisby@nimh.org>
 *
 * {1.34} - 19980630 - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 *                     - gettext instead of catgets for i18n
 *          10/1998  - Andi Kleen. Use interface list primitives.
 *	    20001008 - Bernd Eckenfels, Patch from RH for setting mtu
 *			(default AF was wrong)
 *          20010404 - Arnaldo Carvalho de Melo, use setlocale
 */

#define DFLT_AF "inet"

#include "config.h"

#include <features.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

/* Ugh.  But libc5 doesn't provide POSIX types.  */
#include <asm/types.h>


#if HAVE_HWSLIP
#include <linux/if_slip.h>
#endif

#if HAVE_AFINET6

#ifndef _LINUX_IN6_H
/*
 *    This is in linux/include/net/ipv6.h.
 */

struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __u32 ifr6_prefixlen;
    unsigned int ifr6_ifindex;
};

#endif

#endif				/* HAVE_AFINET6 */

#if HAVE_AFIPX
#if (__GLIBC__ > 2) || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 1)
#include <netipx/ipx.h>
#else
#include "ipx.h"
#endif
#endif
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "../intl.h"
#include "interface.h"
#include "sockets.h"
#include "util.h"

static char *Release = RELEASE;

int opt_a = 0;			/* show all interfaces          */
int opt_v = 0;			/* debugging output flag        */

int addr_family = 0;		/* currently selected AF        */

/* for ipv4 add/del modes */
static int get_nmbc_parent(char *parent, in_addr_t *nm, in_addr_t *bc);
static int set_ifstate(char *parent, in_addr_t ip, in_addr_t nm, in_addr_t bc,
		       int flag);

static int if_print(char *ifname)
{
    int res;

    if (ife_short)
	printf(_("Iface      MTU    RX-OK RX-ERR RX-DRP RX-OVR    TX-OK TX-ERR TX-DRP TX-OVR Flg\n"));

    if (!ifname) {
	res = for_all_interfaces(do_if_print, &opt_a);
    } else {
	struct interface *ife;

	ife = lookup_interface(ifname);
	if (!ife) {
		return -1;
	}
	res = do_if_fetch(ife);
	if (res >= 0)
	    ife_print(ife);
    }
    return res;
}

/* Set a certain interface flag. */
static int set_flag(char *ifname, short flag)
{
    struct ifreq ifr;

    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
	fprintf(stderr, _("%s: ERROR while getting interface flags: %s\n"),
		ifname,	strerror(errno));
	return (-1);
    }
    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags |= flag;
    if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
	perror("SIOCSIFFLAGS");
	return -1;
    }
    return (0);
}

/* Clear a certain interface flag. */
static int clr_flag(char *ifname, short flag)
{
    struct ifreq ifr;
    int fd;

    if (strchr(ifname, ':')) {
        /* This is a v4 alias interface.  Downing it via a socket for
	   another AF may have bad consequences. */
        fd = get_socket_for_af(AF_INET);
	if (fd < 0) {
	    fprintf(stderr, _("No support for INET on this system.\n"));
	    return -1;
	}
    } else
        fd = skfd;

    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
	fprintf(stderr, _("%s: ERROR while getting interface flags: %s\n"),
		ifname, strerror(errno));
	return -1;
    }
    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ifr.ifr_flags &= ~flag;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
	perror("SIOCSIFFLAGS");
	return -1;
    }
    return (0);
}

/** test is a specified flag is set */
static int test_flag(char *ifname, short flags)
{
    struct ifreq ifr;
    int fd;

    if (strchr(ifname, ':')) {
        /* This is a v4 alias interface.  Downing it via a socket for
	   another AF may have bad consequences. */
        fd = get_socket_for_af(AF_INET);
	if (fd < 0) {
	    fprintf(stderr, _("No support for INET on this system.\n"));
	    return -1;
	}
    } else
        fd = skfd;

    safe_strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
	fprintf(stderr, _("%s: ERROR while testing interface flags: %s\n"),
		ifname, strerror(errno));
	return -1;
    }
    return (ifr.ifr_flags & flags);
}

static void usage(int rc)
{
    FILE *fp = rc ? stderr : stdout;
    fprintf(fp, _("Usage:\n  ifconfig [-a] [-v] [-s] <interface> [[<AF>] <address>]\n"));
#if HAVE_AFINET
    fprintf(fp, _("  [add <address>[/<prefixlen>]]\n"));
    fprintf(fp, _("  [del <address>[/<prefixlen>]]\n"));
    fprintf(fp, _("  [[-]broadcast [<address>]]  [[-]pointopoint [<address>]]\n"));
    fprintf(fp, _("  [netmask <address>]  [dstaddr <address>]  [tunnel <address>]\n"));
#endif
#ifdef SIOCSKEEPALIVE
    fprintf(fp, _("  [outfill <NN>] [keepalive <NN>]\n"));
#endif
    fprintf(fp, _("  [hw <HW> <address>]  [mtu <NN>]\n"));
    fprintf(fp, _("  [[-]trailers]  [[-]arp]  [[-]allmulti]\n"));
    fprintf(fp, _("  [multicast]  [[-]promisc]\n"));
    fprintf(fp, _("  [mem_start <NN>]  [io_addr <NN>]  [irq <NN>]  [media <type>]\n"));
#ifdef HAVE_TXQUEUELEN
    fprintf(fp, _("  [txqueuelen <NN>]\n"));
#endif
#ifdef SIOCSIFNAME
    fprintf(fp, _("  [name <newname>]\n"));
#endif
#ifdef HAVE_DYNAMIC
    fprintf(fp, _("  [[-]dynamic]\n"));
#endif
    fprintf(fp, _("  [up|down] ...\n\n"));

    fprintf(fp, _("  <HW>=Hardware Type.\n"));
    fprintf(fp, _("  List of possible hardware types:\n"));
    print_hwlist(0); /* 1 = ARPable */
    fprintf(fp, _("  <AF>=Address family. Default: %s\n"), DFLT_AF);
    fprintf(fp, _("  List of possible address families:\n"));
    print_aflist(0); /* 1 = routeable */
    exit(rc);
}

static void version(void)
{
    printf("%s\n", Release);
    exit(E_VERSION);
}

static int set_netmask(int skfd, struct ifreq *ifr, struct sockaddr *sa)
{
    int err = 0;

    memcpy(&ifr->ifr_netmask, sa, sizeof(struct sockaddr));
    if (ioctl(skfd, SIOCSIFNETMASK, ifr) < 0) {
	fprintf(stderr, "SIOCSIFNETMASK: %s\n",
		strerror(errno));
	err = 1;
    }
    return err;
}

int main(int argc, char **argv)
{
    struct sockaddr_storage _sa, _samask;
    struct sockaddr *sa = (struct sockaddr *)&_sa;
    struct sockaddr *samask = (struct sockaddr *)&_samask;
    struct sockaddr_in *sin = (struct sockaddr_in *)&_sa;
    char host[128];
    const struct aftype *ap;
    const struct hwtype *hw;
    struct ifreq ifr;
    int goterr = 0, didnetmask = 0, neednetmask=0;
    char **spp;
    int fd;
#if HAVE_AFINET6
    extern struct aftype inet6_aftype;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&_sa;
    struct in6_ifreq ifr6;
    unsigned long prefix_len;
    char *cp;
#endif
#if HAVE_AFINET
    extern struct aftype inet_aftype;
#endif

#if I18N
    setlocale(LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif

    /* Find any options. */
    argc--;
    argv++;
    while (argc && *argv[0] == '-') {
	if (!strcmp(*argv, "-a"))
	    opt_a = 1;

	else if (!strcmp(*argv, "-s"))
	    ife_short = 1;

	else if (!strcmp(*argv, "-v"))
	    opt_v = 1;

	else if (!strcmp(*argv, "-V") || !strcmp(*argv, "-version") ||
	    !strcmp(*argv, "--version"))
	    version();

	else if (!strcmp(*argv, "-?") || !strcmp(*argv, "-h") ||
	    !strcmp(*argv, "-help") || !strcmp(*argv, "--help"))
	    usage(E_USAGE);

	else {
	    fprintf(stderr, _("ifconfig: option `%s' not recognised.\n"),
		    argv[0]);
	    fprintf(stderr, _("ifconfig: `--help' gives usage information.\n"));
	    exit(1);
	}

	argv++;
	argc--;
    }

    /* Create a channel to the NET kernel. */
    if ((skfd = sockets_open(0)) < 0) {
	perror("socket");
	exit(1);
    }

    /* Do we have to show the current setup? */
    if (argc == 0) {
	int err = if_print((char *) NULL);
	(void) close(skfd);
	exit(err < 0);
    }
    /* No. Fetch the interface name. */
    spp = argv;
    safe_strncpy(ifr.ifr_name, *spp++, IFNAMSIZ);
    if (*spp == (char *) NULL) {
	int err = if_print(ifr.ifr_name);
	(void) close(skfd);
	exit(err < 0);
    }

    /* The next argument is either an address family name, or an option. */
    if ((ap = get_aftype(*spp)) != NULL)
	spp++; /* it was a AF name */
    else
	ap = get_aftype(DFLT_AF);

    if (ap) {
	addr_family = ap->af;
	skfd = ap->fd;
    }

    /* Process the remaining arguments. */
    while (*spp != (char *) NULL) {
	if (!strcmp(*spp, "arp")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_NOARP);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-arp")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_NOARP);
	    spp++;
	    continue;
	}
#ifdef IFF_PORTSEL
	if (!strcmp(*spp, "media") || !strcmp(*spp, "port")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    if (!strcasecmp(*spp, "auto")) {
		goterr |= set_flag(ifr.ifr_name, IFF_AUTOMEDIA);
	    } else {
		int i, j, newport;
		char *endp;
		newport = strtol(*spp, &endp, 10);
		if (*endp != 0) {
		    newport = -1;
		    for (i = 0; if_port_text[i][0] && newport == -1; i++) {
			for (j = 0; if_port_text[i][j]; j++) {
			    if (!strcasecmp(*spp, if_port_text[i][j])) {
				newport = i;
				break;
			    }
			}
		    }
		}
		spp++;
		if (newport == -1) {
		    fprintf(stderr, _("Unknown media type.\n"));
		    goterr = 1;
		} else {
		    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
			perror("port: SIOCGIFMAP");
			goterr = 1;
			continue;
		    }
		    ifr.ifr_map.port = newport;
		    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
			perror("port: SIOCSIFMAP");
			goterr = 1;
		    }
		}
	    }
	    continue;
	}
#endif

	if (!strcmp(*spp, "trailers")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_NOTRAILERS);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-trailers")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_NOTRAILERS);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "promisc")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_PROMISC);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-promisc")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_PROMISC);
	    if (test_flag(ifr.ifr_name, IFF_PROMISC) > 0)
	    	fprintf(stderr, _("Warning: Interface %s still in promisc mode... maybe other application is running?\n"), ifr.ifr_name);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "multicast")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_MULTICAST);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-multicast")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_MULTICAST);
	    if (test_flag(ifr.ifr_name, IFF_MULTICAST) > 0)
	    	fprintf(stderr, _("Warning: Interface %s still in MULTICAST mode.\n"), ifr.ifr_name);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "allmulti")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_ALLMULTI);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-allmulti")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_ALLMULTI);
	    if (test_flag(ifr.ifr_name, IFF_ALLMULTI) > 0)
	    	fprintf(stderr, _("Warning: Interface %s still in ALLMULTI mode.\n"), ifr.ifr_name);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "up")) {
	    goterr |= set_flag(ifr.ifr_name, (IFF_UP | IFF_RUNNING));
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "down")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_UP);
	    spp++;
	    continue;
	}
#ifdef HAVE_DYNAMIC
	if (!strcmp(*spp, "dynamic")) {
	    goterr |= set_flag(ifr.ifr_name, IFF_DYNAMIC);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-dynamic")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_DYNAMIC);
	    spp++;
	    if (test_flag(ifr.ifr_name, IFF_DYNAMIC) > 0)
	    	fprintf(stderr, _("Warning: Interface %s still in DYNAMIC mode.\n"), ifr.ifr_name);
	    continue;
	}
#endif

	if (!strcmp(*spp, "mtu")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    ifr.ifr_mtu = atoi(*spp);
	    if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFMTU: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#ifdef SIOCSKEEPALIVE
	if (!strcmp(*spp, "keepalive")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    ifr.ifr_data = (caddr_t) (uintptr_t) atoi(*spp);
	    if (ioctl(skfd, SIOCSKEEPALIVE, &ifr) < 0) {
		fprintf(stderr, "SIOCSKEEPALIVE: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

#ifdef SIOCSOUTFILL
	if (!strcmp(*spp, "outfill")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    ifr.ifr_data = (caddr_t) (uintptr_t) atoi(*spp);
	    if (ioctl(skfd, SIOCSOUTFILL, &ifr) < 0) {
		fprintf(stderr, "SIOCSOUTFILL: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

	if (!strcmp(*spp, "-broadcast")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_BROADCAST);
	    if (test_flag(ifr.ifr_name, IFF_BROADCAST) > 0)
	    	fprintf(stderr, _("Warning: Interface %s still in BROADCAST mode.\n"), ifr.ifr_name);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "broadcast")) {
	    if (*++spp != NULL) {
		safe_strncpy(host, *spp, (sizeof host));
		if (ap->input(0, host, &_sa) < 0) {
		    if (ap->herror)
		    	ap->herror(host);
		    else
		    	fprintf(stderr, _("ifconfig: Error resolving '%s' for broadcast\n"), host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		memcpy(&ifr.ifr_broadaddr, sa, sizeof(struct sockaddr));
		if (ioctl(ap->fd, SIOCSIFBRDADDR, &ifr) < 0) {
		    fprintf(stderr, "SIOCSIFBRDADDR: %s\n",
			    strerror(errno));
		    goterr = 1;
		}
		spp++;
	    }
	    goterr |= set_flag(ifr.ifr_name, IFF_BROADCAST);
	    continue;
	}
	if (!strcmp(*spp, "dstaddr")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    safe_strncpy(host, *spp, (sizeof host));
	    if (ap->input(0, host, &_sa) < 0) {
		    if (ap->herror)
		    	ap->herror(host);
		    else
		    	fprintf(stderr, _("ifconfig: Error resolving '%s' for dstaddr\n"), host);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy(&ifr.ifr_dstaddr, sa, sizeof(struct sockaddr));
	    if (ioctl(ap->fd, SIOCSIFDSTADDR, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
			strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "netmask")) {
	    if (*++spp == NULL || didnetmask)
		usage(E_OPTERR);
	    safe_strncpy(host, *spp, (sizeof host));
	    if (ap->input(0, host, &_sa) < 0) {
		    if (ap->herror)
		    	ap->herror(host);
		    else
		    	fprintf(stderr, _("ifconfig: Error resolving '%s' for netmask\n"), host);
		goterr = 1;
		spp++;
		continue;
	    }
	    didnetmask++;
	    goterr |= set_netmask(ap->fd, &ifr, sa);
	    spp++;
	    continue;
	}
#ifdef HAVE_TXQUEUELEN
	if (!strcmp(*spp, "txqueuelen")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    ifr.ifr_qlen = strtoul(*spp, NULL, 0);
	    if (ioctl(skfd, SIOCSIFTXQLEN, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFTXQLEN: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

#ifdef SIOCSIFNAME
	if (!strcmp(*spp, "name")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    safe_strncpy(ifr.ifr_newname, *spp, IFNAMSIZ);
	    if (ioctl(skfd, SIOCSIFNAME, &ifr) < 0) {
		fprintf(stderr, "SIOCSIFNAME: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

	if (!strcmp(*spp, "mem_start")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
		fprintf(stderr, "mem_start: SIOCGIFMAP: %s\n", strerror(errno));
		spp++;
		goterr = 1;
		continue;
	    }
	    ifr.ifr_map.mem_start = strtoul(*spp, NULL, 0);
	    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
		fprintf(stderr, "mem_start: SIOCSIFMAP: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "io_addr")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
		fprintf(stderr, "io_addr: SIOCGIFMAP: %s\n", strerror(errno));
		spp++;
		goterr = 1;
		continue;
	    }
	    ifr.ifr_map.base_addr = strtol(*spp, NULL, 0);
	    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
		fprintf(stderr, "io_addr: SIOCSIFMAP: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "irq")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    if (ioctl(skfd, SIOCGIFMAP, &ifr) < 0) {
		fprintf(stderr, "irq: SIOCGIFMAP: %s\n", strerror(errno));
		goterr = 1;
		spp++;
		continue;
	    }
	    ifr.ifr_map.irq = atoi(*spp);
	    if (ioctl(skfd, SIOCSIFMAP, &ifr) < 0) {
		fprintf(stderr, "irq: SIOCSIFMAP: %s\n", strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "-pointopoint") || !strcmp(*spp, "-pointtopoint")) {
	    goterr |= clr_flag(ifr.ifr_name, IFF_POINTOPOINT);
	    spp++;
	    if (test_flag(ifr.ifr_name, IFF_POINTOPOINT) > 0)
	    	fprintf(stderr, _("Warning: Interface %s still in POINTOPOINT mode.\n"), ifr.ifr_name);
	    continue;
	}
	if (!strcmp(*spp, "pointopoint") || !strcmp(*spp, "pointtopoint")) {
	    if (*(spp + 1) != NULL) {
		spp++;
		safe_strncpy(host, *spp, (sizeof host));
		if (ap->input(0, host, &_sa)) {
		    if (ap->herror)
		    	ap->herror(host);
		    else
		    	fprintf(stderr, _("ifconfig: Error resolving '%s' for pointopoint\n"), host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		memcpy(&ifr.ifr_dstaddr, sa, sizeof(struct sockaddr));
		if (ioctl(ap->fd, SIOCSIFDSTADDR, &ifr) < 0) {
		    fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
			    strerror(errno));
		    goterr = 1;
		}
	    }
	    goterr |= set_flag(ifr.ifr_name, IFF_POINTOPOINT);
	    spp++;
	    continue;
	};

	if (!strcmp(*spp, "hw")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    if ((hw = get_hwtype(*spp)) == NULL)
		usage(E_OPTERR);
	    if (hw->input == NULL) {
	    	fprintf(stderr, _("hw address type `%s' has no handler to set address. failed.\n"), *spp);
	    	spp+=2;
	    	goterr = 1;
	    	continue;
	    }
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    safe_strncpy(host, *spp, (sizeof host));
	    if (hw->input(host, &_sa) < 0) {
		fprintf(stderr, _("%s: invalid %s address.\n"), host, hw->name);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy(&ifr.ifr_hwaddr, sa, sizeof(struct sockaddr));
	    if (ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0) {
		if (errno == EBUSY)
			fprintf(stderr, "SIOCSIFHWADDR: %s - you may need to down the interface\n",
				strerror(errno));
		else
			fprintf(stderr, "SIOCSIFHWADDR: %s\n",
				strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#if HAVE_AFINET || HAVE_AFINET6
	if (!strcmp(*spp, "add")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
#if HAVE_AFINET6
	    if (strchr(*spp, ':')) {
		/* INET6 */
		if ((cp = strchr(*spp, '/'))) {
		    prefix_len = atol(cp + 1);
		    if ((prefix_len < 0) || (prefix_len > 128))
			usage(E_OPTERR);
		    *cp = 0;
		} else {
		    prefix_len = 128;
		}
		safe_strncpy(host, *spp, (sizeof host));
		if (inet6_aftype.input(1, host, &_sa) < 0) {
		    if (inet6_aftype.herror)
		    	inet6_aftype.herror(host);
		    else
		    	fprintf(stderr, _("ifconfig: Error resolving '%s' for add\n"), host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		memcpy(&ifr6.ifr6_addr, &sin6->sin6_addr, sizeof(struct in6_addr));

		fd = get_socket_for_af(AF_INET6);
		if (fd < 0) {
		    fprintf(stderr,
			    _("No support for INET6 on this system.\n"));
		    goterr = 1;
		    spp++;
		    continue;
		}
		if (ioctl(fd, SIOGIFINDEX, &ifr) < 0) {
		    perror("SIOGIFINDEX");
		    goterr = 1;
		    spp++;
		    continue;
		}
		ifr6.ifr6_ifindex = ifr.ifr_ifindex;
		ifr6.ifr6_prefixlen = prefix_len;
		if (ioctl(fd, SIOCSIFADDR, &ifr6) < 0) {
		    perror("SIOCSIFADDR");
		    goterr = 1;
		}
		spp++;
		continue;
	    }
#endif
#if HAVE_AFINET
	    { /* ipv4 address a.b.c.d */
		in_addr_t ip, nm, bc;
		safe_strncpy(host, *spp, (sizeof host));
		if (inet_aftype.input(0, host, &_sa) < 0) {
		    ap->herror(host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		fd = get_socket_for_af(AF_INET);
		if (fd < 0) {
		    fprintf(stderr,
			    _("No support for INET on this system.\n"));
		    goterr = 1;
		    spp++;
		    continue;
		}

		memcpy(&ip, &sin->sin_addr.s_addr, sizeof(ip));

		if (get_nmbc_parent(ifr.ifr_name, &nm, &bc) < 0) {
			fprintf(stderr, _("Interface %s not initialized\n"),
				ifr.ifr_name);
			goterr = 1;
			spp++;
			continue;
		}
		set_ifstate(ifr.ifr_name, ip, nm, bc, 1);

	    }
	    spp++;
	    continue;
#else
	    fprintf(stderr, _("Bad address.\n"));
#endif
	}
#endif

#if HAVE_AFINET || HAVE_AFINET6
	if (!strcmp(*spp, "del")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);

#ifdef SIOCDIFADDR
#if HAVE_AFINET6
	    if (strchr(*spp, ':')) {	/* INET6 */
		if ((cp = strchr(*spp, '/'))) {
		    prefix_len = atol(cp + 1);
		    if ((prefix_len < 0) || (prefix_len > 128))
			usage(E_OPTERR);
		    *cp = 0;
		} else {
		    prefix_len = 128;
		}
		safe_strncpy(host, *spp, (sizeof host));
		if (inet6_aftype.input(1, host, &_sa) < 0) {
		    inet6_aftype.herror(host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		memcpy(&ifr6.ifr6_addr, &sin6->sin6_addr,
		       sizeof(struct in6_addr));

		fd = get_socket_for_af(AF_INET6);
		if (fd < 0) {
		    fprintf(stderr,
			    _("No support for INET6 on this system.\n"));
		    goterr = 1;
		    spp++;
		    continue;
		}
		if (ioctl(fd, SIOGIFINDEX, &ifr) < 0) {
		    perror("SIOGIFINDEX");
		    goterr = 1;
		    spp++;
		    continue;
		}
		ifr6.ifr6_ifindex = ifr.ifr_ifindex;
		ifr6.ifr6_prefixlen = prefix_len;
		if (opt_v)
			fprintf(stderr, "now deleting: ioctl(SIOCDIFADDR,{ifindex=%d,prefixlen=%ld})\n",ifr.ifr_ifindex,prefix_len);
		if (ioctl(fd, SIOCDIFADDR, &ifr6) < 0) {
		    fprintf(stderr, "SIOCDIFADDR: %s\n",
			    strerror(errno));
		    goterr = 1;
		}
		spp++;
		continue;
	    }
#endif
#if HAVE_AFINET
	    {
		/* ipv4 address a.b.c.d */
		in_addr_t ip, nm, bc;
		safe_strncpy(host, *spp, (sizeof host));
		if (inet_aftype.input(0, host, &_sa) < 0) {
		    ap->herror(host);
		    goterr = 1;
		    spp++;
		    continue;
		}
		fd = get_socket_for_af(AF_INET);
		if (fd < 0) {
		    fprintf(stderr, _("No support for INET on this system.\n"));
		    goterr = 1;
		    spp++;
		    continue;
		}

		/* Clear "ip" in case sizeof(unsigned long) > sizeof(sin.sin_addr.s_addr) */
		ip = 0;
		memcpy(&ip, &sin->sin_addr.s_addr, sizeof(ip));

		if (get_nmbc_parent(ifr.ifr_name, &nm, &bc) < 0) {
		    fprintf(stderr, _("Interface %s not initialized\n"),
			    ifr.ifr_name);
		    goterr = 1;
		    spp++;
		    continue;
		}
		set_ifstate(ifr.ifr_name, ip, nm, bc, 0);
	    }
	    spp++;
	    continue;
#else
	    fprintf(stderr, _("Bad address.\n"));
#endif
#else
	    fprintf(stderr, _("Address deletion not supported on this system.\n"));
#endif
	}
#endif
#if HAVE_AFINET6
	if (!strcmp(*spp, "tunnel")) {
	    if (*++spp == NULL)
		usage(E_OPTERR);
	    if ((cp = strchr(*spp, '/'))) {
		prefix_len = atol(cp + 1);
		if ((prefix_len < 0) || (prefix_len > 128))
		    usage(E_OPTERR);
		*cp = 0;
	    } else {
		prefix_len = 128;
	    }
	    safe_strncpy(host, *spp, (sizeof host));
	    if (inet6_aftype.input(1, host, &_sa) < 0) {
		inet6_aftype.herror(host);
		goterr = 1;
		spp++;
		continue;
	    }
	    memcpy(&ifr6.ifr6_addr, &sin6->sin6_addr, sizeof(struct in6_addr));

	    fd = get_socket_for_af(AF_INET6);
	    if (fd < 0) {
		fprintf(stderr, _("No support for INET6 on this system.\n"));
		goterr = 1;
		spp++;
		continue;
	    }
	    if (ioctl(fd, SIOGIFINDEX, &ifr) < 0) {
		perror("SIOGIFINDEX");
		goterr = 1;
		spp++;
		continue;
	    }
	    ifr6.ifr6_ifindex = ifr.ifr_ifindex;
	    ifr6.ifr6_prefixlen = prefix_len;

	    if (ioctl(fd, SIOCSIFDSTADDR, &ifr6) < 0) {
		fprintf(stderr, "SIOCSIFDSTADDR: %s\n",
			strerror(errno));
		goterr = 1;
	    }
	    spp++;
	    continue;
	}
#endif

	/* If the next argument is a valid hostname, assume OK. */
	safe_strncpy(host, *spp, (sizeof host));

	/* FIXME: sa is too small for INET6 addresses, inet6 should use that too,
	   broadcast is unexpected */
	if (ap->getmask) {
	    switch (ap->getmask(host, &_samask, NULL)) {
	    case -1:
		usage(E_OPTERR);
		break;
	    case 1:
		if (didnetmask)
		    usage(E_OPTERR);

		// remeber to set the netmask from samask later
		neednetmask = 1;
		break;
	    }
	}
	if (ap->input == NULL) {
	   fprintf(stderr, _("ifconfig: Cannot set address for this protocol family.\n"));
	   exit(1);
	}
	if (ap->input(0, host, &_sa) < 0) {
	    if (ap->herror)
	    	ap->herror(host);
	    else
	    	fprintf(stderr,_("ifconfig: error resolving '%s' to set address for af=%s\n"), host, ap->name);
	    fprintf(stderr, _("ifconfig: `--help' gives usage information.\n"));
	    exit(1);
	}
	memcpy(&ifr.ifr_addr, sa, sizeof(struct sockaddr));
	{
	    int r = 0;		/* to shut gcc up */
	    switch (ap->af) {
#if HAVE_AFINET
	    case AF_INET:
		fd = get_socket_for_af(AF_INET);
		if (fd < 0) {
		    fprintf(stderr, _("No support for INET on this system.\n"));
		    exit(1);
		}
		r = ioctl(fd, SIOCSIFADDR, &ifr);
		break;
#endif
#if HAVE_AFECONET
	    case AF_ECONET:
		fd = get_socket_for_af(AF_ECONET);
		if (fd < 0) {
		    fprintf(stderr, _("No support for ECONET on this system.\n"));
		    exit(1);
		}
		r = ioctl(fd, SIOCSIFADDR, &ifr);
		break;
#endif
	    default:
		fprintf(stderr,
		_("Don't know how to set addresses for family %d.\n"), ap->af);
		exit(1);
	    }
	    if (r < 0) {
		perror("SIOCSIFADDR");
		goterr = 1;
	    }
	}

       /*
        * Don't do the set_flag() if the address is an alias with a - at the
        * end, since it's deleted already! - Roman
        * Same goes if they used address 0.0.0.0 as the kernel uses this to
        * destroy aliases.
        *
        * Should really use regex.h here, not sure though how well it'll go
        * with the cross-platform support etc.
        */
        {
            char *ptr;
            short int found_colon = 0;
            short int bring_up = 1;
            for (ptr = ifr.ifr_name; *ptr; ptr++ )
                if (*ptr == ':') found_colon++;

            if (found_colon) {
                if (ptr[-1] == '-')
                    bring_up = 0;
                else if (ap->af == AF_INET && sin->sin_addr.s_addr == 0)
                    bring_up = 0;
            }

            if (bring_up)
                goterr |= set_flag(ifr.ifr_name, (IFF_UP | IFF_RUNNING));
        }

	spp++;
    }

    if (neednetmask) {
	goterr |= set_netmask(skfd, &ifr, samask);
	didnetmask++;
    }

    if (opt_v && goterr)
    	fprintf(stderr, _("WARNING: at least one error occured. (%d)\n"), goterr);

    return (goterr);
}

struct ifcmd {
    int flag;
    unsigned long addr;
    char *base;
    int baselen;
};

static unsigned char searcher[256];

static int set_ip_using(const char *name, int c, unsigned long ip)
{
    struct ifreq ifr;
    struct sockaddr_in sin;

    safe_strncpy(ifr.ifr_name, name, IFNAMSIZ);
    memset(&sin, 0, sizeof(struct sockaddr));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ip;
    memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    if (ioctl(skfd, c, &ifr) < 0)
	return -1;
    return 0;
}

static int do_ifcmd(struct interface *x, struct ifcmd *ptr)
{
    char *z, *e;
    struct sockaddr_in *sin;
    int i;

    if (do_if_fetch(x) < 0)
	return 0;
    if (strncmp(x->name, ptr->base, ptr->baselen) != 0)
	return 0; /* skip */
    z = strchr(x->name, ':');
    if (!z || !*z)
	return 0;
    z++;
    for (e = z; *e; e++)
	if (*e == '-') /* deleted */
	    return 0;
    i = atoi(z);
    if (i < 0 || i > 255)
	abort();
    searcher[i] = 1;

    /* copy */
    sin = (struct sockaddr_in *)&x->dstaddr_sas;
    if (sin->sin_addr.s_addr != ptr->addr) {
	return 0;
    }

    if (ptr->flag) {
	/* turn UP */
	if (set_flag(x->name, IFF_UP | IFF_RUNNING) == -1)
	    return -1;
    } else {
	/* turn DOWN */
	if (clr_flag(x->name, IFF_UP) == -1)
	    return -1;
    }

    return 1; /* all done! */
}


static int get_nmbc_parent(char *parent,
			   in_addr_t *nm, in_addr_t *bc)
{
    struct interface *i;
    struct sockaddr_in *sin;

    i = lookup_interface(parent);
    if (!i)
	return -1;
    if (do_if_fetch(i) < 0)
	return 0;
    sin = (struct sockaddr_in *)&i->netmask_sas;
    memcpy(nm, &sin->sin_addr.s_addr, sizeof(*nm));
    sin = (struct sockaddr_in *)&i->broadaddr_sas;
    memcpy(bc, &sin->sin_addr.s_addr, sizeof(*bc));
    return 0;
}

static int set_ifstate(char *parent, in_addr_t ip, in_addr_t nm, in_addr_t bc,
		       int flag)
{
    char buf[IFNAMSIZ];
    struct ifcmd pt;
    int i;

    pt.base = parent;
    pt.baselen = strlen(parent);
    pt.addr = ip;
    pt.flag = flag;
    memset(searcher, 0, sizeof(searcher));
    i = for_all_interfaces((int (*)(struct interface *,void *))do_ifcmd,
			   &pt);
    if (i == -1)
	return -1;
    if (i == 1)
	return 0;

    /* add a new interface */
    for (i = 0; i < 256; i++)
	if (searcher[i] == 0)
	    break;

    if (i == 256)
	return -1; /* FAILURE!!! out of ip addresses */

    if (snprintf(buf, IFNAMSIZ, "%s:%d", parent, i) > IFNAMSIZ)
	return -1;
    if (set_ip_using(buf, SIOCSIFADDR, ip) == -1)
	return -1;
    if (set_ip_using(buf, SIOCSIFNETMASK, nm) == -1)
	return -1;
    if (set_ip_using(buf, SIOCSIFBRDADDR, bc) == -1)
	return -1;
    if (set_flag(buf, IFF_BROADCAST) == -1)
	return -1;
    return 0;
}
