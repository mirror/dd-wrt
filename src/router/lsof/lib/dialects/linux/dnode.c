/*
 * dnode.c - Linux node functions for /proc-based lsof
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

#if defined(HASEPTOPTS) && defined(HASPTYEPT)
#    include <linux/major.h>
#endif /* defined(HASEPTOPTS) && defined(HASPTYEPT) */

/*
 * Local definitions
 */

#define OFFSET_MAX                                                             \
    ((off_t)0x7fffffff) /* this is defined in                                  \
                         * .../src/fs/locks.c and not                          \
                         * in a header file */
#define PIDBUCKS 64     /* PID hash buckets */
#define PINFOBUCKS 512  /* pipe info hash buckets */
#define HASHPID(pid) (((int)((pid * 31415) >> 3)) & (PIDBUCKS - 1))
#define HASHPINFO(ino) (((int)((ino * 31415) >> 3)) & (PINFOBUCKS - 1))

/*
 * Local structure definitions
 */

struct llock {
    int pid;
    dev_t dev;
    INODETYPE inode;
    enum lsof_lock_mode type;
    struct llock *next;
};

/*
 * Local definitions
 */

struct llock **LckH = (struct llock **)NULL; /* PID-hashed locks */

/*
 * Local function prototypes
 */

static void check_lock(struct lsof_context *ctx);

#if defined(HASEPTOPTS)
static void enter_pinfo(struct lsof_context *ctx);
#endif /* defined(HASEPTOPTS) */

/*
 * Local storage
 */

#if defined(HASEPTOPTS)
static pxinfo_t **Pinfo = (pxinfo_t **)NULL; /* pipe endpoint hash buckets */
#    if defined(HASPTYEPT)
static pxinfo_t **PtyInfo = (pxinfo_t **)NULL; /* pseudoterminal endpoint hash
                                                * buckets */
#    endif                                     /* defined(HASPTYEPT) */
static pxinfo_t **PSXMQinfo =
    (pxinfo_t **)NULL; /* posix msg queue endpoint hash buckets */
static pxinfo_t **EvtFDinfo =
    (pxinfo_t **)NULL; /* envetfd endpoint hash buckets */
#endif                 /* defined(HASEPTOPTS) */

/*
 * check_lock() - check lock for file *Lf, process *Lp
 */

static void check_lock(struct lsof_context *ctx) {
    int h;
    struct llock *lp;

    h = HASHPID(Lp->pid);
    for (lp = LckH[h]; lp; lp = lp->next) {
        if (Lp->pid == lp->pid && Lf->dev == lp->dev &&
            Lf->inode == lp->inode) {
            Lf->lock = lp->type;
            return;
        }
    }
}

#if defined(HASEPTOPTS)
static void endpoint_pxinfo_hash(pxinfo_t **pinfo_hash, const size_t nbuckets,
                                 void (*free_elt)(void *)) {
    int h;             /* hash index */
    pxinfo_t *pi, *pp; /* temporary pointers */

    if (!pinfo_hash)
        return;
    for (h = 0; h < nbuckets; h++) {
        if ((pi = pinfo_hash[h])) {
            do {
                pp = pi->next;
                (void)(*free_elt)((FREE_P *)pi);
                pi = pp;
            } while (pi);
            pinfo_hash[h] = (pxinfo_t *)NULL;
        }
    }
}

static void endpoint_enter(struct lsof_context *ctx, pxinfo_t **pinfo_hash,
                           const char *table_name, int id) {
    int h;
    struct lfile *lf;       /* local file structure pointer */
    struct lproc *lp;       /* local proc structure pointer */
    pxinfo_t *np, *pi, *pe; /* inode hash pointers */
    char fd[FDLEN];

    /*
     * Make sure this is a unique entry.
     */
    for (h = HASHPINFO(id), pi = pinfo_hash[h], pe = (pxinfo_t *)NULL; pi;
         pe = pi, pi = pi->next) {
        lf = pi->lf;
        lp = &Lproc[pi->lpx];
        if (pi->ino == id) {
            if ((lp->pid == Lp->pid) && (lf->fd_type == Lf->fd_type) &&
                (lf->fd_num == Lf->fd_num))
                return;
        }
    }
    /*
     * Allocate, fill and link a new pipe info structure used for pty
     * to the end of the pty device hash chain.
     */
    if (!(np = (pxinfo_t *)malloc(sizeof(pxinfo_t)))) {
        fd_to_string(Lf->fd_type, Lf->fd_num, fd);
        (void)fprintf(stderr,
                      "%s: no space for pipeinfo for %s, PID %d, FD %s\n",
                      table_name, Pn, Lp->pid, fd);
        Error(ctx);
    }
    np->ino = id;
    np->lf = Lf;
    np->lpx = Lp - Lproc;
    np->next = (pxinfo_t *)NULL;
    if (pe)
        pe->next = np;
    else
        pinfo_hash[h] = np;
}

static pxinfo_t *
endpoint_find(struct lsof_context *ctx, pxinfo_t **pinfo_hash,
              int (*is_acceptable)(struct lsof_context *, pxinfo_t *, int,
                                   struct lfile *),
              int pid, struct lfile *lf, int id, pxinfo_t *pp) {
    int h;        /* hash result */
    pxinfo_t *pi; /* pipe info pointer */

    if (pinfo_hash) {
        if (pp)
            pi = pp;
        else {
            h = HASHPINFO(id);
            pi = pinfo_hash[h];
        }
        while (pi) {
            if (pi->ino == id && is_acceptable(ctx, pi, pid, lf))
                return (pi);
            pi = pi->next;
        }
    }
    return ((pxinfo_t *)NULL);
}

/*
 * endpoint_accept_other_than_self() -- a helper function return true if
 * fd associated with pi is not the same as fd associated with lf.
 */

static int endpoint_accept_other_than_self(struct lsof_context *ctx,
                                           pxinfo_t *pi, int pid,
                                           struct lfile *lf) {
    struct lfile *ef = pi->lf;
    struct lproc *ep = &Lproc[pi->lpx];
    return (lf->fd_type != ef->fd_type) || (lf->fd_num != ef->fd_num) ||
           (pid != ep->pid);
}

/*
 * clear_pinfo() -- clear allocated pipe info
 */

void clear_pinfo(struct lsof_context *ctx) {
    endpoint_pxinfo_hash(Pinfo, PINFOBUCKS, free);
}

/*
 * enter_pinfo() -- enter pipe info
 *
 * 	entry	Lf = local file structure pointer
 * 		Lp = local process structure pointer
 */

static void enter_pinfo(struct lsof_context *ctx) {
    if (!Pinfo) {
        /*
         * Allocate pipe info hash buckets.
         */
        if (!(Pinfo = (pxinfo_t **)calloc(PINFOBUCKS, sizeof(pxinfo_t *)))) {
            (void)fprintf(stderr, "%s: no space for %d pipe info buckets\n", Pn,
                          PINFOBUCKS);
            Error(ctx);
        }
    }
    endpoint_enter(ctx, Pinfo, "pipeinfo", Lf->inode);
}

/*
 * find_pepti() -- find pipe end point info
 */

pxinfo_t *find_pepti(struct lsof_context *ctx, /* context */
                     int pid,          /* pid of the process owning lf */
                     struct lfile *lf, /* pipe's lfile */
                     pxinfo_t *pp)     /* previous pipe info (NULL == none) */
{
    return endpoint_find(ctx, Pinfo, endpoint_accept_other_than_self, pid, lf,
                         lf->inode, pp);
}

#    if defined(HASPTYEPT)

/*
 * clear_ptyinfo() -- clear allocated pseudoterminal info
 */

void clear_ptyinfo(struct lsof_context *ctx) {
    endpoint_pxinfo_hash(PtyInfo, PINFOBUCKS, free);
}

/*
 * enter_ptmxi() -- enter pty info
 *
 * 	entry	Lf = local file structure pointer
 * 		Lp = local process structure pointer
 */

void enter_ptmxi(struct lsof_context *ctx, /* context */
                 int mn)                   /* minor number of device */
{
    /*
     * Allocate pipe info hash buckets (but used for pty).
     */
    if (!PtyInfo) {
        if (!(PtyInfo = (pxinfo_t **)calloc(PINFOBUCKS, sizeof(pxinfo_t *)))) {
            (void)fprintf(stderr, "%s: no space for %d pty info buckets\n", Pn,
                          PINFOBUCKS);
            Error(ctx);
        }
    }
    endpoint_enter(ctx, PtyInfo, "pty", mn);
}

/*
 * ptyepti_accept_ptmx() -- a helper function return whether lfile is pty ptmx
 * or not
 */

static int ptyepti_accept_ptmx(struct lsof_context *ctx, pxinfo_t *pi, int pid,
                               struct lfile *lf) {
    struct lfile *ef = pi->lf;
    return is_pty_ptmx(ef->rdev);
}

/*
 * ptyepti_accept_slave() -- a helper function returns whether lfile is pty
 * slave or not
 */

static int ptyepti_accept_slave(struct lsof_context *ctx, pxinfo_t *pi, int pid,
                                struct lfile *lf) {
    struct lfile *ef = pi->lf;
    return is_pty_slave(GET_MAJ_DEV(ef->rdev));
}

/*
 * find_ptyepti() -- find pseudoterminal end point info
 */

pxinfo_t *find_ptyepti(struct lsof_context *ctx, /* context */
                       int pid,                  /* PID*/
                       struct lfile *lf,         /* pseudoterminal's lfile */
                       int m,                    /* minor number type:
                                                  *     0 == use tty_index
                                                  *     1 == use minor device */
                       pxinfo_t *pp)             /* previous pseudoterminal info
                                                  * (NULL == none) */
{
    return endpoint_find(ctx, PtyInfo,
                         m ? ptyepti_accept_ptmx : ptyepti_accept_slave, pid,
                         lf, m ? GET_MIN_DEV(lf->rdev) : lf->tty_index, pp);
}

/*
 * is_pty_slave() -- is a pseudoterminal a slave device
 */

int is_pty_slave(int sm) /* slave major device number */
{
    /* linux/Documentation/admin-guide/devices.txt
       -------------------------------------------
       136-143 char	Unix98 PTY slaves
       0 = /dev/pts/0	First Unix98 pseudo-TTY
       1 = /dev/pts/1	Second Unix98 pseudo-TTY
       ...

       These device nodes are automatically generated with
       the proper permissions and modes by mounting the
       devpts filesystem onto /dev/pts with the appropriate
       mount options (distribution dependent, however, on
       *most* distributions the appropriate options are
       "mode=0620,gid=<gid of the "tty" group>".) */
    if ((UNIX98_PTY_SLAVE_MAJOR <= sm) &&
        (sm < (UNIX98_PTY_SLAVE_MAJOR + UNIX98_PTY_MAJOR_COUNT))) {
        return 1;
    }
    return 0;
}

/*
 * is_pty_ptmx() -- is a pseudoterminal a master clone device
 */

int is_pty_ptmx(dev_t dev) /* device number */
{
    if ((GET_MAJ_DEV(dev) == TTYAUX_MAJOR) && (GET_MIN_DEV(dev) == 2))
        return 1;
    return 0;
}
#    endif /* defined(HASPTYEPT) */

/*
 * clear_psxmqinfo -- clear allocate posix mq info
 */

void clear_psxmqinfo(struct lsof_context *ctx) {
    endpoint_pxinfo_hash(PSXMQinfo, PINFOBUCKS, free);
}

/*
 * enter_psxmqinfo() -- enter posix mq info
 *
 *	entry	Lf = local file structure pointer
 *		Lp = local process structure pointer
 */

void enter_psxmqinfo(struct lsof_context *ctx) {
    if (!PSXMQinfo) {
        /*
         * Allocate posix mq info hash buckets.
         */
        if (!(PSXMQinfo =
                  (pxinfo_t **)calloc(PINFOBUCKS, sizeof(pxinfo_t *)))) {
            (void)fprintf(stderr, "%s: no space for %d posix mq info buckets\n",
                          Pn, PINFOBUCKS);
            Error(ctx);
        }
    }
    endpoint_enter(ctx, PSXMQinfo, "psxmqinfo", Lf->inode);
}

/*
 * find_psxmqinfo() -- find posix mq end point info
 */

pxinfo_t *
find_psxmqinfo(struct lsof_context *ctx, /* context */
               int pid,                  /* pid of the process owning lf */
               struct lfile *lf,         /* posix mq's lfile */
               pxinfo_t *pp) /* previous posix mq info (NULL == none) */
{
    return endpoint_find(ctx, PSXMQinfo, endpoint_accept_other_than_self, pid,
                         lf, lf->inode, pp);
}

/*
 * clear_evtfdinfo -- clear allocate eventfd info
 */

void clear_evtfdinfo(struct lsof_context *ctx) {
    endpoint_pxinfo_hash(EvtFDinfo, PINFOBUCKS, free);
}

/*
 * enter_evtfdinfo() -- enter eventfd info
 *
 *	entry	Lf = local file structure pointer
 *		Lp = local process structure pointer
 */

void enter_evtfdinfo(struct lsof_context *ctx, int id) {
    if (!EvtFDinfo) {
        /*
         * Allocate eventfd info hash buckets.
         */
        if (!(EvtFDinfo =
                  (pxinfo_t **)calloc(PINFOBUCKS, sizeof(pxinfo_t *)))) {
            (void)fprintf(stderr, "%s: no space for %d envet fd info buckets\n",
                          Pn, PINFOBUCKS);
            Error(ctx);
        }
    }
    endpoint_enter(ctx, EvtFDinfo, "evtfdinfo", id);
}

/*
 * find_evtfdinfo() -- find eventfd end point info
 */

pxinfo_t *
find_evtfdinfo(struct lsof_context *ctx, /* context */
               int pid,                  /* pid of the process owning lf */
               struct lfile *lf,         /* eventfd's lfile */
               pxinfo_t *pp) /* previous eventfd info (NULL == none) */
{
    void *r = endpoint_find(ctx, EvtFDinfo, endpoint_accept_other_than_self,
                            pid, lf, lf->eventfd_id, pp);
    return r;
}
#endif /* defined(HASEPTOPTS) */

/*
 * get_fields() - separate a line into fields
 */

int get_fields(struct lsof_context *ctx, /* context */
               char *ln,                 /* input line */
               char *sep,                /* separator list */
               char ***fr,               /* field pointer return address */
               int *eb,                  /* indexes of fields where blank or an
                                          * entry from the separator list may be
                                          * embedded and are not separators
                                          * (may be NULL) */
               int en)                   /* number of entries in eb[] (may be
                                          * zero) */
{
    char *bp, *cp, *sp;
    int i, j, n;
    MALLOC_S len;
    static char **fp = (char **)NULL;
    static int nfpa = 0;

    for (cp = ln, n = 0; cp && *cp;) {
        for (bp = cp; *bp && (*bp == ' ' || *bp == '\t'); bp++)
            ;
        ;
        if (!*bp || *bp == '\n')
            break;
        for (cp = bp; *cp; cp++) {
            if (*cp == '\n') {
                *cp = '\0';
                break;
            }
            if (*cp == '\t') /* TAB is always a separator */
                break;
            if (*cp == ' ') {

                /*
                 * See if this field may have an embedded space.
                 */
                if (!eb || !en)
                    break;
                else {
                    for (i = j = 0; i < en; i++) {
                        if (eb[i] == n) {
                            j = 1;
                            break;
                        }
                    }
                    if (!j)
                        break;
                }
            }
            if (sep) {

                /*
                 * See if the character is in the separator list.
                 */
                for (sp = sep; *sp; sp++) {
                    if (*sp == *cp)
                        break;
                }
                if (*sp) {

                    /*
                     * See if this field may have an embedded separator.
                     */
                    if (!eb || !en)
                        break;
                    else {
                        for (i = j = 0; i < en; i++) {
                            if (eb[i] == n) {
                                j = 1;
                                break;
                            }
                        }
                        if (!j)
                            break;
                    }
                }
            }
        }
        if (*cp)
            *cp++ = '\0';
        if (n >= nfpa) {
            nfpa += 32;
            len = (MALLOC_S)(nfpa * sizeof(char *));
            if (fp)
                fp = (char **)realloc((MALLOC_P *)fp, len);
            else
                fp = (char **)malloc(len);
            if (!fp) {
                (void)fprintf(
                    stderr, "%s: can't allocate %d bytes for field pointers.\n",
                    Pn, (int)len);
                Error(ctx);
            }
        }
        fp[n++] = bp;
    }
    *fr = fp;
    return (n);
}

/*
 * get_locks() - get lock information from /proc/locks
 */

void get_locks(struct lsof_context *ctx, /* context */
               char *p)                  /* /proc lock path */
{
    unsigned long bp, ep;
    char buf[MAXPATHLEN], *ec, **fp;
    dev_t dev;
    int ex, i, h, mode, pid;
    INODETYPE inode;
    struct llock *lp, *np;
    FILE *ls;
    long maj, min;
    enum lsof_lock_mode type;
    static char *vbuf = (char *)NULL;
    static size_t vsz = (size_t)0;
    /*
     * Destroy previous lock information.
     */
    if (LckH) {
        for (i = 0; i < PIDBUCKS; i++) {
            for (lp = LckH[i]; lp; lp = np) {
                np = lp->next;
                (void)free((FREE_P *)lp);
            }
            LckH[i] = (struct llock *)NULL;
        }
    } else {

        /*
         * If first time, allocate the lock PID hash buckets.
         */
        LckH =
            (struct llock **)calloc((MALLOC_S)PIDBUCKS, sizeof(struct llock *));
        if (!LckH) {
            (void)fprintf(stderr, "%s: can't allocate %d lock hash bytes\n", Pn,
                          (int)(sizeof(struct llock *) * PIDBUCKS));
            Error(ctx);
        }
    }
    /*
     * Open the /proc lock file, assign a page size buffer to its stream,
     * and read it.
     */
    if (!(ls = open_proc_stream(ctx, p, "r", &vbuf, &vsz, 0)))
        return;
    while (fgets(buf, sizeof(buf), ls)) {
        if (get_fields(ctx, buf, ":", &fp, (int *)NULL, 0) < 10)
            continue;
        if (!fp[1] || strcmp(fp[1], "->") == 0)
            continue;
        /*
         * Get lock type.
         */
        if (!fp[3])
            continue;
        if (*fp[3] == 'R')
            mode = 0;
        else if (*fp[3] == 'W')
            mode = 1;
        else
            continue;
        /*
         * Get PID.
         */
        if (!fp[4] || !*fp[4])
            continue;
        pid = atoi(fp[4]);
        /*
         * Get device number.
         */
        ec = (char *)NULL;
        if (!fp[5] || !*fp[5] || (maj = strtol(fp[5], &ec, 16)) == LONG_MIN ||
            maj == LONG_MAX || !ec || *ec)
            continue;
        ec = (char *)NULL;
        if (!fp[6] || !*fp[6] || (min = strtol(fp[6], &ec, 16)) == LONG_MIN ||
            min == LONG_MAX || !ec || *ec)
            continue;
        dev = (dev_t)makedev((int)maj, (int)min);
        /*
         * Get inode number.
         */
        ec = (char *)NULL;
        if (!fp[7] || !*fp[7] ||
            (inode = strtoull(fp[7], &ec, 0)) == ULONG_MAX || !ec || *ec)
            continue;
        /*
         * Get lock extent.  Convert it and the lock type to a lock character.
         */
        if (!fp[8] || !*fp[8] || !fp[9] || !*fp[9])
            continue;
        ec = (char *)NULL;
        if ((bp = strtoul(fp[8], &ec, 0)) == ULONG_MAX || !ec || *ec)
            continue;
        if (!strcmp(fp[9], "EOF")) /* for Linux 2.4.x */
            ep = OFFSET_MAX;
        else {
            ec = (char *)NULL;
            if ((ep = strtoul(fp[9], &ec, 0)) == ULONG_MAX || !ec || *ec)
                continue;
        }
        ex = ((off_t)bp == (off_t)0 && (off_t)ep == OFFSET_MAX) ? 1 : 0;
        if (mode)
            type = ex ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
        else
            type = ex ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
        /*
         * Look for this lock via the hash buckets.
         */
        h = HASHPID(pid);
        for (lp = LckH[h]; lp; lp = lp->next) {
            if (lp->pid == pid && lp->dev == dev && lp->inode == inode &&
                lp->type == type)
                break;
        }
        if (lp)
            continue;
        /*
         * Allocate a new llock structure and link it to the PID hash bucket.
         */
        if (!(lp = (struct llock *)malloc(sizeof(struct llock)))) {
            (void)snpf(buf, sizeof(buf), "%" INODEPSPEC "u", inode);
            (void)fprintf(
                stderr, "%s: can't allocate llock: PID %d; dev %x; inode %s\n",
                Pn, pid, (int)dev, buf);
            Error(ctx);
        }
        lp->pid = pid;
        lp->dev = dev;
        lp->inode = inode;
        lp->type = type;
        lp->next = LckH[h];
        LckH[h] = lp;
    }
    (void)fclose(ls);
}

/*
 * process_proc_node() - process file node
 */

void process_proc_node(struct lsof_context *ctx, /* context */
                       char *p,                  /* node's readlink() path */
                       char *pbr,      /* node's path before readlink() */
                       struct stat *s, /* stat() result for path */
                       int ss,         /* *s status -- i.e., SB_* values */
                       struct stat *l, /* lstat() result for FD (NULL for
                                        * others) */
                       int ls)         /* *l status -- i.e., SB_* values */
{
    mode_t access;
    mode_t type = 0;
    char *cp;
    struct mounts *mp = (struct mounts *)NULL;
    size_t sz;
    char *tn;
    /*
     * Set the access mode, if possible.
     */
    if (l && (ls & SB_MODE) && ((l->st_mode & S_IFMT) == S_IFLNK)) {
        if ((access = l->st_mode & (S_IRUSR | S_IWUSR)) == S_IRUSR)
            Lf->access = LSOF_FILE_ACCESS_READ;
        else if (access == S_IWUSR)
            Lf->access = LSOF_FILE_ACCESS_WRITE;
        else
            Lf->access = LSOF_FILE_ACCESS_READ_WRITE;
    }
    /*
     * Determine node type.
     */
    if (ss & SB_MODE) {
        type = s->st_mode & S_IFMT;
        switch (type) {
        case S_IFBLK:
            Lf->ntype = Ntype = N_BLK;
            break;
        case S_IFCHR:
            Lf->ntype = Ntype = N_CHR;
            break;
        case S_IFIFO:
            Lf->ntype = Ntype = N_FIFO;
            break;
        case S_IFSOCK:
            /* Lf->ntype = Ntype = N_REGLR;		by alloc_lfile() */
            process_proc_sock(ctx, p, pbr, s, ss, l, ls);
            return;
        case 0:
            if (!strcmp(p, "anon_inode"))
                Lf->ntype = Ntype = N_ANON_INODE;
            break;
        }
    }
    if (Selinet)
        return;
    /*
     * Save the device.  If it is an NFS device, change the node type to N_NFS.
     */
    if (ss & SB_DEV) {
        Lf->dev = s->st_dev;
        Lf->dev_def = 1;
    }
    if ((Ntype == N_CHR || Ntype == N_BLK)) {
        if (ss & SB_RDEV) {
            Lf->rdev = s->st_rdev;
            Lf->rdev_def = 1;

#if defined(HASEPTOPTS) && defined(HASPTYEPT)
            if (FeptE && (Ntype == N_CHR) &&
                is_pty_slave(GET_MAJ_DEV(Lf->rdev))) {
                enter_ptmxi(ctx, GET_MIN_DEV(Lf->rdev));
                Lf->sf |= SELPTYINFO;
            }
#endif /* defined(HASEPTOPTS) && defined(HASPTYEPT) */
        }
    }
    if (Ntype == N_REGLR && (HasNFS == 2)) {
        for (mp = readmnt(ctx); mp; mp = mp->next) {
            if ((mp->ty == N_NFS) && (mp->ds & SB_DEV) && Lf->dev_def &&
                (Lf->dev == mp->dev) &&
                (mp->dir && mp->dirl && !strncmp(mp->dir, p, mp->dirl))) {
                Lf->ntype = Ntype = N_NFS;
                break;
            }
        }
    }
    /*
     * Save the inode number.
     */
    if (ss & SB_INO) {
        Lf->inode = (INODETYPE)s->st_ino;
        Lf->inp_ty = 1;

#if defined(HASEPTOPTS)
        if ((Lf->ntype == N_FIFO) && FeptE) {
            (void)enter_pinfo(ctx);
            Lf->sf |= SELPINFO;
        } else if ((Lf->dev == MqueueDev) && FeptE) {
            (void)enter_psxmqinfo(ctx);
            Lf->sf |= SELPSXMQINFO;
        }
#endif /* defined(HASEPTOPTS) */
    }
    /*
     * Check for a lock.
     */
    if (Lf->dev_def && (Lf->inp_ty == 1))
        (void)check_lock(ctx);
    /*
     * Save the file size.
     */
    switch (Ntype) {
    case N_BLK:
    case N_CHR:
    case N_FIFO:
        if (l && (ls & SB_SIZE) && OffType != OFFSET_UNKNOWN) {
            Lf->off = (SZOFFTYPE)l->st_size;
            Lf->off_def = 1;
        }
        break;
    default:
        if (l && (ls & SB_SIZE) && OffType != OFFSET_UNKNOWN) {
            Lf->off = (SZOFFTYPE)l->st_size;
            Lf->off_def = 1;
        }
        if (ss & SB_SIZE) {
            Lf->sz = (SZOFFTYPE)s->st_size;
            Lf->sz_def = 1;
        }
    }
    /*
     * Record the link count.
     */
    if (ss & SB_NLINK) {
        Lf->nlink = (long)s->st_nlink;
        Lf->nlink_def = 1;
        if (Nlink && (Lf->nlink < Nlink))
            Lf->sf |= SELNLINK;
    }
    /*
     * Format the type name.
     */
    if (ss & SB_MODE) {
        switch (type) {
        case S_IFBLK:
            Lf->type = LSOF_FILE_BLOCK;
            break;
        case S_IFCHR:
            Lf->type = LSOF_FILE_CHAR;
            break;
        case S_IFDIR:
            Lf->type = LSOF_FILE_DIR;
            break;
        case S_IFIFO:
            Lf->type = LSOF_FILE_FIFO;
            break;
        case S_IFREG:
            if (Lf->dev == MqueueDev)
                Lf->type = LSOF_FILE_POSIX_MQ;
            else
                Lf->type = LSOF_FILE_REGULAR;
            break;
        case S_IFLNK:
            Lf->type = LSOF_FILE_LINK;
            break;
        default:
            if (Ntype == N_ANON_INODE)
                Lf->type = LSOF_FILE_ANON_INODE;
            else {
                Lf->type = LSOF_FILE_UNKNOWN_RAW;
                Lf->unknown_file_type_number = (type >> 12) & 0xf;
            }
        }
    } else
        Lf->type = LSOF_FILE_UNKNOWN_STAT;
    /*
     * Record an NFS file selection.
     */
    if (Ntype == N_NFS && Fnfs)
        Lf->sf |= SELNFS;
    /*
     * Test for specified file.
     */
    if (Sfile &&
        is_file_named(ctx, 1, p, mp,
                      ((type == S_IFCHR) || (type == S_IFBLK)) ? 1 : 0))
        Lf->sf |= SELNM;
    /*
     * If no NAME information has been stored, store the path.
     *
     * Store the remote host and mount point for an NFS file.
     */
    if (!Namech[0]) {
        (void)snpf(Namech, Namechl, "%s", p);
        if ((Ntype == N_NFS) && mp && mp->fsname) {
            cp = endnm(ctx, &sz);
            (void)snpf(cp, sz, " (%s)", mp->fsname);
        }
    }
    if (Namech[0])
        enter_nm(ctx, Namech);
}
