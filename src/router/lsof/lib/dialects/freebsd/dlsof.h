/*
 * dlsof.h - FreeBSD header file for lsof
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

/*
 * $Id: dlsof.h,v 1.49 2018/07/14 12:14:55 abe Exp $
 */

#if !defined(FREEBSD_LSOF_H)
#    define FREEBSD_LSOF_H 1

#    include <stddef.h>
#    include <stdlib.h>
#    include <dirent.h>
#    include <nlist.h>
#    include <setjmp.h>
#    include <signal.h>
#    include <unistd.h>

#    if FREEBSDV >= 13000
/* This header is a huge mess.  Please don't support EOL FreeBSD releases. */
#        define _KERNEL 1
#        include <sys/_lock.h>
#        undef _KERNEL
#    endif /* FREEBSDV>=13000 */
#    define _KERNEL 1

#    if defined(HAS_VM_MEMATTR_T)
/*
 * The d_mmap2_t function typedef in <sys/conf.h> may need the definition
 * of vm_memattr_t for a pointer, but that definition is only available
 * under _KERNEL in <sys/types.h>.  Defining _KERNEL before including
 * <sys/types.h> causes many compilation problems, so this expediency
 * (hack) is used when the vm_memattr_t definition is needed.
 */
#        define vm_memattr_t void
#    endif /* defined(HAS_VM_MEMATTR_T) */

#    if defined(NEEDS_BOOLEAN_T)
/*
 * In FreeBSD 9 and above the boolean_t typedef is also needed and is also
 * under _KERNEL in <sys/types.h>.
 */

#        define boolean_t int
#    endif /* defined(NEEDS_BOOLEAN_T) */

#    if defined(NEEDS_DEVICE_T)
/*
 * In FreeBSD 12 <sys/conf.h calls <sys/eventhandler.h> and it needs
 * the device_t typedef.
 */
typedef struct device *device_t;
#    endif /* defined(NEEDS_DEVICE_T) */

/*
 * Define KLD_MODULE to avoid the error "ARM_NMMUS is 0" from ARM's
 * <machine/cpuconf.h>.
 */

#    define KLD_MODULE

/*
 * include <stdbool.h> for refount(9)
 */
#    include <stdbool.h>

#    include <sys/conf.h>

#    if defined(HAS_VM_MEMATTR_T)
#        undef vm_memattr_t
#    endif /* defined(HAS_VM_MEMATTR_T) */

#    if defined(NEEDS_BOOLEAN_T)
#        undef boolean_t
#    endif /* defined(NEEDS_BOOLEAN_T) */

#    if defined(HAS_CONF_MINOR)
#        undef minor
#        include "fbsd_minor.h"
#    endif /* defined(HAS_CONF_MINOR) */

#    undef _KERNEL

#    include <sys/filedesc.h>
#    include <sys/mbuf.h>
#    define NFS
#    define m_stat mnt_stat

struct statfs;
extern int statfs(const char *, struct statfs *);
#    define _KERNEL

#    include <sys/mount.h>

#    if defined(__clang__)
/*
 * This definition is needed when clang is used, because <sys/mount.h> must
 * be #include'd when _KERNEL is defined and that causes the getmntinfo()
 * function prototype to be skipped.
 */
int getmntinfo(struct statfs **, int);
#    endif /* defined(__clang__) */

#    undef _KERNEL

#    include <rpc/types.h>
#    include <sys/protosw.h>
#    include <sys/socket.h>
#    define _WANT_SOCKET
#    include <sys/socketvar.h>
#    include <sys/un.h>
#    define _WANT_UNPCB
#    include <sys/unpcb.h>

#    undef INADDR_LOOPBACK

#    include <sys/callout.h>
#    include <netinet/in.h>
#    include <netinet/in_systm.h>
#    include <netinet/ip.h>
#    include <net/route.h>
#    define _WANT_INPCB /* for FreeBSD 12 and above */
#    include <netinet/in_pcb.h>
#    include <netinet/ip_var.h>
#    include <netinet/tcp.h>
#    include <netinet/tcpip.h>
#    include <netinet/tcp_fsm.h>
#    include <netinet/tcp_timer.h>
#    define _WANT_TCPCB /* for FreeBSD 12 and above */
#    include <netinet/tcp_var.h>
#    include <sys/ucred.h>
#    include <sys/uio.h>

#    if defined(HAS_KVM_VNODE)
#        define _KVM_VNODE
#    endif /* defined(HAS_KVM_VNODE) */
#    include <sys/vnode.h>
#    if defined(HAS_KVM_VNODE)
#        undef _KVM_VNODE
#    endif /* defined(HAS_KVM_VNODE) */

#    include <sys/domain.h>
#    define pmap RPC_pmap
#    include <rpc/rpc.h>
#    include <rpc/pmap_prot.h>
#    undef pmap

#    include <paths.h>
#    include <ufs/ufs/quota.h>

#    include <ufs/ufs/inode.h>

#    if defined(HAS_UFS1_2)
#        define _KERNEL
struct vop_getextattr_args;
struct vop_deleteextattr_args;
struct vop_setextattr_args;
#        include <ufs/ufs/extattr.h>
#        define psignal LSOF_psignal
#        define panicstr bp

#        if defined(__clang__)
/*
 * Two clang work-arounds...
 */
#            define KASSERT(exp, msg)                                          \
                do {                                                           \
                } while (0)
#            include <arpa/inet.h>
#        endif /* defined(__clang__) */

#        include <ufs/ufs/ufsmount.h>

#        if defined(__clang__)
/*
 * Undo the clang work-arounds.
 */
#            undef KASSERT
#        endif /* defined(__clang__) */

#        undef psignal
#        undef panicstr
#        undef _KERNEL
#    endif /* defined(HAS_UFS1_2) */

#    undef i_devvp

#    include <nfs/nfsproto.h>

#    if defined(HASRPCV2H)
#        include <nfs/rpcv2.h>
#    endif /* defined(HASRPCV2H) */

#    include <nfsclient/nfs.h>
#    include <nfsclient/nfsnode.h>

#    define syscall_args __bad_syscall_args
#    include <sys/proc.h>
#    undef __bad_syscall_args
#    include <kvm.h>
#    undef TRUE
#    undef FALSE

#    include <sys/sysctl.h>

#    if defined(HASFDESCFS)
#        define _KERNEL
#        define KERNEL
#        include <fs/fdescfs/fdesc.h>
#        undef _KERNEL
#        undef KERNEL
#    endif /* defined(HASFDESCFS) */

#    if defined(HASNULLFS)
#        define _KERNEL
#        define KERNEL
struct vop_generic_args;
#        include <fs/nullfs/null.h>
#        undef _KERNEL
#        undef KERNEL
#    endif /* defined(HASNULLFS) */

#    if defined(HASPSEUDOFS)
#        include <fs/pseudofs/pseudofs.h>
#    endif /* defined(HASPSEUDOFS) */

#    define P_ADDR ki_paddr
#    define P_COMM ki_comm
#    define P_FD ki_fd
#    define P_PID ki_pid
#    define P_PGID ki_pgid
#    define P_PPID ki_ppid
#    define P_STAT ki_stat
#    define P_VMSPACE ki_vmspace

#    include <vm/vm.h>

#    define _KERNEL
#    include <sys/fcntl.h>

#    if defined(__clang__)
/*
 * This work-around is needed when using clang, because <sys/fcntl.h> must
 * be #include'd under KERNEL and that causes the open() function prototype
 * definition to be skipped.
 */
int open(const char *, int, ...);
#    endif /* defined(__clang__) */
#    undef _KERNEL
#    include <sys/file.h>

#    define HASKQUEUE /* has the kqueue file type */

#    if __FreeBSD_version < 1400066
#        define _KERNEL
#        include <sys/eventvar.h>
#        undef _KERNEL
#    endif

struct vop_advlock_args {
    int dummy;
}; /* to pacify lf_advlock() prototype */
#    undef MALLOC_DECLARE
#    define MALLOC_DECLARE(type) extern struct malloc_type type[1]
/* to pacify <sys/lockf.h> */
#    define _KERNEL

#    if defined(HAS_SYS_SX_H)
#        include <sys/sx.h>
#    endif /* defined(HAS_SYS_SX_H) */

#    if defined(HAS_SI_PRIV) || defined(HAS_CONF_MINOR) ||                     \
        defined(HAS_CDEV2PRIV)
#        include <fs/devfs/devfs_int.h>
#    endif /* defined(SI_PRIV) || defined(HAS_CONF_MINOR) ||                   \
              defined(HAS_CDEV2PRIV) */

#    include <fs/devfs/devfs.h>
#    undef _KERNEL
#    include <sys/lockf.h>

#    define _KERNEL
#    include <sys/pipe.h>
#    undef _KERNEL
#    if defined(HASVMLOCKH)
#        include <vm/lock.h>
#    endif /* defined(HASVMLOCKH) */
#    include <vm/pmap.h>

#    include <vm/vm_map.h>

/*
 * Compensate for removal of MAP_ENTRY_IS_A_MAP from <vm/vm_map.h>,
 *  This work-around was supplied by John Polstra <jdp@polstra.com>.
 */

#    if defined(MAP_ENTRY_IS_SUB_MAP) && !defined(MAP_ENTRY_IS_A_MAP)
#        define MAP_ENTRY_IS_A_MAP 0
#    endif /* defined(MAP_ENTRY_IS_SUB_MAP) && !defined(MAP_ENTRY_IS_A_MAP) */

#    include <vm/vm_object.h>
#    include <vm/vm_pager.h>

#    undef B_NEEDCOMMIT

#    include <sys/bio.h>

#    include <sys/buf.h>
#    include <sys/user.h>

#    undef bcmp    /* avoid _KERNEL conflict */
#    undef bcopy   /* avoid _KERNEL conflict */
#    undef bzero   /* avoid _KERNEL conflict */
#    undef memcmp  /* avoid _KERNEL conflict */
#    undef memcpy  /* avoid _KERNEL conflict */
#    undef memmove /* avoid _KERNEL conflict */
#    undef memset  /* avoid _KERNEL conflict */
#    include <string.h>

#    define COMP_P const void
#    define DEVINCR 1024 /* device table malloc() increment */

#    if !defined(FREEBSD_KA_T)
typedef u_long KA_T;
#    endif /* !defined(FREEBSD_KA_T) */

#    define KMEM "/dev/kmem"
#    define MALLOC_P void
#    define FREE_P MALLOC_P
#    define MALLOC_S size_t
#    define MAXSYSCMDL MAXCOMLEN /* max system command name length */

#    if defined(N_UNIXV)
#        define N_UNIX_TMP(x) #        x
#        define N_UNIX_STR(x) N_UNIX_TMP(x)
#        define N_UNIX N_UNIX_STR(N_UNIXV)
#    endif /* defined(N_UNIXV) */

#    define QSORT_P void

#    if !defined(READLEN_T)
#        define READLEN_T int
#    endif /* !defined(READLEN_T) */

#    define STRNCPY_L size_t
#    define SWAP "/dev/drum"
#    define SZOFFTYPE unsigned long long
/* size and offset internal storage
 * type */
#    define SZOFFPSPEC                                                         \
        "ll" /* SZOFFTYPE print specification                                  \
              * modifier */

/*
 * Global storage definitions (including their structure definitions)
 */

extern struct file *Cfp;

extern kvm_t *Kd;

#    if defined(P_ADDR)
extern KA_T Kpa;
#    endif /* defined(P_ADDR) */

struct l_vfs {
    uint64_t fsid; /* file system ID */

#    if defined(MOUNT_NONE)
    short type; /* type of file system */
#    else       /* !defined(MOUNT_NONE) */
    char *typnm; /* file system type name */
#    endif      /* defined(MOUNT_NONE) */

    char *dir;          /* mounted directory */
    char *fsname;       /* file system name */
    struct l_vfs *next; /* forward link */
};
extern struct l_vfs *Lvfs;

struct mounts {
    char *dir;           /* directory (mounted on) */
    char *fsname;        /* file system
                          * (symbolic links unresolved) */
    char *fsnmres;       /* file system
                          * (symbolic links resolved) */
    dev_t dev;           /* directory st_dev */
    dev_t rdev;          /* directory st_rdev */
    INODETYPE inode;     /* directory st_ino */
    mode_t mode;         /* directory st_mode */
    mode_t fs_mode;      /* file system st_mode */
    struct mounts *next; /* forward link */
};

#    define X_BADFILEOPS "badfileops"
extern KA_T X_bfopsa;
#    define X_NCACHE "ncache"
#    define X_NCSIZE "ncsize"
#    define NL_NAME n_name

extern int Np; /* number of kernel processes */

extern struct kinfo_proc *P; /* local process table copy */

struct sfile {
    char *aname;        /* argument file name */
    char *name;         /* file name (after readlink()) */
    char *devnm;        /* device name (optional) */
    dev_t dev;          /* device */
    dev_t rdev;         /* raw device */
    u_short mode;       /* S_IFMT mode bits from stat() */
    int type;           /* file type: 0 = file system
                         *	      1 = regular file */
    INODETYPE i;        /* inode number */
    int f;              /* file found flag */
    struct sfile *next; /* forward link */
};

#    define XDR_VOID (const xdrproc_t) xdr_void
#    define XDR_PMAPLIST (const xdrproc_t) xdr_pmaplist

struct pcb_lists {
    struct xunpcb *un_stream_pcbs;
    size_t n_un_stream_pcbs;
    struct xunpcb *un_dgram_pcbs;
    size_t n_un_dgram_pcbs;
    struct xunpcb *un_seqpacket_pcbs;
    size_t n_un_seqpacket_pcbs;
    struct xtcpcb *tcp_pcbs;
    size_t n_tcp_pcbs;
    struct xinpcb *udp_pcbs;
    size_t n_udp_pcbs;
};

struct lock_list {
#    ifdef KERN_LOCKF
    struct kinfo_lockf *locks;
    size_t n_locks;
#    endif
};

/*
 * Definitions for rdev.c
 */

#    define DIRTYPE dirent
#    define HASDNAMLEN 1 /* struct DIRTYPE has d_namlen element */

/*
 * Definitions for rnam.c and rnmh.c
 */

#    if defined(HASNCACHE)
#        include <sys/uio.h>
#        if defined(HASNAMECACHE)
#            include <sys/namei.h>
#        else /* !defined(HASNAMECACHE) */
/*
 * The namecache struct definition should come from a header file that
 * can be #include'd, but it has been moved to a kernel source file in
 * 4.0-current for some reason unclear to me.
 *
 * So we must take the risk of defining it here. !!!! DANGER !!!!
 */

struct namecache {
#            if __FreeBSD_version >= 1300000 && __FreeBSD_version < 1300105
    LIST_ENTRY(namecache) nc_hash; /* hash chain */
    LIST_ENTRY(namecache) nc_src;  /* source vnode list */
    TAILQ_ENTRY(namecache) nc_dst; /* destination vnode list */
#            else
    LIST_ENTRY(namecache) nc_src;  /* source vnode list */
    TAILQ_ENTRY(namecache) nc_dst; /* destination vnode list */
    LIST_ENTRY(namecache) nc_hash; /* hash chain */
#            endif
    struct vnode *nc_dvp;          /* vnode of parent of name */
    struct vnode *nc_vp;           /* vnode the name refers to */
    u_char nc_flag;                /* flag bits */
    u_char nc_nlen;                /* length of name */
    char nc_name[16];              /* segment name -- Strictly composed,
                                    * the size of nc_name[] should be zero
                                    * and rnmh.c in lsof/lib should read
                                    * the name with a separate call to
                                    * kvm_read().  Since that causes extra
                                    * (and slow) calls to kvm_read(), the
                                    * size is set here to an experimentally
                                    * derived guess.  The same experiment
                                    * didn't reveal any extra kvm_read()
                                    * suggesting the guess is a safe one.
                                    * (VAA, 10 Apr 2002) */
};
#        endif /* defined(HASNAMECACHE)) */

#        define NCACHE namecache     /* kernel's structure name */
#        define NCACHE_NM nc_name    /* name in NCACHE */
#        define NCACHE_NMLEN nc_nlen /* name length in NCACHE */

#        include <stddef.h>
#        define NCACHE_NXT nc_hash.le_next /* link in NCACHE */

#        define NCACHE_NODEADDR nc_vp /* node address in NCACHE */
#        define NCACHE_PARADDR nc_dvp /* parent node address in NCACHE */

#        if defined(HASNCVPID)
#            define NCACHE_NODEID nc_vpid /* node ID in NCACHE */
#            define NCACHE_PARID nc_dvpid /* parent node ID in NCACHE */
#        endif                            /* DEFINED(HASNCVPID) */
#    endif                                /* defined(HASNCACHE) */

#    define VNODE_VFLAG v_iflag
#    define NCACHE_VROOT VV_ROOT

#    include <libutil.h>

struct lsof_context_dialect {};

#endif /* defined(FREEBSD_LSOF_H) */
