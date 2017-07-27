/*
 * dlsof.h - /dev/kmem-based HP-UX header file for lsof
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
 * $Id: dlsof.h,v 1.19 2007/04/24 16:25:30 abe Exp $
 */


#if	!defined(HPUX_LSOF_H)
#define	HPUX_LSOF_H	1

# if	HPUXV>=1030
#include <fcntl.h>
# endif	/* HPUXV>=1030 */

#include <stdlib.h>
#include <dirent.h>
#include <mntent.h>
#include <setjmp.h>
#include <string.h>
#include <nlist.h>
#include <unistd.h>

# if	HPUXV<1020
#include <sys/vnode.h>
# endif	/* HPUXV<1020 */

# if	HPUXV>=1030
/*
 * Include header files for HP-UX 10.30 and up that have been
 * manufactured with q4 and hand edited.
 */

#include "lla.h"
#include "proc.h"
#include "rnode.h"
#include "nfs_clnt.h"
#include "vnode.h"
# endif	/* HPUXV>=1030 */

#include <sys/domain.h>

# if	HPUXV>=1020
#define	_INCLUDE_STRUCT_FILE
# endif	/* HPUXV>=1020 */

# if	HPUXV>=1030
struct uio {		/* to satisfy function prototypes in <sys/file.h> */
	int dummy;
};
# endif	/* HPUXV>=1030 */

#include <sys/file.h>

# if	HPUXV>=1020
#undef	_INCLUDE_STRUCT_FILE
# endif	/* HPUXV>=1020 */

# if	HPUXV>=1030
#include <sys/stream.h>
#include "sth.h"
# endif	/* HPUXV>=1030 */

#include <sys/mbuf.h>

# if	HPUXV>=800
#undef	_PROTOTYPES
#include <sys/pstat.h>
# endif	/* HPUXV>=800 */

#include <sys/resource.h>

# if	HPUXV<1010
#include <sys/proc.h>
# endif	/* HPUXV<1010 */

#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <netinet/in.h>
#include <net/route.h>

# if	HPUXV<1030
#include <net/raw_cb.h>
#include <netinet/in_pcb.h>
# endif	/* HPUXV<1030 */

#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

# if	HPUXV<1030
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
# else	/* HPUXV>=1030 */
#include <sys/tihdr.h>
/*
 * Include header files for HP-UX 10.30 and up that have been
 * manufactured with q4 and hand editing.
 */

#include "ipc_s.h"
#include "tcp_s.h"
#include "udp_s.h"
# endif	/* HPUXV<1030 */

# if	HPUXV>=1030
#undef	TCP_NODELAY
#undef	TCP_MAXSEG
# endif	/* HPUXV>=1030 */

#include <rpc/types.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>

# if	HPUXV>=1030
#include <rpc/clnt_soc.h>
# endif	/* HPUXV>=1030 */

# if	HPUXV>=1000
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <sys/cdfsdir.h>
#include <sys/cdfs.h>
#include <sys/cdnode.h>
# endif	/* HPUXV>=1000 */

#include <nfs/nfs.h>

/*
 * Structure for Atria's MVFS node (ancestry: lsof 3.61 or older)
 */

struct mvfsnode {
	unsigned long d1[6];
	unsigned long m_ino;			/* node number */
};

# if	HPUXV<1030
#include <nfs/nfs_clnt.h>
#  if	defined(HASRNODE3)
/*
 * This rnode structure definition should come from <nfs/rnode.h>, but HP-UX
 * patched the kernel structures of NFS3 at PHNE_18173, PHNE_19426, PHNE_19937,
 * and PHNE_20091 and didn't supply an updated <nfs/rnode.h>.
 *
 * This definition of rnode was derived via /usr/contrib/binq4.
 */

struct rnode {
	struct rnode *r_next;
	struct vnode r_vnode;
	u_int r_fh3;
	fhandle_t r_fh;
	u_short r_flags;
	short r_error;
	daddr_t r_lastr;
	k_off_t r_size;
	k_off_t r_cachedsize;
	struct ucred *r_rcred;
	struct ucred *r_wcred;
	struct ucred *r_unlcred;
	int *r_unlname;
	struct vnode *r_unldvp;
	struct nfsfattr r_nfsattr;
};
#  else	/* !defined(HASRNODE3) */
#include <nfs/rnode.h>
#  endif	/* defined(HASRNODE3) */
# endif	/* HPUXV<1030 */

#include <nfs/snode.h>

# if	HPUXV>=1000
#define	_KERNEL
#include <nfs/fifonode.h>
#undef	_KERNEL
# endif	/* HPUXV>=1000 */

# if	defined(DTYPE_LLA) && HPUXV<1030
#define	_KERNEL	1
#include <sio/lla.h>
#undef	_KERNEL
# endif	/* defined(DTYPE_LLA) && HPUXV<1030 */

#include <sys/un.h>
#include <sys/unpcb.h>
#include <sys/vfs.h>
#include <sys/vmmac.h>
#include <sys/user.h>

/*
 * The hpux_mount.h header file is manufactured from <sys/mount.h> by the
 * Configure script to get the mount structure without needing to define
 * _KERNEL when including <sys/mount.h>.  Defining _KERNEL causes unresolvable
 * header file complications.
 */

#include "hpux_mount.h"

# if	HPUXV>=800
/*
 * These definitions are from <sys/vfs.h>, defined under the _KERNEL symbol.
 * Unfortunately, defining _KERNEL causes <sys/vfs.h> to include other
 * header files not in <sys>.
 */
#define MOUNT_UFS 0
#define MOUNT_NFS 1
#define MOUNT_CDFS 2
# endif	/* HPUXV>=800 */

# if	defined(HAS_CONST)
#define	COMP_P		const void
# else	/* !defined(HAS_CONST) */
#define	COMP_P		void
# endif	/* defined(HAS_CONST) */

# if	HPUXV>=800
#define	CURDIR	p->p_cdir
#define	ROOTDIR	p->p_rdir
# else	/* HPUXV<800 */
#define CURDIR	u->u_cdir
#define	ROOTDIR	u->u_rdir
# endif	/* HPUXV>=800 */

#define DEVINCR		1024	/* device table malloc() increment */

# if	HPUXV<1030
/*
 * KA_T is defined in dialects/hpux/kmem/hpux11/kernbits.h for HP-UX 10.30
 * and above.
 */
typedef	off_t		KA_T;
# endif	/* HPUXV<1030 */

#define	KMEM		"/dev/kmem"
#define MALLOC_P	void
#define FREE_P		void
#define MALLOC_S	unsigned
#define MOUNTED		MNT_MNTTAB

# if	HPUXV<1000
#define N_UNIX		"/hp-ux"
# else	/* HPUXV>=1000 */
#define N_UNIX		"/stand/vmunix"
# endif	/* HPUXV<1000 */

#define QSORT_P		void
#define	READLEN_T	int
#define STRNCPY_L	size_t

# if	HPUXV>=1000
#define	SZOFFTYPE	unsigned long long
				/* type for size and offset */
#define	SZOFFPSPEC	"ll"	/* SZOFFTYPE printf specification modifier */
# endif	/* HPUXV>=1000 */

#define SWAP		"/dev/swap"

# if	HPUXV<800
#define unp_addr	unp_locaddr
/*
 * HP-UX <8 SWAP must be read in DEV_BSIZE chunks.
 */
#define U_SIZE		(((DEV_BSIZE+sizeof(struct user))/DEV_BSIZE)*DEV_BSIZE)
# endif	/* HPUXV<800 */

# if	HPUXV>=800
#define	U_SIZE		sizeof(struct user)
# endif	/* HPUXV>=800 */

# if	HPUXV>=1030
#define	XDR_PMAPLIST	(xdrproc_t)xdr_pmaplist
#define	XDR_VOID	(xdrproc_t)xdr_void
# endif	/* HPUXV>=1030 */


# if	defined(HAS_AFS)
/*
 * AFS definitions
 */

#define	AFSAPATHDEF	"/usr/adm/afs/kload"
#define	AFSDEV		1		/* AFS "fake" device number */

#  if	defined(HASAOPT)
extern char *AFSApath;			/* alternate AFS name list path
					 * (from -A) */
#  endif	/* defined(HASAOPT) */

extern struct vfs *AFSVfsp;		/* AFS struct vfs kernel pointer */
# endif	/* defined(HAS_AFS) */


/*
 * Global storage definitions (including their structure definitions)
 */

extern int CloneMaj;
extern int HaveCloneMaj;
extern int Kd;
extern KA_T Kpa;

# if	HPUXV>=1010
extern KA_T Ktp;
#endif	/* HPUXV>=1010 */

struct l_vfs {
	KA_T addr;			/* kernel address */
	dev_t dev;			/* device */
	char *dir;			/* mounted directory */
	char *fsname;			/* file system name */

# if	defined(HASFSINO)
	INODETYPE fs_ino;		/* file system inode number */
# endif	/* defined(HASFSINO) */

	struct l_vfs *next;		/* forward link */
};
extern struct l_vfs *Lvfs;

# if	HPUXV<800
extern int Mem;
# endif	/* HPUXV<800 */

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

# if	HPUXV<800 && defined(hp9000s800)
extern int npids;
extern struct proc *proc;
# endif	/* HPUXV<800 && defined(hp9000s800) */

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

# if	HPUXV<800
extern int Swap;
# endif	/* HPUXV<800 */

# if	HPUXV<800 && defined(hp9000s800)
extern struct user *ubase;
# endif	/* HPUXV<800 && defined(hp9000s800) */

# if	HPUXV<800 && defined(hp9000s300)
extern struct pte *Usrptmap;
extern struct pte *usrpt;
# endif	/* HPUXV<800 && defined(hp9000s300) */

extern KA_T Vnfops;


/*
 * Definitions for dvch.c, isfn.c, and rdev.c
 */

#define	CLONEMAJ	CloneMaj	/* clone major variable name */
#define	DIRTYPE		dirent		/* directory structure type */
#define HASDNAMLEN	1		/* DIRTYPE has d_namlen element */
#define	HAS_STD_CLONE	1		/* uses standard clone structure */
#define	HAVECLONEMAJ	HaveCloneMaj	/* clone major status variable name */
#define	MAXSYSCMDL	(PST_CLEN - 1)


/*
 * Definition for rmnt.c
 */

#define MNTSKIP \
	{ if (strcmp(mp->mnt_type, MNTTYPE_IGNORE) == 0) \
		continue; }

/*
 * Definitions for rnch.c
 */

# if     defined(HASNCACHE)
#include <sys/dnlc.h>
#  if	HPUXV<1000
#define	ADDR_NCACHE	1
#  endif	/* HPUXV<1000 */
# endif  /* defined(HASNCACHE) */

#endif	/* HPUX_LSOF_H */
