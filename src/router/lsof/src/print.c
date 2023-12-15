/*
 * print.c - common print support functions for lsof
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#include "common.h"
#include "cli.h"
#include "proto.h"

/*
 * Local definitions, structures and function prototypes
 */

#define HCINC 64 /* host cache size increase chunk */
#define PORTHASHBUCKETS                                                        \
    128 /* port hash bucket count                                              \
         * !!MUST BE A POWER OF 2!! */
#define PORTTABTHRESH                                                          \
    10 /* threshold at which we will switch                                    \
        * from using getservbyport() to                                        \
        * getservent() -- see lkup_port()                                      \
        * and fill_porttab() */

struct hostcache {
    unsigned char a[MAX_AF_ADDR]; /* numeric address */
    int af;                       /* address family -- e.g., AF_INET
                                   * or AF_INET6 */
    char *name;                   /* name */
};

struct porttab {
    int port;
    MALLOC_S nl; /* name length (excluding '\0') */
    int ss;      /* service name status, 0 = lookup not
                  * yet performed */
    char *name;
    struct porttab *next;
};

#if defined(HASNORPC_H)
static struct porttab **Pth[2] = {NULL, NULL};
/* port hash buckets:
 * Pth[0] for TCP service names
 * Pth[1] for UDP service names
 */
#else  /* !defined(HASNORPC_H) */
static struct porttab **Pth[4] = {NULL, NULL, NULL, NULL};
/* port hash buckets:
 * Pth[0] for TCP service names
 * Pth[1] for UDP service names
 * Pth[2] for TCP portmap info
 * Pth[3] for UDP portmap info
 */
#endif /* defined(HASNORPC_H) */

#define HASHPORT(p) (((((int)(p)) * 31415) >> 3) & (PORTHASHBUCKETS - 1))

#if !defined(HASNORPC_H)
static void fill_portmap(struct lsof_context *ctx);
static void update_portmap(struct lsof_context *ctx, struct porttab *pt,
                           char *pn);
#endif /* !defined(HASNORPC_H) */

static void fill_porttab(struct lsof_context *ctx);
static char *lkup_port(struct lsof_context *ctx, int p, int pr, int src);
static char *lkup_svcnam(struct lsof_context *ctx, int h, int p, int pr,
                         int ss);
static int printinaddr(struct lsof_context *ctx);
static int human_readable_size(SZOFFTYPE sz, int print, int col);

#if !defined(HASNORPC_H)
/*
 * fill_portmap() -- fill the RPC portmap program name table via a conversation
 *		     with the portmapper
 *
 * The following copyright notice acknowledges that this function was adapted
 * from getrpcportnam() of the source code of the OpenBSD netstat program.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
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

static void fill_portmap(struct lsof_context *ctx) {
    static int already_run = 0;
    char buf[128], *cp, *nm;
    CLIENT *c;
    int h, port, pr;
    MALLOC_S nl;
    struct pmaplist *p = (struct pmaplist *)NULL;
    struct porttab *pt;
    struct rpcent *r;
    struct TIMEVAL_LSOF tm;

#    if !defined(CAN_USE_CLNT_CREATE)
    struct hostent *he;
    struct sockaddr_in ia;
    int s = RPC_ANYSOCK;
#    endif /* !defined(CAN_USE_CLNT_CREATE) */

    /*
     * Make sure this is only run once.
     */
    if (already_run)
        return;
    already_run = 1;

    /*
     * Construct structures for communicating with the portmapper.
     */

#    if !defined(CAN_USE_CLNT_CREATE)
    zeromem(&ia, sizeof(ia));
    ia.sin_family = AF_INET;
    if ((he = gethostbyname("localhost")))
        MEMMOVE((caddr_t)&ia.sin_addr, he->h_addr, he->h_length);
    ia.sin_port = htons(PMAPPORT);
#    endif /* !defined(CAN_USE_CLNT_CREATE) */

    tm.tv_sec = 60;
    tm.tv_usec = 0;
    /*
     * Get an RPC client handle.  Then ask for a dump of the port map.
     */

#    if defined(CAN_USE_CLNT_CREATE)
    if (!(c = clnt_create("localhost", PMAPPROG, PMAPVERS, "tcp")))
#    else  /* !defined(CAN_USE_CLNT_CREATE) */
    if (!(c = clnttcp_create(&ia, PMAPPROG, PMAPVERS, &s, 0, 0)))
#    endif /* defined(CAN_USE_CLNT_CREATE) */

        return;
    if (clnt_call(c, PMAPPROC_DUMP, XDR_VOID, NULL, XDR_PMAPLIST, (caddr_t)&p,
                  tm) != RPC_SUCCESS) {
        clnt_destroy(c);
        return;
    }
    /*
     * Loop through the port map dump, creating portmap table entries from TCP
     * and UDP members.
     */
    for (; p; p = p->pml_next) {

        /*
         * Determine the port map entry's protocol; ignore all but TCP and UDP.
         */
        if (p->pml_map.pm_prot == IPPROTO_TCP)
            pr = 2;
        else if (p->pml_map.pm_prot == IPPROTO_UDP)
            pr = 3;
        else
            continue;
        /*
         * See if there's already a portmap entry for this port.  If there is,
         * ignore this entry.
         */
        h = HASHPORT((port = (int)p->pml_map.pm_port));
        for (pt = Pth[pr][h]; pt; pt = pt->next) {
            if (pt->port == port)
                break;
        }
        if (pt)
            continue;
        /*
         * Save the registration name or number.
         */
        cp = (char *)NULL;
        if ((r = (struct rpcent *)getrpcbynumber(p->pml_map.pm_prog))) {
            if (r->r_name && strlen(r->r_name))
                cp = r->r_name;
        }
        if (!cp) {
            (void)snpf(buf, sizeof(buf), "%lu",
                       (unsigned long)p->pml_map.pm_prog);
            cp = buf;
        }
        if (!strlen(cp))
            continue;
        /*
         * Allocate space for the portmap name entry and copy it there.
         */
        if (!(nm = mkstrcpy(cp, &nl))) {
            (void)fprintf(stderr,
                          "%s: can't allocate space for portmap entry: ", Pn);
            safestrprt(cp, stderr, 1);
            Error(ctx);
        }
        if (!nl) {
            (void)free((FREE_P *)nm);
            continue;
        }
        /*
         * Allocate and fill a porttab struct entry for the portmap table.
         * Link it to the head of its hash bucket, and make it the new head.
         */
        if (!(pt = (struct porttab *)malloc(sizeof(struct porttab)))) {
            (void)fprintf(stderr,
                          "%s: can't allocate porttab entry for portmap: ", Pn);
            safestrprt(nm, stderr, 1);
            Error(ctx);
        }
        pt->name = nm;
        pt->nl = nl;
        pt->port = port;
        pt->next = Pth[pr][h];
        pt->ss = 0;
        Pth[pr][h] = pt;
    }
    clnt_destroy(c);
}
#endif /* !defined(HASNORPC_H) */

/*
 * fill_porttab() -- fill the TCP and UDP service name port table with a
 *		     getservent() scan
 */

static void fill_porttab(struct lsof_context *ctx) {
    int h, p, pr;
    MALLOC_S nl;
    char *nm;
    struct porttab *pt;
    struct servent *se;

    (void)endservent();
    /*
     * Scan the services data base for TCP and UDP entries that have a non-null
     * name associated with them.
     */
    (void)setservent(1);
    while ((se = getservent())) {
        if (!se->s_name || !se->s_proto)
            continue;
        if (strcasecmp(se->s_proto, "TCP") == 0)
            pr = 0;
        else if (strcasecmp(se->s_proto, "UDP") == 0)
            pr = 1;
        else
            continue;
        if (!se->s_name || !strlen(se->s_name))
            continue;
        p = ntohs(se->s_port);
        /*
         * See if a port->service entry is already cached for this port and
         * prototcol.  If it is, leave it alone.
         */
        h = HASHPORT(p);
        for (pt = Pth[pr][h]; pt; pt = pt->next) {
            if (pt->port == p)
                break;
        }
        if (pt)
            continue;
        /*
         * Add a new entry to the cache for this port and protocol.
         */
        if (!(nm = mkstrcpy(se->s_name, &nl))) {
            (void)fprintf(stderr,
                          "%s: can't allocate %d bytes for port %d name: %s\n",
                          Pn, (int)(nl + 1), p, se->s_name);
            Error(ctx);
        }
        if (!nl) {
            (void)free((FREE_P *)nm);
            continue;
        }
        if (!(pt = (struct porttab *)malloc(sizeof(struct porttab)))) {
            (void)fprintf(stderr,
                          "%s: can't allocate porttab entry for port %d: %s\n",
                          Pn, p, se->s_name);
            Error(ctx);
        }
        pt->name = nm;
        pt->nl = nl - 1;
        pt->port = p;
        pt->next = Pth[pr][h];
        pt->ss = 0;
        Pth[pr][h] = pt;
    }
    (void)endservent();
}

/*
 * gethostnm() - get host name
 */

char *gethostnm(struct lsof_context *ctx, /* context */
                unsigned char *ia,        /* Internet address */
                int af)                   /* address family -- e.g., AF_INET
                                           * or AF_INET6 */
{
    int al = MIN_AF_ADDR;
    char hbuf[256];
    static struct hostcache *hc = (struct hostcache *)NULL;
    static int hcx = 0;
    char *hn, *np;
    struct hostent *he = (struct hostent *)NULL;
    int i, j;
    MALLOC_S len;
    static int nhc = 0;
    /*
     * Search cache.
     */

#if defined(HASIPv6)
    if (af == AF_INET6)
        al = MAX_AF_ADDR;
#endif /* defined(HASIPv6) */

    for (i = 0; i < hcx; i++) {
        if (af != hc[i].af)
            continue;
        for (j = 0; j < al; j++) {
            if (ia[j] != hc[i].a[j])
                break;
        }
        if (j >= al)
            return (hc[i].name);
    }
    /*
     * If -n has been specified, construct a numeric address.  Otherwise, look
     * up host name by address.  If that fails, or if there is no name in the
     * returned hostent structure, construct a numeric version of the address.
     */
    if (Fhost)
        he = gethostbyaddr((char *)ia, al, af);
    if (!he || !he->h_name) {

#if defined(HASIPv6)
        if (af == AF_INET6) {

            /*
             * Since IPv6 numeric addresses use `:' as a separator, enclose
             * them in brackets.
             */
            hbuf[0] = '[';
            if (!inet_ntop(af, ia, hbuf + 1, sizeof(hbuf) - 3)) {
                (void)snpf(&hbuf[1], (sizeof(hbuf) - 1),
                           "can't format IPv6 address]");
            } else {
                len = strlen(hbuf);
                (void)snpf(&hbuf[len], sizeof(hbuf) - len, "]");
            }
        } else
#endif /* defined(HASIPv6) */

            if (af == AF_INET)
                (void)snpf(hbuf, sizeof(hbuf), "%u.%u.%u.%u", ia[0], ia[1],
                           ia[2], ia[3]);
            else
                (void)snpf(hbuf, sizeof(hbuf), "(unknown AF value: %d)", af);
        hn = hbuf;
    } else
        hn = (char *)he->h_name;
    /*
     * Allocate space for name and copy name to it.
     */
    if (!(np = mkstrcpy(hn, (MALLOC_S *)NULL))) {
        (void)fprintf(stderr, "%s: no space for host name: ", Pn);
        safestrprt(hn, stderr, 1);
        Error(ctx);
    }
    /*
     * Add address/name entry to cache.  Allocate cache space in HCINC chunks.
     */
    if (hcx >= nhc) {
        nhc += HCINC;
        len = (MALLOC_S)(nhc * sizeof(struct hostcache));
        if (!hc)
            hc = (struct hostcache *)malloc(len);
        else
            hc = (struct hostcache *)realloc((MALLOC_P *)hc, len);
        if (!hc) {
            (void)fprintf(stderr, "%s: no space for host cache\n", Pn);
            Error(ctx);
        }
    }
    hc[hcx].af = af;
    for (i = 0; i < al; i++) {
        hc[hcx].a[i] = ia[i];
    }
    hc[hcx++].name = np;
    return (np);
}

/*
 * lkup_port() - look up port for protocol
 */

static char *lkup_port(struct lsof_context *ctx, /* context */
                       int p,                    /* port number */
                       int pr,  /* protocol index: 0 = tcp, 1 = udp */
                       int src) /* port source: 0 = local
                                 *		1 = foreign */
{
    int h, nh;
    MALLOC_S nl;
    char *nm, *pn;
    static char pb[128];
    struct porttab *pt;
    /*
     * If the hash buckets haven't been allocated, do so.
     */
    if (!Pth[0]) {

#if defined(HASNORPC_H)
        nh = 2;
#else  /* !defined(HASNORPC_H) */
        nh = FportMap ? 4 : 2;
#endif /* defined(HASNORPC_H) */

        for (h = 0; h < nh; h++) {
            if (!(Pth[h] = (struct porttab **)calloc(
                      PORTHASHBUCKETS, sizeof(struct porttab *)))) {
                (void)fprintf(
                    stderr,
                    "%s: can't allocate %d bytes for %s %s hash buckets\n", Pn,
                    (int)(2 * (PORTHASHBUCKETS * sizeof(struct porttab *))),
                    (h & 1) ? "UDP" : "TCP", (h > 1) ? "portmap" : "port");
                Error(ctx);
            }
        }
    }

#if !defined(HASNORPC_H)
    /*
     * If we're looking up program names for portmapped ports, make sure the
     * portmap table has been loaded.
     */
    if (FportMap)
        (void)fill_portmap(ctx);
#endif /* !defined(HASNORPC_H) */

    /*
     * Hash the port and see if its name has been cached.  Look for a local
     * port first in the portmap, if portmap searching is enabled.
     */
    h = HASHPORT(p);

#if !defined(HASNORPC_H)
    if (!src && FportMap) {
        for (pt = Pth[pr + 2][h]; pt; pt = pt->next) {
            if (pt->port != p)
                continue;
            if (!pt->ss) {
                pn = Fport ? lkup_svcnam(ctx, h, p, pr, 0) : (char *)NULL;
                if (!pn) {
                    (void)snpf(pb, sizeof(pb), "%d", p);
                    pn = pb;
                }
                (void)update_portmap(ctx, pt, pn);
            }
            return (pt->name);
        }
    }
#endif /* !defined(HASNORPC_H) */

    for (pt = Pth[pr][h]; pt; pt = pt->next) {
        if (pt->port == p)
            return (pt->name);
    }
    /*
     * Search for a possible service name, unless the -P option has been
     * specified.
     *
     * If there is no service name, return a %d conversion.
     *
     * Don't cache %d conversions; a zero port number is a %d conversion that
     * is represented by "*".
     */
    pn = Fport ? lkup_svcnam(ctx, h, p, pr, 1) : (char *)NULL;
    if (!pn || !strlen(pn)) {
        if (p) {
            (void)snpf(pb, sizeof(pb), "%d", p);
            return (pb);
        } else
            return ("*");
    }
    /*
     * Allocate a new porttab entry for the TCP or UDP service name.
     */
    if (!(pt = (struct porttab *)malloc(sizeof(struct porttab)))) {
        (void)fprintf(stderr, "%s: can't allocate porttab entry for port %d\n",
                      Pn, p);
        Error(ctx);
    }
    /*
     * Allocate space for the name; copy it to the porttab entry; and link the
     * porttab entry to its hash bucket.
     *
     * Return a pointer to the name.
     */
    if (!(nm = mkstrcpy(pn, &nl))) {
        (void)fprintf(stderr, "%s: can't allocate space for port name: ", Pn);
        safestrprt(pn, stderr, 1);
        Error(ctx);
    }
    pt->name = nm;
    pt->nl = nl;
    pt->port = p;
    pt->next = Pth[pr][h];
    pt->ss = 0;
    Pth[pr][h] = pt;
    return (nm);
}

/*
 * lkup_svcnam() - look up service name for port
 */

static char *lkup_svcnam(struct lsof_context *ctx, /* context */
                         int h,                    /* porttab hash index */
                         int p,                    /* port number */
                         int pr, /* protocol: 0 = TCP, 1 = UDP */
                         int ss) /* search status: 1 = Pth[pr][h]
                                  *		  already searched */
{
    static int fl[PORTTABTHRESH];
    static int fln = 0;
    static int gsbp = 0;
    int i;
    struct porttab *pt;
    static int ptf = 0;
    struct servent *se;
    /*
     * Do nothing if -P has been specified.
     */
    if (!Fport)
        return ((char *)NULL);

    for (;;) {

        /*
         * Search service name cache, if it hasn't already been done.
         * Return the name of a match.
         */
        if (!ss) {
            for (pt = Pth[pr][h]; pt; pt = pt->next) {
                if (pt->port == p)
                    return (pt->name);
            }
        }
        /*
         * If fill_porttab() has been called, there is no service name.
         *
         * Do PORTTABTHRES getservbport() calls, remembering the failures, so
         * they won't be repeated.
         *
         * After PORTABTHRESH getservbyport() calls, call fill_porttab() once,
         */
        if (ptf)
            break;
        if (gsbp < PORTTABTHRESH) {
            for (i = 0; i < fln; i++) {
                if (fl[i] == p)
                    return ((char *)NULL);
            }
            gsbp++;
            if ((se = getservbyport(htons(p), pr ? "udp" : "tcp")))
                return (se->s_name);
            if (fln < PORTTABTHRESH)
                fl[fln++] = p;
            return ((char *)NULL);
        }
        (void)fill_porttab(ctx);
        ptf++;
        ss = 0;
    }
    return ((char *)NULL);
}

/*
 * print_file() - print file
 */

void print_file(struct lsof_context *ctx) {
    char buf[128];
    char *cp = (char *)NULL;
    dev_t dev;
    int devs, len;
    char access;
    char lock;
    char fd[FDLEN];
    char type[TYPEL];

    if (PrPass && !Hdr) {

        /*
         * Print the header line if this is the second pass and the
         * header hasn't already been printed.
         */
        (void)printf("%-*.*s %*s", CmdColW, CmdColW, CMDTTL, PidColW, PIDTTL);

#if defined(HASTASKS)
        if (TaskPrtTid)
            (void)printf(" %*s", TaskTidColW, TASKTIDTTL);
        if (TaskPrtCmd)
            (void)printf(" %-*.*s", TaskCmdColW, TaskCmdColW, TASKCMDTTL);
#endif /* defined(HASTASKS) */

#if defined(HASZONES)
        if (Fzone)
            (void)printf(" %-*s", ZoneColW, ZONETTL);
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
        if (Fcntx)
            (void)printf(" %-*s", CntxColW, CNTXTTL);
#endif /* defined(HASSELINUX) */

#if defined(HASPPID)
        if (Fppid)
            (void)printf(" %*s", PpidColW, PPIDTTL);
#endif /* defined(HASPPID) */

        if (Fpgid)
            (void)printf(" %*s", PgidColW, PGIDTTL);
        (void)printf(" %*s %*s   %*s", UserColW, USERTTL, FdColW - 2, FDTTL,
                     TypeColW, TYPETTL);

#if defined(HASFSTRUCT)
        if (Fsv) {

#    if !defined(HASNOFSADDR)
            if (Fsv & FSV_FA)
                (void)printf(" %*s", FsColW, FSTTL);
#    endif /* !defined(HASNOFSADDR) */

#    if !defined(HASNOFSCOUNT)
            if (Fsv & FSV_CT)
                (void)printf(" %*s", FcColW, FCTTL);
#    endif /* !defined(HASNOFSCOUNT) */

#    if !defined(HASNOFSFLAGS)
            if (Fsv & FSV_FG)
                (void)printf(" %*s", FgColW, FGTTL);
#    endif /* !defined(HASNOFSFLAGS) */

#    if !defined(HASNOFSNADDR)
            if (Fsv & FSV_NI)
                (void)printf(" %*s", NiColW, NiTtl);
#    endif /* !defined(HASNOFSNADDR) */
        }
#endif /* defined(HASFSTRUCT) */

        (void)printf(" %*s", DevColW, DEVTTL);
        if (Foffset)
            (void)printf(" %*s", SzOffColW, OFFTTL);
        else if (Fsize)
            (void)printf(" %*s", SzOffColW, SZTTL);
        else
            (void)printf(" %*s", SzOffColW, SZOFFTTL);
        if (Fnlink)
            (void)printf(" %*s", NlColW, NLTTL);
        (void)printf(" %*s %s\n", NodeColW, NODETTL, NMTTL);
        Hdr++;
    }
    /*
     * Size or print the command.
     *
     * CAUTION: command can be empty, see issue #246,
     * use NULL to represent failure instead of empty string
     */
    cp = Lp->cmd ? Lp->cmd : "(unknown)";
    if (!PrPass) {
        len = safestrlen(cp, 2);
        if (CmdLim && (len > CmdLim))
            len = CmdLim;
        if (len > CmdColW)
            CmdColW = len;
    } else
        safestrprtn(cp, CmdColW, stdout, 2);
    /*
     * Size or print the process ID.
     */
    if (!PrPass) {
        (void)snpf(buf, sizeof(buf), "%d", Lp->pid);
        if ((len = strlen(buf)) > PidColW)
            PidColW = len;
    } else
        (void)printf(" %*d", PidColW, Lp->pid);

#if defined(HASTASKS)
    /*
     * Size or print task ID and command name.
     */
    if (!PrPass) {
        if ((cp = Lp->tcmd)) {
            len = safestrlen(cp, 2);
            if (TaskCmdLim && (len > TaskCmdLim))
                len = TaskCmdLim;
            if (len > TaskCmdColW)
                TaskCmdColW = len;
            TaskPrtCmd = 1;
        }
        if (Lp->tid) {
            (void)snpf(buf, sizeof(buf), "%d", Lp->tid);
            if ((len = strlen(buf)) > TaskTidColW)
                TaskTidColW = len;
            TaskPrtTid = 1;
        }
    } else {
        if (TaskPrtTid) {
            if (Lp->tid)
                (void)printf(" %*d", TaskTidColW, Lp->tid);
            else
                (void)printf(" %*s", TaskTidColW, "");
        }
        if (TaskPrtCmd) {
            cp = Lp->tcmd ? Lp->tcmd : "";
            printf(" ");
            safestrprtn(cp, TaskCmdColW, stdout, 2);
        }
    }
#endif /* defined(HASTASKS) */

#if defined(HASZONES)
    /*
     * Size or print the zone.
     */
    if (Fzone) {
        if (!PrPass) {
            if (Lp->zn) {
                if ((len = strlen(Lp->zn)) > ZoneColW)
                    ZoneColW = len;
            }
        } else
            (void)printf(" %-*s", ZoneColW, Lp->zn ? Lp->zn : "");
    }
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
    /*
     * Size or print the context.
     */
    if (Fcntx) {
        if (!PrPass) {
            if (Lp->cntx) {
                if ((len = strlen(Lp->cntx)) > CntxColW)
                    CntxColW = len;
            }
        } else
            (void)printf(" %-*s", CntxColW, Lp->cntx ? Lp->cntx : "");
    }
#endif /* defined(HASSELINUX) */

#if defined(HASPPID)
    if (Fppid) {

        /*
         * Size or print the parent process ID.
         */
        if (!PrPass) {
            (void)snpf(buf, sizeof(buf), "%d", Lp->ppid);
            if ((len = strlen(buf)) > PpidColW)
                PpidColW = len;
        } else
            (void)printf(" %*d", PpidColW, Lp->ppid);
    }
#endif /* defined(HASPPID) */

    if (Fpgid) {

        /*
         * Size or print the process group ID.
         */
        if (!PrPass) {
            (void)snpf(buf, sizeof(buf), "%d", Lp->pgid);
            if ((len = strlen(buf)) > PgidColW)
                PgidColW = len;
        } else
            (void)printf(" %*d", PgidColW, Lp->pgid);
    }
    /*
     * Size or print the user ID or login name.
     */
    if (!PrPass) {
        if ((len = strlen(printuid(ctx, (UID_ARG)Lp->uid, NULL))) > UserColW)
            UserColW = len;
    } else
        (void)printf(" %*.*s", UserColW, UserColW,
                     printuid(ctx, (UID_ARG)Lp->uid, NULL));
    /*
     * Size or print the file descriptor, access mode and lock status.
     */
    fd_to_string(Lf->fd_type, Lf->fd_num, fd);
    access = access_to_char(Lf->access);
    lock = lock_to_char(Lf->lock);
    if (!PrPass) {
        (void)snpf(buf, sizeof(buf), "%s%c%c", fd,
                   (lock == ' ')     ? access
                   : (access == ' ') ? '-'
                                     : access,
                   lock);
        if ((len = strlen(buf)) > FdColW)
            FdColW = len;
    } else
        (void)printf(" %*.*s%c%c", FdColW - 2, FdColW - 2, fd,
                     (lock == ' ')     ? access
                     : (access == ' ') ? '-'
                                       : access,
                     lock);
    /*
     * Size or print the type.
     */
    file_type_to_string(Lf->type, Lf->unknown_file_type_number, type,
                        sizeof(type));
    if (!PrPass) {
        if ((len = strlen(type)) > TypeColW)
            TypeColW = len;
    } else
        (void)printf(" %*.*s", TypeColW, TypeColW, type);

#if defined(HASFSTRUCT)
    /*
     * Size or print the file structure address, file usage count, and node
     * ID (address).
     */

    if (Fsv) {

#    if !defined(HASNOFSADDR)
        if (Fsv & FSV_FA) {
            cp =
                (Lf->fsv & FSV_FA) ? print_kptr(Lf->fsa, buf, sizeof(buf)) : "";
            if (!PrPass) {
                if ((len = strlen(cp)) > FsColW)
                    FsColW = len;
            } else
                (void)printf(" %*.*s", FsColW, FsColW, cp);
        }
#    endif /* !defined(HASNOFSADDR) */

#    if !defined(HASNOFSCOUNT)
        if (Fsv & FSV_CT) {
            if (Lf->fsv & FSV_CT) {
                (void)snpf(buf, sizeof(buf), "%ld", Lf->fct);
                cp = buf;
            } else
                cp = "";
            if (!PrPass) {
                if ((len = strlen(cp)) > FcColW)
                    FcColW = len;
            } else
                (void)printf(" %*.*s", FcColW, FcColW, cp);
        }
#    endif /* !defined(HASNOFSCOUNT) */

#    if !defined(HASNOFSFLAGS)
        if (Fsv & FSV_FG) {
            if ((Lf->fsv & FSV_FG) && (FsvFlagX || Lf->ffg || Lf->pof))
                cp = print_fflags(ctx, Lf->ffg, Lf->pof);
            else
                cp = "";
            if (!PrPass) {
                if ((len = strlen(cp)) > FgColW)
                    FgColW = len;
            } else
                (void)printf(" %*.*s", FgColW, FgColW, cp);
        }
#    endif /* !defined(HASNOFSFLAGS) */

#    if !defined(HASNOFSNADDR)
        if (Fsv & FSV_NI) {
            cp =
                (Lf->fsv & FSV_NI) ? print_kptr(Lf->fna, buf, sizeof(buf)) : "";
            if (!PrPass) {
                if ((len = strlen(cp)) > NiColW)
                    NiColW = len;
            } else
                (void)printf(" %*.*s", NiColW, NiColW, cp);
        }
#    endif /* !defined(HASNOFSNADDR) */
    }
#endif /* defined(HASFSTRUCT) */

    /*
     * Size or print the device information.
     */

    if (Lf->rdev_def) {
        dev = Lf->rdev;
        devs = 1;
    } else if (Lf->dev_def) {
        dev = Lf->dev;
        devs = 1;
    } else
        devs = 0;
    if (devs) {

#if defined(HASPRINTDEV)
        cp = HASPRINTDEV(Lf, &dev);
#else  /* !defined(HASPRINTDEV) */
        (void)snpf(buf, sizeof(buf), "%u,%u", GET_MAJ_DEV(dev),
                   GET_MIN_DEV(dev));
        cp = buf;
#endif /* defined(HASPRINTDEV) */
    }

    if (!PrPass) {
        if (devs)
            len = strlen(cp);
        else if (Lf->dev_ch)
            len = strlen(Lf->dev_ch);
        else
            len = 0;
        if (len > DevColW)
            DevColW = len;
    } else {
        if (devs)
            (void)printf(" %*.*s", DevColW, DevColW, cp);
        else {
            if (Lf->dev_ch)
                (void)printf(" %*.*s", DevColW, DevColW, Lf->dev_ch);
            else
                (void)printf(" %*.*s", DevColW, DevColW, "");
        }
    }
    /*
     * Size or print the size or offset.
     */
    if (!PrPass) {
        if (!Foffset && Lf->sz_def) {
            if (Fhuman) {
                len = human_readable_size(Lf->sz, 0, 0);
            } else {
                (void)snpf(buf, sizeof(buf), SzOffFmt_d, Lf->sz);
                len = strlen(buf);
            }
        } else if (!Fsize && Lf->off_def) {

            (void)snpf(buf, sizeof(buf), SzOffFmt_0t, Lf->off);
            cp = buf;

            len = strlen(cp);
            if (OffDecDig && len > (OffDecDig + 2)) {

                (void)snpf(buf, sizeof(buf), SzOffFmt_x, Lf->off);
                cp = buf;

                len = strlen(cp);
            }
        } else
            len = 0;
        if (len > SzOffColW)
            SzOffColW = len;
    } else {
        putchar(' ');
        if (!Foffset && Lf->sz_def) {
            if (Fhuman) {
                human_readable_size(Lf->sz, 1, SzOffColW);
            } else {
                (void)snpf(buf, sizeof(buf), SzOffFmt_d, Lf->sz);
                len = strlen(buf);
                (void)printf(SzOffFmt_dv, SzOffColW, Lf->sz);
            }
        } else if (!Fsize && Lf->off_def) {

            (void)snpf(buf, sizeof(buf), SzOffFmt_0t, Lf->off);
            cp = buf;

            if (OffDecDig && (int)strlen(cp) > (OffDecDig + 2)) {

                (void)snpf(buf, sizeof(buf), SzOffFmt_x, Lf->off);
                cp = buf;
            }
            (void)printf("%*.*s", SzOffColW, SzOffColW, cp);
        } else
            (void)printf("%*.*s", SzOffColW, SzOffColW, "");
    }
    /*
     * Size or print the link count.
     */
    if (Fnlink) {
        if (Lf->nlink_def) {
            (void)snpf(buf, sizeof(buf), " %ld", Lf->nlink);
            cp = buf;
        } else
            cp = "";
        if (!PrPass) {
            if ((len = strlen(cp)) > NlColW)
                NlColW = len;
        } else
            (void)printf(" %*s", NlColW, cp);
    }
    /*
     * Size or print the inode information.
     */
    switch (Lf->inp_ty) {
    case 1:

#if defined(HASPRINTINO)
        cp = HASPRINTINO(Lf);
#else  /* !defined(HASPRINTINO) */
        (void)snpf(buf, sizeof(buf), InodeFmt_d, Lf->inode);
        cp = buf;
#endif /* defined(HASPRINTINO) */

        break;
    case 2:
        if (Lf->iproto[0])
            cp = Lf->iproto;
        else
            cp = "";
        break;
    case 3:
        (void)snpf(buf, sizeof(buf), InodeFmt_x, Lf->inode);
        cp = buf;
        break;
    default:
        cp = "";
    }
    if (!PrPass) {
        if ((len = strlen(cp)) > NodeColW)
            NodeColW = len;
    } else {
        (void)printf(" %*.*s", NodeColW, NodeColW, cp);
    }
    /*
     * If this is the second pass, print the name column.  (It doesn't need
     * to be sized.)
     */
    if (PrPass) {
        putchar(' ');

#if defined(HASPRINTNM)
        HASPRINTNM(ctx, Lf);
#else  /* !defined(HASPRINTNM) */
        printname(ctx, 1);
#endif /* defined(HASPRINTNM) */
    }
}

/*
 * printinaddr() - print Internet addresses
 */

static int printinaddr(struct lsof_context *ctx) {
    int i, len, src;
    char *host, *port;
    int nl = Namechl - 1;
    char *np = Namech;
    char pbuf[32];
    /*
     * Process local network address first.  If there's a foreign address,
     * separate it from the local address with "->".
     */
    for (i = 0, *np = '\0'; i < 2; i++) {
        if (!Lf->li[i].af)
            continue;
        host = port = (char *)NULL;
        if (i) {

            /*
             * If this is the foreign address, insert the separator.
             */
            if (nl < 2)

            addr_too_long :

            {
                (void)snpf(Namech, Namechl, "network addresses too long");
                return (1);
            }
                (void)snpf(np, nl, "->");
            np += 2;
            nl -= 2;
        }
        /*
         * Convert the address to a host name.
         */

#if defined(HASIPv6)
        if ((Lf->li[i].af == AF_INET6 &&
             IN6_IS_ADDR_UNSPECIFIED(&Lf->li[i].ia.a6)) ||
            (Lf->li[i].af == AF_INET && Lf->li[i].ia.a4.s_addr == INADDR_ANY))
            host = "*";
        else
            host = gethostnm(ctx, (unsigned char *)&Lf->li[i].ia, Lf->li[i].af);
#else  /* !defined(HASIPv6) */
        if (Lf->li[i].ia.a4.s_addr == INADDR_ANY)
            host = "*";
        else
            host = gethostnm((unsigned char *)&Lf->li[i].ia, Lf->li[i].af);
#endif /* defined(HASIPv6) */

        /*
         * Process the port number.
         */
        if (Lf->li[i].p > 0) {

            if (Fport

#if !defined(HASNORPC_H)
                || FportMap
#endif /* defined(HASNORPC_H) */

            ) {

                /*
                 * If converting port numbers to service names, or looking
                 * up portmap program names and numbers, do so by protocol.
                 *
                 * Identify the port source as local if: 1) it comes from the
                 * local entry (0) of the file's Internet address array; or
                 * 2) it comes from  the foreign entry (1), and the foreign
                 * Internet address matches the local one; or 3) it is the
                 * loopback address 127.0.0.1.  (Test 2 may not always work
                 * -- e.g., on hosts with multiple interfaces.)
                 */
#if !defined(HASNORPC_H)
                if ((src = i) && FportMap) {

#    if defined(HASIPv6)
                    if (Lf->li[0].af == AF_INET6) {
                        if (IN6_IS_ADDR_LOOPBACK(&Lf->li[i].ia.a6) ||
                            IN6_ARE_ADDR_EQUAL(&Lf->li[0].ia.a6,
                                               &Lf->li[1].ia.a6))
                            src = 0;
                    } else
#    endif /* defined(HASIPv6) */

                        if (Lf->li[0].af == AF_INET) {
                            if (Lf->li[i].ia.a4.s_addr ==
                                    htonl(INADDR_LOOPBACK) ||
                                Lf->li[0].ia.a4.s_addr ==
                                    Lf->li[1].ia.a4.s_addr)
                                src = 0;
                        }
                }
#else
                /*
                 * Just for suppressing warnings reported from compiler.
                 *
                 * src is referenced in lkup_port() only if
                 * "!defined(HASNORPC_H)" is true. The condition here is
                 * !defined(HASNORPC_H) is false. Therefore the value of src has
                 * no impact.
                 */
                src = 1;
#endif /* !defined(HASNORPC_H) */

                if (strcasecmp(Lf->iproto, "TCP") == 0)
                    port = lkup_port(ctx, Lf->li[i].p, 0, src);
                else if (strcasecmp(Lf->iproto, "UDP") == 0)
                    port = lkup_port(ctx, Lf->li[i].p, 1, src);
            }
            if (!port) {
                (void)snpf(pbuf, sizeof(pbuf), "%d", Lf->li[i].p);
                port = pbuf;
            }
        } else if (Lf->li[i].p == 0)
            port = "*";
        /*
         * Enter the host name.
         */
        if (host) {
            if ((len = strlen(host)) > nl)
                goto addr_too_long;
            if (len) {
                (void)snpf(np, nl, "%s", host);
                np += len;
                nl -= len;
            }
        }
        /*
         * Enter the port number, preceded by a colon.
         */
        if (port) {
            if (((len = strlen(port)) + 1) >= nl)
                goto addr_too_long;
            (void)snpf(np, nl, ":%s", port);
            np += len + 1;
            nl -= len - 1;
        }
    }
    if (Namech[0]) {
        safestrprt(Namech, stdout, 0);
        return (1);
    }
    return (0);
}

/*
 * print_init() - initialize for printing
 */

void print_init(struct lsof_context *ctx) {

    /*
     * Preset standard values.
     */
    PrPass = (Ffield || Fterse) ? 1 : 0;
    LastPid = -1;
    TaskPrtCmd = TaskPrtTid = 0;
    /*
     * Size columns by their titles.
     */
    CmdColW = strlen(CMDTTL);
    DevColW = strlen(DEVTTL);
    FdColW = strlen(FDTTL);
    if (Fnlink)
        NlColW = strlen(NLTTL);
    NmColW = strlen(NMTTL);
    NodeColW = strlen(NODETTL);
    PgidColW = strlen(PGIDTTL);
    PidColW = strlen(PIDTTL);
    PpidColW = strlen(PPIDTTL);
    if (Fsize)
        SzOffColW = strlen(SZTTL);
    else if (Foffset)
        SzOffColW = strlen(OFFTTL);
    else
        SzOffColW = strlen(SZOFFTTL);

#if defined(HASTASKS)
    TaskCmdColW = strlen(TASKCMDTTL);
    TaskTidColW = strlen(TASKTIDTTL);
#endif /* defined(HASTASKS) */

    TypeColW = strlen(TYPETTL);
    UserColW = strlen(USERTTL);

#if defined(HASFSTRUCT)

#    if !defined(HASNOFSADDR)
    FsColW = strlen(FSTTL);
#    endif /* !defined(HASNOFSADDR) */

#    if !defined(HASNOFSCOUNT)
    FcColW = strlen(FCTTL);
#    endif /* !defined(HASNOFSCOUNT) */

#    if !defined(HASNOFSFLAGS)
    FgColW = strlen(FGTTL);
#    endif /* !defined(HASNOFSFLAGS) */

#    if !defined(HASNOFSNADDR)
    NiColW = strlen(NiTtl);
#    endif /* !defined(HASNOFSNADDR) */
#endif     /* defined(HASFSTRUCT) */

#if defined(HASSELINUX)
    if (Fcntx)
        CntxColW = strlen(CNTXTTL);
#endif /* defined(HASSELINUX) */

#if defined(HASZONES)
    if (Fzone)
        ZoneColW = strlen(ZONETTL);
#endif /* defined(HASZONES) */
}

/*
 * printname() - print output name field
 */

void printname(struct lsof_context *ctx, int nl) /* NL status */
{

#if defined(HASNCACHE)
    char buf[MAXPATHLEN];
    char *cp;
    int fp;
#endif /* defined(HASNCACHE) */

    int ps = 0;

    if (Lf->nm && Lf->nm[0]) {

        /*
         * Print the name characters, if there are some.
         */
        safestrprt(Lf->nm, stdout, 0);
        ps++;
        if (!Lf->li[0].af && !Lf->li[1].af)
            goto print_nma;
    }
    if (Lf->li[0].af || Lf->li[1].af) {
        if (ps)
            putchar(' ');
        /*
         * If the file has Internet addresses, print them.
         */
        if (printinaddr(ctx))
            ps++;
        goto print_nma;
    }
    if (((Lf->ntype == N_BLK) || (Lf->ntype == N_CHR)) && Lf->dev_def &&
        Lf->rdev_def && printdevname(ctx, &Lf->dev, &Lf->rdev, 0, Lf->ntype)) {

        /*
         * If this is a block or character device and it has a name, print it.
         */
        ps++;
        goto print_nma;
    }
    if (Lf->is_com) {

        /*
         * If this is a common node, print that fact.
         */
        (void)fputs("COMMON: ", stdout);
        ps++;
        goto print_nma;
    }

#if defined(HASPRIVNMCACHE)
    if (HASPRIVNMCACHE(ctx, Lf)) {
        ps++;
        goto print_nma;
    }
#endif /* defined(HASPRIVNMCACHE) */

    if (Lf->lmi_srch) {
        struct mounts *mp;
        /*
         * Do a deferred local mount info table search for the file system
         * (mounted) directory name and inode number, and mounted device name.
         */
        for (mp = readmnt(ctx); mp; mp = mp->next) {
            if (Lf->dev == mp->dev) {
                Lf->fsdir = mp->dir;
                Lf->fsdev = mp->fsname;

#if defined(HASFSINO)
                Lf->fs_ino = mp->inode;
#endif /* defined(HASFSINO) */

                break;
            }
        }
        Lf->lmi_srch = 0;
    }
    if (Lf->fsdir || Lf->fsdev) {

        /*
         * Print the file system directory name, device name, and
         * possible path name components.
         */

#if !defined(HASNCACHE) || HASNCACHE < 2
        if (Lf->fsdir) {
            safestrprt(Lf->fsdir, stdout, 0);
            ps++;
        }
#endif /* !defined(HASNCACHE) || HASNCACHE<2 */

#if defined(HASNCACHE)

#    if HASNCACHE < 2
        if (Lf->na) {
            if (NcacheReload) {

#        if defined(NCACHELDPFX)
                NCACHELDPFX
#        endif /* defined(NCACHELDPFX) */

                    (void)
                ncache_load(ctx);

#        if defined(NCACHELDSFX)
                NCACHELDSFX
#        endif /* defined(NCACHELDSFX) */

                NcacheReload = 0;
            }
            if ((cp = ncache_lookup(ctx, buf, sizeof(buf), &fp))) {
                char *cp1;

                if (*cp == '\0')
                    goto print_nma;
                if (fp && Lf->fsdir) {
                    if (*cp != '/') {
                        cp1 = strrchr(Lf->fsdir, '/');
                        if (cp1 == (char *)NULL || *(cp1 + 1) != '\0')
                            putchar('/');
                    }
                } else
                    (void)fputs(" -- ", stdout);
                safestrprt(cp, stdout, 0);
                ps++;
                goto print_nma;
            }
        }
#    else /* HASNCACHE>1 */
        if (NcacheReload) {

#        if defined(NCACHELDPFX)
            NCACHELDPFX
#        endif /* defined(NCACHELDPFX) */

                (void)
            ncache_load();

#        if defined(NCACHELDSFX)
            NCACHELDSFX
#        endif /* defined(NCACHELDSFX) */

            NcacheReload = 0;
        }
        if ((cp = ncache_lookup(buf, sizeof(buf), &fp))) {
            if (fp) {
                safestrprt(cp, stdout, 0);
                ps++;
            } else {
                if (Lf->fsdir) {
                    safestrprt(Lf->fsdir, stdout, 0);
                    ps++;
                }
                if (*cp) {
                    (void)fputs(" -- ", stdout);
                    safestrprt(cp, stdout, 0);
                    ps++;
                }
            }
            goto print_nma;
        }
        if (Lf->fsdir) {
            safestrprt(Lf->fsdir, stdout, 0);
            ps++;
        }
#    endif     /* HASNCACHE<2 */
#endif         /* defined(HASNCACHE) */

        if (Lf->fsdev) {
            if (Lf->fsdir)
                (void)fputs(" (", stdout);
            else
                (void)putchar('(');
            safestrprt(Lf->fsdev, stdout, 0);
            (void)putchar(')');
            ps++;
        }
    }
    /*
     * Print the NAME column addition, if there is one.  If there isn't
     * make sure a NL is printed, as requested.
     */

print_nma:

    if (Lf->nma) {
        if (ps)
            putchar(' ');
        safestrprt(Lf->nma, stdout, 0);
        ps++;
    }
    /*
     * If this file has TCP/IP state information, print it.
     */
    if (!Ffield && Ftcptpi &&
        (Lf->lts.type >= 0

#if defined(HASTCPTPIQ)
         || ((Ftcptpi & TCPTPI_QUEUES) && (Lf->lts.rqs || Lf->lts.sqs))
#endif /* defined(HASTCPTPIQ) */

#if defined(HASTCPTPIW)
         || ((Ftcptpi & TCPTPI_WINDOWS) && (Lf->lts.rws || Lf->lts.wws))
#endif /* defined(HASTCPTPIW) */

             )) {
        if (ps)
            putchar(' ');
        (void)print_tcptpi(ctx, 0);
    }
    if (nl)
        putchar('\n');
}

/*
 * printrawaddr() - print raw socket address
 */

void printrawaddr(struct lsof_context *ctx,
                  struct sockaddr *sa) /* socket address */
{
    char *ep;
    size_t sz;

    ep = endnm(ctx, &sz);
    (void)snpf(ep, sz, "%u/%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
               sa->sa_family, (unsigned char)sa->sa_data[0],
               (unsigned char)sa->sa_data[1], (unsigned char)sa->sa_data[2],
               (unsigned char)sa->sa_data[3], (unsigned char)sa->sa_data[4],
               (unsigned char)sa->sa_data[5], (unsigned char)sa->sa_data[6],
               (unsigned char)sa->sa_data[7], (unsigned char)sa->sa_data[8],
               (unsigned char)sa->sa_data[9], (unsigned char)sa->sa_data[10],
               (unsigned char)sa->sa_data[11], (unsigned char)sa->sa_data[12],
               (unsigned char)sa->sa_data[13]);
}

/*
 * printsockty() - print socket type
 */

char *printsockty(int ty) /* socket type -- e.g., from so_type */
{
    static char buf[64];
    char *cp;

    switch (ty) {

#if defined(SOCK_STREAM)
    case SOCK_STREAM:
        cp = "STREAM";
        break;
#endif /* defined(SOCK_STREAM) */

#if defined(SOCK_STREAM)
    case SOCK_DGRAM:
        cp = "DGRAM";
        break;
#endif /* defined(SOCK_DGRAM) */

#if defined(SOCK_RAW)
    case SOCK_RAW:
        cp = "RAW";
        break;
#endif /* defined(SOCK_RAW) */

#if defined(SOCK_RDM)
    case SOCK_RDM:
        cp = "RDM";
        break;
#endif /* defined(SOCK_RDM) */

#if defined(SOCK_SEQPACKET)
    case SOCK_SEQPACKET:
        cp = "SEQPACKET";
        break;
#endif /* defined(SOCK_SEQPACKET) */

    default:
        (void)snpf(buf, sizeof(buf), "SOCK_%#x", ty);
        return (buf);
    }
    (void)snpf(buf, sizeof(buf), "SOCK_%s", cp);
    return (buf);
}

/*
 * printuid() - print User ID or login name
 */

char *printuid(struct lsof_context *ctx, /* context */
               UID_ARG uid,              /* User IDentification number */
               int *ty)                  /* returned UID type pointer (NULL
                                          * (if none wanted).  If non-NULL
                                          * then: *ty = 0 = login name
                                          *	     = 1 = UID number */
{
    int i;
    struct passwd *pw;
    struct stat sb;
    static struct stat sbs;
    static struct uidcache {
        uid_t uid;
        char nm[LOGINML + 1];
        struct uidcache *next;
    } **uc = (struct uidcache **)NULL;
    struct uidcache *up, *upn;
    static char user[USERPRTL + 1];

    if (Futol) {
        if (CkPasswd) {

            /*
             * Get the mtime and ctime of /etc/passwd, as required.
             */
            if (stat("/etc/passwd", &sb) != 0) {
                (void)fprintf(stderr, "%s: can't stat(/etc/passwd): %s\n", Pn,
                              strerror(errno));
                Error(ctx);
            }
        }
        /*
         * Define the UID cache, if necessary.
         */
        if (!uc) {
            if (!(uc = (struct uidcache **)calloc(UIDCACHEL,
                                                  sizeof(struct uidcache *)))) {
                (void)fprintf(
                    stderr, "%s: no space for %d byte UID cache hash buckets\n",
                    Pn, (int)(UIDCACHEL * (sizeof(struct uidcache *))));
                Error(ctx);
            }
            if (CkPasswd) {
                sbs = sb;
                CkPasswd = 0;
            }
        }
        /*
         * If it's time to check /etc/passwd and if its the mtime/ctime has
         * changed, destroy the existing UID cache.
         */
        if (CkPasswd) {
            if (sbs.st_mtime != sb.st_mtime || sbs.st_ctime != sb.st_ctime) {
                for (i = 0; i < UIDCACHEL; i++) {
                    if ((up = uc[i])) {
                        do {
                            upn = up->next;
                            (void)free((FREE_P *)up);
                        } while ((up = upn) != (struct uidcache *)NULL);
                        uc[i] = (struct uidcache *)NULL;
                    }
                }
                sbs = sb;
            }
            CkPasswd = 0;
        }
        /*
         * Search the UID cache.
         */
        i = (int)((((unsigned long)uid * 31415L) >> 7) & (UIDCACHEL - 1));
        for (up = uc[i]; up; up = up->next) {
            if (up->uid == (uid_t)uid) {
                if (ty)
                    *ty = 0;
                return (up->nm);
            }
        }
        /*
         * The UID is not in the cache.
         *
         * Look up the login name from the UID for a new cache entry.
         */
        if (!(pw = getpwuid((uid_t)uid))) {
            if (!Fwarn) {
                (void)fprintf(stderr, "%s: no pwd entry for UID %lu\n", Pn,
                              (unsigned long)uid);
            }
        } else {

            /*
             * Allocate and fill a new cache entry.  Link it to its hash bucket.
             */
            if (!(upn = (struct uidcache *)malloc(sizeof(struct uidcache)))) {
                (void)fprintf(
                    stderr, "%s: no space for UID cache entry for: %lu, %s)\n",
                    Pn, (unsigned long)uid, pw->pw_name);
                Error(ctx);
            }
            (void)strncpy(upn->nm, pw->pw_name, LOGINML);
            upn->nm[LOGINML] = '\0';
            upn->uid = (uid_t)uid;
            upn->next = uc[i];
            uc[i] = upn;
            if (ty)
                *ty = 0;
            return (upn->nm);
        }
    }
    /*
     * Produce a numeric conversion of the UID.
     */
    (void)snpf(user, sizeof(user), "%*lu", USERPRTL, (unsigned long)uid);
    if (ty)
        *ty = 1;
    return (user);
}

#if !defined(HASNORPC_H)
/*
 * update_portmap() - update a portmap entry with its port number or service
 *		      name
 */

static void update_portmap(struct lsof_context *ctx, /* context */
                           struct porttab *pt,       /* porttab entry */
                           char *pn)                 /* port name */
{
    MALLOC_S al, nl;
    char *cp;

    if (pt->ss)
        return;
    if (!(al = strlen(pn))) {
        pt->ss = 1;
        return;
    }
    nl = al + pt->nl + 2;
    if (!(cp = (char *)malloc(nl + 1))) {
        (void)fprintf(stderr,
                      "%s: can't allocate %d bytes for portmap name: %s[%s]\n",
                      Pn, (int)(nl + 1), pn, pt->name);
        Error(ctx);
    }
    (void)snpf(cp, nl + 1, "%s[%s]", pn, pt->name);
    (void)free((FREE_P *)pt->name);
    pt->name = cp;
    pt->nl = nl;
    pt->ss = 1;
}
#endif /* !defined(HASNORPC_H) */

/*
 * Convert sz to human readable format, print to stdout if print=1
 *
 * Return the length of output
 */
int human_readable_size(SZOFFTYPE sz, int print, int col) {
    char buf[128];
    SZOFFTYPE base = 1024;
    SZOFFTYPE unit = base;
    SZOFFTYPE upper = base * base;
    int suffix_count = 6;
    char *suffix[6] = {"K", "M", "G", "T", "P", "E"};
    int i;
    int len;
    double val;

    if (sz < base) {
        /* <1KB */
        (void)snpf(buf, sizeof(buf), "%" SZOFFPSPEC "uB", sz);
    } else {
        for (i = 0; i < suffix_count - 1; i++) {
            if (sz < upper) {
                break;
            }
            unit = upper;
            upper = upper * base;
        }

        /* Avoid floating point overflow */
        val = (double)(sz / (unit / base)) / base;
        (void)snpf(buf, sizeof(buf), "%.1lf%s", val, suffix[i]);
    }
    if (print) {
        printf("%*s", col, buf);
    }
    return strlen(buf);
}

/*
 * print_proc() - print process
 */
int print_proc(struct lsof_context *ctx) {
    char buf[128], *cp;
    int lc, len, st, ty;
    int rv = 0;
    unsigned long ul;
    char fd[FDLEN];
    char type[TYPEL];
    /*
     * If nothing in the process has been selected, skip it.
     */
    if (!Lp->pss)
        return (0);
    if (Fterse) {
        if (Lp->pid == LastPid) /* eliminate duplicates */
            return (0);
        LastPid = Lp->pid;
        /*
         * The mode is terse and something in the process appears to have
         * been selected.  Make sure of that by looking for a selected file,
         * so that the HASSECURITY and HASNOSOCKSECURITY option combination
         * won't produce a false positive result.
         */
        for (Lf = Lp->file; Lf; Lf = Lf->next) {
            if (is_file_sel(ctx, Lp, Lf)) {
                (void)printf("%d\n", Lp->pid);
                return (1);
            }
        }
        return (0);
    }
    /*
     * If fields have been selected, output the process-only ones, provided
     * that some file has also been selected.
     */
    if (Ffield) {
        for (Lf = Lp->file; Lf; Lf = Lf->next) {
            if (is_file_sel(ctx, Lp, Lf))
                break;
        }
        if (!Lf)
            return (rv);
        rv = 1;
        (void)printf("%c%d%c", LSOF_FID_PID, Lp->pid, Terminator);

#if defined(HASTASKS)
        if (FieldSel[LSOF_FIX_TID].st && Lp->tid)
            (void)printf("%c%d%c", LSOF_FID_TID, Lp->tid, Terminator);
        if (FieldSel[LSOF_FIX_TCMD].st && Lp->tcmd)
            (void)printf("%c%s%c", LSOF_FID_TCMD, Lp->tcmd, Terminator);
#endif /* defined(HASTASKS) */

#if defined(HASZONES)
        if (FieldSel[LSOF_FIX_ZONE].st && Fzone && Lp->zn)
            (void)printf("%c%s%c", LSOF_FID_ZONE, Lp->zn, Terminator);
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
        if (FieldSel[LSOF_FIX_CNTX].st && Fcntx && Lp->cntx && CntxStatus)
            (void)printf("%c%s%c", LSOF_FID_CNTX, Lp->cntx, Terminator);
#endif /* defined(HASSELINUX) */

        if (FieldSel[LSOF_FIX_PGID].st && Fpgid)
            (void)printf("%c%d%c", LSOF_FID_PGID, Lp->pgid, Terminator);

#if defined(HASPPID)
        if (FieldSel[LSOF_FIX_PPID].st && Fppid)
            (void)printf("%c%d%c", LSOF_FID_PPID, Lp->ppid, Terminator);
#endif /* defined(HASPPID) */

        if (FieldSel[LSOF_FIX_CMD].st) {
            putchar(LSOF_FID_CMD);
            safestrprt(Lp->cmd ? Lp->cmd : "(unknown)", stdout, 0);
            putchar(Terminator);
        }
        if (FieldSel[LSOF_FIX_UID].st)
            (void)printf("%c%d%c", LSOF_FID_UID, (int)Lp->uid, Terminator);
        if (FieldSel[LSOF_FIX_LOGIN].st) {
            cp = printuid(ctx, (UID_ARG)Lp->uid, &ty);
            if (ty == 0)
                (void)printf("%c%s%c", LSOF_FID_LOGIN, cp, Terminator);
        }
        if (Terminator == '\0')
            putchar('\n');
    }
    /*
     * Print files.
     */
    for (Lf = Lp->file; Lf; Lf = Lf->next) {
        if (!is_file_sel(ctx, Lp, Lf))
            continue;
        rv = 1;
        /*
         * If no field output selected, print dialect-specific formatted
         * output.
         */
        if (!Ffield) {
            print_file(ctx);
            continue;
        }
        lc = st = 0;
        if (FieldSel[LSOF_FIX_FD].st) {

            fd_to_string(Lf->fd_type, Lf->fd_num, fd);
            (void)printf("%c%s%c", LSOF_FID_FD, fd, Terminator);
            lc++;
        }
        /*
         * Print selected fields.
         */
        if (FieldSel[LSOF_FIX_ACCESS].st) {
            (void)printf("%c%c%c", LSOF_FID_ACCESS, access_to_char(Lf->access),
                         Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_LOCK].st) {
            (void)printf("%c%c%c", LSOF_FID_LOCK, lock_to_char(Lf->lock),
                         Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_TYPE].st) {
            if (Lf->type != LSOF_FILE_NONE) {
                file_type_to_string(Lf->type, Lf->unknown_file_type_number,
                                    type, TYPEL);
                (void)printf("%c%s%c", LSOF_FID_TYPE, type, Terminator);
                lc++;
            }
        }

#if defined(HASFSTRUCT)
        if (FieldSel[LSOF_FIX_FA].st && (Fsv & FSV_FA) && (Lf->fsv & FSV_FA)) {
            (void)printf("%c%s%c", LSOF_FID_FA,
                         print_kptr(Lf->fsa, (char *)NULL, 0), Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_CT].st && (Fsv & FSV_CT) && (Lf->fsv & FSV_CT)) {
            (void)printf("%c%ld%c", LSOF_FID_CT, Lf->fct, Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_FG].st && (Fsv & FSV_FG) && (Lf->fsv & FSV_FG) &&
            (FsvFlagX || Lf->ffg || Lf->pof)) {
            (void)printf("%c%s%c", LSOF_FID_FG,
                         print_fflags(ctx, Lf->ffg, Lf->pof), Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_NI].st && (Fsv & FSV_NI) && (Lf->fsv & FSV_NI)) {
            (void)printf("%c%s%c", LSOF_FID_NI,
                         print_kptr(Lf->fna, (char *)NULL, 0), Terminator);
            lc++;
        }
#endif /* defined(HASFSTRUCT) */

        if (FieldSel[LSOF_FIX_DEVCH].st && Lf->dev_ch && Lf->dev_ch[0]) {
            for (cp = Lf->dev_ch; *cp == ' '; cp++)
                ;
            if (*cp) {
                (void)printf("%c%s%c", LSOF_FID_DEVCH, cp, Terminator);
                lc++;
            }
        }
        if (FieldSel[LSOF_FIX_DEVN].st && Lf->dev_def) {
            if (sizeof(unsigned long) > sizeof(dev_t))
                ul = (unsigned long)((unsigned int)Lf->dev);
            else
                ul = (unsigned long)Lf->dev;
            (void)printf("%c0x%lx%c", LSOF_FID_DEVN, ul, Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_RDEV].st && Lf->rdev_def) {
            if (sizeof(unsigned long) > sizeof(dev_t))
                ul = (unsigned long)((unsigned int)Lf->rdev);
            else
                ul = (unsigned long)Lf->rdev;
            (void)printf("%c0x%lx%c", LSOF_FID_RDEV, ul, Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_SIZE].st && Lf->sz_def) {
            putchar(LSOF_FID_SIZE);

            (void)snpf(buf, sizeof(buf), SzOffFmt_d, Lf->sz);
            cp = buf;

            (void)printf("%s", cp);
            putchar(Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_OFFSET].st && Lf->off_def) {
            putchar(LSOF_FID_OFFSET);

            (void)snpf(buf, sizeof(buf), SzOffFmt_0t, Lf->off);
            cp = buf;

            len = strlen(cp);
            if (OffDecDig && len > (OffDecDig + 2)) {

                (void)snpf(buf, sizeof(buf), SzOffFmt_x, Lf->off);
                cp = buf;
            }
            (void)printf("%s", cp);
            putchar(Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_INODE].st && Lf->inp_ty == 1) {
            putchar(LSOF_FID_INODE);
            (void)printf(InodeFmt_d, Lf->inode);
            putchar(Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_NLINK].st && Lf->nlink_def) {
            (void)printf("%c%ld%c", LSOF_FID_NLINK, Lf->nlink, Terminator);
            lc++;
        }
        if (FieldSel[LSOF_FIX_PROTO].st && Lf->inp_ty == 2) {
            for (cp = Lf->iproto; *cp == ' '; cp++)
                ;
            if (*cp) {
                (void)printf("%c%s%c", LSOF_FID_PROTO, cp, Terminator);
                lc++;
            }
        }
        if (FieldSel[LSOF_FIX_STREAM].st && Lf->nm && Lf->is_stream) {
            if (strncmp(Lf->nm, "STR:", 4) == 0 ||
                strcmp(Lf->iproto, "STR") == 0) {
                putchar(LSOF_FID_STREAM);
                printname(ctx, 0);
                putchar(Terminator);
                lc++;
                st++;
            }
        }
        if (st == 0 && FieldSel[LSOF_FIX_NAME].st) {
            putchar(LSOF_FID_NAME);
            printname(ctx, 0);
            putchar(Terminator);
            lc++;
        }
        if (Lf->lts.type >= 0 && FieldSel[LSOF_FIX_TCPTPI].st) {
            print_tcptpi(ctx, 0);
            lc++;
        }
        if (Terminator == '\0' && lc)
            putchar('\n');
    }
    return (rv);
}

#if defined(HASFSTRUCT)
static char *alloc_fflbuf(struct lsof_context *ctx, char **bp, int *al, int lr);

/*
 * alloc_fflbuf() - allocate file flags print buffer
 */

static char *alloc_fflbuf(struct lsof_context *ctx,
                          char **bp, /* current buffer pointer */
                          int *al,   /* current allocated length */
                          int lr)    /* length required */
{
    int sz;

    sz = (int)(lr + 1); /* allocate '\0' space */
    if (*bp && (sz <= *al))
        return (*bp);
    if (*bp)
        *bp = (char *)realloc((MALLOC_P *)*bp, (MALLOC_S)sz);
    else
        *bp = (char *)malloc((MALLOC_S)sz);
    if (!*bp) {
        (void)fprintf(stderr, "%s: no space (%d) for print flags\n", Pn, sz);
        Error(ctx);
    }
    *al = sz;
    return (*bp);
}
#endif /* defined(HASFSTRUCT) */

#if defined(HASFSTRUCT)
/*
 * print_fflags() - print interpreted f_flag[s]
 */
char *print_fflags(struct lsof_context *ctx,
                   long ffg, /* file structure's flags value */
                   long pof) /* process open files flags value */
{
    int al, ct, fx;
    static int bl = 0;
    static char *bp = (char *)NULL;
    char *sep;
    int sepl;
    struct pff_tab *tp;
    long wf;
    char xbuf[64];
    /*
     * Reduce the supplied flags according to the definitions in Pff_tab[] and
     * Pof_tab[].
     */
    for (ct = fx = 0; fx < 2; fx++) {
        if (fx == 0) {
            sep = "";
            sepl = 0;
            tp = Pff_tab;
            wf = ffg;
        } else {
            sep = ";";
            sepl = 1;
            tp = Pof_tab;
            wf = pof;
        }
        for (; wf && !FsvFlagX; ct += al) {
            while (tp->nm) {
                if (wf & tp->val)
                    break;
                tp++;
            }
            if (!tp->nm)
                break;
            al = (int)strlen(tp->nm) + sepl;
            bp = alloc_fflbuf(ctx, &bp, &bl, al + ct);
            (void)snpf(bp + ct, al + 1, "%s%s", sep, tp->nm);
            sep = ",";
            sepl = 1;
            wf &= ~(tp->val);
        }
        /*
         * If flag bits remain, print them in hex.  If hex output was
         * specified with +fG, print all flag values, including zero,
         * in hex.
         */
        if (wf || FsvFlagX) {
            (void)snpf(xbuf, sizeof(xbuf), "0x%lx", wf);
            al = (int)strlen(xbuf) + sepl;
            bp = alloc_fflbuf(ctx, &bp, &bl, al + ct);
            (void)snpf(bp + ct, al + 1, "%s%s", sep, xbuf);
            ct += al;
        }
    }
    /*
     * Make sure there is at least a NUL terminated reply.
     */
    if (!bp) {
        bp = alloc_fflbuf(ctx, &bp, &bl, 0);
        *bp = '\0';
    }
    return (bp);
}
#endif /* defined(HASFSTRUCT) */