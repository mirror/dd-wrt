/*
 * ddev.c - DEC OSF/1, Digital UNIX, Tru64 UNIX device support functions for
 *	    lsof
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

#ifndef lint
static char copyright[] =
"@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: ddev.c,v 1.17 2005/08/12 15:35:14 abe Exp $";
#endif

#include "lsof.h"


/*
 * Local static values
 */

#if	defined(USELOCALREADDIR)
static struct stat Dirsb;
#endif	/* defined(USELOCALREADDIR) */


/*
 * Local definitions
 */

#define	LIKE_BLK_SPEC	"like block special"
#define	LIKE_CHR_SPEC	"like character special"


/*
 * Local function prototypes
 */

_PROTOTYPE(static int rmdupdev,(struct l_dev ***dp, int n, char *nm));


#if	defined(HASDCACHE)
/*
 * clr_sect() - clear cached clone and pseudo sections
 */

void
clr_sect()
{
	struct clone *c, *c1;

	if (Clone) {
	    for (c = Clone; c; c = c1) {
		c1 = c->next;
		(void) free((FREE_P *)c);
	    }
	    Clone = (struct clone *)NULL;
	}
}
#endif	/* defined(HASDCACHE) */


/*
 * printdevname() - print block and character device names
 */

int
printdevname(dev, rdev, f, nty)
	dev_t *dev;			/* device */
	dev_t *rdev;			/* raw device */
	int f;				/* 1 = follow with '\n' */
	int nty;			/* node type: N_BLK or N_CHR */
{
	struct clone *c;
	struct l_dev *dp;

	readdev(0);
/*
 * Search for clone.
 */

#if     defined(HASDCACHE)

printdevname_again:

#endif  /* defined(HASDCACHE) */

	if ((nty == N_CHR) && Clone && HAVECLONEMAJ && (*dev == DevDev)
	&&  (GET_MAJ_DEV(*rdev) == CLONEMAJ))
	{
	    for (c = Clone; c; c = c->next) {
		if (Devtp[c->dx].rdev == *rdev) {

#if     defined(HASDCACHE)
		    if (DCunsafe && !Devtp[c->dx].v && !vfy_dev(&Devtp[c->dx]))
			goto printdevname_again;
#endif  /* defined(HASDCACHE) */

		    safestrprt(Devtp[c->dx].name, stdout, f);
		    return(1);
		}
	    }
	}
/*
 * Search device table for a full match.
 */

#if	defined(HASBLKDEV)
	if (nty == N_BLK)
	    dp = lkupbdev(dev, rdev, 1, 0);
	else
#endif	/* defined(HASBLKDEV) */

	dp = lkupdev(dev, rdev, 1, 0);
	if (dp) {
	    safestrprt(dp->name, stdout, f);
	    return(1);
	}
/*
 * Search device table for a match without inode number and dev.
 */

#if	defined(HASBLKDEV)
	if (nty == N_BLK)
	    dp = lkupbdev(&DevDev, rdev, 0, 0);
	else
#endif	/* defined(HASBLKDEV) */

	dp = lkupdev(&DevDev, rdev, 0, 0);
	if (dp) {

	/*
	 * A raw device match was found.  Record it as a name column addition.
	 */
	    char *cp, *ttl;
	    int len;

	    ttl = (nty == N_BLK) ? LIKE_BLK_SPEC : LIKE_CHR_SPEC;
	    len = (int)(1 + strlen(ttl) + 1 + strlen(dp->name) + 1);
	    if (!(cp = (char *)malloc((MALLOC_S)(len + 1)))) {
		(void) fprintf(stderr, "%s: no nma space for: (%s %s)\n",
		    Pn, ttl, dp->name);
		Exit(1);
	    }
	    (void) snpf(cp, len + 1, "(%s %s)", ttl, dp->name);
	    (void) add_nma(cp, len);
	    (void) free((FREE_P *)cp);
	    return(0);
	}

#if     defined(HASDCACHE)
/*
 * If the device cache is "unsafe" and we haven't found any match, reload
 * the device cache.
 */
	if (DCunsafe) {
	    (void) rereaddev();
	    goto printdevname_again;
	}
#endif  /* defined(HASDCACHE) */

	return(0);
}


/*
 * readdev() - read names, modes and device types of everything in /dev
 */

void
readdev(skip)
	int skip;			/* skip device cache read if 1 */
{
#if	defined(HASDCACHE)
	int dcrd;
#endif	/* defined(HASDCACHE) */

	struct clone *c;
	DIR *dfp;
	struct DIRTYPE *dp;
	char *fp = (char *)NULL;
	int i = 0;

#if	defined(HASBLKDEV)
	int j = 0;
#endif	/* defined(HASBLKDEV) */

	MALLOC_S nl;
	char *path = (char *)NULL;
	MALLOC_S pl;
	struct stat sb;

	if (Sdev)
	    return;

#if	defined(HASDCACHE)
/*
 * Read device cache, as directed.
 */
	if (!skip) {
	    if (DCstate == 2 || DCstate == 3) {
		if ((dcrd = read_dcache()) == 0)
		    return;
	    }
	} else
	    dcrd = 1;
#endif	/* defined(HASDCACHE) */

	Dstk = (char **)NULL;
	Dstkn = Dstkx = 0;
	(void) stkdir("/dev");
/*
 * Unstack the next /dev or /dev/<subdirectory> directory.
 */
	while (--Dstkx >= 0) {
	    if (!(dfp = OpenDir(Dstk[Dstkx]))) {

#if	defined(WARNDEVACCESS)
		if (!Fwarn) {
		    (void) fprintf(stderr, "%s: WARNING: can't open: ", Pn);
		    safestrprt(Dstk[Dstkx], stderr, 1);
		}
#endif	/* defined(WARNDEVACCESS) */

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
		if (fp) {
		    (void) free((FREE_P *)fp);
		    fp = (char *)NULL;
		}
		if (!(fp = mkstrcat(path, (int)pl, dp->d_name, dp->d_namlen,
			   (char *)NULL, -1, (MALLOC_S *)NULL)))
		{
		    (void) fprintf(stderr, "%s: no space for: ", Pn);
		    safestrprt(path, stderr, 0);
		    safestrprt(dp->d_name, stderr, 1);
		    Exit(1);
		}

#if	defined(USE_STAT)
		if (stat(fp, &sb) != 0)
#else	/* !defined(USE_STAT) */
		if (lstat(fp, &sb) != 0)
#endif	/* defined(USE_STAT) */

		{
		    if (errno == ENOENT)	/* symbolic link to nowhere? */
			continue;

#if	defined(WARNDEVACCESS)
		    if (!Fwarn) {
			int errno_save = errno;

			(void) fprintf(stderr, "%s: can't stat ", Pn);
			safestrprt(fp, stderr, 0);
			(void) fprintf(stderr, ": %s\n", strerror(errno_save));
		    }
#endif	/* defined(WARNDEVACCESS) */

		    continue;
		}
	    /*
	     * If it's a subdirectory, stack its name for later processing.
	     */
		if ((sb.st_mode & S_IFMT) == S_IFDIR) {
		    (void) stkdir(fp);
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
			    Devtp = (struct l_dev *)realloc(
				    (MALLOC_P *)Devtp,
				    (MALLOC_S)(sizeof(struct l_dev)*Ndev));
			if (!Devtp) {
			    (void) fprintf(stderr,
				"%s: no space for character device\n", Pn);
			    Exit(1);
			}
		    }
		    Devtp[i].inode = (INODETYPE)sb.st_ino;
		    if (!(Devtp[i].name = mkstrcpy(fp, (MALLOC_S *)NULL))) {
			(void) fprintf(stderr, "%s: no space for: ", Pn);
			safestrprt(fp, stderr, 1);
			Exit(1);
		    }
		    Devtp[i].rdev = sb.st_rdev;
		    Devtp[i].v = 0;
		/*
		 * Save clone device location.
		 */
		    if (HAVECLONEMAJ
		    &&   GET_MAJ_DEV(Devtp[i].rdev) == CLONEMAJ)
		    {
			if (!(c = (struct clone *)malloc(sizeof(struct clone))))
			{
			    (void) fprintf(stderr,
				"%s: no space for clone device: ", Pn);
			    safestrprt(fp, stderr, 1);
			    Exit(1);
			}
			c->dx = i;
			c->next = Clone;
			Clone = c;
		    }
		    i++;
		}

#if	defined(HASBLKDEV)
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
			    BDevtp = (struct l_dev *)realloc(
				     (MALLOC_P *)BDevtp,
				     (MALLOC_S)(sizeof(struct l_dev)*BNdev));
			if (!BDevtp) {
			    (void) fprintf(stderr,
				"%s: no space for block device\n", Pn);
				Exit(1);
			}
		    }
		    BDevtp[j].inode = (INODETYPE)sb.st_ino;
		    BDevtp[j].name = fp;
		    fp = (char *)NULL;
		    BDevtp[j].rdev = sb.st_rdev;
		    BDevtp[j].v = 0;
		    j++;
		}
#endif	/* defined(HASBLKDEV) */

	    }
	    (void) CloseDir(dfp);
	}
/*
 * Free any allocated space.
 */
	if (Dstk) {
	    (void) free((FREE_P *)Dstk);
	    Dstk = (char **)NULL;
	    Dstkn = Dstkx = 0;
	}
	if (fp)
	    (void) free((FREE_P *)fp);
	if (path)
	    (void) free((FREE_P *)path);
/*
 * Reduce the BDevtp[] (optional) and Devtp[] tables to their minimum
 * sizes; allocate and build sort pointer lists; and sort the tables by
 * device number.
 */

#if	defined(HASBLKDEV)
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

# if	!defined(NOWARNBLKDEV)
	else {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: no block devices found\n", Pn);
	}
# endif	/* !defined(NOWARNBLKDEV) */
#endif	/* defined(HASBLKDEV) */

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

#if	defined(HASDCACHE)
/*
 * Write device cache file, as required.
 */
	if (DCstate == 1 || (DCstate == 3 && dcrd))
	    write_dcache();
#endif	/* defined(HASDCACHE) */

}


#if	defined(USELOCALREADDIR)
/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This is a hacked version of NetBSD's readdir() function written to work
 * around an apparent bug in the Digital UNIX 3.0 getdirentries() system call
 * and/or their "/dev/fd" filesystem.  The problem is that when applied to
 * "/dev/fs", getdirentries() returns the wrong size, which can cause readdir()
 * to run off the end of it's internal buffer and return bogus file names.
 * 
 * The changes from the original NetBSD file are:
 *
 * - uses of the field dd_flags in the DIR structure have been removed since
 *   Digital UNIX doesn't have this field (it seems to be mostly used for
 *   dealing with whiteout's in NetBSD's union filesystem).
 *
 * - uses of the dd_len field were replaced with dd_bufsiz, since this appears
 *   to be where the Digital UNIX opendir() call stashes the size of the buffer
 *   it mallocs.  Why does Digital UNIX have both?  No idea -- as far as I
 *   could tell  dd_len was always 0.
 *
 * - code within "#ifdef BROKEN_GETDIRENTRIES ... #endif" has been added to
 *   workaround the bug.  Note: this code uses the (apparently) unused field,
 *   dd_len, in the Digital UNIX DIR structure.  This is pretty nasty, but
 *   then, this  whole routine *is* just a hack to get around a (hopefully)
 *   temporary  problem in Digital UNIX.
 *
 * This routine has only been tested on a couple of Digital UNIX 3.0 systems.
 * I make no guarantees that it will work for you...!
 *
 * Duncan McEwan (duncan@comp.vuw.ac.nz)
 */

/*
 * Additional changes by Vic Abell <abe@cc.purdue.edu>:
 *
 * - The BROKEN_GETDIRENTRIES symbol was deleted.  Use of this function
 *   is controlled in the lsof distribution by the HASLOCALREADDIR
 *   definition.
 */


/*
 * CloseDir() - close directory
 */

int
CloseDir(dirp)
	register DIR *dirp;
{
	return(closedir(dirp));
}


/*
 * OpenDir() - open directory
 */

DIR *
OpenDir(dir)
	char *dir;
{
	DIR *dirp;

	if ((dirp = opendir(dir))) {

	/*
	 * Get a stat(2) buffer for the directory.
	 *
	 * Warn if the stat(2) buffer operation fails, close the directory,
	 * and respond that the open failed.
	 */
	    if (statsafely(dir, &Dirsb)) {
		int en = errno;

		if (!Fwarn) {
		    (void) fprintf(stderr,
			"%s: WARNING: can't statsafely(", Pn);
		    safestrprt(dir, stderr, 0);
		    (void) fprintf(stderr, "): %s\n", strerror(en));
		}
		(void) CloseDir(dirp);
		dirp = (DIR *)NULL;
	 	errno = en;
	    }
	}
	return(dirp);
}


/*
 * ReadDir() - read next directory entry
 */

extern struct DIRTYPE *
ReadDir(dirp)
	register DIR *dirp;
{
	register struct DIRTYPE *dp;

/*
 * Loop through the directory.
 */
	for (;;) {
	    if (dirp->dd_loc >= dirp->dd_size) {
		dirp->dd_loc = 0;
	    }
	    if (dirp->dd_loc == 0) {
	    	dirp->dd_size = getdirentries(dirp->dd_fd,
			        dirp->dd_buf, dirp->dd_bufsiz, &dirp->dd_seek);

		if (dirp->dd_size <= 0)
		    return((struct DIRTYPE *)NULL);
	    /*
	     * If the size returned by getdirentries() exceeds what it
	     * should be (as determined by a stat(2) of the directory),
	     * set it to the proper value.  This is an adjustment for an
	     * apparent bug in the Digital UNIX 3.[02] getdirentries()
	     * function, when applied to a /dev/fd mount point.
	     *
	     * This check was conceived by Duncan McEwan and modified by
	     * Vic Abell.
	     */
		if (dirp->dd_size > (long)Dirsb.st_size)
		    dirp->dd_size = (long)Dirsb.st_size;
		Dirsb.st_size -= (off_t)dirp->dd_size;
	    }
	    dp = (struct DIRTYPE *)(dirp->dd_buf + dirp->dd_loc);
	    if ((long)dp & 03)		/* bogus pointer check */
		return((struct DIRTYPE *)NULL);
	    if (dp->d_reclen <= 0
	    ||  dp->d_reclen > dirp->dd_bufsiz + 1 - dirp->dd_loc)
		return((struct DIRTYPE *)NULL);
	    dirp->dd_loc += dp->d_reclen;
	    if (dp->d_ino == 0)
		continue;
	    return (dp);
	}
}
#endif	/* defined(USELOCALREADDIR) */


#if	defined(HASDCACHE)
/*
 * rereaddev() - reread device names, modes and types
 */

void
rereaddev()
{
	(void) clr_devtab();

# if	defined(DCACHE_CLR)
	(void) DCACHE_CLR();
# endif	/* defined(DCACHE_CLR) */

	readdev(1);
	DCunsafe = 0;
}
#endif	/* defined(HASDCACHE) */


/*
 * rmdupdev() - remove duplicate (major/minor/inode) devices
 */

static int
rmdupdev(dp, n, nm)
	struct l_dev ***dp;	/* device table pointers address */
	int n;			/* number of pointers */
	char *nm;		/* device table name for error message */
{
	struct clone *c, *cp;
	int i, j, k;
	struct l_dev **p;

	for (i = j = 0, p = *dp; i < n ;) {
	    for (k = i + 1; k < n; k++) {
		if (p[i]->rdev != p[k]->rdev || p[i]->inode != p[k]->inode)
		    break;
	    /*
	     * See if we're deleting a duplicate clone device.  If so,
	     * delete its clone table entry.
	     */
		for (c = Clone, cp = (struct clone *)NULL;
		     c;
		     cp = c, c = c->next)
		{
		    if (&Devtp[c->dx] != p[k])
			continue;
		    if (!cp)
			Clone = c->next;
		    else
			cp->next = c->next;
		    (void) free((FREE_P *)c);
		    break;
		}
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


#if	defined(HASDCACHE)
/*
 * vfy_dev() - verify a device table entry (usually when DCunsafe == 1)
 *
 * Note: rereads entire device table when an entry can't be verified.
 */

int
vfy_dev(dp)
	struct l_dev *dp;		/* device table pointer */
{
	struct stat sb;

	if (!DCunsafe || dp->v)
	    return(1);

#if	defined(USE_STAT)
	if (stat(dp->name, &sb) != 0
#else	/* !defined(USE_STAT) */
	if (lstat(dp->name, &sb) != 0
#endif	/* defined(USE_STAT) */

	||  dp->rdev != sb.st_rdev
	||  dp->inode != (INODETYPE)sb.st_ino) {
	   (void) rereaddev();
	   return(0);
	}
	dp->v = 1;
	return(1);
}
#endif	/* defined(HASDCACHE) */
