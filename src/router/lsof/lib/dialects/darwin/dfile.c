/*
 * dfile.c -- Darwin file processing functions for libproc-based lsof
 */

/*
 * Portions Copyright 2005-2007 Apple Inc.  All rights reserved.
 *
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Allan Nathanson, Apple Inc., and Victor A. Abell, Purdue
 * University.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors, nor Apple Inc. nor Purdue University are
 *    responsible for any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either
 *    by explicit claim or by omission.  Credit to the authors, Apple
 *    Inc. and Purdue University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef lint
static char copyright[] = "@(#) Copyright 2005-2007 Apple Inc. and Purdue "
                          "Research Foundation.\nAll rights reserved.\n";
#endif

#include "common.h"

#if defined(PROC_FP_GUARDED)
extern struct pff_tab Pgf_tab[];
#endif /* defined(PROC_FP_GUARDED) */

/*
 * enter_file_info() -- enter file information
 */

void enter_file_info(
    struct lsof_context *ctx,  /* context */
    struct proc_fileinfo *pfi) /* pointer to process file info */
{
    int f;
    /*
     * Construct access code
     */
    f = pfi->fi_openflags & (FREAD | FWRITE);
    if (f == FREAD)
        Lf->access = LSOF_FILE_ACCESS_READ;
    else if (f == FWRITE)
        Lf->access = LSOF_FILE_ACCESS_WRITE;
    else if (f == (FREAD | FWRITE))
        Lf->access = LSOF_FILE_ACCESS_READ_WRITE;
    /*
     * Save the offset / size
     */
    Lf->off = (SZOFFTYPE)pfi->fi_offset;
    Lf->off_def = 1;
    /*
     * Save file structure information as requested.
     */
    Lf->ffg = (long)pfi->fi_openflags;
    Lf->fsv |= FSV_FG;

#if defined(PROC_FP_GUARDED)
    if (pfi->fi_status & PROC_FP_GUARDED) {
        Lf->guardflags = pfi->fi_guardflags;
    }
#endif /* defined(PROC_FP_GUARDED) */
    Lf->pof = (long)pfi->fi_status;
}

/*
 * enter_vnode_info() -- enter vnode information
 */

void enter_vnode_info(
    struct lsof_context *ctx,    /* context */
    struct vnode_info_path *vip) /* pointer to vnode info with path */
{
    char buf[32];
    enum lsof_file_type type;
    uint32_t unknown_file_type_number = 0;
    dev_t dev = 0;
    int devs = 0;
    struct mounts *mp;
    /*
     * Derive file type.
     */
    switch ((int)(vip->vip_vi.vi_stat.vst_mode & S_IFMT)) {
    case S_IFIFO:
        type = LSOF_FILE_FIFO;
        Ntype = N_FIFO;
        break;
    case S_IFCHR:
        type = LSOF_FILE_CHAR;
        Ntype = N_CHR;
        break;
    case S_IFDIR:
        type = LSOF_FILE_DIR;
        Ntype = N_REGLR;
        break;
    case S_IFBLK:
        type = LSOF_FILE_BLOCK;
        Ntype = N_BLK;
        break;

#if defined(S_IFLNK)
    case S_IFLNK:
        type = LSOF_FILE_LINK;
        Ntype = N_REGLR;
        break;
#endif /* defined(S_IFLNK) */

    case S_IFREG:
        type = LSOF_FILE_REGULAR;
        Ntype = N_REGLR;
        break;
    default:
        type = LSOF_FILE_UNKNOWN_RAW;
        unknown_file_type_number =
            (vip->vip_vi.vi_stat.vst_mode & S_IFMT) >> 12;
        Ntype = N_REGLR;
    }
    if (Lf->type == LSOF_FILE_NONE) {
        Lf->type = type;
        Lf->unknown_file_type_number = unknown_file_type_number;
    }
    Lf->ntype = Ntype;
    /*
     * Save device number and path
     */
    switch (Ntype) {
    case N_FIFO:
        break;
    case N_CHR:
    case N_BLK:
        Lf->rdev = vip->vip_vi.vi_stat.vst_rdev;
        Lf->rdev_def = 1;
        /* fall through */
    default:
        Lf->dev = dev = vip->vip_vi.vi_stat.vst_dev;
        Lf->dev_def = devs = 1;
    }
    /*
     * Save path name.
     */
    vip->vip_path[sizeof(vip->vip_path) - 1] = '\0';
    if (vip->vip_path[0] != '\0') {
        Lf->V_path = mkstrcpy(vip->vip_path, (MALLOC_S *)NULL);
    }
    /*
     * Save node number.
     */
    Lf->inode = (INODETYPE)vip->vip_vi.vi_stat.vst_ino;
    Lf->inp_ty = 1;
    /*
     * Save link count, as requested.
     */
    Lf->nlink = vip->vip_vi.vi_stat.vst_nlink;
    Lf->nlink_def = 1;
    if (Nlink && (Lf->nlink < Nlink))
        Lf->sf |= SELNLINK;
    /*
     * If a device number is defined, locate file system and save its identity.
     */
    if (devs) {
        for (mp = readmnt(ctx); mp; mp = mp->next) {
            if (dev == mp->dev) {
                Lf->fsdir = mp->dir;
                Lf->fsdev = mp->fsname;
                if (mp->is_nfs && Fnfs)
                    Lf->sf |= SELNFS;
                break;
            }
        }
    }
    /*
     * Save the file size.
     */
    switch (Ntype) {
    case N_CHR:
    case N_FIFO:
        break;
    default:
        Lf->sz = (SZOFFTYPE)vip->vip_vi.vi_stat.vst_size;
        Lf->sz_def = 1;
    }
    /*
     * Test for specified file.
     */
    if (Sfile &&
        is_file_named(ctx, NULL,
                      ((Ntype == N_CHR) || (Ntype == N_BLK) ? 1 : 0))) {
        Lf->sf |= SELNM;
    }
    /*
     * Enter name characters.
     */
    if (!Lf->nm && Namech[0])
        enter_nm(ctx, Namech);
}

/*
 * err2nm() -- convert errno to a message in Namech
 */
void err2nm(struct lsof_context *ctx, /* context */
            char *pfx)                /* Namech message prefix */
{
    char *sfx;

    switch (errno) {
    case EBADF:

        /*
         * The file descriptor is no longer available.
         */
        sfx = "FD unavailable";
        break;
    case ESRCH:

        /*
         * The process is no longer available.
         */
        sfx = "process unavailable";
        break;
    default:

        /*
         * All other errors are reported with strerror() information.
         */
        sfx = strerror(errno);
    }
    (void)snpf(Namech, Namechl, "%s: %s", pfx, sfx);
    enter_nm(ctx, Namech);
}

/*
 * print_nm() -- print Name column
 */
void print_nm(struct lsof_context *ctx, struct lfile *lf) {
    unsigned char extra = 0;

    printname(ctx, 0);

#if defined(PROC_PIDLISTFILEPORTS)
    if (lf->fileport)
        extra++;
#endif /* defined(PROC_PIDLISTFILEPORTS) */

#if defined(PROC_FP_GUARDED)
    if (lf->guardflags)
        extra++;
#endif /* defined(PROC_FP_GUARDED) */

    if (extra)
        (void)printf(" (");

#if defined(PROC_PIDLISTFILEPORTS)
    if (lf->fileport)
        (void)printf("fileport=0x%04x", lf->fileport);
#endif /* defined(PROC_PIDLISTFILEPORTS) */

#if defined(PROC_FP_GUARDED)
    if (extra > 1)
        putchar(',');
    if (lf->guardflags) {
        struct pff_tab *tp;
        long gf;

        (void)printf("guard=");
        tp = Pgf_tab;
        gf = lf->guardflags;
        while (gf && !FsvFlagX) {
            while (tp->nm) {
                if (gf & tp->val)
                    break;
                tp++;
            }
            if (!tp->nm)
                break;
            gf &= ~(tp->val);
            (void)printf("%s%s", tp->nm, gf ? "," : "");
        }
        /*
         * If flag bits remain, print them in hex.  If hex output was
         * specified with +fG, print all flag values, including zero,
         * in hex.
         */
        if (gf || FsvFlagX)
            (void)printf("0x%lx", gf);
    }
#endif /* defined(PROC_FP_GUARDED) */

    if (extra)
        (void)printf(")\n");
    else
        putchar('\n');
}

/*
 * print_v_path() -- print vnode's path
 */
int print_v_path(struct lsof_context *ctx, struct lfile *lf) {
    if (lf->V_path) {
        safestrprt(lf->V_path, stdout, 0);
        return (1);
    }
    return (0);
}

/*
 * process_atalk() -- process an Apple Talk file
 */
void process_atalk(struct lsof_context *ctx, /* context */
                   int pid,                  /* PID */
                   int32_t fd)               /* FD */
{
    Lf->type = LSOF_FILE_APPLETALK;
    return;
}

/*
 * process_fsevents() -- process a file system events file
 */
void process_fsevents(struct lsof_context *ctx, /* context */
                      int pid,                  /* PID */
                      int32_t fd)               /* FD */
{
    Lf->type = LSOF_FILE_FSEVENTS;
}

/*
 * process_kqueue() -- process a kernel queue file
 */
void process_kqueue(struct lsof_context *ctx, /* context */
                    int pid,                  /* PID */
                    int32_t fd)               /* FD */
{
    struct kqueue_fdinfo kq;
    int nb;
    /*
     * Get the kernel queue file information.
     */
    Lf->type = LSOF_FILE_KQUEUE;
    nb = proc_pidfdinfo(pid, fd, PROC_PIDFDKQUEUEINFO, &kq, sizeof(kq));
    if (nb <= 0) {
        (void)err2nm(ctx, "kqueue");
        return;
    } else if (nb < sizeof(kq)) {
        (void)fprintf(
            stderr,
            "%s: PID %d, FD %d; proc_pidfdinfo(PROC_PIDFDKQUEUEINFO);\n", Pn,
            pid, fd);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(kq), nb);
        Error(ctx);
    }
    /*
     * Enter the kernel queue file information.
     */
    enter_file_info(ctx, &kq.pfi);
    /*
     * Enter queue counts as NAME column information.
     */
    (void)snpf(Namech, Namechl, "count=%" SZOFFPSPEC "u, state=%#x",
               (SZOFFTYPE)kq.kqueueinfo.kq_stat.vst_size,
               kq.kqueueinfo.kq_state);
    enter_nm(ctx, Namech);
}

/*
 * process_pipe() -- process pipe file
 */
static void process_pipe_common(struct lsof_context *ctx,
                                struct pipe_fdinfo *pi) {
    char dev_ch[32], *ep;
    size_t sz;

    Lf->type = LSOF_FILE_PIPE;
    /*
     * Enter the pipe handle as the device.
     */
    (void)snpf(dev_ch, sizeof(dev_ch), "%s",
               print_kptr((KA_T)pi->pipeinfo.pipe_handle, (char *)NULL, 0));
    enter_dev_ch(ctx, dev_ch);
    /*
     * Enable offset or size reporting.
     */
    Lf->sz = (SZOFFTYPE)pi->pipeinfo.pipe_stat.vst_blksize;
    Lf->sz_def = 1;
    /*
     * If there is a peer handle, enter it in as NAME column information.
     */
    if (pi->pipeinfo.pipe_peerhandle) {
        (void)snpf(
            Namech, Namechl, "->%s",
            print_kptr((KA_T)pi->pipeinfo.pipe_peerhandle, (char *)NULL, 0));
        enter_nm(ctx, Namech);
    } else
        Namech[0] = '\0';
    /*
     * If the pipe has a count, add it to the NAME column.
     */
    if (pi->pipeinfo.pipe_stat.vst_size) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", cnt=%" SZOFFPSPEC "u",
                   (SZOFFTYPE)pi->pipeinfo.pipe_stat.vst_size);
    }
}

void process_pipe(struct lsof_context *ctx, /* context */
                  int pid,                  /* PID */
                  int32_t fd)               /* FD */
{
    int nb;
    struct pipe_fdinfo pi;
    /*
     * Get pipe file information.
     */
    nb = proc_pidfdinfo(pid, fd, PROC_PIDFDPIPEINFO, &pi, sizeof(pi));
    if (nb <= 0) {
        (void)err2nm(ctx, "pipe");
        return;
    } else if (nb < sizeof(pi)) {
        (void)fprintf(
            stderr, "%s: PID %d, FD %d; proc_pidfdinfo(PROC_PIDFDPIPEINFO);\n",
            Pn, pid, fd);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(pi), nb);
        Error(ctx);
    }

    process_pipe_common(ctx, &pi);
}

#if defined(PROC_PIDLISTFILEPORTS)
void process_fileport_pipe(struct lsof_context *ctx, /* context */
                           int pid,                  /* PID */
                           uint32_t fp)              /* FILEPORT */
{
    int nb;
    struct pipe_fdinfo pi;
    /*
     * Get pipe file information.
     */
    nb = proc_pidfileportinfo(pid, fp, PROC_PIDFILEPORTPIPEINFO, &pi,
                              sizeof(pi));
    if (nb <= 0) {
        (void)err2nm(ctx, "pipe");
        return;
    } else if (nb < sizeof(pi)) {
        (void)fprintf(stderr,
                      "%s: PID %d, FILEPORT %u; "
                      "proc_pidfileportinfo(PROC_PIDFILEPORTPIPEINFO);\n",
                      Pn, pid, fp);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(pi), nb);
        Error(ctx);
    }

    process_pipe_common(ctx, &pi);
}
#endif /* PROC_PIDLISTFILEPORTS */

/*
 * process_psem() -- process a POSIX semaphore file
 */
void process_psem(struct lsof_context *ctx, /* context */
                  int pid,                  /* PID */
                  int32_t fd)               /* FD */
{
    int nb;
    struct psem_fdinfo ps;
    /*
     * Get the semaphore file information.
     */
    Lf->type = LSOF_FILE_POSIX_SEMA;
    nb = proc_pidfdinfo(pid, fd, PROC_PIDFDPSEMINFO, &ps, sizeof(ps));
    if (nb <= 0) {
        (void)err2nm(ctx, "semaphore");
        return;
    } else if (nb < sizeof(ps)) {
        (void)fprintf(
            stderr, "%s: PID %d, FD %d; proc_pidfdinfo(PROC_PIDFDPSEMINFO);\n",
            Pn, pid, fd);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(ps), nb);
        Error(ctx);
    }
    /*
     * Enter the semaphore file information.
     */
    enter_file_info(ctx, &ps.pfi);
    /*
     * If there is a semaphore file name, enter it.
     */
    if (ps.pseminfo.psem_name[0]) {
        ps.pseminfo.psem_name[sizeof(ps.pseminfo.psem_name) - 1] = '\0';
        (void)snpf(Namech, Namechl, "%s", ps.pseminfo.psem_name);
        enter_nm(ctx, Namech);
    }
}

/*
 * process_pshm() -- process POSIX shared memory file
 */
static void process_pshm_common(struct lsof_context *ctx,
                                struct pshm_fdinfo *ps) {
    Lf->type = LSOF_FILE_POSIX_SHM;
    /*
     * Enter the POSIX shared memory file information.
     */
    enter_file_info(ctx, &ps->pfi);
    /*
     * If the POSIX shared memory file has a path name, enter it; otherwise,
     * if it has a mapping address, enter that.
     */
    if (ps->pshminfo.pshm_name[0]) {
        ps->pshminfo.pshm_name[sizeof(ps->pshminfo.pshm_name) - 1] = '\0';
        (void)snpf(Namech, Namechl, "%s", ps->pshminfo.pshm_name);
        enter_nm(ctx, Namech);
    } else if (ps->pshminfo.pshm_mappaddr) {
        (void)snpf(
            Namech, Namechl, "obj=%s",
            print_kptr((KA_T)ps->pshminfo.pshm_mappaddr, (char *)NULL, 0));
        enter_nm(ctx, Namech);
    }
    /*
     * Enable offset or size reporting.
     */
    Lf->sz = (SZOFFTYPE)ps->pshminfo.pshm_stat.vst_size;
    Lf->sz_def = 1;
}

void process_pshm(struct lsof_context *ctx, /* context */
                  int pid,                  /* PID */
                  int32_t fd)               /* FD */
{
    int nb;
    struct pshm_fdinfo ps;
    /*
     * Get the POSIX shared memory file information.
     */
    nb = proc_pidfdinfo(pid, fd, PROC_PIDFDPSHMINFO, &ps, sizeof(ps));
    if (nb <= 0) {
        (void)err2nm(ctx, "POSIX shared memory");
        return;
    } else if (nb < sizeof(ps)) {
        (void)fprintf(
            stderr, "%s: PID %d, FD %d; proc_pidfdinfo(PROC_PIDFDPSHMINFO);\n",
            Pn, pid, fd);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(ps), nb);
        Error(ctx);
    }

    process_pshm_common(ctx, &ps);
}

#if defined(PROC_PIDLISTFILEPORTS)
void process_fileport_pshm(struct lsof_context *ctx, /* context */
                           int pid,                  /* PID */
                           uint32_t fp)              /* FILEPORT */
{
    int nb;
    struct pshm_fdinfo ps;
    /*
     * Get the POSIX shared memory file information.
     */
    nb = proc_pidfileportinfo(pid, fp, PROC_PIDFILEPORTPSHMINFO, &ps,
                              sizeof(ps));
    if (nb <= 0) {
        (void)err2nm(ctx, "POSIX shared memory");
        return;
    } else if (nb < sizeof(ps)) {
        (void)fprintf(stderr,
                      "%s: PID %d, FILEPORT %u; "
                      "proc_pidfileportinfo(PROC_PIDFILEPORTPSHMINFO);\n",
                      Pn, pid, fp);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(ps), nb);
        Error(ctx);
    }

    process_pshm_common(ctx, &ps);
}
#endif /* PROC_PIDLISTFILEPORTS */

/*
 * process_vnode() -- process a vnode file
 */
static void process_vnode_common(struct lsof_context *ctx,
                                 struct vnode_fdinfowithpath *vi) {
    /*
     * Enter the file and vnode information.
     */
    enter_file_info(ctx, &vi->pfi);
    enter_vnode_info(ctx, &vi->pvip);
}

void process_vnode(struct lsof_context *ctx, /* context */
                   int pid,                  /* PID */
                   int32_t fd)               /* FD */
{
    int nb;
    struct vnode_fdinfowithpath vi;

    nb = proc_pidfdinfo(pid, fd, PROC_PIDFDVNODEPATHINFO, &vi, sizeof(vi));
    if (nb <= 0) {
        if (errno == ENOENT) {

            /*
             * The file descriptor's vnode may have been revoked.  This is a
             * bit of a hack, since an ENOENT error might not always mean
             * the descriptor's vnode has been revoked.  As the libproc API
             * matures, this code may need to be revisited.
             */
            enter_nm(ctx, "(revoked)");
        } else
            (void)err2nm(ctx, "vnode");
        return;
    } else if (nb < sizeof(vi)) {
        (void)fprintf(
            stderr,
            "%s: PID %d, FD %d: proc_pidfdinfo(PROC_PIDFDVNODEPATHINFO);\n", Pn,
            pid, fd);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(vi), nb);
        Error(ctx);
    }

    process_vnode_common(ctx, &vi);
}

#if defined(PROC_PIDLISTFILEPORTS)
void process_fileport_vnode(struct lsof_context *ctx, /* context */
                            int pid,                  /* PID */
                            uint32_t fp)              /* FILEPORT */
{
    int nb;
    struct vnode_fdinfowithpath vi;

    nb = proc_pidfileportinfo(pid, fp, PROC_PIDFILEPORTVNODEPATHINFO, &vi,
                              sizeof(vi));
    if (nb <= 0) {
        if (errno == ENOENT) {

            /*
             * The file descriptor's vnode may have been revoked.  This is a
             * bit of a hack, since an ENOENT error might not always mean
             * the descriptor's vnode has been revoked.  As the libproc API
             * matures, this code may need to be revisited.
             */
            enter_nm(ctx, "(revoked)");
        } else
            (void)err2nm(ctx, "vnode");
        return;
    } else if (nb < sizeof(vi)) {
        (void)fprintf(stderr,
                      "%s: PID %d, FILEPORT %u: "
                      "proc_pidfdinfo(PROC_PIDFDVNODEPATHINFO);\n",
                      Pn, pid, fp);
        (void)fprintf(stderr, "      too few bytes; expected %ld, got %d\n",
                      sizeof(vi), nb);
        Error(ctx);
    }

    process_vnode_common(ctx, &vi);
}
#endif /* PROC_PIDLISTFILEPORTS */
