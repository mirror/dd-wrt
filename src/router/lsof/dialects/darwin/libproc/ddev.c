/*
 * ddev.c -- Darwin device support functions for libproc-based lsof
 */


/*
 * Portions Copyright 2005 Apple Computer, Inc.  All rights reserved.
 *
 * Copyright 2005 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Allan Nathanson, Apple Computer, Inc., and Victor A.
 * Abell, Purdue University.
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors, nor Apple Computer, Inc. nor Purdue University
 *    are responsible for any consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either
 *    by explicit claim or by omission.  Credit to the authors, Apple
 *    Computer, Inc. and Purdue University must appear in documentation
 *    and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */


#ifndef lint
static char copyright[] =
"@(#) Copyright 2005 Apple Computer, Inc. and Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: ddev.c,v 1.2 2006/03/27 23:23:13 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#if	defined(DVCH_DEVPATH)
#define	DDEV_DEVPATH	DVCH_DEVPATH
#else	/* !defined(DVCH_DEVPATH) */
#define	DDEV_DEVPATH	"/dev"
#endif	/* defined(DVCH_DEVPATH) */

#define	LIKE_BLK_SPEC	"like block special"
#define	LIKE_CHR_SPEC	"like character special"

#if	defined(USE_STAT)
#define	STATFN	stat
#else	/* !defined(USE_STAT) */
#define	STATFN	lstat
#endif	/* defined(USE_STAT) */


/*
 * Local static variables.
 */

static dev_t *ADev = (dev_t *) NULL;	/* device numbers besides DevDev found
					 * inside DDEV_DEVPATH */
static int ADevA = 0;			/* entries allocated to ADev[] */
static int ADevU = 0;			/* entries used in ADev[] */


/*
 * Local function prototypes
 */

_PROTOTYPE(static int rmdupdev,(struct l_dev ***dp, int n, char *nm));
_PROTOTYPE(static void saveADev,(struct stat *s));


#if	defined(HASSPECDEVD)
/*
 * HASSPECDEVD() -- process stat(2) result to see if the device number is
 *		    inside DDEV_DEVPATH "/"
 *
 * exit: s->st_dev changed to DevDev, as required
 */

void
HASSPECDEVD(p, s)
	char *p;			/* file path */
	struct stat *s;			/* stat(2) result for file */
{
	int i;

	switch (s->st_mode & S_IFMT) {
	case S_IFCHR:
	case S_IFBLK:
	    if (s->st_dev == DevDev)
		return;
	    (void) readdev(0);
	    if (!ADev)
		return;
	    for (i = 0; i < ADevU; i++) {
		if (s->st_dev == ADev[i]) {
		    s->st_dev = DevDev;
		    return;
		}
	    }
	}
}
#endif	/* defined(HASSPECDEVD) */


/*
 * printdevname() -- print character device name
 */

int
printdevname(dev, rdev, f, nty)
	dev_t	*dev;		/* device */
	dev_t	*rdev;		/* raw device */
	int	f;		/* 1 = follow with '\n' */
	int	nty;		/* node type: N_BLK or N_chr */
{
	char *cp, *ttl;
	struct l_dev *dp;
	int i, len;
/*
 * See if the device node resides in DDEV_DEVPATH.  If it does, return zero
 * to indicate the vnode path is to be used for the NAME column.
 */
	if (*dev == DevDev)
	    return(0);
	readdev(0);
	for (i = 0; i < ADevU; i++) {
	    if (*dev == ADev[i])
		return(0);
	}
/*
 * This device is not in DDEV_DEVPATH.
 *
 * See if it has a DDEV_DEVPATH analogue by searching the device table for a
 * match without inode number and dev.
 */

#if	defined(HASBLKDEV)
	if (nty == N_BLK)
	    dp = lkupbdev(&DevDev, rdev, 0, 1);
	else
#endif	/* defined(HASBLKDEV) */

	    dp = lkupdev(&DevDev, rdev, 0, 1);
	if (dp) {

	/*
	 * A match was found.  Record it as a name column addition.
	 */
	    ttl = (nty == N_BLK) ? LIKE_BLK_SPEC : LIKE_CHR_SPEC;
	    len = (int)(1 + strlen(ttl) + 1 + strlen(dp->name) + 1);
	    if (!(cp = (char *)malloc((MALLOC_S)(len + 1)))) {
		(void) fprintf(stderr, "%s: no nma space for: (%s %s)\n",
		    Pn, ttl, dp->name);
		Exit(1);
	    }
	    (void) snpf(cp, len + 1, "(%s %s)", ttl, dp->name);
	    (void) add_nma(cp, len);
	    (void) free((MALLOC_P *)cp);
	}
/*
 * Return zero to indicate the vnode path is to be used for the NAME column.
 */
	return(0);
}


/*
 * readdev() -- read device names, modes and types
 */

void
readdev(skip)
	int skip;			/* skip device cache read if 1 --
					 * ignored since device cache not
					 * used */
{
	DIR *dfp;
	int dnamlen;
	struct dirent *dp;
	char *fp = (char *)NULL;
	char *path = (char *)NULL;
	int i = 0;
	int j = 0;
	MALLOC_S pl, sz;
	struct stat sb;
/*
 * Read device names but once.
 */
	if (Sdev)
	    return;
/*
 * Prepare to scan DDEV_DEVPATH.
 */
	Dstkn = Dstkx = 0;
	Dstk = (char **)NULL;
	(void) stkdir(DDEV_DEVPATH);
/*
 * Unstack the next directory.
 */
	while (--Dstkx >= 0) {
	    if (!(dfp = OpenDir(Dstk[Dstkx]))) {

# if	defined(WARNDEVACCESS)
		if (!Fwarn) {
		    (void) fprintf(stderr, "%s: WARNING: can't open: ", Pn);
		    safestrprt(Dstk[Dstkx], stderr, 1);
		}
# endif	/* defined(WARNDEVACCESS) */

		(void) free((FREE_P *)Dstk[Dstkx]);
		Dstk[Dstkx] = (char *)NULL;
		continue;
	    }
	    if (path) {
		(void) free((FREE_P *)path);
		path = (char *)NULL;
	    }
	    if (!(path = mkstrcat(Dstk[Dstkx], -1, "/", 1, (char *)NULL, -1,
				  &pl)))
	    {
		(void) fprintf(stderr, "%s: no space for: ", Pn);
		safestrprt(Dstk[Dstkx], stderr, 1);
		Exit(1);
	    }
	    (void) free((FREE_P *)Dstk[Dstkx]);
	    Dstk[Dstkx] = (char *)NULL;
	/*
	 * Scan the directory.
	 */
	    for (dp = ReadDir(dfp); dp; dp = ReadDir(dfp)) {
		if (dp->d_ino == 0 || dp->d_name[0] == '.')
		    continue;
	    /*
	     * Form the full path name and get its status.
	     */
		dnamlen = (int)dp->d_namlen;
		if (fp) {
		    (void) free((FREE_P *)fp);
		    fp = (char *)NULL;
		}
		if (!(fp = mkstrcat(path, pl, dp->d_name, dnamlen,
				    (char *)NULL, -1, (MALLOC_S *)NULL)))
		{
		    (void) fprintf(stderr, "%s: no space for: ", Pn);
		    safestrprt(path, stderr, 0);
		    safestrprtn(dp->d_name, dnamlen, stderr, 1);
		    Exit(1);
		}
		if (STATFN(fp, &sb) != 0) {
		    if (errno == ENOENT)	/* a sym link to nowhere? */
			continue;

# if	defined(WARNDEVACCESS)
		    if (!Fwarn) {
			int errno_save = errno;

			(void) fprintf(stderr, "%s: can't stat ", Pn);
			safestrprt(fp, stderr, 0);
			(void) fprintf(stderr, ": %s\n", strerror(errno_save));
		    }
# endif	/* defined(WARNDEVACCESS) */

		    continue;
		}
	    /*
	     * If it's a subdirectory, stack its name for later
	     * processing.
	     */
		if ((sb.st_mode & S_IFMT) == S_IFDIR) {

		/*
		 * Skip /dev/fd.
		 */
		    if (strcmp(fp, "/dev/fd"))
			(void) stkdir(fp);
		    continue;
		}
		if ((sb.st_mode & S_IFMT) == S_IFLNK) {

		/*
		 * Ignore symbolic links.
		 */
		    continue;
		}
		if ((sb.st_mode & S_IFMT) == S_IFCHR) {

		/*
		 * Save character device information in Devtp[].
		 */
		    if (i >= Ndev) {
			Ndev += DEVINCR;
			if (!Devtp)
			    Devtp = (struct l_dev *)malloc(
				    (MALLOC_S)(sizeof(struct l_dev)*Ndev));
			else
			    Devtp = (struct l_dev *)realloc((MALLOC_P *)Devtp,
				    (MALLOC_S)(sizeof(struct l_dev)*Ndev));
			if (!Devtp) {
			    (void) fprintf(stderr,
				"%s: no space for character device\n", Pn);
			    Exit(1);
			}
		    }
		    Devtp[i].rdev = sb.st_rdev;
		    Devtp[i].inode = (INODETYPE)sb.st_ino;
		    if (!(Devtp[i].name = mkstrcpy(fp, (MALLOC_S *)NULL))) {
			(void) fprintf(stderr,
			    "%s: no space for device name: ", Pn);
			safestrprt(fp, stderr, 1);
			Exit(1);
		    }
		    Devtp[i].v = 0;
		    i++;
		}

# if	defined(HASBLKDEV)
		if ((sb.st_mode & S_IFMT) == S_IFBLK) {

		/*
		 * Save block device information in BDevtp[].
		 */
		    if (j >= BNdev) {
			BNdev += DEVINCR;
			if (!BDevtp)
			    BDevtp = (struct l_dev *)malloc(
				     (MALLOC_S)(sizeof(struct l_dev)*BNdev));
			else
			    BDevtp = (struct l_dev *)realloc((MALLOC_P *)BDevtp,
				     (MALLOC_S)(sizeof(struct l_dev)*BNdev));
			if (!BDevtp) {
			    (void) fprintf(stderr,
				"%s: no space for block device\n", Pn);
			    Exit(1);
			}
		    }
		    BDevtp[j].name = fp;
		    fp = (char *)NULL;
		    BDevtp[j].inode = (INODETYPE)sb.st_ino;
		    BDevtp[j].rdev = sb.st_rdev;
		    BDevtp[j].v = 0;
		    j++;
		}
# endif	/* defined(HASBLKDEV) */

	    /*
	     * Save a possible new st_dev number within DDEV_DEVPATH.
	     */
		if (sb.st_dev != DevDev)
		    (void) saveADev(&sb);
	    }
	    (void) CloseDir(dfp);
	}
/*
 * Free any unneeded space that was allocated.
 */
	if (ADev && (ADevU < ADevA)) {

	/*
	 * Reduce space allocated to additional DDEV_DEVPATH device numbers.
	 */
	    if (!ADevU) {

	    /*
	     * If no space was used, free the entire allocation.
	     */
		(void) free((FREE_P *)ADev);
		ADev = (dev_t *)NULL;
		ADevA = 0;
	    } else {

	    /*
	     * Reduce the allocation to what was used.
	     */
		sz = (MALLOC_S)(ADevU * sizeof(dev_t));
		if (!(ADev = (dev_t *)realloc((MALLOC_P *)ADev, sz))) {
		    (void) fprintf(stderr, "%s: can't reduce ADev[]\n", Pn);
		    Exit(1);
		}
	    }
	}
	if (!Dstk) {
	    (void) free((FREE_P *)Dstk);
	    Dstk = (char **)NULL;
	}
	if (fp)
	    (void) free((FREE_P *)fp);
	if (path)
	    (void) free((FREE_P *)path);

# if	defined(HASBLKDEV)
/*
 * Reduce the BDevtp[] (optional) and Devtp[] tables to their minimum
 * sizes; allocate and build sort pointer lists; and sort the tables by
 * device number.
 */
	if (BNdev) {
	    if (BNdev > j) {
		BNdev = j;
		BDevtp = (struct l_dev *)realloc((MALLOC_P *)BDevtp,
			 (MALLOC_S)(sizeof(struct l_dev) * BNdev));
	    }
	    if (!(BSdev = (struct l_dev **)malloc(
			  (MALLOC_S)(sizeof(struct l_dev *) * BNdev))))
	    {
		(void) fprintf(stderr,
		    "%s: no space for block device sort pointers\n", Pn);
		Exit(1);
	    }
	    for (j = 0; j < BNdev; j++) {
		BSdev[j] = &BDevtp[j];
	    }
	    (void) qsort((QSORT_P *)BSdev, (size_t)BNdev,
		(size_t)sizeof(struct l_dev *), compdev);
	    BNdev = rmdupdev(&BSdev, BNdev, "block");
	}
	
#  if	!defined(NOWARNBLKDEV)
	else {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: no block devices found\n", Pn);
	}
#  endif	/* !defined(NOWARNBLKDEV) */
# endif	/* defined(HASBLKDEV) */

	if (Ndev) {
	    if (Ndev > i) {
		Ndev = i;
		Devtp = (struct l_dev *)realloc((MALLOC_P *)Devtp,
			(MALLOC_S)(sizeof(struct l_dev) * Ndev));
	    }
	    if (!(Sdev = (struct l_dev **)malloc(
			 (MALLOC_S)(sizeof(struct l_dev *) * Ndev))))
	    {
		(void) fprintf(stderr,
		    "%s: no space for character device sort pointers\n", Pn);
		Exit(1);
	    }
	    for (i = 0; i < Ndev; i++) {
		Sdev[i] = &Devtp[i];
	    }
	    (void) qsort((QSORT_P *)Sdev, (size_t)Ndev,
		(size_t)sizeof(struct l_dev *), compdev);
	    Ndev = rmdupdev(&Sdev, Ndev, "char");
	} else {
	    (void) fprintf(stderr, "%s: no character devices found\n", Pn);
	    Exit(1);
	}
}


/*
 * rmdupdev() - remove duplicate (major/minor/inode) devices
 */

static int
rmdupdev(dp, n, nm)
	struct l_dev ***dp;	/* device table pointers address */
	int n;			/* number of pointers */
	char *nm;		/* device table name for error message */
{
	int i, j, k;
	struct l_dev **p;

	for (i = j = 0, p = *dp; i < n ;) {
	    for (k = i + 1; k < n; k++) {
		if (p[i]->rdev != p[k]->rdev || p[i]->inode != p[k]->inode)
		    break;
	    }
	    if (i != j)
		p[j] = p[i];
	    j++;
	    i = k;
	}
	if (n == j)
	    return(n);
	if (!(*dp = (struct l_dev **)realloc((MALLOC_P *)*dp,
		    (MALLOC_S)(j * sizeof(struct l_dev *)))))
	{
	    (void) fprintf(stderr, "%s: can't realloc %s device pointers\n",
		Pn, nm);
	    Exit(1);
	}
	return(j);
}


/*
 * saveADev() - save additional device number appearing inside DDEV_DEVPATH
 */

static void
saveADev(s)
	struct stat *s;			/* stat(2) buffer for file */
{
	int i;
	MALLOC_S sz;
/*
 * Process VCHR files.
 *
 * Optionally process VBLK files.
 */

#if	defined(HASBLKDEV)
	if (((s->st_mode & S_IFMT) != S_IFBLK)
	&&  ((s->st_mode & S_IFMT) != S_IFCHR))
#else	/* !defined(HASBLKDEV) */
	if ((s->st_mode & S_IFCHR) != S_IFCHR)
#endif	/* defined(HASBLKDEV) */

		return;
/*
 * See if this is a new VBLK or VCHR st_dev value for ADev[].
 */
	for (i = 0; i < ADevU; i++) {
	    if (s->st_dev == ADev[i])
		return;
	}
/*
 * This is a new device number to add to ADev[].
 */
	if (ADevU >= ADevA) {
	    ADevA += 16;
	    sz = (MALLOC_S)(ADevA * sizeof(dev_t));
	    if (ADev)
		ADev = (dev_t *)realloc((MALLOC_P *)ADev, sz);
	    else
		ADev = (dev_t *)malloc(sz);
	    if (!ADev) {
		(void) fprintf(stderr, "%s: no space for ADev[]\n", Pn);
		Exit(1);
	    }
	}
	ADev[ADevU++] = s->st_dev;
}
