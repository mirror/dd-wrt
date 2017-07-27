/*
 * ddev.c - Solaris device support functions for lsof
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
static char *rcsid = "$Id: ddev.c,v 1.20 2005/08/08 19:55:41 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#define	LIKE_BLK_SPEC	"like block special"
#define	LIKE_CHR_SPEC	"like character special"


/*
 * Local static values
 */

static int Devx = 0;			/* current Devtp[] index */


/*
 * Local function prototypes
 */

_PROTOTYPE(static void make_devtp,(struct stat *s, char *p));
_PROTOTYPE(static int rmdupdev,(struct l_dev ***dp, int n, int ty));


/*
 * make_devtp() - make Devtp[] entry
 */

static void
make_devtp(s, p)
	struct stat *s;			/* device lstat() buffer */
	char *p;			/* device path name */
{

/*
 * Make room for another Devtp[] entry.
 */
	if (Devx >= Ndev) {
	    Ndev += DEVINCR;
	    if (!Devtp)
		Devtp = (struct l_dev *)malloc(
			(MALLOC_S)(sizeof(struct l_dev) * Ndev));
	    else
		Devtp = (struct l_dev *)realloc((MALLOC_P *)Devtp,
			(MALLOC_S)(sizeof(struct l_dev) * Ndev));
	    if (!Devtp) {
		(void) fprintf(stderr, "%s: no space for character device\n",
		    Pn);
		Exit(1);
	    }
	}
/*
 * Store the device number, inode number, and name in the Devtp[] entry.
 */
	Devtp[Devx].inode = (INODETYPE)s->st_ino;
	if (!(Devtp[Devx].name = mkstrcpy(p, (MALLOC_S *)NULL))) {
	    (void) fprintf(stderr, "%s: no space for /dev/", Pn);
	    safestrprt(p, stderr, 1);
	    Exit(1);
	}
	Devtp[Devx].rdev = s->st_rdev;
	Devtp[Devx].v = 0;
	Devx++;
}


/*
 * printdevname() - print block or character device name
 */

int
printdevname(dev, rdev, f, nty)
	dev_t *dev;			/* device */
	dev_t *rdev;			/* raw device */
	int f;				/* 1 = print trailing '\n' */
	int nty;			/* node type: N_BLK or N_CHR */
{
	struct clone *c;
	struct l_dev *dp;
	struct pseudo *p;

	readdev(0);
/*
 * Search device table for a full match.
 */

#if	defined(HASDCACHE)

printchdevname_again:

#endif	/* defined(HASDCACHE) */

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
	 * A match was found.  Record it as a name column addition.
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
	    (void) free((MALLOC_P *)cp);
	    return(0);
	}
/*
 * Search for clone parent.
 */
	if ((nty == N_CHR) && Lf->is_stream && Clone && (*dev == DevDev)) {
	    for (c = Clone; c; c = c->next) {
		if (GET_MAJ_DEV(*rdev) == GET_MIN_DEV(c->cd.rdev)) {

#if	defined(HASDCACHE)
		    if (DCunsafe && !c->cd.v && !vfy_dev(&c->cd))
			goto printchdevname_again;
#endif	/* defined(HASDCACHE) */

		    safestrprt(c->cd.name, stdout, f);
		    return(1);
		}	
	    }
	}
/*
 * Search for pseudo device match on major device only.
 */
	if ((nty == N_CHR) && *dev == DevDev) {
	    for (p = Pseudo; p; p = p->next) {
		if (GET_MAJ_DEV(*rdev) == GET_MAJ_DEV(p->pd.rdev)) {

# if	defined(HASDCACHE)
		    if (DCunsafe && !p->pd.v && vfy_dev(&p->pd))
			goto printchdevname_again;
# endif	/* defined(HASDCACHE) */

		    safestrprt(p->pd.name, stdout, f);
		    return(1);
		}
	    }
	}

#if	defined(HASDCACHE)
/*
 * If the device cache is "unsafe" and we haven't found any match, reload
 * the device cache.
 */
	if (DCunsafe) {
	    (void) rereaddev();
	    goto printchdevname_again;
	}
#endif	/* defined(HASDCACHE) */

	return(0);
}


/*
 * read_clone() - read Solaris clone device information
 */

void
read_clone()
{
	struct clone *c;
	char *cn;
	DIR *dfp;
	struct DIRTYPE *dp;
	char *fp = (char *)NULL;
	MALLOC_S fpl;
	char *path;
	MALLOC_S pl;
	struct pseudo *p;
	struct stat sb;
	
	if (Clone || Pseudo)
	    return;
/*
 * Open the /DVCH_DEVPATH/pseudo directory.
 */
	if (!(path = mkstrcat(DVCH_DEVPATH, -1, "/", 1, "pseudo ", -1, &pl))) {
	    (void) fprintf(stderr, "%s: no space for %s/pseudo\n",
		DVCH_DEVPATH, Pn);
	    Exit(1);
	}
	path[pl - 1] = '\0';
	if (!(dfp = OpenDir(path))) {

#if	defined(WARNDEVACCESS)
	    if (!Fwarn) {
		(void) fprintf(stderr, "%s: WARNING: can't open: ", Pn);
		safestrprt(path, stderr, 1);
	    }
#endif	/* defined(WARNDEVACCESS) */

	    (void) free((FREE_P *)path);
	    return;
	}
	path[pl - 1] = '/';
/*
 * Scan the directory.
 */
	for (dp = ReadDir(dfp); dp; dp = ReadDir(dfp)) {
	    if (dp->d_ino == 0 || dp->d_name[0] == '.')
		continue;
	/*
	 * Form the full path name and stat() it.
	 */
	    if (fp) {
		(void) free((FREE_P *)fp);
		fp = (char *)NULL;
	    }
	    if (!(fp = mkstrcat(path, pl, dp->d_name, -1, (char *)NULL, -1,
		       &fpl)))
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
		if (!Fwarn) {
		    int errno_save = errno;

		    (void) fprintf(stderr, "%s: can't stat: ", Pn);
		    safestrprt(fp, stderr, 0);
		    (void) fprintf(stderr, ": %s\n", strerror(errno_save));
		}
		continue;
	    }
	/*
	 * Skip subdirectories and all but character devices.
	 */
	    if ((sb.st_mode & S_IFMT) == S_IFDIR
	    ||  (sb.st_mode & S_IFMT) != S_IFCHR)
		continue;
	/*
	 * Make Devtp[] entry.
	 */
	    make_devtp(&sb, fp);
	 /*
	  * Create a clone structure entry for "clone*:" devices.
	  *
	  * Make special note of network clones -- tcp, and udp.
	  */
	    if (strncmp(&fp[pl], "clone", 5) == 0) {
		if (!(cn = strrchr(&fp[pl], ':')))
		    continue;
	    /*
	     * Allocate a clone structure.
	     */
		if (!(c = (struct clone *)malloc(sizeof(struct clone)))) {
		    (void) fprintf(stderr,
			"%s: no space for network clone device: ", Pn);
		    safestrprt(fp, stderr, 1);
		    Exit(1);
		}
	    /*
	     * Allocate space for the path name.
	     */
		if (!(c->cd.name = mkstrcpy(fp, (MALLOC_S *)NULL))) {
		    (void) fprintf(stderr, "%s: no space for clone name: ", Pn);
		    safestrprt(fp, stderr, 1);
		    Exit(1);
		}
	    /*
	     * Save the inode and device numbers.  Clear the verify flag.
	     */
		c->cd.inode = (INODETYPE)sb.st_ino;
		c->cd.rdev = sb.st_rdev;
		c->cd.v = 0;
	    /*
	     * Make special note of a network clone device.
	     */
		if (!strcmp(++cn, "tcp") || !strcmp(cn, "udp"))
		    c->n = cn - fp;
		else
		    c->n = 0;
	    /*
	     * Link the new clone entry to the rest.
	     */
		c->next = Clone;
		Clone = c;
		continue;
	    }
	/*
	 * Save pseudo device information.
	 */
	    if (GET_MIN_DEV(sb.st_rdev) == 0) {

	    /*
	     * Allocate space for the pseduo device entry.
	     */
		if (!(p = (struct pseudo *) malloc(sizeof(struct pseudo)))) {
		    (void) fprintf(stderr,
			"%s: no space for pseudo device: ", Pn);
		    safestrprt(fp, stderr, 1);
		    Exit(1);
		}
	    /*
	     * Save the path name, and inode and device numbers.  Clear the
	     * verify flag.  Link the entry to the pseudo chain.
	     */
		p->pd.inode = (INODETYPE)sb.st_ino;
		p->pd.name = fp;
		fp = (char *)NULL;
		p->pd.rdev = sb.st_rdev;
		p->pd.v = 0;
		p->next = Pseudo;
		Pseudo = p;
	    }
	}
	(void) CloseDir(dfp);
	if (fp)
	    (void) free((FREE_P *)fp);
	if (path)
	    (void) free((FREE_P *)path);
}


/*
 * readdev() - read names, modes and device types of everything in /dev
 *	       or /device (Solaris)
 */

void
readdev(skip)
	int skip;			/* skip device cache read if 1 */
{

#if	defined(HASDCACHE)
	int dcrd = 0;
#endif	/* defined(HASDCACHE) */

	DIR *dfp;
	struct DIRTYPE *dp;
	char *fp = (char *)NULL;
	MALLOC_S fpl;
	int i;

#if	defined(HASBLKDEV)
	int j = 0;
#endif	/* defined(HASBLKDEV) */

	char *path = (char *)NULL;
	char *ppath = (char *)NULL;
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

	if (!(ppath = mkstrcat(DVCH_DEVPATH, -1, "/", 1, "pseudo", -1,
		      (MALLOC_S *)NULL)))
	{
	    (void) fprintf(stderr, "%s: no space for: %s/pseudo\n",
		Pn, DVCH_DEVPATH);
	    Exit(1);
	}
	read_clone();
	Dstk = (char **)NULL;
	Dstkn = Dstkx = 0;
	(void) stkdir(DVCH_DEVPATH);
/*
 * Unstack the next directory.
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
	/*
	 * Create a directory name buffer with a trailing slash.
	 */
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
		if (!(fp = mkstrcat(path, pl, dp->d_name, -1, (char *)NULL, -1,
			   &fpl)))
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

		/*
		 * Skip Solaris /DVCH_DEV_PATH/pseudo sub-directory;
		 * it has been examined in read_clone().
		 */
		    if (strcmp(fp, ppath) == 0)
			continue;
		    (void) stkdir(fp);
			continue;
		}
		if ((sb.st_mode & S_IFMT) == S_IFCHR) {

		/*
		 * Make Devtp[] entry.
		 */
		    make_devtp(&sb, fp);
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
			    BDevtp = (struct l_dev *)realloc((MALLOC_P *)BDevtp,
				     (MALLOC_S)(sizeof(struct l_dev)*BNdev));
			if (!BDevtp) {
			    (void) fprintf(stderr,
				"%s: no space for block device\n", Pn);
			    Exit(1);
			}
		    }
		    BDevtp[j].rdev = sb.st_rdev;
		    BDevtp[j].inode = (INODETYPE)sb.st_ino;
		    BDevtp[j].name = fp;
		    fp = (char *)NULL;
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
	if (ppath)
	    (void) free((FREE_P *)ppath);
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
	    BNdev = rmdupdev(&BSdev, BNdev, 0);
	} else {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: no block devices found\n", Pn);
	}
#endif	/* defined(HASBLKDEV) */

	if (Ndev) {
	    if (Ndev > Devx) {
		Ndev = Devx;
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
	    Ndev = rmdupdev(&Sdev, Ndev, 1);
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


/*
 * clr_sect() - clear cached clone and pseudo sections
 */

void
clr_sect()
{
	if (Clone) {
	    struct clone *c, *c1;

	    for (c = Clone; c; c = c1) {
		c1 = c->next;
		if (c->cd.name)
		    (void) free((FREE_P *)c->cd.name);
		(void) free((FREE_P *)c);
	    }
	    Clone = (struct clone *)NULL;
	}
	if (Pseudo) {
	    struct pseudo *p, *p1;

	    for (p = Pseudo; p; p = p1) {
		p1 = p->next;
		if (p->pd.name)
		    (void) free((FREE_P *)p->pd.name);
		(void) free((FREE_P *)p);
	    }
	    Pseudo = (struct pseudo *)NULL;
	}
}


#if	defined(HASDCACHE)
/*
 * rw_clone_sect() - read/write the device cache file clone section
 */

int
rw_clone_sect(m)
	int m;				/* mode: 1 = read; 2 = write */
{
	char buf[MAXPATHLEN*2], *cp;
	struct clone *c;
	int i, len, n;

	if (m == 1) {

	/*
	 * Read the clone section header and validate it.
	 */
	    if (!fgets(buf, sizeof(buf), DCfs)) {

bad_clone_sect:

		if (!Fwarn) {
		    (void) fprintf(stderr,
			"%s: bad clone section header in %s: ",
			Pn, DCpath[DCpathX]);
		    safestrprt(buf, stderr, 1);
		}
		return(1);
	    }
	    (void) crc(buf, strlen(buf), &DCcksum);
	    len = strlen("clone section: ");
	    if (strncmp(buf, "clone section: ", len) != 0)
		goto bad_clone_sect;
	    if ((n = atoi(&buf[len])) < 0)
		goto bad_clone_sect;
	/*
	 * Read the clone section lines and create the Clone list.
	 */
	    for (i = 0; i < n; i++) {
		if (!fgets(buf, sizeof(buf), DCfs)) {
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: bad clone line in %s: ", Pn, DCpath[DCpathX]);
			safestrprt(buf, stderr, 1);
		    }
		    return(1);
		}
		(void) crc(buf, strlen(buf), &DCcksum);
	    /*
	     * Allocate a clone structure.
	     */
		if (!(c = (struct clone *)calloc(1, sizeof(struct clone)))) {
		    (void) fprintf(stderr,
			"%s: no space for cached clone: ", Pn);
		    safestrprt(buf, stderr, 1);
		    Exit(1);
		}
	    /*
	     * Enter the clone device number.
	     *
	     * New format clone lines (with an inode number) have a leading
	     * space, so that older lsof versions, not expecting them, will
	     * not use the new format lines.
	     */
		if (buf[0] != ' '
		||  !(cp = x2dev(&buf[1], &c->cd.rdev)) || *cp++ != ' ')
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: bad cached clone device: ", Pn);
			safestrprt(buf, stderr, 1);
		    }
		    return(1);
		}
	    /*
	     * Enter the clone network value.
	     */
		for (c->n = 0; *cp != ' '; cp++) {
		    if (*cp < '0' || *cp > '9') {
			if (!Fwarn) {
			    (void) fprintf(stderr,
				"%s: bad cached clone network flag: ", Pn);
			    safestrprt(buf, stderr, 1);
			}
			return(1);
		    }
		    c->n = (c->n * 10) + (int)(*cp - '0');
		}
	    /*
	     * Enter the clone device inode number.
	     */
		for (c->cd.inode = (INODETYPE)0, ++cp; *cp != ' '; cp++) {
		    if (*cp < '0' || *cp > '9') {
			if (!Fwarn) {
			    (void) fprintf(stderr,
				"%s: bad cached clone inode number: ", Pn);
			    safestrprt(buf, stderr, 1);
			}
			return(1);
		    }
		    c->cd.inode = (INODETYPE)((c->cd.inode * 10)
				+ (int)(*cp - '0'));
		}
	    /*
	     * Enter the clone path name.
	     */
		if ((len = strlen(++cp)) < 2 || *(cp + len - 1) != '\n') {
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: bad cached clone path: ", Pn);
			safestrprt(buf, stderr, 1);
		    }
		    return(1);
		}
		*(cp + len - 1) = '\0';
		if (!(c->cd.name = mkstrcpy(cp, (MALLOC_S *)NULL))) {
		    (void) fprintf(stderr,
			"%s: no space for cached clone path: ", Pn);
		    safestrprt(buf, stderr, 1);
		    Exit(1);
		}
		c->cd.v = 0;
		c->next = Clone;
		Clone = c;
	    }
	    return(0);
	} else if (m == 2) {

	/*
	 * Write the clone section header.
	 */
	    for (c = Clone, n = 0; c; c = c->next, n++)
		;
	    (void) snpf(buf, sizeof(buf), "clone section: %d\n", n);
	    if (wr2DCfd(buf, &DCcksum))
		return(1);
	/*
	 * Write the clone section lines.
	 *
	 *
	 * New format clone lines (with an inode number) have a leading
	 * space, so that older lsof versions, not expecting them, will
	 * not use the new format lines.
	 */
	    for (c = Clone; c; c = c->next) {
		(void) snpf(buf, sizeof(buf), " %lx %d %ld %s\n",
		    (long)c->cd.rdev, c->n, (long)c->cd.inode, c->cd.name);
		if (wr2DCfd(buf, &DCcksum))
		    return(1);
	    }
	    return(0);
	}
/*
 * A shouldn't-happen case: mode neither 1 nor 2.
 */
	(void) fprintf(stderr, "%s: internal rw_clone_sect error: %d\n",
	    Pn, m);
	Exit(1);
	return(1);		/* to make code analyzers happy */
}


/*
 * rereaddev() - reread device names, modes and types
 */

void
rereaddev()
{
	(void) clr_devtab();
	(void) clr_sect();
	Devx = 0;

# if	defined(DCACHE_CLR)
	(void) DCACHE_CLR();
# endif	/* defined(DCACHE_CLR) */

	readdev(1);
	DCunsafe = 0;
}


/*
 * rw_pseudo_sect() - read/write the device cache pseudo section
 */

int
rw_pseudo_sect(m)
	int m;				/* mode: 1 = read; 2 = write */
{
	char buf[MAXPATHLEN*2], *cp;
	struct pseudo *p;
	int i, len, n;

	if (m == 1) {

	/*
	 * Read the pseudo section header and validate it.
	 */
	    if (!fgets(buf, sizeof(buf), DCfs)) {

bad_pseudo_sect:

		if (!Fwarn) {
		    (void) fprintf(stderr,
			"%s: bad pseudo section header in %s: ",
			Pn, DCpath[DCpathX]);
		    safestrprt(buf, stderr, 1);
		}
		return(1);
	    }
	    (void) crc(buf, strlen(buf), &DCcksum);
	    len = strlen("pseudo section: ");
	    if (strncmp(buf, "pseudo section: ", len) != 0)
		goto bad_pseudo_sect;
	    if ((n = atoi(&buf[len])) < 0)
		goto bad_pseudo_sect;
	/*
	 * Read the pseudo section lines and create the Pseudo list.
	 */
	    for (i = 0; i < n; i++) {
		if (!fgets(buf, sizeof(buf), DCfs)) {
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: bad pseudo line in %s: ", Pn, DCpath[DCpathX]);
			safestrprt(buf, stderr, 1);
		    }
		    return(1);
		}
		(void) crc(buf, strlen(buf), &DCcksum);
	    /*
	     * Allocate a pseudo structure.
	     */
		if (!(p = (struct pseudo *)calloc(1, sizeof(struct pseudo)))) {
		    (void) fprintf(stderr,
			"%s: no space for cached pseudo: ", Pn);
		    safestrprt(buf, stderr, 1);
		    Exit(1);
		}
	    /*
	     * Enter the pseudo device number.
	     *
	     * New format pseudo lines (with an inode number) have a leading
	     * space, so that older lsof versions, not expecting them, will
	     * not use the new format lines.
	     */
		if (buf[0] != ' '
		||  !(cp = x2dev(&buf[1], &p->pd.rdev)) || *cp++ != ' ')
		{
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: bad cached pseudo device: ", Pn);
			safestrprt(buf, stderr, 1);
		    }
		    return(1);
		}
	    /*
	     * Enter the pseudo inode number.
	     */
		for (p->pd.inode = (INODETYPE)0; *cp != ' '; cp++) {
		    if (*cp < '0' || *cp > '9') {
			if (!Fwarn) {
			    (void) fprintf(stderr,
				"%s: bad cached pseudo inode number: ", Pn);
			    safestrprt(buf, stderr, 1);
			}
			return(1);
		    }
		    p->pd.inode = (INODETYPE)((p->pd.inode * 10)
				+ (int)(*cp - '0'));
		}
	    /*
	     * Enter the pseudo path name.
	     *
	     *
	     * New format clone lines (with an inode number) have a leading
	     * space, so that older lsof versions, not expecting them, will
	     * not use the new format lines.
	     */
		if ((len = strlen(++cp)) < 2 || *(cp + len - 1) != '\n') {
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "%s: bad cached pseudo path: ", Pn);
			safestrprt(buf, stderr, 1);
		    }
		    return(1);
		}
		if (!(p->pd.name = (char *)malloc(len))) {
		    (void) fprintf(stderr,
			"%s: no space for cached pseudo path: ", Pn);
		    safestrprt(buf, stderr, 1);
		    Exit(1);
		}
		*(cp + len - 1) = '\0';
		(void) snpf(p->pd.name, len, "%s", cp);
		p->pd.v = 0;
		p->next = Pseudo;
		Pseudo = p;
	    }
	    return(0);
	} else if (m == 2) {

	/*
	 * Write the pseudo section header.
	 */
	    for (p = Pseudo, n = 0; p; p = p->next, n++)
		;
	    (void) snpf(buf, sizeof(buf), "pseudo section: %d\n", n);
	    if (wr2DCfd(buf, &DCcksum))
		return(1);
	/*
	 * Write the pseudo section lines.
	 *
	 *
	 * New format pseudo lines (with an inode number) have a leading
	 * space, so that older lsof versions, not expecting them, will
	 * not use the new format lines.
	 */
	    for (p = Pseudo; p; p = p->next) {
		(void) snpf(buf, sizeof(buf), " %lx %ld %s\n", (long)p->pd.rdev,
		    (long)p->pd.inode, p->pd.name);
		if (wr2DCfd(buf, &DCcksum))
		    return(1);
	    }
	    return(0);
	}
/*
 * A shouldn't-happen case: mode neither 1 nor 2.
 */
	(void) fprintf(stderr, "%s: internal rw_pseudo_sect error: %d\n",
	    Pn, m);
	return(1);
}


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


/*
 * rmdupdev() - remove duplicate (major/minor/inode) devices
 */

static int
rmdupdev(dp, n, ty)
	struct l_dev ***dp;	/* device table pointers address */
	int n;			/* number of pointers */
	int ty;			/* type: 0 = block, 1 = char */
{
	struct clone *c, *cp;
	struct l_dev **d;
	int i, j, k;
	struct pseudo *p, *pp;

	for (i = j = 0, d = *dp; i < n ;) {
	    for (k = i + 1; k < n; k++) {
		if (d[i]->rdev != d[k]->rdev || d[i]->inode != d[k]->inode)
		    break;
		if (ty == 0)
		    continue;
	    /*
	     * See if we're deleting a duplicate clone device.  If so,
	     * delete its clone table entry.
	     */
		for (c = Clone, cp = (struct clone *)NULL;
		     c;
		     cp = c, c = c->next)
		{
		    if (c->cd.rdev != d[k]->rdev
		    ||  c->cd.inode != d[k]->inode
		    ||  strcmp(c->cd.name, d[k]->name))
			continue;
		    if (!cp)
			Clone = c->next;
		    else
			cp->next = c->next;
		    if (c->cd.name)
			(void) free((FREE_P *)c->cd.name);
		    (void) free((FREE_P *)c);
		    break;
		}
	    /*
	     * See if we're deleting a duplicate pseudo device.  If so,
	     * delete its pseudo table entry.
	     */
		for (p = Pseudo, pp = (struct pseudo *)NULL;
		     p;
		     pp = p, p = p->next)
		{
		    if (p->pd.rdev != d[k]->rdev
		    ||  p->pd.inode != d[k]->inode
		    ||  strcmp(p->pd.name, d[k]->name))
			continue;
		    if (!pp)
			Pseudo = p->next;
		    else
			pp->next = p->next;
		    if (p->pd.name)
			(void) free((FREE_P *)p->pd.name);
		    (void) free((FREE_P *)p);
		    break;
		}
	    }
	    if (i != j)
		d[j] = d[i];
	    j++;
	    i = k;
	}
	if (n == j)
	    return(n);
	if (!(*dp = (struct l_dev **)realloc((MALLOC_P *)*dp,
		    (MALLOC_S)(j * sizeof(struct l_dev *)))))
	{
	    (void) fprintf(stderr, "%s: can't realloc %s device pointers\n",
		Pn, ty ? "char" : "block");
	    Exit(1);
	}
	return(j);
}
