/*
 * proc.c - common process and file structure functions for lsof
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
#include "dlsof.h"
#include "lsof.h"
#include "proto.h"

#if defined(HASEPTOPTS)
static void prt_pinfo(struct lsof_context *ctx, pxinfo_t *pp, int ps);
static void prt_psxmqinfo(struct lsof_context *ctx, pxinfo_t *pp, int ps);
static void prt_evtfdinfo(struct lsof_context *ctx, pxinfo_t *pp, int ps);
#endif /* defined(HASEPTOPTS) */
#if defined(HASPTYEPT)
static void prt_ptyinfo(struct lsof_context *ctx, pxinfo_t *pp, int prt_edev,
                        int ps);
#endif /* defined(HASPTYEPT) */

/*
 * add_nma() - add to NAME column addition
 */

void add_nma(struct lsof_context *ctx, char *cp, /* string to add */
             int len)                            /* string length */
{
    int nl;
    char fd[FDLEN];

    if (!cp || !len)
        return;
    if (Lf->nma) {
        nl = (int)strlen(Lf->nma);
        Lf->nma =
            (char *)realloc((MALLOC_P *)Lf->nma, (MALLOC_S)(len + nl + 2));
    } else {
        nl = 0;
        Lf->nma = (char *)malloc((MALLOC_S)(len + 1));
    }
    if (!Lf->nma) {
        fd_to_string(Lf->fd_type, Lf->fd_num, fd);
        (void)fprintf(stderr, "%s: no name addition space: PID %ld, FD %s", Pn,
                      (long)Lp->pid, fd);
        Error(ctx);
    }
    if (nl) {
        Lf->nma[nl] = ' ';
        (void)strncpy(&Lf->nma[nl + 1], cp, len);
        Lf->nma[nl + 1 + len] = '\0';
    } else {
        (void)strncpy(Lf->nma, cp, len);
        Lf->nma[len] = '\0';
    }
}

/*
 * alloc_lfile() - allocate local file structure space
 */

void alloc_lfile(struct lsof_context *ctx,
                 enum lsof_fd_type fd_type, /* file descriptor type */
                 int num)                   /* file descriptor number -- -1 if
                                             * none */
{
    int fds;

    if (Lf) {
        /*
         * If reusing a previously allocated structure, release any allocated
         * space it was using.
         */
        if (Lf->dev_ch)
            (void)free((FREE_P *)Lf->dev_ch);
        if (Lf->nm)
            (void)free((FREE_P *)Lf->nm);
        if (Lf->nma)
            (void)free((FREE_P *)Lf->nma);

#if defined(HASLFILEADD) && defined(CLRLFILEADD)
        CLRLFILEADD(Lf)
#endif /* defined(HASLFILEADD) && defined(CLRLFILEADD) */

        /*
         * Othwerise, allocate a new structure.
         */
    } else if (!(Lf = (struct lfile *)malloc(sizeof(struct lfile)))) {
        (void)fprintf(stderr, "%s: no local file space at PID %d\n", Pn,
                      Lp->pid);
        Error(ctx);
    }
    /*
     * Initialize the structure.
     */
    Lf->access = LSOF_FILE_ACCESS_NONE;
    Lf->lock = LSOF_LOCK_NONE;
    Lf->dev_def = Lf->inp_ty = Lf->is_com = Lf->is_nfs = Lf->is_stream =
        Lf->lmi_srch = Lf->nlink_def = Lf->off_def = Lf->sz_def = Lf->rdev_def =
            (unsigned char)0;
    Lf->li[0].af = Lf->li[1].af = 0;
    Lf->lts.type = -1;
    Lf->nlink = 0l;

#if defined(HASMNTSTAT)
    Lf->mnt_stat = (unsigned char)0;
#endif /* defined(HASMNTSTAT) */

#if defined(HASEPTOPTS)
    Lf->chend = 0;
    Lf->eventfd_id = -1;
#    if defined(HASPTYEPT)
    Lf->tty_index = -1;
#    endif /* defined(HASPTYEPT) */
#endif     /* defined(HASEPTOPTS) */

#if defined(HASSOOPT)
    Lf->lts.kai = Lf->lts.ltm = 0;
    Lf->lts.opt = Lf->lts.qlen = Lf->lts.qlim = Lf->lts.pqlen = (unsigned int)0;
    Lf->lts.rbsz = Lf->lts.sbsz = (unsigned long)0;
    Lf->lts.qlens = Lf->lts.qlims = Lf->lts.pqlens = Lf->lts.rbszs =
        Lf->lts.sbszs = (unsigned char)0;
#endif /* defined(HASSOOPT) */

#if defined(HASSOSTATE)
    Lf->lts.ss = 0;
#endif /* defined(HASSOSTATE) */

#if defined(HASTCPOPT)
    Lf->lts.mss = (unsigned long)0;
    Lf->lts.msss = (unsigned char)0;
    Lf->lts.topt = (unsigned int)0;
#endif /* defined(HASTCPOPT) */

#if defined(HASTCPTPIQ)
    Lf->lts.rqs = Lf->lts.sqs = (unsigned char)0;
#endif /* defined(HASTCPTPIQ) */

#if defined(HASTCPTPIW)
    Lf->lts.rws = Lf->lts.wws = (unsigned char)0;
#endif /* defined(HASTCPTPIW) */

#if defined(HASFSINO)
    Lf->fs_ino = 0;
#endif /* defined(HASFSINO) */

#if defined(HASVXFS) && defined(HASVXFSDNLC)
    Lf->is_vxfs = 0;
#endif /* defined(HASVXFS) && defined(HASVXFSDNLC) */

    Lf->inode = (INODETYPE)0;
    Lf->off = (SZOFFTYPE)0;
    if (Lp->pss & PS_PRI)
        Lf->sf = Lp->sf;
    else
        Lf->sf = 0;
    Lf->iproto[0] = '\0';
    Lf->type = LSOF_FILE_NONE;
    Lf->unknown_file_type_number = 0;
    Lf->fd_type = fd_type;
    Lf->fd_num = num;
    Lf->dev_ch = Lf->fsdir = Lf->fsdev = Lf->nm = Lf->nma = (char *)NULL;
    Lf->ch = -1;

#if defined(HASNCACHE) && HASNCACHE < 2
    Lf->na = (KA_T)NULL;
#endif /* defined(HASNCACHE) && HASNCACHE<2 */

    Lf->next = (struct lfile *)NULL;
    Lf->ntype = Ntype = N_REGLR;
    Namech[0] = '\0';

#if defined(HASFSTRUCT)
    Lf->fct = Lf->ffg = Lf->pof = (long)0;
    Lf->fna = (KA_T)NULL;
    Lf->fsv = (unsigned char)0;
#endif /* defined(HASFSTRUCT) */

#if defined(HASLFILEADD) && defined(SETLFILEADD)
    /*
     * Do local initializations.
     */
    SETLFILEADD
#endif /* defined(HASLFILEADD) && defined(SETLFILEADD) */

    /*
     * See if the file descriptor has been selected.
     */
    if (!Fdl || (fd_type == LSOF_FD_NUMERIC && num < 0))
        return;
    fds = ck_fd_status(ctx, fd_type, num);
    switch (FdlTy) {
    case 0: /* inclusion list */
        if (fds == 2)
            Lf->sf |= SELFD;
        break;
    case 1: /* exclusion list */
        if (fds != 1)
            Lf->sf |= SELFD;
    }
}

/*
 * alloc_lproc() - allocate local proc structure space
 */

void alloc_lproc(struct lsof_context *ctx, int pid, /* Process ID */
                 int pgid,                          /* process group ID */
                 int ppid,                          /* parent process ID */
                 UID_ARG uid,                       /* User ID */
                 char *cmd,                         /* command */
                 int pss,                           /* process select state */
                 int sf)                            /* process select flags */
{
    static int sz = 0;

    if (!Lproc) {
        if (!(Lproc = (struct lproc *)malloc(
                  (MALLOC_S)(LPROCINCR * sizeof(struct lproc))))) {
            (void)fprintf(stderr,
                          "%s: no malloc space for %d local proc structures\n",
                          Pn, LPROCINCR);
            Error(ctx);
        }
        sz = LPROCINCR;
    } else if ((Nlproc + 1) > sz) {
        sz += LPROCINCR;
        if (!(Lproc = (struct lproc *)realloc(
                  (MALLOC_P *)Lproc, (MALLOC_S)(sz * sizeof(struct lproc))))) {
            (void)fprintf(stderr,
                          "%s: no realloc space for %d local proc structures\n",
                          Pn, sz);
            Error(ctx);
        }
    }
    Lp = &Lproc[Nlproc++];
    Lp->pid = pid;

#if defined(HASEPTOPTS)
    Lp->ept = 0;
#endif /* defined(HASEPTOPTS) */

#if defined(HASTASKS)
    Lp->tid = 0;
    Lp->tcmd = (char *)NULL;
#endif /* defined(HASTASKS) */

    Lp->pgid = pgid;
    Lp->ppid = ppid;
    Lp->file = (struct lfile *)NULL;
    Lp->sf = (short)sf;
    Lp->pss = (short)pss;
    Lp->uid = (uid_t)uid;
    /*
     * Allocate space for the full command name and copy it there.
     */
    if (!(Lp->cmd = mkstrcpy(cmd, (MALLOC_S *)NULL))) {
        (void)fprintf(stderr, "%s: PID %d, no space for command name: ", Pn,
                      pid);
        safestrprt(cmd, stderr, 1);
        Error(ctx);
    }

#if defined(HASZONES)
    /*
     * Clear the zone name pointer.  The dialect's own code will set it.
     */
    Lp->zn = (char *)NULL;
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
    /*
     * Clear the security context pointer.  The dialect's own code will
     * set it.
     */
    Lp->cntx = (char *)NULL;
#endif /* defined(HASSELINUX) */
}

/*
 * ck_fd_status() - check FD status
 *
 * return: 0 == FD is neither included nor excluded
 *	   1 == FD is excluded
 *	   2 == FD is included
 */

extern int ck_fd_status(struct lsof_context *ctx,
                        enum lsof_fd_type fd_type, /* file descriptor type */
                        int num) /* file descriptor number -- -1 if
                                  * none */
{
    struct fd_lst *fp;

    if (!(fp = Fdl) || (fd_type == LSOF_FD_NUMERIC && num < 0))
        return (0);
    /*
     * Check for an exclusion match.
     */
    if (FdlTy == 1) {
        for (; fp; fp = fp->next) {
            if (fp->fd_type != fd_type)
                continue;
            if (fp->fd_type == LSOF_FD_NUMERIC) {
                if (num >= fp->lo && num <= fp->hi)
                    return (1);
            } else {
                return (1);
            }
        }
        return (0);
    }
    /*
     * If Fdl isn't an exclusion list, check for an inclusion match.
     */
    for (; fp; fp = fp->next) {
        if (fp->fd_type != fd_type)
            continue;
        if (fp->fd_type == LSOF_FD_NUMERIC) {
            if (num >= fp->lo && num <= fp->hi)
                return (2);
        } else {
            return (2);
        }
    }
    return (0);
}

/*
 * comppid() - compare PIDs
 */

int comppid(COMP_P *a1, COMP_P *a2) {
    struct lproc **p1 = (struct lproc **)a1;
    struct lproc **p2 = (struct lproc **)a2;

    if ((*p1)->pid < (*p2)->pid)
        return (-1);
    if ((*p1)->pid > (*p2)->pid)
        return (1);

#if defined(HASTASKS)
    if ((*p1)->tid < (*p2)->tid)
        return (-1);
    if ((*p1)->tid > (*p2)->tid)
        return (1);
#endif /* defined(HASTASKS) */

    return (0);
}

/*
 * ent_inaddr() - enter Internet addresses
 */

void ent_inaddr(struct lsof_context *ctx,
                unsigned char *la, /* local Internet address */
                int lp,            /* local port */
                unsigned char *fa, /* foreign Internet address -- may
                                    * be NULL to indicate no foreign
                                    * address is known */
                int fp,            /* foreign port */
                int af)            /* address family -- e.g, AF_INET,
                                    * AF_INET */
{
    int m;

    if (la) {
        Lf->li[0].af = af;

#if defined(HASIPv6)
        if (af == AF_INET6)
            Lf->li[0].ia.a6 = *(struct in6_addr *)la;
        else
#endif /* defined(HASIPv6) */

            Lf->li[0].ia.a4 = *(struct in_addr *)la;
        Lf->li[0].p = lp;
    } else
        Lf->li[0].af = 0;
    if (fa) {
        Lf->li[1].af = af;

#if defined(HASIPv6)
        if (af == AF_INET6)
            Lf->li[1].ia.a6 = *(struct in6_addr *)fa;
        else
#endif /* defined(HASIPv6) */

            Lf->li[1].ia.a4 = *(struct in_addr *)fa;
        Lf->li[1].p = fp;
    } else
        Lf->li[1].af = 0;
    /*
     * If network address matching has been selected, check both addresses.
     */
    if ((Selflags & SELNA) && Nwad) {
        m = (fa && is_nw_addr(ctx, fa, fp, af)) ? 1 : 0;
        m |= (la && is_nw_addr(ctx, la, lp, af)) ? 1 : 0;
        if (m)
            Lf->sf |= SELNA;
    }
}

/*
 * examine_lproc() - examine local process
 *
 * return: 1 = last process
 */

int examine_lproc(struct lsof_context *ctx) {
    int sbp = 0;

    if (RptTm)
        return (0);
    /*
     * List the process if the process is selected and:
     *
     *	o  listing is limited to a single PID selection -- this one;
     *
     *	o  listing is selected by an ANDed option set (not all options)
     *	   that includes a single PID selection -- this one.
     */
    if ((Lp->sf & SELPID) && !AllProc) {
        if ((Selflags == SELPID) || (Fand && (Selflags & SELPID))) {
            sbp = 1;
            Npuns--;
        }
    }
    /*
     * Deprecate an unselected (or listed) process.
     */
    if (!Lp->pss) {
        (void)free_lproc(Lp);
        Nlproc--;
    }
    /*
     * Indicate last-process if listing is limited to PID selections,
     * and all selected processes have been listed.
     */
    return ((sbp && Npuns == 0) ? 1 : 0);
}

/*
 * free_lproc() - free lproc entry and its associated malloc'd space
 */

void free_lproc(struct lproc *lp) {
    struct lfile *lf, *nf;

    for (lf = lp->file; lf; lf = nf) {
        if (lf->dev_ch) {
            (void)free((FREE_P *)lf->dev_ch);
            lf->dev_ch = (char *)NULL;
        }
        if (lf->nm) {
            (void)free((FREE_P *)lf->nm);
            lf->nm = (char *)NULL;
        }
        if (lf->nma) {
            (void)free((FREE_P *)lf->nma);
            lf->nma = (char *)NULL;
        }

#if defined(HASLFILEADD) && defined(CLRLFILEADD)
        CLRLFILEADD(lf)
#endif /* defined(HASLFILEADD) && defined(CLRLFILEADD) */

        nf = lf->next;
        (void)free((FREE_P *)lf);
    }
    lp->file = (struct lfile *)NULL;
    if (lp->cmd) {
        (void)free((FREE_P *)lp->cmd);
        lp->cmd = (char *)NULL;
    }

#if defined(HASTASKS)
    if (lp->tcmd) {
        (void)free((FREE_P *)lp->tcmd);
        lp->tcmd = (char *)NULL;
    }
#endif /* defined(HASTASKS) */
}

/*
 * is_cmd_excl() - is command excluded?
 */

int is_cmd_excl(struct lsof_context *ctx, char *cmd, /* command name */
                short *pss,                          /* process state */
                short *sf)                           /* process select flags */
{
    int i;
    struct str_lst *sp;
    /*
     * See if the command is excluded by a "-c^<command>" option.
     */
    if (Cmdl && Cmdnx) {
        for (sp = Cmdl; sp; sp = sp->next) {
            if (sp->x && !strncmp(sp->str, cmd, sp->len))
                return (1);
        }
    }
    /*
     * The command is not excluded if no command selection was requested,
     * or if its name matches any -c <command> specification.
     *
     */
    if ((Selflags & SELCMD) == 0)
        return (0);
    for (sp = Cmdl; sp; sp = sp->next) {
        if (!sp->x && !strncmp(sp->str, cmd, sp->len)) {
            sp->f = 1;
            *pss |= PS_PRI;
            *sf |= SELCMD;
            return (0);
        }
    }
    /*
     * The command name doesn't match any -c <command> specification.  See if it
     * matches a -c /RE/[bix] specification.
     */
    for (i = 0; i < NCmdRxU; i++) {
        if (!regexec(&CmdRx[i].cx, cmd, 0, NULL, 0)) {
            CmdRx[i].mc = 1;
            *pss |= PS_PRI;
            *sf |= SELCMD;
            return (0);
        }
    }
    /*
     * The command name matches no -c specification.
     *
     * It's excluded if the only selection condition is command name,
     * or if command name selection is part of an ANDed set.
     */
    if (Selflags == SELCMD)
        return (1);
    return (Fand ? 1 : 0);
}

/*
 * is_file_sel() - is file selected?
 */

int is_file_sel(struct lsof_context *ctx, /* context */
                struct lproc *lp,         /* lproc structure pointer */
                struct lfile *lf)         /* lfile structure pointer */
{
    if (!lf || !lf->sf)
        return (0);
    if (lf->sf & SELEXCLF)
        return (0);

#if defined(HASSECURITY) && defined(HASNOSOCKSECURITY)
    if (Myuid && (Myuid != lp->uid)) {
        if (!(lf->sf & (SELNA | SELNET)))
            return (0);
    }
#endif /* defined(HASSECURITY) && defined(HASNOSOCKSECURITY) */

    if (AllProc)
        return (1);
    if (Fand && ((lf->sf & Selflags) != Selflags))
        return (0);
    return (1);
}

/*
 * is_proc_excl() - is process excluded?
 */

int is_proc_excl(struct lsof_context *ctx, int pid, /* Process ID */
                 int pgid,                          /* process group ID */
                 UID_ARG uid,                       /* User ID */
                 short *pss, /* process select state for lproc */
#if defined(HASTASKS)
                 short *sf, /* select flags for lproc */
                 int tid)   /* task ID (not a task if zero) */
#else
                 short *sf) /* select flags for lproc */
#endif /* defined(HASTASKS) */

{
    int i, j;

    *pss = *sf = 0;

#if defined(HASSECURITY)
/*
 * The process is excluded by virtue of the security option if it
 * isn't owned by the owner of this lsof process, unless the
 * HASNOSOCKSECURITY option is also specified.  In that case the
 * selected socket files of any process may be listed.
 */
#    if !defined(HASNOSOCKSECURITY)
    if (Myuid && Myuid != (uid_t)uid)
        return (1);
#    endif /* !defined(HASNOSOCKSECURITY) */
#endif     /* defined(HASSECURITY) */

    /*
     * If the excluding of process listing by UID has been specified, see if the
     * owner of this process is excluded.
     */
    if (Nuidexcl) {
        for (i = j = 0; (i < Nuid) && (j < Nuidexcl); i++) {
            if (!Suid[i].excl)
                continue;
            if (Suid[i].uid == (uid_t)uid)
                return (1);
            j++;
        }
    }
    /*
     * If the excluding of process listing by PGID has been specified, see if
     * this PGID is excluded.
     */
    if (Npgidx) {
        for (i = j = 0; (i < Npgid) && (j < Npgidx); i++) {
            if (!Spgid[i].x)
                continue;
            if (Spgid[i].i == pgid)
                return (1);
            j++;
        }
    }
    /*
     * If the excluding of process listing by PID has been specified, see if
     * this PID is excluded.
     */
    if (Npidx) {
        for (i = j = 0; (i < Npid) && (j < Npidx); i++) {
            if (!Spid[i].x)
                continue;
            if (Spid[i].i == pid)
                return (1);
            j++;
        }
    }
    /*
     * If the listing of all processes is selected, then this one is not
     * excluded.
     *
     * However, if HASSECURITY and HASNOSOCKSECURITY are both specified, exclude
     * network selections from the file flags, so that the tests in
     * is_file_sel() work as expected.
     */
    if (AllProc) {
        *pss = PS_PRI;

#if defined(HASSECURITY) && defined(HASNOSOCKSECURITY)
        *sf = SelAll & ~(SELNA | SELNET);
#else  /* !defined(HASSECURITY) || !defined(HASNOSOCKSECURITY) */
        *sf = SelAll;
#endif /* defined(HASSECURITY) && defined(HASNOSOCKSECURITY) */

        return (0);
    }
    /*
     * If the listing of processes has been specified by process group ID, see
     * if this one is included or excluded.
     */
    if (Npgidi && (Selflags & SELPGID)) {
        for (i = j = 0; (i < Npgid) && (j < Npgidi); i++) {
            if (Spgid[i].x)
                continue;
            if (Spgid[i].i == pgid) {
                Spgid[i].f = 1;
                *pss = PS_PRI;
                *sf = SELPGID;
                if (Selflags == SELPGID)
                    return (0);
                break;
            }
            j++;
        }
        if ((Selflags == SELPGID) && !*sf)
            return (1);
    }
    /*
     * If the listing of processes has been specified by PID, see if this one is
     * included or excluded.
     */
    if (Npidi && (Selflags & SELPID)) {
        for (i = j = 0; (i < Npid) && (j < Npidi); i++) {
            if (Spid[i].x)
                continue;
            if (Spid[i].i == pid) {
                Spid[i].f = 1;
                *pss = PS_PRI;
                *sf |= SELPID;
                if (Selflags == SELPID)
                    return (0);
                break;
            }
            j++;
        }
        if ((Selflags == SELPID) && !*sf)
            return (1);
    }
    /*
     * If the listing of processes has been specified by UID, see if the owner
     * of this process has been included.
     */
    if (Nuidincl && (Selflags & SELUID)) {
        for (i = j = 0; (i < Nuid) && (j < Nuidincl); i++) {
            if (Suid[i].excl)
                continue;
            if (Suid[i].uid == (uid_t)uid) {
                Suid[i].f = 1;
                *pss = PS_PRI;
                *sf |= SELUID;
                if (Selflags == SELUID)
                    return (0);
                break;
            }
            j++;
        }
        if (Selflags == SELUID && (*sf & SELUID) == 0)
            return (1);
    }

#if defined(HASTASKS)
    if ((Selflags & SELTASK) && tid) {

        /*
         * This is a task and tasks are selected.
         */
        *pss = PS_PRI;
        *sf |= SELTASK;
        if ((Selflags == SELTASK) || (Fand && ((*sf & Selflags) == Selflags)))
            return (0);
    }
#endif /* defined(HASTASKS) */

    /*
     * When neither the process group ID, nor the PID, nor the task, nor the UID
     * is selected:
     *
     *	If list option ANDing of process group IDs, PIDs, UIDs or tasks is
     *	specified, the process is excluded;
     *
     *	Otherwise, it's not excluded by the tests of this function.
     */
    if (!*sf)
        return ((Fand && (Selflags & (SELPGID | SELPID | SELUID | SELTASK)))
                    ? 1
                    : 0);
    /*
     * When the process group ID, PID, task or UID is selected and the process
     * group ID, PID, task or UID list option has been specified:
     *
     *	If list option ANDing has been specified, and the correct
     *	combination of selections are in place, reply that the process is no
     *	excluded;
     * or
     *	If list option ANDing has not been specified, reply that the
     *	process is not excluded by the tests of this function.
     */
    if (Selflags & (SELPGID | SELPID | SELUID | SELTASK)) {
        if (Fand)
            return (((Selflags & (SELPGID | SELPID | SELUID | SELTASK)) != *sf)
                        ? 1
                        : 0);
        return (0);
    }
    /*
     * Finally, when neither the process group ID, nor the PID, nor the UID, nor
     * the task is selected, and no applicable list option has been specified:
     *
     *	If list option ANDing has been specified, this process is
     *	excluded;
     *
     *	Otherwise, it isn't excluded by the tests of this function.
     */
    return (Fand ? 1 : 0);
}

/*
 * link_lfile() - link local file structures
 */

void link_lfile(struct lsof_context *ctx) {
    if (Lf->sf & SELEXCLF)
        return;

#if defined(HASEPTOPTS)
    /*
     * If endpoint info has been requested, clear the SELPINFO flag from the
     * local pipe file structure, since it was set only to insure this file
     * would be linked.  While this might leave no file selection flags set, a
     * later call to the process_pinfo() function might set some.  Also set the
     * EPT_PIPE flag.
     */
    if (FeptE) {
        if (Lf->sf & SELPINFO) {
            Lp->ept |= EPT_PIPE;
            Lf->sf &= ~SELPINFO;
        }

        /*
         * Process posix mq endpoint files the same way by clearing the
         * SELPSXMQINFO flag and setting the EPT_PSXMQ flag, letting a later
         * call to process_psxmqinfo() set selection flags.
         */
        if (Lf->sf & SELPSXMQINFO) {
            Lp->ept |= EPT_PSXMQ;
            Lf->sf &= ~SELPSXMQINFO;
        }

#    if defined(HASUXSOCKEPT)
        /*
         * Process UNIX socket endpoint files the same way by clearing the
         * SELUXINFO flag and setting the EPT_UXS flag, letting a later call to
         * process_uxsinfo() set selection flags.
         */
        if (Lf->sf & SELUXSINFO) {
            Lp->ept |= EPT_UXS;
            Lf->sf &= ~SELUXSINFO;
        }
#    endif /* defined(HASUXSOCKEPT) */

#    if defined(HASPTYEPT)
        /*
         * Process pseudoterminal endpoint files the same way by clearing the
         * SELPTYINFO flag and setting the EPT_PTY flag, letting a later call to
         * process_ptyinfo() set selection flags.
         */
        if (Lf->sf & SELPTYINFO) {
            Lp->ept |= EPT_PTY;
            Lf->sf &= ~SELPTYINFO;
        }
#    endif /* defined(HASPTYEPT) */

        /*
         * Process locally used INET socket endpoint files the same way by
         * clearing the SENETSINFO flag and setting the EPT_NETS flag, letting a
         * later call to process_netsinfo() set selection flags.
         */
        if (Lf->sf & SELNETSINFO) {
            Lp->ept |= EPT_NETS;
            Lf->sf &= ~SELNETSINFO;
        }

#    if defined(HASIPv6)
        /*
         * Process locally used INET6 socket endpoint files the same way by
         * clearing the SENETS6INFO flag and setting the EPT_NETS6 flag, letting
         * a later call to process_nets6info() set selection flags.
         */
        if (Lf->sf & SELNETS6INFO) {
            Lp->ept |= EPT_NETS6;
            Lf->sf &= ~SELNETS6INFO;
        }
#    endif /* defined(HASIPv6) */
           /*
            * Process eventfd endpoint files the same way by clearing the
            * SELEVTFDINFO    flag and setting the EPT_EVTFD flag, letting a later
            * call to    process_evtfdinfo()      set selection flags.
            */
        if (Lf->sf & SELEVTFDINFO) {
            Lp->ept |= EPT_EVTFD;
            Lf->sf &= ~SELEVTFDINFO;
        }
    }
#endif /* defined(HASEPTOPTS) */

    if (Lf->sf)
        Lp->pss |= PS_SEC;
    if (Plf)
        Plf->next = Lf;
    else
        Lp->file = Lf;
    Plf = Lf;
    if (Fnet && (Lf->sf & SELNET))
        Fnet = 2;
    if (Fnfs && (Lf->sf & SELNFS))
        Fnfs = 2;
    if (Ftask && (Lf->sf & SELTASK))
        Ftask = 2;
    Lf = (struct lfile *)NULL;
}

#if defined(HASEPTOPTS)
/*
 * process_pinfo() -- process pipe info, adding it to selected files and
 *		      selecting pipe end files (if requested)
 */

void process_pinfo(struct lsof_context *ctx,
                   int f) /* function:
                           *     0 == process selected pipe
                           *     1 == process end point
                           */
{
    pxinfo_t *pp; /* previous pipe info */

    if (!FeptE)
        return;
    for (Lf = Lp->file; Lf; Lf = Lf->next) {
        if ((Lf->ntype != N_FIFO) || (Lf->inp_ty != 1))
            continue;
        pp = (pxinfo_t *)NULL;
        switch (f) {
        case 0:

            /*
             * Process already selected pipe file.
             */
            if (is_file_sel(ctx, Lp, Lf)) {

                /*
                 * This file has been selected by some criterion other than
                 * its being a pipe.  Look up the pipe's endpoints.
                 */
                do {
                    if ((pp = find_pepti(ctx, Lp->pid, Lf, pp))) {

                        /*
                         * This pipe endpoint is linked to the selected pipe
                         * file.  Add its PID and FD to the name column
                         * addition.
                         */
                        prt_pinfo(ctx, pp, (FeptE == 2));
                        pp = pp->next;
                    }
                } while (pp);
            }
            break;
        case 1:
            if (!is_file_sel(ctx, Lp, Lf) && (Lf->chend & CHEND_PIPE)) {

                /*
                 * This is an unselected end point file.  Select it and add
                 * its end point information to its name column addition.
                 */
                Lf->sf = Selflags;
                Lp->pss |= PS_SEC;
                do {
                    if ((pp = find_pepti(ctx, Lp->pid, Lf, pp))) {
                        prt_pinfo(ctx, pp, 0);
                        pp = pp->next;
                    }
                } while (pp);
            }
            break;
        }
    }
}

/*
 * prt_pinfo() -- print pipe information
 */

static void prt_pinfo(struct lsof_context *ctx, pxinfo_t *pp, /* peer info */
                      int ps) /* processing status:
                               *    0 == process immediately
                               *    1 == process later */
{
    struct lproc *ep; /* pipe endpoint process */
    struct lfile *ef; /* pipe endpoint file */
    int i;            /* temporary index */
    char nma[1024];   /* name addition buffer */
    char fd[FDLEN];

    ep = &Lproc[pp->lpx];
    ef = pp->lf;
    fd_to_string(ef->fd_type, ef->fd_num, fd);
    (void)snpf(nma, sizeof(nma) - 1, "%d,%.*s,%s%c", ep->pid, CmdLim, ep->cmd,
               fd, access_to_char(ef->access));
    (void)add_nma(ctx, nma, strlen(nma));
    if (ps) {

        /*
         * Endpoint files have been selected, so mark this
         * one for selection later. Set the type to PIPE.
         */
        ef->chend = CHEND_PIPE;
        ep->ept |= EPT_PIPE_END;
    }
}

/*
 * process_psxmqinfo() -- posix mq info, adding it to selected files and
 *		          selecting posix mq end files (if requested)
 */

void process_psxmqinfo(struct lsof_context *ctx,
                       int f) /* function:
                               *     0 == process selected posix mq
                               *     1 == process end point
                               */
{
    pxinfo_t *pp; /* previous posix mq info */

    if (!FeptE)
        return;
    for (Lf = Lp->file; Lf; Lf = Lf->next) {
        if (Lf->dev != MqueueDev)
            continue;
        pp = (pxinfo_t *)NULL;
        switch (f) {
        case 0:

            /*
             * Process already selected posix mq file.
             */
            if (is_file_sel(ctx, Lp, Lf)) {

                /*
                 * This file has been selected by some criterion other than
                 * its being a posix mq.  Look up the posix mq's endpoints.
                 */
                do {
                    if ((pp = find_psxmqinfo(ctx, Lp->pid, Lf, pp))) {

                        /*
                         * This posix mq endpoint is linked to the selected
                         * posix mq file.  Add its PID and FD to the name column
                         * addition.
                         */
                        prt_psxmqinfo(ctx, pp, (FeptE == 2));
                        pp = pp->next;
                    }
                } while (pp);
            }
            break;
        case 1:
            if (!is_file_sel(ctx, Lp, Lf) && (Lf->chend & CHEND_PSXMQ)) {

                /*
                 * This is an unselected end point file.  Select it and add
                 * its end point information to its name column addition.
                 */
                Lf->sf = Selflags;
                Lp->pss |= PS_SEC;
                do {
                    if ((pp = find_psxmqinfo(ctx, Lp->pid, Lf, pp))) {
                        prt_psxmqinfo(ctx, pp, 0);
                        pp = pp->next;
                    }
                } while (pp);
            }
            break;
        }
    }
}

/*
 * prt_psxmqinfo() -- print posix mq information
 */

static void prt_psxmqinfo(struct lsof_context *ctx,
                          pxinfo_t *pp, /* peer info */
                          int ps)       /* processing status:
                                         *    0 == process immediately
                                         *    1 == process later */
{
    struct lproc *ep; /* posix mq endpoint process */
    struct lfile *ef; /* posix mq endpoint file */
    int i;            /* temporary index */
    char nma[1024];   /* name addition buffer */
    char fd[FDLEN];

    ep = &Lproc[pp->lpx];
    ef = pp->lf;
    fd_to_string(ef->fd_type, ef->fd_num, fd);
    (void)snpf(nma, sizeof(nma) - 1, "%d,%.*s,%s%c", ep->pid, CmdLim, ep->cmd,
               fd, access_to_char(ef->access));
    (void)add_nma(ctx, nma, strlen(nma));
    if (ps) {

        /*
         * Endpoint files have been selected, so mark this
         * one for selection later. Set the type to posix mq.
         */
        ef->chend = CHEND_PSXMQ;
        ep->ept |= EPT_PSXMQ_END;
    }
}

/*
 * process_evtfdinfo() -- process eventfd info, adding it to selected files and
 *		          selecting envetfd end files (if requested)
 */

void process_evtfdinfo(struct lsof_context *ctx,
                       int f) /* function:
                               *     0 == process selected eventfd
                               *     1 == process end point
                               */
{
    pxinfo_t *pp; /* previous eventfd info */

    if (!FeptE)
        return;
    for (Lf = Lp->file; Lf; Lf = Lf->next) {
        if ((Lf->ntype != N_ANON_INODE) || (Lf->eventfd_id == -1))
            continue;
        pp = (pxinfo_t *)NULL;
        switch (f) {
        case 0:

            /*
             * Process already selected eventfd_id file.
             */
            if (is_file_sel(ctx, Lp, Lf)) {

                /*
                 * This file has been selected by some criterion other than
                 * its being a eventfd.  Look up the eventfd's endpoints.
                 */
                do {
                    if ((pp = find_evtfdinfo(ctx, Lp->pid, Lf, pp))) {

                        /*
                         * This eventfd endpoint is linked to the selected
                         * eventfd file.  Add its PID and FD to the name column
                         * addition.
                         */
                        prt_evtfdinfo(ctx, pp, (FeptE == 2));
                        pp = pp->next;
                    }
                } while (pp);
            }
            break;
        case 1:
            if (!is_file_sel(ctx, Lp, Lf) && (Lf->chend & CHEND_EVTFD)) {

                /*
                 * This is an unselected end point file.  Select it and add
                 * its end point information to its name column addition.
                 */
                Lf->sf = Selflags;
                Lp->pss |= PS_SEC;
                do {
                    if ((pp = find_evtfdinfo(ctx, Lp->pid, Lf, pp))) {
                        prt_evtfdinfo(ctx, pp, 0);
                        pp = pp->next;
                    }
                } while (pp);
            }
            break;
        }
    }
}

/*
 * prt_evtfdinfo() -- print eventfd information
 */

static void prt_evtfdinfo(struct lsof_context *ctx,
                          pxinfo_t *pp, /* peer info */
                          int ps)       /* processing status:
                                         *    0 == process immediately
                                         *    1 == process later */
{
    struct lproc *ep; /* eventfd endpoint process */
    struct lfile *ef; /* eventfd endpoint file */
    int i;            /* temporary index */
    char nma[1024];   /* name addition buffer */
    char fd[FDLEN];

    ep = &Lproc[pp->lpx];
    ef = pp->lf;
    fd_to_string(ef->fd_type, ef->fd_num, fd);
    (void)snpf(nma, sizeof(nma) - 1, "%d,%.*s,%s%c", ep->pid, CmdLim, ep->cmd,
               fd, access_to_char(ef->access));
    (void)add_nma(ctx, nma, strlen(nma));
    if (ps) {

        /*
         * Endpoint files have been selected, so mark this
         * one for selection later. Set the type to PIPE.
         */
        ef->chend = CHEND_EVTFD;
        ep->ept |= EPT_EVTFD_END;
    }
}
#endif /* defined(HASEPTOPTS) */

#if defined(HASPTYEPT)
/*
 * process_ptyinfo() -- process pseudoterminal info, adding it to selected files
 *and selecting pseudoterminal end files (if requested)
 */

void process_ptyinfo(struct lsof_context *ctx,
                     int f) /* function:
                             *  0 == process selected pseudoterminal
                             *  1 == process end point */
{
    pxinfo_t *pp; /* previous pseudoterminal info */
    int mos;      /* master or slave indicator
                   *     0 == slave; 1 == master */
    int pc;       /* print count */

    if (!FeptE)
        return;
    for (Lf = Lp->file; Lf; Lf = Lf->next) {
        if (Lf->rdev_def && is_pty_ptmx(Lf->rdev))
            mos = 1;
        else if (Lf->rdev_def && is_pty_slave(GET_MAJ_DEV(Lf->rdev)))
            mos = 0;
        else
            continue;

        pp = (pxinfo_t *)NULL;
        switch (f) {
        case 0:

            /*
             * Process already selected pseudoterminal file.
             */
            if (is_file_sel(ctx, Lp, Lf)) {

                /*
                 * This file has been selected by some criterion other than
                 * its being a pseudoterminal.  Look up the pseudoterminal's
                 * endpoints.
                 */
                pc = 1;
                do {
                    if ((pp = find_ptyepti(ctx, Lp->pid, Lf, !mos, pp))) {

                        /*
                         * This pseudoterminal endpoint is linked to the
                         * selected pseudoterminal file.  Add its PID, FD and
                         * access mode to the name column addition.
                         */
                        prt_ptyinfo(ctx, pp, (mos && pc), (FeptE == 2));
                        pp = pp->next;
                        pc = 0;
                    }
                } while (pp);
            }
            break;
        case 1:
            if (!is_file_sel(ctx, Lp, Lf) && (Lf->chend & CHEND_PTY)) {

                /*
                 * This is an unselected end point file.  Select it and add
                 * its end point information to its name column addition.
                 */
                Lf->sf = Selflags;
                Lp->pss |= PS_SEC;
                pc = 1;
                do {
                    if ((pp = find_ptyepti(ctx, Lp->pid, Lf, !mos, pp))) {
                        prt_ptyinfo(ctx, pp, (mos && pc), 0);
                        pp = pp->next;
                        pc = 0;
                    }
                } while (pp);
            }
            break;
        }
    }
}

/*
 * prt_ptyinfo() -- print pseudoterminal information
 */

static void prt_ptyinfo(struct lsof_context *ctx, pxinfo_t *pp, /* peer info */
                        int prt_edev, /* print the end point device file */
                        int ps)       /* processing status:
                                       *    0 == process immediately
                                       *    1 == process later */
{
    struct lproc *ep; /* pseudoterminal endpoint process */
    struct lfile *ef; /* pseudoterminal endpoint file */
    int i;            /* temporary index */
    char nma[1024];   /* name addition buffer */
    char fd[FDLEN];

    ep = &Lproc[pp->lpx];
    ef = pp->lf;
    fd_to_string(ef->fd_type, ef->fd_num, fd);
    if (prt_edev) {
        (void)snpf(nma, sizeof(nma) - 1, "->/dev/pts/%d %d,%.*s,%s%c",
                   Lf->tty_index, ep->pid, CmdLim, ep->cmd, fd,
                   access_to_char(ef->access));
    } else {
        (void)snpf(nma, sizeof(nma) - 1, "%d,%.*s,%s%c", ep->pid, CmdLim,
                   ep->cmd, fd, access_to_char(ef->access));
    }
    (void)add_nma(ctx, nma, strlen(nma));
    if (ps) {

        /*
         * Endpoint files have been selected, so mark this
         * one for selection later. Set the type to PTY.
         */
        ef->chend = CHEND_PTY;
        ep->ept |= EPT_PTY_END;
    }
}
#endif /* defined(HASPTYEPT) */

/* Convert lsof_fd_type and fd_num to string, sizeof(buf) should >= FDLEN */
void fd_to_string(enum lsof_fd_type fd_type, int fd_num, char *buf) {
    switch (fd_type) {
    case LSOF_FD_NUMERIC:
        /* strlen("TYPE") == 4, try to match width */
        if (fd_num < 10000)
            (void)snpf(buf, FDLEN, "%d", fd_num);
        else
            (void)snpf(buf, FDLEN, "*%03d", fd_num % 1000);
        break;
    case LSOF_FD_UNKNOWN:
        (void)snpf(buf, FDLEN, "unk");
        break;
    case LSOF_FD_CWD:
        (void)snpf(buf, FDLEN, "cwd");
        break;
    case LSOF_FD_ERROR:
        (void)snpf(buf, FDLEN, "err");
        break;
    case LSOF_FD_NOFD:
        (void)snpf(buf, FDLEN, "NOFD");
        break;
    case LSOF_FD_ROOT_DIR:
        (void)snpf(buf, FDLEN, "rtd");
        break;
    case LSOF_FD_PARENT_DIR:
        (void)snpf(buf, FDLEN, "pd");
        break;
    case LSOF_FD_PROGRAM_TEXT:
        (void)snpf(buf, FDLEN, "txt");
        break;
    case LSOF_FD_LIBRARY_TEXT:
        (void)snpf(buf, FDLEN, "ltx");
        break;
    case LSOF_FD_MEMORY:
        (void)snpf(buf, FDLEN, "mem");
        break;
    case LSOF_FD_DELETED:
        (void)snpf(buf, FDLEN, "DEL");
        break;
    case LSOF_FD_FILEPORT:
        (void)snpf(buf, FDLEN, "fp.");
        break;
    case LSOF_FD_TASK_CWD:
        (void)snpf(buf, FDLEN, "twd");
        break;
    case LSOF_FD_CTTY:
        (void)snpf(buf, FDLEN, "ctty");
        break;
    case LSOF_FD_JAIL_DIR:
        (void)snpf(buf, FDLEN, "jld");
        break;
    case LSOF_FD_VIRTUAL_8086:
        (void)snpf(buf, FDLEN, "v86");
        break;
    case LSOF_FD_MERGE_386:
        (void)snpf(buf, FDLEN, "m86");
        break;
    case LSOF_FD_MMAP_DEVICE:
        (void)snpf(buf, FDLEN, "mmap");
        break;
    case LSOF_FD_LIBRARY_REF:
        (void)snpf(buf, FDLEN, "L%02d", fd_num);
        break;
    case LSOF_FD_MMAP_UNKNOWN:
        (void)snpf(buf, FDLEN, "M%02x", fd_num);
        break;
    case LSOF_FD_PREGION_UNKNOWN:
        (void)snpf(buf, FDLEN, "R%02d", fd_num);
        break;
    default:
        fprintf(stderr, "Unknown fd type: %d\n", (int)fd_type);
        buf[0] = '\0';
        break;
    }
}