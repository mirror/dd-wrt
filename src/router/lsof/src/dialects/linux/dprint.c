/*
 * dprint.c - Linux printing functions for /proc-based lsof
 */

/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
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

#if defined(HASSOSTATE)
#    include <linux/net.h> /* for SS_* */
#endif                     /* defined(HASSOSTATE) */

#if defined(HASSOSTATE)
static char *socket_state_to_str(struct lsof_context *ctx, unsigned int ss);
#endif /* defined(HASSOSTATE) */

/*
 * print_unix() - print state of UNIX domain socket e.g. UNCONNECTED
 */
static void print_unix(struct lsof_context *ctx, int nl) {
    if (Ftcptpi & TCPTPI_STATE) {
#if defined(HASSOSTATE) && defined(HASSOOPT)
        char *cp = (Lf->lts.opt == __SO_ACCEPTCON)
                       ? "LISTEN"
                       : socket_state_to_str(ctx, Lf->lts.ss);

        if (Ffield)
            (void)printf("%cST=%s%c", LSOF_FID_TCPTPI, cp, Terminator);
        else {
            putchar('(');
            (void)fputs(cp, stdout);
            putchar(')');
        }
#endif /* defined(HASSOSTATE) && defined(HASSOOPT) */
    }
    if (nl)
        putchar('\n');
}

/*
 * print_tcptpi() - print TCP/TPI state e.g. ESTBALISHED
 */
void print_tcptpi(struct lsof_context *ctx, /* context */
                  int nl)                   /* 1 == '\n' required */
{
    char buf[128];
    char *cp = (char *)NULL;
    int ps = 0;
    int s;

    if (Lf->type == LSOF_FILE_UNIX) {
        print_unix(ctx, nl);
        return;
    }
    if ((Ftcptpi & TCPTPI_STATE) && Lf->lts.type == 0) {
        if (!TcpSt)
            (void)build_IPstates(ctx);
        if ((s = Lf->lts.state.i + TcpStOff) < 0 || s >= TcpNstates) {
            (void)snpf(buf, sizeof(buf), "UNKNOWN_TCP_STATE_%d",
                       Lf->lts.state.i);
            cp = buf;
        } else
            cp = TcpSt[s];
        if (cp) {
            if (Ffield)
                (void)printf("%cST=%s%c", LSOF_FID_TCPTPI, cp, Terminator);
            else {
                putchar('(');
                (void)fputs(cp, stdout);
            }
            ps++;
        }
    }

#if defined(HASTCPTPIQ)
    if (Ftcptpi & TCPTPI_QUEUES) {
        if (Lf->lts.rqs) {
            if (Ffield)
                putchar(LSOF_FID_TCPTPI);
            else {
                if (ps)
                    putchar(' ');
                else
                    putchar('(');
            }
            (void)printf("QR=%lu", Lf->lts.rq);
            if (Ffield)
                putchar(Terminator);
            ps++;
        }
        if (Lf->lts.sqs) {
            if (Ffield)
                putchar(LSOF_FID_TCPTPI);
            else {
                if (ps)
                    putchar(' ');
                else
                    putchar('(');
            }
            (void)printf("QS=%lu", Lf->lts.sq);
            if (Ffield)
                putchar(Terminator);
            ps++;
        }
    }
#endif /* defined(HASTCPTPIQ) */

#if defined(HASTCPTPIW)
    if (Ftcptpi & TCPTPI_WINDOWS) {
        if (Lf->lts.rws) {
            if (Ffield)
                putchar(LSOF_FID_TCPTPI);
            else {
                if (ps)
                    putchar(' ');
                else
                    putchar('(');
            }
            (void)printf("WR=%lu", Lf->lts.rw);
            if (Ffield)
                putchar(Terminator);
            ps++;
        }
        if (Lf->lts.wws) {
            if (Ffield)
                putchar(LSOF_FID_TCPTPI);
            else {
                if (ps)
                    putchar(' ');
                else
                    putchar('(');
            }
            (void)printf("WW=%lu", Lf->lts.ww);
            if (Ffield)
                putchar(Terminator);
            ps++;
        }
    }
#endif /* defined(HASTCPTPIW) */

    if (!Ffield && ps)
        putchar(')');
    if (nl)
        putchar('\n');
}

#if defined(HASSOSTATE)
/*
 * socket_state_to_str() -- convert socket state number to a string
 *
 * returns "UNKNOWN" for unknown state.
 */
static char *socket_state_to_str(struct lsof_context *ctx, unsigned int ss) {
    char *sr;
    switch (Lf->lts.ss) {
    case SS_UNCONNECTED:
        sr = "UNCONNECTED";
        break;
    case SS_CONNECTING:
        sr = "CONNECTING";
        break;
    case SS_CONNECTED:
        sr = "CONNECTED";
        break;
    case SS_DISCONNECTING:
        sr = "DISCONNECTING";
        break;
    default:
        sr = "UNKNOWN";
        break;
    }
    return sr;
}
#endif /* defined(HASSOSTATE) */