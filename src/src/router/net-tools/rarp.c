/*
 * rarp               This file contains an implementation of the command
 *              that maintains the kernel's RARP cache.  It is derived
 *              from Fred N. van Kempen's arp command.
 *
 * Version:	$Id: rarp.c,v 1.6 2001/04/08 17:05:05 pb Exp $
 *
 * Usage:       rarp -d hostname                      Delete entry
 *              rarp -s hostname ethernet_address     Add entry
 *              rarp -a                               Print entries
 *              rarp -f                               Add frop /etc/ethers
 *
 * Rewritten: Phil Blundell <Philip.Blundell@pobox.com>  1997-08-03
 * gettext instead of catgets: Arnaldo Carvalho de Melo <acme@conectiva.com.br> 1998-06-29
 * 1998-01-01 Bernd Eckenfels	reorganised usage()
 * 2001-04-04 Arnaldo Carvalho de Melo - use setlocale
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define DFLT_HW "ether"

#include "config.h"
#include "intl.h"
#include "net-support.h"
#include "version.h"
#include "pathnames.h"

static char no_rarp_message[] = N_("This kernel does not support RARP.\n");

static char version_string[] = RELEASE "\nrarp 1.03 (2001-04-04)\n";

static struct hwtype *hardware = NULL;

/* Delete an entry from the RARP cache. */
static int rarp_delete(int fd, struct hostent *hp)
{
    struct arpreq req;
    struct sockaddr_in *si;
    unsigned int found = 0;
    char **addr;

    /* The host can have more than one address, so we loop on them. */
    for (addr = hp->h_addr_list; *addr != NULL; addr++) {
	memset((char *) &req, 0, sizeof(req));
	si = (struct sockaddr_in *) &req.arp_pa;
	si->sin_family = hp->h_addrtype;
	memcpy((char *) &si->sin_addr, *addr, hp->h_length);

	/* Call the kernel. */
	if (ioctl(fd, SIOCDRARP, &req) == 0) {
	    found++;
	} else {
	    switch (errno) {
	    case ENXIO:
		break;
	    case ENODEV:
		fputs(_(no_rarp_message), stderr);
		return 1;
	    default:
		perror("SIOCDRARP");
		return 1;
	    }
	}
    }

    if (found == 0)
	printf(_("no RARP entry for %s.\n"), hp->h_name);
    return 0;
}


/* Set an entry in the RARP cache. */
static int rarp_set(int fd, struct hostent *hp, char *hw_addr)
{
    struct arpreq req;
    struct sockaddr_in *si;
    struct sockaddr sap;

    if (hardware->input(hw_addr, &sap)) {
	fprintf(stderr, _("%s: bad hardware address\n"), hw_addr);
	return 1;
    }
    /* Clear and fill in the request block. */
    memset((char *) &req, 0, sizeof(req));
    si = (struct sockaddr_in *) &req.arp_pa;
    si->sin_family = hp->h_addrtype;
    memcpy((char *) &si->sin_addr, hp->h_addr_list[0], hp->h_length);
    req.arp_ha.sa_family = hardware->type;
    memcpy(req.arp_ha.sa_data, sap.sa_data, hardware->alen);

    /* Call the kernel. */
    if (ioctl(fd, SIOCSRARP, &req) < 0) {
	if (errno == ENODEV)
	    fputs(_(no_rarp_message), stderr);
	else
	    perror("SIOCSRARP");
	return 1;
    }
    return 0;
}

/* Process an EtherFile */
static int rarp_file(int fd, const char *name)
{
    char buff[1024];
    char *host, *addr;
    int linenr;
    FILE *fp;
    struct hostent *hp;

    if ((fp = fopen(name, "r")) == NULL) {
	fprintf(stderr, _("rarp: cannot open file %s:%s.\n"), name, strerror(errno));
	return -1;
    }
    /* Read the lines in the file. */
    linenr = 0;
    while (fgets(buff, sizeof(buff), fp)) {
	++linenr;
	if (buff[0] == '#' || buff[0] == '\0')
	    continue;
	if ((addr = strtok(buff, "\n \t")) == NULL)
	    continue;
	if ((host = strtok(NULL, "\n \t")) == NULL) {
	    fprintf(stderr, _("rarp: format error at %s:%u\n"), name, linenr);
	    continue;
	}
	if ((hp = gethostbyname(host)) == NULL) {
	    fprintf(stderr, _("rarp: %s: unknown host\n"), host);
	}
	if (rarp_set(fd, hp, addr) != 0) {
	    fprintf(stderr, _("rarp: cannot set entry from %s:%u\n"), name, linenr);
	}
    }

    (void) fclose(fp);
    return 0;
}

static int display_cache(void)
{
    FILE *fd = fopen(_PATH_PROCNET_RARP, "r");
    char buffer[256];
    if (fd == NULL) {
	if (errno == ENOENT)
	    fputs(_(no_rarp_message), stderr);
	else
	    perror(_PATH_PROCNET_RARP);
	return 1;
    }
    while (feof(fd) == 0) {
	if (fgets(buffer, 255, fd))
	    fputs(buffer, stdout);
    }
    fclose(fd);
    return 0;
}

static void usage(void)
{
    fprintf(stderr, _("Usage: rarp -a                               list entries in cache.\n"));
    fprintf(stderr, _("       rarp -d <hostname>                    delete entry from cache.\n"));
    fprintf(stderr, _("       rarp [<HW>] -s <hostname> <hwaddr>    add entry to cache.\n"));
    fprintf(stderr, _("       rarp -f                               add entries from /etc/ethers.\n"));
    fprintf(stderr, _("       rarp -V                               display program version.\n\n"));

    fprintf(stderr, _("  <HW>=Use '-H <hw>' to specify hardware address type. Default: %s\n"), DFLT_HW);
    fprintf(stderr, _("  List of possible hardware types (which support ARP):\n"));
    print_hwlist(1); /* 1 = ARPable */
    exit(E_USAGE);
}

#define MODE_DISPLAY   1
#define MODE_DELETE    2
#define MODE_SET       3
#define MODE_ETHERS    4

static struct option longopts[] =
{
    {"version", 0, NULL, 'V'},
    {"verbose", 0, NULL, 'v'},
    {"list", 0, NULL, 'a'},
    {"set", 0, NULL, 's'},
    {"delete", 0, NULL, 'd'},
    {"help", 0, NULL, 'h'},
    {NULL, 0, NULL, 0}
};

int main(int argc, char **argv)
{
    int result = 0, mode = 0, c, nargs = 0, verbose = 0;
    char *args[3];
    struct hostent *hp;
    int fd;

#if I18N
    setlocale (LC_ALL, "");
    bindtextdomain("net-tools", "/usr/share/locale");
    textdomain("net-tools");
#endif

    /* Get a default hardware type.  */
    hardware = get_hwtype(DFLT_HW);

    do {
	c = getopt_long(argc, argv, "-ht:aHdsVvf", longopts, NULL);
	switch (c) {
	case EOF:
	    break;
	case 'h':
	    usage();
	case 'V':
	    fprintf(stderr, version_string);
	    exit(E_VERSION);
	    break;
	case 'v':
	    verbose++;
	    break;
	case 'a':
	case 's':
	case 'd':
	    if (mode) {
		fprintf(stderr, _("%s: illegal option mix.\n"), argv[0]);
		usage();
	    } else {
		mode = (c == 'a' ? MODE_DISPLAY : (c == 'd' ? MODE_DELETE : MODE_SET));
	    }
	    break;
	case 'f':
	    mode = MODE_ETHERS;
	    break;
        case 'H':
	case 't':
	    if (optarg) {
		hardware = get_hwtype(optarg);
	    } else {
		usage();
	    }
	    break;
	case 1:
	    if (nargs == 2) {
		usage();
		exit(1);
	    } else {
		args[nargs++] = optarg;
	    }
	    break;
	default:
	    usage();
	}
    } while (c != EOF);

    if (hardware == NULL) {
	fprintf(stderr, _("rarp: %s: unknown hardware type.\n"), optarg);
	exit(1);
    }
    switch (mode) {
    case 0:
	usage();

    case MODE_DISPLAY:
	if (nargs != (mode - 1)) {
	    usage();
	}
	result = display_cache();
	break;

    case MODE_DELETE:
    case MODE_SET:
	if (nargs != (mode - 1)) {
	    usage();
	}
	if ((hp = gethostbyname(args[0])) == NULL) {
	    fprintf(stderr, _("rarp: %s: unknown host\n"), args[0]);
	    exit(1);
	}
	if (fd = socket(PF_INET, SOCK_DGRAM, 0), fd < 0) {
	    perror("socket");
	    exit(1);
	}
	result = (mode == MODE_DELETE) ? rarp_delete(fd, hp) : rarp_set(fd, hp, args[1]);
	close(fd);
	break;

    case MODE_ETHERS:
	if (nargs != 0 && nargs != 1)
	    usage();
	if (fd = socket(PF_INET, SOCK_DGRAM, 0), fd < 0) {
	    perror("socket");
	    exit(1);
	}
	result = rarp_file(fd, nargs ? args[0] : _PATH_ETHERS);
	close(fd);

    }
    exit(result);
}
