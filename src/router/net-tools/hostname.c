/*
 * hostname   This file contains an implementation of the command
 *              that maintains the hostname and the domainname. It
 *              is also used to show the FQDN and the IP-Addresses.
 *
 * Usage:       hostname [-d|-f|-s|-a|-i|-y|-n]
 *              hostname [-h|-V]
 *              hostname {name|-F file}
 *              dnsdmoainname
 *              nisdomainname {name|-F file}
 *
 * Version:     hostname 1.101 (2003-10-11)
 *
 * Author:      Peter Tobias <tobias@et-inf.fho-emden.de>
 *
 * Changes:
 *         {1.90}  Peter Tobias : Added -a and -i options.
 *         {1.91}  Bernd Eckenfels : -v,-V rewritten, long_opts (major rewrite), usage.
 *19960120 {1.95}  Bernd Eckenfels : -y/nisdomainname - support for get/setdomainname added
 *19960218 {1.96}  Bernd Eckenfels : netinet/in.h added
 *19980629 {1.97}  Arnaldo Carvalho de Melo : gettext instead of catgets for i18n
 *20000213 {1.99}  Arnaldo Carvalho de Melo : fixed some i18n strings
 *20010404 {1.100} Arnaldo Carvalho de Melo: use setlocale
 *20031011 {1.101} Maik Broemme: gcc 3.x fixes (default: break)
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "version.h"
#include "net-support.h"
#include "../intl.h"

#if HAVE_AFINET6
#include <sys/socket.h> /* for PF_INET6 */
#include <sys/types.h>  /* for inet_ntop */
#endif

#if HAVE_AFDECnet
#include <netdnet/dn.h>
#endif

static char *Release = RELEASE;

static char *program_name;
static int opt_v;

#define SETHOST		1
#define SETDOMAIN	2
#define SETNODE		3

#if HAVE_AFDECnet
static void setnname(const char *nname)
{
    if (opt_v)
        fprintf(stderr, _("Setting nodename to `%s'\n"),
                nname);
    if (setnodename(nname, strlen(nname))) {
        switch(errno) {
        case EPERM:
            fprintf(stderr, _("%s: you don't have permission to set the node name\n"), program_name);
            break;
        case EINVAL:
            fprintf(stderr, _("%s: name too long\n"), program_name);
            break;
        default:
	    perror(program_name);
	    break;
        }
	exit(1);
    }
}
#endif /* HAVE_AFDECnet */

static void sethname(const char *hname)
{
    if (opt_v)
	fprintf(stderr, _("Setting hostname to `%s'\n"),
		hname);
    if (sethostname(hname, strlen(hname))) {
	switch (errno) {
	case EPERM:
	    fprintf(stderr, _("%s: you don't have permission to set the host name\n"), program_name);
	    break;
	case EINVAL:
	    fprintf(stderr, _("%s: name too long\n"), program_name);
	    break;
	default:
	    perror(program_name);
	    break;
	}
	exit(1);
    }
}

static void setdname(const char *dname)
{
    if (opt_v)
	fprintf(stderr, _("Setting domainname to `%s'\n"),
		dname);
    if (setdomainname(dname, strlen(dname))) {
	switch (errno) {
	case EPERM:
	    fprintf(stderr, _("%s: you don't have permission to set the domain name\n"), program_name);
	    break;
	case EINVAL:
	    fprintf(stderr, _("%s: name too long\n"), program_name);
	    break;
	default:
	    perror(program_name);
	    break;
	}
	exit(1);
    }
}

static void showhname(const char *hname, int c)
{
    struct hostent *hp;
    char *p, **alias;
#if HAVE_AFINET6
    char addr[INET6_ADDRSTRLEN + 1];
#else
    char addr[INET_ADDRSTRLEN + 1];
#endif
    socklen_t len;
    char **addrp;
    bool isfirst = true;

    /* We use -1 so we can guarantee the buffer is NUL terminated. */
    len = sizeof(addr) - 1;
    addr[len] = '\0';

    if (opt_v)
	fprintf(stderr, _("Resolving `%s' ...\n"), hname);
    if (
#if HAVE_AFINET6
        !(hp = gethostbyname2(hname, PF_INET6)) &&
#endif
        !(hp = gethostbyname(hname))) {
	herror(program_name);
	exit(1);
    }

    if (opt_v) {
	fprintf(stderr, _("Result: h_name=`%s'\n"),
		hp->h_name);

	alias = hp->h_aliases;
	while (alias[0])
	    fprintf(stderr, _("Result: h_aliases=`%s'\n"),
		    *alias++);

	for (addrp = hp->h_addr_list; *addrp; ++addrp) {
	    if (inet_ntop(hp->h_addrtype, *addrp, addr, len))
		fprintf(stderr, _("Result: h_addr_list=`%s'\n"), addr);
	    else if (errno == EAFNOSUPPORT)
		fprintf(stderr, _("%s: protocol family not supported\n"),
			program_name);
	    else if (errno == ENOSPC)
		fprintf(stderr, _("%s: name too long\n"), program_name);
	}
    }
    if (!(p = strchr(hp->h_name, '.')) && (c == 'd'))
	return;

    switch (c) {
    case 'a':
	while (hp->h_aliases[0]) {
	    if (isfirst) {
		printf("%s", *hp->h_aliases++);
		isfirst = false;
	    } else
	        printf(" %s", *hp->h_aliases++);
	}
	printf("\n");
	break;
    case 'i':
	for (addrp = hp->h_addr_list; *addrp; ++addrp) {
	    if (inet_ntop(hp->h_addrtype, *addrp, addr, len)) {
		if (isfirst) {
		    printf("%s", addr);
		    isfirst = false;
		} else
		    printf(" %s", addr);
	    }
	    else if (errno == EAFNOSUPPORT)
		fprintf(stderr, _("%s: protocol family not supported\n"),
			program_name);
	    else if (errno == ENOSPC)
		fprintf(stderr, _("%s: name too long\n"), program_name);
	}
	printf("\n");
	break;
    case 'd':
	printf("%s\n", ++p);
	break;
    case 'f':
	printf("%s\n", hp->h_name);
	break;
    }
}

static void setfilename(const char *name, int what)
{
    FILE *fd;
    char *p;
    char fline[MAXHOSTNAMELEN];

    if ((fd = fopen(name, "r")) != NULL) {
	while (fgets(fline, sizeof(fline), fd) != NULL) {
	    if ((p = index(fline, '\n')) != NULL)
		*p = '\0';
	    if (opt_v)
		fprintf(stderr, ">> %s\n", fline);
	    if (fline[0] == '#')
		continue;
            switch(what) {
            case SETHOST:
                sethname(fline);
                break;
            case SETDOMAIN:
                setdname(fline);
                break;
#if HAVE_AFDECnet
            case SETNODE:
                setnname(fline);
                break;
#endif /* HAVE_AFDECnet */
            }
	}
	(void) fclose(fd);
    } else {
	fprintf(stderr, _("%s: can't open `%s'\n"),
		program_name, name);
	exit(1);
    }
}

static void version(void)
{
    printf("%s\n", Release);
    exit(E_VERSION);
}

static void usage(int rc)
{
    FILE *fp = rc ? stderr : stdout;
    fprintf(fp, _("Usage: hostname [-v] {hostname|-F file}      set hostname (from file)\n"));
    fprintf(fp, _("       domainname [-v] {nisdomain|-F file}   set NIS domainname (from file)\n"));
#if HAVE_AFDECnet
    fprintf(fp, _("       nodename [-v] {nodename|-F file}      set DECnet node name (from file)\n"));
#endif
    fprintf(fp, _("       hostname [-v] [-d|-f|-s|-a|-i|-y|-n]  display formatted name\n"));
    fprintf(fp, _("       hostname [-v]                         display hostname\n\n"));
    fprintf(fp, _("       hostname -V|--version|-h|--help       print info and exit\n\n"));
    fprintf(fp, _("    dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n"));
    fprintf(fp, _("    -s, --short           short host name\n"));
    fprintf(fp, _("    -a, --alias           alias names\n"));
    fprintf(fp, _("    -i, --ip-address      addresses for the hostname\n"));
    fprintf(fp, _("    -f, --fqdn, --long    long host name (FQDN)\n"));
    fprintf(fp, _("    -d, --domain          DNS domain name\n"));
    fprintf(fp, _("    -y, --yp, --nis       NIS/YP domainname\n"));
#if HAVE_AFDECnet
    fprintf(fp, _("    -n, --node            DECnet node name\n"));
#endif /* HAVE_AFDECnet */
    fprintf(fp, _("    -F, --file            read hostname or NIS domainname from given file\n\n"));
    fprintf(fp, _(
"   This command can read or set the hostname or the NIS domainname. You can\n"
"   also read the DNS domain or the FQDN (fully qualified domain name).\n"
"   Unless you are using bind or NIS for host lookups you can change the\n"
"   FQDN (Fully Qualified Domain Name) and the DNS domain name (which is\n"
"   part of the FQDN) in the /etc/hosts file.\n"));

    exit(rc);
}


int main(int argc, char **argv)
{
    int c;
    char type = '\0';
    int option_index = 0;
    int what = 0;
    char myname[MAXHOSTNAMELEN + 1] =
    {0};
    char *file = NULL;

    static const struct option long_options[] =
    {
	{"domain", no_argument, 0, 'd'},
	{"file", required_argument, 0, 'F'},
	{"fqdn", no_argument, 0, 'f'},
	{"help", no_argument, 0, 'h'},
	{"long", no_argument, 0, 'f'},
	{"short", no_argument, 0, 's'},
	{"version", no_argument, 0, 'V'},
	{"verbose", no_argument, 0, 'v'},
	{"alias", no_argument, 0, 'a'},
	{"ip-address", no_argument, 0, 'i'},
	{"nis", no_argument, 0, 'y'},
	{"yp", no_argument, 0, 'y'},
#if HAVE_AFDECnet
	{"node", no_argument, 0, 'n'},
#endif /* HAVE_AFDECnet */
	{0, 0, 0, 0}
    };
#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif
    program_name = (rindex(argv[0], '/')) ? rindex(argv[0], '/') + 1 : argv[0];

    if (!strcmp(program_name, "ypdomainname") ||
	!strcmp(program_name, "domainname") ||
	!strcmp(program_name, "nisdomainname"))
	what = 3;
    if (!strcmp(program_name, "dnsdomainname"))
	what = 2;
#if HAVE_AFDECnet
    if (!strcmp(program_name, "nodename"))
        what = 4;
#endif /* HAVE_AFDECnet */

    while ((c = getopt_long(argc, argv, "adfF:h?isVvyn", long_options, &option_index)) != EOF)
	switch (c) {
	case 'd':
	    what = 2;
	    break;
	case 'a':
	case 'f':
	case 'i':
	case 's':
	    what = 1;
	    type = c;
	    break;
	case 'y':
	    what = 3;
	    break;
#if HAVE_AFDECnet
	case 'n':
            what = 4;
            break;
#endif /* HAVE_AFDECnet */
	case 'F':
	    file = optarg;
	    break;
	case 'v':
	    opt_v++;
	    break;
	case 'V':
	    version();
	    break; // not reached
	case 'h':
	    usage(E_USAGE);
	    break; // not reached
	case '?':
	default:
	    usage(E_OPTERR);
	    break; // not reached
	};


    switch (what) {
    case 2:
	if (file || (optind < argc)) {
	    fprintf(stderr, _("%s: You can't change the DNS domain name with this command\n"), program_name);
	    fprintf(stderr, _("\nUnless you are using bind or NIS for host lookups you can change the DNS\n"));
	    fprintf(stderr, _("domain name (which is part of the FQDN) in the /etc/hosts file.\n"));
	    exit(1);
	}
	type = 'd';
	/* NOBREAK */
    case 0:
	if (file) {
	    setfilename(file, SETHOST);
	    break;
	}
	if (optind < argc) {
	    sethname(argv[optind]);
	    break;
	}
    case 1:
	gethostname(myname, sizeof(myname));
	if (opt_v)
	    fprintf(stderr, _("gethostname()=`%s'\n"), myname);
	if (!type)
	    printf("%s\n", myname);
	else if (type == 's') {
	    char *p = strchr(myname, '.');
	    if (p)
		*p = '\0';
	    printf("%s\n", myname);
	} else
	    showhname(myname, type);
	break;
    case 3:
	if (file) {
	    setfilename(file, SETDOMAIN);
	    break;
	}
	if (optind < argc) {
	    setdname(argv[optind]);
	    break;
	}
	if (getdomainname(myname, sizeof(myname)) < 0) {
	    perror("getdomainname()");
	    exit(1);
	}
	if (opt_v)
	    fprintf(stderr, _("getdomainname()=`%s'\n"), myname);
	printf("%s\n", myname);
	break;
#if HAVE_AFDECnet
    case 4:
        if (file) {
            setfilename(file, SETNODE);
            break;
        }
        if (optind < argc) {
            setnname(argv[optind]);
            break;
        }
        getnodename(myname, sizeof(myname));
        if (opt_v)
            fprintf(stderr, _("getnodename()=`%s'\n"), myname);
        printf("%s\n", myname);
        break;
#endif /* HAVE_AFDECnet */
    }
    exit(0);
}
