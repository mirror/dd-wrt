/*
 * dnode2.c - FreeBSD ZFS node functions for lsof
 *
 * This module must be separate to permit use of the OpenSolaris ZFS header
 * files.
 */


/*
 * Copyright 2008 Purdue Research Foundation, West Lafayette, Indiana
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
"@(#) Copyright 2008 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dnode2.c,v 1.6 2015/07/07 20:23:43 abe Exp $";
#endif


#if	defined(HAS_ZFS)

#define _KERNEL

# if	defined(__clang__)
/*
 * A clang workaround...
 *
 * Note: clang's complaint about VOP_FSYNC can't be avoided.
 */
#define	VOP_UNLOCK(vp, f)	((void)0)
# endif	/* defined(__clang__) */

#include <sys/zfs_znode.h>
#undef	_KERNEL

#include "dzfs.h"


/*
 * readzfsnode() -- read the ZFS node
 */

char *
readzfsnode(za, zi, vr)
	KA_T za;			/* ZFS node address */
	zfs_info_t *zi;			/* return ZFS info structure pointer */
	int vr;				/* vnode's (v_flag & VROOT) */
{
	struct znode zn;		/* ZFS node */

# if	defined(HAS_Z_PHYS)
	znode_phys_t zp;		/* ZFS physical node */
# else	/* !defined(HAS_Z_PHYS) */
	KA_T ka;			/* temporary kernel address */
	zfsvfs_t zv;			/* znode's zfsvfs structure */
# endif	/* defined(HAS_Z_PHYS) */

	if (!za
	||  kread(za, (char *)&zn, sizeof(zn))
	) {
	    if (!za)
		return("No ZFS node address");
	    return("Can't read znode");
	}
/*
 * Return items contained in the znode.
 */
	zi->ino = (INODETYPE)zn.z_id;
	zi->ino_def = 1;

# if	!defined(HAS_V_LOCKF)
	zi->lockf = (KA_T)zn.z_lockf;
# endif	/* !defined(HAS_V_LOCKF) */

# if	defined(HAS_Z_PHYS)
/*
 * If the physical znode exists in this ZFS implementation, read it.
 */
	if (!zn.z_phys
	||  kread((KA_T)zn.z_phys, (char *)&zp, sizeof(zp))
	) {
	    if (!zn.z_phys)
		return("No physical znode address");
	    return("Can't read physical znode");
	}
/*
 * Return items contained in the physical znode.
 */
	zi->nl = (long)zp.zp_links;
	zi->rdev = zp.zp_rdev;
	zi->sz = (SZOFFTYPE)zp.zp_size;
	zi->nl_def = zi->rdev_def = zi->sz_def = 1;
# else	/* !defined(HAS_Z_PHYS) */
/*
 * If this implementation has no physical znode, return items now contained
 * in the znode.
 */
	zi->nl = (long)zn.z_links;
	if (vr && (ka = (KA_T)zn.z_zfsvfs)) {
	    if (!kread(ka, (char *)&zv, sizeof(zv))) {
		if ((zn.z_id == zv.z_root)
		&&  (zv.z_ctldir != NULL)
		&&  (zv.z_show_ctldir)
		) {
		    zi->nl++;
		}
	    }
	}
	zi->sz = (SZOFFTYPE)zn.z_size;
	zi->nl_def = zi->sz_def = 1;
# endif	/* defined(HAS_Z_PHYS) */

	return((char *)NULL);
}




# if	defined(__GNUC__) && defined(HAS_CV_TIMEDWAIT_SBT)
/*
 * A gcc work-around
 */

int     _cv_timedwait_sbt(struct cv *cvp, struct lock_object *lock,       
            sbintime_t sbt, sbintime_t pr, int flags)
{
	return(0);
}
# endif	/* defined(__GNUC__) && HAS_CV_TIMEDWAIT_SBT */
#endif	/* defined(HAS_ZFS) */
