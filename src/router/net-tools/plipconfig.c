/*

   plipconfig.c: plip-ifconfig program for the Linux PLIP device driver
   Copyright (c) 1994 John Paul Morrison (VE7JPM).

   version 0.2
   
   Changed by Alan Cox, to reflect the way SIOCDEVPRIVATE is meant to work
   and for the extra parameter added by Niibe.

   plipconfig is a quick hack to set PLIP parameters by using driver
   ioctls.  plipconfig will no doubt be revised many times as the Linux
   PLIP driver and Linux 1.1 mutates.

*/

/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2, as
   published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_plip.h>

#include "config.h"
#include "intl.h"
#include "net-support.h"
#include "version.h"

int opt_a = 0;
int opt_i = 0;
int opt_v = 0;
int skfd = -1;

struct ifreq ifr;
struct plipconf *plip;

char *Release = RELEASE,
     *Version = "plipconfig 0.2",
     *Signature = "John Paul Morrison, Alan Cox et al.";

static void version(void)
{
    printf("%s\n%s\n%s\n", Release, Version, Signature);
    exit(E_VERSION);
}

void usage(void)
{
    fprintf(stderr, _("Usage: plipconfig [-a] [-i] [-v] interface\n"));
    fprintf(stderr, _("                  [nibble NN] [trigger NN]\n"));
    fprintf(stderr, _("       plipconfig -V | --version\n"));
    exit(-1);
}

void print_plip(void)
{
    printf(_("%s\tnibble %lu  trigger %lu\n"), ifr.ifr_name, plip->nibble, plip->trigger);
}

int main(int argc, char **argv)
{
    int ret = 0;
    char **spp;

#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("socket");
	exit(-1);
    }
    /* Find any options. */
    argc--;
    argv++;
    while (argv[0] && *argv[0] == '-') {
	if (!strcmp(*argv, "-a"))
	    opt_a = 1;
	if (!strcmp(*argv, "-v"))
	    opt_v = 1;
	if (!strcmp(*argv, "-V") || !strcmp(*argv, "--version"))
	    version();
	argv++;
	argc--;
    }

    if (argc == 0)
	usage();

    spp = argv;
    strncpy(ifr.ifr_name, *spp++, IFNAMSIZ);
    plip=(struct plipconf *)&ifr.ifr_data;

    plip->pcmd = PLIP_GET_TIMEOUT;	/* get current settings for device */
    if (ioctl(skfd, SIOCDEVPLIP, &ifr) < 0) {
	perror("ioctl");
	exit(-1);
    }
    if (*spp == (char *) NULL) {
	print_plip();
	(void) close(skfd);
	exit(0);
    }
    while (*spp != (char *) NULL) {
	if (!strcmp(*spp, "nibble")) {
	    if (*++spp == NULL)
		usage();
	    plip->nibble = atoi(*spp);
	    spp++;
	    continue;
	}
	if (!strcmp(*spp, "trigger")) {
	    if (*++spp == NULL)
		usage();
	    plip->trigger = atoi(*spp);
	    spp++;
	    continue;
	}
	usage();
    }

    plip->pcmd = PLIP_SET_TIMEOUT;
    if (ioctl(skfd, SIOCDEVPLIP, &ifr) < 0)
	perror("ioctl");

    print_plip();

    /* Close the socket. */
    (void) close(skfd);

    return (ret);
}
