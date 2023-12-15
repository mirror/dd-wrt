/*
 * dproc.c - NetBSD process access functions for lsof
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

#if __NetBSD_Version__ >= 499006200
/*
 * In NetBSD-4.99.62, struct fdfile was added, struct filedesc::fd_ofiles
 * changed type from struct file ** to struct fdfile **, and
 * fd_ofileflags disappeared from struct filedesc, being
 * replaced by fields in struct fdfile.
 */
#    define HAVE_STRUCT_FDFILE 1
#    define FILESTRUCT struct fdfile
#else
#    undef HAVE_STRUCT_FDFILE
#    define FILESTRUCT struct file
#endif
#if __NetBSD_Version__ >= 599001400
/*
 * Between NetBSD-5.99.13 and 5.99.14, struct fdtab was added, and
 * struct filedesc::fd_ofiles and fd_nfiles were replaced by
 * struct filedesc::fd_dt (a pointer to struct fdtab).
 */
#    define HAVE_STRUCT_FDTAB 1
#    define NFILES(fd, dt) ((dt).dt_nfiles)
#    define OFILES(fd, dt) ((fd).fd_dt->dt_ff)
#else
#    undef HAVE_STRUCT_FDTAB
#    define NFILES(fd, dt) ((fd).fd_nfiles)
#    define OFILES(fd, dt) ((fd).fd_ofiles)
#endif

static void enter_vn_text(struct lsof_context *ctx, KA_T va, int *n);
static void get_kernel_access(struct lsof_context *ctx);
static void process_text(struct lsof_context *ctx, KA_T vm);

/*
 * Local static values
 */

static MALLOC_S Nv = 0; /* allocated Vp[] entries */
static KA_T *Vp = NULL; /* vnode address cache */

/*
 * ckkv - check kernel version
 */

void ckkv(struct lsof_context *ctx, /* context */
          char *d,                  /* dialect */
          char *er,                 /* expected release */
          char *ev,                 /* expected version */
          char *ea)                 /* expected architecture */
{

#if defined(HASKERNIDCK)
    size_t l;
    int m[2];
    char v[64];

    if (Fwarn)
        return;
    /*
     * Read kernel version.
     */
    m[0] = CTL_KERN;
    m[1] = KERN_OSRELEASE;
    l = sizeof(v);
    if (sysctl(m, 2, v, &l, NULL, 0) < 0) {
        (void)fprintf(stderr, "%s: CTL_KERN, KERN_OSRELEASE: %s\n", Pn,
                      strerror(errno));
        Error(ctx);
    }
    /*
     * Warn if the actual and expected releases don't match.
     */
    if (!er || strcmp(v, er))
        (void)fprintf(stderr,
                      "%s: WARNING: compiled for %s release %s; this is %s.\n",
                      Pn, d, er ? er : "UNKNOWN", v);
#endif /* defined(HASKERNIDCK) */
}

/*
 * enter_vn_text() - enter a vnode text reference
 */

static void enter_vn_text(struct lsof_context *ctx, /* context */
                          KA_T va,                  /* vnode address */
                          int *n)                   /* Vp[] entries in use */
{
    int i;
    /*
     * Ignore the request if the vnode has already been entered.
     */
    for (i = 0; i < *n; i++) {
        if (va == Vp[i])
            return;
    }
    /*
     * Save the text file information.
     */
    alloc_lfile(ctx, LSOF_FD_PROGRAM_TEXT, -1);
    Cfp = (struct file *)NULL;
    process_node(ctx, (KA_T)va);
    if (Lf->sf)
        link_lfile(ctx);
    if (i >= Nv) {

        /*
         * Allocate space for remembering the vnode.
         */
        Nv += 10;
        if (!Vp)
            Vp = (KA_T *)malloc((MALLOC_S)(sizeof(struct vnode *) * 10));
        else
            Vp = (KA_T *)realloc((MALLOC_P *)Vp, (MALLOC_S)(Nv * sizeof(KA_T)));
        if (!Vp) {
            (void)fprintf(stderr, "%s: no txt ptr space, PID %d\n", Pn,
                          Lp->pid);
            Error(ctx);
        }
    }
    /*
     * Remember the vnode.
     */
    Vp[*n] = va;
    (*n)++;
}

/*
 * gather_proc_info() -- gather process information
 */

void gather_proc_info(struct lsof_context *ctx) {
    struct filedesc fd;
    int i, nf;
    MALLOC_S nb;
    static FILESTRUCT **ofb = NULL;
    static int ofbb = 0;
    short pss, sf;
    int px;
    uid_t uid;

#if defined(HASCWDINFO)
    struct cwdinfo cw;
#    define CDIR cw.cwdi_cdir
#    define RDIR cw.cwdi_rdir
#else /* !defined(HASCWDINFO) */
#    define CDIR fd.fd_cdir
#    define RDIR fd.fd_rdir
#endif /* defined(HASCWDINFO) */

#if defined(HASFSTRUCT)
    static char *pof = (char *)NULL;
    static int pofb = 0;
#endif /* defined(HASFSTRUCT) */

#if defined(HASKVMGETPROC2)
    struct kinfo_proc2 *p;
#    define KVMPROCSZ2 sizeof(struct kinfo_proc2)
#else  /* !defined(HASKVMGETPROC2) */
    struct kinfo_proc *p;
#endif /* defined(HASKVMGETPROC2) */

#if HAVE_STRUCT_FDTAB
    struct fdtab dt;
#endif /* HAVE_STRUCT_FDTAB */

    /*
     * Read the process table.
     */

#if defined(HASKVMGETPROC2)
    P = kvm_getproc2(Kd, KERN_PROC_ALL, 0, KVMPROCSZ2, &Np);
#else  /* !defined(HASKVMGETPROC2) */
    P = kvm_getprocs(Kd, KERN_PROC_ALL, 0, &Np);
#endif /* defined(HASKVMGETPROC2) */

    if (!P) {
        (void)fprintf(stderr, "%s: can't read process table: %s\n", Pn,
                      kvm_geterr(Kd));
        Error(ctx);
    }
    /*
     * Examine proc structures and their associated information.
     */

    for (p = P, px = 0; px < Np; px++, p++) {
        if (p->P_STAT == 0 || p->P_STAT == SZOMB)
            continue;
        /*
         * Read process information, process group structure (if
         * necessary), and User ID (if necessary).
         *
         * See if process is excluded.
         *
         * Read file structure pointers.
         */
        uid = p->P_UID;
        if (is_proc_excl(ctx, (int)p->P_PID, (int)p->P_PGID, (UID_ARG)uid, &pss,
                         &sf)) {
            continue;
        }
        if (!p->P_FD || kread(ctx, (KA_T)p->P_FD, (char *)&fd, sizeof(fd)))
            continue;
        if (!fd.fd_refcnt)
            continue;
#if HAVE_STRUCT_FDTAB
        if (!fd.fd_dt || kread(ctx, (KA_T)fd.fd_dt, (char *)&dt, sizeof(dt)))
            continue;
#endif /* ! HAVE_STRUCT_FDTAB */
        if (fd.fd_lastfile > NFILES(fd, dt))
            continue;

#if defined(HASCWDINFO)
        if (!p->P_CWDI || kread(ctx, (KA_T)p->P_CWDI, (char *)&cw, sizeof(cw)))
            CDIR = RDIR = (struct vnode *)NULL;
#endif /* defined(HASCWDINFO) */

        /*
         * Allocate a local process structure.
         */
        if (is_cmd_excl(ctx, p->P_COMM, &pss, &sf))
            continue;
        alloc_lproc(ctx, (int)p->P_PID, (int)p->P_PGID, (int)p->P_PPID,
                    (UID_ARG)uid, p->P_COMM, (int)pss, (int)sf);
        Plf = (struct lfile *)NULL;
        Kpa = (KA_T)p->P_ADDR;
        /*
         * Save current working directory information.
         */
        if (CDIR) {
            alloc_lfile(ctx, LSOF_FD_CWD, -1);
            Cfp = (struct file *)NULL;
            process_node(ctx, (KA_T)CDIR);
            if (Lf->sf)
                link_lfile(ctx);
        }
        /*
         * Save root directory information.
         */
        if (RDIR) {
            alloc_lfile(ctx, LSOF_FD_ROOT_DIR, -1);
            Cfp = (struct file *)NULL;
            process_node(ctx, (KA_T)RDIR);
            if (Lf->sf)
                link_lfile(ctx);
        }

        /*
         * Save information on the text file.
         */
        if (p->P_VMSPACE)
            process_text(ctx, (KA_T)p->P_VMSPACE);
        /*
         * Read open file structure pointers.
         */
        if (!OFILES(fd, dt) || (nf = NFILES(fd, dt)) <= 0)
            continue;
        nb = (MALLOC_S)(sizeof(FILESTRUCT *) * nf);
        if (nb > ofbb) {
            if (!ofb)
                ofb = (FILESTRUCT **)malloc(nb);
            else
                ofb = (FILESTRUCT **)realloc((MALLOC_P *)ofb, nb);
            if (!ofb) {
                (void)fprintf(stderr, "%s: PID %d, no file * space\n", Pn,
                              p->P_PID);
                Error(ctx);
            }
            ofbb = nb;
        }
        if (kread(ctx, (KA_T)OFILES(fd, dt), (char *)ofb, nb))
            continue;

#if defined(HASFSTRUCT)
        nb = (MALLOC_S)(sizeof(char) * nf);
        if (nb > pofb) {
            if (!pof)
                pof = (char *)malloc(nb);
            else
                pof = (char *)realloc((MALLOC_P *)pof, nb);
            if (!pof) {
                (void)fprintf(stderr, "%s: PID %d, no file flag space\n", Pn,
                              p->P_PID);
                Error(ctx);
            }
            pofb = nb;
        }
#    if !HAVE_STRUCT_FDFILE
        if (!fd.fd_ofileflags || kread(ctx, (KA_T)fd.fd_ofileflags, pof, nb))
            zeromem(pof, nb);
#    endif /* ! HAVE_STRUCT_FDFILE */
#endif     /* defined(HASFSTRUCT) */

        /*
         * Save information on file descriptors.
         */
        for (i = 0; i < nf; i++) {
            if (ofb[i]) {
#if HAVE_STRUCT_FDFILE
                struct fdfile fdf;
                if (kread(ctx, (KA_T)ofb[i], (char *)&fdf, sizeof(fdf)))
                    continue;
                Cfp = fdf.ff_file;
                if (Cfp == NULL)
                    continue;
                if (pof)
                    pof[i] = fdf.ff_exclose;
#else  /* ! HAVE_STRUCT_FDFILE */
                Cfp = ofb[i];
#endif /* ! HAVE_STRUCT_FDFILE */
                alloc_lfile(ctx, LSOF_FD_NUMERIC, i);
                process_file(ctx, (KA_T)Cfp);
                if (Lf->sf) {

#if defined(HASFSTRUCT)
                    Lf->pof = (long)pof[i];
#endif /* defined(HASFSTRUCT) */

                    link_lfile(ctx);
                }
            }
        }
        /*
         * Examine results.
         */
        if (examine_lproc(ctx))
            return;
    }
}

/*
 * get_kernel_access() - get access to kernel memory
 */

static void get_kernel_access(struct lsof_context *ctx) {
    KA_T v;
    /*
     * Check kernel version.
     */
    (void)ckkv(ctx, "NetBSD", LSOF_VSTR, (char *)NULL, (char *)NULL);
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
     * See if the non-KMEM memory and name list files are readable.
     */
    if ((Memory && !is_readable(Memory, 1)) ||
        (Nmlst && !is_readable(Nmlst, 1)))
        Error(ctx);
#endif /* defined(WILLDROPGID) */

    /*
     * Open kernel memory access.
     */
    if ((Kd = kvm_openfiles(Nmlst, Memory, NULL, O_RDONLY, NULL)) == NULL) {
        (void)fprintf(stderr,
                      "%s: kvm_openfiles(execfile=%s, corefile=%s): %s\n", Pn,
                      Nmlst,
                      Memory ? Memory :

#if defined(_PATH_MEM)
                             _PATH_MEM,
#else  /* !defined(_PATH_MEM) */
                             "default",
#endif /* defined(_PATH_MEM) */

                      strerror(errno));
        Error(ctx);
    }
    (void)build_Nl(ctx, Drive_Nl);
    if (kvm_nlist(Kd, Nl) < 0) {
        (void)fprintf(stderr, "%s: can't read namelist from %s\n", Pn, Nmlst);
        Error(ctx);
    }

#if defined(WILLDROPGID)
    /*
     * Drop setgid permission, if necessary.
     */
    if (!Memory)
        (void)dropgid(ctx);
#endif /* defined(WILLDROPGID) */

    /*
     * Read the kernel's page shift amount, if possible.
     */
    if (get_Nl_value(ctx, "pgshift", Drive_Nl, &v) < 0 || !v ||
        kread(ctx, (KA_T)v, (char *)&pgshift, sizeof(pgshift)))
        pgshift = 0;
}

#if !defined(N_UNIX)
/*
 * get_nlist_path() - get kernel name list path
 */

char *get_nlist_path(struct lsof_context *ctx, /* context */
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
                Pn, bfl, bf);
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

void initialize(struct lsof_context *ctx) { get_kernel_access(ctx); }

/*
 * kread() - read from kernel memory
 */

int kread(struct lsof_context *ctx, /* context */
          KA_T addr,                /* kernel memory address */
          char *buf,                /* buffer to receive data */
          READLEN_T len)            /* length to read */
{
    int br;

    br = kvm_read(Kd, (u_long)addr, buf, len);
    return ((br == len) ? 0 : 1);
}

/*
 * process_text() - process text information
 */
void process_text(struct lsof_context *ctx, /* context */
                  KA_T vm)                  /* kernel vm space pointer */
{
    int i, j;
    KA_T ka;
    int n = 0;
    struct vm_map_entry vmme, *e;
    struct vmspace vmsp;

#if !defined(UVM)
    struct pager_struct pg;
    struct vm_object vmo;
#endif /* !defined(UVM) */

    /*
     * Read the vmspace structure for the process.
     */
    if (kread(ctx, vm, (char *)&vmsp, sizeof(vmsp)))
        return;
        /*
         * Read the vm_map structure.  Search its vm_map_entry structure list.
         */

#if !defined(UVM)
    if (!vmsp.vm_map.is_main_map)
        return;
#endif /* !defined(UVM) */

    for (i = 0; i < vmsp.vm_map.nentries; i++) {

        /*
         * Read the next vm_map_entry.
         */
        if (!i)
            e = &vmsp.vm_map.header;
        else {
            if (!(ka = (KA_T)e->next))
                return;
            e = &vmme;
            if (kread(ctx, ka, (char *)e, sizeof(vmme)))
                return;
        }

#if defined(UVM)
        /*
         * Process the uvm_obj pointer of a UVM map entry with a UVM_ET_OBJ
         * type as a vnode pointer.
         */
        if ((e->etype > UVM_ET_OBJ) && e->object.uvm_obj)
            (void)enter_vn_text(ctx, (KA_T)e->object.uvm_obj, &n);
#else  /* !defined(UVM) */
        /*
         * Read the map entry's object and the object's shadow.
         * Look for a PG_VNODE pager handle.
         */
        if (e->is_a_map || e->is_sub_map)
            continue;
        for (j = 0, ka = (KA_T)e->object.vm_object; j < 2 && ka;
             j++, ka = (KA_T)vmo.shadow) {
            if (kread(ctx, ka, (char *)&vmo, sizeof(vmo)))
                break;
            if (!(ka = (KA_T)vmo.pager) ||
                kread(ctx, ka, (char *)&pg, sizeof(pg)))
                continue;
            if (!pg.pg_handle || pg.pg_type != PG_VNODE)
                continue;
            (void)enter_vn_text((KA_T)pg.pg_handle, &n);
        }
#endif /* defined(UVM) */
    }
}
