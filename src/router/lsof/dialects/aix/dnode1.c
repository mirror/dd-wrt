/*
 * dnode1.c - AIX AFS support
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
static char *rcsid = "$Id: dnode1.c,v 1.10 2005/08/08 19:46:38 abe Exp $";
#endif


#if	defined(HAS_AFS)
#include "lsof.h"


/*
 * This is an emulation of the afs_rwlock_t definition that appears in
 * the AFS sources in afs/lock.h.
 */

struct afs_lock {

#  if	HAS_AFS<304
	unsigned long d1[4];
#  else	/* HAS_AFS>=304 */
	unsigned char d1[2];
	unsigned short d2[3];
	struct timeval d3;
	unsigned int d4[3];
#  endif	/* HAS_AFS<304 */

};
typedef struct afs_lock afs_lock_t;
typedef struct afs_lock afs_rwlock_t;

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
	return((struct vnode *)malloc((MALLOC_S)sizeof(struct vcache)));
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
	struct volume *vp;
	static int w = 0;

	if (!ka) {
	    if ((ka = (KA_T)AFSnl[X_AFS_VOL].n_value) == (KA_T)0) {
		if (!w && !Fwarn) {
		    (void) fprintf(stderr,
			"%s: WARNING: no kernel address for: %s\n",
			Pn, AFSnl[X_AFS_VOL]._n._n_name);
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
	    vp = v.next;
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
	struct vmount vm;
	struct vfs v;
/*
 * If this vnode has a v_data pointer, then it probably isn't an AFS vnode;
 * return FALSE.
 *
 * If the vfs struct address of /afs is known and this vnode's v_vfsp matches
 * it, return TRUE.
 *
 * Read this vnode's vfs structure and its mount data.  If the gfs type isn't
 * AFS, return FALSE.  If it is, save the vnode's v_vfsp as AFSVfsp and return
 * TRUE.
 */
	if (AFSVfsp && !vp->v_data && (KA_T)vp->v_vfsp == AFSVfsp)
	    return(1);
	if (vp->v_data || !vp->v_vfsp)
	    return(0);
	if (kread((KA_T)vp->v_vfsp, (char *)&v, sizeof(v)))
	    return(0);
	if (!v.vfs_mdata
	||  kread((KA_T)v.vfs_mdata, (char *)&vm, sizeof(vm)))
	    return(0);
	if (vm.vmt_gfstype != MNT_AFS)
	    return(0);
	AFSVfsp = (KA_T)vp->v_vfsp;
	return(1);
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
	struct vcache *vc;		/* vcache entry */
	int *rfid;			/* root file ID pointer status return */
{
	int err;
	static int f = 0;		/* rootFID structure status:
					 *     -1 = unavailable
					 *	0 = not yet accessed
					 *	1 = available */
	static struct VenusFid r;
	static int w = 0;

	switch (f) {
	case -1:
	    if (vc->v.v_flag & V_ROOT) {
		*rfid = 1;
		return(1);
	    }
	    *rfid = 0;
	    return(0);
	case 0:
	    if (!AFSnl[X_AFS_FID].n_value) {
		err = 1;

rfid_unavailable:

		if (!w && !Fwarn) {
		    (void) fprintf(stderr,
			"%s: WARNING: %s: %s\n", Pn,
			err ? "no kernel address" : "can't read from kernel",
			AFSnl[X_AFS_VOL]._n._n_name);
		    (void) fprintf(stderr,
			"      This may hamper AFS node number reporting.\n");
		    w = 1;
		}
		f = -1;
		if (vc->v.v_flag & V_ROOT) {
		    *rfid = 1;
		    return(1);
		}
		*rfid = 0;
		return(0);
	    }
	    if (kread((KA_T)AFSnl[X_AFS_FID].n_value, (char *)&r, sizeof(r))) {
		err = 0;
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
	an->dev = AFSDEV;
	an->size = (unsigned long)vc->m.Length;
	an->nlink = (long)vc->m.LinkCount;
	an->nlink_st  = 1;
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
		    if (v->v_flag & V_ROOT) {
			an->inode = (INODETYPE)0;
			an->ino_st = 1;
		    } else
			an->ino_st = 0;
		}
	    }
	} else {
	    an->inode = (INODETYPE)((vc->fid.Fid.Vnode
		      +		    (vc->fid.Fid.Volume << 16))
		      & 0x7fffffff);
	    an->ino_st = 1;
	}
	return(0);
}
#endif	/* defined(HAS_AFS) */
