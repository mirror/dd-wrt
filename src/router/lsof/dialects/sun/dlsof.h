/*
 * dlsof.h - Solaris header file for lsof
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
 * $Id: dlsof.h,v 1.48 2012/04/10 16:40:23 abe Exp $
 */


#if	!defined(SOLARIS_LSOF_H)
#define	SOLARIS_LSOF_H	1

#include <fcntl.h>
#include <sys/mntent.h>
#include <sys/mnttab.h>

# if	solaris<20600
#define	_KMEMUSER	1
# else	/* solaris>=20600 */
#include <stddef.h>
# endif	/* solaris<20600 */

#include <stdlib.h>
#include <dirent.h>

# if	defined(HASZONES)
#define	_KERNEL
#include<sys/zone.h>
#undef	_KERNEL
# endif	/* defined(HASZONES) */

#include <kvm.h>
#include <nlist.h>
#include <signal.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <rpc/types.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

# if	solaris>=110000
#define	_KERNEL
# endif	/* solaris>=110000 */

#include <netinet/in.h>

# if	solaris>=110000
#undef	_KERNEL
# endif	/* solaris>=110000 */


# if	solaris>=70000
#include <sys/conf.h>
#include <sys/systeminfo.h>
# endif	/* solaris>=70000 */

#define	_KERNEL
#define	MI_HRTIMING
#include <inet/led.h>

# if	solaris<20600
#undef	staticf
# endif	/* solaris<20600 */

#include <inet/common.h>

# if	solaris>=70000
#include <sys/stropts.h>
# endif	/* solaris>=70000 */

# if	solaris<20600
#include <inet/mi.h>
# endif	/* solaris<20600 */

# if	solaris>=80000
#include <netinet/igmp.h>
#include <netinet/ip6.h>
# endif	/* solaris>=80000 */

# if	defined(HAS_IPCLASSIFIER_H)
#define	ffs		__kernel_ffs
#define	inet_ntop	__inet_ntop
#define	inet_pton	__inet_pton
#define	longjmp		__kernel_longjmp
#define	setjmp		__kernel_setjmp
#  if	solaris>=110000
#define	printf		__kernel_printf
#define	snprintf	__kernel_snprintf
#define	sprintf		__kernel_sprintf
#define	strsignal	__kernel_strsignal
#define	swab		__kernel_swab
#define	vprintf		__kernel_vprintf
#define	vsprintf	__kernel_vsprintf
#define	vsnprintf	__kernel_vsnprintf
#  endif	/* solaris>=110000 */
#include <inet/ipclassifier.h>
#undef	ffs
#undef	inet_ntop
#undef	inet_pton
#undef	longjmp
#undef	setjmp
#  if	solaris>=110000
#undef	printf
#undef	snprintf
#undef	sprintf
#undef	strsignal
#undef	swab
#undef	vprintf
#undef	vsprintf
#undef	vsnprintf
#  endif	/* solaris>=110000 */
# endif	/* defined(HAS_IPCLASSIFIER_H) */

#include <inet/ip.h>
#undef	_KERNEL
#undef	MI_HRTIMING
#define	exit		kernel_exit
#define	rval_t		char
#define	strsignal	kernel_strsignal
#include <sys/strsubr.h>

# if	defined(HAS_SOCKET_PROTO_H)
#define	_KERNEL	1	/* DEBUG */
# endif	/* HAS_SOCKET_PROTO_H */

#include <sys/socketvar.h>

# if	defined(HAS_SOCKET_PROTO_H)
#undef	_KERNEL		/* DEBUG */
# endif	/* HAS_SOCKET_PROTO_H */

#undef	exit
#undef	rval_t
#undef	strsignal

# if	solaris>=80000
#define	_KERNEL	1
# endif	/* solaris>=80000 */

#include <inet/tcp.h>

# if	solaris>=80000
#undef	_KERNEL
# endif	/* solaris>=80000 */

#include <net/route.h>
#include <netinet/in_pcb.h>
#include <sys/stream.h>

# if	solaris<20600
#undef	MAX
#undef	MIN
# endif	/* solaris<20600 */

#include <sys/sysmacros.h>
#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/fs/hsfs_spec.h>
#include <sys/fs/hsfs_node.h>
#include <sys/fs/lofs_node.h>

# if	solaris>=20600
#define	_KERNEL
# endif	/* solaris>=20600 */

#include <sys/fs/namenode.h>

# if	solaris>=20600
#undef	_KERNEL
# endif	/* solaris>=20600 */

#include <sys/tihdr.h>

# if	solaris>=20500
#include <sys/tiuser.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>

# if	solaris>=110000
#define	_KERNEL
#include <rpc/rpc.h>
#undef	_KERNEL
# endif	/* solaris>=110000 */

#include <rpc/clnt_soc.h>
#include <rpc/pmap_prot.h>
#define	_KERNEL
#include <sys/fs/autofs.h>

#  if	solaris>=100000
#define	printf		lsof_printf
#define	snprintf	lsof_snprintf
#define	sprintf		lsof_sprintf
#define	swab		lsof_swab
#define	vprintf		lsof_vprintf
#define	vsnprintf	lsof_vsnprintf
#define	vsprintf	lsof_vsprintf
#include <sys/fs/dv_node.h>
#undef	printf
#undef	snprintf
#undef	sprintf
#undef	swab
#undef	vprintf
#undef	vsnprintf
#undef	vsprintf
#include <sys/contract_impl.h>
#include <sys/ctfs_impl.h>
#include <sys/port_impl.h>
#  endif	/* solaris>=100000 */

#include <sys/door.h>
#undef	_KERNEL
# endif	/* solaris>=20500 */

#  if	!defined(_NETDB_H_)
#include <rpc/rpcent.h>
#  endif	/* !defined(_NETDB_H_) */

#include <sys/t_lock.h>
#include <sys/flock.h>

# if	solaris>=20300
#  if	solaris<20400
/*
 * The lock_descriptor structure definition is missing from Solaris 2.3.
 */

struct lock_descriptor {
	struct lock_descriptor *prev;
	struct lock_descriptor *next;
	struct vnode *vnode;
	struct owner {
		pid_t pid;
		long sysid;
	} owner;
	int flags;
	short type;
	off_t start;
	off_t end;
	struct lock_info {
		struct active_lock_info {
			struct lock_descriptor *ali_stack;
		} li_active;
		struct sleep_lock_info {
			struct flock sli_flock;
			/* Ignore the rest. */
		} li_sleep;
	} info;
};
#define	ACTIVE_LOCK	0x008		/* lock is active */
#  else	/* solaris>=20400 */
#include <sys/flock_impl.h>
#  endif	/* solaris<20400 */
# endif	/* solaris>=20300 */

#include <sys/fstyp.h>
#include <sys/dditypes.h>
#include <sys/ddidmareq.h>
#include <sys/ddi_impldefs.h>
#include <sys/mkdev.h>

# if	defined(HASCACHEFS)
#include <sys/fs/cachefs_fs.h>
# endif	/* defined(HACACHEFS) */

#include <sys/fs/fifonode.h>
#include <sys/fs/pc_fs.h>
#include <sys/fs/pc_dir.h>
#include <sys/fs/pc_label.h>
#include <sys/fs/pc_node.h>

# if	solaris>=20600
#undef	SLOCKED
# endif	/* solaris>=20600 */

#include <sys/fs/snode.h>
#include <sys/fs/tmpnode.h>

# if	solaris>=110000
#define	_KERNEL
# endif	/* solaris>=110000 */

#include <nfs/nfs.h>

# if	solaris>=110000
#undef	_KERNEL
# endif	/* solaris>=110000 */

# if	solaris>=100000
#define	_KERNEL
# endif	/* solaris >= 100000 */

#include <nfs/rnode.h>

#  if	solaris>=100000
#include <nfs/mount.h>
#include <nfs/nfs4.h>
#include <nfs/rnode4.h>
#  endif	/* solaris>=100000 */

# if	solaris>=100000
#undef	_KERNEL
# endif	/* solaris >= 100000 */

#include <sys/proc.h>
#include <sys/user.h>

# if	defined(HASPROCFS)
#include <sys/proc/prdata.h>
# endif	/* defined(HASPROCFS) */

#include <sys/file.h>
#include <vm/hat.h>
#include <vm/as.h>
#include <vm/seg.h>
#include <vm/seg_dev.h>
#include <vm/seg_map.h>
#include <vm/seg_vn.h>
#include <sys/tiuser.h>
#include <sys/t_kuser.h>

# if	solaris<100000
#include <sys/sockmod.h>
# endif	/* solaris<100000 */

/*
 * Structure for Atria's MVFS nodes
 */

struct mvfsnode {
	unsigned long d1[6];
	unsigned long m_ino;		/* node number */
};

extern int nlist();

# if	defined(HAS_AFS) && !defined(AFSAPATHDEF)
#define	AFSAPATHDEF	"/usr/vice/etc/modload/libafs"
# endif	/* defined(HAS_AFS) && !defined(AFSAPATHDEF) */

#define	ALLKMEM		"/dev/allkmem"
#define	COMP_P		const void
#define	CWDLEN		(MAXPATHLEN+1)
#define DEVINCR		1024		/* device table malloc() increment */
#define	DINAMEL		32
#define	DIRTYPE		dirent

# if	solaris>=100000
#define	GET_MAJ_DEV(d)	((major_t)(d >> L_BITSMINOR & L_MAXMAJ))
#define	GET_MIN_DEV(d)	((minor_t)(d & L_MAXMIN))
# endif	/* solaris >= 100000 */

# if	solaris>=70000
typedef	uintptr_t	KA_T;
# else	/* solaris<70000 */
typedef	void *		KA_T;
# endif	/* solaris>=70000 */

# if	solaris>=70000
#define	KA_T_FMT_X	"0x%p"
# endif	/* solaris>=70000 */

# if	solaris>=20501
#define	KMEM		"/dev/mem"
# else	/* solaris<20501 */
#define	KMEM		"/dev/kmem"
# endif	/* solaris>=20501 */

#define MALLOC_P	char
#define FREE_P		MALLOC_P
#define MALLOC_S	unsigned

# if	!defined(MAXEND)
#define	MAXEND		0x7fffffff
# endif	/* !defined(MAXEND) */

#define MAXSEGS		100		/* maximum text segments */
#define	MAXSYSCMDL	MAXCOMLEN	/* max system command name length */
#define NETCLNML	8
#define	N_UNIX		"/dev/ksyms"
#define	PROCMIN		5		/* processes that make a "good" scan */

#  if	defined(HASPROCFS)
#define	PR_ROOTINO	2		/* root inode for proc file system */
#  endif	/* defined(HASPROCFS) */

#define	PROCDFLT	256		/* default size for local proc table --
					 * MUST BE > 4!!!  */
#define PROCSIZE	sizeof(struct proc)
#define	PROCTRYLM	5		/* times to try to read proc table */
#define QSORT_P		char
#define	READLEN_T	int
#define STRNCPY_L	int
#define	STRNML		32		/* stream name length (maximum) */

# if	solaris>=20501
/*
 * Enable large file support.
 */

#  if	solaris>=20600
#define	fstat		fstat64
#define	lstat		lstat64
#define	stat		stat64
#  endif	/* solaris>=20600 */

#define	SZOFFTYPE	unsigned long long
					/* size and offset internal storage
					 * type */
#define	SZOFFPSPEC	"ll"		/* SZOFFTYPE printf specification
					 * modifier */
# endif	/* solaris>=20501 */

#define U_SIZE		sizeof(struct user)


/*
 * Global storage definitions (including their structure definitions)
 */

# if	defined(HAS_AFS)

#  if	defined(HASAOPT)
extern char *AFSApath;			/* alternate AFS name list path
					 * (from -a) */
#  endif	/* defined(HASAOPT) */

extern dev_t AFSdev;			/* AFS file system device number */
extern int AFSdevStat;			/* AFS file system device number
					 * status: 0 = unknown; 1 = known */
extern int AFSfstype;			/* AFS file system type index */
extern KA_T AFSVfsp;			/* AFS struct vfs kernel pointer */
# endif	/* defined(HAS_AFS) */

struct clone {
	struct l_dev cd;		/* device, inode, name, and verify */
	int n;				/* network flag */
	struct clone *next;		/* forward link */
};
extern struct clone *Clone;

extern	major_t	CloneMaj;

# if	defined(HAS_LIBCTF)
/*
 * Definitions for using the CTF library, libctf.
 */

#include <libctf.h>

#define CTF_MEMBER_UNDEF    ~0UL	/* undefined member type */
					/* CTF_member_t element definition */

/*
 * Member structure definition, initialized by CTF_MEMBER() macro calls
 */

typedef struct CTF_member {
    char *m_name;       		/* Member name. */
    ulong_t m_offset;   		/* Member offset, initially in bits,
					 * later bytes */
} CTF_member_t;


/*
 * CTF request structure
 */

typedef struct CTF_request {
    char *name;				/* structure name */
    CTF_member_t *mem;			/* member table */
} CTF_request_t;


/*
 * CTF macroes
 */

#define CTF_MEMBER(name)    { #name, CTF_MEMBER_UNDEF }
#define CTF_MEMBER_READ(ka, s, members, member) \
    kread((KA_T)(ka) + members[MX_ ## member].m_offset, \
          (char *)&s->member, sizeof(s->member))
# endif	/* defined(HAS_LIBCTF) */

extern char **Fsinfo;
extern int Fsinfomax;
extern int HasALLKMEM;
extern int HaveCloneMaj;
extern kvm_t *Kd;

struct l_ino {
	unsigned char dev_def;		/* dev member is defined */
	unsigned char ino_def;		/* ino member is defined */
	unsigned char nl_def;		/* nl member is defined */
	unsigned char rdev_def;		/* rdev member is defined */
	unsigned char sz_def;		/* sz member is defined */
	dev_t dev;			/* device */
	long ino;			/* node number */
	long nl;			/* link count */
	dev_t rdev;			/* "raw" device */
	SZOFFTYPE sz;			/* size */
};

struct l_vfs {
	KA_T addr;			/* kernel address */
	char *dir;			/* mounted directory */
	char *fsname;			/* file system name */
	dev_t dev;			/* device */

# if	defined(HASFSINO)
	INODETYPE fs_ino;		/* file system inode number */
# endif	/* defined(HASFSINO) */

# if	defined(HASMNTSTAT)
	unsigned char mnt_stat;		/* mount point stat() status:
					 *     0 = succeeded
					 *     1 = failed */
# endif	/* defined(HASMNTSTAT) */


# if	solaris>=80000
	nlink_t nlink;			/* directory link count */
	off_t size;			/* directory size */
# endif	/* solaris>=80000 */

	struct l_vfs *next;		/* forward link */
};
extern struct l_vfs *Lvfs;

struct mounts {
        char *dir;              	/* directory (mounted on) */
        char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;			/* file system
					 * (symbolic links resolved) */
        dev_t dev;              	/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory st_ino */
	mode_t mode;			/* directory st_mode */
	mode_t fs_mode;			/* file system st_mode */
        struct mounts *next;    	/* forward link */

# if	defined(HASFSTYPE)
	char *fstype;			/* file system type */
# endif	/* defined(HASFSTYPE) */

# if	solaris>=80000
	nlink_t nlink;			/* directory st_nlink */
	off_t size;			/* directory st_size */
# endif	/* solaris>=80000 */

# if	defined(HASMNTSTAT)
	unsigned char stat;		/* mount point stat() status:
					 *     0 = succeeded
					 *     1 = failed */
# endif	/* defined(HASMNTSTAT) */

};

struct pseudo {
	struct l_dev pd;		/* device, inode, path, verify */
	struct pseudo *next;		/* forward link */
};
extern struct pseudo *Pseudo;


/*
 * Solaris 11 sdev definitions
 */

#define	SDVOP_IP	0		/* Sdev[] devipnet_vnodeops index */
#define	SDVOP_NET	1		/* Sdev[] devnet_vnodeops index */
#define	SDVOP_PTS	2		/* Sdev[] devpts_vnodeops index */
#define	SDVOP_VT	3		/* Sdev[] devvt_vnodeops index */
#define	SDVOP_NUM	4		/* number of Sdev[] entries */


struct sfile {
	char *aname;			/* file name argument */
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
extern int Unof;			/* u_nofiles value */


/*
 * VxFS definitions
 */

#define	VXVOP_FCL	0		/* Vvops[] vx_fcl_vnodeops_p index */
#define	VXVOP_FDD	1		/* Vvops[] fdd_vnops index */
#define	VXVOP_FDDCH	2		/* Vvops[] fdd_chain_vnops index */
#define	VXVOP_REG	3		/* Vvops[] vx_vnodeops index */
#define	VXVOP_REG_P	4		/* Vvops[] vx_vnodeops_p index */
#define	VXVOP_NUM	5		/* number of Vvops[] entries */


/*
 * Kernel name list definitions
 */

#define	NL_NAME		n_name
#define	X_NCACHE	"ncache"
#define	X_NCSIZE	"ncsize"


/*
 * Definitions for dvch.c
 */

# if	defined(HASDCACHE)
#define	DCACHE_CLONE	rw_clone_sect	/* clone function for read_dcache */
#define	DCACHE_CLR	clr_sect	/* function to clear clone and
					 * pseudo caches when reading the
					 * device cache file fails */
#define	DCACHE_PSEUDO	rw_pseudo_sect	/* pseudo function for read_dcache */
# endif	/* defined(HASDCACHE) */

#define	DVCH_DEVPATH	"/devices"


/*
 * Definition for cvfs.c
 */

#define	CVFS_DEVSAVE 	1

# if	solaris>=80000
#define	CVFS_NLKSAVE	1
#define	CVFS_SZSAVE	1
# endif	/* solaris>=80000 */


/*
 * Definitions for rnch.c
 */

# if	defined(HASNCACHE)
#include <sys/dnlc.h>

#  if	!defined(NC_NAMLEN)
#define	HASDNLCPTR	1
#  endif	/* !defined(NC_NAMLEN) */

#  if	solaris>=80000
#define	NCACHE_NEGVN	"negative_cache_vnode"
#  endif	/* solaris>=80000 */
# endif	/* defined(HASNCACHE) */

#endif	/* SOLARIS_LSOF_H */
