/*
 * lib/inet.c This file contains an implementation of the "INET"
 *              support functions for the net-tools.
 *              (NET-3 base distribution).
 *
 * Version:    $Id: inet.c,v 1.13 1999/12/11 13:35:56 freitag Exp $
 *
 * Author:      Fred N. van Kempen, <waltje@uwalt.nl.mugnet.org>
 *              Copyright 1993 MicroWalt Corporation
 *
 * Modified:
 *960113 {1.21} Bernd Eckenfels :       rresolve cache bug.
 *960128 {1.22} Bernd Eckenfels :       endian bug in print
 *960203 {1.23} Bernd Eckenfels :       net-features support
 *960217 {1.24} Bernd Eckenfels :       get_sname
 *960219 {1.25} Bernd Eckenfels :       extern int h_errno
 *960329 {1.26} Bernd Eckenfels :       resolve 255.255.255.255 
 *980101 {1.27} Bernd Eckenfels :	resolve raw sockets in /etc/protocols
 *990302 {1.28} Phil Blundell   :       add netmask to INET_rresolve
 *991007        Kurt Garloff	:	rresolve, resolve: may be hosts
 *		<garloff@suse.de>	store type (host?) in cache 
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include "config.h"

/* FIXME.  Split this file into inet4.c for the IPv4 specific parts
   and inet.c for those shared between IPv4 and IPv6.  */

#if HAVE_AFINET || HAVE_AFINET6
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "util.h"

extern int h_errno;		/* some netdb.h versions don't export this */

/* cache */
struct addr {
    struct sockaddr_in addr;
    char *name;
    int host;
    struct addr *next;
};

struct service {
    int number;
    char *name;
    struct service *next;
};

static struct service *tcp_name = NULL, *udp_name = NULL, *raw_name = NULL;

#if HAVE_AFINET

static struct addr *INET_nn = NULL;	/* addr-to-name cache           */


static int INET_resolve(char *name, struct sockaddr_in *sin, int hostfirst)
{
    struct hostent *hp;
    struct netent *np;

    /* Grmpf. -FvK */
    sin->sin_family = AF_INET;
    sin->sin_port = 0;

    /* Default is special, meaning 0.0.0.0. */
    if (!strcmp(name, "default")) {
	sin->sin_addr.s_addr = INADDR_ANY;
	return (1);
    }
    /* Look to see if it's a dotted quad. */
    if (inet_aton(name, &sin->sin_addr)) {
	return 0;
    }
    /* If we expect this to be a hostname, try hostname database first */
#ifdef DEBUG
    if (hostfirst) fprintf (stderr, "gethostbyname (%s)\n", name);
#endif
    if (hostfirst && 
	(hp = gethostbyname(name)) != (struct hostent *) NULL) {
	memcpy((char *) &sin->sin_addr, (char *) hp->h_addr_list[0], 
		sizeof(struct in_addr));
	return 0;
    }
    /* Try the NETWORKS database to see if this is a known network. */
#ifdef DEBUG
    fprintf (stderr, "getnetbyname (%s)\n", name);
#endif
    if ((np = getnetbyname(name)) != (struct netent *) NULL) {
	sin->sin_addr.s_addr = htonl(np->n_net);
	return 1;
    }
    if (hostfirst) {
	/* Don't try again */
	errno = h_errno;
	return -1;
    }
#ifdef DEBUG
    res_init();
    _res.options |= RES_DEBUG;
#endif

#ifdef DEBUG
    fprintf (stderr, "gethostbyname (%s)\n", name);
#endif
    if ((hp = gethostbyname(name)) == (struct hostent *) NULL) {
	errno = h_errno;
	return -1;
    }
    memcpy((char *) &sin->sin_addr, (char *) hp->h_addr_list[0], 
	   sizeof(struct in_addr));

    return 0;
}


/* numeric: & 0x8000: default instead of *, 
 *	    & 0x4000: host instead of net, 
 *	    & 0x0fff: don't resolve
 */
static int INET_rresolve(char *name, size_t len, struct sockaddr_in *sin, 
			 int numeric, unsigned int netmask)
{
    struct hostent *ent;
    struct netent *np;
    struct addr *pn;
    unsigned long ad, host_ad;
    int host = 0;

    /* Grmpf. -FvK */
    if (sin->sin_family != AF_INET) {
#ifdef DEBUG
	fprintf(stderr, _("rresolve: unsupport address family %d !\n"), sin->sin_family);
#endif
	errno = EAFNOSUPPORT;
	return (-1);
    }
    ad = (unsigned long) sin->sin_addr.s_addr;
#ifdef DEBUG
    fprintf (stderr, "rresolve: %08lx, mask %08x, num %08x \n", ad, netmask, numeric);
#endif
    if (ad == INADDR_ANY) {
	if ((numeric & 0x0FFF) == 0) {
	    if (numeric & 0x8000)
		safe_strncpy(name, "default", len);
	    else
	        safe_strncpy(name, "*", len);
	    return (0);
	}
    }
    if (numeric & 0x0FFF) {
        safe_strncpy(name, inet_ntoa(sin->sin_addr), len);
	return (0);
    }

    if ((ad & (~netmask)) != 0 || (numeric & 0x4000))
	host = 1;
#if 0
    INET_nn = NULL;
#endif
    pn = INET_nn;
    while (pn != NULL) {
	if (pn->addr.sin_addr.s_addr == ad && pn->host == host) {
	    safe_strncpy(name, pn->name, len);
#ifdef DEBUG
	    fprintf (stderr, "rresolve: found %s %08lx in cache\n", (host? "host": "net"), ad);
#endif
	    return (0);
	}
	pn = pn->next;
    }

    host_ad = ntohl(ad);
    np = NULL;
    ent = NULL;
    if (host) {
#ifdef DEBUG
	fprintf (stderr, "gethostbyaddr (%08lx)\n", ad);
#endif
	ent = gethostbyaddr((char *) &ad, 4, AF_INET);
	if (ent != NULL)
	    safe_strncpy(name, ent->h_name, len);
    } else {
#ifdef DEBUG
	fprintf (stderr, "getnetbyaddr (%08lx)\n", host_ad);
#endif
	np = getnetbyaddr(host_ad, AF_INET);
	if (np != NULL)
	    safe_strncpy(name, np->n_name, len);
    }
    if ((ent == NULL) && (np == NULL))
	safe_strncpy(name, inet_ntoa(sin->sin_addr), len);
    pn = (struct addr *) malloc(sizeof(struct addr));
    pn->addr = *sin;
    pn->next = INET_nn;
    pn->host = host;
    pn->name = (char *) malloc(strlen(name) + 1);
    strcpy(pn->name, name);
    INET_nn = pn;

    return (0);
}


static void INET_reserror(char *text)
{
    herror(text);
}


/* Display an Internet socket address. */
static char *INET_print(unsigned char *ptr)
{
    return (inet_ntoa((*(struct in_addr *) ptr)));
}


/* Display an Internet socket address. */
static char *INET_sprint(struct sockaddr *sap, int numeric)
{
    static char buff[128];

    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return safe_strncpy(buff, _("[NONE SET]"), sizeof(buff));

    if (INET_rresolve(buff, sizeof(buff), (struct sockaddr_in *) sap, 
		      numeric, 0xffffff00) != 0)
	return (NULL);

    return (buff);
}

char *INET_sprintmask(struct sockaddr *sap, int numeric, 
		      unsigned int netmask)
{
    static char buff[128];

    if (sap->sa_family == 0xFFFF || sap->sa_family == 0)
	return safe_strncpy(buff, _("[NONE SET]"), sizeof(buff));
    if (INET_rresolve(buff, sizeof(buff), (struct sockaddr_in *) sap, 
		      numeric, netmask) != 0)
	return (NULL);
    return (buff);
}


static int INET_getsock(char *bufp, struct sockaddr *sap)
{
    char *sp = bufp, *bp;
    unsigned int i;
    unsigned val;
    struct sockaddr_in *sin;

    sin = (struct sockaddr_in *) sap;
    sin->sin_family = AF_INET;
    sin->sin_port = 0;

    val = 0;
    bp = (char *) &val;
    for (i = 0; i < sizeof(sin->sin_addr.s_addr); i++) {
	*sp = toupper(*sp);

	if ((*sp >= 'A') && (*sp <= 'F'))
	    bp[i] |= (int) (*sp - 'A') + 10;
	else if ((*sp >= '0') && (*sp <= '9'))
	    bp[i] |= (int) (*sp - '0');
	else
	    return (-1);

	bp[i] <<= 4;
	sp++;
	*sp = toupper(*sp);

	if ((*sp >= 'A') && (*sp <= 'F'))
	    bp[i] |= (int) (*sp - 'A') + 10;
	else if ((*sp >= '0') && (*sp <= '9'))
	    bp[i] |= (int) (*sp - '0');
	else
	    return (-1);

	sp++;
    }
    sin->sin_addr.s_addr = htonl(val);

    return (sp - bufp);
}

static int INET_input(int type, char *bufp, struct sockaddr *sap)
{
    switch (type) {
    case 1:
	return (INET_getsock(bufp, sap));
    case 256:
	return (INET_resolve(bufp, (struct sockaddr_in *) sap, 1));
    default:
	return (INET_resolve(bufp, (struct sockaddr_in *) sap, 0));
    }
}

static int INET_getnetmask(char *adr, struct sockaddr *m, char *name)
{
    struct sockaddr_in *mask = (struct sockaddr_in *) m;
    char *slash, *end;
    int prefix;

    if ((slash = strchr(adr, '/')) == NULL)
	return 0;

    *slash++ = '\0';
    prefix = strtoul(slash, &end, 0);
    if (*end != '\0')
	return -1;

    if (name) {
	sprintf(name, "/%d", prefix);
    }
    mask->sin_family = AF_INET;
    mask->sin_addr.s_addr = htonl(~(0xffffffffU >> prefix));
    return 1;
}


struct aftype inet_aftype =
{
    "inet", NULL, /*"DARPA Internet", */ AF_INET, sizeof(unsigned long),
    INET_print, INET_sprint, INET_input, INET_reserror,
    NULL /*INET_rprint */ , NULL /*INET_rinput */ ,
    INET_getnetmask,
    -1,
    NULL
};

#endif				/* HAVE_AFINET */

static void add2list(struct service **namebase, struct service *item)
{
    if (*namebase == NULL) {
	*namebase = item;
	item->next = NULL;
    } else {
	item->next = *namebase;
	*namebase = item;
    }
}


static struct service *searchlist(struct service *servicebase, int number)
{
    struct service *item;

    for (item = servicebase; item != NULL; item = item->next) {
	if (item->number == number)
	    return (item);
    }
    return (NULL);
}


static int read_services(void)
{
    struct servent *se;
    struct protoent *pe;
    struct service *item;

    setservent(1);
    while ((se = getservent())) {
	/* Allocate a service entry. */
	item = (struct service *) malloc(sizeof(struct service));
	if (item == NULL)
	    perror("netstat");
	item->name = strdup(se->s_name);
	item->number = se->s_port;

	/* Fill it in. */
	if (!strcmp(se->s_proto, "tcp")) {
	    add2list(&tcp_name, item);
	} else if (!strcmp(se->s_proto, "udp")) {
	    add2list(&udp_name, item);
	} else if (!strcmp(se->s_proto, "raw")) {
	    add2list(&raw_name, item);
	}
    }
    endservent();
    setprotoent(1);
    while ((pe = getprotoent())) {
	/* Allocate a service entry. */
	item = (struct service *) malloc(sizeof(struct service));
	if (item == NULL)
	    perror("netstat");
	item->name = strdup(pe->p_name);
	item->number = htons(pe->p_proto);
	add2list(&raw_name, item);
    }
    endprotoent();
    return (0);
}


char *get_sname(int socknumber, char *proto, int numeric)
{
    static char buffer[64], init = 0;
    struct service *item;

    if (socknumber == 0)
	return ("*");
    if (numeric) {
	sprintf(buffer, "%d", ntohs(socknumber));
	return (buffer);
    }
    if (!init) {
	(void) read_services();
	init = 1;
    }
    buffer[0] = '\0';
    if (!strcmp(proto, "tcp")) {
	if ((item = searchlist(tcp_name, socknumber)) != NULL)
	    sprintf(buffer, "%s", item->name);
    } else if (!strcmp(proto, "udp")) {
	if ((item = searchlist(udp_name, socknumber)) != NULL)
	    sprintf(buffer, "%s", item->name);
    } else if (!strcmp(proto, "raw")) {
	if ((item = searchlist(raw_name, socknumber)) != NULL)
	    sprintf(buffer, "%s", item->name);

    }
    if (!buffer[0])
	sprintf(buffer, "%d", ntohs(socknumber));
    return (buffer);
}

#endif				/* HAVE_AFINET || HAVE_AFINET6 */
