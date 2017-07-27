/*
 * dlsof.h - SCO OpenServer header file for lsof
 */


/*
 * Copyright 1995 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dlsof.h,v 1.14 2007/04/24 16:22:40 abe Exp $
 */


#if	!defined(OSR_LSOF_H)
#define	OSR_LSOF_H	1

#include <dirent.h>
#include <fcntl.h>
#include <mnttab.h>
#include <nlist.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>

# if	OSRV>=500
#include <strings.h>
# endif	/* OSRV>=500 */

#include <stdlib.h>
#include <unistd.h>

#include <sys/conf.h>
#include <sys/file.h>
#include <sys/flock.h>
#include <sys/fstyp.h>
#include <sys/immu.h>
#include <sys/inode.h>
#include <sys/region.h>
#include <sys/proc.h>
#include <sys/sysi86.h>

/*
 * This confusing sequence of redefinitions of xdevmap allows lsof to size
 * its copy of the kernel's xdevmap[] table dynamically, based on the
 * kernel's nxdevmaps value.
 *
 * The net result is that there is a dummy struct XDEVMAP[1], defined in
 * dstore.c, that is never used.  The copy of the kernel's xdevmap[] table
 * is stored in the space malloc()'d in dproc.c and addressed by Xdevmap.
 * The last redefinition of xdevmap to Xdevmap causes the macros of
 * <sys/sysmacros.h> to use Xdevmap.
 *
 * All this is done: 1) to avoid having to allocate a large amount of fixed
 * space in advance to a copy of the kernel's xdevmap; and 2) to keep CC from
 * complaining about the absence of a "struct xdevmap xdevmap[]," matching
 * the "extern struct xdevmap xdevmap[]" declaration in <sys/sysmacros.h>,
 * while still allowing lsof to use the equivalent of a "struct xdevmap *"
 * construct instead, particularly with the kernel forms of the major() and
 * minor() macros.
 */

#define	xdevmap	XDEVMAP
#define	_INKERNEL
#include <sys/sysmacros.h>
#undef	_INKERNEL
extern struct XDEVMAP *Xdevmap;
#undef	xdevmap
#define	xdevmap	Xdevmap

#include <sys/stream.h>
#include <sys/time.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/var.h>

# if	defined(HAS_NFS)
#define	multiple_groups	1
#include <sys/fs/nfs/types.h>
#include <sys/fs/nfs/nfs.h>
#include <sys/fs/nfs/ucred.h>
#include <sys/fs/nfs/rnode.h>
# endif	/* defined(HAS_NFS) */

#include <sys/socket.h>
#include <sys/net/domain.h>
#undef	NOGROUP
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>

# if	OSRV<500
#include <sys/net/protosw.h>
#include <sys/net/socketvar.h>
# else	/* OSRV>=500 */
#include <sys/protosw.h>
#include <sys/socketvar.h>
#include <sys/un.h>
#include <sys/fs/hpps.h>
# endif	/* OSRV<500 */

#include <sys/netinet/in.h>
#include <sys/net/route.h>
#include <sys/netinet/in_pcb.h>
#include <sys/netinet/ip_var.h>
#include <sys/netinet/tcp.h>
#include <sys/netinet/tcp_fsm.h>
#include <sys/netinet/tcp_timer.h>
#include <sys/netinet/tcp_var.h>
#include <sys/netinet/udp.h>
#include <sys/utsname.h>

#define	INKERNEL
#include <sys/netinet/udp_var.h>
#undef	INKERNEL


/*
 * Adjust for the availability of symbolic links.
 */

# if	defined(HAS_STATLSTAT)
#define	lstat	statlstat
# else	/* !defined(HAS_STATLSTAT) */
#define	lstat	stat
#define	readlink(path, buf, len)	(-1)
# endif	/* defined(HAS_STATLSTAT) */



#define	COMP_P		const void
#define DEVINCR		1024	/* device table malloc() increment */
#define	DIRTYPE		dirent
typedef	off_t		KA_T;
#define	KMEM		"/dev/kmem"
#define MALLOC_P	void
#define	MNTTAB		"/etc/mnttab"
#define FREE_P		MALLOC_P
#define MALLOC_S	size_t

# if	!defined(MAXPATHLEN)
#define	MAXPATHLEN	1024
# endif	/* !defined(MAXPATHLEN) */

#define MAXSEGS		100	/* maximum text segments */
#define	MAXSYSCMDL	(PSCOMSIZ - 1)	/* max system command name length */

# if	OSRV<500
#define	N_UNIX		"/unix"
# endif	/* OSRV<500 */

#define	PROCBFRD	16	/* count of proc structures buffered */
#define	PROCSIZE	sizeof(struct proc)
#define QSORT_P		void
#define	READLEN_T	unsigned
#define STRNCPY_L	size_t
#define	STRNML		32
#define U_SIZE		sizeof(struct user)


/*
 * Global storage definitions (including their structure definitions)
 */

extern char **Cdevsw;
extern int Cdevcnt;
extern int CloneMajor;
extern int EventMajor;
extern char **Fsinfo;
extern int Fsinfomax;
extern int HaveCloneMajor;
extern int HaveEventMajor;
extern int HaveSockdev;
extern int Hz;
extern int Kd;
extern KA_T Lbolt;

extern int nxdevmaps;			/* maximum kernel xdevmap[] index */

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
	struct mounts *next;		/* forward link */

# if	defined(HASFSTYPE)
	char *fstype;			/* st_fstype */
# endif

};

#define	NL_NAME		n_name		/* name element in struct nlist */

struct sfile {
	char *aname;			/* argument file name */
	char *name;			/* file name (after readlink()) */
	char *devnm;			/* device name (optional) */
	dev_t dev;			/* device */
	dev_t rdev;			/* raw device */
	mode_t mode;			/* S_IFMT mode bits from stat() */
	int type;			/* file type: 0 = file system
				 	 *	      1 = regular file */
	INODETYPE i;			/* inode number */
	int f;				/* file found flag */
	struct sfile *next;		/* forward link */
};

extern int Sockdev;
extern KA_T Socktab;

/*
 * Definitions for dvch.c, isfn.c, and rdev.c
 */

#define	CLONEMAJ	CloneMajor	/* clone major variable name */

# if	defined(HASDCACHE)
#  if	OSRV<500
#define	DVCH_CHOWN	1		/* no fchown() below release 5.0 */
#  endif	/* OSRV<500 */
# endif	/* defined(HASDCACHE) */

#define	HAS_STD_CLONE	1		/* has standard clone structure */
#define	HAVECLONEMAJ	HaveCloneMajor	/* clone major status variable name */

#endif	/* OSR_LSOF_H	*/
