/*
 * dlsof.h - SCO UnixWare header file for lsof
 */


/*
 * Copyright 1996 Purdue Research Foundation, West Lafayette, Indiana
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


#if	!defined(UW_LSOF_H)
#define	UW_LSOF_H	1

#include <dirent.h>
#include <fcntl.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/conf.h>
#include <sys/exec.h>
#include <sys/mkdev.h>
#include <sys/mnttab.h>
#include <sys/mntent.h>
#include <netdb.h>
#include <string.h>
#include <nlist.h>
#include <sys/file.h>
#include <sys/flock.h>
#include <sys/immu.h>
#include <sys/fstyp.h>
#include <sys/fsid.h>

# if	UNIXWAREV>=70103
#include <sys/poll.h>
#include <sys/fs/prdata.h>
#define	PR_PIDDIR	PR_PIDCAT_DIR
#define	PR_AS		PR_PIDCAT_AS
#define	PR_CTL		PR_PIDCAT_CTL
#define	PR_STATUS	PR_PIDCAT_STATUS
#define	PR_MAP		PR_PIDCAT_MAP
#define	PR_CRED		PR_PIDCAT_CRED
#define	PR_SIGACT	PR_PIDCAT_SIGACT
#define	PR_OBJECTDIR	PR_PIDCAT_OBJECTDIR
#define	PR_LWPDIR	PR_PIDCAT_LWP_DIR
#define	PR_LWPIDDIR	PR_PIDCAT_LWP_IDDIR
#define	PR_LWPCTL	PR_PIDCAT_LWP_CTL
#define	PR_LWPSTATUS	PR_PIDCAT_LWP_STATUS
#define	PR_LWPSINFO	PR_PIDCAT_LWP_SINFO
# endif	/* UNIXWAREV>=70103 */

#include <sys/vfs.h>
#include <sys/vnode.h>
#include <sys/fs/memfs_mnode.h>
#include <sys/fs/namenode.h>

# if	UNIXWAREV>=70000
#undef	IREAD
#undef	IWRITE
#undef	IEXEC
# endif	/* UNIXWAREV>=70000 */

#include <sys/fs/snode.h>

# if	UNIXWAREV<20102
#include <fs/proc/prdata.h>
# else	/* UNIXWAREV>=20102 */
#  if	UNIXWAREV<70103
#include <fs/procfs/prdata.h>
#  endif	/* UNIXWAREV<70103 */
# endif	/* UNIXWAREV<20102 */

#include <sys/mount.h>
#include <sys/stream.h>
#include <sys/strsubr.h>
#include <sys/sysmacros.h>
#undef	major
#undef	minor
#define	major(d)	(((d) >> L_BITSMINOR) & L_MAXMAJ)
#define	minor(d)	((d) & L_MAXMIN)
#include <sys/time.h>
#include <sys/fs/s5dir.h>
#include <signal.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/var.h>
#include <sys/procfs.h>
#include <sys/priocntl.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <sys/un.h>
#include <rpc/types.h>
#include <nfs/nfs.h>

#define	_KERNEL
#include <sys/fs/fifonode.h>

# if	UNIXWAREV>=70000
#include <rpc/xdr.h>
# endif	/* UNIXWAREV>=70000 */

#include <nfs/rnode.h>
#undef	_KERNEL

#include <netinet/in.h>

# if	defined(HASIPv6)
#include <netinet/in6.h>
#  if	!defined(IN6_ARE_ADDR_EQUAL)
#define	IN6_ARE_ADDR_EQUAL	IN6_ADDR_EQUAL	/* required by RFC2292 */
#  endif	/* !defined(IN6_ARE_ADDR_EQUAL) */
# endif	/* defined(HASIPv6) */

#include <rpc/rpc.h>
#include <rpc/clnt_soc.h>
#include <rpc/pmap_prot.h>
#include <rpc/rpcent.h>
#include <sys/socket.h>
#include <net/route.h>

# if	defined(HAS_INKERNEL)
#define	INKERNEL
# endif	/* defined(HAS_INKERNEL) */

#include <netinet/in_pcb.h>

# if	defined(HAS_INKERNEL)
#undef	INKERNEL
# endif	/* defined(HAS_INKERNEL) */

# if	UNIXWAREV>=70000
#undef	TCP_MAXSEG
#undef	TCP_NODELAY
# endif	/* UNIXWAREV>=70000 */

#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>

# if	UNIXWAREV<70000
#include <netinet/tcp_kern.h>
# else	/* UNIXWAREV>=70000 */
#include <netinet/tcp_timer.h>

#  if	defined(HAS_INKERNEL)
#define	INKERNEL
#  endif	/* defined(HAS_INKERNEL) */

#include <netinet/tcp_var.h>

#  if	defined(HAS_INKERNEL)
#undef	INKERNEL
#  endif	/* defined(HAS_INKERNEL) */
# endif	/* UNIXWAREV<70000 */

#include <sys/protosw.h>
#include <sys/socketvar.h>
#include <sys/sockmod.h>

# if	UNIXWAREV>=70101
#undef	SS_ISBOUND
#undef	SS_ISCONNECTED
#undef	SS_ISCONNECTING
#undef	SS_CANTRCVMORE
#undef	SS_CANTSENDMORE
#include <sys/socksys.h>
# endif	/* UNIXWAREV>=70101 */

#include <sys/tiuser.h>
#include <vm/hat.h>
#include <vm/as.h>
#include <vm/seg.h>

# if	UNIXWAREV>=70000 && UNIXWAREV<70103
typedef ulong_t channel_t;		/* also in types.h #if _KERNEL */
# endif	/* UNIXWAREV>=70000 && UNIXWAREV<70103 */

#include <vm/seg_dev.h>
#include <vm/seg_map.h>
#include <vm/seg_vn.h>

#define	COMP_P		const void
#define DEVINCR		1024	/* device table malloc() increment */
#define	DIRTYPE		dirent
#define	FSNAMEL		4
typedef	off_t		KA_T;
#define	KMEM		"/dev/kmem"

# if	defined(HAS_UW_NSC)
#define N_UNIX		"/unix"
# else	/* !defined(HAS_UW_NSC) */
#define N_UNIX		"/stand/unix"
# endif	/* defined(HAS_UW_NSC) */

# if	UNIXWAREV<70103
#define MALLOC_P	char
#define MALLOC_S	unsigned
# else	/* UNIXWAREV>=70103 */
#define MALLOC_P	void
#define MALLOC_S	size_t
# endif	/* UNIXWAREV<70103 */

#define FREE_P		MALLOC_P
#define MAXSEGS		100	/* maximum text segments */
#define	MAXSYSCMDL	(PSCOMSIZ - 1)	/* max system command name length */
#define	PROCFS		"/proc"
#define	PROCINCR	32	/* local proc table malloc increment */
#define	PROCMIN		3	/* processes that make a "good" scan */
#define	PROCSIZE	sizeof(struct proc)
#define	PROCTRYLM	5	 /* times to try to read proc table */
#define QSORT_P		char

# if	UNIXWAREV<7000
#define	READLEN_T	unsigned
# else	/* UNIXWAREV>=7000 */
#define	READLEN_T	size_t
# endif	/* UNIXWAREV<7000 */

# if	defined(HASPROCFS)
#define	PNSIZ		5	/* size of /HASPROCFS names */
#define	PR_INOBIAS	64L	/* /HASPROCFS PID to i_number bias */
#define	PR_ROOTINO	2	/* /HASPROCFS root inode number */
# endif

#define STRNCPY_L	size_t
#define	STRNML		32

# if	UNIXWAREV>=70000
#define	SZOFFTYPE	unsigned long long
					/* type for size and offset */
#define	SZOFFPSPEC	"ll"		/* SZOFFTYPE printf specification
					 * modifier */
/*
 * Use the 64 bit stat() functions, so that lsof can get parameters on
 * large and small files.
 */
#define	fstat		fstat64
#define	lstat		lstat64
#define	stat		stat64
# else	/* UNIXWAREV<70000 */
#define	SZOFFTYPE	unsigned long	/* type for size and offset */
#define	SZOFFPSPEC	"l"		/* SZOFFTYPE printf specification modifier */
# endif	/* UNIXWAREV>=7000 */

#define U_SIZE		sizeof(struct user)


/*
 * Global storage definitions (including their structure definitions)
 */

extern int CloneMaj;
extern char **Fsinfo;
extern int Fsinfomax;
extern int HaveCloneMaj;
extern int Kd;

struct l_ino {
	dev_t dev;			/* device */
	long nlink;			/* link count */
	char *nm;			/* name */
	INODETYPE number;		/* inode number */
	dev_t rdev;			/* raw device */
	SZOFFTYPE size;			/* file size */
	unsigned char dev_def;		/* dev is defined */
	unsigned char nlink_def;	/* link count is defined */
	unsigned char number_def;	/* number is defined */
	unsigned char rdev_def;		/* rdev is defined */
	unsigned char size_def;		/* size is defined */
};

struct mounts {
	char *dir;			/* directory (mounted on) */
	char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;           	/* file system
					 * (symbolic links resolved) */
	dev_t dev;			/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory st_ino */
	mode_t mode;			/* directory st_mode */
	mode_t fs_mode;			/* file system st_mode */
	struct mounts *next;    	/* forward link */

# if	defined(HASFSTYPE)
	char *fstype;			/* st_fstype */
# endif

};

extern short Nfstyp;

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

#include <setjmp.h>


/*
 * Definition for ckfa.c
 */

#define	CKFA_XDEVTST strcmp(sb.st_fstype,"cdfs")==0


/*
 * Definition for dvch.c, isfn.c, and rdev.c
 */

#define	CLONEMAJ	CloneMaj	/* clone major variable name */
#define	HAS_STD_CLONE	1		/* has standard clone handling */
#define	HAVECLONEMAJ	HaveCloneMaj	/* clone major status variable name */


/*
 * Definitions for rnch.c
 */

#if     defined(HASNCACHE)
#include <sys/dnlc.h>
#endif  /* defined(HASNCACHE) */

#endif	/* UW_LSOF_H */
