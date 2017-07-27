/*
 * dmnt.c -- Darwin mount support functions for libproc-based lsof
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
"@(#) Copyright 2005  Apple Computer, Inc. and Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dmnt.c,v 1.6 2012/04/10 16:41:04 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local static information
 */

static struct mounts *Lmi = (struct mounts *)NULL;	/* local mount info */
static int Lmist = 0;					/* Lmi status */

/*
 * readmnt() -- read mount table
 */

struct mounts *
readmnt()
{
#if	defined(DIR_MNTSTATUS_TRIGGER)
	struct {
		uint32_t	length;
		uint32_t	mount_flags;
	} ab;
	struct attrlist al;
#endif	/* defined(DIR_MNTSTATUS_TRIGGER) */
	char *dn = (char *)NULL;
	char *ln;
	struct statfs *mb = (struct statfs *)NULL;
	struct mounts *mtp;
	int n;
	struct stat sb;

	if (Lmi || Lmist)
	    return(Lmi);
/*
 * Access mount information.
 */
	if ((n = getmntinfo(&mb, MNT_NOWAIT)) <= 0) {
	    (void) fprintf(stderr, "%s: no mount information\n", Pn);
	    return(0);
	}
/*
 * Read mount information.
 */
	for (; n; n--, mb++) {

	    if (!mb->f_type)
		continue;
	/*
	 * Avoid file systems that are not appropriate paths to
	 * user data (e.g. automount maps, triggers).
	 */
#if	defined(DIR_MNTSTATUS_TRIGGER)
	    (void) bzero((char *)&al, sizeof(al));
	    al.bitmapcount = ATTR_BIT_MAP_COUNT;
	    al.dirattr     = ATTR_DIR_MOUNTSTATUS;
	    if (getattrlist(mb->f_mntonname, &al, &ab, sizeof(ab), 0) == 0) {
		if (ab.mount_flags & DIR_MNTSTATUS_TRIGGER)
			continue;	// if mount trigger
	    }
#else	/* !defined(DIR_MNTSTATUS_TRIGGER) */
	    if (mb->f_flags & MNT_AUTOMOUNTED) {
		if (!strncmp(mb->f_mntfromname, "map ", 4)
		||  !strcmp(mb->f_mntfromname, "trigger"))
		    continue;
	    }
#endif	/* defined(DIR_MNTSTATUS_TRIGGER) */

	/*
	 * Interpolate a possible symbolic directory link.
	 */
	    if (dn)
		(void) free((FREE_P *)dn);
	    if (!(dn = mkstrcpy(mb->f_mntonname, (MALLOC_S *)NULL))) {

no_space_for_mount:

		(void) fprintf(stderr, "%s: no space for mount at ", Pn);
		safestrprt(mb->f_mntonname, stderr, 0);
		(void) fprintf(stderr, " (");
		safestrprt(mb->f_mntfromname, stderr, 0);
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
		    (void) fprintf(stderr, "%s: WARNING: can't stat() ", Pn);

		    safestrprt(mb->f_fstypename, stderr, 0);

		    (void) fprintf(stderr, " file system ");
		    safestrprt(mb->f_mntonname, stderr, 1);
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
		(void) bzero((char *)&sb, sizeof(sb));
		sb.st_dev = (dev_t)mb->f_fsid.val[0];
		sb.st_mode = S_IFDIR | 0777;
		if (!Fwarn) {
		    (void) fprintf(stderr,
			"      assuming \"dev=%x\" from mount table\n",
			sb.st_dev);
		}
	    }
	/*
	 * Allocate and fill a local mount structure.
	 */
	    if (!(mtp = (struct mounts *)malloc(sizeof(struct mounts))))
		goto no_space_for_mount;
	    mtp->dir = dn;
	    dn = (char *)NULL;
	    mtp->next = Lmi;
	    mtp->dev = sb.st_dev;
	    mtp->rdev = sb.st_rdev;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;
	    mtp->is_nfs = strcasecmp(mb->f_fstypename, "nfs") ? 0 : 1;
	/*
	 * Interpolate a possible file system (mounted-on) device name link.
	 */
	    if (!(dn = mkstrcpy(mb->f_mntfromname, (MALLOC_S *)NULL)))
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
	}
/*
 * Clean up and return the local mount info table address.
 */
	if (dn)
	    (void) free((FREE_P *)dn);
	Lmist = 1;
	return(Lmi);
}
