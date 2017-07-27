/*
 * dfile.c - AIX file processing functions for lsof
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
static char *rcsid = "$Id: dfile.c,v 1.13 2005/08/08 19:46:38 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local structures
 */

struct hsfile {
	struct sfile *s;		/* the Sfile table address */
	struct hsfile *next;		/* the next hash bucket entry */
};


/*
 * Local static variables
 */

static struct hsfile *HbyFdi =		/* hash by file buckets */
	(struct hsfile *)NULL;
static int HbyFdiCt = 0;		/* HbyFdi entry count */
static struct hsfile *HbyFrd =		/* hash by file raw device buckets */
	(struct hsfile *)NULL;
static int HbyFrdCt = 0;		/* HbyFrd entry count */
static struct hsfile *HbyFsd =		/* hash by file system buckets */
	(struct hsfile *)NULL;
static int HbyFsdCt = 0;		/* HbyFsd entry count */
static struct hsfile *HbyMPC =		/* hash by MPC file buckets */
	(struct hsfile *)NULL;
static int HbyMPCCt = 0;		/* HbyMPC entry count */
static struct hsfile *HbyNm =		/* hash by name buckets */
	(struct hsfile *)NULL;
static int HbyNmCt = 0;			/* HbyNm entry count */


/*
 * Local definitions
 */

#define	SFDIHASH	4094		/* Sfile hash by (device,inode) number
					 * pair bucket count (power of 2!) */
#define	SFFSHASH	128		/* Sfile hash by file system device
					 * number bucket count (power of 2!) */
#define SFHASHDEVINO(maj, min, ino, mod) ((int)(((int)((((int)(maj+1))*((int)((min+1))))+ino)*31415)&(mod-1)))
					/* hash for Sfile by major device,
					 * minor device, and inode, modulo m
					 * (m must be a power of 2) */
#define	SFMPCHASH	1024		/* Sfile hash by MPC device number */
#define	SFNMHASH	4096		/* Sfile hash by name bucket count
					   (power of 2!) */
#define	SFRDHASH	1024		/* Sfile hash by raw device number
					 * bucket count (power of 2!) */
#define SFHASHRDEVI(maj, min, rmaj, rmin, ino, mod) ((int)(((int)((((int)(maj+1))*((int)((min+1))))+((int)(rmaj+1)*(int)(rmin+1))+ino)*31415)&(mod-1)))
					/* hash for Sfile by major device,
					 * minor device, major raw device,
					 * minor raw device, and inode, modulo
					 * mod (mod must be a power of 2) */


/*
 * hashSfile() - hash Sfile entries for use in is_file_named() searches
 */

void
hashSfile()
{
	static int hs = 0;
	int i;
	struct sfile *s;
	struct hsfile *sh, *sn;
/*
 * Do nothing if there are no file search arguments cached or if the
 * hashes have already been constructed.
 */
	if (!Sfile || hs)
	    return;
/*
 * Allocate hash buckets by (device,inode), file system device, MPC device,
 * and file name.
 */
	if (!(HbyFdi = (struct hsfile *)calloc((MALLOC_S)SFDIHASH,
					       sizeof(struct hsfile))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate space for %d (dev,ino) hash buckets\n",
		Pn, SFDIHASH);
	    Exit(1);
	}
	if (!(HbyFrd = (struct hsfile *)calloc((MALLOC_S)SFRDHASH,
					       sizeof(struct hsfile))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate space for %d rdev hash buckets\n",
		Pn, SFRDHASH);
	    Exit(1);
	}
	if (!(HbyFsd = (struct hsfile *)calloc((MALLOC_S)SFFSHASH,
					       sizeof(struct hsfile))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate space for %d file sys hash buckets\n",
		Pn, SFFSHASH);
	    Exit(1);
	}
	if (!(HbyMPC = (struct hsfile *)calloc((MALLOC_S)SFMPCHASH,
					       sizeof(struct hsfile))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate space for %d MPC file hash buckets\n",
		Pn, SFMPCHASH);
	    Exit(1);
	}
	if (!(HbyNm = (struct hsfile *)calloc((MALLOC_S)SFNMHASH,
					      sizeof(struct hsfile))))
	{
	    (void) fprintf(stderr,
		"%s: can't allocate space for %d name hash buckets\n",
		Pn, SFNMHASH);
	    Exit(1);
	}
	hs++;
/*
 * Scan the Sfile chain, building file, file system, MPC file, and file
 * name hash bucket chains.
 */
	for (s = Sfile; s; s = s->next) {
	    for (i = 0; i < 4; i++) {
		if (i == 0) {
		    if (!s->aname)
			continue;
		    sh = &HbyNm[hashbyname(s->aname, SFNMHASH)];
		    HbyNmCt++;
		} else if (i == 1) {
		    if (s->type) {
			sh = &HbyFdi[SFHASHDEVINO(GET_MAJ_DEV(s->dev),
						  GET_MIN_DEV(s->dev),
						  s->i,
						  SFDIHASH)];
			HbyFdiCt++;
		    } else {
			sh = &HbyFsd[SFHASHDEVINO(GET_MAJ_DEV(s->dev),
						  GET_MIN_DEV(s->dev),
						  0,
						  SFFSHASH)];
			HbyFsdCt++;
		    }
		} else if (i == 2) {
		    if (s->type && (s->ch < 0) && (s->mode == S_IFCHR))
		    {
			sh = &HbyMPC[SFHASHDEVINO(GET_MAJ_DEV(s->dev),
						  GET_MIN_DEV(s->dev),
						  0,
						  SFMPCHASH)];
			HbyMPCCt++;
		    } else
			continue;
		} else if (i == 3) {
		    if (s->type
		    &&  (((s->mode == S_IFCHR) && (s->ch < 0))
		    ||   ((s->mode == S_IFBLK))))
		    {
			sh = &HbyFrd[SFHASHRDEVI(GET_MAJ_DEV(s->dev),
						 GET_MIN_DEV(s->dev),
						 GET_MAJ_DEV(s->rdev),
						 GET_MIN_DEV(s->rdev),
						 s->i,
						 SFRDHASH)];
			HbyFrdCt++;
		    } else
			continue;
		}
		if (!sh->s) {
		    sh->s = s;
		    sh->next = (struct hsfile *)NULL;
		    continue;
		} else {
		    if (!(sn = (struct hsfile *)malloc(
				(MALLOC_S)sizeof(struct hsfile))))
		    {
			(void) fprintf(stderr,
			    "%s: can't allocate hsfile bucket for: %s\n",
			    Pn, s->aname);
			Exit(1);
		    }
		    sn->s = s;
		    sn->next = sh->next;
		    sh->next = sn;
		}
	    }
	}
}


/*
 * is_file_named() - is file named?
 */

int
is_file_named(p, ty, ch, ic)
	char *p;			/* path name; NULL = search by device
					 * and inode (from *Lf) */
	enum vtype ty;			/* vnode type */
	chan_t ch;			/* gnode channel */
	int ic;				/* is clone file (4.1.4 and above) */
{
	int dmaj, dmin, maj, min, rmaj, rmin;
	static int dsplit = 0;
	char *ep;
	int f = 0;
	struct sfile *s;
	struct hsfile *sh;
	size_t sz;
/*
 * Split the device numbers into their major and minor numbers.
 *
 * THis is necessitated by 64 bit AIX architectures, which store two different
 * types of device numbers in 64 bit dev_t's.  The two types can't be compared
 * directly, but must be compared by extracting their major and minor numbers
 * and comparing them.
 */
	readdev(0);
	if (!dsplit) {
	    dmaj = GET_MAJ_DEV(DevDev);
	    dmin = GET_MIN_DEV(DevDev);
	    dsplit = 1;
	}
	if (Lf->dev_def) {
	    maj = GET_MAJ_DEV(Lf->dev);
	    min = GET_MIN_DEV(Lf->dev);
	}
	if (Lf->rdev_def) {
	   rmaj = GET_MAJ_DEV(Lf->rdev);
	   rmin = GET_MIN_DEV(Lf->rdev);
	}

#if	AIXV>=4140
/*
 * Check for a clone match.
 */
	if (ic
	&&  HbyFdiCt
	&&  CloneMaj >= 0
	&&  (Lf->dev_def && (maj = dmaj) && (min == dmin))
	&&  Lf->rdev_def
	&&  (Lf->inp_ty == 1 || Lf->inp_ty == 3))
	{
	    for (sh=&HbyFdi[SFHASHDEVINO(CloneMaj, rmaj, Lf->inode, SFDIHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s)
		&&  (GET_MAJ_DEV(s->rdev) == CloneMaj)
		&&  (GET_MIN_DEV(s->rdev) == rmaj)
		&&  (s->i == Lf->inode))
		{
		    f = 3;
		    break;
		}
	    }
	}
#endif	/* AIXV>=4140 */

/*
 * Check for a path name match, as requested.
 */
	if (!f && p && HbyNmCt) {
	    for (sh = &HbyNm[hashbyname(p, SFNMHASH)]; sh; sh = sh->next) {
		if ((s = sh->s) && strcmp(p, s->aname) == 0) {
		    f = 2;
		    break;
		}
	    }
	}
/*
 * Check for a regular AIX multiplexed file, matching the channel if
 * it was supplied by the caller.
 */
	if (!f && HbyMPCCt && ty == VMPC
	&&  (Lf->dev_def && (maj == dmaj) && (min == dmin))
	&&  Lf->rdev_def)
	{
	    for (sh = &HbyMPC[SFHASHDEVINO(rmaj, rmin, 0, SFMPCHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s)
		&&  (GET_MAJ_DEV(s->dev) == rmaj)
		&&  (GET_MIN_DEV(s->dev) == rmin)
		&&  (s->ch < 0 || ch == s->ch)) {
		    f = 1;
		    break;
		}
	    }
	}
/*
 * Check for a regular file.
 */
	if (!f && HbyFdiCt && Lf->dev_def
	&&  (Lf->inp_ty == 1 || Lf->inp_ty == 3))
	{
	    for (sh = &HbyFdi[SFHASHDEVINO(maj, min, Lf->inode, SFDIHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s)
		&&  (maj == GET_MAJ_DEV(s->dev))
		&&  (min == GET_MIN_DEV(s->dev))
		&&  (Lf->inode == s->i))
		{
		    f = 1;
		    break;
		}
	    }
	}
/*
 * Check for a file system.
 */
	if (!f && HbyFsdCt && Lf->dev_def) {
	    for (sh = &HbyFsd[SFHASHDEVINO(maj, min, 0, SFFSHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s)
		&&  (maj == GET_MAJ_DEV(s->dev))
		&&  (min == GET_MIN_DEV(s->dev))
		) {
		    f = 1;
		    break;
		}
	    }
	}
/*
 * Check for a character or block device file.
 */
	if (!f && HbyFrdCt
	&&  ((ty == VCHR) || (ty == VBLK))
	&&  (Lf->dev_def && (maj == dmaj) && (min == dmin))
	&&  Lf->rdev_def
	&& (Lf->inp_ty == 1 || Lf->inp_ty == 3))
	{
	    for (sh = &HbyFrd[SFHASHRDEVI(maj, min, rmaj, rmin,
					  Lf->inode, SFRDHASH)];
		 sh;
		 sh = sh->next)
	    {
		if ((s = sh->s)
		&&  (GET_MAJ_DEV(s->rdev) == rmaj)
		&&  (GET_MIN_DEV(s->rdev) == rmin)
		&&  (((ty == VCHR) && (s->mode == S_IFCHR) && (s->ch < 0))
		||   ((ty == VBLK) && (s->mode == S_IFBLK))))
		{
		    f = 1;
		    break;
		}
	    }
	}
/*
 * Convert the name if a match occurred.
 */
	if (f) {
	    if (f == 2) {
		(void) snpf(Namech, Namechl, "%s", p);

#if	AIXV>=4140
	    } else if (f == 3 && ClonePtc >= 0 && (maj == ClonePtc)) {
		(void) snpf(Namech, Namechl, "%s/%d", s->name, min);

#endif	/* AIXV>=4140 */

	    } else if (s->type) {

	    /*
	     * If the search argument isn't a file system, propagate it
	     * to Namech[]; otherwise, let printname() compose the name.
	     */
		(void) snpf(Namech, Namechl, "%s", s->name);
		if (ty == VMPC && s->ch < 0) {
		    ep = endnm(&sz);
		    (void) snpf(ep, sz, "/%d", ch);
		}
		if (s->devnm) {
		    ep = endnm(&sz);
		    (void) snpf(ep, sz, " (%s)", s->devnm);
		}
	    }
	    s->f = 1;
	    return(1);
	}
	return(0);
}


/*
 * print_dev() - print device
 */

char *
print_dev(lf, dev)
	struct lfile *lf;		/* file whose device to be printed */
	dev_t *dev;			/* pointer to device to be printed */

{
	static char buf[128];
	int maj = GET_MAJ_DEV(*dev);
	int min = GET_MIN_DEV(*dev);

#if	AIXV>=3200
	if (*dev & SDEV_REMOTE) {
	    (void) snpf(buf, sizeof(buf), "NFS,%d", (min & ~SDEV_REMOTE));
	    return(buf);
	}
#endif	/* AIXV>=3200 */

	(void) snpf(buf, sizeof(buf), "%d,%d", maj, min);
	return(buf);
}


/*
 * readvfs() - read vfs structure
 */

struct l_vfs *
readvfs(vn)
	struct vnode *vn;		/* vnode */
{
	struct gfs g;
	void *mp;
	char *s1, *s2;
	uint ul;
	struct vfs v;
	struct vmount *vm;
	struct l_vfs *vp;


	if (!vn->v_vfsp)
	    return((struct l_vfs *)NULL);
	for (vp = Lvfs; vp; vp = vp->next) {
	    if ((KA_T)vn->v_vfsp == vp->addr)
		return(vp);
	}
	if (!(vp = (struct l_vfs *)malloc((MALLOC_S)sizeof(struct l_vfs)))) {
	    (void) fprintf(stderr, "%s: PID %d, no space for vfs\n",
		Pn, Lp->pid);
	    Exit(1);
	}
	vp->dir = (char *)NULL;
	vp->fsname = (char *)NULL;
/*
 * Read the vfs structure.
 */
	if (kread((KA_T)vn->v_vfsp, (char *)&v, sizeof(v))) {

vfs_exit:
	    (void) free((FREE_P *)vp);
	    return((struct l_vfs *)NULL);
	}
/*
 * Locate AIX mount information.
 */
	if (!v.vfs_gfs || kread((KA_T)v.vfs_gfs, (char *)&g, sizeof(g)))
	    goto vfs_exit;
	if (!v.vfs_mdata
	||  kread((KA_T)((char *)v.vfs_mdata
		  + offsetof(struct vmount, vmt_length)),
		  (char *)&ul, sizeof(ul)))
	    goto vfs_exit;
	if (!(mp = (void *)malloc((MALLOC_S)ul))) {
	    (void) fprintf(stderr, "%s: PID %d, no space for mount data\n",
		Pn, Lp->pid);
	    Exit(1);
	}
	if (kread((KA_T)v.vfs_mdata, (char *)mp, (int)ul)) {
	    (void) free((FREE_P *)mp);
	    goto vfs_exit;
	}
	vm = (struct vmount *)mp;
	vp->vmt_flags = vm->vmt_flags;
	vp->vmt_gfstype = vm->vmt_gfstype;

#if	AIXV>=3200
	if ((vp->vmt_flags & MNT_REMOTE)

# if	defined(HAS_SANFS) && defined(MNT_SANFS)
	&& (vp->vmt_gfstype != MNT_SANFS)
# endif	/* defined(HAS_SANFS) && defined(MNT_SANFS) */

	) {
	    vp->dev = 0x80000000 | vm->vmt_vfsnumber;
# if	AIXA>=1
	    vp->dev |= 0x8000000000000000;
# endif	/* AIXA>=1 */
	} else
#endif	/* AIXV>=3200 */

#if	defined(HAS_AFS)
	    if (vm->vmt_gfstype == MNT_AFS)
		vp->dev = AFSDEV;
	    else
#endif	/* defined(HAS_AFS) */

#if	AIXA>1
	if (vm->vmt_gfstype == MNT_PROCFS) {

	/*
	 * !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!
	 *
	 * The following *hack* is required to make the vmount structure's
	 * device number match what stat(2) errnoneously returns in ia64
	 * AIX >= 5.
	 *
	 * REMOVE THIS CODE WHEN STAT(2) IS FIXED!!!
	 */
		vp->dev = (dev_t)(vm->vmt_fsid.fsid_dev & 0x7fffffffffffffff);
	/*
	 * !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!   !!!DEBUG!!!
	 */

	}
	else
#endif	/* AIXA>1 */

		vp->dev = (dev_t)vm->vmt_fsid.fsid_dev;
	if ((s1 = vmt2dataptr(vm, VMT_STUB))) {
	    if (!(vp->dir = mkstrcpy(s1, (MALLOC_S *)NULL))) {

readvfs_aix1:
		(void) fprintf(stderr, "%s: PID %d, readvfs, no space\n",
		    Pn, Lp->pid);
		Exit(1);
	    }
	} else
	    vp->dir = (char *)NULL;
	s1 = vmt2dataptr(vm, VMT_HOST);
	if (!(s2 = vmt2dataptr(vm, VMT_OBJECT)) || *s1 == '\0')
	    s2 = g.gfs_name;
	if (!s1 && !s2)
	    vp->fsname = (char *)NULL;
	else {
	    if (vm->vmt_flags & MNT_REMOTE) {
		if (!(vp->fsname = mkstrcat(s1 ? s1 : "",
					    -1,
					   (s1 && *s1) ? ":" : "",
					   -1, s2, -1,
					   (MALLOC_S *)NULL)))
		    goto readvfs_aix1;
	    } else {
		if (!(vp->fsname = mkstrcpy(s2, (MALLOC_S *)NULL)))
		    goto readvfs_aix1;
	    }
	}
	(void) free((FREE_P *)mp);
	vp->next = Lvfs;
	vp->addr = (KA_T)vn->v_vfsp;

#if	defined(HAS_AFS)
	if (!AFSVfsp && vm->vmt_gfstype == MNT_AFS)
	    AFSVfsp = (KA_T)vn->v_vfsp;
#endif	/* defined(HAS_AFS) */

	Lvfs = vp;
	return(vp);
}
