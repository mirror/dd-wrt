/*
 * dmnt.c - Solaris mount support functions for lsof
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
static char *rcsid = "$Id: dmnt.c,v 1.15 2005/08/29 10:24:25 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local static definitions
 */

static struct mounts *Lmi = (struct mounts *)NULL;	/* local mount info */
static int Lmist = 0;					/* Lmi status */


_PROTOTYPE(static char *getmntdev,(char *o, int l, struct stat *s, char *f));


/*
 * getmntdev() - get mount entry's device number
 */

static char *
getmntdev(o, l, s, f)
	char *o;			/* start of device option */
	int l;				/* length of device keyword (not
					 * including `=') */
	struct stat *s;			/* pointer to stat buffer to create */
	char *f;			/* file system type */
{
	char *opte;

	memset((char *)s, 0, sizeof(struct stat));
	if (!(opte = x2dev(o + l + 1, &s->st_dev)))
	    return((char *)NULL);

#if	solaris>=70000 && L_BITSMAJOR!=L_BITSMAJOR32
/*
 * If this is a Solaris 7 system with a 64 bit kernel, convert the 32 bit
 * device number to a 64 bit device number.
 */
	s->st_dev = (((s->st_dev >> L_BITSMINOR32) & L_MAXMAJ32) << L_BITSMINOR)
		  | (s->st_dev & L_MAXMIN32);
#endif	/* solaris>=70000 && L_BITSMAJOR!=L_BITSMAJOR32 */

	s->st_mode = S_IFDIR | 0777;

#if	defined(HASFSTYPE)
	if (f) {
	    (void) strncpy(s->st_fstype, f, sizeof(s->st_fstype));
	    s->st_fstype[sizeof(s->st_fstype) - 1] = '\0';
	}
#endif	/* defined(HASFSTYPE) */

	return(opte);
}


/*
 * readmnt() - read mount table
 */

struct mounts *
readmnt()
{
	int devl, ignore;
	char *cp, *dir, *fs;
	char *dn = (char *)NULL;
	char *ln;
	FILE *mfp;
	struct mounts *mtp;
	char *dopt, *dopte;
	struct stat sb;
	struct mnttab me;
	struct mnttab *mp;

#if	defined(HASPROCFS)
	int procfs = 0;
#endif	/* defined(HASPROCFS) */

	unsigned char stat;
	char *zopt;

#if 	defined(HASZONES)
	int zwarn = 0;
#endif 	/* definesd(HASZONES) */

	if (Lmi || Lmist)
	    return(Lmi);
	devl = strlen(MNTOPT_DEV);
/*
 * Open access to the mount table and read mount table entries.
 */
	if (!(mfp = fopen(MNTTAB, "r"))) {
            (void) fprintf(stderr, "%s: can't access %s\n", Pn, MNTTAB);
            return(0);
        }
	for (mp = &me; getmntent(mfp, mp) == 0;) {

	/*
	 * Skip loop-back mounts, since they are aliases for legitimate file
	 * systems and there is no way to determine that a vnode refers to a
	 * loop-back alias.
	 */
	    if (strcmp(mp->mnt_fstype, MNTTYPE_LO) == 0)
		continue;
	/*
	 * Save pointers to the directory and file system names for later use.
	 *
	 * Check the file system name.  If it doesn't begin with a `/'
	 * but contains a `:' not followed by a '/', ignore this entry.
	 */
	    dir = mp->mnt_mountp;
	    fs = mp->mnt_special;
	    if (*fs != '/' && (cp = strchr(fs, ':')) && *(cp+1) != '/')
		continue;
	/*
	 * Check for a "ignore" type (SunOS) or "ignore" option (Solaris).
	 */
	    if (hasmntopt(mp, MNTOPT_IGNORE))
		ignore = 1;
	    else
		ignore = 0;
	/*
	 * Interpolate a possible symbolic directory link.
	 */
	    if (dn)
		(void) free((FREE_P *)dn);
	    if (!(dn = mkstrcpy(dir, (MALLOC_S *)NULL))) {

no_space_for_mount:

		(void) fprintf(stderr, "%s: no space for mount ", Pn);
		safestrprt(fs, stderr, 0);
		(void) fprintf(stderr, " (");
		safestrprt(dir, stderr, 0);
		(void) fprintf(stderr, ")\n");
		Exit(1);
	    }
	    if (!(ln = Readlink(dn))) {
		if (!Fwarn) {
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
		continue;
	    }
	    if (ln != dn) {
		(void) free((FREE_P *)dn);
		dn = ln;
	    }
	    if (*dn != '/')
		continue;
	/*
	 * Stat() the directory.
	 *
	 * Avoid the stat() if the mount entry has an "ignore" option and
	 * try to use the mount entry's device number instead.
	 */
	    dopt = hasmntopt(mp, MNTOPT_DEV);
	    if (ignore) {
		if (!dopt
		||  !(dopte = getmntdev(dopt, devl, &sb,

#if	defined(HASFSTYPE)
				       mp->mnt_fstype
#else	/* !defined(HASFSTYPE) */
				       (char *)NULL
#endif	/* defined(HASFSTYPE) */

				      ))
		)
		    continue;
		stat = 1;
	    } else if (statsafely(dn, &sb)) {
		if (dopt) {
		    if (!(dopte = getmntdev(dopt, devl, &sb,

#if	defined(HASFSTYPE)
					   mp->mnt_fstype
#else	/* !defined(HASFSTYPE) */
					   (char *)NULL
#endif	/* defined(HASFSTYPE) */

					  ))
		    )
			dopt = (char *)NULL;
		} else
		    dopte = (char *)NULL;
		if (!Fwarn) {

#if	defined(HASZONES)
		    if ((zopt = hasmntopt(mp, "zone")) && dopte)
			zwarn++;
#else	/* !defined(HASZONES) */
		    zopt = (char *)NULL;
#endif	/* defined(HASZONES) */

		    if (!zopt || !dopte) {
			(void) fprintf(stderr,
			    "%s: WARNING: can't stat() ", Pn);
			safestrprt(mp->mnt_fstype, stderr, 0);
			(void) fprintf(stderr, " file system ");
			safestrprt(dir, stderr, 1);
			(void) fprintf(stderr,
			    "      Output information may be incomplete.\n");
			if (dopte) {
			    (void) fprintf(stderr,
				"      assuming \"%.*s\" from %s\n",
				(int)(dopte - dopt), dopt, MNTTAB);
			}
		   }
		}
		if (!dopt)
		    continue;
		stat = 1;
	    } else
		stat = 0;
	/*
	 * Allocate and fill a local mount structure.
	 */
	    if (!(mtp = (struct mounts *)malloc(sizeof(struct mounts))))
		goto no_space_for_mount;

#if	defined(HASFSTYPE)
	    if (!(mtp->fstype = mkstrcpy(sb.st_fstype, (MALLOC_S *)NULL)))
		goto no_space_for_mount;
#endif	/* defined(HASFSTYPE) */

	    mtp->dir = dn;
	    dn = (char *)NULL;
	    mtp->next = Lmi;
	    mtp->dev = sb.st_dev;
	    mtp->rdev = sb.st_rdev;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;

#if	solaris>=80000
	    mtp->nlink = sb.st_nlink;
	    mtp->size = sb.st_size;
#endif	/* solaris>=80000 */

#if	defined(HASMNTSTAT)
	    mtp->stat = stat;
#endif	/* defined(HASMNTSTAT) */

#if	defined(HASPROCFS)
	    if (strcmp(sb.st_fstype, HASPROCFS) == 0) {

	    /*
	     * Save information on exactly one proc file system.
	     */
		if (procfs)
		    Mtprocfs = (struct mounts *)NULL;
		else {
		    procfs = 1;
		    Mtprocfs = mtp;
		}
	    }
#endif	/* defined(HASPROCFS) */

	/*
	 * Interpolate a possible file system (mounted-on) device name link.
	 */
	    if (!(dn = mkstrcpy(fs, (MALLOC_S *)NULL)))
		goto no_space_for_mount;
	    mtp->fsname = dn;
	    ln = Readlink(dn);
	    dn = (char *)NULL;
	/*
	 * Stat() the file system (mounted-on) name and add file system
	 * information to the local mount table entry.
	 */
	    if (!ln || statsafely(ln, &sb))
		sb.st_mode = 0;
	    mtp->fsnmres = ln;
	    mtp->fs_mode = sb.st_mode;
	    Lmi = mtp;

#if	defined(HAS_AFS)
	/*
	 * If an AFS device number hasn't yet been defined, look for it.
	 */
	    if (!AFSdevStat
	    &&  mtp->dir && strcmp(mtp->dir, "/afs") == 0
	    &&  mtp->fsname && strcmp(mtp->fsname, "AFS") == 0) {
		AFSdev = mtp->dev;
		AFSdevStat = 1;
	    }
#endif	/* defined(HAS_AFS) && solaris>=20600 */

        }
	(void) fclose(mfp);

#if	defined(HASZONES)
/*
 * If some zone file systems were encountered, issue a warning.
 */
	if (!Fwarn && zwarn) {
	    (void) fprintf(stderr,
		"%s: WARNING: can't stat() %d zone file system%s", Pn, zwarn,
		    (zwarn == 1) ? "" : "s");
	    (void) fprintf(stderr, "; using dev= option%s\n",
		    (zwarn == 1) ? "" : "s");
	}
#endif	/* defined(HASZONES) */

/*
 * Clean up and return local mount info table address.
 */
	if (dn)
	    (void) free((FREE_P *)dn);
	Lmist = 1;
	return(Lmi);
}


/*
 * readvfs() - read vfs structure
 */

struct l_vfs *
readvfs(ka, la, lv)
	KA_T ka;			/* vfs structure kernel address, if
					 * must be read from kernel */
	struct vfs *la;			/* local vfs structure address, non-
					 * NULL if already read from kernel */
	struct vnode *lv;		/* local vnode */
{
	struct vfs *v, tv;
	struct l_vfs *vp;

	if (!ka && !la)
	    return((struct l_vfs *)NULL);
	for (vp = Lvfs; vp; vp = vp->next) {
	    if (ka == vp->addr)
		return(vp);
	}
	if (!(vp = (struct l_vfs *)malloc(sizeof(struct l_vfs)))) {
	    (void) fprintf(stderr, "%s: PID %d, no space for vfs\n",
		Pn, Lp->pid);
	    Exit(1);
	}
	vp->dir = (char *)NULL;
	vp->fsname = (char *)NULL;

#if	defined(HASFSINO)
	vp->fs_ino = 0;
#endif	/* defined(HASFSINO) */

/*
 * Read vfs structure from kernel, if necessary.
 */
	if (la)
	    v = la;
	else {
	    v = &tv;
	    if (kread((KA_T)ka, (char *)v, sizeof(tv))) {
		(void) free((FREE_P *)vp);
		return((struct l_vfs *)NULL);
	    }
	}

#if	defined(HAS_AFS)
/*
 * Fake the device number for an AFS device.
 */
	if (v->vfs_fstype == AFSfstype) {
	    if (!AFSdevStat)
		(void) readmnt();
	    v->vfs_dev = AFSdevStat ? AFSdev : 0;
	}
#endif	/* defined(HAS_AFS) */

/*
 * Complete mount information.
 */

	(void) completevfs(vp, (dev_t *)&v->vfs_dev);
	vp->next = Lvfs;
	vp->addr = ka;
	Lvfs = vp;
	return(vp);
}
