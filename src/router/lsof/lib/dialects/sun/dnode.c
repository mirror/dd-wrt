/*
 * dnode.c - Solaris node reading functions for lsof
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

#if solaris >= 110000
#    include <sys/fs/sdev_impl.h>
#endif /* solaris>=110000 */

#undef fs_bsize
#include <sys/fs/ufs_inode.h>

#if solaris >= 110000 && defined(HAS_LIBCTF)
/*
 * Sockfs support for Solaris 11 via libctf
 */

/*
 * Sockfs internal structure definitions
 *
 * The structure definitions may look like kernel structures, but they are
 * not.  They have been defined to have member names that duplicate those
 * used by the kernel that are of interest to lsof.  Member values are
 * obtained via the CTF library, libctf.
 *
 * Robert Byrnes developed the CTF library access code and contributed it
 * to lsof.
 */

struct soaddr {              /* sadly, CTF doesn't grok this
                              * structure */
    struct sockaddr *soa_sa; /* address */
    t_uscalar_t soa_len;     /* length in bytes */
    t_uscalar_t soa_maxlen;  /* maximum length */
};

typedef struct sotpi_info {
    dev_t sti_dev;                  /* sonode device */
    struct soaddr sti_laddr;        /* local address */
    struct soaddr sti_faddr;        /* peer address */
    struct so_ux_addr sti_ux_laddr; /* bound local address */
    struct so_ux_addr sti_ux_faddr; /* bound peer address */
    t_scalar_t sti_serv_type;       /* service type */
} sotpi_info_t;

/*
 * CTF definitions for sockfs
 */

static int Sockfs_ctfs = 0; /* CTF initialization status for
                             * sockfs */

#    if defined(_LP64)
#        define SOCKFS_MOD_FORMAT "/kernel/%s/genunix"
#    else /* !defined(_LP64) */
#        define SOCKFS_MOD_FORMAT "/kernel/fs/sockfs"
#    endif /* defined(_LP64) */

/* sockfs module pathname template to
 * which the kernel's instruction type
 * set is added for CTF access */

/*
 * Sockfs access definitions and structures
 */

#    define SOADDR_TYPE_NAME "soaddr"

static CTF_member_t soaddr_members[] = {CTF_MEMBER(soa_sa),
#    define MX_soa_sa 0

                                        CTF_MEMBER(soa_len),
#    define MX_soa_len 1

                                        CTF_MEMBER(soa_maxlen),
#    define MX_soa_maxlen 2

                                        {NULL, 0}};

#    define SOTPI_INFO_TYPE_NAME "sotpi_info_t"

static CTF_member_t sotpi_info_members[] = {CTF_MEMBER(sti_dev),
#    define MX_sti_dev 0

                                            CTF_MEMBER(sti_laddr),
#    define MX_sti_laddr 1

                                            CTF_MEMBER(sti_faddr),
#    define MX_sti_faddr 2

                                            CTF_MEMBER(sti_ux_laddr),
#    define MX_sti_ux_laddr 3

                                            CTF_MEMBER(sti_ux_faddr),
#    define MX_sti_ux_faddr 4

                                            CTF_MEMBER(sti_serv_type),
#    define MX_sti_serv_type 5

                                            {NULL, 0}};

/*
 * CTF sockfs request table
 */

static CTF_request_t Sockfs_requests[] = {
    {SOTPI_INFO_TYPE_NAME, sotpi_info_members}, {NULL, NULL}};

/*
 * Sockfs function prototypes
 */

static int read_nsti(struct lsof_context *ctx, struct sonode *so,
                     sotpi_info_t *stpi);
#endif /* solaris>=110000 && defined(HAS_LIBCTF) */

#if defined(HAS_ZFS) && defined(HAS_LIBCTF)
/*
 * ZFS support via libctf
 */

/*
 * ZFS internal structure definitions
 *
 * The structure definitions may look like kernel structures, but they are
 * not.  They have been defined to have member names that duplicate those
 * used by the kernel that are of interest to lsof.  Member values are
 * obtained via the CTF library, libctf.
 *
 * Robert Byrnes developed the CTF library access code and contributed it
 * to lsof.
 */

typedef struct zfsvfs {
    vfs_t *z_vfs; /* pointer to VFS */
} zfsvfs_t;

typedef struct znode_phys {
    uint64_t zp_size;  /* file size (ZFS below 5) */
    uint64_t zp_links; /* links (ZFS below 5) */
} znode_phys_t;

typedef struct znode {
    zfsvfs_t *z_zfsvfs;   /* pointer to associated vfs */
    vnode_t *z_vnode;     /* pointer to associated vnode */
    uint64_t z_id;        /* node ID */
    znode_phys_t *z_phys; /* pointer to persistent znode (ZFS
                           * below 5) */
    uint64_t z_links;     /* links (ZFS 5 and above) */
    uint64_t z_size;      /* file size (ZFS 5 and above) */
} znode_t;

/*
 * CTF definitions for ZFS
 */

static int ZFS_ctfs = 0; /* CTF initialization status for ZFS */

#    if defined(_LP64)
#        define ZFS_MOD_FORMAT "/kernel/fs/%s/zfs"
#    else /* !defined(_LP64) */
#        define ZFS_MOD_FORMAT "/kernel/fs/zfs"
#    endif /* defined(_LP64) */

/* ZFS module pathname template to
 * which the kernel's instruction type
 * set is added for CTF access */

/*
 * ZFS access definitions and structures
 */

#    define ZNODE_TYPE_NAME "znode_t"

static CTF_member_t znode_members[] = {CTF_MEMBER(z_zfsvfs),
#    define MX_z_zfsvfs 0

                                       CTF_MEMBER(z_vnode),
#    define MX_z_vnode 1

                                       CTF_MEMBER(z_id),
#    define MX_z_id 2

                                       CTF_MEMBER(z_link_node),
#    define MX_z_link_node 3

                                       CTF_MEMBER(z_phys),
#    define MX_z_phys 4

                                       CTF_MEMBER(z_links),
#    define MX_z_links 5

                                       CTF_MEMBER(z_size),
#    define MX_z_size 6

                                       {NULL, 0}};

#    define ZNODE_PHYS_TYPE_NAME "znode_phys_t"

static CTF_member_t znode_phys_members[] = {CTF_MEMBER(zp_size),
#    define MX_zp_size 0

                                            CTF_MEMBER(zp_links),
#    define MX_zp_links 1

                                            {NULL, 0}};

#    define ZFSVFS_TYPE_NAME "zfsvfs_t"

static CTF_member_t zfsvfs_members[] = {CTF_MEMBER(z_vfs),
#    define MX_z_vfs 0

                                        {NULL, 0}};

/*
 * CTF ZFS request table
 */

static CTF_request_t ZFS_requests[] = {
    {ZNODE_TYPE_NAME, znode_members},
    {ZNODE_PHYS_TYPE_NAME, znode_phys_members},
    {ZFSVFS_TYPE_NAME, zfsvfs_members},
    {NULL, NULL}};

/*
 * Missing members exceptions -- i.e., CTF_getmem won't consider it
 * an error if any of these members are undefined.
 */

typedef struct CTF_exception {
    char *tynm;  /* type name */
    char *memnm; /* member name */
} CTF_exception_t;

static CTF_exception_t CTF_exceptions[] = {{ZNODE_TYPE_NAME, "z_phys"},
                                           {ZNODE_TYPE_NAME, "z_links"},
                                           {ZNODE_TYPE_NAME, "z_size"},
                                           {NULL, NULL}};

/*
 * ZFS function prototypes
 */

static int read_nzn(struct lsof_context *ctx, KA_T na, KA_T nza, znode_t *z);
static int read_nznp(struct lsof_context *ctx, KA_T nza, KA_T nzpa,
                     znode_phys_t *zp);
static int read_nzvfs(struct lsof_context *ctx, KA_T nza, KA_T nzva,
                      zfsvfs_t *zv);
#endif /* defined(HAS_ZFS) && defined(HAS_LIBCTF) */

static struct l_dev *finddev(struct lsof_context *ctx, dev_t *dev, dev_t *rdev,
                             int flags);

/*
 * Finddev() "look-in " flags
 */

#define LOOKDEV_TAB 0x01    /* look in device table */
#define LOOKDEV_CLONE 0x02  /* look in Clone table */
#define LOOKDEV_PSEUDO 0x04 /* look in Pseudo table */
#define LOOKDEV_ALL (LOOKDEV_TAB | LOOKDEV_CLONE | LOOKDEV_PSEUDO)
/* look all places */

/*
 * SAM-FS definitions
 */

#define SAMFS_NMA_MSG "(limited SAM-FS info)"

/*
 * Voptab definitions
 */

typedef struct build_v_optab {
    char *dnm;  /* drive_NL name */
    char *fsys; /* file system type name */
    int nty;    /* node type index (i.e., N_*) */
} build_v_optab_t;

static build_v_optab_t Build_v_optab[] = {
    {"auvops", "autofs", N_AUTO},
    {"avops", "afs", N_AFS},
    {"afsops", "afs", N_AFS},
    {"ctfsadir", NULL, N_CTFSADIR},
    {"ctfsbund", NULL, N_CTFSBUND},
    {"ctfscdir", NULL, N_CTFSCDIR},
    {"ctfsctl", NULL, N_CTFSCTL},
    {"ctfsevt", NULL, N_CTFSEVT},
    {"ctfslate", NULL, N_CTFSLATE},
    {"ctfsroot", NULL, N_CTFSROOT},
    {"ctfsstat", NULL, N_CTFSSTAT},
    {"ctfssym", NULL, N_CTFSSYM},
    {"ctfstdir", NULL, N_CTFSTDIR},
    {"ctfstmpl", NULL, N_CTFSTMPL},

#if defined(HASCACHEFS)
    {"cvops", NULL, N_CACHE},
#endif /* defined(HASCACHEFS) */

    {"devops", "devfs", N_DEV},
    {"doorops", NULL, N_DOOR},
    {"fdops", "fd", N_FD},
    {"fd_ops", "fd", N_FD},
    {"fvops", "fifofs", N_FIFO},
    {"hvops", "hsfs", N_HSFS},
    {"lvops", "lofs", N_LOFS},
    {"mntops", "mntfs", N_MNT},
    {"mvops", "mvfs", N_MVFS},
    {"n3vops", NULL, N_NFS},

#if solaris >= 100000
    {"n4vops", NULL, N_NFS4},
#else  /* solaris<100000 */
    {"n4vops", NULL, N_NFS},
#endif /* solaris>=100000 */

    {"nmvops", "namefs", N_NM},
    {"nvops", NULL, N_NFS},
    {"pdvops", "pcfs", N_PCFS},
    {"pfvops", "pcfs", N_PCFS},
    {"portvops", NULL, N_PORT},
    {"prvops", "proc", N_PROC},
    {"sam1vops", NULL, N_SAMFS},
    {"sam2vops", NULL, N_SAMFS},
    {"sam3vops", NULL, N_SAMFS},
    {"sam4vops", NULL, N_SAMFS},
    {"sckvops", "sockfs", N_SOCK},
    {"devipnetops", "sdevfs", N_SDEV},
    {"devnetops", "sdevfs", N_SDEV},
    {"devptsops", "sdevfs", N_SDEV},
    {"devvtops", "sdevfs", N_SDEV},
    {"socketvops", "sockfs", N_SOCK},
    {"sdevops", "sdevfs", N_SDEV},
    {"shvops", "sharedfs", N_SHARED},
    {"sncavops", "sockfs", N_SOCK},
    {"stpivops", "sockfs", N_SOCK},
    {"spvops", "specfs", N_REGLR},
    {"tvops", "tmpfs", N_TMP},
    {"uvops", "ufs", N_REGLR},
    {"vvfclops", "vxfs", N_VXFS},
    {"vvfops", "vxfs", N_VXFS},
    {"vvfcops", "vxfs", N_VXFS},
    {"vvops", "vxfs", N_VXFS},
    {"vvops_p", "vxfs", N_VXFS},
    {"zfsdops", "zfs", N_ZFS},
    {"zfseops", "zfs", N_ZFS},
    {"zfsfops", "zfs", N_ZFS},
    {"zfsshops", "zfs", N_ZFS},
    {"zfssymops", "zfs", N_ZFS},
    {"zfsxdops", "zfs", N_ZFS},
    {NULL, NULL, 0} /* table end */
};

typedef struct v_optab {
    char *fsys;           /* file system type name */
    int fx;               /* Fsinfo[] index (-1 if none) */
    int nty;              /* node type index (i.e., N_*) */
    KA_T v_op;            /* vnodeops address */
    struct v_optab *next; /* next entry */
} v_optab_t;

static v_optab_t **FxToVoptab = (v_optab_t **)NULL;
/* table to convert file system index
 * to Voptab address[] -- built by
 * build_Voptab() */
static v_optab_t **Voptab = (v_optab_t **)NULL;
/* table to convert vnode v_op
 * addresses to file system name and
 * node type -- built by build_Voptab()
 * and addressed through the HASHVOP()
 * macro */

#define VOPHASHBINS                                                            \
    256 /* number of Voptab[] hash bins --                                     \
         * MUST BE A POWER OF TWO! */

/*
 * Local function prototypes
 */

static void build_Voptab(struct lsof_context *ctx);
static enum lsof_lock_mode isvlocked(struct lsof_context *ctx,
                                     struct vnode *va);
static int readinode(struct lsof_context *ctx, KA_T ia, struct inode *i);
static void read_mi(struct lsof_context *ctx, KA_T s, dev_t *dev, caddr_t so,
                    int *so_st, KA_T *so_ad, struct l_dev **sdp);

#if solaris >= 20500
#    if solaris >= 20600
static int read_nan(struct lsof_context *ctx, KA_T na, KA_T aa,
                    struct fnnode *rn);
static int read_nson(struct lsof_context *ctx, KA_T na, KA_T sa,
                     struct sonode *sn);
static int read_nusa(struct lsof_context *ctx, struct soaddr *so,
                     struct sockaddr_un *ua);
#    else  /* solaris<20600 */
static int read_nan(struct lsof_context *ctx, KA_T na, KA_T aa,
                    struct autonode *a);
#    endif /* solaris>=20600 */
static int idoorkeep(struct lsof_context *ctx, struct door_node *d);
static int read_ndn(struct lsof_context *ctx, KA_T na, KA_T da,
                    struct door_node *d);
#endif /* solaris>=20500 */

#if solaris >= 110000
static int read_nsdn(struct lsof_context *ctx, KA_T na, KA_T sa,
                     struct sdev_node *sdn, struct vattr *sdva);
#endif /* solaris>=110000 */

static int read_nfn(struct lsof_context *ctx, KA_T na, KA_T fa,
                    struct fifonode *f);
static int read_nhn(struct lsof_context *ctx, KA_T na, KA_T ha,
                    struct hsnode *h);
static int read_nin(struct lsof_context *ctx, KA_T na, KA_T ia,
                    struct inode *i);
static int read_nmn(struct lsof_context *ctx, KA_T na, KA_T ia,
                    struct mvfsnode *m);
static int read_npn(struct lsof_context *ctx, KA_T na, KA_T pa,
                    struct pcnode *p);
static int read_nrn(struct lsof_context *ctx, KA_T na, KA_T ra,
                    struct rnode *r);

#if solaris >= 100000
static int read_nctfsn(struct lsof_context *ctx, int ty, KA_T na, KA_T ca,
                       char *cn);
static int read_nprtn(struct lsof_context *ctx, KA_T na, KA_T ra, port_t *p);
static int read_nrn4(struct lsof_context *ctx, KA_T na, KA_T ra,
                     struct rnode4 *r);
#endif /* solaris>=100000 */

static int read_nsn(struct lsof_context *ctx, KA_T na, KA_T sa,
                    struct snode *s);
static int read_ntn(struct lsof_context *ctx, KA_T na, KA_T ta,
                    struct tmpnode *t);
static int read_nvn(struct lsof_context *ctx, KA_T na, KA_T va,
                    struct vnode *v);

#if defined(HASPROCFS)
static int read_npi(struct lsof_context *ctx, KA_T na, struct vnode *v,
                    struct pid *pids);
#endif /* defined(HASPROCFS) */

static char *ent_fa(KA_T *a1, KA_T *a2, char *d, int *len);
static int is_socket(struct lsof_context *ctx, struct vnode *v);
static int read_cni(struct lsof_context *ctx, struct snode *s, struct vnode *rv,
                    struct vnode *v, struct snode *rs, struct dev_info *di,
                    char *din, int dinl);

#if defined(HASCACHEFS)
static int read_ncn(struct lsof_context *ctx, KA_T na, KA_T ca,
                    struct cnode *cn);
#endif /* defined(HASCACHEFS) */

static int read_nln(struct lsof_context *ctx, KA_T na, KA_T la,
                    struct lnode *ln);
static int read_nnn(struct lsof_context *ctx, KA_T na, KA_T nna,
                    struct namenode *n);

#if solaris < 100000
static void savesockmod(struct so_so *so, struct so_so *sop, int *so_st);
#else  /* solaris>=100000 */
static int read_ndvn(struct lsof_context *ctx, KA_T na, KA_T da,
                     struct dv_node *dv, dev_t *dev, unsigned char *devs);
#endif /* solaris<100000 */

/*
 * Local static values
 */

static KA_T Spvops = (KA_T)0; /* specfs vnodeops address -- saved
                               * by build_Voptab() */
static KA_T Vvops[VXVOP_NUM]; /* addresses of:
                               *   vx_fcl_dnodeops_p (VXVOP_FCL)
                               *   fdd_vnops (VXVOP_FDD)
                               *   fdd_chain_vnops (VXVOP_FDDCH),
                               *   vx_vnodeops (VXVOP_REG)
                               *   vx_vnodeops_p (VXVOP_REG_P)
                               *   -- saved by build_Voptab() */

/*
 * Local macros
 *
 * GETVOPS() -- get direct or indirect *vnodeops address
 *
 * HASHVOP() -- hash the vnode's v_op address
 */

#if defined(VOPNAME_OPEN) && solaris >= 100000
#    define GETVOPS(name, nl, ops)                                             \
        if (get_Nl_value(ctx, name, nl, &ops) < 0)                                  \
            ops = (KA_T)0;                                                     \
        else if (kread(ctx, ops, (char *)&ops, sizeof(ops)))                   \
        ops = (KA_T)0
#else /* !defined(VOPNAME_OPEN) || solaris<100000 */
#    define GETVOPS(name, nl, ops)                                             \
        if (get_Nl_value(ctx, name, nl, &ops) < 0)                                  \
        ops = (KA_T)0
#endif /* defined(VOPNAME_OPEN) && solaris>=100000 */

#define HASHVOP(ka)                                                            \
    ((int)((((ka & 0x1fffffff) * 31415) >> 3) & (VOPHASHBINS - 1)))

/*
 * build_Voptab() -- build Voptab[]
 */

static void build_Voptab(struct lsof_context *ctx) {
    build_v_optab_t *bp;      /* Build_v_optab[] pointer */
    int fx;                   /* temporary file system type index */
    int h;                    /* hash index */
    int i, j;                 /* temporary indexes */
    KA_T ka;                  /* temporary kernel address */
    v_optab_t *nv, *vp, *vpp; /* Voptab[] working pointers */
    int vv = 0;               /* number of Vvops[] addresses that
                               * have been located */
                              /*
                               * If Voptab[] is allocated, return; otherwise allocate space for Voptab[]
                               * and FxToVoptab[] amd fill them.
                               */
    if (Voptab)
        return;
    /*
     * During first call, allocate space for Voptab[] and FxToVoptab[].
     */

    if (!(Voptab =
              (v_optab_t **)calloc((MALLOC_S)VOPHASHBINS, sizeof(v_optab_t)))) {
        (void)fprintf(stderr, "%s: no space for Voptab\n", Pn);
        Error(ctx);
    }
    if (!(FxToVoptab =
              (v_optab_t **)calloc((MALLOC_S)Fsinfomax, sizeof(v_optab_t *)))) {
        (void)fprintf(stderr, "%s: no space for FxToVoptab\n", Pn);
        Error(ctx);
    }
    for (i = 0; i < VXVOP_NUM; i++) {
        Vvops[i] = (KA_T)NULL;
    }
    /*
     * Use Build_v_optab[] to build Voptab[].
     */
    for (bp = Build_v_optab; bp->dnm; bp++) {

        /*
         * Get the kernel address for the symbol.  Do nothing if it can't
         * be determined.
         */
        GETVOPS(bp->dnm, Drive_Nl, ka);
        if (!ka)
            continue;
        /*
         * Check the Voptab[] for the address.
         */
        h = HASHVOP(ka);
        for (vp = Voptab[h], vpp = (v_optab_t *)NULL; vp; vp = vp->next) {
            if (vp->v_op == ka)
                break;
            vpp = vp;
        }
        if (vp) {

            /*
             * Ignore duplicates.
             */
            continue;
        }
        /*
         * No Voptab[] entry was found, so allocate space for a new
         * v_optab_t structure, determine its file system type index,
         * fill it and link it to the Voptab[].
         */
        if (!(nv = (v_optab_t *)malloc((MALLOC_S)sizeof(v_optab_t)))) {
            (void)fprintf(stderr, "%s: out of Voptab space at: %s\n", Pn,
                          bp->dnm);
            Error(ctx);
        }
        nv->fsys = bp->fsys;
        nv->fx = -1;
        nv->nty = bp->nty;
        nv->next = (v_optab_t *)NULL;
        nv->v_op = ka;
        if (bp->fsys) {
            for (i = 0; i < Fsinfomax; i++) {
                if (!strcmp(bp->fsys, Fsinfo[i])) {
                    nv->fx = i;
                    break;
                }
            }
        }
        if (!Voptab[h])
            Voptab[h] = nv;
        else
            vpp->next = nv;
        /*
         * Handle special v_op addresses:
         *
         *   special vnode ops;
         *   VxFS ops.
         */
        if (!Spvops) {
            if (!strcmp(bp->dnm, "spvops"))
                Spvops = ka;
        }
        for (i = 0; (i < VXVOP_NUM) && (vv < VXVOP_NUM); i++) {
            if (Vvops[i])
                continue;
            switch (i) {
            case VXVOP_FCL:
                if (!strcmp(bp->dnm, "vvfclops")) {
                    Vvops[i] = ka;
                    vv++;
                }
                break;
            case VXVOP_FDD:
                if (!strcmp(bp->dnm, "vvfops")) {
                    Vvops[i] = ka;
                    vv++;
                }
                break;
            case VXVOP_FDDCH:
                if (!strcmp(bp->dnm, "vvfcops")) {
                    Vvops[i] = ka;
                    vv++;
                }
                break;
            case VXVOP_REG:
                if (!strcmp(bp->dnm, "vvops")) {
                    Vvops[i] = ka;
                    vv++;
                }
                break;
            case VXVOP_REG_P:
                if (!strcmp(bp->dnm, "vvops_p")) {
                    Vvops[i] = ka;
                    vv++;
                }
                break;
            }
        }
    }
    /*
     * Link Voptab[] entries to FxToVoptab[] entries.
     */
    for (h = 0; h < VOPHASHBINS; h++) {
        for (vp = Voptab[h]; vp; vp = vp->next) {
            if (!vp->fsys)
                continue;
            if (((fx = vp->fx) >= 0) && (fx < Fsinfomax)) {
                if (!FxToVoptab[fx])
                    FxToVoptab[fx] = vp;
                continue;
            }
            for (i = 0; i < Fsinfomax; i++) {
                if (!strcmp(Fsinfo[i], vp->fsys)) {
                    vp->fx = i;
                    if (!FxToVoptab[i])
                        FxToVoptab[i] = vp;
                    break;
                }
            }
        }
    }
}

#if defined(HAS_LIBCTF)
/*
 * CTF_getmem() -- get CTF members
 */

int CTF_getmem(struct lsof_context *ctx, /* context*/
               ctf_file_t *f,            /* CTF file handle */
               const char *mod,          /* module name */
               const char *ty,           /* type */
               CTF_member_t *mem)        /* member table */
{
    int err;             /* error flag */
    ctf_id_t id;         /* CTF ID */
    CTF_member_t *mp;    /* member pointer */
    CTF_exception_t *xp; /* exception table pointer */
    int xs;              /* exception status */
                         /*
                          * Look up the type.
                          */
    if ((id = ctf_lookup_by_name(f, ty)) == CTF_ERR) {
        (void)fprintf(stderr, "%s: ctf_lookup_by_name: %s: %s: %s\n", Pn, mod,
                      ty, ctf_errmsg(ctf_errno(f)));
        return (1);
    }
    /*
     * Get member offsets.
     */
    if (ctf_member_iter(f, id, CTF_memCB, mem) == CTF_ERR) {
        (void)fprintf(stderr, "%s: ctf_member_iter: %s: %s: %s\n", Pn, mod, ty,
                      ctf_errmsg(ctf_errno(f)));
        return (1);
    }
    /*
     * Examine members.
     */
    for (err = 0, mp = mem; mp->m_name; mp++) {
        if (mp->m_offset == CTF_MEMBER_UNDEF) {

            /*
             * Check for an undefined member exception.  Report an error if
             * no exception is found.
             */
            for (xp = CTF_exceptions, xs = 0; xp->tynm; xp++) {
                if (!strcmp(xp->tynm, ty) && !strcmp(xp->memnm, mp->m_name)) {
                    xs = 1;
                    break;
                }
            }
            if (!xs) {
                (void)fprintf(
                    stderr,
                    "%s: getmembers: %s: %s: %s: struct member undefined\n", Pn,
                    mod, ty, mp->m_name);
                err = 1;
            }
        } else {

            /*
             * Convert bit offsets to byte offsets.
             */
            if ((mp->m_offset % NBBY) != 0) {
                (void)fprintf(
                    stderr,
                    "%s: getmembers: %s: %s: %s: struct member is bit field\n",
                    Pn, mod, ty, mp->m_name);
                err = 1;
            } else
                mp->m_offset /= NBBY;
        }
    }
    return (err);
}

/*
 * CTF_init - initialize CTF library access
 */

void CTF_init(struct lsof_context *ctx, /* context */
              int *i,                   /* initialization status */
              char *t,                  /* kernel module template */
              CTF_request_t *r)         /* CTF requests */
{
    int err;       /* error status */
    ctf_file_t *f; /* CTF file info handle */

#    if defined(_LP64)
    static char isa[256 + 1]; /* kernel instruction set name */
    static int isas = 0;      /* isa[] status */
#    endif                    /* defined(_LP64) */

    char kernmod[MAXPATHLEN];    /* kernel module pathname */
    char *kmp;                   /* kernel module path name pointer */
    static char pfn[256 + 1];    /* system platform name */
    static int pfns = 0;         /* pfn[] status: -1 = request failed
                                  *		  0 = none requested
                                  *		 >0 = available */
    char pfxkernmod[MAXPATHLEN]; /* prefixed kernel module name */
    struct stat sb;              /* stat(2) buffer */

    if (*i)
        return;

#    if defined(_LP64)
    /*
     * If CTF access hasn't been initialized and a 64 bit kernel is in use,
     * determine the name of the kernel's instruction set, and construct the
     * pathname of the kernel module, using the supplied template.
     */
    if (!isas) {
        if (sysinfo(SI_ARCHITECTURE_K, isa, sizeof(isa) - 1) == -1) {
            (void)fprintf(stderr, "%s: sysinfo: %s\n", Pn, strerror(errno));
            Error(ctx);
        }
        isas = 1;
        isa[sizeof(isa) - 1] = '\0';
    }
    (void)snprintf(kernmod, sizeof(kernmod) - 1, t, isa);
    kernmod[sizeof(kernmod) - 1] = '\0';
#    else  /* !defined(_LP64) */
    /*
     * If CTF access hasn't been initialized and a 32 bit kernel is in use, the
     * supplied template is the module path name.
     */
    (void)strncpy(kernmod, t, sizeof(kernmod) - 1);
#    endif /* defined(_LP64) */

    kernmod[sizeof(kernmod) - 1] = '\0';
    kmp = kernmod;
    if (statsafely(ctx, kmp, &sb)) {

        /*
         * The module at the specified path does not exist or is inaccessible.
         *
         * Get the platform name and construct a prefix from it for module path
         * name and see if that exists and is accessible.
         *
         * If it is, let CTF_init() use it; otherwise let CTF_init() fail on
         * the specified path.
         */
        if (pfns >= 0) {
            if (!pfns)
                pfns = sysinfo(SI_MACHINE, pfn, sizeof(pfn) - 1);
            if (pfns > 0) {
                pfn[sizeof(pfn) - 1] = '\0';
                (void)snprintf(pfxkernmod, sizeof(pfxkernmod) - 1,
                               "/platform/%s/%s", pfn,
                               (kernmod[0] == '/') ? &kernmod[1] : kernmod);
                pfxkernmod[sizeof(pfxkernmod) - 1] = '\0';
                if (!stat(pfxkernmod, &sb))
                    kmp = pfxkernmod;
            }
        }
    }
    /*
     * Open the module file and read its CTF info.
     */
    if ((f = ctf_open(kmp, &err)) == NULL) {
        (void)fprintf(stderr, "%s: ctf_open: %s: %s\n", Pn, kmp,
                      ctf_errmsg(err));
        Error(ctx);
    }
    for (err = 0; r->name; r++) {
        if (CTF_getmem(ctx, f, kmp, r->name, r->mem))
            err = 1;
    }
    (void)ctf_close(f);
    if (err)
        Error(ctx);
    *i = 1;
}

/*
 * CTF_memCB() - Callback function for ctf_member_iter()
 */

int CTF_memCB(const char *name, /* structure member name */
              ctf_id_t id,      /* CTF ID */
              ulong_t offset,   /* member offset */
              void *arg)        /* member table */
{
    CTF_member_t *mp;
    /*
     * Check for members of interest and record their offsets.
     */
    for (mp = (CTF_member_t *)arg; mp->m_name; mp++) {
        if (!strcmp(name, mp->m_name)) {
            mp->m_offset = offset;
            break;
        }
    }
    return (0);
}
#endif /* defined(HAS_LIBCTF) */

/*
 * ent_fa() - enter fattach addresses in NAME column addition
 */

static char *ent_fa(KA_T *a1, /* first fattach address (NULL OK) */
                    KA_T *a2, /* second fattach address */
                    char *d,  /* direction ("->" or "<-") */
                    int *len) /* returned description length */
{
    static char buf[1024];
    size_t bufl = sizeof(buf);
    char tbuf[32];
    /*
     * Form the fattach description.
     */
    if (!a1)

#if solaris < 20600
        (void)snpf(buf, bufl, "(FA:%s%s)", d, print_kptr(*a2, (char *)NULL, 0));
#else  /* solaris>=20600 */
        (void)snpf(buf, bufl, "(FA:%s%s)", d, print_kptr(*a2, (char *)NULL, 0));
#endif /* solaris<20600 */

    else

#if solaris < 20600
        (void)snpf(buf, bufl, "(FA:%s%s%s)",
                   print_kptr(*a1, tbuf, sizeof(tbuf)), d,
                   print_kptr(*a2, (char *)NULL, 0));
#else  /* solaris>=20600 */
        (void)snpf(buf, bufl, "(FA:%s%s%s)",
                   print_kptr(*a1, tbuf, sizeof(tbuf)), d,
                   print_kptr(*a2, (char *)NULL, 0));
#endif /* solaris<20600 */

    *len = (int)strlen(buf);
    return (buf);
}

/*
 * is_socket() - is the stream a socket?
 */

static int is_socket(struct lsof_context *ctx, /* context */
                     struct vnode *v)          /* vnode pointer */
{
    char *cp, *ep, *pf;
    int i, j, len, n, pfl;
    major_t maj;
    minor_t min;
    static struct tcpudp {
        int ds;
        major_t maj;
        minor_t min;
        char *proto;
    } tcpudp[] = {
        {0, 0, 0, "tcp"},
        {0, 0, 0, "udp"},

#if defined(HASIPv6)
        {0, 0, 0, "tcp6"},
        {0, 0, 0, "udp6"},
#endif /* defined(HASIPv6) */

    };
#define NTCPUDP (sizeof(tcpudp) / sizeof(struct tcpudp))

    static int tcpudps = 0;

    if (!v->v_stream)
        return (0);
    maj = (major_t)GET_MAJ_DEV(v->v_rdev);
    min = (minor_t)GET_MIN_DEV(v->v_rdev);
    /*
     * Fill in tcpudp[], as required.
     */
    if (!tcpudps) {

#if solaris < 80000
        pf = "/devices/pseudo/clone";
#else  /* solaris>=80000 */
        pf = "/devices/pseudo/";
#endif /* solaris<80000 */

        for (i = n = 0, pfl = (int)strlen(pf); (i < Ndev) && (n < NTCPUDP);
             i++) {
            if (strncmp(Devtp[i].name, pf, pfl) ||
                !(ep = strrchr((cp = &Devtp[i].name[pfl]), ':')) ||
                (strncmp(++ep, "tcp", 3) && strncmp(ep, "udp", 3)))
                continue;

#if solaris < 80000
            if (*(ep + 3))
#else  /* solaris>=80000 */
            len = (*(ep + 3) == '6') ? 4 : 3;
            if (*(ep + len) || ((cp + len) >= ep) || strncmp(cp, ep, len))
#endif /* solaris<80000 */

                continue;
            for (j = 0; j < NTCPUDP; j++) {
                if (!tcpudp[j].ds && !strcmp(ep, tcpudp[j].proto)) {
                    tcpudp[j].ds = 1;
                    tcpudp[j].maj = (major_t)GET_MAJ_DEV(Devtp[i].rdev);
                    tcpudp[j].min = (minor_t)GET_MIN_DEV(Devtp[i].rdev);
                    n++;
                    break;
                }
            }
        }
        tcpudps = n ? 1 : -1;
    }
    /*
     * Check for known IPv[46] TCP or UDP device.
     */
    for (i = 0; (i < NTCPUDP) && (tcpudps > 0); i++) {
        if (tcpudp[i].ds

#if solaris < 80000
            && (maj == tcpudp[i].min)
#else  /* solaris>=80000 */
            && (maj == tcpudp[i].maj)
#endif /* solaris<80000 */

        ) {
            process_socket(ctx, (KA_T)v->v_stream, tcpudp[i].proto);
            return (1);
        }
    }
    return (0);
}

/*
 * isvlocked() - is Solaris vnode locked?
 */

static enum lsof_lock_mode isvlocked(struct lsof_context *ctx, /* context */
                                     struct vnode *va) /* local vnode address */
{

#if solaris < 20500
    struct filock f;
    KA_T ff;
    KA_T fp;
#endif /* solaris<20500 */

    int i, l;

#if solaris >= 20300
    struct lock_descriptor ld;
    KA_T lf;
    KA_T lp;
#    if solaris < 20500
#        define LOCK_END ld.info.li_sleep.sli_flock.l_len
#        define LOCK_FLAGS ld.flags
#        define LOCK_NEXT ld.next
#        define LOCK_OWNER ld.owner.pid
#        define LOCK_START ld.start
#        define LOCK_TYPE ld.type
#    else /* solaris>=20500 */
#        define LOCK_END ld.l_flock.l_len
#        define LOCK_FLAGS ld.l_state
#        define LOCK_NEXT ld.l_next
#        define LOCK_OWNER ld.l_flock.l_pid
#        define LOCK_START ld.l_start
#        define LOCK_TYPE ld.l_type
#    endif /* solaris<20500 */
#endif     /* solaris>=20300 */

    if (va->v_filocks == NULL)
        return LSOF_LOCK_NONE;

#if solaris < 20500
#    if solaris > 20300 ||                                                     \
        (solaris == 20300 && defined(P101318) && P101318 >= 45)
    if (Ntype == N_NFS)
#    endif /* solaris>20300 || (solaris==20300 && defined(P101318) &&          \
              P101318>=45) */

    {
        ff = fp = (KA_T)va->v_filocks;
        i = 0;
        do {
            if (kread(ctx, fp, (char *)&f, sizeof(f)))
                return LSOF_LOCK_NONE;
            i++;
            if (f.set.l_pid != (pid_t)Lp->pid)
                continue;
            if (f.set.l_whence == 0 && f.set.l_start == 0 &&
                f.set.l_len == MAXEND)
                l = 1;
            else
                l = 0;
            switch (f.set.l_type & (F_RDLCK | F_WRLCK)) {
            case F_RDLCK:
                return l ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
            case F_WRLCK:
                return l ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
            case F_RDLCK | F_WRLCK:
                return LSOF_LOCK_READ_WRITE;
            default:
                return LSOF_LOCK_SOLARIS_NFS;
            }
        } while ((fp = (KA_T)f.next) && (fp != ff) && (i < 10000));
    }
#endif /* solaris<20500 */

#if solaris >= 20300
    lf = lp = (KA_T)va->v_filocks;
    i = 0;
    do {
        if (kread(ctx, lp, (char *)&ld, sizeof(ld)))
            return LSOF_LOCK_NONE;
        i++;
        if (!(LOCK_FLAGS & ACTIVE_LOCK) || LOCK_OWNER != (pid_t)Lp->pid)
            continue;
        if (LOCK_START == 0 && (LOCK_END == 0

#    if solaris < 20500
                                || LOCK_END == MAXEND
#    else  /* solaris>=20500 */
                                || LOCK_END == MAXEND
#    endif /* solaris<20500 */

                                ))
            l = 1;
        else
            l = 0;
        switch (LOCK_TYPE) {
        case F_RDLCK:
            return l ? LSOF_LOCK_READ_FULL : LSOF_LOCK_READ_PARTIAL;
        case F_WRLCK:
            return l ? LSOF_LOCK_WRITE_FULL : LSOF_LOCK_WRITE_PARTIAL;
        case (F_RDLCK | F_WRLCK):
            return LSOF_LOCK_READ_WRITE;
        default:
            /* It was 'L' since 1997, dunno what is it */
            return LSOF_LOCK_UNKNOWN;
        }
    } while ((lp = (KA_T)LOCK_NEXT) && (lp != lf) && (i < 10000));
    return LSOF_LOCK_NONE;
#endif /* solaris>=20300 */
}

/*
 * finddev() - look up device by device number
 */

static struct l_dev *finddev(struct lsof_context *ctx, /* context */
                             dev_t *dev,               /* device */
                             dev_t *rdev,              /* raw device */
                             int flags) /* look flags -- see LOOKDEV_* symbol
                                         * definitions */
{
    struct clone *c;
    struct l_dev *dp;
    struct pseudo *p;

    if (!Sdev)
        readdev(ctx, 0);
        /*
         * Search device table for match.
         */

#if defined(HASDCACHE)

finddev_again:

#endif /* defined(HASDCACHE) */

    if (flags & LOOKDEV_TAB) {
        if ((dp = lkupdev(ctx, dev, rdev, 0, 0)))
            return (dp);
    }
    /*
     * Search for clone.
     */
    if ((flags & LOOKDEV_CLONE) && Clone) {
        for (c = Clone; c; c = c->next) {
            if (GET_MAJ_DEV(*rdev) == GET_MIN_DEV(c->cd.rdev)) {

#if defined(HASDCACHE)
                if (DCunsafe && !c->cd.v && !vfy_dev(ctx, &c->cd))
                    goto finddev_again;
#endif /* defined(HASDCACHE) */

                return (&c->cd);
            }
        }
    }
    /*
     * Search for pseudo device match on major device only.
     */
    if ((flags & LOOKDEV_PSEUDO) && Pseudo) {
        for (p = Pseudo; p; p = p->next) {
            if (GET_MAJ_DEV(*rdev) == GET_MAJ_DEV(p->pd.rdev)) {

#if defined(HASDCACHE)
                if (DCunsafe && !p->pd.v && !vfy_dev(ctx, &p->pd))
                    goto finddev_again;
#endif /* defined(HASDCACHE) */

                return (&p->pd);
            }
        }
    }
    return ((struct l_dev *)NULL);
}

#if solaris >= 20500
/*
 * idoorkeep() -- identify door keeper process
 */

static int idoorkeep(struct lsof_context *ctx, /* context */
                     struct door_node *d)      /* door's node */
{
    char buf[1024];
    size_t bufl = sizeof(buf);
    struct proc dp;
    struct pid dpid;
    /*
     * Get the proc structure and its pid structure for the door target.
     */
    if (!d->door_target ||
        kread(ctx, (KA_T)d->door_target, (char *)&dp, sizeof(dp)))
        return (0);
    if (!dp.p_pidp || kread(ctx, (KA_T)dp.p_pidp, (char *)&dpid, sizeof(dpid)))
        return (0);
    /*
     * Form a description of the door.
     *
     * Put the description in the NAME column addition field.  If there's
     * already something there, allocate more space and add the door description
     * to it.
     */
    if (Lp->pid == (int)dpid.pid_id)
        (void)snpf(buf, bufl, "(this PID's door)");
    else {
        (void)snpf(buf, bufl, "(door to %.64s[%ld])", dp.p_user.u_comm,
                   (long)dpid.pid_id);
    }
    (void)add_nma(ctx, buf, (int)strlen(buf));
    return (1);
}
#endif /* solaris>=20500 */

/*
 * process_node() - process vnode
 */

void process_node(struct lsof_context *ctx, /* context */
                  KA_T va)                  /* vnode kernel space address */
{

#if defined(HASCACHEFS)
    struct cnode cn;
#endif /* defined(HASCACHEFS) */

    dev_t dev, rdev, trdev;
    unsigned char devs = 0;
    unsigned char fxs = 0;
    unsigned char ins = 0;
    unsigned char kvs = 0;
    unsigned char nns = 0;
    unsigned char pnl = 0;
    unsigned char rdevs = 0;
    unsigned char rvs = 0;
    unsigned char rfxs = 0;
    unsigned char sdns = 0;
    unsigned char tdef;
    unsigned char trdevs = 0;
    unsigned char unix_sock = 0;
    struct dev_info di;
    char din[DINAMEL];
    char *ep;
    struct fifonode f;
    char *fa = (char *)NULL;
    int fal;
    static int ft = 1;
    struct vnode fv, rv;
    int fx, rfx;
    struct hsnode h;
    struct inode i;
    int j;
    KA_T ka, vka;
    struct lnode lo;
    struct vfs kv, rkv;
    int len, llc, nl, snl, sepl;
    struct mvfsnode m;
    struct namenode nn;
    struct l_vfs *nvfs, *vfs;
    struct pcnode pc;
    struct pcfs pcfs;
    struct rnode r;
    KA_T realvp = (KA_T)NULL;
    struct snode rs;
    struct snode s;
    char fd[FDLEN];

#if solaris >= 110000
    char *nm, *sep;
    size_t nmrl, tl;
    struct sdev_node sdn;
    struct vattr sdva;
    sotpi_info_t sti;
    int stis = 0;
#endif /* solaris>=110000 */

    struct l_dev *sdp = (struct l_dev *)NULL;
    size_t sz;
    struct tmpnode t;
    char tbuf[128], *ty, ubuf[128];
    int tbufx;
    enum vtype type;
    struct sockaddr_un ua;
    static struct vnode *v = (struct vnode *)NULL;
    KA_T vs;
    int vty = 0;
    int vty_tmp;

#if solaris >= 20500
#    if solaris >= 20600
    struct fnnode fnn;
    struct pairaddr {
        short f;
        unsigned short p;
    } * pa;
    KA_T peer;
    struct sonode so;
    KA_T soa, sona;
#    else  /* solaris<20600 */
    struct autonode au;
#    endif /* solaris>=20600 */

    struct door_node dn;
    int dns = 0;
#endif /* solaris >=20500 */

#if solaris < 100000
    KA_T so_ad[2];
    struct so_so soso;
    int so_st = 0;
#else  /* solaris>=100000 */
    union {
        ctfs_adirnode_t adir;
        ctfs_bunode_t bun;
        ctfs_cdirnode_t cdir;
        ctfs_ctlnode_t ctl;
        ctfs_evnode_t ev;
        ctfs_latenode_t late;
        ctfs_rootnode_t root;
        ctfs_symnode_t sym;
        ctfs_tdirnode_t tdir;
        ctfs_tmplnode_t tmpl;
    } ctfs;
    dev_t dv_dev;
    struct dv_node dv;
    unsigned char dv_devs = 0;
    unsigned char dvs = 0;
    port_t pn;
    struct rnode4 r4;
#endif /* solaris<100000 */

#if defined(HASPROCFS)
    struct procfsid *pfi;
    struct pid pids;
#endif /* defined(HASPROCFS) */

#if defined(HAS_AFS)
    struct afsnode an;
#endif /* defined(HAS_AFS) */

#if defined(HASVXFS)
    struct l_ino vx;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
    vfs_t zgvfs;
    unsigned char zns = 0;
    znode_t zn;
    zfsvfs_t zvfs;
#endif /* defined(HAS_ZFS) */

    /*
     * Do first-time only operations.
     */

#if solaris < 100000
    so_ad[0] = so_ad[1] = (KA_T)0;
#endif /* solaris<100000 */

    if (ft) {
        (void)build_Voptab(ctx);
        ft = 0;
    }
    /*
     * Read the vnode.
     */
    if (!va) {
        enter_nm(ctx, "no vnode address");
        return;
    }
    if (!v) {

        /*
         * Allocate space for the vnode or AFS vcache structure.
         */

#if defined(HAS_AFS)
        v = alloc_vcache();
#else  /* !defined(HAS_AFS) */
        v = (struct vnode *)malloc(sizeof(struct vnode));
#endif /* defined(HAS_AFS) */

        if (!v) {
            (void)fprintf(stderr, "%s: can't allocate %s space\n", Pn,

#if defined(HAS_AFS)
                          "vcache"
#else  /* !defined(HAS_AFS) */
                          "vnode"
#endif /* defined(HAS_AFS) */

            );
            Error(ctx);
        }
    }
    if (readvnode(ctx, va, v)) {
        enter_nm(ctx, Namech);
        return;
    }

#if defined(HASNCACHE)
    Lf->na = va;
#endif /* defined(HASNCACHE) */

#if defined(HASFSTRUCT)
    Lf->fna = va;
    Lf->fsv |= FSV_NI;
#endif /* defined(HASFSTRUCT) */

#if defined(HASLFILEADD) && defined(HAS_V_PATH)
    Lf->V_path = (KA_T)v->v_path;
#endif /* defined(HASLFILEADD) && defined(HAS_V_PATH) */

    vs = (KA_T)v->v_stream;
    /*
     * Check for a Solaris socket.
     */
    if (is_socket(ctx, v))
        return;
    /*
     * Obtain the Solaris virtual file system structure.
     */
    if ((ka = (KA_T)v->v_vfsp)) {
        if (kread(ctx, ka, (char *)&kv, sizeof(kv))) {
            vka = va;

        vfs_read_error:

            (void)snpf(Namech, Namechl - 1, "vnode at %s: can't read vfs: %s",
                       print_kptr(vka, tbuf, sizeof(tbuf)),
                       print_kptr(ka, (char *)NULL, 0));
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            return;
        }
        kvs = 1;
    } else
        kvs = 0;
    /*
     * Derive the virtual file system structure's device number from
     * its file system ID for NFS and High Sierra file systems.
     */
    if (kvs && ((fx = kv.vfs_fstype - 1) >= 0) && (fx < Fsinfomax)) {
        fxs = 1;
        if (strcmp(Fsinfo[fx], "nfs") == 0 || strcmp(Fsinfo[fx], "nfs3") == 0 ||
            strcmp(Fsinfo[fx], "hsfs") == 0)
            kv.vfs_dev = (dev_t)kv.vfs_fsid.val[0];
    } else {
        fx = -1;
        fxs = 0;
    }
    /*
     * Determine the Solaris vnode type.
     */
    if ((Ntype = vop2ty(ctx, v, fx)) < 0) {
        if (v->v_type == VFIFO) {
            vty = N_REGLR;
            Ntype = N_FIFO;
        } else if (vs) {
            Ntype = vty = N_STREAM;
            Lf->is_stream = 1;
        }
        if (Ntype < 0) {
            (void)snpf(Namech, Namechl - 1,
                       "unknown file system type%s%s%s, v_op: %s",
                       fxs ? " (" : "", fxs ? Fsinfo[fx] : "", fxs ? ")" : "",
                       print_kptr((KA_T)v->v_op, (char *)NULL, 0));
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            return;
        }
    } else {
        vty = Ntype;
        if (v->v_type == VFIFO)
            Ntype = N_FIFO;
        else if (vs && Ntype != N_SOCK) {
            Ntype = vty = N_STREAM;
            Lf->is_stream = 1;
        }
    }
    /*
     * See if this Solaris node has been fattach'ed to another node.
     * If it has, read the namenode, and enter the node addresses in
     * the NAME column addition.
     *
     * See if it's covering a socket as well and process accordingly.
     */
    if (vty == N_NM) {
        if (read_nnn(ctx, va, (KA_T)v->v_data, &nn))
            return;
        nns = 1;
        if (nn.nm_mountpt)

#if solaris >= 20500
            fa = ent_fa(
                (KA_T *)((Ntype == N_FIFO || v->v_type == VDOOR) ? NULL : &va),
                (KA_T *)&nn.nm_mountpt, "->", &fal);
#else  /* solaris<20500 */
            fa = ent_fa((KA_T *)((Ntype == N_FIFO) ? NULL : &va),
                        (KA_T *)&nn.nm_mountpt, "->", &fal);
#endif /* solaris>=20500 */

        if (Ntype != N_FIFO && nn.nm_filevp &&
            !kread(ctx, (KA_T)nn.nm_filevp, (char *)&rv, sizeof(rv))) {
            rvs = 1;
            if ((ka = (KA_T)rv.v_vfsp) &&
                !kread(ctx, ka, (char *)&rkv, sizeof(rkv)) &&
                ((rfx = rkv.vfs_fstype - 1) >= 0) && (rfx < Fsinfomax)) {
                rfxs = 1;
            } else {
                rfx = fx;
                rfxs = fxs;
            }

#if defined(HASNCACHE)
            Lf->na = (KA_T)nn.nm_filevp;
#endif /* defined(HASNCACHE) */

            if (is_socket(ctx, &rv))
                return;
        }
    }
    if (Selinet && Ntype != N_SOCK)
        return;
    /*
     * See if this Solaris node is served by spec_vnodeops.
     */
    if (Spvops && Spvops == (KA_T)v->v_op)
        Ntype = N_SPEC;
    /*
     * Determine the Solaris lock state.
     */
    Lf->lock = isvlocked(ctx, v);
    /*
     * Establish the Solaris local virtual file system structure.
     */
    if (!(ka = (KA_T)v->v_vfsp) || !kvs)
        vfs = (struct l_vfs *)NULL;
    else if (!(vfs = readvfs(ctx, ka, &kv, v))) {
        vka = va;
        goto vfs_read_error;
    }
    /*
     * Read the afsnode, autonode, cnode, door_node, fifonode, fnnode, lnode,
     * inode, pcnode, rnode, snode, tmpnode, znode, etc.
     */
    switch (Ntype) {
    case N_SPEC:

        /*
         * A N_SPEC node is a node that resides in in an underlying file system
         * type -- e.g. NFS, HSFS.  Its vnode points to an snode.  Subsequent
         * node structures are implied by the underlying node type.
         */
        if (read_nsn(ctx, va, (KA_T)v->v_data, &s))
            return;
        realvp = (KA_T)s.s_realvp;
        if (!realvp && s.s_commonvp) {
            if (read_cni(ctx, &s, &rv, v, &rs, &di, din, sizeof(din)) == 1)
                return;
            if (!rv.v_stream) {
                if (din[0]) {
                    (void)snpf(Namech, Namechl, "COMMON: %s", din);
                    Namech[Namechl - 1] = '\0';
                    Lf->is_com = 1;
                }
                break;
            }
        }
        if (!realvp) {

            /*
             * If the snode lacks a real vnode (and also lacks a common vnode),
             * it's original type is N_STREAM or N_REGLR, and it has a stream
             * pointer, get the module names.
             */
            if ((vty == N_STREAM || vty == N_REGLR) && vs) {
                Lf->is_stream = 1;
                vty = N_STREAM;

#if solaris < 100000
                read_mi(ctx, vs, (dev_t *)&s.s_dev, (caddr_t)&soso, &so_st,
                        so_ad, &sdp);
#else  /* solaris>=100000 */
                read_mi(ctx, vs, (dev_t *)&s.s_dev, NULL, NULL, NULL, &sdp);
#endif /* solaris<100000 */

                vs = (KA_T)NULL;
            }
        }
        break;

#if defined(HAS_AFS)
    case N_AFS:
        if (readafsnode(ctx, va, v, &an))
            return;
        break;
#endif /* defined(HAS_AFS) */

#if solaris >= 20500
    case N_AUTO:

#    if solaris < 20600
        if (read_nan(ctx, va, (KA_T)v->v_data, &au))
#    else  /* solaris>=20600 */
        if (read_nan(ctx, va, (KA_T)v->v_data, &fnn))
#    endif /* solaris<20600 */

            return;
        break;

#    if solaris >= 100000
    case N_DEV:
        if (read_ndvn(ctx, va, (KA_T)v->v_data, &dv, &dv_dev, &dv_devs))
            return;
        dvs = 1;
        break;
#    endif /* solaris>=100000 */

    case N_DOOR:
        if (read_ndn(ctx, va, (KA_T)v->v_data, &dn))
            return;
        dns = 1;
        break;
#endif /* solaris>=20500 */

#if defined(HASCACHEFS)
    case N_CACHE:
        if (read_ncn(ctx, va, (KA_T)v->v_data, &cn))
            return;
        break;
#endif /* defined(HASCACHEFS) */

#if solaris >= 100000
    case N_CTFSADIR:
    case N_CTFSBUND:
    case N_CTFSCDIR:
    case N_CTFSCTL:
    case N_CTFSEVT:
    case N_CTFSLATE:
    case N_CTFSROOT:
    case N_CTFSSTAT:
    case N_CTFSSYM:
    case N_CTFSTDIR:
    case N_CTFSTMPL:
        if (read_nctfsn(ctx, Ntype, va, (KA_T)v->v_data, (char *)&ctfs))
            return;
        break;
#endif /* solaris>=100000 */

#if solaris >= 20600
    case N_SOCK:
        sona = (KA_T)v->v_data;
        if (read_nson(ctx, va, sona, &so))
            return;
        break;
#endif /* solaris>=20600 */

    case N_MNT:
        /* Information comes from the l_vfs structure. */
        break;
    case N_MVFS:
        if (read_nmn(ctx, va, (KA_T)v->v_data, &m))
            return;
        break;
    case N_NFS:
        if (read_nrn(ctx, va, (KA_T)v->v_data, &r))
            return;
        break;

#if solaris >= 100000
    case N_NFS4:
        if (read_nrn4(ctx, va, (KA_T)v->v_data, &r4))
            return;
        break;
#endif /* solaris>=100000 */

    case N_NM:
        if (nns)
            realvp = (KA_T)nn.nm_filevp;

#if defined(HASNCACHE)
        Lf->na = (KA_T)nn.nm_filevp;
#endif /* defined(HASNCACHE) */

        break;
    case N_FD:
        break; /* no successor node */
    case N_FIFO:

        /*
         * Solaris FIFO vnodes are usually linked to a fifonode.  One
         * exception is a FIFO vnode served by nm_vnodeops; it is linked
         * to a namenode, and the namenode points to the fifonode.
         *
         * Non-pipe fifonodes are linked to a vnode thorough fn_realvp.
         */
        if (vty == N_NM && nns) {
            if (nn.nm_filevp) {
                if (read_nfn(ctx, va, (KA_T)nn.nm_filevp, &f))
                    return;
                realvp = (KA_T)NULL;
                vty = N_FIFO;
            } else {
                (void)snpf(Namech, Namechl - 1,
                           "FIFO namenode at %s: no fifonode pointer",
                           print_kptr((KA_T)v->v_data, (char *)NULL, 0));
                Namech[Namechl - 1] = '\0';
                return;
            }
        } else {
            if (read_nfn(ctx, va, (KA_T)v->v_data, &f))
                return;
            realvp = (KA_T)f.fn_realvp;
        }
        if (!realvp) {
            Lf->inode = (INODETYPE)(nns ? nn.nm_vattr.va_nodeid : f.fn_ino);

#if solaris >= 80000 /* Solaris 8 and above hack! */
#    if defined(_LP64)
            if (Lf->inode >= (unsigned long)0xbaddcafebaddcafe)
#    else  /* !defined(_LP64) */
            if (Lf->inode >= (unsigned long)0xbaddcafe)
#    endif /* defined(_LP64) */

                Lf->inp_ty = 0;
            else
#endif /* solaris>=80000 Solaris 8 and above hack! */

                Lf->inp_ty = 1;
            enter_dev_ch(ctx, print_kptr((KA_T)v->v_data, (char *)NULL, 0));
            if (f.fn_flag & ISPIPE) {
                (void)snpf(tbuf, sizeof(tbuf), "PIPE");
                tbufx = (int)strlen(tbuf);
            } else
                tbufx = 0;

#if solaris < 20500
            if (f.fn_mate) {
                (void)snpf(&tbuf[tbufx], sizeof(tbuf) - tbufx, "->%s",
                           print_kptr((KA_T)f.fn_mate, (char *)NULL, 0));
                tbufx = (int)strlen(tbuf);
            }
#else  /* solaris>=20500 */
            if (f.fn_dest) {
                (void)snpf(&tbuf[tbufx], sizeof(tbuf) - tbufx, "->%s",
                           print_kptr((KA_T)f.fn_dest, (char *)NULL, 0));
                tbufx = (int)strlen(tbuf);
            }
#endif /* solaris<20500 */

            if (tbufx)
                (void)add_nma(ctx, tbuf, tbufx);
            break;
        }
        break;

    case N_HSFS:
        if (read_nhn(ctx, va, (KA_T)v->v_data, &h))
            return;
        break;
    case N_LOFS:
        llc = 0;
        do {
            rvs = 0;
            if (read_nln(ctx, va, llc ? (KA_T)rv.v_data : (KA_T)v->v_data,
                         &lo)) {
                return;
            }
            if (!(realvp = (KA_T)lo.lo_vp)) {
                (void)snpf(Namech, Namechl - 1, "lnode at %s: no real vnode",
                           print_kptr((KA_T)v->v_data, (char *)NULL, 0));
                Namech[Namechl - 1] = '\0';
                enter_nm(ctx, Namech);
                return;
            }
            if (read_nvn(ctx, (KA_T)v->v_data, (KA_T)realvp, &rv))
                return;
            rvs = 1;
            llc++;
            if ((ka = (KA_T)rv.v_vfsp) &&
                !kread(ctx, ka, (char *)&rkv, sizeof(rkv)) &&
                ((rfx = rkv.vfs_fstype - 1) >= 0) && (rfx < Fsinfomax)) {
                rfxs = 1;
            } else {
                rfx = fx;
                rfxs = fxs;
            }
            if (((vty_tmp = vop2ty(ctx, &rv, rfx)) == N_LOFS) && (llc > 1000)) {
                (void)snpf(Namech, Namechl - 1, "lnode at %s: loop > 1000",
                           print_kptr((KA_T)v->v_data, (char *)NULL, 0));
                Namech[Namechl - 1] = '\0';
                enter_nm(ctx, Namech);
                return;
            }
        } while (vty_tmp == N_LOFS);
        break;
    case N_PCFS:
        if (read_npn(ctx, va, (KA_T)v->v_data, &pc))
            return;
        break;

#if solaris >= 100000
    case N_PORT:
        if (read_nprtn(ctx, va, (KA_T)v->v_data, &pn))
            return;
        break;
#endif /* solaris>=100000 */

#if defined(HASPROCFS)
    case N_PROC:
        if (read_npi(ctx, va, v, &pids))
            return;
        break;
#endif /* defined(HASPROCFS) */

#if solaris >= 110000
    case N_SDEV:
        if (read_nsdn(ctx, va, (KA_T)v->v_data, &sdn, &sdva))
            return;
        sdns = 1;
        break;
#endif /* solaris>=110000 */

    case N_SAMFS:
        (void)add_nma(ctx, SAMFS_NMA_MSG, (int)strlen(SAMFS_NMA_MSG));
        break;
    case N_SHARED:
        break; /* No more sharedfs information is available. */
    case N_STREAM:
        if (read_nsn(ctx, va, (KA_T)v->v_data, &s))
            return;
        if (vs) {
            Lf->is_stream = 1;

#if solaris < 100000
            read_mi(ctx, vs, (dev_t *)&s.s_dev, (caddr_t)&soso, &so_st, so_ad,
                    &sdp);
#else  /* solaris>=100000 */
            read_mi(ctx, vs, (dev_t *)&s.s_dev, NULL, NULL, NULL, &sdp);
#endif /* solaris<100000 */

            vs = (KA_T)NULL;
        }
        break;
    case N_TMP:
        if (read_ntn(ctx, va, (KA_T)v->v_data, &t))
            return;
        break;

#if defined(HASVXFS)
    case N_VXFS:
        if (read_vxnode(ctx, va, v, vfs, fx, &vx, Vvops))
            return;
        break;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
    case N_ZFS:
        if (read_nzn(ctx, va, (KA_T)v->v_data, &zn))
            return;
        zns = 1;
        break;
#endif /* defined(HAS_ZFS) */

    case N_REGLR:
    default:
        if (read_nin(ctx, va, (KA_T)v->v_data, &i))
            return;
        ins = 1;
    }
    /*
     * If the node has a real vnode pointer, follow it.
     */
    if (realvp) {
        if (rvs) {
            *v = rv;
            fx = rfx;
            fxs = rfxs;
        } else {
            if (read_nvn(ctx, (KA_T)v->v_data, (KA_T)realvp, v))
                return;
            else {

#if defined(HASNCACHE)
                Lf->na = (KA_T)realvp;
#endif /* defined(HASNCACHE) */

                if ((ka = (KA_T)v->v_vfsp) &&
                    !kread(ctx, ka, (char *)&kv, sizeof(kv))) {
                    kvs = 1;
                }
                if (kvs && ((fx = kv.vfs_fstype - 1) >= 0) &&
                    (fx < Fsinfomax)) {
                    fxs = 1;
                }
            }
        }
        /*
         * If the original vnode type is N_STREAM, if there is a stream
         * pointer and if there is no sdev_node, get the module names.
         */
        if (vty == N_STREAM && vs && !sdns) {
            Lf->is_stream = 1;

#if solaris < 100000
            read_mi(ctx, vs, (dev_t *)&s.s_dev, (caddr_t)&soso, &so_st, so_ad,
                    &sdp);
#else  /* solaris>=100000 */
            read_mi(ctx, vs, (dev_t *)&s.s_dev, NULL, NULL, NULL, &sdp);
#endif /* solaris<100000 */

            vs = (KA_T)NULL;
        }
        /*
         * Get the real vnode's type.
         */
        if ((vty = vop2ty(ctx, v, fx)) < 0) {
            if (Ntype != N_FIFO && vs)
                vty = N_STREAM;
            else {

#if solaris < 100000
                (void)snpf(Namech, Namechl - 1,
                           "unknown file system type, v_op: %s",
                           print_kptr((KA_T)v->v_op, (char *)NULL, 0));
#else  /* solaris>=100000 */
                (void)snpf(Namech, Namechl - 1,
                           "unknown file system type (%s), v_op: %s",
                           fxs ? Fsinfo[fx] : "unknown",
                           print_kptr((KA_T)v->v_op, (char *)NULL, 0));
#endif /* solaris<100000 */

                Namech[Namechl - 1] = '\0';
            }
        }
        if (Ntype == N_NM || Ntype == N_AFS)
            Ntype = vty;
        /*
         * Base further processing on the "real" vnode.
         */
        Lf->lock = isvlocked(ctx, v);
        switch (vty) {

#if defined(HAS_AFS)
        case N_AFS:
            if (readafsnode(ctx, va, v, &an))
                return;
            break;
#endif /* defined(HAS_AFS) */

#if solaris >= 20500
        case N_AUTO:

#    if solaris < 20600
            if (read_nan(ctx, va, (KA_T)v->v_data, &au))
#    else  /* solaris>=20600 */
            if (read_nan(ctx, va, (KA_T)v->v_data, &fnn))
#    endif /* solaris<20600 */

                return;
            break;

#    if solaris >= 100000
        case N_DEV:
            if (read_ndvn(ctx, va, (KA_T)v->v_data, &dv, &dv_dev, &dv_devs))
                return;
            dvs = 1;
            break;
#    endif /* solaris>=100000 */

        case N_DOOR:

#    if solaris < 20600
            if (read_ndn(ctx, realvp, (KA_T)v->v_data, &dn))
#    else  /* solaris>=20600 */
            if (read_ndn(ctx, va, (KA_T)v->v_data, &dn))
#    endif /* solaris<20500 */

                return;
            dns = 1;
            break;
#endif /* solaris>=20500 */

#if defined(HASCACHEFS)
        case N_CACHE:
            if (read_ncn(ctx, va, (KA_T)v->v_data, &cn))
                return;
            break;
#endif /* defined(HASCACHEFS) */

#if solaris >= 100000
        case N_CTFSADIR:
        case N_CTFSBUND:
        case N_CTFSCDIR:
        case N_CTFSCTL:
        case N_CTFSEVT:
        case N_CTFSLATE:
        case N_CTFSROOT:
        case N_CTFSSTAT:
        case N_CTFSSYM:
        case N_CTFSTDIR:
        case N_CTFSTMPL:
            if (read_nctfsn(ctx, vty, va, (KA_T)v->v_data, (char *)&ctfs))
                return;
            break;
#endif /* solaris>=100000 */

        case N_HSFS:
            if (read_nhn(ctx, va, (KA_T)v->v_data, &h))
                return;
            break;
        case N_MNT:
            /* Information comes from the l_vfs structure. */
            break;
        case N_MVFS:
            if (read_nmn(ctx, va, (KA_T)v->v_data, &m))
                return;
            break;
        case N_NFS:
            if (read_nrn(ctx, va, (KA_T)v->v_data, &r))
                return;
            break;

#if solaris >= 100000
        case N_NFS4:
            if (read_nrn4(ctx, va, (KA_T)v->v_data, &r4))
                return;
            break;
#endif /* solaris>=100000 */

        case N_NM:
            if (read_nnn(ctx, va, (KA_T)v->v_data, &nn))
                return;
            nns = 1;
            break;

#if solaris >= 100000
        case N_PORT:
            if (read_nprtn(ctx, va, (KA_T)v->v_data, &pn))
                return;
            break;
#endif /* solaris>=100000 */

        case N_PCFS:
            if (read_npn(ctx, va, (KA_T)v->v_data, &pc))
                return;
            break;
        case N_SAMFS:
            (void)add_nma(ctx, SAMFS_NMA_MSG, (int)strlen(SAMFS_NMA_MSG));

#if solaris >= 110000
        case N_SDEV:
            if (read_nsdn(ctx, va, (KA_T)v->v_data, &sdn, &sdva))
                return;
            if (Lf->is_stream) {

                /*
                 * This stream's real node is an sdev_node, so it's not really
                 * a stream.  Reverse prior stream settings.
                 */
                Lf->is_stream = 0;
                Namech[0] = '\0';
            }
            sdns = 1;
            break;
#endif /* solaris>=110000 */

            break;

#if solaris >= 20600
        case N_SOCK:
            sona = (KA_T)v->v_data;
            if (read_nson(ctx, va, sona, &so))
                return;
            break;
#endif /* solaris>=20600 */

        case N_STREAM:
            if (vs) {
                Lf->is_stream = 1;

#if solaris < 100000
                read_mi(ctx, vs, (dev_t *)&s.s_dev, (caddr_t)&soso, &so_st,
                        so_ad, &sdp);
#else  /* solaris>=100000 */
                read_mi(ctx, vs, (dev_t *)&s.s_dev, NULL, NULL, NULL, &sdp);
#endif /* solaris<100000 */

                vs = (KA_T)NULL;
            }
            break;
        case N_TMP:
            if (read_ntn(ctx, va, (KA_T)v->v_data, &t))
                return;
            break;

#if defined(HASVXFS)
        case N_VXFS:
            if (read_vxnode(ctx, va, v, vfs, fx, &vx, Vvops))
                return;
            break;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
        case N_ZFS:
            if (read_nzn(ctx, va, (KA_T)v->v_data, &zn))
                return;
            zns = 1;
            break;
#endif /* defined(HAS_ZFS) */

        case N_REGLR:
        default:
            if (read_nin(ctx, va, (KA_T)v->v_data, &i))
                return;
            ins = 1;
        }
        /*
         * If this is a Solaris loopback node, use the "real" node type.
         */
        if (Ntype == N_LOFS)
            Ntype = vty;
    }
    /*
     * Get device and type for printing.
     */
    switch (((Ntype == N_FIFO) || (vty == N_SDEV)) ? vty : Ntype) {

#if defined(HAS_AFS)
    case N_AFS:
        dev = an.dev;
        devs = 1;
        break;
#endif /* defined(HAS_AFS) */

#if solaris >= 20500
    case N_AUTO:
        if (kvs) {
            dev = (dev_t)kv.vfs_fsid.val[0];
            devs = 1;
        }
        break;

#    if solaris >= 100000
    case N_DEV:
        if (dv_devs) {
            dev = dv_dev;
            devs = 1;
        } else if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
        rdev = v->v_rdev;
        rdevs = 1;
        break;
#    endif /* solaris>=100000 */

    case N_DOOR:

#    if solaris < 20600
        if (kvs) {
            dev = (dev_t)kv.vfs_fsid.val[0];
            devs = 1;
        }
#    else  /* solaris>=20600 */
        if (nns) {
            dev = (dev_t)nn.nm_vattr.va_fsid;
            devs = 1;
        } else if (dns) {
            dev = (dev_t)dn.door_index;
            devs = 1;
        }
#    endif /* solaris<20600 */

        break;
#endif /* solaris>=20500 */

#if defined(HASCACHEFS)
    case N_CACHE:
#endif /* defined(HASCACHEFS) */

    case N_HSFS:
    case N_PCFS:
        if (kvs) {
            dev = kv.vfs_dev;
            devs = 1;
        }
        break;

#if solaris >= 100000
    case N_CTFSADIR:
    case N_CTFSBUND:
    case N_CTFSCDIR:
    case N_CTFSCTL:
    case N_CTFSEVT:
    case N_CTFSLATE:
    case N_CTFSROOT:
    case N_CTFSSTAT:
    case N_CTFSSYM:
    case N_CTFSTDIR:
    case N_CTFSTMPL:
        if (kvs) {
            dev = kv.vfs_dev;
            devs = 1;
        }
        break;
#endif /* solaris>=100000 */

    case N_FD:
        if (kvs) {
            dev = kv.vfs_dev;
            devs = 1;
        }
        if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
            rdev = v->v_rdev;
            rdevs = 1;
        }
        break;

    case N_MNT:

#if defined(CVFS_DEVSAVE)
        if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
#endif /* defined(CVFS_DEVSAVE) */

        break;
    case N_MVFS:

#if defined(CVFS_DEVSAVE)
        if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
#endif /* defined(CVFS_DEVSAVE) */

        break;
    case N_NFS:
        dev = r.r_attr.va_fsid;
        devs = 1;
        break;

#if solaris >= 100000
    case N_NFS4:
        dev = r4.r_attr.va_fsid;
        devs = 1;
        break;
#endif /* solaris>=100000 */

    case N_NM:
        if (nns) {
            dev = (dev_t)nn.nm_vattr.va_fsid;
            devs = 1;
        } else
            enter_dev_ch(ctx, "    NMFS");
        break;

#if solaris >= 100000
    case N_PORT:
        if (kvs) {
            dev = kv.vfs_dev;
            devs = 1;
        }
        break;
#endif /* solaris>=100000 */

#if defined(HASPROCFS)
    case N_PROC:
        if (kvs) {
            dev = kv.vfs_dev;
            devs = 1;
        }
        break;
#endif /* defined(HASPROCFS) */

    case N_SAMFS:
        if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
            rdev = v->v_rdev;
            rdevs = 1;
        } else if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
        break;

#if solaris >= 110000
    case N_SDEV:
        if (sdns) {
            if (v->v_type == VDIR) {
                dev = v->v_rdev;
                devs = 1;
            } else {
                rdev = v->v_rdev;
                rdevs = 1;
            }
        }
        break;
#endif /* solaris>=110000 */

    case N_SHARED:
        if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
        break;

#if solaris >= 20600
    case N_SOCK:
        if (so.so_family == AF_UNIX)

        /*
         * Process an AF_UNIX socket node.
         */

#    if solaris >= 110000
        {

            /*
             * Process a Solaris >= 11 AF_UNIX socket node:
             *
             * Get its sotpi_info_t structure;
             */
            if (read_nsti(ctx, &so, &sti))
                return;
            /*
             * Get its device numbers.  If they are located, start the NAME
             * column with the device name, followed by "->".
             */
            nm = Namech;
            nmrl = Namechl - 1;
            Namech[Namechl - 1] = '\0';
            if (!sdp)
                sdp = finddev(ctx, &DevDev, &sti.sti_dev, LOOKDEV_ALL);
            if (sdp) {
                dev = DevDev;
                rdev = v->v_rdev;
                trdev = sdp->rdev;
                devs = rdevs = trdevs = 1;
                Lf->inode = (INODETYPE)sdp->inode;
                Lf->inp_ty = 1;
                (void)snpf(nm, nmrl, "%s", sdp->name);
                tl = strlen(nm);
                nm += tl;
                nmrl -= tl;
                sep = "->";
            } else {
                devs = rdevs = trdevs = 0;
                sep = "";
            }
            /*
             * Add the socket node's address to the NAME column.
             */
            sepl = strlen(sep);
            if (sona && ((nmrl - sepl) > 0)) {
                (void)snpf(nm, nmrl, "%s%s", sep,
                           print_kptr(sona, (char *)NULL, 0));
                tl = strlen(nm);
                nm += tl;
                nmrl -= tl;
            }
            /*
             * Add the service type to the NAME column.
             */
            switch (sti.sti_serv_type) {
            case T_CLTS:
                ty = "dgram";
                break;
            case T_COTS:
                ty = "stream";
                break;
            case T_COTS_ORD:
                ty = "stream-ord";
                break;
            default:
                ty = (char *)NULL;
            }
            if (ty && (nmrl > 1)) {
                (void)snpf(nm, nmrl, " %s", ty);
                tl = strlen(nm);
                nm += tl;
                nmrl -= tl;
            }
            /*
             * Add the vnode and connected addresses to the NAME column,
             * as indicated by the socket node state.
             */
            if ((so.so_state & SS_ISBOUND) && (nmrl > 36) &&
                (sti.sti_ux_laddr.soua_magic == SOU_MAGIC_EXPLICIT)) {
                (void)snpf(nm, nmrl, " Vn=%s",
                           print_kptr((KA_T)sti.sti_ux_laddr.soua_vp,
                                      (char *)NULL, 0));
                tl = strlen(nm);
                nm += tl;
                nmrl -= tl;
            }
            if ((so.so_state & SS_ISCONNECTED) && (nmrl > 38) &&
                (sti.sti_ux_faddr.soua_magic == SOU_MAGIC_EXPLICIT)) {
                (void)snpf(nm, nmrl, " Conn=%s ",
                           print_kptr((KA_T)sti.sti_ux_faddr.soua_vp,
                                      (char *)NULL, 0));
                tl = strlen(nm);
                nm += tl;
                nmrl -= tl;
            }
            /*
             * Put local and connected UNIX addresses in the NAME column, if
             * they exist and as indicated by the socket node's state.
             */
            if ((so.so_state & SS_ISBOUND) &&
                ((len = read_nusa(ctx, &sti.sti_laddr, &ua)) > 0) &&
                (nmrl > (len + 5))) {
                if (Sfile && is_file_named(ctx, ua.sun_path, Ntype, VSOCK, 0))
                    Lf->sf |= SELNM;
                if (len > nmrl)
                    len = nmrl;
                if (len > 0) {
                    ua.sun_path[len] = '\0';
                    (void)snpf(nm, nmrl, " Lcl=%s", ua.sun_path);
                    tl = strlen(nm);
                    nm += tl;
                    nmrl -= tl;
                }
            }
            if ((so.so_state & SS_ISCONNECTED) &&
                ((len = read_nusa(ctx, &sti.sti_faddr, &ua)) > 0) &&
                (nmrl > (len + 5))) {
                if (Sfile && is_file_named(ctx, ua.sun_path, Ntype, VSOCK, 0))
                    Lf->sf |= SELNM;
                if (len > nmrl)
                    len = nmrl;
                if (len > 0) {
                    ua.sun_path[len] = '\0';
                    (void)snpf(nm, nmrl, " Rem=%s", ua.sun_path);
                    tl = strlen(nm);
                    nm += tl;
                    nmrl -= tl;
                }
            }
        } else {

            /*
             * Process Solaris >= 11 AF_INET, AF_INET6 and AF_ROUTE VSOCK
             * nodes.
             */
            switch (so.so_family) {
            case AF_INET:
            case AF_INET6:
            case AF_ROUTE:
                if (process_VSOCK(ctx, (KA_T)va, v, &so))
                    return;
            }
        }
#    else /* solaris<110000 */
        {

            /*
             * Process an AF_UNIX socket node for Solaris < 11:
             *	  Locate its device numbers;
             *    Enter the sonode address as the device (netstat's local
             *	  address);
             *    Get a non-NULL local sockaddr_un and enter it in Namech;
             *    Get a non-NULL foreign sockaddr_un and enter it in Namech;
             *    Check for matches on sockaddr_un.sun_path names.
             */

            if (!sdp)
                sdp = finddev(&DevDev,

#        if solaris < 100000
                              &so.so_vnode.v_rdev,
#        else  /* solaris>=100000 */
                              &so.so_dev,
#        endif /* solaris<100000 */

                              LOOKDEV_ALL);

            if (sdp) {
                dev = DevDev;

#        if solaris < 100000
                rdev = so.so_vnode.v_rdev;
#        else  /* solaris>=100000 */
                rdev = so.so_dev;
#        endif /* solaris<100000 */

                trdev = sdp->rdev;
                devs = rdevs = trdevs = 1;
                Lf->inode = (INODETYPE)sdp->inode;
                Lf->inp_ty = 1;
                (void)snpf(Namech, Namechl - 1, "%s", sdp->name);
                Namech[Namechl - 1] = '\0';
            } else
                devs = 0;
            nl = snl = (int)strlen(Namech);

            if ((len = read_nusa(&so.so_laddr, &ua))) {
                if (Sfile && is_file_named(ctx, ua.sun_path, Ntype, VSOCK, 0))
                    Lf->sf |= SELNM;
                sepl = Namech[0] ? 2 : 0;
                if (len > (Namechl - nl - sepl - 1))
                    len = Namechl - nl - sepl - 1;
                if (len > 0) {
                    ua.sun_path[len] = '\0';
                    (void)snpf(&Namech[nl], Namechl - nl, "%s%s",
                               sepl ? "->" : "", ua.sun_path);
                    nl += (len + sepl);
                }
            }
            if ((len = read_nusa(&so.so_faddr, &ua))) {
                if (Sfile && is_file_named(ctx, ua.sun_path, Ntype, VSOCK, 0))
                    Lf->sf |= SELNM;
                sepl = Namech[0] ? 2 : 0;
                if (len > (Namechl - nl - sepl - 1))
                    len = Namechl - nl - sepl - 1;
                if (len > 0) {
                    ua.sun_path[len] = 0;
                    (void)snpf(&Namech[nl], Namechl - nl, "%s%s",
                               sepl ? "->" : "", ua.sun_path);
                    nl += (len + sepl);
                }
            }
            if ((nl == snl)

#        if defined(HASSOUXSOUA)
                && so.so_ux_laddr.soua_magic == SOU_MAGIC_IMPLICIT
#        else  /* !defined(HASSOUXSOUA) */
                && so.so_ux_laddr.sou_magic == SOU_MAGIC_IMPLICIT
#        endif /* defined(HASSOUXSOUA) */

            ) {

                /*
                 * There are no addresses; this must be a socket pair.
                 * Print its identity.
                 */
                pa = (struct pairaddr *)&ua;
                if (!(peer = (KA_T)((int)pa->p)))

#        if defined(HASSOUXSOUA)
                    peer = (KA_T)so.so_ux_laddr.soua_vp;
#        else  /* !defined(HASSOUXSOUA) */
                    peer = (KA_T)so.so_ux_laddr.sou_vp;
#        endif /* defined(HASSOUXSOUA) */

                if (peer)
                    (void)snpf(ubuf, sizeof(ubuf), "(socketpair: %s)",
                               print_kptr(peer, (char *)NULL, 0));
                else
                    (void)snpf(ubuf, sizeof(ubuf), "(socketpair)");
                len = (int)strlen(ubuf);
                sepl = Namech[0] ? 2 : 0;
                if (len > (Namechl - nl - sepl - 1))
                    len = Namechl - nl - sepl - 1;
                if (len > 0) {
                    (void)snpf(&Namech[nl], Namechl - nl, "%s%s",
                               sepl ? "->" : "", ubuf);
                    nl += (len + sepl);
                }
            }
            /*
             * Add the local and foreign addresses, ala `netstat -f unix` to
             * the name.
             */

#        if defined(HASSOUXSOUA)
            soa = (KA_T)so.so_ux_faddr.soua_vp;
#        else  /* !defined(HASSOUXSOUA) */
            soa = (KA_T)so.so_ux_faddr.sou_vp;
#        endif /* defined(HASSOUXSOUA) */

            (void)snpf(ubuf, sizeof(ubuf), "%s(%s%s%s)", Namech[0] ? " " : "",
                       print_kptr((KA_T)v->v_data, (char *)NULL, 0),
                       soa ? "->" : "",
                       soa ? print_kptr(soa, tbuf, sizeof(tbuf)) : "");
            len = (int)strlen(ubuf);
            if (len <= (Namechl - nl - 1)) {
                (void)snpf(&Namech[nl], Namechl - nl, "%s", ubuf);
                nl += len;
            }
            /*
             * If there is a bound vnode, add its address to the name.
             */

            if (so.so_ux_bound_vp) {
                (void)snpf(
                    ubuf, sizeof(ubuf), "%s(Vnode=%s)", Namech[0] ? " " : "",
                    print_kptr((KA_T)so.so_ux_bound_vp, (char *)NULL, 0));
                len = (int)strlen(ubuf);
                if (len <= (Namechl - nl - 1)) {
                    (void)snpf(&Namech[nl], Namechl - nl, "%s", ubuf);
                    nl += len;
                }
            }
        }
#    endif     /* solaris>=110000 */

        break;

#endif /* solaris>=20600 */

    case N_SPEC:

#if solaris < 100000
        if (((Ntype = vty) == N_STREAM) && so_st) {
            if (Funix)
                Lf->sf |= SELUNX;
            unix_sock = 1;
            if (so_ad[0]) {
                if (sdp) {
                    if (vfs) {
                        dev = vfs->dev;
                        devs = 1;
                    }
                    rdev = sdp->rdev;
                    rdevs = 1;
                    Lf->inode = (INODETYPE)sdp->inode;
                    Lf->inp_ty = 1;
                    (void)snpf(ubuf, sizeof(ubuf), "(%s%s%s)",
                               print_kptr(so_ad[0], (char *)NULL, 0),
                               so_ad[1] ? "->" : "",
                               so_ad[1]
                                   ? print_kptr(so_ad[1], tbuf, sizeof(tbuf))
                                   : "");
                } else {
                    enter_dev_ch(print_kptr(so_ad[0], (char *)NULL, 0));
                    if (so_ad[1])
                        (void)snpf(ubuf, sizeof(ubuf), "(->%s)",
                                   print_kptr(so_ad[1], (char *)NULL, 0));
                }
                if (!Lf->nma &&
                    (Lf->nma = (char *)malloc((int)strlen(ubuf) + 1))) {
                    (void)snpf(Lf->nma, (int)strlen(ubuf) + 1, "%s", ubuf);
                }
            } else if (soso.lux_dev.addr.tu_addr.ino) {
                if (vfs) {
                    dev = vfs->dev;
                    devs = 1;
                }
                rdev = soso.lux_dev.addr.tu_addr.dev;
                rdevs = 1;
            } else {
                int dc, dl, dr;

#    if solaris < 20400
                dl = (soso.lux_dev.addr.tu_addr.dev >> 16) & 0xffff;
                dr = (soso.rux_dev.addr.tu_addr.dev >> 16) & 0xffff;
#    else  /* solaris>=20400 */
                dl = soso.lux_dev.addr.tu_addr.dev & 0xffff;
                dr = soso.rux_dev.addr.tu_addr.dev & 0xffff;
#    endif /* solaris<20400 */

                dc = (dl << 16) | dr;
                enter_dev_ch(print_kptr((KA_T)dc, (char *)NULL, 0));
                devs = 0;
            }
            if (soso.laddr.buf && soso.laddr.len == sizeof(ua)) {
                if (kread(ctx, (KA_T)soso.laddr.buf, (char *)&ua, sizeof(ua)) ==
                    0) {
                    ua.sun_path[sizeof(ua.sun_path) - 1] = '\0';
                    if (ua.sun_path[0]) {
                        if (Sfile &&
                            is_file_named(ctx, ua.sun_path, Ntype, type, 0))
                            Lf->sf |= SELNM;
                        len = (int)strlen(ua.sun_path);
                        nl = (int)strlen(Namech);
                        sepl = Namech[0] ? 2 : 0;
                        if (len > (Namechl - nl - sepl - 1))
                            len = Namechl - nl - sepl - 1;
                        if (len > 0) {
                            ua.sun_path[len] = '\0';
                            (void)snpf(&Namech[nl], Namechl - nl, "%s%s",
                                       sepl ? "->" : "", ua.sun_path);
                        }
                    }
                }
            }
        } else
#endif /* solaris<100000 */

        {
            if (vfs) {
                dev = vfs->dev;
                devs = 1;
            }
            rdev = s.s_dev;
            rdevs = 1;
        }
        break;
    case N_STREAM:
        if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
        rdev = s.s_dev;
        rdevs = 1;
        break;
    case N_TMP:
        dev = t.tn_attr.va_fsid;
        devs = 1;
        break;

#if defined(HASVXFS)
    case N_VXFS:
        dev = vx.dev;
        devs = vx.dev_def;
        if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
            rdev = vx.rdev;
            rdevs = vx.rdev_def;
        }
        break;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
    case N_ZFS:
        if (zns) {
            if (!read_nzvfs(ctx, (KA_T)v->v_data, (KA_T)zn.z_zfsvfs, &zvfs) &&
                zvfs.z_vfs &&
                !kread(ctx, (KA_T)zvfs.z_vfs, (char *)&zgvfs, sizeof(zgvfs))) {
                dev = zgvfs.vfs_dev;
                devs = 1;
            }
        }
        if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
            rdev = v->v_rdev;
            rdevs = 1;
        }
        break;
#endif /* defined(HAS_ZFS) */

    default:
        if (ins) {
            dev = i.i_dev;
            devs = 1;
        } else if (nns) {
            dev = nn.nm_vattr.va_fsid;
            devs = 1;
        } else if (vfs) {
            dev = vfs->dev;
            devs = 1;
        }
        if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
            rdev = v->v_rdev;
            rdevs = 1;
        }
    }
    type = v->v_type;
    if (devs && vfs && !vfs->dir) {
        (void)completevfs(ctx, vfs, &dev);

#if defined(HAS_AFS)
        if (vfs->dir && (Ntype == N_AFS || vty == N_AFS) && !AFSVfsp)
            AFSVfsp = (KA_T)v->v_vfsp;
#endif /* defined(HAS_AFS) */
    }
    /*
     * Obtain the inode number.
     */
    switch (vty) {

#if defined(HAS_AFS)
    case N_AFS:
        if (an.ino_st) {
            Lf->inode = (INODETYPE)an.inode;
            Lf->inp_ty = 1;
        }
        break;
#endif /* defined(HAS_AFS) */

#if solaris >= 20500
    case N_AUTO:

#    if solaris < 20600
        Lf->inode = (INODETYPE)au.an_nodeid;
#    else  /* solaris>=20600 */
        Lf->inode = (INODETYPE)fnn.fn_nodeid;
#    endif /* solaris<20600 */

        Lf->inp_ty = 1;
        break;

#    if solaris >= 100000
    case N_DEV:
        if (dvs) {
            Lf->inode = (INODETYPE)dv.dv_ino;
            Lf->inp_ty = 1;
        }
        break;
#    endif /* solaris>=100000 */

    case N_DOOR:
        if (nns && (Lf->inode = (INODETYPE)nn.nm_vattr.va_nodeid)) {
            Lf->inp_ty = 1;
            break;
        }
        if (dns) {
            if ((Lf->inode = (INODETYPE)dn.door_index))
                Lf->inp_ty = 1;
        }
        break;
#endif /* solaris>=20500 */

#if defined(HASCACHEFS)
    case N_CACHE:
        Lf->inode = (INODETYPE)cn.c_fileno;
        Lf->inp_ty = 1;
        break;
#endif /* defined(HASCACHEFS) */

#if solaris >= 100000
    case N_CTFSADIR:
    case N_CTFSBUND:
    case N_CTFSCDIR:
    case N_CTFSCTL:
    case N_CTFSEVT:
    case N_CTFSLATE:
    case N_CTFSROOT:
    case N_CTFSSTAT:
    case N_CTFSSYM:
    case N_CTFSTDIR:
    case N_CTFSTMPL:
        /* Method of computing CTFS inode not known. */
        break;
#endif /* solaris>=10000 */

    case N_FD:
        if (v->v_type == VDIR)
            Lf->inode = (INODETYPE)2;
        else
            Lf->inode = (INODETYPE)(GET_MIN_DEV(v->v_rdev) * 100);
        Lf->inp_ty = 1;
        break;
    case N_HSFS:
        Lf->inode = (INODETYPE)h.hs_nodeid;
        Lf->inp_ty = 1;
        break;

    case N_MNT:

#if defined(HASFSINO)
        if (vfs) {
            Lf->inode = vfs->fs_ino;
            Lf->inp_ty = 1;
        }
#endif /* defined(HASFSINO) */

        break;
    case N_MVFS:
        Lf->inode = (INODETYPE)m.m_ino;
        Lf->inp_ty = 1;
        break;
    case N_NFS:
        Lf->inode = (INODETYPE)r.r_attr.va_nodeid;
        Lf->inp_ty = 1;
        break;

#if solaris >= 100000
    case N_NFS4:
        Lf->inode = (INODETYPE)r4.r_attr.va_nodeid;
        Lf->inp_ty = 1;
        break;
#endif /* solaris>=100000 */

    case N_NM:
        Lf->inode = (INODETYPE)nn.nm_vattr.va_nodeid;
        Lf->inp_ty = 1;
        break;

#if defined(HASPROCFS)
    case N_PROC:

        /*
         * The proc file system inode number is defined when the
         * prnode is read.
         */
        break;
#endif /* defined(HASPROCFS) */

    case N_PCFS:
        if (kvs && kv.vfs_data &&
            !kread(ctx, (KA_T)kv.vfs_data, (char *)&pcfs, sizeof(pcfs))) {

#if solaris >= 70000
#    if defined(HAS_PC_DIRENTPERSEC)
            Lf->inode = (INODETYPE)pc_makenodeid(
                pc.pc_eblkno, pc.pc_eoffset, pc.pc_entry.pcd_attr,
                IS_FAT32(&pcfs)
                    ? ltohs(pc.pc_entry.pcd_scluster_lo) |
                          (ltohs(pc.pc_entry.un.pcd_scluster_hi) << 16)
                    : ltohs(pc.pc_entry.pcd_scluster_lo),
                pc_direntpersec(&pcfs));
#    else  /* !defined(HAS_PC_DIRENTPERSEC) */
            Lf->inode = (INODETYPE)pc_makenodeid(
                pc.pc_eblkno, pc.pc_eoffset, pc.pc_entry.pcd_attr,
                IS_FAT32(&pcfs)
                    ? ltohs(pc.pc_entry.pcd_scluster_lo) |
                          (ltohs(pc.pc_entry.un.pcd_scluster_hi) << 16)
                    : ltohs(pc.pc_entry.pcd_scluster_lo),
                pcfs.pcfs_entps);
#    endif /* defined(HAS_PC_DIRENTPERSEC) */
#else      /* solaris<70000 */
            Lf->inode = (INODETYPE)pc_makenodeid(pc.pc_eblkno, pc.pc_eoffset,
                                                 &pc.pc_entry, pcfs.pcfs_entps);
#endif     /* solaris>=70000 */

            Lf->inp_ty = 1;
        }
        break;

    case N_REGLR:
        if (nns) {
            if ((Lf->inode = (INODETYPE)nn.nm_vattr.va_nodeid))
                Lf->inp_ty = 1;
        } else if (ins) {
            if ((Lf->inode = (INODETYPE)i.i_number))
                Lf->inp_ty = 1;
        }
        break;
    case N_SAMFS:
        break; /* No more SAM-FS information is available. */

#if solaris >= 110000
    case N_SDEV:
        if (sdns) {
            Lf->inode = (INODETYPE)sdva.va_nodeid;
            Lf->inp_ty = 1;
        }
        break;
#endif /* solaris>=110000 */

    case N_SHARED:
        (void)snpf(Lf->iproto, sizeof(Lf->iproto), "SHARED");
        Lf->inp_ty = 2;
        break;
    case N_STREAM:

#if solaris < 100000
        if (so_st && soso.lux_dev.addr.tu_addr.ino) {
            if (Lf->inp_ty) {
                nl = Lf->nma ? (int)strlen(Lf->nma) : 0;
                (void)snpf(ubuf, sizeof(ubuf), "%s(Inode=%lu)", nl ? " " : "",
                           (unsigned long)soso.lux_dev.addr.tu_addr.ino);
                len = nl + (int)strlen(ubuf) + 1;
                if (Lf->nma)
                    Lf->nma = (char *)realloc(Lf->nma, len);
                else
                    Lf->nma = (char *)malloc(len);
                if (Lf->nma)
                    (void)snpf(&Lf->nma[nl], len - nl, "%s", ubuf);
            } else {
                Lf->inode = (INODETYPE)soso.lux_dev.addr.tu_addr.ino;
                Lf->inp_ty = 1;
            }
        }
#endif /* solaris<100000 */

        break;
    case N_TMP:
        Lf->inode = (INODETYPE)t.tn_attr.va_nodeid;
        Lf->inp_ty = 1;
        break;

#if defined(HASVXFS)
    case N_VXFS:
        if (vx.ino_def) {
            Lf->inode = (INODETYPE)vx.ino;
            Lf->inp_ty = 1;
        } else if (type == VCHR)
            pnl = 1;
        break;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
    case N_ZFS:
        if (zns) {
            Lf->inode = (INODETYPE)zn.z_id;
            Lf->inp_ty = 1;
        }
        break;
#endif /* defined(HAS_ZFS) */
    }
    /*
     * Obtain the file size.
     */
    switch (Ntype) {

#if defined(HAS_AFS)
    case N_AFS:
        Lf->sz = (SZOFFTYPE)an.size;
        Lf->sz_def = 1;
        break;
#endif /* defined(HAS_AFS) */

#if solaris >= 20500
    case N_AUTO:

#    if solaris < 20600
        Lf->sz = (SZOFFTYPE)au.an_size;
#    else  /* solaris >=20600 */
        Lf->sz = (SZOFFTYPE)fnn.fn_size;
#    endif /* solaris < 20600 */

        Lf->sz_def = 1;
        break;
#endif /* solaris>=20500 */

#if defined(HASCACHEFS)
    case N_CACHE:
        Lf->sz = (SZOFFTYPE)cn.c_size;
        Lf->sz_def = 1;
        break;
#endif /* defined(HASCACHEFS) */

#if solaris >= 100000
    case N_CTFSADIR:
    case N_CTFSBUND:
    case N_CTFSCDIR:
    case N_CTFSCTL:
    case N_CTFSEVT:
    case N_CTFSLATE:
    case N_CTFSROOT:
    case N_CTFSSTAT:
    case N_CTFSSYM:
    case N_CTFSTDIR:
    case N_CTFSTMPL:
        /* Method of computing CTFS size not known. */
        break;
#endif /* solaris>=100000 */

    case N_FD:
        if (v->v_type == VDIR)
            Lf->sz = (Unof + 2) * 16;
        else
            Lf->sz = (unsigned long)0;
        Lf->sz_def = 1;
        break;

#if solaris >= 20600
    case N_SOCK:
        break;
#endif /* solaris>=20600 */

    case N_HSFS:
        Lf->sz = (SZOFFTYPE)h.hs_dirent.ext_size;
        Lf->sz_def = 1;
        break;
    case N_NM:
        Lf->sz = (SZOFFTYPE)nn.nm_vattr.va_size;
        Lf->sz_def = 1;
        break;

#if solaris >= 100000
    case N_DEV:
        break;
#endif /* solaris>=100000 */

    case N_DOOR:
    case N_FIFO:
        break;
    case N_MNT:

#if defined(CVFS_SZSAVE)
        if (vfs) {
            Lf->sz = (SZOFFTYPE)vfs->size;
            Lf->sz_def = 1;
        } else
#endif /* defined(CVFS_SZSAVE) */

            break;
    case N_MVFS:
        /* The location of file size isn't known. */
        break;
    case N_NFS:
        if (!(type == VCHR || type == VBLK)) {
            Lf->sz = (SZOFFTYPE)r.r_size;
            Lf->sz_def = 1;
        }
        break;

#if solaris >= 100000
    case N_NFS4:
        if (!(type == VCHR || type == VBLK)) {
            Lf->sz = (SZOFFTYPE)r4.r_size;
            Lf->sz_def = 1;
        }
        break;
#endif /* solaris>=100000 */

    case N_PCFS:
        Lf->sz = (SZOFFTYPE)pc.pc_size;
        Lf->sz_def = 1;
        break;

#if solaris >= 100000
    case N_PORT:
        Lf->sz = (SZOFFTYPE)pn.port_curr;
        Lf->sz_def = 1;
        break;
#endif /* solaris>=100000 */

#if defined(HASPROCFS)
    case N_PROC:

        /*
         * The proc file system size is defined when the
         * prnode is read.
         */
        break;
#endif /* defined(HASPROCFS) */

    case N_REGLR:
        if (type == VREG || type == VDIR) {
            if (ins | nns) {
                Lf->sz = (SZOFFTYPE)(nns ? nn.nm_vattr.va_size : i.i_size);
                Lf->sz_def = 1;
            }
        }
        break;

#if solaris >= 110000
    case N_SDEV:
        if (sdns) {
            if (type == VREG || type == VDIR) {
                Lf->sz = (SZOFFTYPE)sdva.va_size;
                Lf->sz_def = 1;
            }
        }
        break;
#endif /* solaris>=110000 */

    case N_SAMFS:
        break; /* No more SAM-FS information is available. */
    case N_SHARED:
        break; /* No more sharedfs information is available. */
    case N_STREAM:
        break;
    case N_TMP:
        Lf->sz = (SZOFFTYPE)t.tn_attr.va_size;
        Lf->sz_def = 1;
        break;

#if defined(HASVXFS)
    case N_VXFS:
        if (type == VREG || type == VDIR) {
            Lf->sz = (SZOFFTYPE)vx.sz;
            Lf->sz_def = vx.sz_def;
        }
        break;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
    case N_ZFS:
        if (zns) {
            if (type == VREG || type == VDIR) {
                Lf->sz = (SZOFFTYPE)zn.z_size;
                Lf->sz_def = 1;
            }
        }
        break;
#endif /* defined(HAS_ZFS) */
    }
    /*
     * Record link count.
     */

    switch (Ntype) {

#if defined(HAS_AFS)
    case N_AFS:
        Lf->nlink = an.nlink;
        Lf->nlink_def = an.nlink_st;
        break;
#endif /* defined(HAS_AFS) */

#if solaris >= 20500
    case N_AUTO:
        break;

#    if defined(HASCACHEFS)
    case N_CACHE:
        Lf->nlink = (long)cn.c_attr.va_nlink;
        Lf->nlink_def = 1;
        break;
#    endif /* defined(HASCACHEFS) */

#endif /* solaris>=20500 */

#if solaris >= 100000
    case N_CTFSADIR:
    case N_CTFSBUND:
    case N_CTFSCDIR:
    case N_CTFSCTL:
    case N_CTFSEVT:
    case N_CTFSLATE:
    case N_CTFSROOT:
    case N_CTFSSTAT:
    case N_CTFSSYM:
    case N_CTFSTDIR:
    case N_CTFSTMPL:
        /* Method of computing CTFS link count not known. */
        break;
#endif /* solaris>=100000 */

    case N_FD:
        Lf->nlink = (v->v_type == VDIR) ? 2 : 1;
        Lf->nlink_def = 1;
        break;

#if solaris >= 20600
    case N_SOCK: /* no link count */
        break;
#endif /* solaris>=20600 */

    case N_HSFS:
        Lf->nlink = (long)h.hs_dirent.nlink;
        Lf->nlink_def = 1;
        break;
    case N_NM:
        Lf->nlink = (long)nn.nm_vattr.va_nlink;
        Lf->nlink_def = 1;
        break;

#if solaris >= 100000
    case N_DEV:
        if (dvs) {
            Lf->nlink = (long)dv.dv_nlink;
            Lf->nlink_def = 1;
        }
        break;
#endif /* solaris>=100000 */

    case N_DOOR:
        Lf->nlink = (long)v->v_count;
        Lf->nlink_def = 1;
        break;
    case N_FIFO:
        break;
    case N_MNT:

#if defined(CVFS_NLKSAVE)
        if (vfs) {
            Lf->nlink = (long)vfs->nlink;
            Lf->nlink_def = 1;
        }
#endif /* defined(CVFS_NLKSAVE) */

        break;
    case N_MVFS: /* no link count */
        break;
    case N_NFS:
        Lf->nlink = (long)r.r_attr.va_nlink;
        Lf->nlink_def = 1;
        break;

#if solaris >= 100000
    case N_NFS4:
        Lf->nlink = (long)r4.r_attr.va_nlink;
        Lf->nlink_def = 1;
        break;
#endif /* solaris>=100000 */

    case N_PCFS:
        break;

#if defined(HASPROCFS)
    case N_PROC:
        break;
#endif /* defined(HASPROCFS) */

    case N_REGLR:
        if (ins) {
            Lf->nlink = (long)i.i_nlink;
            Lf->nlink_def = 1;
        }
        break;
    case N_SAMFS:
        break; /* No more SAM-FS information is available. */

#if solaris >= 110000
    case N_SDEV:
        if (sdns) {
            Lf->nlink = (long)sdva.va_nlink;
            Lf->nlink_def = 1;
        }
        break;
#endif /* solaris>=110000 */

    case N_SHARED:
        break; /* No more sharedfs information is available. */
    case N_STREAM:
        break;
    case N_TMP:
        Lf->nlink = (long)t.tn_attr.va_nlink;
        Lf->nlink_def = 1;
        break;

#if defined(HASVXFS)
    case N_VXFS:
        Lf->nlink = vx.nl;
        Lf->nlink_def = vx.nl_def;
        break;
#endif /* defined(HASVXFS) */

#if defined(HAS_ZFS)
    case N_ZFS:
        if (zns) {
            Lf->nlink = (long)MIN(zn.z_links, UINT32_MAX);
            Lf->nlink_def = 1;
        }
        break;
#endif /* defined(HAS_ZFS) */
    }
    if (Nlink && Lf->nlink_def && (Lf->nlink < Nlink))
        Lf->sf |= SELNLINK;

#if defined(HASVXFS)
        /*
         * Record a VxFS file.
         */

#    if defined(HASVXFSDNLC)
    Lf->is_vxfs = (Ntype == N_VXFS) ? 1 : 0;
#    endif /* defined(HASVXFSDNLC) */
#endif     /* defined(HASVXFS) */

    /*
     * Record an NFS selection.
     */
    if (Fnfs) {
        if ((Ntype == N_NFS) || (Ntype == N_NFS4))
            Lf->sf |= SELNFS;
    }

#if solaris >= 20500
    /*
     * If this is a Solaris 2.5 and greater autofs entry, save the autonode name
     * (less than Solaris 2.6) or fnnode name (Solaris 2.6 and greater).
     */
    if (Ntype == N_AUTO && !Namech[0]) {

#    if solaris < 20600
        if (au.an_name[0])
            (void)snpf(Namech, Namechl - 1, "%s", au.an_name);
        Namech[Namechl - 1] = '\0';
#    else  /* solaris>=20600 */
        if (fnn.fn_name && (len = fnn.fn_namelen) > 0 && len < (Namechl - 1)) {
            if (kread(ctx, (KA_T)fnn.fn_name, Namech, len))
                Namech[0] = '\0';
            else
                Namech[len] = '\0';
        }
#    endif /* solaris<20600 */
    }
    /*
     * If there is no local virtual file system pointer, or if its directory and
     * file system names are NULL, and if there is a namenode, and if we're
     * using the device number from it, see if its nm_mountpt vnode pointer
     * leads to a local virtual file system structure with non-NULL directory
     * and file system names.  If it does, switch to that local virtual file
     * system pointer.
     */
    if (nns && (!vfs || (!vfs->dir && !vfs->fsname)) && devs &&
        (dev == nn.nm_vattr.va_fsid) && nn.nm_mountpt) {
        if (!readvnode(ctx, (KA_T)nn.nm_mountpt, &fv) && fv.v_vfsp) {
            if ((nvfs = readvfs(ctx, (KA_T)fv.v_vfsp, (struct vfs *)NULL,
                                nn.nm_filevp)) &&
                !nvfs->dir) {
                (void)completevfs(ctx, nvfs, &dev);
            }

#    if defined(HASNCACHE)
            if (nvfs && nvfs->dir && nvfs->fsname) {
                fa = (char *)NULL;
                vfs = nvfs;
            }
#    endif /* defined(HASNCACHE) */
        }
    }

#    if defined(HASNCACHE)
    /*
     * If there's a namenode and its device and node number match this one,
     * use the nm_mountpt's address for name cache lookups.
     */
    if (nns && devs && (dev == nn.nm_vattr.va_fsid) && (Lf->inp_ty == 1) &&
        (Lf->inode == (INODETYPE)nn.nm_vattr.va_nodeid))
        Lf->na = (KA_T)nn.nm_mountpt;
#    endif /* defined(HASNCACHE) */
#endif     /* solaris>=20500 */

    /*
     * Save the file system names.
     */
    if (vfs) {
        Lf->fsdir = vfs->dir;
        Lf->fsdev = vfs->fsname;

#if defined(HASMNTSTAT)
        Lf->mnt_stat = vfs->mnt_stat;
#endif /* defined(HASMNTSTAT) */

        if (!Lf->fsdir && !Lf->fsdev && kvs && fxs) {

            /*
             * The file system names are unknown.
             *
             * Set the file system device to the file system type and clear
             * the doubtful device numbers.
             */
            Lf->fsdev = Fsinfo[fx];
            devs = 0;
            rdevs = 0;
        }

#if defined(HASFSINO)
        else
            Lf->fs_ino = vfs->fs_ino;
#endif /* defined(HASFSINO) */
    }
    /*
     * Save the device numbers, and their states.
     *
     * Format the vnode type, and possibly the device name.
     */
    switch (type) {

    case VNON:
        Lf->type = LSOF_FILE_VNODE_VNON;
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        break;
    case VREG:
    case VDIR:
        Lf->type = (type == VREG) ? LSOF_FILE_VNODE_VREG : LSOF_FILE_VNODE_VDIR;
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        break;
    case VBLK:
        Lf->type = LSOF_FILE_VNODE_VBLK;
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        Ntype = N_BLK;
        break;
    case VCHR:
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        if (unix_sock) {
            Lf->type = LSOF_FILE_UNIX;
            break;
        }
        Lf->type = LSOF_FILE_VNODE_VCHR;
        if (Lf->is_stream == 0 && Lf->is_com == 0)
            Ntype = N_CHR;
        break;

#if solaris >= 20500
    case VDOOR:
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        Lf->type = LSOF_FILE_VNODE_VDOOR;
        if (dns)
            (void)idoorkeep(ctx, &dn);
        break;
#endif /* solaris>=20500 */

    case VLNK:
        Lf->type = LSOF_FILE_VNODE_VLNK;
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        break;

#if solaris >= 100000
    case VPORT:
        Lf->type = LSOF_FILE_VNODE_VPORT;
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        break;
#endif /* solaris>=100000 */

#if solaris >= 20600
    case VPROC:

        /*
         * The proc file system type is defined when the prnode is read.
         */
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        Lf->type = LSOF_FILE_NONE;
        break;
#endif /* solaris>=20600 */

#if defined(HAS_VSOCK)
    case VSOCK:

#    if solaris >= 20600
        if (so.so_family == AF_UNIX) {
            Lf->type = LSOF_FILE_UNIX;
            if (Funix)
                Lf->sf |= SELUNX;
        } else {
            if (so.so_family == AF_INET) {

#        if defined(HASIPv6)
                Lf->type = LSOF_FILE_IPV4;
#        else  /* !defined(HASIPv6) */
                Lf->type = LSOF_FILE_INET;
#        endif /* defined(HASIPv6) */

                (void)snpf(Namech, Namechl - 1, printsockty(so.so_type));
                Namech[Namechl - 1] = '\0';
                if (TcpStIn || UdpStIn || TcpStXn || UdpStXn)
                    Lf->sf |= SELEXCLF;
                else if (Fnet && (FnetTy != 6))
                    Lf->sf |= SELNET;
            }

#        if defined(HASIPv6)
            else if (so.so_family == AF_INET6) {
                Lf->type = LSOF_FILE_IPV6;
                (void)snpf(Namech, Namechl - 1, printsockty(so.so_type));
                Namech[Namechl - 1] = '\0';
                if (TcpStIn || UdpStIn || TcpStXn || UdpStXn)
                    Lf->sf |= SELEXCLF;
                else if (Fnet && (FnetTy != 4))
                    Lf->sf |= SELNET;
            }
#        endif /* defined(HASIPv6) */

            else {
                Lf->type = LSOF_FILE_SOCKET;
                (void)printunkaf(ctx, so.so_family, 0);
                ep = endnm(ctx, &sz);
                (void)snpf(ep, sz, ", %s", printsockty(so.so_type));
            }
        }
#    endif /* solaris>=20600 */

        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        break;
#endif /* defined(HAS_VSOCK) */

    case VBAD:
        Lf->type = LSOF_FILE_VNODE_VBAD;
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        break;
    case VFIFO:
        Lf->type = LSOF_FILE_VNODE_VFIFO;
        if (!Lf->dev_ch || Lf->dev_ch[0] == '\0') {
            Lf->dev = dev;
            Lf->dev_def = devs;
            Lf->rdev = rdev;
            Lf->rdev_def = rdevs;
        }
        break;
    default:
        Lf->dev = dev;
        Lf->dev_def = devs;
        Lf->rdev = rdev;
        Lf->rdev_def = rdevs;
        Lf->type = LSOF_FILE_UNKNOWN_RAW;
        Lf->unknown_file_type_number = type;
    }
    Lf->ntype = Ntype;
    /*
     * If this a Solaris common vnode/snode void some information.
     */
    if (Lf->is_com)
        Lf->sz_def = Lf->inp_ty = 0;
    /*
     * If a file attach description remains, put it in the NAME column addition.
     */
    if (fa)
        (void)add_nma(ctx, fa, fal);

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
    if ((Lf->inp_ty == 0) && (type == VCHR)) {
        find_ch_ino(ctx);
        /*
         * If the VCHR inode number still isn't known and this is a COMMON
         * vnode file or a stream, or if a pseudo node ID lookup has been
         * requested, see if an inode number can be derived from a pseudo
         * or clone device node.
         *
         * If it can, save the pseudo or clone device for temporary
         * use when searching for a match with a named file argument.
         */
        if ((Lf->inp_ty == 0) && (Lf->is_com || Lf->is_stream || pnl) &&
            (Clone || Pseudo)) {
            if (!sdp) {
                if (rdevs || devs) {
                    if (Lf->is_stream && !pnl)
                        sdp = finddev(ctx, devs ? &dev : &DevDev,
                                      rdevs ? &rdev : &Lf->dev, LOOKDEV_CLONE);
                    else
                        sdp = finddev(ctx, devs ? &dev : &DevDev,
                                      rdevs ? &rdev : &Lf->dev, LOOKDEV_PSEUDO);
                    if (!sdp)
                        sdp = finddev(ctx, devs ? &dev : &DevDev,
                                      rdevs ? &rdev : &Lf->dev, LOOKDEV_ALL);
                    if (sdp) {
                        if (!rdevs) {
                            Lf->rdev = Lf->dev;
                            Lf->rdev_def = rdevs = 1;
                        }
                        if (!devs) {
                            Lf->dev = DevDev;
                            devs = Lf->dev_def = 1;
                        }
                    }
                }
            } else {

                /*
                 * A local device structure has been located.  Make sure
                 * that it's accompanied by device settings.
                 */
                if (!devs && vfs) {
                    dev = Lf->dev = vfs->dev;
                    devs = Lf->dev_def = 1;
                }
                if (!rdevs) {
                    Lf->rdev = rdev = sdp->rdev;
                    Lf->rdev_def = rdevs = 1;
                }
            }
            if (sdp) {

                /*
                 * Process the local device information.
                 */
                trdev = sdp->rdev;
                Lf->inode = sdp->inode;
                Lf->inp_ty = trdevs = 1;
                if (!Namech[0] || Lf->is_com) {
                    (void)snpf(Namech, Namechl - 1, "%s", sdp->name);
                    Namech[Namechl - 1] = '\0';
                }
                if (Lf->is_com && !Lf->nma) {
                    len = (int)strlen("(COMMON)") + 1;
                    if (!(Lf->nma = (char *)malloc(len))) {
                        fd_to_string(Lf->fd_type, Lf->fd_num, fd);
                        (void)fprintf(
                            stderr,
                            "%s: no space for (COMMON): PID %d; FD %s\n", Pn,
                            Lp->pid, fd);
                        Error(ctx);
                    }
                    (void)snpf(Lf->nma, len, "(COMMON)");
                }
            }
        }
    }
    /*
     * Record stream status.
     */
    if (Lf->inp_ty == 0 && Lf->is_stream && strcmp(Lf->iproto, "STR") == 0)
        Lf->inp_ty = 2;
        /*
         * Test for specified file.
         */

#if defined(HASPROCFS)
    if (Ntype == N_PROC) {
        if (Procsrch) {
            Procfind = 1;
            Lf->sf |= SELNM;
        } else {
            for (pfi = Procfsid; pfi; pfi = pfi->next) {
                if ((pfi->pid && pfi->pid == pids.pid_id)

#    if defined(HASPINODEN)
                    || (Lf->inp_ty == 1 && Lf->inode == pfi->inode)
#    endif /* defined(HASPINODEN) */

                ) {
                    pfi->f = 1;
                    if (!Namech[0]) {
                        (void)snpf(Namech, Namechl - 1, "%s", pfi->nm);
                        Namech[Namechl - 1] = '\0';
                    }
                    Lf->sf |= SELNM;
                    break;
                }
            }
        }
    } else
#endif /* defined(HASPROCFS) */

    {
        if (Sfile) {
            if (trdevs) {
                rdev = Lf->rdev;
                Lf->rdev = trdev;
                tdef = Lf->rdev_def;
                Lf->rdev_def = 1;
            }
            if (is_file_named(ctx, NULL, Ntype, type, 1))
                Lf->sf |= SELNM;
            if (trdevs) {
                Lf->rdev = rdev;
                Lf->rdev_def = tdef;
            }
        }
    }
    /*
     * Enter name characters.
     */
    if (Namech[0])
        enter_nm(ctx, Namech);
}

/*
 * read_cni() - read common snode information
 */

static int read_cni(struct lsof_context *ctx, /* context */
                    struct snode *s,          /* starting snode */
                    struct vnode *rv,         /* "real" vnode receiver */
                    struct vnode *v,          /* starting vnode */
                    struct snode *rs,         /* "real" snode receiver */
                    struct dev_info *di,      /* dev_info structure receiver */
                    char *din,                /* device info name receiver */
                    int dinl)                 /* sizeof(*din) */
{
    char tbuf[32];

    if (read_nvn(ctx, (KA_T)v->v_data, (KA_T)s->s_commonvp, rv))
        return (1);
    if (read_nsn(ctx, (KA_T)s->s_commonvp, (KA_T)rv->v_data, rs))
        return (1);
    *din = '\0';
    if (rs->s_dip) {
        if (kread(ctx, (KA_T)rs->s_dip, (char *)di, sizeof(struct dev_info))) {
            (void)snpf(Namech, Namechl - 1,
                       "common snode at %s: no dev info: %s",
                       print_kptr((KA_T)rv->v_data, tbuf, sizeof(tbuf)),
                       print_kptr((KA_T)rs->s_dip, (char *)NULL, 0));
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            return (1);
        }
        if (di->devi_name &&
            kread(ctx, (KA_T)di->devi_name, din, dinl - 1) == 0)
            din[dinl - 1] = '\0';
    }
    return (0);
}

/*
 * readinode() - read inode
 */

static int readinode(struct lsof_context *ctx, /* context */
                     KA_T ia,                  /* inode kernel address */
                     struct inode *i)          /* inode buffer */
{
    if (kread(ctx, (KA_T)ia, (char *)i, sizeof(struct inode))) {
        (void)snpf(Namech, Namechl - 1, "can't read inode at %s",
                   print_kptr((KA_T)ia, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if solaris >= 20500
/*
 * read_ndn() - read node's door node
 */

static int read_ndn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing vnode's address */
                    KA_T da,                  /* door node's address */
                    struct door_node *dn)     /* door node receiver */
{
    char tbuf[32];

    if (!da || kread(ctx, (KA_T)da, (char *)dn, sizeof(struct door_node))) {
        (void)snpf(Namech, Namechl - 1, "vnode at %s: can't read door_node: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(da, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=20500 */

/*
 * read_mi() - read stream's module information
 */

static void read_mi(struct lsof_context *ctx, /* context */
                    KA_T s,             /* kernel stream pointer address */
                    dev_t *rdev,        /* raw device pointer */
                    caddr_t so,         /* so_so return (Solaris) */
                    int *so_st,         /* so_so status */
                    KA_T *so_ad,        /* so_so addresses */
                    struct l_dev **sdp) /* returned device pointer */
{
    struct l_dev *dp;
    int i, j, k, nl;
    KA_T ka;
    struct module_info mi;
    char mn[STRNML];
    struct stdata sd;
    struct queue q;
    struct qinit qi;
    KA_T qp;
    /*
     * If there is no stream pointer, or we can't read the stream head,
     * return.
     */
    if (!s)
        return;
    if (kread(ctx, (KA_T)s, (char *)&sd, sizeof(sd))) {
        (void)snpf(Namech, Namechl - 1, "can't read stream head: %s",
                   print_kptr(s, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return;
    }
    /*
     * Follow the stream head to each of its queue structures, retrieving the
     * module names from each queue's q_info->qi_minfo->mi_idname chain of
     * structures.  Separate each additional name from the previous one with
     * "->".
     *
     * Ignore failures to read all but queue structure chain entries.
     *
     * Ignore module names that end in "head".
     */
    k = 0;
    Namech[0] = '\0';
    if (!(dp = finddev(ctx, &DevDev, rdev, LOOKDEV_CLONE)))
        dp = finddev(ctx, &DevDev, rdev, LOOKDEV_ALL);
    if (dp) {
        (void)snpf(Namech, Namechl - 1, "%s", dp->name);
        Namech[Namechl - 1] = '\0';
        k = (int)strlen(Namech);
        *sdp = dp;
    } else
        (void)snpf(Lf->iproto, sizeof(Lf->iproto), "STR");
    nl = sizeof(mn) - 1;
    mn[nl] = '\0';
    qp = (KA_T)sd.sd_wrq;
    for (i = 0; qp && i < 20; i++, qp = (KA_T)q.q_next) {
        if (!qp || kread(ctx, qp, (char *)&q, sizeof(q)))
            break;
        if ((ka = (KA_T)q.q_qinfo) == (KA_T)NULL ||
            kread(ctx, ka, (char *)&qi, sizeof(qi)))
            continue;
        if ((ka = (KA_T)qi.qi_minfo) == (KA_T)NULL ||
            kread(ctx, ka, (char *)&mi, sizeof(mi)))
            continue;
        if ((ka = (KA_T)mi.mi_idname) == (KA_T)NULL || kread(ctx, ka, mn, nl))
            continue;
        if ((j = (int)strlen(mn)) < 1)
            continue;
        if (j >= 4 && strcmp(&mn[j - 4], "head") == 0)
            continue;

#if solaris < 100000
        if (strcmp(mn, "sockmod") == 0) {

            /*
             * Save the Solaris sockmod device and inode numbers.
             */
            if (so) {

                struct so_so s;

                if (!kread(ctx, (KA_T)q.q_ptr, (char *)&s, sizeof(s))) {
                    if (!(*so_st))
                        so_ad[0] = (KA_T)q.q_ptr;
                    else
                        so_ad[1] = (KA_T)q.q_ptr;
                    (void)savesockmod(&s, (struct so_so *)so, so_st);
                }
            }
        }
#endif /* solaris<100000 */

        if (k) {
            if ((k + 2) > (Namechl - 1))
                break;
            (void)snpf(&Namech[k], Namechl - k, "->");
            k += 2;
        }
        if ((k + j) > (Namechl - 1))
            break;
        (void)snpf(&Namech[k], Namechl - k, "%s", mn);
        k += j;
    }
}

#if solaris >= 20500

/*
 * read_nan(na, ca, cn) - read node's autofs node
 */

static int read_nan(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T aa,                  /* autofs node address */

#    if solaris < 20600
                    struct autonode *rn) /* autofs node receiver */
#    else                                /* solaris>=20600 */
                    struct fnnode *rn) /* autofs node receiver */
#    endif                               /* solaris<20600 */

{
    char tbuf[32];

#    if solaris < 20600
    if (!aa || kread(ctx, (KA_T)aa, (char *)rn, sizeof(struct autonode)))
#    else  /* solaris>=20600 */
    if (!aa || kread(ctx, (KA_T)aa, (char *)rn, sizeof(struct fnnode)))
#    endif /* solaris<20600 */

    {
        (void)snpf(Namech, Namechl - 1,

#    if solaris < 20600
                   "node at %s: can't read autonode: %s",
#    else  /* solaris>=20600 */
                   "node at %s: can't read fnnode: %s",
#    endif /* solaris<20600 */

                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(aa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=20500 */

#if defined(HASCACHEFS)
/*
 * read_ncn(na, ca, cn) - read node's cache node
 */

static int read_ncn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T ca,                  /* cache node address */
                    struct cnode *cn)         /* cache node receiver */
{
    char tbuf[32];

    if (!ca || kread(ctx, (KA_T)ca, (char *)cn, sizeof(struct cnode))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read cnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ca, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* defined(HASCACHEFS) */

#if solaris >= 100000
/*
 * read_nctfsn(ty, na, ca, cn) - read node's cache node
 */

static int read_nctfsn(struct lsof_context *ctx, /* context */
                       int ty,   /* node type -- i.e., N_CTFS* */
                       KA_T na,  /* containing node's address */
                       KA_T ca,  /* cache node address */
                       char *cn) /* CTFS node receiver */
{
    char *cp, *nm, tbuf[32];
    READLEN_T sz;

    switch (ty) {
    case N_CTFSADIR:
        nm = "ADIR";
        sz = (READLEN_T)sizeof(ctfs_adirnode_t);
        break;
    case N_CTFSBUND:
        nm = "BUND";
        sz = (READLEN_T)sizeof(ctfs_bunode_t);
        break;
    case N_CTFSCDIR:
        nm = "CDIR";
        sz = (READLEN_T)sizeof(ctfs_cdirnode_t);
        break;
    case N_CTFSCTL:
        nm = "CTL";
        sz = (READLEN_T)sizeof(ctfs_ctlnode_t);
        break;
    case N_CTFSEVT:
        nm = "EVT";
        sz = (READLEN_T)sizeof(ctfs_evnode_t);
        break;
    case N_CTFSLATE:
        nm = "LATE";
        sz = (READLEN_T)sizeof(ctfs_latenode_t);
        break;
    case N_CTFSROOT:
        nm = "ROOT";
        sz = (READLEN_T)sizeof(ctfs_rootnode_t);
        break;
    case N_CTFSSTAT:
        nm = "STAT";
        sz = (READLEN_T)sizeof(ctfs_ctlnode_t);
        break;
    case N_CTFSSYM:
        nm = "SYM";
        sz = (READLEN_T)sizeof(ctfs_symnode_t);
        break;
    case N_CTFSTDIR:
        nm = "TDIR";
        sz = (READLEN_T)sizeof(ctfs_tdirnode_t);
        break;
    case N_CTFSTMPL:
        nm = "TMPL";
        sz = (READLEN_T)sizeof(ctfs_tmplnode_t);
        break;
    default:
        (void)snpf(Namech, Namechl - 1, "unknown CTFS node type: %d", ty);
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    if (!ca || kread(ctx, (KA_T)ca, cn, sz)) {
        (void)snpf(Namech, Namechl - 1,
                   "node at %s: can't read CTFS %s node: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)), nm,
                   print_kptr(ca, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=100000 */

/*
 * read_nfn() - read node's fifonode
 */

static int read_nfn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T fa,                  /* fifonode address */
                    struct fifonode *f)       /* fifonode receiver */
{
    char tbuf[32];

    if (!fa || readfifonode(ctx, fa, f)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read fifonode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(fa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

/*
 * read_nhn() - read node's High Sierra node
 */

static int read_nhn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T ha,                  /* hsnode address */
                    struct hsnode *h)         /* hsnode receiver */
{
    char tbuf[32];

    if (!ha || readhsnode(ctx, ha, h)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read hsnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ha, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

/*
 * read_nin() - read node's inode
 */

static int read_nin(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T ia,                  /* kernel inode address */
                    struct inode *i)          /* inode receiver */
{
    char tbuf[32];

    if (!ia || readinode(ctx, ia, i)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read inode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ia, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

/*
 * read_nln(na, la, ln) - read node's loopback node
 */

static int read_nln(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T la,                  /* loopback node address */
                    struct lnode *ln)         /* loopback node receiver */
{
    char tbuf[32];

    if (!la || kread(ctx, (KA_T)la, (char *)ln, sizeof(struct lnode))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read lnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(la, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

/*
 * read_nnn() - read node's namenode
 */

static int read_nnn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T nna,                 /* namenode address */
                    struct namenode *nn)      /* namenode receiver */
{
    char tbuf[32];

    if (!nna || kread(ctx, (KA_T)nna, (char *)nn, sizeof(struct namenode))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read namenode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(nna, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

/*
 * read_nmn() - read node's mvfsnode
 */

static int read_nmn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T ma,                  /* kernel mvfsnode address */
                    struct mvfsnode *m)       /* mvfsnode receiver */
{
    char tbuf[32];

    if (!ma || kread(ctx, (KA_T)ma, (char *)m, sizeof(struct mvfsnode))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read mvfsnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ma, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if defined(HASPROCFS)
/*
 * read_npi() - read node's /proc file system information
 */

static int read_npi(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    struct vnode *v,          /* containing vnode */
                    struct pid *pids)         /* pid structure receiver */
{
    struct as as;
    struct proc p;
    struct prnode pr;
    char tbuf[32];

#    if solaris >= 20600
    prcommon_t pc, ppc;
    int pcs, ppcs, prpcs, prppcs;
    struct proc pp;
    kthread_t thread;
    pid_t prpid;
    id_t prtid;
    char *ty = (char *)NULL;
#    endif /* solaris>=20600 */

    if (!v->v_data || kread(ctx, (KA_T)v->v_data, (char *)&pr, sizeof(pr))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read prnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr((KA_T)v->v_data, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }

#    if solaris < 20600
    /*
     * For Solaris < 2.6:
     *	* Read the proc structure, get the process size and PID;
     *	* Return the PID;
     *	* Enter a name, constructed from the file system and PID;
     *	* Enter an inode number, constructed from the PID.
     */
    if (!pr.pr_proc) {
        if (v->v_type == VDIR) {
            (void)snpf(Namech, Namechl - 1, "/%s", HASPROCFS);
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            Lf->inode = (INODETYPE)PR_ROOTINO;
            Lf->inp_ty = 1;
        } else {
            (void)snpf(Namech, Namechl - 1, "/%s/", HASPROCFS);
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            Lf->inp_ty = 0;
        }
        return (0);
    }
    if (kread(ctx, (KA_T)pr.pr_proc, (char *)&p, sizeof(p))) {
        (void)snpf(Namech, Namechl - 1, "prnode at %s: can't read proc: %s",
                   print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
                   print_kptr((KA_T)pr.pr_proc, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    if (p.p_as && !kread(ctx, (KA_T)p.p_as, (char *)&as, sizeof(as))) {
        Lf->sz = (SZOFFTYPE)as.a_size;
        Lf->sz_def = 1;
    }
    if (!p.p_pidp ||
        kread(ctx, (KA_T)p.p_pidp, (char *)pids, sizeof(struct pid))) {
        (void)snpf(Namech, Namechl - 1, "proc struct at %s: can't read pid: %s",
                   print_kptr((KA_T)pr.pr_proc, tbuf, sizeof(tbuf)),
                   print_kptr((KA_T)p.p_pidp, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    (void)snpf(Namech, Namechl, "/%s/%d", HASPROCFS, (int)pids->pid_id);
    Namech[Namechl - 1] = '\0';
    Lf->inode = (INODETYPE)ptoi(pids->pid_id);
    Lf->inp_ty = 1;
#    else /* solaris>=20600 */
    /*
     * Enter the >= Solaris 2.6 inode number.
     */
    Lf->inode = (INODETYPE)pr.pr_ino;
    Lf->inp_ty = 1;
    /*
     * Read the >= Solaris 2.6 prnode common structures.
     *
     * Return the PID number.
     *
     * Identify the lwp PID (the thread ID).
     */
    if (pr.pr_common &&
        kread(ctx, (KA_T)pr.pr_common, (char *)&pc, sizeof(pc)) == 0) {
        pcs = 1;
        if (pc.prc_proc &&
            kread(ctx, (KA_T)pc.prc_proc, (char *)&p, sizeof(p)) == 0)
            prpcs = 1;
        else
            prpcs = 0;
    } else
        pcs = prpcs = 0;
    if (pr.pr_pcommon &&
        kread(ctx, (KA_T)pr.pr_pcommon, (char *)&ppc, sizeof(ppc)) == 0) {
        ppcs = 1;
        if (ppc.prc_proc &&
            kread(ctx, (KA_T)ppc.prc_proc, (char *)&pp, sizeof(pp)) == 0)
            prppcs = 1;
        else
            prppcs = 0;
    } else
        ppcs = prppcs = 0;
    if (prpcs && p.p_pidp &&
        kread(ctx, (KA_T)p.p_pidp, (char *)pids, sizeof(struct pid)) == 0)
        prpid = pids->pid_id;
    else if (prppcs && pp.p_pidp &&
             kread(ctx, (KA_T)pp.p_pidp, (char *)pids, sizeof(struct pid)) == 0)
        prpid = pids->pid_id;
    else
        pids->pid_id = prpid = (pid_t)0;
    if (pcs && pc.prc_thread &&
        kread(ctx, (KA_T)pc.prc_thread, (char *)&thread, sizeof(kthread_t)) ==
            0)
        prtid = thread.t_tid;
    else if (ppcs && ppc.prc_thread &&
             kread(ctx, (KA_T)ppc.prc_thread, (char *)&thread,
                   sizeof(kthread_t)) == 0)
        prtid = thread.t_tid;
    else
        prtid = (id_t)0;
    /*
     * Identify the Solaris 2.6 /proc file system name, file size, and file
     * type.
     */
    switch (pr.pr_type) {
    case PR_PROCDIR:
        (void)snpf(Namech, Namechl - 1, "/%s", HASPROCFS);
        Lf->type = LSOF_FILE_PROC_DIR;
        break;
    case PR_PIDDIR:
        (void)snpf(Namech, Namechl - 1, "/%s/%d", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_DIR;
        break;
    case PR_AS:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/as", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_AS;
        if (prpcs &&
            kread(ctx, (KA_T)pc.prc_proc, (char *)&p, sizeof(p)) == 0 &&
            p.p_as && kread(ctx, (KA_T)p.p_as, (char *)&as, sizeof(as)) == 0) {
            Lf->sz = (SZOFFTYPE)as.a_size;
            Lf->sz_def = 1;
        }
        break;
    case PR_CTL:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/ctl", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_CTRL;
        break;
    case PR_STATUS:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/status", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_STATUS;
        break;
    case PR_LSTATUS:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lstatus", HASPROCFS,
                   (int)prpid);
        Lf->type = LSOF_FILE_PROC_LSTATUS;
        break;
    case PR_PSINFO:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/psinfo", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_PSINFO;
        break;
    case PR_LPSINFO:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lpsinfo", HASPROCFS,
                   (int)prpid);
        Lf->type = LSOF_FILE_PROC_LPS_INFO;
        break;
    case PR_MAP:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/map", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_MAP;
        break;
    case PR_RMAP:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/rmap", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_RMAP;
        break;
    case PR_XMAP:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/xmap", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_XMAP;
        break;
    case PR_CRED:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/cred", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_CRED;
        break;
    case PR_SIGACT:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/sigact", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_SIGACT;
        break;
    case PR_AUXV:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/auxv", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_AUXV;
        break;

#        if defined(HASPR_LDT)
    case PR_LDT:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/ldt", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_LDT;
        break;
#        endif /* defined(HASPR_LDT) */

    case PR_USAGE:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/usage", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_USAGE;
        break;
    case PR_LUSAGE:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lusage", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_LUSAGE;
        break;
    case PR_PAGEDATA:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/pagedata", HASPROCFS,
                   (int)prpid);
        Lf->type = LSOF_FILE_PROC_PAGE_DATA;
        break;
    case PR_WATCH:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/watch", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_WATCH;
        break;
    case PR_CURDIR:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/cwd", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_CWD;
        break;
    case PR_ROOTDIR:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/root", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_ROOT;
        break;
    case PR_FDDIR:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/fd", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_FD_DIR;
        break;
    case PR_FD:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/fd/%d", HASPROCFS, (int)prpid,
                   pr.pr_index);
        Lf->type = LSOF_FILE_PROC_FD;
        break;
    case PR_OBJECTDIR:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/object", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_OBJ_DIR;
        break;
    case PR_OBJECT:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/object/", HASPROCFS,
                   (int)prpid);
        Lf->type = LSOF_FILE_PROC_OBJ;
        break;
    case PR_LWPDIR:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lpw", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_LWP_DIR;
        break;
    case PR_LWPIDDIR:
        (void)snpf(Namech, Namechl, "/%s/%d/lwp/%d", HASPROCFS, (int)prpid,
                   (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_DIR;
        break;
    case PR_LWPCTL:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lwp/%d/lwpctl", HASPROCFS,
                   (int)prpid, (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_CTL;
        break;
    case PR_LWPSTATUS:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lwp/%d/lwpstatus", HASPROCFS,
                   (int)prpid, (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_STATUS;
        break;
    case PR_LWPSINFO:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lwp/%d/lwpsinfo", HASPROCFS,
                   (int)prpid, (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_SINFO;
        break;
    case PR_LWPUSAGE:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lwp/%d/lwpusage", HASPROCFS,
                   (int)prpid, (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_USAGE;
        break;
    case PR_XREGS:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lwp/%d/xregs", HASPROCFS,
                   (int)prpid, (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_XREGS;
        break;

#        if defined(HASPR_GWINDOWS)
    case PR_GWINDOWS:
        (void)snpf(Namech, Namechl - 1, "/%s/%d/lwp/%d/gwindows", HASPROCFS,
                   (int)prpid, (int)prtid);
        Lf->type = LSOF_FILE_PROC_LWP_GWINDOWS;
        break;
#        endif /* defined(HASPR_GWINDOWS) */

#        if defined(PR_PIDFILE)
    case PR_PIDFILE:
        (void)snpf(Namech, Namechl - 1, "/%s/%d", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_OLD_PID;
        break;
#        endif /* defined(PR_PIDFILE) */

#        if defined(PR_LWPIDFILE)
    case PR_LWPIDFILE:
        (void)snpf(Namech, Namechl - 1, "/%s/%d", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_OLD_LWP;
        break;
#        endif /* defined(PR_LWPIDFILE) */

    case PR_OPAGEDATA:
        (void)snpf(Namech, Namechl - 1, "/%s/%d", HASPROCFS, (int)prpid);
        Lf->type = LSOF_FILE_PROC_OLD_PAGE;
        break;
    default:
        Lf->type = LSOF_FILE_UNKNOWN_RAW;
        Lf->unknown_file_type_number = pr.pr_type;
    }
    /*
     * Record the Solaris 2.6 /proc file system inode number.
     */
    Lf->inode = (INODETYPE)pr.pr_ino;
    Lf->inp_ty = 1;
#    endif     /* solaris<20600 */

    Namech[Namechl - 1] = '\0';
    enter_nm(ctx, Namech);
    return (0);
}
#endif /* defined(HASPROCFS) */

/*
 * read_npn() - read node's pcnode
 */

static int read_npn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T pa,                  /* pcnode address */
                    struct pcnode *p)         /* pcnode receiver */
{
    char tbuf[32];

    if (!pa || kread(ctx, pa, (char *)p, sizeof(struct pcnode))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read pcnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(pa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if solaris >= 100000
/*
 * read_nprtn() - read node's port node
 */

static int read_nprtn(struct lsof_context *ctx, /* context */
                      KA_T na,                  /* containing node's address */
                      KA_T pa,                  /* port node address */
                      port_t *p)                /* port node receiver */
{
    char tbuf[32];

    if (!pa || kread(ctx, pa, (char *)p, sizeof(port_t))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read port node: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(pa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=100000 */

/*
 * read_nrn() - read node's rnode
 */

static int read_nrn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T ra,                  /* rnode address */
                    struct rnode *r)          /* rnode receiver */
{
    char tbuf[32];

    if (!ra || readrnode(ctx, ra, r)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read rnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ra, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if solaris >= 100000
/*
 * read_nrn4() - read node's rnode4
 */

static int read_nrn4(struct lsof_context *ctx, /* context */
                     KA_T na,                  /* containing node's address */
                     KA_T ra,                  /* rnode address */
                     struct rnode4 *r)         /* rnode receiver */
{
    char tbuf[32];

    if (!ra || kread(ctx, (KA_T)ra, (char *)r, sizeof(struct rnode4))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read rnode4: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ra, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=100000 */

#if solaris >= 110000
/*
 * read_nsdn() - read node's sdev_node
 */

static int read_nsdn(struct lsof_context *ctx, /* context */
                     KA_T na,                  /* containing node's address */
                     KA_T sa,                  /* sdev_node address */
                     struct sdev_node *sdn,    /* sdev_node receiver */
                     struct vattr *sdva)       /* sdev_node's vattr receiver */
{
    KA_T va;
    char tbuf[32], tbuf1[32];

    if (!sa || kread(ctx, (KA_T)sa, (char *)sdn, sizeof(struct sdev_node))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read sdev_node: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(sa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    if (!(va = (KA_T)sdn->sdev_attr) ||
        kread(ctx, va, (char *)sdva, sizeof(struct vattr))) {
        (void)snpf(Namech, Namechl - 1,
                   "node at %s; sdev_node at %s: can't read vattr: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(sa, tbuf1, sizeof(tbuf1)),
                   print_kptr(va, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=110000 */

#if solaris >= 20600
/*
 * read_nson() - read node's sonode
 */

static int read_nson(struct lsof_context *ctx, /* context */
                     KA_T na,                  /* containing node's address */
                     KA_T sa,                  /* sonode address */
                     struct sonode *sn)        /* sonode receiver */

{
    char tbuf[32];

    if (!sa || kread(ctx, (KA_T)sa, (char *)sn, sizeof(struct sonode))) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read sonode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(sa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=20600 */

/*
 * read_nsn() - read node's snode
 */

static int read_nsn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T sa,                  /* snode address */
                    struct snode *s)          /* snode receiver */
{
    char tbuf[32];

    if (!sa || readsnode(ctx, sa, s)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read snode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(sa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if solaris >= 110000
/*
 * read_nsti() - read socket node's info
 */

static int read_nsti(struct lsof_context *ctx, /* context */
                     struct sonode *so,        /* socket's sonode */
                     sotpi_info_t *stpi)       /* local socket info receiver */
{
    char tbuf[32];

    (void)CTF_init(ctx, &Sockfs_ctfs, SOCKFS_MOD_FORMAT, Sockfs_requests);
    if (!so || !so->so_priv ||
        CTF_MEMBER_READ(so->so_priv, stpi, sotpi_info_members, sti_dev) ||
        CTF_MEMBER_READ(so->so_priv, stpi, sotpi_info_members, sti_laddr) ||
        CTF_MEMBER_READ(so->so_priv, stpi, sotpi_info_members, sti_faddr) ||
        CTF_MEMBER_READ(so->so_priv, stpi, sotpi_info_members, sti_ux_laddr) ||
        CTF_MEMBER_READ(so->so_priv, stpi, sotpi_info_members, sti_ux_faddr) ||
        CTF_MEMBER_READ(so->so_priv, stpi, sotpi_info_members, sti_serv_type)) {
        (void)snpf(Namech, Namechl - 1, "sonode at %s: can't read so_priv: %s",
                   print_kptr((KA_T)so, tbuf, sizeof(tbuf)),
                   print_kptr((KA_T)so->so_priv, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* solaris>=110000 */

/*
 * read_ntn() - read node's tmpnode
 */

static int read_ntn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T ta,                  /* tmpnode address */
                    struct tmpnode *t)        /* tmpnode receiver */
{
    char tbuf[32];

    if (!ta || readtnode(ctx, ta, t)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read tnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(ta, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if solaris >= 20600
/*
 * read_nusa() - read sondode's UNIX socket address
 */

static int read_nusa(struct lsof_context *ctx, /* context */
                     struct soaddr *so,      /* kernel socket info structure */
                     struct sockaddr_un *ua) /* local sockaddr_un address */
{
    KA_T a;
    int len;
    int min = offsetof(struct sockaddr_un, sun_path);

    ua->sun_path[0] = '\0';

    if (!(a = (KA_T)so->soa_sa) || (len = so->soa_len) < (min + 2) ||
        len > (int)sizeof(struct sockaddr_un) ||
        kread(ctx, a, (char *)ua, len) || ua->sun_family != AF_UNIX)
        return (0);
    len -= min;
    if (len >= sizeof(ua->sun_path))
        len = sizeof(ua->sun_path) - 1;
    ua->sun_path[len] = '\0';
    return ((int)strlen(ua->sun_path));
}
#endif /* solaris>=20600 */

/*
 * read_nvn() - read node's vnode
 */

static int read_nvn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* node's address */
                    KA_T va,                  /* vnode address */
                    struct vnode *v)          /* vnode receiver */
{
    char tbuf[32];

    if (readvnode(ctx, va, v)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read real vnode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(va, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

#if defined(HAS_ZFS)
/*
 * read_nzn() - read node's ZFS node
 */

static int read_nzn(struct lsof_context *ctx, /* context */
                    KA_T na,                  /* containing node's address */
                    KA_T nza,                 /* znode address */
                    znode_t *zn)              /* znode receiver */
{
    int err = 0;      /* error flag */
    CTF_member_t *mp; /* member pointer */
    char tbuf[32];    /* temporary buffer */
    znode_phys_t zp;  /* physical znode */

    (void)CTF_init(ctx, &ZFS_ctfs, ZFS_MOD_FORMAT, ZFS_requests);
    if (!nza || CTF_MEMBER_READ(nza, zn, znode_members, z_zfsvfs) ||
        CTF_MEMBER_READ(nza, zn, znode_members, z_vnode) ||
        CTF_MEMBER_READ(nza, zn, znode_members, z_id) ||
        CTF_MEMBER_READ(nza, zn, znode_members, z_phys) ||
        CTF_MEMBER_READ(nza, zn, znode_members, z_links) ||
        CTF_MEMBER_READ(nza, zn, znode_members, z_size)) {
        (void)snpf(Namech, Namechl - 1, "node at %s: can't read znode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(nza, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    /*
     * If the physical znode pointer is defined, read the physizal znode
     * and propagate its values to the znode.
     */
    if (znode_members[MX_z_phys].m_offset != CTF_MEMBER_UNDEF) {
        err = read_nznp(ctx, nza, (KA_T)zn->z_phys, &zp);
        if (!err) {
            zn->z_links = zp.zp_links;
            zn->z_size = zp.zp_size;
        }
    } else {

        /*
         * Make sure z_link and z_size are defined when z_phys isn't.
         */
        if (znode_members[MX_z_links].m_offset == CTF_MEMBER_UNDEF) {
            (void)snpf(Namech, Namechl - 1,
                       "node at %s: can't read z_links: %s",
                       print_kptr(na, tbuf, sizeof(tbuf)),
                       print_kptr(nza, (char *)NULL, 0));
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            err = 1;
        }
        if (znode_members[MX_z_size].m_offset == CTF_MEMBER_UNDEF) {
            (void)snpf(Namech, Namechl - 1, "node at %s: can't read z_size: %s",
                       print_kptr(na, tbuf, sizeof(tbuf)),
                       print_kptr(nza, (char *)NULL, 0));
            Namech[Namechl - 1] = '\0';
            enter_nm(ctx, Namech);
            err = 1;
        }
    }
    return (err);
}

/*
 * read_nznp() - read znode's persistent znode
 */

static int read_nznp(struct lsof_context *ctx, /* context */
                     KA_T nza,                 /* containing znode's address */
                     KA_T nzpa,                /* persistent znode address */
                     znode_phys_t *zp)         /* persistent znode receiver */
{
    char tbuf[32];

    (void)CTF_init(ctx, &ZFS_ctfs, ZFS_MOD_FORMAT, ZFS_requests);
    if (!nzpa || CTF_MEMBER_READ(nzpa, zp, znode_phys_members, zp_size) ||
        CTF_MEMBER_READ(nzpa, zp, znode_phys_members, zp_links)) {
        (void)snpf(Namech, Namechl - 1,
                   "znode at %s: "
                   "can't read znode_phys: %s",
                   print_kptr(nza, tbuf, sizeof(tbuf)),
                   print_kptr(nzpa, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}

/*
 * read_nzvfs() - read znode's associated vfs
 */

static int read_nzvfs(struct lsof_context *ctx, /* context */
                      KA_T nza,                 /* containing znode's address */
                      KA_T nzva,                /* associated vfs address */
                      zfsvfs_t *zv)             /* associated vfs receiver */
{
    char tbuf[32];

    (void)CTF_init(ctx, &ZFS_ctfs, ZFS_MOD_FORMAT, ZFS_requests);
    if (!nzva || CTF_MEMBER_READ(nzva, zv, zfsvfs_members, z_vfs)) {
        (void)snpf(Namech, Namechl - 1, "znode at %s: can't read zfsvfs: %s",
                   print_kptr(nza, tbuf, sizeof(tbuf)),
                   print_kptr(nzva, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    return (0);
}
#endif /* defined(HAS_ZFS) */

#if solaris < 100000
/*
 * savesockmod() - save addresses from sockmod so_so structure
 */

static void
savesockmod(struct so_so *so,  /* new so_so structure pointer */
            struct so_so *sop, /* previous so_so structure pointer */
            int *so_st)        /* status of *sop (0 if not loaded) */
{

#    if solaris < 20500
    dev_t d1, d2, d3;
#    endif /* solaris<20500 */

#    define luxadr lux_dev.addr.tu_addr
#    define luxdev lux_dev.addr.tu_addr.dev
#    define luxino lux_dev.addr.tu_addr.ino
#    define ruxadr rux_dev.addr.tu_addr
#    define ruxdev rux_dev.addr.tu_addr.dev
#    define ruxino rux_dev.addr.tu_addr.ino

#    if solaris < 20500
    /*
     * If either address in the new structure is missing a device number, clear
     * its corresponding inode number.  Then sort the inode-less device numbers.
     */
    if (!so->luxdev)
        so->luxino = (ino_t)0;
    if (!so->ruxdev)
        so->ruxino = (ino_t)0;
    if (!so->luxino && !so->ruxino) {
        if (so->luxdev > so->ruxdev) {
            d2 = so->luxdev;
            d1 = so->luxdev = so->ruxdev;
            so->ruxdev = d2;
        } else {
            d1 = so->luxdev;
            d2 = so->ruxdev;
        }
    } else
        d1 = d2 = (dev_t)0;
    /*
     * If the previous structure hasn't been loaded, save the new one in it with
     * adjusted or sorted addresses.
     */
    if (!*so_st) {
        if (so->luxdev && so->luxino) {
            *sop = *so;
            sop->ruxdev = (dev_t)0;
            sop->ruxino = (ino_t)0;
            *so_st = 1;
            return;
        }
        if (so->ruxdev && so->ruxino) {
            *sop = *so;
            sop->luxadr = sop->ruxadr;
            sop->ruxdev = (dev_t)0;
            sop->ruxino = (ino_t)0;
            *so_st = 1;
            return;
        }
        *sop = *so;
        *so_st = 1;
        return;
    }
    /*
     * See if the new sockmod addresses need to be merged with the previous
     * ones:
     *
     *	*  Don't merge if the previous so_so structure's lux_dev has a non-
     *	   zero device and a non-zero inode number.
     *
     *	*  If either of the device/inode pairs in the new structure is non-
     *	   zero, propagate them to the previous so_so structure.
     *
     *	*  Don't merge if the both device numbers in the new structure are
     *	   zero.
     */
    if (sop->luxdev && sop->luxino)
        return;
    if (so->luxdev && so->luxino) {
        sop->luxadr = so->luxadr;
        sop->ruxdev = (dev_t)0;
        sop->ruxino = (ino_t)0;
        return;
    }
    if (so->ruxdev && so->ruxino) {
        sop->luxadr = so->ruxadr;
        sop->ruxdev = (dev_t)0;
        sop->ruxino = (ino_t)0;
        return;
    }
    if (!so->luxdev && !so->ruxdev)
        return;
    /*
     * Check the previous structure's device numbers:
     *
     *	*  If both are zero, replace the previous structure with the new one.
     *
     *	*  Choose the minimum and maximum non-zero device numbers contained in
     *	   either structure.
     */
    if (!sop->luxdev && !sop->ruxdev) {
        *sop = *so;
        return;
    }
    if (!sop->luxdev && (d1 || d2)) {
        if (d1) {
            sop->luxdev = d1;
            d1 = (dev_t)0;
        } else {
            sop->luxdev = d2;
            d2 = (dev_t)0;
        }
        if (sop->luxdev > sop->ruxdev) {
            d3 = sop->luxdev;
            sop->luxdev = sop->ruxdev;
            sop->ruxdev = d3;
        }
    }
    if (!sop->ruxdev && (d1 || d2)) {
        if (d1) {
            sop->ruxdev = d1;
            d1 = (dev_t)0;
        } else {
            sop->ruxdev = d2;
            d2 = (dev_t)0;
        }
        if (sop->luxdev > sop->ruxdev) {
            d3 = sop->luxdev;
            sop->luxdev = sop->ruxdev;
            sop->ruxdev = d3;
        }
    }
    if (sop->luxdev && sop->ruxdev) {
        if (d1) {
            if (d1 < sop->luxdev)
                sop->luxdev = d1;
            else if (d1 > sop->ruxdev)
                sop->ruxdev = d1;
        }
        if (d2) {
            if (d2 < sop->luxdev)
                sop->luxdev = d2;
            else if (d2 > sop->ruxdev)
                sop->ruxdev = d2;
        }
    }
#    else  /* solaris>=20500 */
    /*
     * Save the first sockmod structure.
     */
    if (!*so_st) {
        *so_st = 1;
        *sop = *so;
    }
#    endif /* solaris<20500 */
}
#endif /* solaris<100000 */

/*
 * vop2ty() - convert vnode operation switch address to internal type
 */

int vop2ty(struct lsof_context *ctx, /* context */
           struct vnode *vp,         /* local vnode pointer */
           int fx)                   /* file system index (-1 if none) */
{
    int h;
    register int i;
    KA_T ka;
    int nty;
    v_optab_t *nv, *v, *vt;

#if defined(HAS_AFS)
    static int afs = 0; /* afs test status: -1 = no AFS
                         *		     0 = not tested
                         *		     1 = AFS */
#endif                  /* defined(HAS_AFS) */

    /*
     * Locate the node type by hashing the vnode's v_op address into the
     * Voptab[].
     */
    if (!(ka = (KA_T)vp->v_op))
        return (-1);
    h = HASHVOP(ka);
    for (v = Voptab[h]; v; v = v->next) {
        if (ka == v->v_op)
            break;
    }
    if (!v) {

        /*
         * If there's no entry in the Voptab[] for the v_op address, see if
         * an entry can be found via the file system type and FxToVoptab[].
         */
        if ((fx >= 0) && (fx < Fsinfomax) && (v = FxToVoptab[fx])) {

            /*
             * There's an FxToVoptab[] mapping, so add an entry to Voptab[]
             * for the v_op address.
             */
            if (!(nv = (v_optab_t *)malloc((MALLOC_S)sizeof(v_optab_t)))) {
                (void)fprintf(stderr, "%s: can't add \"%s\" to Voptab\n", Pn,
                              Fsinfo[fx]);
                Error(ctx);
            }
            *nv = *v;
            nv->v_op = ka;
            h = HASHVOP(ka);
            nv->next = Voptab[h];
            Voptab[h] = v = nv;
        }
    }
    if (!v)
        return (-1);

#if defined(HAS_AFS)
    /*
     * Do special AFS checks.
     */
    if (v->nty == N_AFS) {
        if (vp->v_data || !vp->v_vfsp)
            return (-1);
        switch (afs) {
        case -1:
            return (-1);
        case 0:
            if (!hasAFS(vp)) {
                afs = -1;
                return (-1);
            }
            afs = 1;
            return (N_AFS);
        case 1:
            if ((KA_T)vp->v_vfsp == AFSVfsp)
                return (N_AFS);
        }
        return (-1);
    }
#endif /* defined(HAS_AFS) */

    return (v->nty);
}

#if solaris >= 100000
/*
 * read_ndvn() -- read node's dv_node
 */

static int read_ndvn(struct lsof_context *ctx, /* context */
                     KA_T na,                  /* containing vnode's address */
                     KA_T da,                  /* containing vnode's v_data */
                     struct dv_node *dv,       /* dv_node receiver */
                     dev_t *dev,               /* underlying file system device
                                                * number receptor */
                     unsigned char *devs)      /* status of *dev */
{
    struct vnode rv;
    struct snode s;
    char tbuf[32];
    struct vfs v;
    /*
     * Read the snode.
     */
    if (!da || kread(ctx, (KA_T)da, (char *)&s, sizeof(s))) {
        (void)snpf(Namech, Namechl - 1,
                   "dv_node vnode at %s: can't read snode: %s",
                   print_kptr(na, tbuf, sizeof(tbuf)),
                   print_kptr(da, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    /*
     * Read the snode's real vnode.
     */
    if (!s.s_realvp ||
        kread(ctx, (KA_T)s.s_realvp, (char *)&rv, sizeof(struct dv_node))) {
        (void)snpf(Namech, Namechl - 1,
                   "dv_node snode at %s: can't read real vnode: %s",
                   print_kptr(da, tbuf, sizeof(tbuf)),
                   print_kptr((KA_T)s.s_realvp, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    /*
     * Read the real vnode's dv_node.
     */
    if (!rv.v_data || kread(ctx, (KA_T)rv.v_data, (char *)dv, sizeof(rv))) {
        (void)snpf(Namech, Namechl - 1,
                   "dv_node real vnode at %s: can't read dv_node: %s",
                   print_kptr((KA_T)s.s_realvp, tbuf, sizeof(tbuf)),
                   print_kptr((KA_T)rv.v_data, (char *)NULL, 0));
        Namech[Namechl - 1] = '\0';
        enter_nm(ctx, Namech);
        return (1);
    }
    /*
     * Return the device number of the underlying file system, if possible.
     */
    if (rv.v_vfsp && !kread(ctx, (KA_T)rv.v_vfsp, (char *)&v, sizeof(v))) {
        *dev = v.vfs_dev;
        *devs = 1;
    }
    return (0);
}
#endif /* solaris<100000 */
