/*
 * lib/masq_info.c    This file contains a the functio masq_info
 *                      to print a table of current masquerade connections.
 *
 * NET-LIB      A collection of functions used from the base set of the
 *              NET-3 Networking Distribution for the LINUX operating
 *              system. (net-tools, net-drivers)
 *
 * Version:     $Id: masq_info.c,v 1.8 2009/09/06 22:52:01 vapier Exp $
 *
 * Author:      Bernd 'eckes' Eckenfels <net-tools@lina.inka.de>
 *              Copyright 1999 Bernd Eckenfels, Germany
 *
 * Modifications:
 *
 *960217 {0.01} Bernd Eckenfels:        creatin from the code of
 *                                      Jos Vos' ipfwadm 2.0beta1
 *950218 {0.02} Bernd Eckenfels:        <linux/if.h> added
 *
 *980405 {0.03} Arnaldo Carvalho:       i18n CATGETS -> gettext
 *
 *              This program is free software; you can redistribute it
 *              and/or  modify it under  the terms of  the GNU General
 *              Public  License as  published  by  the  Free  Software
 *              Foundation;  either  version 2 of the License, or  (at
 *              your option) any later version.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include "net-support.h"
#include "pathnames.h"
#include "version.h"
#include "config.h"
#include "intl.h"
#include "net-features.h"

#if HAVE_FW_MASQUERADE

struct masq {
    unsigned long expires;	/* Expiration timer */
    char *proto;		/* Which protocol are we talking? */
    union {
	struct sockaddr_storage src_sas;
	struct sockaddr_in src;	/* Source IP address */
    };
    union {
	struct sockaddr_storage dst_sas;
	struct sockaddr_in dst;	/* Destination IP address */
    };
    unsigned short sport, dport;	/* Source and destination ports */
    unsigned short mport;	/* Masqueraded port */
    unsigned long initseq;	/* Add delta from this seq. on */
    short delta;		/* Delta in sequence numbers */
    short pdelta;		/* Delta in sequence numbers before last */
};

static const struct aftype *ap;	/* current address family       */
static int has_pdelta;

static void print_masq(struct masq *ms, int numeric_host, int numeric_port,
		       int ext)
{
    unsigned long minutes, seconds, sec100s;

    printf("%-4s", ms->proto);

    sec100s = ms->expires % 100L;
    seconds = (ms->expires / 100L) % 60;
    minutes = ms->expires / 6000L;

    printf("%3ld:%02ld.%02ld ", minutes, seconds, sec100s);

    if (ext > 1) {
	if (has_pdelta)
	    printf("%10lu %5hd %5hd ", ms->initseq,
		   ms->delta, ms->pdelta);
	else
	    printf("%10lu %5hd     - ", ms->initseq,
		   ms->delta);
    }
    printf("%-20s ", ap->sprint(&ms->src_sas, numeric_host));
    printf("%-20s ", ap->sprint(&ms->dst_sas, numeric_host));

    printf("%s -> ", get_sname(ms->sport, ms->proto, numeric_port));
    printf("%s", get_sname(ms->dport, ms->proto, numeric_port));
    printf(" (%s)\n", get_sname(ms->mport, ms->proto, numeric_port));
}


static int read_masqinfo(FILE * f, struct masq *mslist, int nmslist)
{
    int n, nread = 0;
    struct masq *ms;
    char buf[256];
    uint32_t src_addr, dst_addr;

    for (nread = 0; nread < nmslist; nread++) {
	ms = &mslist[nread];
	if (has_pdelta) {
	    if ((n = fscanf(f, " %s %"PRIx32":%hX %"PRIx32":%hX %hX %lX %hd %hd %lu",
			    buf,
			    &src_addr, &ms->sport,
			    &dst_addr, &ms->dport,
			    &ms->mport, &ms->initseq, &ms->delta,
			    &ms->pdelta, &ms->expires)) == -1)
		return nread;
	    memcpy(&ms->src.sin_addr.s_addr, &src_addr, 4);
	    memcpy(&ms->dst.sin_addr.s_addr, &dst_addr, 4);
	} else {
	    if ((n = fscanf(f, " %s %"PRIx32":%hX %"PRIx32":%hX %hX %lX %hd %lu",
			    buf,
			    &src_addr, &ms->sport,
			    &dst_addr, &ms->dport,
			    &ms->mport, &ms->initseq, &ms->delta,
			    &ms->expires)) == -1)
		return nread;
	    memcpy(&ms->src.sin_addr.s_addr, &src_addr, 4);
	    memcpy(&ms->dst.sin_addr.s_addr, &dst_addr, 4);
	}
	if ((has_pdelta && (n != 10)) || (!has_pdelta && (n != 9))) {
	    EINTERN("masq_info.c", "ip_masquerade format error");
	    return (-1);
	}
	ms->src.sin_family = AF_INET;
	ms->dst.sin_family = AF_INET;

	if (strcmp("IP", buf) == 0)
	    ms->proto = "ip";
	else if (strcmp("TCP", buf) == 0)
	    ms->proto = "tcp";
	else if (strcmp("UDP", buf) == 0)
	    ms->proto = "udp";
	else if (strcmp("ICMP", buf) == 0)
	    ms->proto = "icmp";
	else if (strcmp("GRE", buf) == 0)
	    ms->proto = "gre";
	else if (strcmp("ESP", buf) == 0)
	    ms->proto = "esp";
	else {
	    EINTERN("masq_info.c", "ip_masquerade unknown type");
	    return (-1);
	}

	/* we always keep these addresses in network byte order */
	ms->src.sin_addr.s_addr = htonl(ms->src.sin_addr.s_addr);
	ms->dst.sin_addr.s_addr = htonl(ms->dst.sin_addr.s_addr);
	ms->sport = htons(ms->sport);
	ms->dport = htons(ms->dport);
	ms->mport = htons(ms->mport);
    }
    return nread;
}


int ip_masq_info(int numeric_host, int numeric_port, int ext)
{
    FILE *f;
    int i;
    char buf[256];
    struct masq *mslist;
    int ntotal = 0, nread;

    if (!(f = fopen(_PATH_PROCNET_IP_MASQ, "r"))) {
	if (errno != ENOENT) {
	    perror(_PATH_PROCNET_IP_MASQ);
	    return (-1);
	}
	ESYSNOT("netstat", "ip_masquerade");
	return (1);
    }
    if ((ap = get_aftype("inet")) == NULL) {
	ENOSUPP("masq_info", "AF INET");
	fclose(f);
	return (-1);
    }
    if (fgets(buf, sizeof(buf), f) == NULL) {
	EINTERN("masq_info", "fgets() failed");
	fclose(f);
	return (-1);
    }
    has_pdelta = strstr(buf, "PDelta") ? 1 : 0;

    mslist = (struct masq *) malloc(16 * sizeof(struct masq));
    if (!mslist) {
	EINTERN("masq_info", "malloc() failed");
	fclose(f);
	return (-1);
    }
    while ((nread = read_masqinfo(f, &(mslist[ntotal]), 16)) == 16) {
	ntotal += nread;
	mslist = (struct masq *) realloc(mslist,
				    (ntotal + 16) * sizeof(struct masq));
	if (!mslist) {
	    EINTERN("masq_info", "realloc() failed");
	    fclose(f);
	    return (-1);
	}
    }
    fclose(f);

    if (nread < 0) {
	if (mslist)
	    free(mslist);
	return (-1);
    }
    ntotal += nread;

    if (ntotal > 0) {
	printf(_("IP masquerading entries\n"));
	switch (ext) {
	case 1:
	    printf(_("prot   expire source               destination          ports\n"));
	    break;
	default:
	    printf(_("prot   expire    initseq delta prevd source               destination          ports\n"));
	    break;
	}
	for (i = 0; i < ntotal; i++)
	    print_masq(&(mslist[i]), numeric_host, numeric_port, ext);
    }

    free(mslist);
    return 0;
}
#endif
