/*
 * dmnt.c - FreeBSD mount support functions for lsof
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
static char *rcsid = "$Id: dmnt.c,v 1.17 2013/01/02 17:01:43 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local static information
 */

static struct mounts *Lmi = (struct mounts *)NULL;	/* local mount info */
static int Lmist = 0;					/* Lmi status */

#undef	HAS_MNT_NAMES

#if	FREEBSDV<2000
static char *mnt_names[] = { "none", "ufs", "nfs", "mfs", "pc", "iso9600",
			     "procfs", "devfs" };
#define	HAS_MNT_NAMES	1
#else	/* FREEBSDV>=2000 */
# if	defined(MOUNT_NONE)
static char *mnt_names[] = INITMOUNTNAMES;
#define	HAS_MNT_NAMES	1
# endif	/* defined(MOUNT_NONE)) */
#endif	/* FREEBSDV<2000 */


#if	FREEBSDV>=5000 && defined(HAS_NO_SI_UDEV)
/*
 * Dev2Udev() -- convert a kernel device number to a user device number
 */

dev_t
Dev2Udev(c)

# if	defined(HAS_CONF_MINOR) || defined(HAS_CDEV2PRIV)
	KA_T c;
# else	/* !defined(HAS_CONF_MINOR) && !defined(HAS_CDEV2PRIV) */
	struct cdev *c;
# endif	/* defined(HAS_CONF_MINOR) || defined(HAS_CDEV2PRIV) */

{

# if	!defined(HAS_CONF_MINOR) && !defined(HAS_CDEV2PRIV)
	char *cp;
	char *dn = (char *)NULL;
	char *ln = (char *)NULL;
	struct statfs *mb;
	int n, sr;
	static u_int s;
	struct stat sb;
	static int ss = 0;
# endif	/* !defined(HAS_CONF_MINOR) && !defined(HAS_CDEV2PRIV) */

# if	defined(HAS_CONF_MINOR) || defined(HAS_CDEV2PRIV)
	KA_T ca;
	struct cdev_priv cp;

	if (!c)
	    return(NODEV);

#  if	defined(HAS_CDEV2PRIV)
	ca = (KA_T)cdev2priv((struct cdev *)c);
#  else	/* !defined(HAS_CDEV2PRIV) */
	ca = (KA_T)member2struct(cdev_priv, cdp_c, c);
#  endif	/* defined(HAS_CDEV2PRIV) */

	if (kread((KA_T)ca, (char *)&cp, sizeof(cp)))
	    return(NODEV);
	return((dev_t)cp.cdp_inode);
# else	/* !defined(HAS_CONF_MINOR) && !defined(HAS_CDEV2PRIV) */
#  if	defined(HAS_SI_PRIV)
/*
 * If the cdev structure has a private sub-structure, read it.
 */
	struct cdev_priv sp;

	if (!c->si_priv || kread((KA_T)c->si_priv, (char *)&sp, sizeof(sp)))
	    return(0);
#  endif	/* defined(HAS_SI_PRIV) */

	if (ss) {

#  if	defined(HAS_SI_PRIV)
	    return(sp.cdp_inode ^ s);
#  else	/* !defined(HAS_SI_PRIV) */
	    return(c->si_inode ^ s);
#  endif	/* defined(HAS_SI_PRIV) */

	}

/*
 * Determine the random udev seed from stat(2) operations on "/" and
 * its device.
 */
	if ((n = getmntinfo(&mb, MNT_NOWAIT)) <= 0) {
	    (void) fprintf(stderr, "%s: no mount information\n", Pn);
	    Exit(1);
	}
	for (; n; n--, mb++) {

# if	defined(MOUNT_NONE)
	    if (mb->f_type == MOUNT_NONE || mb->f_type >= MOUNT_MAXTYPE)
# else	/* !defined(MOUNT_NONE) */
	    if (!mb->f_type)
# endif	/* defined(MOUNT_NONE) */

		continue;
	/*
	 * Get the real directory name.  Ignore all but the root directory;
	 * safely stat("/").
	 */
	    if (dn)
		(void) free((FREE_P *)dn);
	    if (!(dn = mkstrcpy(mb->f_mntonname, (MALLOC_S *)NULL))) {

Dev2Udev_no_space:

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
	    ln = (char *)NULL;
	    if (strcmp(dn, "/"))
		continue;
	    if (statsafely(dn, &sb))
		continue;
	/*
	 * Get the real device name and safely stat(2) it.
	 */
	    (void) free((FREE_P *)dn);
	    if (!(dn = mkstrcpy(mb->f_mntfromname, (MALLOC_S *)NULL)))
		goto Dev2Udev_no_space;
	    ln = Readlink(dn);
	    if ((sr = statsafely(ln, &sb))) {

	    /*
	     * If the device stat(2) failed, see if the device name indicates
	     * an NFS mount, a cd9660 device, or a ZFS mount.  If any condition
	     * is true, set the user device number seed to zero.
	     */
		if (((cp = strrchr(ln, ':')) && (*(cp + 1) == '/'))
		||  !strcasecmp(mb->f_fstypename, "cd9660")
		||  !strcasecmp(mb->f_fstypename, "zfs")
		) {
		    ss = 1;
		    s = (u_int)0;
		}
	    }
	    if (ln != dn)
		(void) free((FREE_P *)ln);
	    ln = (char *)NULL;
	    (void) free((FREE_P *)dn);
	    dn = (char *)NULL;
	    if (sr && !ss)
		continue;
	    if (!ss) {
		ss = 1;
		s = (u_int)sb.st_ino ^ (u_int)sb.st_rdev;
	    }
	    break;
	}
/*
 * Free string copies, as required.
 */
	if (dn)
	    (void) free((FREE_P *)dn);
	if (ln)
	    (void) free((FREE_P *)ln);
/*
 * If the device seed is known, return its application to the cdev structure's
 * inode.
 */
	if (ss) {

#  if	defined(HAS_SI_PRIV)
	    return(sp.cdp_inode ^ s);
#  else	/* !defined(HAS_SI_PRIV) */
	    return(c->si_inode ^ s);
#  endif	/* defined(HAS_SI_PRIV) */

	}
	(void) fprintf(stderr, "%s: can't determine user device random seed.\n",	    Pn);
	Exit(1);

# endif	/* !defined(HAS_CONF_MINOR) */

}
#endif	/* FREEBSDV>=5000 && defined(HAS_NO_SI_UDEV) */


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
	struct stat sb;

#if	defined(HASPROCFS)
	unsigned char procfs = 0;
#endif	/* defined(HASPROCFS) */

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

#if	defined(MOUNT_NONE)
	    if (mb->f_type == MOUNT_NONE || mb->f_type >= MOUNT_MAXTYPE)
#else	/* !defined(MOUNT_NONE) */
	    if (!mb->f_type)
#endif	/* defined(MOUNT_NONE) */

		continue;
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

#if	defined(HAS_MNT_NAMES)
		    safestrprt(mnt_names[mb->f_type], stderr, 0);
#else	/* !defined(HAS_MNT_NAMES) */
		    safestrprt(mb->f_fstypename, stderr, 0);
#endif	/* defined(HAS_MNT_NAMES) */

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

#if	defined(HASPROCFS)

#if	defined(MOUNT_NONE)
	    if (mb->f_type == MOUNT_PROCFS)
#else	/* !defined(MOUNT_NONE) */
	    if (strcasecmp(mb->f_fstypename, "procfs") == 0)
#endif	/* defined(MOUNT_NONE) */

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

	    mtp->next = Lmi;
	    mtp->dev = sb.st_dev;
	    mtp->rdev = sb.st_rdev;
	    mtp->inode = (INODETYPE)sb.st_ino;
	    mtp->mode = sb.st_mode;
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


/*
 * readvfs() - read vfs structure
 */

struct l_vfs *
readvfs(vm)
	KA_T vm;			/* kernel mount address from vnode */
{
	struct mount m;
	struct l_vfs *vp;
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
	if (!(vp->dir = mkstrcpy(m.m_stat.f_mntonname, (MALLOC_S *)NULL))
	||  !(vp->fsname = mkstrcpy(m.m_stat.f_mntfromname, (MALLOC_S *)NULL)))
	{
	    (void) fprintf(stderr, "%s: PID %d, no space for mount names\n",
		Pn, Lp->pid);
	    Exit(1);
	}
	vp->addr = vm;
	vp->fsid = m.m_stat.f_fsid;

#if	defined(MOUNT_NONE)
	vp->type = m.m_stat.f_type;
#else	/* !defined(MOUNT_NONE) */
	{
	    int len;

	    if ((len = strlen(m.m_stat.f_fstypename))) {
		if (len > (MFSNAMELEN - 1))
		    len = MFSNAMELEN - 1;
		if (!(vp->typnm = mkstrcat(m.m_stat.f_fstypename, len,
				  (char *)NULL, -1, (char *)NULL, -1,
				  (MALLOC_S *)NULL)))
		{
		    (void) fprintf(stderr,
			"%s: no space for fs type name: ", Pn);
		    safestrprt(m.m_stat.f_fstypename, stderr, 1);
		    Exit(1);
		}
	    } else
		vp->typnm = "";
	}
#endif	/* defined(MOUNT_NONE) */

	vp->next = Lvfs;
	Lvfs = vp;
	return(vp);
}
