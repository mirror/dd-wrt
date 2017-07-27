/*
 * dnode.c - DEC OSF/1, Digital UNIX, Tru64 UNIX node functions for lsof
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
static char *rcsid = "$Id: dnode.c,v 1.23 2006/03/27 20:40:59 abe Exp $";
#endif


#include "lsof.h"


/*
 * Local definitions
 */

#if	ADVFSV>=500
/*
 * AdvFS (MSFS) definitions for AdvFS version 5.0 and above.
 */

struct fs_stat {			/* file system stat(2) info structure
					 * for AdvFS 5.0 and above */
	unsigned int num;		/* node number */
	unsigned int d1;
	mode_t d2;
	uid_t d3;
	gid_t d4;
	dev_t rdev;			/* character or block device number */
	off_t size;			/* file size */

# if	defined(__arch32__)
	unsigned int d5;
# endif	/* defined(__arch32__) */

	time_t d6;
	int d7;
	time_t d8;
	int d9;
	time_t d10;
	int d11;
	unsigned int d12[5];
	unsigned short nlink;		/* link count */
};

struct fsContext {			/* file system context for AdvFS 5.0
					 * and above */
	short d1[2];
	unsigned int d2[2];
	long d3;
	int d4[2];
	lock_data_t d5;
	long d6;
	simple_lock_data_t d7;
	unsigned int d8[2];
	long d9;
	struct fs_stat st;		/* file stats */
};

struct advfsnode {			/* AdvFS (MSFS) node definition for
					 * AdvFS 5.0 and above */
	unsigned long d1;
	struct fsContext *a_con;	/* context pointer */
};
#endif	/* ADVFSV>=500 */

#if	DUV>=50000
typedef struct cnode {			/* CFS node structure definition for
					 * Tru64 UNIX 5.0 and above */
        udecl_simple_lock_data(, d1)
	unsigned int d2;
	time_t d3;
	unsigned long d4[3];

# if	DUV<50100
	int d5[2];
	off_t d6;
# endif	/* DUV<50100 */

	unsigned long d7;
        vattr_t          c_attr;        /* 96:Cached vnode attributes */
} cnode_t;
#endif	/* DUV>=50000 */

struct l_lock {				/* local lock info */
	struct eflock set;		/* lock data */
	struct l_lock *next;
};

struct l_flinfo {			/* local file lock info */
	struct vnode *vp;		/* identity of locked vnode */
	struct l_lock *lp;		/* lock information */
	struct l_flinfo *next;
};

#define	L_FLINFO_HSZ	256		/* local file lock information hash
					 * table size (must be a power of 2) */
#define L_FLINFO_HASH(va)	(((int)((long)(va) * 31415L) >> 5) & (L_FLINFO_HSZ - 1))


/*
 * Local static variables
 */

static struct l_flinfo **Flinfo = (struct l_flinfo **)NULL;
					/* local file lock hash buckets */
static int FlinfoSt = 0;		/* Flinfo[] load status */


/*
 * Local function prototypes
 */

_PROTOTYPE(static char isvlocked,(struct vnode *vp));
_PROTOTYPE(static int load_flinfo,(void));
_PROTOTYPE(static int readvnode,(KA_T va, struct vnode *v));
_PROTOTYPE(static void get_proc_sz,(struct procnode *pn));


/*
 * clr_flinfo() - clear local file lock table information
 */

void
clr_flinfo()
{
	struct l_lock *lf, *lfn;
	int i;
	struct l_flinfo *fi, *fin;

	if (!Flinfo && !FlinfoSt)
	    return;
	for (i = 0; i < L_FLINFO_HSZ; i++) {
	    if (!(fi = Flinfo[i]))
		continue;
	    do {
		if ((lf = fi->lp)) {
		    do {
			lfn = lf->next;
			(void) free((FREE_P *)lf);
		    } while ((lf = lfn));
		}
		fin = fi->next;
		(void) free((FREE_P *)fi);
	    } while ((fi = fin));
	    Flinfo[i] = (struct l_flinfo *)NULL;
	}
	FlinfoSt = 0;
}


/*
 * get_proc_sz() - get size of /proc file system file
 */

static void
get_proc_sz(pn)
	struct procnode *pn;		/* pointer to procnode */
{
	struct vm_map m;
	struct proc *p;
	KA_T pa;
	int px;
	struct task t;
/*
 * Search for procnode's process by PID.
 */
	for (p = Ps, px = 0; px < Psn; p++, px++) {
		if (p->p_pid == pn->prc_pid)
			break;
	}
	if (px >= Psn)
		return;
/*
 * Get the task structure address, then read the task structure.  Set
 * the procnode's file size from the memory map information in the task
 * structure.
 */

# if	DUV<30000
	if (!(pa = (KA_T)p->task))
		return;
# else	/* DUV>=30000 */
	if (!(pa = Pa[px]))
		return;
	pa = (KA_T)((char *)pa - sizeof(t));
# endif	/* DUV<30000 */

	if (kread(pa, (char *)&t, sizeof(t)))
		return;
	if (!t.map || kread((KA_T)t.map, (char *)&m, sizeof(m)))
		return;
	Lf->sz = (SZOFFTYPE)m.vm_size;
	Lf->sz_def = 1;
}


/*
 * isvlocked() - is vnode locked?
 */

static char
isvlocked(vp)
	struct vnode *vp;		/* vnode's kernel address */
{
	struct l_flinfo *fp;
	int i, l;
	struct l_lock *lp;

	if (!Flinfo || !FlinfoSt) {
	    if (!load_flinfo())
		return(' ');
	}
/*
 * Hash the vnode address and see if there's a local file lock information
 * structure for it.
 */
	i = L_FLINFO_HASH(vp);
	for (fp = Flinfo[i]; fp; fp = fp->next) {
	    if (fp->vp == vp)
		break;
	}
	if (!fp)
	    return(' ');
/*
 * Search the vnode's lock list for one held by this process.
 */
	for (lp = fp->lp; lp; lp = lp->next) {
	    if (lp->set.l_rsys || lp->set.l_pid != (pid_t)Lp->pid)
		continue;
	    if (lp->set.l_whence == 0 && lp->set.l_start == 0
	    &&  ((lp->set.l_len == 0x8000000000000000)
	    ||   (lp->set.l_len == 0x7fffffffffffffff)))
		l = 1;
	    else
		l = 0;
	    if (lp->set.l_type == F_WRLCK)
		return(l ? 'W' : 'w');
	    else if (lp->set.l_type == F_RDLCK)
		return(l ? 'R' : 'r');
	    return(' ');
	}
	return(' ');
}


/*
 * load_flinfo() - load local file lock information
 */

static int
load_flinfo()
{
	struct flino fi;
	struct filock fl;
	KA_T fif, fip, flf, flp;
	int i;
	struct l_flinfo *lfi;
	struct l_lock *ll;
	KA_T v;

	if (Flinfo && FlinfoSt)
	    return(1);
/*
 * Get kernel fids chain pointer.
 */
	if (get_Nl_value("fids", Drive_Nl, &v) < 0 || !v
	||  kread((KA_T)v, (char *)&fip, sizeof(fip)))
	    return(0);
/*
 * Define local hash buckets, if necessary.
 */
	if (!Flinfo) {
	    if (!(Flinfo = (struct l_flinfo **)calloc(sizeof(struct flinfo *),
						      L_FLINFO_HSZ)))
	    {
		(void) fprintf(stderr,
		    "%s: can't allocate %d byte local lock hash buckets\n",
		    Pn, L_FLINFO_HSZ * sizeof(struct l_flinfo *));
		Exit(1);
	    }
	}
/*
 * Follow the fids chain.
 */
	if (!(fif = fip))
	    return(1);
    /*
     * Follow the filock chain for this fid entry.
     * Duplicate it via the lock file lock information hash buckets.
     */
	do {
	    if (kread(fip, (char *)&fi, sizeof(fi)))
		return(0);
	    if (!(flf = (KA_T)fi.fl_flck))
		continue;
	/*
	 * Allocate a local file lock information structure for this fid.
	 */
	    if (!(lfi = (struct l_flinfo *)malloc(sizeof(struct l_flinfo)))) {
		(void) fprintf(stderr,
		    "%s: no space for local vnode lock info struct\n", Pn);
		Exit(1);
	    }
	    lfi->vp = fi.vp;
	    lfi->lp = (struct l_lock *)NULL;
	    lfi->next = (struct l_flinfo *)NULL;
	/*
	 * Follow the flino's filock chain, duplicating it locally.
	 */
	    flp = flf;
	    do {
		if (kread(flp, (char *)&fl, sizeof(fl)))
		    break;
	    /*
	     * Allocate a local lock information structure and link it
	     * to the chain for its vnode.
	     */
		if (!(ll = (struct l_lock *)malloc(sizeof(struct l_lock)))) {
		    (void) fprintf(stderr,
			"%s: no space for local lock struct\n", Pn);
		    Exit(1);
		}
		ll->next = lfi->lp;
		lfi->lp = ll;
		ll->set = fl.set;
	    } while ((flp = (KA_T)fl.next) && flp != flf);
	/*
	 * Link the file lock information structure to its hash bucket.
	 */
	    i = L_FLINFO_HASH(lfi->vp);
	    lfi->next = Flinfo[i];
	    Flinfo[i] = lfi;
	} while ((fip = (KA_T)fi.next) && fip != fif);
	FlinfoSt = 1;
	return(1);
}


/*
 * process_node() - process vnode
 */

void
process_node(va)
	KA_T va;			/* vnode kernel space address */
{
	struct advfsnode *a = (struct advfsnode *)NULL;
	struct cdnode *c = (struct cdnode *)NULL;
	dev_t dev, rdev;
	unsigned char devs = 0;
	unsigned char rdevs = 0;
	struct inode *i = (struct inode *)NULL;
	struct mfsnode *m = (struct mfsnode *)NULL;
	struct procnode *p = (struct procnode *)NULL;
	struct procfsid *pfi;
	struct rnode *r = (struct rnode *)NULL;
	struct spec_node *s = (struct spec_node *)NULL;
	struct spec_node sn;
	struct s5inode *s5 = (struct s5inode *)NULL;
	char *ty;
	enum vtype type;
	unsigned long ul;
	static struct vnode *v = (struct vnode *)NULL;
	struct l_vfs *vfs;

#if	DUV>=30000
	struct fifonode *f = (struct fifonode *)NULL;
	struct fifonode fn;
	static struct vnode *fv = (struct vnode *)NULL;
#endif	/* DUV>=30000 */

#if	DUV>=50000
	cnode_t *cn = (cnode_t *)NULL;
	struct fsContext fsc;
	int fscs = 0;
#endif	/* DUV>=50000 */

/*
 * Read the vnode.
 */
	if (!va) {
	    enter_nm("no vnode address");
	    return;
	}
	if (!v) {

	/*
	 * Allocate space for the Digital UNIX vnode.
	 */
	    if (!(v = (struct vnode *)malloc(sizeof(struct vnode)-1+Vnmxp))) {
		(void) fprintf(stderr, "%s: no space for vnode buffer\n", Pn);
		Exit(1);
	    }

#if	DUV>=30000
	    if (!(fv = (struct vnode *)malloc(sizeof(struct vnode)-1+Vnmxp))) {
		(void) fprintf(stderr, "%s: no space for fvnode buffer\n", Pn);
		Exit(1);
	    }
#endif	/* DUV>=30000 */

	}
	if (readvnode(va, v)) {
	    enter_nm(Namech);
	    return;
	}

#if	defined(HASNCACHE)
	Lf->na = va;
# if	defined(HASNCVPID)
	Lf->id = (unsigned long)v->v_id;
# endif	/* defined(HASNCVPID) */
#endif	/* defined(HASNCACHE) */

#if	defined(HASFSTRUCT)
	Lf->fsv |= FSV_NI;
	Lf->fna = va;
#endif	/* defined(HASFSTRUCT) */

/*
 * Get the mount structure and determine the vnode type.
 */
	if (!v->v_mount)
	    vfs = (struct l_vfs *)NULL;
	else
	    vfs = readvfs((KA_T)v->v_mount);
	if (vfs) {
	    switch (vfs->type) {
	    case MOUNT_NFS:

#if	defined(MOUNT_NFS3)
	    case MOUNT_NFS3:
#endif	/* defined(MOUNT_NFS3) */

		Ntype = N_NFS;
		break;
	    }
	    if (Ntype == N_REGLR) {
		switch (v->v_type) {
		case VFIFO:
		    Ntype = N_FIFO;
		    break;
		}
	    }
	}
/*
 * Determine the lock type.
 */
	if (FILEPTR && (FILEPTR->f_flag & FSHLOCK))
	    Lf->lock = 'R';
	else if (FILEPTR && (FILEPTR->f_flag & FEXLOCK))
	    Lf->lock = 'W';
	else
	    Lf->lock = isvlocked((struct vnode *)va);
/*
 * Define the specific Digital UNIX node pointer.
 */

#if	DUV>=30000
	if (Ntype == N_FIFO) {
	    if (v->v_fifonode
	    &&  !kread((KA_T)v->v_fifonode, (char *)&fn, sizeof(fn)))
		f = &fn;
	}
#endif	/* DUV>=30000 */

	switch (v->v_tag) {
	case VT_CDFS:
	    c = (struct cdnode *)v->v_data;
	    break;

#if	DUV>=50000
	case VT_CFS:
	    cn = (cnode_t *)v->v_data;
	    break;
#endif	/* DUV>=50000 */

	case VT_MFS:
	    m = (struct mfsnode *)v->v_data;
	    break;
	case VT_NFS:
	    r = (struct rnode *)v->v_data;
	    break;
	case VT_NON:

#if     DUV<20000
	    if (v->v_specinfo
	    &&  !kread((KA_T)v->v_specinfo, (char *)&sn, sizeof(sn)))
		s = &sn;
	    else
#else	/* DUV>=20000 */
# if	DUV>=30000
	    if (!f)
# endif	/* DUV>=30000 */
#endif  /* DUV<20000 */

		s = (struct spec_node *)v->v_data;
	    break;
	case VT_PRFS:
	    p = (struct procnode *)v->v_data;
	    break;
	case VT_S5FS:
	    s5 = (struct s5inode *)v->v_data;
	    break;
	case VT_MSFS:
	    a = (struct advfsnode *)v->v_data;

#if	ADVFSV>=500
	    if (a->a_con
	    &&  !kread((KA_T)a->a_con, (char *)&fsc, sizeof(fsc)))
		fscs = 1;
#endif	/* ADVFSV>=500 */

	    break;
	case VT_UFS:
	    i = (struct inode *)v->v_data;
	    break;
	default:
	    (void) snpf(Namech, Namechl, "unknown node type, v_tag=%d",
		v->v_tag);
	    enter_nm(Namech);
	    return;
	}
/*
 * Get device and type for printing.
 */
	type = v->v_type;
	if (a) {
	    if (vfs && vfs->dev) {
		dev = vfs->dev;
		devs = 1;
	    }
	    if ((type == VCHR) || (type == VBLK)) {

#if	ADVFSV>=500
		if (fscs) {
		    rdev = fsc.st.rdev;
		    rdevs = 1;
		}
#else	/* ADVFSV<500 */
		rdev = a->a_rdev;
		rdevs = 1;
#endif	/* ADVFSV>=500 */

	    }
	} else if (c) {
	    dev = c->cd_dev;
	    devs = 1;
	}

#if	DUV>=50000
	else if (cn) {
	    if (vfs && vfs->dev) {
		dev = vfs->dev;
		devs = 1;
	    }
	    if ((type == VCHR) || (type == VBLK)) {
		if (cn->c_attr.va_mask & AT_RDEV) {
		    rdev = cn->c_attr.va_rdev;
		    rdevs = 1;
		}
	    }
	}
#endif	/* DUV>=50000 */

	else if (i) {
	    if (i->i_dev) {
		dev = i->i_dev;
		devs = 1;
	    } else if (vfs && vfs->dev) {
		dev = vfs->dev;
		devs = 1;
	    }
	    if ((type == VCHR) || (type == VBLK)) {
		rdev = i->i_din.di_db[0];
		rdevs = 1;
	    }
	} else if (r) {
	    dev = r->r_attr.va_fsid;
	    devs = 1;
	    if ((type == VCHR) || (type == VBLK)) {
		rdev = r->r_attr.va_rdev;
		rdevs = 1;
	    }
	} else if (s) {
	    if (vfs && vfs->dev)
		dev = vfs->dev;
	    else
		dev = DevDev;
	    devs = 1;
	    rdev = s->sn_vattr.va_rdev;
	    rdevs = 1;
	    if (!lkupdev(&dev, &rdev, 0, 0) && HaveCloneMaj)
		rdev = makedev(CloneMaj, GET_MAJ_DEV(rdev));
	} else if (s5) {
	    dev = s5->i_dev;
	    devs = 1;
	} else if (f) {
	    if (vfs && vfs->dev) {
		dev = vfs->dev;
		devs = 1;
	    }
	}
/*
 * Obtain the inode number.
 */
	if (a) {

#if	ADVFSV>=500
	    if (fscs) {
		Lf->inode = (INODETYPE)fsc.st.num;
		Lf->inp_ty = 1;
	    }
#else	/* ADVFSV<500 */
	    Lf->inode = (INODETYPE)a->a_number;
	    Lf->inp_ty = 1;
#endif	/* ADVFSV>=500 */


#if	defined(HASTAGTOPATH)
	/*
	 * Record the Digital UNIX 4.0 or greater, ADVFS 4.0 or greater
	 * ADVFS sequence number for later use with tag_to_path().
	 */
	    Lf->advfs_seq = a->a_seq;
	    Lf->advfs_seq_stat = 1;
#endif	/* defined(HASTAGTOPATH) */

	} else if (c) {
	    Lf->inode = (INODETYPE)c->cd_number;
	    Lf->inp_ty = 1;
	}

#if	DUV>=50000
	else if (cn) {
	    if (cn->c_attr.va_mask & AT_NODEID) {
		Lf->inode = (INODETYPE)cn->c_attr.va_fileid;
		Lf->inp_ty = 1;
	    }
	}
#endif	/* DUV>=50000 */

	else if (i) {
	    Lf->inode = (INODETYPE)i->i_number;
	    Lf->inp_ty = 1;
	} else if (p) {
	    Lf->inode = (INODETYPE)((type == VDIR) ? PR_ROOTINO
					: p->prc_pid + PR_INOBIAS);
	    Lf->inp_ty = 1;
	} else if (r) {
	    Lf->inode = (INODETYPE)r->r_attr.va_fileid;
	    Lf->inp_ty = 1;
	} else if (s5) {
	    Lf->inode = (INODETYPE)s5->i_number;
	    Lf->inp_ty = 1;
	}

#if	DUV>=30000
	else if (f) {
	    Lf->inode = (INODETYPE)f->fn_fileid;
	    Lf->inp_ty = 1;
	}
#endif	/* DUV>=30000 */

/*
 * Obtain the file size.
 */
	if (Foffset) {
	    Lf->off_def = 1;

#if	DUV>=30000
	    if (Ntype == N_FIFO && f)
		Lf->off = (unsigned long)
		    (Lf->access == 'r') ? f->fn_rptr : f->fn_wptr;
#endif	/* DUV>=30000 */

	} else {
	    switch (Ntype) {
	    case N_FIFO:

#if	DUV>=30000
		if (f) {
		    Lf->sz = (SZOFFTYPE)f->fn_size;
		    Lf->sz_def = 1;
		} else if (!Fsize)
		    Lf->off_def = 1;
#else	/* DUV<30000 */
		if (!Fsize)
		    Lf->off_def = 1;
#endif	/* DUV>=30000 */

		break;
	    case N_NFS:
		if (r) {
		    Lf->sz = (SZOFFTYPE)r->r_attr.va_qsize;
		    Lf->sz_def = 1;
		}
		break;
	    case N_REGLR:
		if (type == VREG || type == VDIR) {
		    if (a) {

#if	ADVFSV>=500
			if (fscs) {
			    Lf->sz = (SZOFFTYPE)fsc.st.size;
			    Lf->sz_def = 1;
			}
#else	/* ADVFSV<500 */
			Lf->sz = (SZOFFTYPE)a->a_size;
			Lf->sz_def = 1;
#endif	/* ADVFSV>=500 */

		    } else if (c) {
			Lf->sz = (SZOFFTYPE)c->cd_size;
			Lf->sz_def = 1;
		    }

#if	DUV>=50000
		    else if (cn) {
			if (cn->c_attr.va_mask & AT_SIZE) {
			    Lf->sz = (SZOFFTYPE)cn->c_attr.va_qsize;
			    Lf->sz_def = 1;
			}
		    }
#endif	/* DUV>=50000 */

		    else if (i) {
			Lf->sz = (SZOFFTYPE)i->i_din.di_qsize;
			Lf->sz_def = 1;
		    } else if (m) {
			Lf->sz = (SZOFFTYPE)m->mfs_size;
			Lf->sz_def = 1;
		    } else if (p) {
			if (type != VDIR)
				get_proc_sz(p);
		    } else if (s5) {
			Lf->sz = (SZOFFTYPE)s5->i_size;
			Lf->sz_def = 1;
		    }
		} else if ((type == VBLK) || (type == VCHR) && !Fsize)
		    Lf->off_def = 1;
	    }
	}
	if (Fnlink) {
	    switch(Ntype) {
	    case N_FIFO:			/* no link count */
		break;
	    case N_NFS:
		Lf->nlink = (long)r->r_attr.va_nlink;
		Lf->nlink_def = 1;
		break;
	    case N_REGLR:

#  if	ADVFSV>=400
		if (a) {

#if	ADVFSV>=500
		    if (fscs) {
			Lf->nlink = (long)fsc.st.nlink;
			Lf->nlink_def = 1;
		    }
#else	/* ADVFSV<500 */
		    Lf->nlink = (long)a->a_nlink;
		    Lf->nlink_def = 1;
#endif	/* ADVFSV>=500 */

		    break;
		}
#  endif	/* ADVFSV>=400 */

		if (c) {
		    Lf->nlink = (long)c->cd_nlink;
		    Lf->nlink_def = 1;
		}

#if	DUV>=50000
		else if (cn) {
		    if (cn->c_attr.va_mask & AT_NLINK) {
			Lf->nlink = (long)cn->c_attr.va_nlink;
			Lf->nlink_def = 1;
		    }
		}
#endif	/* DUV>=50000 */

		else if (i) {
		    Lf->nlink = (long)i->i_din.di_nlink;
		    Lf->nlink_def = 1;
		} else if (s5) {
		    Lf->nlink = (long)s5->i_nlink;
		    Lf->nlink_def = 1;
		}
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
	    ty = "FIFO";

#if	DUV>=30000
	    if ((!devs || !dev) && f) {
		vfs = (struct l_vfs *)NULL;
		devs = Lf->dev_def = 0;
		ul = (unsigned long)v->v_fifonode;
		enter_dev_ch(print_kptr((KA_T)(ul&0xffffffff),(char *)NULL,0));
	    }
#endif	/* DUV>=30000 */

	    break;
	default:
	    (void) snpf(Lf->type, sizeof(Lf->type), "%04o", (type & 0xfff));
	    ty = (char *)NULL;
	}
	if (ty)
	    (void) snpf(Lf->type, sizeof(Lf->type), "%s", ty);
	Lf->ntype = Ntype;
/*
 * Save the file system names.
 */
	if (vfs) {
	    if (vfs->dir && *vfs->dir)
		Lf->fsdir = vfs->dir;
	    if (vfs->fsname && *vfs->fsname)
		Lf->fsdev = vfs->fsname;

#if	defined(HASFSINO)
	    if (vfs->fs_ino)
		Lf->fs_ino = vfs->fs_ino;
#endif	/* defined(HASFSINO) */

	}
/*
 * Handle some special cases:
 *
 * 	ioctl(fd, TIOCNOTTY) files;
 *	FIFOs (Digital UNIX V3.0 and higher);
 *	memory node files;
 *	/proc files.
 */

	if (type == VBAD)
	    (void) snpf(Namech, Namechl, "(revoked)");
	if (m) {
	    devs = Lf->dev_def = Lf->rdev_def = rdevs = 0;
	    (void) snpf(Namech, Namechl, "%#x", m->mfs_baseoff);
	    (void) enter_dev_ch("memory");
	} else if (p) {
	    devs = Lf->dev_def = Lf->rdev_def = rdevs = 0;
	    if (type != VDIR)
		(void) snpf(Namech, Namechl, "/proc/%d", p->prc_pid);
	    else
		(void) snpf(Namech, Namechl, "/proc");
	}

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
	if (p) {
	    if (Procsrch) {
		Procfind = 1;
		Lf->sf |= SELNM;
	    } else {
		for (pfi = Procfsid; pfi; pfi = pfi->next) {
		    if ((pfi->pid && pfi->pid == p->prc_pid)

#if	defined(HASPINODEN)
		    ||  (Lf->inp_ty == 1 && pfi->inode == Lf->inode)
#endif	/* defined(HASPINODEN) */

		    ) {
			    pfi->f = 1;
			    Lf->sf |= SELNM;
			    break;
		    }
		}
	    }
	} else {
	    if (Sfile && is_file_named((char *)NULL, (type == VCHR) ? 1 : 0))
		Lf->sf |= SELNM;
	}
/*
 * Enter name characters.
 */
	if (Namech[0])
	    enter_nm(Namech);
}


/*
 * readvnode() - read vnode
 */

static int
readvnode(va, v)
	KA_T va;			/* vnode kernel space address */
	struct vnode *v;		/* vnode buffer pointer */
{

	if (kread((KA_T)va, (char *)v, sizeof(struct vnode) - 1 + Vnmxp)) {
	    (void) snpf(Namech, Namechl, "can't read vnode at %s",
		print_kptr(va, (char *)NULL, 0));
	    return(1);
	}
	return(0);
}
