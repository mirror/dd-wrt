/*
   Modifications:
   1998-07-01 - Arnaldo Carvalho de Melo - GNU gettext instead of catgets,
   snprintf instead of sprintf
 */

#include "config.h"

#if HAVE_AFINET6
#include <asm/types.h>
#include <asm/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
/* #include <net/route.h> realy broken */
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#ifndef __GLIBC__
#include <netinet6/ipv6_route.h>	/* glibc doesn't have this */
#endif
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "net-features.h"

/* neighbour discovery from linux-2.4.0/include/net/neighbour.h */

#define NUD_INCOMPLETE  0x01
#define NUD_REACHABLE   0x02
#define NUD_STALE       0x04
#define NUD_DELAY       0x08
#define NUD_PROBE       0x10
#define NUD_FAILED      0x20

#define NUD_NOARP       0x40
#define NUD_PERMANENT   0x80
#define NUD_NONE        0x00

#define NTF_PROXY       0x08    /* == ATF_PUBL */
#define NTF_ROUTER      0x80
#define NTF_02          0x02  /* waiting for answer of Alexey -eckes */
#define NTF_04          0x04  /* waiting for answer of Alexey -eckes */

/* */


extern struct aftype inet6_aftype;


int rprint_fib6(int ext, int numeric)
{
    char buff[4096], iface[16], flags[16];
    char addr6[128], naddr6[128];
    struct sockaddr_in6 saddr6, snaddr6;
    int num, iflags, metric, refcnt, use, prefix_len, slen;
    FILE *fp = fopen(_PATH_PROCNET_ROUTE6, "r");
    
    char addr6p[8][5], saddr6p[8][5], naddr6p[8][5];

    if (!fp) {
	perror(_PATH_PROCNET_ROUTE6);
        printf(_("INET6 (IPv6) not configured in this system.\n"));
	return 1;
    }
    printf(_("Kernel IPv6 routing table\n"));

    printf(_("Destination                                 "
	     "Next Hop                                "
	     "Flags Metric Ref    Use Iface\n"));

    while (fgets(buff, 1023, fp)) {
	num = sscanf(buff, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %4s%4s%4s%4s%4s%4s%4s%4s %02x %4s%4s%4s%4s%4s%4s%4s%4s %08x %08x %08x %08x %s\n",
		     addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		     addr6p[4], addr6p[5], addr6p[6], addr6p[7],
		     &prefix_len,
		     saddr6p[0], saddr6p[1], saddr6p[2], saddr6p[3],
		     saddr6p[4], saddr6p[5], saddr6p[6], saddr6p[7],
		     &slen,
		     naddr6p[0], naddr6p[1], naddr6p[2], naddr6p[3],
		     naddr6p[4], naddr6p[5], naddr6p[6], naddr6p[7],
		     &metric, &use, &refcnt, &iflags, iface);
#if 0
	if (num < 23)
	    continue;
#endif
	if (!(iflags & RTF_UP))
	    continue;
	/* Fetch and resolve the target address. */
	snprintf(addr6, sizeof(addr6), "%s:%s:%s:%s:%s:%s:%s:%s",
		 addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		 addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
	inet6_aftype.input(1, addr6, (struct sockaddr *) &saddr6);
	snprintf(addr6, sizeof(addr6), "%s/%d",
		 inet6_aftype.sprint((struct sockaddr *) &saddr6, 1),
		 prefix_len);

	/* Fetch and resolve the nexthop address. */
	snprintf(naddr6, sizeof(naddr6), "%s:%s:%s:%s:%s:%s:%s:%s",
		 naddr6p[0], naddr6p[1], naddr6p[2], naddr6p[3],
		 naddr6p[4], naddr6p[5], naddr6p[6], naddr6p[7]);
	inet6_aftype.input(1, naddr6, (struct sockaddr *) &snaddr6);
	snprintf(naddr6, sizeof(naddr6), "%s",
		 inet6_aftype.sprint((struct sockaddr *) &snaddr6, 1));

	/* Decode the flags. */
	strcpy(flags, "U");
	if (iflags & RTF_GATEWAY)
	    strcat(flags, "G");
	if (iflags & RTF_HOST)
	    strcat(flags, "H");
	if (iflags & RTF_DEFAULT)
	    strcat(flags, "D");
	if (iflags & RTF_ADDRCONF)
	    strcat(flags, "A");
	if (iflags & RTF_CACHE)
	    strcat(flags, "C");

	/* Print the info. */
	printf("%-43s %-39s %-5s %-6d %-2d %7d %-8s\n",
	       addr6, naddr6, flags, metric, refcnt, use, iface);
    }

    (void) fclose(fp);
    return (0);
}

int rprint_cache6(int ext, int numeric)
{
    char buff[4096], iface[16], flags[16];
    char addr6[128], haddr[20], statestr[20];
    struct sockaddr_in6 saddr6;
    int type, num, refcnt, prefix_len, location, state, gc;
    long tstamp, expire, ndflags, reachable, stale, delete;
    FILE *fp = fopen(_PATH_PROCNET_NDISC, "r");
    char addr6p[8][5], haddrp[6][3];

    if (!fp) {
	ESYSNOT("nd_print", "ND Table");
	return 1;
    }
    printf(_("Kernel IPv6 Neighbour Cache\n"));

    if (ext == 2)
	printf(_("Neighbour                                   "
		 "HW Address        "
		 "Iface    Flags Ref State\n"));
    else
	printf(_("Neighbour                                   "
		 "HW Address        "
	"Iface    Flags Ref State            Stale(sec) Delete(sec)\n"));


    while (fgets(buff, 1023, fp)) {
	num = sscanf(buff, "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %08lx %08lx %08lx %04x %04x %04lx %8s %2s%2s%2s%2s%2s%2s\n",
		     addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		     addr6p[4], addr6p[5], addr6p[6], addr6p[7],
		     &location, &prefix_len, &type, &state, &expire, &tstamp, &reachable, &gc, &refcnt,
		     &ndflags, iface,
	haddrp[0], haddrp[1], haddrp[2], haddrp[3], haddrp[4], haddrp[5]);

	/* Fetch and resolve the nexthop address. */
	snprintf(addr6, sizeof(addr6), "%s:%s:%s:%s:%s:%s:%s:%s",
		 addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		 addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
	inet6_aftype.input(1, addr6, (struct sockaddr *) &saddr6);
	snprintf(addr6, sizeof(addr6), "%s/%d",
	       inet6_aftype.sprint((struct sockaddr *) &saddr6, numeric),
		 prefix_len);

	/* Fetch the  hardware address. */
	snprintf(haddr, sizeof(haddr), "%s:%s:%s:%s:%s:%s",
	haddrp[0], haddrp[1], haddrp[2], haddrp[3], haddrp[4], haddrp[5]);

	/* Decode the flags. */
	flags[0] = '\0';
	if (ndflags & NTF_ROUTER)
	    strcat(flags, "R");
	if (ndflags & NTF_04)
	    strcat(flags, "x");
	if (ndflags & NTF_02)
	    strcat(flags, "h");
	if (ndflags & NTF_PROXY)
	    strcat(flags, "P");

	/* Decode the state */
	switch (state) {
	case NUD_NONE:
	    strcpy(statestr, "NONE");
	    break;
	case NUD_INCOMPLETE:
	    strcpy(statestr, "INCOMPLETE");
	    break;
	case NUD_REACHABLE:
	    strcpy(statestr, "REACHABLE");
	    break;
	case NUD_STALE:
	    strcpy(statestr, "STALE");
	    break;
	case NUD_DELAY:
	    strcpy(statestr, "DELAY");
	    break;
	case NUD_PROBE:
	    strcpy(statestr, "PROBE");
	    break;
	case NUD_FAILED:
	    strcpy(statestr, "FAILED");
	    break;
	case NUD_NOARP:
	    strcpy(statestr, "NOARP");
	    break;
	case NUD_PERMANENT:
	    strcpy(statestr, "PERM");
	    break;
	default:
	    snprintf(statestr, sizeof(statestr), "UNKNOWN(%02x)", state);
	    break;
	}

	/* Print the info. */
	printf("%-43s %-17s %-8s %-5s %-3d %-16s",
	       addr6, haddr, iface, flags, refcnt, statestr);

	stale = 0;
	if (state == NUD_REACHABLE)
	    stale = reachable > tstamp ? reachable - tstamp : 0;
	delete = gc > tstamp ? gc - tstamp : 0;
	if (ext != 2) {
	    printf(" %-9ld ", stale / HZ);
	    if (refcnt)
		printf(" * ");
	    else
		printf(" %-7ld ", delete / HZ);
	}
	printf("\n");
    }

    (void) fclose(fp);
    return (0);
}

int INET6_rprint(int options)
{
    int ext = options & FLAG_EXT;
    int numeric = options & (FLAG_NUM_HOST | FLAG_SYM);
    int rc = E_INTERN;

    if (options & FLAG_FIB)
	if ((rc = rprint_fib6(ext, numeric)))
	    return (rc);

    if (options & FLAG_CACHE)
	if ((rc = rprint_cache6(ext, numeric)))
	    return (rc);
    return (rc);
}

#endif				/* HAVE_AFINET6 */
