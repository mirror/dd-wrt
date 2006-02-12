/****************************************************************
	Copyright 1989, 1991, 1992 by Carnegie Mellon University

                      All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of CMU not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.
******************************************************************/
/*
 * Copyright (c) 1983,1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <stdio.h>

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#include "winstub.h"
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif

#include "main.h"
#include <net-snmp/net-snmp-includes.h>
#include "netstat.h"

static char    *inetname(struct in_addr);

struct stat_table {
    int             entry;      /* entry number in table */
    /*
     * format string to printf(description, value, plural(value)); 
     */
    /*
     * warning: the %d must be before the %s 
     */
    char            description[80];
};

static oid      oid_ipstats[] = { 1, 3, 6, 1, 2, 1, 4, 0, 0 };
struct stat_table ip_stattab[] = {
    {3, "%d total datagram%s received"},
    {4, "%d datagram%s with header errors"},
    {5, "%d datagram%s with an invalid destination address"},
    {6, "%d datagram%s forwarded"},
    {7, "%d datagram%s with unknown protocol"},
    {8, "%d datagram%s discarded"},
    {9, "%d datagram%s delivered"},
    {10, "%d output datagram request%s"},
    {11, "%d output datagram%s discarded"},
    {12, "%d datagram%s with no route"},
    {14, "%d fragment%s received"},
    {15, "%d datagram%s reassembled"},
    {16, "%d reassembly failure%s"},
    {17, "%d datagram%s fragmented"},
    {18, "%d fragmentation failure%s"},
    {19, "%d fragment%s created"}
};

static oid      oid_udpstats[] = { 1, 3, 6, 1, 2, 1, 7, 0, 0 };
struct stat_table udp_stattab[] = {
    {1, "%d total datagram%s received"},
    {2, "%d datagram%s to invalid port"},
    {3, "%d datagram%s dropped due to errors"},
    {4, "%d output datagram request%s"}
};

static oid      oid_tcpstats[] = { 1, 3, 6, 1, 2, 1, 6, 0, 0 };
struct stat_table tcp_stattab[] = {
    {5, "%d active open%s"},
    {6, "%d passive open%s"},
    {7, "%d failed attempt%s"},
    {8, "%d reset%s of established connections"},
    {9, "%d current established connection%s"},
    {10, "%d segment%s received"},
    {11, "%d segment%s sent"},
    {12, "%d segment%s retransmitted"}
};

static oid      oid_icmpstats[] = { 1, 3, 6, 1, 2, 1, 5, 0, 0 };
struct stat_table icmp_stattab[] = {
    {1, "%d total message%s received"},
    {2, "%d message%s dropped due to errors"},
    {14, "%d ouput message request%s"},
    {15, "%d output message%s discarded"}
};

struct stat_table icmp_inhistogram[] = {
    {3, "Destination unreachable: %d"},
    {4, "Time Exceeded: %d"},
    {5, "Parameter Problem: %d"},
    {6, "Source Quench: %d"},
    {7, "Redirect: %d"},
    {8, "Echo Request: %d"},
    {9, "Echo Reply: %d"},
    {10, "Timestamp Request: %d"},
    {11, "Timestamp Reply: %d"},
    {12, "Address Mask Request: %d"},
    {13, "Addrss Mask Reply:%d"},
};

struct stat_table icmp_outhistogram[] = {
    {16, "Destination unreachable: %d"},
    {17, "Time Exceeded: %d"},
    {18, "Parameter Problem: %d"},
    {19, "Source Quench: %d"},
    {20, "Redirect: %d"},
    {21, "Echo Request: %d"},
    {22, "Echo Reply: %d"},
    {23, "Timestamp Request: %d"},
    {24, "Timestamp Reply: %d"},
    {25, "Address Mask Request: %d"},
    {26, "Addrss Mask Reply:%d"},
};

struct tcpconn_entry {
    oid             instance[10];
    struct in_addr  localAddress;
    int             locAddrSet;
    u_short         localPort;
    int             locPortSet;
    struct in_addr  remoteAddress;
    int             remAddrSet;
    u_short         remotePort;
    int             remPortSet;
    int             state;
    int             stateSet;
    struct tcpconn_entry *next;
};

struct udp_entry {
    oid             instance[5];
    struct in_addr  localAddress;
    int             locAddrSet;
    u_short         localPort;
    int             locPortSet;
    struct udp_entry *next;
};

#define TCPCONN_STATE	1
#define TCPCONN_LOCADDR	2
#define TCPCONN_LOCPORT	3
#define TCPCONN_REMADDR	4
#define TCPCONN_REMPORT	5

static oid      oid_tcpconntable[] = { 1, 3, 6, 1, 2, 1, 6, 13, 1 };
#define TCP_ENTRY 9

#define UDP_LOCADDR 	1
#define UDP_LOCPORT	2

static oid      oid_udptable[] = { 1, 3, 6, 1, 2, 1, 7, 5, 1 };
#define UDP_ENTRY 9

const char     *tcpstates[] = {
    "",
    "CLOSED",
    "LISTEN",
    "SYNSENT",
    "SYNRECEIVED",
    "ESTABLISHED",
    "FINWAIT1",
    "FINWAIT2",
    "CLOSEWAIT",
    "LASTACK",
    "CLOSING",
    "TIMEWAIT"
};
#define TCP_NSTATES 11

int             validUShortAssign(unsigned short *, int, const char *);

/*
 * Print a summary of connections related to an Internet
 * protocol (currently only TCP).  For TCP, also give state of connection.
 */
void
protopr(const char *name)
{
    struct tcpconn_entry *tcpconn = NULL, *tcplast = NULL, *tp, *newtp;
    struct udp_entry *udpconn = NULL, *udplast = NULL, *up, *newup;
    netsnmp_pdu    *request = NULL, *response = NULL;
    netsnmp_variable_list *vp;
    oid            *instance;
    int             first, status;


    response = NULL;
    if (strcmp(name, "tcp") == 0) {
        request = snmp_pdu_create(SNMP_MSG_GETNEXT);
        snmp_add_null_var(request, oid_tcpconntable,
                          sizeof(oid_tcpconntable) / sizeof(oid));
        status = STAT_SUCCESS;
    } else
        status = STAT_TIMEOUT;
    while (status == STAT_SUCCESS) {
        if (response)
            snmp_free_pdu(response);
        response = NULL;
        status = snmp_synch_response(Session, request, &response);
        if (status != STAT_SUCCESS
            || response->errstat != SNMP_ERR_NOERROR) {
            snmp_sess_perror("SNMP request failed", Session);
            break;
        }
        vp = response->variables;
        if (!vp)
            break;
        if (vp->name_length != 20 ||
            memcmp(vp->name, oid_tcpconntable, sizeof(oid_tcpconntable))) {
            break;
        }

        request = snmp_pdu_create(SNMP_MSG_GETNEXT);
        snmp_add_null_var(request, vp->name, vp->name_length);

        instance = vp->name + 10;
        for (tp = tcpconn; tp != NULL; tp = tp->next) {
            if (!memcmp(instance, tp->instance, sizeof(tp->instance)))
                break;
        }
        if (tp == NULL) {
            tp = (struct tcpconn_entry *) calloc(1,
                                                 sizeof(struct
                                                        tcpconn_entry));
            if (tp == NULL)
                break;
            if (tcplast != NULL)
                tcplast->next = tp;
            tcplast = tp;
            if (tcpconn == NULL)
                tcpconn = tp;
            memmove(tp->instance, instance, sizeof(tp->instance));
        }

        if (vp->name[TCP_ENTRY] == TCPCONN_STATE) {
            tp->state = *vp->val.integer;
            tp->stateSet = 1;
        }

        if (vp->name[TCP_ENTRY] == TCPCONN_LOCADDR) {
            memmove(&tp->localAddress, vp->val.string, sizeof(u_long));
            tp->locAddrSet = 1;
        }

        if (vp->name[TCP_ENTRY] == TCPCONN_LOCPORT) {
            if (validUShortAssign(&tp->localPort, *vp->val.integer,
                                  "TCPCONN_LOCPORT"))
                tp->locPortSet = 1;
        }

        if (vp->name[TCP_ENTRY] == TCPCONN_REMADDR) {
            memmove(&tp->remoteAddress, vp->val.string, sizeof(u_long));
            tp->remAddrSet = 1;
        }

        if (vp->name[TCP_ENTRY] == TCPCONN_REMPORT) {
            if (validUShortAssign(&tp->remotePort, *vp->val.integer,
                                  "TCPCONN_REMPORT"))
                tp->remPortSet = 1;
        }
    }
    if (response)
        snmp_free_pdu(response);
    response = NULL;

    for (first = 1, tp = tcpconn, newtp = NULL; tp != NULL; tp = tp->next) {
        if (newtp)
            free(newtp);
        newtp = tp;
        if (!(tp->stateSet && tp->locAddrSet
              && tp->locPortSet && tp->remAddrSet && tp->remPortSet)) {
            printf("incomplete entry\n");
            continue;
        }
        if (!aflag && tp->state == MIB_TCPCONNSTATE_LISTEN)
            continue;
        if (first) {
            printf("Active Internet (%s) Connections", name);
            if (aflag)
                printf(" (including servers)");
            putchar('\n');
            printf("%-5.5s %-28.28s %-28.28s %s\n",
                   "Proto", "Local Address", "Foreign Address", "(state)");
            first = 0;
        }
        printf("%-5.5s ", "tcp");
        inetprint(&tp->localAddress, tp->localPort, "tcp");
        inetprint(&tp->remoteAddress, tp->remotePort, "tcp");
        if (tp->state < 1 || tp->state > TCP_NSTATES)
            printf(" %d", tp->state);
        else
            printf(" %s", tcpstates[tp->state]);
        putchar('\n');
    }
    if (newtp)
        free(newtp);


    response = NULL;
    if (strcmp(name, "udp") == 0) {
        request = snmp_pdu_create(SNMP_MSG_GETNEXT);
        snmp_add_null_var(request, oid_udptable,
                          sizeof(oid_udptable) / sizeof(oid));
        status = STAT_SUCCESS;
    } else
        status = STAT_TIMEOUT;
    while (status == STAT_SUCCESS) {
        if (response)
            snmp_free_pdu(response);
        response = NULL;
        status = snmp_synch_response(Session, request, &response);
        if (status != STAT_SUCCESS
            || response->errstat != SNMP_ERR_NOERROR) {
            fprintf(stderr, "SNMP request failed\n");
            break;
        }
        vp = response->variables;
        if (!vp)
            break;
        if (vp->name_length != 15 ||
            memcmp(vp->name, oid_udptable, sizeof(oid_udptable))) {
            break;
        }

        request = snmp_pdu_create(SNMP_MSG_GETNEXT);
        snmp_add_null_var(request, vp->name, vp->name_length);

        instance = vp->name + 10;
        for (up = udpconn; up != NULL; up = up->next) {
            if (!memcmp(instance, up->instance, sizeof(up->instance)))
                break;
        }
        if (up == NULL) {
            up = (struct udp_entry *) calloc(1, sizeof(struct udp_entry));
            if (up == NULL)
                break;
            if (udplast != NULL)
                udplast->next = up;
            udplast = up;
            if (udpconn == NULL)
                udpconn = up;
            memmove(up->instance, instance, sizeof(up->instance));
        }

        if (vp->name[UDP_ENTRY] == UDP_LOCADDR) {
            memmove(&up->localAddress, vp->val.string, sizeof(u_long));
            up->locAddrSet = 1;
        }

        if (vp->name[UDP_ENTRY] == UDP_LOCPORT) {
            if (validUShortAssign(&up->localPort, *vp->val.integer,
                                  "UDP_LOCPORT"))
                up->locPortSet = 1;
        }
    }
    if (response)
        snmp_free_pdu(response);
    response = NULL;

    for (first = 1, up = udpconn, newup = NULL; up != NULL; up = up->next) {
        if (newup)
            free(newup);
        newup = up;
        if (!(up->locAddrSet && up->locPortSet)) {
            printf("incomplete entry\n");
            continue;
        }
        if (first) {
            printf("Active Internet (%s) Connections", name);
            putchar('\n');
            printf("%-5.5s %-28.28s\n", "Proto", "Local Address");
            first = 0;
        }
        printf("%-5.5s ", "udp");
        inetprint(&up->localAddress, up->localPort, "udp");
        putchar('\n');
    }
    if (newup)
        free(newup);

}

int
validUShortAssign(unsigned short *pushort, int ival, const char *errstr)
{
    u_long          ulval = (u_long) ival;
    if (ival > 65535) {
        printf("Warning: %s value %ld (0x%lx) is not a port address\n",
               errstr, ulval, ulval);
        return 0;
    }
    *pushort = (unsigned short) ulval;
    return 1;
}


/*
 * Dump UDP statistics structure.
 */
void
udp_stats(void)
{
    oid             varname[MAX_OID_LEN], *udpentry;
    size_t          varname_len;
    netsnmp_variable_list *var;
    int             count;
    struct stat_table *sp = udp_stattab;

    memmove(varname, oid_udpstats, sizeof(oid_udpstats));
    varname_len = sizeof(oid_udpstats) / sizeof(oid);
    udpentry = varname + 7;
    printf("udp:\n");
    count = sizeof(udp_stattab) / sizeof(struct stat_table);
    while (count--) {
        *udpentry = sp->entry;
        var = getvarbyname(Session, varname, varname_len);
        if (var && var->val.integer) {
            putchar('\t');
            printf(sp->description, *var->val.integer,
                   plural((int) *var->val.integer));
            putchar('\n');
        }
        if (var)
            snmp_free_var(var);
        sp++;
    }

}

/*
 * Dump TCP statistics structure.
 */
void
tcp_stats(void)
{
    oid             varname[MAX_OID_LEN], *tcpentry;
    size_t          varname_len;
    netsnmp_variable_list *var;
    int             count;
    struct stat_table *sp = tcp_stattab;

    memmove(varname, oid_tcpstats, sizeof(oid_tcpstats));
    varname_len = sizeof(oid_tcpstats) / sizeof(oid);
    tcpentry = varname + 7;
    printf("tcp:\n");
    count = sizeof(tcp_stattab) / sizeof(struct stat_table);
    while (count--) {
        *tcpentry = sp->entry;
        var = getvarbyname(Session, varname, varname_len);
        if (var && var->val.integer) {
            putchar('\t');
            printf(sp->description, *var->val.integer,
                   plural((int) *var->val.integer));
            putchar('\n');
        }
        if (var)
            snmp_free_var(var);
        sp++;
    }

}

/*
 * Dump IP statistics structure.
 */
void
ip_stats(void)
{
    oid             varname[MAX_OID_LEN], *ipentry;
    size_t          varname_len;
    netsnmp_variable_list *var;
    int             count;
    struct stat_table *sp = ip_stattab;

    memmove(varname, oid_ipstats, sizeof(oid_ipstats));
    varname_len = sizeof(oid_ipstats) / sizeof(oid);
    ipentry = varname + 7;
    printf("ip:\n");
    count = sizeof(ip_stattab) / sizeof(struct stat_table);
    while (count--) {
        *ipentry = sp->entry;
        var = getvarbyname(Session, varname, varname_len);
        if (var && var->val.integer) {
            putchar('\t');
            printf(sp->description, *var->val.integer,
                   plural((int) *var->val.integer));
            putchar('\n');
        }
        if (var)
            snmp_free_var(var);
        sp++;
    }

}

/*
 * Dump ICMP statistics.
 */
void
icmp_stats(void)
{
    oid             varname[MAX_OID_LEN], *icmpentry;
    size_t          varname_len;
    netsnmp_variable_list *var;
    int             count, first;
    struct stat_table *sp;

    memmove(varname, oid_icmpstats, sizeof(oid_icmpstats));
    varname_len = sizeof(oid_icmpstats) / sizeof(oid);
    icmpentry = varname + 7;
    printf("icmp:\n");
    sp = icmp_stattab;
    count = sizeof(icmp_stattab) / sizeof(struct stat_table);
    while (count--) {
        *icmpentry = sp->entry;
        var = getvarbyname(Session, varname, varname_len);
        if (var && var->val.integer) {
            putchar('\t');
            printf(sp->description, *var->val.integer,
                   plural((int) *var->val.integer));
            putchar('\n');
        }
        if (var)
            snmp_free_var(var);
        sp++;
    }

    sp = icmp_outhistogram;
    first = 1;
    count = sizeof(icmp_outhistogram) / sizeof(struct stat_table);
    while (count--) {
        *icmpentry = sp->entry;
        var = getvarbyname(Session, varname, varname_len);
        if (var && var->val.integer && *var->val.integer != 0) {
            if (first) {
                printf("\tOutput Histogram:\n");
                first = 0;
            }
            printf("\t\t");
            printf(sp->description, *var->val.integer,
                   plural((int) *var->val.integer));
            putchar('\n');
        }
        if (var)
            snmp_free_var(var);
        sp++;
    }

    sp = icmp_inhistogram;
    first = 1;
    count = sizeof(icmp_inhistogram) / sizeof(struct stat_table);
    while (count--) {
        *icmpentry = sp->entry;
        var = getvarbyname(Session, varname, varname_len);
        if (var && var->val.integer && *var->val.integer != 0) {
            if (first) {
                printf("\tInput Histogram:\n");
                first = 0;
            }
            printf("\t\t");
            printf(sp->description, *var->val.integer,
                   plural((int) *var->val.integer));
            putchar('\n');
        }
        if (var)
            snmp_free_var(var);
        sp++;
    }
}

/*
 * Pretty print an Internet address (net address + port).
 * If the nflag was specified, use numbers instead of names.
 */
void
inetprint(struct in_addr *in, u_short port, const char *proto)
{
    struct servent *sp = 0;
    char            line[80], *cp;
    int             width;

    sprintf(line, "%.*s.", 22, inetname(*in));
    cp = (char *) strchr(line, '\0');
    if (!nflag && port)
        sp = getservbyport(htons(port), proto);
    if (sp || port == 0)
        sprintf(cp, "%.8s", sp ? sp->s_name : "*");
    else
        sprintf(cp, "%d", port);
    width = 28;
    printf(" %-*.*s", width, width, line);
}

/*
 * Construct an Internet address representation.
 * If the nflag has been supplied, give 
 * numeric value, otherwise try for symbolic name.
 */
static char    *
inetname(struct in_addr in)
{
    register char  *cp;
    static char     line[50];
    struct hostent *hp;
    struct netent  *np;
    static char     domain[MAXHOSTNAMELEN + 1];
    static int      first = 1;

    if (first && !nflag) {
        first = 0;
        if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
            (cp = (char *) strchr(domain, '.')))
            (void) strcpy(domain, cp + 1);
        else
            domain[0] = 0;
    }
    cp = 0;
    if (!nflag && in.s_addr != INADDR_ANY) {
        u_long          net = inet_netof(in);
        u_long          lna = inet_lnaof(in);

        if (lna == INADDR_ANY) {
            np = getnetbyaddr(net, AF_INET);
            if (np)
                cp = np->n_name;
        }
        if (cp == 0) {
            hp = gethostbyaddr((char *) &in, sizeof(in), AF_INET);
            if (hp) {
                if ((cp = (char *) strchr(hp->h_name, '.')) &&
                    !strcmp(cp + 1, domain))
                    *cp = 0;
                cp = (char *) hp->h_name;
            }
        }
    }
    if (in.s_addr == INADDR_ANY)
        strcpy(line, "*");
    else if (cp) {
        strncpy(line, cp, sizeof(line));
        line[ sizeof(line)-1 ] = 0;
    } else {
        in.s_addr = ntohl(in.s_addr);
#define C(x)	(unsigned)((x) & 0xff)
        sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
                C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
    }
    return (line);
}
