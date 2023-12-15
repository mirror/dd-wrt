/*
 * dnode.c - FreeBSD node functions for lsof
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

#ifndef lint
static char copyright[] =
    "@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
#endif

#include "common.h"

#if defined(HAS_LOCKF_ENTRY)
#    include "./lockf_owner.h"
#endif /* defined(HAS_LOCKF_ENTRY) */

#if defined(HASPTSFN) && defined(DTYPE_PTS)
#    include <sys/tty.h>
#endif /* defined(HASPTSFN) && defined(DTYPE_PTS) */

static void get_lock_state_kvm(struct lsof_context *ctx, KA_T f);

/*
 * get_lock_state_*() - get the lock state
 */

#ifdef KERN_LOCKF
static void get_lock_state_sysctl(struct lsof_context *ctx,
                                  struct kinfo_file *kf,
                                  struct lock_list *locks) {
    struct kinfo_lockf key, *lock;

    key.kl_sysid = 0;
    key.kl_pid = (pid_t)Lp->pid;
    key.kl_file_fsid = kf->kf_un.kf_file.kf_file_fsid;
    key.kl_file_fileid = kf->kf_un.kf_file.kf_file_fileid;
    lock = bsearch(&key, locks->locks, locks->n_locks, sizeof(*locks->locks),
                   cmp_kinfo_lockf);
    if (lock != NULL) {
        int whole_file = (lock->kl_start == 0 && lock->kl_len == 0);
        if (lock->kl_rw == KLOCKF_RW_READ)
            Lf->lock =
                whole_file ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
        else if (lock->kl_rw == KLOCKF_RW_WRITE)
            Lf->lock =
                whole_file ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
    }
}
#endif /* KERN_LOCKF */

static void get_lock_state_kvm(struct lsof_context *ctx, /* context */
                               KA_T f) /* inode's lock pointer */
{
    struct lockf lf; /* lockf structure */
    int lt;          /* lock type */

#if defined(HAS_LOCKF_ENTRY)
    struct lockf_entry le; /* lock_entry structure */
    KA_T lef, lep;         /* lock_entry pointers */
    struct lock_owner lo;  /* lock owner structure */

    if (!f || kread(ctx, f, (char *)&lf, sizeof(lf)))
        return;
    if (!(lef = (KA_T)lf.ls_active.lh_first))
        return;
    lep = lef;
    do {
        if (kread(ctx, lep, (char *)&le, sizeof(le)))
            return;
        if (!le.lf_owner ||
            kread(ctx, (KA_T)le.lf_owner, (char *)&lo, sizeof(lo)))
            continue;
        if (lo.lo_pid == (pid_t)Lp->pid) {
            if (le.lf_start == (off_t)0 && le.lf_end == 0x7fffffffffffffffLL)
                lt = 1;
            else
                lt = 0;
            if (le.lf_type == F_RDLCK)
                Lf->lock = lt ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
            else if (le.lf_type == F_WRLCK)
                Lf->lock = lt ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
            return;
        }
    } while ((lep = (KA_T)le.lf_link.le_next) && (lep != lef));
#else /* !defined(HAS_LOCKF_ENTRY) */

    unsigned char l; /* lock status */
    KA_T lfp;        /* lockf structure pointer */

    if ((lfp = f)) {

        /*
         * Determine the lock state.
         */
        do {
            if (kread(ctx, lfp, (char *)&lf, sizeof(lf)))
                break;
            l = 0;
            switch (lf.lf_flags & (F_FLOCK | F_POSIX)) {
            case F_FLOCK:
                if (Cfp && (struct file *)lf.lf_id == Cfp)
                    l = 1;
                break;
            case F_POSIX:

#    if defined(P_ADDR)
                if ((KA_T)lf.lf_id == Kpa)
                    l = 1;
#    endif /* defined(P_ADDR) */

                break;
            }
            if (!l)
                continue;
            if (lf.lf_start == (off_t)0 && lf.lf_end == 0xffffffffffffffffLL)
                lt = 1;
            else
                lt = 0;
            if (lf.lf_type == F_RDLCK)
                Lf->lock = lt ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
            else if (lf.lf_type == F_WRLCK)
                Lf->lock = lt ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
            break;
        } while ((lfp = (KA_T)lf.lf_next) && (lfp != f));
    }
#endif     /* defined(HAS_LOCKF_ENTRY) */
}

#if defined(HASKQUEUE)
/*
 * process_kf_kqueue() - process kqueue file
 *
 * Strictly speaking this function should appear in dfile.c, because it is
 * a file processing function.  However, the Net and Open BSD sources don't
 * require a dfile.c, so this is the next best location for the function.
 */
void process_kf_kqueue(struct lsof_context *ctx, /* context */
                       struct kinfo_file *kf,    /* kernel file*/
                       KA_T ka /* kernel address */) {
#    if __FreeBSD_version < 1400062
    struct kqueue kq; /* kqueue structure */
#    endif            /* __FreeBSD_version < 1400062 */

    Lf->type = LSOF_FILE_KQUEUE;
    enter_dev_ch(ctx, print_kptr(ka, (char *)NULL, 0));
#    if __FreeBSD_version >= 1400062
    (void)snpf(Namech, Namechl, "count=%d, state=%#x",
               kf->kf_un.kf_kqueue.kf_kqueue_count,
               kf->kf_un.kf_kqueue.kf_kqueue_state);
#    else  /* __FreeBSD_version < 1400062 */
    if (!ka || kread(ctx, ka, (char *)&kq, sizeof(kq)))
        return;
    (void)snpf(Namech, Namechl, "count=%d, state=%#x", kq.kq_count,
               kq.kq_state);
#    endif /* __FreeBSD_version >= 1400062 */
    enter_nm(ctx, Namech);
}
#endif /* defined(HASKQUEUE) */

#if defined(KF_TYPE_EVENTFD)
void process_eventfd(struct lsof_context *ctx, struct kinfo_file *kf) {
    Lf->type = LSOF_FILE_EVENTFD;
#    if __FreeBSD_version >= 1400062
    enter_dev_ch(
        ctx, print_kptr(kf->kf_un.kf_eventfd.kf_eventfd_addr, (char *)NULL, 0));
#    endif /* __FreeBSD_version >= 1400062 */
    (void)snpf(Namech, Namechl, "value=%ju, flags=0x%x",
               kf->kf_un.kf_eventfd.kf_eventfd_value,
               kf->kf_un.kf_eventfd.kf_eventfd_flags);
    enter_nm(ctx, Namech);
}
#endif /* defined(KF_TYPE_EVENTFD) */

void process_shm(struct lsof_context *ctx, struct kinfo_file *kf) {
    Lf->type = LSOF_FILE_SHM;
    Lf->sz = kf->kf_un.kf_file.kf_file_size;
    Lf->sz_def = 1;
    Lf->off_def = 0;
    if (kf->kf_un.kf_file.kf_file_fileid != VNOVAL) {
        Lf->inode = kf->kf_un.kf_file.kf_file_fileid;
        Lf->inp_ty = 1;
    }
    if (kf->kf_path[0]) {
        snpf(Namech, Namechl, "%s", kf->kf_path);
        enter_nm(ctx, Namech);
    }
}

void process_procdesc(struct lsof_context *ctx, struct kinfo_file *kf) {
    char pidstr[50];

    Lf->type = LSOF_FILE_PROCDESC;
    snpf(pidstr, sizeof(pidstr), "pid=%d", kf->kf_un.kf_proc.kf_pid);
    add_nma(ctx, pidstr, strlen(pidstr));
    if (kf->kf_path[0]) {
        snpf(Namech, Namechl, "%s", kf->kf_path);
        enter_nm(ctx, Namech);
    }
}

static enum lsof_file_type parse_proc_path(struct kinfo_file *kf,
                                           int *proc_pid) {
    enum lsof_file_type ty;
    char *basename;

    ty = LSOF_FILE_NONE;
    basename = strrchr(kf->kf_path, '/');
    if (basename) {
        ++basename;
        if (!strcmp(basename, "cmdline")) {
        } else if (!strcmp(basename, "dbregs")) {
        } else if (!strcmp(basename, "etype")) {
            ty = LSOF_FILE_PROC_EXEC_TYPE;
        } else if (!strcmp(basename, "file")) {
            ty = LSOF_FILE_PROC_FILE;
        } else if (!strcmp(basename, "fpregs")) {
            ty = LSOF_FILE_PROC_FP_REGS;
        } else if (!strcmp(basename, "map")) {
            ty = LSOF_FILE_PROC_MAP;
        } else if (!strcmp(basename, "mem")) {
            ty = LSOF_FILE_PROC_MEMORY;
        } else if (!strcmp(basename, "note")) {
            ty = LSOF_FILE_PROC_PROC_NOTIFIER;
        } else if (!strcmp(basename, "notepg")) {
            ty = LSOF_FILE_PROC_GROUP_NOTIFIER;
        } else if (!strcmp(basename, "osrel")) {
        } else if (!strcmp(basename, "regs")) {
            ty = LSOF_FILE_PROC_REGS;
        } else if (!strcmp(basename, "rlimit")) {
        } else if (!strcmp(basename, "status")) {
            ty = LSOF_FILE_PROC_STATUS;
        } else {
            /* we're excluded all files - must be a directory, either
             * /proc/<pid> or /proc itself */
            ty = LSOF_FILE_PROC_DIR;
        }
        if (ty != LSOF_FILE_NONE && ty != LSOF_FILE_PROC_DIR) {
            char *parent_dir;
            --basename;
            *basename = '\0';
            parent_dir = strrchr(kf->kf_path, '/');
            if (parent_dir)
                *proc_pid = strtol(++parent_dir, NULL, 10);
            *basename = '/';
        }
    }
    return ty;
}

/*
 * process_vnode() - process vnode
 */

void process_vnode(struct lsof_context *ctx, struct kinfo_file *kf,
                   struct xfile *xfile, struct lock_list *locks) {
    dev_t dev = 0, rdev = 0;
    unsigned char devs;
    unsigned char rdevs;
    KA_T va;
    struct vnode *v, vb;
    struct l_vfs *vfs;
    uint64_t fsid;
    char vfs_path[PATH_MAX];

#if defined(HASNULLFS)
#    if !defined(HASPRINTDEV)
    char dbuf[32];
#    endif /* !defined(HASPRINTDEV) */
    char *dp, *np, tbuf[1024];
    struct null_node nu;
    int sc = 0;
#endif /* defined(HASNULLFS) */

    int proc_pid = 0;
    struct procfsid *pfi;

    struct stat st;
    const int kf_vtype = kf->kf_vnode_type;

    fsid = kf->kf_un.kf_file.kf_file_fsid;
    va = xfile ? xfile->xf_vnode : 0;
    strcpy(vfs_path, kf->kf_path);

#if defined(HASNULLFS)

process_overlaid_node:

    if (++sc > 1024) {
        (void)snpf(Namech, Namechl, "too many overlaid nodes");
        enter_nm(ctx, Namech);
        return;
    }
#endif /* defined(HASNULLFS) */

    /*
     * Initialize miscellaneous variables.  This is done so that processing an
     * overlaid node will be a fresh start.
     */
    devs = rdevs = 0;
    Namech[0] = '\0';

    /*
     * Read the vnode.
     */
    v = NULL;
    if (va) {
        v = &vb;
        if (kread(ctx, (KA_T)va, (char *)v, sizeof(struct vnode)))
            v = NULL;
    }

    if (xfile) {

#if defined(HASFSTRUCT)
        Lf->fna = (KA_T)xfile->xf_vnode;
        Lf->fsv |= FSV_NI;
#endif /* defined(HASFSTRUCT) */
    }

    /*
     * Get the vnode type.
     */
    vfs = readvfs(ctx, fsid, vfs_path);
    if (vfs) {
        fsid = vfs->fsid;

#if defined(MOUNT_NONE)
        switch (vfs->type) {
        case MOUNT_NFS:
            Ntype = N_NFS;
            break;

        case MOUNT_PROCFS:
            Ntype = N_PROC;
            break;
        }
#else /* !defined(MOUNT_NONE) */
        if (strcasecmp(vfs->typnm, "nfs") == 0)
            Ntype = N_NFS;

        else if (strcasecmp(vfs->typnm, "procfs") == 0)
            Ntype = N_PROC;

#    if defined(HASPSEUDOFS)
        else if (strcasecmp(vfs->typnm, "pseudofs") == 0)
            Ntype = N_PSEU;
#    endif /* defined(HASPSEUDOFS) */

#    if defined(HAS_TMPFS)
        else if (strcasecmp(vfs->typnm, "tmpfs") == 0)
            Ntype = N_TMP;
#    endif /* defined(HAS_TMPFS) */
#endif     /* defined(MOUNT_NONE) */
    }
    if (Ntype == N_REGLR) {
        switch (kf_vtype) {
        case KF_VTYPE_VFIFO:
            Ntype = N_FIFO;
            break;
        default:
            break;
        }
    }

#ifdef KERN_LOCKF
    get_lock_state_sysctl(ctx, kf, locks);
#elif defined(HAS_V_LOCKF)
    if (v && v->v_lockf)
        (void)get_lock_state_kvm(ctx, (KA_T)v->v_lockf);
#endif /* KERN_LOCKF */

    /*
     * Deal with special filesystems.
     */
    if (vfs && (!strcmp(vfs->typnm, "null") || !strcmp(vfs->typnm, "nullfs"))) {

#if defined(HASNULLFS)
        if (sc == 1) {

            /*
             * If this is the first null_node, enter a name addition containing
             * the mounted-on directory, the file system name, and the device
             * number.
             */
            if (vfs && (vfs->dir || vfs->fsname || vfs->fsid != VNOVAL)) {
                if (vfs->fsid != VNOVAL) {

#    if defined(HASPRINTDEV)
                    dp = HASPRINTDEV(Lf, &vfs->fsid);
#    else  /* !defined(HASPRINTDEV) */
                    (void)snpf(dbuf, sizeof(dbuf) - 1, "%d,%d",
                               GET_MAJ_DEV(vfs->fsid), GET_MIN_DEV(vfs->fsid));
                    dbuf[sizeof(dbuf) - 1] = '\0';
                    dp = dbuf;
#    endif /* defined(HASPRINTDEV) */

                } else
                    dp = (char *)NULL;
                (void)snpf(tbuf, sizeof(tbuf) - 1, "(nullfs%s%s%s%s%s%s%s)",
                           (vfs && vfs->fsname) ? " " : "",
                           (vfs && vfs->fsname) ? vfs->fsname : "",
                           (vfs && vfs->dir) ? " on " : "",
                           (vfs && vfs->dir) ? vfs->dir : "",
                           (dp && vfs && vfs->dir) ? " (" : "",
                           (dp && vfs && vfs->dir) ? dp : "",
                           (dp && vfs && vfs->dir) ? ")" : "");
                tbuf[sizeof(tbuf) - 1] = '\0';
                np = tbuf;
            } else
                np = "(nullfs)";
            (void)add_nma(ctx, np, (int)strlen(np));
        }
        fsid = VNOVAL;
        /* -------dir--------
         * /nullfs_mountpoint/path/to/file
         * /original_mountpoint/path/to/file
         * ------fsname--------
         */
        memmove(&vfs_path[strlen(vfs->fsname) + 1],
                &vfs_path[strlen(vfs->dir) + 1],
                strlen(vfs_path) - strlen(vfs->dir) + 1);
        memcpy(vfs_path, vfs->fsname, strlen(vfs->fsname));
        goto process_overlaid_node;
#endif /* defined(HASNULLFS) */
    }

    /*
     * Get device and type for printing.
     */
    if (fsid != VNOVAL) {
        dev = fsid;
        devs = 1;
    }
    if (kf_vtype == KF_VTYPE_VCHR || kf_vtype == KF_VTYPE_VBLK) {
        rdev = kf->kf_un.kf_file.kf_file_rdev;
        rdevs = 1;
    }

    /*
     * Obtain the inode number.
     */
    if (kf->kf_un.kf_file.kf_file_fileid != VNOVAL) {
        Lf->inode = kf->kf_un.kf_file.kf_file_fileid;
        Lf->inp_ty = 1;
    }

    /*
     * Obtain the file size.
     */
    switch (Ntype) {
    case N_FIFO:
        break;
    case N_PROC:
        Lf->sz = kf->kf_un.kf_file.kf_file_size;
        Lf->sz_def = 1;
        break;
#if defined(HASPSEUDOFS)
    case N_PSEU:
        Lf->sz = 0;
        Lf->sz_def = 1;
        break;
#endif /* defined(PSEUDOFS) */
    case N_REGLR:
#if defined(HAS_TMPFS)
    case N_TMP:
#endif /* defined(HAS_TMPFS) */
        if (kf_vtype == KF_VTYPE_VREG || kf_vtype == KF_VTYPE_VDIR) {
            Lf->sz = kf->kf_un.kf_file.kf_file_size;
            Lf->sz_def = 1;
        }
        break;
    default:
        Lf->sz = kf->kf_un.kf_file.kf_file_size;
        Lf->sz_def = 1;
    }
    /*
     * Record the link count.
     */
    /* Read nlink from kernel if provided, otherwise call stat() */
#if defined(HAS_KF_FILE_NLINK)
    Lf->nlink = kf->kf_un.kf_file.kf_file_nlink;
    Lf->nlink_def = 1;
#else
    if (kf->kf_path[0] && stat(kf->kf_path, &st) == 0) {
        Lf->nlink = st.st_nlink;
        Lf->nlink_def = 1;
    }
#endif
    if (Lf->nlink_def && Nlink && (Lf->nlink < Nlink))
        Lf->sf |= SELNLINK;
    /*
     * Record an NFS file selection.
     */
    if (Ntype == N_NFS && Fnfs)
        Lf->sf |= SELNFS;
    /*
     * Save the file system names.
     */
    if (vfs) {
        Lf->fsdir = vfs->dir;
        Lf->fsdev = vfs->fsname;
    }
    /*
     * Save the device numbers and their states.
     *
     * Format the vnode type, and possibly the device name.
     */
    Lf->dev = dev;
    Lf->dev_def = devs;
    Lf->rdev = rdev;
    Lf->rdev_def = rdevs;
    switch (kf_vtype) {
    case KF_VTYPE_VNON:
        Lf->type = LSOF_FILE_VNODE_VNON;
        break;
    case KF_VTYPE_VREG:
    case KF_VTYPE_VDIR:
        Lf->type = (kf_vtype == KF_VTYPE_VREG) ? LSOF_FILE_VNODE_VREG
                                               : LSOF_FILE_VNODE_VDIR;
        break;
    case KF_VTYPE_VBLK:
        Lf->type = LSOF_FILE_VNODE_VBLK;
        Ntype = N_BLK;
        break;
    case KF_VTYPE_VCHR:
        Lf->type = LSOF_FILE_VNODE_VCHR;
        Ntype = N_CHR;
        break;
    case KF_VTYPE_VLNK:
        Lf->type = LSOF_FILE_VNODE_VLNK;
        break;
    case KF_VTYPE_VSOCK:
        Lf->type = LSOF_FILE_VNODE_VSOCK;
        break;
    case KF_VTYPE_VBAD:
        Lf->type = LSOF_FILE_VNODE_VBAD;
        break;
    case KF_VTYPE_VFIFO:
        Lf->type = LSOF_FILE_VNODE_VFIFO;
        break;
    default:
        Lf->type = LSOF_FILE_UNKNOWN_RAW;
        Lf->unknown_file_type_number = kf_vtype;
    }
    Lf->ntype = Ntype;
    /*
     * Handle some special cases:
     *
     * 	ioctl(fd, TIOCNOTTY) files;
     *	memory node files;
     *	/proc files.
     */

    if (kf_vtype == KF_VTYPE_VBAD)
        (void)snpf(Namech, Namechl, "(revoked)");

    else if (Ntype == N_PROC) {
        Lf->type = parse_proc_path(kf, &proc_pid);
    }

#if defined(HASBLKDEV)
    /*
     * If this is a VBLK file and it's missing an inode number, try to
     * supply one.
     */
    if ((Lf->inp_ty == 0) && (kf_vtype == KF_VTYPE_VBLK))
        find_bl_ino();
#endif /* defined(HASBLKDEV) */

    /*
     * If this is a VCHR file and it's missing an inode number, try to
     * supply one.
     */
    if ((Lf->inp_ty == 0) && (kf_vtype == KF_VTYPE_VCHR))
        find_ch_ino(ctx);
    /*
     * Test for specified file.
     */

    if (Ntype == N_PROC && (Procsrch || Procfsid)) {
        if (Procsrch) {
            Procfind = 1;
            Lf->sf |= SELNM;
        } else {
            for (pfi = Procfsid; pfi; pfi = pfi->next) {
                if ((pfi->pid && proc_pid && pfi->pid == proc_pid)

#if defined(HASPINODEN)
                    || (Lf->inp_ty == 1 && Lf->inode == pfi->inode)
#else  /* !defined(HASPINODEN) */
                        if (pfi->pid == p->pfs_pid)
#endif /* defined(HASPINODEN) */

                ) {
                    pfi->f = 1;
                    if (!Namech[0])
                        (void)snpf(Namech, Namechl, "%s", pfi->nm);
                    Lf->sf |= SELNM;
                    break;
                }
            }
        }
    } else {
        if (Sfile && is_file_named(ctx, (char *)NULL,
                                   ((kf_vtype == KF_VTYPE_VCHR) ||
                                    (kf_vtype == KF_VTYPE_VBLK))
                                       ? 1
                                       : 0))
            Lf->sf |= SELNM;
    }
    /*
     * Enter name characters.
     */
    if (Namech[0])
        enter_nm(ctx, Namech);
    else if (kf->kf_path[0]) {
        snpf(Namech, Namechl, "%s", kf->kf_path);
        if (vfs && vfs->fsname) {
            char *cp;
            size_t sz;
            cp = endnm(ctx, &sz);
            snpf(cp, sz, " (%s)", vfs->fsname);
        }
        enter_nm(ctx, Namech);
    }
}

/*
 * process_pipe() - process a file structure whose type is DTYPE_PIPE
 */

void process_pipe(struct lsof_context *ctx, struct kinfo_file *kf, KA_T pa) {
    char dev_ch[32], *ep;
    size_t sz;

#if __FreeBSD_version < 1400062
    struct pipe p;
    int have_kpipe = (pa && kread(ctx, pa, (char *)&p, sizeof(p)) == 0);
#endif

    Lf->type = LSOF_FILE_PIPE;
    (void)snpf(dev_ch, sizeof(dev_ch), "%s",
               print_kptr(kf->kf_un.kf_pipe.kf_pipe_addr, (char *)NULL, 0));
    enter_dev_ch(ctx, dev_ch);
#if __FreeBSD_version >= 1400062
    Lf->sz = (SZOFFTYPE)kf->kf_un.kf_pipe.kf_pipe_buffer_size;
    Lf->sz_def = 1;
#else  /* __FreeBSD_version < 1400062 */
    if (have_kpipe) {
        Lf->sz = (SZOFFTYPE)p.pipe_buffer.size;
        Lf->sz_def = 1;
    }
#endif /* __FreeBSD_version >= 1400062 */
    if (kf->kf_un.kf_pipe.kf_pipe_peer)
        (void)snpf(
            Namech, Namechl, "->%s",
            print_kptr((KA_T)kf->kf_un.kf_pipe.kf_pipe_peer, (char *)NULL, 0));
    else
        Namech[0] = '\0';
    if (kf->kf_un.kf_pipe.kf_pipe_buffer_cnt) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", cnt=%d", kf->kf_un.kf_pipe.kf_pipe_buffer_cnt);
    }
#if __FreeBSD_version >= 1400062
    if (kf->kf_un.kf_pipe.kf_pipe_buffer_in) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", in=%d", kf->kf_un.kf_pipe.kf_pipe_buffer_in);
    }
    if (kf->kf_un.kf_pipe.kf_pipe_buffer_out) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", out=%d", kf->kf_un.kf_pipe.kf_pipe_buffer_out);
    }
#else  /* __FreeBSD_version < 1400062 */
    if (have_kpipe && p.pipe_buffer.in) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", in=%d", p.pipe_buffer.in);
    }
    if (have_kpipe && p.pipe_buffer.out) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", out=%d", p.pipe_buffer.out);
    }
#endif /* __FreeBSD_version >= 1400062 */
       /*
        * Enter name characters.
        */
    if (Namech[0])
        enter_nm(ctx, Namech);
}

#if defined(DTYPE_PTS)
/*
 * process_pts - process a file structure whose type is DTYPE_PTS
 */

void process_pts(struct lsof_context *ctx, struct kinfo_file *kf) {
    Lf->type = LSOF_FILE_PTS;
    /*
     * Convert the tty's cdev from kernel to user form.
     *     -- already done in the kernel, file sys/kern/tty_pts.c, function
     * ptsdev_fill_kinfo().
     *
     * Set the device number to DevDev, the device number of /dev.
     *
     * Set the inode number to the device number.
     *
     * Set the file type to N_CHR for a character device (That's what a PTS is.)
     *
     * Force the use of offset from file structure.
     *
     * Set rdev to the converted device.
     *
     * Force the reloading of the device cache.
     */
    Lf->dev = DevDev;
    Lf->inode = (INODETYPE)kf->kf_un.kf_pts.kf_pts_dev;
    Lf->inp_ty = Lf->dev_def = Lf->rdev_def = 1;
    Lf->ntype = N_CHR;
    Lf->rdev = kf->kf_un.kf_pts.kf_pts_dev;
    DCunsafe = 1;
}
#endif /* defined(DTYPE_PTS) */
