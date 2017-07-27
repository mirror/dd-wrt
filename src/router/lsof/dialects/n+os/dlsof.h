/*
 * dlsof.h - NEXTSTEP and OPENSTEP header file for lsof
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
 * $Id: dlsof.h,v 1.14 2006/03/28 22:08:17 abe Exp $
 */


#if	!defined(LSOF_NEXT_H)
#define	LSOF_NEXT_H	1

#include <c.h>
#include <stdlib.h>
#include <string.h>
#include <mntent.h>
#include <nlist.h>
#include <signal.h>
#include <setjmp.h>

# if	!defined(NCPUS)
#define NCPUS	1
# endif

#include <mach/mach.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/xdr.h>
#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#include <sys/vnode.h>
#include <sys/wait.h>
#include <nfs/rnode.h>
#include <sys/dir.h>
#include <sys/domain.h>

# if	!defined(KERNEL)
#define KERNEL
# endif

#include <sys/file.h>
#undef  KERNEL
#include <sys/mbuf.h>
#include <ufs/mount.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/stat.h>
#include <sys/ucred.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <net/route.h>
#include <net/raw_cb.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <sys/un.h>
#include <sys/unpcb.h>

# if	!defined(SHOW_UTT)
#define SHOW_UTT
# endif

/*
 * Define simple_lock_t size.
 */

# if	STEPV>=40 && defined(m68k)
#define SIMPLE_LOCK_SIZE 0
# elif	defined(hppa) /* && (STEPV<40 || !defined(m68k)) */
#define SIMPLE_LOCK_SIZE 4
# else	/* (STEPV<40 || !defined(m68k)) && !defined(hppa) */
#define SIMPLE_LOCK_SIZE 1
# endif	/* STEPV>=40 && defined(m68k) */

# if	!defined(SIMPLE_LOCK_SIZE)
#define	SIMPLE_LOCK_SIZE	1
# endif	/* !defined(SIMPLE_LOCK_SIZE) */

# if	STEPV>=40
/*
 * Define lock_data_t that was removed from OPENSTEP 4.x's <kernserv/lock.h>.
 */

typedef struct lock {
        char            *thread;
        unsigned int    read_count:16,
                        want_upgrade:1,
                        want_write:1,
                        waiting:1,
                        can_sleep:1,
                        recursion_depth:12;

#  if	SIMPLE_LOCK_SIZE>0
	caddr_t		interlock[SIMPLE_LOCK_SIZE];
#  endif	/* SIMPLE_LOCK_SIZE>0 */

} lock_data_t;
# endif	/* STEPV>=40 */

#include <sys/user.h>
#define u_comm	uu_comm
#define u_cdir	uu_cdir
#define u_rdir	uu_rdir
#undef	SHOW_UTT
#include <sys/proc.h>
#include <sys/vfs.h>
#include <ufs/inode.h>

typedef	int	pid_t;


/*
 * The following substitution compensates for the snode.h that NeXT does
 * not supply in NEXTSTEP 2.0 and above.  The value of interest is s_realvp.
 */

struct snode {
	struct	snode *s_next;		/* must be first */
	struct	vnode s_vnode;		/* vnode associated with this snode */
	struct	vnode *s_realvp;	/* vnode for the fs entry (if any) */
};


/*
 * Miscellaneous definitions.
 */

#define	COMP_P		const void
#define DEVINCR		1024		/* device table malloc() increment */
typedef	off_t		KA_T;
#define	KMEM		"/dev/kmem"
#define MALLOC_P	void
#define FREE_P		MALLOC_P
#define MALLOC_S	size_t
#define	MAXSYSCMDL	MAXCOMLEN	/* max system command name length */
#define	PROCDFLT	256	/* default size of local proc table */
#define	PROCMIN		5	/* processes that make a "good" scan */
#define PROCSIZE	sizeof(struct proc)
#define	PROCTRYLM	5	/* times to try to read proc table */
#define QSORT_P		void
#define	READLEN_T	int
#define STRNCPY_L	int
#define U_SIZE		sizeof(struct user)

#  if	!defined(VMUNIX)
#define VMUNIX		"/mach"
#  endif

#define	N_UNIX		VMUNIX


# if	defined(HAS_AFS)
/*
 * AFS definitions
 */

#define	AFSAPATHDEF	"/usr/vice/etc/afs_loadable"
#define	AFSDEV		1		/* AFS "fake" device number */

#  if	defined(HASAOPT)
extern char *AFSApath;			/* alternate AFS name list path
					 * (from -A) */
#  endif	/* defined(HASAOPT) */

extern struct vfs *AFSVfsp;		/* AFS struct vfs kernel pointer */
# endif	/* defined(HAS_AFS) */


/*
 * Local mount information
 */

struct mounts {
	char *dir;              	/* directory (mounted on) */
	char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;           	/* file system
					 * (symbolic links resolved) */
	dev_t dev;              	/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory inode number */
	u_short mode;			/* directory st_mode */
	u_short fs_mode;		/* file system st_mode */
	struct mounts *next;    	/* forward link */
};


/*
 * Defines for kernel name list
 */

#define	NL_NAME		n_un.n_name


/*
 * For kernel name cache processing
 */

# if	defined(HASNCACHE)
#include <sys/dnlc.h>
#define	X_NCACHE	"nch"
#define	X_NCSIZE	"ncsz"
# endif	/* defined(HASNCACHE) */


/*
 * Defines for library readdev() function
 */

#define	DIRTYPE		direct
#define	HASDNAMLEN	1


/*
 * Search file information
 */

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


/*
 * Miscellaneous external definitions
 */

extern struct file *Fileptr;
#define	FILEPTR	Fileptr			/* for process_file() in lib/prfp.c */
extern int Kd;

#endif	/* LSOF_NEXT_H */
