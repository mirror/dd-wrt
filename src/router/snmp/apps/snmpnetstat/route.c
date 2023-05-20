/*
 * $OpenBSD: route.c,v 1.66 2004/11/17 01:47:20 itojun Exp $
 */
/*
 * $NetBSD: route.c,v 1.15 1996/05/07 02:55:06 thorpej Exp $
 */

/*
 * Copyright (c) 1983, 1988, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <net-snmp/net-snmp-config.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <net-snmp/net-snmp-includes.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifndef INET
#define INET
#endif

#include "main.h"
#include "netstat.h"
#include "ffs.h"
#ifdef HAVE_WINSOCK_H
#include "winstub.h"
#endif

#define SET_MASK 0x01
#define SET_GWAY 0x02
#define SET_IFNO 0x04
#define SET_TYPE 0x08
#define SET_PRTO 0x10
#define SET_ALL  0x1f

struct route_entry {
    in_addr_t       destination;
    in_addr_t       mask;
    in_addr_t       gateway;
    int             ifNumber;
    int             type;
    int             proto;
    int             af;
    int             set_bits;
    char            ifname[64];
};

void            p_rtnode(struct route_entry *rp);

/*
 * Print routing tables.
 */
void
routepr(void)
{
    struct route_entry route, *rp = &route;
    oid             rtcol_oid[] = { 1, 3, 6, 1, 2, 1, 4, 21, 1, 0 };
    size_t          rtcol_len = OID_LENGTH(rtcol_oid);
    union {
        in_addr_t       addr;
        char            data[4];
    } tmpAddr;
    netsnmp_variable_list *var = NULL, *vp;
    char           *cp;

    printf("Routing tables (ipRouteTable)\n");
    pr_rthdr(AF_INET);

#define ADD_RTVAR( x ) rtcol_oid[ rtcol_len-1 ] = x; \
    snmp_varlist_add_variable( &var, rtcol_oid, rtcol_len, ASN_NULL, NULL,  0)
    ADD_RTVAR(2);               /* ipRouteIfIndex */
    ADD_RTVAR(7);               /* ipRouteNextHop */
    ADD_RTVAR(8);               /* ipRouteType    */
    ADD_RTVAR(9);               /* ipRouteProto   */
    ADD_RTVAR(11);              /* ipRouteMask    */
#undef ADD_RTVAR

    /*
     * Now walk the ipRouteTable, reporting the various route entries
     */
    while (1) {
        if (netsnmp_query_getnext(var, ss) != SNMP_ERR_NOERROR)
            break;
        rtcol_oid[rtcol_len - 1] = 2;   /* ifRouteIfIndex */
        if (snmp_oid_compare(rtcol_oid, rtcol_len,
                             var->name, rtcol_len) != 0)
            break;              /* End of Table */
        if (var->type == SNMP_NOSUCHOBJECT ||
            var->type == SNMP_NOSUCHINSTANCE ||
            var->type == SNMP_ENDOFMIBVIEW)
            break;
        memset(&route, 0, sizeof(struct route_entry));
        /*
         * Extract ipRouteDest index value
         */
        cp = tmpAddr.data;
        cp[0] = var->name[10] & 0xff;
        cp[1] = var->name[11] & 0xff;
        cp[2] = var->name[12] & 0xff;
        cp[3] = var->name[13] & 0xff;
        rp->destination = tmpAddr.addr;

        for (vp = var; vp; vp = vp->next_variable) {
            switch (vp->name[9]) {
            case 2:            /* ifRouteIfIndex */
                rp->ifNumber = *vp->val.integer;
                rp->set_bits |= SET_IFNO;
                break;
            case 7:            /* ipRouteNextHop */
                memmove(&rp->gateway, vp->val.string, 4);
                rp->set_bits |= SET_GWAY;
                break;
            case 8:            /* ipRouteType    */
                rp->type = *vp->val.integer;
                rp->set_bits |= SET_TYPE;
                break;
            case 9:            /* ipRouteProto   */
                rp->proto = *vp->val.integer;
                rp->set_bits |= SET_PRTO;
                break;
            case 11:           /* ipRouteMask    */
                memmove(&rp->mask, vp->val.string, 4);
                rp->set_bits |= SET_MASK;
                break;
            }
        }
        if (rp->set_bits != SET_ALL) {
            continue;           /* Incomplete query */
        }

        p_rtnode(rp);
    }
}


int
route4pr(int af)
{
    struct route_entry route, *rp = &route;
    oid             rtcol_oid[] = { 1, 3, 6, 1, 2, 1, 4, 24, 4, 1, 0 }; /* ipCidrRouteEntry */
    size_t          rtcol_len = OID_LENGTH(rtcol_oid);
    netsnmp_variable_list *var = NULL, *vp;
    union {
        in_addr_t       addr;
        unsigned char   data[4];
    } tmpAddr;
    int             printed = 0;
    int             hdr_af = AF_UNSPEC;

    if (af != AF_UNSPEC && af != AF_INET)
        return 0;

#define ADD_RTVAR( x ) rtcol_oid[ rtcol_len-1 ] = x; \
    snmp_varlist_add_variable( &var, rtcol_oid, rtcol_len, ASN_NULL, NULL,  0)
    ADD_RTVAR(5);               /* ipCidrRouteIfIndex */
    ADD_RTVAR(6);               /* ipCidrRouteType    */
    ADD_RTVAR(7);               /* ipCidrRouteProto   */
#undef ADD_RTVAR

    /*
     * Now walk the ipCidrRouteTable, reporting the various route entries
     */
    while (1) {
        oid            *op;
        unsigned char  *cp;

        if (netsnmp_query_getnext(var, ss) != SNMP_ERR_NOERROR)
            break;
        rtcol_oid[rtcol_len - 1] = 5;   /* ipRouteIfIndex */
        if (snmp_oid_compare(rtcol_oid, rtcol_len,
                             var->name, rtcol_len) != 0)
            break;              /* End of Table */
        if (var->type == SNMP_NOSUCHOBJECT ||
            var->type == SNMP_NOSUCHINSTANCE ||
            var->type == SNMP_ENDOFMIBVIEW)
            break;
        memset(&route, 0, sizeof(struct route_entry));
        rp->af = AF_INET;
        op = var->name + rtcol_len;
        cp = tmpAddr.data;
        cp[0] = *op++ & 0xff;
        cp[1] = *op++ & 0xff;
        cp[2] = *op++ & 0xff;
        cp[3] = *op++ & 0xff;
        rp->destination = tmpAddr.addr;
        cp = tmpAddr.data;
        cp[0] = *op++ & 0xff;
        cp[1] = *op++ & 0xff;
        cp[2] = *op++ & 0xff;
        cp[3] = *op++ & 0xff;
        rp->mask = tmpAddr.addr;
        op++;                   /* ipCidrRouteTos */
        cp = tmpAddr.data;
        cp[0] = *op++ & 0xff;
        cp[1] = *op++ & 0xff;
        cp[2] = *op++ & 0xff;
        cp[3] = *op++ & 0xff;
        rp->gateway = tmpAddr.addr;
        rp->set_bits = SET_MASK | SET_GWAY;

        for (vp = var; vp; vp = vp->next_variable) {
            switch (vp->name[rtcol_len - 1]) {
            case 5:            /* ipCidrRouteIfIndex */
                rp->ifNumber = *vp->val.integer;
                rp->set_bits |= SET_IFNO;
                break;
            case 6:            /* ipCidrRouteType    */
                rp->type = *vp->val.integer;
                rp->set_bits |= SET_TYPE;
                break;
            case 7:            /* ipCidrRouteProto   */
                rp->proto = *vp->val.integer;
                rp->set_bits |= SET_PRTO;
                break;
            }
        }
        if (rp->set_bits != SET_ALL) {
            continue;           /* Incomplete query */
        }

        if (hdr_af != rp->af) {
            if (hdr_af != AF_UNSPEC)
                printf("\n");
            hdr_af = rp->af;
            printf("Routing tables (ipCidrRouteTable)\n");
            pr_rthdr(hdr_af);
        }
        p_rtnode(rp);
        printed++;
    }
    snmp_free_varbind(var);
    return printed;
}


struct iflist {
    int             index;
    char            name[64];
    struct iflist  *next;
}              *Iflist = NULL;

void
get_ifname(char *name, int ifIndex)
{
    oid             ifdescr_oid[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 2, 0 };
    size_t          ifdescr_len = OID_LENGTH(ifdescr_oid);
    oid             ifxcol_oid[] = { 1, 3, 6, 1, 2, 1, 31, 1, 1, 1, 1, 0 };
    size_t          ifxcol_len = OID_LENGTH(ifxcol_oid);
    netsnmp_variable_list *var = NULL;
    struct iflist  *ip;

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

    ifxcol_oid[ifxcol_len - 1] = ifIndex;
    snmp_varlist_add_variable(&var, ifxcol_oid, ifxcol_len,
                              ASN_NULL, NULL, 0);
    if (netsnmp_query_get(var, ss) == SNMP_ERR_NOERROR) {
        if (var->val_len >= sizeof(ip->name))
            var->val_len = sizeof(ip->name) - 1;
        memmove(ip->name, var->val.string, var->val_len);
        ip->name[var->val_len] = '\0';
        strcpy(name, ip->name);
        snmp_free_varbind(var);
        return;
    }

    ifdescr_oid[ifdescr_len - 1] = ifIndex;
    snmp_varlist_add_variable(&var, ifdescr_oid, ifdescr_len,
                              ASN_NULL, NULL, 0);
    if (netsnmp_query_get(var, ss) == SNMP_ERR_NOERROR) {
        if (var->val_len >= sizeof(ip->name))
            var->val_len = sizeof(ip->name) - 1;
        memmove(ip->name, var->val.string, var->val_len);
        ip->name[var->val_len] = '\0';
        snmp_free_varbind(var);
    } else {
        sprintf(ip->name, "if%d", ifIndex);
    }
    strcpy(name, ip->name);
}

/*
 * column widths; each followed by one space
 */
#ifndef NETSNMP_ENABLE_IPV6
#define	WID_DST(af)	26      /* width of destination column */
#define	WID_GW(af)	18      /* width of gateway column */
#else
/*
 * width of destination/gateway column
 */
#if 1
/*
 * strlen("fe80::aaaa:bbbb:cccc:dddd@gif0") == 30, strlen("/128") == 4
 */
#define	WID_DST(af)	((af) == AF_INET6 ? (nflag ? 34 : 26) : 26)
#define	WID_GW(af)	((af) == AF_INET6 ? (nflag ? 30 : 18) : 18)
#else
/*
 * strlen("fe80::aaaa:bbbb:cccc:dddd") == 25, strlen("/128") == 4
 */
#define	WID_DST(af)	((af) == AF_INET6 ? (nflag ? 29 : 18) : 18)
#define	WID_GW(af)	((af) == AF_INET6 ? (nflag ? 25 : 18) : 18)
#endif
#endif                          /* NETSNMP_ENABLE_IPV6 */

/*
 * Print header for routing table columns.
 */
void
pr_rthdr(int af)
{
    /*
     * if (Aflag)
     * printf("%-*.*s ", PLEN, PLEN, "Address");
     * if (Sflag)
     * printf("%-*.*s ",
     * WID_DST(af), WID_DST(af), "Source");
     */
    printf("%-*.*s ", WID_DST(af), WID_DST(af), "Destination");
    printf("%-*.*s %-6.6s  %s\n",
           WID_GW(af), WID_GW(af), "Gateway", "Flags", "Interface");
}

#if 0
/*
 * Print header for PF_KEY entries.
 */
void
pr_encaphdr(void)
{
    if (Aflag)
        printf("%-*s ", PLEN, "Address");
    printf("%-18s %-5s %-18s %-5s %-5s %-22s\n",
           "Source", "Port", "Destination",
           "Port", "Proto", "SA(Address/Proto/Type/Direction)");
}
#endif

char           *
routename(in_addr_t in)
{
    char           *cp;
    static char     line[MAXHOSTNAMELEN];
    char            host[MAXHOSTNAMELEN];
    static char     domain[MAXHOSTNAMELEN];
    static int      first = 1;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    if (first) {
        first = 0;
        if (gethostname(line, sizeof line) == 0 &&
            (cp = strchr(line, '.')))
            strlcpy(domain, cp + 1, sizeof domain);
        else
            domain[0] = '\0';
    }
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = in;
    if (getnameinfo((struct sockaddr *) &sin, sizeof(sin), host,
                    sizeof(host), NULL, 0, nflag ? NI_NUMERICHOST : 0) < 0)
        strlcpy(host, "?", sizeof(host));
    if ((cp = strchr(host, '.')) && strcmp(cp + 1, domain) == 0)
        *cp = '\0';
    strlcpy(line, host, sizeof(line));
    return line;
}

#define C(x) (unsigned int)((x) & 0xffU)
/*
 * Return the name of the network whose address is given.
 * The address is assumed to be that of a net or subnet, not a host.
 */
char           *
netname(in_addr_t in, in_addr_t mask)
{
    char           *cp = NULL;
    static char     line[MAXHOSTNAMELEN];
    struct netent  *np = NULL;
    int             mbits;

    in = ntohl(in);
    mask = ntohl(mask);
    if (!nflag && in != INADDR_ANY) {
        if ((np = getnetbyaddr(in, AF_INET)) != NULL)
            cp = np->n_name;
    }
    mbits = mask ? 33 - _ffs(mask) : 0;
    if (cp) {
        strlcpy(line, cp, sizeof(line));
    } else if (mbits < 9)
        snprintf(line, sizeof line, "%u/%d", C(in >> 24), mbits);
    else if (mbits < 17)
        snprintf(line, sizeof line, "%u.%u/%d",
                 C(in >> 24), C(in >> 16), mbits);
    else if (mbits < 25)
        snprintf(line, sizeof line, "%u.%u.%u/%d",
                 C(in >> 24), C(in >> 16), C(in >> 8), mbits);
    else
        snprintf(line, sizeof line, "%u.%u.%u.%u/%d", C(in >> 24),
                 C(in >> 16), C(in >> 8), C(in), mbits);
    return (line);
}

#undef NETSNMP_ENABLE_IPV6
#ifdef NETSNMP_ENABLE_IPV6
char           *
netname6(struct sockaddr_in6 *sa6, struct in6_addr *mask)
{
    static char     line[MAXHOSTNAMELEN + 1];
    struct sockaddr_in6 sin6;
    u_char         *p;
    u_char         *lim;
    int             masklen, final = 0, illegal = 0;
    int             i;
    char            hbuf[NI_MAXHOST];
    int             flag = 0;
    int             error;

    sin6 = *sa6;

    masklen = 0;
    lim = (u_char *) (mask + 1);
    i = 0;
    if (mask) {
        for (p = (u_char *) mask; p < lim; p++) {
            if (final && *p) {
                illegal++;
                sin6.sin6_addr.s6_addr[i++] = 0x00;
                continue;
            }

            switch (*p & 0xff) {
            case 0xff:
                masklen += 8;
                break;
            case 0xfe:
                masklen += 7;
                final++;
                break;
            case 0xfc:
                masklen += 6;
                final++;
                break;
            case 0xf8:
                masklen += 5;
                final++;
                break;
            case 0xf0:
                masklen += 4;
                final++;
                break;
            case 0xe0:
                masklen += 3;
                final++;
                break;
            case 0xc0:
                masklen += 2;
                final++;
                break;
            case 0x80:
                masklen += 1;
                final++;
                break;
            case 0x00:
                final++;
                break;
            default:
                final++;
                illegal++;
                break;
            }

            if (!illegal)
                sin6.sin6_addr.s6_addr[i++] &= *p;
            else
                sin6.sin6_addr.s6_addr[i++] = 0x00;
        }
    } else
        masklen = 128;

    if (masklen == 0 && IN6_IS_ADDR_UNSPECIFIED(&sin6.sin6_addr))
        return ("default");

    if (illegal)
        fprintf(stderr, "illegal prefixlen\n");

    if (nflag)
        flag |= NI_NUMERICHOST;
    error = getnameinfo((struct sockaddr *) &sin6, sin6.sin6_len,
                        hbuf, sizeof(hbuf), NULL, 0, flag);
    if (error)
        snprintf(hbuf, sizeof(hbuf), "invalid");

    snprintf(line, sizeof(line), "%s/%d", hbuf, masklen);
    return line;
}

char           *
routename6(struct sockaddr_in6 *sa6)
{
    static char     line[NI_MAXHOST];
    const int       niflag = NI_NUMERICHOST;

    if (getnameinfo((struct sockaddr *) sa6, sa6->sin6_len,
                    line, sizeof(line), NULL, 0, niflag) != 0)
        strlcpy(line, "", sizeof line);
    return line;
}
#endif                          /*NETSNMP_ENABLE_IPV6 */

char           *
s_rtflags(struct route_entry *rp)
{
    static char     flag_buf[10];
    char           *cp = flag_buf;

    *cp++ = '<';
    *cp++ = 'U';                /* route is in use */
    if (rp->mask == 0xffffffff)
        *cp++ = 'H';            /* host */
    if (rp->proto == 4)
        *cp++ = 'D';            /* ICMP redirect */
    if (rp->type == 4)
        *cp++ = 'G';            /* remote destination/net */
    *cp++ = '>';
    *cp = 0;
    return flag_buf;
}

void
p_rtnode(struct route_entry *rp)
{
    get_ifname(rp->ifname, rp->ifNumber);
    printf("%-*.*s ",
           WID_DST(AF_INET), WID_DST(AF_INET),
           (rp->destination == INADDR_ANY) ? "default" :
           (rp->set_bits & SET_MASK) ?
           (rp->mask == 0xffffffff ?
            routename(rp->destination) :
            netname(rp->destination, rp->mask)) :
           netname(rp->destination, 0L));
    printf("%-*.*s %-6.6s  %s\n",
           WID_GW(af), WID_GW(af),
           rp->gateway ? routename(rp->gateway) : "*",
           s_rtflags(rp), rp->ifname);
}
