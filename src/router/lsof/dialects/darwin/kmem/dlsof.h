/*
 * dlsof.h - Darwin header file for /dev/kmem-based lsof
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
 * $Id: dlsof.h,v 1.11 2005/11/01 20:24:51 abe Exp $
 */


#if	!defined(DARWIN_LSOF_H)
#define	DARWIN_LSOF_H	1

#include <stdlib.h>
#include <dirent.h>
#include <nlist.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/conf.h>
#include <sys/filedesc.h>
#include <sys/ucred.h>

#if	DARWINV<800
#include <sys/mount.h>
#define	m_stat	mnt_stat
#else	/* DARWINV>=800 */
#include <sys/mount_internal.h>
#define	m_stat	mnt_vfsstat
#endif	/* DARWINV>=800 */

#if	DARWINV<800
#include <sys/uio.h>
#include <sys/vnode.h>
#else	/* DARWINV>=800 */
#include <sys/vnode.h>
#define	_SYS_SYSTM_H_
struct nameidata { int dummy; };	/* to satisfy function  prototypes */
#include <sys/vnode_internal.h>
#endif	/* DARWINV>=800 */

#include <rpc/types.h>
#define	KERNEL_PRIVATE
#include <sys/socketvar.h>
#undef	KERNEL_PRIVATE
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/unpcb.h>

# if	defined(AF_NDRV)
#include <net/if_var.h>
#define	KERNEL
#include <sys/kern_event.h>
#undef	KERNEL
#include <net/ndrv.h>
#  if	DARWINV>=530
#define	KERNEL        1
#include <net/ndrv_var.h>
#undef  KERNEL
#  endif	/* DARWINV>=530 */
# endif	/* defined(AF_NDRV) */

# if	defined(AF_SYSTEM)
#include <sys/queue.h>
#define	KERNEL
#include <sys/kern_event.h>
#undef	KERNEL
# endif	/* defined(AF_SYSTEM) */

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <net/route.h>
#include <netinet6/ipsec.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <arpa/inet.h>
#include <net/raw_cb.h>
#include <sys/domain.h>
#define	pmap	RPC_pmap
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#undef	pmap

#include <sys/quota.h>
#include <sys/event.h>

# if	DARWINV<800
#include <paths.h>
#undef	MAXNAMLEN
#include <ufs/ufs/quota.h>
#include <paths.h>
#include <ufs/ufs/quota.h>
#include <ufs/ufs/inode.h>
#include <nfs/rpcv2.h>
#include <nfs/nfs.h>
#include <nfs/nfsproto.h>
#include <nfs/nfsnode.h>

#  if	DARWINV<600
#include <hfs/hfs.h>
#undef	offsetof
# else	/* DARWINV>=600 */
#define	KERNEL
#include <hfs/hfs_cnode.h>
#undef	KERNEL
#  endif        /* DARWINV<600 */
# endif	/* DARWINV<800 */

# if	DARWINV<800
#define	time	t1		/* hack to make dn_times() happy */
#include <miscfs/devfs/devfsdefs.h>
#undef	time
# endif	/* DARWINV<800 */

# if	DARWINV<800
#define	KERNEL
#include <miscfs/fdesc/fdesc.h>
#undef	KERNEL
# endif	/* DARWINV<800 */

# if	DARWINV<800
#include <sys/proc.h>
# else	/* DARWINV>=800 */
#define	PROC_DEF_ENABLED
#define	sleep	kernel_sleep
#include <sys/proc_internal.h>
#undef	sleep
# endif	/* DARWINV<800 */

#include <kvm.h>
#undef	TRUE
#undef	FALSE

# if	DARWINV<800
#include <sys/sysctl.h>
# else	/* DARWINV>=800 */
#include "/usr/include/sys/sysctl.h"
# endif	/* DARWINV<800 */

# if	DARWINV<800
#define	KERNEL
#include <sys/fcntl.h>
#include <sys/file.h>
#undef	KERNEL
# else	/* DARWINV>=800 */
#include <sys/fcntl.h>
#include <sys/file_internal.h>
# endif	/* DARWINV<800 */

# if	defined(HASKQUEUE)
#include <sys/eventvar.h>
# endif	/* defined(HASKQUEUE) */

# if	defined(DTYPE_PSXSEM)
#define	HASPSXSEM				/* has the POSIX semaphore file
						 * type */
# endif	/* defined(DTYPE_PSXSEM) */

# if	defined(DTYPE_PSXSHM)
#define	HASPSXSHM				/* has the POSIX shared memory
						 * file type */
# endif	/* defined(DTYPE_PSXSHM) */

struct vop_advlock_args { int dummy; };	/* to satisfy lf_advlock() prototype */
#include <sys/lockf.h>
#include <sys/lock.h>

/*
 * Compensate for removal of MAP_ENTRY_IS_A_MAP from <vm/vm_map.h>,
 *  This work-around was supplied by John Polstra <jdp@polstra.com>.
 */

# if	defined(MAP_ENTRY_IS_SUB_MAP) && !defined(MAP_ENTRY_IS_A_MAP)
#define	MAP_ENTRY_IS_A_MAP	0
# endif	/* defined(MAP_ENTRY_IS_SUB_MAP) && !defined(MAP_ENTRY_IS_A_MAP) */

#undef	B_NEEDCOMMIT
#include <sys/buf.h>
#include <sys/signal.h>
#define	user_sigaltstack	sigaltstack
#include <sys/user.h>

#define	COMP_P		const void
#define	DEVINCR		1024	/* device table malloc() increment */
#define	DIRTYPE		dirent	/* directory entry type */

typedef	u_long		KA_T;

#define	KMEM		"/dev/kmem"
#define	LOGINML		MAXLOGNAME
#define	MALLOC_P	void
#define	FREE_P		MALLOC_P
#define	MALLOC_S	size_t

#define	N_UNIX	"/mach_kernel"

#define	QSORT_P		void
#define	READLEN_T	int
#define	STRNCPY_L	size_t
#define	SWAP		"/dev/drum"

# if	DARWINV>=800
#define	SZOFFTYPE	unsigned long long
					/* size and offset internal storage
					 * type */
#define	SZOFFPSPEC	"ll"		/* SZOFFTYPE printf specification
					 * modifier */
# endif	/* DARWINV>=800 */


/*
 * Global storage definitions (including their structure definitions)
 */

struct file * Cfp;

extern int Kd;				/* KMEM descriptor */
extern KA_T Kpa;

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

#define	XDR_VOID	(const xdrproc_t)xdr_void 
#define	XDR_PMAPLIST	(const xdrproc_t)xdr_pmaplist


/*
 * Definitions for rnmh.c
 */

# if     defined(HASNCACHE)
#include <sys/uio.h>
#include <sys/namei.h>

#  if	!defined(offsetof)
#define	offsetof(type, member)	((size_t)(&((type *)0)->member))
#  endif	/* !defined(offsetof) */

#define	NCACHE		namecache	/* kernel's structure name */

#define	NCACHE_NM	nc_name		/* name in NCACHE */

#  if	DARWINV<700
#define	NCACHE_NMLEN	nc_nlen		/* name length in NCACHE */
#  endif	/* DARWINV<700 */

#define	NCACHE_NXT	nc_hash.le_next	/* link in NCACHE */
#define	NCACHE_NODEADDR	nc_vp		/* node address in NCACHE */
#define	NCACHE_PARADDR	nc_dvp		/* parent node address in NCACHE */

#  if	defined(HASNCVPID)
#define	NCACHE_NODEID	nc_vpid		/* node ID in NCACHE */
#define	NCACHE_PARID	nc_dvpid	/* parent node ID in NCACHE */
#  endif	/* defined(HASNCVPID) */
# endif  /* defined(HASNCACHE) */

#endif	/* DARWIN_LSOF_H */
