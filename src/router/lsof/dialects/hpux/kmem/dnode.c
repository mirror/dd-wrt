/*
 * dnode.c - /dev/kmem-based HP-UX node functions for lsof
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
static char *rcsid = "$Id: dnode.c,v 1.21 2007/04/24 16:25:30 abe Exp $";
#endif

#if	defined(HPUXKERNBITS) && HPUXKERNBITS>=64
#define _INO_T
typedef int ino_t;
#define _TIME_T
typedef int time_t;
#endif	/* defined(HPUXKERNBITS) && HPUXKERNBITS>=64 */

#include "lsof.h"
#include <sys/inode.h>

#if	HPUXV>=900
_PROTOTYPE(static void enter_nma,(char *b));
_PROTOTYPE(static int islocked,(KA_T lp));
#endif	/* HPUXV>=900 */

_PROTOTYPE(static int getnodety,(struct vnode *v));
_PROTOTYPE(static int readinode,(KA_T ia, struct inode *i));
_PROTOTYPE(static int read_nmn,(KA_T na, KA_T ia, struct mvfsnode *m));


#if	HPUXV>=900
/*
 * enter_nma() - enter NAME column addition
 */

static void
enter_nma(b)
	char *b;			/* addition buffer */
{
	if (Lf->nma)
	    return;
	if (strlen(b) < 1)
	    return;
	Lf->nma = mkstrcpy(b, (MALLOC_S *)NULL);
}


/*
 * islocked() - is node locked?
 */

static int
islocked(lp)
	KA_T lp;			/* local locklist struct pointer */
{
	static int ety = -1;
	static unsigned int ei = 0;
	static SZOFFTYPE el = 0;
	int l;
	struct locklist ll;
	KA_T llf, llp;

	if (!(llf = (KA_T)lp))
	    return((int)' ');
	llp = llf;
/*
 * Compute the end test value the first time through.
 */

	if (ety == -1) {

# if	HPUXV<1020
	    ety = 0;
	    ei = 0x7fffffff;
# else	/* HPUXV>=1020 */
	    if (sizeof(ll.ll_end) == 4) {
		ety = 0;
		ei = 0x80000000;
	    } else {
		ety = 1;
		el = 0x10000000000ll;
	    }
# endif	/* HPUXV<1020 */

	}

/*
 * Search the locklist chain for this process.
 */
	do {
	    if (kread(llp, (char *)&ll, sizeof(ll)))
		return((int)' ');

#if	!defined(L_REMOTE)
#define	L_REMOTE	0x1		/* from HP-UX 9.01 */
#endif	/* !defined(L_REMOTE) */

# if	HPUXV<1010
	    if (ll.ll_flags & L_REMOTE || ll.ll_proc != (KA_T)Kpa)
# else	/* HPUXV>=1010 */
	    if (ll.ll_flags & L_REMOTE || (KA_T)ll.ll_kthreadp != Ktp)
# endif	/* HPUXV<1010 */

		continue;
	    l = 0;
	    if (ll.ll_start == 0) {
		switch (ety) {
		case 0:
		    if (ll.ll_end == ei)
			l = 1;
		break;
		case 1:
		    if (ll.ll_end == el)
			l = 1;
		break;
		}
	    }
	    if (ll.ll_type == F_WRLCK)
		return((int)(l ? 'W' : 'w'));
	    else if (ll.ll_type == F_RDLCK)
		return((int)(l ? 'R' : 'r'));
	    return((int)' ');
	}

# if	HPUXV<1010
	while ((llp = (KA_T)ll.ll_link) && llp != llf);
# else	/* HPUXV>=1010 */
	while ((llp = (KA_T)ll.ll_fwd) && llp != llf);
# endif	/* HPUXV<1010 */

	return((int)' ');
}
#endif	/* HPUXV>=900 */


/*
 * getnodety() - get node type
 */

static int
getnodety(v)
	struct vnode *v;		/* local vnode copy */
{

#if	defined(HAS_AFS)
	static int afs = 0;		/* AFS test status: -1 = no AFS
					 *		     0 = not tested
					 *		     1 = AFS present */
	struct afsnode an;
#endif	/* defined(HAS_AFS) */

	static int ft = 1;
	static KA_T avops;
	static KA_T cvops;
	static KA_T fvops;
	static KA_T mvops;
	static KA_T nvops;
	static KA_T nvops3;
	static KA_T nv3ops;
	static KA_T pvops;
	static KA_T svops;
	static KA_T uvops;
	static KA_T vvops;
/*
 * Do first-time only operations.
 */
	if (ft) {
	    if (get_Nl_value("avops", Drive_Nl, &avops) < 0)
		avops = (unsigned long)0;
	    if (get_Nl_value("cvops", Drive_Nl, &cvops) < 0)
		cvops = (unsigned long)0;
	    if (get_Nl_value("fvops", Drive_Nl, &fvops) < 0)
		fvops = (unsigned long)0;
	    if (get_Nl_value("mvops", Drive_Nl, &mvops) < 0)
		mvops = (unsigned long)0;
	    if (get_Nl_value("nvops", Drive_Nl, &nvops) < 0)
		nvops = (unsigned long)0;
	    if (get_Nl_value("nvops3", Drive_Nl, &nvops3) < 0)
		nvops3 = (unsigned long)0;
	    if (get_Nl_value("nv3ops", Drive_Nl, &nv3ops) < 0)
		nv3ops = (unsigned long)0;
	    if (get_Nl_value("pvops", Drive_Nl, &pvops) < 0)
		pvops = (unsigned long)0;
	    if (get_Nl_value("svops", Drive_Nl, &svops) < 0)
		svops = (unsigned long)0;
	    if (get_Nl_value("uvops", Drive_Nl, &uvops) < 0)
		uvops = (unsigned long)0;
	    if (get_Nl_value("vvops", Drive_Nl, &vvops) < 0)
		vvops = (unsigned long)0;
	    ft = 0;
	}
/*
 * Determine the vnode type.
 */
	if (uvops && uvops == (unsigned long)v->v_op)
	    return(N_REGLR);
	else if (nvops && nvops == (unsigned long)v->v_op)
	    return(N_NFS);
	else if (nvops3 && nvops3 == (unsigned long)v->v_op)
	    return(N_NFS);
	else if (nv3ops && nv3ops == (unsigned long)v->v_op)
	    return(N_NFS);
	else if (mvops && mvops == (unsigned long)v->v_op)
	    return(N_MVFS);

#if	defined(HASVXFS)
	else if (vvops && vvops == (unsigned long)v->v_op)
	    return(N_VXFS);
#endif	/* defined(HASVXFS) */

#if	HPUXV>=1000
	else if (cvops && cvops == (unsigned long)v->v_op)
	    return(N_CDFS);
	else if (fvops && fvops == (unsigned long)v->v_op)
	    return(N_FIFO);
	else if (pvops && pvops == (unsigned long)v->v_op)
	    return(N_PIPE);
	else if (svops && svops == (unsigned long)v->v_op)
	    return(N_SPEC);
#else	/* HPUXV<1000 */
	else if (v->v_type == VFIFO)
	    return(N_FIFO);
#endif	/* HPUXV<1000 */

#if	defined(HAS_AFS)
	/*
	 * Caution: this AFS test should be the last one.
	 */
	
	else if (avops) {
	    if (avops == (unsigned long)v->v_op)
		return(N_AFS);
	    else {

unknown_v_op:
		(void) snpf(Namech, Namechl,
		    "unknown file system type; v_op: %s",
		    print_kptr((KA_T)v->v_op, (char *)NULL, 0));
		enter_nm(Namech);
		return(-1);
	    }
	} else if (v->v_data || !v->v_vfsp)
	    goto unknown_v_op;
	else {
	    switch (afs) {
	    case -1:
		goto unknown_v_op;
	    case 0:
		if (!hasAFS(v)) {
		    afs = -1;
		    goto unknown_v_op;
		}
		afs = 1;
		return(N_AFS);
		break;
	    case 1:
		if (v->v_vfsp == AFSVfsp)
		    return(N_AFS);
		else
		    goto unknown_v_op;
	    }
	}
#else	/* !defined(HAS_AFS) */
	else {
	    (void) snpf(Namech, Namechl,
		"unknown file system type; v_op: %s",
		print_kptr((KA_T)v->v_op, (char *)NULL, 0));
	    enter_nm(Namech);
	    return(-1);
	}
#endif	/* defined(HAS_AFS) */

}


/*
 * process_node() - process vnode
 */

void
process_node(va)
	KA_T va;			/* vnode kernel space address */

{

#if	defined(HAS_AFS)
	struct afsnode an;
#endif	/* defined(HAS_AFS) */

	dev_t dev, rdev;
	int devs = 0;
	struct inode i;
	int ins = 0;
	struct mvfsnode m;
	struct rnode r;
	int rdevs = 0;
	int rns = 0;
	char tbuf[32], *ty;
	enum vtype type;
	static struct vnode *v = (struct vnode *)NULL;
	struct l_vfs *vfs;
	int vty;

#if	HPUXV>=900
	char fb[128];
	int fns = 0;
	int rp, sz, wp;
	struct vnode rv;
	struct snode s;
#endif	/* HPUXV>=900 */

#if	HPUXV>=1000
	struct cdnode c;
	struct fifonode f;
	struct vattr vat;
	int vats = 0;
#endif	/* HPUXV>=1000 */

/*
 * Read the vnode.
 */
	if ( ! va) {
	    enter_nm("no vnode address");
	    return;
	}
	if (!v) {

	/*
	 * Allocate space for the vnode or AFS vcache structure.
	 */

#if	defined(HAS_AFS)
	    v = alloc_vcache();
#else	/* !defined(HAS_AFS) */
	    v = (struct vnode *)malloc(sizeof(struct vnode));
#endif	/* defined(HAS_AFS) */

	    if (!v) {
		(void) fprintf(stderr, "%s: can't allocate %s space\n", Pn,

#if	defined(HAS_AFS)
		    "vcache"
#else	/* !defined(HAS_AFS) */
		    "vnode"
#endif	/* defined(HAS_AFS) */

		);
		Exit(1);
	    }
	}
	if (readvnode(va, v)) {
	    enter_nm(Namech);
	    return;
	}

#if	defined(HASNCACHE)
	Lf->na = va;
#endif	/* defined(HASNCACHE) */

#if	defined(HASFSTRUCT)
	Lf->fna = va;
	Lf->fsv |= FSV_NI;
#endif	/* defined(HASFSTRUCT) */

/*
 * Get the primary vnode type.
 */
	vty = getnodety(v);
	if (vty == -1)
	    return;
	Ntype = vty;
/*
 * Determine lock type.
 */

#if	HPUXV<900
	if (v->v_shlockc || v->v_exlockc) {
	    if (v->v_shlockc && v->v_exlockc)
		Lf->lock = 'u';
	    else if (v->v_shlockc)
		Lf->lock = 'R';
	    else
		Lf->lock = 'W';
	}
#else	/* HPUXV>900 */
# if	HPUXV>=1000
	Lf->lock = (char)islocked((KA_T)v->v_locklist);
# endif	/* HPUXV>=1000 */
#endif	/* HPUXV<900 */

/*
 * Establish the local virtual file system structure.
 */
	if (!v->v_vfsp)
	    vfs = (struct l_vfs *)NULL;
	else if (!(vfs = readvfs(v))) {
	    (void) snpf(Namech, Namechl, "can't read vfs for %s at %s",
		print_kptr(va, tbuf, sizeof(tbuf)),
		print_kptr((KA_T)v->v_vfsp, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
/*
 * Read the cdnode, fifonode, inode, rnode, snode, or vache struct.
 */
	switch (Ntype) {

#if	defined(HAS_AFS)
	case N_AFS:
	    if (readafsnode(va, v, &an))
		return;
	    break;
#endif	/* defined(HAS_AFS) */

#if	defined(HASVXFS)
	case N_VXFS:
	    if (!v->v_data || read_vxnode(v, vfs, &dev, &devs, &rdev, &rdevs)) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read vx_inode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    break;
#endif	/* defined(HASVXFS) */

#if	HPUXV>=1000
	case N_CDFS:
	    if (!v->v_data
	    ||  kread((KA_T)v->v_data, (char *)&c, sizeof(c))) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read cdnode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    break;
	case N_FIFO:
	case N_PIPE:
	    if (!v->v_data
	    ||  kread((KA_T)v->v_data, (char *)&f, sizeof(f))) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read fifonode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    fns = 1;
	    if (f.fn_vap
	    &&  kread((KA_T)f.fn_vap, (char *)&vat, sizeof(vat)) == 0)
		vats = 1;
	    break;
#endif	/* HPUXV>=1000 */

	case N_MVFS:
	    if (read_nmn(va, (KA_T)v->v_data, &m))
		return;
	    break;
	case N_NFS:
	    if (!v->v_data || readrnode((KA_T)v->v_data, &r)) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read rnode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)), 
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    rns = 1;
	    break;

#if	HPUXV>=1000
	case N_SPEC:
	    if ((v->v_type == VBLK) || (v->v_type == VCHR)) {
		if (!v->v_data || readsnode((KA_T)v->v_data, &s)) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read snode(%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		if (!s.s_realvp
		||  readvnode((KA_T)s.s_realvp, &rv)) {
		    (void) snpf(Namech, Namechl,
			"snode at %s: can't read real vnode (%s)",
			print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)s.s_realvp, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}

#if	defined(HASVXFS)
		if (getnodety(&rv) == N_VXFS) {
		    if (!rv.v_data
		    ||  read_vxnode(&rv, vfs, &dev, &devs, &rdev, &rdevs)) {
			(void) snpf(Namech, Namechl,
			    "vnode at %s: can't read vx_inode (%s)",
			    print_kptr(va, tbuf, sizeof(tbuf)),
			    print_kptr((KA_T)rv.v_data, (char *)NULL, 0));
			enter_nm(Namech);
			return;
		    }
		    Ntype = N_VXFS;
		    break;
		}
#endif	/* defined(HASVXFS) */

		if (!rv.v_data || readinode((KA_T)rv.v_data, &i)) {
		    (void) snpf(Namech, Namechl,
			"snode at %s: can't read inode (%s)",
			print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)rv.v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		ins = 1;
		break;
	    }
	    if (!v->v_data || readinode((KA_T)v->v_data, &i)) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read inode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    ins = 1;
	    break;
#endif	/* HPUXV>=1000 */

#if	HPUXV>=900 && HPUXV<1000
	case N_FIFO:
	    if (v->v_fstype == VNFS_FIFO) {
		if (!v->v_data || readsnode((KA_T)v->v_data, &s)) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read snode (%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		if (!s.s_realvp || readvnode((KA_T)s.s_realvp, &rv)) {
		    (void) snpf(Namech, Namechl,
			"snode at %s: can't read real vnode (%s)",
			print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)s.s_realvp, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		if (!rv.v_data || readrnode((KA_T)rv.v_data, &r)) {
		    (void) snpf(Namech, Namechl,
			"snode at %s: can't read real rnode (%s)",
			print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)s.s_realvp, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		rns = 1;
		break;
	    }
	    /* fall through */
#endif	/* HPUXV>=900 && HPUXV<1000 */

	case N_REGLR:
	default:
	    if (!v->v_data || readinode((KA_T)v->v_data, &i)) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read inode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    ins = 1;

#if	HPUXV>=900 && HPUXV<1000
	    if (v->v_type == VFIFO)
		Ntype = N_FIFO;
#endif	/* HPUXV>=900 && HPUXV<1000 */

	}

#if	HPUXV>=900 && HPUXV<1000
	Lf->lock = (char)islocked((KA_T)i.i_locklist);
#endif	/* HPUXV>=900 && HPUXV<1000 */

/*
 * Get device and type for printing.
 */
	switch (Ntype) {

#if	defined(HAS_AFS)
	case N_AFS:
	    dev = an.dev;
	    devs = 1;
	    break;
#endif	/* defined(HAS_AFS) */

	case N_MVFS:
	    if (vfs) {
		dev = vfs->dev;
		devs = 1;
	    }
	    break;
	case N_NFS:
	    dev = vfs ? vfs->dev : 0;
	    devs = 1;
	    break;

#if	HPUXV>=1000
	case N_CDFS:
	    dev = c.cd_dev;
	    devs = 1;
	    break;
	case N_FIFO:
	case N_PIPE:
	    if (vfs && vfs->fsname) {
		dev = vfs->dev;
		devs = 1;
	    } else if (vats && (dev_t)vat.va_fsid != NODEV) {
		dev = (dev_t)vat.va_fsid;
		devs = 1;
	    } else
		enter_dev_ch(print_kptr(va, (char *)NULL, 0));
	    break;
#endif	/* _HPUX>=1000 */

#if	defined(HASVXFS)
	case N_VXFS:
	    /* obtained via read_vxnode */
	    break;
#endif	/* defined(HASVXFS) */

	case N_SPEC:
	default:

#if	HPUXV>=800
	    if (vfs && vfs->fsname) {
		dev = vfs->dev;
		devs = 1;
	    } else if (ins) {
		dev = i.i_dev;
		devs = 1;
	    }
	    if ((v->v_type == VBLK) || (v->v_type == VCHR)) {
		rdev = v->v_rdev;
		rdevs = 1;
	    }
#else	/* HPUXV<800 */
	    if (ins) {
		dev = i.i_dev;
		devs = 1;
	    }
	    if ((v->v_type == VCHR) || (v->v_type == VBLK)) {
		rdev = v->v_rdev;
		rdevs = 1;
	    }
#endif	/* HPUXV>=800 */

	}
	type = v->v_type;
/*
 * Obtain the inode number.
 */
	switch (Ntype) {

#if	defined(HAS_AFS)
	case N_AFS:
	    if (an.ino_st) {
		Lf->inode = (INODETYPE)an.inode;
		Lf->inp_ty = 1;
	    }
	    break;
#endif	/* defined(HAS_AFS) */

	case N_MVFS:
	    Lf->inode = (INODETYPE)m.m_ino;
	    Lf->inp_ty = 1;
	    break;
	case N_NFS:

#if	HPUXV<1030
	    Lf->inode = (INODETYPE)r.r_nfsattr.na_nodeid;
#else	/* HPUXV>=1030 */
	    Lf->inode = (INODETYPE)r.r_attr.va_nodeid;
#endif	/* HPUXV<1030 */

	    Lf->inp_ty = 1;
	    break;

#if	HPUXV>=1000
	case N_CDFS:
	    Lf->inode = (INODETYPE)c.cd_num;
	    Lf->inp_ty = 1;
	    break;
	case N_FIFO:
	case N_PIPE:
	    if (vats) {
		Lf->inode = (INODETYPE)vat.va_nodeid;
		Lf->inp_ty = 1;
	    } else {
		Lf->inode = (INODETYPE)v->v_nodeid;
		Lf->inp_ty = 1;
	    }
	    break;
#endif	/* HPUXV>=1000 */

#if	defined(HASVXFS)
	case N_VXFS:
	    /* set in read_vxnode() */
	    break;
#endif	/* defined(HASVXFS) */

#if	HPUXV<1000
	case N_FIFO:

# if	HPUXV>=900
	    if (rns) {
		Lf->inode = (INODETYPE)r.r_nfsattr.na_nodeid;
		Lf->inp_ty = 1;
		break;
	    }
# endif	/* HPUXV>=900 */
	    /* fall through */

#endif	/* HPUXV<1000 */

	case N_BLK:
	case N_REGLR:
	case N_SPEC:
	    if (ins) {
		Lf->inode = (INODETYPE)i.i_number;
		Lf->inp_ty = 1;
	    }
	}

#if	HPUXV>=1030
/*
 * Check for an HP-UX 10.30 and above stream.
 */
	if (v->v_stream) {
	    KA_T ip, pcb;
	    char *pn = (char *)NULL;

	    Lf->dev = dev;
	    Lf->dev_def = devs;
	    Lf->rdev = rdev;
	    Lf->rdev_def = rdevs;
	    if (read_mi((KA_T)v->v_stream, &ip, &pcb, &pn))
		return;
	    if (ip && pcb) {
		process_stream_sock(ip, pcb, pn, type);
		return;
	    }
	    Lf->is_stream = 1;
	}
#endif	/* HPUXV>=1030 */

/*
 * Obtain the file size.
 */
	if (Foffset)
	    Lf->off_def = 1;
	else {
	    switch (Ntype) {

#if	defined(HAS_AFS)
	    case N_AFS:
		Lf->sz = (SZOFFTYPE)an.size;
		Lf->sz_def = 1;
		break;
#endif	/* defined(HAS_AFS) */

#if	HPUXV>=1000
	    case N_CDFS:
		Lf->sz = (SZOFFTYPE)c.cd_cdc.cdc_size;
		Lf->sz_def = 1;
		break;
	    case N_PIPE:
		if (vats) {
		    Lf->sz = (SZOFFTYPE)vat.va_size;
		    Lf->sz_def = 1;
		}
		break;
#endif	/* HPUXV>=1000 */

#if	HPUXV>=900
	    case N_FIFO:

# if	HPUXV<1000
		if (ins) {
		    rp = i.i_frptr;
		    sz = (int)i.i_fifosize;
		    wp = i.i_fwptr;
		} else if (rns)
		    Lf->sz = (SZOFFTYPE)r.r_nfsattr.na_size;
# else	/* HPUXV>=1000 */
		if (fns) {
		    rp = f.fn_rptr;
		    sz = f.fn_size;
		    wp = f.fn_wptr;
		}
# endif	/* HPUXV<1000 */

		if (Fsize || (Lf->access != 'r' && Lf->access != 'w')) {
		    if (fns || ins) {
			(void) snpf(fb, sizeof(fb), "rd=%#x; wr=%#x", rp, wp);
			(void) enter_nma(fb);
		    }
		    if (fns || ins || rns) {
			Lf->sz = (SZOFFTYPE)sz;
			Lf->sz_def = 1;
		    }
		    break;
		}
		if (fns || ins) {
		    Lf->off = (unsigned long)((Lf->access == 'r') ? rp
								  : wp);
		    (void) snpf(fb, sizeof(fb), "%s=%#x",
			(Lf->access == 'r') ? "rd" : "wr",
			(Lf->access == 'r') ?  rp  :  wp);
		    (void) enter_nma(fb);
		}
		Lf->off_def = 1;
		break;
#endif	/* HPUXV>=900 */

	    case N_MVFS:
		/* The location of the file size isn't known. */
		break;
	    case N_NFS:

#if	HPUXV<1030
		Lf->sz = (SZOFFTYPE)r.r_nfsattr.na_size;
#else	/* HPUXV>=1030 */
		Lf->sz = (SZOFFTYPE)r.r_attr.va_size;
#endif	/* HPUXV<1030 */

		Lf->sz_def = 1;
		break;

#if	defined(HASVXFS)
	    case N_VXFS:
		/* set in read_vxnode() */
		break;
#endif	/* defined(HASVXFS) */

	    case N_SPEC:
	    case N_REGLR:
		if ((type == VCHR || type == VBLK) && !Fsize)
		    Lf->off_def = 1;
		else if (ins) {
		    Lf->sz = (SZOFFTYPE)i.i_size;
		    Lf->sz_def = 1;
		}
		break;
	    }
	}
/*
 * Record link count.
 */
	if (Fnlink) {
	    switch(Ntype) {

# if	defined(HAS_AFS)
	    case N_AFS:
		Lf->nlink = an.nlink;
		Lf->nlink_def = an.nlink_st;
		break;
# endif	/* defined(HAS_AFS) */

	    case N_MVFS:
		/* The location of the link count isn't known. */
		break;
	    case N_NFS:

#if	HPUXV<1030
		Lf->nlink = r.r_nfsattr.na_nlink;
#else	/* HPUXV>=1030 */
		Lf->nlink = r.r_attr.va_nlink;
#endif	/* HPUXV<1030 */

		Lf->nlink_def = 1;
		break;

# if	HPUXV>=1000
	    case N_CDFS:		/* no link count? */
		break;
# endif	/* HPUXV>=1000 */

	    case N_FIFO:
	    case N_PIPE:

# if	HPUXV>=1000
		if (vats) {
		    Lf->nlink = (long)vat.va_nlink;
		    Lf->nlink_def = 1;
		}
# endif	/* HPUXV>=1000 */

		break;

# if	defined(HASVXFS)
	    case N_VXFS:
		/* set in read_vxnode() */
		break;
# endif	/* defined(HASVXFS) */

	    case N_SPEC:
	    default:
		if (ins) {
		    Lf->nlink = (long)i.i_nlink;
		    Lf->nlink_def = 1;
		}
		break;
	    }
	    if (Nlink && Lf->nlink_def && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}
/*
 * Record an NFS file selection.
 */
	if (Ntype == N_NFS && Fnfs)
	    Lf->sf |= SELNFS;
/*
 * Save the file system names.
 */
	if (vfs) {
	    Lf->fsdir = vfs->dir;
	    Lf->fsdev = vfs->fsname;

#if	defined(HASFSINO)
	    Lf->fs_ino = vfs->fs_ino;
#endif	/* defined(HASFSINO) */

	}
/*
 * Save the device numbers and their states.
 *
 * Format the vnode type, and possibly the device name.
 */
	Lf->dev = dev;
	Lf->dev_def = devs;
	Lf->rdev = rdev;
	Lf->rdev_def = rdevs;
	switch (type) {
	case VNON:
	    ty ="VNON";
	    break;
	case VREG:
	case VDIR:
	    ty = (type == VREG) ? "VREG" : "VDIR";
	    break;
	case VBLK:
	    ty = "VBLK";
	    Ntype = N_BLK;
	    break;
	case VCHR:
	    ty = "VCHR";
	    Ntype = N_CHR;
	    break;
	case VLNK:
	    ty = "VLNK";
	    break;

#if	defined(VSOCK)
	case VSOCK:
	    ty = "SOCK";
	    break;
#endif	/* defined(VSOCK) */

	case VBAD:
	    ty = "VBAD";
	    break;
	case VFIFO:
	    switch (Ntype) {

#if	HPUXV>=1000
	    case N_FIFO:
		ty = "FIFO";
		break;
	    case N_PIPE:
		ty = "PIPE";
		break;
#endif	/* HPUXV>=1000 */

	    default:
		ty = "FIFO";
	    }
	    break;
	default:
	    (void) snpf(Lf->type, sizeof(Lf->type), "%04o", (type & 0xfff));
	    ty = (char *)NULL;
	}
	if (ty)
	    (void) snpf(Lf->type, sizeof(Lf->type), "%s", ty);
	Lf->ntype = Ntype;

#if	defined(HASBLKDEV)
/*
 * If this is a VBLK file and it's missing an inode number, try to
 * supply one.
 */
	if ((Lf->inp_ty == 0) && (type == VBLK))
	    find_bl_ino();
#endif	/* defined(HASBLKDEV) */

/*
 * If this is a VCHR file and it's missing an inode number, try to
 * supply one.
 */
	if ((Lf->inp_ty == 0) && (type == VCHR))
	    find_ch_ino();
/*
 * Test for specified file.
 */
	if (Sfile && is_file_named((char *)NULL,
				   ((type == VCHR) || (type == VBLK) ? 1 : 0)))
	    Lf->sf |= SELNM;
/*
 * Enter name characters.
 */
	if (Namech[0])
	    enter_nm(Namech);
}



/*
 * readinode() - read inode
 */

static int
readinode(ia, i)
	KA_T ia;			/* inode kernel address */
	struct inode *i;		/* inode buffer */
{
	if (kread((KA_T)ia, (char *)i, sizeof(struct inode))) {
	    (void) snpf(Namech, Namechl, "can't read inode at %s",
		print_kptr(ia, (char *)NULL, 0));
	    return(1);
	}
	return(0);
}


/*
 * read_nmn() - read node's mvfsnode
 */

static int
read_nmn(na, ma, m)
	KA_T na;                        /* containing node's address */
	KA_T ma;                        /* kernel mvfsnode address */
	struct mvfsnode *m;             /* mvfsnode receiver */
{
	char tbuf[32];

	if (!ma || kread((KA_T)ma, (char *)m, sizeof(struct mvfsnode))) {
	    (void) snpf(Namech, Namechl, "node at %s: can't read mvfsnode: %s",
		print_kptr(na, tbuf, sizeof(tbuf)),
		print_kptr(ma, (char *)NULL, 0));
	    enter_nm(Namech);
	    return(1);
	}
	return(0);
}
