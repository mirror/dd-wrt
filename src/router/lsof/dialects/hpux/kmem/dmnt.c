/*
 * dmnt.c - /dev/kmem-based HP-UX mount support functions for lsof
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
static char *rcsid = "$Id: dmnt.c,v 1.8 2005/08/08 19:50:23 abe Exp $";
#endif

#if	defined(HPUXKERNBITS) && HPUXKERNBITS>=64
#define	_TIME_T
typedef	int	time_t;
#endif	/* defined(HPUXKERNBITS) && HPUXKERNBITS>=64 */

#include "lsof.h"


/*
 * Local static definitions
 */

static struct mounts *Lmi = (struct mounts *)NULL;	/* local mount info */


/*
 * completevfs() - complete local vfs structure
 */
void

#if	HPUXV>=800
completevfs(vfs, dev, v)
	struct l_vfs *vfs;		/* local vfs structure pointer */
	dev_t *dev;			/* device */
	struct vfs *v;			/* kernel vfs structure */
#else	/* HPUXV<800 */
completevfs(vfs, dev)
	struct l_vfs *vfs;		/* local vfs structure pointer */
	dev_t *dev;			/* device */
#endif	/* HPUXV>=800 */

{
	struct mounts *mp;
/*
 * If only Internet socket files are selected, don't bother completing the
 * local vfs structure.
 */
	if (Selinet)
	    return;

#if	HPUXV>=800
/*
 * On HP-UX 8 and above, first search the local mount table for a match on
 * the file system name from the vfs structure.
 */
	if (v) {
	    for (mp = readmnt(); mp; mp = mp->next) {
		if (strcmp(mp->fsname, v->vfs_name) == 0) {
		    vfs->dev = mp->dev;
		    vfs->dir = mp->dir;
		    vfs->fsname = mp->fsname;

# if	defined(HASFSINO)
		    vfs->fs_ino = mp->inode;
# endif	/* defined(HASFSINO) */

		    return;
		}
	    }
	}
#endif	/* HPUXV>=800 */

/*
 * Search for a match on device number.
 */
	for (mp = readmnt(); mp; mp = mp->next) {
	    if (mp->dev == *dev) {
		vfs->dev = mp->dev;
		vfs->dir = mp->dir;
		vfs->fsname = mp->fsname;

#if	defined(HASFSINO)
		vfs->fs_ino = mp->inode;
#endif	/* defined(HASFSINO) */

		return;
	    }
	}

#if	HPUXV>=800
/*
 * If the file system name and device number searches fail, use the
 * vfs structure name, if there is one.  Determine the device number
 * with statsafely().
 */
	if (v && v->vfs_name[0]) {
		
	    struct stat sb;

	    if (!(vfs->dir = mkstrcpy(v->vfs_name, (MALLOC_S *)NULL))) {
		(void) fprintf(stderr, "%s: no space for vfs name: ", Pn);
		safestrprt(v->vfs_name, stderr, 1);
		Exit(1);
	    }
	    if (statsafely(v->vfs_name, &sb) == 0)
		vfs->dev = sb.st_dev;
	    else
		vfs->dev = (dev_t)0;

# if	defined(HASFSINO)
	    vfs->fs_ino = (INODETYPE)0;
# endif	/* defined(HASFSINO) */

	}
#endif	/* HPUXV>=800 */

}


/*
 * readvfs() - read vfs structure
 */

struct l_vfs *
readvfs(lv)
	struct vnode *lv;		/* local vnode */
{
	struct mount m;
	struct mntinfo mi;
	int ms;
	dev_t td;
	struct vfs v;
	struct l_vfs *vp;

	if (!lv->v_vfsp)
	    return((struct l_vfs *)NULL);
	for (vp = Lvfs; vp; vp = vp->next) {
	    if ((KA_T)lv->v_vfsp == vp->addr)
		return(vp);
	}
	if ((vp = (struct l_vfs *)malloc(sizeof(struct l_vfs))) == NULL) {
	    (void) fprintf(stderr, "%s: PID %d, no space for vfs\n",
		Pn, Lp->pid);
	    Exit(1);
	}
	vp->dev = 0;
	vp->dir = (char *)NULL;
	vp->fsname = (char *)NULL;

#if	defined(HASFSINO)
	vp->fs_ino = 0;
#endif	/* defined(HASFSINO) */

	if (lv->v_vfsp && kread((KA_T)lv->v_vfsp, (char *)&v, sizeof(v))) {
	    (void) free((FREE_P *)vp);
	    return((struct l_vfs *)NULL);
	}
/*
 * Complete the mount information.
 */
	if (Ntype == N_NFS) {

	/*
	 * The device number for an NFS file is found by following the vfs
	 * private data pointer to an mntinfo structure.
	 */
	    if (v.vfs_data
	    &&  kread((KA_T)v.vfs_data, (char *)&mi, sizeof(mi)) == 0) {

#if	HPUXV<1020
		td = (dev_t)makedev(255, (int)mi.mi_mntno);
#else	/* HPUXV>=1020 */
		td = mi.mi_mntno;
#endif	/* HPUXV<1020 */

#if	HPUXV>=800
		(void) completevfs(vp, &td, (struct vfs *)NULL);
#else	/* HPUXV<800 */
		(void) completevfs(vp, &td);
#endif	/* HPUXV>=800 */

	    }
	} else {
	    if (v.vfs_data) {
		if (kread((KA_T)v.vfs_data, (char *)&m, sizeof(m)) == 0)
		    ms = 1;
		else
		    ms = 0;
	    }

#if	defined(HAS_AFS)
	/*
	 * Fake the device number for an AFS device.
	 */
	    else if (Ntype == N_AFS) {
		m.m_dev = AFSDEV;
		ms = 1;
	    }
#endif	/* defined(HAS_AFS) */

	    else
		ms = 0;
	    if (ms)

#if	HPUXV>=800
# if	HPUXV<1000
		(void) completevfs(vp, (dev_t *)&m.m_dev, &v);
# else	/* HPUXV>=1000 */
		(void) completevfs(vp, v.vfs_dev ? (dev_t *)&v.vfs_dev 
						 : (dev_t *)&m.m_dev,
				   &v);
# endif	/* HPUXV<1000 */
#else	/* HPUXV<800 */
		(void) completevfs(vp, (dev_t *)&m.m_dev);
#endif	/* HPUXV>=800 */

	}
/*
 * Complete local vfs structure and link to the others.
 */
	vp->next = Lvfs;
	vp->addr = (KA_T)lv->v_vfsp;
	Lvfs = vp;
	return(vp);
}
