/*
 * dproc.c - FreeBSD process access functions for lsof
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

static void get_kernel_access(struct lsof_context *ctx);

/*
 * Local static values
 */

static int cmp_xfiles_pid_fd(const void *a, const void *b) {
    const struct xfile *aa, *bb;

    aa = (struct xfile *)a;
    bb = (struct xfile *)b;
    if (aa->xf_pid < bb->xf_pid) {
        return -1;
    } else if (aa->xf_pid > bb->xf_pid) {
        return 1;
    } else {
        if (aa->xf_fd < bb->xf_fd) {
            return -1;
        } else if (aa->xf_fd > bb->xf_fd) {
            return 1;
        } else {
            return 0;
        }
    }
}

static int read_xfiles(struct xfile **xfiles, size_t *count) {
    int mib[2];
    size_t len;

    mib[0] = CTL_KERN;
    mib[1] = KERN_FILE;
    *xfiles = NULL;
    if (!sysctl(mib, 2, NULL, &len, NULL, 0)) {
        /* FreeBSD 9 under-reports the required memory, so increase it
         * ourselves: */
        len *= 2;
        *xfiles = malloc(len);
        if (*xfiles) {
            if (!sysctl(mib, 2, *xfiles, &len, NULL, 0)) {
                *count = len / sizeof(struct xfile);
                return 0;
            }
        }
    }
    free(*xfiles);
    *xfiles = NULL;
    *count = 0;
    return 1;
}

#ifdef KERN_LOCKF
int cmp_kinfo_lockf(const void *a, const void *b) {
    const struct kinfo_lockf *aa, *bb;

    aa = (const struct kinfo_lockf *)a;
    bb = (const struct kinfo_lockf *)b;
    if (aa->kl_sysid < bb->kl_sysid)
        return -1;
    if (aa->kl_sysid > bb->kl_sysid)
        return 1;
    if (aa->kl_pid < bb->kl_pid)
        return -1;
    if (aa->kl_pid > bb->kl_pid)
        return 1;
    if (aa->kl_file_fsid < bb->kl_file_fsid)
        return -1;
    if (aa->kl_file_fsid > bb->kl_file_fsid)
        return 1;
    if (aa->kl_file_fileid < bb->kl_file_fileid)
        return -1;
    if (aa->kl_file_fileid > bb->kl_file_fileid)
        return 1;
    return 0;
}

static int read_lockf(struct kinfo_lockf **locks, size_t *count) {
    int mib[2];
    size_t len;

    mib[0] = CTL_KERN;
    mib[1] = KERN_LOCKF;
    *locks = NULL;
    if (!sysctl(mib, 2, NULL, &len, NULL, 0)) {
        *locks = malloc(len);
        if (*locks) {
            if (!sysctl(mib, 2, *locks, &len, NULL, 0)) {
                *count = len / sizeof(struct kinfo_lockf);
                return 0;
            }
        }
    }
    free(*locks);
    *locks = NULL;
    *count = 0;
    return 1;
}
#endif

static int kf_flags_to_fflags(int kf_flags) {
    static const struct {
        int fflag;
        int kf_flag;
    } fflags_table[] = {
        {FAPPEND, KF_FLAG_APPEND},      {FASYNC, KF_FLAG_ASYNC},
        {FFSYNC, KF_FLAG_FSYNC},        {FHASLOCK, KF_FLAG_HASLOCK},
        {FNONBLOCK, KF_FLAG_NONBLOCK},  {FREAD, KF_FLAG_READ},
        {FWRITE, KF_FLAG_WRITE},        {O_CREAT, KF_FLAG_CREAT},
        {O_DIRECT, KF_FLAG_DIRECT},     {O_EXCL, KF_FLAG_EXCL},
        {O_EXEC, KF_FLAG_EXEC},         {O_EXLOCK, KF_FLAG_EXLOCK},
        {O_NOFOLLOW, KF_FLAG_NOFOLLOW}, {O_SHLOCK, KF_FLAG_SHLOCK},
        {O_TRUNC, KF_FLAG_TRUNC}};
    int i;
    int fflags;

    fflags = 0;
    for (i = 0; i < sizeof(fflags_table) / sizeof(fflags_table[0]); i++)
        if (kf_flags & fflags_table[i].kf_flag)
            fflags |= fflags_table[i].fflag;
    return fflags;
}

/* Based on process_file() in lib/prfd.c */
static void process_kinfo_file(struct lsof_context *ctx, struct kinfo_file *kf,
                               struct xfile *xfile, struct pcb_lists *pcbs,
                               struct lock_list *locks) {
    Lf->off = kf->kf_offset;
    Lf->off_def = 1;
    if (kf->kf_ref_count) {
        if ((kf->kf_flags & (KF_FLAG_READ | KF_FLAG_WRITE)) == KF_FLAG_READ)
            Lf->access = LSOF_FILE_ACCESS_READ;
        else if ((kf->kf_flags & (KF_FLAG_READ | KF_FLAG_WRITE)) ==
                 KF_FLAG_WRITE)
            Lf->access = LSOF_FILE_ACCESS_WRITE;
        else if ((kf->kf_flags & (KF_FLAG_READ | KF_FLAG_WRITE)) ==
                 (KF_FLAG_READ | KF_FLAG_WRITE))
            Lf->access = LSOF_FILE_ACCESS_READ_WRITE;
    }

    Lf->fct = (long)kf->kf_ref_count;
    Lf->fsv |= FSV_CT;
    if (xfile) {
        Lf->fsa = xfile->xf_file;
        Lf->fsv |= FSV_FA;
        Lf->fna = (KA_T)xfile->xf_data;
        Lf->fsv |= FSV_NI;
    }
    if (xfile)
        Lf->ffg = (long)xfile->xf_flag;
    else
        Lf->ffg = kf_flags_to_fflags(kf->kf_flags);
    Lf->fsv |= FSV_FG;

    switch (kf->kf_type) {
    case KF_TYPE_FIFO:
    case KF_TYPE_VNODE:
        process_vnode(ctx, kf, xfile, locks);
        break;
    case KF_TYPE_SOCKET:
        process_socket(ctx, kf, pcbs);
        break;
    case KF_TYPE_KQUEUE:
        process_kf_kqueue(ctx, kf, xfile ? xfile->xf_data : 0UL);
        break;
    case KF_TYPE_PIPE:
        if (!Selinet)
            process_pipe(ctx, kf, xfile ? xfile->xf_data : 0UL);
        break;
    case KF_TYPE_PTS:
        process_pts(ctx, kf);
        break;
#if defined(KF_TYPE_EVENTFD)
    case KF_TYPE_EVENTFD:
        process_eventfd(ctx, kf);
        break;
#endif /* defined(KF_TYPE_EVENTFD) */
    case KF_TYPE_SHM:
        process_shm(ctx, kf);
        break;
    case KF_TYPE_PROCDESC:
        process_procdesc(ctx, kf);
        break;
    default:
        /* FIXME: unlike struct file, xfile doesn't have f_ops which should be
         * printed here */
        snpf(Namech, Namechl, "%p file struct, ty=%d",
             xfile ? (void *)xfile->xf_file : NULL, kf->kf_type);
        enter_nm(ctx, Namech);
    }
}

static void process_file_descriptors(struct lsof_context *ctx,
                                     struct kinfo_proc *p, short ckscko,
                                     struct xfile *xfiles, size_t n_xfiles,
                                     struct pcb_lists *pcbs,
                                     struct lock_list *locks) {
    struct kinfo_file *kfiles;
    int n_kfiles = 0;
    int i;

    kfiles = kinfo_getfile(p->P_PID, &n_kfiles);
    /* Pre-cache the mount info, as files w/o paths may need it from other files
     * with paths on the same fs */
    for (i = 0; i < n_kfiles; i++) {
        if (kfiles[i].kf_fd < 0 || kfiles[i].kf_type == KF_TYPE_FIFO ||
            kfiles[i].kf_type == KF_TYPE_VNODE)
            readvfs(ctx, kfiles[i].kf_un.kf_file.kf_file_fsid,
                    kfiles[i].kf_path);
    }
    for (i = 0; i < n_kfiles; i++) {
        struct xfile key, *xfile;

        key.xf_pid = p->P_PID;
        key.xf_fd = kfiles[i].kf_fd;
        xfile =
            bsearch(&key, xfiles, n_xfiles, sizeof(*xfiles), cmp_xfiles_pid_fd);

        if (!ckscko && kfiles[i].kf_fd == KF_FD_TYPE_CWD) {
            alloc_lfile(ctx, LSOF_FD_CWD, -1);
            process_vnode(ctx, &kfiles[i], xfile, locks);
            if (Lf->sf)
                link_lfile(ctx);
        } else if (!ckscko && kfiles[i].kf_fd == KF_FD_TYPE_ROOT) {
            alloc_lfile(ctx, LSOF_FD_ROOT_DIR, -1);
            process_vnode(ctx, &kfiles[i], xfile, locks);
            if (Lf->sf)
                link_lfile(ctx);
        } else if (!ckscko && kfiles[i].kf_fd == KF_FD_TYPE_JAIL) {
            alloc_lfile(ctx, LSOF_FD_JAIL_DIR, -1);
            process_vnode(ctx, &kfiles[i], xfile, locks);
            if (Lf->sf)
                link_lfile(ctx);
        } else if (!ckscko && kfiles[i].kf_fd == KF_FD_TYPE_TEXT) {
            alloc_lfile(ctx, LSOF_FD_PROGRAM_TEXT, -1);
            process_vnode(ctx, &kfiles[i], xfile, locks);
            if (Lf->sf)
                link_lfile(ctx);
        } else if (!ckscko && kfiles[i].kf_fd == KF_FD_TYPE_CTTY) {
            alloc_lfile(ctx, LSOF_FD_CTTY, -1);
            process_vnode(ctx, &kfiles[i], xfile, locks);
            if (Lf->sf)
                link_lfile(ctx);
        } else if (!ckscko && kfiles[i].kf_fd < 0) {
            if (!Fwarn)
                fprintf(stderr, "%s: WARNING -- unsupported fd type %d\n", Pn,
                        kfiles[i].kf_fd);
        } else {
            alloc_lfile(ctx, LSOF_FD_NUMERIC, kfiles[i].kf_fd);
            process_kinfo_file(ctx, &kfiles[i], xfile, pcbs, locks);
            if (Lf->sf)
                link_lfile(ctx);
        }
    }
    free(kfiles);
}

/*
 * gather_proc_info() -- gather process information
 */

void gather_proc_info(struct lsof_context *ctx) {

    int mib[3];
    size_t len;
    short cckreg; /* conditional status of regular file
                   * checking:
                   *     0 = unconditionally check
                   *     1 = conditionally check */
    short ckscko; /* socket file only checking status:
                   *     0 = none
                   *     1 = check only socket files,
                   *	   including TCP and UDP
                   *	   streams with eXPORT data,
                   *	   where supported */

    int pgid;
    int ppid = 0;
    short pss, sf;
    int px;
    int tid; /* thread (task) ID */
    uid_t uid;

    struct kinfo_proc *p;
    struct xfile *xfiles;
    size_t n_xfiles;
    struct pcb_lists *pcbs;
    struct lock_list locks;

#if defined(HASFSTRUCT) && !defined(HAS_FILEDESCENT)
    static char *pof = (char *)NULL;
    static int pofb = 0;
#endif /* defined(HASFSTRUCT) && !defiled(HAS_FILEDESCENT) */

    /*
     * Define socket and regular file conditional processing flags.
     *
     * If only socket files have been selected, or socket files have been
     * selected, ANDed with other selection options, enable the skipping of
     * regular files.
     *
     * If socket files and some process options have been selected, enable
     * conditional skipping of regular file; i.e., regular files will be skipped
     * unless they belong to a process selected by one of the specified options.
     */
    if (Selflags & SELNW) {

        /*
         * Some network files selection options have been specified.
         */
        if (Fand || !(Selflags & ~SELNW)) {

            /*
             * Selection ANDing or only network file options have been
             * specified, so set unconditional skipping of regular files
             * and socket file only checking.
             */
            cckreg = 0;
            ckscko = 1;
        } else {

            /*
             * If ORed file selection options have been specified, or no
             * ORed process selection options have been specified, enable
             * unconditional file checking and clear socket file only
             * checking.
             *
             * If only ORed process selection options have been specified,
             * enable conditional file skipping and socket file only checking.
             */
            if ((Selflags & SELFILE) || !(Selflags & SelProc))
                cckreg = ckscko = 0;
            else
                cckreg = ckscko = 1;
        }
    } else {

        /*
         * No network file selection options were specified.  Enable
         * unconditional file checking and clear socket file only checking.
         */
        cckreg = ckscko = 0;
    }
    /*
     * Read the process table.
     */

#if !defined(KERN_PROC_PROC)
#    define KERN_PROC_PROC KERN_PROC_ALL
#endif /* !defined(KERN_PROC_PROC) */

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = Ftask ? KERN_PROC_ALL : KERN_PROC_PROC;
    len = 0;
    if (sysctl(mib, 3, NULL, &len, NULL, 0) == 0) {
        P = malloc(len);
        if (P) {
            if (sysctl(mib, 3, P, &len, NULL, 0) < 0) {
                free(P);
                P = NULL;
            }
        }
    }
    if (P == NULL) {
        (void)fprintf(stderr, "%s: can't read process table: %s\n", Pn,
                      strerror(errno));
        Error(ctx);
    }
    Np = len / sizeof(struct kinfo_proc);
    if (read_xfiles(&xfiles, &n_xfiles) && !Fwarn)
        fprintf(stderr, "%s: WARNING -- reading xfile list failed: %s\n", Pn,
                strerror(errno));
    qsort(xfiles, n_xfiles, sizeof(*xfiles), cmp_xfiles_pid_fd);
    pcbs = read_pcb_lists();
    if (!pcbs && !Fwarn)
        fprintf(stderr, "%s: WARNING -- reading PCBs failed: %s\n", Pn,
                strerror(errno));
#ifdef KERN_LOCKF
    if (read_lockf(&locks.locks, &locks.n_locks) && !Fwarn)
        fprintf(stderr, "%s: WARNING -- reading lockf list failed: %s\n", Pn,
                strerror(errno));
    qsort(locks.locks, locks.n_locks, sizeof(*locks.locks), cmp_kinfo_lockf);
#endif

    /*
     * Examine proc structures and their associated information.
     */

    for (p = P, px = 0; px < Np; p++, px++)

    {

        if (p->P_STAT == 0 || p->P_STAT == SZOMB)
            continue;
        pgid = p->P_PGID;
        uid = p->ki_uid;

#if defined(HASPPID)
        ppid = p->P_PPID;
#endif /* defined(HASPPID) */

#if defined(HASTASKS)
        /*
         * See if process,including its tasks, is excluded.
         */
        tid = Ftask ? (int)p->ki_tid : 0;
        if (is_proc_excl(ctx, p->P_PID, pgid, (UID_ARG)uid, &pss, &sf, tid))
            continue;
#else  /* !defined(HASTASKS) */
        /*
         * See if process is excluded.
         */
        if (is_proc_excl(ctx, p->P_PID, pgid, (UID_ARG)uid, &pss, &sf))
            continue;
#endif /* defined(HASTASKS) */

        /*
         * Allocate a local process structure.
         */
        if (is_cmd_excl(ctx, p->P_COMM, &pss, &sf))
            continue;
        if (cckreg) {

            /*
             * If conditional checking of regular files is enabled, enable
             * socket file only checking, based on the process' selection
             * status.
             */
            ckscko = (sf & SelProc) ? 0 : 1;
        }
        alloc_lproc(ctx, p->P_PID, pgid, ppid, (UID_ARG)uid, p->P_COMM,
                    (int)pss, (int)sf);
        Plf = (struct lfile *)NULL;

#if defined(HASTASKS)
        /*
         * Save the task (thread) ID.
         */
        Lp->tid = tid;
#endif /* defined(HASTASKS) */

#if defined(P_ADDR)
        /*
         * Save the kernel proc struct address, if P_ADDR is defined.
         */
        Kpa = (KA_T)p->P_ADDR;
#endif /* defined(P_ADDR) */

        process_file_descriptors(ctx, p, ckscko, xfiles, n_xfiles, pcbs,
                                 &locks);

        /*
         * Unless threads (tasks) are being processed, examine results.
         */
        if (!Ftask) {
            if (examine_lproc(ctx))
                break;
        }
    }

    free(xfiles);
    free_pcb_lists(pcbs);
#ifdef KERN_LOCKF
    free(locks.locks);
#endif
}

/*
 * get_kernel_access() - get access to kernel memory
 */

static void get_kernel_access(struct lsof_context *ctx) {

    /*
     * Check kernel version.
     */
    (void)ckkv(ctx, "FreeBSD", LSOF_VSTR, (char *)NULL, (char *)NULL);
    /*
     * Set name list file path.
     */
    if (!Nmlst)

#if defined(N_UNIX)
        Nmlst = N_UNIX;
#else  /* !defined(N_UNIX) */
    {
        if (!(Nmlst = get_nlist_path(ctx, 1))) {
            (void)fprintf(stderr, "%s: can't get kernel name list path\n", Pn);
            Error(ctx);
        }
    }
#endif /* defined(N_UNIX) */

#if defined(WILLDROPGID)
    /*
     * If kernel memory isn't coming from KMEM, drop setgid permission
     * before attempting to open the (Memory) file.
     */
    if (Memory)
        (void)dropgid(ctx);
#else  /* !defined(WILLDROPGID) */
    /*
     * See if the non-KMEM memory and the name list files are readable.
     */
    if ((Memory && !is_readable(Memory, 1)) ||
        (Nmlst && !is_readable(Nmlst, 1)))
        Error(ctx);
#endif /* defined(WILLDROPGID) */

    /*
     * Open kernel memory access.
     */

    if ((Kd = kvm_open(Nmlst, Memory, NULL, O_RDONLY, NULL)) == NULL)

    {
        (void)fprintf(stderr, "%s: kvm_open%s(execfile=%s, corefile=%s): %s\n",
                      Pn,

                      "",

                      Nmlst ? Nmlst : "default",
                      Memory ? Memory :

#if defined(_PATH_MEM)
                             _PATH_MEM,
#else  /* !defined(_PATH_MEM) */
                             "default",
#endif /* defined(_PATH_MEM) */

                      strerror(errno));
        return;
    }
    (void)build_Nl(ctx, Drive_Nl);
    if (kvm_nlist(Kd, Nl) < 0) {
        (void)fprintf(stderr, "%s: can't read namelist from %s\n", Pn, Nmlst);
        Error(ctx);
    }

#if defined(X_BADFILEOPS)
    /*
     * Get kernel's badfileops address (for process_file()).
     */
    if (get_Nl_value(ctx, X_BADFILEOPS, (struct drive_Nl *)NULL, &X_bfopsa) <
            0 ||
        !X_bfopsa) {
        X_bfopsa = (KA_T)0;
    }
#endif /* defined(X_BADFILEOPS) */

#if defined(WILLDROPGID)
    /*
     * Drop setgid permission, if necessary.
     */
    if (!Memory)
        (void)dropgid(ctx);
#endif /* defined(WILLDROPGID) */
}

#if !defined(N_UNIX)
/*
 * get_nlist_path() - get kernel name list path
 */

char *get_nlist_path(struct lsof_context *ctx,
                     int ap) /* on success, return an allocated path
                              * string pointer if 1; return a
                              * constant character pointer if 0;
                              * return NULL if failure */
{
    const char *bf;
    static char *bfc;
    MALLOC_S bfl;
    /*
     * Get bootfile name.
     */
    if ((bf = getbootfile())) {
        if (!ap)
            return ("");
        bfl = (MALLOC_S)(strlen(bf) + 1);
        if (!(bfc = (char *)malloc(bfl))) {
            (void)fprintf(
                stderr, "%s: can't allocate %d bytes for boot file path: %s\n",
                Pn, (int)bfl, bf);
            Error(ctx);
        }
        (void)snpf(bfc, bfl, "%s", bf);
        return (bfc);
    }
    return ((char *)NULL);
}
#endif /* !defined(N_UNIX) */

/*
 * initialize() - perform all initialization
 */

void initialize(struct lsof_context *ctx) {
#if __FreeBSD_version < 1400062
    get_kernel_access(ctx);
#endif /* __FreeBSD_version < 1400062 */
}

/*
 * kread() - read from kernel memory
 */

int kread(struct lsof_context *ctx, /* context */
          KA_T addr,                /* kernel memory address */
          char *buf,                /* buffer to receive data */
          READLEN_T len)            /* length to read */
{
    int br;

    if (!Kd)
        return 1;
    br = kvm_read(Kd, (u_long)addr, buf, len);

    return ((br == len) ? 0 : 1);
}
