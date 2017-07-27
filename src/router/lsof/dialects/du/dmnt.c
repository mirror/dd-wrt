/*
 * dmnt.c - DEC OSF/1, Digital UNIX, Tru64 UNIX mount support functions for
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
static char *rcsid = "$Id: dmnt.c,v 1.11 2005/08/08 19:56:44 abe Exp $";
#endif


#include "lsof.h"

#undef	KERNEL
#include <sys/fs_types.h>		/* this defines char *mnt_names[] */


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
	struct statfs *mb;
	struct mounts *mtp;
	int n;
	int procfs = 0;
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
	    if (mb->f_type == MOUNT_NONE || mb->f_type >= MOUNT_MAXTYPE)
		continue;
	/*
	 * Avoid file systems under automounter control if they're not
	 * currently mounted.
	 */
	    if (mb->f_type == MOUNT_NFS) {

	    /*
	     * The mount-from name of some unmounted file systems under
	     * automounter control end with ``:(pid<n>):'' -- where <n>
	     * is the PID of the automounter process.
	     */
		if ((ln = strchr(mb->f_mntfromname, ':'))) {
		    if (strncmp(ln+1, "(pid", 4) == 0 && isdigit(*(ln+5))) {
			for (ln += 6; *ln && isdigit(*ln); ln++) {
			    ;
			}
			if (*ln == ')' && *(ln+1) == '\0')
			    continue;
		    }
		}
	    /*
	     * Another automounter mount-from name form is "amd:<n>" --
	     * where <n> is the PID of the automounter process.
	     */
		if (strncmp(mb->f_mntfromname, "amd:", 4) == 0
		&&  isdigit(mb->f_mntfromname[4])) {
		    ln = &mb->f_mntfromname[5];
		    while (*ln && isdigit(*ln)) {
			ln++;
		    }
		    if (!*ln || (*ln == ':' && *(ln+1) == '\0'))
			continue;
		}
	    }
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
		    (void) fprintf(stderr,
			"%s: WARNING: can't stat() %s file system: ",
			Pn, mnt_names[mb->f_type]);
		    safestrprt(mb->f_mntonname, stderr, 1);
		    (void) fprintf(stderr,
			"      Output information may be incomplete.\n");
		}
		if (mb->f_type != MOUNT_PROCFS

#if	!defined(ADVFSV) || ADVFSV<400
		&&  mb->f_type != MOUNT_MSFS
#endif	/* !defined(ADVFSV) || ADVFSV<400 */

		) {
		    memset((char *)&sb, 0, sizeof(sb));
		    sb.st_dev = (dev_t)mb->f_fsid.val[0];
		    sb.st_mode = S_IFDIR | 0777;
		    if (!Fwarn) {
			(void) fprintf(stderr,
			    "      assuming dev=%x from mount table\n",
			    sb.st_dev);
		    }
		} else
		    continue;
	    }
	/*
	 * Allocate and fill a local mount structure.
	 */
	    if (!(mtp = (struct mounts *)malloc(sizeof(struct mounts))))
		goto no_space_for_mount;
	    mtp->dir = dn;
	    dn = (char *)NULL;
	    mtp->dev = sb.st_dev;
	    mtp->fsid = mb->f_fsid;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;
	    mtp->next = Lmi;
	    mtp->rdev = sb.st_rdev;
	/*
	 * Interpolate a possible file system (mounted-on) device path.
	 */
	    if (!(dn = mkstrcpy(mb->f_mntfromname, (MALLOC_S *)NULL)))
		goto no_space_for_mount;
	    mtp->fsname = dn;
	    ln = Readlink(dn);
	    dn = (char *)NULL;
	/*
	 * Stat the file system (mounted-on) name and add file sysem
	 * information to the local mount table.
	 */
	    if (!ln || statsafely(ln, &sb))
		sb.st_mode = 0;
	    mtp->fsnmres = ln;
	    mtp->fs_mode = sb.st_mode;
	    Lmi = mtp;
	    if (mb->f_type == MOUNT_PROCFS) {

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
	}
/*
 * Clean up and return the local mount info table address.
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
readvfs(vm)
	KA_T vm;		/* mount address in vnode */
{
	struct mount m;
	struct l_vfs *vp;
	fsid_t f;
	struct mounts *mp;

#if	DUV>=40000
	int bl;
	char fb[MAX_MNT_PATHLEN+1];
	char ob[MAX_MNT_PATHLEN+1];
#endif	/* DUV>=40000 */

/*
 * Search for match on existing entry.
 */
	for (vp = Lvfs; vp; vp = vp->next) {
	    if (vm == vp->addr)
		return(vp);
	}
/*
 * Read the (new) mount structure, allocate a local entry, and fill it.
 */
	if (kread((KA_T)vm, (char *)&m, sizeof(m)) != 0)
	    return((struct l_vfs *)NULL);
	if (!(vp = (struct l_vfs *)malloc(sizeof(struct l_vfs)))) {
	    (void) fprintf(stderr, "%s: PID %d, no space for vfs\n",
		Pn, Lp->pid);
	    Exit(1);
	}

#if	DUV<40000
	if (!(vp->dir = mkstrcpy(m.m_stat.f_mntonname, (MALLOC_S *)NULL))
	||  !(vp->fsname = mkstrcpy(m.m_stat.f_mntfromname, (MALLOC_S *)NULL)))
#else	/* DUV>=40000 */
	bl = sizeof(ob) - 1;
	if (!m.m_stat.f_mntonname
	||  kread((KA_T)m.m_stat.f_mntonname, ob, bl))
	    bl = 0;
	ob[bl] = '\0';
	bl = sizeof(fb) - 1;
	if (!m.m_stat.f_mntfromname
	||  kread((KA_T)m.m_stat.f_mntfromname, fb, bl))
	    bl = 0;
	fb[bl] = '\0';
	if (!(vp->dir = mkstrcpy(ob, (MALLOC_S *)NULL))
	||  !(vp->fsname = mkstrcpy(fb, (MALLOC_S *)NULL)))
#endif	/* DUV<40000 */

	{
	    (void) fprintf(stderr, "%s: PID %d, no space for mount names\n",
		Pn, Lp->pid);
	    Exit(1);
	}
	vp->addr = vm;
	vp->fsid = m.m_stat.f_fsid;
	vp->type = m.m_stat.f_type;

#if	defined(HASFSINO)
	vp->fs_ino = 0;
#endif	/* defined(HASFSINO) */

	vp->next = Lvfs;
	Lvfs = vp;
/*
 * Derive the device and raw device numbers from a search for the
 * file system ID in the local mount table.
 */
	vp->dev = vp->rdev = 0;
	for (f = vp->fsid, mp = readmnt(); mp; mp = mp->next) {
	    if (f.val[0] == mp->fsid.val[0]
	    &&  f.val[1] == mp->fsid.val[1])
	    {
		vp->dev = mp->dev;
		vp->rdev = mp->rdev;

#if	defined(HASFSINO)
		vp->fs_ino = mp->inode;
#endif	/* defined(HASFSINO) */

		break;
	    }
	}
	return(vp);
}
