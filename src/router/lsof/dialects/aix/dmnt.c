/*
 * dmnt.c - AIX mount support functions for lsof
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
static char *rcsid = "$Id: dmnt.c,v 1.13 2005/08/08 19:46:38 abe Exp $";
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
	char *dir, *fs, *h, *ln, *ty;
	char *dn = (char *)NULL;
	struct mounts *mtp;
	int nm;
	struct stat sb;
	MALLOC_S sz;
	struct vmount *v;
	struct vmount *vt = (struct vmount *)NULL;

	if (Lmi || Lmist)
	    return(Lmi);
/*
 * Read the table of vmount structures.
 */
	for (sz = sizeof(struct vmount);;) {
	    if (!(vt = (struct vmount *)malloc(sz))) {
		(void) fprintf(stderr, "%s: no space for vmount table\n", Pn);
		return(0);
	    }
	    nm = mntctl(MCTL_QUERY, sz, (unsigned char *)vt);
	    if (nm > 0) {
		if (vt->vmt_revision != VMT_REVISION) {
		    (void) fprintf(stderr,
			"%s: stale file system, rev %d != %d\n",
			Pn, vt->vmt_revision, VMT_REVISION);
		    return(0);
		}
		break;
	    }
	    if (nm == 0) {
		sz = (unsigned)vt->vmt_revision;
		(void) free((FREE_P *)vt);
	    } else {
		(void) fprintf(stderr, "%s: mntctl error: %s\n",
		    Pn, strerror(errno));
		return(0);
	    }
	}
/*
 * Scan the vmount structures and build Lmi.
 */
	for (v = vt; nm--; v = (struct vmount *)((char *)v + v->vmt_length)) {
	    dir = (char *)vmt2dataptr(v, VMT_STUB);
	    fs = (char *)vmt2dataptr(v, VMT_OBJECT);
	    h = (char *)vmt2dataptr(v, VMT_HOST);
            if (statsafely(dir, &sb)) {
		if (!Fwarn) {

		/*
		 * Issue stat() failure warning.
		 */
		    switch(v->vmt_gfstype) {

#if	defined(HAS_AFS)
		    case MNT_AFS:
			ty = "afs";
			break;
#endif	/* defined(HAS_AFS) */

#if	defined(MNT_AIX) && defined(MNT_J2) && MNT_AIX==MNT_J2
		    case MNT_AIX:
			ty = "jfs2";
			break;
#else	/* !defined(MNT_AIX) || !defined(MNT_J2) || MNT_AIX!=MNT_J2 */
# if	defined(MNT_AIX)
		    case MNT_AIX:
			ty = "oaix";
			break;
# endif	/* defined(MNT_AIX) */
# if	defined(MNT_J2)
		    case MNT_J2:
			ty = "jfs2";
			break;
# endif	/* defined(MNT_J2) */
#endif	/* defined(MNT_AIX) && defined(MNT_H2) && MNT_AIX==MNT_J2 */

		    case MNT_CDROM:
			ty = "cdrom";
			break;
		    case MNT_JFS:
			ty = "jfs";
			break;
		    case MNT_NFS:
			ty = "nfs";
			break;

#if	defined(MNT_NFS3)
		    case MNT_NFS3:
			ty = "nfs3";
			break;
#endif	/* defined(MNT_NFS3) */

#if	defined(HASPROCFS)
		    case MNT_PROCFS:
			ty = HASPROCFS;
			break;
#endif	/* defined(HASPROCFS) */

#if	defined(MNT_SANFS)
		    case MNT_SANFS:
			ty = "sanfs";
			break;
#endif	/* defined(MNT_SANFS) */

		    default:
			ty = "unknown";
		    }
		    (void) fprintf(stderr,
			"%s: WARNING: can't stat() %s file system %s\n",
			Pn, ty, dir);
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
	    /*
	     * Assemble alternate device number and mode flags.
	     */
		(void) bzero((char *)&sb, sizeof(sb));
		if (v->vmt_flags & MNT_REMOTE) {

#if	AIXA<2
		    sb.st_dev = (dev_t)(SDEV_REMOTE | v->vmt_vfsnumber);
#else	/* AIXA>=2 */
		    sb.st_dev = (dev_t)(SDEV_REMOTE | (SDEV_REMOTE << 32)
			      |         v->vmt_vfsnumber);
#endif	/* AIXA<2 */

		} else {

#if	defined(HAS_AFS)
		    if (v->vmt_gfstype == MNT_AFS)
			sb.st_dev = AFSDEV;
		    else
#endif	/* defined(HAS_AFS) */

#if	AIXA>=2 && defined(HASPROCFS)
		    if (v->vmt_gfstype == MNT_PROCFS) {

		    /*
		     * !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!
		     *
		     * The following *hack* is required to make the vmount
		     * structure's device number match what stat(2)
		     * errnoneously returns on ia64 AIX 5L.
 		     *
		     * REMOVE THIS CODE WHEN STAT(2) IS FIXED!!!
		     */
			sb.st_dev = (dev_t)(v->vmt_fsid.val[0]
				  &         0x7fffffffffffffff);
		    /*
		     * !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!
 		     */

		    }
		    else
#endif	/* AIXA>=2 && defined(HASPROCFS) */

			sb.st_dev = (dev_t)v->vmt_fsid.val[0];
		}
		if (!Fwarn)
		    (void) fprintf(stderr,
			"      assuming \"dev=%#lx\" from mount table\n",
			sb.st_dev);
		sb.st_mode = S_IFDIR | 0777;
	    }
	/*
	 * Allocate space for the directory (mounted on) and resolve
	 * any symbolic links.
	 */
	    if (dn)
		(void) free((FREE_P *)dn);
	    if (!(dn = mkstrcpy(dir, (MALLOC_S *)NULL))) {

no_space_for_mount:

		(void) fprintf(stderr, "%s: no space for mount at %s (%s)\n",
		    Pn, fs, dir);
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
	 * Allocate a local mounts structure and fill the directory information.
	 */
	    if (!(mtp = (struct mounts *)malloc(
			(MALLOC_S)sizeof(struct mounts))))
		goto no_space_for_mount;
	    mtp->dir = dn;
	    dn = (char *)NULL;
	    mtp->dev = sb.st_dev;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;
	    mtp->rdev = sb.st_rdev;

#if	defined(HASFSTYPE)
	    mtp->fstype = sb.st_vfstype;
#endif	/* defined(HASFSTYPE) */

	    mtp->next = Lmi;
	/*
	 * Form the file system (mounted-on) device name.  Resolve any
	 * symbolic links.  Allocate space for the result and store it in
	 * the local mounts structure.
	 */
	    if (h && (v->vmt_flags & MNT_REMOTE)) {
		if (!(dn = mkstrcat(h, -1, *h ? ":" : "", 1, fs, -1,
				    (MALLOC_S *)NULL)))
		    goto no_space_for_mount;
	    } else {
		if (!(dn = mkstrcpy(fs, (MALLOC_S *)NULL)))
		    goto no_space_for_mount;
	    }
	    mtp->fsname = dn;
	    ln = Readlink(dn);
	    dn = (char *)NULL;
	/*
	 * Stat the file system (mounted-on) device name to get its modes.
	 * Set the modes to zero if the stat fails.  Add file system
	 * (mounted-on) device information to the local mountsstructure.
	 */
	    if (!ln || statsafely(ln, &sb))
		sb.st_mode = 0;
	    mtp->fsnmres = ln;
	    mtp->fs_mode = sb.st_mode;
	    Lmi = mtp;
        }
/*
 * Clean up and return local mount info table address.
 */
	if (dn)
	    (void) free((FREE_P *)dn);
	if (vt)
	    (void) free((FREE_P *)vt);
	Lmist = 1;
	return(Lmi);
}
