/*
 * dzfs.h - FreeBSD header file for ZFS
 */


/*
 * Copyright 2008 Purdue Research Foundation, West Lafayette, Indiana
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
 * $Id: dzfs.h,v 1.3 2011/08/07 22:51:28 abe Exp $
 */


#if	!defined(FREEBSD_ZFS_H)
#define	FREEBSD_ZFS_H	1
# if	defined(HAS_ZFS)


/*
 * The _PROTOTYPE macro provides strict ANSI C prototypes if __STDC__
 * is defined, and old-style K&R prototypes otherwise.
 *
 * (With thanks to Andy Tanenbaum)
 */

# if	defined(__STDC__)
#define	_PROTOTYPE(function, params)	function params
# else	/* !defined(__STDC__) */
#define	_PROTOTYPE(function, params)	function()
# endif /* defined(__STDC__) */


/*
 * The following define keeps gcc>=2.7 from complaining about the failure
 * of the Exit() function to return.
 *
 * Paul Eggert supplied it.
 */

# if	defined(__GNUC__) && !(__GNUC__<2 || (__GNUC__==2 && __GNUC_MINOR__<7))
#define	exiting	__attribute__((__noreturn__))
# else	/* !gcc || gcc<2.7 */
#define	exiting
# endif	/* gcc && gcc>=2.7 */

# if	!defined(INODETYPE)
#define	INODETYPE	unsigned long long
# endif	/* !defined(INODETYPE) */

# if	!defined(FREEBSD_KA_T)
#  if	FREEBSDV<2000
typedef	off_t		KA_T;
#  else	/* FREEBSDV>=2000 */
typedef	u_long		KA_T;
#  endif	/* FREEBSDV<2000 */
#define	FREEBSD_KA_T	1		/* for dlsof.h */
# endif	/* !defined(FREEBSD_KA_T) */

# if	!defined(READLEN_T)
#define	READLEN_T	int
# endif	/* !defined(READLEN_T) */

# if	!defined(SZOFFTYPE)
#define	SZOFFTYPE	unsigned long long
# endif	/* !defined(SZOFFTYPE) */


/*
 * Structure for passing znode info
 */

typedef struct zfs_info {
	INODETYPE ino;			/* inode number */
	KA_T lockf;			/* znode's z_lockf pointer */
	long nl;			/* number of links */
	dev_t rdev;			/* "raw" device number */
	SZOFFTYPE sz;			/* size */
	unsigned char ino_def;		/* ino defined status */
	unsigned char nl_def;		/* nl defined status */
	unsigned char rdev_def;		/* rdev defined status */
	unsigned char sz_def;		/* sz defined status */
} zfs_info_t;

_PROTOTYPE(extern int kread,(KA_T addr, char *buf, READLEN_T len));
_PROTOTYPE(extern char *readzfsnode,(KA_T va, zfs_info_t *zi, int vr));

# endif	/* defined(HAS_ZFS) */
#endif	/* defined(FREEBSD_DZFS_H) */
