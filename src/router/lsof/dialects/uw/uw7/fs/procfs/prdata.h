#ifndef _FS_PROCFS_PRDATA_H     /* wrapper symbol for kernel use */
#define _FS_PROCFS_PRDATA_H     /* subject to change without notice */

#ident  "@(#)kern:fs/procfs/prdata.h    1.19.2.1"
#ident  "$Header: $"

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _KERNEL_HEADERS

#include <fs/vnode.h>           /* SVR4.2COMPAT */
#include <io/uio.h>             /* SVR4.0COMPAT */
#include <mem/seg.h>            /* SVR4.0COMPAT */
#include <proc/proc.h>          /* REQUIRED */
#include <util/ksynch.h>        /* REQUIRED */
#include <util/types.h>         /* REQUIRED */

#elif defined(_KERNEL) 

#include <sys/vnode.h>          /* SVR4.2COMPAT */
#include <sys/uio.h>            /* SVR4.0COMPAT */
#include <vm/seg.h>             /* SVR4.0COMPAT */
#include <sys/proc.h>           /* REQUIRED */
#include <sys/ksynch.h>         /* REQUIRED */
#include <sys/types.h>          /* REQUIRED */

#endif /* _KERNEL_HEADERS */

#if defined(_KERNEL) || defined(_KMEMUSER)

/*
 * Common file object to which all /proc vnodes for a specific process
 * or LWP refer.
 */
typedef struct prcommon {
        lock_t  prc_mutex;      /* Lock for prc_flags and prc_rdwriters */
        uchar_t prc_flags;      /* flags */
        ulong_t prc_rdwriters;  /* # holding or queued for process r/w lock 



*/
        sv_t    prc_rdwrdone;   /* sync object to wait for r/w refs done */
        sv_t    prc_stopsv;     /* sync object to wait for proc or LWP stop 



*/
        struct pollhead *prc_pollhead;  /* Pointer to list of pollers */
        pid_t   prc_pid;        /* process-ID */
        uint_t  prc_opens;      /* Number of opens of prnodes */
        uint_t  prc_writers;    /* Number of write opens of prnodes */
        proc_t  *prc_proc;      /* Associated process */
        lwp_t   *prc_lwp;       /* Associated LWP */
        lwpid_t prc_lwpid;      /* LWP id */
        int     prc_slot;       /* Associated process slot number */
} prcommon_t;

/* prc_flags */
#define PRC_DESTROY     0x01    /* process or LWP is being destroyed */
#define PRC_LWP         0x02    /* structure refers to an LWP */
#define PRC_SYS         0x04    /* process is a system process */

/*
 * prc_mutex:   Per prcommon structure lock protecting fields within
 *              a prcommon structure.
 */
extern lkinfo_t prc_mutex_lkinfo;

/*
 * Node types for /proc files (directories and files contained therein).
 */
typedef enum prnodetype {
        PR_PROCDIR,             /* 00 /proc */
        PR_PIDDIR,              /* 01 /proc/pid */
        PR_AS,                  /* 02 /proc/pid/as */
        PR_CTL,                 /* 03 /proc/pid/ctl */
        PR_STATUS,              /* 04 /proc/pid/status */
        PR_PSINFO,              /* 05 /proc/pid/psinfo */
        PR_MAP,                 /* 06 /proc/pid/map */
        PR_CRED,                /* 07 /proc/pid/cred */
        PR_SIGACT,              /* 08 /proc/pid/sigact */
        PR_OBJECTDIR,           /* 09 /proc/pid/object */
        PR_LWPDIR,              /* 0A /proc/pid/lwp */
        PR_LWPIDDIR,            /* 0B /proc/pid/lwp/lwpid */
        PR_LWPCTL,              /* 0C /proc/pid/lwp/lwpid/lwpctl */
        PR_LWPSTATUS,           /* 0D /proc/pid/lwp/lwpid/lwpstatus */
        PR_LWPSINFO             /* 0F /proc/pid/lwp/lwpid/lwpsinfo */
} prnodetype_t;

typedef struct prnode {
        lock_t          pr_mutex;       /* Locks pr_flags and child 
pr_files */
        prnodetype_t    pr_type;        /* Node type */
        mode_t          pr_mode;        /* File mode */
        ino_t           pr_ino;         /* Node id (for stat(2)) */
        ulong_t         pr_flags;       /* Private flags */
        prcommon_t      *pr_common;     /* common data structure */
        prcommon_t      *pr_pcommon;    /* process common data structure */
        struct vnode    *pr_parent;     /* Parent directory */
        struct vnode    **pr_files;     /* Contained files (for directory) 
*/
        struct vnode    *pr_next;       /* List in chain of (invalid) 
vnodes */
        uint_t          pr_index;       /* Position within parent */
        struct vnode    pr_vnode;       /* Embedded vnode */
} prnode_t;

/*
 * Values for pr_flags.
 */
#define PR_INVAL        0x01            /* Vnode is invalidated */


#define VTOP(vp)        ((prnode_t *)((vp)->v_data))
#define PTOV(pnp)       (&(pnp)->pr_vnode)

/*
 * Tables used by prmakenode() in constructing a /proc vnode from
 * a <directory, name> pair.
 */
typedef struct prntable {
        char            *prn_comp;      /* Name within directory */
        prnodetype_t    prn_ctype;      /* Node type of result vnode */
        int             prn_zvisible;   /* Visibility if a zombie */
        int             prn_nasvisible; /* Visibility if no address space 
*/
        vtype_t         prn_ftype;      /* File type of result vnode */
        mode_t          prn_mode;       /* File mode of result vnode */
} prntable_t;


#endif /* _KERNEL || _KMEMUSER */


#ifdef _KERNEL

/*
 * pr_mutex:    Per-structure lock protecting the pr_files field
 *              and the pr_flags field of a child.
 */
extern lkinfo_t pr_mutex_lkinfo;

extern prntable_t       pdtable[], ldtable[];
extern int              npdent, nldent;

extern struct vnodeops prvnodeops;
extern struct vfs *procvfs;
extern dev_t procdev;

struct pstatus;
struct psinfo;
struct lwpstatus;
struct lwpsinfo;
struct pfamily;

extern ulong_t  prsize(prnode_t *);
extern void     prgetsigact(proc_t *, struct sigaction *);
extern void     prgetaction(proc_t *, int, struct sigaction *);
extern int      prusrio(proc_t *, enum uio_rw, struct uio *);
extern lwp_t    *prchoose(proc_t *);
extern greg_t   prgetpc(gregset_t);
extern int      prgetscall(gregset_t);
extern caddr_t  prgetpsaddr(lwp_t *);
extern void     prdebugon(lwp_t *);
extern void     prdebugoff(lwp_t *);
extern void     prgetpfamily(const user_t *up, struct pfamily *fp);
extern int      prwritectl_family(ulong_t, vnode_t *, uio_t *, int, cred_t 
*);
extern boolean_t pr_p_rdwr(prcommon_t *, boolean_t);
extern void     pr_v_rdwr(prcommon_t *);
extern boolean_t pr_p_mutex(prcommon_t *);
#ifdef DEBUG
extern int      prfilesempty(vnode_t **, int);
#endif
extern int      prvpsegs(proc_t *, vnode_t **);
extern void     prmapname(vnode_t *, char *, ino_t *, cred_t *);
extern int      prcountsegs(struct as *);
extern int      prnsegs(struct as *);
extern int      setisempty(ulong_t *, unsigned int);
extern int      prgetpstatus(proc_t *, struct pstatus *);
extern void     prgetpsinfo(proc_t *, struct psinfo *);
extern int      prgetlwpstatus(lwp_t *, struct lwpstatus *);
extern void     prgetlwpsinfo(lwp_t *, struct lwpsinfo *);
extern void     prchlvl(lid_t);

#define prino(slot, lwpid, type) (((lwpid)<<18) | ((slot)<<5) | type + 2)

/*
 * Is an LWP stopped on an event of interest to /proc?
 */
#define ISTOP(lwp)      ((lwp)->l_stat == SSTOP \
                          && ((lwp)->l_whystop == PR_REQUESTED \
                              || (lwp)->l_whystop == PR_SIGNALLED \
                              || (lwp)->l_whystop == PR_SYSENTRY \
                              || (lwp)->l_whystop == PR_SYSEXIT \
                              || (lwp)->l_whystop == PR_FAULTED))

/*
 * Assign one set to another (possible different sizes).
 *
 * Assigning to a smaller set causes members to be lost.
 * Assigning to a larger set causes extra members to be cleared.
 */
#define prassignset(ap, sp)                                     \
{                                                               \
        register int _i_ = sizeof(*(ap))/sizeof(ulong_t);       \
        while (--_i_ >= 0)                                      \
                ((ulong_t*)(ap))[_i_] =                         \
                  (_i_ >= sizeof(*(sp))/sizeof(ulong_t)) ?      \
                  0L : ((ulong_t*)(sp))[_i_];                   \
}

/*
 * Determine whether or not a set (of arbitrary size) is empty.
 */
#define prisempty(sp) setisempty((ulong_t *)(sp), 
sizeof(*(sp))/sizeof(ulong_t))

#endif /* _KERNEL */

#if defined(__cplusplus)
        }
#endif

#endif  /* _FS_PROCFS_PRDATA_H */
