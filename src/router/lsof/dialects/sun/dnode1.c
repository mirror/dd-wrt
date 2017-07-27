/*
 * dnode1.h - Solaris AFS support
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
static char *rcsid = "$Id: dnode1.c,v 1.9 2005/08/08 19:55:41 abe Exp $";
#endif


#if	defined(HAS_AFS)
#include "lsof.h"

#include <rpc/xdr.h>
#define __XDR_INCLUDE__
#define int32   old_solaris_int32

# if	solaris>=20600
#undef	SHARED
#undef	PRIVATE
# endif	/* solaris>=20600 */

#include <afs/param.h>
#include <afs/afsint.h>
#include <afs/vldbint.h>


/*
 * This is an emulation of the afs_rwlock_t definition that appears in
 * the AFS sources in afs/lock.h.
 */

# if	defined(AFS_SUN5_ENV)
#define AFS_NOBOZO_LOCK
# endif	/* defined(AFS_SUN5_ENV) */

#define INSTRUMENT_LOCKS

# if	defined(AFS_FINEGR_SUNLOCK)
typedef	kmutex_	afs_lock_t;
typedef	krwlock_t afs_rwlock_t;
# endif	/* !defined(AFS_FINEGR_SUNLOCK) */

struct afs_lock {

# if	solaris>=20500
    unsigned char d1[2];
    unsigned short d1_5[3];
# else	/* solaris < 20500 */
    unsigned char d1[4];
# endif	/* solaris>=20500 */

    struct timeval d2;

# if	defined(INSTRUMENT_LOCKS)
    unsigned int d3[3];
# endif	/* defined(INSTRUMENT_LOCKS) */

};
typedef struct afs_lock afs_lock_t;
typedef struct afs_lock afs_rwlock_t;


/*
 * This is an emulation of the afs_bozoLock_t definition that appears in
 * the AFS sources in afs/lock.h.
 */

struct afs_bozoLock {
    short d1;
    char d2[2];
    char *d3;
};

# if	!defined(AFS_NOBOZO_LOCK)
typedef struct afs_bozoLock afs_bozoLock_t;
# else	/* defined(AFS_NOBOZO_LOCK) */
#  if	defined(AFS_SUN5_ENV)
typedef kmutex_t afs_bozoLock_t;
#  else	/* !defined(AFS_SUN5_ENV) */
typedef struct afs_bozoLock afs_bozoLock_t;
#  endif	/* defined(AFS_SUN5_ENV) */
# endif	/* !defined(AFS_NOBOZO_LOCK) */

#define	KERNEL
#include <afs/afs.h>
#undef	KERNEL


/*
 * Local function prototypes
 */

_PROTOTYPE(static struct volume *getvolume,(struct VenusFid *f, int *vols));
_PROTOTYPE(static int is_rootFid,(struct vcache *vc, int *rfid));


/*
 * alloc_vcache() - allocate space for vcache structure
 */

struct vnode *
alloc_vcache()
{
	return((struct vnode *)malloc(sizeof(struct vcache)));
}


/*
 * ckAFSsym() - check for missing X_AFS_* symbols in AFS name list file
 */

void
ckAFSsym(nl)
	struct nlist *nl;		/* copy of Nl[] when empty */
{
	char *path = AFSAPATHDEF;
	int i;
	KA_T v;

# if	defined(HASAOPT)
	if (AFSApath)
	    path = AFSApath;
# endif	/* defined(HASAOPT) */

/*
 * If an alternate AFS name list file was specified, see if it can be read.
 */
	if (!is_readable(path, 0)) {
	    if (!Fwarn)
		(void) fprintf(stderr,
		    "%s: WARNING: can't access AFS name list file: %s\n",
		    Pn, path);
	    return;
	}
/*
 * Read the AFS modload symbols and compare its non-zero values with
 * the non-zero values in Nl[].  Quit if there is any mis-match.
 */
	if (nlist(path, nl) < 0)
	    return;
	for (i = 0; Nl[i].n_name && Nl[i].n_name[0]; i++) {
	    if (!nl[i].n_value || !Nl[i].n_value)
		continue;
	    if (nl[i].n_value != Nl[i].n_value)
		return;
	}
/*
 * If any AFS symbol that doesn't have a value in Nl[] has one from
 * the AFS modload file, copy its modload value to Nl[].
 */
	if ((i = get_Nl_value("arFid", Drive_Nl, &v)) >= 0
	&&  !Nl[i].n_value && nl[i].n_value)
	    Nl[i].n_value = nl[i].n_value;
	if ((i = get_Nl_value("avops", Drive_Nl, &v)) >= 0
	&&  !Nl[i].n_value && nl[i].n_value)
	    Nl[i].n_value = nl[i].n_value;
	if ((i = get_Nl_value("avol", Drive_Nl, &v)) >= 0
	&&  !Nl[i].n_value && nl[i].n_value)
	    Nl[i].n_value = nl[i].n_value;
}


/*
 * getvolume() - get volume structure
 */

static struct volume *
getvolume(f, vols)
	struct VenusFid *f;		/* file ID pointer */
	int *vols;			/* afs_volumes status return */
{
	int i;
	static KA_T ka = 0;
	KA_T kh;
	static struct volume v;
	KA_T vp;
	static int w = 0;

	if (!ka) {
	    if (get_Nl_value("avol", Drive_Nl, &ka) < 0 || !ka) {
		if (!w && !Fwarn) {
		    (void) fprintf(stderr,
			"%s: WARNING: no kernel address for afs_volumes\n", Pn);
		    (void) fprintf(stderr,
			"      This may hamper AFS node number reporting.\n");
		    w = 1;
		}
		*vols = 0;
		return((struct volume *)NULL);
	    }
	}
	*vols = 1;
	i = (NVOLS - 1) & f->Fid.Volume;
	kh = (KA_T)((char *)ka + (i * sizeof(struct volume *)));
	if (kread(kh, (char *)&vp, sizeof(vp)))
	    return((struct volume *)NULL);
	while (vp) {
	    if (kread((KA_T)vp, (char *)&v, sizeof(v)))
		return((struct volume *)NULL);
	    if (v.volume == f->Fid.Volume && v.cell == f->Cell)
		return(&v);
	    vp = (KA_T)v.next;
	}
	return((struct volume *)NULL);
}


/*
 * hasAFS() - test for AFS presence via vfs structure
 */

int
hasAFS(vp)
	struct vnode *vp;		/* vnode pointer */
{
	struct mounts *mp;
	int n;
	struct vfs v;
/*
 * If this vnode has a v_data pointer, then it probably isn't an AFS vnode;
 * return FALSE.
 *
 * If the vfs struct address of /afs is known and this vnode's v_vfsp matches
 * it, return TRUE.
 *
 * Read this vnode's vfs structure and see if it's device (fsid.val[0]) is
 * AFSdev.  If it is, record the AFS vfs struct address and return TRUE.
 */
	if (AFSVfsp && !vp->v_data && (KA_T)vp->v_vfsp == AFSVfsp)
	    return(1);
	if (!AFSdevStat)
	    (void) readmnt();
	if (!AFSdevStat
	||  vp->v_data
	||  !vp->v_vfsp
	||  kread((KA_T)vp->v_vfsp, (char *)&v, sizeof(v))
	||  v.vfs_data)
	    return(0);
	if ((dev_t)v.vfs_fsid.val[0] == AFSdev) {
	    AFSVfsp = (KA_T)vp->v_vfsp;
	    return(1);
	}
/*
 * Search the local mount table for /afs devices.  Count /afs devices,
 * and skip a device number test for them.  A match on device number for
 * non-AFS devices produces a FALSE return.
 */
	for (mp = readmnt(), n = 0; mp; mp = mp->next) {
	    if (AFSdevStat && mp->dev == AFSdev
	    &&  mp->dir && strcmp(mp->dir, "/afs") == 0
	    &&  mp->fsname && strcmp(mp->fsname, "AFS") == 0)
		n++;
	    else if (mp->dev == (dev_t)v.vfs_fsid.val[0])
		return(0);
	}
/*
 * If there is exactly one /afs device, assume its vfs struct address is
 * the one for this vnode, record it, and return TRUE.
 */
	if (n == 1) {
	    AFSVfsp = (KA_T)vp->v_vfsp;
	    return(1);
	}
	return(0);
}


/*
 * is_rootFid() - is the file ID the root file ID
 *
 * return: 0	= is not root file ID
 *	   1	= is root file ID
 *	   rfid = 0 if root file ID structure address not available
 *		  1 if root file ID structure address available
 */

static int
is_rootFid(vc, rfid)
	struct vcache *vc;		/* vcache structure */
	int *rfid;			/* root file ID pointer status return */
{
	KA_T arFid;
	char *err;
	static int f = 0;		/* rootFid structure status:
					 *     -1 = unavailable
					 *	0 = not yet accessed
					 *	1 = available */
	static struct VenusFid r;
	static int w = 0;

	switch (f) {
	case -1:
	    if (vc->v.v_flag & VROOT) {
		*rfid = 1;
		return(1);
	    }
	    *rfid = 0;
	    return(0);
	case 0:
	    if (get_Nl_value("arFid", Drive_Nl, &arFid) < 0 || !arFid) {
		err = "no afs_rootFid kernel address";

rfid_unavailable:

		if (!w && !Fwarn) {
		    (void) fprintf(stderr,
			"%s: WARNING: AFS root Fid error: %s\n", Pn, err);
		    (void) fprintf(stderr,
			"      This may hamper AFS node number reporting.\n");
		    w = 1;
		}
		f = -1;
		if (vc->v.v_flag & VROOT) {
		    *rfid = 1;
		    return(1);
		}
		*rfid = 0;
		return(0);
	    }
	    if (kread(arFid, (char *)&r, sizeof(r))) {
		err = "can't read afs_rootFid from kernel";
		goto rfid_unavailable;
	    }
	    f = 1;
	    /* fall through */
	case 1:
	    *rfid = 1;
	    if (vc->fid.Fid.Unique == r.Fid.Unique
	    &&  vc->fid.Fid.Vnode == r.Fid.Vnode
	    &&  vc->fid.Fid.Volume == r.Fid.Volume
	    &&  vc->fid.Cell == r.Cell)
		return(1);
	}
	*rfid = 0;
	return(0);
}


/*
 * readafsnode() - read AFS node
 */

int
readafsnode(va, v, an)
	KA_T va;			/* kernel vnode address */
	struct vnode *v;		/* vnode buffer pointer */
	struct afsnode *an;		/* afsnode recipient */
{
	char *cp, tbuf[32];
	KA_T ka;
	int len, rfid, vols;
	struct vcache *vc;
	struct volume *vp;

	cp = ((char *)v + sizeof(struct vnode));
	ka = (KA_T)((char *)va + sizeof(struct vnode));
	len = sizeof(struct vcache) - sizeof(struct vnode);
	if (kread(ka, cp, len)) {
	    (void) snpf(Namech, Namechl,
		"vnode at %s: can't read vcache remainder from %s",
		print_kptr(va, tbuf, sizeof(tbuf)),
		print_kptr(ka, (char *)NULL, 0));
	    enter_nm(Namech);
	    return(1);
	}
	vc = (struct vcache *)v;
	if (!AFSdevStat)
	    (void) readmnt();
	an->dev = AFSdevStat ? AFSdev: 0;
	an->size = (unsigned long)vc->m.Length;
	an->nlink = (long)vc->m.LinkCount;
	an->nlink_st = 1;
/*
 * Manufacture the "inode" number.
 */
	if (vc->mvstat == 2) {
	    if ((vp = getvolume(&vc->fid, &vols))) {
		an->inode = (INODETYPE)((vp->mtpoint.Fid.Vnode
			  +		(vp->mtpoint.Fid.Volume << 16))
			  & 0x7fffffff);
		if (an->inode == (INODETYPE)0) {
		    if (is_rootFid(vc, &rfid))
			an->ino_st = 1;
		    else if (rfid) {
			an->inode = (INODETYPE)2;
			an->ino_st = 1;
		    } else
			an->ino_st = 0;
		} else
		    an->ino_st = 1;
	    } else {
		if (vols) {
		    an->inode = (INODETYPE)2;
		    an->ino_st = 1;
		} else {
		    if (v->v_flag & VROOT) {
			an->inode = (INODETYPE)0;
			an->ino_st = 1;
		    } else
			an->ino_st = 0;
		}
	    }
	} else {
	    an->inode = (INODETYPE)((vc->fid.Fid.Vnode
		      +			(vc->fid.Fid.Volume << 16))
		      & 0x7fffffff);
	    an->ino_st = 1;
	}
	return(0);
}
#endif	/* defined(HAS_AFS) */
