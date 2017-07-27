/*
 * dlsof.h - DEC OSF/1, Digital UNIX, Tru64 UNIX header file for lsof
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
 * $Id: dlsof.h,v 1.27 2006/03/27 20:40:59 abe Exp $
 */


#if	!defined(DU_LSOF_H)
#define	DU_LSOF_H	1

#include <fcntl.h>
#include <fstab.h>

# if	DUV<30000 || DUV>=50000
#include <sys/mount.h>
# endif	/* DUV<30000 || DUV>=50000 */

#include <dirent.h>
#include <nlist.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <cdfs/cdfsnode.h>
#include <machine/hal_sysinfo.h>
#include <rpc/types.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/table.h>
#include <sys/un.h>
#include <sys/unpcb.h>
#include <sys/domain.h>
#include <netinet/in.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <net/route.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <net/raw_cb.h>
#undef	queue
#undef	queue_t
#define	queue	___queue
#define	queue_t	___queue_t
#include <sys/stream.h>
#undef	___queue
#undef	___queue_t
#define	___queue queue
#define	___queue_t queue_t

# if	DUV<30000
#include <nfs/nfs.h>
#define	KERNEL_FILE
#include <sys/file.h>
# endif	/* DUV<30000 */

#include <ufs/inode.h>

#  if	DUV>=50100
#include <sys/systm.h>
#define	_SYS_USER_H_
#include <machine/pcb.h>
#undef	_SYS_USER_H_
#  endif	/* DUV>=50100 */

/*
 * The following header files need _KERNEL and KERNEL defined.  Some
 * ugly #undef preparation is necessary.
 */

#define _KERNEL	1
#define KERNEL	1
#undef	MACRO_END
#undef	PIPSIZ
#undef	i_forw
#undef	i_gen
#undef	i_gid
#undef	i_lock
#undef	i_mode
#undef	i_nlink
#undef	i_rdev
#undef	i_size
#undef	i_uid

# if	DUV>=30000
#undef	m_data
#undef	m_next
#include <sys/file.h>
#include <sys/fifonode.h>
#  if	DUV<50000
#include <sys/mount.h>
#  endif	/* DUV<50000 */
# endif	/* DUV>=30000 */

#undef	calloc
#define	calloc	___calloc
#undef	exit
#define	exit	___exit
#define	pmap	___pmap
#undef	pt_entry_t
#define	pt_entry_t ___pt_entry_t
#undef	timer_t
#define	timer_t	___timer_t

# if	DUV>=50000
#include "du5_sys_malloc.h"
#undef	_SYS_WAIT_H_			/* allow <sys/user.h> to
					 * #include <sys/wait.h> while
					 * _KERNEL is defined */
# endif	/* DUV>=50000 */

# if	DUV<40000
#include <kern/task.h>
#undef	___calloc
#define	___calloc calloc
#undef	___exit
#define	___exit	exit
#undef	___pt_entry_t
#undef	___timer_t
# endif	/* DUV<40000 */

#include <s5fs/s5param.h>
#include <s5fs/s5inode.h>
#include <sys/procfs.h>
#include <sys/proc.h>

# if	DUV>=40000
#undef	___calloc
#define	___calloc calloc
#undef	___exit
#define	___exit	exit
# endif	/* DUV>=40000 */

#include <sys/user.h>
#undef	u_comm
#define	u_comm	uu_comm
#include <sys/flock.h>

# if	DUV>=30000
#undef	u
#endif	/* DUV>=30000 */

#include <sys/specdev.h>
#include <sys/vnode.h>

# if	DUV>=30000
#define	quotactl	__quotactl
#include <nfs/nfs.h>
#undef	quotactl
# endif	/* DUV>=30000 */

#include <nfs/rnode.h>
#include <ufs/mfsnode.h>
#include <vm/vm_anon.h>
#include <vm/u_mape_seg.h>

# if	DUV>=40000
#include <vm/vm_ubc.h>
# else	/* DUV<40000 */
#include <vm/vm_vp.h>
# endif	/* DUV>=40000 */


# if	!defined(HASSPECNODE)
/*
 * The spec_node is not defined in a distributed header file, but in
 * a kernel source file.
 */

struct spec_node {
	struct vnode *sn_vnode;
	struct vattr sn_vattr;
};
# endif	/* !defined(HASSPECNODE) */


# if	ADVFSV<500
/*
 * This is an educated guess at an ADVFS/MSFS node for AdvFS versions below 5.
 *
 * Information that became available to me for AdvFS 5.0 and higher indicates
 * multiple adjacent structures are involved.  Those definitions may be found
 * in dnode.c inside an ADVFSV #if|#endif block.
 */

struct advfsnode {

#  if	ADVFSV<200
	unsigned long d1[19];
#  else	/* ADVFSV>=200 */
#   if	ADVFSV<300
	unsigned long d1[20];
#   else	/* ADVFSV>=300 */
#    if	ADVFSV<400
	unsigned long d1[21];
#    else	/* ADVFSV>=400 */
	unsigned long d1[17];
#    endif	/* ADVFSV>=400 */
#   endif	/* ADVFSV<300 */
#  endif	/* ADVFSV<200 */

	ino_t a_number;
	int a_seq;
	unsigned long d3;
	int d4;
	dev_t a_rdev;
	unsigned long a_size;

#  if	ADVFSV>=400
	unsigned long d5[5];
	int d6;
	int a_nlink;
#  endif	/* ADVFSV>=400 */

};
# endif	/* ADVFSV<500 */


# if	defined(HASTAGTOPATH)
/*
 * Define the structure used for passing inode and sequence numbers to the
 * ADVFS 4.0 and greater tag_to_path() -lmsfs function.
 *
 * This structure definition was provided by Dean Brock <brock@cs.unca.edu>.
 */
	typedef struct {
	    int ml_ino;
	    int ml_seq;
	} mlBfTagT;
# endif	/* defined(HASTAGTOPATH) */


# if	DUV<50000
#define	COMP_P		void
typedef	unsigned long	KA_T;
# else	/* DUV>=50000 */
#define	COMP_P		const void
typedef	off_t		KA_T;
#endif	/* DUV<50000 */

#define DEVINCR		1024		/* device table malloc() increment */
#define	DIRTYPE		dirent
#define	KMEM		"/dev/kmem"
#define MALLOC_P	char
#define FREE_P		MALLOC_P
#define MALLOC_S	size_t
#define MAXSYSCMDL	MAXCOMLEN	/* max system command name length */
#define	PNSIZ		5		/* /proc PID name component length */
#define PR_INOBIAS	64		/* /proc inode number bias */
#define PR_ROOTINO	2		/* /proc root inode number */
#define	PROCMIN		3		/* processes that make a "good" scan */
#define	PROCTRYLM	5		/* times to try to read proc table */
#define QSORT_P		char
#define	READLEN_T	int
#define STRNCPY_L	int
#define	U_SIZE		sizeof(struct user)


/*
 * Global storage definitions (including their structure definitions)
 */

extern int CloneMaj;
extern struct file *Fileptr;
#define	FILEPTR	Fileptr			/* for process_file() in lib/prfp.c */
extern int	HaveCloneMaj;
extern int Kd;

struct l_vfs {
	KA_T addr;			/* kernel address */
	fsid_t fsid;			/* file system ID */
	short type;			/* type of file system */
	char *dir;			/* mounted directory */
	char *fsname;			/* file system name */

# if	defined(HASFSINO)
	INODETYPE fs_ino;			/* file system inode number */
# endif	/* defined(HASFSINO) */

	dev_t dev;			/* device number */
	dev_t rdev;			/* raw device number */
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
	fsid_t fsid;			/* directory file system ID */
};
extern struct mounts *Mtab;

#define	X_NCACHE	"ncache"
#define	X_NCSIZE	"ncsize"
#define NL_NAME		n_name

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

# if	DUV>=30000
extern KA_T *Pa;			/* kernel proc structure addresses */
# endif	/* DUV>=30000 */

extern struct proc *Ps;			/* local proc structures */
extern int Psn;				/* entries in Pa[] and Ps[] */

extern int Vnmxp;


/*
 * Definitions for dvch.c, isfn.c, and rdev.c
 */

#define	CLONEMAJ	CloneMaj	/* clone major variable name */
#define	DCACHE_CLR	clr_sect	/* function to clear clone cache
					 * when reading the device cache
					 * file fails */
#define	HASDNAMLEN	1		/* DIRTYPE has d_namlen element */
#define	HAS_STD_CLONE	1		/* has standard clone structure */
#define	HAVECLONEMAJ	HaveCloneMaj	/* clone major variable status name */


/*
 * Definitions for rnam.c
 */

# if     defined(HASNCACHE) && DUV<50100
#include <sys/namei.h>
#define	NCACHE		namecache	/* kernel's structure name */
#define	NCACHE_NM	nc_name		/* name in NCACHE */
#define	NCACHE_NMLEN	nc_nlen		/* name length in NCACHE */
#define	NCACHE_NODEADDR	nc_vp		/* node address in NCACHE */
#define	NCACHE_PARADDR	nc_dvp		/* parent node address in NCACHE */

#  if	defined(HASNCVPID)
#define	NCACHE_NODEID	nc_vpid		/* node ID in NCACHE */
#define	NCACHE_PARID	nc_dvpid	/* parent node ID in NCACHE */
#  endif	/* defined(HASNCVPID) */
# endif  /* defined(HASNCACHE) && DUV<50100 */

#endif	/* !DU_LSOF_H */
