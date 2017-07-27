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
 * $Id: dlsof.h,v 1.47 2015/07/07 20:23:43 abe Exp $
 */


#if	!defined(FREEBSD_LSOF_H)
#define	FREEBSD_LSOF_H	1

#include <stdlib.h>
#include <dirent.h>
#include <nlist.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

# if	FREEBSDV>=4000
#  if	FREEBSDV>=5000
#   if	FREEBSDV<6020
#    if	defined(__alpha__)
/*
 * For Alpha below 6.2, #include <machine/pcpu.h> before #define'ing _KERNEL.
 * Then #define PCPU_MD_FIELDS independently.  This hack avoids a compiler
 * complaint about register use.
 */

#include <machine/pcpu.h>       
#define PCPU_MD_FIELDS                                                  \
	struct alpha_pcb pc_idlepcb;            /* pcb for idling */    \
	u_int64_t       pc_idlepcbphys;         /* pa of pc_idlepcb */  \
	u_int64_t       pc_pending_ipis;        /* pending IPI's */     \
	u_int32_t       pc_next_asn;            /* next ASN to alloc */ \
	u_int32_t       pc_current_asngen       /* ASN rollover check */
#    endif	/* defined(__alpha__) */
#   endif	/* FREEBSDV<6020 */
#define	_KERNEL	1
#  endif	/* FREEBSDV>=5000 */

#  if	defined(HAS_VM_MEMATTR_T)
/*
 * The d_mmap2_t function typedef in <sys/conf.h> may need the definition
 * of vm_memattr_t for a pointer, but that definition is only available
 * under _KERNEL in <sys/types.h>.  Defining _KERNEL before including
 * <sys/types.h> causes many compilation problems, so this expediency
 * (hack) is used when the vm_memattr_t definition is needed.
 */
#define	vm_memattr_t	void
#  endif	/* defined(HAS_VM_MEMATTR_T) */

#  if	defined(NEEDS_BOOLEAN_T)
/*
 * In FreeBSD 9 and above the boolean_t typedef is also needed and is also
 * under _KERNEL in <sys/types.h>.
 */

#define	boolean_t	int
#  endif	/* defined(NEEDS_BOOLEAN_T) */

#include <sys/conf.h>

#  if	defined(HAS_VM_MEMATTR_T)
#undef	vm_memattr_t
#  endif	/* defined(HAS_VM_MEMATTR_T) */

#  if	defined(NEEDS_BOOLEAN_T)
#undef	boolean_t
#  endif	/* defined(NEEDS_BOOLEAN_T) */

#  if	defined(HAS_CONF_MINOR)
#undef	minor
#include "fbsd_minor.h"
#  endif	/* defined(HAS_CONF_MINOR) */

#  if	FREEBSDV>=5000
#undef	_KERNEL
#  endif	/* FREEBSDV>=5000 */
# endif	/* FREEBSDV>=4000 */

#include <sys/filedesc.h>
#include <sys/mbuf.h>
#define	NFS
#define m_stat	mnt_stat

# if	FREEBSDV>=3020
#define	_KERNEL
# endif	/* FREEBSDV>=3020 */

#include <sys/mount.h>

# if	FREEBSDV>=3020
#  if	defined(__clang__)
/*
 * This definition is needed when clang is used, because <sys/mount.h> must
 * be #include'd when _KERNEL is defined and that causes the getmntinfo()
 * function prototype to be skipped.
 */
int     getmntinfo(struct statfs **, int);
#  endif	/* defined(__clang__) */

#undef	_KERNEL
# endif	/* FREEBSDV>=3020 */

#include <rpc/types.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/un.h>
#include <sys/unpcb.h>

# if	FREEBSDV>=3000
#undef	INADDR_LOOPBACK
# endif	/* FREEBSDV>=3000 */

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <sys/ucred.h>
#include <sys/uio.h>

# if	defined(HAS_KVM_VNODE)
#define	_KVM_VNODE
# endif	/* defined(HAS_KVM_VNODE) */
#include <sys/vnode.h>
# if	defined(HAS_KVM_VNODE)
#undef	_KVM_VNODE
# endif	/* defined(HAS_KVM_VNODE) */

#include <net/raw_cb.h>
#include <sys/domain.h>
#define	pmap	RPC_pmap
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#undef	pmap

# if	FREEBSDV<2000
#include <ufs/quota.h>
#include <ufs/inode.h>
#include <ufs/ufsmount.h>
#include <ufs/mfsnode.h>
# else	/* FREEBSDV>=2000 */
#include <paths.h>
#include <ufs/ufs/quota.h>

#  if	FREEBSDV>=4000 && FREEBSDV<5000
#   if	defined(__alpha__) || defined(__sparc64__)
#define	dev_t	void *
#   endif	/* defined(__alpha__) || defined(__sparc64__) */
#  endif /* FREEBSDV>=4000 && FREEBSDV<5000 */

#include <ufs/ufs/inode.h>

# if	defined(HAS_UFS1_2)
#define	_KERNEL
struct vop_getextattr_args;
struct vop_deleteextattr_args;
struct vop_setextattr_args;
#include <ufs/ufs/extattr.h>
#define	psignal	LSOF_psignal
#define	panicstr bp

#  if	defined(__clang__)
/*
 * Two clang work-arounds...
 */
#define	KASSERT(exp,msg) do {} while (0)
#include <arpa/inet.h>
#  endif	/* defined(__clang__) */

#include <ufs/ufs/ufsmount.h>

#  if	defined(__clang__)
/*
 * Undo the clang work-arounds.
 */
#undef	KASSERT
#  endif	/* defined(__clang__) */

#undef	psignal
#undef	panicstr
#undef	_KERNEL
# endif	/* defined(HAS_UFS1_2) */

#  if	FREEBSDV>=5010
#undef	i_devvp
#  endif	/* FREEBSDV>=5010 */

#  if	FREEBSDV>=4000 && FREEBSDV<5000
#   if	defined(__alpha__) || defined(__sparc64__)
#undef	dev_t
#   endif	/* defined(__alpha__) || defined(__sparc64__) */
#  endif /* FREEBSDV>=4000 && FREEBSDV<5000 */

#  if   FREEBSDV<2020
#include <ufs/mfs/mfsnode.h>
#  endif        /* FREEBSDV<2020 */

# endif	/* FREEBSDV<2000 */

# if	FREEBSDV<5000
#include <nfs/nfsv2.h>
# else	/* FREEBSDV>=5000 */
#include <nfs/nfsproto.h>
# endif	/* FREEBSDV<5000 */

# if	defined(HASRPCV2H)
#include <nfs/rpcv2.h>
# endif	/* defined(HASRPCV2H) */

# if	FREEBSDV>=5000
#include <nfsclient/nfs.h>
#include <nfsclient/nfsnode.h>
# else	/* FREEBSDV<5000 */
#include <nfs/nfs.h>
#include <nfs/nfsnode.h>
# endif	/* FREEBSDV>=5000 */

#include <sys/proc.h>
#include <kvm.h>
#undef	TRUE
#undef	FALSE

# if	FREEBSDV<2000
#include <sys/kinfo.h>
# else	/* FREEBSDV>=2000 */
#include <sys/sysctl.h>
# endif	/* FREEBSDV<2000 */

# if	defined(HASFDESCFS)
#define	_KERNEL
#define	KERNEL
#  if	FREEBSDV>=5000
#include <fs/fdescfs/fdesc.h>
#  else	/* FREEBSDV<5000 */
#include <miscfs/fdesc/fdesc.h>
#  endif	/* FREEBSDV>=5000 */
#undef	_KERNEL
#undef	KERNEL
# endif	/* defined(HASFDESCFS) */

# if	defined(HASNULLFS)
#define	_KERNEL
#define	KERNEL
struct vop_generic_args;
#  if	FREEBSDV>=5000
#include <fs/nullfs/null.h>
#  else	/* FREEBSDV<5000 */
#include <miscfs/nullfs/null.h>
#  endif	/* FREEBSDV>=5000 */
#undef	_KERNEL
#undef	KERNEL
# endif	/* defined(HASNULLFS) */

# if	defined(HASPROCFS)
#  if	FREEBSDV<2000
#include <procfs/pfsnode.h>
# else	/* FREEBSDV>=2000 */
#  if	FREEBSDV<5000
#include <miscfs/procfs/procfs.h>
#  endif	/* FREEBSDV<5000 */
#include <machine/reg.h>
# endif	/* FREEBSDV<2000 */

#define	PNSIZ		5
# endif	/* defined(HASPROCFS) */

# if	defined(HASPSEUDOFS)
#include <fs/pseudofs/pseudofs.h>
# endif	/* defined(HASPSEUDOFS) */

# if	defined(HAS_ZFS)
#include "dzfs.h"
# endif	/* defined(HAS_ZFS) */


# if	FREEBSDV<2000
#define	P_COMM		p_comm
#define	P_FD		p_fd
#define	P_PID		p_pid
#define	P_PGID		p_pgrp
#define	P_STAT		p_stat
#define	P_VMSPACE	p_vmspace
# else	/* FREEBSDV>=2000 */
#  if	FREEBSDV<5000
#define	P_ADDR		kp_eproc.e_paddr
#define	P_COMM		kp_proc.p_comm
#define	P_FD		kp_proc.p_fd
#define	P_PID		kp_proc.p_pid
#define	P_PGID		kp_eproc.e_pgid
#define	P_PPID		kp_eproc.e_ppid
#define	P_STAT		kp_proc.p_stat
#define	P_VMSPACE	kp_proc.p_vmspace
#  else	/* FREEBSDV>=5000 */
#define	P_ADDR		ki_paddr
#define	P_COMM		ki_comm
#define	P_FD		ki_fd
#define	P_PID		ki_pid
#define	P_PGID		ki_pgid
#define	P_PPID		ki_ppid
#define	P_STAT		ki_stat
#define	P_VMSPACE	ki_vmspace
#  endif	/* FREEBSDV<5000 */
# endif	/* FREEBSDV<2000 */

#include <vm/vm.h>

#define	_KERNEL
#define	KERNEL
#include <sys/fcntl.h>

/*
 * The following circumventions were first needed in FreeBSD 8.0-CURRENT some
 * time in August 2008 to avoid conflicts in /usr/src/sys/sys/libkern.h> and
 * /usr/src/sys/sys/systm.h, called by <sys/file.h> or the header files it
 * #include's when KERNEL or _KERNEL is #define'd.
 *
 * The circumventions may be needed or may be erroneous for earlier FreeBSD
 * versions where testing was not possible.
 */

#  if	defined(__clang__)
/*
 * This work-around is needed when using clang, because <sys/fcntl.h> must
 * be #include'd under KERNEL and that causes the open() function prototype
 * definition to be skipped.
 */
int     open(const char *, int, ...);
#  endif	/* defined(__clang__) */

#define	intrmask_t	int
#define	log	log_kernel_lsof

# if	!defined(HAS_PAUSE_SBT)
#define	pause	pause_kernel_lsof
# endif	/* !defined(HAS_PAUSE_SBT) */

#define	asprintf asprintf_kernel_lsof
#define	setenv	setenv_kernel_lsof
#define	vasprintf vasprintf_kernel_lsof
#define	uintfptr_t	int
#define	_SYS_LIBKERN_H_
#include <sys/file.h>

/*
 * Attempt to remove the circumventions.
 */

#undef	_SYS_LIBKERN_H_
#undef	asprintf_kernel_lsof
#undef	intrmask_t_lsof
#undef	log_kernel_lsof

# if	!defined(HAS_PAUSE_SBT)
#undef	pause_kernel_lsof
# endif	/* !defined(HAS_PAUSE_SBT) */

#undef	setenv_kernel_lsof
#undef	vasprintf_kernel_lsof
#undef	uintfptr_t
#undef	_KERNEL
#undef	KERNEL

# if	defined(DTYPE_KQUEUE)
#define	HASKQUEUE				/* has the kqueue file type */
#   if	FREEBSDV>=4090
#define	_KERNEL
#   endif	/* FREEBSDV>=4090 */
#include <sys/eventvar.h>
#   if	FREEBSDV>=4090
#undef	_KERNEL
#   endif	/* FREEBSDV>=4090 */
# endif	/* defined(DTYPE_KQUEUE) */

# if	FREEBSDV<2000
#include <ufs/lockf.h>
# else	/* FREEBSDV>=2000 */
struct vop_advlock_args { int dummy; };	/* to pacify lf_advlock() prototype */
#  if	FREEBSDV>=5000
#undef	MALLOC_DECLARE
#define	MALLOC_DECLARE(type)	extern struct malloc_type type[1]
					/* to pacify <sys/lockf.h> */
#define	_KERNEL

#   if	defined(HAS_SYS_SX_H)
#include <sys/sx.h>
#   endif	/* defined(HAS_SYS_SX_H) */

#   if	defined(HAS_SI_PRIV) || defined(HAS_CONF_MINOR) || defined(HAS_CDEV2PRIV)
#include <fs/devfs/devfs_int.h>
#   endif	/* defined(SI_PRIV) || defined(HAS_CONF_MINOR) || defined(HAS_CDEV2PRIV) */

#include <fs/devfs/devfs.h>
#undef	_KERNEL
#  endif	/* FREEBSDV>=5000 */
#include <sys/lockf.h>
# endif	/* FREEBSDV<2000 */

#  if   FREEBSDV>=2020
#   if	FREEBSDV>=4090
#define	_KERNEL
#   endif	/* FREEBSDV>=4090 */
#include <sys/pipe.h>
#   if	FREEBSDV>=4090
#undef	_KERNEL
#   endif	/* FREEBSDV>=4090 */
#   if	defined(HASVMLOCKH)
#include <vm/lock.h>
#   endif	/* defined(HASVMLOCKH) */
#include <vm/pmap.h>
#  endif        /* FREEBSDV>=2020 */

#include <vm/vm_map.h>

/*
 * Compensate for removal of MAP_ENTRY_IS_A_MAP from <vm/vm_map.h>,
 *  This work-around was supplied by John Polstra <jdp@polstra.com>.
 */

# if	defined(MAP_ENTRY_IS_SUB_MAP) && !defined(MAP_ENTRY_IS_A_MAP)
#define MAP_ENTRY_IS_A_MAP	0
# endif	/* defined(MAP_ENTRY_IS_SUB_MAP) && !defined(MAP_ENTRY_IS_A_MAP) */

#include <vm/vm_object.h>
#include <vm/vm_pager.h>

# if   FREEBSDV>=2020
#undef	B_NEEDCOMMIT

#  if	FREEBSDV>=5000
#include <sys/bio.h>
#  endif	/* FREEBSDV>=5000 */

#include <sys/buf.h>
#include <sys/user.h>

#  if	FREEBSDV<5000
#include <ufs/mfs/mfsnode.h>
#  endif	/* FREEBSDV<5000 */
# endif        /* FREEBSDV>=2020 */

#include <string.h>


#define	COMP_P		const void
#define DEVINCR		1024	/* device table malloc() increment */

# if	!defined(FREEBSD_KA_T)
#  if	FREEBSDV<2000
typedef	off_t		KA_T;
#  else	/* FREEBSDV>=2000 */
typedef	u_long		KA_T;
#  endif	/* FREEBSDV<2000 */
# endif	/* !defined(FREEBSD_KA_T) */

#define	KMEM		"/dev/kmem"
#define MALLOC_P	void
#define FREE_P		MALLOC_P
#define MALLOC_S	size_t
#define	MAXSYSCMDL	MAXCOMLEN	/* max system command name length */

# if	defined(N_UNIXV)
#define	N_UNIX_TMP(x)	#x
#define	N_UNIX_STR(x)	N_UNIX_TMP(x)
#define	N_UNIX		N_UNIX_STR(N_UNIXV)
# endif	/* defined(N_UNIXV) */

#define QSORT_P		void

# if	!defined(READLEN_T)
#define	READLEN_T	int
# endif	/* !defined(READLEN_T) */

#define STRNCPY_L	size_t
#define SWAP		"/dev/drum"
#define	SZOFFTYPE	unsigned long long
					/* size and offset internal storage
					 * type */
#define	SZOFFPSPEC	"ll"		/* SZOFFTYPE print specification
					 * modifier */


/*
 * Global storage definitions (including their structure definitions)
 */

struct file * Cfp;

# if	FREEBSDV>=2000
extern kvm_t *Kd;
# endif	/* FREEBSDV>=2000 */

# if	defined(P_ADDR)
extern KA_T Kpa;
# endif	/* defined(P_ADDR) */

struct l_vfs {
	KA_T addr;			/* kernel address */
	fsid_t	fsid;			/* file system ID */

# if	defined(MOUNT_NONE)
	short type;			/* type of file system */
# else	/* !defined(MOUNT_NONE) */
	char *typnm;			/* file system type name */
# endif	/* defined(MOUNT_NONE) */

	char *dir;			/* mounted directory */
	char *fsname;			/* file system name */
	struct l_vfs *next;		/* forward link */
};
extern struct l_vfs *Lvfs;

struct mounts {
        char *dir;              	/* directory (mounted on) */
	char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;           	/* file system
					 * (symbolic links resolved) */
        dev_t dev;              	/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory st_ino */
	mode_t mode;			/* directory st_mode */
	mode_t fs_mode;			/* file system st_mode */
        struct mounts *next;    	/* forward link */
};

#define	X_NCACHE	"ncache"
#define	X_NCSIZE	"ncsize"
#define	NL_NAME		n_name

extern int Np;				/* number of kernel processes */

# if	FREEBSDV>=2000
extern struct kinfo_proc *P;		/* local process table copy */
# endif	/* FREEBSDV>=2000 */

struct sfile {
	char *aname;			/* argument file name */
	char *name;			/* file name (after readlink()) */
	char *devnm;			/* device name (optional) */
	dev_t dev;			/* device */
	dev_t rdev;			/* raw device */
	u_short mode;			/* S_IFMT mode bits from stat() */
	int type;			/* file type: 0 = file system
				 	 *	      1 = regular file */
	INODETYPE i;			/* inode number */
	int f;				/* file found flag */
	struct sfile *next;		/* forward link */

};

# if	FREEBSDV==4100 || FREEBSDV==4110
#define	XDR_VOID	(xdrproc_t)xdr_void
#define	XDR_PMAPLIST	(xdrproc_t)xdr_pmaplist
# endif	/* FREEBSDV==4100 || FREEBSDV==4110 */

# if	FREEBSDV>=5000
#define	XDR_VOID	(const xdrproc_t)xdr_void
#define	XDR_PMAPLIST	(const xdrproc_t)xdr_pmaplist
# endif	/* FREEBSDV>=5000 */


/*
 * Definitions for rdev.c
 */

#define	DIRTYPE	dirent
#define HASDNAMLEN	1	/* struct DIRTYPE has d_namlen element */


/*
 * Definitions for rnam.c and rnmh.c
 */

# if     defined(HASNCACHE)
#include <sys/uio.h>
#  if	FREEBSDV<4000 || (FREEBSDV>=4000 && defined(HASNAMECACHE))
#include <sys/namei.h>
#  else	/* FREEBSDV>=4000 && !defined(HASNAMECACHE) */
/*
 * The namecache struct definition should come from a header file that
 * can be #include'd, but it has been moved to a kernel source file in
 * 4.0-current for some reason unclear to me.
 *
 * So we must take the risk of defining it here. !!!! DANGER !!!!
 */

struct	namecache {
	LIST_ENTRY(namecache) nc_hash;	/* hash chain */
	LIST_ENTRY(namecache) nc_src;	/* source vnode list */
	TAILQ_ENTRY(namecache) nc_dst;	/* destination vnode list */
	struct	vnode *nc_dvp;		/* vnode of parent of name */
	struct	vnode *nc_vp;		/* vnode the name refers to */
	u_char	nc_flag;		/* flag bits */
	u_char	nc_nlen;		/* length of name */
	char	nc_name[16];		/* segment name -- Strictly composed,
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
#  endif	/* FREEBSDV<4000 || (FREEBSDV>=4000 && defined(HASNAMECACHE)) */

#define	NCACHE		namecache	/* kernel's structure name */
#define	NCACHE_NM	nc_name		/* name in NCACHE */
#define	NCACHE_NMLEN	nc_nlen		/* name length in NCACHE */

#  if	FREEBSDV<2005
#define	NCACHE_NXT	nc_nxt		/* link in NCACHE */
#  else	/* FREEBSDV>=2005 */
#   if	FREEBSDV<2010
#define	NCACHE_NXT	nc_lru.tqe_next	/* link in NCACHE */
#   else	/* FREEBSDV>=2010 */
#include <stddef.h>
#define	NCACHE_NXT	nc_hash.le_next	/* link in NCACHE */
#   endif	/* FREEBSDV<2010 */
#  endif	/* FREEBSDV<2005 */

#define	NCACHE_NODEADDR	nc_vp		/* node address in NCACHE */
#define	NCACHE_PARADDR	nc_dvp		/* parent node address in NCACHE */

#  if	defined(HASNCVPID)
#define	NCACHE_NODEID	nc_vpid		/* node ID in NCACHE */
#define	NCACHE_PARID	nc_dvpid	/* parent node ID in NCACHE */
#  endif	/* DEFINED(HASNCVPID) */
# endif  /* defined(HASNCACHE) */

# if	FREEBSDV>=5000
#define	VNODE_VFLAG	v_iflag
#define	NCACHE_VROOT	VV_ROOT
# endif	/* FREEBSDV>=5000 */

#endif	/* defined(FREEBSD_LSOF_H) */
