/*
 * dlsof.h - pstat-based HP-UX header file for lsof
 */


/*
 * Copyright 1999 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dlsof.h,v 1.8 2008/10/21 16:17:50 abe Exp $
 */


#if	!defined(HPUX_LSOF_H)
#define	HPUX_LSOF_H	1

#include <stddef.h>
#include <stdlib.h>
#include <dirent.h>
#include <mntent.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>

# if	defined(HASIPv6)
#include <netinet/in6.h>
# endif	/* defined(HASIPv6) */

#include <rpc/types.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>

#include <sys/fstyp.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/pstat.h>

# if	defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS==64 && defined(_APP32_64BIT_OFF_T)
#define	TMP_APP32_64BIT_OFF_T	_APP32_64BIT_OFF_T
#undef	_APP32_64BIT_OFF_T
# endif

# if	!defined(__LP64__) && defined(_LARGEFILE64_SOURCE) && HPUXV>=1123
/*
 * Make sure a 32 bit lsof for HPUX>=1123 uses [l]stat64 when
 * _LARGEFILE64_SOURCE is defined.
 */

#define	stat	stat64
#define	lstat	lstat64
# endif	/* !defined(__LP64__) && defined(_LARGEFILE64_SOURCE) && HPUXV>=1123 */

#include <sys/socket.h>

# if	defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS==64 && defined(_APP32_64BIT_OFF_T)
#define	_APP32_64BIT_OFF_T	TMP_APP32_64BIT_OFF_T
#undef	TMP_APP32_64BIT_OFF_T
# endif
 
#include <sys/tihdr.h>
#include <sys/un.h>


/*
 * This definition is needed for the common function prototype definitions
 * in "proto.h".  The /proc-based lsof also uses it to make sure its
 * manufactured node ID number has 64 bits.
 */

typedef	unsigned long long	KA_T;
#define	KA_T_FMT_X		"%#llx"


/*
 * Local definitions
 */

# if	defined(HAS_CONST)
#define	COMP_P		const void
# else	/* !defined(HAS_CONST) */
#define	COMP_P		void
# endif	/* defined(HAS_CONST) */

#define DEVINCR		1024	/* device table malloc() increment */
#define MALLOC_P	void
#define FREE_P		void
#define MALLOC_S	unsigned
#define MOUNTED		MNT_MNTTAB
#define QSORT_P		void
#define	READLEN_T	int
#define STRNCPY_L	size_t
#define	SZOFFTYPE	unsigned long long
#define	SZOFFPSPEC	"ll"	/* SZOFFTYPE printf specification modifier */
#define	XDR_PMAPLIST	(xdrproc_t)xdr_pmaplist
#define	XDR_VOID	(xdrproc_t)xdr_void


/* 
 * Local macros
 */

#define	IS_PSFILEID(p)	((p)->psf_fsid.psfs_id || (p)->psf_fsid.psfs_type)
					/* is psfiled active? */


/*
 * Global storage definitions (including their structure definitions)
 */

extern _T_LONG_T CloneMaj;		/* clone major device number */
extern int HaveCloneMaj;		/* clone major status */

struct mounts {
	char *dir;              	/* directory (mounted on) */
	char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;           	/* file system
					 * (symbolic links resolved) */
	char *mnt_fstype;		/* file system type -- e.g.,
					 * MNTTYPE_NFS */
	int stat_fstype;		/* st_fstype */
	dev_t dev;              	/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory st_ino */
	mode_t mode;			/* directory st_mode */
	mode_t fs_mode;			/* file system st_mode */
	u_char is_nfs;			/* file system type is MNTTYPE_NFS or
					 * MNTTYPE_NFS3 */
	struct mounts *next;    	/* forward link */
};

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

extern char **Fsinfo;
extern int Fsinfomax;
extern int HasNFS;


/*
 * Definitions for dvch.c, isfn.c, and rdev.c
 */

#define	CLONEMAJ	CloneMaj	/* clone major variable name */
#define	DIRTYPE	dirent
#define HASDNAMLEN	1		/* DIRTYPE has d_namlen element */
#define	HAS_STD_CLONE	1		/* uses standard clone structure */
#define	HAVECLONEMAJ	HaveCloneMaj	/* clone major status variable name */
#define	MAXSYSCMDL	(PST_UCOMMLEN - 1)
					/* max system command name length */


/*
 * Definition for rmnt.c
 */

#define MNTSKIP \
	{ if (strcmp(mp->mnt_type, MNTTYPE_IGNORE) == 0) \
		continue; }
#define	RMNT_FSTYPE		mnt_type
#define	MOUNTS_FSTYPE		mnt_fstype

# if	defined(HASFSTYPE) && HASFSTYPE==2
#define	RMNT_STAT_FSTYPE	st_fstype
#define	MOUNTS_STAT_FSTYPE	stat_fstype
# endif	/* defined(HASFSTYPE) && HASFSTYPE==2 */

#endif	/* HPUX_LSOF_H */
