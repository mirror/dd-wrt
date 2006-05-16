/*****************************************************************
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
#include <ctype.h>

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
#define	LOOPBACKNET 127

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

struct route_entry {
    oid             instance[4];
    struct in_addr  destination;
    int             set_destination;
    struct in_addr  mask;
    int             set_mask;
    struct in_addr  gateway;
    int             set_gateway;
    int             ifNumber;
    int             set_ifNumber;
    int             type;
    int             set_type;
    int             proto;
    int             set_proto;
    char            ifname[64];
    int             set_name;
};

#define RTDEST	    1
#define RTIFINDEX   2
#define RTNEXTHOP   7
#define RTTYPE	    8
#define RTPROTO	    9
#define RTMASK	   11

static oid      oid_rttable[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1 };
static oid      oid_rtdest[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 1 };
static oid      oid_rtifindex[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 2 };
static oid      oid_rtnexthop[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 7 };
static oid      oid_rttype[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 8 };
static oid      oid_rtproto[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 9 };
static oid      oid_rtmask[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 11 };
static oid      oid_ifdescr[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 2 };
static oid      oid_ipnoroutes[] = { 1, 3, 6, 1, 2, 1, 4, 12, 0 };


/*
 * Print routing tables.
 */
void
routepr(void)
{
    struct route_entry route, *rp = &route;
    netsnmp_pdu    *request, *response;
    netsnmp_variable_list *vp;
    char            name[16], *flags;
    oid            *instance, type;
    int             toloopback, status;
    char            ch;

    printf("Routing tables\n");
    printf("%-26.26s %-18.18s %-6.6s  %s\n",
           "Destination", "Gateway", "Flags", "Interface");


    request = snmp_pdu_create(SNMP_MSG_GETNEXT);

    snmp_add_null_var(request, oid_rtdest,
                      sizeof(oid_rtdest) / sizeof(oid));
    snmp_add_null_var(request, oid_rtmask,
                      sizeof(oid_rtmask) / sizeof(oid));
    snmp_add_null_var(request, oid_rtifindex,
                      sizeof(oid_rtifindex) / sizeof(oid));
    snmp_add_null_var(request, oid_rtnexthop,
                      sizeof(oid_rtnexthop) / sizeof(oid));
    snmp_add_null_var(request, oid_rttype,
                      sizeof(oid_rttype) / sizeof(oid));
    snmp_add_null_var(request, oid_rtproto,
                      sizeof(oid_rtproto) / sizeof(oid));

    while (request) {
        status = snmp_synch_response(Session, request, &response);
        if (status != STAT_SUCCESS
            || response->errstat != SNMP_ERR_NOERROR) {
            fprintf(stderr, "SNMP request failed\n");
            break;
        }
        instance = NULL;
        request = NULL;
        rp->set_destination = 0;
        rp->set_mask = 0;
        rp->set_ifNumber = 0;
        rp->set_gateway = 0;
        rp->set_type = 0;
        rp->set_proto = 0;
        for (vp = response->variables; vp; vp = vp->next_variable) {
            if (vp->name_length != 14 ||
                memcmp(vp->name, oid_rttable, sizeof(oid_rttable))) {
                continue;       /* if it isn't in this subtree, just continue */
            }

            if (instance != NULL) {
                oid            *ip, *op;
                int             count;

                ip = instance;
                op = vp->name + 10;
                for (count = 0; count < 4; count++) {
                    if (*ip++ != *op++)
                        break;
                }
                if (count < 4)
                    continue;   /* not the right instance, ignore */
            } else {
                instance = vp->name + 10;
            }
            /*
             * At this point, this variable is known to be in the routing table
             * subtree, and is of the right instance for this transaction.
             */

            if (request == NULL)
                request = snmp_pdu_create(SNMP_MSG_GETNEXT);
            snmp_add_null_var(request, vp->name, vp->name_length);

            type = vp->name[9];
            switch ((char) type) {
            case RTDEST:
                memmove(&rp->destination, vp->val.string, sizeof(u_long));
                rp->set_destination = 1;
                break;
            case RTMASK:
                memmove(&rp->mask, vp->val.string, sizeof(u_long));
                rp->set_mask = 1;
                break;
            case RTIFINDEX:
                rp->ifNumber = *vp->val.integer;
                rp->set_ifNumber = 1;
                break;
            case RTNEXTHOP:
                memmove(&rp->gateway, vp->val.string, sizeof(u_long));
                rp->set_gateway = 1;
                break;
            case RTTYPE:
                rp->type = *vp->val.integer;
                rp->set_type = 1;
                break;
            case RTPROTO:
                rp->proto = *vp->val.integer;
                rp->set_proto = 1;
                break;
            }
        }
        snmp_free_pdu(response);
        if (!(rp->set_destination && rp->set_gateway
              && rp->set_type && rp->set_ifNumber)) {
            if (request)
                snmp_free_pdu(request);
            request = NULL;
            continue;
        }
        toloopback = *(char *) &rp->gateway == LOOPBACKNET;
        printf("%-26.26s ",
               (rp->destination.s_addr == INADDR_ANY) ? "default" :
               (toloopback) ? routename(rp->destination) :
               rp->set_mask ? netname(rp->destination, rp->mask.s_addr) :
               netname(rp->destination, 0L));
        printf("%-18.18s ", routename(rp->gateway));
        flags = name;
        *flags++ = 'U';         /* route is in use */
        /*
         * this !toloopback shouldnt be necessary 
         */
        if (!toloopback && rp->type == MIB_IPROUTETYPE_REMOTE)
            *flags++ = 'G';
        if (toloopback)
            *flags++ = 'H';
        if (rp->proto == MIB_IPROUTEPROTO_ICMP)
            *flags++ = 'D';     /* redirect */
        *flags = '\0';
        printf("%-6.6s ", name);
        get_ifname(rp->ifname, rp->ifNumber);
        ch = rp->ifname[strlen(rp->ifname) - 1];
        ch = '5';               /* force the if statement */
        if (isdigit(ch))
            printf(" %.32s\n", rp->ifname);
        else
            printf(" %.32s%d\n", rp->ifname, rp->ifNumber);

    }
}

struct iflist {
    int             index;
    char            name[64];
    struct iflist  *next;
}              *Iflist = NULL;

void
get_ifname(char *name, int ifIndex)
{
    netsnmp_pdu    *pdu, *response;
    netsnmp_variable_list *vp;
    struct iflist  *ip;
    oid             varname[MAX_OID_LEN];
    int             status;

    for (ip = Iflist; ip; ip = ip->next) {
        if (ip->index == ifIndex)
            break;
    }
    if (ip) {
        strcpy(name, ip->name);
        return;
    }
    ip = (struct iflist *) malloc(sizeof(struct iflist));
    if (ip == NULL)
        return;
    ip->next = Iflist;
    Iflist = ip;
    ip->index = ifIndex;
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    memmove(varname, oid_ifdescr, sizeof(oid_ifdescr));
    varname[10] = (oid) ifIndex;
    snmp_add_null_var(pdu, varname, sizeof(oid_ifdescr) / sizeof(oid) + 1);
    status = snmp_synch_response(Session, pdu, &response);
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
        vp = response->variables;
        if (vp->val_len >= sizeof(ip->name))
            vp->val_len = sizeof(ip->name) - 1;
        memmove(ip->name, vp->val.string, vp->val_len);
        ip->name[vp->val_len] = '\0';
        snmp_free_pdu(response);
    } else {
        sprintf(ip->name, "if%d", ifIndex);
    }
    strcpy(name, ip->name);
}

static          u_long
forgemask(u_long a)
{
    u_long          m;
    if (IN_CLASSA(a))
        m = IN_CLASSA_NET;
    else if (IN_CLASSB(a))
        m = IN_CLASSB_NET;
    else
        m = IN_CLASSC_NET;
    return m;
}

static void
domask(char *dst, u_long addr, u_long mask)
{
    int             b, i;
    if (!mask || forgemask(addr) == mask) {
        *dst = '\0';
        return;
    }
    i = 0;
    for (b = 0; b < 32; b++)
        if (mask & (1 << b)) {
            int             bb;
            i = b;
            for (bb = b + 1; bb < 32; bb++)
                if (!(mask & (1 << bb))) {
                    i = -1;     /* non-contig */
                    break;
                }
            break;
        }
    if (i == -1)
        sprintf(dst, "&0x%lx", mask);
    else
        sprintf(dst, "/%d", 32 - i);
}

char           *
routename(struct in_addr in)
{
    register char  *cp;
    static char     line[MAXHOSTNAMELEN + 1];
    struct hostent *hp;
    static char     domain[MAXHOSTNAMELEN + 1];
    static int      first = 1;

    if (first) {
        first = 0;
        if (gethostname(domain, MAXHOSTNAMELEN) == 0 &&
            (cp = (char *) strchr(domain, '.')))
            (void) strcpy(domain, cp + 1);
        else
            domain[0] = 0;
    }
    cp = 0;
    if (!nflag) {
        hp = gethostbyaddr((char *) &in, sizeof(struct in_addr), AF_INET);
        if (hp) {
            if ((cp = (char *) strchr(hp->h_name, '.')) &&
                !strcmp(cp + 1, domain))
                *cp = 0;
            cp = (char *) hp->h_name;
        }
    }
    if (cp)
        strncpy(line, cp, sizeof(line) - 1);
    else {
#define C(x)	(unsigned)((x) & 0xff)
        in.s_addr = ntohl(in.s_addr);
        sprintf(line, "%u.%u.%u.%u", C(in.s_addr >> 24),
                C(in.s_addr >> 16), C(in.s_addr >> 8), C(in.s_addr));
    }
    return (line);
}

/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
char           *
netname(struct in_addr in, u_long mask)
{
    char           *cp = NULL;
    static char     line[MAXHOSTNAMELEN + 1];
    struct netent  *np = 0;
    u_long          net, omask;
    u_long          i;
    int             subnetshift;

    i = ntohl(in.s_addr);
    omask = mask = ntohl(mask);
    if (!nflag && i != INADDR_ANY) {
        if (mask == INADDR_ANY) {
            if (IN_CLASSA(i)) {
                mask = IN_CLASSA_NET;
                subnetshift = 8;
            } else if (IN_CLASSB(i)) {
                mask = IN_CLASSB_NET;
                subnetshift = 8;
            } else {
                mask = IN_CLASSC_NET;
                subnetshift = 4;
            }
            /*
             * If there are more bits than the standard mask
             * would suggest, subnets must be in use.
             * Guess at the subnet mask, assuming reasonable
             * width subnet fields.
             */
            while (i & ~mask)
                mask = (long) mask >> subnetshift;
        }
        net = i & mask;
        while ((mask & 1) == 0)
            mask >>= 1, net >>= 1;
        np = getnetbyaddr(net, AF_INET);
        if (np)
            cp = np->n_name;
    }
    if (cp)
        strncpy(line, cp, sizeof(line) - 1);
    else if ((i & 0xffffff) == 0)
        sprintf(line, "%u", C(i >> 24));
    else if ((i & 0xffff) == 0)
        sprintf(line, "%u.%u", C(i >> 24), C(i >> 16));
    else if ((i & 0xff) == 0)
        sprintf(line, "%u.%u.%u", C(i >> 24), C(i >> 16), C(i >> 8));
    else
        sprintf(line, "%u.%u.%u.%u", C(i >> 24),
                C(i >> 16), C(i >> 8), C(i));
    domask(line + strlen(line), i, omask);
    return (line);
}

/*
 * Print routing statistics
 */
void
rt_stats(void)
{
    netsnmp_variable_list *var;

    printf("routing:\n");
    var =
        getvarbyname(Session, oid_ipnoroutes,
                     sizeof(oid_ipnoroutes) / sizeof(oid));
    if (var) {
        printf("\t%lu destination%s found unreachable\n",
               *var->val.integer, plural((int) *var->val.integer));
        snmp_free_var(var);
    } else {
        printf("\tCouldn't get ipOutNoRoutes variable\n");
    }
}

/*
 * Request a variable with a GET REQUEST message on the given
 * session.  If the variable is found, a
 * pointer to a netsnmp_variable_list object will be returned.
 * Otherwise, NULL is returned.  The caller must free the returned
 * variable_list object when done with it.
 */
netsnmp_variable_list *
getvarbyname(netsnmp_session * sp, oid * name, size_t len)
{
    netsnmp_pdu    *request, *response;
    netsnmp_variable_list *var = NULL, *vp;
    int             status;

    request = snmp_pdu_create(SNMP_MSG_GET);

    snmp_add_null_var(request, name, len);

    status = snmp_synch_response(sp, request, &response);

    if (status == STAT_SUCCESS) {
        if (response->errstat == SNMP_ERR_NOERROR) {
            for (var = response->variables; var; var = var->next_variable) {
                if (var->name_length == len
                    && !memcmp(name, var->name, len * sizeof(oid)))
                    break;      /* found our match */
            }
            if (var != NULL) {
                /*
                 * Now unlink this var from pdu chain so it doesn't get freed.
                 * The caller will free the var.
                 */
                if (response->variables == var) {
                    response->variables = var->next_variable;
                } else {
                    for (vp = response->variables; vp;
                         vp = vp->next_variable) {
                        if (vp->next_variable == var) {
                            vp->next_variable = var->next_variable;
                            break;
                        }
                    }
                }
                if (var->type == SNMP_NOSUCHOBJECT ||
                    var->type == SNMP_NOSUCHINSTANCE ||
                    var->type == SNMP_ENDOFMIBVIEW) {
                       snmp_free_var(var);
                       var =  NULL;
                }
            }
        }
    } else if (status != STAT_TIMEOUT)
        snmp_sess_perror("snmpnetstat", sp);
    if (response)
        snmp_free_pdu(response);
    return var;
}
