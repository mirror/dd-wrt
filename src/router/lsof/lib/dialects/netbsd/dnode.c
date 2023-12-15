/*
 * dnode.c - NetBSD node functions for lsof
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

#if defined(HAS_LOCKF_H)
#    include "lockf.h"
#endif

#if defined(HAS_DINODE_U)
#    define DINODE_U dinode_u
#else /* !defined(HAS_DINODE_U) */
#    define DINODE_U i_din
#endif /* defined(HAS_DINODE_U) */

#if defined(HASFDESCFS) && HASFDESCFS == 1
static int lkup_dev_tty(struct lsof_context *ctx, dev_t *dr, INODETYPE *ir);
#endif /* defined(HASFDESCFS) && HASFDESCFS==1 */

#if defined(HAS_UM_UFS)
#    define UFS1 UM_UFS1
#    define UFS2 UM_UFS2
#endif /* defined(HAS_UM_UFS) */

#if defined(HASPROCFS)
static void getmemsz(struct lsof_context *ctx, pid_t pid);

#    if !defined(PGSHIFT)
#        define PGSHIFT pgshift
#    endif /* !defined(PGSHIFT) */

/*
 * getmemsz() - get memory size of a /proc/<n>/mem entry
 */

static void getmemsz(struct lsof_context *ctx, pid_t pid) {
    int n;
    struct vmspace vm;

#    if defined(HASKVMGETPROC2)
    struct kinfo_proc2 *p;
#    else  /* !defined(HASKVMGETPROC2) */
    struct kinfo_proc *p;
#    endif /* defined(HASKVMGETPROC2) */

    for (n = 0, p = P; n < Np; n++, p++) {
        if (p->P_PID == pid) {
            if (!p->P_VMSPACE ||
                kread(ctx, (KA_T)p->P_VMSPACE, (char *)&vm, sizeof(vm)))
                return;
            Lf->sz = (SZOFFTYPE)ctob(vm.vm_tsize + vm.vm_dsize + vm.vm_ssize);
            Lf->sz_def = 1;
            return;
        }
    }
}
#    undef PGSHIFT
#endif /* defined(HASPROCFS) */

#if defined(HASFDESCFS) && HASFDESCFS == 1
/*
 * lkup_dev_tty() - look up /dev/tty
 */

static int lkup_dev_tty(struct lsof_context *ctx, /* context */
                        dev_t *dr,     /* place to return device number */
                        INODETYPE *ir) /* place to return inode number */
{
    int i;

    readdev(ctx, 0);

#    if defined(HASDCACHE)

lkup_dev_tty_again:

#    endif /* defined(HASDCACHE) */

    for (i = 0; i < Ndev; i++) {
        if (strcmp(Devtp[i].name, "/dev/tty") == 0) {

#    if defined(HASDCACHE)
            if (DCunsafe && !Devtp[i].v && !vfy_dev(ctx, &Devtp[i]))
                goto lkup_dev_tty_again;
#    endif /* defined(HASDCACHE) */

            *dr = Devtp[i].rdev;
            *ir = Devtp[i].inode;
            return (1);
        }
    }

#    if defined(HASDCACHE)
    if (DCunsafe) {
        (void)rereaddev(ctx);
        goto lkup_dev_tty_again;
    }
#    endif /* defined(HASDCACHE) */

    return (-1);
}
#endif /* defined(HASFDESCFS) && HASFDESCFS==1 */

#if defined(HASKQUEUE)
/*
 * process_kqueue() -- process kqueue file
 *
 * Strictly speaking this function should appear in dfile.c, because it is
 * a file processing function.  However, the Net and Open BSD sources don't
 * require a dfile.c, so this is the next best location for the function.
 */

void process_kqueue(struct lsof_context *ctx, /* context */
                    KA_T ka) /* kqueue file structure address */
{
    Lf->type = LSOF_FILE_KQUEUE;
    enter_dev_ch(ctx, print_kptr(ka, (char *)NULL, 0));
}
#endif /* defined(HASKQUEUE) */

/*
 * process_node() - process vnode
 */

void process_node(struct lsof_context *ctx, /* context */
                  KA_T va)                  /* vnode kernel space address */
{
    dev_t dev, rdev;
    unsigned char devs;
    unsigned char lt;
    unsigned char ns;
    unsigned char rdevs;
    char *ep;
#if defined(HAS_LOCKF_H)
    struct lockf lf, *lff, *lfp;
#endif
    struct inode i;
    struct mfsnode m;
#if defined(HASTMPFS)
    struct tmpfs_node tmp;
#endif /* defined(HASTMPFS) */
    struct nfsnode n;
    enum nodetype {
        NONODE,
        CDFSNODE,
        DOSNODE,
        EXT2NODE,
        FDESCNODE,
        INODE,
        KERNFSNODE,
        MFSNODE,
        NFSNODE,
        PFSNODE,
        PTYFSNODE,
        TMPFSNODE
    } nty;
    enum vtype type;
    struct vnode *v, vb;
    struct l_vfs *vfs;

#if defined(HAS9660FS)
    dev_t iso_dev;
    INODETYPE iso_ino;
    long iso_nlink;
    int iso_stat;
    SZOFFTYPE iso_sz;
#endif /* defined(HAS9660FS) */

#if defined(HASFDESCFS)
    struct fdescnode f;

#    if HASFDESCFS == 1
    static dev_t f_tty_dev;
    static INODETYPE f_tty_ino;
    static int f_tty_s = 0;
#    endif /* HASFDESCFS==1 */

#endif /* defined(HASFDESCFS) */

#if defined(HASEXT2FS)
#    if defined(HASI_E2FS_PTR)
    struct ext2fs_dinode ed;
#    endif /* defined(HASI_E2FS_PTR) */
    struct ext2fs_dinode *edp = (struct ext2fs_dinode *)NULL;
#endif /* defined(HASEXT2FS) */

#if defined(HASI_FFS1)
    unsigned char ffs = 0;
    unsigned char u1s = 0;
    unsigned char u2s = 0;
    struct ufs1_dinode u1;
    struct ufs2_dinode u2;
    struct ufsmount um;
#endif /* defined(HASI_FFS1) */

#if defined(HASKERNFS)
    struct kernfs_node kn;
    struct stat ksb;
    int ksbs;
    struct kern_target kt;
    int ktnl;
    char ktnm[MAXPATHLEN + 1];
#endif /* defined(HASKERNFS) */

#if defined(HASMSDOSFS)
    struct denode d;
    u_long dpb;
    INODETYPE nn;
    struct msdosfsmount pm;
#endif /* defined(HASMSDOSFS) */

#if defined(HASNFSVATTRP)
    struct vattr nv;
#    define NVATTR nv
#else /* !defined(HASNFSVATTRP) */
#    define NVATTR n.n_vattr
#endif /* defined(HASNFSVATTRP) */

#if defined(HASNULLFS)
    struct null_node nu;
    int sc = 0;
    struct l_vfs *nvfs = (struct l_vfs *)NULL;
#endif /* defined(HASNULLFS) */

#if defined(HASPROCFS)
    struct pfsnode p;
    struct procfsid *pfi;
    size_t sz;
#endif /* defined(HASPROCFS) */

#if defined(HASPTYFS)
    struct ptyfsnode pt;
#    if __NetBSD_Version__ >= 499006200
#        define specinfo specnode
#        define vu_specinfo vu_specnode
#        define si_rdev sn_rdev
#    endif
    struct specinfo si;
#endif /* defined(HASPTYFS) */

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
    nty = NONODE;
    Namech[0] = '\0';

#if defined(HAS9660FS)
    iso_stat = 0;
#endif /* defined(HAS9660FS) */

#if defined(HASKERNFS)
    ksbs = 0;
#endif /* defined(HASKERNFS) */

#if defined(HASEXT2FS)
    edp = (struct ext2fs_dinode *)NULL;
#endif /* defined(HASEXT2FS) */

#if defined(HASI_FFS1)
    ffs = u1s = u2s = 0;
#endif /* defined(HASI_FFS1) */

    /*
     * Read the vnode.
     */
    if (!va) {
        enter_nm(ctx, "no vnode address");
        return;
    }
    v = &vb;
    if (readvnode(ctx, va, v)) {
        enter_nm(ctx, Namech);
        return;
    }

#if defined(HASNCACHE)
    Lf->na = va;
#    if defined(HASNCVPID)
    Lf->id = v->v_id;
#    endif /* defined(HASNCVPID) */
#endif     /* defined(HASNCACHE) */

#if defined(HASFSTRUCT)
    Lf->fna = va;
    Lf->fsv |= FSV_NI;
#endif /* defined(HASFSTRUCT) */

    /*
     * Get the vnode type.
     */
    if (!v->v_mount)
        vfs = (struct l_vfs *)NULL;
    else {
        vfs = readvfs(ctx, (KA_T)v->v_mount);
        if (vfs) {
            if (strcmp(vfs->type, MOUNT_NFS) == 0)
                Ntype = N_NFS;

#if defined(HASKERNFS)
            else if (strcmp(vfs->type, MOUNT_KERNFS) == 0)
                Ntype = N_KERN;
#endif /* defined(HASKERNFS) */

#if defined(HASPROCFS)
            else if (strcmp(vfs->type, MOUNT_PROCFS) == 0)
                Ntype = N_PROC;
#endif /* defined(HASPROCFS) */

#if defined(HAS9660FS)
            else if (strcmp(vfs->type, MOUNT_CD9660) == 0)
                Ntype = N_CDFS;
#endif /* defined(HAS9660FS) */
        }
    }
    if (Ntype == N_REGLR) {
        switch (v->v_type) {
        case VFIFO:
            Ntype = N_FIFO;
            break;
        }
    }
    /*
     * Read the successor node.
     */
    switch (v->v_tag) {

#if defined(HAS9660FS)
    case VT_ISOFS:
        if (read_iso_node(ctx, v, &iso_dev, &iso_ino, &iso_nlink, &iso_sz)) {
            (void)snpf(Namech, Namechl, "can't read iso_node at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        iso_stat = 1;
        nty = CDFSNODE;
        break;
#endif /* defined(HAS9660FS) */

#if defined(HASFDESCFS)
    case VT_FDESC:
        if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&f, sizeof(f))) {
            (void)snpf(Namech, Namechl, "can't read fdescnode at: %x",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        nty = FDESCNODE;
        break;
#endif /* defined(HASFDESCFS) */

#if defined(HASKERNFS)
    case VT_KERNFS:

        /*
         * Read the kernfs_node.
         */
        if (!v->v_data ||
            kread(ctx, (KA_T)v->v_data, (char *)&kn, sizeof(kn))) {
            if (v->v_type != VDIR || !(v->VNODE_VFLAG && NCACHE_VROOT)) {
                (void)snpf(Namech, Namechl, "can't read kernfs_node at: %s",
                           print_kptr((KA_T)v->v_data, (char *)NULL, 0));
                enter_nm(ctx, Namech);
                return;
            } else
                kn.kf_kt = (struct kern_target *)NULL;
        }
        /*
         * Generate the /kern file name by reading the kern_target to which
         * the kernfs_node points.
         */
        if (kn.kf_kt &&
            kread(ctx, (KA_T)kn.kf_kt, (char *)&kt, sizeof(kt)) == 0 &&
            (ktnl = (int)kt.kt_namlen) > 0 && kt.kt_name) {
            if (ktnl > (sizeof(ktnm) - 1))
                ktnl = sizeof(ktnm) - 1;
            if (!kread(ctx, (KA_T)kt.kt_name, ktnm, ktnl)) {
                ktnm[ktnl] = 0;
                ktnl = strlen(ktnm);
                if (ktnl > (MAXPATHLEN - strlen(_PATH_KERNFS) - 2)) {
                    ktnl = MAXPATHLEN - strlen(_PATH_KERNFS) - 2;
                    ktnm[ktnl] = '\0';
                }
                (void)snpf(Namech, Namechl, "%s/%s", _PATH_KERNFS, ktnm);
            }
        }
        /*
         * If this is the /kern root directory, its name, inode number and
         * size are fixed; otherwise, safely stat() the file to get the
         * inode number and size.
         */
        if (v->v_type == VDIR && (v->VNODE_VFLAG & NCACHE_VROOT)) {
            (void)snpf(Namech, Namechl, "%s", _PATH_KERNFS);
            ksb.st_ino = (ino_t)2;
            ksb.st_size = DEV_BSIZE;
            ksbs = 1;
        } else if (Namech[0] && statsafely(Namech, &ksb) == 0)
            ksbs = 1;
        nty = KERNFSNODE;
        break;
#endif /* defined(HASKERNFS) */

    case VT_MFS:
        if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&m, sizeof(m))) {
            (void)snpf(Namech, Namechl, "can't read mfsnode at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        nty = MFSNODE;
        break;

#if defined(HASTMPFS)
    case VT_TMPFS:
        if (!v->v_data ||
            kread(ctx, (KA_T)v->v_data, (char *)&tmp, sizeof(tmp))) {
            (void)snpf(Namech, Namechl, "can't read tmpfs_node at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        nty = TMPFSNODE;
        break;
#endif /* defined(HASTMPFS) */

#if defined(HASMSDOSFS)
    case VT_MSDOSFS:
        if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&d, sizeof(d))) {
            (void)snpf(Namech, Namechl, "can't read denode at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        nty = DOSNODE;
        break;
#endif /* defined(HASMSDOSFS) */

    case VT_NFS:
        if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&n, sizeof(n))) {
            (void)snpf(Namech, Namechl, "can't read nfsnode at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }

#if defined(HASNFSVATTRP)
        if (!n.n_vattr ||
            kread(ctx, (KA_T)n.n_vattr, (char *)&nv, sizeof(nv))) {
            (void)snpf(Namech, Namechl, "can't read n_vattr at: %x",
                       print_kptr((KA_T)n.n_vattr, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
#endif /* defined(HASNFSVATTRP) */

        nty = NFSNODE;
        break;

#if defined(HASNULLFS)
    case VT_NULL:
        if ((sc == 1) && vfs)
            nvfs = vfs;
        if (!v->v_data ||
            kread(ctx, (KA_T)v->v_data, (char *)&nu, sizeof(nu))) {
            (void)snpf(Namech, Namechl, "can't read null_node at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        if (!nu.null_lowervp) {
            (void)snpf(Namech, Namechl, "null_node overlays nothing");
            enter_nm(ctx, Namech);
            return;
        }
        va = (KA_T)nu.null_lowervp;
        goto process_overlaid_node;
#endif /* defined(HASNULLFS) */

#if defined(HASPROCFS)
    case VT_PROCFS:
        if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&p, sizeof(p))) {
            (void)snpf(Namech, Namechl, "can't read pfsnode at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        nty = PFSNODE;
        break;
#endif /* defined(HASPROCFS) */

#if defined(HASPTYFS)
    case VT_PTYFS:
        if (!v->v_data ||
            kread(ctx, (KA_T)v->v_data, (char *)&pt, sizeof(pt))) {
            (void)snpf(Namech, Namechl, "can't read ptyfsnode at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }
        nty = PTYFSNODE;
        break;
#endif /* defined(HASPTYFS) */

#if defined(HASEXT2FS)
    case VT_EXT2FS:
#endif /* defined(HASEXT2FS) */

#if defined(HASLFS)
    case VT_LFS:
#endif /* defined(HASLFS) */

    case VT_UFS:
        if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&i, sizeof(i))) {
            (void)snpf(Namech, Namechl, "can't read inode at: %s",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            enter_nm(ctx, Namech);
            return;
        }

#if defined(HASEXT2FS)
        if (v->v_tag == VT_EXT2FS) {
            nty = EXT2NODE;

#    if defined(HASI_E2FS_PTR)
            if (i.DINODE_U.e2fs_din &&
                !kread(ctx, (KA_T)i.DINODE_U.e2fs_din, (char *)&ed, sizeof(ed)))
                edp = &ed;
#    else /* !defined(HASI_E2FS_PTR) */
#        if HASEXT2FS < 2
            edp = &i.DINODE_U.e2fs_din;
#        else  /* HASEXT2FS>=2 */
            edp = &i.i_e2din;
#        endif /* HASEXT2FS>=2 */
#    endif     /* defined(HASI_E2FS_PTR) */

        } else
#endif /* defined(HASEXT2FS) */

        {
            nty = INODE;

#if defined(HASI_FFS1)
            /*
             * If there are multiple FFS's, read the relevant structures.
             */
            if (i.i_ump &&
                !kread(ctx, (KA_T)i.i_ump, (char *)&um, sizeof(um))) {
                if (um.um_fstype == UFS1) {
                    ffs = 1;
                    if (i.DINODE_U.ffs1_din &&
                        !kread(ctx, (KA_T)i.DINODE_U.ffs1_din, (char *)&u1,
                               sizeof(u1))) {
                        u1s = 1;
                    }
                } else if (um.um_fstype == UFS2) {
                    ffs = 2;
                    if (i.DINODE_U.ffs2_din &&
                        !kread(ctx, (KA_T)i.DINODE_U.ffs2_din, (char *)&u2,
                               sizeof(u2))) {
                        u2s = 1;
                    }
                }
            }
#endif /* defined(HASI_FFS1) */
        }

#if defined(HAS_LOCKF_H)
        if ((lff = i.i_lockf)) {

            /*
             * Determine the lock state.
             */
            lfp = lff;
            do {
                if (kread(ctx, (KA_T)lfp, (char *)&lf, sizeof(lf)))
                    break;
                lt = 0;
                switch (lf.lf_flags & (F_FLOCK | F_POSIX)) {
                case F_FLOCK:
                    if (Cfp && (struct file *)lf.lf_id == Cfp)
                        lt = 1;
                    break;
                case F_POSIX:
                    if ((KA_T)lf.lf_id == Kpa)
                        lt = 1;

#    if defined(HAS_LWP_H) && !defined(HAS_LF_LWP)
                    else {

                        struct lwp lw;

                        if (!kread(ctx, (KA_T)lf.lf_id, (char *)&lw,
                                   sizeof(lw)) &&
                            (KA_T)lw.l_proc == Kpa)
                            lt = 1;
                    }
#    endif /* defined(HAS_LWP_H) && !defined(HAS_LF_LWP) */

                    break;
                }
                if (!lt)
                    continue;
                if (lf.lf_start == (off_t)0 &&
                    lf.lf_end == 0xffffffffffffffffLL)
                    lt = 1;
                else
                    lt = 0;
                if (lf.lf_type == F_RDLCK)
                    Lf->lock =
                        lt ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
                else if (lf.lf_type == F_WRLCK)
                    Lf->lock =
                        lt ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
                else if (lf.lf_type == (F_RDLCK | F_WRLCK))
                    Lf->lock = LSOF_LOCK_READ_WRITE;
                break;
            } while ((lfp = lf.lf_next) && lfp != lff);
        }
#endif
        break;
    default:
        if (v->v_type == VBAD || v->v_type == VNON)
            break;
        (void)snpf(Namech, Namechl, "unknown file system type: %d", v->v_tag);
        enter_nm(ctx, Namech);
        return;
    }
    /*
     * Get device and type for printing.
     */
    type = v->v_type;
    switch (nty) {

#if defined(HASMSDOSFS)
    case DOSNODE:
        dev = d.de_dev;
        devs = 1;
        break;
#endif /* defined(HASMSDOSFS) */

#if defined(HASFDESCFS)
    case FDESCNODE:

#    if defined(HASFDLINK)
        if (f.fd_link && !kread(ctx, (KA_T)f.fd_link, Namech, Namechl - 1)) {
            Namech[Namechl - 1] = '\0';
            break;
        }
#    endif /* defined(HASFDLINK) */

#    if HASFDESCFS == 1
        if (f.fd_type == Fctty) {
            if (f_tty_s == 0)
                f_tty_s = lkup_dev_tty(ctx, &f_tty_dev, &f_tty_ino);
            if (f_tty_s == 1) {
                dev = DevDev;
                rdev = f_tty_dev;
                Lf->inode = f_tty_ino;
                devs = Lf->inp_ty = rdevs = 1;
            }
        }
        break;
#    endif /* HASFDESCFS==1 */
#endif     /* defined(HASFDESCFS) */

#if defined(HASEXT2FS)
    case EXT2NODE:

        dev = i.i_dev;
        devs = 1;
        if ((type == VCHR) || (type == VBLK)) {

#    if defined(HASI_E2FS_PTR)
            if (edp) {
                rdev = edp->e2di_rdev;
                rdevs = 1;
            }
#    else /* !defined(HASI_E2FS_PTR) */
#        if HASEXT2FS < 2
            rdev = i.DINODE_U.e2fs_din.e2di_rdev;
#        else  /* HASEXT2FS>=2 */
            rdev = i.i_e2din.e2di_rdev;
#        endif /* HASEXT2FS>=2 */
            rdevs = 1;
#    endif     /* defined(HASI_E2FS_PTR) */
        }
        break;
#endif /* defined(HASEXT2FS) */

    case INODE:
        dev = i.i_dev;
        devs = 1;
        if ((type == VCHR) || (type == VBLK)) {

#if defined(HASI_FFS)
            rdev = i.i_ffs_rdev;
            rdevs = 1;
#else /* !defined(HASI_FFS) */
#    if defined(HASI_FFS1)
            if (ffs == 1) {
                if (u1s) {
                    rdev = u1.di_rdev;
                    rdevs = 1;
                }
            } else if (ffs == 2) {
                if (u2s) {
                    rdev = u2.di_rdev;
                    rdevs = 1;
                }
            }
#    else  /* !defined(HASI_FFS1) */
            rdev = i.i_rdev;
            rdevs = 1;
#    endif /* defined(HASI_FFS1) */
#endif     /* defined(HASI_FFS) */
        }
        break;

#if defined(HASKERNFS)
    case KERNFSNODE:
        if (vfs) {

#    if defined(HASSTATVFS)
            dev = (dev_t)vfs->fsid.__fsid_val[0];
#    else  /* !defined(HASSTATVFS) */
            dev = (dev_t)vfs->fsid.val[0];
#    endif /* defined(HASSTATVFS) */

            devs = 1;
        }
        break;
#endif /* defined(HASKERNFS) */

#if defined(HAS9660FS)
    case CDFSNODE:
        if (iso_stat) {
            dev = iso_dev;
            devs = 1;
        }
        break;
#endif /* defined(HAS9660FS) */

    case NFSNODE:
        dev = NVATTR.va_fsid;
        devs = 1;
        break;

#if defined(HASPTYFS)
    case PTYFSNODE:
        if (v->v_un.vu_specinfo &&
            !kread(ctx, (KA_T)v->v_un.vu_specinfo, (char *)&si, sizeof(si))) {
            rdev = si.si_rdev;
            rdevs = 1;
        }
        if (vfs) {

#    if defined(HASSTATVFS)
            dev = (dev_t)vfs->fsid.__fsid_val[0];
#    else  /* !defined(HASSTATVFS) */
            dev = (dev_t)vfs->fsid.val[0];
#    endif /* defined(HASSTATVFS) */

            devs = 1;
        }
        break;
#endif /* defined(HASPTYFS) */

#if defined(HASTMPFS)
    case TMPFSNODE:
        if (vfs) {

#    if defined(HASSTATVFS)
            dev = (dev_t)vfs->fsid.__fsid_val[0];
#    else  /* !defined(HASSTATVFS) */
            dev = (dev_t)vfs->fsid.val[0];
#    endif /* defined(HASSTATVFS) */

            devs = 1;
        }
        break;
#endif /* defined(HASTMPFS) */
    }
    /*
     * Obtain the inode number.
     */
    switch (nty) {

#if defined(HASMSDOSFS)
    case DOSNODE:
        if (d.de_pmp && !kread(ctx, (KA_T)d.de_pmp, (char *)&pm, sizeof(pm))) {
            dpb = (u_long)(pm.pm_BytesPerSec / sizeof(struct direntry));
            if (d.de_Attributes & ATTR_DIRECTORY) {
                if (d.de_StartCluster == MSDOSFSROOT)
                    nn = (INODETYPE)1;
                else
                    nn = (INODETYPE)(cntobn(&pm, d.de_StartCluster) * dpb);
            } else {
                if (d.de_dirclust == MSDOSFSROOT)
                    nn = (INODETYPE)(roottobn(&pm, 0) * dpb);
                else
                    nn = (INODETYPE)(cntobn(&pm, d.de_dirclust) * dpb);
                nn += (INODETYPE)(d.de_diroffset / sizeof(struct direntry));
            }
            Lf->inode = nn;
            Lf->inp_ty = 1;
        }
        break;
#endif /* defined(HASMSDOSFS) */

#if defined(HASEXT2FS)
    case EXT2NODE:
#endif /* defined(HASEXT2FS) */

    case INODE:
        Lf->inode = (INODETYPE)i.i_number;
        Lf->inp_ty = 1;
        break;

#if defined(HASKERNFS)
    case KERNFSNODE:
        if (ksbs) {
            Lf->inode = (INODETYPE)ksb.st_ino;
            Lf->inp_ty = 1;
        }
        break;
#endif /* defined(HASKERNFS) */

#if defined(HAS9660FS)
    case CDFSNODE:
        if (iso_stat) {
            Lf->inode = iso_ino;
            Lf->inp_ty = 1;
        }
        break;
#endif /* defined(HAS9660FS) */

    case NFSNODE:
        Lf->inode = (INODETYPE)NVATTR.va_fileid;
        Lf->inp_ty = 1;
        break;

#if defined(HASPROCFS)
    case PFSNODE:
        Lf->inode = (INODETYPE)p.pfs_fileno;
        Lf->inp_ty = 1;
        break;
#endif /* defined(HASPROCFS) */

#if defined(HASPTYFS)
    case PTYFSNODE:
        if (pt.ptyfs_type == PTYFSptc) {
            if (pt.ptyfs_fileno > 0x3fffffff)
                Lf->inode = (INODETYPE)(pt.ptyfs_fileno & 0x3fffffff);
            else
                Lf->inode = (INODETYPE)(pt.ptyfs_fileno - 1);
        } else
            Lf->inode = (INODETYPE)pt.ptyfs_fileno;
        Lf->inp_ty = 1;
        break;
#endif /* defined(HASPTYFS) */

#if defined(HASTMPFS)
    case TMPFSNODE:
        Lf->inode = (INODETYPE)tmp.tn_id;
        Lf->inp_ty = 1;
        break;
#endif /* defined(HASTMPFS) */
    }

    /*
     * Obtain the file size.
     */
    switch (Ntype) {
#if defined(HAS9660FS)
    case N_CDFS:
        if (iso_stat) {
            Lf->sz = (SZOFFTYPE)iso_sz;
            Lf->sz_def = 1;
        }
        break;
#endif /* defined(HAS9660FS) */

    case N_FIFO:
        break;

#if defined(HASKERNFS)
    case N_KERN:
        if (ksbs) {
            Lf->sz = (SZOFFTYPE)ksb.st_size;
            Lf->sz_def = 1;
        }
        break;
#endif /* defined(HASKERNFS) */

    case N_NFS:
        if (nty == NFSNODE) {
            Lf->sz = (SZOFFTYPE)NVATTR.va_size;
            Lf->sz_def = 1;
        }
        break;

#if defined(HASPROCFS)
    case N_PROC:
        if (nty == PFSNODE) {
            switch (p.pfs_type) {
            case Proot:
            case Pproc:
                Lf->sz = (SZOFFTYPE)DEV_BSIZE;
                Lf->sz_def = 1;
                break;
            case Pcurproc:
                Lf->sz = (SZOFFTYPE)DEV_BSIZE;
                Lf->sz_def = 1;
                break;
            case Pmem:
                (void)getmemsz(ctx, p.pfs_pid);
                break;
            case Pregs:
                Lf->sz = (SZOFFTYPE)sizeof(struct reg);
                Lf->sz_def = 1;
                break;

#    if defined(FP_QSIZE)
            case Pfpregs:
                Lf->sz = (SZOFFTYPE)sizeof(struct fpreg);
                Lf->sz_def = 1;
                break;
#    endif /* defined(FP_QSIZE) */
            }
        }
        break;
#endif /* defined(HASPROCFS) */

    case N_REGLR:
        if (type == VREG || type == VDIR) {
            switch (nty) {
            case INODE:

#if defined(HASI_FFS)

                Lf->sz = (SZOFFTYPE)i.i_ffs_size;
                Lf->sz_def = 1;
                break;
#else /* !defined(HASI_FFS) */
#    if defined(HASI_FFS1)

                if (ffs == 1) {
                    if (u1s) {
                        Lf->sz = (SZOFFTYPE)u1.di_size;
                        Lf->sz_def = 1;
                    }
                } else if (ffs == 2) {
                    if (u2s) {
                        Lf->sz = (SZOFFTYPE)u2.di_size;
                        Lf->sz_def = 1;
                    }
                }
                break;
#    else  /* !defined(HASI_FFS1) */
                Lf->sz = (SZOFFTYPE)i.i_size;
                Lf->sz_def = 1;
#    endif /* defined(HASI_FFS1) */
#endif     /* defined(HASI_FFS) */

                break;

#if defined(HASMSDOSFS)
            case DOSNODE:
                Lf->sz = (SZOFFTYPE)d.de_FileSize;
                Lf->sz_def = 1;
                break;
#endif /* defined(HASMSDOSFS) */

            case MFSNODE:
                Lf->sz = (SZOFFTYPE)m.mfs_size;
                Lf->sz_def = 1;
                break;

#if defined(HASTMPFS)
            case TMPFSNODE:
                Lf->sz = (SZOFFTYPE)tmp.tn_size;
                Lf->sz_def = 1;
                break;
#endif /* defined(HASTMPFS) */

#if defined(HASEXT2FS)
            case EXT2NODE:
#    if defined(HASI_E2FS_PTR)
                if (edp) {
                    Lf->sz = (SZOFFTYPE)edp->e2di_size;
                    Lf->sz_def = 1;
                }
#    else  /* !defined(HASI_E2FS_PTR) */
                Lf->sz = (SZOFFTYPE)i.i_e2fs_size;
                Lf->sz_def = 1;
#    endif /* defined(HASI_E2FS_PTR) */
                break;
#endif /* defined(HASEXT2FS) */
            }
        }
        break;
    }
    /*
     * Record the link count.
     */
    switch (Ntype) {

#if defined(HAS9660FS)
    case N_CDFS:
        if (iso_stat) {
            Lf->nlink = iso_nlink;
            Lf->nlink_def = 1;
        }
        break;
#endif /* defined(HAS9660FS) */

#if defined(HASKERNFS)
    case N_KERN:
        if (ksbs) {
            Lf->nlink = (long)ksb.st_nlink;
            Lf->nlink_def = 1;
        }
        break;
#endif /* defined(HASKERNFS) */

    case N_NFS:
        if (nty == NFSNODE) {
            Lf->nlink = (long)NVATTR.va_nlink;
            Lf->nlink_def = 1;
        }
        break;
    case N_REGLR:
        switch (nty) {
        case INODE:

#if defined(HASEFFNLINK)
            Lf->nlink = (long)i.HASEFFNLINK;
#else /* !defined(HASEFFNLINK) */
#    if defined(HASI_FFS)
            Lf->nlink = (long)i.i_ffs_nlink;
#    else /* !defined(HASI_FFS) */
#        if defined(HASI_FFS1)
            if (ffs == 1) {
                if (u1s)
                    Lf->nlink = (long)u1.di_nlink;
            } else if (ffs == 2) {
                if (u2s)
                    Lf->nlink = (long)u2.di_nlink;
            }
#        else  /* !defined(HASI_FFS1) */

            Lf->nlink = (long)i.i_nlink;
#        endif /* defined(HASI_FFS1) */
#    endif     /* defined(HASI_FFS) */
#endif         /* defined(HASEFFNLINK) */

            Lf->nlink_def = 1;
            break;

#if defined(HASMSDOSFS)
        case DOSNODE:
            Lf->nlink = (long)d.de_refcnt;
            Lf->nlink_def = 1;
            break;
#endif /* defined(HASMSDOSFS) */

#if defined(HASEXT2FS)
        case EXT2NODE:
#    if defined(HASI_E2FS_PTR)
            if (edp) {
                Lf->nlink = (long)edp->e2di_nlink;
                Lf->nlink_def = 1;
            }
#    else  /* !defined(HASI_E2FS_PTR) */
            Lf->nlink = (long)i.i_e2fs_nlink;
            Lf->nlink_def = 1;
#    endif /* defined(HASI_E2FS_PTR) */

            break;

#endif /* defined(HASEXT2FS) */
        }
        break;
    }
    if (Lf->nlink_def && Nlink && (Lf->nlink < Nlink))
        Lf->sf |= SELNLINK;
    /*
     * Record an NFS file selection.
     */
    if (Ntype == N_NFS && Fnfs)
        Lf->sf |= SELNFS;

#if defined(HASNULLFS)
    /*
     * If there is a saved nullfs vfs pointer, propagate its device number.
     */
    if (nvfs) {

#    if defined(HASSTATVFS)
        dev = nvfs->fsid.__fsid_val[0];
#    else  /* !defined(HASSTATVFS) */
        dev = nvfs->fsid.val[0];
#    endif /* defined(HASSTATVFS) */

        devs = 1;
    }
#endif /* defined(HASNULLFS) */

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
    switch (type) {
    case VNON:
        Lf->type = LSOF_FILE_VNODE_VNON;
        break;
    case VREG:
        Lf->type = LSOF_FILE_VNODE_VREG;
        break;
    case VDIR:
        Lf->type = LSOF_FILE_VNODE_VDIR;
        break;
    case VBLK:
        Lf->type = LSOF_FILE_VNODE_VBLK;
        Ntype = N_BLK;
        break;
    case VCHR:
        Lf->type = LSOF_FILE_VNODE_VCHR;
        Ntype = N_CHR;
        break;
    case VLNK:
        Lf->type = LSOF_FILE_VNODE_VLNK;
        break;

#if defined(VSOCK)
    case VSOCK:
        Lf->type = LSOF_FILE_VNODE_VSOCK;
        break;
#endif /* defined(VSOCK) */

    case VBAD:
        Lf->type = LSOF_FILE_VNODE_VBAD;
        break;
    case VFIFO:
        Lf->type = LSOF_FILE_VNODE_VFIFO;
        break;
    default:
        Lf->type = LSOF_FILE_UNKNOWN_RAW;
        Lf->unknown_file_type_number = type;
    }
    Lf->ntype = Ntype;
    /*
     * Handle some special cases:
     *
     * 	ioctl(fd, TIOCNOTTY) files;
     *	/kern files
     *	memory node files;
     *	/proc files;
     *	ptyfs files.
     */

    if (type == VBAD)
        (void)snpf(Namech, Namechl, "(revoked)");
    else if (nty == MFSNODE) {
        Lf->dev_def = Lf->rdev_def = 0;
        (void)snpf(Namech, Namechl, "%#x", m.mfs_baseoff);
        enter_dev_ch(ctx, "memory");
    }

#if defined(HASPROCFS)
    else if (nty == PFSNODE) {
        Lf->dev_def = Lf->rdev_def = 0;
        (void)snpf(Namech, Namechl, "/%s", HASPROCFS);
        switch (p.pfs_type) {
        case Proot:
            Lf->type = LSOF_FILE_PROC_DIR;
            break;
        case Pcurproc:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/curproc");
            Lf->type = LSOF_FILE_PROC_CUR_PROC;
            break;
        case Pproc:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_DIR;
            break;
        case Pfile:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/file", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_FILE;
            break;
        case Pmem:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/mem", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_MEMORY;
            break;
        case Pregs:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/regs", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_REGS;
            break;
        case Pfpregs:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/fpregs", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_FP_REGS;
            break;

#    if defined(Pctl)
        case Pctl:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/ctl", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_CTRL;
            break;
#    endif /* defined(Pctl) */

        case Pstatus:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/status", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_STATUS;
            break;
        case Pnote:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/note", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_PROC_NOTIFIER;
            break;
        case Pnotepg:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/notepg", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_GROUP_NOTIFIER;
            break;

#    if defined(Pfd)
        case Pfd:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/fd", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_FD;
            break;
#    endif /* defined(Pfd) */

#    if defined(Pmap)
        case Pmap:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/map", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_MAP;
            break;
#    endif /* defined(Pmap) */

#    if defined(Pmaps)
        case Pmaps:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%d/maps", p.pfs_pid);
            Lf->type = LSOF_FILE_PROC_MAPS;
            break;
#    endif /* defined(Pmaps) */
        }
    }
#endif /* defined(HASPROCFS) */

#if defined(HASPTYFS)
    else if (nty == PTYFSNODE) {
        (void)snpf(Namech, Namechl, "%s", Lf->fsdir);
        Lf->nlink = 1;
        Lf->nlink_def = 1;
        switch (pt.ptyfs_type) {
        case PTYFSpts:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%lu", (unsigned long)pt.ptyfs_pty);
            break;
        case PTYFSptc:
            ep = endnm(ctx, &sz);
            (void)snpf(ep, sz, "/%lu (master)", (unsigned long)pt.ptyfs_pty);
            break;
        case PTYFSroot:
            Lf->sz = 512;
            Lf->sz_def = 1;
            break;
        }
    }
#endif /* defined(HASPTYFS) */

#if defined(HASBLKDEV)
    /*
     * If this is a VBLK file and it's missing an inode number, try to
     * supply one.
     */
    if ((Lf->inp_ty == 0) && (type == VBLK))
        find_bl_ino(ctx);
#endif /* defined(HASBLKDEV) */

    /*
     * If this is a VCHR file and it's missing an inode number, try to
     * supply one.
     */
    if ((Lf->inp_ty == 0) && (type == VCHR))
        find_ch_ino(ctx);
        /*
         * Test for specified file.
         */

#if defined(HASPROCFS)
    if (Ntype == N_PROC) {
        if (Procsrch) {
            Procfind = 1;
            Lf->sf |= SELNM;
        } else if (nty == PFSNODE) {
            for (pfi = Procfsid; pfi; pfi = pfi->next) {
                if ((pfi->pid && pfi->pid == p.pfs_pid)

#    if defined(HASPINODEN)
                    || ((Lf->inp_ty == 1) && (pfi->inode == Lf->inode))
#    endif /* defined(HASPINODEN) */

                ) {
                    pfi->f = 1;
                    if (Namech[0] && pfi->nm)
                        (void)snpf(Namech, Namechl, "%s", pfi->nm);
                    Lf->sf |= SELNM;
                    break;
                }
            }
        }
    } else
#endif /* defined(HASPROCFS) */

    {
        if (Namech[0]) {
            enter_nm(ctx, Namech);
            ns = 1;
        } else
            ns = 0;
        if (Sfile &&
            is_file_named(ctx, (char *)NULL,
                          ((type == VCHR) || (type == VBLK)) ? 1 : 0)) {
            Lf->sf |= SELNM;
        }
        if (ns)
            Namech[0] = '\0';
    }
    /*
     * Enter name characters.
     */
    if (Namech[0])
        enter_nm(ctx, Namech);
}

#if defined(HAS_SYS_PIPEH)
/*
 * process_pipe() - process a file structure whose type is DTYPE_PIPE
 */

void process_pipe(struct lsof_context *ctx, /* context */
                  KA_T pa)                  /* pipe structure kernel address */
{
    char *ep;
    struct pipe p;
    size_t sz;

    if (!pa || kread(ctx, (KA_T)pa, (char *)&p, sizeof(p))) {
        (void)snpf(Namech, Namechl, "can't read DTYPE_PIPE pipe struct: %#s",
                   print_kptr(pa, (char *)NULL, 0));
        enter_nm(ctx, Namech);
        return;
    }
    Lf->type = LSOF_FILE_PIPE;
    enter_dev_ch(ctx, print_kptr(pa, (char *)NULL, 0));
    Lf->sz = (SZOFFTYPE)p.pipe_buffer.size;
    Lf->sz_def = 1;
    if (p.pipe_peer)
        (void)snpf(Namech, Namechl, "->%s",
                   print_kptr((KA_T)p.pipe_peer, (char *)NULL, 0));
    else
        Namech[0] = '\0';
    if (p.pipe_buffer.cnt) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", cnt=%d", p.pipe_buffer.cnt);
    }
    if (p.pipe_buffer.in) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", in=%d", p.pipe_buffer.in);
    }
    if (p.pipe_buffer.out) {
        ep = endnm(ctx, &sz);
        (void)snpf(ep, sz, ", out=%d", p.pipe_buffer.out);
    }
    /*
     * Enter name characters.
     */
    if (Namech[0])
        enter_nm(ctx, Namech);
}
#endif /* defined(HAS_SYS_PIPEH) */
