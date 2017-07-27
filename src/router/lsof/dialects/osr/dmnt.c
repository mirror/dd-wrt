/*
 * dmnt.c - SCO OpenServer mount support functions for lsof
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

#ifndef lint
static char copyright[] =
"@(#) Copyright 1995 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dmnt.c,v 1.8 2005/08/08 19:54:32 abe Exp $";
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
	int br, fd;
	int bx = sizeof(struct mnttab);
	char *cp;
	char dvnm[MAXPATHLEN], fsnm[MAXPATHLEN];
 	MALLOC_S dvnml, fsnml;
	MALLOC_S len;
	char *ln = (char *)NULL;
	struct mnttab m;
	struct mounts *mtp;
	struct stat sb;

	if (Lmi || Lmist)
	    return(Lmi);
/*
 * Open access to the mount table.
 */
	if ((fd = open(MNTTAB, O_RDONLY, 0)) < 0) {
	    (void) fprintf(stderr, "%s: can't open %s\n", Pn, MNTTAB);
	    Exit(1);
	}
/*
 * Read the first mount table entry.
 */
	br = read(fd, (char *)&m, bx);
	dvnml = fsnml = 0;
/*
 * Process the next complete mount table entry.
 */
	while (br == bx) {
	    if (!dvnml) {

	    /*
	     * Start the device and file system name assemblies.
	     */
		dvnml = strlen(m.mt_dev);
		if (dvnml >= MAXPATHLEN)
		    dvnml = MAXPATHLEN - 1;
		(void) strncpy(dvnm, m.mt_dev, dvnml);
		dvnm[dvnml] = '\0';
		fsnml = strlen(m.mt_filsys);
		if (fsnml >= MAXPATHLEN)
		    fsnml = MAXPATHLEN - 1;
		(void) strncpy(fsnm, m.mt_filsys, fsnml);
		fsnm[fsnml] = '\0';
	    }
	    while ((br = read(fd, (char *)&m, bx)) == bx
	    &&      strcmp(m.mt_filsys, "nothing") == 0
	    &&	    strcmp(m.mt_dev,    "nowhere") == 0) {

	    /*
	     * Add the "nothing/nowhere" extensions to the assemblies.
	     */
		len = strlen(&m.mt_dev[8]);
		if (len >= (MAXPATHLEN - dvnml))
		    len = MAXPATHLEN - dvnml - 1;
		if (len) {
		    (void) strncpy(&dvnm[dvnml], &m.mt_dev[8], len);
		    dvnml += len;
		    dvnm[dvnml] = '\0';
		}
		len = strlen(&m.mt_filsys[8]);
		if (len >= (MAXPATHLEN - fsnml))
		    len = MAXPATHLEN - fsnml - 1;
		if (len) {
		    (void) strncpy(&fsnm[fsnml], &m.mt_filsys[8], len);
		    fsnml += len;
		    fsnm[fsnml] = '\0';
		}
	    }
	/*
	 * Skip automount place markers.
	 */
	    if ((cp = strrchr(dvnm, ':')) && strncmp(cp, ":(pid", 5) == 0) {
		dvnml = fsnml = 0;
		continue;
	    }
	/*
	 * Interpolate a possible symbolic directory link.
	 */
	    if (ln) {
		(void) free((FREE_P *)ln);
		ln = (char *)NULL;
	    }
	    if (!(ln = Readlink(fsnm))) {
		if (!Fwarn){
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
		dvnml = fsnml = 0;
		continue;
	    }
	    if (*ln != '/')
		continue;
	    if (ln == fsnm) {

	    /*
	     * Allocate space for a copy of the file system name.
	     */
		if (!(ln = mkstrcpy(fsnm, (MALLOC_S *)NULL))) {

no_space_for_mount:

		    (void) fprintf(stderr, "%s: no space for mount at ", Pn);
		    safestrprt(fsnm, stderr, 0);
		    (void) fprintf(stderr, " (");
		    safestrprt(dvnm, stderr, 0);
		    (void) fprintf(stderr, ")\n");
		    Exit(1);
		}
	    }
	/*
	 * Stat() the directory.
	 */
	    if (statsafely(ln, &sb)) {
		if (!Fwarn) {
		    (void) fprintf(stderr,
			"%s: WARNING: can't stat() file system: ", Pn);
		    safestrprt(fsnm, stderr, 1);
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
		dvnml = fsnml = 0;
		continue;
	    }
	/*
	 * Allocate and fill a local mount structure.
	 */
	    if (!(mtp = (struct mounts *)malloc(sizeof(struct mounts))))
		goto no_space_for_mount;
	    mtp->dir = ln;
	    ln = (char *)NULL;
	    mtp->next = Lmi;
	    mtp->dev = sb.st_dev;
	    mtp->rdev = sb.st_rdev;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;
	/*
	 * Interpolate a possible file system (mounted-on) device name link
	 */
	    if (!(cp = mkstrcpy(dvnm, (MALLOC_S *)NULL)))
		goto no_space_for_mount;
	    mtp->fsname = cp;
	    ln = Readlink(cp);
	/*
	 * Stat() the file system (mounted-on) name and add file system
	 * information to the local mount table entry.
	 */
	    if (statsafely(ln, &sb))
		sb.st_mode = 0;
	    mtp->fsnmres = ln;
	    ln = (char *)NULL;
	    mtp->fs_mode = sb.st_mode;
	    Lmi = mtp;
	    dvnml = fsnml = 0;
	}
	(void) close(fd);
/*
 * Clean up and return the local mount information table address.
 */
	if (ln)
	    (void) free((FREE_P *)ln);
	Lmist = 1;
	return(Lmi);
}
