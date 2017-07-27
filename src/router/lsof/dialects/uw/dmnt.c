/*
 * dmnt.c - SCO UnixWare mount support functions for lsof
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

#ifndef lint
static char copyright[] =
"@(#) Copyright 1996 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dmnt.c,v 1.7 2005/08/13 16:21:41 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local static definitions
 */

static struct mounts *Lmi = (struct mounts *)NULL;	/* local mount info */
static int Lmist = 0;					/* Lmi status */


/*
 * readmnt() - read mount table
 */

struct mounts *
readmnt()
{
	char *dn = (char *)NULL;
	char *ln;
	struct mnttab me;
	FILE *mfp;
	struct mounts *mtp;
	char *opt, *opte;
	struct stat sb;

#if	defined(HASPROCFS)
	int procfs = 0;
#endif

	if (Lmi || Lmist)
	    return(Lmi);
/*
 * Open access to the mount table and read mount table entries.
 */
	if (!(mfp = fopen(MNTTAB, "r"))) {
	    (void) fprintf(stderr, "%s: can't access %s\n", Pn, MNTTAB);
	    return(0);
        }
	while (!getmntent(mfp, &me)) {

	/*
	 * Skip loop-back mounts, since they are aliases for legitimate file
	 * systems and there is no way to determine that a vnode refers to a
	 * loop-back alias.
	 *
	 * Also skip file systems of type MNTTYPE_IGNORE or with the option
	 * MNTOPT_IGNORE, since they are auto-mounted file systems whose
	 * real entries (if they are mounted) will be separately identified
	 * by getmntent().
	 */
	    if (!strcmp(me.mnt_fstype, MNTTYPE_LO)
	    ||  !strcmp(me.mnt_fstype, MNTTYPE_IGNORE))
		continue;
	/*
	 * Interpolate a possible symbolic directory link.
	 */
	    if (dn)
		(void) free((FREE_P *)dn);
	    if (!(dn = mkstrcpy(me.mnt_mountp, (MALLOC_S *)NULL))) {

no_space_for_mount:

		(void) fprintf(stderr, "%s: no space for mount at ",Pn);
		safestrprt(me.mnt_special, stderr, 0);
		(void) fprintf(stderr, " (");
		safestrprt(me.mnt_mountp, stderr, 0);
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
	 */
	    if (statsafely(dn, &sb)) {
		if (!Fwarn) {
		    (void) fprintf(stderr, "%s: can't stat()", Pn);

#if	defined(HASFSTYPE)
		    putc(' ', stderr);
		    safestrprt(me.mnt_fstype, stderr, 0);
#endif	/* defined(HASFSTYPE) */

		    (void) fprintf(stderr, " file system ");
		    safestrprt(me.mnt_mountp, stderr, 1);
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
		if (!(opt = strstr(me.mnt_mntopts, "dev="))) {
		    (void) memset(&sb, 0, sizeof(sb));
		    if (!(opte = x2dev(opt + 4, &sb.st_dev))) {
			sb.st_mode = S_IFDIR | 0777;

#if	defined(HASFSTYPE)
			(void) strncpy(sb.st_fstype, me.mnt_fstype,
				       sizeof(sb.st_fstype));
			sb.st_fstype[sizeof(sb.st_fstype) - 1 ] = '\0';
#endif	/* HASFSTYPE */

			if (!Fwarn) {
			    (void) fprintf(stderr,
				"      assuming \"%.*s\" from %s\n",
				(opte - opt), opt, MNTTAB);
			}
		    } else
			opt = (char *)NULL;
		}
		if (!opt)
		    continue;
	    }
	/*
	 * Allocate and fill a local mount structure.
	 */
	    if (!(mtp = (struct mounts *)malloc(sizeof(struct mounts))))
		goto no_space_for_mount;

#if	defined(HASFSTYPE)
	    if (!(mtp->fstype = mkstrcpy(sb.st_fstype, (MALLOC_S *)NULL)))
		goto no_space_for_mount;
#endif	/* HASFSTYPE */

	    mtp->dir = dn;
	    dn = (char *)NULL;
	    mtp->next = Lmi;
	    mtp->dev = sb.st_dev;
	    mtp->rdev = sb.st_rdev;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;

#if	defined(HASPROCFS)
# if	defined(HASFSTYPE)
	    if (!strcmp(sb.st_fstype, HASPROCFS))
# else	/* !defined*HASFSTYPE) */
	    if (!strcmp(me.mnt_special, "/proc"))
# endif	/* defined(HASFSTYPE) */

	    {

	    /*
	     * Save information on exactly one procfs file system.
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
	 * Interpolate a possible file system (mounted-on device) name link.
	 */
	    if (!(dn = mkstrcpy(me.mnt_special, (MALLOC_S *)NULL)))
		goto no_space_for_mount;
	    mtp->fsname = dn;
	    ln = Readlink(dn);
	    dn = (char *)NULL;
	/*
	 * Stat() the file system (mounted-on) name and add file system
	 * information to the local mounts structure.
	 */
	    if (!ln || statsafely(ln, &sb))
		sb.st_mode = 0;
	    mtp->fsnmres = ln;
	    mtp->fs_mode = sb.st_mode;
	    Lmi = mtp;
        }
	(void) fclose(mfp);
/*
 * Clean up and return local mount info table address.
 */
	if (dn)
	    (void) free((FREE_P *)dn);
	Lmist = 1;
	return(Lmi);
}
