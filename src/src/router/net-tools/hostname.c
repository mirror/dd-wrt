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
 * Version:     hostname 1.96 (1996-02-18)
 *
 * Author:      Peter Tobias <tobias@et-inf.fho-emden.de>
 *
 * Changes:
 *      {1.90}  Peter Tobias :          Added -a and -i options.
 *      {1.91}  Bernd Eckenfels :       -v,-V rewritten, long_opts 
 *                                      (major rewrite), usage.
 *960120 {1.95} Bernd Eckenfels :       -y/nisdomainname - support for get/
 *                                      setdomainname added 
 *960218 {1.96} Bernd Eckenfels :       netinet/in.h added
 *980629 {1.97} Arnaldo Carvalho de Melo : gettext instead of catgets for i18n
 *20000213 {1.99} Arnaldo Carvalho de Melo : fixed some i18n strings
 *20010404 {1.100} Arnaldo Carvalho de Melo: use setlocale
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "config.h"
#include "version.h"
#include "../intl.h"

#if HAVE_AFDECnet
#include <netdnet/dn.h>
#endif

char *Release = RELEASE, *Version = "hostname 1.100 (2001-04-14)";

static char *program_name;
static int opt_v;

static void sethname(char *);
static void setdname(char *);
static void showhname(char *, int);
static void usage(void);
static void version(void);
static void setfilename(char *, int);

#define SETHOST		1
#define SETDOMAIN	2
#define SETNODE		3

#if HAVE_AFDECnet
static void setnname(char *nname)
{
    if (opt_v)
        fprintf(stderr, _("Setting nodename to `%s'\n"),
                nname);
    if (setnodename(nname, strlen(nname))) {
        switch(errno) {
        case EPERM:
            fprintf(stderr, _("%s: you must be root to change the node name\n"), program_name);
            break;
        case EINVAL:
            fprintf(stderr, _("%s: name too long\n"), program_name);
            break;
        default:
        }
	exit(1);
    }
}
#endif /* HAVE_AFDECnet */

static void sethname(char *hname)
{
    if (opt_v)
	fprintf(stderr, _("Setting hostname to `%s'\n"),
		hname);
    if (sethostname(hname, strlen(hname))) {
	switch (errno) {
	case EPERM:
	    fprintf(stderr, _("%s: you must be root to change the host name\n"), program_name);
	    break;
	case EINVAL:
	    fprintf(stderr, _("%s: name too long\n"), program_name);
	    break;
	default:
	}
	exit(1);
    };
}

static void setdname(char *dname)
{
    if (opt_v)
	fprintf(stderr, _("Setting domainname to `%s'\n"),
		dname);
    if (setdomainname(dname, strlen(dname))) {
	switch (errno) {
	case EPERM:
	    fprintf(stderr, _("%s: you must be root to change the domain name\n"), program_name);
	    break;
	case EINVAL:
	    fprintf(stderr, _("%s: name too long\n"), program_name);
	    break;
	default:
	}
	exit(1);
    };
}

static void showhname(char *hname, int c)
{
    struct hostent *hp;
    register char *p, **alias;
    struct in_addr **ip;

    if (opt_v)
	fprintf(stderr, _("Resolving `%s' ...\n"), hname);
    if (!(hp = gethostbyname(hname))) {
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

	ip = (struct in_addr **) hp->h_addr_list;
	while (ip[0])
	    fprintf(stderr, _("Result: h_addr_list=`%s'\n"),
		    inet_ntoa(**ip++));
    }
    if (!(p = strchr(hp->h_name, '.')) && (c == 'd'))
	return;

    switch (c) {
    case 'a':
	while (hp->h_aliases[0])
	    printf("%s ", *hp->h_aliases++);
	printf("\n");
	break;
    case 'i':
	while (hp->h_addr_list[0])
	    printf("%s ", inet_ntoa(*(struct in_addr *) *hp->h_addr_list++));
	printf("\n");
	break;
    case 'd':
	printf("%s\n", ++p);
	break;
    case 'f':
	printf("%s\n", hp->h_name);
	break;
    case 's':
	if (p != NULL)
	    *p = '\0';
	printf("%s\n", hp->h_name);
	break;
    default:
    }
}

static void setfilename(char *name, int what)
{
    register FILE *fd;
    register char *p;
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
    fprintf(stderr, "%s\n%s\n", Release, Version);
    exit(5); /* E_VERSION */
}

static void usage(void)
{
    fprintf(stderr, _("Usage: hostname [-v] {hostname|-F file}      set hostname (from file)\n"));
    fprintf(stderr, _("       domainname [-v] {nisdomain|-F file}   set NIS domainname (from file)\n"));
#if HAVE_AFDECnet
    fprintf(stderr, _("       nodename [-v] {nodename|-F file}      set DECnet node name (from file)\n"));
#endif
    fprintf(stderr, _("       hostname [-v] [-d|-f|-s|-a|-i|-y|-n]  display formatted name\n"));
    fprintf(stderr, _("       hostname [-v]                         display hostname\n\n"));
    fprintf(stderr, _("       hostname -V|--version|-h|--help       print info and exit\n\n"));
    fprintf(stderr, _("    dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n"));
    fprintf(stderr, _("    -s, --short           short host name\n"));
    fprintf(stderr, _("    -a, --alias           alias names\n"));
    fprintf(stderr, _("    -i, --ip-address      addresses for the hostname\n"));
    fprintf(stderr, _("    -f, --fqdn, --long    long host name (FQDN)\n"));
    fprintf(stderr, _("    -d, --domain          DNS domain name\n"));
    fprintf(stderr, _("    -y, --yp, --nis       NIS/YP domainname\n"));
#if HAVE_AFDECnet
    fprintf(stderr, _("    -n, --node            DECnet node name\n"));
#endif /* HAVE_AFDECnet */
    fprintf(stderr, _("    -F, --file            read hostname or NIS domainname from given file\n\n"));
    fprintf(stderr, _(
"   This command can read or set the hostname or the NIS domainname. You can\n"
"   also read the DNS domain or the FQDN (fully qualified domain name).\n"
"   Unless you are using bind or NIS for host lookups you can change the\n"
"   FQDN (Fully Qualified Domain Name) and the DNS domain name (which is\n"
"   part of the FQDN) in the /etc/hosts file.\n"));

    exit(4); /* E_USAGE */
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
	case '?':
	case 'h':
	default:
	    usage();

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
	else
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
	getdomainname(myname, sizeof(myname));
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
