/*
 * dnode.c - NEXTSTEP and OPENSTEP node functions for lsof
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
static char *rcsid = "$Id: dnode.c,v 1.17 2006/03/28 22:08:17 abe Exp $";
#endif


#include "lsof.h"


#if	STEPV>=31
/*
 * Local definitions
 */

struct l_lockf {			/* local lock info */
	short type;			/* lock type */
	off_t start, end;		/* lock start and end */
	pid_t pid;			/* owning process ID */
	struct l_lockf *next;
};

struct l_svn {				/* shadow vnode */
	KA_T vp;			/* associated vnode */
	struct l_lockf *lp;		/* local lock chain */
	struct l_svn *next;
};

struct posix_proc {
	pid_t p_pid;
};
#define	POSIX_KERN	1
#include <ufs/lockf.h>

#define SVNHASH(n)	(((int)((long)(n) * 31415l) >> 5) & (LF_SVNODE_HSZ - 1))


/*
 * Local static variables
 */

static struct l_svn **Svnc = (struct l_svn **)NULL;
					/* local shadow vnode cache */
static int SvncSt = 0;			/* Svnc[] load status */


/*
 * Local function prototypes
 */

_PROTOTYPE(static char isvlocked,(KA_T vp));
_PROTOTYPE(static int load_svnc,(void));


/*
 * clr_svnc() - clear shadow vnode cache
 */

void
clr_svnc()
{
	struct l_lockf *lf, *lfn;
	int i;
	struct l_svn *sv, *svn;

	if (!Svnc || !SvncSt)
	    return;
	for (i = 0; i < LF_SVNODE_HSZ; i++) {
	    if (!(sv = Svnc[i]))
		continue;
	    do {
		if ((lf = sv->lp)) {
		    do {
			lfn = lf->next;
			(void) free((FREE_P *)lf);
		    } while ((lf = lfn));
		}
		svn = sv->next;
		(void) free((FREE_P *)sv);
	    } while ((sv = svn));
	    Svnc[i] = (struct l_svn *)NULL;
	}
	SvncSt = 0;
}


/*
 * isvlocked() - is vnode locked?
 */

static char
isvlocked(vp)
	KA_T vp;			/* vnode's kernel address */
{
	int i;
	struct l_lockf *lp;
	struct l_svn *sv;

	if (!Svnc || !SvncSt) {
	    if (!load_svnc())
		return(' ');
	}
/*
 * Hash the vnode address and see if there's a shadow (lock) vnode structure
 * assigned to it.
 */
	i = SVNHASH(vp);
	for (sv = Svnc[i]; sv; sv = sv->next) {
	    if ((KA_T)sv->vp == vp)
		break;
	}
	if (!sv)
	    return(' ');
/*
 * Search the lock owners represented by the shadow vnode's lock chain
 * for this process.
 */
	for (lp = sv->lp; lp; lp = lp->next) {
	    if (lp->pid == (pid_t)Lp->pid) {
		if (lp->start == 0 && lp->end == 0x7fffffff)
		    i = 1;
		else
		    i = 0;
		if (lp->type == F_RDLCK)
		    return(i ? 'R' : 'r');
		else if (lp->type == F_WRLCK)
		    return(i ? 'W' : 'w');
		return(' ');
	    }
	}
	return(' ');
}


/*
 * load_svnc() - load the shadow vnode cache
 */

int
load_svnc()
{
	int i, j;
	static KA_T kp = (KA_T)NULL;
	struct lockf lf, *lp;
	struct l_lockf *lsf;
	struct l_svn *lsv;
	struct posix_proc p;
	struct lf_svnode *sn, *sp[LF_SVNODE_HSZ], sv;

	if (Svnc && SvncSt)
	    return(1);
/*
 * Get the shadow vnode hash table address from the kernel.
 */
	if (!kp) {
	    if (get_Nl_value("lfsvh", Drive_Nl, &kp) < 0 || !kp)
		return(0);
	}
/*
 * Define local hash buckets, if necessary.
 */
	if (!Svnc) {
	    if (!(Svnc = (struct l_svn **)calloc(sizeof(struct l_svn *),
						LF_SVNODE_HSZ)))
	    {
		(void) fprintf(stderr,
		    "%s: no space for %d local shadow vnode hash buckets\n",
		    Pn, LF_SVNODE_HSZ);
		Exit(1);
	    }
	}
/*
 * Search the hash buckets of the shadow vnode table.
 */
	if (kread(kp, (char *)&sp, sizeof(sp)))
	    return(0);
	for (i = 0; i < LF_SVNODE_HSZ; i++) {
	    if (!(sn = sp[i]))
		continue;
	    do {

	    /*
	     * Duplicate the chain of shadow vnodes in the bucket.
	     */
		if (kread((KA_T)sn, (char *)&sv, sizeof(sv))
		||  !sv.lf_vnodep
		||  !sv.lf_lockfp)
		    break;
	    /*
	     * Allocate and initialize a local shadow vnode structure.
	     */
		if (!(lsv = (struct l_svn *)malloc(sizeof(struct l_svn)))) {
		    (void) fprintf(stderr,
			"%s: no space for local shadow vnode -- PID: %ld\n",
			Pn, Lp->pid);
		    Exit(1);
		}
		lsv->vp = (KA_T)sv.lf_vnodep;
		lsv->lp = (struct l_lockf *)NULL;
		lsv->next = (struct l_svn *)NULL;
		lp = sv.lf_lockfp;
		do {

		/*
		 * Duplicate the lock chain for this shadow vnode.
		 */
		    if (kread((KA_T)lp, (char *)&lf, sizeof(lf)))
			break;
		    if (!lf.lf_posix_procp
		    ||  kread((KA_T)lf.lf_posix_procp, (char *)&p, sizeof(p))
		    ||  !p.p_pid)
			continue;
		    if (!(lsf=(struct l_lockf *)malloc(sizeof(struct l_lockf))))
		    {
			(void) fprintf(stderr,
			    "%s: no space for local lock struct -- PID: %ld\n",
			    Pn, Lp->pid);
			Exit(1);
		    }
		    lsf->type = lf.lf_type;
		    lsf->start = lf.lf_start;
		    lsf->end = lf.lf_end;
		    lsf->pid = (pid_t)p.p_pid;
		    lsf->next = lsv->lp;
		    lsv->lp = lsf;
		} while ((lp = lf.lf_next));
	    /*
	     * Link the shadow vnode to its local hash bucket.
	     */
		j = SVNHASH(lsv->vp);
		lsv->next = Svnc[j];
		Svnc[j] = lsv;
	    } while ((sn = sv.lf_next));
	}
	SvncSt = 1;
	return(1);
}
#endif	/* STEPV>=31 */


/*
 * process_node() - process vnode
 */

void
process_node(va)
	KA_T va;			/* vnode kernel space address */
{
	dev_t dev, rdev;
	int devs = 0;
	static int ft = 1;
	static KA_T fvops = (KA_T)0;
	struct inode i;
	int ins = 0;
	static KA_T nvops = (KA_T)0;
	struct rnode r;
	int rdevs = 0;
	struct vnode rv;
	struct snode s;
	static KA_T svops = (KA_T)0;
	char tbuf[32], *ty;
	static KA_T uvops = (KA_T)0;
	enum vtype type;
	static struct vnode *v = (struct vnode *)NULL;

#if	defined(HAS_AFS)
	static int afs = 0;		/* AFS test status: -1 = no AFS
					 *		     0 = not tested
					 *		     1 = AFS present */
	struct afsnode an;
	static KA_T avops = (KA_T)0;
#endif	/* defined(HAS_AFS) */

/*
 * Read the vnode.
 */
	if (!va) {
	    enter_nm("no vnode address");
	    return;
	}
/*
 * Read the vnode.
 */
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

# if	defined(HASNCACHE)
	Lf->na = va;
# endif	/* defined(HASNCACHE) */

# if	defined(HASFSTRUCT)
	Lf->fna = va;
	Lf->fsv |= FSV_NI;
# endif	/* defined(HASFSTRUCT) */

/*
 * Get vnode operations addresses, as required.
 */
	if (ft) {

#if	defined(HAS_AFS)
	    (void) get_Nl_value("avops", Drive_Nl, &avops);
#endif	/* defined(HAS_AFS) */

	    (void) get_Nl_value("fvops", Drive_Nl, &fvops);
	    (void) get_Nl_value("nvops", Drive_Nl, &nvops);
	    (void) get_Nl_value("svops", Drive_Nl, &svops);
	    (void) get_Nl_value("uvops", Drive_Nl, &uvops);
	    ft = 0;
	}
/*
 * Determine the vnode type.
 */
	if ((uvops && (KA_T)v->v_op == uvops)
	||  (svops && (KA_T)v->v_op == svops))
	    Ntype = N_REGLR;
	else if (nvops && (KA_T)v->v_op == nvops)
	    Ntype = N_NFS;
	else if (fvops && (KA_T)v->v_op == fvops)
	    Ntype = N_FIFO;

#if	defined(HAS_AFS)
	/*
	 * Caution: this AFS test should be the last one.
	 */
	
	else if (avops) {
	    if ((KA_T)v->v_op == avops)
		Ntype = N_AFS;
	    else {

unknown_v_op:
		(void) snpf(Namech, Namechl,
		    "unknown file system type; v_op: %s",
			print_kptr((KA_T)v->v_op, (char *)NULL, 0));
		enter_nm(Namech);
		return;
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
		Ntype = N_AFS;
		AFSVfsp = (KA_T)v->v_vfsp;
		break;
	    case 1:
		if ((KA_T)v->v_vfsp == AFSVfsp)
		    Ntype = N_AFS;
		else
		    goto unknown_v_op;
	    }
	}
#else	/* !defined(HAS_AFS) */
	else {
	    (void) snpf(Namech, Namechl, "unknown file system type; v_op: %s",
		print_kptr((KA_T)v->v_op, (char *)NULL, 0));
	    enter_nm(Namech);
	    return;
	}
#endif	/* defined(HAS_AFS) */

/*
 * Determine the lock type.
 */
	if (v->v_shlockc || v->v_exlockc) {
	    if (FILEPTR && (FILEPTR->f_flag & FSHLOCK))
		Lf->lock = 'R';
	    else if (FILEPTR && (FILEPTR->f_flag & FEXLOCK))
		Lf->lock = 'W';
	    else

#if	STEPV>=31
		Lf->lock = isvlocked(va);
#else	/* STEPV<31 */
		Lf->lock = ' ';
#endif	/* STEPV>=31 */

	}
/*
 * Read the inode, rnode, snode, or vcache struct.
 */
	switch (Ntype) {

#if	defined(HAS_AFS)
	case N_AFS:
	    if (readafsnode(va, v, &an))
		return;
	    break;
#endif	/* defined(HAS_AFS) */

	case N_NFS:
	    if (!v->v_data || readrnode((KA_T)v->v_data, &r)) {
		(void) snpf(Namech, Namechl,
		    "vnode at %s: can't read rnode (%s)",
		    print_kptr(va, tbuf, sizeof(tbuf)),
		    print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		enter_nm(Namech);
		return;
	    }
	    break;
	case N_REGLR:
	default:

	/*
	 * VBLK, VCHR and VFIFO vnodes point to an snode.  The snode's s_realvp
	 * usually points to a real vnode, which points to an inode.
	 */
	    if (v->v_type == VBLK || v->v_type == VCHR || v->v_type == VFIFO) {
		if (!v->v_data || readsnode((KA_T)v->v_data, &s)) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read snode(%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		if (s.s_realvp) {
		    if (readvnode((KA_T)s.s_realvp, &rv)) {
			(void) snpf(Namech, Namechl,
			    "snode at %s: can't read real vnode (%s)",
			    print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
			    print_kptr((KA_T)s.s_realvp, (char *)NULL, 0));
			enter_nm(Namech);
			return;
		    }
		    if (!rv.v_data || readinode((KA_T)rv.v_data, &i)) {
			(void) snpf(Namech, Namechl,
			    "snode at %s: can't read inode (%s)",
			    print_kptr((KA_T)v->v_data, tbuf, sizeof(tbuf)),
			    print_kptr((KA_T)rv.v_data, (char *)NULL, 0));
			enter_nm(Namech);
			return;
		    }
		    ins = 1;
		}
		break;
	    } else {
		if (!v->v_data || readinode((KA_T)v->v_data, &i)) {
		    (void) snpf(Namech, Namechl,
			"vnode at %s: can't read inode (%s)",
			print_kptr(va, tbuf, sizeof(tbuf)),
			print_kptr((KA_T)v->v_data, (char *)NULL, 0));
		    enter_nm(Namech);
		    return;
		}
		ins = 1;
	    }
	}
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

	case N_NFS:
	    dev = r.r_attr.va_fsid;
	    devs = 1;
	    if (dev & 0x8000)
		dev |= 0xff00;
	    break;
	case N_FIFO:
	case N_REGLR:
	    if (ins) {
		dev = i.i_dev;
		devs = 1;
	    }
	    if ((v->v_type == VBLK) || (v->v_type == VCHR)) {
		rdev = v->v_rdev;
		rdevs = 1;
	    }
	}
	type = v->v_type;
/*
 * Obtain the inode number.
 */
	switch(Ntype) {

#if	defined(HAS_AFS)
	case N_AFS:
	    if (an.ino_st) {
		Lf->inode = (INODETYPE)an.inode;
		Lf->inp_ty = 1;
	    }
	    break;
#endif	/* defined(HAS_AFS) */

	case N_NFS:
	    Lf->inode = (INODETYPE)r.r_attr.va_nodeid;
	    Lf->inp_ty = 1;
	    break;
	case N_FIFO:
	case N_REGLR:
	    if (ins) {
		Lf->inode = (INODETYPE)i.i_number;
		Lf->inp_ty = 1;
	    }
	}
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

	    case N_NFS:
		Lf->sz = (SZOFFTYPE)r.r_attr.va_size;
		Lf->sz_def = 1;
		break;
	    case N_FIFO:
		Lf->off_def = 1;
		break;
	    case N_REGLR:
		if (type == VREG || type == VDIR) {
		    if (ins) {
			Lf->sz = (SZOFFTYPE)i.i_size;
			Lf->sz_def = 1;
		    }
		}
		else if ((type == VCHR || type == VBLK) && !Fsize)
		    Lf->off_def = 1;
		break;
	    }
	}
/*
 * Record the link count.
 */
	if (Fnlink) {

#if	defined(HAS_AFS)
	    case N_AFS:
		Lf->nlink = an.nlink;
		Lf->nlink_def = an.nlink_st;
		break;
#endif	/* defined(HAS_AFS) */

	    switch (Ntype) {
	    case N_NFS:
		Lf->nlink = (long)r.r_attr.va_nlink;
		Lf->nlink_def = 1;
		break;
	    case N_REGLR:
		if (ins) {
		    Lf->nlink = (long)i.i_nlink;
		    Lf->nlink_def = 1;
		}
		break;
	    }
	    if (Lf->nlink_def && Nlink && (Lf->nlink < Nlink))
		Lf->sf |= SELNLINK;
	}
/*
 * Record an NFS file selection.
 */
	if (Ntype == N_NFS && Fnfs)
	    Lf->sf |= SELNFS;
/*
 * Defer file system info lookup until printname().
 */
	Lf->lmi_srch = 1;
/*
 * Save the device numbers and their states.
 *
 * Format the vnode type.
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
#endif

	case VBAD:
	    ty = "VBAD";
	    break;
	case VFIFO:
	    ty = "FIFO";
	    break;
	default:
	    (void) snpf(Lf->type, sizeof(Lf->type), "%04o", (type & 0xfff));
	    ty = NULL;
	}
	if (ty)
	    (void) snpf(Lf->type, sizeof(Lf->type), ty);
	Lf->ntype = Ntype;
/*
 * If this is a VBLK file and it's missing an inode number, try to
 * supply one.
 */
	if ((Lf->inp_ty == 0) && (Lf->ntype == N_BLK))
	    find_bl_ino();
/*
 * If this is a VCHR file and it's missing an inode number, try to
 * supply one.
 */
	if ((Lf->inp_ty == 0) && (Lf->ntype == N_CHR))
	    find_ch_ino();
/*
 * Test for specified file.
 */
	if (Sfile && is_file_named((char *)NULL,
				   ((type == VCHR) || (type == VBLK)) ? 1 : 0))
	    Lf->sf |= SELNM;
/*
 * Enter name characters.
 */
	if (Namech[0])
	    enter_nm(Namech);
}
